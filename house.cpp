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
\brief House System Functions' Bodies
\todo a lot of functions need rewrite, now are commented out - Akron
\todo like other parts of NoX, houses should be moved into "House" namespace, IMHO - Akron
*/

#include "nxwcommn.h"
#include "network.h"
#include "sndpkg.h"
#include "debug.h"
#include "itemid.h"
#include "set.h"
#include "house.h"
#include "npcai.h"

std::map< SERIAL, P_CHAR > houses;

LOGICAL CheckBuildSite(int x, int y, int z, int sx, int sy);

/*!
\todo take a look to initialization, we could initialize the vector with the variables
*/
void mtarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt)
{
	UI08 multitarcrs[26]= { 0x99, 0x01, 0x40, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	targetok[s]=1;
	multitarcrs[2]=a1;
	multitarcrs[3]=a2;
	multitarcrs[4]=a3;
	multitarcrs[5]=a4;
	multitarcrs[18]=b1;
	multitarcrs[19]=b2;
	sysmessage(s, txt);
	Xsend(s, multitarcrs, 26);
//AoS/	Network->FlushBuffer(s);
}

/*!
\author Zippy
\brief Build an house

Triggered by double clicking a deed-> the deed's morex is read
for the house section in house.cpp. Extra items can be added
using HOUSE ITEM, (this includes all doors!) and locked "LOCK"
Space around the house with SPACEX/Y and CHAR offset CHARX/Y/Z
*/
void buildhouse(int s, int i)
{
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	int loopexit=0;//where they click, and the house/key items
	UI32 x, y, k, sx = 0, sy = 0, icount=0;
	signed char z;
	int hitem[100];//extra "house items" (up to 100)
	char sect[512];                         //file reading
	char itemsdecay = 0;            // set to 1 to make stuff decay in houses
	static int looptimes=0;         //for targeting
	int cx=0,cy=0,cz=8;             //where the char is moved to when they place the house (Inside, on the steps.. etc...)(Offset)
	int boat=0;//Boats
	int hdeed=0;//deed id #
	int norealmulti=0,nokey=0,othername=0;
	char name[512];
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	Location charpos= pc->getPosition();

	SI16 id = INVALID; //house ID

	if(LongFromCharPtr(buffer[s] +11)== INVALID) return; // do nothing if user cancels, avoids CRASH!

	hitem[0]=0;//avoid problems if there are no HOUSE_ITEMs by initializing the first one as 0
	if (i)
	{
		cScpIterator* iter = NULL;
		char script1[1024];
		char script2[1024];
		sprintf(sect, "SECTION HOUSE %d", i);//and BTW, .find() adds SECTION on there for you....

		iter = Scripts::House->getNewIterator(sect);
		if (iter==NULL) return;

		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if (!(strcmp(script1,"ID")))
				{
					id = hex2num(script2);
				}
				else if (!(strcmp(script1,"SPACEX")))
				{
					sx=str2num(script2)+1;
				}
				else if (!(strcmp(script1,"SPACEY")))
				{
					sy=str2num(script2)+1;
				}
				else if (!(strcmp(script1,"CHARX")))
				{
					cx=str2num(script2);
				}
				else if (!(strcmp(script1,"CHARY")))
				{
					cy=str2num(script2);
				}
				else if (!(strcmp(script1,"CHARZ")))
				{
					cz=str2num(script2);
				}
				else if( !(strcmp(script1, "ITEMSDECAY" )))
				{
					itemsdecay = str2num( script2 );
				}
				else if (!(strcmp(script1,"HOUSE_ITEM")))
				{
					hitem[icount]=str2num(script2);
					icount++;
				}
				else if (!(strcmp(script1, "HOUSE_DEED")))
				{
					hdeed=str2num(script2);
				}
				else if (!(strcmp(script1, "BOAT"))) boat=1;//Boats

				else if (!(strcmp(script1, "NOREALMULTI"))) norealmulti=1; // LB bugfix for pentas crashing client
				else if (!(strcmp(script1, "NOKEY"))) nokey=1;
				else if (!(strcmp(script1, "NAME")))
				{
					strcpy(name,script2);
					othername=1;
				}
			}
		}
		while ( (strcmp(script1,"}")) && (++loopexit < MAXLOOPS) );
		safedelete(iter);

		if (!id)
		{
			ErrOut("Bad house script # %i!\n",i);
			return;
		}
	}

	if(!looptimes)
	{
		if (i)
		{

			addid1[s]=0x40;addid2[s]=100;//Used in addtarget
			if (norealmulti) target(s, 0, 1, 0, 245, TRANSLATE("Select a place for your structure: ")); else
				mtarget(s, 0, 1, 0, 0, (id>>8) -0x40, (id%256), TRANSLATE("Select location for building."));

		}
		else
		{
			mtarget(s, 0, 1, 0, 0, addid1[s]-0x40, addid2[s], TRANSLATE("Select location for building."));
		}
		looptimes++;//for when we come back after they target something
		return;
	}
	if(looptimes)
	{
		looptimes=0;
		if(!pc->IsGM() && SrvParms->houseintown==0)
		{
			if ((region[pc->region].priv & RGNPRIV_GUARDED) && itemById::IsHouse(id) ) // popy
			{
			    sysmessage(s,TRANSLATE(" You cannot build houses in town!"));
			    return;
			}
		}

		x = ShortFromCharPtr(buffer[s] +11); //where they targeted
		y = ShortFromCharPtr(buffer[s] +13);
		z = buffer[s][16] + Map->TileHeight(ShortFromCharPtr(buffer[s] +17));

#define XBORDER 200
#define YBORDER 200

		//XAN : House placing fix :)
		if ( (( x<XBORDER || y <YBORDER ) || ( x>(UI32)((MapTileWidth*8)-XBORDER) || y >(UI32)((MapTileHeight*8)-YBORDER) ))  )
		{
			sysmessage(s, TRANSLATE("You cannot build your structure there!"));
			return;
		}


		/*
		if (ishouse(id1, id2)) // strict checking only for houses ! LB
		{
			if(!(CheckBuildSite(x,y,z,sx,sy)))
			{
				sysmessage(s,TRANSLATE("Can not build a house at that location (CBS)!"));
				return;
			}
		}*/


		for (k=0;k<sx;k++)//check the SPACEX and SPACEY to make sure they are valid locations....
		{
			for (UI32 l=0;l<sy;l++)
			{

				Location loc;

				loc.x=x+k;

				loc.y=y+l;

				loc.z=z;
				if (( !validNPCMove(x+k,y+l,z, pc ) ) &&
					((charpos.x != x+k)&&(charpos.y != y+l)))
					/*This will take the char making the house out of the space check, be careful
					you don't build a house on top of your self..... this had to be done So you
					could extra space around houses, (12+) and they would still be buildable.*/
				{
					sysmessage(s, TRANSLATE("You cannot build your stucture there."));
					return;
					//ConOut("Invalid %i,%i [%i,%i]\n",k,l,x+k,y+l);
				} //else ConOut("DEBUG: Valid at %i,%i [%i,%i]\n",k,l,x+k,y+l);

				P_ITEM pi_ii=findmulti(loc);
				if (ISVALIDPI(pi_ii) && !(norealmulti))
				{
					sysmessage(s,TRANSLATE("You cant build structures inside structures"));
					return;
				}
			}
		}

		//Boats ->
		if((id % 256)>=18)
			sprintf(temp,"%s's house",pc->getCurrentNameC());//This will make the little deed item you see when you have showhs on say the person's name, thought it might be helpful for GMs.
		else
			strcpy(temp, "a mast");
		if(norealmulti)
			strcpy(temp, name);
		//--^

		if (othername)
			strcpy(temp,name);

		if (id == INVALID)
			return;

		P_ITEM pHouse = item::CreateFromScript( "$item_hardcoded" );
		VALIDATEPI(pHouse);
		pHouse->setId( id );
		pHouse->setCurrentName( temp );

		pc->making=0;

		pHouse->setPosition(x, y, z);
		pHouse->setDecay( false );
		pHouse->setNewbie( false );
		pHouse->setDispellable( false );
		pHouse->more4 = itemsdecay; // set to 1 to make items in houses decay
		pHouse->morex=hdeed; // crackerjack 8/9/99 - for converting back *into* deeds
		pHouse->setOwnerSerial32(pc->getSerial32());
		if (pHouse->isInWorld()) 
		{
			regions::add(pHouse);
		}
		if (!hitem[0] && !boat)
		{
			pc->teleport();
			return;//If there's no extra items, we don't really need a key, or anything else do we? ;-)
		}

		if(boat) //Boats
		{
			if(!boats::Build(s,pHouse, id%256/*id2*/))
			{
				pHouse->deleteItem();
				return;
			}
		}

		if (i)
			archive::DeleItem(pc->fx1); // this will del the deed no matter where it is

		pc->fx1=-1; //reset fx1 so it does not interfere
		// bugfix LB ... was too early reseted

		P_ITEM pKey=NULL;
		P_ITEM pKey2=NULL;

		P_ITEM pBackPack = pc->getBackpack();

		//Key...
		//Altered key naming to include pc's name. Integrated backpack and bankbox handling (Sparhawk)
		if ((id%256 >=0x70) && (id%256 <=0x73))
		{
			sprintf(temp,"%s's tent key",pc->getCurrentNameC());
			pKey = item::CreateFromScript( "$item_iron_key", pBackPack ); //iron key for tents
			pKey2= item::CreateFromScript( "$item_iron_key", pBackPack );
		}
		else if(id%256 <=0x18)
		{
			sprintf(temp,"%s's ship key",pc->getCurrentNameC());
			pKey= item::CreateFromScript( "$item_bronze_key", pBackPack ); //Boats -Rusty Iron Key
			pKey2= item::CreateFromScript( "$item_bronze_key", pBackPack );
			
		}
		else
		{
			sprintf(temp,"%s's house key",pc->getCurrentNameC());
			pKey= item::CreateFromScript( "$item_gold_key", pBackPack ); //gold key for everything else;
			pKey2= item::CreateFromScript( "$item_gold_key", pBackPack );
		}

		VALIDATEPI( pKey )
		VALIDATEPI( pKey2 )

		pKey->Refresh();
		pKey2->Refresh();

		pHouse->st = pKey->getSerial32();		// Create link from house to housekeys to allow easy renaming of
		pHouse->st2= pKey2->getSerial32();	// house, housesign and housekeys without having to loop trough
														// all world items (Sparhawk)


		pKey->more1=pHouse->getSerial().ser1;//use the house's serial for the more on the key to keep it unique
		pKey->more2=pHouse->getSerial().ser2;
		pKey->more3=pHouse->getSerial().ser3;
		pKey->more4=pHouse->getSerial().ser4;
		pKey->type=ITYPE_KEY;
		pKey->setNewbie();
		pKey2->more1=pHouse->getSerial().ser1;//use the house's serial for the more on the key to keep it unique
		pKey2->more2=pHouse->getSerial().ser2;
		pKey2->more3=pHouse->getSerial().ser3;
		pKey2->more4=pHouse->getSerial().ser4;
		pKey2->type=ITYPE_KEY;
		pKey2->setNewbie();

		P_ITEM bankbox = pc->GetBankBox();
		if(bankbox!=NULL) // we sould add a key in bankbox only if the player has a bankbox =)
		{

			P_ITEM p_key3=item::CreateFromScript( "$item_gold_key" );
			VALIDATEPI(p_key3);
			p_key3->setCurrentName( "a house key" );
			p_key3->more1=pHouse->getSerial().ser1;
			p_key3->more2=pHouse->getSerial().ser2;
			p_key3->more3=pHouse->getSerial().ser3;
			p_key3->more4=pHouse->getSerial().ser4;
			p_key3->type=ITYPE_KEY;
			p_key3->setNewbie();
			bankbox->AddItem(p_key3);
		}
		if(nokey)
		{
			pKey->deleteItem(); // No key for .. nokey items
			pKey2->deleteItem(); // No key for .. nokey items
		}

		for (k=0;k<icount;k++)//Loop through the HOUSE_ITEMs
		{
			cScpIterator* iter = NULL;
			char script1[1024];
			char script2[1024];
			sprintf(sect,"SECTION HOUSE ITEM %i",hitem[k]);
			iter = Scripts::House->getNewIterator(sect);

			if (iter!=NULL)
			{
				P_ITEM pi_l=NULL;
				loopexit=0;
				do
				{
					iter->parseLine(script1, script2);
					if (script1[0]!='}')
					{
						if (!(strcmp(script1,"ITEM")))
						{
							pi_l=item::CreateScriptItem(s,str2num(script2),0);//This opens the item script... so we gotta keep track of where we are with the other script.

							if(ISVALIDPI(pi_l))
							{

					
							pi_l->magic=2;//Non-Movebale by default
							pi_l->setDecay( false ); //since even things in houses decay, no-decay by default
							pi_l->setNewbie( false );
							pi_l->setDispellable( false );
							pi_l->setPosition(x, y, z);
							pi_l->setOwnerSerial32(pc->getSerial32());
							// SPARHAWK 2001-01-28 Added House sign naming
							if (pi_l->IsSign())
								if ((id%256 >=0x70) && (id%256<=0x73))
									pi_l->setCurrentName("%s's tent",pc->getCurrentNameC());
								else if (id%256<=0x18)
									pi_l->setCurrentName("%s's ship",pc->getCurrentNameC());
								else
									pi_l->setCurrentName("%s's house",pc->getCurrentNameC());

							}
						}
						if (!(strcmp(script1,"DECAY")))
						{
							if (ISVALIDPI(pi_l)) pi_l->setDecay();
						}
						if (!(strcmp(script1,"NODECAY")))
						{
							if (ISVALIDPI(pi_l)) pi_l->setDecay( false );
						}
						if (!(strcmp(script1,"PACK")))//put the item in the Builder's Backpack
						{
							if(ISVALIDPI(pi_l)) {
								P_ITEM back = pc->getBackpack();
								if( ISVALIDPI( back ) ) {
									back->AddItem( pi_l );
								}
							}
						}
						if (!(strcmp(script1,"MOVEABLE")))
						{
							if (ISVALIDPI(pi_l)) pi_l->magic=1;
						}
						if (!(strcmp(script1,"LOCK")))//lock it with the house key
						{
							if (ISVALIDPI(pi_l)) {
								pi_l->more1=pHouse->getSerial().ser1;
								pi_l->more2=pHouse->getSerial().ser2;
								pi_l->more3=pHouse->getSerial().ser3;
								pi_l->more4=pHouse->getSerial().ser4;
							}
						}
						if (!(strcmp(script1,"X")))//offset + or - from the center of the house:
						{
							if (ISVALIDPI(pi_l)) pi_l->setPosition('x', x+str2num(script2));
						}
						if (!(strcmp(script1,"Y")))
						{
							if (ISVALIDPI(pi_l)) pi_l->setPosition('y', y+str2num(script2));
						}
						if (!(strcmp(script1,"Z")))
						{
							if (ISVALIDPI(pi_l)) pi_l->setPosition('z', z+str2num(script2));
						}
					}
				}
				while ( (strcmp(script1,"}")) && (++loopexit < MAXLOOPS) );

				if (ISVALIDPI(pi_l)) 
					if (pi_l->isInWorld()) 
					{
						regions::add(pi_l);
					}
				safedelete(iter);
			}
		}
			
        NxwSocketWrapper sw;
		sw.fillOnline( pc, false );
        for( sw.rewind(); !sw.isEmpty(); sw++ ) {
			NXWCLIENT ps_i = sw.getClient();
			if(ps_i==NULL) 
				continue;
			P_CHAR pc_i=ps_i->currChar();
			if(ISVALIDPC(pc_i))
				pc_i->teleport();
		}
                //</Luxor>
		if (!(norealmulti))
		{
			charpos.x= x+cx; //move char inside house
			charpos.y= y+cy;
			charpos.dispz= charpos.z= z+cz;

			pc->setPosition( charpos );
			//ConOut("Z: %i Offset: %i Char: %i Total: %i\n",z,cz,chars[currchar[s]].z,z+cz);
			pc->teleport();
		}
	}
}

