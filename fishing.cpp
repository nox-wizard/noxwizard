  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file
\brief Store all fishing related functions
\author Ripper
\date 31/01/2001
*/

#include "nxwcommn.h"
#include "sndpkg.h"
#include "debug.h"

int SpawnFishingMonster(P_CHAR pc, char* cScript, char* cList, char* cNpcID)
{
	/*This function gets the random monster number from
	the script and list specified.
	npcs::AddRespawnNPC passing the new number*/

	if (region[pc->region].priv&0x01 && SrvParms->guardsactive) //guarded
		return INVALID;

	char sect[512];
	int i=0,item[256]={0};

    cScpIterator* iter = NULL;
    char script1[1024];
	
	sprintf(sect, "SECTION %s %s", cList, cNpcID);
    iter = Scripts::Fishing->getNewIterator(sect);
    if (iter==NULL) return INVALID;

	int loopexit=0;
 	do
	{
  		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if((script1[0]!='}')&&(script1[0]!='{'))
		{
			item[i]=str2num(script1);
			i++;
		}
	}
 	while(script1[0]!='}' && (++loopexit < MAXLOOPS) );
 	safedelete(iter);

 	if(i>0)
 	{
  		i=rand()%(i);
		if(item[i]!=-1)
		{
			return DEREF_P_CHAR(npcs::AddRespawnNPC(pc,item[i]));
		}
	}
	return INVALID;
}

