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
#include "data.h"
#include "scp_parser.h"
#include "fishing.h"
#include "globals.h"
#include "chars.h"
#include "items.h"
#include "basics.h"
#include "inlines.h"
#include "scripts.h"
#include "range.h"


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
	
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR( pc, INVALID )
	
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
		if((item[i]!=INVALID) && (nInPack) ) {
			item::CreateFromScript( item[i], pc->getBackpack());
			return item[i];
		}
	}
	return INVALID;
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

	data::seekMap(x, y,map);
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

	data::seekTile(((buffer[s][0x11]<<8)+buffer[s][0x12]), tile);
	if(!(strstr((char *) tile.name, "water") || strstr((char *) tile.name, "lava")))
	{
		data::seekLand(map.id, land);
		if (!(land.flags&TILEFLAG_WET))//not a "wet" tile
		{
			return false;
		}
	}
	
	return true;
}


void Fishing::target_fish( NXWCLIENT ps, P_TARGET t )
{

	NXWSOCKET  s=ps->toInt();
	P_CHAR pPlayer=ps->currChar();
	VALIDATEPC( pPlayer );

	Location charpos= pPlayer->getPosition();
	Location whereFish = t->getLocation();

	int px = whereFish.x;
	int py = whereFish.y;

	if( dist( charpos, whereFish )>6.0 )	// too far away from target
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
	pPlayer->playAction(0x0B);
	if (fishing_data.randomtime!=0)
		pPlayer->fishingtimer=rand()%fishing_data.randomtime+fishing_data.basetime;
	else
		pPlayer->fishingtimer=fishing_data.basetime;
	pPlayer->playSFX(0x023F);
//	pPlayer->hidden=UNHIDDEN;
	pPlayer->unHide();
	Fish(DEREF_P_CHAR(pPlayer));		
}


// LB: added fish stacking !!
void Fishing::Fish(CHARACTER i)
{
//	const int max_fish_piles = 1;		// attention: thats per fish *type*, so the efffective limit of piles is *3
//	const int max_fish_stacksize = 15;	// attention: rela max size = value+1

	int ii;
	int idnum;
	SI16 color;

	P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
	P_ITEM pc_bp = pc->getBackpack();
	NXWSOCKET s = pc->getSocket();
        
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
		default: {

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
				if((color <0x0002) || (color > 0x03E9)) color = 0x03E9;

				if( ((color&0x4000)>>14) + ((color&0x8000)>>15) )
				{
					color = DBYTE2WORD(0x01, rand()%255);
				}
			} else color=0;
		
			/**** end of exotic fish stuff stuff */
		
			//fishes_around_player=item::Find_items_around_player(i, 0x09, idnum, 2, 2, max_fish_piles, fish_sers); // lets search for fish in a 2*2 rectangle around the player
			
			//P_ITEM pFish;
			//if (fishes_around_player<=0) // no fish around -> spawn a new one
		
				//Luxor - Now fishes are spawned into backpack
			//{
				//Luxor: fishes should be read from items.xss
				//pFish=item::SpawnItem(i,1,"#",1,0x0900+idnum,(c1<<8)+c2,0);
			P_ITEM fish = item::CreateFromScript( "$item_fish" );
			VALIDATEPI(fish);
		
			fish->setColor(color);
			fish->setId( fish->getId() | idnum );
		
			if (ISVALIDPI(pc_bp))
				pc_bp->AddItem( fish );
			else {
				fish->MoveTo( charpos ); //Luxor bug fix
				fish->Refresh();
			}
		}
	}

	if(color>0)
	{
		pc->sysmsg(TRANSLATE("You pull out an exotic fish!"));
	}
	else
	{
		pc->sysmsg(TRANSLATE("You pull out a fish!"));
	}
}

