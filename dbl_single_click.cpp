  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "nxwcommn.h"
#include "network.h"
#include "itemid.h"
#include "sndpkg.h"
#include "srvparms.h"
#include "debug.h"
#include "amx/amxcback.h"
#include "trigger.h"
#include "magic.h"
#include "house.h"
#include "npcai.h"
#include "layer.h"
#include "mount.h"
#include "nxw_utils.h"
#include "data.h"
#include "boats.h"
#include "books.h"
#include "set.h"
#include "dbl_single_click.h"
#include "titles.h"
#include "rcvpkg.h"
#include "map.h"
#include "items.h"
#include "chars.h"
#include "basics.h"
#include "inlines.h"
#include "skills.h"
#include "classes.h"
#include "range.h"
#include "scp_parser.h"
#include "nox-wizard.h"
#include "utils.h"
#include "fishing.h"

/*!
\brief apply wear out to item, delete if necessary
\author Ripper
\return bool
\param pi pointer to item to wear out
\note only used on Item_ToolWearOut(NXWSOCKET  s, P_ITEM pi)
\todo merge whit Item_ToolWearOut(NXWSOCKET  s, P_ITEM pi)
*/
/*static bool Item_ToolWearOut(P_ITEM pi)
{
	if(chance(5))
		pi->hp--; //Take off a hit point
	if(pi->hp<=0)
	{
		pi->deleteItem();
		return true;
	}
	else
		return false;
}*/

/*!
\brief apply wear out to item, delete if necessary
\author Ripper, rewritten by Luxor
\return bool
\param s socket of player who wear out the item
\param pi pointer to item to wear out
\todo become a method of cItem
*/
static bool Item_ToolWearOut(NXWSOCKET  s, P_ITEM pi)
{
	VALIDATEPIR(pi, false);
	if ( s < 0 || s >= now )
		return false;

	P_CHAR pc = pointers::findCharBySerial(currchar[s]);
	VALIDATEPCR(pc, false);
	if( chance(5) ) { // has item been destroyed ??
		pi->hp--;
		if ( pi->hp <= 0 ) {
			pc->sysmsg("Your %s has been destroyed", pi->getCurrentNameC());
			pi->Delete();
			return true;
		}
	}
	return false;
}


