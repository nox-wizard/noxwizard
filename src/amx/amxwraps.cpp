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
#include "basics.h"
#include "cmdtable.h"
#include "speech.h"
#include "sndpkg.h"
#include "sregions.h"
#include "tmpeff.h"
#include "magic.h"
//#include "amx_vm.h"
#include "amxscript.h"
#include "amxwraps.h"
#include "amxcback.h"
#include "set.h"
#include "amxvarserver.h"
#include "amxfile.h"
#include "menu.h"
#include "oldmenu.h"
#include "itemid.h"
#include "timers.h"
#include "data.h"
#include "scp_parser.h"
#include "fishing.h"
#include "archive.h"
#include "map.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "classes.h"
#include "nox-wizard.h"
#include "range.h"
#include "skills.h"
#include "utils.h"
#include "jail.h"
#include "party.h"
#include "gmpages.h"
#include "house.h"
#include "msgboard.h"
#include "layer.h"
#include "dragdrop.h"
#include "accounts.h"
#include "spawn.h"
#include "resourcemap.h"
#include "srvparms.h"

#ifdef _WINDOWS
#include "nxwgui.h"
#endif
#include "version.h"


#ifdef WIN32
#pragma message("     WARNING: removing warning doesn't remove the cause of the warning...")
#pragma message("              maybe smaller type may be used instead of truncating big one")
#pragma warning(disable: 4244) //Disable "possible loss of data" warning
#pragma warning(disable: 4100) //Disable "possible loss of data" warning
#endif

//property system lies in amxwrap2.cpp
/* Sparhawk 2001-09-15 added access to calendar properties */
NATIVE2(_getCharProperty);
NATIVE2(_setCharProperty);
NATIVE2(_getItemProperty);
NATIVE2(_setItemProperty);
NATIVE2(_getCalProperty);
NATIVE2(_setMenuProperty);
NATIVE2(_getMenuProperty);
NATIVE2(_getRaceProperty);
NATIVE2(_getRaceGlobalProp);
NATIVE2(_party_getProperty );
NATIVE2(_party_setProperty );
NATIVE2(_region_getProperty );
NATIVE2(_region_setProperty );

NATIVE2(_guild_setProperty);
NATIVE2(_guild_getProperty);
NATIVE2(_guildMember_setProperty);
NATIVE2(_guildMember_getProperty);
NATIVE2(_guildRecruit_setProperty);
NATIVE2(_guildRecruit_getProperty);



int g_nCurrentSocket;
int g_nTriggeredItem;
int g_nTriggerType;
int g_nStringMode = 0;

extern bool g_bByPass;
extern int g_nMoment;

extern "C" {
	/************** THIS PORTION OF CODE IS OF NoX-Wizard ********************/
	/*** PLZ REFER TO SMALL WEBSITE FOR ORIGINAL SOURCE CODE OF THIS LIBRARY! :D **/

	char g_cAmxPrintBuffer[2000];
	int  g_nAmxPrintPtr = 0;

	#define NO_DEF_FUNCS

	void amx_printf(char *txt, ...) // Printf to nxwconsole
	{
		va_list argptr;
		char buffer[1024];

		va_start( argptr, txt );
		vsnprintf( buffer, sizeof(buffer)-1, txt, argptr );
		va_end( argptr );

		g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
		strcat(g_cAmxPrintBuffer, buffer);
		g_nAmxPrintPtr=strlen(g_cAmxPrintBuffer);

	}


	int amx_putchar(char i)	{ g_cAmxPrintBuffer[g_nAmxPrintPtr++] = i; return i;}

	int amx_fflush(void* n)   { g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0'; ConOut("%s",g_cAmxPrintBuffer); g_nAmxPrintPtr=0; return 0;}

	int amx_getch(void)
	{
	  return 0; /** XAN : YOU CAN'T STOP THE SERVER FOR INPUT!!! :) */
	}

	char *amx_gets(char *a,int b)
	{
	  return NULL; /** XAN : YOU CAN'T STOP THE SERVER FOR INPUT!!! :) */
	}

}


int g_AmxErrNo = AMXERRNO_OK;

/*
\brief return the error type
\author Xanathar
\return error type
*/
NATIVE (_geterror)
{
    int n = g_AmxErrNo;
    g_AmxErrNo = AMXERRNO_OK;
    return n;
}

/*
\brief set the window title
\author Xanathar
\since 0.30
\param 1: new title
\return 0 if success, INVALID if not supported
*/
NATIVE(_setWindowTitle)
{
	if (ServerScp::g_nLoadDebugger!=0) return INVALID;
	cell *cstr;
	amx_GetAddr(amx,params[1],&cstr);
	printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;

	if (ServerScp::g_nDeamonMode==0) return INVALID;

	#ifdef __unix__
	    // Linux & *BSD & BeOS 5 :]
	    ConOut("\033]0;%s\007", g_cAmxPrintBuffer);
		return 0;
	#endif

	#ifdef _WINDOWS
	    // Win32 GUI mode :]
		SetWindowText(g_HWnd, g_cAmxPrintBuffer);
		return 0;
	#endif

	#ifdef WIN32
		//here : win32 console :)
		SetConsoleTitle(g_cAmxPrintBuffer);
		return 0;
	#endif
}

/*
\brief bypasses default server behaviour
\author Xanathar
\since 0.10
*/
NATIVE(_bypass)
{ g_bByPass = true; return 0; }

/*
\brief get the frame status
\author Xanathar
\since 0.20
\return 1 in the after phase of a frame override, 0 otherwise
*/
NATIVE(_getFrameStatus)
{ return (g_nMoment==AMX_AFTER) ? 1 : 0; }

/*
\brief perform a CFG command configuring a server.scp entry
\author Xanathar
\since 0.50
\param 1: the command
*/
NATIVE(_cfgServerOption)
{
	cell *cstr;
	amx_GetAddr(amx,params[1],&cstr);
	printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
  	return cfg_command(g_cAmxPrintBuffer);
}

/*!
\brief Prepares a socket for a menu, initializing it and destroying any previous menu
\author Xanathar, update Endymion
\since 0.20
\param 1 character
\param 2 number of pages
\param 3 number of item for page
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_mnu_Prepare)
{

	P_CHAR pc = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pc, INVALID );

	if ( !pc->IsOnline() )
		return INVALID;

	if(pc->custmenu!=INVALID) {
		Menus.removeMenu( pc->custmenu, pc );
		pc->custmenu=INVALID;
	}

	P_OLDMENU menu = (P_OLDMENU)Menus.insertMenu( new cOldMenu() );
	pc->custmenu = menu->serial;
	menu->setParameters( params[3], params[2] );
	return 0;
}

/*!
\brief Set menu style and eventually the color
\author Xanathar, update Endymion
\since 0.20
\param 1 character
\param 2 style
\param 3 color ( default 0 )
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_mnu_SetStyle)
{
	P_CHAR pc = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pc, INVALID );

	if ( !pc->IsOnline() )
		return INVALID;


	P_OLDMENU menu = (P_OLDMENU)Menus.getMenu( pc->custmenu );
	menu->style = params[2];
	menu->color = params[3];

	return 0;
}

/*!
\brief Set the menu title
\author Xanathar, update Endymion
\since 0.20
\param 1 character
\param 2 title
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_mnu_SetTitle)
{

	P_CHAR pc = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pc, INVALID );

	if ( !pc->IsOnline() )
		return INVALID;

	cell *cstr;
	amx_GetAddr(amx,params[2],&cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';

	std::wstring t;
	string2wstring( std::string( g_cAmxPrintBuffer ), t );

	P_OLDMENU menu = (P_OLDMENU)Menus.getMenu( pc->custmenu );
	menu->title = t;
	g_nAmxPrintPtr=0;

	return 0;
}

/*!
\brief Set the menu color
\author Xanathar, update Endymion
\since 0.20
\param 1 character
\param 2 color
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_mnu_SetColor)
{
	P_CHAR pc = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pc, INVALID );

	if ( !pc->IsOnline() )
		return INVALID;

	P_OLDMENU menu = (P_OLDMENU)Menus.getMenu( pc->custmenu );
	menu->color = params[2];
	return 0;
}

/*!
\brief Adds an item at a given position of a correctly inizialized menu
\author Xanathar, update Endymion
\since 0.20
\param 1 character
\param 2 page number
\param 3 index number
\param 4 text
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_mnu_AddItem)
{
	P_CHAR pc = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pc, INVALID );

	if ( !pc->IsOnline() )
		return INVALID;

	cell *cstr;
	amx_GetAddr(amx,params[4],&cstr);
	printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';

	std::wstring t;
	string2wstring( std::string( g_cAmxPrintBuffer ), t );

	P_OLDMENU menu = (P_OLDMENU)Menus.getMenu( pc->custmenu );
	menu->addMenuItem( params[2], params[3], t );
	g_nAmxPrintPtr=0;
	return 0;
}

/*!
\brief Show up the menu window
\author Xanathar, update Endymion
\since 0.20
\param 1 character
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_mnu_Show)
{
	P_CHAR pc = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pc, INVALID );

	if ( !pc->IsOnline() )
		return INVALID;

	P_OLDMENU menu = (P_OLDMENU)Menus.getMenu( pc->custmenu );
	menu->show( pc );
	return 0;
}

/*
\brief Set the callback for this menu events
\author Xanathar, update Endymion
\since 0.20
\param 1 character
\param 2 callback
\return 0 if success, INVALID if error ( es socket invalid )
\note callback can be obtained by call to funcidx
*/
NATIVE(_mnu_SetCallback)
{
	P_CHAR pc = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pc, INVALID );

	if ( !pc->IsOnline() )
		return INVALID;

	P_OLDMENU menu = (P_OLDMENU)Menus.getMenu( pc->custmenu );
	menu->setCallBack( params[2] );
	return 0;
}





/*
\brief get the item that invoked the trigger
\author Xanathar
\since 0.53
\return the item
\note valid in triggers only
\note deprecated
*/
NATIVE(_getTriggeredItem)
{ return g_nTriggeredItem; }

/*
\brief get the type of item that invoked the trigger
\author Xanathar
\since 0.53
\return the type
\note valid in triggers only
\note deprecated
*/
NATIVE(_getTriggeredItemTrigType)
{ return g_nTriggerType; }

/*
\brief get the NoxWizard Version
\author Xanathar
\since 0.10
\return the version number
\note es 0.23 = 0x0023
*/
NATIVE(_getNXWVersion)
{ return VERHEXCODE; }

/*
\brief get the NoxWizard Version type
\author Xanathar
\since 0.50
\return the version type
\note the type is a bitfield of VERTYPE_ definitions ( see nxw.inc )
*/
NATIVE(_getNXWVersionType)
{ return VERTYPE; }

/*
\brief get the platform where NoxWizard is running
\author Xanathar
\since 0.50
\return the platoform
\note platform is one of PLATFORM_ definitions ( see nxw.inc )
*/
NATIVE(_getNXWPlatform)
{
	return PLATFORMID;
}

NATIVE(_sprintf)
{
	cell *src;
	if (params[2] < 0)
		return 0;
	amx_GetAddr(amx,params[2],&src);
	printstring(amx,src,params+3,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	cell *dest;
	amx_GetAddr(amx,params[1],&dest);
	amx_SetString(dest, g_cAmxPrintBuffer, g_nStringMode);
	g_nAmxPrintPtr=0;
	return strlen( g_cAmxPrintBuffer );
}

/*
\brief Send a sysmessage to given character using given color
\author Endymion
\param 1 character
\param 2 color
\param 3 text
\return 1 or 0 if error
*/
NATIVE(_chr_message)
{

	P_CHAR pc = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pc, false );

	NXWCLIENT ps = pc->getClient();
	if( ps==NULL )
		return false;

	cell *cstr;
	amx_GetAddr(amx,params[3],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	if (g_nAmxPrintPtr > 512 )
	{
		LogWarning("chr_message: String longer than 512 Characters, cutting off after 511 characters");
		g_cAmxPrintBuffer[512] = '\0';
	}
	g_nAmxPrintPtr=0;

	ps->sysmsg( params[2], g_cAmxPrintBuffer );
	return true;

}

/*
\brief get the current string mode used
\author Xanathar
\since 0.10
\return the string mode
*/
NATIVE(_getStringMode)
{ 	return g_nStringMode; }

/*
\brief set the current string mode used
\author Xanathar
\since 0.10
\param 1: new string mode ( 0=unpacked, 1=packet )
\return the string mode
*/
NATIVE(_setStringMode)
{	g_nStringMode = params[1];	return g_nStringMode; }

/*
\brief get the current time
\author Xanathar
\since 0.10
\return the current time
*/
NATIVE(_getCurrentTime)
{	return uiCurrentTime; }

/*
\brief get the current system time (in s)
\author Keldan
\since 0.83
\return the current system time (in s)
*/
NATIVE(_getSystemTime)
{   return getsysclock(); }

/*
\brief get if the character Yell is a server broadcast
\author Xanathar
\since 0.10
\param 1: the character
\return true or false
*/
NATIVE(_canBroadcast)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, false );
	return pc->CanBroadcast();
}

/*
\brief get if the character can see serial number
\author Xanathar
\since 0.10
\param 1: the character
\return true or false
*/
NATIVE(_canSeeSerials)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, false );
	return pc->CanSeeSerials();
}

/*
\brief get if the character can snoop everywhere always
\author Xanathar
\since 0.10
\param 1: the character
\return true or false ( false also if not valid character )
*/
NATIVE(_canSnoop)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, false );
	return pc->CanSnoop();
}

/*
\brief Jail the given character
\author Endymion
\since 0.10
\param 1 the jailer ( can be INVALID )
\param 2 the player
\param 3 second ( if 0, release player if jailed )
\return true or false ( false also if not valid character )
*/
NATIVE(_chr_jail)
{

	P_CHAR jailer = pointers::findCharBySerial( params[1] );

	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, false );

	if( params[3]==0 ) {
		if( pc->jailed )
			prison::release( jailer, pc );
	}
	else
		prison::jail( jailer, pc, params[3] );

	return 1;
}

/*
\brief Show the status gump of the target char to the calling char or the gump of the caller itself
\author Wintermute
\since 0.82
\param 1 the caller 
\param 2 the target ( can be INVALID )
\return true or false ( false also if not valid character )
*/
NATIVE (_chr_showStatus )
{
	P_CHAR pc=pointers::findCharBySerial(params[1]);
	P_CHAR target=pointers::findCharBySerial(params[2]);
	if ( !ISVALIDPC(pc))
		return 0;
	if ( !ISVALIDPC(target))
		target=pc;
	if (target->amxevents[EVENT_CHR_ONOPENSTATUS]!=NULL) 
	{
		g_bByPass = false;
		target->amxevents[EVENT_CHR_ONOPENSTATUS]->Call( target->getSerial32(), pc->getSerial32() );
		if (g_bByPass) 
		{
			return 1;
		}
	}
	statwindow(target, pc);
	return 1;
}

/*
\brief get the total gold in character bank box
\author Xanathar
\since 0.10
\param 1: the character
\return the number of gold or INVALID if not valid character
*/
NATIVE(_CountBankGold)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	return pc->countBankGold();
}

/*
\brief get the total gold in character backpack
\author Xanathar
\since 0.10
\param 1: the character
\return the number of gold or INVALID if not valid character
*/
NATIVE(_CountGold)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	return pc->CountGold();
}

/*NATIVE(_CountItems)
{ return chars[params[1]].CountItems(params[2], params[3]); }
*/

/*
\brief get number of item of given type and given color in backpack?
\author ??
\since ??
\param 1: the character
\param 2: the item type?
\param 3: the color?
\return the number of item or INVALID if not valid character
*/
NATIVE(_CountColoredItems)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	return pc->CountItems(params[2], params[3]);
}


/*
\brief disturb the meditation of the character
\author Xanathar
\since 0.10
\param 1: the character
\return INVALID if not valid character, 0 else
*/
NATIVE(_disturbMed)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->disturbMed();
	return 0;
}

/*
\brief get the char's bankbox
\author Xanathar
\since 0.10
\note bug fixed by Luxor
\param 1: the character
\param 1: bank box type ( BANKBOX_BANK, BANKBOX_WARE )
\return INVALID if not valid character, else the bank
*/
NATIVE(_GetBankBox)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
        P_ITEM pi = pc->GetBankBox(params[2]);
        VALIDATEPIR( pi, INVALID );
        return pi->getSerial32();
}

/*
\brief get the serial of the item on the specified layer of the character
\author Xanathar
\since 0.10
\param 1: the character
\param 1: layer
\return INVALID on failure, item serial on success
*/
NATIVE(_GetItemOnLayer)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	P_ITEM pi=pc->GetItemOnLayer(params[2]);
	return ISVALIDPI(pi) ? pi->getSerial32() : INVALID;
}

/*
\brief get character's privileges
\author Xanathar
\since 0.10
\param 1: the character
\return INVALID if not valid character, else the privileges
*/
NATIVE(_GetPriv)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	return pc->GetPriv();
}

/*
\brief get character's shield
\author Xanathar
\since 0.10
\param 1: the character
\return INVALID if not valid character or not shield equipped, else the shield
*/
NATIVE(_GetShield)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
    P_ITEM pi = pc->getShield();
    return (ISVALIDPI(pi))? pi->getSerial32() : INVALID;
}

/*
\brief get character's weapon
\author Xanathar
\since 0.10
\param 1: the character
\return INVALID if not valid character or not weapon equipped, else the weapon
*/
NATIVE(_GetWeapon)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
    P_ITEM pi = pc->getWeapon();
    return (ISVALIDPI(pi))? pi->getSerial32() : INVALID;
}

/*
\brief check if character is conselour
\author Xanathar
\since 0.10
\param 1: the character
\return true or false ( false also if not valid character )
*/
NATIVE(_isCounselor)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, false );
	return pc->IsCounselor();
}

/*
\brief check if character is criminal
\author Xanathar
\since 0.10
\param 1: the character
\return true or false ( false also if not valid character )
*/
NATIVE(_isCriminal)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, false );
	return pc->IsCriminal();
}

/*
\brief check if character is Grey for any reason
\author Xanathar
\since 0.20
\param 1: the character
\return true or false ( false also if not valid character )
\bug If not valid char return INVALID, i think it's correct false
*/
NATIVE(_isGrey)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->IsGrey();
}

/*
\brief check if character is Game Master
\author Xanathar
\since 0.10
\param 1: the character
\return true or false ( false also if not valid character )
\bug If not valid char return INVALID, i think it's correct false
*/
NATIVE(_isGM)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->IsGM();
}

/*
\brief check if character is Game Master or Conselour
\author Xanathar
\since 0.54
\param 1: the character
\return true or false ( false also if not valid character )
\bug If not valid char return INVALID, i think it's correct false
*/
NATIVE(_isGMorCounselor)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->IsGMorCounselor();
}

/*
\brief check if character have human body
\author Xanathar
\since 0.10
\param 1: the character
\return true or false ( false also if not valid character )
\bug If not valid char return INVALID, i think it's correct false
*/
NATIVE(_isHuman)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->HasHumanBody();
}

/*
\brief check if character is blue flagged
\author Xanathar
\since 0.10
\param 1: the character
\return true or false ( false also if not valid character )
\bug If not valid char return INVALID, i think it's correct false
*/
NATIVE(_isInnocent)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->IsInnocent();
}

/*
\brief check if character is invulnerable
\author Xanathar
\since 0.10
\param 1: the character
\return true or false ( false also if not valid character )
\bug If not valid char return INVALID, i think it's correct false
*/
NATIVE(_isInvul)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->IsInvul();
}

/*
\brief check if character is a murderer
\author Xanathar
\since 0.10
\param 1: the character
\return true or false ( false also if not valid character )
\bug If not valid char return INVALID, i think it's correct false
*/
NATIVE(_isMurderer)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->IsMurderer();
}

/*
\brief check if character is a GM ( not considering restriction )
\author Xanathar
\since 0.10
\param 1: the character
\return true or false ( false also if not valid character )
\bug If not valid char return INVALID, i think it's correct false
*/
NATIVE(_isTrueGM)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->IsGM();
}

/*
\brief make the character invulnerable
\author Xanathar
\since 0.54
\param 1: the character
\return 0 or INVALID if not valid character
*/
NATIVE(_makeInvulnerable)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->MakeInvulnerable();
    return 0;
}

/*
\brief make the character vulnerable
\author Xanathar
\since 0.10
\param 1: the character
\return 0 or INVALID if not valid character
*/
NATIVE(_makeVulnerable)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->MakeVulnerable(); return 0;
}

/*
\brief teleport char to given position
\author Xanathar
\since 0.10
\param 1: the character
\param 2: x location
\param 3: y location
\param 4: z location
\return 0 or INVALID if not valid character
*/
NATIVE(_moveTo)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->MoveTo( params[2],params[3],params[4] );
	pc->teleport();
	return 0;
}

/*
\brief set the given character criminal
\author Xanathar
\since 0.10
\param 1: the character
\return 0 or INVALID if not valid character
*/
NATIVE(_setCriminal)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->SetCriminal();
	return 0;
}

/*
\brief set the given character grey for a limited time
\author Xanathar
\since 0.20
\param 1: the character
\return 0 or INVALID if not valid character
*/
NATIVE(_setGrey)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->SetGrey();
	return 0;
}

/*
\brief set the given character grey forever
\author Xanathar
\since 0.20
\param 1: the character
\return 0 or INVALID if not valid character
*/
NATIVE(_setPermaGrey)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->SetPermaGrey();
	return 0;
}

/*
\brief set the given character innocent
\author Xanathar
\since 0.10
\param 1: the character
\return 0 or INVALID if not valid character
*/
NATIVE(_setInnocent)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->SetInnocent();
	return 0;
}

/*
\brief set the given character murderer
\author Xanathar
\since 0.10
\param 1: the character
\return 0 or INVALID if not valid character
*/
NATIVE(_setMurderer)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->SetMurderer();
	return 0;
}

/*
\brief set the characters privileges
\author Xanathar
\since 0.10
\param 1: the character
\param 2: priviliges value
\return 0 or INVALID if not valid character
*/
NATIVE(_setPriv)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->SetPriv(params[2]); return 0;
}

/*
\brief unhide the character
\author Xanathar
\since 0.10
\param 1: the character
\return 0 or INVALID if not valid character
*/
NATIVE(_unhide)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->unHide();
	return 0;
}

/*
\brief set the multi serial
\author Xanathar
\since 0.10
\param 1: the character
\param 2: the multi serial
\return 0 or INVALID if not valid character
\warning use carefully
*/
NATIVE(_setMultiSerial)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->setMultiSerial(params[2]);
	return 0;
}

/*
\brief set the owner serial
\author Xanathar
\since 0.10
\param 1: the character
\param 2: the owner serial
\return 0 or INVALID if not valid character
*/
NATIVE(_setOwnSerial)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->setOwnerSerial32(params[2]);
	return 0;
}

/*
\brief set the owner serial ( without updating the internal sctructure )
\author Xanathar
\since 0.54
\param 1: the character
\param 2: the owner serial
\return 0 or INVALID if not valid character
*/
NATIVE(_setOwnSerialOnly)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	pc->setOwnerSerial32Only(params[2]);
	return 0;
}

