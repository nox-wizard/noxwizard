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
#include "sndpkg.h"
#include "debug.h"
#include "race.h"
#include "house.h"
#include "commands.h"
#include "addmenu.h"

#define AMXGUMPFUNC "__nxwGumps"

extern void tweakmenu(NXWSOCKET  s, SERIAL serial);


void gumps::Button(int s, UI32 button, char tser1, char tser2, char tser3, char tser4, UI32 type, char radio)
{
	if (s < 0) return; //Luxor
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC( pc );
	

/*ndEndy REMOVED LIST

  RACE
  CUSTMENU
  HOUSE
  COMMANDs
  COMMANDs UNUSED i think

*/

  
	
	
	/* ndEndy RACE - REMOVED NOW
	// Sparhawk: 	race gumps, all race gumps are currently hardcoded
	//
	if ( type >= 100 && type <= 129 )
	{
		Race::handleButton( s, type, button );
		return;
	}*/

	/*ndEndy REMOVED NOW
	//<XAN> : CustmMenus
	if (type==130) {
		int seed = tser1+(tser2<<8)+(tser3<<16)+(tser4<<24);
		if (pc->customMenu != NULL)
			pc->customMenu->buttonSelected(s, button, seed  );
		return;
	}*/

	/*ndEndy Spar gumps tweak, cstat and user
	if(button>10000) {
		i=button-10000;
		gumps::Menu(s, i,NULL);
		return;
	}
	else
	{
		serial=calcserial(tser1,tser2,tser3,tser4);
		//
		// Sparhawk: new gump API, currently only activated for cstats, istats and tweak char & item
		//
		/*switch( type )
		{
			case   1	:	// character properties
			case   2	:	// item properties
			case  20	:	// region choice list
			case  21	:	// region properties
			case  50	:	// guild stone menu
			case  51	:	// guild members
			case  52	:	// guild recruits
			case  53	:	// guild properties
			case 999	:
				nxwGump::handleGump( pc, buffer[s] );
				return;
			default		:
				break;
		}*//*
		//
		// Luxor: new gump API activated also for users
		//
		if ( nxwGump::handleGump( pc, buffer[s] ) )
			return;
	}
*/
	/* ndEndy COMMANDs - HOUSE
	switch (type)
	{
	case 1: //Tweaking an Item
//		j=calcItemFromSer( serial );
		break;
	case 2: //Tweaking a Character
//		j=calcCharFromSer( serial );
		break;
	case 4:
//		j = 1;
		break;
	case 5: // House maintenance (Sparhawk)
		housesign_serial = calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
		pHouseSign = pointers::findItemBySerial(housesign_serial);

		if( ISVALIDPI(pHouseSign) )
		{
			house_serial = calcserial(pHouseSign->more1,pHouseSign->more2,pHouseSign->more3,pHouseSign->more4);
			pHouse = pointers::findItemBySerial(house_serial);

			if ( !ISVALIDPI(pHouse) )
			{
				ErrOut("housegump cannot determine index for house with serial %d\n", house_serial);
				pc->sysmsg(TRANSLATE("HouseGump failed - house not found\n"));
				return;
			}
		}
		else
		{
			ErrOut("housegump cannot determine index for house sign with serial %d\n", housesign_serial);
			pc->sysmsg(TRANSLATE("HouseGump failed - house sign not found\n"));
			return;
		}
		break;
	default:
		if (type < 200)
			return;
	}

	/*if (type==3) //Townstones
		j = calcItemFromSer( serial );*/ // townstones taken out by LB

		
/*ndEndy COMMAND UNUSED
	switch (type)
	{
	case 1: // Item
		switch( button )
		{
		case 2:		entrygump( s, tser1, tser2, tser3, tser4, type, button, 50, "Enter a new name for the item. (# = default name)" );	break;
		case 3:		entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter the new ID number for the item in hex." );			break;
		case 4:		entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter the new hue for the item in hex." );				break;
		case 5:		entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter the new X coordinate for the item in decimal." );	break;
		case 6:		entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter the new Y coordinate for the item in decimal." );	break;
		case 7:		entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter the new Z coordinate for the item in decimal." );	break;
		case 8:		entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter the new type for the item in decimal." );			break;
		case 9:		entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter the new itemhand for the item in decimal." );		break;//Xuri
		case 10:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter the new layer for the item in decimal." );			break;
		case 11:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter the new amount for the item in decimal." );			break;
		case 12:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 8, "Enter the new More for the item in hex." );				break;
		case 13:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 8, "Enter the new MoreB for the item in hex." );				break;
		case 14:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 1, "Enter the new stackable toggle for the item. (0/1)" );	break;
		case 15:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 1, "Enter the new dyeable toggle for the item. (0/1)" );		break;
		case 16:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 1, "Enter the new corpse toggle for the item. (0/1)" );		break;
		case 17:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter the new LODAMAGE value for the item in decimal." );	break;//|
		case 18:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter the new HIDAMAGE value for the item in decimal." );	break;//| both of these replace the old "attack value" (Xuri)
		case 19:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter the new defence value for the item in decimal." );	break;
		case 20:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 1, "Enter the new magic value for the item in decimal." );	break;
		case 21:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 1, "Enter the new visible value for the item in decimal." );	break;
		//start addons by Xuri
		case 22:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter the new HP value for the item in decimal." );		break;
		case 23:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter the new Max HP value for the item in decimal." );	break;
		case 24:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter the new Speed value for the item in decimal." );	break;
		case 25:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter the new Rank value for the item in decimal." );		break;
		case 26:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter the new Value for the item in decimal." );			break;
		case 27:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter the new Good value for the item in decimal." );		break;
		case 28:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter the new Made Skill value for the item in decimal." );	break;
		case 29:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 50, "Enter the new Creator name for the item." );				break;
		//end addons by Xuri
		default:	WarnOut( "Unknown button pressed %i", button );																		break;
		}
		break;
	case 2:
		switch( button )
		{
		case 2:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 50, "Enter a new Name for the character." );							break;
		case 3:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 50, "Enter a new Title for the character." );							break;
		case 4:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter a new X coordinate for the character in decimal." );			break;
		case 5:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter a new Y coordinate for the character in decimal." );			break;
		case 6:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter a new Z coordinate for the character in decimal." );			break;
		case 7:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 3, "Enter a new Direction for the character in decimal." );				break;
		case 8:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter a new Body Type for the character in hex." );					break;
		case 9:	entrygump( s, tser1, tser2, tser3, tser4, type, button, 4, "Enter a new Skin Hue for the character in hex." );					break;
		case 10: entrygump( s, tser1, tser2, tser3, tser4, type, button, 3, "Enter a new Defence value for the character in decimal." );		break;
		case 11: entrygump( s, tser1, tser2, tser3, tser4, type, button, 1, "Enter a new Hunger value for the character in decimal(0-6).");		break;
		case 12: entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter a new Strength value for the character in decimal.");		break;
		case 13: entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter a new Dexterity value for the character in decimal.");		break;
		case 14: entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter a new Intelligence value for the character in decimal.");	break;
		case 15: entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter a new Karma value for the character in decimal.");	break;
		case 16: entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter a new Fame value for the character in decimal.");	break;
		case 17: entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter a new Kills value for the character in decimal.");	break;
		case 18: entrygump( s, tser1, tser2, tser3, tser4, type, button, 5, "Enter a new Jail time for the character in seconds");	break;

		}
		break;
	case 4:			// Wholist
		if(button<200)
		{
			button-=7;
			pc->making=button;
			Commands::WhoCommand(s,type,button);
		}
		else
		{
			i=pc->making;
			if (i<0)
			{
			  pc->sysmsg("selected character not found");
			  return;
			}

			serial=whomenudata[i];
			serhash=serial%HASHMAX;
			pc_c=pointers::findCharBySerial( serial );

			if (!ISVALIDPC(pc_c))
			{
			  pc->sysmsg("selected character not found");
			  return;
			}

			switch(button)
			{
			case 200://gochar
				pc->doGmEffect(); 	// have a flamestrike at origin and at player destination point 	//Aldur
				pc->MoveTo( pc_c->getPosition() );
				pc->teleport();
				pc->doGmEffect();
				break;

			case 201://xtele
				pc_c->MoveTo( pc->getPosition() );
				pc_c->teleport();
				break;

			case 202://jail char
				if(pc_c->getSerial32()==pc->getSerial32())
				{
					pc->sysmsg("You cannot jail yourself!");
					break;
				}
				else
				{
					Targ->JailTarget (s,pc->getSerial32());
					break;
				}
			case 203://release
				Targ->ReleaseTarget(s,pc_c->getSerial32());
				break;
			case 204:
				if(pc_c->getSerial32()==pc->getSerial32())
				{
					pc->sysmsg("You cannot kick yourself");
					break; // lb
				}
				else
				{
					if( pc_c->IsOnline() )
					{
						pc->sysmsg("you cant kick an offline player");
						break;
					}
					pc->sysmsg("Kicking player");
					pc_c->kick();
					break;
				}
			default:
				ErrOut("Switch fallout. gumps.cpp, gumpbutton()\n"); //Morrolan
			}
			return;
		}
		break;
	case 5: // House maintenance (Sparhawk)
		i = ShortFromCharPtr(buffer[s] +21);
		if (button != 20 && button != 2)
		{
			addid1[s] = pHouse->getSerial().ser1;
			addid2[s] = pHouse->getSerial().ser2;
			addid3[s] = pHouse->getSerial().ser3;
			addid4[s] = pHouse->getSerial().ser4;
		}
		switch (button)
		{
			case 20: // Change house sign's appearance
				if (i>0)
				{
					buffer[s][7] = addid1[s];
					buffer[s][8] = addid2[s];
					buffer[s][9] = addid3[s];
					buffer[s][10] = addid4[s];
					addx[s] = buffer[s][21];
					addy[s] = buffer[s][22];
					Targ->IDtarget(s);
					sysmessage(s, TRANSLATE("House sign changed."));
				}
				return;
			case 0:
				return;
			case 2:  // Bestow ownership upon someone else
				target(s, 0, 1, 0, 227, TRANSLATE("Select person to transfer ownership to."));
				return;
			case 3:  // Turn house into a deed
				deedhouse(s, pHouse);
				return;
			case 4:  // Kick someone out of house
				target(s, 0, 1, 0, 228, TRANSLATE("Select person to eject from house."));
				return;
			case 5:  // Ban somebody
				target(s, 0, 1, 0, 229, TRANSLATE("Select person to ban from house."));
				return;
			case 6:
			case 8: // Remove someone from house list
				target(s, 0, 1, 0, 231, TRANSLATE("Select person to remove from house registry."));
				return;
			case 7: // Make someone a friend
				target(s, 0, 1, 0, 230, TRANSLATE("Select person to make a friend of the house."));
				return;
			case 13: // Rename house, housesign and house keys
				// 1) transform name in UO client wide char format to char format
				int idx;
				for (idx = 1; idx <= ShortFromCharPtr(buffer[s] +25); idx++)
					temp[idx-1] = buffer[s][26+idx*2];
				temp[idx-1] = 0;

				// 2) rename items
				pHouse->setCurrentName( temp );			// rename house
				pHouseSign->setCurrentName( temp );		// rename house sign

				pHouseKey = pointers::findItemBySerial(pHouse->st);
				if ( ISVALIDPI(pHouseKey) )
					pHouseKey->setCurrentName( temp );	// rename house key (originally put in backpack

				pHouseKey = pointers::findItemBySerial(pHouse->st2);
				if ( ISVALIDPI(pHouseKey) )
					pHouseKey->setCurrentName( temp );	// rename house key (originally put in bankbox)

				// 3) display success
				pc->sysmsg(TRANSLATE("House renamed to %s"), pHouse->getCurrentNameC() );
				return;
			default:
				pc->sysmsg(TRANSLATE("HouseGump Called - Button=%i"), button);
				return;
			}
	default :
		if (type > 200) // scripted gumps
			AmxFunction::g_prgOverride->CallFn( AmxFunction::g_prgOverride->getFnOrdinal(AMXGUMPFUNC),s, type-200, button, radio);
		break;
	}

  */
}