#define CASE(FUNC) else if( ( pi->FUNC() ) )
#define CASEOR(A, B) else if( (pi->A())||(pi->B()) )
static void doubleclick_itemid( NXWSOCKET s, P_CHAR pc, P_ITEM pi, P_ITEM pack );
/*!
\brief Double click
\author Ripper, rewrite by Endymion
\param ps client of player dbclick
\note Completely redone by Morrolan 20-07-99
\warning I use a define CASE for make more readable the code, if you change name of P_ITEM pi chage also the macro
\todo los
*/
void doubleclick(NXWCLIENT ps)
{

	if(ps==NULL) return;
	P_CHAR pc = ps->currChar();
	VALIDATEPC( pc );
	NXWSOCKET s = ps->toInt();

	// the 0x80 bit in the first byte is used later for "keyboard" and should be ignored
	SERIAL serial = LongFromCharPtr(buffer[s] +1) & 0x7FFFFFFF;

	if (isCharSerial(serial))
	{
		P_CHAR pd=pointers::findCharBySerial(serial);
		if(ISVALIDPC(pd))
			dbl_click_character(ps, pd);
		return;
	}

	P_ITEM pi = pointers::findItemBySerial(serial);
	VALIDATEPI(pi);

	if (pi->amxevents[EVENT_IONDBLCLICK]!=NULL) {
		g_bByPass = false;
		pi->amxevents[EVENT_IONDBLCLICK]->Call( pi->getSerial32(), pc->getSerial32() );
		if (g_bByPass==true)
			return;
	}
	/*
	g_bByPass = false;
	pi->runAmxEvent( EVENT_IONDBLCLICK, pi->getSerial32(), s );
	if (g_bByPass==true)
		return;
	*/

	if (!checkItemUsability(pc , pi, ITEM_USE_DBLCLICK))
		return;

	Location charpos= pc->getPosition();

	if (pc->IsHiddenBySpell()) return;	//Luxor: cannot use items if under invisible spell

	if ( !pc->IsGM() && pc->objectdelay >= uiCurrentTime )
	{
		pc->sysmsg(TRANSLATE("You must wait to perform another action."));
		return;
	}
	else
		pc->objectdelay = SrvParms->objectdelay * MY_CLOCKS_PER_SEC + uiCurrentTime;

	///MODIFY, CANT CLICK ITEM AT DISTANCE >2//////////////
	if ( (pc->distFrom(pi)>2) && !pc->IsGM() && !(pc->nxwflags[0] & cChar::flagSpellTelekinesys) ) //Luxor: let's check also for the telekinesys spell
	{
		pc->sysmsg( TRANSLATE("Must be closer to use this!"));
		pc->objectdelay=0;
		return;
	}


//<Anthalir> VARIAIBLI

	tile_st item;

	P_ITEM pack= pc->getBackpack();
	VALIDATEPI( pack );


	data::seekTile( pi->getId(), item );
//////FINEVARIABILI
	if ( ServerScp::g_nEquipOnDclick )
	{
		// equip the item only if it is in the backpack of the player
		if ((pi->getContSerial() == pack->getSerial32()) && (item.quality != 0) && (item.quality != LAYER_BACKPACK) && (item.quality != LAYER_MOUNT))
		{
			int drop[2]= {-1, -1};	// list of items to drop, there no reason for it to be larger
			int curindex= 0;

			NxwItemWrapper wea;
			wea.fillItemWeared( pc, true, true, true );
			for( wea.rewind(); !wea.isEmpty(); wea++ )
			{

				P_ITEM pj=wea.getItem();
				if(!ISVALIDPI(pj))
					continue;
				if ((item.quality == LAYER_1HANDWEAPON) || (item.quality == LAYER_2HANDWEAPON))// weapons
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
					if (pj->layer == item.quality)
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
								pc->UnEquip( p_drop );

						}
					}
				}
				pc->playSFX( itemsfx(pi->getId()) );
				pc->Equip( pi );
			}
			else
			{
				if (drop[0] == -1)
				{
					pc->playSFX( itemsfx(pi->getId()) );
					pc->Equip( pi );
				}
			}
			return;
		}
	} // </Anthalir>


	//<Luxor>: Circle of transparency bug fix
	P_ITEM pCont;
	Location dst;

	pCont = pi->getOutMostCont();

	if(pCont->isInWorld()) {
		dst = pCont->getPosition();
	} else {
		P_CHAR pg_dst = pointers::findCharBySerial( pCont->getContSerial() );
		VALIDATEPC(pg_dst);
		dst = pg_dst->getPosition();
	}

	Location charPos = pc->getPosition();
	charPos.z = dst.z;
	charPos.dispz = dst.dispz;

	if ( !pc->IsGM() && !lineOfSight( charPos, dst ) && !(pc->nxwflags[0] & cChar::flagSpellTelekinesys) ) {
		pc->sysmsg( TRANSLATE( "You cannot reach the item" ) );
		return;
	}
	//</Luxor>

	P_CHAR itmowner = pi->getPackOwner();

	if(!pi->isInWorld()) {
		if (isItemSerial(pi->getContSerial()) && pi->type != ITYPE_CONTAINER)
		{// Cant use stuff that isn't in your pack.

			if ( ISVALIDPC(itmowner) && (itmowner->getSerial32()!=pc->getSerial32()) )
					return;
		}
		else
			if (isCharSerial(pi->getContSerial()) && pi->type!=(UI32)INVALID)
			{// in a character.
				P_CHAR wearedby = pointers::findCharBySerial(pi->getContSerial());
				if (ISVALIDPC(wearedby))
					if (wearedby->getSerial32()!=pc->getSerial32() && pi->layer!=LAYER_UNUSED_BP && pi->type!=ITYPE_CONTAINER)
						return;
			}
	}

	if ((pi->magic==4) && (pi->secureIt==1))
	{
		if (!pc->isOwnerOf(pi) || !pc->IsGMorCounselor())
		{
			pc->sysmsg( TRANSLATE("That is a secured chest!"));
			return;
		}
	}

	if (pi->magic == 4)
	{
		pc->sysmsg( TRANSLATE("That item is locked down."));
		return;
	}

	if (pc->dead && pi->type!=ITYPE_RESURRECT) // if you are dead and it's not an ankh, FORGET IT!
	{
		pc->sysmsg(TRANSLATE("You may not do that as a ghost."));
		return;
	}
	else if (!pc->IsGMorCounselor() && pi->layer!=0 && !pc->IsWearing(pi))
	{// can't use other people's things!
		if (!(pi->layer==LAYER_BACKPACK  && SrvParms->rogue==1)) // bugfix for snooping not working, LB
		{
			pc->sysmsg(TRANSLATE("You cannot use items equipped by other players."));
			return;
		}
	}


	// Begin checking objects that we force an object delay for (std objects)
	// start trigger stuff
	if (pi->trigger > 0)
	{
		if (pi->trigtype == 0)
		{
			if ( TIMEOUT( pi->disabled ) ) // changed by Magius(CHE) §
			{
				triggerItem(s, pi, TRIGTYPE_DBLCLICK); // if players uses trigger
				return;
			}
			else
			{
				if ( pi->disabledmsg!=NULL )
					pc->sysmsg("%s", pi->disabledmsg->c_str());
				else
					pc->sysmsg(TRANSLATE("That doesnt seem to work right now."));
				return;
			}
		}
		else
		{
			pc->sysmsg( TRANSLATE("You are not close enough to use that."));
			return;
		}
	}

	// check this on trigger in the event that the .trigger property is not set on the item
	// trigger code.  Check to see if item is envokable by id
	else if (checkenvoke( pi->getId() ))
	{
		pc->envokeitem = pi->getSerial32();
		pc->envokeid = pi->getId();
		P_TARGET targ = clientInfo[s]->newTarget( new cObjectTarget() );
		targ->code_callback=target_envoke;
		targ->send( ps );
		ps->sysmsg( TRANSLATE("What will you use this on?"));
		return;
	}
	// end trigger stuff
	// BEGIN Check items by type

	int los = 0;


	BYTE map1[20] = "\x90\x40\x01\x02\x03\x13\x9D\x00\x00\x00\x00\x13\xFF\x0F\xFF\x01\x90\x01\x90";
	BYTE map2[12] = "\x56\x40\x01\x02\x03\x05\x00\x00\x00\x00\x00";
	// By Polygon: This one is needed to show the location on treasure maps
	BYTE map3[12] = "\x56\x40\x01\x02\x03\x01\x00\x00\x00\x00\x00";


	P_TARGET targ = NULL;

	switch (pi->type)
	{
	case ITYPE_RESURRECT:
		// Check for 'resurrect item type' this is the ONLY type one can use if dead.
		if (pc->dead)
		{
			pc->resurrect();
			pc->sysmsg(TRANSLATE("You have been resurrected."));
			return;
		}
		else
		{
			pc->sysmsg(TRANSLATE("You are already living!"));
			return;
		}
	case ITYPE_BOATS:// backpacks - snooping a la Zippy - add check for SrvParms->rogue later- Morrolan

		if (pi->type2 == 3)
		{
			switch( pi->getId() & 0xFF ) {
				case 0x84:
				case 0xD5:
				case 0xD4:
				case 0x89:
					Boats->PlankStuff(pc, pi);
					break;
				default:
					pc->sysmsg( TRANSLATE("That is locked."));
					break;
			}
			return;
		}
	case ITYPE_CONTAINER: // bugfix for snooping not working, lb
	case ITYPE_UNLOCKED_CONTAINER:
		// Wintermute: GMs or Counselors should be able to open trapped containers always
		if (pi->moreb1 > 0 && !pc->IsGMorCounselor()) {
			magic::castAreaAttackSpell(pi->getPosition("x"), pi->getPosition("y"), magic::SPELL_EXPLOSION);
			pi->moreb1--;
		}
		//Magic->MagicTrap(currchar[s], pi); // added by AntiChrist
		// only 1 and 63 can be trapped, so pleaz leave it here :) - Anti
	case ITYPE_NODECAY_ITEM_SPAWNER: // nodecay item spawner..Ripper
	case ITYPE_DECAYING_ITEM_SPAWNER: // decaying item spawner..Ripper
		if (pi->isInWorld() || (pc->IsGMorCounselor()) || // Backpack in world - free access to everyone
			( isCharSerial(pi->getContSerial()) && pi->getContSerial()==pc->getSerial32()))	// primary pack
		{
			pc->showContainer(pi);
			pc->objectdelay=0;
			return;
		}
		else if( isItemSerial(pi->getContSerial()) )
		{
			P_ITEM pio = pi->getOutMostCont();
			if (pio->getContSerial()==pc->getSerial32() || pio->isInWorld() )
			{
				pc->showContainer(pi);
				pc->objectdelay=0;
				return;
			}
		}
		if(ISVALIDPC(itmowner))
			snooping(pc, pi );
		return;
	case ITYPE_TELEPORTRUNE:
		targ = clientInfo[s]->newTarget( new cLocationTarget() );
		targ->code_callback = target_tele;
		targ->send( ps );
		ps->sysmsg( TRANSLATE("Select teleport target."));
		return;
	case ITYPE_KEY:
		targ = clientInfo[s]->newTarget( new cItemTarget() );
		targ->code_callback = target_key;
		targ->buffer[0]= pi->more1;
		targ->buffer[1]= pi->more2;
		targ->buffer[2]= pi->more3;
		targ->buffer[3]= pi->more4;
		targ->send( ps );
		ps->sysmsg( TRANSLATE("Select item to use the key on."));
		return;
	case ITYPE_LOCKED_ITEM_SPAWNER:
	case ITYPE_LOCKED_CONTAINER:

		// Added traps effects by AntiChrist
		// Wintermute: GMs or Counselors should be able to open locked containers always
		if ( !pc->IsGMorCounselor() )
		{
			if (pi->moreb1 > 0 ) {
				magic::castAreaAttackSpell(pi->getPosition().x, pi->getPosition().y, magic::SPELL_EXPLOSION);
				pi->moreb1--;
			}

			pc->sysmsg(TRANSLATE("This item is locked."));
			return;
		}
		else
		{
			pc->showContainer(pi);
 			return;
		}
	case ITYPE_SPELLBOOK:
		if (ISVALIDPI(pack)) // morrolan
			if(pi->getContSerial()==pack->getSerial32() || pc->IsWearing(pi))
				ps->sendSpellBook(pi);
			else
				pc->sysmsg(TRANSLATE("If you wish to open a spellbook, it must be equipped or in your main backpack."));
			return;
	case ITYPE_MAP:
		LongToCharPtr(pi->getSerial32(), map1 +1);
		map2[1] = map1[1];
		map2[2] = map1[2];
		map2[3] = map1[3];
		map2[4] = map1[4];
/*
		By Polygon:
		Assign areas and map size before sending
*/
		map1[7] = pi->more1;	// Assign topleft x
		map1[8] = pi->more2;
		map1[9] = pi->more3;	// Assign topleft y
		map1[10] = pi->more4;
		map1[11] = pi->moreb1;	// Assign lowright x
		map1[12] = pi->moreb2;
		map1[13] = pi->moreb3;	// Assign lowright y
		map1[14] = pi->moreb4;
		int width, height;		// Tempoary storage for w and h;
		width = 134 + (134 * pi->morez);	// Calculate new w and h
		height = 134 + (134 * pi->morez);
		ShortToCharPtr(width, map1 +15);
		ShortToCharPtr(height, map1 +17);
//		END OF: By Polygon

		Xsend(s, map1, 19);
//AoS/		Network->FlushBuffer(s);
		Xsend(s, map2, 11);
//AoS/		Network->FlushBuffer(s);
		return;
	case ITYPE_BOOK:
		Books::DoubleClickBook(s, pi);
		return;
	case ITYPE_DOOR:
		dooruse(s, pi);
		return;
	case ITYPE_LOCKED_DOOR:
		// Wintermute: GMs or Counselors should be able to open locked doors always
		if ( pc->IsGMorCounselor())
 		{
 			dooruse(s, pi);
 			return;
 		}

		if (ISVALIDPI(pack))
		{
			NxwItemWrapper si;
			si.fillItemsInContainer( pack );
			for( si.rewind(); !si.isEmpty(); si++ )
			{
				P_ITEM pj = si.getItem();
				if (ISVALIDPI(pj) && pj->type==ITYPE_KEY)
					if (((pj->more1 == pi->more1) && (pj->more2 == pi->more2) &&
						 (pj->more3 == pi->more3) && (pj->more4 == pi->more4)) )
					{
						pc->sysmsg(TRANSLATE("You quickly unlock, use, and then relock the door."));
						dooruse(s, pi);
						return;
					}
			}
		}
		pc->sysmsg(TRANSLATE("This door is locked."));
		return;
	case ITYPE_FOOD:

		if (pc->hunger >= 6)
		{
			pc->sysmsg( TRANSLATE("You are simply too full to eat any more!"));
			return;
		}
		else
		{
			switch (RandomNum(0, 2))
			{
				case 0: pc->playSFX(0x3A); break;
				case 1: pc->playSFX(0x3B); break;
				case 2: pc->playSFX(0x3C); break;
			}

			switch (pc->hunger)
			{
				case 0:  pc->sysmsg( TRANSLATE("You eat the food, but are still extremely hungry.")); break;
				case 1:  pc->sysmsg( TRANSLATE("You eat the food, but are still extremely hungry.")); break;
				case 2:  pc->sysmsg( TRANSLATE("After eating the food, you feel much less hungry.")); break;
				case 3:  pc->sysmsg( TRANSLATE("You eat the food, and begin to feel more satiated.")); break;
				case 4:  pc->sysmsg( TRANSLATE("You feel quite full after consuming the food.")); break;
				case 5:  pc->sysmsg( TRANSLATE("You are nearly stuffed, but manage to eat the food."));	break;
				default: pc->sysmsg( TRANSLATE("You are simply too full to eat any more!")); break;
			}

			if (pi->poisoned)
			{
				pc->sysmsg(TRANSLATE("The food was poisoned!"));
				pc->applyPoison(PoisonType(pi->poisoned));

			}

			pi->ReduceAmount(1);
		    pc->hunger++;
		}
		return;
	case ITYPE_WAND: // -Fraz- Modified and tuned up, Wands must now be equipped or in pack
	case ITYPE_MANAREQ_WAND: // magic items requiring mana (xan)
		if (ISVALIDPI(pack))
		{
			if (pi->getContSerial() == pack->getSerial32() || pc->IsWearing(pi))
			{
				if (pi->morez != 0)
				{
					pi->morez--;
					if (magic::beginCasting(
						static_cast<magic::SpellId>((8*(pi->morex - 1)) + pi->morey - 1),
						ps,
						(pi->type==ITYPE_WAND) ? magic::CASTINGTYPE_ITEM : magic::CASTINGTYPE_NOMANAITEM))
						{
							if (pi->morez == 0)
							{
								pi->type = pi->type2;
								pi->morex = 0;
								pi->morey = 0;
								pi->offspell = 0;
							}
						}
				}
			}
			else
			{
				pc->sysmsg(TRANSLATE("If you wish to use this, it must be equipped or in your backpack."));
			}
		}
		return; // case 15 (magic items)
/*////////////////////REMOVE/////////////////////////////////////
	case 18: // crystal ball?
		switch (RandomNum(0, 9))
		{
		case 0: itemmessage(s, TRANSLATE("Seek out the mystic llama herder."), pi->getSerial32());									break;
		case 1: itemmessage(s, TRANSLATE("Wherever you go, there you are."), pi->getSerial32());									break;
		case 4: itemmessage(s, TRANSLATE("The message appears to be too cloudy to make anything out of it."), pi->getSerial32());	break;
		case 5: itemmessage(s, TRANSLATE("You have just lost five strength.. not!"), pi->getSerial32());							break;
		case 6: itemmessage(s, TRANSLATE("You're really playing a game you know"), pi->getSerial32());								break;
		case 7: itemmessage(s, TRANSLATE("You will be successful in all you do."), pi->getSerial32());								break;
		case 8: itemmessage(s, TRANSLATE("You are a person of culture."), pi->getSerial32());										break;
		default: itemmessage(s, TRANSLATE("Give me a break! How much good fortune do you expect!"), pi->getSerial32());				break;
		}// switch
		soundeffect2(pc_currchar, 0x01EC);
		return;// case 18 (crystal ball?)
*/////////////////////ENDREMOVE/////////////////////////////////////
	case ITYPE_POTION: // potions
			if (pi->morey != 3)
				pc->drink(pi);   //Luxor: delayed potions drinking
			else    //explosion potion
				usepotion(pc, pi);
			return;
	case ITYPE_RUNE:
			if (pi->morex==0 && pi->morey==0 && pi->morez==0)
			{
				pc->sysmsg( TRANSLATE("That rune is not yet marked!"));
			}
			else
			{
				pc->runeserial = pi->getSerial32();
				pc->sysmsg( TRANSLATE("Enter new rune name."));
			}
			return;
	case ITYPE_SMOKE:
			pc->smoketimer = pi->morex*MY_CLOCKS_PER_SEC + getclock();
			pi->ReduceAmount(1);
			return;
	case ITYPE_RENAME_DEED:
			pc->namedeedserial = pi->getSerial32();
			pc->sysmsg( TRANSLATE("Enter your new name."));
			pi->ReduceAmount(1);
			return;
	case ITYPE_POLYMORPH:
			pc->setId( pi->morex );
			pc->teleport();
			pi->type = ITYPE_POLYMORPH_BACK;
			return;
	case ITYPE_POLYMORPH_BACK:
			pc->setId( pc->getOldId() );
			pc->teleport();
			pi->type = ITYPE_POLYMORPH;
			return;
	case ITYPE_ARMY_ENLIST:
			enlist(s, pi->morex);
			pi->Delete();
			return;
	case ITYPE_TELEPORT:
			pc->MoveTo( pi->morex,pi->morey,pi->morez );
			pc->teleport();
			return;
	case ITYPE_DRINK:
			switch (rand()%2)
			{
				case 0: pc->playSFX(0x0031); break;
				case 1: pc->playSFX(0x0030); break;
			}
			pi->ReduceAmount(1);
			pc->sysmsg( TRANSLATE("Gulp !"));
			return;
	case ITYPE_GUILDSTONE:
			if ( pi->getId() == 0x14F0  ||  pi->getId() == 0x1869 )	// Check for Deed/Teleporter + Guild Type
			{
				pc->fx1 = DEREF_P_ITEM(pi);
				Guilds->StonePlacement(s);
				return;
			}
			else if (pi->getId() == 0x0ED5)	// Check for Guildstone + Guild Type
			{
				pc->fx1 = DEREF_P_ITEM(pi);
				Guilds->Menu(s, 1);
				return;
			}
			else
				WarnOut("Unhandled guild item type named: %s with ID of: %X\n", pi->getCurrentNameC(), pi->getId());
			return;
	case ITYPE_PLAYER_VENDOR_DEED:			// PlayerVendors deed
			{
			P_CHAR vendor = npcs::AddNPCxyz(-1, 2117, charpos.x, charpos.y, charpos.z);
			if ( !ISVALIDPC(vendor) )
			{
				WarnOut("npc-script couldnt find vendor !\n");
				return;
			}

			los = 0;
			vendor->npcaitype = NPCAI_PLAYERVENDOR;
			vendor->MakeInvulnerable();
			vendor->unHide();
			vendor->stealth=INVALID;
			vendor->dir = pc->dir;
			vendor->npcWander = WANDER_NOMOVE;
			vendor->SetInnocent();
			vendor->setOwnerSerial32( pc->getSerial32() );
			vendor->tamed = false;
			pi->Delete();
			vendor->teleport();
			char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
			sprintf( temp, TRANSLATE("Hello sir! My name is %s and i will be working for you."), vendor->getCurrentNameC());
			vendor->talk(s, temp, 0);

			return;
			}
	case ITYPE_TREASURE_MAP:
			Skills::Decipher(pi, s);
			return;

	case ITYPE_DECIPHERED_MAP:
			map1[ 1] = map2[1] = map3[1] = pi->getSerial().ser1;
			map1[ 2] = map2[2] = map3[2] = pi->getSerial().ser2;
			map1[ 3] = map2[3] = map3[3] = pi->getSerial().ser3;
			map1[ 4] = map2[4] = map3[4] = pi->getSerial().ser4;
			map1[ 7] = pi->more1;	// Assign topleft x
			map1[ 8] = pi->more2;
			map1[ 9] = pi->more3;	// Assign topleft y
			map1[10] = pi->more4;
			map1[11] = pi->moreb1;	// Assign lowright x
			map1[12] = pi->moreb2;
			map1[13] = pi->moreb3;	// Assign lowright y
			map1[14] = pi->moreb4;
			ShortToCharPtr(0x0100, map1 +15);			// Let width and height be 256
			ShortToCharPtr(0x0100, map1 +17);
			Xsend(s, map1, 19);
//AoS/			Network->FlushBuffer(s);

			Xsend(s, map2, 11);
//AoS/			Network->FlushBuffer(s);

			// Generate message to add a map point
			SI16 posx, posy;					// tempoary storage for map point
			SI16 tlx, tly, lrx, lry;				// tempoary storage for map extends
			tlx = (pi->more1 << 8) | pi->more2;
			tly = (pi->more3 << 8) | pi->more4;
			lrx = (pi->moreb1 << 8) | pi->moreb2;
			lry = (pi->moreb3 << 8) | pi->moreb4;
			posx = (256 * (pi->morex - tlx)) / (lrx - tlx);		// Generate location for point
			posy = (256 * (pi->morey - tly)) / (lry - tly);
			ShortToCharPtr(posx, map3 +7);				// Store the point position
			ShortToCharPtr(posy, map3 +9);
			Xsend(s, map3, 11);					// Fire data to client :D
//AoS/			Network->FlushBuffer(s);
			return;
		default:
			break;
	}
	///END IDENTIFICATION BY TYPE

