/*!
\mainpage Nox-Wizard documentation

	<h1>NoX-Wizard Emu (NXW)</h1>
	UO Server Emulation Program
	<i>Work based on the Wolfpack and UOX Projects.</i>

	This Project started on June 2001, by Xanathar and Ummon
	Currently it's coded by Luxor, Sparhawk, Elcabesa, Anthalir and Endymion

	Copyright 1997, 98 by Marcus Rating (Cironian)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	<b>In addition to that license, if you are running this program or modified
	versions of it on a public system you HAVE TO make the complete source of
	the version used by you available or provide people with a location to
	download it.</b>

	-----------------------------------------------------------------------------

	NoX-Wizard also contains portions of code from the Small toolkit.

	The software toolkit "Small", the compiler, the abstract machine and the
	documentation, are copyright (c) 1997-2001 by ITB CompuPhase.
	See amx_vm.c and/or small_license.txt for more information about this.

	==============================================================================

	You can find info about the authors in the AUTHORS file.

	<a href="http://noxwizard.sourceforge.net/">NoX-Wizard Homepage</a>

	\section link Some links
	Link to documentation page that can be useful
	\subsection uo_protocol UO Protocol
	Here is a page regarding the uo protocol: <a href='http://gonzo.kiev.ua/guide/node1.html'>http://gonzo.kiev.ua/guide/node1.html</a>

*/

#include "nxwcommn.h"
#include "basics.h"
#include "cmdtable.h"
#include "speech.h"
#include "sndpkg.h"
#include "sregions.h"
#include "remadmin.h"
#include "srvparms.h"
#include "amx/amxcback.h"
#include "crontab.h"
#include "version.h"
#include "calendar.h"
#include "ntservice.h"
#include "trigger.h"
#include "collector.h"
#include "magic.h"
#include "set.h"
#include "addmenu.h"
#include "race.h"
#include "party.h"
#include "npcai.h"
#include "network.h"
#include "tmpeff.h"
#include "layer.h"

#include "ai.h"

#ifdef _WINDOWS
	#include "nxwgui.h"
#endif

extern void initSignalHandlers();

extern void checkGarbageCollect(); //!< Remove items which were in deleted containers



bool g_bInMainCycle = false;
char INKEY; //xan, used for wingui menus :) [gui mode under Win32 :)]
void LoadOverrides ();
extern "C" int g_nTraceMode;

#include "debug.h"

RemoteAdmin TelnetInterface;	//!< remote administration


static void item_char_test()
{
	LogMessage("Starting item consistancy check");
	ConOut("Starting item consistancy check... \n");


	cAllObjectsIter objs;
	for( objs.rewind(); !objs.IsEmpty(); objs++ )
	{
		SERIAL ser=objs.getSerial();

		if( isItemSerial( ser ) ) {

			P_ITEM pi=(P_ITEM)(objs.getObject() );

			if (pi->getSerial32()==INVALID) {
				WarnOut("item %s [serial: %i] has invalid serial!",pi->getCurrentNameC(),pi->getSerial32());
				LogWarning("ALERT ! item %s [serial: %i] has invalid serial!",pi->getCurrentNameC(),pi->getSerial32());
			}

			// item is contained in himself
			if (pi->getSerial32()==pi->getContSerial())
			{
				WarnOut("item %s [serial: %i] has dangerous container value, autocorrecting",pi->getCurrentNameC(),pi->getSerial32());
				LogWarning("ALERT ! item %s [serial: %i] has dangerous container value, autocorrecting",pi->getCurrentNameC(),pi->getSerial32());
				pi->setContSerial(INVALID);
			}

			// item is owned by himself
			if (pi->getSerial32()==pi->getOwnerSerial32())
			{
				WarnOut("item %s [serial: %i] has dangerous owner value",pi->getCurrentNameC(),pi->getSerial32());
				LogWarning("ALERT ! item %s [serial: %i] has dangerous owner value",pi->getCurrentNameC(),pi->getSerial32());
				pi->setOwnerSerial32(INVALID);
			}

		}
		else {
			P_CHAR p_pet = (P_CHAR)(objs.getObject());

			if (p_pet->isStabled())
			{
				P_CHAR stablemaster=pointers::findCharBySerial(p_pet->getStablemaster());
				if (!ISVALIDPC(stablemaster))
				{
					p_pet->unStable();
					regions::add(p_pet);
					pointers::addCharToLocationMap( p_pet );
					p_pet->timeused_last=getclock();
					p_pet->time_unused=0;
					LogMessage("Stabled animal got freed because stablemaster died");
					InfoOut("stabled animal got freed because stablemaster died\n");
				}
			 }
		}
	}
	ConOut("[DONE]\n");
}

int validhair(int a, int b) // Is selected hair type valid
{

	if( a != 0x20 )
		return 0;
	switch( b )
	{
	case 0x3B:
	case 0x3C:
	case 0x3D:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4A:
		return 1;
	default:
		return 0;
	}
}

int validbeard(int a, int b) // Is selected beard type valid
{
	if( a != 0x20 )
		return 0;
	switch( b )
	{
	case 0x3E:
	case 0x3F:
	case 0x40:
	case 0x41:
	case 0x4B:
	case 0x4C:
	case 0x4D:
		return 1;
	default:
		return 0;
	}
}



/*!
\brief called when a player creates a new character
\author ?
\param s the player's socket
*/
void charcreate( NXWSOCKET  s ) // All the character creation stuff
{

	if (s < 0) return; //Luxor

	unsigned int i ;
	signed int ii ;
	int totalstats,totalskills;

	NxwCharWrapper sc;
	accounts::GetAllChars( acctno[s], sc );


	if ( sc.size() >= ServerScp::g_nLimitRoleNumbers) {
		Network->Disconnect(s);
		return;
	}

	P_CHAR pc = archive::character::New();

	pc->setCurrentName( (const char*)&buffer[s][10] );
	accounts::AddCharToAccount( acctno[s], pc );

	/*SERIAL currserial = pc->getSerial32();
	vector<SERIAL>::iterator currCharIt( currchar.begin()), currCharEnd(currchar.end());
	UI32 nSize;*/
	/*for ( ; currCharIt != currCharEnd; currCharIt++) {
		if ((*currCharIt) == currserial) {
			currchar.erase(currCharIt);
			break;
		}
	}
	*/
	/*currCharIt = currchar.begin();
	nSize = currchar.size();
	for (i = 0; i < nSize; i++) {
		if (s == i) {
			currchar.erase(currCharIt);
			//ConOut("Same socket found, erasing...");
		}
		currCharIt++;
	}
	currchar.push_back(pc->getSerial32());
	currCharIt = currchar.begin();
	nSize = currchar.size();
	if (s < nSize) {
		for (i = 0; i < nSize; i++) {
			if (i == s) {
				currchar.insert(currCharIt, pc->getSerial32());
				break;
			}
			currCharIt++;
		}
	} else if (s == nSize) {
		currchar.push_back(pc->getSerial32());
	} else if (s > nSize) {
		for (i = 0; i < (s - nSize); i++)
			currchar.push_back(-1);

		currchar.push_back(pc->getSerial32());
	}*/
	currchar[s] = pc->getSerial32();

	pc->setClient(new cNxwClientObj(s));

	/*nSize = currchar.size();
	ConOut("Player Connected - currchar status:");
	P_CHAR pj = NULL;
	for (i = 0; i < nSize; i++) {
		pj = MAKE_CHAR_REF(currchar[i]);
		ConOut("currchar[%i] = %s\n", i, pj->getCurrentNameC());
	}*/

	//allclients.insert( make_pair( pc->getSerial32(), cNxwClientObj( s ) ) );

	pc->npc=false;

	if (buffer[s][0x46] != 0x00)
	{
		pc->SetBodyType( BODY_FEMALE );
	}
	else {
		pc->SetBodyType( BODY_MALE );
	}

	pc->setSkinColor(ShortFromCharPtr(buffer[s] + 0x50) | 0x8000);

	if ( (pc->getSkinColor()<0x83EA) || (pc->getSkinColor()>0x8422) )
	{
		pc->setSkinColor(0x83EA);
	}

	pc->SetOldBodyType(pc->GetBodyType());
	pc->setOldSkinColor(pc->getSkinColor());
	pc->SetPriv(defaultpriv1);
	pc->priv2=defaultpriv2;

	if (acctno[s]==0)
	{
		pc->SetPriv(0xE7);
		pc->priv3[0]=0xFFFFFFFF; // lb, meta gm
		pc->priv3[1]=0xFFFFFFFF;
		pc->priv3[2]=0xFFFFFFFF;
		pc->priv3[3]=0xFFFFFFFF;
		pc->priv3[4]=0xFFFFFFFF;
		pc->priv3[5]=0xFFFFFFFF;
		pc->priv3[6]=0xFFFFFFFF;
		pc->menupriv=INVALID; // lb, menu priv
	}
	else
	{
		for(i=0;i<7;i++) pc->priv3[i]=metagm[2][i]; //normal player defaults for all but account 0 chars, Ummon
	}


	Location charpos;
	charpos.x= str2num(start[buffer[s][0x5B]][2]);
	charpos.y= str2num(start[buffer[s][0x5B]][3]);
	charpos.dispz= charpos.z= str2num(start[buffer[s][0x5B]][4]);
	pc->MoveTo( charpos );

	pc->dir=4;
	pc->namedeedserial=INVALID;

	pc->st3 = pc->hp = buffer[s][0x47];
	pc->setStrength(buffer[s][0x47]);

	if ( pc->st3 > 60 )
	{
		pc->st3 = pc->hp = 60; // fix for hack exploit
		pc->setStrength(60);
	}

	if ( pc->st3 < 10 )
	{
		pc->st3 = pc->hp = 10;
		pc->setStrength(10);
	}

	totalstats = pc->st3;

	pc->dx3 = pc->stm = pc->dx = buffer[s][0x48];
	if ( pc->dx3 > 60 ) pc->dx3 = pc->stm = pc->dx = 60; // fix for hack exploit
	if ( pc->dx3 < 10 ) pc->dx3 = pc->stm = pc->dx = 10;
	if ( pc->dx3 + totalstats > 80 ) pc->dx3 = pc->stm = pc->dx = 80 - totalstats;
	totalstats += pc->dx3;

	pc->in3 = pc->mn = pc->in = buffer[s][0x49];
	if ( pc->in3 > 60 ) pc->in3 = pc->mn = pc->in = 60;		// fix for hack exploit
	if ( pc->in3 < 10 ) pc->in3 = pc->mn = pc->in = 10;
	if ( pc->in3 + totalstats > 80) pc->in3 = pc->mn = pc->in = 80 - totalstats;

	if (buffer[s][0x4b]>50) buffer[s][0x4b]=50; // fixes for hack exploit
	totalskills=buffer[s][0x4b];
	if (buffer[s][0x4d]>50) buffer[s][0x4d]=50;
	if (buffer[s][0x4d]+totalskills>100) buffer[s][0x4d]=100-totalskills;
	totalskills+=buffer[s][0x4d];
	if (buffer[s][0x4f]>50) buffer[s][0x4f]=50;
	if (buffer[s][0x4f]+totalskills>100) buffer[s][0x4f]=100-totalskills;

	for (ii=0;ii<TRUESKILLS;ii++)
	{
		pc->baseskill[ii]=0;
		if (ii==buffer[s][0x4a]) pc->baseskill[buffer[s][0x4a]]=buffer[s][0x4b]*10;
		if (ii==buffer[s][0x4c]) pc->baseskill[buffer[s][0x4c]]=buffer[s][0x4d]*10;
		if (ii==buffer[s][0x4e]) pc->baseskill[buffer[s][0x4e]]=buffer[s][0x4f]*10;
		Skills::updateSkillLevel(pc, ii);
	}

	if (validhair(buffer[s][0x52],buffer[s][0x53]))
	{
		P_ITEM pi = item::CreateFromScript( "$item_short_hair" );
		VALIDATEPI(pi);
		pi->setId( ShortFromCharPtr(buffer[s] + 0x52) );
		pi->setColor( ShortFromCharPtr(buffer[s] + 0x54) );
		if ((pi->color()<0x044E) || (pi->color()>0x04AD) )
		{
			pi->setColor(0x044E);
		}
		pi->setContSerial(pc->getSerial32());
		pi->layer=LAYER_HAIR;
	}

	if ( (validbeard(buffer[s][0x56],buffer[s][0x57])) && (pc->GetBodyType() == BODY_MALE) )
	{
		P_ITEM pi = item::CreateFromScript( "$item_short_beard" );
		VALIDATEPI(pi);
		pi->setId( ShortFromCharPtr(buffer[s] + 0x56) );
		pi->setColor( ShortFromCharPtr(buffer[s] + 0x58) );
		if ( (pi->color()<0x044E) || (pi->color()>0x04AD) )
		{
			pi->setColor(0x044E);
		}
		pi->setContSerial(pc->getSerial32());
		pi->layer=LAYER_BEARD;
	}

	P_ITEM pi;

// - create the backpack
	pi= item::CreateFromScript( "$item_backpack");
	VALIDATEPI(pi);
	pc->packitemserial= pi->getSerial32();
	pi->setCont(pc);

// - create pants
	if( RandomNum(0, 1)==0 )
	{
		if( pc->GetBodyType() == BODY_MALE )
			pi= item::CreateFromScript( "$item_long_pants");
		else
			pi= item::CreateFromScript( "$item_a_skirt");
	}
	else
	{
		if( pc->GetBodyType() == BODY_MALE  )
			pi= item::CreateFromScript( "$item_short_pants");
		else
			pi= item::CreateFromScript( "$item_a_kilt");
	}

	VALIDATEPI(pi);

	// pant/skirt color -> old client code, random color
	pi->setColor( ShortFromCharPtr(buffer[s] +102) );
	pi->setCont(pc);

	if( !(rand()%2) )
		pi= item::CreateFromScript( "$item_fancy_shirt");
	else
		pi= item::CreateFromScript( "$item_shirt");

	VALIDATEPI(pi);
	pi->setColor( ShortFromCharPtr(buffer[s] +100) );
	pi->setCont(pc);

// what is this ??? (Anthalir)
/*
#ifdef SPECIAL
	{	// limit the scope of pi
		n=item::SpawnItem(s,1,"#",0,0x09,0x15,0,0,0,0);
		if(n==INVALID) return;//AntiChrist to preview crashes
		const P_ITEM pi=MAKE_ITEMREF_LR(n);	// on error return

		switch (RandomNum(0, 6))
		{
		case 0:	pi->id1=0x15; pi->id2=0x4b; break;
		case 1:	pi->id1=0x15; pi->id2=0x45; break;
		case 2:	pi->id1=0x15; pi->id2=0x47; break;
		case 3:	pi->id1=0x15; pi->id2=0x49; break;
		case 4:	pi->id1=0x17; pi->id2=0x1c; break;
		case 5:	pi->id1=0x1f; pi->id2=0x0b; break;
		case 6:	pi->id1=0x14; pi->id2=0x51; break;
		}
		setserial(n, c, 4);
		pi->layer=0x06;
	}
#endif
*/

	// Give the character some gold
	if ( goldamount > 0 )
	{
		pi = item::CreateFromScript( "$item_gold_coin", pc->getBackpack(), goldamount );
	}


	perm[s]=1;

	newbieitems(pc);

	Network->startchar(s);
}