/*
\brief add given item at the given cantainer in the given position
\author Xanathar
\since 0.10
\param 1 the container
\param 2 the item
\param 3 x location or INVALID if rand pos
\param 4 y location or INVALID if rand pos
\return true or false
*/
NATIVE(_contAddItem)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, false );
	P_ITEM pi1 = pointers::findItemBySerial(params[2]);
	VALIDATEPIR( pi1, false );
	return pi->AddItem(pi1, params[3],params[4]);
}


/*
\brief get the item color
\author Xanathar
\since 0.10
\param 1: the item
\return the color or INVALID if not valid item
*/
NATIVE(_color)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	return pi->getColor();
}

/*
\brief Pile an item into container
\author Xanathar
\since 0.10
\param 1: container
\param 2: item
\return true or false
*/
NATIVE(_contPileItem)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, false );
	P_ITEM pi1 = pointers::findItemBySerial(params[2]);
	VALIDATEPIR( pi1, false );
	return pi->AddItem(pi1);
}

/*
\brief Count item by id and eventually color in the given container
\author Xanathar
\since 0.10
\param 1: container
\param 2: item
\param 3: color
\return item count or INVALID if not valid item
*/
NATIVE(_contCountItems)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	return pi->CountItems(params[2], params[3]);
}

/*
\brief Count item by ScriptID
\author Keldan
\since 0.82
\param 1: container
\param 2: item ScriptID
\return item count or INVALID if not valid item
*/
NATIVE(_countItemsByID)
{
   P_ITEM pi = pointers::findItemBySerial(params[1]);
   VALIDATEPIR( pi, INVALID );
   return pi->CountItemsByID (params[2], true);
}

/*NATIVE(_contCountColoredItems)
{	return items[params[1]].CountItems(params[2], params[3]); }
*/

/*
\brief Delete amount of item in the given container by id and eventually color
\author Xanathar
\since 0.10
\param 1: container
\param 2: amount
\param 3: id
\param 4: color
\return item count or INVALID if not valid item
*/
NATIVE(_contDelAmount)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	return pi->DeleteAmount(params[2], params[3],params[4]);
}

/*
\brief Delete amount of item in the given container by ScriptID
\author Keldan
\since 0.82
\param 1: container
\param 2: amount
\param 3: ScriptID
\return item count or INVALID if not valid item
*/
NATIVE(_DelAmountByID)
{
   P_ITEM pi = pointers::findItemBySerial(params[1]);
   VALIDATEPIR( pi, INVALID );
   return pi->DeleteAmountByID(params[2], params[3]);
}

/*
\brief move given item to specified position
\author Xanathar
\since 0.10
\param 1: item
\param 2: x location
\param 3: y location
\param 4: z location
\return 0 or INVALID if not valid item
*/
NATIVE(_imoveto)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	pi->MoveTo( params[2], params[3],params[4] );
	pi->Refresh();
	return 0;
}

/*
\brief pile an item into another
\author Xanathar
\since 0.10
\param 1: first item
\param 2: second item
\return false if not piled, true else
\note if piled can be first item is deleted
*/
NATIVE(_pileitem)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, false );
	P_ITEM pi1 = pointers::findItemBySerial(params[2]);
	VALIDATEPIR( pi1, false );
	return pi->PileItem(pi1);
}

/*
\brief reduce the amount of the given pileable item
\author Xanathar
\since 0.10
\param 1: container
\param 2: item
\return rest count or INVALID if not valid item
*/
NATIVE(_reduceamount)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	return pi->ReduceAmount(params[2]);
}

/*
\brief set the container serial
\author Xanathar
\since 0.10
\param 1: item
\param 2: serial
\return 0 or INVALID  if not valid item
*/
NATIVE(_isetContSerial)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	pi->setContSerial(params[2]);
	return 0;
}

/*
\brief set the container serial without done the appropriate remove aperation
\author Xanathar
\since 0.54
\param 1: item
\param 2: serial
\return 0 or INVALID  if not valid item
\warning use carefully
*/
NATIVE(_isetContSerialonly)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	pi->setContSerial(params[2], false, false);
	return 0;
}

/*
\brief Shows the given container to the chr passed 
\author Wintermute
\since 0.82
\param 1: container serial
\param 2: character serial
*/

NATIVE(_itm_showContainer)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR(pi, INVALID);
	if ( pi->isContainer() )
	{
		P_CHAR pc=pointers::findCharBySerial(params[2]);
		wearIt(pc->getSocket(), pi);
		pc->showContainer(pi);
		// pc->objectdelay=0;
	}
	return 0;
}


/*
\brief set the multi serial
\author Xanathar
\since 0.10
\param 1: item
\param 2: serial
\return 0 or INVALID  if not valid item
*/
NATIVE(_isetMultiSerial)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	pi->SetMultiSerial(params[2]);
	return 0;
}

/*
\brief set the owner serial
\author Xanathar
\since 0.10
\param 1: item
\param 2: serial
\return 0 or INVALID  if not valid item
*/
NATIVE(_isetOwnSerial)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	pi->setOwnerSerial32(params[2]); return 0;
}

/*
\brief set the owner serial without done the appropriate remove aperation
\author Xanathar
\since 0.54
\param 1: item
\param 2: serial
\return 0 or INVALID  if not valid item
\warning use carefully
*/
NATIVE(_isetOwnSerialOnly)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	pi->setOwnerSerial32Only(params[2]);
	return 0;
}

/*
\brief set the item serial
\author Xanathar
\since 0.10
\param 1: item
\param 2: new serial
\return 0 or INVALID  if not valid item
\warning use carefully
*/
NATIVE(_isetSerial)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	pi->setSerial32(params[2]);
	return 0;
}

/*
\brief create a new item from the script
\author Xanathar
\since 0.54
\param 1 itemtype
\param 2 cont
\param 3 amount ( if INVALID use xss default )
\return created item or INVALID if not valid item
\note it's up to you to position the item in the world
*/
NATIVE(_itm_createFromScript)
{
	P_OBJECT po = NULL;
	if ( params[2] != INVALID )
		po = objects.findObject( params[2] );

    P_ITEM pi = item::CreateFromScript(params[1], po, params[3]);
	VALIDATEPIR(pi, INVALID);
    pi->Refresh();
	return pi->getSerial32();
}

/*
\brief create a new item from the script
\author Luxor
\param 1 scriptid
\param 2 container to put into
\param 3 amount ( if INVALID use xss default )
\since 0.82
*/
NATIVE(_itm_createByDef)
{
	P_OBJECT po = NULL;
	if ( params[2] != INVALID )
		po = objects.findObject( params[2] );

        P_ITEM pi = NULL;
	cell *cstr;
	amx_GetAddr( amx, params[1], &cstr );
	printstring( amx, cstr, params+1, (int)(params[0]/sizeof(cell))-1 );
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';

	pi = item::CreateFromScript( g_cAmxPrintBuffer, po, params[3] );
	g_nAmxPrintPtr = 0;
	if ( ISVALIDPI( pi ) )
		return pi->getSerial32();

	return INVALID;
}

/*
\brief create a new item into backpack
\author Endymion
\param 1 scriptid
\param 2 character
\param 3 amount ( if INVALID use xss default )
\since 0.82
*/
NATIVE(_itm_createInBp)
{
	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );

	P_ITEM pi = item::CreateFromScript( params[1], pc->getBackpack(), params[3] );

	return ISVALIDPI( pi )? pi->getSerial32() : INVALID;

}

/*
\brief create a new item into backpack
\author Endymion
\param 1 scriptid
\param 2 character
\param 3 amount ( if INVALID use xss default )
\since 0.82
*/
NATIVE(_itm_createInBpDef)
{
	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );

	cell *cstr;
	amx_GetAddr( amx, params[1], &cstr );
	printstring( amx, cstr, params+1, (int)(params[0]/sizeof(cell))-1 );
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';

	P_ITEM pi = item::CreateFromScript( g_cAmxPrintBuffer, pc->getBackpack(), params[3] );
	g_nAmxPrintPtr = 0;
	return ISVALIDPI( pi )? pi->getSerial32() : INVALID;

}

/*
\brief create a new item into bank
\author Endymion
\param 1 scriptid
\param 2 character
\param 3 amount ( if INVALID use xss default )
\since 0.82
*/
NATIVE(_itm_createInBank)
{
	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );

	P_ITEM pi = item::CreateFromScript( params[1], pc->GetBankBox(), params[3] );

	return ISVALIDPI( pi )? pi->getSerial32() : INVALID;

}

/*
\brief create a new item into bank
\author Endymion
\param 1 scriptid
\param 2 character
\param 3 amount ( if INVALID use xss default )
\since 0.82
*/
NATIVE(_itm_createInBankDef)
{
	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );

	cell *cstr;
	amx_GetAddr( amx, params[1], &cstr );
	printstring( amx, cstr, params+1, (int)(params[0]/sizeof(cell))-1 );
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';

	P_ITEM pi = item::CreateFromScript( g_cAmxPrintBuffer, pc->GetBankBox(), params[3] );
	g_nAmxPrintPtr = 0;
	return ISVALIDPI( pi )? pi->getSerial32() : INVALID;

}

/*
\brief Check if given id is a weapon
\author Endymion
\since 0.82
\param 1 the id
\param 2 weapon type
\param 3 weapon type 2 ( or INVALID if not used )
\param 4 weapon type 3 ( or INVALID if not used )
\return true or false
*/
NATIVE(_itm_isWeaponLike)
{
	return isWeaponLike( params[1], static_cast<WEAPONTYPE>(params[2]), static_cast<WEAPONTYPE>(params[3]), static_cast<WEAPONTYPE>(params[4]) );
}

/*
\brief get the backpack of given character
\author Endymion
\param 1 the character
\param 2 if true backpack is created if not exist
\return backpack item or INVALID if not valid char or haven't backpack
*/
NATIVE(_chr_getBackpack)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	P_ITEM pi= pc->getBackpack();
	if( !ISVALIDPI( pi ) ) {
		P_ITEM bp = item::CreateFromScript( "$item_backpack", pc );
		if( ISVALIDPI(bp) )
		{
			pc->packitemserial=bp->getSerial32();
			return bp->getSerial32();
		}
		else
			return INVALID;
	}
	else
		return pi->getSerial32();
}

/*
\brief check if character can keep the equipment worn
\author Xanathar
\since 0.50
\param 1: character
\return 0 or INVALID if not valid char
*/
NATIVE(_chr_CheckEquipment)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->checkEquipement();
    return 0;
}

/*
\brief Check the item for decay
\author Xanathar
\since 0.50
\param 1: item
\return 0 or INVALID if not valid item
*/
NATIVE(_itm_checkDecay)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR(pi, INVALID);
	pi->doDecay();
	return 0;
}

/*
\brief Delete an item
\author Xanathar
\since 0.50
\param 1: item
\return 0 or INVALID if not valid item
*/
NATIVE(_itm_remove)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR(pi, INVALID);
	pi->Delete();
	return 0;
}

/*
\brief Add a new NPC
\author Xanathar
\since 0.50
\param 1: npc
\param 2: x location
\param 3: y location
\param 4: z location
\return npc or INVALID if not valid npc
*/
NATIVE(_chr_addNPC) //addNPC npcnum, x,y,z
{
    P_CHAR pc = npcs::AddNPC(INVALID, NULL, params[1], params[2], params[3], params[4]);
    VALIDATEPCR(pc, INVALID);
    return pc->getSerial32();
}

/*
\brief Add a new NPC from an npc list
\author Wintermute
\since 0.82
\param 1: npc
\param 2: x location
\param 3: y location
\param 4: z location
\return npc or INVALID if not valid npc
*/
NATIVE(_chr_addRandomNPC) //addNPC npcnum, x,y,z
{
	char temp[34];
	sprintf(temp, "%lu", params[1]);
	int npcnum = npcs::AddRandomNPC(INVALID, &temp[0]);
    P_CHAR pc = npcs::AddNPC(INVALID, NULL, npcnum, params[2], params[3], params[4]);
    VALIDATEPCR(pc, INVALID);
    return pc->getSerial32();
}


/*
\brief Remove a character
\author Xanathar
\since 0.50
\param 1: character
\return 0 or INVALID if not valid char
*/
NATIVE(_chr_remove)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->Delete();
    return 0;
}

/*
\brief Calculates character attac power
\author Xanathar
\since 0.50
\param 1: character
\return attac power or INVALID if not valid char
*/
NATIVE(_chr_calcAtt)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->calcAtt();
}

/*
\brief Calculates character defence power of the given body part
\author Xanathar
\since 0.50
\param 1: character
\param 2: body item for calculate
\return defence power or INVALID if not valid char or item
*/
NATIVE(_chr_calcDef)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->calcDef(params[2]);
}

/*
\brief the given item fish
\author Xanathar
\since 0.50
\param 1: character
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_fish)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
	Fishing::Fish (DEREF_P_CHAR(pc));
    return 0;
}

/*
\brief Get the guild the char is a member of if any
\author Sparhawk
\since 0.82.rc3
\param 1: character
\return serial of the guild or 0 if not member of a guild
*/
NATIVE( _chr_getGuild )
{
	P_CHAR pChar = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pChar, INVALID);

	P_GUILD guild = pChar->getGuild();
	return ( guild!=NULL )? guild->getSerial() : INVALID;
}

/*
\brief Set the guild the char is a member of
\author Sparhawk
\since 0.82.rc3
\param 1 character serial
\param 2 guild serial
\note	Not implemented yet...
\return 1
*/
NATIVE( _chr_setGuild )
{
	return 1;
}

/*
\brief the given item fish
\author Xanathar
\since 0.50
\param 1: first character
\param 2: second character
\return 1 if same guild, 2 if guild are in war ( war o chaos/order ), 0 else or INVALID if not valid character
*/
NATIVE(_chr_guildCompare)
{
    P_CHAR pc1 = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc1, INVALID);
    P_CHAR pc2 = pointers::findCharBySerial(params[2]);
    VALIDATEPCR(pc2, INVALID);
    return Guildz.compareGuilds (pc1->getGuild(),pc2->getGuild());
}


/*
\brief given character cast field
\author Xanathar
\since 0.50
\param 1: caster
\param 2: x location
\param 3: y location
\param 4: z location
\param 5: field type ( see note )
\return 0 or INVALID if not valid caster character
\note fiel type:
	0=wall of stone,
	1=fire field,
	2=poison field,
	3=paralyze field,
	4=energy field
*/
NATIVE(_magic_castFieldSpell)
{

	P_CHAR mage = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( mage, INVALID );
	TargetLocation targ(params[2],params[3],params[4]);
	mage->castSpell(static_cast<magic::SpellId>( params[5] ), targ, SPELLFLAG_DONTCHECKSPELLBOOK|SPELLFLAG_DONTCHECKSKILL|SPELLFLAG_SILENTSPELL|SPELLFLAG_NOUSEMANA|SPELLFLAG_DONTREQREAGENTS );
	return 0;

}

/*
\brief Check if given spell is contained in spell book
\author Xanathar
\since 0.50
\param 1: spell book
\param 2: circle
\param 3: spell
\return true or false ( false also if not valid item )
*/
NATIVE(_magic_checkSpellbook)
{
	//return //Magic->CheckBook(params[2], params[3], params[1]);

	return 0;
}

/*
\brief reverses attacker and defender, according to reflection
\author Xanathar
\since 0.50
\param 1: attacker
\param 2: defender
\return true or false ( false also if not valid item )
*/
NATIVE(_magic_checkMagicReflection)
{
/*	int *a, *d;
	amx_GetAddr(amx,params[1], reinterpret_cast<long**>(&a));
	amx_GetAddr(amx,params[2], reinterpret_cast<long**>(&d));
	return //Magic->CheckMagicReflect(*a,*d);
*/
	return 0;
}

NATIVE(_magic_checkMana)
{
	return 0;
}

#define CASTTOPC(ATT,DEF,SPELL) \
 \
	P_CHAR pc_att = pointers::findCharBySerial( ATT ); \
	VALIDATEPCR( pc_att, INVALID ); \
	P_CHAR pc_def = pointers::findCharBySerial( DEF ); \
	VALIDATEPCR( pc_def, INVALID ); \
	TargetLocation targ(pc_def);\
	pc_att->castSpell( SPELL , targ, SPELLFLAG_DONTCHECKSPELLBOOK|SPELLFLAG_DONTCHECKSKILL|SPELLFLAG_SILENTSPELL|SPELLFLAG_NOUSEMANA|SPELLFLAG_DONTREQREAGENTS );\
	return 0; \
\

NATIVE(_magic_castExplosion)
{
	CASTTOPC( params[1], params[2], magic::SPELL_EXPLOSION )
}

NATIVE(_magic_cast)
{
	CASTTOPC( params[1], params[2], static_cast<magic::SpellId>( params[3] ) )
}

NATIVE(_magic_castClumsy)
{
	CASTTOPC( params[1], params[2], magic::SPELL_CLUMSY )
}

NATIVE(_magic_castCurse)
{
	CASTTOPC( params[1], params[2], magic::SPELL_CURSE )
}

NATIVE(_magic_castFeebleMind)
{
	CASTTOPC( params[1], params[2], magic::SPELL_FEEBLEMIND )
}

NATIVE(_magic_castFireball)
{
	CASTTOPC( params[1], params[2], magic::SPELL_FIREBALL )
}

NATIVE(_magic_castFlameStrike)
{
	CASTTOPC( params[1], params[2], magic::SPELL_FLAMESTRIKE )
}

NATIVE(_magic_castHarm)
{
	CASTTOPC( params[1], params[2], magic::SPELL_HARM )
}

NATIVE(_magic_castMagicArrow)
{
	CASTTOPC( params[1], params[2], magic::SPELL_MAGICARROW )
}

NATIVE(_magic_castLightning)
{
	CASTTOPC( params[1], params[2], magic::SPELL_LIGHTNING )
}

NATIVE(_magic_castMindBlast)
{
	CASTTOPC( params[1], params[2], magic::SPELL_MINDBLAST )
}

NATIVE(_magic_castParalyze)
{
	CASTTOPC( params[1], params[2], magic::SPELL_PARALYZE )
}

NATIVE(_magic_castWeaken)
{
	CASTTOPC( params[1], params[2], magic::SPELL_WEAKEN )
}

/*NATIVE(_getSandBoxStatus)
{ return g_nSandBoxLevel; }
*/

/*
\brief Send a complete network packet to given socket
\author Xanathar
\since 0.50
\param 1: character
\param 2: lenght
\param 3: data
\return 0 or INVALID if not valid socket
*/
NATIVE(_send_send)
{

	P_CHAR pc = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pc, false );

	NXWCLIENT ps = pc->getClient();
	if ( ps == NULL )
		return false;


	char *buf;
	cell *cstr;

	buf = new char[params[2]];

	amx_GetAddr(amx,params[3],&cstr);

	for (int i=0; i<params[2]; i++)
	{
		buf[i] = static_cast<char>(cstr[i] & 0xFF);
	}

	Xsend( ps->toInt(), buf, params[2]);
	return true;
}


/*
\brief Send an attack stuff
\author Xanathar
\since ??
\param 1: first character
\param 2: second character
\return 0 or INVALID if not valid character/s
*/
NATIVE(_chr_attackStuff)
{
    P_CHAR pc1 = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc1, INVALID);
    P_CHAR pc2 = pointers::findCharBySerial(params[2]);
    VALIDATEPCR(pc2, INVALID);
    if (pc2->getClient() != NULL) {
		pc2->attackStuff( pc1 );
        return 0;
    } else {
        return INVALID;
    }
}


/*
\brief Send an attack confirmation to given socket for the given character
\author Xanathar
\since 0.50
\param 1: character who
\param 2: character to
\return 0 or INVALID if not valid socket or character
*/
NATIVE(_send_confirmAttack)
{
    P_CHAR pc = pointers::findCharBySerial(params[2]);
    VALIDATEPCR( pc, INVALID );

	P_CHAR who = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( who, INVALID );

	NXWCLIENT ps = who->getClient();
	if ( ps == NULL )
		return INVALID;

    SndAttackOK( ps->toInt(), pc->getSerial32() );
	return 0;
}

/*
\brief test if status request from client is for character stats
\author Sparhawk
\since 0.82
\return true or false
*/
NATIVE(_rcve_statsRequest )
{
	return ( buffer[params[1]][5] == 4 );
}

/*
\brief test if status request from client is for character skills
\author Sparhawk
\since 0.82
\return true or false
*/
NATIVE(_rcve_skillsRequest )
{
	return ( buffer[params[1]][5] == 5 );
}


/*
\brief Activate a temp effect
\author Xanathar
\since 0.53
\param 1 type
\param 2 source object
\param 3 destination object
\param 4 more
\param 5 duration
\param 6 callback
\return 0
\note if type is 121 a custom tempeffect are created ana callback must be the index
	to a temp effect callback function. Else a native tempeffect is called
\note olny the lowest 24 bit of more are valid
*/
NATIVE(_tempfx_activate)
{
/*	tempeffect(params[2], params[3], params[1], (params[4])&0xFF, (params[4]>>8)&0xFF,
		(params[4]>>16)&0xFF, params[5], params[6]);*/
	//<Luxor>
	P_OBJECT src = NULL, dest = NULL;
	if ( isCharSerial(params[2]) )
	{
		src = (P_OBJECT)pointers::findCharBySerial(params[2]);
	}
	else if ( isItemSerial(params[2]) )
	{
		src = (P_OBJECT)pointers::findItemBySerial(params[2]);
	}
	if ( isCharSerial(params[3]) )
	{
		dest = (P_OBJECT)pointers::findCharBySerial(params[3]);
	}
	else if ( isItemSerial(params[3]) )
	{
		dest = (P_OBJECT)pointers::findItemBySerial(params[3]);
	}

	tempfx::add(src, dest, params[1], (params[4])&0xFF, (params[4]>>8)&0xFF,
		(params[4]>>16)&0xFF, (params[4]>>24)&0xFF,params[5], params[6]);
	//</Luxor>
	return 0;
}

/*
\brief Delete a temp effect
\author Endymion
\since 0.82
\param 1 character
\param 2 type
\param 3 execute expire code
\param 4 function callback index if type is a custom tempeffect
\return 0, INVALID if error
*/
NATIVE(_tempfx_delete)
{
	P_OBJECT src = NULL;
	if ( isCharSerial(params[2]) )
	{
		src = (P_OBJECT)pointers::findCharBySerial(params[2]);
		if ( ! ISVALIDPC(src) )
			return 1;
	}
	else if ( isItemSerial(params[2]) )
	{
		src = (P_OBJECT)pointers::findItemBySerial(params[2]);
		if ( ! ISVALIDPI(src) )
			return 1;
	}

	src->delTempfx( params[2], params[3], params[4] );
	return 0;
}

/*
\brief Check if a tempfx is active
\author Endymion
\since 0.82
\param 1 character
\param 2 type
\param 3 function callback index if type is a custom tempeffect
\return 1 if exist, 0 if not, INVALID if error
*/
NATIVE(_tempfx_isActive)
{
	P_OBJECT src = NULL;

	if ( isCharSerial(params[1]) )
	{
		src = (P_OBJECT)pointers::findCharBySerial(params[1]);
		if ( ! ISVALIDPC( (P_CHAR) src) )
			return 0;
	}
	else if ( isItemSerial(params[1]) )
	{
		src = (P_OBJECT)pointers::findItemBySerial(params[1]);
		if ( ! ISVALIDPI((P_ITEM) src) )
			return 0;
	}

	return ( src->getTempfx( params[2], params[3] ) != NULL )? 1 : 0;

}

