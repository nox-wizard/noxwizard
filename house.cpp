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
#include "data.h"
#include "boats.h"
#include "scp_parser.h"
#include "archive.h"
#include "map.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "classes.h"
#include "scripts.h"
#include "sregions.h"


LOGICAL CheckBuildSite(int x, int y, int z, int sx, int sy);
std::map< SERIAL, P_HOUSE > cHouses::houses;
std::map< int, UI32VECTOR> cHouses::houseitems;
/*!
\todo take a look to initialization, we could initialize the vector with the variables
*/
void mtarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt)
{
	UI08 multitarcrs[26]= { 0x99, 0x01, 0x40, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

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

SERIAL cHouse::getSerial()
{
	return houseserial;
}

void cHouse::setSerial(SERIAL itemLink)
{
	houseserial=itemLink;
}

SI32 cHouse::getKeycode()
{
	return keycode;
}

/*!
\author Wintermute, original code by Zippy
\brief Build an house

Triggered by double clicking a deed-> the deed's morex is read
for the house section in house.cpp. Extra items can be added
using HOUSE ITEM, (this includes all doors!) and locked "LOCK"
Space around the house with SPACEX/Y and CHAR offset CHARX/Y/Z
*/

void cHouse::createHouse(UI32 houseNumber)
{
	int hitem[100];//extra "house items" (up to 100)
	char sect[512];                         //file reading
	char name[512];
	P_ITEM house = pointers::findItemBySerial(this->getSerial());
	UI32 sx = 0, sy = 0, icount=0;
	int loopexit=0;//the house/key items
	hitem[0]=0;//avoid problems if there are no HOUSE_ITEMs by initializing the first one as 0
	if (houseNumber)
	{
		cScpIterator* iter = NULL;
		char script1[1024];
		char script2[1024];
		sprintf(sect, "SECTION HOUSE %d", houseNumber);//and BTW, .find() adds SECTION on there for you....

		iter = Scripts::House->getNewIterator(sect);
		if (iter==NULL) return;

		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if (!(strcmp(script1,"ID")))
				{
					house->setId(hex2num(script2));
				}
				else if (!(strcmp(script1,"HOUSE_ITEM")))
				{
					cHouses::addHouseItem(houseNumber, str2num(script2));
				}
				else if (!(strcmp(script1,"SPACEX1")))
				{
					spacex1=str2num(script2)+1;
				}
				else if (!(strcmp(script1,"SPACEY1")))
				{
					spacey1=str2num(script2)+1;
				}
				else if (!(strcmp(script1,"SPACEX2")))
				{
					spacex2=str2num(script2)+1;
				}
				else if (!(strcmp(script1,"SPACEY2")))
				{
					spacey2=str2num(script2)+1;
				}
				else if (!(strcmp(script1,"CHARX")))
				{
					char_x=str2num(script2);
				}
				else if (!(strcmp(script1,"CHARY")))
				{
					char_y=str2num(script2);
				}
				else if (!(strcmp(script1,"CHARZ")))
				{
					char_z=str2num(script2);
				}
				else if( !(strcmp(script1, "ITEMSDECAY" )))
				{
					house->more4=str2num( script2 );
				}
				else if (!(strcmp(script1, "HOUSE_DEED")))
				{
					housedeed=str2num(script2);
				}
				else if (!(strcmp(script1, "BOAT"))) 
				{
					ErrOut("Bad house script # %i!\n",houseNumber);
					return;
				}
				else if (!(strcmp(script1, "NOREALMULTI"))) norealmulti=1; // LB bugfix for pentas crashing client
				else if (!(strcmp(script1, "NOKEY"))) this->publicHouse=1;
				else if (!(strcmp(script1, "NAME")))
				{
					strcpy(name,script2);
					house->setCurrentName(name);
				}
			}
		}
		while ( (strcmp(script1,"}")) && (++loopexit < MAXLOOPS) );
		safedelete(iter);

		if (!house->getId())
		{
			ErrOut("Bad house script # %i!\n",houseNumber);
			return;
		}
		
	}
}

void cHouse::transfer(SERIAL newOwner)
{
	P_ITEM house = pointers::findItemBySerial(this->getSerial());
	this->friends.clear();
	this->coowners.clear();
	this->banned.clear();
	cHouses::killkeys(house->getSerial32());

}

bool cHouse::isRealMulti()
{
	return norealmulti==0;
}