//taken from 6904t2(5/10/99) - AntiChrist
void callguards( CHARACTER p )
{
	P_CHAR	caller = MAKE_CHAR_REF( p );

	if ( !ISVALIDPC( caller ) )
		return;

	if( !(region[caller->region].priv&0x01 ) || !SrvParms->guardsactive || !TIMEOUT( caller->antiguardstimer ) || caller->dead )
		return;

	caller->antiguardstimer=uiCurrentTime+(MY_CLOCKS_PER_SEC*10);

	/*
	Sparhawk:	1. when instant guard is set and offender nearby caller spawn guard near caller and leave attacking to checkAI
			2. when instant guard is not set and offender nearby caller walk toward caller and leave attacking to checkAI
	*/
	bool offenders = false;
	vector < P_CHAR > guards;

//	int loopexit=0; // unused variable

	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( caller->getPosition(), VISRANGE, true, false  );
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {

		P_CHAR character=sc.getChar();
		if(!ISVALIDPC(character))
			continue;
		if( caller->getSerial32() != character->getSerial32() && caller->distFrom( character )  <= 15 && !character->dead && !character->IsHidden())
		{
			if ((!character->IsInnocent() || character->npcaitype == NPCAI_EVIL) && !character->IsHidden() )
				offenders = true;
			else
				if ((character->npcaitype == NPCAI_TELEPORTGUARD || character->npcaitype == NPCAI_GUARD) && !character->war && character->npcWander != 1)
					guards.push_back( character );
		}
	}
	if ( offenders )
	{
		if ( guards.empty() && ServerScp::g_nInstantGuard == 1 )
		{
			P_CHAR  guard = npcs::AddNPCxyz( caller->getSocket(), region[caller->region].guardnum[(rand()%10)+1], caller->getPosition());

			if ( ISVALIDPC( guard ) )
			{
				guard->npcaitype=NPCAI_TELEPORTGUARD;
				guard->npcWander=2;
				guard->setNpcMoveTime();
				guard->summontimer = uiCurrentTime + MY_CLOCKS_PER_SEC * 25 ;

				guard->playSFX( 0x01FE );
				guard->staticFX(0x372A, 9, 6);

				guard->teleport();
				guard->talkAll( TRANSLATE("Don't fear, help is near"), 0 );
			}
		}
		else
		{
			P_CHAR guard;
			while( !guards.empty() )
			{
				guard = guards.back();
				guard->oldnpcWander = guard->npcWander;
				guard->npcWander = 1;
				guard->ftargserial = caller->getSerial32();
				guard->antiguardstimer=uiCurrentTime+(MY_CLOCKS_PER_SEC*10); // Sparhawk this should become server configurable
				guard->talkAll( TRANSLATE("Don't fear, help is on the way"), 0 );
				//guard->antispamtimer = uiCurrentTime+MY_CLOCKS_PER_SEC*5;
				guards.pop_back();
			}
		}
	}
}

extern "C" void breakOnFirstFuncz();

/*!
\brief facilitate console control. SysOp keys and localhost controls
\author ?
*/
void checkkey ()
{
	char c;
	int i,j=0;

	// Akron: borland c++ builder doesn't accept kbhit() in windows mode
	// Also to force only remote admin under unix, we can remove the kbhit() test

#if !defined __BORLANDC__ && !defined __unix__
	if ((kbhit())||(INKEY!='\0'))
#else
	if ( INKEY != '\0' )
#endif
	{
		if (INKEY!='\0') {
			c = toupper(INKEY);
			INKEY = '\0';
			secure = 0;
		}
	#if !defined __BORLANDC__ && !defined __unix__
		else {
			c = toupper(getch());
		}
	#endif

		if (c=='S')
		{
			if (secure)
			{
				InfoOut("Secure mode disabled. Press ? for a commands list.\n");
				secure=0;
				return;
			}
			else
			{
				InfoOut("Secure mode re-enabled.\n");
				secure=1;
				return;
			}
		} else {
			if (secure && c != '?')  //Allows help in secure mode.
			{
				InfoOut("Secure mode prevents keyboard commands! Press 'S' to disable.\n");
				return;
			}

			switch(c)
			{
			case '\x1B':
				keeprun=false;
				break;
			case 'B':
			case 'b':
				if (ServerScp::g_nLoadDebugger==0) {
					InfoOut("You must enable debugger starting NoX-Wizard with the -debug option.\n");
					break;
				}
				breakOnFirstFuncz();
				InfoOut("Debug break scheduled for next amx call\n");
				break;
			case 'C':
			case 'c':
				if (ServerScp::g_nLoadDebugger==0) {
					InfoOut("You must enable debugger starting NoX-Wizard with the -debug option.\n");
					break;
				}
				g_bStepInTriggers = true;
				InfoOut("Debug break scheduled for next trigger call\n");
				break;
			case 'N':
			case 'n':
				if (ServerScp::g_nLoadDebugger==0) {
					InfoOut("You must enable debugger starting NoX-Wizard with the -debug option.\n");
					break;
				}
				g_nTraceMode=1-g_nTraceMode;
				InfoOut("Native tracing %s\n", (g_nTraceMode!=0) ? "enabled" : "disabled");
				break;
			case 'Q':
			case 'q':
				InfoOut("Immediate Shutdown initialized!\n");
				keeprun=false;
				break;
			case 'T':
			case 't':
				endtime=uiCurrentTime+(MY_CLOCKS_PER_SEC*60*2);
				endmessage(0);
				break;
			case '#':
				if ( !cwmWorldState->Saving() )
				{
					cwmWorldState->saveNewWorld();
				}
				break;
			case 'L':
			case 'l':
				if (g_nShowLayers)
				{
					InfoOut("Layer display disabled.\n");
					g_nShowLayers=false;
				}
				else
				{
					InfoOut("Layer display enabled.\n");
					g_nShowLayers=true;
				}
				break;
			case 'D':	// Disconnect account 0 (useful when client crashes)
			case 'd':
				{
					int found = 0;
					InfoOut( "Disconnecting account 0 players... ");
					for (i=0;i<now;i++)
						if (acctno[i]==0 && perm[i])
						{
							found++;
							Network->Disconnect(i);
						}
					if (found>0) ConOut( "[ OK ] (%d disconnected)\n", found);
					else ConOut( "[FAIL] (no account 0 players online)\n", found);
				}
				break;
			case 'H':
			case 'h':				// Enable/Disable heartbeat
				if (heartbeat)
					InfoOut("NoX-Wizard: Heartbeat Disabled\n");
				else
					InfoOut("NoX-Wizard: Heartbeat Enabled\n");
				heartbeat = !heartbeat;
				break;
			case 'P':
			case 'p':				// Display profiling information
				ConOut("----------------------------------------------------------------\n");
				ConOut("Performace Dump:\n");
				ConOut("Network code: %fmsec [%i samples]\n" , (float)((float)networkTime/(float)networkTimeCount) , networkTimeCount);
				ConOut("Timer code: %fmsec [%i samples]\n" , (float)((float)timerTime/(float)timerTimeCount) , timerTimeCount);
				ConOut("Auto code: %fmsec [%i samples]\n" , (float)((float)autoTime/(float)autoTimeCount) , autoTimeCount);
				ConOut("Loop Time: %fmsec [%i samples]\n" , (float)((float)loopTime/(float)loopTimeCount) , loopTimeCount);
				//ConOut("Characters: %i (Dynamic)		Items: %i (Dynamic)\n" , charcount , char_mem::cmem , itemcount , item_mem::imem);
				ConOut("Simulation Cycles: %f per sec\n" , (1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount))));
				break;
			case 'W':
			case 'w':				// Display logged in chars
				ConOut("----------------------------------------------------------------\n");
				ConOut("Current Users in the World:\n");
				j = 0;  //Fix bug counting ppl online.
				for (i=0;i<now;i++)
				{
					P_CHAR pc_i=MAKE_CHAR_REF(currchar[i]);
					if(ISVALIDPC(pc_i) && perm[i]) //Keeps NPC's from appearing on the list
					{
						ConOut("%i) %s [ %08x ]\n", j, pc_i->getCurrentNameC(), pc_i->getSerial32());
						j++;
					}
				}
				ConOut("Total Users Online: %d\n", j);
				break;
			case 'A': //reload the accounts file
			case 'a':
				InfoOut("Reloading accounts file...");
				accounts::LoadAccounts();
				ConOut("[DONE]\n");
				break;
			case 'x':
			case 'X':
				InfoOut("Reloading XSS scripts...\n");
				deleteNewScripts();
				newScriptsInit();
				ConOut("[DONE]\n");
				break;
			case 'r':
			case 'R':
				InfoOut("Command is disabled\n");
				break;
/*				ConOut("NoX-Wizard: Reloading server.cfg, spawn.scp, and regions.scp...\n");
				loadspawnregions();
				loadregions();
				ConOut("Loading vital scripts... ");
				loadmetagm();
				loadmenuprivs();
				ConOut("[DONE]\n");
				loadserverscript();
				ConOut("NoX-Wizard: Reloading IP Blocking rules...");
				Network->LoadHosts_deny();
				ConOut("[DONE]\n");
				break;
*/			case '?':
				ConOut("Console commands:\n");
				ConOut("	<Esc> or Q: Shutdown the server.\n");
				ConOut("	T - System Message: The server is shutting down in 2 minutes.\n");
				ConOut("	# - Save world\n");
				ConOut("	L - Toggle layer display");
				if (g_nShowLayers)
					ConOut(" [currently enabled]\n");

				else
					ConOut(" [currently disabled]\n");
				ConOut("	D - Disconnect Account 0\n");
				ConOut("	P - Preformance Dump\n");
				ConOut("	W - Display logged in characters\n");
				ConOut("	A - Reload accounts file\n");
				ConOut("	X - Reload XSS scripts\n");
				ConOut("	S - Toggle Secure mode ");
				if (secure)
					ConOut("[enabled]\n");
				else
					ConOut("[disabled]\n");
				ConOut("	B - Set breakpoint on first amx function [DEBUG MODE ONLY]\n");
				ConOut("	C - Set breakpoint on first legacy trigger [DEBUG MODE ONLY]\n\n");
#ifndef _WINDOWS
				ConOut("	N - Toggle tracing of native functions [DEBUG MODE ONLY]\n");
#endif
				ConOut("	? - Commands list (this)\n");
				ConOut("End of commands list.\n");
				break;
			default:
				InfoOut("Key %c [%x] does not preform a function.\n",c,c);
				break;
			}
		}
	}
}



/*!
\brief ?
\author ?
\since <ver>
\remark better to make an extra function cauze in loaditem it could be the case that the
        glower is loaded before the pack
\note TO REMOVE
*//*
void start_glow()
{

	for(int i=0; i<itemcount; i++)
	{
		const P_ITEM pi = MAKE_ITEM_REF(i);
		if( ISVALIDPI( pi ) )
		{
			if (pi->glow>0 && pi->free==0)
			{
				if (!pi->isInWorld())
				{
					P_CHAR pc = pointers::findCharBySerial( pi->getContSerial() ); // find equipped glowing items
					if ( !ISVALIDPC( pc )  )
					{
						P_ITEM cont = (P_ITEM)pi->getContainer();
						if(ISVALIDPI( cont )) {
							pc = pi->getPackOwner(); // find equipped glowing items
							if( ISVALIDPC( pc ) )
							{
								pc->addHalo(pi);
								pc->glowHalo(pi);
							}
						}
					}
				}
			}
		}
	}
}*/

#if defined(__unix__)
/*!
\brief signal handlers
\author ?
\param signal the signal received
*/
void signal_handler(int signal)
{
//	ConOut("In signal handler\n") ;
	switch (signal)
	{
	case SIGHUP:
		//loadspawnregions();
		loadregions();
		loadmetagm();
		loadserverscript();
		Network->LoadHosts_deny();
		break ;

	case SIGUSR1:
		accounts::LoadAccounts();
		break ;
	case SIGUSR2:
		cwmWorldState->saveNewWorld();
		saveserverscript();
		break ;
	case SIGTERM:
		keeprun = false ;
		break ;
	default:
		break ;
	}
}
#endif
/*!
\brief initialize daemon
\author ?
*/
void init_deamon()
{
#if defined(__unix__)

	int i ;
	pid_t pid ;

	if ((pid = fork() ) != 0)
		exit(0) ; //
	setsid() ;
	signal(SIGHUP, SIG_IGN) ;
	if ((pid=fork()) != 0)
	{
		fstream fPid ;
		fPid.open("nxwpid",ios::out) ;
		fPid << pid <<endl;
		fPid.close() ;
		exit(0) ;

	}
	// We should close any dangling descriptors we have
	for (i=0 ; i < 64 ; i++)
		close(i) ;

	// Ok, we are a true deamon now, so we should setup our signal handler
	// We can use SIGHUP, SIGINT, and SIGWINCH as we should never receive them
	// So we will use SIGHUP to reload our scripts (kinda a standard for sighup to be reload)
	// We will use a SIGUSR2 to be world save
	// and SIGUSR1 for an Account reload
	/*
	signal(SIGUSR2,&signal_handler);
	signal(SIGHUP,&signal_handler);
	signal(SIGUSR1,&signal_handler);
	signal(SIGTERM,&signal_handler);
	*/
#endif
}