/*
\brief Flee from target
\author Endymion
\since 0.82
\param 1 npc
\param 2 from character
\param 3 seconds
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_flee)
{
	P_CHAR npc=pointers::findCharBySerial( params[1] );
	VALIDATEPCR( npc, INVALID );

	P_CHAR from=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( from, INVALID );

	npc->flee( from, params[3] );
	return 0;
}

/*
\brief Get an event handler name from a given character
\author Sparhawk
\since 0.82
\param 1: character
\param 2: event
\param 3: string into which event handler name is returned
\return 0 or INVALID if not valid character else return length of param[4]
*/
NATIVE(_chr_getEventHandler)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
  VALIDATEPCR(pc, INVALID);
  if( params[2] < 0 || params[2] >= ALLCHAREVENTS )
  	return INVALID;

	char str[100];
	cell *cptr;

	if( pc->amxevents[params[2]] != NULL )
		strcpy(str, pc->amxevents[params[2]]->getFuncName() );
	else
			str[0] = '\0';
	/*
	AmxEvent* event = pc->getAmxEvent( params[2] );
	if( event )
		strcpy(str, event->getFuncName() );
	else
			str[0] = '\0';
	*/
	amx_GetAddr(amx,params[3],&cptr);
	amx_SetString(cptr,str, g_nStringMode);
	return strlen(str);
}
/*
\brief Set an event handler to given character
\author Xanathar
\since 0.50
\param 1: character
\param 2: event
\param 3: type ( EVENTTYPE_STATIC or EVENTTYPE_DYNAMIC, see nxw.inc )
\param 4: format
\return 0 or INVALID if not valid character
\note EVENTTYPE_DYNAMIC are not kept across server restart
*/
NATIVE(_chr_setEventHandler)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);

	cell *cstr;

	amx_GetAddr(amx,params[4],&cstr);
	printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	pc->amxevents[params[2]] = newAmxEvent(g_cAmxPrintBuffer,params[3]!=0);
	//pc->delAmxEvent( params[2] );
	//pc->setAmxEvent( params[2], g_cAmxPrintBuffer, params[3] !=0 );
	g_nAmxPrintPtr=0;
	return 0;
}
/*
\brief Get an event handler name from a given item
\author Sparhawk
\since 0.82
\param 1: item
\param 2: event
\param 3: string into which event handler name is returned
\return 0 or INVALID if not valid character else return length of param[4]
*/
NATIVE(_itm_getEventHandler)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
  VALIDATEPIR(pi, INVALID);
  if( params[2] < 0 || params[2] >= ALLITEMEVENTS )
  	return INVALID;

	char str[100];
	cell *cptr;

	if( pi->amxevents[params[2]] != NULL )
		strcpy(str, pi->amxevents[params[2]]->getFuncName() );
	else
			str[0] = '\0';
	/*
	AmxEvent* event = pi->getAmxEvent( params[2] );
	if( event )
		strcpy(str, event->getFuncName() );
	else
		str[0] = '\0';
	*/
	amx_GetAddr(amx,params[3],&cptr);
	amx_SetString(cptr,str, g_nStringMode);
	return strlen(str);
}
/*
\brief Set an event handler to given item
\author Xanathar
\since 0.50
\param 1: item
\param 2: event
\param 3: type ( EVENTTYPE_STATIC or EVENTTYPE_DYNAMIC, see nxw.inc )
\param 4: format
\return 0 or INVALID if not valid item
\note EVENTTYPE_DYNAMIC are not kept across server restart
*/
NATIVE(_itm_setEventHandler)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR(pi, INVALID);

	cell *cstr;

	amx_GetAddr(amx,params[4],&cstr);
	printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	pi->amxevents[params[2]] = newAmxEvent(g_cAmxPrintBuffer,params[3]!=0);
	//pi->delAmxEvent( params[2] );
	//pi->setAmxEvent( params[2], g_cAmxPrintBuffer, params[3] != 0 );
	g_nAmxPrintPtr=0;
	return 0;
}

/*
\brief Delete event from given character
\author Xanathar
\since ??
\param 1: character
\param 2: event
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_delEventHandler)
{
	//we don't clear the event in other ways since it's contained in amxcback hash
	//queues :)
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	pc->amxevents[params[2]] = NULL;
	//pc->delAmxEvent( params[2] );
	return 0;
}

/*
\brief Delete event from given item
\author Xanathar
\since ??
\param 1: item
\param 2: event
\return 0 or INVALID if not valid item
*/
NATIVE(_itm_delEventHandler)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR(pi, INVALID);
	pi->amxevents[ params[2] ]=NULL;
	//pi->delAmxEvent( params[2] );
	return 0;
}

/*
\brief send a movign effect to a given character to another
\author Xanathar
\since 0.53
\param 1: source character
\param 2: destination character
\param 3: id ( artwork gump of effect )
\param 4: speed
\param 5: loop
\param 6: explode
\return 0 or INVALID if not valid character/s
*/
NATIVE (_send_movingfx)
{
    P_CHAR pc1 = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc1, INVALID);
    P_CHAR pc2 = pointers::findCharBySerial(params[2]);
    VALIDATEPCR(pc2, INVALID);

    movingeffect(DEREF_P_CHAR(pc1), DEREF_P_CHAR(pc2), (params[3]>>8)&0xFF, params[3]&0xFF, params[4], params[5],
		params[6]);

	return 0;
}


/*
\brief send a stat update to given character
\author Xanathar
\since 0.53
\param 1: character
\return 0 or INVALID if not valid character
\note not needed if you use chr_setProperty
*/
NATIVE (_send_statUpdate)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->updateStats(STAT_HP);
    pc->updateStats(STAT_MANA);
    pc->updateStats(STAT_STAMINA);
    return 0;
}


/*
\brief Does all the stuff involved in a player helping another ( like healing criminal as such )
\author Xanathar
\since 0.53
\param 1: target character
\param 2: helper character
\return 0 or INVALID if not valid character/s
*/
NATIVE (_chr_helpStuff)
{
    P_CHAR pc1 = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc1, INVALID);
    P_CHAR pc2 = pointers::findCharBySerial(params[2]);
    VALIDATEPCR(pc2, INVALID);
	pc2->helpStuff(pc1);
	return 0;
}


/*
\brief Set the weather in a region
\author Xanathar
\since 0.53
\param 1: region
\param 2: weather type
\return old weather type
*/
NATIVE (_rgn_setWeather)
{

	int oldw = region[params[1]].wtype;
	region[params[1]].wtype = params[2];

	NxwSocketWrapper sw;
	sw.fillOnline();

	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if(ps!=NULL) {
			weather(ps->toInt(),0);
			pweather(ps->toInt());
		}
	}
	return oldw;
}

/*
\brief Get the weather in a region
\author Xanathar
\since 0.53
\param 1: region
\return weather type
*/
NATIVE (_rgn_getWeather)
{
	return region[params[1]].wtype;
}

/*
\brief Get the guarded status of a region
\author Xanathar
\since 0.53
\param 1: region
\return guarded
*/
NATIVE (_rgn_getGuarded)
{
	return region[params[1]].priv & 0x01;
}

/*
\brief Set the guarded status of a region
\author Sparhawk
\since 0.82
\param 1: region
\param 2: true/false for guarded status
*/
NATIVE (_rgn_setGuarded)
{
	if( params[2] )
		region[params[1]].priv |= 0x01;
	else
		region[params[1]].priv &= ~0x01;
	return 1;
}

/*
\brief Check if mark is allowed
\author Xanathar
\since 0.53
\param 1: region
\return can mark
*/
NATIVE (_rgn_getMarkAllowed)
{
	return region[params[1]].priv & 0x02;
}

/*
\brief Set the mark status of a region
\author Sparhawk
\since 0.82
\param 1: region
\param 2: true/false mark status
*/
NATIVE (_rgn_setMarkAllowed )
{
	if( params[2] )
		region[params[1]].priv |= 0x02;
	else
		region[params[1]].priv &= ~0x02;
	return 1;
}

/*
\brief check if gate is allowed
\author Xanathar
\since 0.53
\param 1: region
\return can gate
*/
NATIVE (_rgn_getGateAllowed)
{
	return region[params[1]].priv & 0x04;
}

/*
\brief Set the gate status of a region
\author Sparhawk
\since 0.82
\param 1: region
\param 2: true/false gate status
*/
NATIVE (_rgn_setGateAllowed )
{
	if( params[2] )
		region[params[1]].priv |= 0x04;
	else
		region[params[1]].priv &= ~0x04;
	return 1;
}

/*
\brief check if recall is allowed
\author Xanathar
\since 0.53
\param 1: region
\return can recall
*/
NATIVE (_rgn_getRecallAllowed )
{
	return region[params[1]].priv & 0x08;
}

/*
\brief Set the recall status of a region
\author Sparhawk
\since 0.82
\param 1: region
\param 2: true/false recall status
*/
NATIVE (_rgn_setRecallAllowed )
{
	if( params[2] )
		region[params[1]].priv |= 0x08;
	else
		region[params[1]].priv &= ~0x08;
	return 1;
}

/*
\brief check if magic damage region
\author Xanathar
\since 0.53
\param 1: region
\return no magic damage
*/
NATIVE (_rgn_getNoMagicDamage )
{
	return region[params[1]].priv & 0x40;
}

/*
\brief Set the magic damage status of a region
\author Sparhawk
\since 0.82
\param 1: region
\param 2: true/false magic damage status
*/
NATIVE (_rgn_setNoMagicDamage )
{
	if( params[2] )
		region[params[1]].priv |= 0x040;
	else
		region[params[1]].priv &= ~0x040;
	return 1;
}

/*
\brief check if region has been loaded from regions.xss
\author Sparhawk
\since 0.82
\param 1: region
\return true or false
*/
NATIVE (_rgn_isValid )
{
	return region[params[1]].inUse;
}

/*
\brief Get region name
\author Sparhawk
\since 0.82
\param 1: region
\param 2: string into which event handler name is returned
\return 0 or INVALID if not valid character else return length of param[4]
*/
NATIVE( _rgn_getName )
{
	if( params[1] < 0 || params[1] > 255 )
		return INVALID;

	char str[100];
	cell *cptr;
	str[0] = '\0';
	strcpy( str, region[params[1]].name );
	amx_GetAddr( amx, params[2], &cptr);
	amx_SetString( cptr, str, g_nStringMode);
	return strlen(str);
}

/*
\brief Get region number
\author Wintermute
\since 0.82
\param 1: x koordinate
\param 2: y koordinate
\return region or INVALID if not valid character else return length of param[4]
*/
NATIVE( _rgn_getRegion )
{
	return calcRegionFromXY(params[1], params[2]);
}

/*
\brief Respawn all spawns in region
\author Wintermute
\since 0.82
\param 1: the region
\param 2: the spawning effect 
\return false if invalid region, true otherwise
*/
NATIVE( _region_respawn )
{
	if ( ! region[params[1]].inUse )
		return false;
	Spawns->doRegionSpawn(params[1],static_cast<SPAWNFLAG_ENUM>( params[2]), params[3]);
	return true;
}

/*
\brief Clear all spawns in region
\author Wintermute
\since 0.82
\param 1: x koordinate
\param 2: y koordinate
\return region or INVALID if not valid character else return length of param[4]
*/
NATIVE( _rgn_clearSpawn )
{
	if ( ! region[params[1]].inUse )
		return false;
	Spawns->doRegionClearSpawn(params[1],static_cast<SPAWNTYPE_ENUM>( params[2]));
	return true;
}
/*
\brief send a bolt effect
\author Xanathar
\since 0.53
\param 1: character
\return 0 or INVALID if not valid character
*/
NATIVE (_send_boltfx)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	pc->boltFX(true); 
	return 0;
}

/*
\brief Send a quest arrow
\author Endymion
\param 1 character
\param 2 active
\param 3 x location
\param 4 y location
\return 0 or INVALID if not valid character
*/
NATIVE (_send_questArrow)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	cPacketQuestArrow pkg;
	pkg.active=params[2];
	pkg.x=params[3];
	pkg.y=params[4];
	pkg.send( pc->getClient() );
	return 0;
}

/*
\brief given character do the specified action
\author Xanathar
\since 0.53
\param 1: character
\param 2: action
\return 0 or INVALID if not valid character
\note like the ACTION command or the ACT statement in trigger
*/
NATIVE (_chr_action)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
	pc->playAction(params[2]);
	return 0;
}


/*
\brief Check the given skill in character
\author Xanathar
\since 0.53
\param 1 character
\param 2 skill
\param 3 minimun skill required
\param 4 maximum skill required
\return  0 failed, 1 right or INVALID if not valid character or socket
\bug are used params[5] but not documented
*/
NATIVE (_chr_checkSkill)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->checkSkill( static_cast<Skill>(params[2]), params[3], params[4], params[5]!=0);
}


/*
\brief Un mount the given character
\author Xanathar
\since 0.53
\param 1 character
\return 0 or INVALID if not valid character
*/
NATIVE (_chr_unmountHorse)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->unmountHorse();
	return 0;
}

/*
\brief The character mount the given animal
\author Xanathar
\since 0.53
\param 1 socket
\param 2 animal
\return 0 or INVALID if not valid character/s
*/
NATIVE (_chr_mountHorse)
{
    NXWCLIENT s = getClientFromSocket(params[1]);
	if(s==NULL) return INVALID;
    P_CHAR pc = s->currChar();
    VALIDATEPCR(pc, INVALID);
    P_CHAR horse = pointers::findCharBySerial(params[2]);
    VALIDATEPCR(horse, INVALID);

	pc->mounthorse( horse );
	return 0;
}

/*
\brief Given character play specified sound
\author Xanathar
\since 0.53
\param 1 character
\param 2 sound
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_sound)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->playSFX(params[2]);
	return 0;
}

/*
\brief Play a sound effect on a item
\author Xanathar
\since 0.54
\param 1 item
\param 2 sound
\return 0 or INVALID if not valid item
*/
NATIVE(_itm_sound)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
    VALIDATEPIR(pi, INVALID);
	soundeffect3(pi, params[2]);
	return 0;
}

/*
\brief Open a set
\author Endymion
\since 0.82
\return the set
*/
NATIVE( _set_create )
{
	return amxSet::create( );
}

/*
\brief Close a set
\author Endymion
\since 0.82
\param 1 the set
\return 0
*/
NATIVE( _set_delete )
{
	amxSet::deleteSet(params[1]);
	return 0;
}

/*
\brief Clear a set
\author Endymion
\since 0.82
\param 1 the set
\return 0
*/
NATIVE( _set_clear )
{
	return 0;
}

/*
\brief Rewind a set
\author Endymion
\since 0.82
\param 1 the set
\return 0
*/
NATIVE( _set_rewind )
{
	amxSet::rewind(params[1]);
	return 0;
}

/*
\brief Move to next item
\author Endymion
\since 0.82
\param 1 the set
\return 0
*/
NATIVE( _set_next )
{
	amxSet::next(params[1]);
	return 0;
}

/*
\brief Check if at end of the given set
\author Endymion
\since 0.82
\param 1 the set
\return 1 if at end, 0 else
*/
NATIVE( _set_end )
{
	return amxSet::end(params[1]);
}


/*
\brief Get number of element contained in a set
\author Endymion
\since 0.82
\param 1 the set
\return element count
*/
NATIVE ( _set_size )
{
	return amxSet::size( params[1] );
}

/*
\brief Get current object of given set
\author Endymion
\since 0.82
\param 1 the set
\return the object
\note move index to next
*/
NATIVE( _set_get )
{
	return amxSet::get( params[1] );
}

/*
\brief Get current character of given set
\author Endymion
\since 0.82
\param 1 the set
\return the character
\note move index to next
*/
NATIVE( _set_getChar )
{
	P_CHAR pc=pointers::findCharBySerial( amxSet::get( params[1] ) );
	return ISVALIDPC(pc)? pc->getSerial32() : INVALID;
}

/*
\brief Get current item of given set
\author Endymion
\since 0.82
\param 1 the set
\return the item
\note move index to next
*/
NATIVE( _set_getItem )
{
	P_ITEM pi=pointers::findItemBySerial( amxSet::get( params[1] ) );
	return ISVALIDPI(pi)? pi->getSerial32() : INVALID;
}

/*
\brief Add to given set the value
\author Endymion
\since 0.82
\param 1 the set
\param 2 the value
\return 0
*/
NATIVE( _set_add )
{
	amxSet::add( params[1], params[2] );
	return 0;
}

/*
\brief Add to given set the owned npc of given character
\author Endymion
\since 0.82
\param 1 the set
\param 2 the owner
\param 3 include stabled
\param 4 only following
\return 0
*/
NATIVE( _set_addOwnedNpcs )
{
	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );
	amxSet::addOwnedNpcs( params[1], pc, params[3]!=0, params[4]!=0 );
	return 0;
}

/*
\brief Add to given set the npcs near location
\author Endymion
\since 0.82
\param 1 the set
\param 2 the x location
\param 3 the y location
\param 4 distance
\return 0
*/
NATIVE( _set_addNpcsNearXY )
{
	amxSet::addNpcsNearXY( params[1], params[2], params[3], params[4] );
	return 0;
}

/*
\brief Add to given set the npcs near object
\author Endymion
\since 0.82
\param 1 the set
\param 2 the object
\param 4 distance
\return 0
*/
NATIVE( _set_addNpcsNearObj )
{
	if( isCharSerial( params[2] ) ) {
		amxSet::addNpcsNearObj( params[1], pointers::findCharBySerial(params[2]), params[3] );
	}
	else {
		amxSet::addNpcsNearObj( params[1], pointers::findItemBySerial(params[2]), params[3] );
	}
	return 0;
}

/*
\brief Add to given set the party friend of given character
\author Endymion
\since 0.82
\param 1 the set
\param 2 the character
\param 3 distance
\param 4 exclude this
\return 0
*/
NATIVE( _set_addPartyFriend )
{
	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );
	amxSet::addPartyFriend( params[1], pc, params[3], params[4]!=0 );
	return 0;
}

/*
\brief Add to given set the item in given container
\author Endymion
\since 0.82
\param 1 the set
\param 2 item
\param 3 include sub containers
\param 4 only first subcontainer
\return 0
*/
NATIVE( _set_addItemsInCont )
{
	P_ITEM pi=pointers::findItemBySerial( params[2] );
	VALIDATEPIR( pi, INVALID )
	amxSet::addItemsInContainer( params[1], pi, params[3]!=0, params[4]!=0 );
	return 0;
}

/*
\brief Add to given set the items weared by given character
\author Endymion
\since 0.82
\param 1 the set
\param 2 character
\param 3 include like hair
\param 4 include protected layer
\param 5 exclude illegar layer
\return 0
*/
NATIVE( _set_addItemWeared )
{
	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );
	amxSet::addItemWeared( params[1], pc, params[3]!=0, params[4]!=0, params[5]!=0 );
	return 0;
}

/*
\brief Add to given set the items at given location
\author Endymion
\since 0.82
\param 1 the set
\param 2 x
\param 3 y
\param 4 type
\return 0
*/
NATIVE( _set_addItemsAtXY )
{
	amxSet::addItemsAtXY( params[1], params[2], params[3], params[4] );
	return 0;
}

/*
\brief Add to given set the items near given location
\author Endymion
\since 0.82
\param 1 the set
\param 2 x
\param 3 y
\param 4 distance
\param 5 exclude not moveable
\return 0
*/
NATIVE( _set_addItemsNearXY )
{
	amxSet::addItemsNearXY( params[1], params[2], params[3], params[4], params[5]!=0 );
	return 0;
}

/*
\brief Add to given set all online players
\author Endymion
\since 0.82
\param 1 the set
\return 0
\note are added ONLY client chaarcter online
*/
NATIVE( _set_addAllOnlinePl )
{
	amxSet::addAllOnlinePlayers( params[1] );
	return 0;
}

/*
\brief Add to given set all online players near given character
\author Endymion
\since 0.82
\param 1 the set
\param 2 the object ( character or item )
\param 3 exclude this ( only used if object is a char )
\param 4 distance
\return 0
\note are added ONLY client chaarcter online
\note if item is in a subcontainer, is returned the true location, the ocation of in world container
*/
NATIVE( _set_addOnlinePlNearObj )
{
	if( isCharSerial(params[2]) ) {
		P_CHAR pc=pointers::findCharBySerial( params[2] );
		VALIDATEPCR( pc, INVALID );
		amxSet::addOnlinePlayersNearChar( params[1], pc, params[3]!=0, params[4] );
	}
	else {
		P_ITEM pi=pointers::findItemBySerial( params[2] );
		VALIDATEPIR( pi, INVALID );
		amxSet::addOnlinePlayersNearItem( params[1], pi, params[4] );
	}
	return 0;
}


/*
\brief Add to given set all online players near location
\author Endymion
\since 0.82
\param 1 the set
\param 2 x
\param 3 y
\param 4 distance
\return 0
\note are added ONLY client chaarcter online
*/
NATIVE( _set_addOnlinePlNearXY )
{
	amxSet::addOnlinePlayersNearXY( params[1], params[2], params[3], params[4] );
	return 0;
}

/*
\brief Add to given set all guild member
\author Endymion
\since 0.82
\param 1 the set
\param 2 the guild
\return 0
*/
NATIVE( _set_addGuildMembers )
{
	amxSet::addGuildMembers( params[1], params[2] );
	return 0;
}

/*
\brief Add to given set all guild recruit
\author Endymion
\since 0.82
\param 1 the set
\param 2 the guild
\return 0
*/
NATIVE( _set_addGuildRecruit )
{
	amxSet::addGuildRecruits( params[1], params[2] );
	return 0;
}

/*
\brief Add to given set all guild or guild political
\author Endymion
\since 0.82
\param 1 the set
\param 2 the guild ( if INVALID all, else use options )
\return 0
*/
NATIVE( _set_addGuilds )
{
	amxSet::addGuilds( params[1], params[2], params[3] );
	return 0;
}

/*
\brief Add to given set all house co owners
\author Wintermute
\since 0.82
\param 1 the set
\param 2 the house
\return 0
*/
NATIVE( _set_addHouseCoowners )
{
	if ( params[2] == INVALID )
	{
		LogError("Illegal house serial used" );
		return 1;
	}
	amxSet::addHouseCoowners( params[1], params[2] );
	return 0;
}

/*
\brief Add to given set all house friends
\author Wintermute
\since 0.82
\param 1 the set
\param 2 the house
\return 0
*/
NATIVE( _set_addHouseFriends )
{
	if ( params[2] == INVALID )
	{
		LogError("Illegal house serial used" );
		return 1;
	}
	amxSet::addHouseFriends( params[1], params[2] );
	return 0;
}

/*
\brief Add to given set all house bans
\author Wintermute
\since 0.82
\param 1 the set
\param 2 the house
\return 0
*/
NATIVE( _set_addHouseBans )
{
	if ( params[2] == INVALID )
	{
		LogError("Illegal house serial used" );
		return 1;
	}
	amxSet::addHouseBans( params[1], params[2] );
	return 0;
}

/*
\brief Add to given set all house bans
\author Wintermute
\since 0.82
\param 1 the set
\param 2 the house
\return 0
*/
NATIVE( _set_getOwnedHouses )
{
	if ( params[2] == INVALID )
	{
		LogError("Illegal house serial used" );
		return 1;
	}
	if ( ! isCharSerial(params[2]))
	{
		LogError("Illegal char serial used in set_getOwnedHouses" );
		return 1;
	}
	amxSet::addOwnHouses( params[1], params[2] );
	return 0;
}