void gumps::Input(int s)
{
	
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	
	char type, index ;
//	unsigned char tser1, tser2, tser3, tser4;
	char *text;
	int c1,body,b,k;
	SERIAL serial;

/*	tser1=buffer[s][3];
	tser2=buffer[s][4];
	tser3=buffer[s][5];
	tser4=buffer[s][6];
	serial=calcserial(tser1,tser2,tser3,tser4); */
	serial = LongFromCharPtr(buffer[s] +3);
	type=buffer[s][7];
	index=buffer[s][8];
	text=(char*)&buffer[s][12];

	Guilds->GumpInput(s,type,index,text);

	if (type==1 && (pc->IsGM()))
	{
		P_ITEM pj= pointers::findItemBySerial(serial);
		VALIDATEPI(pj);

		if (buffer[s][9]==0)
		{
			tweakmenu(s, pj->getSerial32());
			return;
		}
		switch( index )
		{
		case 2:		pj->setCurrentName(text);	break;
		case 3:		pj->setId( hex2num(text) );	break;	 // ID
		case 4:		pj->setColor( hex2num(text) );	break;	// Hue
		case 5:		pj->setPosition("x", str2num(text) );	break;	// X
		case 6:		pj->setPosition("y", str2num(text) );	break;	// Y
		case 7:		pj->setPosition("z", str2num(text) );	break;	// Z
		case 8:		pj->type = str2num(text);	break;	 // Type
		case 9:		pj->itmhand = str2num(text);	break;	// Itemhand - added by Xuri
		case 10:	pj->layer = str2num(text);	break;	// Layer
		case 11:	pj->amount = str2num(text);	break;	// Amount
		case 12:	k = hex2num( text );	// More
					pj->more1 = (unsigned char)(k>>24);
					pj->more2 = (unsigned char)(k>>16);
					pj->more3 = (unsigned char)(k>>8);
					pj->more4 = (unsigned char)(k%256);
					break;
		case 13: 	k = hex2num( text );	// MoreB
					pj->moreb1 = (unsigned char)(k>>24);
					pj->moreb2 = (unsigned char)(k>>16);
					pj->moreb3 = (unsigned char)(k>>8);
					pj->moreb4 = (unsigned char)(k%256);
					break;
		case 14: 	pj->pileable = str2num(text);	break;	// Pileable
		case 15:	pj->dye = str2num(text);	break;	// Dye
		case 16:	pj->corpse = str2num(text);	break;	// Corpse
		case 17:	pj->lodamage = str2num(text);	break;	// LoDamage
		case 18:	pj->hidamage = str2num(text);	break;	// HiDamage
		case 19:	pj->def = str2num(text);	break;	// Def
		case 20:	pj->magic = str2num(text);	break;	// Magic
		case 21:	pj->visible = str2num(text);	break;	// Visible
		case 22:	pj->hp = str2num(text);		break;	// Current Hitpoints
		case 23:	pj->maxhp = str2num(text);	break;	// MAX Hitpoints
		case 24:	pj->spd = str2num(text);	break;	// Speed (for Combat)
		case 25:	pj->rank = str2num(text);	break;	// Rank
		case 26:	pj->value = str2num(text);	break;	// Value
		case 27:	pj->good = str2num(text);	break;	// Good(for Adv.Trade system)
		case 28:	pj->madewith = str2num(text);	break;	// Made Skill
		case 29:	pj->creator = text;		break;	// Creator
		}

		pj->Refresh();
		tweakmenu(s, pj->getSerial32());
	}

	if (type==2 && (pc->IsGM()))
	{
		P_CHAR pc_j= pointers::findCharBySerial(serial);
		VALIDATEPC(pc_j);

		if (buffer[s][9]==0)
		{
	 		tweakmenu(s, pc_j->getSerial32());
			return;
		}

		switch( index )
		{
		case 2:		pc_j->setCurrentName( text );			break;  // Name
		case 3:		pc_j->title = text;				break;	// Title
		case 4:		pc_j->setPosition("x", str2num(text) );		break;	// X
		case 5:		pc_j->setPosition("y", str2num(text) );		break;	// Y
		case 6:	 	pc_j->setPosition("z", str2num(text) );
				pc_j->setPosition("dz", str2num(text) );	break;	// Z
		case 7:		pc_j->dir = str2num(text) & 0x0F;		break;  // make sure the high-bits are clear // Dir
		case 8: // Body
			k = hex2num( text );
			if (k>=0x000 && k<=0x3e1) // lord binary, body-values >0x3e crash the client
			{
			pc_j->SetBodyType(k);
			pc_j->SetOldBodyType(k);
		       c1 = pc_j->getSkinColor(); // transparency for mosnters allowed, not for palyers,
		                                              // if polymorphing from monster to player we have to switch from transparent to semi-transparent
		                                              // or we have that sit-down-client crash
               b=c1&0x4000;
			   if (b==16384 && (k >=0x0190 && k<=0x03e1))
			   {
				if (c1!=0x8000)
				{
					pc_j->setSkinColor(0xF000);
					pc_j->setOldSkinColor(0xF000);	
				}
			   }
			}
			break;
		case 9:		k = hex2num( text );		// Skin
					body = pc_j->GetBodyType();

                    b=k&0x4000; // Lord binary --> that touchy transparency bit !
		                        // fixes a client crash with transparently dyed skin.
		                        // ( only with body values 190-03e1 --> no monster )
		                        // if such a char wants to sit down the client crashes.
		                        // this is a CLIENT(!) BUG I cant do anythhing

		           if (b==16384 && (body >=0x0190 && body<=0x03e1)) k=0xf000; // but assigning the only "transparent" value that works, namly semi-trasnparency.

                   if (k!=0x8000) // 0x8000 also crashes client ...
				   {
					pc_j->setSkinColor(k);
					pc_j->setOldSkinColor(k);
				   }
					break;

		case 10:	pc_j->def = str2num(text);		break;	// Defence
		case 11:	pc_j->hunger = str2num(text);		break;	// Hunger
		case 12:	pc_j->setStrength( str2num(text) );	break;	// Strength
		case 13:	pc_j->dx = str2num(text);		break;	// Dexterity
		case 14:	pc_j->in = str2num(text);		break;	// Intelligence
		case 15:	pc_j->SetKarma( str2num(text) );	break;	// Karma
		case 16:	pc_j->SetFame( str2num(text) );		break;	// Fame
		case 17:	pc_j->kills = str2num(text);		break;	// Kills

		}

		pc_j->teleport();
		tweakmenu(s, pc_j->getSerial32());
	}
}