#ifdef _WINDOWS
void updateMenus();
#endif

void angelMode();

#ifndef _WINDOWS
	#ifndef USES_QT
		int main(int argc, char *argv[])
	#else
		int qtmain(int argc, char *argv[])
	#endif
#else
	extern "C" int win32_main(int argc, char *argv[])
#endif
{
#ifdef WIN32
	#define CIAO_IF_ERROR if (error==1) { Network->SockClose();  DeleteClasses(); 	if (ServerScp::g_nDeamonMode==0) {MessageBox(NULL, "Startup failure. System halted.\nYou can look at the last output to determine the cause of the error.\nPress OK to terminate.", "NoX-Wizard Fatal Error", MB_ICONSTOP);} exit(INVALID); }
#else
	#define CIAO_IF_ERROR if (error==1) { Network->SockClose();  DeleteClasses(); exit(INVALID); }
#endif //win32



	int i;
	unsigned long tempSecs;
	unsigned long tempMilli;
	unsigned long loopSecs;
	unsigned long loopMilli;
	unsigned long tempTime;
	unsigned int uiNextCheckConn=0;//CheckConnection rate - AntiChrist ( thnx to LB )
	unsigned long CheckClientIdle=0;
	int r;

	initclock() ;

	if ((argc>1)&&(strstr(argv[1], "-debug")))	// activate debugger if requested
		ServerScp::g_nLoadDebugger = 1;
	if ((argc>1)&&(strstr(argv[1], "-check")))	// activate check if requested
		ServerScp::g_nCheckBySmall = 1;

	uiCurrentTime=getclock();
	serverstarttime=getclock();

	loadserverdefaults();
	preloadSections("config/server.cfg");
	preloadSections("custom/server.cfg");

	initConsole();

	ConOut("Starting NoX-Wizard...\n\n");

	//XAN : moved here 'cos nxw needs early initialization
	//(has vital data in server.cfg, needed for proper "bootstrap" :))

	loadserverscript();

#ifdef __unix__
    //thx to punt and Plastique :]
    signal(SIGPIPE, SIG_IGN);
	if (ServerScp::g_nDeamonMode!=0) {
		ConOut("Going into deamon mode... bye...\n");
		init_deamon();
	}
#endif
	if (ServerScp::g_nDeamonMode!=0)	// if in daemon mode => disable debugger
		ServerScp::g_nLoadDebugger = 0;

	ConOut("Applying interface settings... ");
	constart();
	ConOut("[ OK ]\n");

	MapTileWidth = ServerScp::g_nMapWidth;
	MapTileHeight = ServerScp::g_nMapHeight;

	openings = 0;
	scpfilename[0] = 0;

	StartClasses();

	CIAO_IF_ERROR;

	Map->Cache=ServerScp::g_nMapCache;
	commitserverscript(); // second phase setup
/*
	if ( (argc>1) && (strstr(argv[1], "-syra")) )
	{

	}
	else
*/
	{
		ConOut("\nLoading scripts with new method...\n");
		newScriptsInit();


		//Now lets load the custom scripts, if they have them defined...
		i=0;

		CIAO_IF_ERROR; // errors here crashes further startup process. so stop insted of crash

		ConOut("\n");
		SetGlobalVars();

		SkillVars();	// Set Creator Variables

		ConOut("Loading skills...");
		loadskills();
		ConOut("[DONE]\n");

		ConOut("Loading Weapon type...");
		loadweaponsinfo();
		ConOut("[DONE]\n");


		ConOut("Loading mountable creature...");
		loadmounts();
		ConOut("[DONE]\n");

		ConOut("Loading container...");
		loadcontainers();
		ConOut("[DONE]\n");

		ConOut("Building pointer arrays...");
		BuildPointerArray();
		ConOut("[DONE]\n");

		ConOut("Loading accounts...");
		accounts::LoadAccounts();
		ConOut("[DONE]\n");

		keeprun=(Network->kr); //LB. for some technical reasons global varaibles CANT be changed in constructors in c++.
		error=Network->faul; // i hope i can find a cleaner solution for that, but this works !!!
		// has to here and not at the cal cause it would get overriten later

		CIAO_IF_ERROR;

		ConOut("Loading areas...");
		areas::loadareas();
		ConOut("[DONE]\n");


		ConOut("Loading spawn regions...");
		spawns::loadFromScript();
		ConOut("[DONE]\n");

		ConOut("Loading regions...");
		loadregions();
		ConOut("[DONE]\n");

		MsgBoards::MsgBoardMaintenance();

		CIAO_IF_ERROR;

		ConOut("\n");
	}

	Map->Load();

	ConOut("Loading Mul files info...\n");
	//TILEDATA
		ConOut("	Loading tiledata ");
		if( server_data.cache_tiledata )
			ConOut("( Caching )");
		ConOut("... ");
		tiledata::tiledata= new tiledata::cTiledata( tiledata::path.c_str(), static_cast<bool>(server_data.cache_tiledata) );
		ConOut("[DONE]\n");
	//
	ConOut("[DONE]\n");

/*
	if ((argc>1)&&(strstr(argv[1], "-syra"))) {
		angelMode();
		exit(99);
	}
*/
	if (!keeprun) error = 1;
#ifdef WIN32
	if (error==1) { Network->SockClose(); DeleteClasses(); if (ServerScp::g_nDeamonMode==0) {MessageBox(NULL, "Startup failure. System halted.\nSome MUL files couldn't be loaded. Fix pathnames in server.cfg.\nPress OK to terminate.", "NoX-Wizard Fatal Error", MB_ICONSTOP);} exit(INVALID); }
#else
	if (error==1) { Network->SockClose(); DeleteClasses(); exit(INVALID); }
#endif //win32

	ConOut("Loading Teleport...");
	read_in_teleport();
	ConOut("[DONE]\n");

	ConOut("Initializing random number seed... [ OK ]\n");

	srand(uiCurrentTime); // initial randomization call
	ConOut("Loading vital scripts... ");
	loadmetagm();
	ConOut("[DONE]\n");
//	ConOut("Loading NoX-Wizard extensions...\n");

	npcs::initNpcSpells();

	TelnetInterface.Init();	// initialise remote admin interface

	initTimers();
	initAmxEvents();
	LoadOverrides ();
	initCronTab();
	Calendar::loadCalendarScp();

	Translation::init_translation(); //belli marco

	serverstarttime=getclock();

	CIAO_IF_ERROR;

	ConOut("\n");
	cwmWorldState->loadNewWorld();

	CIAO_IF_ERROR; // LB prevents file corruption

	ConOut("Clearing all trades...");
	clearalltrades();
	ConOut(" [DONE]\n");

	//ndEndy.. Very laggy stuff, not very usefull, modify house db and boat db insted of search avery time
	//
	//Boats --Check the multi status of every item character at start up to get them set!
	//ConOut("Initializing multis...");
	//InitMultis();
	//ConOut(" [DONE]\n");
	//ConOut(" [FAIL]  <-!!! DISABLED FOR THIS DEBUG VERSION\n");
	//End Boats --^

/*	ConOut("Loading IM menus...");
	im_loadmenus( "inscribe.gmp", TellScroll ); //loading gump for inscribe()
	ConOut(" [DONE]\n");
*/
	gcollect();

//	ConOut("Initializing glowing-items...");
//	start_glow();
//	ConOut(" [DONE]\n"); // Magius(CHE) (1)
	FD_ZERO(&conn);
	starttime=uiCurrentTime;
	endtime=0;
	lclock=0;
	ConOut("Initializing Que System...");
	initque(); // Initialize gmpages[] array

	ConOut(" [DONE]\nLoading custom titles...");
	loadcustomtitle();
	ConOut(" [DONE]\n");

	ConOut("Initializing creatures... ");
	init_creatures(); //lb, initilises the creatures array (with soudfiles and other creatures infos)
	ConOut("[DONE]\n");

	ConOut("Initializing magic... ");
	//Magic->InitSpells();
	magic::loadSpellsFromScript();
	ConOut("[DONE]\n");

	ConOut("Initializing races... ");
	Race::parse();
	ConOut("[DONE]\n");

	ConOut("Loading IP blocking rules... ");
	Network->LoadHosts_deny();
	ConOut("[DONE]\n");
	item_char_test(); //LB
	Guilds->CheckConsistancy(); // LB


	ConOut("\n");


	ConOut("\n");


	clearscreen();

	ConOut("%s %s %s.%s [%s] compiled by %s\nProgrammed by: %s", PRODUCT, VER, VERNUMB, HOTFIX, OS, NAME, PROGRAMMERS);
	ConOut("\nBased on Wolfpack 12.5b1 (http://www.wpdev.org)");
	ConOut("\nWeb-site : http://noxwizard.sourceforge.net/\n");
	ConOut("\n");
	ConOut("Copyright (C) 1997, 98 Marcus Rating (Cironian)\n\n");
	ConOut("This program is free software; you can redistribute it and/or modify\n");
	ConOut("it under the terms of the GNU General Public License as published by\n");
	ConOut("the Free Software Foundation; either version 2 of the License, or\n");
	ConOut("(at your option) any later version.\n\n");
	ConOut("Uses a modified version of scripting technologies from ITB CompuPhase\n");
	ConOut("see http://www.compuphase.com/small.htm for further details\n\n");
	ConOut("Running on %s\n", getOSVersionString());

////////////////////////////////////////////////////////////////////////////////////////////////////
// Removed by Dupois July 18, 2000!
// - Crashes server during if '#' - Save World option is used and corrupts the world file
//	quite nicely I might add. safer to just use the old checkkey() call in the main's while loop.
//InitKbThread();
////////////////////////////////////////////////////////////////////////////////////////////////////

	//ConOut("NoX-Wizard: Startup Complete.\n\n");


	if (SrvParms->server_log)
		ServerLog.Write("-=Server Startup=-\n=======================================================================\n");

	uiCurrentTime=getclock();
	serverstarttime=getclock(); // dont remove, its absolutly necassairy that its 3 times in the startup sequence for several timing reasons.

	CIAO_IF_ERROR;

	ConOut("\nMap size : %dx%d", MapTileWidth, MapTileHeight);

	if ((MapTileWidth==768)&&(MapTileHeight==512)) ConOut(" [standard Britannia/Sosaria map size]\n");
	else if ((MapTileWidth==288)&&(MapTileHeight==200)) ConOut(" [standard Ilshenar map size]\n");
	else ConOut(" [custom map size]\n");

	if (ServerScp::g_nAutoDetectIP==1)  {
		ConOut("\nServer waiting connections on all interfaces at TCP port %i\n", g_nMainTCPPort);
	} else {
		ConOut("\nServer waiting connections at IP %s, TCP port %i\n", serv[0][1], g_nMainTCPPort);
	}
	//	ConOut("\nServer waiting connections on all interfaces at TCP port %s\n", serv[0][2]);

    // print allowed clients
    std::string t;
    std::vector<std::string>::const_iterator vis( clientsAllowed.begin() ), vis_end( clientsAllowed.end() );

    ConOut("\nAllowed clients : ");
    for ( ; vis != vis_end;  ++vis)
    {
    t = (*vis);  // a bit pervert to store c++ strings and operate with c strings, admitably

	  if ( t == "SERVER_DEFAULT" )
	  {
		  ConOut("%s : %s\n", t.c_str(), SUPPORTED_CLIENT);
		  break;
	  }
	  else if ( t == "ALL" )
	  {
		  ConOut("ALL\n");
		  break;
	  }

	  ConOut("%s,", t.c_str());
    }
    ConOut("\n");

	initSignalHandlers();


#ifdef _WINDOWS
	updateMenus();
#endif
	if (ServerScp::g_nLoadDebugger) {
		SDbgOut("                         -=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		SDbgOut("                         ||  RUNNING IN DEBUG MODE  ||\n");
		SDbgOut("                         -=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	}

	if (ServerScp::g_nCheckBySmall) {
		SDbgOut("                         -=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		SDbgOut("                         ||  RUNNING IN CHECK MODE  ||\n");
		SDbgOut("                         -=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	}

#ifdef EXTREMELY_UNTESTED
	if (!ServerScp::g_nLoadDebugger) {
		SDbgOut("\nWARNING!\n");
		SDbgOut("This version needs much much tests. This should be considered a test only\n");
		SDbgOut("release and should *NOT* be used on production shards. \n");
		SDbgOut("Please test it deeply [with XWolf if you can] and report bugs on noxwizard \n");
		SDbgOut("forums. \n\n");
	}
#endif

	pointers::init(); //Luxor


	checkGarbageCollect();

	if( ServerScp::g_nCheckBySmall ) {
		InfoOut("Check of all object started...");

		AmxFunction checkItems( "__check_Item" );
		AmxFunction checkNpcs( "__check_Npc" );
		AmxFunction checkPlayers( "__check_Player" );

		cAllObjectsIter objs;
		P_CHAR pc = NULL;
		P_ITEM pi = NULL;
		for( objs.rewind(); !objs.IsEmpty(); objs++ ) {
			if ( isCharSerial( objs.getSerial() ) && ISVALIDPC( ( pc=static_cast<P_CHAR>(objs.getObject())) ) ) {
				if( pc->npc )
					checkNpcs.Call( pc->getSerial32() );
				else
					checkPlayers.Call( pc->getSerial32() );
			}
			else if ( isItemSerial( objs.getSerial() ) && ISVALIDPI( ( pi=static_cast<P_ITEM>(objs.getObject())) ) ) {
				checkItems.Call( pi->getSerial32() );
			}
		}

		ConOut("[DONE]\n");
		ServerScp::g_nCheckBySmall = false;

		cwmWorldState->saveNewWorld();

		ConOut("Closing sockets...");
		Network->SockClose();
		ConOut("Saving server.cfg...\n");
		saveserverscript();
		ConOut("[DONE]\n");
		ConOut("Deleting Classes...");
		DeleteClasses();
		ConOut("[DONE]\n");
		return 0;
	}

	InfoOut("Server started\n");

	spawns::doSpawnAll();

	//OnStart
	AMXEXEC(AMXT_SPECIALS,0,0,AMX_AFTER);
	g_bInMainCycle = true;
	while (keeprun)
	{

		keeprun = (!pollCloseRequests());
		// Uncomment by Dupois July 18, 2000! see note above about InitKbThread()
		//g_MenuCollector.cleanup();
		checkkey();
		checkTimer();
		checkCronTab();
		//OnLoop
		AMXEXEC(AMXT_SPECIALS,2,0,AMX_AFTER);

		switch(speed.nice)
		{
			case 0: break;	// very unnice - hog all cpu time
			case 1: if (now!=0) Sleep(10); else Sleep(100); break;
			case 2: Sleep(10); break;
			case 3: Sleep(40); break;// very nice
			case 4: if (now!=0) Sleep(10); else Sleep(400); break; // anti busy waiting
			case 5: if (now!=0) Sleep(40); else Sleep(500); break;

			default: Sleep(10); break;
		}


#ifdef WIN32	//xan : cpu cooling under win9x/me (hlt op not supported in ring3 on winnt)
	#if ( !defined __BORLANDC__ || __BORLANDC__ >= 0x0560 ) && !defined __GNUC__
		// Akron: borland c++ builder 5 have asm problems
		// also MingW32 can't compile it
		if ((ServerScp::g_nCPUCooling!=0)&&(!(getOSVersion()==OSVER_WINNT))) {
			for (i=0; i<ServerScp::g_nCPUCooling; i++)
				__asm hlt
		}
	#endif
#endif


		if(loopTimeCount >= 1000)
		{
			loopTimeCount = 0;
			loopTime = 0;
		}
		loopTimeCount++;

		StartMilliTimer(loopSecs, loopMilli);
                //testAI();
		if(networkTimeCount >= 1000)
		{
			networkTimeCount = 0;
			networkTime = 0;
		}

		StartMilliTimer(tempSecs, tempMilli);

		if ( TIMEOUT( CheckClientIdle ) )
		{
			CheckClientIdle=((SrvParms->inactivitytimeout/2)*MY_CLOCKS_PER_SEC)+uiCurrentTime;

			for (r=0;r<now;r++)
			{
				P_CHAR pc_r=MAKE_CHAR_REF(currchar[r]);
				if(!ISVALIDPC(pc_r))
					continue;
				if (!pc_r->IsGM()
					&& pc_r->clientidletime<uiCurrentTime
					&& perm[r]
					)
				{
					ConOut("Player %s disconnected due to inactivity !\n", pc_r->getCurrentNameC());
					//sysmessage(r,"you have been idle for too long and have been disconnected!");
					char msg[3];
					msg[0]=0x53;
					msg[1]=0x07;
					Xsend(r, msg, 2);
					Network->Disconnect(r);
				}

			}
		}
		if( TIMEOUT( uiNextCheckConn ) ) // Cut lag on CheckConn by not doing it EVERY loop.
		{
			Network->CheckConn();
			TelnetInterface.CheckConn();
			uiNextCheckConn = (unsigned int)( uiCurrentTime + 250 );
		}

		Network->CheckMessage();
		TelnetInterface.CheckInp();

		tempTime = CheckMilliTimer(tempSecs, tempMilli);

		networkTime += tempTime;
		networkTimeCount++;

		if(timerTimeCount >= 1000)
		{
			timerTimeCount = 0;
			timerTime = 0;
		}

		StartMilliTimer(tempSecs, tempMilli);

		checktimers();

		uiCurrentTime=getclock();//getclock() only once
		tempTime = CheckMilliTimer(tempSecs, tempMilli);
		timerTime += tempTime;
		timerTimeCount++;

		if(autoTimeCount >= 1000)
		{
			autoTimeCount = 0;
			autoTime = 0;
		}
		StartMilliTimer(tempSecs, tempMilli);

		checkauto();

		tempTime = CheckMilliTimer(tempSecs, tempMilli);
		autoTime += tempTime;
		autoTimeCount++;

		Network->ClearBuffers();
		tempTime = CheckMilliTimer(loopSecs, loopMilli);
		loopTime += tempTime;

	}

	AMXEXEC(AMXT_SPECIALS,1,0,AMX_AFTER);

	cwmWorldState->saveNewWorld();

	sysbroadcast(TRANSLATE("The server is shutting down."));
	if (SrvParms->html>0)
	{
		ConOut("Writing offline HTML page...");
		offlinehtml();//HTML	// lb, the if prevents a crash on shutdown if html deactivated ...
		ConOut(" Done.\n");
	}
	ConOut("Closing sockets...");

	Network->SockClose();

	gcollect();		// cleanup before saving, especially items of deleted chars (Duke, 10.1.2001)

	ConOut(" Done.\n");
	ConOut("Saving server.cfg...\n");
	//saveserverscript(1);
	saveserverscript();
	ConOut("\n");
	ConOut("Deleting Classes...");
	DeleteClasses();
	ConOut("[DONE]\n");

	if (NewErrorsLogged())
		ConOut("New ERRORS have been logged. Please send the logs/error*.log and logs/critical*.log files to the dev team !\n");
	if (NewWarningsLogged())
		ConOut("New WARNINGS have been logged. Probably scripting errors. See the logs/warnings*.log for details !\n");

	if (error) {
		ConOut("ERROR: Server terminated by error!\n");

		if (SrvParms->server_log)
			ServerLog.Write("Server Shutdown by Error!\n=======================================================================\n\n\n");

	} else {
		ConOut("NoX-Wizard: Server shutdown complete!\n");
		if (SrvParms->server_log)
			ServerLog.Write("Server Shutdown!\n=======================================================================\n\n\n");

	}
	return 0;
}

// clock() is supposed to return CPU time used - it doesn't on Windows, but
// does on Linux. Thus it doesn't go up on Linux, and time seems to stand
// still. This function emulates clock()

void initclock()
{
#if defined(__unix__)
	timeval t ;
	gettimeofday(&t,NULL) ; // ftime has been obseloated
	initialserversec = t.tv_sec ;
	initialservermill = t.tv_usec/ 1000 ;
#else
	timeb t ;
	::ftime(&t) ;
	initialserversec = t.time ;
	initialservermill = t.millitm ;
#endif
}

void telltime( NXWSOCKET  s )
{
	char tstring[60];
	char tstring2[60];
	int hour = Calendar::g_nHour % 12;
	if (hour==0) hour = 12;
	int ampm = (Calendar::g_nHour>=12) ? 1 : 0;
	int minute = Calendar::g_nMinute;
	int lhour;
	lhour=hour;

	if ((minute>=0)&&(minute<=14)) strcpy(tstring,TRANSLATE("It is"));
	else if ((minute>=15)&&(minute<=30)) strcpy(tstring,TRANSLATE("It is a quarter past"));
	else if ((minute>=30)&&(minute<=45)) strcpy(tstring,TRANSLATE("It is half past"));
	else
	{
		strcpy(tstring,TRANSLATE("It is a quarter till"));
		lhour++;
		if (lhour==0) lhour=12;
	}
	switch( lhour )
	{
	case 1: sprintf( tstring2, TRANSLATE("%s one o'clock"), tstring );	   break;
	case 2: sprintf( tstring2, TRANSLATE("%s two o'clock"), tstring );	   break;
	case 3: sprintf( tstring2, TRANSLATE("%s three o'clock"), tstring );	   break;
	case 4: sprintf( tstring2, TRANSLATE("%s four o'clock"), tstring );	   break;
	case 5: sprintf( tstring2, TRANSLATE("%s five o'clock"), tstring );	   break;
	case 6: sprintf( tstring2, TRANSLATE("%s six o'clock"), tstring );	   break;
	case 7: sprintf( tstring2, TRANSLATE("%s seven o'clock"), tstring );	   break;
	case 8: sprintf( tstring2, TRANSLATE("%s eight o'clock"), tstring );	   break;
	case 9: sprintf( tstring2, TRANSLATE("%s nine o'clock"), tstring );	   break;
	case 10: sprintf( tstring2, TRANSLATE("%s ten o'clock"), tstring );	   break;
	case 11: sprintf( tstring2, TRANSLATE("%s eleven o'clock"), tstring );	   break;
	case 12:
		if( ampm )
			sprintf( tstring2, TRANSLATE("%s midnight."), tstring );
		else
			sprintf( tstring2, TRANSLATE("%s noon."), tstring );
		break;
	}

	if (lhour==12) strcpy(tstring, tstring2);
	else if (ampm)
	{
		if ((lhour>=1)&&(lhour<6)) sprintf(tstring,TRANSLATE("%s in the afternoon."),tstring2);
		else if ((lhour>=6)&&(lhour<9)) sprintf(tstring,TRANSLATE("%s in the evening."),tstring2);
		else sprintf(tstring,TRANSLATE("%s at night."),tstring2);
	}
	else
	{
		if ((lhour>=1)&&(lhour<5)) sprintf(tstring,TRANSLATE("%s at night."),tstring2);
		else sprintf(tstring,TRANSLATE("%s in the morning."),tstring2);
	}

	sysmessage(s,tstring);
}

void impaction(int s, int act)
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC( pc );
	if ( pc->isMounting() && (act==0x10 || act==0x11))
	{
		pc->playAction(0x1b);
		return;
	}
	if ( pc->isMounting() || ( pc->GetBodyType() < 0x190 ) && act == 0x22 )
		return;
	pc->playAction(act);
}

/*!
\brief direction from character a to position x,y
\author ?
\return int direction
\param a first character
\param x X-coordinate
\param y Y-coordinate
*/
int chardirxyz(int a, int x, int y)
{
	P_CHAR pc = MAKE_CHAR_REF( a );
	VALIDATEPCR( pc, INVALID );

	int dir,xdif,ydif;

	xdif = x - pc->getPosition().x;
	ydif = y - pc->getPosition().y;

	if ((xdif==0)&&(ydif<0)) dir=0;
	else if ((xdif>0)&&(ydif<0)) dir=1;
	else if ((xdif>0)&&(ydif==0)) dir=2;
	else if ((xdif>0)&&(ydif>0)) dir=3;
	else if ((xdif==0)&&(ydif>0)) dir=4;
	else if ((xdif<0)&&(ydif>0)) dir=5;
	else if ((xdif<0)&&(ydif==0)) dir=6;
	else if ((xdif<0)&&(ydif<0)) dir=7;
	else dir=INVALID;

	return dir;
}


int fielddir(CHARACTER s, int x, int y, int z)
{
//WARNING: unreferenced formal parameter z

	P_CHAR pc=MAKE_CHAR_REF(s);
	VALIDATEPCR(pc,0);

	int dir=chardirxyz(s, x, y);
	switch (dir)
	{
	case 0:
	case 4:
		return 0;
	case 2:
	case 6:
		return 1;
	case 1:
	case 3:
	case 5:
	case 7:
	case INVALID:
		switch(pc->dir) //crashfix, LB
		{
		case 0:
		case 4:
			return 0;

		case 2:
		case 6:
			return 1;

		case 1:
		case 3:
		case 5:
		case 7:
			return 1;

		default:
			LogError("Switch fallout. NoX-Wizard.cpp, fielddir()\n"); //Morrolan
			return 0;
		}
	default:
		LogError("Switch fallout. NoX-Wizard.cpp, fielddir()\n"); //Morrolan
		return 0;
	}
}

/*!
\brief makes an npc attacking someone
\author Luxor
\param pc the npc attacker
\param pc_target the victim
*/
void npcattacktarget(P_CHAR pc, P_CHAR pc_target)
{
	VALIDATEPC(pc);
	VALIDATEPC(pc_target);

	if ( !pc->npc )
		return;

	if ( pc->dead || pc_target->dead )
		return;

	if ( pc->getSerial32() == pc_target->getSerial32() )
		return;

	if ( !pc->losFrom(pc_target) )
		return;

	if( pc->amxevents[ EVENT_CHR_ONBEGINATTACK ]!=NULL ) {
		pc->amxevents[ EVENT_CHR_ONBEGINATTACK ]->Call( pc->getSerial32(), pc_target->getSerial32() );
		if (g_bByPass==true)
			return;
	}
	/*
	pc->runAmxEvent( EVENT_CHR_ONBEGINATTACK, pc->getSerial32(), pc_target->getSerial32() );
	if (g_bByPass==true)
		return;
	*/
	if( pc->amxevents[ EVENT_CHR_ONBEGINDEFENSE ]!=NULL ) {
		pc->amxevents[ EVENT_CHR_ONBEGINDEFENSE ]->Call( pc_target->getSerial32(), pc->getSerial32() );
		if (g_bByPass==true)
			return;
	}
	/*
	pc->runAmxEvent( EVENT_CHR_ONBEGINDEFENSE, pc_target->getSerial32(), pc->getSerial32() );
	if (g_bByPass==true)
		return;
	*/

	pc->playMonsterSound(SND_STARTATTACK);

	pc->targserial = pc_target->getSerial32();
	pc->attackerserial = pc_target->getSerial32();
	pc->SetAttackFirst();

	if ( !pc->war )
		pc->toggleCombat();
	pc->setNpcMoveTime();

	P_CHAR pc_target_targ = pointers::findCharBySerial(pc_target->targserial);
	if ( !ISVALIDPC(pc_target_targ) || pc_target_targ->dead || pc_target->distFrom(pc_target_targ) > 15 ) {
		if (!pc_target->npc && pc_target->war) {
			pc_target->targserial = pc->getSerial32();
			pc_target->attackerserial = pc->getSerial32();
		} else if (pc_target->npc) {
			if ( !pc_target->war )
				pc_target->toggleCombat();

			pc_target->targserial = pc->getSerial32();
			pc_target->attackerserial = pc->getSerial32();
			pc_target->setNpcMoveTime();
		}
	}

	pc->emoteall( "You see %s attacking %s!", 1, pc->getCurrentNameC(), pc_target->getCurrentNameC() );

	return;
}


/*!
\brief Initilizes the gmpages[] and counspages[] arrays and also jails
\author ?
*/
void initque()
{
	int i;
	for(i=1;i<MAXPAGES;i++)
	{
		gmpages[i].name[0] = '\0';
		gmpages[i].reason[0] = '\0';
		gmpages[i].serial.ser1='\x00';
		gmpages[i].serial.ser2='\x00';
		gmpages[i].serial.ser3='\x00';
		gmpages[i].serial.ser4='\x00';
		gmpages[i].timeofcall[0]=0;
		gmpages[i].handled=1;
	}
	for(i=1;i<MAXPAGES;i++)
	{
		counspages[i].name[0] = '\0';
		counspages[i].reason[0] = '\0';
		counspages[i].serial.ser1='\x00';
		counspages[i].serial.ser2='\x00';
		counspages[i].serial.ser3='\x00';
		counspages[i].serial.ser4='\x00';
		counspages[i].timeofcall[0]=0;
		counspages[i].handled=1;
	}

//Sabrewulf: scriptable jails locations
//jails are the locations 1-10

	int jailx, jaily, jailz;

	for (int loc = 1; loc <= 10; loc ++)
	{
		jailx = jaily = jailz =INVALID;

		location2xyz(loc, jailx, jaily, jailz);
		if (jailx < 0 || jaily < 0 || jailz < 0) //if location is not found
			prison::standardJailxyz (loc, jailx, jaily, jailz); //uses the standard jails

		prison::addCell( loc, jailx, jaily, jailz );
	}

}


/*!
\author Unknown, rewritten by Luxor
\brief add gold to char handled by socket
\param s socket
\param totgold amount of gold to add
*/
void addgold(int s, int totgold)
{
	if ( s < 0 || s >= now )
		return;

	P_CHAR pc = pointers::findCharBySerial( currchar[s] );
	P_ITEM pi = item::CreateFromScript( "$item_gold_coin", pc->getBackpack() );
	if ( ISVALIDPI( pi ) ) {
		pi->setAmount( totgold );
		pi->Refresh();
	}
}

void usepotion(P_CHAR pc, P_ITEM pi)
{
	int x;

	VALIDATEPC(pc);

	NXWSOCKET s = pc->getSocket();

	switch(pi->morey)
	{
	case 1: // Agility Potion
		pc->staticFX(0x373a, 0, 15);
		switch(pi->morez)
		{
		case 1:
			tempfx::add(pc, pc, tempfx::SPELL_AGILITY, 5+RandomNum(1,10), 0, 0, 120);
			pc->sysmsg(TRANSLATE("You feel more agile!"));
			break;
		case 2:
			tempfx::add(pc, pc, tempfx::SPELL_AGILITY, 10+RandomNum(1,20), 0, 0, 120);
			pc->sysmsg(TRANSLATE("You feel much more agile!"));
			break;
		default:
			ErrOut("Switch fallout. NoX-Wizard.cpp, usepotion()\n");
			return;
		}
		pc->playSFX(0x01E7);
		if (s!=INVALID)
			pc->updateStats(2);
		break;

	case 2: // Cure Potion
		if (pc->poisoned<1)
			pc->sysmsg(TRANSLATE("The potion had no effect."));
		else
		{
			switch(pi->morez)
			{
			case 1:
				x=RandomNum(1,100);
				if (pc->poisoned==POISON_WEAK && x<81) pc->poisoned=POISON_NONE;
				if (pc->poisoned==POISON_NORMAL && x<41) pc->poisoned=POISON_NONE;
				if (pc->poisoned==POISON_GREATER && x<21) pc->poisoned=POISON_NONE;
				if (pc->poisoned==POISON_DEADLY && x< 6) pc->poisoned=POISON_NONE;
				break;
			case 2:
				x=RandomNum(1,100);
				if (pc->poisoned==POISON_WEAK) pc->poisoned=POISON_NONE;
				if (pc->poisoned==POISON_NORMAL && x<81) pc->poisoned=POISON_NONE;
				if (pc->poisoned==POISON_GREATER && x<41) pc->poisoned=POISON_NONE;
				if (pc->poisoned==POISON_DEADLY && x<21) pc->poisoned=POISON_NONE;
				break;
			case 3:
				x=RandomNum(1,100);
				if (pc->poisoned==POISON_WEAK) pc->poisoned=POISON_NONE;
				if (pc->poisoned==POISON_NORMAL) pc->poisoned=POISON_NONE;
				if (pc->poisoned==POISON_GREATER && x<81) pc->poisoned=POISON_NONE;
				if (pc->poisoned==POISON_DEADLY && x<61) pc->poisoned=POISON_NONE;
				break;
			default:
				ErrOut("Switch fallout. NoX-Wizard.cpp, usepotion()\n"); //Morrolan
				return;
			}
			if (pc->poisoned)
				pc->sysmsg(TRANSLATE("The potion was not able to cure this poison."));
			else
			{
				pc->staticFX(0x373A, 0, 15);
				pc->playSFX( 0x01E0); //cure sound - SpaceDog
				pc->sysmsg(TRANSLATE("The poison was cured."));
			}
		}
		impowncreate(s,pc,1); //Lb, makes the green bar blue or the blue bar blue !
		break;

	case 3: // Explosion Potion
		if (region[pc->region].priv&0x01) // Ripper 11-14-99
		{
			pc->sysmsg(TRANSLATE(" You cant use that in town!"));
			return;
		}
		addid1[s]= pi->getSerial().ser1;
		addid2[s]= pi->getSerial().ser2;
		addid3[s]= pi->getSerial().ser3;
		addid4[s]= pi->getSerial().ser4;
		pc->sysmsg(TRANSLATE("Now would be a good time to throw it!"));
		tempfx::add(pc, pc, tempfx::EXPLOTIONMSG, 0, 1, 3);
		tempfx::add(pc, pc, tempfx::EXPLOTIONMSG, 0, 2, 2);
		tempfx::add(pc, pc, tempfx::EXPLOTIONMSG, 0, 3, 1);
		tempfx::add(pc, pi, tempfx::EXPLOTIONEXP, 0, 4, 0);
		target(s,0,1,0,207,"*throw*");
		return; // lb bugfix, break is wronh here because it would delete bottle

	case 4: // Heal Potion
		switch(pi->morez)
		{
		case 1:
			pc->hp=qmin(pc->hp+5+RandomNum(1,5)+pc->skill[17]/100,pc->getStrength());
			pc->sysmsg(TRANSLATE("You feel better!"));
			break;
		case 2:
			pc->hp=qmin(pc->hp+15+RandomNum(1,10)+pc->skill[17]/50,pc->getStrength());
			pc->sysmsg(TRANSLATE("You feel more healty!"));
			break;
		case 3:
			pc->hp=qmin(pc->hp+20+RandomNum(1,20)+pc->skill[17]/40, pc->getStrength());
			pc->sysmsg(TRANSLATE("You feel much more healty!"));
			break;

		default:
			ErrOut("Switch fallout. NoX-Wizard.cpp, usepotion()\n"); //Morrolan
			return;
		}

		if (s!=INVALID)
			pc->updateStats(0);

		pc->staticFX(0x376A, 9, 6); // Sparkle effect
		pc->playSFX(0x01F2); //Healing Sound - SpaceDog
		break;

	case 5: // Night Sight Potion
		pc->staticFX(0x376A, 9, 6); // Sparkle effect
		tempfx::add(pc, pc, tempfx::SPELL_LIGHT, 0, 0, 0,(720*secondsperuominute*MY_CLOCKS_PER_SEC));
		pc->playSFX(0x01E3);
		break;

	case 6: // Poison Potion
		if(pc->poisoned < (PoisonType)pi->morez)
			pc->poisoned=(PoisonType)pi->morez;
		if(pi->morez>4)
			pi->morez=4;
		pc->poisonwearofftime=uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParms->poisontimer); // lb, poison wear off timer setting
		impowncreate(s,pc,1); //Lb, sends the green bar !
		pc->playSFX(0x0246); //poison sound - SpaceDog
		pc->sysmsg(TRANSLATE("You poisoned yourself! *sigh*")); //message -SpaceDog
		break;

	case 7: // Refresh Potion
		switch(pi->morez)
		{
			case 1:
				pc->stm=qmin(pc->stm+20+RandomNum(1,10), pc->dx);
				pc->sysmsg(TRANSLATE("You feel more energetic!"));
				break;

			case 2:
				pc->stm=qmin(pc->stm+40+RandomNum(1,30), pc->dx);
				pc->sysmsg(TRANSLATE("You feel much more energetic!"));
				break;

			default:
				ErrOut("Switch fallout. NoX-Wizard.cpp, usepotion()\n"); //Morrolan
				return;
		}
		if (s!=INVALID)
			pc->updateStats(2);
		pc->staticFX(0x376A, 9, 6); // Sparkle effect
		pc->playSFX(0x01F2); //Healing Sound
		break;

	case 8: // Strength Potion
		pc->staticFX(0x373A, 0, 15); // Sparkle effect
		switch(pi->morez)
		{
		case 1:
			tempfx::add(pc, pc, tempfx::SPELL_STRENGHT, 5+RandomNum(1,10), 0, 0, 120);
			pc->sysmsg(TRANSLATE("You feel more strong!"));
			break;
		case 2:
			tempfx::add(pc, pc, tempfx::SPELL_STRENGHT, 10+RandomNum(1,20), 0, 0, 120);
			pc->sysmsg(TRANSLATE("You feel much more strong!"));
			break;
		default:
			ErrOut("Switch fallout. NoX-Wizard.cpp, usepotion()\n");
			return;
		}
		pc->playSFX(0x01EE);
		break;

	case 9: // Mana Potion
		switch(pi->morez)
		{
		case 1:
			pc->mn=qmin(pc->mn+10+pi->morex/100, (unsigned)pc->in);
			break;

		case 2:
			pc->mn=qmin(pc->mn+20+pi->morex/50, (unsigned)pc->in);
			break;

		default:
			ErrOut("Switch fallout. NoX-Wizard.cpp, usepotion()\n");
			return;
		}
		if (s!=INVALID)
			pc->updateStats(1);
		pc->staticFX(0x376A, 9, 6); // Sparkle effect
		pc->playSFX(0x01E7); //agility sound - SpaceDog
		break;

	case 10: //LB's LSD potion, 5'th november 1999
		if (pi->id()!=0x1841) return; // only works with an special flask
		if (s==INVALID) return;
		if (LSD[s]==1)
		{
			pc->sysmsg(TRANSLATE("no,no,no,cant you get enough ?"));
			return;
		}
		tempfx::add(pc, pc, tempfx::LSD, 60+RandomNum(1,120), 0, 0); // trigger effect
		pc->staticFX(0x376A, 9, 6); // Sparkle effect
		pc->playSFX(0x00F8, true); // lsd sound :)
		break;

	default:
		ErrOut("Switch fallout. NoX-Wizard.cpp, usepotion()\n"); //Morrolan
		return;
	}

	pc->playSFX(0x0030);
	if (pc->HasHumanBody() && !pc->isMounting())
		pc->playAction(0x22);

	pi->ReduceAmount( 1 );

	if (pi->morey!=3)
	{
		int lsd=pi->morey; // save morey before overwritten

		pi = archive::item::New();

		pi->setId(0x0F0E);

		if (lsd==10) // empty Lsd potions
		{
			pi->setId(0x183d);
		}

		pi->pileable=1;

		P_ITEM pack=pc->getBackpack();
		if (ISVALIDPI(pack)) {
			pack->AddItem( pi );
		}
		else {
			pi->MoveTo( pc->getPosition() );
			pi->setDecay();
			pi->Refresh();
		}
	}
}

void StoreItemRandomValue(P_ITEM pi,int tmpreg)
{ // Function Created by Magius(CHE) for trade System

	int max=0,min=0;

	VALIDATEPI(pi);

	if (pi->good<0) return;

	if (tmpreg<0)
	{
		P_ITEM pio=pi->getOutMostCont();
		if (pio->isInWorld())
			tmpreg=calcRegionFromXY( pio->getPosition() );
		else
		{
			P_CHAR pc=pointers::findCharBySerial(pio->getContSerial());
			if (!pc) return;
			tmpreg=calcRegionFromXY( pc->getPosition() );
		}
	}

	if (tmpreg<0 || tmpreg>255 || pi->good<0 || pi->good>255) return;

	min=region[tmpreg].goodrnd1[pi->good];
	max=region[tmpreg].goodrnd2[pi->good];

	if (max!=0 || min!=0)
		pi->rndvaluerate=(int) RandomNum(min,max);
}



/*!
\brief Load metaGM permission

Load metaGM permission from metagm.scp script and add them to metagm array used
for makeGM, makeCNS and SetPriv3

\author Lord Binary Ummon
\remark  If two different command has same permission bit
		\li in plus mode:
			they are both added if at least one is listed
		\li in minus mode:
		they are both removed if at least one is listed
*/
void loadmetagm()
{
	char sect[512];

	int i; //section counter
	int n; //for skipping command search until after mode token has been found
	int totalsection=0; //total number of sections less the first foundamental three (0,1,2)
	int plus=1;
	int plusmode;
/*	int cmd_cnt;
	int cmd_found;*/
	P_COMMAND cmd;
    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];

	for (i=0;i<256;i++) //for each of 256 possible "command clearance" section
	{
		//for each of 7 section of priv3, each expressed as an integer (4 byte), so 32 bits per section.
		for (int a=0;a<7;a++) metagm[i][a]=0;
	}

	i=INVALID;
	totalsection=0;
	do //look for every possible "command clearance" section
	{
		i++;
		sprintf(sect, "SECTION COMMAND_CLEARANCE %i", i);
		safedelete(iter);
		iter = Scripts::MetaGM->getNewIterator(sect);

		if (iter!=NULL)
		{
			totalsection++;
			n=0;
			plusmode=INVALID;
			int loopexit=0;
			do //look into section
			{
				iter->parseLine(script1, script2);
				if ((script1[0]!='}')&&(script1[0]!='{'))
				{
					if (!(strcmp("MODE+",script1))) { n=1; plusmode=1;} // plus mode
					if (!(strcmp("MODE-",script1))) { n=1; plusmode=0;}	// minus mode

					if (plusmode>INVALID && n==0) // only check for command words AFTER the mode token
					{
						//cmd_cnt=0;
						//cmd_found=INVALID;
//						int loopexit=0; // unused variable

						/*while((command_table[cmd_cnt].cmd_name) && (cmd_found == INVALID) && (++loopexit < MAXLOOPS) ) // search for the command
						{
							if(!(strcmp(command_table[cmd_cnt].cmd_name, script1))) cmd_found=cmd_cnt;
							cmd_cnt++;
						}*/

						cmd= commands->findCommand(script1);

						if (cmd==NULL) // not found ?
						{
							WarnOut("found unknown command %s in meta gm script\n",script1);
						}
						else // found it!
						{
							if (cmd->cmd_priv_m!=255)
								metagm[i][cmd->cmd_priv_m]=(metagm[i][cmd->cmd_priv_m] | (plus << (cmd->cmd_priv_b)));
						}
					}
					n=0; //if mode token was found then parse commands
				}
			} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

			if (plusmode==INVALID) //mode token not found
			{
				error=1;
				keeprun=false;
				WarnOut("\n Meta-Gm script parsing error, mode keyword missing, section# %i - closing NoX-Wizard\n",i);
			}

			if (plusmode==0) //if minus mode, reverse every permission.
				for (int aa=0; aa<7; aa++)
					metagm[i][aa]=~metagm[i][aa];
		}

	} while (i<255);

	safedelete(iter);
//	ConOut("Meta Gm script loaded... %i priv3 shortcut/s\n",totalsection-3);
}

void StartMilliTimer(unsigned long &Seconds, unsigned long &Milliseconds)
{
#if defined(__unix__)
	timeval t ;
	gettimeofday(&t,NULL) ;
	Seconds = t.tv_sec ;
	Milliseconds = t.tv_usec/1000 ;
#else

	timeb t;
	::ftime( &t );
	Seconds = t.time;
	Milliseconds = t.millitm;
#endif
}


unsigned long CheckMilliTimer(unsigned long &Seconds, unsigned long &Milliseconds)
{
	unsigned long newSec ;
	unsigned long newMill ;

	#ifdef __unix__
	timeval t ;
	gettimeofday(&t,NULL) ;
	newSec = t.tv_sec ;
	newMill = t.tv_usec/1000 ;

	#else
	struct timeb t;
	::ftime( &t );
	newSec = t.time ;
	newMill = t.millitm ;

	#endif
	return( 1000 * ( newSec - Seconds ) + ( newMill - Milliseconds ) );
}


/*!
\brief initialise creatures

assigns the basesound, soundflag, who_am_i flag of all npcs

\date 09/08/1999 creation

soundflags	0: normal, 5 sounds (attack-started, idle, attack, defence, dying, see nxwcommn.h)
				1: birds .. only one "bird-shape" and zillions of sounds ...
				2: only 3 sounds -> (attack,defence,dying)
				3: only 4 sounds ->	(attack-started,attack,defnce,dying)
				4: only 1 sound !!

who_am_i bit # 1 creature can fly (must have the animations, so better dont change)
				# 2 anti-blink: these creatures dont have animation #4, if not set creaure will randomly dissapear in battle
								if you find a creature that blinks while fighting, set that bit
				# 3 animal-bit		(currently not used/set)
				# 4 water creatures (currently not used/set)
icon: used for tracking, to set the appropriate icon
*/
void init_creatures()
{

	memset(&creatures[0], 0, 2048*sizeof( creat_st ));

	creatures[0x01].basesound=0x01AB;							// Ogre
	creatures[0x01].icon=8415;

	creatures[0x02].basesound=0x016F;							// Ettin
	creatures[0x02].icon=8408;

	creatures[0x03].basesound=0x01D7;							// Zombie
	creatures[0x03].icon=8428;

	creatures[0x04].basesound=0x0174;							// Gargoyle
	creatures[0x04].who_am_i+=1; // set can_fly_bit
	creatures[0x04].icon=8409;

	creatures[0x05].basesound=0x008f;							// Eagle
	creatures[0x05].who_am_i+=1; // set can_fly bit
	creatures[0x05].who_am_i+=2; // set anti blink bit
	creatures[0x05].icon=8434;

	creatures[0x06].basesound=0x007d;							// Bird
	creatures[0x06].who_am_i+=1; //set fly bit
	creatures[0x06].soundflag=1; // birds need special treatment cause there are about 20 bird-sounds
	creatures[0x06].icon=8430;

	creatures[0x07].basesound=0x01b0;							// Orc
	creatures[0x07].icon=8416;

	creatures[0x08].basesound=0x01ba;							// corpser
	creatures[0x08].soundflag=3;
	creatures[0x08].icon=8402;


	creatures[0x09].basesound=0x0165;							// daemon
	creatures[0x09].who_am_i+=1;
	creatures[0x09].icon=8403;

	creatures[0x0a].basesound=0x0165;							// daemon 2
	creatures[0x0a].who_am_i+=1;
	creatures[0x0a].icon=8403;

	creatures[0x0c].basesound=362;								// Green dragon
	creatures[0x0c].who_am_i+=1; // flying creature
	creatures[0x0c].icon=8406;

	creatures[0x0d].basesound=263;								// air-ele
	creatures[0x0d].icon=8429;

	creatures[0x0e].basesound=268;								// earth-ele
	creatures[0x0e].icon=8407;

	creatures[0x0f].basesound=273;								// fire-ele
	creatures[0x0f].icon=8435;

	creatures[0x10].basesound=0x0116;							// water ele
	creatures[0x10].icon=8459;

	creatures[0x11].basesound=0x01b0;							// Orc	2
	creatures[0x11].icon=8416;

	creatures[0x12].basesound=0x016F;							// Ettin 2
	creatures[0x12].icon=8408;

	creatures[0x15].basesound=219;								// Giant snake
	creatures[0x15].icon=8446;

	creatures[0x16].basesound=377;								// gazer
	creatures[0x16].icon=8426;

	creatures[0x18].basesound=412;								// liche
	creatures[0x18].icon=8440; // counldnt find a better one :(

	creatures[0x1a].basesound=382;								// ghost 1
	creatures[0x1a].icon=8457;

	creatures[0x1c].basesound=387;								// giant spider
	creatures[0x1c].icon=8445;

	creatures[0x1d].basesound=158;								// gorialla
	creatures[0x1d].icon=8437;

	creatures[0x1e].basesound=402;								// harpy
	creatures[0x1e].icon=8412;

	creatures[0x1f].basesound=407;								// headless
	creatures[0x1f].icon=8458;

	creatures[0x21].basesound=417;								// lizardman
	creatures[0x23].basesound=417;
	creatures[0x24].basesound=417;
	creatures[0x25].basesound=417;
	creatures[0x26].basesound=417;
	creatures[0x21].icon=creatures[0x23].icon=
	creatures[0x24].icon=creatures[0x25].icon=
	creatures[0x26].icon=8414;

	creatures[0x27].basesound=422;								// mongbat
	creatures[0x27].who_am_i+=1; // yes, they can fly
	creatures[0x27].icon=8441;

	creatures[0x29].basesound=0x01b0;							// orc 3
	creatures[0x29].icon=8416;

	creatures[0x2a].basesound=437;								// ratman
	creatures[0x2c].basesound=437;
	creatures[0x2d].basesound=437;
	creatures[0x2a].icon=creatures[0x2c].icon=
	creatures[0x2d].icon=8419;

	creatures[0x2f].basesound=0x01ba;							// Reaper
	creatures[0x2f].icon=8442;

	creatures[0x30].basesound=397;								// giant scorprion
	creatures[0x30].icon=8420;

	creatures[0x32].basesound=452;								// skeleton 2
	creatures[0x32].icon=8423;

	creatures[0x33].basesound=456;								// slime
	creatures[0x33].icon=8424;

	creatures[0x34].basesound=219;								// Snake
	creatures[0x34].icon=8444;

	creatures[0x35].basesound=461;								// troll
	creatures[0x35].icon=8425;
	creatures[0x36].basesound=461;								// troll 2
	creatures[0x36].icon=8425;
	creatures[0x37].basesound=461;								// troll 3
	creatures[0x37].icon=8425;

	creatures[0x38].basesound=452;								// skeleton 3
	creatures[0x38].icon=8423;

	creatures[0x39].basesound=452;								// skeleton 4
	creatures[0x39].icon=8423;

	creatures[0x3a].basesound=466;								// wisp
	creatures[0x3a].icon=8448;

	creatures[0x3b].basesound=362;								// red dragon
	creatures[0x3b].who_am_i+=1; // set fly bit
	creatures[0x3c].basesound=362;								// smaller red dragon
	creatures[0x3c].who_am_i+=1;
	creatures[0x3d].basesound=362;								// smaller green dragon
	creatures[0x3d].who_am_i+=1;
	creatures[0x3b].icon=creatures[0x3c].icon=
	creatures[0x3d].icon=8406;

	creatures[0x96].basesound=477;								// sea serpant
	creatures[0x96].basesound=3;
	creatures[0x96].icon=8446; // normal serpant icon

	creatures[0x97].basesound=138;								// dolphin
	creatures[0x97].icon=8433; // correct icon ???

	creatures[0xc8].basesound=168;								// white horse
	creatures[0xc8].icon=8479;

	creatures[0xc9].basesound=105;								// cat
	creatures[0xc9].who_am_i+=2; // set blink flag
	creatures[0xc9].icon=8475;

	creatures[0xca].basesound=90;								// alligator
	creatures[0xca].icon=8410;

	creatures[0xcb].basesound=196;								// small pig
	creatures[0xcb].icon=8449;

	creatures[0xcc].basesound=168;								// brown horse
	creatures[0xcc].icon=8481;

	creatures[0xcd].basesound=201;								// rabbit
	creatures[0xcd].soundflag=2;								// rabbits only have 3 sounds, thus need special treatment
	creatures[0xcd].icon=8485;

	creatures[0xcf].basesound=214;								// wooly sheep
	creatures[0xcf].icon=8427;

	creatures[0xd0].basesound=110;								// chicken
	creatures[0xd0].icon=8401;

	creatures[0xd1].basesound=153;								// goat
	creatures[0xd1].icon=8422; // theres no goat icon, so i took a (differnt) sheep

	creatures[0xd3].basesound=95;								// brown bear
	creatures[0xd3].icon=8399;

	creatures[0xd4].basesound=95;								// grizzly bear
	creatures[0xd4].icon=8411;

	creatures[0xd5].basesound=95;								// polar bear
	creatures[0xd5].icon=8417;

	creatures[0xd6].basesound=186;								// panther
	creatures[0xd6].who_am_i+=2;
	creatures[0xd6].icon=8473;

	creatures[0xd7].basesound=392;								// giant rat
	creatures[0xd7].icon=8400;

	creatures[0xd8].basesound=120;								// cow
	creatures[0xd8].icon=8432;

	creatures[0xd9].basesound=133;								// dog
	creatures[0xd9].icon=8405;

	creatures[0xdc].basesound=183;								// llama
	creatures[0xdc].soundflag=2;
	creatures[0xdc].icon=8438;

	creatures[0xdd].basesound=224;								// walrus
	creatures[0xdd].icon=8447;

	creatures[0xdf].basesound=216;								// lamb/shorn sheep
	creatures[0xdf].soundflag=2;
	creatures[0xdf].icon=8422;

	creatures[0xe1].basesound=229;								// jackal
	creatures[0xe1].who_am_i+=2; // set anti blink bit
	creatures[0xe1].icon=8426;

	creatures[0xe2].basesound=168;								// yet another horse
	creatures[0xe2].icon=8484;

	creatures[0xe4].basesound=168;								// horse ...
	creatures[0xe4].icon=8480;

	creatures[0xe7].basesound=120;								// brown cow
	creatures[0xe7].who_am_i+=2;
	creatures[0xe7].icon=8432;

	creatures[0xe8].basesound=100;								// bull
	creatures[0xe8].who_am_i+=2;
	creatures[0xe8].icon=8431;

	creatures[0xe9].basesound=120;								// b/w cow
	creatures[0xe9].who_am_i+=2;
	creatures[0xe9].icon=8451;

	creatures[0xea].basesound=130;								// deer
	creatures[0xea].soundflag=2;
	creatures[0xea].icon=8404;

	creatures[0xed].basesound=130;								// small deer
	creatures[0xed].soundflag=2;
	creatures[0xed].icon=8404;

	creatures[0xee].basesound=204;								// rat
	creatures[0xee].icon=8483;

	creatures[0x122].basesound=196;								// Boar
	creatures[0x122].icon=8449;

	creatures[0x123].basesound=168;								// pack horse
	creatures[0x123].icon=8486;

	creatures[0x124].basesound=183;								// pack llama
	creatures[0x124].soundflag=2;
	creatures[0x124].icon=8487;

	creatures[0x23d].basesound=263;								// e-vortex
	creatures[0x23e].basesound=512;								// blade spritit
	creatures[0x23e].soundflag=4;

	creatures[0x600].basesound=115;								// cougar;
	creatures[0x600].icon=8473;

	creatures[0x190].icon=8454;
	creatures[0x191].icon=8455;
}

/*!
\brief increase or decrease the karma of the char
\author Endymion
\since 0.82a
\param value positive or negative value to add to karma
\param killed ptr to killed char
\note every increase of karma have an event and stuff related
*/
void cChar::IncreaseKarma( SI32 value, P_CHAR killed )
{

	// nEffect = 1 positive karma effect


	P_CHAR pc=this;
	SI32 nKarma = value;

	if (nKarma==0 || pc->npc ) return;

	int nChange=0, nEffect=0;


	int nCurKarma = pc->GetKarma();

	if(nCurKarma>10000)
		pc->SetKarma(10000);
	else
		if(nCurKarma<-10000)
			pc->SetKarma(-10000);

	if(nCurKarma<nKarma && nKarma>0)
	{
		nChange=((nKarma-nCurKarma)/75);
		//chars[nCharID].karma=(nCurKarma+nChange);
		pc->karma += nChange;
		nEffect=1;
	}

	//AntiChrist - modified to avoid crashes
	//the nKilledID==INVALID check and the chars[nKilledID] check were in the same line
	//That may cause some crash with some compilator caus there's no a defined
	//order in executing these if checks
	if(nCurKarma>nKarma)
		if ( !ISVALIDPC(killed) )
		{
			nChange=((nCurKarma-nKarma)/50);
			//chars[nCharID].karma=(nCurKarma-nChange);
			pc->karma += nChange;
			nEffect=0;
		}
		else
			if( killed->GetKarma()>0 )
			{
				nChange=((nCurKarma-nKarma)/50);
				//chars[nCharID].karma=(nCurKarma-nChange);
				pc->karma += nChange;
				nEffect=0;
			}

	if( (nChange==0) )
		return;


	if (pc->amxevents[EVENT_CHR_ONREPUTATIONCHG])
	{
		g_bByPass = false;
		int n = nChange;
		if (!nEffect) n = -nChange;
		pc->amxevents[EVENT_CHR_ONREPUTATIONCHG]->Call(pc->getSerial32(), n, REPUTATION_KARMA);
		if (g_bByPass==true) return;
	}
	/*
	pc->runAmxEvent( EVENT_CHR_ONREPUTATIONCHG, pc->getSerial32(), (!nEffect ? -nChange : nChange), REPUTATION_KARMA);
	if (g_bByPass==true)
		return;
	*/
	if(nChange<=25)
	{
		if(nEffect)
			pc->sysmsg( TRANSLATE("You have gained a little karma."));
		else
			pc->sysmsg( TRANSLATE("You have lost a little karma."));

		return;
	}

	else if(nChange<=75)
	{
		if(nEffect)
			pc->sysmsg( TRANSLATE("You have gained some karma."));
		else
			pc->sysmsg( TRANSLATE("You have lost some karma."));

		return;
	}

	else if(nChange<=100)
	{
		if(nEffect)
			pc->sysmsg( TRANSLATE("You have gained alot of karma."));
		else
			pc->sysmsg( TRANSLATE("You have lost alot of karma."));

		return;
	}

	else if(nChange>100)
	{
		if(nEffect)
			pc->sysmsg( TRANSLATE("You have gained a huge amount of karma."));
		else
			pc->sysmsg( TRANSLATE("You have lost a huge amount of karma."));

		return;
	}
}

/*!
\brief increase or decrease the fame of the char
\author Endymion
\since 0.82a
\param value positive or negative value to add to karma
\note every increase of karma have an event and stuff related
*/
void cChar::modifyFame( SI32 value )
{

	P_CHAR pc=this;
	SI32 nFame=value;

	if ( nFame==0 || pc->npc ) //NPCs don't gain fame.
		return;

	int nChange=0, nEffect=0;

	int nCurFame= pc->fame;

	if(nCurFame>nFame) // if player fame greater abort function
	{
		if(nCurFame>10000)
			pc->fame=10000;

		return;
	}

	if(nCurFame<nFame)
	{
		nChange=(nFame-nCurFame)/75;
		pc->fame=(nCurFame+nChange);
		nEffect=1;
	}

	if(pc->dead)
	{
		if(nCurFame<=0)
		{
			pc->fame=0;
		}
		else
		{
			nChange=(nCurFame-0)/25;
			pc->fame=(nCurFame-nChange);
		}
		pc->deaths++;
		nEffect=0;
	}
	if( nChange==0 )
		return;


	if (pc->amxevents[EVENT_CHR_ONREPUTATIONCHG])
	{
		g_bByPass = false;
		int n = nChange;
		if (!nEffect) n = -nChange;
		pc->amxevents[EVENT_CHR_ONREPUTATIONCHG]->Call(pc->getSerial32(), n, REPUTATION_FAME);
		if (g_bByPass==true) return;
	}
	/*
	pc->runAmxEvent( EVENT_CHR_ONREPUTATIONCHG, pc->getSerial32(), (!nEffect ? -nChange : nChange), REPUTATION_FAME);
	if (g_bByPass==true)
		return;
	*/
	if(nChange<=25)
	{
		if(nEffect)
			pc->sysmsg( TRANSLATE("You have gained a little fame."));
		else
			pc->sysmsg( TRANSLATE("You have lost a little fame."));
		return;
	}

	else if(nChange<=75)
	{
		if(nEffect)
			pc->sysmsg( TRANSLATE("You have gained some fame."));
		else
			pc->sysmsg( TRANSLATE("You have lost some fame."));

		return;
	}
	else if(nChange<=100)
	{
		if(nEffect)
			pc->sysmsg( TRANSLATE("You have gained alot of fame."));
		else
			pc->sysmsg( TRANSLATE("You have lost alot of fame."));

		return;
	}
	else if(nChange>100)
	{
		if(nEffect)
			pc->sysmsg( TRANSLATE("You have gained a huge amount of fame."));
		else
			pc->sysmsg( TRANSLATE("You have lost a huge amount of fame."));

		return;
	}
}

void enlist(int s, int listnum) // listnum is stored in items morex
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	
	int x,j;
//	char sect[512];
    cScpIterator* iter = NULL;
    char script1[1024];

	//sprintf(sect, "SECTION ITEMLIST %i", listnum);

	iter = Scripts::Items->getNewIterator("SECTION ITEMLIST %i", listnum);
    if (iter==NULL) return;

	int loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			x=str2num(script1);
			P_ITEM pj=item::CreateFromScript( x, pc->getBackpack() );
			VALIDATEPI(pj);
			j= DEREF_P_ITEM(pj);
			pj->Refresh();
		}
	}
	while((strcmp(script1,"}")) && (++loopexit < MAXLOOPS) );
    safedelete(iter);
}



