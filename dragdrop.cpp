  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

// dragdrop.cpp: implementation of dragging and dropping
// cut from NoX-Wizard.cpp by Duke, 23.9.2000
//////////////////////////////////////////////////////////////////////

/* 									CHANGELOG
	--------------------------------------------------------------------------------------------------------------------
	Date		Developer	Description
	--------------------------------------------------------------------------------------------------------------------
 	20-02-2002  	Sparhawk 	Fixed disappearing items after dragging them on paperdoll, set checks in item_bouncex()
					from id1 >= 0x40 to id1 < 0x40
					Removed obsolete item checks in wear_item()
					Added bounce_item call when non gm player tries to drag item on paperdoll of gm.
	24-02-2002	Sparhawk	Fixed	compile error on void return values in CheckWhereItem and cleaned code
	15-03-2002	Sparhawk	Optimized abstract container size() in loops

*/


#include "nxwcommn.h"
#include "network.h"
#include "debug.h"
#include "basics.h"
#include "sndpkg.h"
#include "speech.h"
#include "itemid.h"
#include "bounty.h"
#include "srvparms.h"
#include "set.h"
#include "layer.h"
#include "npcai.h"
#include "nxw_utils.h"
#include "weight.h"
#include "data.h"
#include "books.h"
#include "boats.h"
#include "archive.h"
#include "trade.h"
#include "map.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "skills.h"
#include "nox-wizard.h"
#include "party.h"



typedef struct _PKGx08
{
//0x08 Packet
//Drop Item(s) (14 bytes)
//* BYTE cmd
//* BYTE[4] item id
	long Iserial;
//* BYTE[2] xLoc
	short TxLoc;
//* BYTE[2] yLoc
	short TyLoc;
//* BYTE zLoc
	signed char TzLoc;
//* BYTE[4] Move Into (FF FF FF FF if normal world)
	long Tserial;
} PKGx08;

void UpdateStatusWindow(NXWSOCKET socket, P_ITEM pi)
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[socket] );
	VALIDATEPC( pc );
	VALIDATEPI( pi );
	P_ITEM pack = pc->getBackpack();
	VALIDATEPI( pack );

	if( pi->getContSerial() != pack->getSerial32() || pi->getContSerial() == pc->getSerial32() )
		statwindow( pc, pc );
}

static void Sndbounce5( NXWSOCKET socket )
{
	if ( socket >= 0 && socket < now)
	{
		unsigned char bounce[2]= { 0x27, 0x00 };
		bounce[1] = 5;
		Xsend(socket, bounce, 2);
	}
}

// Name:	item_bounce3
// Purpose:	holds some statements that were COPIED some 50 times
// Remarks:	temporary functions to revamp the 30 occurences of the 'bouncing bugfix'
// History:	init Duke, 10.8.2000 / bugfix for bonus stats, Xanathar, 05-aug-2001
static void item_bounce3(const P_ITEM pi)
{
	VALIDATEPI( pi );
	pi->setContSerial( pi->getContSerial(true) );
	pi->setPosition( pi->getOldPosition() );
	pi->layer=pi->oldlayer;

	P_CHAR pc = pointers::findCharBySerial( pi->getContSerial(true) );
	if(pc==NULL)
		return ;
	VALIDATEPC( pc );
	if ( pi->layer > 0 )
	{
		// Xanathar -- add BONUS STATS given by equipped special items
		pc->setStrength( pc->getStrength() + pi->st2, true );
		//pc->st += pi->st2;
		pc->dx += pi->dx2;
		pc->in += pi->in2;
		// Xanathar -- for poisoned items
		if (pi->poisoned)
		{
			pc->poison += pi->poisoned;
			if ( pc->poison < 0)
				pc->poison = 0;
		}
	}
}

static void item_bounce4(const NXWSOCKET  socket, const P_ITEM pi)
{
	VALIDATEPI( pi );
	item_bounce3(pi);
	if( (pi->getId() >>8) < 0x40)
		senditem( socket, pi );
}

static void item_bounce5(const NXWSOCKET socket, const P_ITEM pi)
{
	VALIDATEPI( pi );
	item_bounce3(pi);
	senditem(socket, pi);
}

static void item_bounce6(const NXWCLIENT client, const P_ITEM pi)
{
	if ( client != NULL )
	{
		VALIDATEPI(pi);
		Sndbounce5( client->toInt() );
		if ( client->isDragging() )
		{
			client->resetDragging();
			item_bounce4( client->toInt(), pi );
		}
	}
}