/*!
\brief Turn a house into a deed if posible
\author CrackerJack
\param s socket of player
\param pi pointer to the house item
*/
void deedhouse(NXWSOCKET s, P_ITEM pi)
{
	SI32 x1, y1, x2, y2;
	VALIDATEPI(pi);
	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	Location charpos= pc->getPosition();


	if(pi->getOwnerSerial32() == pc->getSerial32() || pc->IsGM()) // bugfix LB, was =
	{
		Map->MultiArea(pi, &x1,&y1,&x2,&y2);

		P_ITEM pi_ii=item::CreateFromScript( pi->morex, pc->getBackpack() ); // need to make before delete
		VALIDATEPI(pi_ii);

		sysmessage( s, TRANSLATE("Demolishing House %s"), pi->getCurrentNameC());
		pi->deleteItem();
		sysmessage(s, TRANSLATE("Converted into a %s."), pi_ii->getCurrentNameC());
		// door/sign delete

		NxwCharWrapper sc;
		sc.fillCharsNearXYZ( charpos, BUILDRANGE, true, false );
		for( sc.rewind(); !sc.isEmpty(); sc++ ) {
			P_CHAR p_index=sc.getChar();
			if( ISVALIDPC(p_index) ) {

				Location charpos2= p_index->getPosition();
				if( (charpos2.x >= (UI32)x1) && (charpos2.y >= (UI32)y1) && (charpos2.x <= (UI32)x2) && (charpos2.y <= (UI32)y2) )
				{
                
					if( p_index->npcaitype == NPCAI_PLAYERVENDOR )
					{
						char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

						sprintf( temp, TRANSLATE("A vendor deed for %s"), p_index->getCurrentNameC() );
						P_ITEM pDeed = item::CreateFromScript( "$item_employment_deed", pc->getBackpack() );
						VALIDATEPI(pDeed);

						pDeed->Refresh();
						sprintf(temp, TRANSLATE("Packed up vendor %s."), p_index->getCurrentNameC());
						p_index->deleteChar();
						sysmessage(s, temp);
					}
				}		
			}
		}
        
		NxwItemWrapper si;
		si.fillItemsNearXYZ( charpos, BUILDRANGE, false );
		for( si.rewind(); !si.isEmpty(); si++ ) {
		{
			P_ITEM p_item=si.getItem();
			if(ISVALIDPI(p_item)) {
				if( (p_item->getPosition().x >= (UI32)x1) &&
					(p_item->getPosition().x <= (UI32)x2) &&
					(p_item->getPosition().y >= (UI32)y1) &&
					(p_item->getPosition().y <= (UI32)y2) )
					{
							p_item->deleteItem();
					}
				}
			}
		}

		killkeys( pi->getSerial32() );
		sysmessage(s,TRANSLATE("All house items and keys removed."));
		/*
		charpos.z= charpos.dispz= Map->MapElevation(charpos.x, charpos.y);
		pc->setPosition( charpos );
		*/
		pc->setPosition('z', Map->MapElevation(charpos.x, charpos.y));
		pc->setPosition('d', Map->MapElevation(charpos.x, charpos.y));
		pc->teleport();
		return;
	}
}