/*!
\brief make someone criminal
\author Repsys Ripper
\param pc character to make criminal
\remark cleaned and modified by Luxor to use CRIMINAL temp effect
*/
void criminal(P_CHAR pc)
{
	VALIDATEPC(pc);
	if ((!pc->npc)&&(!pc->IsCriminal() || !pc->IsMurderer()))
	{//Not an npc, not grey, not red
		tempfx::add(pc, pc, tempfx::CRIMINAL, 0, 0, 0); //Luxor
		if(region[pc->region].priv&0x01 && SrvParms->guardsactive) { //guarded
			if (ServerScp::g_nInstantGuard == 1)
				npcs::SpawnGuard( pc, pc, pc->getPosition() ); // LB bugfix
		}
	}
}

void setcharflag(P_CHAR pc)// repsys ...Ripper
{
	VALIDATEPC(pc);
	setcharflag2(pc);
	//if( setcharflag2(pc) )
		/*if (pc->amxevents[EVENT_CHR_ONFLAGCHG])
			pc->amxevents[EVENT_CHR_ONFLAGCHG]->Call(pc->getSerial32(), pc->getSocket() );*/
}

LOGICAL setcharflag2(P_CHAR pc)// repsys ...Ripper
{
	LOGICAL flagHasChanged = false;

	if (!pc->npc)
	{
		if ( pc->kills >= (unsigned) repsys.maxkills )
		{
			if( !pc->IsMurderer() )
				flagHasChanged = true;
			//
			//! \todo - TODO check out logic of next 2 statements (Sparhawk)
			//
			pc->SetMurderer();
			pc->murderrate = (repsys.murderdecay*MY_CLOCKS_PER_SEC)+uiCurrentTime;
		}
		/*else

			if ( pc->crimflag == 0 )
			{
				if( !pc->IsInnocent() )
					flagHasChanged = true;
				pc->SetInnocent();
			}
			else
			{
				if( !pc->IsCriminal() )
					flagHasChanged = true;
				pc->SetCriminal();
			}
			*/
	}
	else
	{
		switch (pc->npcaitype)
		{
			case NPCAI_EVIL:
			case NPCAI_EVILHEALER:
			case NPCAI_MADNESS: // EV & BS
				if( !pc->IsMurderer() )
					flagHasChanged = true;
				pc->SetMurderer();
				break;
			case NPCAI_HEALER:
			case NPCAI_TELEPORTGUARD:
			case NPCAI_BEGGAR:
			case NPCAI_ORDERGUARD:
			case NPCAI_CHAOSGUARD:
			case NPCAI_BANKER:
			case NPCAI_GUARD:
			case NPCAI_PLAYERVENDOR:
			case 30: // ?
			case 40: // ? old teleport guards?
				if( !pc->IsInnocent() )
					flagHasChanged = true;
				pc->SetInnocent();
				break;

			default:
				if ( pc->HasHumanBody() )
				{
					if( !pc->IsInnocent() )
						flagHasChanged = true;
					pc->SetInnocent();
				}
				else if (server_data.animals_guarded == 1 && pc->npcaitype == NPCAI_GOOD && !pc->tamed)
				{
					if (region[pc->region].priv & RGNPRIV_GUARDED)	// in a guarded region, with guarded animals, animals == blue
					{
						if( !pc->IsInnocent() )
							flagHasChanged = true;
						pc->SetInnocent();
					}
					else				// if the region's not guarded, they're gray
					{
						if( !pc->IsCriminal() )
							flagHasChanged = true;
						pc->SetCriminal();
					}
				}
				else if (pc->getOwnerSerial32()>INVALID && pc->tamed)
				{
					P_CHAR pc_owner = pointers::findCharBySerial( pc->getOwnerSerial32());
					if(ISVALIDPC(pc_owner))
					{
						if( pc->flag != pc_owner->flag )
							flagHasChanged = true;
						pc->flag = pc_owner->flag;
					}
				}
				else
				{
					if( !pc->IsCriminal() )
						flagHasChanged = true;
					pc->SetCriminal();
				}
				break;
		}
	}

	return flagHasChanged;
}