/*
\brief Add to given set all items inside house
\author Wintermute
\since 0.82
\param 1 the set
\param 2 the house
\param 3 optional a type to filter the returned items
\return 0
*/
NATIVE( _set_getItemsInHouse )
{
	if ( params[2] == INVALID )
	{
		LogError("Illegal house serial used" );
		return 1;
	}
	amxSet::addItemsInsideHouse( params[1], params[2], params[3] );
	return 0;
}


/*
\brief Add to given set all items inside house
\author Wintermute
\since 0.82
\param 1 the set
\param 2 the house
\param 3 optional a type to filter the returned items
\return 0
*/
NATIVE( _set_getItemsOutsideHouse )
{
	if ( params[2] == INVALID )
	{
		LogError("Illegal house serial used" );
		return 1;
	}
	amxSet::addItemsOutsideHouse( params[1], params[2], params[3] );
	return 0;
}

/*
\brief Add to given set all items inside house
\author Wintermute
\since 0.82
\param 1 the set
\param 2 the house
\param 3 optional a type to filter the returned items
\return 0
*/
NATIVE( _set_getCharsInHouse )
{
	if ( params[2] == INVALID )
	{
		LogError("Illegal house serial used" );
		return 1;
	}
	amxSet::addCharsInsideHouse( params[1], params[2] );
	return 0;
}


/*
\brief Add to given set all items inside house
\author Wintermute
\since 0.82
\param 1 the set
\param 2 the house
\param 3 optional a type to filter the returned items
\return 0
*/
NATIVE( _set_getCharsOutsideHouse )
{
	if ( params[2] == INVALID )
	{
		LogError("Illegal house serial used" );
		return 1;
	}
	amxSet::addCharsOutsideHouse( params[1], params[2] );
	return 0;
}

/*!
\brief Add to given set all char serials of an account
\author Endymion
\since 0.82
\param 1 the set
\param 2 onlyActive if true are added only active races
\return 0
*/
NATIVE( _set_addAccountChars )
{
	if (( params[2] == INVALID ) || (params[2] > Accounts->Count() ) )
	{
		LogError("Illegal account number used" );
		return 1;
	}
	amxSet::addAccountChars(params[1], params[2] );
	return 0;
}

/*!
\brief Add to given set all race
\author Endymion
\since 0.82
\param 1 the set
\param 2 onlyActive if true are added only active races
\return 0
*/
NATIVE( _set_addAllRaces )
{
	return 0;
}

/*!
\brief Add to given set all race
\author Endymion
\since 0.82
\param 1 the set
\param 2 race the race
\param 3 prop the property
\param 4 subProp the sub property
\return 0
*/
NATIVE( _set_getRaceStuff )
{
	return 0;
}

/*
\brief Resurrect a character
\author Xanathar
\param 1: the character
\return 0 il resurrect else INVALID
*/
NATIVE(_chr_resurrect)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc,INVALID);
	if (pc->dead && ( pc->IsOnline() || pc->npc ) ) 
	{
		pc->resurrect();
		return 0;
	}
	else
		return INVALID;
}

/*
\brief Kills a character
\author Wintermute
\param 1: the character
\return 0 il resurrect else INVALID
*/
NATIVE(_chr_kill)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc,INVALID);
	if (!pc->dead && ( pc->IsOnline() || pc->npc ) )
	{
		pc->attackerserial=INVALID;
		pc->Kill();
		if ( pc->npc )
			pc->Delete();

		return 0;
	}
	else
		return INVALID;
}


/*
\brief generic npctalk/npcemote handler
\author Sparhawk modified by Frodo
\since 0.60
\param 1 speaker
\param 2 listener
\param 3 version ( RUNIC or EMOTE or _ for plain text )
\param 4 color
\param 5 antispam flag
\param 6 text
\return 0 or INVALID if not valid characters
*/


NATIVE(_chr_speech)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
	P_CHAR pc2 = pointers::findCharBySerial(params[2]);

    VALIDATEPCR(pc, INVALID);

	int flag_all;

	if(!ISVALIDPC(pc2))
		
		flag_all=1;
	
	else 
		
		flag_all=0;


    cell *cstr;
	amx_GetAddr(amx, params[6], &cstr);
	printstring(amx,cstr,params+6,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;

	switch( params[3] )
	{
		case NXW_SPEECH_TALK : 
				
				if(!flag_all) pc->talk( pc2->getSocket(), g_cAmxPrintBuffer, (char) params[5], params[4]);
				pc->talkAll( g_cAmxPrintBuffer, (char) params[6], params[4]);
			
				break;
		
		case NXW_SPEECH_TALK_RUNIC : 
			
				if(!flag_all) pc->talkRunic( pc2->getSocket(), g_cAmxPrintBuffer, (char) params[5]);
				pc->talkAllRunic( g_cAmxPrintBuffer, (char) params[5]);
				
				break;
		
		case NXW_SPEECH_EMOTE : 
			
				if(!flag_all) pc->emote( pc2->getSocket(), g_cAmxPrintBuffer,(char) params[5]);
				pc->emoteall( g_cAmxPrintBuffer, (char) params[6]);

				break;
		
		default : WarnOut("_chr_speech called with invalid params[1] (%d)\n", params[3]);
			break;
	}
	return 0;
}


/*
\brief sends message on top of char visible to one socket only
\author Sparhawk
\since 0.82
\param 1: show to whom
\param 2: show whom
\param 3: message
\param 4: optional color
\return 0 or INVALID if not valid character
*/
NATIVE( _chr_showMessage )
{
	P_CHAR pc1 = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc1, INVALID);
	P_CHAR pc2 = pointers::findCharBySerial(params[2]);
	VALIDATEPCR(pc2, INVALID);

	if (pc1->getClient() != NULL)
	{
		cell *cstr;
		amx_GetAddr(amx, params[3], &cstr);
		amx_GetString(g_cAmxPrintBuffer, cstr);

		NXWSOCKET s = pc1->getClient()->toInt();

		UI08 sysname[30]={ 0x00, };
		strcpy((char *)sysname, "System");

		SendSpeechMessagePkt(s, pc2->getSerial32(), pc2->getId(), 6, params[4], (UI16)pc1->fonttype, sysname, g_cAmxPrintBuffer);
		return 0;
	}
	return INVALID;
}

/*
\brief shows the paperdoll of a player to the character
\author Wintermute
\since 0.82
\param 1: show to whom
\param 2: show whom
\return 0 or INVALID if not valid character
*/
NATIVE( _chr_showPaperdoll )
{
	P_CHAR pc1 = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc1, INVALID);
	P_CHAR pc2 = pointers::findCharBySerial(params[2]);
	VALIDATEPCR(pc2, INVALID);

	if (pc1->getClient() != NULL)
	{
		pc1->showPaperdoll(pc2);
		return 0;
	}
	return INVALID;
}

/*
\brief get distance from two character
\author Luxor
\since 0.82
\param 1: first character
\param 2: second character
\return distance or INVALID if not valid character/s
*/
NATIVE(_chr_distance)
{
    P_CHAR pc1 = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc1, INVALID);
    P_CHAR pc2 = pointers::findCharBySerial(params[2]);
    VALIDATEPCR(pc2, INVALID);

    return pc1->distFrom(pc2);
}

/*
\brief A character Attack another
\author Luxor
\since 0.82
\param 1: attacker
\param 2: target
\return 0 or INVALID if not valid character/s
*/
NATIVE(_chr_npcattack)
{
    P_CHAR pc1 = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc1, INVALID);
    P_CHAR pc2 = pointers::findCharBySerial(params[2]);
    VALIDATEPCR(pc2, INVALID);

    npcattacktarget(pc1, pc2);
	return 0;
}

/*
\brief Update a character
\author Luxor
\since 0.82
\param 1: character
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_updatechar)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	pc->teleport();
	return 0;
}

/*
\brief Play a static effect to a character
\author Luxor
\since 0.82
\param 1: character
\param 2: effect
\return 0 or INVALID if not valid character
*/
NATIVE(_send_staticfx)
{
//Magic->doStaticEffect(params[1], params[2]);
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->staticFX(params[2], 0, 10, NULL);
    return 0;
}

/*
\brief Release NPC
\author Luxor
\since 0.7
\param 1: character
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_npcRelease)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
	if (pc->summontimer)
	{
		pc->summontimer=uiCurrentTime;
	}
	pc->ftargserial=INVALID;
	pc->npcWander=WANDER_FREELY_CIRCLE;
	pc->setOwnerSerial32(-1);
	pc->tamed = false;
	return 0;
}

/*
\brief Check if a character own anothe given character
\author Luxor
\since 0.7
\param 1: character owner
\param 2: character owned
\return 0 if no, 1 if yes or INVALID if not valid character
*/
NATIVE(_chr_owns)
{
//	if (chars[params[1]].Owns(&chars[params[2]]))
    P_CHAR pc1 = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc1, INVALID);
    P_CHAR pc2 = pointers::findCharBySerial(params[2]);
    VALIDATEPCR(pc2, INVALID);

    if( pc1->isOwnerOf(pc2) )
		return 1;
	return 0;
}


/*
\brief Teleport a character
\author Luxor
\since 0.7
\param 1: character
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_teleport)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	pc->teleport();
	return 0;
}

/*
\brief Refresh an item
\author Luxor
\since 0.7
\param 1: item
\return 0 or INVALID if not valid item
*/
NATIVE(_itm_refresh)
{
    P_ITEM pi = pointers::findItemBySerial(params[1]);
    VALIDATEPIR(pi, INVALID);

    pi->Refresh();
	return 0;
}

/*
\brief Line of sight
\author Luxor
\since 0.7
\param 1: socket
\param 2: x first location
\param 3: y first location
\param 4: z first location
\param 5: x second location
\param 6: y second location
\param 7: z second location
\param 8: items to consider
\return 0 blocked, 1 not blocked or INVALID if not valid character
*/
NATIVE(_chr_lineOfSight)
{
	return line_of_sight( params[2], params[3], params[4], params[5], params[6], params[7], params[8]);
}



/*
\brief return the guild type
\author Sparhawk
\since 0.82a
\param 1: char of whom to get the guild type from
\return guild type
*/
NATIVE( _chr_getGuildType )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
	return pc->GetGuildType();
}
/*
\brief set the guild type
\author Sparhawk
\since 0.82a
\param 1: char id
\param 2: new guild type
\return 0 or INVALID if not valid character
*/
NATIVE( _chr_setGuildType )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
	pc->SetGuildType( static_cast<short>(params[2]) );
	return 0;
}

/*
\brief return the guild traitor status
\author Sparhawk
\since 0.82a
\param 1: char of whom to get the guild traitor status from
\return guild traitor status
*/
NATIVE( _chr_isGuildTraitor )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
	return pc->IsGuildTraitor();
}
/*
\brief set the guild traitor status
\author Sparhawk
\since 0.82a
\param 1: char id
\param 2: new guild traitor status 0 = false 1 = true
\return 0 or INVALID if not valid character
*/
NATIVE( _chr_setGuildTraitor )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
	params[2] ? pc->SetGuildTraitor() :pc->ResetGuildTraitor();
	return 0;
}

/*
\brief return the guild title toggle status
\author Sparhawk
\since 0.82a
\param 1: char of whom to get the guild title status from
\return guild traitor status
*/
NATIVE( _chr_hasGuildToggle )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
	return pc->HasGuildTitleToggle();
}
/*
\brief set the guild title toggle status
\author Sparhawk
\since 0.82a
\param 1: char id
\param 2: new guild title toggle status 0 = false 1 = true
*/
NATIVE( _chr_setGuildToggle )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
	if (params[2])
		pc->SetGuildTitleToggle();
	else
		pc->ResetGuildTitleToggle();
	return 0;
}

/*
\brief return the guild fealty
\author Sparhawk
\since 0.82a
\param 1: char of whom to get the guild fealty from
\return serial: guild fealty
*/
NATIVE( _chr_getGuildFealty )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	return pc->GetGuildFealty();
}
/*
\brief set the guild fealty
\author Sparhawk
\since 0.82a
\param 1: char id
\param 2: serial - new guild fealty
*/
NATIVE( _chr_setGuildFealty )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	pc->SetGuildFealty( static_cast<SERIAL>(params[2]) );
	return 0;
}

/*
\brief return the guild number
\author Sparhawk
\since 0.82a
\param 1: char of whom to get the guild number from
\return guild number
*/
NATIVE( _chr_getGuildNumber )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	return pc->GetGuildNumber();
}
/*
\brief set the guild number
\author Sparhawk
\since 0.82a
\param 1: char id
\param 2: new guild number
*/
NATIVE( _chr_setGuildNumber )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	pc->SetGuildNumber( static_cast<SI32>(params[2]) );
	return 0;
}

/*
\brief return the guild title
\author Sparhawk
\since 0.82a
\param 1: char of whom to get the guild number from
\param 2: title[]
\return guild title length
*/
NATIVE( _chr_getGuildTitle )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	char str[100];
  	cell *cptr;
  	strcpy(str, pc->GetGuildTitle() );

  	amx_GetAddr(amx,params[2],&cptr);
  	amx_SetString(cptr,str, g_nStringMode);

  	return strlen(str);

}
/*
\brief set the guild title
\author Sparhawk
\since 0.82a
\param 1: char id
\param 2: new guild title
*/

NATIVE( _chr_setGuildTitle )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	cell *cstr;
  	amx_GetAddr(amx,params[2],&cstr);
  	printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
  	g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';
  	pc->SetGuildTitle( g_cAmxPrintBuffer );
  	g_nAmxPrintPtr=0;
  	return 0;
}

/*
\brief return the creation day since EPOCH
\author Sparhawk
\since 0.82a
\param 1: char of whom to get the guild number from

\return guild number
*/
NATIVE( _chr_getCreationDay )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	return pc->GetCreationDay();
}

/*
\brief set the creation day
\author Sparhawk
\since 0.82a
\param 1: char id
\param 2: new creation day
*/
NATIVE( _chr_setCreationDay )
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	pc->SetCreationDay( static_cast<TIMERVAL>(params[2]) );
	return 0;
}



/*
\brief Morph a character
\author Luxor
\since 0.82
\param  1: character
\param  2: new body
\param  3: new skin
\param  4: new hair style
\param  5: new hair color
\param  6: new beard style
\param  7: new beard color
\param  8: backup
\param  9: new name
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_morph)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

	pc->polymorph=true;
    // pc->morph( params[2], params[3], params[4], params[5], params[6], params[7], (g_cAmxPrintBuffer[0]!=0)? g_cAmxPrintBuffer : NULL, (params[8] > 0));
	// TODO: FIXME!
	//pc->morph( params[2], params[3]);
    return 0;
}

/*
\brief UnMorph a character
\author Luxor
\since 0.82
\param  1: character
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_unmorph)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
	pc->polymorph=false;
    pc->morph();
    return 0;
}

/*
\brief Speech
\author Anthalir
\since 0.7
\param 1: chr
\param 2: item
\param 3: text
\return 0 or INVALID if not valid character
*/
NATIVE(_itm_speech)
{

	P_CHAR pc = pointers::findCharBySerial( params[1] );

	P_ITEM cur = pointers::findItemBySerial(params[2]);
	VALIDATEPIR(cur,INVALID);

	cell *cstr;
	amx_GetAddr(amx,params[3],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;

	if ( params[1] == INVALID )
	{
		itemtalk(cur,g_cAmxPrintBuffer);	//Numbersix: if char = -1
		return 0;							// =>item speaks to all in range
	}

	UI08 sysname[30]={ 0x00, };
	strcpy((char *)sysname, "System");

	SendSpeechMessagePkt( pc->getSocket(), cur->getSerial32(), 0x0101, 6, 0x0481, 0x0003, sysname, g_cAmxPrintBuffer );

	return 0;
}

/*
\brief given character equip specific item
\author Anthalir
\since 0.7
\param 1 character
\param 2 item
\return equip or INVALID if not valid character or item
*/
NATIVE(_chr_equip)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    P_ITEM pi = pointers::findItemBySerial(params[2]);
    VALIDATEPIR(pi, INVALID);
	int drop[2]= {-1, -1};	// list of items to drop, there no reason for it to be larger
	int curindex= 0;
	if ( ! checkWearable(pc, pi) )
		return INVALID;
	tile_st item;
	data::seekTile( pi->getId(), item );

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
		if ( pi->getSound() == INVALID )
			pc->playSFX( itemsfx(pi->getId()) );
		return pc->Equip( pi );
	}
	else
	{
		if (drop[0] == -1)
		{
			if ( pi->getSound() == INVALID )
				pc->playSFX( itemsfx(pi->getId()) );
			return pc->Equip( pi );
		}
	}

    return INVALID;
}


/*
\brief Item bounce to pack
\author Anthalir
\since 0.7
\param 1: character
\param 2: item
\return ? or INVALID if not valid character or item
*/
NATIVE(_ItemBounceToPack)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    P_ITEM pi = pointers::findItemBySerial(params[2]);
    VALIDATEPIR(pi, INVALID);
	return pc->UnEquip( pi );
}


/*
\brief Launchs the web browser
\author Luxor
\since 0.82
\param 1: character
\param 2: text
\return 0
*/
NATIVE(_weblaunch)
{
	//modified by SlasHeR
	//replaced socket with char
	cell *cstr;

	P_CHAR pc = pointers::findCharBySerial(params[1]);

        VALIDATEPCR(pc, INVALID);

	int s = pc->getSocket();
	// end by SlasHeR
	if (s<0) return 0;
	amx_GetAddr(amx,params[2],&cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';

	weblaunch(s, g_cAmxPrintBuffer);
	g_nAmxPrintPtr=0;
	return 0;
}

/*
\brief System Broadcast
\author Luxor
\since 0.82
\param 1 text
\return 0
*/
NATIVE(_sysbroadcast)
{
	cell *cstr;
	amx_GetAddr(amx,params[1],&cstr);
	printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	sysbroadcast(g_cAmxPrintBuffer);
	g_nAmxPrintPtr=0;
	return 0;
}

/*
\brief Get number of define
\author Endymion
\since 0.82
\param 1 define
\return the define number
*/
NATIVE(_getIntFromDefine) {
	cell *cstr;
	amx_GetAddr(amx,params[1],&cstr);
	printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	int def = xss::getIntFromDefine( g_cAmxPrintBuffer, params[2] );
	g_nAmxPrintPtr=0;
	return def;
}


/*
\brief Poisons an character
\author Luxor
\since 0.82
\param 1: character
\param 2: poison type
\param 3: duration of poison ( secs ). if -1 default duration is used
\return poison or INVALID if not valid character
*/
NATIVE(_chr_poison)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->applyPoison(static_cast<PoisonType>(params[2]), params[3]);
	return params[2];
}

/*
\brief Returns the combat skill used by the item
\author Luxor
\since 0.82
\param 1: item
\return the combat skill used by the item
*/
NATIVE(_itm_getCombatSkill)
{
        P_ITEM pi = pointers::findItemBySerial(params[1]);
        VALIDATEPIR(pi, INVALID);
        return pi->getCombatSkill();
}


/*
\brief Opens a skill makemenu
\author Luxor
\since 0.82
\param 1 the character
\param 2 the number of the make menu to be opened
\param 3 the skill that's using the makemenu
\param 4 the first material
\param 5 the second material
\return 1 if the makemenu was opened, -1 if not.
*/
NATIVE(_chr_skillMakeMenu)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    if (pc->getClient() == NULL) return INVALID;

	Skills::MakeMenu( pc, params[2], params[3], MAKE_ITEM_REF(params[4]), MAKE_ITEM_REF(params[5]) );
    return 1;
}

/*
\brief Hides given character
\author Luxor
\since 0.82
\param 1: character
\param 2: type ( 0=unhide, 1=hide, 2=hidebyspell )
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_hide)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

    switch(params[2])
	{
		case 0:
			pc->unHide();
			break;
		case 1:
			pc->hidden = 1;
			pc->teleport( TELEFLAG_NONE );
			break;
		case 2:
			pc->hideBySpell();
			break;
		default: return 0;
	}
	return 0;
}

/*
\brief Applys damage to a character
\author Luxor
\since 0.82
\param 1: character
\param 2: amount
\param 3: type
\param 4: stat damaged
\return 1 or INVALID if not valid character
*/
NATIVE(_chr_applyDamage)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

    pc->damage(params[2], static_cast<DamageType>(params[3]), static_cast<StatType>(params[4]));
	return 1;
}

/*
\brief Set random name
\author Luxor
\since 0.82
\param 1: character
\param 2: new name
\return 1 or INVALID if not valid character
*/
NATIVE(_chr_setRandomName)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);

	cell *cstr;
	amx_GetAddr(amx,params[2],&cstr);
	printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	std::string value( g_cAmxPrintBuffer );
	pc->setCurrentName( ( cObject::getRandomScriptValue( std::string("RANDOMNAME"), value ) ) );
	g_nAmxPrintPtr=0;
	return 1;
}

/*
\brief made "possessor" character possess "possessed" character
\author Keldan
\since 0.82
\param 1: possessing character
\param 2: possessed character
\return 1 or INVALID if not valid character
*/
NATIVE(_chr_possess)
{
	P_CHAR possessor = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(possessor, INVALID);
	P_CHAR possessed = pointers::findCharBySerial(params[2]);
	VALIDATEPCR(possessed, INVALID);

	possessor->possess(possessed);
	return 1;
}

/*
\brief Get local variable error
\author Sparhawk
\since 0.82
\return the error
*/
NATIVE(_chr_getLocalVarErr)
{
	return amxVS.getError();
}

/*
\brief check if given are a local variable
\author Sparhawk
\since 0.82
\param 1: character
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_chr_isaLocalVar)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	return amxVS.existsVariable( pc->getSerial32(), params[2], params[3] );
}

/*
\brief Delete given local variable
\author Sparhawk
\since 0.82
\param 1: character
\param 2: variable
\return ??
*/
NATIVE(_chr_delLocalVar)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	return amxVS.deleteVariable( pc->getSerial32(), params[2] );
}

/*
\brief Add a local variable ( int type )
\author Sparhawk
\since 0.82
\param 1: character
\param 2: variable id
\param 3: initial value
\return true on success else false
*/
NATIVE(_chr_addLocalIntVar)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	return amxVS.insertVariable( pc->getSerial32(), params[2], params[3] );
}

/*
\brief Get a local variable ( int type )
\author Sparhawk
\since 0.82
\param 1: character
\param 2: variable id
\return true on success else false
*/
NATIVE(_chr_getLocalIntVar)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	SI32 value;
	amxVS.selectVariable( pc->getSerial32(), params[2], value );
	return value;
}

/*
\brief Set a local variable ( int type )
\author Sparhawk
\since 0.82
\param 1: character
\param 2: variable id
\param 3: value
\return true on success else false
*/
NATIVE (_chr_setLocalIntVar)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	return amxVS.updateVariable( pc->getSerial32(), params[2], params[3] );
}

/*
\brief Add a local variable ( int vector )
\author Sparhawk
\since 0.82
\param 1: character
\param 2: variable id
\param 3: number of elements
\param 4: initial value
\return true on success else false
*/
NATIVE( _chr_addLocalIntVec )
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	return amxVS.insertVariable( pc->getSerial32(), params[2], params[3], params[4] );
}