// removes houses - without regions. slow but necassairy for house decay
// LB 19-September 2000
/*!
\brief Remove Houses, without regions, slow but necessariry for house decay
\todo Need rewrite, now commented out
*/
void killhouse(ITEM i)
{
/*	P_CHAR pc;
	P_ITEM pi;
	int x1, y1, x2, y2;


	pi = MAKE_ITEM_REF(i);

	Map->MultiArea(pi, &x1, &y1, &x2, &y2);
	SERIAL serial = pi->getSerial32();

	int a;
	for (a = 0; a < charcount; a++) // deleting npc-vendors attched to the decying house
	{
		pc = MAKE_CHAR_REF(a);
		Location charpos= pc->getPosition();

		if ((charpos.x >= x1) && (charpos.y >= y1) && (charpos.x <= x2) && (charpos.y <= y2) && !pc->free)
		{
			if (pc->npcaitype == NPCAI_PLAYERVENDOR) // player vendor in right place, delete !
			{
				pc->deleteChar();
			}
		}
	}

	for (a = 0; a < itemcount; a++) // deleting itmes inside house
	{
		pi = MAKE_ITEM_REF(a);
		if ((pi->getPosition("x") >= x1) && 
			(pi->getPosition("y") >= y1) && 
			(pi->getPosition("x") <= x2) && 
			(pi->getPosition("y") <= y2) && 
			(!pi->free))
		{
			if (pi->type != ITYPE_GUILDSTONE) // dont delete guild stones !
			{
				pi->deleteItem();
			}
		}
	}

	// deleting house keys
	killkeys(serial);
	*/
}