int SpawnFishingItem(NXWSOCKET  s,int nInPack, char* cScript, char* cList, char* cItemID)
{
 	/*This function gets the random item number from the list and recalls
 	  SpawnItemBackpack2 passing the new number*/
	char sect[512];
	int i=0,item[256]={0};
    
	cScpIterator* iter = NULL;
    char script1[1024];

	sprintf(sect, "SECTION %s %s", cList, cItemID);
    iter = Scripts::Fishing->getNewIterator(sect);
    if (iter==NULL) return -1;
	int loopexit=0;

 	do
 	{
  		strcpy(script1, iter->getEntry()->getFullLine().c_str());
  		if ((script1[0]!='}')&&(script1[0]!='{'))
  		{
			item[i]=str2num(script1);
   			i++;
		}
	}
	while( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	safedelete(iter);
 	if(i>0)
 	{
  		i=rand()%(i);
		if(item[i]!=-1)
		if(nInPack)
		{
			item::SpawnItemBackpack2(s,item[i],1);
			return item[i];
		}
	}
	return -1;
}

inline bool isWaterTarget(NXWSOCKET  s)
{
	tile_st tile;
	map_st map;
	land_st land;

	if(buffer[s][0x11]==0x17 && buffer[s][0x12]==0x98 || buffer[s][0x11]==0x17 && buffer[s][0x12]==0x9B || buffer[s][0x11]==0x17 && buffer[s][0x12]==0x9C || buffer[s][0x11]==0x17 && buffer[s][0x12]==0x99 || buffer[s][0x11]==0x17 && buffer[s][0x12]==0x97 || buffer[s][0x11]==0x17 && buffer[s][0x12]==0x9A)
	{
		return true;
	} 
	if(buffer[s][0x1]!=0x01) return false;

	int x = (buffer[s][0xB] << 8) + buffer[s][0xC];
	int y = (buffer[s][0xD] << 8) + buffer[s][0xE];

	map = Map->SeekMap0(x, y);
	switch(map.id)
	{
		//water tiles:
		case 0x00A8:
		case 0x00A9:
		case 0x00AA:
		case 0x00Ab:
		case 0x0136:
		case 0x0137:
		case 0x3FF0:
		case 0x3FF1:
		case 0x3FF2:
		case 0x2FF3:
			return true;
		default:
			break;
	}

	Map->SeekTile(((buffer[s][0x11]<<8)+buffer[s][0x12]), &tile);
	if(!(strstr((char *) tile.name, "water") || strstr((char *) tile.name, "lava")))
	{
		Map->SeekLand(map.id, &land);
		if (!(land.flag1&0x80))//not a "wet" tile
		{
			return false;
		}
	}
	
	return true;
}


void cFishing::FishTarget(NXWCLIENT ps)
{
	int px,py,cx,cy;
	NXWSOCKET  s=ps->toInt();
	P_CHAR pPlayer=ps->currChar();
	if (!pPlayer) return;

	Location charpos= pPlayer->getPosition();

	px=((buffer[s][0x0b]<<8)+(buffer[s][0x0c]%256));
	py=((buffer[s][0x0d]<<8)+(buffer[s][0x0e]%256));
	cx=abs((int)charpos.x - px);
	cy=abs((int)charpos.y - py);

	if( (cx>6) || (cy>6) )	// too far away from target
	{
		ps->sysmsg(TRANSLATE("You are too far away to reach that"));
		return;
	}

	if( !isWaterTarget(s) )	// target is not a water tile
	{
		ps->sysmsg(TRANSLATE("You can only fish in water !!"));
		return;
	}
	
	pPlayer->facexy(px, py);
	pPlayer->playAction(0x0b);
	if (fishing_data.randomtime!=0)
		pPlayer->fishingtimer=rand()%fishing_data.randomtime+fishing_data.basetime;
	else
		pPlayer->fishingtimer=fishing_data.basetime;
	soundeffect(s,0x02,0x3F);
//	pPlayer->hidden=0;
	pPlayer->unHide();
	Fish(currchar[s]);		
}


// LB: added fish stacking !!
void cFishing::Fish(CHARACTER i)
{
//	const int max_fish_piles = 1;		// attention: thats per fish *type*, so the efffective limit of piles is *3
//	const int max_fish_stacksize = 15;	// attention: rela max size = value+1

	int ii,b;
	int idnum;
	int s=calcSocketFromChar(i);
	int color,c1,c2;


    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
	P_ITEM pc_bp = pc->getBackpack();
        
	Location charpos= pc->getPosition();
	if(pc->stm<=2) //Luxor bug fix
	{
		pc->stm=0;
		pc->sysmsg( TRANSLATE("You are too tired to fish, you need to rest!"));
		return;
	}

	pc->stm-=2; // lose 2 stamina each cast.
		
	if(!pc->checkSkill( FISHING, 0, 1000))
	{
		pc->sysmsg( TRANSLATE("You fish for a while, but fail to catch anything."));
		return;
	}


	// New Random fishing up treasures and monsters...Ripper
	unsigned short skill=pc->skill[FISHING];
	int fishup=(RandomNum(0,100));
	switch (fishup)
		{
		case 0:
            if(skill>=200) 
			{ 
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "5" ); // random boots
				pc->sysmsg(  TRANSLATE("You fished up an old pair of boots!") ); 
			} 
            break;
		case 1:
            if(skill>=970) 
			{ 
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "1" ); // random paintings 
				pc->sysmsg(  TRANSLATE("You fished up an ancient painting!") ); 
			} 
            break;
		case 2:
            if(skill>=950) 
			{ 
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "2" ); // random weapons 
				pc->sysmsg(  TRANSLATE("You fished up an ancient weapon!") ); 
			} 
            break;
		case 3:
            if(skill>=950) 
			{ 
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "3" ); // random armor 
				pc->sysmsg(  TRANSLATE("You fished up an ancient armor!") ); 
			} 
            break;
		case 4:
            if(skill>=700) 
			{ 
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "4" ); // random treasure
				pc->sysmsg(  TRANSLATE("You fished up some treasure!") ); 
			} 
            break;
		case 5:
            if(skill>=400) 
			{ 
				if (SpawnFishingMonster( MAKE_CHAR_REF(i),"fishing.scp", "MONSTERLIST", "7" ) != -1) // random monsters 
					pc->sysmsg(  TRANSLATE("You fished up a hughe fish!") );
				else
					pc->sysmsg(  TRANSLATE("You wait for a while, but nothing happens"));
			} 
            break;
		case 6:
            if(skill>=800) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "6" ); // random chests
				pc->sysmsg(  TRANSLATE("You fished up an old chest!") );
			} 
            break;
		case 7:
            if(skill>=700) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "8" ); // random seashells
				pc->sysmsg(  TRANSLATE("You fished up a seashell!") );
			} 
            break;
		case 8:
            if(skill>=700) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "9" ); // random skulls
				pc->sysmsg(  TRANSLATE("You fished up a skull!") );
			} 
            break;
		case 9:
            if(skill>=900) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "10" ); // random nets
				pc->sysmsg(  TRANSLATE("You fished up a net!") );
			} 
            break;
		case 10:
            if(skill>=900) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "11" ); // random gold
				pc->sysmsg(  TRANSLATE("You fished up some gold!") );
			} 
            break;
		case 11:
            if(skill>=400) 
			{
				SpawnFishingItem( s, 1, "fishing.scp", "FISHLIST", "12" ); // random bones
				pc->sysmsg(  TRANSLATE("You fished up some bones!") );
			} 
            break;
		default:

	ii=rand()%3;
	idnum=0xCC+ii;

	///**** exotic fish stuff *****//
	double mv=-0.087087087*(float)pc->skill[FISHING]+100.087087087086; // gm fish -> 1/13 % probability on new spawn(!) to have exotic color, neophyte: 1/92 % probability, linear interpolation in between
	int no_idea_for_variable_name = (int) mv;
	if (no_idea_for_variable_name<=0) no_idea_for_variable_name=1; // prevent modulo crashes	
	if (rand()%no_idea_for_variable_name==0) 
	{ 		
		color=(charpos.x + charpos.y);
		color+= rand()%10;
		color= color%0x03E9; 
		c1= color >> 8;
		c2= color % 256;		
		if ((((c1<<8)+c2)<0x0002) || (((c1<<8)+c2)>0x03E9) )
		{
			c1=0x03;
			c2=0xE9;
		}
		b=((((c1<<8)+c2)&0x4000)>>14)+((((c1<<8)+c2)&0x8000)>>15);
		if (b)
		{
			c1=0x1;
			c2=rand()%255;
		}
	} else c1=c2=0;

	/**** end of exotic fish stuff stuff */

	//fishes_around_player=item::Find_items_around_player(i, 0x09, idnum, 2, 2, max_fish_piles, fish_sers); // lets search for fish in a 2*2 rectangle around the player
	
	//P_ITEM pFish;
	//if (fishes_around_player<=0) // no fish around -> spawn a new one

        //Luxor - Now fishes are spawned into backpack
	//{
		//Luxor: fishes should be read from items.xss
		//pFish=item::SpawnItem(i,1,"#",1,0x0900+idnum,(c1<<8)+c2,0);
		P_ITEM fish = item::CreateFromScript(-1, 8108);
		VALIDATEPI(fish);

		fish->color1= c1;
		fish->color2= c2;
		fish->id2= idnum;
                //<Luxor>
                if (ISVALIDPI(pc_bp)) {
                        fish->setCont(pc_bp);
                        if (!pc_bp->ContainerPileItem(fish))// try to pile
                                fish->SetRandPosInCont(pc_bp);
                } else
                        fish->MoveTo( charpos ); //Luxor bug fix
                //</Luxor>
                
		fish->Refresh();