/*!
\brief Get an item
\author Unknow, revamped by Endymion
\param client the client
*/
void get_item( NXWCLIENT client ) // Client grabs an item
{
	if ( client == NULL)
		return;

	P_CHAR pc_currchar = client->currChar();
	VALIDATEPC( pc_currchar );

	NXWSOCKET s = client->toInt();

	P_ITEM pi = pointers::findItemBySerPtr(buffer[s]+1);
	VALIDATEPI(pi);

	//Luxor: not-movable items
	/*if (pi->magic == 2 || (isCharSerial(pi->getContSerial()) && pi->getContSerial() != pc_currchar->getSerial32()) ) {
		if (isCharSerial(pi->getContSerial())) {
			P_CHAR pc_i = pointers::findCharBySerial(pi->getContSerial());
			if (ISVALIDPC(pc_i))
				pc_i->sysmsg("Warning, backpack bug located!");
		}
		if (client->isDragging()) {
        		client->resetDragging();
			UpdateStatusWindow(s,pi);
        	}
		pi->setContSerial( pi->getContSerial(true) );
		pi->setPosition( pi->getOldPosition() );
		pi->layer = pi->oldlayer;
		pi->Refresh();
		return;
	}*/

	pc_currchar->disturbMed(); // Meditation

	tile_st item;
 	data::seekTile( pi->getId(), item );

	// Check if item is equiped
 	if( pi->getContSerial() == pc_currchar->getSerial32() && pi->layer == item.quality )
 	{
 		if( pc_currchar->UnEquip( pi, 1 ) == 1 )	// bypass called
 		{
 			if( client->isDragging() )
 			{
 				UI08 cmd[1]= {0x29};
 				client->resetDragging();
 				Xsend(s, cmd, 1);
//AoS/				Network->FlushBuffer(s);
 			}
 			return;
 		}
 	}


	P_CHAR owner=NULL;
	P_ITEM container=NULL;
	if ( !pi->isInWorld() ) { // Find character owning item

		if ( isCharSerial( pi->getContSerial()))
		{
			owner = pointers::findCharBySerial( pi->getContSerial());
		}
		else  // its an item
		{
			//Endymion Bugfix:
			//before check the container.. but if this cont is a subcont?
			//so get the outmostcont and check it else:
			//can loot without lose karma in subcont
			//can steal in trade ecc
			//not very good :P
			container = pi->getOutMostCont();
			if( isCharSerial( container->getContSerial() ) )
				owner=pointers::findCharBySerial( container->getContSerial() );
		}

		if ( ISVALIDPC( owner ) && owner->getSerial32()!=pc_currchar->getSerial32() )
		{
			if ( !pc_currchar->IsGM() && owner->getOwnerSerial32() != pc_currchar->getSerial32() )
			{// Own serial stuff by Zippy -^ Pack aniamls and vendors.
				UI08 bounce[2]= { 0x27, 0x00 };
				Xsend(s, bounce, 2);
//AoS/				Network->FlushBuffer(s);
				if (client->isDragging())
				{
					client->resetDragging();
					pi->setContSerial(pi->getContSerial(),true,false);
					item_bounce3(pi);
				}
				return;
			}
		}
	}

	if ( ISVALIDPI( container ) )
	{

		if ( container->layer == 0 && container->getId() == 0x1E5E)
		{
			// Trade window???
			SERIAL serial = calcserial( pi->moreb1, pi->moreb2, pi->moreb3, pi->moreb4);
			if ( serial == INVALID )
				return;

			P_ITEM piz = pointers::findItemBySerial(serial );
			if ( ISVALIDPI( piz ) )
				if ( piz->morez || container->morez )
				{
					piz->morez = 0;
					container->morez = 0;
					sendtradestatus( piz, container );
				}


			//<Luxor>
			if (pi->amxevents[EVENT_ITAKEFROMCONTAINER]!=NULL)
			{
				g_bByPass = false;
				pi->amxevents[EVENT_ITAKEFROMCONTAINER]->Call( pi->getSerial32(), pi->getContSerial(), pc_currchar->getSerial32() );
				if (g_bByPass)
				{
					Sndbounce5(s);
					if (client->isDragging())
					{
						client->resetDragging();
						UpdateStatusWindow(s,pi);
					}
					pi->setContSerial( pi->getContSerial(true) );
					pi->setPosition( pi->getOldPosition() );
					pi->layer = pi->oldlayer;
					pi->Refresh();
					return;
                		}
			}
			//</Luxor>

			/*
			//<Luxor>
			g_bByPass = false;
			pi->runAmxEvent( EVENT_ITAKEFROMCONTAINER, pi->getSerial32(), pi->getContSerial(), s );
			if (g_bByPass)
			{
				Sndbounce5(s);
				if (client->isDragging())
				{
					client->resetDragging();
					UpdateStatusWindow(s,pi);
				}
				pi->setContSerial( pi->getContSerial(true) );
				pi->setPosition( pi->getOldPosition() );
				pi->layer = pi->oldlayer;
				pi->Refresh();
				return;
			}
			//</Luxor>
			*/

			if ( container->corpse )
			{
				if ( container->getOwnerSerial32() != pc_currchar->getSerial32())
				{ //Looter :P

					pc_currchar->unHide();
					bool bCanLoot = false;
					if( pc_currchar->party!=INVALID ) {
						P_CHAR dead = pointers::findCharBySerial( container->getOwnerSerial32() ) ;
						if( ISVALIDPC( dead ) && dead->party==pc_currchar->party ) {
							P_PARTY party = Partys.getParty( pc_currchar->party );
							if( party!=NULL ) {
								P_PARTY_MEMBER member = party->getMember( pc_currchar->getSerial32() );
								if( member!=NULL )
									bCanLoot = member->canLoot;
							}
						}
					}
					if ( !bCanLoot && container->more2==1 )
					{
						pc_currchar->IncreaseKarma(-5);
						setCrimGrey(pc_currchar, ServerScp::g_nLootingWillCriminal);
						pc_currchar->sysmsg( TRANSLATE("You are loosing karma!"));
					}
				}
			} // corpse stuff

			container->SetMultiSerial(INVALID);

			//at end reset decay of container
			container->setDecayTime();

		} // end cont valid
	}

	if ( !pi->corpse )
	{
		UpdateStatusWindow(s, pi);

		tile_st tile;
		data::seekTile( pi->getId(), tile);

		if (!pc_currchar->IsGM() && (( pi->magic == 2 || ((tile.weight == 255) && ( pi->magic != 1))) && !pc_currchar->canAllMove() )  ||
			(( pi->magic == 3|| pi->magic == 4) && !pc_currchar->isOwnerOf( pi )))
		{
			UI08 bounce[2]={ 0x27, 0x00 };
			Xsend(s, bounce, 2);
//AoS/			Network->FlushBuffer(s);
			if (client->isDragging()) // only restore item if it got draggged before !!!
			{
				client->resetDragging();
				item_bounce4(s, pi );
			}
		} // end of can't get
		else
		{
			// AntiChrist bugfix for the bad bouncing bug ( disappearing items when bouncing )
			client->setDragging();
			pi->setOldPosition( pi->getPosition() ); // first let's save the position

			pi->oldlayer = pi->layer;	// then the layer
			pi->layer = 0;

			if (!pi->isInWorld())
				pc_currchar->playSFX(0x0057);

			if (pi->amount>1)
			{
				SI16 amount = ShortFromCharPtr(buffer[s] +5);
				if (amount > pi->amount)
					amount = pi->amount;
				else if (amount < pi->amount)
				{ //get not all but a part of item ( piled? ), but anyway make a new one item

					P_ITEM pin =archive::item::New();
					(*pin)=(*pi);

					pin->amount = pi->amount - amount;

					pin->setContSerial(pi->getContSerial());	//Luxor
					pin->setPosition( pi->getPosition() );

					/*if( !pin->isInWorld() && isItemSerial( pin->getContSerial() ) )
						pin->SetRandPosInCont( (P_ITEM)pin->getContainer() );*/

					if ( pin->getOwnerSerial32() != INVALID )
						pin->setOwnerSerial32( pi->getOwnerSerial32() );


					statwindow(pc_currchar,pc_currchar);
					pin->Refresh();//AntiChrist
				}

				if ( pi->getId() == ITEMID_GOLD)
				{
					P_ITEM pack= pc_currchar->getBackpack();
					if (ISVALIDPI(pack)) // lb
						if ( pi->getContSerial() == pack->getSerial32())
							statwindow(pc_currchar, pc_currchar);
				}

				pi->amount = amount;

			} // end if corpse
#ifdef SPAR_I_LOCATION_MAP
			pointers::delFromLocationMap( pi );
#else
			mapRegions->remove( pi );
#endif
			pi->setPosition( 0, 0, 0 );
			pi->setContSerial( INVALID );
		}
	}

	int amt = 0, wgt;
	wgt = (int)weights::LockeddownWeight( pi, &amt);
	pc_currchar->weight += wgt;
	statwindow(pc_currchar, pc_currchar);
}