/////////////////READ UP :D////////////////////////////////

	///BEGIN IDENTIFICATION BY ID
	if (pi->IsSpellScroll())
	{
		if (ISVALIDPI(pack))
			if( pi->getContSerial()==pack->getSerial32()) {
				magic::SpellId spn = magic::spellNumberFromScrollId(pi->getId());	// avoid reactive armor glitch
				if ((spn>=0)&&(magic::beginCasting(spn, ps, magic::CASTINGTYPE_SCROLL)))
					pi->ReduceAmount(1);							// remove scroll if successful
			}
			else pc->sysmsg(TRANSLATE("The scroll must be in your backpack to envoke its magic."));
	}
	CASE(IsAnvil) {
		targ = clientInfo[s]->newTarget( new cItemTarget() );
		targ->code_callback=Skills::target_repair;
		targ->send( ps );
		ps->sysmsg( TRANSLATE("Select item to be repaired."));
	}
	CASE(IsAxe) {
		targ = clientInfo[s]->newTarget( new cTarget() );
		targ->code_callback=target_axe;
		targ->buffer[0]=pi->getSerial32();
		targ->send( ps );
		ps->sysmsg( TRANSLATE("What would you like to use that on ?"));
	}
	CASEOR(IsFeather, IsShaft) {
		targ = clientInfo[s]->newTarget( new cItemTarget() );
		targ->buffer[0]= pi->getSerial32();
		targ->code_callback=Skills::target_fletching;
		targ->send( ps );
		ps->sysmsg( TRANSLATE("What would you like to use this with?"));
	}
	CASEOR( IsFencing1H, IsSword ) {
		targ = clientInfo[s]->newTarget( new cTarget() );
		targ->code_callback=target_sword;
		targ->send( ps );
		ps->sysmsg( TRANSLATE("What would you like to use that on ?"));
	}
	else ///BEGIN IDENTIFICATION BY ID ( RAW MODE, DEPRECATED )
		doubleclick_itemid( s, pc, pi, pack );

}