void gumps::Menu(NXWSOCKET  s, int m,P_ITEM pi_it)
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	P_ITEM pi_j;


	char sect[512];
	int loopexit=0;
	short int length, length2, textlines=0;
	UI32 i;
	bool house_gump=false, new_decay=false;
	bool new_houseName = false; // House maintenance (Sparhawk)
	int is,j=-1,ds;
	char tt[255];
	char tt2[255];

	cScpIterator* iter = NULL;
	char script1[1024];

	sprintf(sect, "SECTION GUMPMENU %i", m);
	iter = Scripts::Gumps->getNewIterator(sect);

	if (iter==NULL) return;

	length=21;
	length2=1;

	loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if (((script1[0]!='}')&&(script1[0]!='{'))&&(strncmp(script1, "type ", 5)!=0))
		{

			/*sprintf(tt,"pos1: %s\n",script1);
			LogMessage(tt);*/

			length+=strlen(script1)+4;
			length2+=strlen(script1)+4;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	length+=3;

	sprintf(sect, "SECTION GUMPTEXT %i", m);
	safedelete(iter);
	iter = Scripts::Gumps->getNewIterator(sect);
	if (iter==NULL) return;

	loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			/*sprintf(tt,"pos2: %s\n",script1);
			LogMessage(tt);*/

			if (!strcmp(script1,"Decay Status :")) { new_decay=true; }
			if (!strcmp(  script1, "House Name") )
				new_houseName = true;
			length+=(strlen(script1)*2)+2;
			textlines++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	UI08 gump1[22]={ 0xB0, 0x00, };

	ShortToCharPtr(length, gump1 +1); 		// Total Length
	LongToCharPtr(pc->getSerial32(), gump1 +3);	// id
	LongToCharPtr((UI32)m+200, gump1 +7); 		// Gump Number 0x12 ,  200 + gump_id: for scripted gumps
	LongToCharPtr(0x0000006E, gump1 +11); 		// X
	LongToCharPtr(0x00000046, gump1 +15); 		// Y
	ShortToCharPtr(length2, gump1 +19); 		// command section length

	sprintf(sect, "SECTION GUMPMENU %i", m);
	safedelete(iter);
	iter = Scripts::Gumps->getNewIterator(sect);
	if (iter==NULL) return;
	// typecode setting - Crackerjack 8/8/99
	strcpy(script1, iter->getEntry()->getFullLine().c_str()); //discard the shitty {
	strcpy(script1, iter->getEntry()->getFullLine().c_str());
	if(!strncmp(script1, "type ", 5)) {
		gump1[10]=atoi(&script1[5]);
		script1[0]='}';
		house_gump=true;
	}

	//House maintenace (Sparhawk)
    	if (house_gump && (new_decay || new_houseName) )
	{
		if (ISVALIDPI(pi_it)) // should be !=-1 for house's gumps (only!), but to be on safe side, lets check it.
		{
			// 1) get house item#

			is=pi_it->getSerial32();
			pi_j = pointers::findItemBySerial(is); // pi_j =?= pi_it

			if(ISVALIDPI(pi_j))
			{
				is=calcserial(pi_j->more1,pi_j->more2,pi_j->more3,pi_j->more4);
				pi_j = pointers::findItemBySerial(is);
			}

			// 2) calc decay % number

			if (new_decay)
			{
				//ConOut("adjusting size for decay number length\n");
				if (ISVALIDPI(pi_j))
				{
					if (SrvParms->housedecay_secs!=0)
						ds=((pi_j->time_unused)*100)/(SrvParms->housedecay_secs);
			    		else
						ds=-1;
					numtostr(ds,  tt);
				}
				else
					ds=-1;

				//if (j>-1) ConOut("decay: %i proz house_age: %i max_age: %i \n",ds,items[j].time_unused,server_data.housedecay_secs);

            		// 3) adjust sizes because "decay status :" string is scripted, but the % number added dynamically

				if (ds>=0 && ds<10)        length+=8;    // space digit space % =4*2
				else if (ds>=10 && ds<100) length+=10;   // space digit digit space % = 5*2
				else if (ds>=100)          length+=12;   // space digit digit digit space % = 6*2

				if (ds==-1) length+=10;
			}

			// 3) calc housename length
			if (new_houseName)
			{
				//InfoOut("adjusting size for real house name length\n");
				length -= strlen("House Name") * 2;
				length += strlen(pi_j->getCurrentNameC()) * 2;
			}

	        	ShortToCharPtr(length, gump1 +1);
		}
	}
	Xsend(s, gump1, 21);

	loopexit=0;
	do
	{
		if(script1[0]!='}') {
			sprintf(sect, "{ %s }", script1);

			Xsend(s, sect, strlen(sect));
		}
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	// remark: gump2[0]==0, = termination of the sequence above
	UI08 gump2[3]={ 0x00, }; // ^-- term. + len => 3 BYTE
	ShortToCharPtr(textlines, gump2 +1);
	Xsend(s, gump2, 3);

	sprintf(sect, "SECTION GUMPTEXT %i", m);
	safedelete(iter);
	iter = Scripts::Gumps->getNewIterator(sect);
	if (iter==NULL) return;

	loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{

			/*sprintf(tt,"pos3: %s\n",script1);
			LogMessage(tt);*/

			if (house_gump && ISVALIDPI(pi_j))
			{
				if (!strcmp(script1,"Decay Status :"))
				{
					strcpy(tt2,script1);
					strcat(tt," %");
					strcat(script1, " ");
					strcat(script1,tt);
					//ConOut("final string: %s\n",script1);
					//ConOut("orig_len : %i new_len: %i\n",strlen(tt2),strlen(script1));
				}
				else	// SPARHAWK 2002-01-28 house gump now shows house name
					if (!strcmp(script1,"House Name"))
						strcpy( script1, pi_j->getCurrentNameC());
			}

			UI08 gump3[2]= {0x00, 0x00};
			ShortToCharPtr(strlen(script1), gump3);
			Xsend(s, gump3, 2);
			gump3[0]=0;	// script_text in unicode !? [Kheru]
			for (i=0;i<strlen(script1);i++)
			{
				gump3[1]=script1[i];
				Xsend(s, gump3, 2);
			}
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	safedelete(iter);
//AoS/	Network->FlushBuffer(s);
}



void whomenu(int s, int type) //WhoList--By Homey-- Thx Zip and Taur helping me on this
// added also logged out chars+bugfixing , LB
{
	char sect[512];
	short int length, length2, textlines;
	int x;
	UI32 line, i, k;
	static char menuarray1[10*(MAXCLIENT)+50][50]; // there shoundbe be more than 5*MAXLIENT palyers, 7 to be on save side
	static char menuarray[10*(MAXCLIENT)+50][50];
	unsigned int linecount=0;
	unsigned int linecount1=0,pagenum=1/*,position=40,linenum=1,buttonnum=7*/;

	NxwSocketWrapper sw;
	sw.fillOnline();

	int j=sw.size();

	//--static pages
	strcpy(menuarray[linecount++], "nomove");
	strcpy(menuarray[linecount++], "noclose");
	strcpy(menuarray[linecount++], "page 0");
	strcpy(menuarray[linecount++], "resizepic 0 0 5120 320 340");    //The background
	strcpy(menuarray[linecount++], "button 20 300 2130 2129 1 0 1"); //OKAY
	strcpy(menuarray[linecount++], "text 20 10 300 0");           //text <Spaces from Left> <Space from top> <Length, Color?> <# in order>
	sprintf(menuarray[linecount++], "page %i",pagenum);
	//--Start User List

	k=0;
	/*for(i=0;i<charcount;i++)
	{
		P_CHAR pc_i=MAKE_CHAR_REF(i);
		if(!ISVALIDPC(pc_i))
			continue;
			
		if (!pc_i->npc && !pc_i->free)
		{
		  if(k>0 && (!(k%10)))
		  {
			position=40;
			pagenum++;
			sprintf(menuarray[linecount++], "page %i",pagenum);
		  }

		  k++;

		  sprintf(menuarray[linecount++], "text 40 %i 300 %i",position,linenum); //usernames
		  sprintf(menuarray[linecount++], "button 20 %i 1209 1210 1 0 %i",position,buttonnum);
		  position+=20;
		  linenum++;
		  buttonnum++;
		}
	}*/


	//ConOut("k:%i pages: %i\n",k,pagenum);

	pagenum=1; //lets make some damn buttons
	for (i=0;i<k;i+=10)
	{
	   sprintf(menuarray[linecount++], "page %i", pagenum);
	   if (i>=10)
		sprintf(menuarray[linecount++], "button 150 300 2223 2223  0 %i",pagenum-1); //back button
	   if ((k>10) && ((i+10)<k))
	   sprintf(menuarray[linecount++], "button %i 300 2224 2224  0 %i", 150+(20*(pagenum>1)),pagenum+1); //forward button
	   pagenum++;
	}

	length=21;
	length2=1;

	for(line=0;line<linecount;line++)
	{

		if (strlen(menuarray[line])==0)
			break;
		{
			length+=strlen(menuarray[line])+4;
			length2+=strlen(menuarray[line])+4;
		}
	}

	length+=3;
	textlines=0;
	line=0;

	sprintf(menuarray1[linecount1++], "Users currently online: %i",j);

	//Start user list

	x=0;

	/*for(i=0;i<charcount;i++)
	{
		P_CHAR pc= MAKE_CHAR_REF(i);
		if(!ISVALIDPC(pc))
			continue;

		if (!pc->npc && !pc->free )
			if( !pc->IsOnline()) 
			{
				sprintf(menuarray1[linecount1++], "Player: %s [offline]",pc->getCurrentNameC());
				whomenudata[x++]=pc->getSerial32();
				//ConOut("name: %s\n",chars[i].name);
			}
			else {
				sprintf(menuarray1[linecount1++], "Player %s [online]",pc->getCurrentNameC());
				whomenudata[x++]=pc->getSerial32();
			}
	}*/

	for(line=0;line<linecount1;line++)
	{

		if (strlen(menuarray1[line])==0)
			break;
		{
			length+=strlen(menuarray1[line])*2 +2;
			textlines++;
		}
	}

	UI08 gump1[22]={ 0xB0, 0x00, };

	ShortToCharPtr(length, gump1 +1); 		// Total Length
	LongToCharPtr(0x409151E7, gump1 +3);		// id
	LongToCharPtr((UI32)type, gump1 +7); 		// Gump Number
	LongToCharPtr(0x0000006E, gump1 +11); 		// X
	LongToCharPtr(0x00000046, gump1 +15); 		// Y
	ShortToCharPtr(length2, gump1 +19); 		// command section length
	Xsend(s, gump1, 21);

	for(line=0;line<linecount;line++)
	{
		sprintf(sect, "{ %s }", menuarray[line]);
		Xsend(s, sect, strlen(sect));
	}

	UI08 gump2[3]={ 0x00, };
	ShortToCharPtr(textlines, gump2 +1);
	Xsend(s, gump2, 3);

	UI08 gump3[2]={0x00, };
	for(line=0;line<linecount1;line++)
	{
		if (strlen(menuarray1[line])==0)
			break;
		{
			ShortToCharPtr(strlen(menuarray1[line]), gump3);
			Xsend(s, gump3, 2);
			gump3[0]=0;
			for (i=0;i<strlen(menuarray1[line]);i++)
			{
				gump3[1]=menuarray1[line][i];
				Xsend(s, gump3, 2);
			}
		}
	}
//AoS/	Network->FlushBuffer(s);
}

void playermenu(int s, int type) //WhoList2 with offline players--By Ripper
// added also logged out chars+bugfixing , LB
{
	char sect[512];
	short int length, length2, textlines;
	int j,x;
	UI32 line, i, k;
	static char menuarray1[10*(MAXCLIENT)+50][50]; // there shoundbe be more than 5*MAXLIENT palyers, 7 to be on save side
	static char menuarray[10*(MAXCLIENT)+50][50];
	unsigned int linecount=0;
	unsigned int linecount1=0,pagenum=1,position=40,linenum=1,buttonnum=7;

	//--static pages
	strcpy(menuarray[linecount++], "nomove");
	strcpy(menuarray[linecount++], "noclose");
	strcpy(menuarray[linecount++], "page 0");
	strcpy(menuarray[linecount++], "resizepic 0 0 5120 320 340");    //The background
	strcpy(menuarray[linecount++], "button 20 300 2130 2129 1 0 1"); //OKAY
	strcpy(menuarray[linecount++], "text 20 10 300 0");           //text <Spaces from Left> <Space from top> <Length, Color?> <# in order>
	sprintf(menuarray[linecount++], "page %i",pagenum);
	//--Start User List
	j = 0;
	k = 0;

	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWCLIENT ps_i=sw.getClient();
		if( ps_i==NULL)
			continue;
		
//		P_CHAR pc= ps_i->currChar(); // unused variable

		++j;
		if(k>0 && (!(k%10)))
		{
			position=40;
			pagenum++;
			sprintf(menuarray[linecount++], "page %i",pagenum);
		}
		++k;
		sprintf(menuarray[linecount++], "text 40 %i 300 %i",position,linenum); //usernames
		sprintf(menuarray[linecount++], "button 20 %i 1209 1210 1 0 %i",position,buttonnum);
		position+=20;
		++linenum;
		++buttonnum;
	}


	//ConOut("k:%i pages: %i\n",k,pagenum);

	pagenum=1; //lets make some damn buttons
	for (i=0;i<k;i+=10)
	{
		sprintf(menuarray[linecount++], "page %i", pagenum);
		if (i>=10)
			sprintf(menuarray[linecount++], "button 150 300 2223 2223  0 %i",pagenum-1); //back button
		if ((k>10) && ((i+10)<k))
		sprintf(menuarray[linecount++], "button %i 300 2224 2224  0 %i", 150+(20*(pagenum>1)),pagenum+1); //forward button
		pagenum++;
	}

	length=21;
	length2=1;

	for(line=0;line<linecount;line++)
	{

		if (strlen(menuarray[line])==0)
			break;
		{
			length+=strlen(menuarray[line])+4;
			length2+=strlen(menuarray[line])+4;
		}
	}

	length+=3;
	textlines=0;
	line=0;

	sprintf(menuarray1[linecount1++], "Users currently online: %i",j);

	//Start user list

	x=0;
	sw.clear();
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWCLIENT ps_i=sw.getClient();
		if(ps_i==NULL)
			continue;
		
		P_CHAR pc= ps_i->currChar();
		if(ISVALIDPC(pc)) {
			sprintf(menuarray1[linecount1++], "Player %s [online]",pc->getCurrentNameC());
			whomenudata[x++]=pc->getSerial32();
		}
	}


	for(line=0;line<linecount1;line++)
	{

		if (strlen(menuarray1[line])==0)
			break;
		{
			length+=strlen(menuarray1[line])*2 +2;
			textlines++;
		}
	}

	UI08 gump1[22]={ 0xB0, 0x00, };

	ShortToCharPtr(length, gump1 +1); 		// Total Length
	LongToCharPtr(0x409151E7, gump1 +3);		// id
	LongToCharPtr((UI32)type, gump1 +7); 		// Gump Number
	LongToCharPtr(0x0000006E, gump1 +11); 		// X
	LongToCharPtr(0x00000046, gump1 +15); 		// Y
	ShortToCharPtr(length2, gump1 +19); 		// command section length
	Xsend(s, gump1, 21);

	for(line=0;line<linecount;line++)
	{
		sprintf(sect, "{ %s }", menuarray[line]);
		Xsend(s, sect, strlen(sect));
	}

	UI08 gump2[3]={ 0x00, };
	ShortToCharPtr(textlines, gump2 +1);
	Xsend(s, gump2, 3);

	UI08 gump3[2]={0x00, };
	for(line=0;line<linecount1;line++)
	{
		if (strlen(menuarray1[line])==0)
			break;
		{
			ShortToCharPtr(strlen(menuarray1[line]), gump3);
			Xsend(s, gump3, 2);
			gump3[0]=0;
			for (i=0;i<strlen(menuarray1[line]);i++)
			{
				gump3[1]=menuarray1[line][i];
				Xsend(s, gump3, 2);
			}
		}
	}
//AoS/	Network->FlushBuffer(s);
}