void cHouses::makeKeys(P_HOUSE pHouse, P_CHAR pc)
{
	P_ITEM pKey=NULL;
	P_ITEM pKey2=NULL;
	P_ITEM house = pointers::findItemBySerial(pHouse->getSerial());

	P_ITEM pBackPack = pc->getBackpack();
	SI16 id=house->getId();
	char temp[100];
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

	house->st = pKey->getSerial32();		// Create link from house to housekeys to allow easy renaming of
	house->st2= pKey2->getSerial32();	// house, housesign and housekeys without having to loop trough
													// all world items (Sparhawk)


	pKey->more1=(pHouse->getSerial()>>24)&0xFF;//use the house's serial for the more on the key to keep it unique
	pKey->more2=(pHouse->getSerial()>>16)&0xFF;
	pKey->more3=(pHouse->getSerial()>>8)&0xFF;
	pKey->more4=(pHouse->getSerial())&0xFF;
	pKey->moreb1=(pHouse->getKeycode()>>24)&0xFF;
	pKey->moreb2=(pHouse->getKeycode()>>16)&0xFF;
	pKey->moreb3=(pHouse->getKeycode()>>8)&0xFF;
	pKey->moreb4=(pHouse->getKeycode())&0xFF;
	pKey->type=ITYPE_KEY;
	pKey->setNewbie();
	pKey2->more1=(pHouse->getSerial()>>24)&0xFF;//use the house's serial for the more on the key to keep it unique
	pKey2->more2=(pHouse->getSerial()>>16)&0xFF;
	pKey2->more3=(pHouse->getSerial()>>8)&0xFF;
	pKey2->more4=(pHouse->getSerial())&0xFF;
	pKey2->moreb1=(pHouse->getKeycode()>>24)&0xFF;
	pKey2->moreb2=(pHouse->getKeycode()>>16)&0xFF;
	pKey2->moreb3=(pHouse->getKeycode()>>8)&0xFF;
	pKey2->moreb4=(pHouse->getKeycode())&0xFF;
	pKey2->type=ITYPE_KEY;
	pKey2->setNewbie();

	P_ITEM bankbox = pc->GetBankBox();
	if(bankbox!=NULL) // we sould add a key in bankbox only if the player has a bankbox =)
	{

		P_ITEM p_key3=item::CreateFromScript( "$item_gold_key" );
		VALIDATEPI(p_key3);
		p_key3->setCurrentName( "a house key" );
		p_key3->more1=(pHouse->getSerial()>>24)&0xFF;
		p_key3->more2=(pHouse->getSerial()>>16)&0xFF;
		p_key3->more3=(pHouse->getSerial()>>8)&0xFF;
		p_key3->more4=(pHouse->getSerial())&0xFF;
		p_key3->moreb1=(pHouse->getKeycode()>>24)&0xFF;
		p_key3->moreb2=(pHouse->getKeycode()>>16)&0xFF;
		p_key3->moreb3=(pHouse->getKeycode()>>8)&0xFF;
		p_key3->moreb4=(pHouse->getKeycode())&0xFF;
		p_key3->type=ITYPE_KEY;
		p_key3->setNewbie();
		bankbox->AddItem(p_key3);
	}

}

/*!
\author Wintermute
\brief Build an house
Create an image of the house at the mouse pointer, allowing the char to drag it to the right position
\param builder, the char, who is building the house
\param housedeed, the housedeed that is used to build
*/

void cHouses::buildhouse( P_CHAR builder, P_ITEM housedeed)
{
	NXWCLIENT ps = builder->getClient();
	SI16 id = INVALID;
	P_HOUSE newHouse = new cHouse();
	P_TARGET targ = NULL;
	P_ITEM pHouse = item::CreateFromScript( "$item_hardcoded" );
	VALIDATEPI(pHouse);
	builder->fx1=housedeed->getSerial32();
	newHouse->setSerial(pHouse->getSerial32());
	newHouse->createHouse(housedeed->morex);
	id = pHouse->getId();
	if (ps->isDragging()) 
	{
		ps->resetDragging();
		// UpdateStatusWindow(builder->getSocket(),pi);
	}

	pHouse->setDecay( false );
	pHouse->setNewbie( false );
	pHouse->setDispellable( false );
	pHouse->setOwnerSerial32(builder->getSerial32());
	pHouse->setPosition (0,0,0);
	newHouse->setOwner(builder->getSerial32());
	houses.insert( make_pair( newHouse->getSerial(), newHouse ) );

	mtarget(builder->getSocket(), 0, 1, 0, 0, (id>>8) -0x40, (id%256), TRANSLATE("Select location for building."));
	targ = clientInfo[builder->getSocket()]->newTarget( new cLocationTarget() );
	targ->code_callback=cHouses::target_buildhouse;
	targ->buffer[0]=newHouse->getSerial();
	targ->send( ps );
	ps->sysmsg( TRANSLATE("Where do you want to dig?"));
}