void wear_item(NXWCLIENT ps) // Item is dropped on paperdoll
{
	if ( ps == NULL )
		return;
	NXWSOCKET s = ps->toInt();
	if (s < 0)
		return;
	P_CHAR pc=ps->currChar();
	VALIDATEPC( pc );
	P_CHAR pck = pointers::findCharBySerPtr(buffer[s]+6);
	VALIDATEPC( pck );
	if( pck->dead )  //Exploit fix: Dead ppl can't equip anything.
		return;
	P_ITEM pi=pointers::findItemBySerPtr(buffer[s]+1);
	VALIDATEPI(pi);

	bool resetDragging = false;

	if( (pi->getId()>>8) >= 0x40)  // LB, client crashfix if multi-objects are moved to PD
		resetDragging = true;

	tile_st tile;
	int serial/*, letsbounce=0*/; // AntiChrist (5) - new ITEMHAND system

	data::seekTile(pi->getId(), tile);

	if( ( clientDimension[s]==3 ) &&  (tile.quality==0) )
	{
		ps->sysmsg(TRANSLATE("You can't wear that"));
		resetDragging = true;
	}
	else {
		P_ITEM outmost = pi->getOutMostCont();
		P_CHAR vendor = pointers::findCharBySerial( outmost->getContSerial() );
		if( ISVALIDPC( vendor ) && ( vendor->getOwnerSerial32() != pc->getSerial32() ) )
		{
			resetDragging = true;
		}

	}

	if( resetDragging ) {
		Sndbounce5(s);
		if (ps->isDragging())
		{
			ps->resetDragging();
			item_bounce4(s,pi);
			UpdateStatusWindow(s,pi);
		}
		return;
	}

	if ( pck->getSerial32() == pc->getSerial32() || pc->IsGM() )
	{

		if ( !pc->IsGM() && pi->st > pck->getStrength() && !pi->isNewbie() ) // now you can equip anything if it's newbie
		{
			ps->sysmsg(TRANSLATE("You are not strong enough to use that."));
			resetDragging = true;
		}
		else if ( !pc->IsGM() && !checkItemUsability(pc, pi, ITEM_USE_WEAR) )
		{
			resetDragging = true;
		}
		else if ( (pc->getId() == BODY_MALE) && ( pi->getId()==0x1c00 || pi->getId()==0x1c02 || pi->getId()==0x1c04 || pi->getId()==0x1c06 || pi->getId()==0x1c08 || pi->getId()==0x1c0a || pi->getId()==0x1c0c ) ) // Ripper...so males cant wear female armor
		{
			ps->sysmsg(TRANSLATE("You cant wear female armor!"));
			resetDragging = true;
		}
		else if ((((pi->magic==2)||((tile.weight==255)&&(pi->magic!=1))) && !pc->canAllMove()) ||
				( (pi->magic==3|| pi->magic==4) && !(pi->getOwnerSerial32()==pc->getSerial32())))
		{
			resetDragging = true;
		}

		if( resetDragging ) {
			Sndbounce5(s);
			if (ps->isDragging())
			{
				ps->resetDragging();
				item_bounce4(s,pi);
				UpdateStatusWindow(s,pi);
			}
			return;
		}



		// - AntiChrist (4) - checks for new ITEMHAND system
		// - now you can't equip 2 hnd weapons with 1hnd weapons nor shields!!
		serial= pck->getSerial32(); //xan -> k not cc :)

		P_ITEM pj = NULL;
 		P_CHAR pc_currchar= pck;
// 		P_ITEM pack= pc_currchar->getBackpack();
                //<Luxor>

		P_ITEM pW = pc_currchar->getWeapon();
		if (tile.quality == 1 || tile.quality == 2)
		{ //weapons layers
			if ( (pi->layer == LAYER_2HANDWEAPON && ISVALIDPI(pc_currchar->getShield())) )
			{
				ps->sysmsg("You cannot wear two weapons.");
				Sndbounce5(s);
				if (ps->isDragging())
				{
        			ps->resetDragging();
					UpdateStatusWindow(s,pi);
	        	}
				pi->setContSerial( pi->getContSerial(true) );
				pi->setPosition( pi->getOldPosition() );
				pi->layer = pi->oldlayer;
				pi->Refresh();
				return;
			}
			if (ISVALIDPI(pW))
			{
				if (pi->itmhand != 3 && pi->lodamage != 0 && pi->itmhand == pW->itmhand)
				{
					ps->sysmsg("You cannot wear two weapons.");
					Sndbounce5(s);
					if (ps->isDragging())
					{
						ps->resetDragging();
						UpdateStatusWindow(s,pi);
					}
					pi->setContSerial( pi->getContSerial(true) );
					pi->setPosition( pi->getOldPosition() );
					pi->layer = pi->oldlayer;
					pi->Refresh();
					return;
				}
			}
		}
		//</Luxor>

		if ( ServerScp::g_nUnequipOnReequip )
		{
			int drop[2]= {-1, -1};	// list of items to drop
									// there no reason for it to be larger
			int curindex= 0;

			NxwItemWrapper si;
			si.fillItemWeared( pc_currchar, false, true, true );
			for( si.rewind(); !si.isEmpty(); si++ )
			{
				// we CANNOT directly bounce the item, or the containersearch() function will not work
				// so we store the item ID in letsbounce, and at the end we bounce the item

				pj=si.getItem();
				if(!ISVALIDPI(pj))
					continue;

				if ((tile.quality == 1) || (tile.quality == 2))// weapons
				{
					if (pi->itmhand == 2) // two handed weapons or shield
					{
						if (pj->itmhand == 2)
							drop[curindex++]= DEREF_P_ITEM(pj);

						if ( (pj->itmhand == 1) || (pj->itmhand == 3) )
							drop[curindex++]= DEREF_P_ITEM(pj);
					}

					if (pi->itmhand == 3)
					{
						if ((pj->itmhand == 2) || pj->itmhand == 3)
							drop[curindex++]= DEREF_P_ITEM(pj);
					}

					if ((pi->itmhand == 1) && ((pj->itmhand == 2) || (pj->itmhand == 1)))
						drop[curindex++]= DEREF_P_ITEM(pj);
				}
				else	// not a weapon
				{
					if (pj->layer == tile.quality)
						drop[curindex++]= DEREF_P_ITEM(pj);
				}
			}

			if (ServerScp::g_nUnequipOnReequip)
			{
				if (drop[0] > -1)	// there is at least one item to drop
				{
					for (int i= 0; i< 2; i++)
					{
						if (drop[i] > -1)
						{
							P_ITEM p_drop=MAKE_ITEM_REF(drop[i]);
							if(ISVALIDPI(p_drop))
								pc_currchar->UnEquip( p_drop, 1);
						}
					}
				}
				pc->playSFX( itemsfx(pi->getId()) );
				pc_currchar->Equip(pi, 1);
			}
			else
			{
				if (drop[0] == -1)
				{
					pc->playSFX( itemsfx(pi->getId()) );
					pc_currchar->Equip(pi, 1);
				}
			}
		}

		if (!(pc->IsGM())) //Ripper..players cant equip items on other players or npc`s paperdolls.
		{
			if ((pck->getSerial32() != pc->getSerial32())/*&&(chars[s].npc!=k)*/) //-> really don't understand this! :|, xan
			{
				ps->sysmsg(TRANSLATE("You can't put items on other people!"));
				item_bounce6(ps,pi);
				return;
			}
		}

		NxwSocketWrapper sws;
		sws.fillOnline( pi );
		for( sws.rewind(); !sws.isEmpty(); sws++ )
			SendDeleteObjectPkt( sws.getSocket(), pi->getSerial32() );

		pi->layer=buffer[s][5];
		pi->setContSerial(LongFromCharPtr(buffer[s] +6));

		if (g_nShowLayers) InfoOut("Item equipped on layer %i.\n",pi->layer);

		wearIt(s,pi);

		NxwSocketWrapper sw;
		sw.fillOnline( pck, false );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWSOCKET j=sw.getSocket();
			if( j!=INVALID )
				wornitems(j, pck );
		}

		pc->playSFX( itemsfx(pi->getId()) );
		weights::NewCalc(pc);	// Ison 2-20-99
		statwindow(pc_currchar,pc_currchar);

//		if (pi->glow>0)
//		{
//			pc->removeHalo(pi); // if gm equips on differnt player it needs to be deleted out of the hashteble
//			pck->addHalo(pi);
//			pck->glowHalo(pi);
//		}

		if ( pck->Equip(pi, 1) == 2)	// bypass called
		{
			P_ITEM pack = pck->getBackpack();
			pc->playSFX( itemsfx(pi->getId()) );
			pi->layer= 0;
			pi->setContSerial( pack->getSerial32() );
			sendbpitem(s, pi);
			return;
		}

	}
}