/*!
\brief initialise globals variables
\author ?
*/
void SetGlobalVars()
{
	int i=0;
	ConOut("Initializing global variables...");

	w_anim[0]=0; w_anim[1]=0; w_anim[2]=0;

	for (i=0; i>ALLSKILLS; i++) { strcpy(title[i].other, "old titles.scp error"); }
	completetitle = new char[1024];
	for (i=0;i<(MAXCLIENT);i++) { LSD[i]=0; DRAGGED[i]=0; EVILDRAGG[i]=0; clientDimension[i]=2; noweather[i]=1; } // LB
	for (i=0;i<MAXLAYERS;i++) layers[i]=0;

	save_counter=0;

	nettimeout.tv_sec=0;
	nettimeout.tv_usec=0;
	keeprun=true;
	error=0;
	now=0;
	secure=1;
//	charcount=0;
//	itemcount=0;
//	charcount2=1;
//	itemcount2=0x40000000;
	donpcupdate=0;
	wtype=0;

	xcounter=0;
	ycounter=0;
	executebatch=0;
	g_nShowLayers=false;
	ConOut(" [ OK ]\n");
}


/*!
\brief ?
\author ?
*/
void BuildPointerArray()
{
	int memerrflg=0;
	for (int i=0;i<HASHMAX;i++)
	{
		//cownsp[i].pointer = NULL;
		//ownsp[i].pointer = NULL;
		//imultisp[i].pointer = NULL;
		//cmultisp[i].pointer = NULL;
//		glowsp[i].pointer = NULL;

#define PTRPREALLOC 25
		// init them
		// xan : we'll use realloc, so use malloc, not new for sake of portability
		//if(( imultisp[i].pointer = reinterpret_cast<int*>(malloc(PTRPREALLOC*sizeof(int)))) == NULL)
		//	memerrflg=1;
		//if(( cmultisp[i].pointer = reinterpret_cast<int*>(malloc(PTRPREALLOC*sizeof(int)))) == NULL)
		//	memerrflg=1;
		/*if(( glowsp[i].pointer = reinterpret_cast<int*>(malloc(PTRPREALLOC*sizeof(int)))) == NULL)
			memerrflg=1;*/

		if (memerrflg)
		{
			ConOut("\nFatal Error: Couldn't allocate pointer memory!\n");
			Network->kr=0;
			return;
		}
		//imultisp[i].max=cmultisp[i].max=/*glowsp[i].max=*/25;
		//for (int j=0;j<25;j++)
		//	imultisp[i].pointer[j]=cmultisp[i].pointer[j]=/*glowsp[i].pointer[j]=*/INVALID;
	}
}