/*!
\author Wintermute, original code by Zippy
\brief Build an house

Triggered by double clicking a deed-> the deed's morex is read
for the house section in house.cpp. Extra items can be added
using HOUSE ITEM, (this includes all doors!) and locked "LOCK"
Space around the house with SPACEX/Y and CHAR offset CHARX/Y/Z
*/
void cHouses::target_buildhouse( NXWCLIENT ps, P_TARGET t )
{
	NXWSOCKET s = ps->toInt();
	SERIAL houseserial=t->buffer[0];
	P_ITEM iHouse=pointers::findItemBySerial(houseserial);
	P_HOUSE pHouse=cHouses::findHouse(houseserial);
	int housenumber;
	UI32 x, y;
	SI32 k, icount=0;
	signed char z;
	int boat=0;//Boats
	char sect[512];                         //file reading

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	x = t->getLocation().x; //where they targeted
	y = t->getLocation().y;
	z = t->getLocation().z;

	Location charpos= pc->getPosition();

	SI16 id = iHouse->getId(); //house ID

	if(!pc->IsGM() && SrvParms->houseintown==0)
	{
		if ((region[calcRegionFromXY(x,y)].priv & RGNPRIV_GUARDED) ) // popy
		{
			sysmessage(s,TRANSLATE(" You cannot build houses in town!"));
			return;
		}
	}

#define XBORDER 200
#define YBORDER 200

	//XAN : House placing fix :)
	if ( (( x<XBORDER || y <YBORDER ) || ( x>(UI32)((map_width*8)-XBORDER) || y >(UI32)((map_height*8)-YBORDER) ))  )
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


	for (k=-pHouse->getLeftXRange();k<pHouse->getRightXRange();k++)//check the SPACEX and SPACEY to make sure they are valid locations....
	{
		for (SI32 l=-pHouse->getUpperYRange();l<pHouse->getLowerYRange();l++)
		{
			Location loc;
			loc.x=x+k;
			loc.y=y+l;
			loc.z=z;

			Location newpos = Loc( x+k, y+l, z );
			if ( (isWalkable( newpos ) == illegal_z ) &&
				((charpos.x != x+k)&&(charpos.y != y+l)) )
				/*This will take the char making the house out of the space check, be careful
				you don't build a house on top of your self..... this had to be done So you
				could extra space around houses, (12+) and they would still be buildable.*/
			{
				sysmessage(s, TRANSLATE("You cannot build your stucture there."));
				pHouse->remove();
				iHouse->Delete();
				cHouses::Delete(pHouse->getSerial());
				delete pHouse;
				return;
				//ConOut("Invalid %i,%i [%i,%i]\n",k,l,x+k,y+l);
			} //else ConOut("DEBUG: Valid at %i,%i [%i,%i]\n",k,l,x+k,y+l);

			P_HOUSE house2=cHouses::findHouse(loc);
			if ( house2 == NULL )
				continue;
			P_ITEM pi_ii=pointers::findItemBySerial(house2->getSerial());
			if ((ISVALIDPI(pi_ii) && (pHouse->isRealMulti())) || (house2 != NULL ))
			{
				sysmessage(s,TRANSLATE("You cant build structures inside structures"));
				pHouse->remove();
				iHouse->Delete();
				cHouses::Delete(pHouse->getSerial());
				delete pHouse;
				return;
			}
		}
	}


	if (id == INVALID)
		return;
	iHouse->setPosition (t->getLocation());
	if (iHouse->isInWorld()) 
	{
		mapRegions->add(iHouse);
	}

	P_ITEM pFx1 = MAKE_ITEM_REF( pc->fx1 );
	housenumber=pFx1->morex;
	if ( pFx1 != 0 )
		pFx1->Delete(); // this will del the deed no matter where it is

	pc->fx1=-1; //reset fx1 so it does not interfere
	// bugfix LB ... was too early reseted

	cHouses::makeKeys(pHouse, pc);
	UI32VECTOR items=cHouses::getHouseItems(housenumber);
	UI32VECTOR::iterator item=items.begin();
	for (;item != items.end();item++)//Loop through the HOUSE_ITEMs
	{
		cScpIterator* iter = NULL;
		char script1[1024];
		char script2[1024];
		sprintf(sect,"SECTION HOUSE ITEM %i",*item);
		iter = Scripts::House->getNewIterator(sect);

		if (iter!=NULL)
		{
			P_ITEM pi_l=NULL;
			int loopexit=0;
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
						if (ISVALIDPI(pi_l)) pi_l->setContSerial((pc->getBackpack())->getSerial32());
						if (ISVALIDPI(pi_l)) pi_l->setPosition("x", rand()%90+31);
						if (ISVALIDPI(pi_l)) pi_l->setPosition("y", rand()%90+31);
						if (ISVALIDPI(pi_l)) pi_l->setPosition("z", 9);
					}
					if (!(strcmp(script1,"MOVEABLE")))
					{
						if (ISVALIDPI(pi_l)) pi_l->magic=1;
					}
					if (!(strcmp(script1,"LOCK")))//lock it with the house key
					{
						if (ISVALIDPI(pi_l)) {
							pi_l->more1=iHouse->getSerial().ser1;
							pi_l->more2=iHouse->getSerial().ser2;
							pi_l->more3=iHouse->getSerial().ser3;
							pi_l->more4=iHouse->getSerial().ser4;
						}
					}
					if (!(strcmp(script1,"X")))//offset + or - from the center of the house:
					{
						if (ISVALIDPI(pi_l)) pi_l->setPosition("x", x+str2num(script2));
					}
					if (!(strcmp(script1,"Y")))
					{
						if (ISVALIDPI(pi_l)) pi_l->setPosition("y", y+str2num(script2));
					}
					if (!(strcmp(script1,"Z")))
					{
						if (ISVALIDPI(pi_l)) pi_l->setPosition("z", z+str2num(script2));
					}
				}
			}
			while ( (strcmp(script1,"}")) && (++loopexit < MAXLOOPS) );

			if (ISVALIDPI(pi_l)) 
				if (pi_l->isInWorld()) 
				{
					mapRegions->add(pi_l);
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
	if (pHouse->isRealMulti())
	{
		int newx, newy, newz;
		pHouse->getCharPos(newx, newy, newz);
		charpos.x= x+newx; //move char inside house
		charpos.y= y+newy;
		charpos.dispz= charpos.z= z+newz;

		pc->setPosition( charpos );
		//ConOut("Z: %i Offset: %i Char: %i Total: %i\n",z,cz,chars[currchar[s]].z,z+cz);
		pc->teleport();
	}
}

void cHouse::getCharPos(int &x, int &y, int &z)
{
	x=char_x;
	y=char_y;
	z=char_z;
}

void cHouse::setPublicState(bool state)
{
	this->publicHouse=state;
}

void cHouse::togglePublicState()
{
	this->publicHouse=!this->publicHouse;
}

/*!
\brief does all the work for house decay

checks all items if they are houses. if so, check its time stamp. if its too old remove it
\todo need rewrite, now is commented out...
*/
int cHouses::check_house_decay()
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


/*!
\author Luxor
\note This function use a BAD method based on cAllObjects, will be substituted with a map system.
*/
void cHouses::killkeys(SERIAL serial) // Crackerjack 8/11/99
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
				pi->Delete();
		}
	}
}