/*!
\brief does all the work for house decay

checks all items if they are houses. if so, check its time stamp. if its too old remove it
\todo need rewrite, now is commented out...
*/
int check_house_decay()
{
/*	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	P_ITEM pi;
	bool is_house;
	int houses=0;
	int decayed_houses=0;
	unsigned long int timediff;
	unsigned long int ct=getclock();

	//CWatch *Watch=new CWatch();

	for (int i=0; i<itemcount; i++)
	{
		pi=MAKE_ITEM_REF(i); // there shouldnt be an error here !
		is_house=pi->IsHouse();
		if (is_house && !pi->free)
		{
			// its a house -> check its unused time
			//ConOut("id2: %x time_unused: %i max: %i\n",pi->id2,pi->time_unused,server_data.housedecay_secs);

			if (pi->time_unused>SrvParms->housedecay_secs) // not used longer than max_unused time ? delete the house
			{
				decayed_houses++;
				sprintf(temp,"%s decayed! not refreshed for > %i seconds!\n",pi->getCurrentNameC(),SrvParms->housedecay_secs);
				LogMessage(temp);
				killhouse(i);
			}
			else // house ok -> update unused-time-attribute
			{
				timediff=(ct-pi->timeused_last)/MY_CLOCKS_PER_SEC;
				pi->time_unused+=timediff; // might be over limit now, but it will be cought next check anyway
				pi->timeused_last=ct;	// if we don't do that and housedecay is checked every 11 minutes,
									// it would add 11,22,33,... minutes. So now timeused_last should in fact
									// be called timeCHECKED_last. but as there is a new timer system coming up
									// that will make things like this much easier, I'm too lazy now to rename
									// it (Duke, 16.2.2001)
			}
			houses++;
		}
	}
	//delete Watch;
	return decayed_houses;
*/
	return 0;
}