/*!
\brief initialise multis
\author unknown, modified by Luxor
\note Really slow because it uses the cAllObjectsIter, must change this soon.
*/
void InitMultis()
{
	cAllObjectsIter objs;
	for( objs.rewind(); !objs.IsEmpty(); objs++ )
	{
	/*for (i=0;i<charcount;i++)
	{*/
		if ( !isCharSerial(objs.getSerial()) ) continue;
		P_CHAR pc_i = (P_CHAR)(objs.getObject());
		if(!ISVALIDPC(pc_i))
			continue;

		P_ITEM multi=findmulti( pc_i->getPosition() );
		if (ISVALIDPI(multi))
		{
			if (multi->type==117)
				//setserial(i,DEREF_P_ITEM(multi),8);
				pc_i->setMultiSerial(multi->getSerial32());
			else
				pc_i->setMultiSerial32Only(INVALID);
		}
	}

	P_ITEM pi;
	for( objs.rewind(); !objs.IsEmpty(); objs++ )
	{
	/*for (i=0;i<itemcount;i++)
	{*/
		if ( isCharSerial(objs.getSerial()) ) continue;
		pi=(P_ITEM)(objs.getObject());
		if(!ISVALIDPI(pi))
			continue;

		//Endymion modified from !pi->isInWorld() to pi->isInWorld()
		if (pi->isInWorld() && (pi->getSerial32()!=INVALID))
		{
			P_ITEM multi=findmulti( pi->getPosition() );
			if (ISVALIDPI(multi))
				if (multi->getSerial32()!=pi->getSerial32())
					//setserial(DEREF_P_ITEM(pi),DEREF_P_ITEM(multi),7);
					pi->SetMultiSerial(multi->getSerial32());
				else
					pi->setMultiSerial32Only(INVALID);
		}
	}
}