void tline( int line, SERIAL serial )
{
	char type;
	if (isCharSerial(serial))
		type = 2;
	else
		type = 1;
	if( --line == 0 ) strcpy( script1, "page 0" );
	if( --line == 0 ) strcpy( script1, "resizepic 0 0 2520 400 350" );
	if( --line == 0 ) strcpy( script1, "text 40 10 32 0" );
	if( --line == 0 ) strcpy( script1, "button 35 280 2130 2129 1 0 1" );
	if( type == 1 )
	{
		P_ITEM pi = pointers::findItemBySerial(serial);
		if( --line == 0 )
			sprintf( script1, "tilepic 300 180 %i", pi->id());
	}
	if( --line == 0 ) strcpy( script1, "page 1" );
	if( --line == 0 ) strcpy( script1, "button 366 320 2224 2224 0 2");
	if( --line == 0 ) strcpy( script1, "text 288 317 32 1");
	if( --line == 0 ) strcpy( script1, "text 90 70 0 3");
	if( --line == 0 ) strcpy( script1, "text 200 70 16 4");
	if( --line == 0 ) strcpy( script1, "button 30 70 2116 2115 1 0 2");
	if( --line == 0 ) strcpy( script1, "text 90 95 0 5");
	if( --line == 0 ) strcpy( script1, "text 200 95 16 6");
	if( --line == 0 ) strcpy( script1, "button 30 95 2116 2115 1 0 3");
	if( --line == 0 ) strcpy( script1, "text 90 120 0 7");
	if( --line == 0 ) strcpy( script1, "text 200 120 16 8");
	if( --line == 0 ) strcpy( script1, "button 30 120 2116 2115 1 0 4");
	if( --line == 0 ) strcpy( script1, "text 90 145 0 9");
	if( --line == 0 ) strcpy( script1, "text 200 145 16 10");
	if( --line == 0 ) strcpy( script1, "button 30 145 2116 2115 1 0 5");
	if( --line == 0 ) strcpy( script1, "text 90 170 0 11");
	if( --line == 0 ) strcpy( script1, "text 200 170 16 12");
	if( --line == 0 ) strcpy( script1, "button 30 170 2116 2115 1 0 6");
	if( --line == 0 ) strcpy( script1, "text 90 195 0 13");
	if( --line == 0 ) strcpy( script1, "text 200 195 16 14");
	if( --line == 0 ) strcpy( script1, "button 30 195 2116 2115 1 0 7");
	if( --line == 0 ) strcpy( script1, "text 90 220 0 15");
	if( --line == 0 ) strcpy( script1, "text 200 220 16 16");
	if( --line == 0 ) strcpy( script1, "button 30 220 2116 2115 1 0 8");
	if( type == 2 )
	{
		if( --line == 0 ) strcpy( script1, "text 90 245 0 17");
		if( --line == 0 ) strcpy( script1, "text 200 245 16 18");
		if( --line == 0 ) strcpy( script1, "button 30 245 2116 2115 1 0 9");
	}
	if( type == 1 )
	{
		if( --line == 0 ) strcpy( script1, "text 90 245 0 19");
		if( --line == 0 ) strcpy( script1, "text 200 245 16 20");
		if( --line == 0 ) strcpy( script1, "button 30 245 2116 2115 1 0 10");
	}
	if( --line == 0 ) strcpy( script1, "page 2");
	if( --line == 0 ) strcpy( script1, "button 40 320 2223 2223 0 1");
	if( --line == 0 ) strcpy( script1, "text 65 317 32 2");
	if( --line == 0 ) strcpy( script1, "button 366 320 2224 2224 0 3");
	if( --line == 0 ) strcpy( script1, "text 288 317 32 1");
	if( type == 1 )
	{
		if( --line == 0 ) strcpy( script1, "text 90 70 0 17");
		if( --line == 0 ) strcpy( script1, "text 200 70 16 18");
		if( --line == 0 ) strcpy( script1, "button 30 70 2116 2115 1 0 9");
		if( --line == 0 ) strcpy( script1, "text 90 95 0 21");
		if( --line == 0 ) strcpy( script1, "text 200 95 16 22");
		if( --line == 0 ) strcpy( script1, "button 30 95 2116 2115 1 0 11");
		if( --line == 0 ) strcpy( script1, "text 90 120 0 23");
		if( --line == 0 ) strcpy( script1, "text 200 120 16 24");
		if( --line == 0 ) strcpy( script1, "button 30 120 2116 2115 1 0 12");
		if( --line == 0 ) strcpy( script1, "text 90 145 0 25");
		if( --line == 0 ) strcpy( script1, "text 200 145 16 26");
		if( --line == 0 ) strcpy( script1, "button 30 145 2116 2115 1 0 13");
		if( --line == 0 ) strcpy( script1, "text 90 170 0 27");
		if( --line == 0 ) strcpy( script1, "text 200 170 16 28");
		if( --line == 0 ) strcpy( script1, "button 30 170 2116 2115 1 0 14");
		if( --line == 0 ) strcpy( script1, "text 90 195 0 29");
		if( --line == 0 ) strcpy( script1, "text 200 195 16 30");
		if( --line == 0 ) strcpy( script1, "button 30 195 2116 2115 1 0 15");
		if( --line == 0 ) strcpy( script1, "text 90 220 0 31");
		if( --line == 0 ) strcpy( script1, "text 200 220 16 32");
		if( --line == 0 ) strcpy( script1, "button 30 220 2116 2115 1 0 16");
		if( --line == 0 ) strcpy( script1, "text 90 245 0 33");
		if( --line == 0 ) strcpy( script1, "text 200 245 16 34");
		if( --line == 0 ) strcpy( script1, "button 30 245 2116 2115 1 0 17");
	}
	if( type == 2 )
	{
		if( --line==0 ) strcpy( script1, "text 90 70 0 19");
		if( --line==0 ) strcpy( script1, "text 200 70 16 20");
		if( --line==0 ) strcpy( script1, "button 30 70 2116 2115 1 0 10");
		if( --line==0 ) strcpy( script1, "text 90 95 0 21");
		if( --line==0 ) strcpy( script1, "text 200 95 16 22");
		if( --line==0 ) strcpy( script1, "button 30 95 2116 2115 1 0 11");
		if( --line==0 ) strcpy( script1, "text 90 120 0 23");
		if( --line==0 ) strcpy( script1, "text 200 120 16 24");
		if( --line==0 ) strcpy( script1, "button 30 120 2116 2115 1 0 12");
		if( --line==0 ) strcpy( script1, "text 90 145 0 25");
		if( --line==0 ) strcpy( script1, "text 200 145 16 26");
		if( --line==0 ) strcpy( script1, "button 30 145 2116 2115 1 0 13");
		if( --line==0 ) strcpy( script1, "text 90 170 0 27");
		if( --line==0 ) strcpy( script1, "text 200 170 16 28");
		if( --line==0 ) strcpy( script1, "button 30 170 2116 2115 1 0 14");
		//Karma
		if( --line==0 ) strcpy( script1, "text 90 195 0 29");
		if( --line==0 ) strcpy( script1, "text 200 195 16 30");
		if( --line==0 ) strcpy( script1, "button 30 195 2116 2115 1 0 15");
		//Fame
		if( --line==0 ) strcpy( script1, "text 90 220 0 31");
		if( --line==0 ) strcpy( script1, "text 200 220 16 32");
		if( --line==0 ) strcpy( script1, "button 30 220 2116 2115 1 0 16");
		//Kills
		if( --line==0 ) strcpy( script1, "text 90 245 0 33");
		if( --line==0 ) strcpy( script1, "text 200 245 16 34");
		if( --line==0 ) strcpy( script1, "button 30 245 2116 2115 1 0 17");

	}
	if( --line == 0 ) strcpy( script1, "page 3");
	if( --line == 0 ) strcpy( script1, "button 40 320 2223 2223 0 2");
	if( --line == 0 ) strcpy( script1, "text 65 317 32 2");
	if( --line == 0 ) strcpy( script1, "button 366 320 2224 2224 0 4" );
	if( --line == 0 ) strcpy( script1, "text 288 317 32 1" );
	if( type == 1 )
	{
		if( --line == 0 ) strcpy( script1, "text 90 70 0 35");
		if( --line == 0 ) strcpy( script1, "text 200 70 16 36");
		if( --line == 0 ) strcpy( script1, "button 30 70 2116 2115 1 0 18");
		if( --line == 0 ) strcpy( script1, "text 90 95 0 37");
		if( --line == 0 ) strcpy( script1, "text 200 95 16 38");
		if( --line == 0 ) strcpy( script1, "button 30 95 2116 2115 1 0 19");
		if( --line == 0 ) strcpy( script1, "text 90 120 0 39");
		if( --line == 0 ) strcpy( script1, "text 200 120 16 40");
		if( --line == 0 ) strcpy( script1, "button 30 120 2116 2115 1 0 20");
		if( --line == 0 ) strcpy( script1, "text 90 145 0 41");
		if( --line == 0 ) strcpy( script1, "text 200 145 16 42");
		if( --line == 0 ) strcpy( script1, "button 30 145 2116 2115 1 0 21");
		if( --line == 0 ) strcpy( script1, "text 90 170 0 43");
		if( --line == 0 ) strcpy( script1, "text 200 170 16 44");
		if( --line == 0 ) strcpy( script1, "button 30 170 2116 2115 1 0 22");
		if( --line == 0 ) strcpy( script1, "text 90 195 0 45");
		if( --line == 0 ) strcpy( script1, "text 200 195 16 46");
		if( --line == 0 ) strcpy( script1, "button 30 195 2116 2115 1 0 23");
		if( --line == 0 ) strcpy( script1, "text 90 220 0 47");
		if( --line == 0 ) strcpy( script1, "text 200 220 16 48");
		if( --line == 0 ) strcpy( script1, "button 30 220 2116 2115 1 0 24");
		if( --line == 0 ) strcpy( script1, "text 90 245 0 49");
		if( --line == 0 ) strcpy( script1, "text 200 245 16 50");
		if( --line == 0 ) strcpy( script1, "button 30 245 2116 2115 1 0 25");
	}
	if( type == 2 )
	{
		if( --line == 0 ) strcpy( script1, "text 90 70 0 35");
		if( --line == 0 ) strcpy( script1, "text 200 70 16 36");
		if( --line == 0 ) strcpy( script1, "button 30 70 2116 2115 1 0 18");

	}
	if( --line == 0 ) strcpy( script1, "page 4");
	if( --line == 0 ) strcpy( script1, "button 40 320 2223 2223 0 3");
	if( --line == 0 ) strcpy( script1, "text 65 317 32 2");
	if (type==1)
	{
		if( --line == 0 ) strcpy( script1, "text 90 70 0 51");
		if( --line == 0 ) strcpy( script1, "text 200 70 16 52");
		if( --line == 0 ) strcpy( script1, "button 30 70 2116 2115 1 0 26");
		if( --line == 0 ) strcpy( script1, "text 90 95 0 53");
		if( --line == 0 ) strcpy( script1, "text 200 95 16 54");
		if( --line == 0 ) strcpy( script1, "button 30 95 2116 2115 1 0 27");
		if( --line == 0 ) strcpy( script1, "text 90 120 0 55");
		if( --line == 0 ) strcpy( script1, "text 200 120 16 56");
		if( --line == 0 ) strcpy( script1, "button 30 120 2116 2115 1 0 28");
		if( --line == 0 ) strcpy( script1, "text 90 145 0 57");
		if( --line == 0 ) strcpy( script1, "text 200 145 16 58");
		if( --line == 0 ) strcpy( script1, "button 30 145 2116 2115 1 0 29");

		//end addons by Xuri
	}
	if (--line==0) strcpy( script1,  "}");
}