/*
\brief Get a local variable ( int vector )
\author Sparhawk
\since 0.82
\param 1: character
\param 2: variable id
\param 3: index (0 based )
\return value or 0 on error
*/
NATIVE(_chr_getLocalIntVec)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	SI32 value;
	amxVS.selectVariable( pc->getSerial32(), params[2], params[3], value );
	return value;
}

/*
\brief Set a local variable ( int vector )
\author Sparhawk
\since 0.82
\param 1: character
\param 2: variable id
\param 3: 0 based index value
\param 4: value
\return true on success else false
*/
NATIVE (_chr_setLocalIntVec)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	return amxVS.updateVariable( pc->getSerial32(), params[2], params[3], params[4] );
}

/*
\brief Add a local variable ( String type )
\author Sparhawk
\since 0.82
\param 1: character
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_chr_addLocalStrVar)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	cell *cstr;
	amx_GetAddr(amx,params[3],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	LOGICAL success = amxVS.insertVariable( pc->getSerial32(), params[2], g_cAmxPrintBuffer );
	g_nAmxPrintPtr=0;
	g_cAmxPrintBuffer[0] = '\0';
	return success;
}


/*
\brief Get a local variable ( String type )
\author Sparhawk
\since 0.82
\param 1: character
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_chr_getLocalStrVar)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	g_cAmxPrintBuffer[0] = '\0';
	cell *cptr;
	std::string str;
	if( amxVS.selectVariable( pc->getSerial32(), params[2], str ) )
	{
 		strcpy( g_cAmxPrintBuffer, str.c_str() );
		amx_GetAddr( amx, params[3], &cptr );
		amx_SetString( cptr, g_cAmxPrintBuffer, g_nStringMode);
		return true;
	}
	return false;
}

NATIVE( _var_countLocalVar )
{
	return amxVS.countVariable( params[1] );
}

NATIVE( _var_firstLocalVar )
{
	return amxVS.firstVariable( params[1] );
}

NATIVE( _var_nextLocalVar )
{
	return amxVS.nextVariable( params[1], params[2] );
}

/*
\brief Get the size of a local variable
\author Sparhawk
\since 0.82
\param 1: object serial
\param 2: variable id
\param 3: optional 0 based index value for vectors
\return true on success else false
*/
NATIVE ( _var_sizeofLocalVar )
{
	return amxVS.size( params[1], params[2], params[3] );
}

/*
\brief Set a local variable ( String type )
\author Sparhawk
\since 0.82
\param 1: character
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_chr_setLocalStrVar)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	cell *cstr;
	amx_GetAddr(amx,params[3],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	LOGICAL success = amxVS.updateVariable( pc->getSerial32(), params[2], g_cAmxPrintBuffer );
	g_nAmxPrintPtr=0;
	g_cAmxPrintBuffer[0] = '\0';
	return success;
}

/*
\brief Get local varible error
\author Sparhawk
\since 0.82
\return the error
*/
NATIVE(_itm_getLocalVarErr)
{
  return amxVS.getError();
}

/*
\brief Check if given is a local variable
\author Sparhawk
\since 0.82
\param 1: item
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_itm_isaLocalVar)
{
    P_ITEM pi = pointers::findItemBySerial(params[1]);
    VALIDATEPIR(pi, INVALID);
		return amxVS.existsVariable( pi->getSerial32(), params[2], params[3] );
}

/*
\brief Delete given local variable
\author Sparhawk
\since 0.82
\param 1: item
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_itm_delLocalVar)
{
    P_ITEM pi = pointers::findItemBySerial(params[1]);
    VALIDATEPIR(pi, INVALID);
    //return pi->localProperty->deleteVar( params[2], params[3] );
		return amxVS.deleteVariable( pi->getSerial32(), params[2] );
}

/*
\brief Add a local variable ( int type )
\author Sparhawk
\since 0.82
\param 1: item
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_itm_addLocalIntVar)
{
    P_ITEM pi = pointers::findItemBySerial(params[1]);
    VALIDATEPIR(pi, INVALID);
	return amxVS.insertVariable( pi->getSerial32(), params[2], params[3] );
}

/*
\brief Get given local variable ( int type )
\author Sparhawk
\since 0.82
\param 1: item
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_itm_getLocalIntVar)
{
    P_ITEM pi = pointers::findItemBySerial(params[1]);
    VALIDATEPIR(pi, INVALID);
		SI32 value;
		amxVS.selectVariable( pi->getSerial32(), params[2], value );
		return value;
}

/*
\brief Set given local variable ( int type )
\author Sparhawk
\since 0.82
\param 1: item
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE (_itm_setLocalIntVar)
{
    P_ITEM pi = pointers::findItemBySerial(params[1]);
    VALIDATEPIR(pi, INVALID);
		return amxVS.updateVariable( pi->getSerial32(), params[2], params[3] );
}

/*
\brief Add given local variable ( String type )
\author Sparhawk
\since 0.82
\param 1: item
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_itm_addLocalStrVar)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR(pi, INVALID);
	cell *cstr;
	amx_GetAddr(amx,params[3],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	LOGICAL success = amxVS.insertVariable( pi->getSerial32(), params[2], g_cAmxPrintBuffer );
	g_nAmxPrintPtr=0;
	g_cAmxPrintBuffer[0] = '\0';
	return success;
}


/*
\brief Get given local variable ( String type )
\author Sparhawk
\since 0.82
\param 1: item
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_itm_getLocalStrVar)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR(pi, INVALID);
	g_cAmxPrintBuffer[0] = '\0';
	cell *cptr;
	std::string str;
	if( amxVS.selectVariable( pi->getSerial32(), params[2], str ) )
	{
 		strcpy( g_cAmxPrintBuffer, str.c_str() );
		amx_GetAddr( amx, params[3], &cptr );
		amx_SetString( cptr, g_cAmxPrintBuffer, g_nStringMode);
		return true;
	}
	return false;
}

/*
\brief Set given local variable ( String type )
\author Sparhawk
\since 0.82
\param 1: item
\param 2: ??
\param 3: ??
\return ??
*/
NATIVE(_itm_setLocalStrVar)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR(pi, INVALID);
	cell *cstr;
	amx_GetAddr(amx,params[3],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	LOGICAL success = amxVS.updateVariable( pi->getSerial32(), params[2], g_cAmxPrintBuffer );
	g_nAmxPrintPtr=0;
	g_cAmxPrintBuffer[0] = '\0';
	return success;
}

/*
\brief Check if NPC can move in given location
\author Sparhawk
\since 0.82
\param 1: x location
\param 2: y location
\return true or false
*/
NATIVE(_map_canMoveHere)
{
	return canNpcWalkHere( Loc( static_cast<UI32>(params[1]), static_cast<UI32>(params[2]), 0 ) );
}

/*
\brief Get distance from two location
\author Sparhawk
\since 0.82
\param 1: x first location
\param 2: y first location
\param 3: z first location
\param 4: x second location
\param 5: y second location
\param 6: z second location
\return distance
*/
NATIVE(_map_distance)
{
	return cell(dist( params[1], params[2], params[3], params[4], params[5], params[6] ));
}

/*
\brief Get Tile name
\author Luxor
\since 0.82
\param 1: x first location
\param 2: y first location
\param 3: the name
\return INVALID or lenght of name ( name is in param 3 )
*/
NATIVE(_map_getTileName)
{
    if (params[1] < 0 || params[2] < 0) return INVALID;

	staticVector s;
	data::collectStatics( params[1], params[2], s );
    for( UI32 i = 0; i < s.size(); i++ ) {

		tile_st tile;
        if( data::seekTile( s[i].id, tile ) ) {

			char str[100];
  			cell *cptr;
	  		strcpy(str, (char*)(tile.name));

  			amx_GetAddr(amx,params[3],&cptr);
	  		amx_SetString(cptr,str, g_nStringMode);

  			return strlen(str);

		}
    }
    return INVALID;
}

/*
\brief return true if position is located under a static item
\author Keldan
\since 0.82
\param 1: x first location
\param 2: y first location
\param 2: z first location
\return INVALID or true if under a "roof" (any static item in fact) or false
*/
NATIVE(_map_isUnderStatic)
{
	if (params[1] < 0 || params[2] < 0) return INVALID;

	staticVector s;
	data::collectStatics( params[1], params[2], s );
    for( UI32 i = 0; i < s.size(); i++ ) {

		tile_st tile;
		if( data::seekTile( s[i].id, tile ) )
			if( ( tile.height + s[i].z ) >params[3] ) // a roof  must be higher than player's z !
				return true;
	}
	return false;
}

/*
\brief Get Tile ID
\author Keldan
\since 0.82
\param 1: x first location
\param 2: y first location
\param 2: z first location
\return INVALID or ID of tile
*/
NATIVE(_map_getTileID)
{
	if (params[1] < 0 || params[2] < 0) return INVALID;

	staticVector s;
	data::collectStatics( params[1], params[2], s );
    for( UI32 i = 0; i < s.size(); i++ ) {
		if( s[i].z == params[3])
			return s[i].id;
	}
	return INVALID;
}

/*
\brief Get Tile ID
\author Keldan
\since 0.82
\param 1: x first location
\param 2: y first location
\return INVALID or ID of tile
*/
NATIVE(_map_getFloorTileID)
{
    if (params[1] < 0 || params[2] < 0) return INVALID;
    map_st map;
    data::seekMap(params[1], params[2],map);
    return map.id;
}



/*
\brief Get Height of a Map Location
\author Frodo
\since 0.82
\param 1: x of the location
\param 2: y of the location
\return INVALID or z of the location 
*/
NATIVE(_map_getZ)
{
	map_st m;
	if ( !data::seekMap( params[1], params[2], m ) )
		return INVALID;
	return m.z;
}





////////////////////////////////////////////////////////////
/////////////////////// GUILD //////////////////////////////
////////////////////////////////////////////////////////////


/*
\brief Create a new guild
\author Endymion
\since 0.82
\param 1 the serial
\return guild serial or INVALID if error
\note serial is same of guildstone
*/
NATIVE(_guild_create)
{

	P_GUILD guild = Guildz.addGuild( params[1] );
	return ( guild!=NULL )? guild->getSerial() : INVALID;
}

/*
\brief Remove a guild
\author Wintermute
\since 0.82
\param 1 the serial
\note serial is same of guildstone
*/
NATIVE(_guild_remove)
{
	Guildz.removeGuild( params[1] );
	return true;
}

/*
\brief Add a member to a guild
\author Endymion
\since 0.82
\param 1 the guild
\param 2 the new member
\param 3 the rank
\param 4 title toggle
\param 5 title
\return member or INVALID if error
\note member serial is same of player serial
*/
NATIVE(_guild_addMember)
{

	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return INVALID;

	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );


	P_GUILD_MEMBER member = guild->addMember( pc );
	if ( member==NULL)	return INVALID;

	member->rank = params[3];
	member->toggle = static_cast<GUILD_TITLE_TOGGLE>( params[4] );

	std::string title;

	cell *cstr;
	amx_GetAddr(amx,params[5],&cstr);
	printstring(amx,cstr,params+6,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;

	member->title = g_cAmxPrintBuffer;

	return member->serial;

}

/*
\brief A member resign from his guild
\author Endymion
\since 0.82
\param 1 the guild
\param 2 the member
\return true or false if error
*/
NATIVE(_guild_resignMember)
{

	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return false;

	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, false );

	guild->resignMember( pc );
	return true;
}

/*
\brief Find the index of the member within the guild
\author Wintermute
\since 0.82
\param 1 the guild
\param 2 the member
\return integer, index of member
*/
NATIVE(_guild_getMemberIdx)
{
	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return INVALID;

	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );

	return guild->getMemberPosition(pc->getSerial32());
}

/*
\brief Find the member at the given the index within the guild
\author Wintermute
\since 0.82
\param 1 the guild
\param 2 the index of the member
\return serial, serial of member, invalid if none
*/
NATIVE(_guild_memberAtIndex)
{
	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return INVALID;

	return guild->getMemberByIndex( params[2] );
}

/*
\brief Find the index of the member within the guild
\author Wintermute
\since 0.82
\param 1 the guild
\param 2 the member
\return integer, index of member
*/
NATIVE(_guild_recruitIndex)
{
	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return INVALID;

	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );

	return guild->getRecruitIndex(pc->getSerial32());
}

/*
\brief Find the member at the given the index within the guild
\author Wintermute
\since 0.82
\param 1 the guild
\param 2 the index of the member
\return serial, serial of member, invalid if none
*/
NATIVE(_guild_recruitAtIndex)
{
	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return INVALID;

	return guild->getRecruitByIndex( params[2] );
}
/*
\brief Add a recuit to a guild
\author Endymion
\since 0.82
\param 1 the guild
\param 2 the new recruit
\param 3 the recruiter
\return recruit or INVALID if error
\note recruit serial is same of player serial
*/
NATIVE(_guild_addRecruit)
{

	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return INVALID;

	P_CHAR recruit=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( recruit, INVALID );

	P_CHAR recruiter=pointers::findCharBySerial( params[3] );
	VALIDATEPCR( recruiter, INVALID );

	P_GUILD_RECRUIT guild_recruit = guild->addNewRecruit( recruit, recruiter );
	return guild_recruit->serial;
}

/*
\brief Given recruit are refused
\author Endymion
\since 0.82
\param 1 the guild
\param 2 the recruit
\return true or false if error
*/
NATIVE(_guild_refuseRecruit)
{

	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return false;

	P_CHAR recruit=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( recruit, false );

	guild->refuseRecruit( recruit );
	return true;
}

/*!
\brief returns if the two guilds are at war
\author Wintermute
\since 0.82
\fn guild_hasWarWith( const guild, const otherguild )
\param 1 the guild
\param 2 the other guild
*/
NATIVE(_guild_hasWarWith)
{

	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return false;

	P_GUILD guild2 = Guildz.getGuild( params[2] );
	if ( guild2==NULL )	return false;

	if ( guild->hasWarWith(guild2->getSerial()) )
		return true;
	return false;
}

/*!
\brief returns if the first guild has offered peace to another warring guild
\author Wintermute
\since 0.82
\fn guild_hasPeaceWith( const guild, const otherguild )
\param 1 the guild
\param 2 the other guild
*/
NATIVE(_guild_hasPeaceWith)
{

	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return false;

	P_GUILD guild2 = Guildz.getGuild( params[2] );
	if ( guild2==NULL )	return false;

	if ( guild->hasPeaceWith(guild2->getSerial()) )
		return true;
	return false;
}

/*!
\brief returns if the first guild has an alliance with another guild
\author Wintermute
\since 0.82
\fn guild_hasAllianceWith( const guild, const otherguild )
\param 1 the guild
\param 2 the other guild
*/
NATIVE(_guild_hasAllianceWith)
{

	P_GUILD guild = Guildz.getGuild( params[1] );
	if ( guild==NULL )	return false;

	P_GUILD guild2 = Guildz.getGuild( params[2] );
	if ( guild2==NULL )	return false;

	if ( guild->hasAllianceWith(guild2->getSerial()) )
		return true;
	return false;
}







/*
\brief Log a message
\author Sparhawk
\since 0.82
\param 1: Format
\return 0
*/
NATIVE(_log_message)
{
  cell *cstr;
  amx_GetAddr(amx,params[1],&cstr);
  printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
  g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
  LogMessage(g_cAmxPrintBuffer);
  g_nAmxPrintPtr=0;
  return 0;
}

/*
\brief Log a warning
\author Sparhawk
\since 0.82
\param 1: Format
\return 0
*/
NATIVE(_log_warning)
{
  cell *cstr;
  amx_GetAddr(amx,params[1],&cstr);
  printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
  g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
  LogWarning(g_cAmxPrintBuffer);
  g_nAmxPrintPtr=0;
  return 0;
}

/*
\brief Log an error
\author Sparhawk
\since 0.82
\param 1: Format
\return 0
*/
NATIVE(_log_error)
{
  cell *cstr;
  amx_GetAddr(amx,params[1],&cstr);
  printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
  g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
  LogError(g_cAmxPrintBuffer);
  g_nAmxPrintPtr=0;
  return 0;
}

/*
\brief Log a critical error
\author Sparhawk
\since 0.82
\param 1: Format
\return 0
*/
NATIVE(_log_critical)
{
  cell *cstr;
  amx_GetAddr(amx,params[1],&cstr);
  printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
  g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
  LogCritical(g_cAmxPrintBuffer);
  g_nAmxPrintPtr=0;
  return 0;
}

/*
\brief Open a file
\author Sparhawk
\since 0.82
\param 1: File name
\param 2: Open mode
\return File identifier or INVALID
*/
NATIVE( _file_open )
{
	cell *cstr;
	amx_GetAddr(amx,params[1],&cstr);
	printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	char fileName[1024];
	strncpy( fileName, g_cAmxPrintBuffer, 1024 );
	g_nAmxPrintPtr=0;
	amx_GetAddr(amx,params[2],&cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
	return amxFileServer.open( fileName, g_cAmxPrintBuffer );
}

/*
\brief Close a file
\author Sparhawk
\since 0.82
\param 1: File identifier obtained from file_open
\return True or false
*/
NATIVE( _file_close )
{
	return amxFileServer.close( params[1] );
}

/*
\brief Check for end of file
\author Sparhawk
\since 0.82
\param 1: File identifier obtained from file_open
\return True or false
*/
NATIVE( _file_eof )
{
	return amxFileServer.eof( params[1] );
}

/*
\brief Write a string to a file
\author Sparhawk
\since 0.82
\param 1: File identifier obtained from file_open
\param 2: Text string
\return True on success else false
*/
NATIVE( _file_write )
{
	cell *cstr;
	amx_GetAddr(amx,params[2],&cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
	return amxFileServer.write( params[1], g_cAmxPrintBuffer );
}

/*
\brief Read a line from a file
\author Sparhawk
\since 0.82
\param 1: File identifier obtained from file_open
\param 2: Array into which data is read
\return True
*/
NATIVE( _file_read )
{
	cell *cptr;
	g_cAmxPrintBuffer[0] = '\0';
	std::string str( amxFileServer.read( params[1] ) );
	strcpy( g_cAmxPrintBuffer, str.c_str() );
	amx_GetAddr( amx, params[2], &cptr );
	amx_SetString( cptr, g_cAmxPrintBuffer, g_nStringMode);
	return true;
}

/*
\brief Call an AMX function without parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction )
{
	if (params[1] < 0) return INVALID;
	return AmxFunction::g_prgOverride->CallFn(params[1]);
}

/*
\brief Call an AMX function with 1 parameter
\author Luxor
\since 0.82
*/
NATIVE( _callFunction1P )
{
	if (params[1] < 0) return INVALID;
	return AmxFunction::g_prgOverride->CallFn(params[1], params[2]);
}

/*
\brief Call an AMX function with 2 parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction2P )
{
	if (params[1] < 0) return INVALID;
	return AmxFunction::g_prgOverride->CallFn(params[1], params[2], params[3]);
}

/*
\brief Call an AMX function with 3 parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction3P )
{
	if (params[1] < 0) return INVALID;
	return AmxFunction::g_prgOverride->CallFn(params[1], params[2], params[3], params[4]);
}

/*
\brief Call an AMX function with 4 parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction4P )
{
	if (params[1] < 0) return INVALID;
	return AmxFunction::g_prgOverride->CallFn(params[1], params[2], params[3], params[4], params[5]);
}

/*
\brief Call an AMX function with 5 parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction5P )
{
	if (params[1] < 0) return INVALID;
	return AmxFunction::g_prgOverride->CallFn(params[1], params[2], params[3], params[4], params[5], params[6]);
}

/*
\brief Call an AMX function with 6 parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction6P )
{
	if (params[1] < 0) return INVALID;
	return AmxFunction::g_prgOverride->CallFn(params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
}

/*
\brief Check if a character is online
\author Luxor
\since 0.82
*/
NATIVE( _chr_isOnline )
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, 0);
	return pc->IsOnline();
}

/*
\brief Check if a character is frozen
\author Luxor
\since 0.82
*/
NATIVE( _chr_isFrozen )
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, 0);
	return pc->isFrozen();
}

/*
\brief Make a character doing his combat animation
\author Luxor
\since 0.82
*/
NATIVE( _chr_doCombatAction )
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, 0);
	pc->playCombatAction();
	return 1;
}

/*
\brief Stable the caracter
\author Endymion
\since 0.82
\param 1 the npc to stable
\param 2 the stablemaster
*/
NATIVE( _chr_stable )
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, 0);
	if( !pc->npc ) return 0;
	P_CHAR stablemaster = pointers::findCharBySerial(params[2]);
	VALIDATEPCR( stablemaster, 0 );
	pc->stable( stablemaster );
	return 1;
}

/*
\brief Unstable the character
\author Endymion
\since 0.82
\param 1 the npc to unstable
*/
NATIVE( _chr_unStable )
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, 0);
	pc->unStable( );
	return 1;
}




//
// New Menu API
//

/*!
\brief Create a new menu
\author Endymion
\since 0.82
\param 1 x
\param 2 y
\param 3 is moveable
\param 4 is closeable
\param 5 is disposeable
\param 6 function callback
\return the menu serial
*/
NATIVE ( _gui_create )
{
	P_MENU menu = Menus.insertMenu( new cMenu( MENUTYPE_CUSTOM, params[1], params[2], (params[3]?true:false), (params[4]?true:false), (params[5]?true:false) ) );
	VALIDATEPMR( menu, INVALID );

	cell *cstr;
	amx_GetAddr(amx, params[6], &cstr);
	amx_GetString(g_cAmxPrintBuffer, cstr);
	menu->setCallBack( std::string( g_cAmxPrintBuffer ) );
	g_nAmxPrintPtr=0;
	return menu->serial;
}

/*!
\brief Delete a menu
\author Endymion
\since 0.82
\param 1 the menu serial
\return true if is deleted or false if error
*/
NATIVE ( _gui_delete )
{
	return ( Menus.removeMenu( params[1] )==INVALID )? 1 : 0;
}

/*!
\brief Delete a menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 the character
\return true if is show or false if error
*/
NATIVE ( _gui_show )
{
	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, 0 );

	P_MENU menu = Menus.getMenu( params[1] );
	VALIDATEPMR( menu, 0 );

	menu->show( pc );
	return 1;
}

/*!
\brief Add background at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 the gump
\param 3 the width
\param 4 the height
\return false if error, true else
*/
NATIVE( _gui_addBackground )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addBackground( params[2], params[3], params[4] );

	return 1;
}

/*!
\brief Add a new button at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 button up gump
\param 5 button down gump
\param 6 return code
\param 7 pressable, if true can be pressed
\return false if error, true else
*/
NATIVE ( _gui_addButton )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addButton( params[2], params[3], params[4], params[5], params[6], params[7] );

	return 1;
}

/*!
\brief Add a new button at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 button up gump
\param 5 button down gump
\param 6 return code
\param 7 pressable, if true can be pressed
\param 8 function callback
\return false if error, true else
*/
NATIVE ( _gui_addButtonFn )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	cell *cstr;
	amx_GetAddr(amx,params[8],&cstr);
	printstring(amx,cstr,params+9,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
	FUNCIDX fn = AmxFunction::g_prgOverride->getFnOrdinal( g_cAmxPrintBuffer );

	menu->addButtonFn( params[2], params[3], params[4], params[5], params[6], params[7], fn );
	g_nAmxPrintPtr=0;

	return 1;
}