LOGICAL cHouses::house_speech( P_CHAR pc, NXWSOCKET socket, std::string &talk)
{
	//
	// NOTE: Socket and pc checking allready done in talking()
	//
	P_HOUSE house=cHouses::findHouse(pc->getPosition());
	P_ITEM ihouse=pointers::findItemBySerial(house->getSerial());
	//
	// As we don't want a error logged when not in a house we cannot use VALIDATEPIR here
	//
	if( !ISVALIDPI( ihouse ) )
		return false;

	//
	// if pc is not a friend or owner, we don't care what he says
	//
	if( !house->isCoOwner(pc) && !house->isFriend(pc) && house->getOwner() != pc->getSerial32() )
		return false;
	//
	// house ban
	//
	if( talk.find("I BAN THEE") != std::string::npos )
	{
		P_TARGET targ = clientInfo[socket]->newTarget( new cCharTarget() );
		targ->code_callback=cHouses::target_houseBan;
		targ->buffer[0]=ihouse->getSerial32();
		targ->send( getClientFromSocket( socket) );
		sysmessage( socket, TRANSLATE("Select person to ban from house."));
		return true;
	}
	//
	// kick out of house
	//
	if( talk.find("REMOVE THYSELF") != std::string::npos )
	{
		P_TARGET targ = clientInfo[socket]->newTarget( new cCharTarget() );
		targ->code_callback=cHouses::target_houseEject;
		targ->buffer[0]=ihouse->getSerial32();
		targ->send( getClientFromSocket( socket) );
		sysmessage( socket, TRANSLATE("Select person to eject from house."));
		return true;
	}
	//
	// Lock down item
	//
	if ( talk.find("I WISH TO LOCK THIS DOWN") != std::string::npos )
	{
		P_TARGET targ = clientInfo[socket]->newTarget( new cItemTarget() );
		targ->code_callback=cHouses::target_houseLockdown;
		targ->buffer[0]=ihouse->getSerial32();
		targ->send( getClientFromSocket( socket) );
		sysmessage( socket, TRANSLATE("Select item to lock down"));
		return true;
	}
	//
	// Unlock down item
	//
	if ( talk.find("I WISH TO RELEASE THIS") != std::string::npos )
	{
		P_TARGET targ = clientInfo[socket]->newTarget( new cItemTarget() );
		targ->code_callback=cHouses::target_houseRelease;
		targ->buffer[0]=ihouse->getSerial32();
		targ->send( getClientFromSocket( socket) );
		sysmessage( socket, TRANSLATE("Select item to release"));
		return true;
	}
	//
	// Secure item
	//
	if ( talk.find("I WISH TO SECURE THIS") != std::string::npos )
	{
		P_TARGET targ = clientInfo[socket]->newTarget( new cItemTarget() );
		targ->code_callback=cHouses::target_houseSecureDown;
		targ->buffer[0]=ihouse->getSerial32();
		targ->send( getClientFromSocket( socket) );
		sysmessage( socket, TRANSLATE("Select item to secure"));
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
			checkz=mapElevation(checkx,checky);
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


cHouse::cHouse() 
{
	owner=INVALID;
	norealmulti=0;
	keycode=0;
	publicHouse=false;
}

void cHouse::getCorners( SI32 &x1, SI32 &x2, SI32 &y1, SI32 &y2 )
{
	P_ITEM iHouse = pointers::findItemBySerial(houseserial);
	getMultiCorners( iHouse, x1, y1, x2, y2 );
	return;
}

int cHouse::getUpperYRange()
{
	return spacey1;
}

int cHouse::getLowerYRange()
{
	return spacey2;
}
int cHouse::getLeftXRange()
{
	return spacex1;
}
int cHouse::getRightXRange()
{
	return spacex2;
}

SERIAL cHouse::getOwner()
{
	return owner;
}

void cHouse::setOwner(SERIAL newOwner)
{
	owner=newOwner;
	// TBD: delete all previous keys to the house
}

/*!
\brief Turn a house into a deed if posible
\author Wintermute, original code CrackerJack
\param P_CHAR deedMaker, the player who is deeding the house
*/

void cHouse::deedhouse(P_CHAR deedMaker)
{
	VALIDATEPC(deedMaker);
	Location charpos= deedMaker->getPosition();
	P_ITEM iHouse = pointers::findItemBySerial(houseserial);


	if(this->getOwner() == deedMaker->getSerial32() || deedMaker->IsGM()) // bugfix LB, was =
	{
		P_ITEM pi_ii=item::CreateFromScript( iHouse->morex, deedMaker->getBackpack() ); // need to make before delete
		VALIDATEPI(pi_ii);

		sysmessage( deedMaker->getSocket(), TRANSLATE("Demolishing House %s"), iHouse->getCurrentNameC());
		sysmessage(deedMaker->getSocket(), TRANSLATE("Converted into a %s."), pi_ii->getCurrentNameC());
		// door/sign delete

		NxwCharWrapper sc;
		sc.fillCharsNearXYZ( charpos, BUILDRANGE, true, false );
		for( sc.rewind(); !sc.isEmpty(); sc++ ) {
			P_CHAR p_index=sc.getChar();
			if( ISVALIDPC(p_index) ) {

				Location charpos2= p_index->getPosition();
				if( inHouse(charpos2) )
				{
                
					if( p_index->npcaitype == NPCAI_PLAYERVENDOR )
					{
						char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

						sprintf( temp, TRANSLATE("A vendor deed for %s"), p_index->getCurrentNameC() );
						P_ITEM pDeed = item::CreateFromScript( "$item_employment_deed", deedMaker->getBackpack() );
						VALIDATEPI(pDeed);

						pDeed->Refresh();
						sprintf(temp, TRANSLATE("Packed up vendor %s."), p_index->getCurrentNameC());
						p_index->Delete();
						sysmessage(deedMaker->getSocket(), temp);
					}
				}		
			}
		}
		this->remove();
		iHouse->Delete();
		sysmessage(deedMaker->getSocket(),TRANSLATE("All house items and keys removed."));
		deedMaker->setPosition("z", mapElevation(charpos.x, charpos.y));
		deedMaker->setPosition("dz", mapElevation(charpos.x, charpos.y));
		deedMaker->teleport();

	}
}

void cHouse::remove()
{
	NxwItemWrapper si;
	P_ITEM iHouse = pointers::findItemBySerial(houseserial);
	si.fillItemsNearXYZ( iHouse->getPosition(), BUILDRANGE, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM p_item=si.getItem();
		if(ISVALIDPI(p_item)) 
		{
			if( inHouse(p_item->getPosition()))
			{
					p_item->Delete();
			}
		}
	}
	this->friends.clear();
	this->coowners.clear();
	this->banned.clear();
	cHouses::killkeys( this->getSerial() );
}

bool cHouse::isInsideHouse(P_ITEM pi)
{
	Location itemLoc=pi->getPosition();
	if ( isInsideHouse(itemLoc.x, itemLoc.y, itemLoc.z) )
		return true;
	return false;
}

bool cHouse::isInsideHouse(P_CHAR pc)
{
	Location charLoc=pc->getPosition();
	if ( isInsideHouse(charLoc.x, charLoc.y, charLoc.z) )
		return true;
	return false;
}

bool cHouse::isInsideHouse(Location where)
{
	if ( isInsideHouse(where.x, where.y, where.z) )
		return true;
	return false;
}

bool cHouse::isInsideHouse(int x, int y, int z)
{
	if ( ! inHouse(x, y) )
		return false;
	// ToDo: Test if the item is really inside the house(meaning: under a roof)
	return false;
}

bool cHouse::inHouse(P_ITEM pi)
{
	Location itemLoc=pi->getPosition();
	return inHouse(itemLoc.x, itemLoc.y);
}

bool cHouse::inHouse(Location where)
{
	return inHouse(where.x, where.y);
}

bool cHouse::inHouse(int x, int y)
{
	P_ITEM iHouse = pointers::findItemBySerial(houseserial);
	Location houseLoc=iHouse->getPosition();
	if (( x >= houseLoc.x-this->spacex1 ) && ( x <= houseLoc.x+this->spacex2))
		if (( y >= houseLoc.y-this->spacey1 ) && ( y <= houseLoc.y+this->spacey2))
			return true;
	return false;
}

void cHouse::addFriend(P_CHAR newfriend)
{
	this->friends.push_back(newfriend->getSerial32());
}

void cHouse::removeFriend(P_CHAR newfriend)
{
	vector<SERIAL>::iterator itRemove = friends.begin();
	for ( ; itRemove < friends.end(); itRemove++)
		if ( *itRemove == newfriend->getSerial32() ) friends.erase(itRemove);

}

std::vector<SERIAL>::iterator cHouse::getHouseFriends()
{
	return this->friends.begin();
}


/*!
\brief Checks if somebody is on the friends list
\param pc the char to be tested
*/
bool cHouse::isFriend(P_CHAR pc)
{
	std::vector< SERIAL>::iterator iter;
	for ( iter = friends.begin();iter != friends.end(); iter++ )
	{
	    if (*iter == pc->getSerial32() )
			return true;
	}
	return false;

}

void cHouse::addCoOwner(P_CHAR newCoOwner)
{
	this->coowners.push_back(newCoOwner->getSerial32());
}

void cHouse::removeCoOwner(P_CHAR newCoOwner)
{
	vector<SERIAL>::iterator itRemove = coowners.begin();
	for ( ; itRemove < coowners.end(); itRemove++)
		if ( *itRemove == newCoOwner->getSerial32() ) coowners.erase(itRemove);

}

std::vector<SERIAL>::iterator cHouse::getHouseCoOwners()
{
	return this->coowners.begin();
}

/*!
\brief Checks if somebody is on the friends list
\param pc the char to be tested
*/
bool cHouse::isCoOwner(P_CHAR pc)
{
	std::vector< SERIAL>::iterator iter;
	for ( iter = coowners.begin();iter != coowners.end(); iter++ )
	{
	    if (*iter == pc->getSerial32() )
			return true;
	}
	return false;

}

void cHouse::addBan(P_CHAR newBanned)
{
	this->banned.push_back(newBanned->getSerial32());
}

void cHouse::removeBan(P_CHAR bannedChar)
{
	vector<SERIAL>::iterator itRemove = banned.begin();
	for ( ; itRemove < banned.end(); itRemove++)
		if ( *itRemove == bannedChar->getSerial32() ) banned.erase(itRemove);

}

std::vector<SERIAL>::iterator cHouse::getHouseBans()
{
	return this->banned.begin();
}

/*!
\brief Checks if somebody is on the friends list
\param pc the char to be tested
*/
bool cHouse::isBanned(P_CHAR pc)
{
	std::vector< SERIAL>::iterator iter;
	for ( iter = banned.begin();iter != banned.end(); iter++ )
	{
	    if (*iter == pc->getSerial32() )
			return true;
	}
	return false;

}

bool cHouse::isPublicHouse()
{
	return this->publicHouse == 1;
}

SI32 cHouse::getCurrentZPosition(P_CHAR pc)
{
	SI32 tempZ=0;
	P_ITEM iHouse = pointers::findItemBySerial(houseserial);

	Location pos=pc->getPosition();
	multiVector m;
	int itemCount =data::seekMulti( (short) (iHouse->getId()-0x4000), m );
	if ( itemCount < 0 )
		return 0;
	for (int i =0;i < itemCount;++i)
	{
		if (( m[i].flags != 0 ) && ( m[i].x == pos.x ) && ( m[i].y == pos.y ))
		{
			const int tmpTop = tempZ + m[i].height;
			if ( tmpTop <= tempZ + MaxZstep && tmpTop >= tempZ-1 )
			{
				tempZ=m[i].height;
				break;
			}
			else if ( tmpTop >= tempZ - MaxZstep && tmpTop < tempZ - 1 )
			{
				tempZ=m[i].height;
				break;
			}
		}                                                                                                                 
	}
	tempZ+=dynamicElevation(pos);
	if ( pos.z != tempZ )
		return tempZ;
	return 0;
}

cHouse *cHouses::findHouse(Location loc)
{
	return findHouse(loc.x, loc.y);
}

cHouse *cHouses::findHouse(int x, int y)
{
	std::map< SERIAL, P_HOUSE >::iterator allHouses (houses.begin());
	for ( ;allHouses!=houses.end();allHouses++)
	{
		P_HOUSE house=allHouses->second;
		if ( house->inHouse(x,y) )
			return house;
	}
	return NULL;
}

cHouse *cHouses::findHouse(SERIAL houseSerial)
{
	std::map< SERIAL, P_HOUSE >::iterator theHouse (houses.find(houseSerial));
	if (theHouse != houses.end() )
		return theHouse->second;
	return NULL;
}

std::map< SERIAL, P_HOUSE >::iterator cHouses::findHouses(SERIAL owner)
{
	std::map< SERIAL, P_HOUSE > ownedHouses;
	std::map< SERIAL, P_HOUSE >::iterator allHouses (houses.begin());
	for ( ;allHouses!=houses.end();allHouses++)
	{
		P_HOUSE house=allHouses->second;
		if ( house->getOwner() == owner )
			ownedHouses.insert(make_pair(house->getSerial(),house));
	}
	
	return ownedHouses.begin();
}

void cHouses::Delete(SERIAL houseserial)
{
	std::map< SERIAL, P_HOUSE >::iterator removeHouse(houses.find(houseserial));
	if ( removeHouse != houses.end() )
		houses.erase(removeHouse);
}

// buffer 0 the sign
void cHouses::target_houseOwner( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR curr=ps->currChar();
	VALIDATEPC(curr);

	P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc);

	P_ITEM pSign=pointers::findItemBySerial( t->buffer[0] );
	VALIDATEPI(pSign);

	P_ITEM pHouse=pointers::findItemBySerial(calcserial(pSign->more1, pSign->more2, pSign->more3, pSign->more4));
	VALIDATEPI(pHouse);


	NXWSOCKET s = ps->toInt();
	if(pc->getSerial32() == curr->getSerial32())
	{
		sysmessage(s, "you already own this house!");
		return;
	}

	pSign->setOwnerSerial32(pc->getSerial32());

	pHouse->setOwnerSerial32(pc->getSerial32());

	cHouses::killkeys( pHouse->getSerial32() );


	NXWCLIENT osc=pc->getClient();
	NXWSOCKET os= (osc!=NULL)? osc->toInt() : INVALID;

	P_ITEM pi3=item::CreateFromScript( "$item_gold_key" ); //gold key for everything else
	VALIDATEPI(pi3);
	pi3->setCurrentName( "a house key" );
	if(os!=INVALID)
	{
		pi3->setCont( pc->getBackpack() );
	}
	else
	{
		pi3->MoveTo( pc->getPosition() );
	}
	pi3->Refresh();
	pi3->more1= pHouse->getSerial().ser1;
	pi3->more2= pHouse->getSerial().ser2;
	pi3->more3= pHouse->getSerial().ser3;
	pi3->more4= pHouse->getSerial().ser4;
	pi3->type=7;

	sysmessage(s, "You have transferred your house to %s.", pc->getCurrentNameC());
	char temp[520];
	sprintf(temp, "%s has transferred a house to %s.", curr->getCurrentNameC(), pc->getCurrentNameC());

	NxwSocketWrapper sw;
	sw.fillOnline( pc, false );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET k=sw.getSocket();
		if(k!=INVALID)
			sysmessage(k, temp);
	}
}