void target_selectdyevat( NXWCLIENT ps, P_TARGET t )
{
    P_ITEM pi=pointers::findItemBySerial(t->getClicked());
    VALIDATEPI(pi);

    if( pi->getId()==0x0FAB ||                     //dye vat
        pi->getId()==0x0EFF || pi->getId()==0x0E27 )  //hair dye
            SndDyevat( ps->toInt(), pi->getSerial32(), pi->getId() );
        else
            ps->sysmsg( TRANSLATE("You can only use this item on a dye vat."));
}

void target_dyevat( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR curr = ps->currChar();
	VALIDATEPC(curr);

	P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	if( pi->dye )//if dyeable
	{

		P_CHAR pc = pi->getPackOwner();

		if( !ISVALIDPC(pc) || ( pc->getSerial32()==curr->getSerial32() ) ) //in world or owned
		{
			pi->setColor( t->buffer[0] );
			pi->Refresh();
			curr->playSFX(0x023E); // plays the dye sound, LB
		}
		else
			curr->sysmsg(TRANSLATE("That is not yours!!"));
	}
	else
		curr->sysmsg( TRANSLATE("You can only dye clothes with this.") );
}


static void doubleclick_itemid( NXWSOCKET s, P_CHAR pc, P_ITEM pi, P_ITEM pack )
{
	P_TARGET targ = NULL;
	NXWCLIENT ps = getClientFromSocket( s );

	switch (pi->getId())
	{
		case 0x0FA9: // dye
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=target_selectdyevat;
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Which dye vat will you use this on?") );
			return;// dye
		case 0x0FAB:// dye vat
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=target_dyevat;
			targ->buffer[0]=pi->getColor();
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select the clothing to use this on.") );
			return;// dye vat
		case 0x100A:
		case 0x100B:// archery butte
			Skills::AButte(s, pi);
			return;// archery butte
		case 0x0E9C:
			if (pc->checkSkill( MUSICIANSHIP, 0, 1000))
				pc->playSFX( 0x38);
			else
				pc->playSFX( 0x39);
			return;
		case 0x0E9D:
		case 0x0E9E:
			if (pc->checkSkill(  MUSICIANSHIP, 0, 1000))
				pc->playSFX( 0x52 );
			else
				pc->playSFX( 0x53 );
			return;
		case 0x0EB1:
		case 0x0EB2:
			if (pc->checkSkill( MUSICIANSHIP, 0, 1000))
				pc->playSFX(0x45);
			else
				pc->playSFX( 0x46);
			return;
		case 0x0EB3:
		case 0x0EB4:
			if (pc->checkSkill( MUSICIANSHIP, 0, 1000))
				pc->playSFX( 0x4C);
			else
				pc->playSFX( 0x4D);
			return;
		case 0x102A:// Hammer
		case 0x102B:
		case 0x0FBB:// tongs
		case 0x0FBC:
		case 0x13E3:// smith's hammers
		case 0x13E4:
		case 0x0FB4:// sledge hammers
		case 0x0FB5:
			if (!Item_ToolWearOut(s, pi)) {
				targ = clientInfo[s]->newTarget( new cItemTarget() );
				targ->code_callback=Skills::target_smith;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("Select material to use.") );
			}
			return; // Smithy
		case 0x1026:// Chisels
		case 0x1027:
		case 0x1028:// Dove Tail Saw
		case 0x1029:
		case 0x102C:// Moulding Planes
		case 0x102D:
		case 0x102E:// Nails
		case 0x102F:
		case 0x1030:// Jointing plane
		case 0x1031:
		case 0x1032:// Smoothing plane
		case 0x1033:
		case 0x1034:// Saw
		case 0x1035:
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_carpentry;
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select material to use.") );
			return; // carpentry
		case 0x0E85:// pickaxes
		case 0x0E86:
		case 0x0F39:// shovels
		case 0x0F3A:
			if (!Item_ToolWearOut(s, pi))
			{
				targ = clientInfo[s]->newTarget( new cLocationTarget() );
				targ->code_callback=Skills::target_mine;
				targ->buffer[0]=pi->getSerial32();
				targ->send( ps );
				ps->sysmsg( TRANSLATE("Where do you want to dig?"));
			}
			return; // mining
		case 0x0DF9:
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_wheel;
			targ->buffer[0]=THREAD;
			targ->buffer[1]=pi->getSerial32();
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select spinning wheel to spin cotton.") );
			return;
		case 0x0DF8: // wool to yarn
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_wheel;
			targ->buffer[0]=YARN;
			targ->buffer[1]=pi->getSerial32();
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select your spin wheel to spin wool."));
			return;
		case 0x0FA0:
		case 0x0FA1: // thread to Bolt
		case 0x0E1D:
		case 0x0E1F:
		case 0x0E1E:  // yarn to cloth
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_loom;
			targ->buffer[0]=pi->getSerial32();
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select loom to make your cloth"));
			return;
		case 0x0F9D: // sewing kit for tailoring
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_tailoring;
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select material to use."));
			return;
		case 0x19B7:
		case 0x19B9:
		case 0x19BA:
		case 0x19B8: // smelt ore
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_smeltOre;
			targ->buffer[0]=pi->getSerial32();
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select forge to smelt ore on."));// smelting  for all ore changed by Myth 11/12/98
			return;
		case 0x1E5E:
		case 0x1E5F: // Message board opening
			MsgBoards::MsgBoardEvent(s);
			return;
		case 0x0DE1:
		case 0x0DE2: // camping
  			if ( !item_inRange( pc, pi, 3 ) )
  			{
				pc->sysmsg(TRANSLATE("You are to far away to reach that"));
				return;
  			}
  					//</Luxor>
			if (pc->checkSkill(  CAMPING, 0, 500)) // Morrolan TODO: insert logout code for campfires here
			{
				P_ITEM pFire = item::CreateFromScript( "$item_a_campfire" );
				if(ISVALIDPI(pFire))
				{
					pFire->type = 45;
					pFire->dir = 2;
					pFire->setDecay();
					if (pi->isInWorld())
						pFire->MoveTo( pi->getPosition() );
					else
						pFire->MoveTo( pc->getPosition() );
					pFire->setDecayTime();
					pFire->Refresh();// AntiChrist
					pi->ReduceAmount(1);
				}
			}
			else
			{
				pc->sysmsg(TRANSLATE("You fail to light a fire."));
			}
			return; // camping
		case 0x1508: // magic statue?
			if (pc->checkSkill( ITEMID, 0, 10))
			{
				pi->setId( 0x1509 );
				pi->type = 45;
				pi->Refresh();// AntiChrist
			}
			else
			{
				pc->sysmsg(TRANSLATE("You failed to use this statue."));
			}
			return;
		case 0x1509:
			if (pc->checkSkill(  ITEMID, 0, 10))
			{
				pi->setId( 0x1508 );
				pi->type = 45;
				pi->Refresh();// AntiChrist
			}
			else
			{
				pc->sysmsg(TRANSLATE("You failed to use this statue."));
			}
			return;
		case 0x1230:
		case 0x1246: // guillotines?
			if (pc->checkSkill(  ITEMID, 0, 10))
			{
				pi->setId( 0x1245 );
				pi->type = 45;
				pi->Refresh();// AntiChrist
			}
			else
			{
				pc->sysmsg(TRANSLATE("You failed to use this."));
			}
			return;
		case 0x1245: // Guillotine stop animation
			if (pc->checkSkill(  ITEMID, 0, 10))
			{
				pi->setId( 0x1230 );
				pi->type = 45;
				pi->Refresh();// AntiChrist
			}
			else
			{
				pc->sysmsg(TRANSLATE("You failed to use this."));
			}
			return;
		case 0x0DBF:
		case 0x0DC0:// fishing
			if( pi->getContSerial()==pc->getSerial32() || pi->getContSerial()==pack->getSerial32() ) {
				targ = clientInfo[s]->newTarget( new cLocationTarget() );
				targ->code_callback = Fishing::target_fish;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("Fish where?"));
			}
			else
				pc->sysmsg( TRANSLATE("If you wish to use this, it must be equipped or in your backpack.") );
			return;
		case 0x104B:
		case 0x104C:
		case 0x1086: // Clock and bracelet
			telltime(ps);
			return;
		case 0x0E9B: // Mortar for Alchemy
			pc->objectdelay = ((SrvParms->objectdelay * MY_CLOCKS_PER_SEC)*3) + uiCurrentTime;
			if (pi->type == ITYPE_MANAREQ_WAND)
			{
				targ = clientInfo[s]->newTarget( new cItemTarget() );
				targ->code_callback=Skills::target_bottle;
				targ->buffer[0]=pi->getSerial32();
				targ->send( ps );
				ps->sysmsg( TRANSLATE("Where is an empty bottle for your potion?") );
			}
			else
			{
				targ = clientInfo[s]->newTarget( new cItemTarget() );
				targ->code_callback=Skills::target_alchemy;
				targ->buffer[0]=pi->getSerial32();
				targ->send( ps );
				ps->sysmsg( TRANSLATE("What do you wish to grind with your mortar and pestle?"));
			}
			return; // alchemy
		case 0x0E21: // healing
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_healingSkill;
			targ->buffer[0]=pi->getSerial32();
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Who will you use the bandages on?"));
			return;
		case 0x1057:
		case 0x1058: // sextants
			{
				char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp2 var
				getSextantCoords( pc->getPosition().x, pc->getPosition().y, (pc->getPosition().x >= 5121), temp);
				pc->sysmsg(TRANSLATE("You are at: %s"), temp);
			}
			return;
		case 0x0E27:
		case 0x0EFF:   // Hair Dye
			usehairdye(s,pi);
			return;
		case 0x14FB:
		case 0x14FC:
		case 0x14FD:
		case 0x14FE: // lockpicks
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_lockpick;
			targ->buffer[0]=pi->getSerial32();
			targ->send( ps );
			ps->sysmsg( TRANSLATE("What lock would you like to pick?"));
			return;
		case 0x097A: // Raw Fish steaks
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_cookOnFire;
			targ->buffer[0]=0x097B;
			targ->buffer[1]=pi->getSerial32();
			targ->buffer_str[0] = "fish steaks";
			targ->send( ps );
			ps->sysmsg( TRANSLATE("What would you like to cook this on?"));
			return;
		case 0x09b9: // Raw Bird
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_cookOnFire;
			targ->buffer[0]=0x09B7;
			targ->buffer[1]=pi->getSerial32();
			targ->buffer_str[0] = "bird";
			targ->send( ps );
			ps->sysmsg( TRANSLATE("What would you like to cook this on?"));
			return;
		case 0x1609: // Raw Lamb
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_cookOnFire;
			targ->buffer[0]=0x160A;
			targ->buffer[1]=pi->getSerial32();
			targ->buffer_str[0] = "lamb";
			targ->send( ps );
			ps->sysmsg( TRANSLATE("What would you like to cook this on?"));
			return;
		case 0x09F1: // Raw Ribs
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_cookOnFire;
			targ->buffer[0]=0x09F2;
			targ->buffer[1]=pi->getSerial32();
			targ->buffer_str[0] = "ribs";
			targ->send( ps );
			ps->sysmsg( TRANSLATE("What would you like to cook this on?"));
			return;
		case 0x1607: // Raw Chicken Legs
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_cookOnFire;
			targ->buffer[0]=0x1608;
			targ->buffer[1]=pi->getSerial32();
			targ->buffer_str[0] = "chicken legs";
			targ->send( ps );
			ps->sysmsg( TRANSLATE("What would you like to cook this on?"));
			return;
		case 0x0C4F:
		case 0x0C50:
		case 0x0C51:
		case 0x0C52:
		case 0x0C53:
		case 0x0C54: // cotton plants
			{
				if (!pc->isMounting())
					pc->playAction(0x0D);
				else
					pc->playAction(0x1D);
				pc->playSFX(0x013E);
				P_ITEM itm = item::CreateFromScript( "$item_bales_of_cotton", pc->getBackpack() );
				if (ISVALIDPI(itm)) {
					pc->sysmsg(TRANSLATE("You reach down and pick some cotton."));
				}
			}
			return; // cotton
		case 0x105B:
		case 0x105C:
		case 0x1053:
		case 0x1054: // tinker axle
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_tinkerAxel;
			targ->buffer[0]=pi->getSerial32();
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select part to combine that with."));
			return;
		case 0x1051:
		case 0x1052:
		case 0x1055:
		case 0x1056:
		case 0x105D:
		case 0x105E:
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_tinkerAwg;
			targ->buffer[0]=pi->getSerial32();
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select part to combine it with."));
			return;
		case 0x104F:
		case 0x1050:
		case 0x104D:
		case 0x104E:// tinker clock
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback=Skills::target_tinkerClock;
			targ->buffer[0]=pi->getSerial32();
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select part to combine with"));
			return;
		case 0x1059:
		case 0x105A:// tinker sextant
			if (pc->checkSkill(  TINKERING, 500, 1000))
			{
				pc->sysmsg(TRANSLATE("You create the sextant."));
				P_ITEM pi_c = item::CreateFromScript( "$item_sextant", pc->getBackpack() );
				if (ISVALIDPI(pi_c))
					pi_c->setDecay();
				pi->ReduceAmount(1);
			}
			else
				pc->sysmsg(TRANSLATE("you fail to create the sextant."));
			return;
		case 0x1070:
		case 0x1074: // training dummies
			if (item_inRange(pc, pi, 1))
			{
				if (pc->hidden==HIDDEN_BYSKILL)
					pc->unHide();
				Skills::TDummy(s);
			}
			else
				pc->sysmsg(TRANSLATE("You need to be closer to use that."));
				return;
		case 0x1071:
		case 0x1073:
		case 0x1075:
		case 0x1077:// swinging training dummy
			pc->sysmsg(TRANSLATE("You must wait for it to stop swinging !"));
			return;
		//case 0x1EA8:
		//	slotmachine(s, DEREF_P_ITEM(pi));
		//	return; // Ripper
		case 0x1EBC: // tinker's tools
			targ = clientInfo[s]->newTarget( new cItemTarget() );
			targ->code_callback = Skills::target_tinkering;
			targ->send( ps );
			ps->sysmsg( TRANSLATE("Select material to use."));
			return;
		default:
			pc->sysmsg( TRANSLATE("You can't think of a way to use that item."));
			break;
	}


}