void addthere(int s, int xx, int yy, int zz, int t)
{
	
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	
	//unsigned int j;
	//ConOut("addthere!x=%d y=%d z=%d\n",xx,yy,zz);
	tile_st tile;

	P_ITEM pi=item::CreateFromScript( "$item_hardcoded" );
	VALIDATEPI(pi);

        pi->setId( (addid1[s]<<8) | addid2[s] );
	pi->setPosition(xx, yy, zz);
	pi->amount=1;
	pi->doordir=0;
	pi->type=t;

	if (pi->isInWorld()) 
	{
		regions::add(pi); //Add to mapRegions
	}

	Map->SeekTile(pi->id(), &tile);
	if(pc->making==999) 
		pc->making=DEREF_P_ITEM(pi); // store item #

	if(tile.flag2&0x08) 
		pi->pileable=1;

	pi->Refresh();
	addid1[s]=0;
	addid2[s]=0;
}

/*!
\author Luxor
\note This function use a BAD method based on cAllObjects, will be substituted with a map system.
*/
void killkeys(SERIAL serial) // Crackerjack 8/11/99
{
	if ( serial <= INVALID )
		return;

	cAllObjectsIter objs;
	P_ITEM pi = NULL;
	for( objs.rewind(); !objs.IsEmpty(); objs++ ) {
		if ( !isItemSerial( objs.getSerial() ) )
			continue;

		if ( ISVALIDPI( (pi=static_cast<P_ITEM>(objs.getObject())) ) ) {
			if ( pi->type == ITYPE_KEY && calcserial(pi->more1, pi->more2, pi->more3, pi->more4) == serial )
				pi->deleteItem();
		}
	}
}