// buffer[0] house
void cHouses::target_houseEject( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc = MAKE_CHAR_REF(t->getClicked());
	VALIDATEPC(pc);
	P_ITEM pi_h=MAKE_ITEM_REF(t->buffer[0]);
	VALIDATEPI(pi_h);

	NXWSOCKET s=ps->toInt();

	Location pcpos= pc->getPosition();

	SI32 sx, sy, ex, ey;
    getMultiCorners(pi_h, sx,sy,ex,ey);
    if((pcpos.x>=(UI32)sx) && (pcpos.y>=(UI32)sy) && (pcpos.x<=(UI32)ex) && (pcpos.y<=(UI32)ey))
    {
		pc->MoveTo( ex, ey, pcpos.z );
        pc->teleport();
        sysmessage(s, TRANSLATE("Player ejected."));
    }
	else 
		sysmessage(s, TRANSLATE("That is not inside the house."));

}

//buffer[0] house
void cHouses::target_houseBan( NXWCLIENT ps, P_TARGET t )
{
	target_houseEject(ps, t);	// first, eject the player

	P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc);

	P_CHAR curr=ps->currChar();
	VALIDATEPC(curr);

	NXWSOCKET s = ps->toInt();

	P_HOUSE house=cHouses::findHouse(t->buffer[0]);
	P_ITEM iHouse = pointers::findItemBySerial(house->getSerial());
	if(ISVALIDPI(iHouse))
	{
		if(pc->getSerial32() == curr->getSerial32())
			return;
		if(!house->isBanned(pc))
		{
			sysmessage(s, "%s has been banned from this house.", pc->getCurrentNameC());
		}
		else 
		{
			sysmessage(s, "That player is already on a house register.");
		}
	}
}