/*!
\brief Add a new checkbox at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 off gump
\param 5 on gump
\param 6 checked
\param 7 result
\return false if error, true else
*/
NATIVE( _gui_addCheckbox )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addCheckbox( params[2], params[3], params[4], params[5], params[6], params[7] );

	return 1;
}

/*!
\brief Add a new ?? at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 width
\param 5 height
\return false if error, true else
*/
NATIVE( _gui_addCheckTrans )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addCheckertrans( params[2], params[3], params[4], params[5] );

	return 1;
}

/*!
\brief Add a new Cropped text at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 width
\param 5 height
\param 6 hue
\param 7 text
\return false if error, true else
*/
NATIVE( _gui_addCroppedText )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	cell *cstr;
	amx_GetAddr(amx,params[7],&cstr);
	printstring(amx,cstr,params+8,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
	std::wstring s;
	string2wstring( std::string( g_cAmxPrintBuffer ), s );

	menu->addCroppedText( params[2], params[3], params[4], params[5], s, params[6] );

	return 1;
}

/*!
\brief Add a new gump at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 gump
\param 5 color
\return false if error, true else
*/
NATIVE( _gui_addGump )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addGump( params[2], params[3], params[4], params[5] );

	return 1;
}

/*!
\brief Add a new html gump at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 width
\param 5 height
\param 6 html
\param 7 Has back ??
\param 8 can scroll
\return false if error, true else
*/
NATIVE( _gui_addHtmlGump )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	cell *cstr;
	amx_GetAddr(amx,params[6], &cstr);
	wstring s;
	amx_GetStringUnicode( s, cstr );


	menu->addHtmlGump( params[2], params[3], params[4], params[5], s, params[7], params[8] );

	return 1;
}

/*!
\brief Add a new input field at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 width
\param 5 height
\param 6 text id
\param 7 color
\param 8 initial value
\return false if error, true else
*/
NATIVE( _gui_addInputField )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	cell *cstr;
	amx_GetAddr(amx,params[8],&cstr);
	printstring(amx,cstr,params+9,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
	std::wstring s;
	std::string s1( g_cAmxPrintBuffer );
	string2wstring( s1, s );
	//string2wstring( std::string( g_cAmxPrintBuffer ), s );
/* move to unicode when exist printstring unicode
	wstring s;
	amx_GetStringUnicode( s, cstr );
*/
	menu->addInputField( params[2], params[3], params[4], params[5], params[6], s, params[7] );

	return 1;
}

/*!
\brief Add a new property field at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 width
\param 5 height
\param 6 property
\param 7 sub property
\param 8 color
\return false if error, true else
*/
NATIVE( _gui_addPropField )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addPropertyField( params[2], params[3], params[4], params[5], params[6], params[7], params[8] );

	return 1;
}

/*!
\brief Add a new radio button at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 off gump
\param 5 on gump
\param 6 checked
\param 7 result
\return false if error, true else
*/
NATIVE( _gui_addRadioButton )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addRadioButton( params[2], params[3], params[4], params[5], params[6], params[7] );

	return 1;
}

/*!
\brief Add a resized gump at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 gump
\param 5 width
\param 6 height
\return false if error, true else
*/
NATIVE (  _gui_addResizeGump )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addResizeGump( params[2], params[3], params[4], params[5], params[6] );
	return 1;
}

/*!
\brief Add text at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 color
\param 5 text
\return false if error, true else
*/
NATIVE ( _gui_addText )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	cell *cstr;
	amx_GetAddr(amx,params[5],&cstr);
	printstring(amx,cstr,params+6,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
	std::wstring s;
	string2wstring( std::string( g_cAmxPrintBuffer ), s );
/* move to unicode when possible use printstring unicode version
	wstring s;
	amx_GetStringUnicode( s, cstr );
*/
	menu->addText( params[2], params[3], s, params[4] );
	return 1;
}

/*!
\brief Add a new tile picture at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 tile
\param 5 color
\return false if error, true else
*/
NATIVE( _gui_addTilePic )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addTilePic( params[2], params[3], params[4], params[5] );
	return 1;
}

/*!
\brief Add a new tiled gump at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 width
\param 5 height
\param 6 gump
\param 7 color
\return false if error, true else
*/
NATIVE( _gui_addTiledGump )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addTiledGump( params[2], params[3], params[4], params[5], params[6], params[7] );
	return 1;
}

/*!
\brief Add a new ?? at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 width
\param 5 height
\param 6 clilocid
\param 7 Has back ??
\param 8 can scroll
\return false if error, true else
*/
NATIVE( _gui_addXmfHtmlGump )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	cell *cstr;
	amx_GetAddr(amx,params[4],&cstr);
	wstring s;
	amx_GetStringUnicode( s, cstr );

	menu->addXmfHtmlGump( params[2], params[3], params[4], params[5], s, params[6], params[7] );
	return 1;
}

/*!
\brief Add a new page at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 the page
\return false if error, true else
*/
NATIVE( _gui_addPage )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addPage( params[2] );
	return 1;
}


/*!
\brief Add a new group at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 the group
\return false if error, true else
*/
NATIVE( _gui_addGroup )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addGroup( params[2] );
	return 1;
}

/*!
\brief Add a new page button at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 up
\param 5 down
\param 6 page to go
\return false if error, true else
*/
NATIVE( _gui_addPageButton )
{
	cMenu* menu = static_cast<cMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	menu->addPageButton( params[2], params[3], params[4], params[5], params[6] );
	return 1;
}

/*!
\brief Create a new icon list menu
\author Endymion
\since 0.82
\param 1 function callback
\param 2 question
\return the menu serial
*/
NATIVE( _gui_createIconList )
{
	cIconListMenu* menu = (cIconListMenu*)Menus.insertMenu( new cIconListMenu() );
	VALIDATEPMR( menu, INVALID );

	cell *cstr;
	amx_GetAddr(amx, params[1], &cstr);
	amx_GetString(g_cAmxPrintBuffer, cstr);
	menu->setCallBack( std::string( g_cAmxPrintBuffer ) );
	g_nAmxPrintPtr=0;

	amx_GetAddr(amx,params[2],&cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	menu->question = g_cAmxPrintBuffer;
	g_nAmxPrintPtr=0;

	return menu->serial;
}

/*!
\brief Add a new icon at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 model
\param 3 color
\param 4 data
\param 5 response
\return false if error, true else
*/
NATIVE( _gui_addIcon )
{
	cIconListMenu* menu = static_cast<cIconListMenu*>( Menus.getMenu( params[1] ) );
	VALIDATEPMR( menu, 0 );

	cell *cstr;
	amx_GetAddr(amx,params[5],&cstr);
	printstring(amx,cstr,params+6,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;

	menu->addIcon( params[2], params[3], params[4], std::string( g_cAmxPrintBuffer ) );
	return 1;
}



//
// New Target API
//

/*!
\brief Create a new target
\author Endymion
\since 0.82
\param 1 chr
\param 2 param
\param 3 param
\param 4 doNow
\param 5 func callback
\return the target serial
*/
NATIVE( _target_create )
{
	P_CHAR pc = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( pc, INVALID );

	NXWCLIENT ps = pc->getClient();
	if( ps==NULL )
		return INVALID;

	P_TARGET targ = clientInfo[ ps->toInt() ]->newTarget( new cTarget() );
	targ->code_callback=amxCallback;
	targ->buffer[0]=params[2];
	targ->buffer[1]=params[3];

	cell *cstr;
	amx_GetAddr(amx,params[5],&cstr);
	printstring(amx,cstr,params+6,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;

	targ->amx_callback = new AmxFunction( g_cAmxPrintBuffer );

	if( params[4] )
		targ->send( ps );

	return targ->serial;

}

/*!
\brief Send target to character
\author Endymion
\since 0.82
\param 1 the target serial
\param 2 the chr
\return true if sender, false if error
*/
NATIVE( _target_do )
{
	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, false );

	NXWCLIENT ps = pc->getClient();
	if( ps==NULL )
		return false;

	P_TARGET targ = clientInfo[ps->toInt()]->getTarget();
	if( ( targ!=NULL ) && ( targ->serial==params[1] ) ) {
		targ->send( ps );
		return true;
	}
	else
		return false;


}


//
// New Party API
//

/*!
\brief Create a new party
\author Endymion
\since 0.82
\param 1 leader
\return the party serial
*/
NATIVE( _party_create )
{
	P_CHAR leader = pointers::findCharBySerial( params[1] );
	VALIDATEPCR( leader, INVALID );

	P_PARTY party = Partys.createParty();
	party->addMember( leader );

	return party->serial;
}

/*!
\brief Add given member to party
\author Endymion
\since 0.82
\param 1 the party
\param 2 new member
\return true or false if error
*/
NATIVE( _party_addMember )
{
	P_PARTY party = Partys.getParty( params[1] );
	if( party==NULL )
		return false;

	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, false );

	party->addMember( pc );
	return true;
}


/*!
\brief Remove given member from party
\author Endymion
\since 0.82
\param 1 the party
\param 2 member to remove
\return true or false if error
*/
NATIVE( _party_delMember )
{
	P_PARTY party = Partys.getParty( params[1] );
	if( party==NULL )
		return false;

	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, false );

	party->removeMember( pc );
	return true;
}

/*!
\brief add a new candidate to given party
\author Endymion
\since 0.82
\param 1 the party
\param 2 the leader
\param 3 new candidate
\return true or false if error
*/
NATIVE( _party_addCandidate )
{
	P_PARTY party = Partys.getParty( params[1] );
	if( party==NULL )
		return false;

	P_CHAR leader = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( leader, false );

	P_CHAR cand = pointers::findCharBySerial( params[3] );
	VALIDATEPCR( cand, false );

	party->addCandidate( leader, cand );
	return true;
}

/*!
\brief Remove given candidate from party
\author Endymion
\since 0.82
\param 1 the party
\param 2 member to remove
\return true or false if error
*/
NATIVE( _party_delCandidate )
{
	P_PARTY party = Partys.getParty( params[1] );
	if( party==NULL )
		return false;

	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, false );

	party->removeCandidate( pc->getSerial32() );
	return true;
}

/*!
\brief Check if given character is candidate to party
\author Endymion
\since 0.82
\param 1 the party
\param 2 member to remove
\return true or false if error
*/
NATIVE( _party_isCandidate )
{
	P_PARTY party = Partys.getParty( params[1] );
	if( party==NULL )
		return false;

	P_CHAR pc = pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, false );

	return party->isCandidate( pc->getSerial32() );

}

/*!
\brief Send given message to all members of party
\author Endymion
\since 0.82
\param 1 the party
\param 2 color
\param 3 message
\return INVALID if error
*/
NATIVE( _party_broadcast )
{
	P_PARTY party = Partys.getParty( params[1] );
	if( party==NULL )
		return INVALID;

	cell *cstr;
	amx_GetAddr(amx,params[3],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;

	std::wstring w;
	string2wstring( std::string( g_cAmxPrintBuffer ), w );
	party->talkToAll( w, params[2] );

	return true;
}


//
// Gm paging system
//

/*!
\brief Register a Gm page sent from a player.
\author Frodo and stonedz
\since 0.82
\param 1 character
\param 2 reason of the page
\return false if the player already sent more than 3 pages
*/
NATIVE ( _addGmPage )
{	
	
	extern cGmpagesMap* pages;
	
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
    
	cell *cstr;

	amx_GetAddr(amx,params[2],&cstr);

	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);

    g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,100)] = '\0';

	P_GMPAGE page = new cGmpage(pc->getSerial32(), g_cAmxPrintBuffer);
	g_nAmxPrintPtr=0;
	
	if (pages->addPage(page) == NULL )
		return false;
	return true;

}

/*!
\brief Return a set with all paging characters,every character appears only once in the set, even if he did multiple pages.
\author Frodo and stonedz
\param 1 the set
\return false if no pages are present at the moment
*/
NATIVE ( _getGmPageList )
{
	
	extern cGmpagesMap* pages;
	
	int i, empty=1;

	SERIAL_VECTOR pagers = pages->getAllPagers();
	
	for(i=0; i<pagers.size(); i++){

		amxSet::add( params[1], pagers[i] );
		empty=0;
	}
	
	if(empty) 
		return false;
	else 
		return true;
}

/*!
\brief Fills param 3 and param 4 with the reason and the time of the page done by the param 1 player.
\author Frodo and stonedz
\since 0.82
\param 1 character
\param 2 page number (range 1-3)
\param 3 string to be filled with the reason of selected page
\param 4 string to be filled with the time the selected page was sent
\return false if no page corresponds to the given one 
*/
NATIVE ( _chr_getGmPage )
{
	extern cGmpagesMap* pages;

	P_GMPAGE page=pages->findPage(params[1], params[2]);

	if( pages==NULL )
		return false;
	
	char str1[100];
	char str2[100];

	cell *cptr1;
	cell *cptr2;

	strcpy(str1, (page->getReason()).c_str());
	strcpy(str2, (page->getTime()).c_str());

    amx_GetAddr(amx,params[3],&cptr1);
    amx_GetAddr(amx,params[4],&cptr2);

	amx_SetString(cptr1,str1, g_nStringMode);
	amx_SetString(cptr2,str2, g_nStringMode);

	return true;

}

/*!
\brief Deletes the given page from the map.
\author Frodo and stonedz
\param 1 character
\param 2 page number (range 1-3)
\return false if no page corresponds to the given one
*/
NATIVE ( _chr_solveGmPage )
{
	extern cGmpagesMap* pages;
	
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	
	if (pages->deletePage(pc->getSerial32(), (UI08) params[2] ) == false)
		return false;
	
	return true;
}



// Command System Params


/*!
\brief Return a set with all the params give by the char with the last command calling
\author Frodo
\param 1 the char
\param 2 the set
\return false if no params are present at the moment
*/

/*NATIVE ( _chr_getParams )
{
	
	//extern cGmpagesMap* pages;
	
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, INVALID);
	
	int i, empty=1;

	for(i=1; i<9 && strcmp((char*)pc->getCommandParams(i).c_str(), " ")!=0 ; i++){


		amxSet::add( params[2], (char*)pc->getCommandParams(i).c_str() );
		empty=0;
	}
	
	if(empty) 
		return false;
	else 
		return true;
}*/



/*!
\brief returns the given property and subpropoerty value for the house
\author Wintermute
\since 0.82
\fn house_getProperty
\param 1 the house serial to get the property from
\param 2 the house property to get
\param 3 the subproperty to get
*/

NATIVE ( _house_getProperty )
{
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {
		case T_INT: {
			int p = house_getIntProperty( house, params[2], params[3]);
			cell i = p;
			return i;
		}
		case T_BOOL: {
			bool p = house_getBoolProperty( house, params[2], params[3]);
			cell i = p;
			return i;
		}
		case T_SHORT: {
			short p = house_getShortProperty( house, params[2], params[3]);
			cell i = p;
			return i;
		}
		case T_CHAR: {
			char p = house_getCharProperty( house, params[2], params[3]);
			cell i = p;
			return i;
		}
		case T_STRING: {

			char str[100];
	  		strcpy(str, house_getStrProperty( house, params[2], params[3]));

  			cell *cptr;
  			amx_GetAddr(amx,params[4],&cptr);
	  		amx_SetString(cptr,str, g_nStringMode);

  			return strlen(str);

		}
	}
  	return INVALID;
}

/*!
\brief sets the given property and subpropoerty value for the house
\author Wintermute
\since 0.82
\fn house_setProperty
\param 1 the house serial to get the property from
\param 2 the house property to set
\param 3 the subproperty to set
*/

NATIVE ( _house_setProperty )
{
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {
		case T_INT: {
			house_setIntProperty( house, params[2], params[3], params[4]);
		}
		case T_BOOL: {
			house_setBoolProperty( house, params[2], params[3], params[4]);
		}
		case T_SHORT: {
			house_setShortProperty( house, params[2], params[3], params[4]);
		}
		case T_CHAR: {
			house_setCharProperty( house, params[2], params[3], params[4]);
		}
		case T_STRING: {
			cell *cstr;
  			amx_GetAddr(amx,params[2],&cstr);
  			printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
  			g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';
			house_setStrProperty( house, params[2], params[3], g_cAmxPrintBuffer);
  			g_nAmxPrintPtr=0;

		}
	}
	return true;
}

/*!
\brief locks the given item, making it unmovable and increase the number of locked items by one in the house, fails if maximum amount of lockable items is exceeded
\author Wintermute
\since 0.82
\fn house_lockItem
\param house, the house serial to get the property from
\param item, the serial of the item to lock

*/
NATIVE ( _house_lockItem )
{
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	//(const house, const item);
    P_ITEM pi=pointers::findItemBySerial( params[2]);
    if(ISVALIDPI(pi))
	{
		if( house != NULL)
		{
			if( pi->isFieldSpellItem() )
			{
				return false;
			}
			if (pi->type==12 || pi->type==13 || pi->type==203)
			{
				return false;
			}
			if ( pi->IsAnvil() )
			{
				return false;
			}
			if ( pi->IsForge() )
			{
				return false;
			}
			house->increaseLockedItems();
			pi->magic = 4;  // Default as stored by the client, perhaps we should keep a backup?
			pi->setOwnerSerial32Only(house->getOwner());
			pi->SetMultiSerial(house->getSerial());
			pi->Refresh();
			return true;
		}
	}
	else
		LogWarning("Illegal item serial %d used in _house_lockItem\n", params[1]);
	return false;
}

/*!
\brief unlocks the given item, making it movable and decrease the number of locked items by one in the house
\author Wintermute
\since 0.82
\fn house_unlockItem
\param house, the house serial
\param item, the serial of the item to unlock
*/

NATIVE ( _house_unlockItem )
{
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	//(const house, const item);
    P_ITEM pi=pointers::findItemBySerial( params[2]);
    if(ISVALIDPI(pi))
	{
		if( house != NULL)
		{
			if ( pi->magic != 4 )
			{
				return false;
			}
			if( pi->isFieldSpellItem() )
			{
				return false;
			}
			if (pi->type==12 || pi->type==13 || pi->type==203)
			{
				return false;
			}
			if ( pi->IsAnvil() )
			{
				return false;
			}
			if ( pi->IsForge() )
			{
				return false;
			}
			house->decreaseLockedItems();
			pi->magic = 1;  // Default as stored by the client, perhaps we should keep a backup?
			pi->setOwnerSerial32Only(INVALID);
			pi->SetMultiSerial(INVALID);
			pi->Refresh();
			return true;
		}
	}
	else
		LogWarning("Illegal item serial %d used in _house_lockItem\n", params[1]);
	return false;
}

/*!
\brief secures the given container, making it openable only by owner and coowner and increase the number of secured items by one in the house, fails if maximum amount of secured items is exceeded
\author Wintermute
\since 0.82
\fn house_secureContainer
\param house, the house serial to get the property from
\param item, the serial of the container to secure
*/

NATIVE ( _house_secureContainer )
{
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	//(const house, const container);
    P_ITEM pi=pointers::findItemBySerial( params[2]);
    if(ISVALIDPI(pi))
	{
		if( pi->isFieldSpellItem() )
		{
			return false;
		}
		if (pi->type==12 || pi->type==13 || pi->type==203)
		{
			return false;
		}
		if(pi->magic==4)
		{
			return false;
		}

		if( house != NULL && house->inHouse(pi) && pi->type==1)
		{
			if ( house->getSecuredItems() < house->getMaxSecuredItems() )
			{
				pi->magic = 4;  // LOCKED DOWN!
				pi->secureIt = 1;
				pi->setOwnerSerial32Only(house->getOwner());
				pi->SetMultiSerial(house->getSerial());
				pi->Refresh();
				return true;
			}
		}
	}
	return false;
}

/*!
\brief unsecures the given container, making it openable only by everyone and decrease the number of secured items by one in the house
\author Wintermute
\since 0.82
\fn house_unsecureContainer
\param house, the house serial
\param item, the serial of the container to secure
*/

NATIVE ( _house_unsecureContainer )
{
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	//(const house, const container);
    P_ITEM pi=pointers::findItemBySerial( params[2]);
    if(ISVALIDPI(pi))
	{
        if( pi->isFieldSpellItem() )
        {
            return false;
        }
        if (pi->type==12 || pi->type==13 || pi->type==203)
        {
            return false;
        }
        if( house != NULL && pi->getMultiSerial32() == house->getSerial() )
        {
			if ( pi->secureIt == 1 )
				house->decreaseSecuredItems();
            pi->magic = 1;  // Default as stored by the client, perhaps we should keep a backup?
            pi->secureIt = 0;
            pi->Refresh();
            return true;
        }
	}
 	return false;
}

/*!
\brief place a trace barrel at the given location
\author Wintermute
\since 0.82
\fn house_placeTrashBarrel
\param house, the house serial
\param x,y,z position of the trashbarrel
*/

NATIVE ( _house_placeTrashBarrel )
{
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	//(const house, const x, const y, const z);
	return false;
}

/*!
\brief returns a boolean if the given obj is inside a house (meaning it is below a roof)
\author Wintermute
\since 0.82
\fn house_isInsideHouse
\param house, the house serial
\param obj, the item or char serial
\return boolean
*/

NATIVE ( _house_isInsideHouse )
{
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	//(const house, const obj);
	P_OBJECT obj = objects.findObject(params[2]);
	if ( obj != NULL )
		return house->isInsideHouse(obj->getPosition());
	else
		LogWarning("Illegal object serial %d used in _house_isInHouse\n", params[2]);
	return false;
}

/*!
\brief returns a boolean if the given obj is inside a house area (meaning it is within the limits of spacex and spacey from the center of the house)
\author Wintermute
\since 0.82
\fn house_isInHouse
\param 1 the house serial
\param 2 the item or char serial
\return boolean
*/

NATIVE ( _house_isInHouse )
{
	//(const house, const obj);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	P_OBJECT obj = objects.findObject(params[2]);
	if ( obj != NULL )
		return house->inHouse(obj->getPosition());
	else
		LogWarning("Illegal object serial %d used in _house_isInHouse\n", params[2]);
	return false;
}

/*!
\brief adds a friend to the list of house friends
\author Wintermute
\since 0.82
\fn house_addFriend
\param house, the house serial
\param the char serial of the new friend
\return boolean
*/

NATIVE ( _house_addFriend )
{
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	//(const house, const chr);
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		house->addFriend(pc);
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_addFriend\n", params[2]);
		return false;
	}
	return true;
}

/*!
\brief removes a friend from the list of house friends
\author Wintermute
\since 0.82
\fn house_removeFriend
\param house, the house serial
\param the char serial of the new friend
\return boolean
*/

NATIVE ( _house_removeFriend )
{
	//(const house, const chr);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		house->removeFriend(pc);
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_removeFriend\n", params[2]);
		return false;
	}
	return true;
}

/*!
\brief tells if a char is friend of the house
\author Wintermute
\since 0.82
\fn house_isFriend
\param house, the house serial
\param the char serial of the character
\return boolean
*/

NATIVE ( _house_isFriend )
{
	//(const house, const chr);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		return house->isFriend(pc);
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_isFriend\n", params[2]);
		return false;
	}
	return false;
}

/*!
\brief adds a co owner  to the list of house owners
\author Wintermute
\since 0.82
\fn house_addCoOwner
\param house, the house serial
\param the char serial of the new co owner
\return boolean
*/