static bool ItemDroppedOnPet(NXWCLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	if (ps == NULL) return false;
	VALIDATEPIR(pi, false);
	P_CHAR pet = pointers::findCharBySerial(pp->Tserial);
	VALIDATEPCR(pet, false);
	NXWSOCKET  s = ps->toInt();
	P_CHAR pc = ps->currChar();
	VALIDATEPCR(pc, false);

	if((pet->hunger<6) && (pi->type==ITYPE_FOOD))//AntiChrist new hunger code for npcs
	{
		pc->playSFX( 0x3A+(rand()%3) );	//0x3A - 0x3C three different sounds

		if(pi->poisoned)
		{
			pet->applyPoison(PoisonType(pi->poisoned));
		}

		std::string itmname;
		if( pi->getCurrentName() == "#" )
		{
			char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
			pi->getName(temp2);
			itmname = temp2;
		}
		else itmname = pi->getCurrentName();

		pet->emotecolor = 0x0026;
		pet->emoteall(TRANSLATE("* You see %s eating %s *"), 1, pet->getCurrentNameC(), itmname.c_str() );
		pet->hunger++;
	} else
	{
		ps->sysmsg(TRANSLATE("It doesn't appear to want the item"));
		Sndbounce5(s);
		if (ps->isDragging())
		{
			ps->resetDragging();
			item_bounce5(s,pi);

		}
	}
	return true;
}

static bool ItemDroppedOnGuard(NXWCLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	if (ps == NULL)
		return false;
	VALIDATEPIR(pi, false);
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	NXWSOCKET  s=ps->toInt();
	P_CHAR pc = ps->currChar();
	VALIDATEPCR(pc,false);

	P_CHAR pc_t=pointers::findCharBySerial(pp->Tserial); //the guard
	VALIDATEPCR(pc_t,false);
	// Search for the key word "the head of"
	if( strstr( pi->getCurrentNameC(), "the head of" ) ) //!!! Wrong! it must check the ItemID, not the name :(
	{
		// This is a head of someone, see if the owner has a bounty on them
		P_CHAR own=pointers::findCharBySerial(pi->getOwnerSerial32());
		VALIDATEPCR(own,false);

		if( own->questBountyReward > 0 )
		{
			// Give the person the bounty assuming that they are not the
			// same person as the reward is for
			if( pc->getSerial32() != own->getSerial32() )
			{
				// give them the gold for bringing the villan to justice
				addgold( s, own->questBountyReward );
				pc->playSFX( goldsfx( own->questBountyReward ) );

				// Now thank them for their hard work
				sprintf( temp, TRANSLATE("Excellent work! You have brought us the head of %s. Here is your reward of %d gold coins."),
					own->getCurrentNameC(), own->questBountyReward );
				pc_t->talk( s, temp, 0);

				// Delete the Bounty from the bulletin board
				BountyDelete(own );

				// xan : increment fame & karma :)
				pc->modifyFame( ServerScp::g_nBountyFameGain );
				pc->IncreaseKarma(ServerScp::g_nBountyKarmaGain);
			}
			else
				pc_t->talk( s, TRANSLATE("You can not claim that prize scoundrel. You are lucky I don't strike you down where you stand!"),0);

			// Delete the item
			pi->Delete();
		}
	}
	return true;
}

static bool ItemDroppedOnBeggar(NXWCLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	if (ps == NULL)
		return false;

	VALIDATEPIR(pi, false);

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	NXWSOCKET  s=ps->toInt();
	P_CHAR pc=ps->currChar();
	VALIDATEPCR(pc,false);

	P_CHAR pc_t=pointers::findCharBySerial(pp->Tserial); //beggar
	VALIDATEPCR(pc_t,false);

	if(pi->getId()!=ITEMID_GOLD)
	{
		sprintf(temp,TRANSLATE("Sorry %s i can only use gold"), pc->getCurrentNameC());
		pc_t->talk( s,temp,0);
		Sndbounce5(s);
		if (ps->isDragging())
		{
			ps->resetDragging();
			item_bounce5(s,pi);
			return true;
		}
	}
	else
	{
		sprintf(temp,TRANSLATE("Thank you %s for the %i gold!"), pc->getCurrentNameC(), pi->amount);
		pc_t->talk( s,temp,0);
		if(pi->amount<=100)
		{
			pc->IncreaseKarma(10);
			ps->sysmsg(TRANSLATE("You have gain a little karma!"));
		}
		else if(pi->amount>100)
		{
			pc->IncreaseKarma(50);
			ps->sysmsg(TRANSLATE("You have gain some karma!"));
		}
		pi->Delete();
		return true;
	}
	return true;
}