// buffer[0] the house
void cHouses::target_houseFriend( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR Friend = pointers::findCharBySerial( t->getClicked() );

	P_CHAR curr=ps->currChar();
	VALIDATEPC(curr);

	NXWSOCKET s = ps->toInt();

	P_HOUSE house=cHouses::findHouse(t->buffer[0]);
	P_ITEM iHouse = pointers::findItemBySerial(house->getSerial());

	if(ISVALIDPC(Friend) && ISVALIDPI(iHouse))
	{
		if(Friend->getSerial32() == curr->getSerial32())
		{
			sysmessage(s,"You cant do that!");
			return;
		}
		
		if(! house->isFriend(Friend))
		{
			sysmessage(s, "%s has been made a friend of the house.", Friend->getCurrentNameC());
		}
		else 
		{
			sysmessage(s, "That player is already a friend of the house.");
		}
	}
}

// bugffer[0] the hose
/*
void cHouses::target_houseUnlist( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
    P_ITEM pi= pointers::findItemBySerial( t->buffer[0] );
	NXWSOCKET s = ps->toInt();
    if(ISVALIDPC(pc) && ISVALIDPI(pi))
    {
        int r=del_hlist(DEREF_P_CHAR(pc), DEREF_P_ITEM(pi));
        if(r>0)
        {
            sysmessage(s, TRANSLATE("%s has been removed from the house registry."), pc->getCurrentNameC());
        }
        else
            sysmessage(s, TRANSLATE("That player is not on the house registry."));
    }
}

*/