void ttext(int line, SERIAL serial)
{
	char type;
	if (isCharSerial(serial))
		type = 2;
	else
		type = 1;

	if (type==1) { if( --line == 0 ) strcpy( script1, "Item Properties"); }
	if (type==2) { if( --line == 0 ) strcpy( script1, "Character Properties"); }
	if( --line == 0 ) strcpy( script1, "Next page");
	if( --line == 0 ) strcpy( script1, "Previous page");
	if (type==1)
	{
		P_ITEM pi = pointers::findItemBySerial(serial);
		if (pi == NULL)
			return;

		if( --line == 0 ) strcpy( script1, "Name");
		if( --line == 0 ) strcpy( script1, pi->getCurrentNameC() );
		if( --line == 0 ) strcpy( script1, "ID");
		if( --line == 0 ) sprintf( script1,"0x%x (%i)", pi->id(), pi->id());
		if( --line == 0 ) strcpy( script1, "Hue");
		if( --line == 0 ) sprintf( script1,"0x%x (%i)", pi->color(), pi->color());
		if( --line == 0 ) strcpy( script1, "X");
		if( --line == 0 ) sprintf( script1,"%i (0x%x)", pi->getPosition("x"), pi->getPosition("x"));
		if( --line == 0 ) strcpy( script1, "Y");
		if( --line == 0 ) sprintf( script1,"%i (0x%x)", pi->getPosition("y"), pi->getPosition("y"));
		if( --line == 0 ) strcpy( script1, "Z");
		if( --line == 0 ) sprintf( script1,"%i (0x%x)", pi->getPosition("z"), pi->getPosition("z"));
		if( --line == 0 ) strcpy( script1, "Type");
		if( --line == 0 ) sprintf( script1,"%i", pi->type);
		if( --line == 0 ) strcpy( script1,  "ItemHand" );
		if( --line == 0 ) sprintf( script1, "%i", pi->itmhand );
		if( --line == 0 ) strcpy( script1, "Layer");
		if( --line == 0 ) sprintf( script1,"%i (0x%x)", pi->layer, pi->layer);
		if( --line == 0 ) strcpy( script1, "Amount");
		if( --line == 0 ) sprintf( script1,"%i", pi->amount);
		if( --line == 0 ) strcpy( script1, "More");
		if( --line == 0 ) sprintf( script1,"0x%x", (pi->more1<<24)+(pi->more2<<16)+(pi->more3<<8)+pi->more4);
		if( --line == 0 ) strcpy( script1, "MoreB");
		if( --line == 0 ) sprintf( script1,"0x%x", (pi->moreb1<<24)+(pi->moreb2<<16)+(pi->moreb3<<8)+pi->moreb4);
		if( --line == 0 ) strcpy( script1, "Stackable");
		if( --line == 0 ) sprintf( script1,"%i", pi->pileable);
		if( --line == 0 ) strcpy( script1, "Dyeable");
		if( --line == 0 ) sprintf( script1,"%i", pi->dye );
		if( --line == 0 ) strcpy( script1, "Corpse");
		if( --line == 0 ) sprintf( script1,"%i", pi->corpse );
		if( --line == 0 ) strcpy( script1, "LoDamage" );
		if( --line == 0 ) sprintf( script1,"%i", pi->lodamage );
		if( --line == 0 ) strcpy( script1, "HiDamage" );
		if( --line == 0 ) sprintf( script1,"%i", pi->hidamage );
		if( --line == 0 ) strcpy( script1, "Defense");
		if( --line == 0 ) sprintf( script1,"%i", pi->def);
		if( --line == 0 ) strcpy( script1, "Magic");
		if( --line == 0 ) sprintf( script1,"%i", pi->magic);
		if( --line == 0 ) strcpy( script1, "Visible");
		if( --line == 0 ) sprintf( script1,"%i", pi->visible);
		if( --line == 0 ) strcpy( script1, "HitPoints" );
		if( --line == 0 ) sprintf( script1, "%i", pi->hp );
		if( --line == 0 ) strcpy( script1, "MaxHitPoints" );
		if( --line == 0 ) sprintf( script1, "%i", pi->maxhp );
		if( --line == 0 ) strcpy( script1, "Speed" );
		if( --line == 0 ) sprintf( script1, "%i", pi->spd );
		if( --line == 0 ) strcpy( script1, "Rank");
		if( --line == 0 ) sprintf( script1,"%i", pi->rank);
		if( --line == 0 ) strcpy( script1, "Value");
		if( --line == 0 ) sprintf( script1,"%i", pi->value);
		if( --line == 0 ) strcpy( script1, "Good" );
		if( --line == 0 ) sprintf( script1,"%i", pi->good );
		if( --line == 0 ) strcpy( script1, "Made Skill" );
		if( --line == 0 ) sprintf( script1, "%i", pi->madewith );
		if( --line == 0 ) strcpy( script1, "Creator" );
		if( --line == 0 ) strcpy(script1, pi->creator.c_str() );

	}
	if( type == 2 )
	{
		P_CHAR pc_j = pointers::findCharBySerial(serial);
		if (pc_j == NULL)
			return;

		Location charpos= pc_j->getPosition();

		if( --line == 0 ) strcpy( script1, "Name" );
		if( --line == 0 ) strcpy( script1,  pc_j->getCurrentNameC() );
		if( --line == 0 ) strcpy( script1, "Title" );
		if( --line == 0 ) strcpy( script1,  pc_j->title.c_str() );
		if( --line == 0 ) strcpy( script1, "X" );
		if( --line == 0 ) sprintf( script1,"%i", charpos.x );
		if( --line == 0 ) strcpy( script1, "Y" );
		if( --line == 0 ) sprintf( script1,"%i", charpos.y );
		if( --line == 0 ) strcpy( script1, "Z" );
		if( --line == 0 ) sprintf( script1,"%i", charpos.z );
		if( --line == 0 ) strcpy( script1, "Direction" );
		if( --line == 0 ) sprintf( script1,"%i", pc_j->dir );
		if( --line == 0 ) strcpy( script1, "Body" );
		if( --line == 0 ) sprintf( script1,"(0x%x) %i", pc_j->GetBodyType(), pc_j->GetBodyType() );
		if( --line == 0 ) strcpy( script1, "Skin" );
		if( --line == 0 ) sprintf( script1, "(0x%x) %i", pc_j->getSkinColor(), pc_j->getSkinColor() );
		if( --line == 0 ) strcpy( script1, "Defence" );
		if( --line == 0 ) sprintf( script1,"%i", pc_j->def );
		if( --line == 0 ) strcpy( script1, "Hunger" );
		if( --line == 0 ) sprintf( script1,"%i", pc_j->hunger );
		if( --line == 0 ) strcpy( script1, "Strength" );
		if( --line == 0 ) sprintf( script1,"%i", pc_j->getStrength() );
		if( --line == 0 ) strcpy( script1, "Dexterity" );
		if( --line == 0 ) sprintf( script1,"%i", pc_j->dx );
		if( --line == 0 ) strcpy( script1, "Intelligence" );
		if( --line == 0 ) sprintf( script1,"%i", pc_j->in );
		if( --line == 0 ) strcpy( script1, "Karma" );
		if( --line == 0 ) sprintf( script1,"%i", pc_j->GetKarma() );
		if( --line == 0 ) strcpy( script1, "Fame" );
		if( --line == 0 ) sprintf( script1,"%i", pc_j->GetFame() );
		if( --line == 0 ) strcpy( script1, "Kills" );
		if( --line == 0 ) sprintf( script1,"%i", pc_j->kills );

	}
	if( --line == 0) strcpy( script1, "}");
}


