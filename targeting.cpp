  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "client.h"
#include "basics.h"
#include "amx/amxcback.h"
#include "cmdtable.h"
#include "itemid.h"
#include "sndpkg.h"
#include "debug.h"
#include "srvparms.h"
#include "nxw_utils.h"
#include "party.h"
#include "set.h"
#include "trigger.h"
#include "magic.h"
#include "house.h"
#include "targeting.h"
#include "npcai.h"
#include "layer.h"
#include "commands.h"
#include "tmpeff.h"
#include "jail.h"
#include "weight.h"
#include "data.h"
#include "boats.h"
#include "scp_parser.h"
#include "archive.h"
#include "fishing.h"
#include "rcvpkg.h"
#include "map.h"
#include "chars.h"
#include "items.h"
#include "skills.h"
#include "classes.h"
#include "inlines.h"
#include "basics.h"
#include "range.h"
#include "scripts.h"
#include "nox-wizard.h"
#include "utils.h"




void CarveTarget(NXWSOCKET s, int feat, int ribs, int hides, int fur, int wool, int bird)
{
        if ( s < 0 || s >= now )
		return;
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC( pc );

	P_ITEM pi1 = item::CreateFromScript( "$item_blood_puddle" );
	VALIDATEPI(pi1);
	pi1->setId( 0x122A );
	P_ITEM pi2=MAKE_ITEM_REF(npcshape[0]);
	VALIDATEPI(pi2);
	mapRegions->remove(pi1);
	pi1->setPosition( pi2->getPosition() );
	mapRegions->add(pi1); // lord Binary
	pi1->magic=2;//AntiChrist - makes the item unmovable
	pi1->setDecayTime();
	pi1->Refresh();

	if(feat>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_feathers", pc->getBackpack(), feat );
		VALIDATEPI(pi);
		pi->Refresh();
		sysmessage(s,TRANSLATE("You pluck the bird and get some feathers."));
	}
	if(ribs>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_cuts_of_raw_ribs", pc->getBackpack(), ribs );
		VALIDATEPI(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You carve away some meat."));
	}

	if(hides>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_hide", pc->getBackpack(), hides );
		VALIDATEPI(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You skin the corpse and get the hides."));
	}
	if(fur>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_hide", pc->getBackpack(), fur );
		VALIDATEPI(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You skin the corpse and get the hides."));
	}
	if(wool>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_piles_of_wool", pc->getBackpack(), wool );
		VALIDATEPI(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You skin the corpse and get some unspun wool."));
	}
	if(bird>0)
	{
		P_ITEM pi = item::CreateFromScript( "$item_raw_bird", pc->getBackpack(), bird );
		VALIDATEPI(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You carve away some raw bird."));
	}

	weights::NewCalc(pc);
}

/*!
\author AntiChrist
\date 03/11/1999
\brief New Carving System
\note Human-corpse carving code added
\note Scriptable carving product added
*/
static void newCarveTarget(NXWSOCKET  s, ITEM i)
{
	bool deletecorpse=false;
	char sect[512];
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	P_ITEM pi1 = item::CreateFromScript( "$item_blood_puddle" );
	VALIDATEPI(pi1);
	pi1->setId( 0x122A );
	P_ITEM pi2=MAKE_ITEM_REF(npcshape[0]);
	VALIDATEPI(pi2);
	P_ITEM pi3=MAKE_ITEM_REF(i);
	VALIDATEPI(pi3);
	mapRegions->remove(pi1);
	pi1->setPosition( pi2->getPosition() );
	mapRegions->add(pi1); // lord Binary
	pi1->magic=2;//AntiChrist - makes the item unmovable
	pi1->setDecayTime();

	pi1->Refresh();

	//if it's a human corpse
	if(pi3->morey)
	{
		pc->modifyFame(ServerScp::g_nChopFameLoss); // Ripper..lose fame and karma and criminal.
		pc->IncreaseKarma(+ServerScp::g_nChopKarmaLoss);
		pc->sysmsg(TRANSLATE("You lost some fame and karma!"));
		setCrimGrey(pc, ServerScp::g_nChopWillCriminal);//Blue and not attacker and not guild

		//create the Head
		sprintf(temp,"the head of %s",pi3->getSecondaryNameC());
        P_ITEM pi = item::CreateFromScript( "$item_hardcoded" );
        VALIDATEPI(pi);
        pi->setId( 0x1DA0 );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		//AntiChrist & Magius(CHE) - store item's owner, so that lately
		//if we want to know the owner we can do it
		pi->setSameOwnerAs(pi3);
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Heart
		sprintf(temp,"the heart of %s",pi3->getSecondaryNameC());
		pi=item::CreateFromScript( "$item_a_heart" );
		VALIDATEPI(pi);
		pi->setCurrentName( temp );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Body
		sprintf(temp,"the body of %s",pi3->getSecondaryNameC());
        pi = item::CreateFromScript( "$item_hardcoded" );
        VALIDATEPI(pi);
        pi->setId( 0x1DAD );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Left Arm
		sprintf(temp,"the left arm of %s",pi3->getSecondaryNameC());
        pi = item::CreateFromScript( "$item_hardcoded" );
        VALIDATEPI(pi);
        pi->setId( 0x1DA1 );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Right Arm
		sprintf(temp,"the right arm of %s",pi3->getSecondaryNameC());
        pi = item::CreateFromScript( "$item_hardcoded" );
        VALIDATEPI(pi);
        pi->setId( 0x1DA2 );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Left Leg
		sprintf(temp,"the left leg of %s",pi3->getSecondaryNameC());
        pi = item::CreateFromScript( "$item_hardcoded" );
        VALIDATEPI(pi);
        pi->setId( 0x1DAE );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Rigth Leg
		sprintf(temp,"the right leg of %s",pi3->getSecondaryNameC());
        pi = item::CreateFromScript( "$item_hardcoded" );
        VALIDATEPI(pi);
        pi->setId( 0x1DA4 );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//human: always delete corpse!
		deletecorpse=true;
	}
		else
	{
		cScpIterator* iter = NULL;
		char script1[1024];
		char script2[1024];
		sprintf(sect,"SECTION CARVE %i",pi3->carve);
		iter = Scripts::Carve->getNewIterator(sect);
		if (iter==NULL) return;


		int loopexit=0;
		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if (!(strcmp("ADDITEM",script1)))
				{
					std::string itemnum, amount;
					splitLine( script2, itemnum, amount );
					int amt = str2num( amount );
					if( amt == 0 )
						amt=INVALID;
					P_ITEM pi = item::CreateFromScript( (char*)itemnum.c_str(), pi3, amt );
					if( ISVALIDPI(pi) ) {
						pi->layer=0;
						pi->Refresh();//let's finally refresh the item
					}
                }
            }
        }
        while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

        safedelete(iter);
    }


    if(deletecorpse)//if corpse has to be deleted
    {

		NxwItemWrapper si;
		si.fillItemsInContainer( pi3, false );
		for( si.rewind(); !si.isEmpty(); si++ )
        {
            P_ITEM pj=si.getItem();
			if(ISVALIDPI(pj)) {
				pj->setContSerial(INVALID);
				pj->MoveTo( pi3->getPosition() );
				pj->setDecayTime();
				pj->Refresh();
			}
        }
        pi3->Delete();
    }
}

static void CorpseTarget(const NXWCLIENT pC)
{
	if (pC == NULL) return;
    int n=0;
    NXWSOCKET  s = pC->toInt();

    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcItemFromSer(serial);
    P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPI(pi);
    P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
    if(ISVALIDPI(pi))
    {
        if(item_inRange(pc,pi,1))
        {
            npcshape[0]=i;
            pc->playAction(0x20);
            n=1;
            if(pi->more1==0)
            {
                pi->more1=1;//corpse being carved...can't carve it anymore

                if(pi->morey || pi->carve>-1)
                {//if specified, use enhanced carving system!
                    newCarveTarget(s, i);//AntiChrist
                } else
                {//else use standard carving
                    switch(pi->amount) {
                    case 0x01: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Ogre
                    case 0x02: CarveTarget(s, 0, 5, 0, 0, 0, 0); break; //Ettin
                    case 0x03: break;   //Zombie
                    case 0x04: break;   //Gargoyle
                    case 0x05: CarveTarget(s,36, 0, 0, 0, 0, 1); break; //Eagle
                    case 0x06: CarveTarget(s,25, 0, 0, 0, 0, 1); break; //Bird
                    case 0x07: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Orc w/axe
                    case 0x08: break;   //Corpser
                    case 0x09: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Deamon
                    case 0x0A: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Deamon w/sword
                    case 0x0B: break;   //-NULL-
                    case 0x0C: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Dragon (green)
                    case 0x0D: break;   //Air Elemental
                    case 0x0E: break;   //Earth Elemental
                    case 0x0F: break;   //Fire Elemental
                    case 0x10: break;   //Water Elemental
                    case 0x11: CarveTarget(s, 0, 3, 0, 0, 0, 0); break; //Orc
                    case 0x12: CarveTarget(s, 0, 5, 0, 0, 0, 0); break; //Ettin w/club
                    case 0x13: break; //-NULL-
                    case 0x14: break; //-NULL-
                    case 0x15: CarveTarget(s, 0, 4,20, 0, 0, 0); break; //Giant Serpent
                    case 0x16: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Gazer
                    case 0x17: break;   //-NULL-
                    case 0x18: break;   //Liche
                    case 0x19: break;   //-NULL-
                    case 0x1A: break;   //Ghoul
                    case 0x1B: break;   //-NULL-
                    case 0x1C: break;   //Spider
                    case 0x1D: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Gorilla
                    case 0x1E: CarveTarget(s,50, 0, 0, 0, 0, 1); break; //Harpy
                    case 0x1F: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Headless
                    case 0x20: break;   //-NULL-
                    case 0x21: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman
                    case 0x0122: CarveTarget(s, 0,10, 0, 0, 0, 0); break; // Boar
                    case 0x23: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman w/spear
                    case 0x24: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman w/mace
                    case 0x25: break;   //-NULL-
                    case 0x26: break;   //-NULL-
                    case 0x27: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Mongbat
                    case 0x28: break;   //-NULL-
                    case 0x29: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Orc w/club
                    case 0x2A: break;   //Ratman
                    case 0x2B: break;   //-NULL-
                    case 0x2C: break;   //Ratman w/axe
                    case 0x2D: break;   //Ratman w/dagger
                    case 0x2E: break;   //-NULL-
                    case 0x2F: break;   //Reaper
                    case 0x30: break;   //Scorpion
                    case 0x31: break;   //-NULL-
                    case 0x32: break;   //Skeleton
                    case 0x33: break;   //Slime
                    case 0x34: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Snake
                    case 0x35: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll w/axe
                    case 0x36: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll
                    case 0x37: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll w/club
                    case 0x38: break;   //Skeleton w/axe
                    case 0x39: break;   //Skeleton w/sword
                    case 0x3A: break;   //Wisp
                    case 0x3B: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Dragon (red)
                    case 0x3C: CarveTarget(s, 0,10,20, 0, 0, 0); break; //Drake (green)
                    case 0x3D: CarveTarget(s, 0,10,20, 0, 0, 0); break; //Drake (red)
                    case 0x46: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen Matriarche - t2a
                    case 0x47: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen drone - t2a
                    case 0x48: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen warrior, Terathen Avenger - t2a
                    case 0x4B: CarveTarget(s, 0,4, 0, 0, 0, 0); break; //Titan - t2a
                    case 0x4C: CarveTarget(s, 0, 4, 0, 0, 0, 0); break; //Cyclopedian Warrior - t2a
                    case 0x50: CarveTarget(s, 0,10, 2, 0, 0, 0); break; //Giant Toad - t2a
                    case 0x51: CarveTarget(s, 0, 4, 1, 0, 0, 0); break; //Bullfrog - t2a
                    case 0x55: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian apprentice, Ophidian Shaman - t2a
                    case 0x56: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian warrior, Ophidian Enforcer, Ophidian Avenger - t2a
                    case 0x57: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian Matriarche - t2a
                    case 0x5F: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Kraken - t2a
                        //case 0x3E-case 0x95: break; //-NULL-
                    case 0x96: CarveTarget(s, 0,10, 0, 0, 0, 0); break; //Sea Monster
                    case 0x97: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Dolphin
                        //case 0x98-case 0xC7: break; //-NULL-
                    case 0xC8: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (tan)
                    case 0xC9: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Cat
                    case 0xCA: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Alligator
                    case 0xCB: CarveTarget(s, 0, 6, 0, 0, 0, 0); break; //Pig
                    case 0xCC: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (dark)
                    case 0xCD: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Rabbit
                    case 0xCE: CarveTarget(s, 0, 1,12, 0 ,0, 0); break; //Lava Lizard - t2a
                    case 0xCF: CarveTarget(s, 0, 3, 0, 0, 1, 0); break; //Sheep
                    case 0xD0: CarveTarget(s,25, 0, 0, 0, 0, 1); break; //Chicken
                    case 0xD1: CarveTarget(s, 0, 2, 8, 0, 0, 0); break; //Goat
                    case 0xD2: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Desert Ostarge - t2a
                    case 0xD3: CarveTarget(s, 0, 1, 12, 0, 0, 0); break; //Bear
                    case 0xD4: CarveTarget(s, 0, 1, 0, 2, 0, 0); break; //Grizzly Bear
                    case 0xD5: CarveTarget(s, 0, 2, 0, 3, 0, 0); break; //Polar Bear
                    case 0xD6: CarveTarget(s, 0, 1, 10, 0, 0, 0); break; //Cougar
                    case 0xD7: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Giant Rat
                    case 0xD8: CarveTarget(s, 0, 8,12, 0, 0, 0); break; //Cow (black)
                    case 0xD9: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Dog
                    case 0xDA: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Frenzied Ostard - t2a
                    case 0xDB: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Forest Ostard - t2a
                    case 0xDC: CarveTarget(s, 0, 1, 12,0, 0, 0); break; //Llama
                    case 0xDD: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Walrus
                    case 0xDE: break;   //-NULL-
                    case 0xDF: CarveTarget(s, 0, 3, 0, 0, 0, 0); break; //Sheep (BALD)
                    case 0xE1: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Timber Wolf
                    case 0xE2: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (Silver)
                    case 0xE3: break;   //-NULL-
                    case 0xE4: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (tan)
                    case 0xE5: break;   //-NULL-
                    case 0xE6: break;   //-NULL-
                    case 0xE7: CarveTarget(s, 0, 8,12, 0, 0, 0); break; //Cow (brown)
                    case 0xE8: CarveTarget(s, 0,10,15, 0, 0, 0); break; //Bull (brown)
                    case 0xE9: CarveTarget(s, 0,10,15, 0, 0, 0); break; //Bull (d-brown)
                    case 0xEA: CarveTarget(s, 0, 6,15, 0, 0, 0); break; //Great Heart
                    case 0xEB: break;   //-NULL-
                    case 0xEC: break;   //-NULL-
                    case 0xED: CarveTarget(s, 0, 5, 8, 0, 0, 0); break; //Hind
                    case 0xEE: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Rat
                        //case 0xEF-case 0xFF: break; //-NULL-
                    default:
                        LogError("Fallout of switch statement, corpsetarget()");
                    }// switch
                }//if morey || carve>-1
            } else {
                 sysmessage(s, TRANSLATE("You carve the corpse but find nothing usefull."));
            }// if more1==0
        //break;
        }
    }// if i!=-1
    if (!n) sysmessage(s, TRANSLATE("That is too far away."));
}




int BuyShop(NXWSOCKET s, SERIAL c)
{
    P_ITEM buyRestockContainer=NULL, buyNoRestockContainer=NULL;

    P_CHAR pc = MAKE_CHAR_REF(c);
	VALIDATEPCR(pc,0);
	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(curr,0);


	NxwItemWrapper si;
	si.fillItemWeared( pc, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ )
    {
        P_ITEM pi=si.getItem();
		if(!ISVALIDPI(pi)) continue;

		if( pi->layer==LAYER_TRADE_RESTOCK )
            buyRestockContainer=pi;

        if( pi->layer==LAYER_TRADE_NORESTOCK )
            buyNoRestockContainer=pi;

		if( ISVALIDPI(buyRestockContainer) && ISVALIDPI(buyNoRestockContainer) )
			break;
    }

    if (!ISVALIDPI(buyRestockContainer) || !ISVALIDPI(buyNoRestockContainer) )
        return 0;

    impowncreate(s, pc, 0); // Send the NPC again to make sure info is current. (OSI does this we might not have to)

    sendshopinfo(s, DEREF_P_CHAR(pc), buyRestockContainer); // Send normal shop items
//  sendshopinfo(s, c, buyNoRestockContainer); // Send items sold to shop by players
    SndShopgumpopen(s,pc->getSerial32());
    statwindow(curr,curr); // Make sure the gold total has been sent.
    return 1;
}



void target_playerVendorBuy( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc = MAKE_CHAR_REF(t->buffer[0]);
	VALIDATEPC(pc);
    P_CHAR pc_currchar = ps->currChar();
	VALIDATEPC(pc_currchar);

	NXWSOCKET s = ps->toInt();

    P_ITEM pBackpack= pc_currchar->getBackpack();
    if (!pBackpack) {sysmessage(s,TRANSLATE("Time to buy a backpack")); return; } //LB

    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    P_ITEM pi=pointers::findItemBySerial(serial);     // the item
    if (pi==NULL) return;
    if (pi->isInWorld()) return;
    int price=pi->value;


	P_ITEM thepack=(P_ITEM)pi->getContainer();
	VALIDATEPI(thepack);
	P_CHAR pNpc= thepack->getPackOwner();               // the vendor

    if(DEREF_P_CHAR(pNpc)!=pc->getSerial32() || pc->npcaitype!=NPCAI_PLAYERVENDOR) return;

    if (pc_currchar->isOwnerOf(pc))
    {
        pc->talk(s, TRANSLATE("I work for you, you need not buy things from me!"),0);
        return;
    }

    int gleft=pc_currchar->CountGold();
    if (gleft<pi->value)
    {
        pc->talk(s, TRANSLATE("You cannot afford that."),0);
        return;
    }
    pBackpack->DeleteAmount(price,0x0EED);  // take gold from player

    pc->talk(s, TRANSLATE("Thank you."),0);
    pc->holdg+=pi->value; // putting the gold to the vendor's "pocket"

    // sends item to the proud new owner's pack
    pi->setContSerial( pBackpack->getSerial32() );
    pi->Refresh();

}


void target_envoke( NXWCLIENT ps, P_TARGET t )
{
	
	P_CHAR curr=ps->currChar();
	
	SERIAL serial=t->getClicked();
	if( isItemSerial( serial ) )
	{
        P_ITEM pi = MAKE_ITEM_REF(serial);
		VALIDATEPI( pi );
        triggerItem( ps->toInt(),pi, TRIGTYPE_ENVOKED );
        curr->envokeid=0x0000;
    }
	else if( isCharSerial( serial ) )
	{
        P_CHAR pc = MAKE_CHAR_REF(serial);
		VALIDATEPC(pc);
        triggerNpc( ps->toInt(), pc, TRIGTYPE_NPCENVOKED );
        curr->envokeid=0x0000;
    }
	else {
        triggerTile( ps->toInt() );
        curr->envokeid=0x0000;
	}
}


void target_key( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	P_ITEM pi = pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	NXWSOCKET s = ps->toInt();

    if ((pi->more1==0)&&(pi->more2==0)&&
            (pi->more3==0)&&(pi->more4==0))
        {
            if ( pi->type==ITYPE_KEY && (item_inRange(MAKE_CHAR_REF(currchar[s]),pi,2) || (!pi->isInWorld()) ) )
            {
                if (!pc->checkSkill(TINKERING, 400, 1000))
                {
                    sysmessage(s,TRANSLATE("You fail and destroy the key blank."));
                    // soundeffect3( pi, <whatever> );
                    pi->Delete();
                }
                else
                {
                    pi->more1=t->buffer[0];
                    pi->more2=t->buffer[1];
                    pi->more3=t->buffer[2];
                    pi->more4=t->buffer[3];
                    // soundeffect3( pi, <whatever> );
                    sysmessage(s, TRANSLATE("You copy the key.")); //Morrolan can copy keys
                }
            }
            return;
        }//if
        else if (((pi->more1==t->buffer[0])&&(pi->more2==t->buffer[1])&&
            (pi->more3==t->buffer[2])&&(pi->more4==t->buffer[3]))||
            (t->buffer[0]==(unsigned char) 0xFF))
        {
            if (((pi->type==ITYPE_CONTAINER)||(pi->type==ITYPE_UNLOCKED_CONTAINER))&&(item_inRange(pc,pi,2)))
            {
                if(pi->type==ITYPE_CONTAINER) pi->type=ITYPE_LOCKED_ITEM_SPAWNER;
                if(pi->type==ITYPE_UNLOCKED_CONTAINER) pi->type=ITYPE_LOCKED_CONTAINER;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You lock the container."));
                return;
            }
            else if ((pi->type==ITYPE_KEY)&&(item_inRange(pc,pi,2)))
            {
                pc->keyserial=pi->getSerial32();
                sysmessage(s,TRANSLATE("Enter new name for key."));//morrolan rename keys
                return;
            }
            else if ((pi->type==ITYPE_LOCKED_ITEM_SPAWNER)||(pi->type==ITYPE_LOCKED_CONTAINER)&&(item_inRange(pc,pi,2)))
            {
                if(pi->type==ITYPE_LOCKED_ITEM_SPAWNER) pi->type=ITYPE_CONTAINER;
                if(pi->type==ITYPE_LOCKED_CONTAINER) pi->type=ITYPE_UNLOCKED_CONTAINER;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You unlock the container."));
                return;
            }
            else if ((pi->type==ITYPE_DOOR)&&(item_inRange(pc,pi,2)))
            {
                pi->type=ITYPE_LOCKED_DOOR;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You lock the door."));
                return;
            }
            else if ((pi->type==ITYPE_LOCKED_DOOR)&&(item_inRange(pc,pi,2)))
            {
                pi->type=ITYPE_DOOR;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You unlock the door."));
                return;
            }
            else if (pi->getId()==0x0BD2)
            {
                sysmessage(s, TRANSLATE("What do you wish the sign to say?"));
                pc->keyserial=pi->getSerial32(); //Morrolan sign kludge
                return;
            }

            //Boats ->
            else if(pi->type==ITYPE_BOATS && pi->type2==3)
            {
                Boats->OpenPlank(pi);
                pi->Refresh();
            }
            //End Boats --^
        }//else if
        else
        {
            if (pi->type==ITYPE_KEY) sysmessage (s, TRANSLATE("That key is not blank!"));
            else if (pi->more1==0x00) sysmessage(s, TRANSLATE("That does not have a lock."));
            else sysmessage(s, TRANSLATE("The key does not fit into that lock."));
            return;
        }//else
}

void target_attack( NXWCLIENT ps, P_TARGET t ) 
{

    P_CHAR pc_t1= pointers::findCharBySerial( t->buffer[0] );
    VALIDATEPC(pc_t1);
	P_CHAR pc_t2=pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc_t2);

	NXWSOCKET s = ps->toInt();

    AttackStuff(s,pc_t2); //xan : flag them all!
    npcattacktarget(pc_t1, pc_t2);
}

void target_follow( NXWCLIENT ps, P_TARGET t ) 
{

    P_CHAR pc = pointers::findCharBySerial( t->buffer[0] );
    VALIDATEPC(pc);

	P_CHAR pc2 = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc2);

    pc->ftargserial=pc2->getSerial32();
    pc->npcWander=WANDER_FOLLOW;
}

void target_axe( NXWCLIENT ps, P_TARGET t )
{
    UI16 id=t->getModel();
    if (itemById::IsTree(id))
		Skills::target_tree( ps, t );
    else if (itemById::IsCorpse(id))
        CorpseTarget(ps);
    else if (itemById::IsLog(id)) //Luxor bug fix
		Skills::target_bowcraft( ps, t );
}


void target_sword( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);

	UI16 id = t->getModel();
	if (itemById::IsTree2(id))
	{
		Location pcpos= pc->getPosition();
		Location location = t->getLocation();

		if( dist( location, pcpos )>5 )
		{
			pc->sysmsg(TRANSLATE("You are to far away to reach that"));
			return;
		}

		pc->playAction( pc->isMounting() ? 0x0D : 0x01D );
		pc->playSFX(0x013E);

		P_ITEM pi=item::CreateFromScript( "$item_kindling" );
		VALIDATEPI(pi);

		pi->setPosition( pcpos );
		mapRegions->add(pi);

		pi->Refresh();
		pc->sysmsg(TRANSLATE("You hack at the tree and produce some kindling."));
	}
	else if(itemById::IsLog(id)) // vagrant
	{
		Skills::target_bowcraft( ps, t );
	} 
	else if(itemById::IsCorpse(id))
	{
		CorpseTarget(ps);
	} 
	else
		pc->sysmsg(TRANSLATE("You can't think of a way to use your blade on that."));
}