void StartClasses()
{
	ConOut("Initializing classes...");


	// Classes nulled now, lets get them set up :)
	cwmWorldState=new CWorldMain;
	Guilds=new cGuilds;
	Map=new cMapStuff;
	Network=new cNetwork;

	spawns::initialize();
	restocks::initialize();
	regions::initialize();

	ConOut(" [ OK ]\n");
}

void DeleteClasses()
{
	delete cwmWorldState;
	delete Guilds;
	delete Map;
	delete Network;
	if( tiledata::tiledata )	delete tiledata::tiledata;

	accounts::finalize();
	
	//objects.clear();
}
////////////////////////////
// garbage collection
////////////////////////////
static bool s_bGarbageCollect = false;			//! garbage must be checked ?

//! request garbage collecting
void gcollect()
{
	s_bGarbageCollect = true;
}

/*!
\brief Remove items which were in deleted containers
\remarks \remark rewritten by Anthalir
*/
void checkGarbageCollect ()
{
	int removed, rtotal=0, corrected=0;
	int loopexit=0;
	bool first=true;

	if (!s_bGarbageCollect) return;
	s_bGarbageCollect = false;

	LogMessage("Performing Garbage Collection...");

	cAllObjectsIter objs;

	do
	{

		removed=0;

		for( objs.rewind(); !objs.IsEmpty(); objs++ )
		{

			if( isCharSerial( objs.getSerial() ) )
			{
				if( first ) {
					P_CHAR pc=(P_CHAR)(objs.getObject());
					if( pc->getOwnerSerial32()!=INVALID ) {
						P_CHAR own=pointers::findCharBySerial( pc->getOwnerSerial32() );
						if(!ISVALIDPC(own)) {
							pc->setOwnerSerial32( INVALID );
							++corrected;
						}
					}
				}
			}
			else {

				P_ITEM pi=(P_ITEM)(objs.getObject());

				if( pi->isInWorld() )
					continue;

				// find the container if theres one.
				P_CHAR pc_j= pointers::findCharBySerial(pi->getContSerial());
				P_ITEM pi_j= pointers::findItemBySerial(pi->getContSerial());

				// if container serial is invalid
				if( ((pc_j==NULL) ) &&
					((pi_j==NULL) ) )
				{
					pi->Delete();
					++removed;
				}
			}
		}
		rtotal+=removed;
		first=false;
	} while (removed>0 && (++loopexit < MAXLOOPS) );

	if(rtotal>0)
	{
		LogMessage("Gargbage Collector removed %i items",rtotal);
		WarnOut("Gargbage Collector removed %i items\n",rtotal);
	}
	if(corrected>0)
	{
		LogMessage("Gargbage Collector corrected %i char",corrected);
		WarnOut("Gargbage Collector corrected %i char\n",corrected);
	}
}