/*!
\brief Checks if somebody is on the house list
\param pi pointer to the house item
\param s1 serial (byte1)
\param s2 serial (byte2)
\param s3 serial (byte3)
\param s4 serial (byte4)
\param li pointer to variable to put items[] index of list item in or NULL
\return 0 if character is not on house list, else the type of list
\todo change the 4 chars to a single serial32
*/
int on_hlist(P_ITEM pi, unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4, int *li)
{

	VALIDATEPIR( pi, 0);
	if( !pi->isInWorld() )
		return 0;
	
	P_ITEM p_ci=NULL;

	NxwItemWrapper si;
	si.fillItemsNearXYZ( pi->getPosition(), BUILDRANGE, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		p_ci=si.getItem();
		if(ISVALIDPI(p_ci)) {

			if((p_ci->morey== (UI32)pi->getSerial32())&&
			   (p_ci->more1== s1)&&(p_ci->more2==s2)&&
			   (p_ci->more3== s3)&&(p_ci->more4==s4))
				{
				    if(li!=NULL) *li=DEREF_P_ITEM(p_ci);
						return p_ci->morex;
				}
		}
	}

	return 0;
}
/*
int on_hlist(int h, unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4, int *li)
{
	int cc;
	int cl=-1;
	int ci=-1;

	cc=regions::GetCell(items[h].x,items[h].y);
	do {
		cl=regions::GetNextItem(cc, cl);
		if(cl==-1) break;
		ci=regions::GetItem(cc, cl);
		if(ci<1000000) {
			if((items[ci].contserial==items[h].serial)&&
				(items[ci].more1==s1)&&(items[ci].more2==s2)&&
				(items[ci].more3==s3)&&(items[ci].more4==s4))
			{
				if(li!=NULL) *li=ci;
				return items[ci].morex;
			}
		}
	} while(ci!=-1);
	return 0;
}
*/