void tweakmenu(NXWSOCKET  s, SERIAL serial)
{
	char sect[512];
	short int length, length2, textlines;
	UI32 i, line;
	int loopexit=0;

	char type;
	if (isCharSerial(serial))
		type = 2;
	else
		type = 1;


	length=21;
	length2=1;
	line=0;

	do
	{
		line++;
		tline(line, serial);
		if (script1[0]!='}')
		{
			length+=strlen(script1)+4;
			length2+=strlen(script1)+4;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	length+=3;
	textlines=0;
	line=0;


	loopexit=0;
	do
	{
		line++;
		ttext(line, serial);
		if (script1[0]!='}')
		{
			length+=(strlen(script1)*2)+2;
			textlines++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

	UI08 gump1[22]={ 0xB0, 0x00, };

	ShortToCharPtr(length, gump1 +1); 		// Total Length
	LongToCharPtr(serial, gump1 +3);		// id
	LongToCharPtr((UI32)type, gump1 +7); 		// Gump Number
	LongToCharPtr(0x0000006E, gump1 +11); 		// X
	LongToCharPtr(0x00000046, gump1 +15); 		// Y
	ShortToCharPtr(length2, gump1 +19); 		// command section length
	Xsend(s, gump1, 21);

	line=0;
	loopexit=0;
	do
	{
		line++;
		tline(line, serial);
		if (script1[0]!='}')
		{
			sprintf(sect, "{ %s }", script1);
			Xsend(s, sect, strlen(sect));
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

	UI08 gump2[3]={ 0x00, };
	ShortToCharPtr(textlines, gump2 +1);
	Xsend(s, gump2, 3);
	line=0;

	UI08 gump3[2]={0x00, };
	loopexit=0;
	do
	{
		line++;
		ttext(line, serial);
		if (script1[0]!='}')
		{
			ShortToCharPtr(strlen(script1), gump3);
			Xsend(s, gump3, 2);
			gump3[0]=0;
			for (i=0;i<strlen(script1);i++)
			{
				gump3[1]=script1[i];
				Xsend(s, gump3, 2);
			}
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
//AoS/	Network->FlushBuffer(s);
}

void entrygump(int s, unsigned char tser1, unsigned char tser2, unsigned char tser3, unsigned char tser4, unsigned char type, char index, short int maxlength, char *text1)
{
	short int length;
	SERIAL tser = calcserial(tser1, tser2, tser3, tser4);
	UI08 textentry1[11]={ 0xAB, 0x00, };
	UI08 textentry2[8]={ 0x00, };

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	sprintf(temp, "(%i chars max)", maxlength);
	length=11+strlen(text1)+1+8+strlen(temp)+1;
	ShortToCharPtr(length, textentry1 +1);
	LongToCharPtr(tser, textentry1 +3);
	textentry1[7]=type;
	textentry1[8]=index;
	ShortToCharPtr(strlen(text1)+1, textentry1 +9);
	Xsend(s, textentry1, 11);
	Xsend(s, text1, strlen(text1)+1);
	textentry2[0]=0x01;
	textentry2[1]=0x01;
	LongToCharPtr(maxlength, textentry2 +2);
	ShortToCharPtr(strlen(temp)+1, textentry2 +6);
	Xsend(s, textentry2, 8);
	Xsend(s, temp, strlen(temp)+1);
//AoS/	Network->FlushBuffer(s);
}


/*!
\brief Choice from GMMenu, Itemmenu or Makemenu received
\note This routine is changed b Magius(CHE) to add Rank_system!
\note Ported to std::string by Akron
\param s socket that opened the gump
*/
void choice(int s) // Choice from GMMenu, Itemmenu or Makemenu received
{ // This routine is changed by Magius(CHE) to add Rank_system!
	if (s < 0 || s >= now) return; //Luxor
	int main, sub,loopexit=0;
	char sect[512];
	int i;
	int seed = (buffer[s][4]<<24)+(buffer[s][3]<<16)+(buffer[s][2]<<8)+(buffer[s][1]);
	cScpIterator* iter = NULL;
	std::string script1, script2, lscomm, lsnum;

	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	main = ShortFromCharPtr(buffer[s]+5);
	sub  = ShortFromCharPtr(buffer[s]+7);

//    ConOut("main:%i sub:%i \n",main,sub);
	// if ((main!=0) && (sub==0)) ConOut("add menu (gm menu) closed- including its submenus\n");

	if ((main>=8000)&&(main<=8100)) Guilds->GumpChoice(s,main,sub);

/*	if( (main&0xFF00)==0xFF00)
	{
		if (im_choice(s, main, sub)==0) return;
	}
*//*
	else if (main<ITEMMENUOFFSET) // GM Menus
	{
	}
	else if(main>=MAKEMENUOFFSET && main<TRACKINGMENUOFFSET)
	{
		sprintf(sect, "SECTION MAKEMENU %i", main-MAKEMENUOFFSET);
		iter = Scripts::Create->getNewIterator(sect);
	}// PolyMorph spell menu (scriptable) by AntiChrist (9/99)
	else if(main>=POLYMORPHMENUOFFSET && main<POLYMORPHMENUOFFSET+50)
	{
		WarnOut("--Polymorph disabled due to magic rewrite--\n");
		return;
	}
	else if(main==CUSTOMICONLISTVALUE)
	{
		int btn = sub+9;
		if (sub == 9) sub = 1;
		if (pc->customMenu != NULL)
			pc->customMenu->buttonSelected(s, btn, seed );
		return;
	}
	else if((main-TRACKINGMENUOFFSET)>=TRACKINGMENUOFFSET+1&&(main-TRACKINGMENUOFFSET)<=TRACKINGMENUOFFSET+3) // TRACKING
	{
		if(!sub) return;
		if(!pc->checkSkill(TRACKING, 0, 1000))
		{
			pc->sysmsg(TRANSLATE("You fail your attempt at tracking."));
			return;
		}
		Skills::TrackingMenu(s,sub-1);
	}
	else if((main-TRACKINGMENUOFFSET+sub)>=TRACKINGMENUOFFSET+1&&(main-TRACKINGMENUOFFSET)<=TRACKINGMENUOFFSET+3) // TRACKING
	{
		if(!sub) return;
		if(!pc->checkSkill(TRACKING, 0, 1000))
		{
			pc->sysmsg(TRANSLATE("You fail your attempt at tracking."));
			return;
		}
		Skills::CreateTrackingMenu(s,main-TRACKINGMENUOFFSET+sub);
	}

	if (iter==NULL) return; // no iter loaded :]


	iter->getEntry()->getFullLine(); //discards the "{"
	script1 = iter->getEntry()->getFullLine();

	i=0;
	iter->parseLine(script1, script2);

	loopexit=0;
	do
	{
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			i++;
			if (main>=MAKEMENUOFFSET && main<TRACKINGMENUOFFSET)
			{
				iter->parseLine(script1, script2);
				itemmake[s].needs=str2num(script2);

				iter->parseLine(script1, script2);
				itemmake[s].minskill=str2num(script2);
				itemmake[s].maxskill=itemmake[s].minskill*SrvParms->skilllevel;
				// ConOut("needs %i, has %i\nskillneed %i, skillhas %i\n",itemmake[s].need,itemmake[s].has,itemmake[s].minskill,chars[currchar[s]].skill[chars[currchar[s]].making]);
				// Duke: we must count with the same criteria as in MakeMenu() !
				if (pc->making > 0 && pc->making < TRUESKILLS) //Luxor
				if (    itemmake[s].has<itemmake[s].needs
					|| (itemmake[s].has2 && itemmake[s].has2<itemmake[s].needs)
					|| (pc->skill[pc->making] < itemmake[s].minskill))
					i--;    // skip this item
			}

			iter->parseLine(script1, script2);
			lscomm = script1;
			lsnum = script2;
			itemmake[s].number=str2num(script2);

			iter->parseLine(script1, script2);
			if ( "RANK" == script1 )
			{
				gettokennum(script2, 0);
				itemmake[s].minrank=str2num(gettokenstr);
				gettokennum(script2, 1);
				itemmake[s].maxrank=str2num(gettokenstr);
				iter->parseLine(script1, script2);		// found a rank line, so read one more
			} else
			{ // Set maximum rank if the item is not ranked!
				itemmake[s].minrank=itemmake[s].maxrank=10;
			}
			if (SrvParms->rank_system==0)
			{
				itemmake[s].minrank=itemmake[s].maxrank=10;
			}

			if (i==sub)
			{
				scriptcommand(s, lscomm, lsnum);
				if (itemmake[s].maxskill<200) itemmake[s].maxskill=200;
			}
		}
	}
	while ((script1[0]!='}')&&(i!=sub) && (++loopexit < MAXLOOPS));

	safedelete(iter);*/
}

void gumps::Open(int s, int i, int num1, int num2)
{
	P_CHAR pc=MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
	
	UI08 shopgumpopen[7]={ 0x24, 0x00, };
	LongToCharPtr(pc->getSerial32(), shopgumpopen +1);
	shopgumpopen[5]= num1;
	shopgumpopen[6]= num2;
	Xsend(s, shopgumpopen, 7);
//AoS/	Network->FlushBuffer(s);
}