static bool ItemDroppedOnTrainer(NXWCLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	if (ps == NULL) return false;
	VALIDATEPIR(pi, false);
	NXWSOCKET  s = ps->toInt();
	P_CHAR pc = ps->currChar();
	VALIDATEPCR(pc,false);
	P_CHAR pc_t = pointers::findCharBySerial(pp->Tserial);
	VALIDATEPCR(pc_t,false);

	if( pi->getId() == ITEMID_GOLD )
	{ // They gave the NPC gold
		UI08 sk=pc_t->trainingplayerin;
		pc_t->talk( s, TRANSLATE("I thank thee for thy payment. That should give thee a good start on thy way. Farewell!"),0);

		int sum = pc->getSkillSum();
		int delta = pc_t->getTeachingDelta(pc, sk, sum);

		if(pi->amount>delta) // Paid too much
		{
			pi->amount-=delta;

			Sndbounce5(s);
			if (ps->isDragging())
			{
				ps->resetDragging();
				item_bounce5(s,pi);
			}
		}
		else
		{
			if(pi->amount < delta)		// Gave less gold
				delta = pi->amount;		// so adjust skillgain
			pi->Delete();
		}
		pc->baseskill[sk]+=delta;
		Skills::updateSkillLevel(pc, sk);
		updateskill(s,sk);

		pc->trainer=-1;
		pc_t->trainingplayerin=0xFF;
		pc->playSFX( itemsfx(pi->getId()) );
	}
	else // Did not give gold
	{
		pc_t->talk( s, TRANSLATE("I am sorry, but I can only accept gold."),0);
		Sndbounce5(s);
		if (ps->isDragging())
		{
			ps->resetDragging();
			item_bounce5(s,pi);
		}
	}//if items[i]=gold
	return true;
}

static bool ItemDroppedOnSelf(NXWCLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	if (ps == NULL) return false;
	VALIDATEPIR(pi, false);
//	NXWSOCKET  s=ps->toInt();
//	CHARACTER cc=ps->currCharIdx();
	P_CHAR pc = ps->currChar(); // MAKE_CHAR_REF(cc);
	VALIDATEPCR(pc, false);

	Location charpos = pc->getPosition();

	if (pi->getId() >= 0x4000 ) // crashfix , prevents putting multi-objects ni your backback
	{
		ps->sysmsg(TRANSLATE("Hey, putting houses in your pack crashes your back and client !"));
		pi->MoveTo( charpos );
		pi->Refresh();//AntiChrist
		return true;
	}

//	if (pi->glow>0) // glowing items
//	{
//		pc->addHalo(pi);
//		pc->glowHalo(pi);
//	}

	P_ITEM pack = pc->getBackpack();
	if (pack==NULL) // if player has no pack, put it at its feet
	{
		pi->MoveTo( charpos );
		pi->Refresh();
	}
	else
	{
		pack->AddItem(pi); // player has a pack, put it in there

		weights::NewCalc(pc);//AntiChrist bugfixes
		statwindow(pc,pc);
		pc->playSFX( itemsfx(pi->getId()) );
	}
	return true;
}

static bool ItemDroppedOnChar(NXWCLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	if (ps == NULL) return true;
	VALIDATEPIR(pi, false);
	NXWSOCKET  s = ps->toInt();
//	CHARACTER cc=ps->currCharIdx();
	P_CHAR pTC = pointers::findCharBySerial(pp->Tserial);	// the targeted character
	VALIDATEPCR(pTC, false);
	P_CHAR pc_currchar = ps->currChar(); //MAKE_CHAR_REF(cc);
	VALIDATEPCR(pc_currchar, false);
	Location charpos = pc_currchar->getPosition();

	if (!pTC) return true;

	if (pc_currchar->getSerial32() != pTC->getSerial32() /*DEREF_P_CHAR(pTC)!=cc*/)
	{
		if (pTC->npc)
		{
			if(!pTC->HasHumanBody())
			{
				ItemDroppedOnPet( ps, pp, pi);
			}
			else	// Item dropped on a Human character
			{
				// Item dropped on a Guard (possible bounty quest)
				if( ( pTC->npc == 1 ) && ( pTC->npcaitype == NPCAI_TELEPORTGUARD ) )
				{
					ItemDroppedOnGuard( ps, pp, pi);
				}
				if ( pTC->npcaitype == NPCAI_BEGGAR )
				{
					ItemDroppedOnBeggar( ps, pp, pi);
				}

				//This crazy training stuff done by Anthracks (fred1117@tiac.net)
				if(pc_currchar->trainer != pTC->getSerial32())

				{
					pTC->talk(s, TRANSLATE("Thank thee kindly, but I have done nothing to warrant a gift."),0);
					Sndbounce5(s);
					if (ps->isDragging())
					{
						ps->resetDragging();
						item_bounce5(s,pi);
					}
					return true;
				}
				else // The player is training from this NPC
				{
					ItemDroppedOnTrainer( ps, pp, pi);
				}
			}//if human or not
		}
		else // dropped on another player
		{
			// By Polygon: Avoid starting the trade if GM drops item on logged on char (crash fix)
			if ((pc_currchar->IsGM()) && !pTC->IsOnline())
			{
				// Drop the item in the players pack instead
				// Get the pack
				P_ITEM pack = pTC->getBackpack();
				if (pack != NULL)	// Valid pack?
				{
					pack->AddItem(pi);	// Add it
					weights::NewCalc(pTC);
				}
				else	// No pack, give it back to the GM
				{
					pack = pc_currchar->getBackpack();
					if (pack != NULL)	// Valid pack?
					{
						pack->AddItem(pi);	// Add it
						weights::NewCalc(pc_currchar);
					}
					else	// Even GM has no pack?
					{
						// Drop it to it's feet
						pi->MoveTo( charpos );
						pi->Refresh();
					}
				}
			}
			else
			{
                                //<Luxor>: secure trade
                 P_ITEM tradeCont = tradestart(pc_currchar, pTC);
                 if (ISVALIDPI(tradeCont)) {
                    tradeCont->AddItem( pi, 30, 30 );
                 } else {
                    Sndbounce5(s);
                    if (ps->isDragging()) {
                 		ps->resetDragging();
                 		UpdateStatusWindow(s,pi);
                   	}
                 }
                 //</Luxor>
		        }
	        }
	}
	else // dumping stuff to his own backpack !
	{
		ItemDroppedOnSelf( ps, pp, pi);
	}
	return true;
}