NATIVE ( _house_addCoOwner )
{
	//(const house, const chr);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		house->addCoOwner(pc);
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_addCoOwner\n", params[2]);
		return false;
	}
	return true;
}

/*!
\brief removes a co owner from the list of house owners
\author Wintermute
\since 0.82
\fn house_removeCoOwner
\param house, the house serial
\param the char serial of the new co owner
\return boolean
*/

NATIVE ( _house_removeCoOwner )
{
	//(const house, const chr);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in _house_removeCoOwner\n", params[1]);
		return false;
	}
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		house->removeCoOwner(pc);
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_removeCoOwner\n", params[2]);
		return false;
	}
	return true;
}

/*!
\brief tells if a char is co owner of the house
\author Wintermute
\since 0.82
\fn house_isCoOwner
\param house, the house serial
\param the char serial of the character
\return boolean
*/

NATIVE ( _house_isCoOwner )
{
	//(const house, const chr);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in _house_isCoOwner\n", params[1]);
		return false;
	}
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		return house->isCoOwner(pc);
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_isCoOwner\n", params[2]);
		return false;
	}
	return false;
}

/*!
\brief adds a character to the list of banned people
\author Wintermute
\since 0.82
\fn house_addBan
\param house, the house serial
\param the char serial of the banned character
\return boolean
*/

NATIVE ( _house_addBan )
{
	//(const house, const chr);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in _house_addBan\n", params[1]);
		return false;
	}
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		house->addBan(pc);
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_addBan\n", params[2]);
		return false;
	}
	return true;
}

/*!
\brief removes  a character from the list of banned people
\author Wintermute
\since 0.82
\fn house_removeBan
\param house, the house serial
\param the char serial of the banned character
\return boolean
*/
NATIVE ( _house_removeBan )
{
	//(const house, const chr);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in _house_removeBan\n", params[1]);
		return false;
	}
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		house->removeBan(pc);
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_removeBan\n", params[2]);
		return false;
	}
	return true;
}

/*!
\brief tells if  a character is on the list of banned people
\author Wintermute
\since 0.82
\fn house_isBanned
\param house, the house serial
\param the char serial of the banned character
\return boolean
*/

NATIVE ( _house_isBanned )
{
	//(const house, const chr);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in _house_isBanned\n", params[1]);
		return false;
	}
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		return house->isBanned(pc);
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_isBanned\n", params[2]);
		return false;
	}
	return false;
}

/*!
\brief transfers the house to a new owner, removing all people from the friend, coowner and banned list, removing all keys to the house and making a new pair for the new owner
\author Wintermute
\since 0.82
\fn house_addBan
\param house, the house serial
\param the char serial of the new owner
\return boolean
*/

NATIVE ( _house_transfer )
{
	//(const house, const newOwner);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		house->transfer(params[2]);
		return true;
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_transfer\n", params[2]);
		return false;
	}
	return true;
}

/*!
\brief changes the locks on the house, making a new keycode, therefore rendering all previous keys useless
\author Wintermute
\since 0.82
\fn house_changeLocks
\param house, the house serial
*/

NATIVE ( _house_changeLocks )
{
	//(const house);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	house->changeLocks();
	return true;
}

/*!
\brief creates a new pair of keys for the current keycode
\author Wintermute
\since 0.82
\fn house_makeKeys
\param house, the house serial
*/

NATIVE ( _house_makeKeys )
{
	//(const house, const chr);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	P_CHAR pc = pointers::findCharBySerial(params[2]);
	if (ISVALIDPC(pc) )
	{
		cMulti::makeKeys(house,pc);
		return true;
	}
	else
	{
		LogWarning("Illegal char serial %d used in _house_makeKeys\n", params[2]);
		return false;
	}
	return true;
}

/*!
\brief remove all keys currently in the worldsave for this house
\author Wintermute
\since 0.82
\fn house_deleteKeys
\param house, the house serial
*/

NATIVE ( _house_deleteKeys )
{
	//(const house);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in house_getProperty\n", params[1]);
		return false;
	}
	cHouses::killkeys(house->getSerial());
	return true;
}

/*!
\brief remove the house and all item within the house limits
\author Wintermute
\since 0.82
\fn house_delete
\param house, the house serial
*/

NATIVE ( _house_delete )
{
	//(const house);
	P_HOUSE house = cHouses::findHouse(params[1]);
	if ( house == NULL )
	{
		LogWarning("Illegal house serial %d used in _house_delete\n", params[1]);
		return false;
	}
	house->remove();
	P_ITEM iHouse = pointers::findItemBySerial(params[1]);
	if ( ! ISVALIDPI(iHouse))
	{
		LogWarning("No item found for house serial %d used in _house_delete\n", params[1]);
		return false;
	}
	iHouse->Delete();
	return true;
}


/*!
\brief perform a worldsave
\author stonedz
\since 0.82
\fn world_save
\return false if the save does not succed, true if the save succeds
*/

NATIVE ( _world_save )
{
	//timer and message to be added
	if ( cwmWorldState->Saving() )
		return false;
	else
	{
		cwmWorldState->saveNewWorld();
		return true;
	}

}

/*!
\brief set the post type for a char
\author stonedz
\since 0.82
\fn chr_setPostType
\param 1 character
\param 2 Post type (0=local post, 1=regional post, 2= global post)
\return true if the operation succedes, false elsewhere
*/

NATIVE ( _chr_setPostType )
{

     P_CHAR pc = pointers::findCharBySerial(params[1]);
	if (ISVALIDPC(pc))
	{
		MsgBoards::MsgBoardSetPostType( pc->getSocket() , (MsgBoards::PostType) params[2] );
		return true;	
	}
	return false;
	
}



/*!
\brief reload all accounts
\author stonedz
\since 0.82
\fn reload_accounts
\return true if succeds.
\
*/

NATIVE (_reload_accounts )
{

	InfoOut("Reloading accounts file...");
	Accounts->LoadAccounts();
	cAllObjectsIter objs;
	for( objs.rewind(); !objs.IsEmpty(); objs++ )
	{
		if( isCharSerial( objs.getSerial() ) )
		{
			P_CHAR pc = (P_CHAR) objs.getObject();
			if ( ! pc->npc ) // seems to be a player
			{
				Accounts->AddCharToAccount(pc->account, pc);
			}
		}
	}
	ConOut("[DONE]\n");
	
	return true;
}

/*!
\brief reloads scripts
\author stonedz
\since 0.82
\fn reload_scripts
\return none
*/

NATIVE ( _reload_scripts )
{
	InfoOut("Reloading XSS scripts...");
	deleteNewScripts();
	newScriptsInit();
	ConOut("[DONE]\n");
	return true;
}

/*!
\brief reload the commands.txt file into memory
\author stonedz
\since 0.82
\fn reload_commands
\return none
*/

NATIVE ( _reload_commands )
{
	InfoOut("Reloading commands.txt into memory...");
	extern cCommandMap* commands;
	commands->clearCommandMap();
	commands->fillCommandMap();
	ConOut("[DONE]\n");
	return true;
}


/*!
\brief set light level in world
\author stonedz
\since 0.82
\fn setLightLevel
\param 1 light level: 0=brightest, 15=darkest, -1=enable day/night cycles.
\return true if the operation succedes, false elsewhere
*/

NATIVE ( _setLightLevel )
{

	worldfixedlevel=strtonum(params[1]);
	if (worldfixedlevel!=255) setabovelight(worldfixedlevel);
	else setabovelight(worldcurlevel);
	return true;	

}

/*!
\brief shuts down the server in param1 seconds.
\author stonedz
\since 0.82
\fn shutdown
\param 1 seconds until shutdown, if param 1 is 0 (zero) any previous shoutdown procedure will be interrupted.
\param 2 optional message to be printed on clients after the custom message.
\return true if operation succeed.
*/