/*!
\brief Adds somebody to a house list
\param c chars[] index
\param h items[] index
\param t list tyle
\return 1 if successful addition, 2 if the char was alredy on a house list, 3 if the character is not on property
*/
int add_hlist(int c, int h, int t)
{
	int sx, sy, ex, ey;

	P_CHAR pc=MAKE_CHAR_REF(c);
	VALIDATEPCR(pc,3);

	P_ITEM pi_h=MAKE_ITEM_REF(h);
	VALIDATEPIR(pi_h,3);

	if(on_hlist(pi_h, pc->getSerial().ser1, pc->getSerial().ser2, pc->getSerial().ser3, pc->getSerial().ser4, NULL))
		return 2;


	
	Map->MultiArea(pi_h, &sx,&sy,&ex,&ey);
	// Make an object with the character's serial & the list type
	// and put it "inside" the house item.
	Location charpos= pc->getPosition();

	if((charpos.x >= (UI32)sx) && (charpos.y >= (UI32)sy) && (charpos.x <= (UI32)ex) && (charpos.y <= (UI32)ey))
	{
		P_ITEM pi=archive::getNewItem();
		

		pi->morex= t;
		pi->more1= pc->getSerial().ser1;
		pi->more2= pc->getSerial().ser2;
		pi->more3= pc->getSerial().ser3;
		pi->more4= pc->getSerial().ser4;
		pi->morey= pi_h->getSerial32();

		pi->setDecay( false );
		pi->setNewbie( false );
		pi->setDispellable( false );
		pi->visible= 0;
		pi->setCurrentName(TRANSLATE("friend of house"));

		pi->setPosition( pi_h->getPosition() );

		regions::add(pi);
		return 1;
	}
	return 3;
}