/*	} else // fishes around ?
	{
		int c = -1;
		min=1234567;
		mc=0; // crash prevention if for some strange reason no min is found
		for (d=0;d<fishes_around_player;d++) // lets pick the smallest pile form the return list
		{
			c = calcItemFromSer( fish_sers[d] );
			if (c>-1)
			{
				ss=items[c].amount;
				if (ss<min) { min=ss; mc=c; }
			}
		}

		if (items[mc].amount>max_fish_stacksize) // if smaleest fish-stack > max_stacksize spawn a new fish anyway
		{
			if (fishes_around_player>=max_fish_piles)
			{
				sysmessage(s,TRANSLATE("you catch a fish, but no place for it left"));
				return;
			}
			
			//d=item::SpawnItem(-1,i,1,"#",1,0x09,idnum,c1,c2,0,0);
			//Luxor: Fishes should be created from items.xss
			P_ITEM fish = item::CreateFromScript(-1, 8108);
			VALIDATEPI(fish);

			//items[d].type=14;
			fish->color1= c1;
			fish->color2= c2;
			fish->id2= idnum;
			fish->setPosition( charpos );
			mapRegions->RemoveItem(d);
			mapRegions->AddItem(d); // lord Binary
			fish->Refresh();
		} else // if fish stack <=max_ -> just increase stack !!
		{
			P_ITEM pi= MAKE_ITEM_REF(c);
			VALIDATEPI(pi);

			pi->amount++;
			pi->Refresh();
		}
	} // end else fishes around
        */
        
	if(c2>0)
	{
		sysmessage(s,TRANSLATE("You pull out an exotic fish!"));
	}
	else
	{
	    sysmessage(s,TRANSLATE("You pull out a fish!"));
	}
	break;
	}
}