void dump_item(NXWCLIENT ps, PKGx08 *pp) // Item is dropped on ground or a character
{
	if (ps == NULL) return;

	tile_st tile;
	NXWSOCKET  s=ps->toInt();

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	P_ITEM pi=pointers::findItemBySerial(pp->Iserial);


	if (!ISVALIDPI(pi))
	{
		LogError("client sent bad itemserial %d",pp->Iserial);
		return;
	}

	if ( isCharSerial(pi->getContSerial()) && pi->getContSerial() != pc->getSerial32() ) {
		P_CHAR pc_i = pointers::findCharBySerial(pi->getContSerial());
		if (ISVALIDPC(pc_i))
			pc_i->sysmsg("Warning, backpack disappearing bug located!");

		if (ps->isDragging()) {
                        ps->resetDragging();
                        UpdateStatusWindow(s,pi);
                }
		pi->setContSerial( pi->getContSerial(true) );
                pi->setPosition( pi->getOldPosition() );
                pi->layer = pi->oldlayer;
                pi->Refresh();
	}

	if (pi->magic == 2) { //Luxor -- not movable objects
		if (ps->isDragging()) {
                        ps->resetDragging();
                        UpdateStatusWindow(s,pi);
                }
		pi->setContSerial( pi->getContSerial(true) );
		pi->MoveTo( pi->getOldPosition() );
		pi->layer = pi->oldlayer;
		pi->Refresh();
		return;
	}




        if(pi!=NULL)
	{
		weights::NewCalc(pc);
		statwindow(pc,pc);
	}


	//Ripper...so order/chaos shields disappear when on ground.
	if( pi->getId()==0x1BC3 || pi->getId()==0x1BC4 )
	{
		pc->playSFX( 0x01FE);
		staticeffect(DEREF_P_CHAR(pc), 0x37, 0x2A, 0x09, 0x06);
		pi->Delete();
		return;
	}


	//test UOP blocking Tauriel 1-12-99
	if (!pi->isInWorld())
	{
		item_bounce6(ps,pi);
		return;
	}




	data::seekTile(pi->getId(), tile);
	if (!pc->IsGM() && ((pi->magic==2 || (tile.weight==255 && pi->magic!=1))&&!pc->canAllMove()) ||
		( (pi->magic==3 || pi->magic==4) && !(pi->getOwnerSerial32()==pc->getSerial32())))
	{
		item_bounce6(ps,pi);
		return;
	}

	if (buffer[s][5]!=(unsigned char)'\xFF')
	{


        if (pi->amxevents[EVENT_IDROPINLAND]!=NULL) {
	       	g_bByPass = false;
        	pi->amxevents[EVENT_IDROPINLAND]->Call( pi->getSerial32(), pc->getSerial32() );
			if (g_bByPass) {
				pi->Refresh();
				return;
			}
		}

		/*
		//<Luxor>
		g_bByPass = false;
		pi->runAmxEvent( EVENT_IDROPINLAND, pi->getSerial32(), pc->getSerial32() );
	        if (g_bByPass) {
			pi->Refresh();
			return;
		}
        //</Luxor>
		*/
		NxwSocketWrapper sw;
		sw.fillOnline( pi );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			SendDeleteObjectPkt( sw.getSocket(), pi->getSerial32() );
		}

		pi->MoveTo(pp->TxLoc,pp->TyLoc,pp->TzLoc);
		pi->setContSerial(-1);

		P_ITEM p_boat = Boats->GetBoat(pi->getPosition());

		if(ISVALIDPI(p_boat))
		{
			pi->SetMultiSerial(p_boat->getSerial32());
		}


		pi->Refresh();
	}
	else
	{
		if ( !ItemDroppedOnChar(ps, pp, pi) ) {
			//<Luxor>: Line of sight check
			//This part avoids the circle of transparency walls bug

			//-----
			if ( !lineOfSight( pc->getPosition(), Loc( pp->TxLoc, pp->TyLoc, pp->TzLoc ) ) ) {
		                ps->sysmsg(TRANSLATE("You cannot place an item there!"));

        	        	Sndbounce5(s);
	                	if (ps->isDragging()) {
	                        	ps->resetDragging();
                        		UpdateStatusWindow(s,pi);
                		}
                		pi->setContSerial( pi->getContSerial(true) );
                		pi->setPosition( pi->getOldPosition() );
                		pi->layer = pi->oldlayer;
                		pi->Refresh();
                		return;
        		}
        		//</Luxor>

	        	//<Luxor> Items count check
	        	if (!pc->IsGM()) {
				NxwItemWrapper si;
				si.fillItemsAtXY( pp->TxLoc, pp->TyLoc );
				if (si.size() >= 2) { //Only 2 items permitted
					ps->sysmsg(TRANSLATE("There is not enough space there!"));
					Sndbounce5(s);
					if (ps->isDragging()) {
						ps->resetDragging();
						UpdateStatusWindow(s,pi);
					}
					if (ISVALIDPI(pc->getBackpack())) {
						pi->setCont(pc->getBackpack());
						pi->SetRandPosInCont(pc->getBackpack());
					} else {
						pi->setContSerial( pi->getContSerial(true) );
						pi->setPosition( pi->getOldPosition() );
					}
					pi->layer = pi->oldlayer;
					pi->Refresh();
					return;
				}
			}
        		//</Luxor>
		}

		weights::NewCalc(pc);  // Ison 2-20-99
		statwindow(pc,pc);
		pc->playSFX( itemsfx(pi->getId()) );

		//Boats !
		if (pc->getMultiSerial32() > 0) //How can they put an item in a multi if they aren't in one themselves Cut lag by not checking everytime something is put down
		{
			P_ITEM multi = pointers::findItemBySerial( pc->getMultiSerial32() );
			if (ISVALIDPI(multi))
			{
				multi=findmulti( pi->getPosition() );
				if (ISVALIDPI(multi))
					//setserial(DEREF_P_ITEM(pi),DEREF_P_ITEM(multi),7);
					pi->SetMultiSerial(multi->getSerial32());
			}
		}
		//End Boats
	}
}