NATIVE ( _shutdown )
{
	
	cell *cstr;
	amx_GetAddr(amx, params[2], &cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
 	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
	
		
	if ((int)params[1]==0) //interrupts previous shutdown
	{
		endtime=0;
		sysbroadcast(TRANSLATE("Previous shutdown has been interrupted!\n"));
		InfoOut("Previous shutdown has been interrupted\n");
	}
	else 
	{
		endtime=uiCurrentTime+(MY_CLOCKS_PER_SEC*(int) params[1]); //sets the shutdown time
	
		sysbroadcast(TRANSLATE("The server will shutdown in %d minutes and %d seconds.\n"),(int)params[1]/60, (int)params[1]%60);
	
		if (g_cAmxPrintBuffer[0]!='\0')
			sysbroadcast("%s\n", g_cAmxPrintBuffer);
	
		InfoOut("The server will shutdown in %d minutes and %d seconds.\n%s\n",(int)params[1]/60, (int)params[1]%60, g_cAmxPrintBuffer);
	}
	return true;
}

/*!
\brief sets the real seconds per one UO minute (default is 5)
\author stonedz
\since 0.82
\param 1 seconds to set an UO minute.
\return none
*/

NATIVE ( _setSecondsPerUoMinute )
{

	if ((int)params[1]>0)
		secondsperuominute=(int)params[1];
	else
		secondsperuominute=5; // standard value
	
	InfoOut("Real seconds per UO minute are now set to %d.\n",secondsperuominute);
	
	return true;

}

/*!
\brief Perform garbage collection
\author stonedz
\since 0.82
\return none
*/

NATIVE ( _garbageCollection )
{
	InfoOut("Performing garbage collection...");
	gcollect();
	ConOut("[DONE]\n");
	return true;

}

/*!
\brief
\author Frodo
\since 0.82
\param 1 character
\param 2 string to fill with the speech
\param 3 1 if you want to exclude an eventual initial 'command
\return true if the operation succedes, false elsewhere
*/

NATIVE ( _chr_getCmdSpeech )
{
    
	char str[256];

	cell *cptr;
	
	char* sp;
	char* pch;

	int n;

	if (params[1] < 0) return false;

	P_CHAR pc = pointers::findCharBySerial(params[1]);
	
	VALIDATEPCR(pc, INVALID);

	sp = pc->getCommandSpeech();
	
	if(strlen(sp)>=256) return false;

	pch=sp;
	
	if(params[3]==1){

			if(sp[0]=='\''){

			pch = strtok (sp," ");
			n=strlen(pch);
			pch = &sp[n+1];
		}
	}
	
	
	strcpy(str, pch);

    amx_GetAddr(amx,params[2],&cptr);

	amx_SetString(cptr,str, g_nStringMode);
	
	return true;
}

NATIVE (_recompileSmall )
{
	UI32 tempStringMode=g_nStringMode;
	initAmxEvents();
	LoadOverrides ();
	AMXEXEC(AMXT_SPECIALS,0,0,AMX_AFTER);
	g_nAmxPrintPtr=0;
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nStringMode=tempStringMode;
	params[0]=params[1]=params[2]=params[3]=params[4]=params[5]=params[6]=params[7]=params[8]=0;
	return true;
}
// resource map functions
NATIVE (_createResourceMap)
{
	ResourceMapType type=(ResourceMapType)params[1];
	cResourceMap *map;	
	cell *cstr;
	g_nAmxPrintPtr=0;	
 	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	amx_GetAddr(amx, params[3], &cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
 	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;	
	std::string filename = g_cAmxPrintBuffer;
	if ( type == RESOURCEMAP_STRING )
	{
		cResourceStringMap *newMap=new cResourceStringMap(filename, params[2]);
		newMap->setType(RESOURCEMAP_STRING);
		map=newMap;
	}
	else if ( type == RESOURCEMAP_LOCATION )
	{
		cResourceLocationMap *newMap=new cResourceLocationMap(filename, params[2]);
		newMap->setType(RESOURCEMAP_LOCATION);
		map=newMap;
	}
	else if ( type == RESOURCEMAP_STRINGSTRING )
	{
		cResourceStringStringMap *newMap=new cResourceStringStringMap(filename, params[2]);
		newMap->setType(RESOURCEMAP_STRINGSTRING);
		map=newMap;
	}
	else if ( type == RESOURCEMAP_LOCATIONSTRING )
	{
		cResourceLocationStringMap *newMap=new cResourceLocationStringMap(filename, params[2]);
		newMap->setType(RESOURCEMAP_LOCATIONSTRING);
		map=newMap;
	}
	return cResourceMap::addMap(map);

}

NATIVE (_deleteResourceMap )
{
	cResourceMap::deleteMap(params[1]);
	return true;
}

NATIVE (_getResourceLocationValue )
{
	UI32 resourceIndex = params[1];
	cResourceLocationMap * myMap = (cResourceLocationMap *) cResourceMap::getMap(resourceIndex);
	if ( myMap != NULL && myMap->getType() == RESOURCEMAP_LOCATION)
	{
		cCoord newKey(params[2], params[3],params[4]);
		return myMap->getValue(newKey);
	}
	else
	{
		LogError("Illegal or wrong type of resource map used with index %d", resourceIndex);
	}
	return -1;
}

NATIVE (_setResourceLocationValue )
{
	UI32 resourceIndex = params[1];
	cResourceLocationMap * myMap = (cResourceLocationMap *) cResourceMap::getMap(resourceIndex);
	if ( myMap != NULL && myMap->getType() == RESOURCEMAP_LOCATION)
	{
		cCoord newKey(params[3], params[4],params[5]);
		myMap->setValue(newKey, params[2]);
	}
	return 1;
}

NATIVE (_getResourceStringValue )
{
	UI32 resourceIndex = params[1];
	cResourceStringMap * myMap = (cResourceStringMap *) cResourceMap::getMap(resourceIndex);
	if ( myMap != NULL && myMap->getType() == RESOURCEMAP_STRING)
	{
		cell *cstr=NULL;
		amx_GetAddr(amx, params[3], &cstr);
		printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
 		g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
		g_nAmxPrintPtr=0;	
		std::string key = g_cAmxPrintBuffer;
		return myMap->getValue(key);
	}
	return -1;
}
NATIVE (_setResourceStringValue )
{
	UI32 resourceIndex = params[1];
	cResourceStringMap * myMap = (cResourceStringMap *) cResourceMap::getMap(resourceIndex);
	if ( myMap != NULL && myMap->getType() == RESOURCEMAP_STRING)
	{
		cell *cstr;
		amx_GetAddr(amx, params[3], &cstr);
		printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
 		g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
		g_nAmxPrintPtr=0;	
		std::string key = g_cAmxPrintBuffer;
		myMap->setValue(key, params[2]);
	}
	return 0;
}

NATIVE (_getResourceStringString )
{
	UI32 resourceIndex = params[1];
	cResourceStringStringMap * myMap = (cResourceStringStringMap *) cResourceMap::getMap(resourceIndex);
	if ( myMap != NULL && myMap->getType() == RESOURCEMAP_STRING)
	{
		cell *cstr=NULL;
		amx_GetAddr(amx, params[2], &cstr);
		printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
 		g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
		g_nAmxPrintPtr=0;	
		std::string key = g_cAmxPrintBuffer;
		std::string value = myMap->getValue(key);
		amx_GetAddr(amx,params[3],&cstr);
		amx_SetString(cstr,(char *) value.c_str(), g_nStringMode);
		return true;
	}
	return 0;
}
NATIVE (_setResourceStringString )
{
	UI32 resourceIndex = params[1];
	cResourceStringStringMap * myMap = (cResourceStringStringMap *) cResourceMap::getMap(resourceIndex);
	if ( myMap != NULL && myMap->getType() == RESOURCEMAP_STRING)
	{
		cell *cstr;
		amx_GetAddr(amx, params[2], &cstr);
		printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
 		g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
		g_nAmxPrintPtr=0;	
		std::string key = g_cAmxPrintBuffer;
		amx_GetAddr(amx, params[3], &cstr);
		printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
 		g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
		g_nAmxPrintPtr=0;	
		std::string value = g_cAmxPrintBuffer;
		myMap->setValue(key, value);
	}
	return 0;
}


NATIVE (_isChar )
{
	P_CHAR pc=pointers::findCharBySerial(params[1]);
	if ( !ISVALIDPC(pc))
		return 0;
	else
		return 1;
}

NATIVE (_isItem )
{
	P_ITEM pi=pointers::findItemBySerial(params[1]);
	if ( !ISVALIDPI(pi))
		return 0;
	else
		return 1;
}

NATIVE ( _chr_disconnect )
{
	P_CHAR pc=pointers::findCharBySerial(params[1]);
	if ( !ISVALIDPC(pc))
		return 0;
	else
		if ( pc->IsOnline() )
			Network->Disconnect(pc->getSocket());
	return 1;
}

NATIVE ( _chr_ban )
{
	P_CHAR pc=pointers::findCharBySerial(params[1]);
	if ( !ISVALIDPC(pc))
		return 0;
	else
	{
		cAccount *acc = Accounts->GetAccount(pc->account);
		if ( acc != NULL ) 
		{
			acc->ban=true;
			if ( pc->IsOnline() )
				Network->Disconnect(pc->getSocket());
		}
	}
	return 1;
}

NATIVE ( _chr_unban )
{
	P_CHAR pc=pointers::findCharBySerial(params[1]);
	if ( !ISVALIDPC(pc))
		return 0;
	else
	{
		cAccount *acc = Accounts->GetAccount(pc->account);
		if ( acc != NULL ) 
			acc->ban=false;
	}
	return 1;
		
}

/*
\brief returns the current ip of the account
\param 1 account number
*/

NATIVE ( _chr_getIP )
{
	P_CHAR pc=pointers::findCharBySerial(params[1]);
	if ( !ISVALIDPC(pc) || ! pc->IsOnline())
		return 0;
	else
	{
		cell *cptr;
		amx_GetAddr(amx, params[2], &cptr);
		UI32 ip = clientInfo[pc->getSocket()]->getClientIp();
		char buf [16];
		sprintf(buf, "%d.%d.%d.%d", ip&0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF);
		amx_SetString(cptr, buf	, g_nStringMode);
	}
	return 1;
}


/*
\brief Deletes the account number denoted by param 1
\param 1 account number to be deleted
*/

NATIVE ( _account_delete )
{
	return Accounts->RemoveAccount(params[1]);
}

/*
\brief Blocks the account number denoted by param 1
\param 1 account number to be blocked
*/

NATIVE ( _account_block )
{
	cAccount *acc = Accounts->GetAccount(params[1]);
	if ( acc != NULL ) 
		acc->ban=true;
	return 1;
		
}

/*
\brief changes the password for the account
\param 1 account number to be changed
\param 2 new password
*/

NATIVE ( _account_changePW )
{
	cell *cstr;
	amx_GetAddr(amx, params[2], &cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
 	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;	
	std::string accpassword = g_cAmxPrintBuffer;
	cAccount *acc = Accounts->GetAccount(params[1]);
	if ( acc != NULL ) 
		acc->changePassword(accpassword);
	return 1;
		
}

/*
\brief returns the current ip of the account
\param 1 account number
*/
NATIVE ( _account_getLastIP )
{
	cAccount *acc = Accounts->GetAccount(params[1]);
	cell *cptr;
	amx_GetAddr(amx, params[2], &cptr);
	amx_SetString(cptr,	(char * )(acc->getLastIp().c_str()), g_nStringMode);
	if ( acc != NULL ) 
		return 0;
	return 1;
}

/*
\brief returns the name of the account
\param 1 account number
*/
NATIVE ( _account_getName )
{
	cAccount *acc = Accounts->GetAccount(params[1]);
	cell *cptr;
	amx_GetAddr(amx, params[2], &cptr);
	if ( acc != NULL ) 
	{
		amx_SetString(cptr,	(char * )(acc->name.c_str()), g_nStringMode);
		return 0;
	}
	return 1;
}

/*
\brief Shows the sell gump of character 1 to character 2
\param 1 account number to be blocked
*/

NATIVE ( _chr_showSellGump )
{
	P_CHAR pc=pointers::findCharBySerial(params[1]);
	P_CHAR seller=pointers::findCharBySerial(params[2]);
	if ( !ISVALIDPC(pc))
		return 0;
	if ( !ISVALIDPC(seller) || !seller->IsOnline())
		return 0;
	sellstuff(seller->getSocket(), params[1] );
	return 1;
}

NATIVE ( _chr_showBuyGump )
{
	P_CHAR pc=pointers::findCharBySerial(params[1]);
	P_CHAR buyer=pointers::findCharBySerial(params[2]);
	if ( !ISVALIDPC(pc))
		return 0;
	if ( !ISVALIDPC(buyer) || !buyer->IsOnline())
		return 0;
	sellstuff(buyer->getSocket(), params[1] );
	return 1;
}

NATIVE ( _loadDefaults )
{
	loadserverdefaults();
	return 1;
}

NATIVE ( _pdump )
{
	char s_szCmdTableTemp[255];

	P_CHAR pc=pointers::findCharBySerial(params[1]);
	if ( !ISVALIDPC(pc))
		return 0;

	pc->sysmsg("Performance Dump:");

	sprintf(s_szCmdTableTemp, "Network code: %fmsec [%i]" , (float)((float)networkTime/(float)networkTimeCount) , networkTimeCount);
	pc->sysmsg(s_szCmdTableTemp);

	sprintf(s_szCmdTableTemp, "Timer code: %fmsec [%i]" , (float)((float)timerTime/(float)timerTimeCount) , timerTimeCount);
	pc->sysmsg(s_szCmdTableTemp);

	sprintf(s_szCmdTableTemp, "Auto code: %fmsec [%i]" , (float)((float)autoTime/(float)autoTimeCount) , autoTimeCount);
	pc->sysmsg(s_szCmdTableTemp);

	sprintf(s_szCmdTableTemp, "Loop Time: %fmsec [%i]" , (float)((float)loopTime/(float)loopTimeCount) , loopTimeCount);
	pc->sysmsg(s_szCmdTableTemp);

	sprintf(s_szCmdTableTemp, "Simulation Cycles/Sec: %f" , (1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount))));
	pc->sysmsg(s_szCmdTableTemp);

	pc->sysmsg("Characters: %i/Dynamic    Items: %i/Dynamic" , charCount, itemCount);
	return 1;
}
/*!
\file

<h2>API standard syntax :</h2>
for generic system functions : functionName
for other functions xxxx_functionName where
xxxx is :

<ul>
	<li>	chr - for charachter related functions
	<li>	itm - for item related functions
	<li>	menu - for menu acces ( 0.82 )
	<li>	mnu - for menu access (0.20s)
	<li>	magic - for magic access (0.50s)
	<li>	send - for packet send functions (0.50s)
	<li>	direct - for direct access functions (0.53b)
	<li>	tempfx - for temp effects access functions (0.53b)
	<li>	rgn - for region access functions (0.54b)
	<li>	trig - for triggers functions (0.54b)
	<li>	set - for sets (0.55x)
	<li>	cal - for calendar access (Sparhawk 2001-09-15}
	<li>	map - for map access (Sparhawk 082.)
	<li>	guild - for guild acces ( Endymion 0.82 )
	<li>	log - for logging messages ( Sparhawk 0.82 )
</ul>
*/

AMX_NATIVE_INFO nxw_API[] = {
// Standard commands :
 { "callFunction", _callFunction },
 { "callFunction1P", _callFunction1P },
 { "callFunction2P", _callFunction2P },
 { "callFunction3P", _callFunction3P },
 { "callFunction4P", _callFunction4P },
 { "callFunction5P", _callFunction5P },
 { "callFunction6P", _callFunction6P },
 { "sprintf", _sprintf },
 { "bypass", _bypass },
 { "setWindowTitle", _setWindowTitle },
 { "getFrameStatus", _getFrameStatus },
 { "getNXWVersion", _getNXWVersion },
 { "getNXWVersionType", _getNXWVersionType },
 { "getNXWPlatform", _getNXWPlatform },
 { "getStringMode", _getStringMode },
 { "setStringMode", _setStringMode },
 { "getCurrentTime", _getCurrentTime },
 { "getSystemTime", _getSystemTime },
 { "cfgServerOption", _cfgServerOption },
 { "weblaunch", _weblaunch },
 { "broadcast", _sysbroadcast },
 { "getIntFromDefine", _getIntFromDefine },
// Charachter functions :
 { "chr_message",  _chr_message },
 { "chr_addNPC",  _chr_addNPC},
 { "chr_addRandomNPC",  _chr_addRandomNPC},
 { "chr_canBroadcast", _canBroadcast },
 { "chr_canSeeSerials", _canSeeSerials},
 { "chr_canSnoop", _canSnoop},
 { "chr_jail", _chr_jail },
 { "chr_countBankGold", _CountBankGold},
 { "chr_countGold", _CountGold},
 { "chr_countItems", _CountColoredItems},
 { "chr_disturbMed", _disturbMed},
 { "chr_getBankBox", _GetBankBox},
 { "chr_getItemOnLayer", _GetItemOnLayer},
 { "chr_getPriv", _GetPriv},
 { "chr_getProperty", _getCharProperty },
 { "chr_getShield", _GetShield},
 { "chr_getWeapon", _GetWeapon},
 { "chr_isCounselor", _isCounselor},
 { "chr_isCriminal", _isCriminal},
 { "chr_isGM", _isGM},
 { "chr_isGMorCns", _isGMorCounselor},
 { "chr_isHuman", _isHuman},
 { "chr_isInnocent", _isInnocent},
 { "chr_isInvul", _isInvul},
 { "chr_isMurderer", _isMurderer},
 { "chr_isTrueGM", _isTrueGM},
 { "chr_isGrey", _isGrey},
 { "chr_makeInvul", _makeInvulnerable},
 { "chr_makeVulnerable", _makeVulnerable},
 { "chr_moveTo", _moveTo},
 { "chr_remove", _chr_remove },
 { "chr_setCriminal", _setCriminal},
 { "chr_setGrey", _setGrey},
 { "chr_setPermaGrey", _setPermaGrey},
 { "chr_setInnocent", _setInnocent},
 { "chr_setMurderer", _setMurderer},
 { "chr_setPriv", _setPriv},
 { "chr_setMultiSerial", _setMultiSerial},
 { "chr_setOwnSerial", _setOwnSerial},
 { "chr_setOwnSerOnly", _setOwnSerialOnly},
 { "chr_setProperty", _setCharProperty },
 { "chr_checkEquipment", _chr_CheckEquipment },
 { "chr_unhide", _unhide},
 { "chr_calcAtt", _chr_calcAtt },
 { "chr_calcDef", _chr_calcDef },
 { "chr_fish", _chr_fish },
 { "chr_isOnline", _chr_isOnline },
 { "chr_doCombatAction", _chr_doCombatAction },
 { "chr_stable", _chr_stable },
 { "chr_unStable", _chr_unStable },
//
// Sparhawk: 	Guild related functions to be ported to new guild system
//		To maintain backward compatability these will be implemented as stock functions that map to the new guild functions
//
 { "chr_guildCompare", _chr_guildCompare },
 { "chr_getGuild", _chr_getGuild },
 { "chr_setGuild", _chr_setGuild },
 { "chr_getGuildType", _chr_getGuildType },
 { "chr_setGuildType", _chr_setGuildType },
 { "chr_isGuildTraitor", _chr_isGuildTraitor },
 { "chr_setGuildTraitor", _chr_setGuildTraitor },
 { "chr_hasGuildToggle", _chr_hasGuildToggle },
 { "chr_setGuildToggle", _chr_setGuildToggle },
 { "chr_getGuildFealty", _chr_getGuildFealty },
 { "chr_setGuildFealty", _chr_setGuildFealty },
 { "chr_getGuildNumber", _chr_getGuildNumber },
 { "chr_setGuildNumber", _chr_setGuildNumber },
 { "chr_getGuildTitle", _chr_getGuildTitle },
 { "chr_setGuildTitle", _chr_setGuildTitle },
//
//
 { "chr_flee", _chr_flee },
 { "chr_attackStuff", _chr_attackStuff },
 { "chr_helpStuff", _chr_helpStuff },
 { "chr_action", _chr_action },
 { "chr_checkSkill", _chr_checkSkill },
 { "chr_unmountHorse", _chr_unmountHorse },
 { "chr_mountHorse", _chr_mountHorse },
 { "chr_getBackpack", _chr_getBackpack },
 { "chr_getEventHandler", _chr_getEventHandler  },
 { "chr_setEventHandler", _chr_setEventHandler  },
 { "chr_delEventHandler", _chr_delEventHandler  },
 { "chr_sound", _chr_sound },
 { "chr_resurrect", _chr_resurrect },
 { "chr_update", _chr_updatechar },
 { "chr_owns", _chr_owns },
 { "chr_npcAttack", _chr_npcattack },
 { "chr_npcRelease", _chr_npcRelease },
 { "chr_distance", _chr_distance },
 { "chr_speech", _chr_speech },
 { "chr_teleport", _chr_teleport },
 { "chr_lineOfSight", _chr_lineOfSight },
 { "chr_poison", _chr_poison },
 { "chr_hide", _chr_hide },
 { "chr_applyDamage", _chr_applyDamage },
 { "chr_setRandomName", _chr_setRandomName},
 { "chr_showMessage", _chr_showMessage },
 { "chr_showPaperdoll", _chr_showPaperdoll },
 { "chr_setCreationDay", _chr_setCreationDay },
 { "chr_getCreationDay", _chr_getCreationDay },
 { "chr_morph", _chr_morph },
 { "chr_unmorph", _chr_unmorph },
 { "chr_skillMakeMenu", _chr_skillMakeMenu },
 { "chr_possess", _chr_possess },
 { "chr_equip", _chr_equip},
 { "chr_kill", _chr_kill},
 { "chr_showStatus", _chr_showStatus},
 { "chr_getIP", _chr_getIP},
 { "chr_disconnect", _chr_disconnect},
 { "chr_ban", _chr_ban},
 { "chr_unban", _chr_unban},
 { "chr_showSellGump", _chr_showSellGump},
 { "chr_showBuyGump", _chr_showBuyGump},
//
// Local property functions
//
 { "chr_getLocalVarErr", _chr_getLocalVarErr },
 { "chr_isaLocalVar",    _chr_isaLocalVar    },
 { "chr_delLocalVar",    _chr_delLocalVar    },
 { "chr_addLocalIntVar", _chr_addLocalIntVar },
 { "chr_getLocalIntVar", _chr_getLocalIntVar },
 { "chr_setLocalIntVar", _chr_setLocalIntVar },
 { "chr_addLocalStrVar", _chr_addLocalStrVar },
 { "chr_getLocalStrVar", _chr_getLocalStrVar },
 { "chr_setLocalStrVar", _chr_setLocalStrVar },
 { "chr_countLocalVar",  _var_countLocalVar  },
 { "chr_firstLocalVar",  _var_firstLocalVar  },
 { "chr_nextLocalVar",   _var_nextLocalVar   },

 { "chr_addLocalIntVec", _chr_addLocalIntVec },
 { "chr_getLocalIntVec", _chr_getLocalIntVec },
 { "chr_setLocalIntVec", _chr_setLocalIntVec },
 { "chr_sizeofLocalVar", _var_sizeofLocalVar },

// Items functions :
 { "itm_getProperty", _getItemProperty},
 { "itm_setProperty", _setItemProperty},
 { "itm_isWeaponLike", _itm_isWeaponLike},
 { "itm_contAddItem", _contAddItem },
 { "itm_color", _color },
 { "itm_contPileItem", _contPileItem },
 { "itm_contCountItems", _contCountItems },
 { "itm_countItemsByID", _countItemsByID },
 { "itm_contDelAmount", _contDelAmount },
 { "itm_delAmountByID", _DelAmountByID },
 { "itm_moveTo", _imoveto },
 { "itm_pileItem", _pileitem },
 { "itm_reduceAmount", _reduceamount },
 { "itm_setContSerial", _isetContSerial },
 { "itm_setContSerOnly", _isetContSerialonly },

 { "itm_setMultiSerial", _isetMultiSerial },
 { "itm_setOwnSerial", _isetOwnSerial },
 { "itm_setOwnSerOnly", _isetOwnSerialOnly },
 { "itm_setSerial", _isetSerial },
 { "itm_create", _itm_createFromScript },
 { "itm_createByDef", _itm_createByDef },
 { "itm_createInBp", _itm_createInBp },
 { "itm_createInBpDef", _itm_createInBpDef },
 { "itm_createInBank", _itm_createInBank },
 { "itm_createInBankDef", _itm_createInBankDef },
 { "itm_checkDecay", _itm_checkDecay },
 { "itm_remove", _itm_remove },
 { "itm_getEventHandler", _itm_getEventHandler },
 { "itm_setEventHandler", _itm_setEventHandler  },
 { "itm_delEventHandler", _itm_delEventHandler  },
 { "itm_sound", _itm_sound },
 { "itm_refresh", _itm_refresh},
 { "itm_speech", _itm_speech},
 { "itm_BounceToPack", _ItemBounceToPack},
 { "itm_getCombatSkill", _itm_getCombatSkill},
 { "itm_showContainer", _itm_showContainer},
//
// Local property functions
//
 { "itm_getLocalVarErr", _itm_getLocalVarErr },
 { "itm_isaLocalVar",    _itm_isaLocalVar    },
 { "itm_delLocalVar",    _itm_delLocalVar    },
 { "itm_addLocalIntVar", _itm_addLocalIntVar },
 { "itm_getLocalIntVar", _itm_getLocalIntVar },
 { "itm_setLocalIntVar", _itm_setLocalIntVar },
 { "itm_addLocalStrVar", _itm_addLocalStrVar },
 { "itm_getLocalStrVar", _itm_getLocalStrVar },
 { "itm_setLocalStrVar", _itm_setLocalStrVar },
 { "itm_countLocalVar",  _var_countLocalVar  },
 { "itm_firstLocalVar",  _var_firstLocalVar  },
 { "itm_nextLocalVar",   _var_nextLocalVar   },
/*
 { "itm_addLocalIntVec", _itm_addLocalIntVec },
 { "itm_getLocalIntVec", _itm_getLocalIntVec },
 { "itm_setLocalIntVec", _itm_setLocalIntVec },
 { "itm_sizeofLocalVar", _var_sizeofLocalVar },
*/

// Magic functions :
 { "magic_castField", _magic_castFieldSpell },
 { "magic_chkSpellbook", _magic_checkSpellbook },
 { "magic_chkMagicRef", _magic_checkMagicReflection },
 { "magic_chkMana", _magic_checkMana },
 { "magic_castExplosion", _magic_castExplosion },
 { "magic_castClumsy", _magic_castClumsy },
 { "magic_castCurse", _magic_castCurse },
 { "magic_castFeeblem", _magic_castFeebleMind },
 { "magic_castFireball", _magic_castFireball },
 { "magic_castFlameStr", _magic_castFlameStrike },
 { "magic_castHarm", _magic_castHarm },
 { "magic_castMagicArr", _magic_castMagicArrow },
 { "magic_castLightning", _magic_castLightning },
 { "magic_castMindBlast", _magic_castMindBlast },
 { "magic_castParalyze", _magic_castParalyze },
 { "magic_castWeaken", _magic_castWeaken },
 { "magic_cast",  _magic_cast },
// Send functions :
 { "send_send", _send_send },
 { "send_confirmAttack", _send_confirmAttack },
 { "send_movingfx", _send_movingfx },
 { "send_staticfx", _send_staticfx },
 { "send_statUpdate", _send_statUpdate },
 { "send_boltfx", _send_boltfx },
 { "send_questArrow", _send_questArrow },
// Temp Effects functions :
 { "tempfx_activate", _tempfx_activate },
 { "tempfx_delete", _tempfx_delete },
 { "tempfx_isActive", _tempfx_isActive },
// Old Menu functions :
 { "mnu_prepare", _mnu_Prepare },
 { "mnu_setStyle", _mnu_SetStyle },
 { "mnu_setTitle", _mnu_SetTitle },
 { "mnu_setColor", _mnu_SetColor },
 { "mnu_addItem", _mnu_AddItem },
 { "mnu_show", _mnu_Show },
 { "mnu_setCallback", _mnu_SetCallback },
// New Menu functions :

 { "gui_create", _gui_create },
 { "gui_delete", _gui_delete },
 { "gui_show", _gui_show },
 { "gui_getProperty", _getMenuProperty },
 { "gui_setProperty", _setMenuProperty },
 { "gui_addBackground", _gui_addBackground },
 { "gui_addButton", _gui_addButton },
 { "gui_addButtonFn", _gui_addButtonFn },
 { "gui_addCheckbox", _gui_addCheckbox },
 { "gui_addCheckTrans", _gui_addCheckTrans },
 { "gui_addCroppedText", _gui_addCroppedText },
 { "gui_addGump", _gui_addGump },
 { "gui_addHtmlGump", _gui_addHtmlGump },
 { "gui_addInputField", _gui_addInputField },
 { "gui_addPropField", _gui_addPropField },
 { "gui_addRadioButton", _gui_addRadioButton },
 { "gui_addResizeGump", _gui_addResizeGump },
 { "gui_addText", _gui_addText },
 { "gui_addTilePic", _gui_addTilePic },
 { "gui_addTiledGump", _gui_addTiledGump },
 { "gui_addXmfHtmlGump", _gui_addXmfHtmlGump },
 { "gui_addPage", _gui_addPage },
 { "gui_addPageButton", _gui_addPageButton },
 { "gui_addGroup", _gui_addGroup },

 { "gui_createIconList", _gui_createIconList },
 { "gui_addIcon", _gui_addIcon },
// Region functions :
 { "rgn_setWeather", _rgn_setWeather },
 { "rgn_getWeather", _rgn_getWeather },
 { "rgn_canMark", _rgn_getMarkAllowed },
 { "rgn_setMark", _rgn_setMarkAllowed },
 { "rgn_canGate", _rgn_getGateAllowed },
 { "rgn_setGate", _rgn_setGateAllowed },
 { "rgn_canRecall", _rgn_getRecallAllowed },
 { "rgn_setRecall", _rgn_setRecallAllowed },
 { "rgn_noMagicDamage", _rgn_getNoMagicDamage },
 { "rgn_setMagicDamage", _rgn_setNoMagicDamage },
 { "rgn_isGuarded", _rgn_getGuarded },
 { "rgn_setGuarded", _rgn_setGuarded },
 { "rgn_isValid", _rgn_isValid },
 { "rgn_getName", _rgn_getName },
 { "rgn_getRegion", _rgn_getRegion },
 { "rgn_getProperty", _region_getProperty },
 { "rgn_setProperty", _region_setProperty },
 { "rgn_respawn", _region_respawn },
 { "rgn_clearSpawn", _rgn_clearSpawn },

// Trigger functions :
 { "trig_getTItem", _getTriggeredItem },
 { "trig_getTrigType", _getTriggeredItemTrigType },
// Set functions :
 { "set_create", _set_create },
 { "set_delete", _set_delete },
 { "set_clear", _set_clear },

 { "set_rewind", _set_rewind },
 { "set_next", _set_next },
 { "set_end", _set_end },
 { "set_size", _set_size },

 { "set_get", _set_get },
 { "set_getChar", _set_getChar },
 { "set_getItem", _set_getItem },
 { "set_add", _set_add },

 { "set_addOwnedNpcs", _set_addOwnedNpcs },
 { "set_addPartyFriend", _set_addPartyFriend },
 { "set_addNpcsNearXY", _set_addNpcsNearXY },
 { "set_addNpcsNearObj", _set_addNpcsNearObj },
 { "set_addItemsInCont", _set_addItemsInCont },
 { "set_addItemWeared", _set_addItemWeared },
 { "set_addItemsAtXY", _set_addItemsAtXY },
 { "set_addItemsNearXY", _set_addItemsNearXY },
 { "set_addAllOnlinePl", _set_addAllOnlinePl },
 { "set_addOnPlNearObj", _set_addOnlinePlNearObj },
 { "set_addOnPlNearXY", _set_addOnlinePlNearXY },
 { "set_addGuildMembers", _set_addGuildMembers },
 { "set_addGuildRecruit", _set_addGuildRecruit },
 { "set_addGuilds", _set_addGuilds },
 { "set_addAllRaces", _set_addAllRaces },
 { "set_getRaceStuff", _set_getRaceStuff },

 // House sets
 { "set_addCoOwners", _set_addHouseCoowners },
 { "set_addFriends", _set_addHouseFriends },
 { "set_addBanned", _set_addHouseBans },
 { "set_getOwnedHouses", _set_getOwnedHouses },
 { "set_getItemsInHouse", _set_getItemsInHouse },
 { "set_getItemsOutsideHouse", _set_getItemsOutsideHouse },
 { "set_getCharsInHouse", _set_getCharsInHouse },
 { "set_getCharsOutsideHouse", _set_getCharsOutsideHouse },
 
 // Account sets
 { "set_addAccountChars", _set_addAccountChars },

 // calendar properties - [Sparhawk] 2001-09-15
 { "cal_getProperty"		,	_getCalProperty			},
// Map functions - for experimental small npc ai Sparhawk
 { "map_canMoveHere", _map_canMoveHere },
 { "map_distance", _map_distance },
 { "map_getTileName", _map_getTileName},
 { "map_isUnderStatic", _map_isUnderStatic}, //Keldan 2003/02/09
 { "map_getTileID", _map_getTileID}, //Keldan, posted 2003/01/27 - added 2003/03/01
 { "map_getFloorTileID", _map_getFloorTileID}, // Keldan, posted 2003/01/27 - added 2003/03/01
 { "map_getZ", _map_getZ}, // Frodo 2003/12/27
// Guild function and properties - Endymion
 { "guild_setProperty", _guild_setProperty },
 { "guild_getProperty", _guild_getProperty },
 { "guild_create", _guild_create },
 { "guild_remove", _guild_remove },
 { "guild_addMember", _guild_addMember },
 { "guild_resignMember", _guild_resignMember },
 { "guild_memberAtIndex", _guild_memberAtIndex },
 { "guild_getMemberIdx", _guild_getMemberIdx },
 { "guild_getRecruitIdx", _guild_recruitIndex },
 { "guild_getRecrAtIdx", _guild_recruitAtIndex },
 { "guild_addRecruit", _guild_addRecruit },
 { "guild_refuseRecruit", _guild_refuseRecruit },
 { "guild_hasWarWith", _guild_hasWarWith },
 { "guild_hasPeaceWith", _guild_hasPeaceWith },
 { "guild_hasAllianceWith", _guild_hasAllianceWith },
// Guild member function and properties - Endymion
 { "gmember_setProperty", _guildMember_setProperty },
 { "gmember_getProperty", _guildMember_getProperty },
// Guild member function and properties - Endymion
 { "grecrui_setProperty", _guildRecruit_setProperty },
 { "grecrui_getProperty", _guildRecruit_getProperty },
// Timer function - Endymion
// Log message functions - Sparhawk
 { "log_message", _log_message },
 { "log_warning", _log_warning },
 { "log_error", _log_error },
 { "log_critical", _log_critical },
// file functions - Sparhawk
 { "file_open",	_file_open },
 { "file_close", _file_close },
 { "file_read", _file_read },
 { "file_write", _file_write },
 { "file_eof", _file_eof },
// race functions - Endymion
 { "race_getProperty",	_getRaceProperty },
 { "race_getGlobalProp",	_getRaceGlobalProp },
// target functions - Endymion
 { "target_create",	_target_create },
 { "target_do",	_target_do },
// { "target_getProperty", _target_getProperty },
// { "target_setProperty", _target_getProperty },
// party functions - Endymion
 { "party_create",	_party_create },
 { "party_addMember",	_party_addMember },
 { "party_delMember",	_party_delMember },
 { "party_addCandidate",	_party_addCandidate },
 { "party_delCandidate",	_party_delCandidate },
 { "party_isCandidate",	_party_isCandidate },
 { "party_getProperty",	_party_getProperty },
 { "party_setProperty",	_party_setProperty },
 { "party_broadcast",	_party_broadcast },
// gm paging system
 { "addGmPage", _addGmPage },
 { "getGmPageList", _getGmPageList },
 { "chr_getGmPage", _chr_getGmPage },
 { "chr_solveGmPage",  _chr_solveGmPage },
// house system
 { "house_getProperty",  _house_getProperty },
 { "house_setProperty",  _house_setProperty},
 { "house_lockItem",  _house_lockItem },
 { "house_unlockItem",  _house_unlockItem },
 { "house_secureContainer",  _house_secureContainer },
 { "house_unsecureContainer",  _house_unsecureContainer },
 { "house_placeTrashBarrel",  _house_placeTrashBarrel },
 { "house_isInsideHouse",  _house_isInsideHouse },
 { "house_isInHouse",  _house_isInHouse },
 { "house_addFriend",  _house_addFriend },
 { "house_removeFriend",  _house_removeFriend },
 { "house_isFriend",  _house_isFriend },
 { "house_addCoOwner",  _house_addCoOwner },
 { "house_removeCoOwner",  _house_removeCoOwner },
 { "house_isCoOwner",  _house_isCoOwner },
 { "house_addBan",  _house_addBan },
 { "house_removeBan",  _house_removeBan },
 { "house_isBanned",  _house_isBanned },
 { "house_transfer",_house_transfer  },
 { "house_changeLocks",  _house_changeLocks },
 { "house_makeKeys",  _house_makeKeys },
 { "house_deleteKeys",  _house_deleteKeys },
 { "house_delete",  _house_delete },
 
// misc APIs need to be renamed
 { "world_save", _world_save },
 { "chr_setPostType", _chr_setPostType },
 { "setLightLevel", _setLightLevel },
 { "reload_accounts", _reload_accounts },
 { "reload_scripts", _reload_scripts },
 { "reload_commands", _reload_commands },
 { "shutdown", _shutdown },
 { "setSecondsPerUoMinute", _setSecondsPerUoMinute },
 { "garbageCollection", _garbageCollection },
 { "recompileSmall", _recompileSmall },
 { "isChar", _isChar },
 { "isItem", _isItem },
 { "loadDefaults", _loadDefaults },
 { "pdump", _pdump },

 // resource map api
 { "createResourceMap", _createResourceMap },
 { "deleteResourceMap", _deleteResourceMap },
 { "getResourceLocationValue", _getResourceLocationValue },
 { "setResourceLocationValue", _setResourceLocationValue },
 { "getResourceStringValue", _getResourceStringValue },
 { "setResourceStringValue", _setResourceStringValue },
 { "getResourceStringString", _getResourceStringString },
 { "setResourceStringString", _setResourceStringString },

// speech APIs
 { "chr_getSpeech", _chr_getCmdSpeech },
// accounts
 { "account_delete", _account_delete },
 { "account_block", _account_block },
 { "account_changePW", _account_changePW },
 { "account_getLastIP", _account_getLastIP },
 { "account_getName", _account_getName},

// Terminator :
 { NULL, NULL }
};