void target_fetch( NXWCLIENT ps, P_TARGET t )
{
    ps->sysmsg( TRANSLATE("Fetch is not available at this time.") );
}

void target_guard( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	P_CHAR pPet = pointers::findCharBySerial(t->buffer[0]);
    VALIDATEPC(pPet);

    P_CHAR pToGuard = pointers::findCharBySerial( t->getClicked() );
    if( !ISVALIDPC(pToGuard) || pToGuard->getSerial32() != pPet->getOwnerSerial32() )
    {
        ps->sysmsg( TRANSLATE("Currently can't guard anyone but yourself!" ));
        return;
    }
    pPet->npcaitype = NPCAI_PETGUARD;
    pPet->ftargserial=pc->getSerial32();
    pPet->npcWander=WANDER_FOLLOW;
    ps->sysmsg( TRANSLATE("Your pet is now guarding you."));
    pc->guarded = true;
}

void target_transfer( NXWCLIENT ps, P_TARGET t )
{

    P_CHAR pc1 = pointers::findCharBySerial( t->buffer[0] );
	VALIDATEPC(pc1);
    P_CHAR pc2 = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc2);

	//Araknesh Call OnTransfer Event Passing Animal,NewOwner

	if (pc1->amxevents[EVENT_CHR_ONTRANSFER])
	{
		g_bByPass = false;
		pc1->amxevents[EVENT_CHR_ONTRANSFER]->Call(pc1->getSerial32(),pc2->getSerial32());
		if (g_bByPass==true) return ;
	}
	/*
	pc1->runAmxEvent( EVENT_CHR_ONTRANSFER, pc1->getSerial32(),pc2->getSerial32());
	if (g_bByPass==true)
		return ;
	*/
    char bb[120];
    sprintf(bb,TRANSLATE("* %s will now take %s as his master *"), pc1->getCurrentNameC(), pc2->getCurrentNameC());
    pc1->talkAll(bb,0);

    pc1->setOwner( pc2 );
    pc1->npcWander=WANDER_FOLLOW;
    pc1->ftargserial=INVALID;
    pc1->npcWander=WANDER_NOMOVE;
}

 //Throws the potion and places it (unmovable) at that spot