void pack_item(NXWCLIENT ps, PKGx08 *pp) // Item is put into container
{
	if (ps == NULL) return;

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	int serial/*, serhash*/;
	tile_st tile;
//	bool abort=false;
	NXWSOCKET  s=ps->toInt();

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	Location charpos= pc->getPosition();

	P_ITEM pack;

	P_ITEM pCont = pointers::findItemBySerial(pp->Tserial);
	VALIDATEPI(pCont);

	P_ITEM pItem = pointers::findItemBySerial(pp->Iserial);
	VALIDATEPI(pItem);

	if (pItem->getId() >= 0x4000)
	{
//		abort=true; // LB crashfix that prevents moving multi objcts in BP's
		ps->sysmsg(TRANSLATE("Hey, putting houses in your pack crashes your back and client!"));
	}

	//ndEndy recurse only a time
	P_ITEM contOutMost = pCont->getOutMostCont();
	P_CHAR contOwner = ( !contOutMost->isInWorld() )? pointers::findCharBySerial( contOutMost->getContSerial() ) : NULL;

	if( ISVALIDPC(contOwner) ) {
		//if ((contOwner->npcaitype==NPCAI_PLAYERVENDOR) && (contOwner->npc) && (contOwner->getOwnerSerial32()!=pc->getSerial32()) )
		if ( contOwner->getSerial32() != pc->getSerial32() && contOwner->getOwnerSerial32() != pc->getSerial32() && !pc->IsGM() ) { // Luxor
			ps->sysmsg(TRANSLATE("This aint your backpack!"));
			Sndbounce5(s);
			if (ps->isDragging()) {
				ps->resetDragging();
				item_bounce3(pItem);
				if (pCont->getId() >= 0x4000)
					senditem(s, pCont);
			}
			return;
		}
	}

	if (pCont->amxevents[EVENT_IONPUTITEM]!=NULL) {
		g_bByPass = false;
		pCont->amxevents[EVENT_IONPUTITEM]->Call( pCont->getSerial32(), pItem->getSerial32(), pc->getSerial32() );
		if (g_bByPass)
		{
			item_bounce6(ps,pItem);
			return;
		}
	}
	/*
	g_bByPass = false;
	pCont->runAmxEvent( EVENT_IONPUTITEM, pCont->getSerial32(), pItem->getSerial32(), pc->getSerial32() );
	if (g_bByPass)
	{	//AntiChrist to preview item disappearing
		item_bounce6(ps,pItem);
		return;
	}
	*/

	if (pCont->layer==0 && pCont->getId() == 0x1E5E &&
		pCont->getContSerial()==pc->getSerial32())
	{
		// Trade window???
		serial=calcserial(pCont->moreb1, pCont->moreb2, pCont->moreb3, pCont->moreb4);
		if(serial==-1) return;

		P_ITEM pi_z = pointers::findItemBySerial(serial);

		if (ISVALIDPI(pi_z))
			if ((pi_z->morez || pCont->morez))
			{
				pi_z->morez=0;
				pCont->morez=0;
				sendtradestatus( pi_z, pCont );
			}
	}

	if(SrvParms->usespecialbank)//only if special bank is activated
	{
		if(pCont->morey==MOREY_GOLDONLYBANK && pCont->morex==MOREX_BANK && pCont->type==ITYPE_CONTAINER)
		{
			if ( pItem->getId() == ITEMID_GOLD )
			{//if they're gold ok
				pc->playSFX( goldsfx(2) );
			} else
			{//if they're not gold..bounce on ground
				ps->sysmsg(TRANSLATE("You can only put golds in this bank box!"));

				pItem->setContSerial(-1);
				pItem->MoveTo( charpos );
				pItem->Refresh();
				pc->playSFX( itemsfx(pItem->getId()) );
				return;
			}
		}
	}

	// Xanathars's Bank Limit Code
	if (ServerScp::g_nBankLimit != 0) {

		if( ISVALIDPI( contOutMost ) && contOutMost->morex==MOREX_BANK ) {

			int n = contOutMost->CountItems( INVALID, INVALID, false);
			n -= contOutMost->CountItems( ITEMID_GOLD, INVALID, false);
			if( pItem->type == ITYPE_CONTAINER )
				n += pItem->CountItems( INVALID, INVALID, false);
			else
				++n;
			if( n > ServerScp::g_nBankLimit ) {
				ps->sysmsg(TRANSLATE("You exceeded the number of maximimum items in bank of %d"), ServerScp::g_nBankLimit);
				item_bounce6(ps,pItem);
				return;
			}

		}
	}


	//ndEndy this not needed because when is dragging cont serial is INVALID
	//testing UOP Blocking Tauriel 1-12-99
	if (!pItem->isInWorld())
	{
		item_bounce6(ps,pItem);
		return;
	}

	data::seekTile(pItem->getId(), tile);
	if ((((pItem->magic==2)||((tile.weight==255)&&(pItem->magic!=1)))&&!pc->canAllMove) ||
				( (pItem->magic==3|| pItem->magic==4) && !(pItem->getOwnerSerial32()==pc->getSerial32())))
	{
		Sndbounce5(s);
		if (ps->isDragging())
		{
			ps->resetDragging();
			item_bounce3(pItem);
			if (pCont->getId() >= 0x4000)
				senditem(s, pCont);
		}
		return;
	}
	// - Trash container
	if( pCont->type==ITYPE_TRASH)
	{
		pItem->Delete();
		ps->sysmsg(TRANSLATE("As you let go of the item it disappears."));
		return;
	}
	// - Spell Book
	if (pCont->type==ITYPE_SPELLBOOK)
	{
		if (!pItem->IsSpellScroll72())
		{
			ps->sysmsg(TRANSLATE("You can only place spell scrolls in a spellbook!"));
			Sndbounce5(s);
			if (ps->isDragging())
			{
				ps->resetDragging();
				item_bounce3(pItem);
			}
			if (pCont->getId() >= 0x4000)
				senditem(s, pCont);
			return;
		}
		pack= pc->getBackpack();
		if(ISVALIDPI(pack))
		{
			if ((!(pCont->getContSerial()==pc->getSerial32())) &&
				(!(pCont->getContSerial()==pack->getSerial32())) && (!(pc->CanSnoop())))
			{
				ps->sysmsg(TRANSLATE("You cannot place spells in other peoples spellbooks."));
				item_bounce6(ps,pItem);
				return;
			}

			if( strncmp(pItem->getCurrentNameC(), "#", 1) )
				pItem->getName(temp2);
			else
				strcpy(temp2,pItem->getCurrentNameC());

			NxwItemWrapper sii;
			sii.fillItemsInContainer( pCont, false );
			for( sii.rewind(); !sii.isEmpty(); sii++ ) {

				P_ITEM pi_ci=sii.getItem();

					if (ISVALIDPI(pi_ci))
					{
						if( strncmp(pi_ci->getCurrentNameC(), "#", 1) )

							pi_ci->getName(temp);
						else
							strcpy(temp,pi_ci->getCurrentNameC());

						if(!(strcmp(temp,temp2)) || !(strcmp(temp,"All-Spell Scroll")))
						{
							ps->sysmsg(TRANSLATE("You already have that spell!"));
							item_bounce6(ps,pItem);
							return;
						}
					}
				// Juliunus, to prevent ppl from wasting scrolls.
				if (pItem->amount > 1)
				{
					ps->sysmsg(TRANSLATE("You can't put more than one scroll at a time in your book."));
					item_bounce6(ps,pItem);
					return;
				}
			}
		}
		pCont->AddItem( pItem );
		ps->sendSpellBook(pCont);
		return;
	}

	if (pCont->type == ITYPE_CONTAINER) {

		if ( ISVALIDPC(contOwner) )
		{
			if ( (contOwner->npcaitype==NPCAI_PLAYERVENDOR) && (contOwner->npc) && (contOwner->getOwnerSerial32()==pc->getSerial32()) )
			{
				pc->fx1= DEREF_P_ITEM(pItem);
				pc->fx2=17;
				pc->sysmsg(TRANSLATE("Set a price for this item."));
			}
		}

		short xx=pp->TxLoc;
		short yy=pp->TyLoc;

		pCont->AddItem(pItem,xx,yy);

		pc->playSFX( itemsfx(pItem->getId()) );
		statwindow(pc,pc);
	}
	// end of player run vendors

	else
		// - Unlocked item spawner or unlockable item spawner
		if (pCont->type==ITYPE_UNLOCKED_CONTAINER || pCont->type==ITYPE_NODECAY_ITEM_SPAWNER || pCont->type==ITYPE_DECAYING_ITEM_SPAWNER)
		{
			pCont->AddItem(pItem, pp->TxLoc, pp->TyLoc); //Luxor
			pc->playSFX( itemsfx(pItem->getId()) );

		}
		else  // - Pileable
			if (pCont->pileable && pItem->pileable)
			{
				if ( !pCont->PileItem( pItem ) )
				{
					item_bounce6(ps,pItem);
					return;
				}
			}
			else
			{
				if( pItem->getContSerial( true )==INVALID  ) //current cont serial is invalid because is dragging
				{
					NxwSocketWrapper sw;
					sw.fillOnline( pItem->getPosition() );
					for( sw.rewind(); !sw.isEmpty(); sw++ )
						SendDeleteObjectPkt(sw.getSocket(), pItem->getSerial32() );
					mapRegions->remove(pItem);
				}

				pItem->setPosition( pp->TxLoc, pp->TyLoc, pp->TzLoc);
				pItem->setContSerial( pCont->getContSerial() );

				pItem->Refresh();
			}


}