void cHouses::target_houseLockdown( NXWCLIENT ps, P_TARGET t )
// PRE:     S is the socket of a valid owner/coowner and is in a valid house
// POST:    either locks down the item, or puts a message to the owner saying he's a moron
// CODER:   Abaddon
// DATE:    17th December, 1999
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);
	NXWSOCKET s = ps->toInt();

    P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
    if(ISVALIDPI(pi))
    {

        // time to lock it down!

        if( pi->isFieldSpellItem() )
        {
            sysmessage(s,TRANSLATE("you cannot lock this down!"));
            return;
        }
        if (pi->type==12 || pi->type==13 || pi->type==203)
        {
            sysmessage(s, TRANSLATE("You cant lockdown doors or signs!"));
            return;
        }
        if ( pi->IsAnvil() )
        {
            sysmessage(s, TRANSLATE("You cant lockdown anvils!"));
            return;
        }
        if ( pi->IsForge() )
        {
            sysmessage(s, TRANSLATE("You cant lockdown forges!"));
            return;
        }

        P_ITEM multi = findmulti( pi->getPosition() );
        if( ISVALIDPI(multi))
        {
            if(pi->magic==4)
            {
                sysmessage(s,TRANSLATE("That item is already locked down, release it first!"));
                return;
            }
            pi->magic = 4;  // LOCKED DOWN!
            clientInfo[s]->dragging=false;
            pi->setOwnerSerial32Only(pc->getSerial32());
            pi->Refresh();
            return;
        }
        else
        {
            // not in a multi!
            sysmessage( s, TRANSLATE("That item is not in your house!" ));
            return;
        }
    }
    else
    {
        sysmessage( s, TRANSLATE("Invalid item!" ));
        return;
    }
}

void cHouses::target_houseSecureDown( NXWCLIENT ps, P_TARGET t )
// For locked down and secure chests
{
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);
	NXWSOCKET s = ps->toInt();

    P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
    if(ISVALIDPI(pi))
    {
        // time to lock it down!

        if( pi->isFieldSpellItem() )
        {
            sysmessage(s,TRANSLATE("you cannot lock this down!"));
            return;
        }
        if (pi->type==12 || pi->type==13 || pi->type==203)
        {
            sysmessage(s, TRANSLATE("You cant lockdown doors or signs!"));
            return;
        }
        if(pi->magic==4)
        {
            sysmessage(s,TRANSLATE("That item is already locked down, release it first!"));
            return;
        }

        P_ITEM multi = findmulti( pi->getPosition() );
        if( ISVALIDPI(multi) && pi->type==1)
        {
            pi->magic = 4;  // LOCKED DOWN!
            pi->secureIt = 1;
            clientInfo[s]->dragging=false;
            pi->setOwnerSerial32Only(pc->getSerial32());
            pi->Refresh();
            return;
        }
        if(pi->type!=1)
        {
            sysmessage(s,TRANSLATE("You can only secure chests!"));
            return;
        }
        else
        {
            // not in a multi!
            sysmessage( s, TRANSLATE("That item is not in your house!" ));
            return;
        }
    }
    else
    {
        sysmessage( s, TRANSLATE("Invalid item!" ));
        return;
    }
}

void cHouses::target_houseRelease( NXWCLIENT ps, P_TARGET t )
// PRE:     S is the socket of a valid owner/coowner and is in a valid house, the item is locked down
// POST:    either releases the item from lockdown, or puts a message to the owner saying he's a moron
// CODER:   Abaddon
// DATE:    17th December, 1999
// update: 5-8-00
{
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);
	NXWSOCKET s = ps->toInt();

    P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
    if(ISVALIDPI(pi))
    {
        if(pi->getOwnerSerial32() != pc->getSerial32())
        {
            sysmessage(s,TRANSLATE("This is not your item!"));
            return;
        }
        if( pi->isFieldSpellItem() )
        {
            sysmessage(s,TRANSLATE("you cannot release this!"));
            return;
        }
        if (pi->type==12 || pi->type==13 || pi->type==203)
        {
            sysmessage(s, TRANSLATE("You cant release doors or signs!"));
            return;
        }
        // time to lock it down!
        P_ITEM multi = findmulti( pi->getPosition() );
        if( ISVALIDPI(multi) && pi->magic==4 || pi->type==1)
        {
            pi->magic = 1;  // Default as stored by the client, perhaps we should keep a backup?
            pi->secureIt = 0;
            pi->Refresh();
            return;
        }
        else if( !ISVALIDPI(multi) )
        {
            // not in a multi!
            sysmessage( s, TRANSLATE("That item is not in your house!" ));
            return;
        }
    }
    else
    {
        sysmessage( s, TRANSLATE("Invalid item!" ));
        return;
    }
}

void cHouses::safeoldsave()
{
	std::string oldFileName=std::string( SrvParms->savePath + SrvParms->houseWorldfile + SrvParms->worldfileExtension );
	std::string newFileName=std::string( SrvParms->savePath + SrvParms->houseWorldfile + SrvParms->worldfileExtension + "$" );
	remove( newFileName.c_str() );
	rename( oldFileName.c_str(), newFileName.c_str() );
}

bool cHouses::save( )
{
	safeoldsave();
	std::string filename( std::string( SrvParms->savePath + SrvParms->houseWorldfile + SrvParms->worldfileExtension) );
	ofstream output;
	output.open(filename.c_str());
	if( ! output.is_open() )
	{
		ErrOut("Error, couldn't open %s for writing. Check file permissions.\n", filename.c_str() );
		tempfx::tempeffectson();
		return false;
	}

	output << "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << endl;
	output << "// || NoX-Wizard guild save (nxwguild.wsc)                                ||" << endl;
	output << "// || Automatically generated on worldsaves                               ||" << endl;
	output << "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << endl;
	output << "// || Generated by NoX-Wizard version " << VERNUMB << " " << OSFIX << "               ||" << endl;
	output << "// || Requires NoX-Wizard version 0.82 to be read correctly               ||" << endl;
	output << "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << endl <<endl << endl;

	std::map< SERIAL, P_HOUSE >::iterator iter( houses.begin() ), end( houses.end() );
	for( ; iter!=end; iter++ )
	{
		iter->second->save( &output );
	}

	output.close();

	return true;
}