void target_expPotion( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

    Location loc=t->getLocation();

	NXWSOCKET s=ps->toInt();

    if(line_of_sight(s, pc->getPosition(), loc, WALLS_CHIMNEYS + DOORS + ROOFING_SLANTED))
    {
        P_ITEM pi=pointers::findItemBySerial( t->buffer[0] );
        if (ISVALIDPI(pi)) // crashfix LB
        {
            pi->MoveTo( loc );
            pi->setContSerial(INVALID);
            pi->magic=2; //make item unmovable once thrown
            movingeffect2(DEREF_P_CHAR(pc), DEREF_P_ITEM(pi), 0x0F, 0x0D, 0x11, 0x00, 0x00);
            pi->Refresh();
        }
    }
    else 
		pc->sysmsg(TRANSLATE("You cannot throw the potion there!"));
}


void target_trigger( NXWCLIENT ps, P_TARGET t )
{

	P_ITEM pi = MAKE_ITEM_REF(t->getClicked());
	VALIDATEPI(pi);
    
	triggerItem(ps->toInt(), pi, TRIGTYPE_TARGET);

}

void target_npcMenu( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	npcs::AddRespawnNPC(pc,t->buffer[0]);
}

/*!
\brief implements the 'telestuff GM command
\author Endymion
*/
void target_telestuff( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);

	NXWSOCKET s = ps->toInt();

	P_OBJECT po = objects.findObject( t->getClicked() );

	if( ISVALIDPO(po) ) { //clicked on obj to move
		P_TARGET targ=clientInfo[s]->newTarget( new cLocationTarget() );
		targ->code_callback=target_telestuff;
		targ->buffer[0]=po->getSerial32();
		targ->send(ps);
		ps->sysmsg( TRANSLATE("Select location to put this object.") );
	} else { //on ground.. so move it
		
		Location loc=t->getLocation();
		loc.z+=tileHeight( t->getModel() );

		SERIAL serial = t->buffer[0];
		if( isCharSerial(serial) ) {
			P_CHAR pt = pointers::findCharBySerial( serial );
			VALIDATEPC(pt);

			pt->MoveTo( loc );
			pt->teleport();
		} else if ( isItemSerial(serial) ) {
			P_ITEM pi = pointers::findItemBySerial( serial );
			VALIDATEPI(pi);

			pi->MoveTo(loc);
			pi->Refresh();
		}
	}
}

/*!
\author Luxor
\param s socket to attack
\brief Manages all attack command
*/
void target_allAttack( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

    P_CHAR pc_target = pointers::findCharBySerial( t->getClicked() );
    VALIDATEPC(pc_target);


	NxwCharWrapper sc;
    sc.fillOwnedNpcs( pc, false, true );
    pc->attackStuff(pc_target);
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pet=sc.getChar();
		if( ISVALIDPC(pet))
			npcattacktarget(pet, pc_target);
    }

}



void target_xTeleport( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	SERIAL serial = t->getClicked();
	if( isCharSerial( serial ) ) {
		P_CHAR pc_i = pointers::findCharBySerial( serial );
		if( ISVALIDPC( pc_i ) )
		{
			pc_i->MoveTo( pc->getPosition() );
			pc_i->teleport();
		}
	}
	else if( isItemSerial( serial ) ) {
		P_ITEM pi = pointers::findItemBySerial( serial );
		if( ISVALIDPI( pi ) ) {
			pi->MoveTo( pc->getPosition() );
			pi->Refresh();
		}
	}
}