void drop_item(NXWCLIENT ps) // Item is dropped
{

	if (ps == NULL) return;

	NXWSOCKET  s=ps->toInt();
//	CHARACTER cc=ps->currCharIdx();

	PKGx08 pkgbuf, *pp=&pkgbuf;

	pp->Iserial=LongFromCharPtr(buffer[s]+1);
	pp->TxLoc=ShortFromCharPtr(buffer[s]+5);
	pp->TyLoc=ShortFromCharPtr(buffer[s]+7);
	pp->TzLoc=buffer[s][9];
	pp->Tserial=LongFromCharPtr(buffer[s]+10);

    //#define debug_dragg

	if (clientDimension[s]==3)
	{
	  // UO:3D clients send SOMETIMES two dragg packets for a single dragg action.
	  // sometimes we HAVE to swallow it, sometimes it has to be interpreted
	  // if UO:3D specific item loss problems are reported, this is probably the code to blame :)
	  // LB

	  P_ITEM pi = pointers::findItemBySerial(pp->Iserial);

	  #ifdef debug_dragg
	    if (ISVALIDPI(pi)) { sprintf(temp, "%04x %02x %02x %01x %04x i-name: %s EVILDRAG-old: %i\n",pp->Iserial, pp->TxLoc, pp->TyLoc, pp->TzLoc, pp->Tserial, pi->name, clientInfo[s]->evilDrag); ConOut(temp); }
		else { sprintf(temp, "blocked: %04x %02x %02x %01x %04x i-name: invalid item EVILDRAG-old: %i\n",pp->Iserial, pp->TxLoc, pp->TyLoc, pp->TzLoc, pp->Tserial, clientInfo[s]->evilDrag); ConOut(temp); }
	  #endif

	  if  ( (pp->TxLoc==-1) && (pp->TyLoc==-1) && (pp->Tserial==0)  && (clientInfo[s]->evilDrag) )
	  {
		  clientInfo[s]->evilDrag=false;
          #ifdef debug_dragg
		    ConOut("Swallow only\n");
          #endif
		  return;
	  }	 // swallow! note: previous evildrag !

	  else if ( (pp->TxLoc==-1) && (pp->TyLoc==-1) && (pp->Tserial==0)  && (!clientInfo[s]->evilDrag) )
	  {
          #ifdef debug_dragg
		    ConOut("Bounce & Swallow\n");
          #endif

		  item_bounce6(ps, pi);
		  return;
	  }
	  else if ( ( (pp->TxLoc!=-1) && (pp->TyLoc!=-1) && ( pp->Tserial!=-1)) || ( (pp->Iserial>=0x40000000) && (pp->Tserial>=0x40000000) ) )
		  clientInfo[s]->evilDrag=true; // calc new evildrag value
	  else clientInfo[s]->evilDrag=false;
	}

	#ifdef debug_dragg
	  else
	  {
		P_ITEM pi = pointers::findItemBySerial(pp->Iserial);

	     if (ISVALIDPI(pi)) { sprintf(temp, "blocked: %04x %02x %02x %01x %04x i-name: %s EVILDRAG-old: %i\n",pp->Iserial, pp->TxLoc, pp->TyLoc, pp->TzLoc, pp->Tserial, pi->name, clientInfo[s]->evilDrag); ConOut(temp); }
	  }
	#endif


//	if ( (buffer[s][10]>=0x40) && (buffer[s][10]!=0xff) )
	if ( isItemSerial(pp->Tserial) && (pp->Tserial != INVALID)  ) // Invalid target => invalid container => put inWorld !!!
		pack_item(ps,pp);
	else
		dump_item(ps,pp);
}