bool cHouses::load()
{
	std::string filename( std::string( SrvParms->savePath + SrvParms->houseWorldfile + SrvParms->worldfileExtension) );
	cStringFile file( filename, "r" );

	if( file.eof() )
	{
		WarnOut("Houses file [ %s ] not found.\n", filename.c_str());
		return false;
	}

	ConOut("Loading houses ");
	do
	{
		std::string a, b, c;
		file.read( a, b, c );
		if ( a=="SECTION" )
		{
			if( b =="HOUSE" )
			{
				P_HOUSE house = new cHouse();
				house->setSerial(str2num( c ));
				house->load(file);
				houses.insert( make_pair( house->getSerial(), house ) );
			}
		}
	}
	while ( !file.eof() );
	return true;
}

void cHouse::save(ofstream *output)
{
	*output << "SECTION HOUSE " << houseserial << endl;
	*output << "{" << endl;
	*output << "	OWNER " << owner<< endl;
	*output << "	KEYCODE " << keycode<< endl;
	*output << "	CHARX " << char_x << endl;
	*output << "	CHARY " << char_y << endl;
	*output << "	CHARZ " << char_z << endl;
	
	*output << "	SPACEX1 " << (int) spacex1 << endl;
	*output << "	SPACEX2 " << (int) spacex2 << endl;
	*output << "	SPACEY1 " << (int) spacey1 << endl;
	*output << "	SPACEY2 " << (int) spacey2 << endl;
	*output << "	HOUSEDEED " << housedeed << endl;
	if ( publicHouse) *output << "	PUBLIC" << endl;
	if ( norealmulti) *output << "	NOREALMULTI" << endl;
	std::vector<SERIAL>::iterator liststart (friends.begin()), listend(friends.end());
	for( ; liststart!=listend; ++liststart)
	{
		*output << "FRIEND " << *liststart << endl;
	}
	liststart=coowners.begin();
	listend=coowners.end();
	for( ; liststart!=listend; ++liststart)
	{
		*output <<"COOWNER " << *liststart << endl;
	}
	liststart=banned.begin();
	listend=banned.end();
	for( ; liststart!=listend; ++liststart)
	{
		*output << "BANNED " << *liststart << endl;
	}
	*output << "}" << endl << endl;
	return ;
}

void cHouse::load(cStringFile& input)
{
	do
	{
		std::string l, r;
		input.read( l, r );

		if( l[0]=='{' )
			continue;

		if( l[0]=='}' )
			break;

		switch( l[0] )
		{
			case 'A':
			case 'a':
			case 'B':
			case 'b':
				if ( l=="BANNED" )
				{
					P_CHAR banned=pointers::findCharBySerial(str2num(r));
					if (banned != NULL )
						this->addBan(banned);
				}
				break;
			case 'C':
			case 'c':
				if ( l=="COOWNER" )
				{
					P_CHAR coowner=pointers::findCharBySerial(str2num(r));
					if (coowner != NULL )
						this->addCoOwner(coowner);
				}
				else if ( l == "CHARX" )
					char_x = str2num(r);
				else if ( l == "CHARY" )
					char_y = str2num(r);
				else if ( l == "CHARZ" )
					char_z = str2num(r);
				break;
			case 'F':
			case 'f':
				if ( l=="FRIENDS" )
				{
					P_CHAR Friend=pointers::findCharBySerial(str2num(r));
					if (Friend != NULL )
						this->addFriend(Friend);
				}
				break;
			case 'H':
			case 'h':
				if ( l=="HOUSEDEED" )
				{
					housedeed=str2num(r);
				}
				break;
			case 'K':
			case 'k':
				if ( l=="KEYCODE" )
					keycode=str2num(r);
				break;
			case 'N':
			case 'n':
				if ( l=="NOREALMULTI" )
					norealmulti=true;
				break;
			case 'O':
			case 'o':
				if ( l=="OWNER" )
					owner=str2num(r);
				break;
			case 'P':
			case 'p':
				if ( l=="PUBLIC" )
					publicHouse=true;
				break;
			case 'S':
			case 's':
				if ( l == "SPACEX1" )
					spacex1 = str2num(r);
				else if ( l == "SPACEX2" )
					spacex2 = str2num(r);
				else if ( l == "SPACEY1" )
					spacey1 = str2num(r);
				else if ( l == "SPACEY2" )
					spacey2 = str2num(r);
				break;
		}

	}  while( !input.eof() );

	return ;
}

void cHouses::addHouseItem(int housenumber, int itemNumber)
{
	UI32VECTOR *itemVec;
	std::map<int, UI32VECTOR>>::iterator itemVecIt = houseitems.find(housenumber);
	if ( itemVecIt != houseitems.end())
	{
		itemVec=&itemVecIt->second;
		UI32VECTOR::iterator items=itemVec->begin();
		for ( ; items != itemVec->end();items++)
		{
			if ( *items == itemNumber )
				return; // item already inserted
		}
	}
	else
		itemVec=new UI32VECTOR();
	itemVec->push_back(itemNumber);	
	houseitems.insert(make_pair(housenumber, *itemVec));
}

UI32VECTOR cHouses::getHouseItems(int housenumber)
{
	std::map<int, UI32VECTOR>>::iterator itemVec = houseitems.find(housenumber);
	if (itemVec != houseitems.end() )
		return itemVec->second;
	return NULL;

}

void cHouses::addHouse(P_HOUSE newHouse )
{
	houses.insert(make_pair(newHouse->getSerial(), newHouse));
}