/*!
\brief Remove somebody from a house list
\param c chars[] index
\param h items[] index for house
\todo change to P_CHAR & P_ITEM
\return 0 if the player was not on a list, else the list which the player was in.
*/
int del_hlist(int c, int h)
{
	int hl, li;

	P_CHAR pc=MAKE_CHAR_REF(c);
	VALIDATEPCR(pc,0);

	P_ITEM pi=MAKE_ITEM_REF(h);
	VALIDATEPIR(pi,0);

	hl=on_hlist(pi, pc->getSerial().ser1, pc->getSerial().ser2, pc->getSerial().ser3, pc->getSerial().ser4, &li);
	if(hl) {
		P_ITEM pli=MAKE_ITEM_REF(li);
		if(ISVALIDPI(pli)) {
			regions::remove(pli);
			pli->deleteItem();
		}
	}
	return(hl);
}

LOGICAL house_speech( P_CHAR pc, NXWSOCKET socket, std::string &talk)
{
	//
	// NOTE: Socket and pc checking allready done in talking()
	//
	int  fr;
	P_ITEM pi = findmulti( pc->getPosition() );
	//
	// As we don't want a error logged when not in a house we cannot use VALIDATEPIR here
	//
	if( !ISVALIDPI( pi ) )
		return false;

	//
	// if pc is not a friend or owner, we don't care what he says
	//
	fr=on_hlist(pi, pc->getSerial().ser1, pc->getSerial().ser2, pc->getSerial().ser3, pc->getSerial().ser4, NULL);
	if( fr != H_FRIEND && pi->getOwnerSerial32() != pc->getSerial32() )
		return false;
	//
	// house ban
	//
	if( talk.find("I BAN THEE") != std::string::npos )
	{
		addid1[socket] = pi->getSerial().ser1;
		addid2[socket] = pi->getSerial().ser2;
		addid3[socket] = pi->getSerial().ser3;
		addid4[socket] = pi->getSerial().ser4;
		target(socket, 0, 1, 0, 229, TRANSLATE("Select person to ban from house."));
		return true;
	}
	//
	// kick out of house
	//
	if( talk.find("REMOVE THYSELF") != std::string::npos )
	{
		addid1[socket] = pi->getSerial().ser1;
		addid2[socket] = pi->getSerial().ser2;
		addid3[socket] = pi->getSerial().ser3;
		addid4[socket] = pi->getSerial().ser4;
		target(socket, 0, 1, 0, 228, TRANSLATE("Select person to eject from house."));
		return true;
	}
	//
	// Lock down item
	//
	if ( talk.find("I WISH TO LOCK THIS DOWN") != std::string::npos )
	{
		target(socket, 0, 1, 0, 232, TRANSLATE("Select item to lock down"));
		return true;
	}
	//
	// Unlock down item
	//
	if ( talk.find("I WISH TO RELEASE THIS") != std::string::npos )
	{
		target(socket, 0, 1, 0, 233, TRANSLATE("Select item to release"));
		return true;
	}
	//
	// Secure item
	//
	if ( talk.find("I WISH TO SECURE THIS") != std::string::npos )
	{
		target(socket, 0, 1, 0, 234, TRANSLATE("Select item to secure"));
		return true;
	}
	return false;
}


LOGICAL CheckBuildSite(int x, int y, int z, int sx, int sy)
{
	signed int checkz;
	//char statc;
	int checkx;
	int checky;
	int ycount=0;
	checkx=x-(sx/2);
	for (;checkx<(x+(sx/2));checkx++)
	{
		checky=y-(sy/2);
		for (;checky<(y+(sy/2));checky++)
		{
			checkz=Map->MapElevation(checkx,checky);
			if ((checkz>(z-2))&&(checkz<(z+2)))
			{
				ycount++;
			}
			//	statc=Map->StaHeight(checkx,checky,checkz);
			//	if (statc>0)
			//		statb=true;
		}
	}
	if (ycount==(sx*sy)) //&& (statb==false))
		return true;
	else
		return false;
}