/*!
\brief Single click
\author Ripper
\param s socket of player who click
*/
void singleclick(NXWSOCKET  s)
{
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC( pc_currchar );
	pc_currchar->doSingleClick( LongFromCharPtr(buffer[s] +1));
}


/*!
\brief Double clicks over PC/NPCs
\param ps clientof player who click
\param target pointer to the pc/npc clicked
*/
void dbl_click_character(NXWCLIENT ps, P_CHAR target)
{
	if(ps==NULL) return;
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);
	VALIDATEPC(target);
	NXWSOCKET s = ps->toInt();

	if( pc->war && (pc->getSerial32()!=target->getSerial32()) )
		AttackStuff( s, target );

	int keyboard = buffer[s][1]&0x80;

	P_ITEM pack	= target->getBackpack();


	switch( target->getId() )
	{
		// Handle pack animals
		case	0x0123	:
		case	0x0124	:
			if ( target->npc )
			{

				if ( target->getOwnerSerial32() == pc->getSerial32() || pc->IsGMorCounselor() )
				{
					if (ISVALIDPI(pack)) {
						pc->showContainer(pack);
						SetTimerSec( &(pc->objectdelay), SrvParms->objectdelay );
					}
					else
						WarnOut("Pack animal %i has no backpack!\n",target->getSerial32());
				}
				else
				{
					if ( pc->checkSkill( SNOOPING, 0, 1000 ) )
					{
						if (ISVALIDPI(pack) )
						{
							pc->showContainer(pack);
							pc->sysmsg(TRANSLATE("You successfully snoop the pack animal.") );
							SetTimerSec( &(pc->objectdelay), SrvParms->objectdelay+SrvParms->snoopdelay );
						}
						else
							WarnOut("Pack animal %i has no backpack!\n",target->getSerial32());
					}
					else
					{
						pc->sysmsg( TRANSLATE("You failed to snoop the pack animal.") );
						pc->IncreaseKarma( ServerScp::g_nSnoopKarmaLoss  );
						pc->modifyFame( ServerScp::g_nSnoopFameLoss );
						setCrimGrey(pc, ServerScp::g_nSnoopWillCriminal );
						SetTimerSec( &(pc->objectdelay), SrvParms->objectdelay+SrvParms->snoopdelay );
					}
				}
			}
			return;
		//
		// Handle humanoids
		//
		case BODY_MALE		:
		case BODY_FEMALE	:
		case BODY_DEADMALE	:
		case BODY_DEADFEMALE	:
		case BODY_GMSTAFF	:
			if (target->npc && target->npcaitype==NPCAI_PLAYERVENDOR)//PlayerVendors
			{
				target->talk(s,TRANSLATE("Take a look at my goods."),0);
				if ( ISVALIDPI(pack))
					pc->showContainer(pack);
			}
			else if ( pc->getSerial32() == target->getSerial32() )
			{//dbl-click self
				if ( (!keyboard) && ( pc->unmountHorse() == 0 ) ) return; //on horse
				//if not on horse, treat ourselves as any other char
			}//self

			UI08 pdoll[66] = { 0x88, 0x00, 0x05, 0xA8, 0x90, 0x00, };

			LongToCharPtr(target->getSerial32(), pdoll +1);

			completetitle = complete_title(target);
			if ( strlen(completetitle) >= 60 )
				completetitle[60]=0;
			strcpy((char*)&pdoll[5], completetitle);
			Xsend(s, pdoll, 66);
//AoS/			Network->FlushBuffer(s);
			return;
		//
		// Handle others
		//
	}
	/*	// Handle mountable npcs
		case 0x00c8	:
		case 0x00e2	:
		case 0x00e4	:
		case 0x00cc	:
		case 0x00dc	:
		case 0x00d2	:
		case 0x00da	:
		case 0x00db	:
		case 0x007a	:	// LBR	unicorn
		case 0x00bb	:	// 	ridgeback
		case 0x0317	:	//	giant beetle
		case 0x0319	:	//	skeletal mount
		case 0x031a	:	//	swamp dragon
		case 0x031f	:	//	armor dragon*/
	std::map<SI32,SI32>::iterator iter = mountinfo.find(target->getId());
	if( iter!=mountinfo.end() ) {
		if ( target->npc )	// riding a morphed player char is not allowed
			{
				if (pc->distFrom(target)<2 || pc->IsGMorCounselor())
				{
					//cannot ride animals under polymorph effect
					if ( pc->polymorph) {
						pc->sysmsg( TRANSLATE("You cannot ride anything under polymorph effect."));
					}
					else
						if ( pc->dead) {
							pc->sysmsg(TRANSLATE("You are dead and cannot do that."));
						}
						else
							if (target->war) {
								pc->sysmsg(TRANSLATE("Your pet is in battle right now!"));
							}
							else

								pc->mounthorse( target);


				}
				else  {
					pc->sysmsg( TRANSLATE("You need to get closer."));
				}
			}
		return;
	}

	pc->sysmsg(TRANSLATE("You cannot open monsters paperdolls."));
}