namespace Scripts {
	cScpScript* Advance = NULL; //cScpScript("advance.scp");
	cScpScript* Calendar = NULL; //cScpScript("calendar.scp");
	cScpScript* Carve = NULL; //cScpScript("carve.scp");
	cScpScript* Colors = NULL; //cScpScript("colors.scp");
	cScpScript* Create = NULL; //cScpScript("create.scp");
	cScpScript* CronTab = NULL; //cScpScript("crontab.scp");
	cScpScript* Envoke = NULL; //cScpScript("envoke.scp");
	cScpScript* Fishing = NULL; //cScpScript("fishing.scp");
	cScpScript* Gumps = NULL; //cScpScript("gumps.scp");
	cScpScript* HardItems = NULL; //cScpScript("harditems.scp");
	cScpScript* House = NULL; //cScpScript("house.scp");

	cScpScript* HostDeny = NULL; //cScpScript("host_deny.xss");
	cScpScript* HtmlStrm = NULL; //cScpScript("htmlstrm.scp");
	cScpScript* Items = NULL; //cScpScript("items.scp");
	cScpScript* Location = NULL; //cScpScript("location.scp");
	cScpScript* MenuPriv = NULL; //cScpScript("menupriv.scp");
	cScpScript* Menus = NULL; //cScpScript("menus.scp");
	cScpScript* MetaGM = NULL; //cScpScript("metagm.scp");
	cScpScript* Misc = NULL; //cScpScript("misc.scp");
	cScpScript* MList = NULL; //cScpScript("mlist.scp");
	cScpScript* MsgBoard = NULL; //cScpScript("msgboard.scp");
	cScpScript* Necro = NULL; //cScpScript("necro.scp");
	cScpScript* Newbie = NULL; //cScpScript("newbie.scp");
	cScpScript* Npc = NULL; //cScpScript("npc.scp");
	cScpScript* NpcMagic = NULL; //cScpScript("npcmagic.scp");
	cScpScript* NTrigrs = NULL; //cScpScript("ntrigrs.scp");
	cScpScript* Override = NULL; //cScpScript("override.scp");
	cScpScript* Polymorph = NULL; //cScpScript("polymorph.scp");
	cScpScript* Regions = NULL; //cScpScript("regions.scp");
	cScpScript* Skills = NULL; //cScpScript("skills.scp");
	cScpScript* Spawn = NULL; //cScpScript("spawn.scp");
	cScpScript* Speech = NULL; //cScpScript("speech.scp");
	cScpScript* Spells = NULL; //cScpScript("spells.scp");
	cScpScript* Teleport = NULL; //cScpScript("teleport.scp");
	cScpScript* Titles = NULL; //cScpScript("titles.scp");
	cScpScript* Tracking = NULL; //cScpScript("tracking.scp");
	cScpScript* Triggers = NULL; //cScpScript("triggers.scp");
	cScpScript* WTrigrs = NULL; //cScpScript("wtrigrs.scp");
	cScpScript* Mountable = NULL; //cScpScript("mount.xss");
	cScpScript* WeaponInfo = NULL; //cScpScript("weaponinfo.xss");
	cScpScript* Containers = NULL; //cScpScript("containers.xss");
	cScpScript* Areas = NULL; //cScpScript("areas.xss");
};

void newScriptsInit()
{
	cScpScript* Dummy = new cScpScript("scripts/symbols.xss");
	safedelete(Dummy);
	Scripts::Advance = new cScpScript("scripts/advance.xss");
//	Scripts::Calendar = new cScpScript("calendar.scp");
	Scripts::Carve = new cScpScript("scripts/carve.xss");
	Scripts::Colors = new cScpScript("scripts/colors.xss");
	Scripts::Create = new cScpScript("scripts/create.xss");
//	Scripts::CronTab = new cScpScript("crontab.scp");
	Scripts::Envoke = new cScpScript("scripts/envoke.xss");
	Scripts::Fishing = new cScpScript("scripts/fishing.xss");
	Scripts::Gumps = new cScpScript("scripts/gumps.xss");
	Scripts::HardItems = new cScpScript("scripts/harditems.xss");
	Scripts::House = new cScpScript("scripts/house.xss");

	Scripts::HostDeny = new cScpScript("config/hostdeny.xss");
	Scripts::HtmlStrm = new cScpScript("scripts/html.xss");
	Scripts::Items = new cScpScript("scripts/items.xss");
	Scripts::Location = new cScpScript("scripts/location.xss");
//	Scripts::MenuPriv = new cScpScript("menupriv.scp");
	Scripts::Menus = new cScpScript("scripts/menus.xss");
	Scripts::MetaGM = new cScpScript("scripts/metagm.xss");
	Scripts::Misc = new cScpScript("scripts/misc.xss");
	Scripts::MsgBoard = new cScpScript("scripts/msgboard.xss");
	Scripts::Necro = new cScpScript("scripts/necro.xss");
	Scripts::Newbie = new cScpScript("scripts/newbie.xss");
	Scripts::Npc = new cScpScript("scripts/npc.xss");
	Scripts::NpcMagic = new cScpScript("scripts/npcmagic.xss");
//	Scripts::NTrigrs = new cScpScript("ntrigrs.scp");
//	Scripts::Override = new cScpScript("override.scp");
	Scripts::Polymorph = new cScpScript("scripts/polymorph.xss");
	Race::load("scripts/race.xss");
	Scripts::Regions = new cScpScript("scripts/regions.xss");
	Scripts::Skills = new cScpScript("scripts/skills.xss");
	Scripts::Spawn = new cScpScript("scripts/spawn.xss");
	Scripts::Speech = new cScpScript("scripts/speech.xss");
	Scripts::Spells = new cScpScript("scripts/spells.xss");
//	Scripts::Teleport = new cScpScript("teleport.scp");
	Scripts::Titles = new cScpScript("scripts/titles.xss");
	Scripts::Tracking = new cScpScript("scripts/tracking.xss");
	Scripts::Triggers = new cScpScript("scripts/triggers.xss");
//	Scripts::WTrigrs = new cScpScript("wtrigrs.scp");
	Scripts::Mountable = new cScpScript("scripts/mounts.xss");
	Scripts::WeaponInfo = new cScpScript("scripts/weaponinfo.xss");
	Scripts::Containers = new cScpScript("scripts/containers.xss");
	Scripts::Areas = new cScpScript("scripts/areas.xss");
}


void deleteNewScripts()
{
	cScpScript* Dummy = new cScpScript("scripts/symbols.xss");
	safedelete(Dummy);//
	safedelete(Scripts::Advance );//= new cScpScript("scripts/advance.xss");
//	Scripts::Calendar = new cScpScript("calendar.scp");
	safedelete(Scripts::Carve );//= new cScpScript("carve.scp");
	safedelete(Scripts::Colors );//= new cScpScript("colors.scp");
	safedelete(Scripts::Create );//= new cScpScript("create.scp");
//	Scripts::CronTab = new cScpScript("crontab.scp");
	safedelete(Scripts::Envoke );//= new cScpScript("envoke.scp");
	safedelete(Scripts::Fishing );//= new cScpScript("fishing.scp");
	safedelete(Scripts::Gumps );//= new cScpScript("gumps.scp");
	safedelete(Scripts::HardItems );//= new cScpScript("harditems.scp");
	safedelete(Scripts::House );//= new cScpScript("house.scp");

	safedelete(Scripts::HostDeny);
	safedelete(Scripts::HtmlStrm );//= new cScpScript("htmlstrm.scp");
	safedelete(Scripts::Items );//= new cScpScript("items.scp");
	safedelete(Scripts::Location );//= new cScpScript("location.scp");
	safedelete(Scripts::MenuPriv );//= new cScpScript("menupriv.scp");
	safedelete(Scripts::Menus );//= new cScpScript("scripts/menus.xss");
	safedelete(Scripts::MetaGM );//= new cScpScript("metagm.scp");
	safedelete(Scripts::Misc );//= new cScpScript("misc.scp");
	safedelete(Scripts::MList );//= new cScpScript("mlist.scp");
	safedelete(Scripts::MsgBoard);// = new cScpScript("msgboard.scp");
	safedelete(Scripts::Necro );//= new cScpScript("scripts/necro.xss");
	safedelete(Scripts::Newbie );//= new cScpScript("newbie.scp");
	safedelete(Scripts::Npc );// new cScpScript("scripts/npc.xss");
	safedelete(Scripts::NpcMagic);// = new cScpScript("npcmagic.scp");
//	safedelete(Scripts::NTrigrs);// = new cScpScript("ntrigrs.scp");
//	Scripts::Override = new cScpScript("override.scp");
	safedelete(Scripts::Polymorph);// = new cScpScript("polymorph.scp");
	safedelete(Scripts::Regions);// = new cScpScript("regions.scp");
	safedelete(Scripts::Skills);// = new cScpScript("skills.scp");
	safedelete(Scripts::Spawn);// = new cScpScript("spawn.scp");
	safedelete(Scripts::Speech);// = new cScpScript("speech.scp");
	safedelete(Scripts::Spells);// = new cScpScript("spells.scp");
	safedelete(Scripts::Teleport);// = new cScpScript("teleport.scp");
	safedelete(Scripts::Titles);// = new cScpScript("titles.scp");
	safedelete(Scripts::Tracking);// = new cScpScript("tracking.scp");
	safedelete(Scripts::Triggers);// = new cScpScript("triggers.scp");
//	safedelete(Scripts::WTrigrs);// = new cScpScript("wtrigrs.scp");
	safedelete(Scripts::Mountable);
	safedelete(Scripts::WeaponInfo);
	safedelete(Scripts::Containers);
	safedelete(Scripts::Areas);
}

#ifndef _EXTERNANGEL
void angelMode()
{
}
#endif
