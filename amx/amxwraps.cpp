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
#include "nxwGump.h"
#include "menu.h"

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
#include "collector.h"

//property system lies in amxwrap2.cpp
/* Sparhawk 2001-09-15 added access to calendar properties */
NATIVE2(_getCharProperty);
NATIVE2(_setCharProperty);
NATIVE2(_getItemProperty);
NATIVE2(_setItemProperty);
NATIVE2(_getCalProperty);
NATIVE2(_setGuildProperty);
NATIVE2(_getGuildProperty);

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

/*!
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

/*!
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

/*!
\brief bypasses default server behaviour
\author Xanathar
\since 0.10
*/
NATIVE(_bypass)
{ g_bByPass = true; return 0; }

/*!
\brief get the frame status
\author Xanathar
\since 0.20
\return 1 in the after phase of a frame override, 0 otherwise
*/
NATIVE(_getFrameStatus)
{ return (g_nMoment==AMX_AFTER) ? 1 : 0; }

/*!
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
\brief brings up a target, that will invoke the specific callback when the choice is made
\author Xanathar
\since 0.20
\param 1: sockete
\param 2: callback
\param 3: format[]
*/
NATIVE(_getTarget)
{
	cell *cstr;
	amx_GetAddr(amx,params[3],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
	return amxTarget(params[1], params[2], g_cAmxPrintBuffer);
}

/*!
\brief add a timer to the timer queque
\author Xanathar
\since 0.20
\param 1: function index
\param 2: interval
*/
NATIVE(addTimer)
{ return addTimer(params[1],params[2],params[3],params[4]); }

/*!
\brief prepares a socket for a menu, initializing it and destroying any previous menu
\author Xanathar
\since 0.20
\param 1: socket
\param 1: number of pages
\param 1: number of item for page
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_menu_Prepare)
{
//	int s = calcSocketFromChar(params[1]);
	if ( params[1] < 0 )
		return INVALID;
    NXWCLIENT s = getClientFromSocket(params[1]);
    P_CHAR pc = s->currChar();
    //P_CHAR pc = &chars[currchar[params[1]]];
	VALIDATEPCR( pc, INVALID );
	if (pc->customMenu != NULL)
		safedelete(pc->customMenu);
	pc->customMenu = new cAmxMenu();
	pc->customMenu->setParameters(params[3],params[2]);
	return 0;
}

/*!
\brief set menu style and eventually the color
\author Xanathar
\since 0.20
\param 1: socket
\param 2: style
\param 3: color ( default 0 )
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_menu_SetStyle)
{
	if ( params[1] < 0 )
		return INVALID;
    NXWCLIENT s = getClientFromSocket(params[1]);
    P_CHAR pc = s->currChar();
	VALIDATEPCR( pc, INVALID );
	pc->customMenu->setStyle(params[2], params[3]);
	return 0;
}

/*!
\brief set the menu title
\author Xanathar
\since 0.20
\param 1: socket
\param 2: title
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_menu_SetTitle)
{
	if ( params[1] < 0 )
		return INVALID;
    NXWCLIENT s = getClientFromSocket(params[1]);
    P_CHAR pc = s->currChar();
	VALIDATEPCR( pc, INVALID );
	cell *cstr;
	amx_GetAddr(amx,params[2],&cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	pc->customMenu->setTitle(g_cAmxPrintBuffer);
	g_nAmxPrintPtr=0;
	return 0;
}

/*!
\brief set the menu color
\author Xanathar
\since 0.20
\param 1: socket
\param 2: color
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_menu_SetColor)
{
	if ( params[1] < 0 )
		return INVALID;
    NXWCLIENT s = getClientFromSocket(params[1]);
	P_CHAR pc = s->currChar();
	pc->customMenu->setColor(params[2]);
	return 0;
}

/*!
\brief adds an item at a given position of a correctly inizialized menu
\author Xanathar
\since 0.20
\param 1: socket
\param 2: page number
\param 3: index number
\param 4: text
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_menu_AddItem)
{
	if ( params[1] < 0 )
		return INVALID;
    NXWCLIENT s = getClientFromSocket(params[1]);
    P_CHAR pc = s->currChar();
	VALIDATEPCR( pc, INVALID );
	cell *cstr;
	amx_GetAddr(amx,params[4],&cstr);
	printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	pc->customMenu->addMenuItem(params[2],params[3],g_cAmxPrintBuffer);
	g_nAmxPrintPtr=0;
	return 0;
}

NATIVE(_gump_show)
{
//	Gumps->Menu(s, params[1],-1);
	return 0;
}

/*!
\brief pop up the menu window
\author Xanathar
\since 0.20
\param 1: socket
\return 0 if success, INVALID if error ( es socket invalid )
*/
NATIVE(_menu_Show)
{
	if ( params[1] < 0 )
		return INVALID;
    NXWCLIENT s = getClientFromSocket(params[1]);
    P_CHAR pc = s->currChar();
	VALIDATEPCR( pc, INVALID );
	pc->customMenu->buildMenu();
	pc->customMenu->showMenu( params[1] );
	return 0;
}

/*!
\brief set the callback for this menu events
\author Xanathar
\since 0.20
\param 1: socket
\param 1: callback
\return 0 if success, INVALID if error ( es socket invalid )
\note callback can be obtained by call to funcidx
*/
NATIVE(_menu_SetCallback)
{
	if ( params[1] < 0 )
		return INVALID;
    NXWCLIENT s = getClientFromSocket(params[1]);
    P_CHAR pc = s->currChar();
	VALIDATEPCR( pc, INVALID );
	pc->customMenu->setCallback(params[2]);
	return 0;
}

//
// GUMP API -- still highly experimental
//

NATIVE ( _gui_create )
{
	return ( nxwGump::createGump( params[1], params[2], params[3], (params[4]?true:false), (params[5]?true:false), (params[6]?true:false), params[7] ) ? 1 : 0);
}

NATIVE ( _gui_delete )
{
	return nxwGump::deleteGump( params[1] );
}

NATIVE ( _gui_show )
{
	P_CHAR pc = pointers::findCharBySerial( params[2] ); //Luxor bug fix
	VALIDATEPCR( pc, 0 );
	return ( nxwGump::showGump( params[1], pc ) ? 1 : 0 );
}

NATIVE ( _gui_addButton )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
			thisGump->addButton( params[2], params[3], params[4], params[5], params[6] );
			return 1;
	}
	return 0;
}

NATIVE ( _gui_addGump )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
			thisGump->addGump( params[2], params[3], params[4], params[5] );
			return 1;
	}
	return 0;
}

NATIVE ( _gui_addPage )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
		thisGump->addPage( params[2] );
		return 1;
	}
	return 0;
}

NATIVE (  _gui_addResizeGump )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
		thisGump->addResizeGump( params[2], params[3], params[4], params[5], params[6] );
		return 1;
	}
	return 0;
}

NATIVE ( _gui_addPageButton )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
		thisGump->addPageButton( params[2], params[3], params[4], params[5], params[6] );
		return 1;
	}
	return 0;
}

NATIVE ( _gui_addRadioButton )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
		thisGump->addRadioButton( params[2], params[3], params[4], params[5], params[6], params[7] );
		return 1;
	}
	return 0;
}

NATIVE ( _gui_addText )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
		cell *cstr;
		amx_GetAddr(amx,params[4],&cstr);
		printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
		g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
		g_nAmxPrintPtr=0;
		thisGump->addText( params[2], params[3], g_cAmxPrintBuffer, params[5] );
		return 1;
	}
	return 0;
}

NATIVE ( _gui_addCroppedText )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
		cell *cstr;
		amx_GetAddr(amx,params[6],&cstr);
		printstring(amx,cstr,params+7,(int)(params[0]/sizeof(cell))-1);
		g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
		g_nAmxPrintPtr=0;
		thisGump->addCroppedText( params[2], params[3], params[4], params[5], g_cAmxPrintBuffer, params[7] );
		return 1;
	}
	return 0;
}

NATIVE ( _gui_addTilePic )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
		thisGump->addTilePic( params[2], params[3], params[4], params[5] );
		return 1;
	}
	return 0;
}

NATIVE ( _gui_addTiledGump )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
		thisGump->addTiledGump( params[2], params[3], params[4], params[5], params[6], params[7] );
		return 1;
	}
	return 0;
}

NATIVE ( _gui_addHtmlGump )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
		cell *cstr;
		amx_GetAddr(amx,params[6],&cstr);
		printstring(amx,cstr,params+7,(int)(params[0]/sizeof(cell))-1);
		g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
		g_nAmxPrintPtr=0;
		thisGump->addHtmlGump( params[2], params[3], params[4], params[5], g_cAmxPrintBuffer, params[7], params[8] );
		return 1;
	}
	return 0;
}

NATIVE ( _gui_addInputField )
{
	nxwGump* thisGump = nxwGump::selectGump( params[1] );
	if ( thisGump != 0 )
	{
		cell *cstr;
		amx_GetAddr(amx,params[7],&cstr);
		printstring(amx,cstr,params+8,(int)(params[0]/sizeof(cell))-1);
		g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
		g_nAmxPrintPtr=0;
		thisGump->addInputField( params[2], params[3], params[4], params[5], params[6], g_cAmxPrintBuffer, params[8] );
		return 1;
	}
	return 0;
}

NATIVE ( _gui_getInputField )
{
	std::string value;
	if( nxwGump::selectResponse( params[1], value ) )
	{
	  char str[100];
  	cell *cptr;
	  strcpy(str, value.c_str() );
  	amx_GetAddr(amx,params[2],&cptr);
	  amx_SetString(cptr,str, g_nStringMode);
		return 1;
	}
	return 0;
}


//
//
//

/*!
\brief get the item that invoked the trigger
\author Xanathar
\since 0.53
\return the item
\note valid in triggers only
\note deprecated
*/
NATIVE(_getTriggeredItem)
{ return g_nTriggeredItem; }

/*!
\brief get the type of item that invoked the trigger
\author Xanathar
\since 0.53
\return the type
\note valid in triggers only
\note deprecated
*/
NATIVE(_getTriggeredItemTrigType)
{ return g_nTriggerType; }

/*!
\brief get the NoxWizard Version
\author Xanathar
\since 0.10
\return the version number
\note es 0.23 = 0x0023
*/
NATIVE(_getNXWVersion)
{ return VERHEXCODE; }

/*!
\brief get the NoxWizard Version type
\author Xanathar
\since 0.50
\return the version type
\note the type is a bitfield of VERTYPE_ definitions ( see nxw.inc )
*/
NATIVE(_getNXWVersionType)
{ return VERTYPE; }

/*!
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

/*!
\brief print a message to a given socket
\author Xanathar
\since 0.10
\param 1: socket
\param 2: text
\return
\bug there are a  if (params[1] < 0) return 0; i think not 0 buy INVALID or not?
*/
NATIVE(_nprintf)
{
  cell *cstr;
  if (params[1] < 0) return 0;
  NXWCLIENT s = getClientFromSocket(params[1]);
  if (s == NULL) return 0;
  amx_GetAddr(amx,params[2],&cstr);
  printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
  g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
  s->sysmsg(g_cAmxPrintBuffer);
  g_nAmxPrintPtr=0;
  return 0;
}

/*!
\brief print a message to a given socket using a given color
\author Xanathar
\since 0.20
\param 1: socket
\param 2: color
\param 3: text
\return
\bug there are a  if (params[1] < 0) return 0; i think not 0 buy INVALID or not?
*/
NATIVE(_ncprintf)
{
  cell *cstr;
  if (params[1] < 0) return 0;
  NXWCLIENT s = getClientFromSocket(params[1]);
  amx_GetAddr(amx,params[3],&cstr);
  printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
  g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
  s->sysmsg(params[2],g_cAmxPrintBuffer);
  g_nAmxPrintPtr=0;
  return 0;
}

/*!
\brief get the socket used by the given char
\author Xanathar
\since 0.60
\param 1: character
\return the socket, if error is INVALID
*/
NATIVE(_getSocketFromChar)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    if (ISVALIDPC(pc) && pc->getClient() != NULL) return pc->getClient()->toInt();
    return INVALID;
}

/*!
\brief get current socket
\author Xanathar
\since 0.10
\return the socket, if error is INVALID
\note deprecated
*/
NATIVE(_getCurrentSocket)
{
	WarnOut("AMX function using obsolete getCurrentSocket() function!\n");
	return g_nCurrentSocket;
}

/*!
\brief get the targetted char
\author Xanathar
\since 0.10
\return the targetted char
*/
NATIVE(_getCharTarget)
{
    P_CHAR pc = MAKE_CHAR_REF(calcCharFromPtr(buffer[g_nCurrentSocket]+7));
    VALIDATEPCR(pc, INVALID);
    return pc->getSerial32();
}

/*!
\brief get the targetted item
\author Xanathar
\since 0.10
\return the targetted item
*/
NATIVE(_getItemTarget)
{
    P_ITEM pi = MAKE_ITEM_REF(calcItemFromPtr(buffer[g_nCurrentSocket]+7));
    VALIDATEPIR(pi, INVALID);
    return pi->getSerial32();
}


/*!
\brief get the current string mode used
\author Xanathar
\since 0.10
\return the string mode
*/
NATIVE(_getStringMode)
{ 	return g_nStringMode; }

/*!
\brief set the current string mode used
\author Xanathar
\since 0.10
\param 1: new string mode ( 0=unpacked, 1=packet )
\return the string mode
*/
NATIVE(_setStringMode)
{	g_nStringMode = params[1];	return g_nStringMode; }

/*!
\brief get the current time
\author Xanathar
\since 0.10
\return the current time
*/
NATIVE(_getCurrentTime)
{	return uiCurrentTime; }

/*!
\brief get the current system time (in s)
\author Keldan
\since 0.83
\return the current system time (in s)
*/
NATIVE(_getSystemTime)
{   return getsysclock(); }

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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


/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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
    return pc->IsTrueGM();
}

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
\brief get the character by given socket
\author ??
\since ??
\param 1: the socket
\return the character
*/
NATIVE(_getCharFromSocket)
{
	if ( params[1] < 0 )
		return INVALID;
    NXWCLIENT s = getClientFromSocket(params[1]);
    if (s == NULL) return INVALID;
    P_CHAR pc = s->currChar();
    VALIDATEPCR(pc, INVALID);
	return pc->getSerial32();
}

/*!
\brief add given item at the given cantainer in the given position
\author Xanathar
\since 0.10
\param 1: the container
\param 2: the item
\param 3: x location
\param 4: y location
\return true or false
\bug return INVALID insted of false in VALIDATEPIR, i think it's an error
*/
NATIVE(_contAddItem)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	P_ITEM pi1 = pointers::findItemBySerial(params[2]);
	VALIDATEPIR( pi1, INVALID );
	return pi->AddItem(pi1, params[3],params[4]); 
}


/*!
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
	return pi->color();
}

/*!
\brief Pile an item into container
\author Xanathar
\since 0.10
\param 1: container
\param 2: item
\return true or false
\bug return INVALID insted of false in VALIDATEPIR, i think it's an error
*/
NATIVE(_contPileItem)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	P_ITEM pi1 = pointers::findItemBySerial(params[2]);
	VALIDATEPIR( pi1, INVALID );
	return pi->AddItem(pi1);
}

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
\brief pile an item into a container
\author Xanathar
\since 0.10
\param 1: container
\param 2: item
\return INVALID if error or not piled, or piled Into item
*/
NATIVE(_pileitem)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
	VALIDATEPIR( pi, INVALID );
	P_ITEM pi1 = pointers::findItemBySerial(params[2]);
	VALIDATEPIR( pi1, INVALID );
	return pi->PileItem(pi1);
}

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
\brief create a new item from the script
\author Xanathar
\since 0.54
\param 1: itemtype
\return created item or INVALID if not valid item
\note it's up to you to position the item in the world
*/
NATIVE(_itm_createFromScript)
{
    P_ITEM pi = item::CreateFromScript(-1,params[1]);
	VALIDATEPIR(pi, INVALID);
    pi->Refresh();
	return pi->getSerial32();
}

/*!
\brief get the backpack of given character
\author Xanathar
\since 0.50
\param 1: character
\return backpack item or INVALID if not valid char or haven't backpack
*/
NATIVE(_itm_getCharBackPack)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR( pc, INVALID );
	P_ITEM pi= pc->getBackpack();
	VALIDATEPIR( pi, INVALID );
	return pi->getSerial32();
}

/*!
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

/*!
\brief created an item of given type in the backpack of char with given socket
\author Xanathar
\since ??
\param 1: socket
\param 2: item type
\return created item or INVALID if not valid socket or item
*/
NATIVE(_createiteminbackpack)
{
    NXWCLIENT s = getClientFromSocket(params[1]);
    P_ITEM pi = item::SpawnItemBackpack2(s->toInt(), params[2] , 0);
    VALIDATEPIR(pi, INVALID);
    return pi->getSerial32();
}

/*!
\brief Spawn given item in the bank of char with given socket
\author Xanathar
\since ??
\param 1: socket
\param 2: item
\return Spawn item or INVALID if not valid socket or item
*/
NATIVE(_itm_spawnBank)
{
    NXWCLIENT s = getClientFromSocket(params[1]);
	if( s==NULL ) return INVALID;
    P_CHAR pc = s->currChar();
    VALIDATEPCR(pc, INVALID);
    P_ITEM pi = item::SpawnItemBank(DEREF_P_CHAR(pc), params[2]);
    VALIDATEPIR(pi, INVALID);
    return pi->getSerial32();
}

/*!
\brief Spawn given item in the bank of given char
\author Xanathar
\since ??
\param 1: character
\param 2: item
\return Spawn item or INVALID if not valid char or item
*/
NATIVE(_chr_spawnIteminBank)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    P_ITEM pi = item::SpawnItemBank(DEREF_P_CHAR(pc), params[2]);
    VALIDATEPIR(pi, INVALID);
    return pi->getSerial32();
}

/*!
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

/*!
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
	pi->deleteItem();
	return 0;
}

/*!
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

/*!
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
    pc->deleteChar();
    return 0;
}

/*!
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

/*!
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

/*!
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
    Fishing->Fish (DEREF_P_CHAR(pc));
    return 0;
}

/*!
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
	return pChar->getGuild();
}

/*!
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

/*!
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
    return Guilds->Compare (pc1,pc2);
}


/*!
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

/*!
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

/*!
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

/*!
\brief Send a complete network packet to given socket
\author Xanathar
\since 0.50
\param 1: socket
\param 2: lenght
\param 3: data
\return 0 or INVALID if not valid socket
*/
NATIVE(_send_send)
{ // sendToUoClient(socket, lenght, const data[]);
  if ((params[2]<=0)||(params[2]>65535)) return INVALID;

  char *buf;
  cell *cstr;

  buf = new char[params[2]];

  amx_GetAddr(amx,params[3],&cstr);

  for (int i=0; i<params[2]; i++)
  {
	  buf[i] = static_cast<char>(cstr[i] & 0xFF);
  }

  Xsend(params[1], buf, params[2]);
  return 0;
}


/*!
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


/*!
\brief Send an attack confirmation to given socket for the given character
\author Xanathar
\since 0.50
\param 1: socket
\param 2: character
\return 0 or INVALID if not valid socket or character
*/
NATIVE(_send_confirmAttack)
{
    P_CHAR pc = pointers::findCharBySerial(params[2]);
    VALIDATEPCR(pc, INVALID);
    SndAttackOK(params[1], pc->getSerial32()); return 0;
}

/*!
\brief test if status request from client is for character stats
\author Sparhawk
\since 0.82
\return true or false
*/
NATIVE(_rcve_statsRequest )
{
	return ( buffer[params[1]][5] == 4 );
}

/*!
\brief test if status request from client is for character skills
\author Sparhawk
\since 0.82
\return true or false
*/
NATIVE(_rcve_skillsRequest )
{
	return ( buffer[params[1]][5] == 5 );
}


/*!
\brief get number of active socket
\author Xanathar
\since 0.50
\return socket count
\note socket numeration is 0 based
*/
NATIVE(_getSocketCount)
{ return now; }


/*!
\brief Activate a temp effect
\author Xanathar
\since 0.53
\param 1: type
\param 2: source character
\param 3: destination character
\param 4: more
\param 5: duration
\param 6: callback
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
	tempfx::add(pointers::findCharBySerial(params[2]), pointers::findCharBySerial(params[3]), params[1], (params[4])&0xFF, (params[4]>>8)&0xFF,
		(params[4]>>16)&0xFF, params[5], params[6]);
	//</Luxor>
	return 0;
}
/*!
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
	/*
	if( pc->amxevents[params[2]] != NULL )
		strcpy(str, pc->amxevents[params[2]]->getFuncName() );
	else
			str[0] = '\0';
	*/
	AmxEvent* event = pc->getAmxEvent( params[2] );
	if( event )
		strcpy(str, event->getFuncName() );
	else
			str[0] = '\0';
	amx_GetAddr(amx,params[3],&cptr);
	amx_SetString(cptr,str, g_nStringMode);
	return strlen(str);
}
/*!
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
	//pc->amxevents[params[2]] = newAmxEvent(g_cAmxPrintBuffer,params[3]!=0);
	pc->delAmxEvent( params[2] );
	pc->setAmxEvent( params[2], g_cAmxPrintBuffer, params[3] !=0 );
	g_nAmxPrintPtr=0;
	return 0;
}
/*!
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
	/*
	if( pi->amxevents[params[2]] != NULL )
		strcpy(str, pi->amxevents[params[2]]->getFuncName() );
	else
			str[0] = '\0';
	*/
	AmxEvent* event = pi->getAmxEvent( params[2] );
	if( event )
		strcpy(str, event->getFuncName() );
	else
		str[0] = '\0';
	amx_GetAddr(amx,params[3],&cptr);
	amx_SetString(cptr,str, g_nStringMode);
	return strlen(str);
}
/*!
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
	//pi->amxevents[params[2]] = newAmxEvent(g_cAmxPrintBuffer,params[3]!=0);
	pi->delAmxEvent( params[2] );
	pi->setAmxEvent( params[2], g_cAmxPrintBuffer, params[3] != 0 );
	g_nAmxPrintPtr=0;
	return 0;
}

/*!
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
	//pc->amxevents[params[2]] = NULL;
	pc->delAmxEvent( params[2] );
	return 0;
}

/*!
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
	pi->delAmxEvent( params[2] );
	return 0;
}

/*!
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


/*!
\brief send an hardcoded target to given socket
\author Xanathar
\since 0.53
\param 1: socket
\param 2: a1
\param 3: a2
\param 4: a3
\param 5: a4
\param 6: format
\return 0 or INVALID if not valid socket
*/
NATIVE (_nativeTarget)
{
	cell *cstr;
	amx_GetAddr(amx,params[6],&cstr);
    printstring(amx,cstr,params+7,(int)(params[0]/sizeof(cell))-1);
    g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
    g_nAmxPrintPtr=0;

	target(params[1], params[2], params[3], params[4], params[5], g_cAmxPrintBuffer);
	return 0;
}

/*!
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


/*!
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


/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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


/*!
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
    bolteffect(DEREF_P_CHAR(pc), true);
	return 0;
}

NATIVE (_direct_castSpell)
{
//	Magic->NewCastSpell( params[1] );
//	gic::castSpell(params[1]
	return 0;
}

/*!
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


/*!
\brief set the hardcoded spell type
\author Xanathar
\since 0.53
\param 1: socket
\param 2: type
\return old spell type or INVALID if not valid character or socket
*/
NATIVE (_direct_setSpellType)
{
    NXWCLIENT s = getClientFromSocket(params[1]);
	if( s==NULL ) return INVALID;
    P_CHAR pc = s->currChar();
    VALIDATEPCR(pc, INVALID);
    int ov = pc->spelltype;
	pc->spelltype = static_cast<magic::CastingType>(params[2]);
	return ov;
}

/*!
\brief Check the given skill in character
\author Xanathar
\since 0.53
\param 1: character
\param 2: skill
\param 3: minimun skill required
\param 4: maximum skill required
\return  0 failed, 1 right or INVALID if not valid character or socket
\bug are used params[5] but not documented
*/
NATIVE (_chr_checkSkill)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->checkSkill( static_cast<Skill>(params[2]), params[3], params[4], params[5]!=0);
}


/*!
\brief Un mount the given character
\author Xanathar
\since 0.53
\param 1: character
\return 0 or INVALID if not valid character
*/
NATIVE (_chr_unmountHorse)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->unmountHorse();
	return 0;
}

/*!
\brief The character mount the given animal
\author Xanathar
\since 0.53
\param 1: socket
\param 2: animal
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

/*!
\brief Given character play specified sound
\author Xanathar
\since 0.53
\param 1: character
\param 2: sound
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_sound)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    pc->playSFX(params[2]);
	return 0;
}

/*!
\brief Play a sound effect on a item
\author Xanathar
\since 0.54
\param 1: item
\param 2: sound
\return 0 or INVALID if not valid item
*/
NATIVE(_itm_sound)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);
    VALIDATEPIR(pi, INVALID);
	soundeffect3(pi, params[2]);
	return 0;
}

/*!
\brief Open a set
\author Endymion
\since 0.82
\return the set
*/
NATIVE( _set_create )
{
	return amxSet::create( );
}

/*!
\brief Close a set
\author Endymion
\since 0.82
\param 1: the set
\return 1
*/
NATIVE( _set_delete )
{
	amxSet::deleteSet(params[1]);
	return 0;
}

/*!
\brief Rewind a set
\author Endymion
\since 0.82
\param 1: the set
\return 0
*/
NATIVE( _set_rewind )
{
	amxSet::rewind(params[1]);
	return 0;
}

/*!
\brief Move to next item
\author Endymion
\since 0.82
\param 1: the set
\return 0
*/
NATIVE( _set_next )
{
	amxSet::next(params[1]);
	return 0;
}

/*!
\brief Check if at end of the given set
\author Endymion
\since 0.82
\param 1: the set
\return 1 if at end, 0 else
*/
NATIVE( _set_end )
{
	return amxSet::end(params[1]);
}


/*!
\brief Get number of element contained in a set
\author Endymion
\since 0.82
\param 1: the set
\return element count
*/
NATIVE ( _set_size )
{
	return amxSet::size( params[1] );
}

/*!
\brief Get current object of given set
\author Endymion
\since 0.82
\param 1: the set
\return the object
*/
NATIVE( _set_get )
{
	return amxSet::get( params[1] );
}

/*!
\brief Get current object of given set
\author Endymion
\since 0.82
\param 1: the set
\return the object
*/
NATIVE( _set_add )
{
//	amxSet::add( params[1], params[2] ); 
#warning Missing in Action .... ehm .. missing function !!! -_-'
	return 0;
}

NATIVE( _set_addOwnedNpcs ) 
{
	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );
	amxSet::addOwnedNpcs( params[1], pc, params[3]!=0, params[4]!=0 );
	return 0;
}

NATIVE( _set_addCharsNearXYZ ) 
{
	amxSet::addCharsNearXYZ( params[1], params[2], params[3], params[4], params[5]!=0, params[6]!=0 );
	return 0;
}

NATIVE( _set_addPartyFriend ) 
{
	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );
	amxSet::addPartyFriend( params[1], pc, params[3], params[4]!=0 );
	return 0;
}

NATIVE( _set_addItemsInContainer ) 
{
	P_ITEM pi=pointers::findItemBySerial( params[2] );
	VALIDATEPIR( pi, INVALID )
	amxSet::addItemsInContainer( params[1], pi, params[3]!=0, params[4]!=0 );
	return 0;
}

NATIVE( _set_addItemWeared ) 
{
	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );
	amxSet::addItemWeared( params[1], pc, params[3]!=0, params[4]!=0, params[5]!=0 );
	return 0;
}

NATIVE( _set_addItemsAtXY ) 
{
	amxSet::addItemsAtXY( params[1], params[2], params[3], params[4] );
	return 0;
}

NATIVE( _set_addItemsNearXYZ ) 
{
	amxSet::addItemsNearXYZ( params[1], params[2], params[3], params[4], params[5]!=0 );
	return 0;
}

NATIVE( _set_addAllOnlinePlayers )
{
	amxSet::addAllOnlinePlayers( params[1] );
	return 0;
}

NATIVE( _set_addOnlinePlayersNearChar )
{
	P_CHAR pc=pointers::findCharBySerial( params[2] );
	VALIDATEPCR( pc, INVALID );
	amxSet::addOnlinePlayersNearChar( params[1], pc, params[3]!=0, params[4] );
	return 0;
}

NATIVE( _set_addOnlinePlayersNearItem )
{
	P_ITEM pi=pointers::findItemBySerial( params[2] );
	VALIDATEPIR( pi, INVALID )
	amxSet::addOnlinePlayersNearItem( params[1], pi, params[3] );
	return 0;
}

NATIVE( _set_addOnlinePlayersNearXY )
{
	amxSet::addOnlinePlayersNearXY( params[1], params[2], params[3], params[4] );
	return 0;
}

/*!
\brief Resurrect a character
\author Xanathar
\param 1: the character
\return 0 il resurrect else INVALID
*/
NATIVE(_chr_resurrect)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc,INVALID);
	if (pc->dead && pc->IsOnline() ) {
		pc->resurrect();
		return 0;
	}
	else 
		return INVALID;
}


/*!
\brief generic npctalk/npcemote handler
\author Sparhawk
\since 0.60
\param 1: required function
\param 2: player character (only used for npctalk,npctalk_runic,npcemote)
\param 3: npc who'll do the talking or emoting
\param 4: text to talk or emote
\param 5: antispam flag
\return 0 or INVALID if not valid character
\note handle npctalk, npctalkall, npctalk_runic, npctalkall_runic, npcemote, npcemoteall
*/
NATIVE(_chr_speech)
{
    P_CHAR pc = pointers::findCharBySerial(params[2]);
    P_CHAR pc2 = pointers::findCharBySerial(params[3]);
    VALIDATEPCR(pc2, INVALID);

	switch( params[1] )
	{
		case NXW_SPEECH_TALK:
		case NXW_SPEECH_TALK_RUNIC:
		case NXW_SPEECH_EMOTE:
		VALIDATEPCR(pc, INVALID);
	}

    cell *cstr;
	amx_GetAddr(amx, params[4], &cstr);
	printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;

	switch( params[1] )
	{
		case NXW_SPEECH_TALK : pc2->talk(DEREF_P_CHAR(pc), g_cAmxPrintBuffer, (char) params[5]);
			break;
		case NXW_SPEECH_TALK_ALL : pc2->talkAll( g_cAmxPrintBuffer, (char) params[5]);
			break;
		case NXW_SPEECH_TALK_RUNIC : pc2->talkRunic(DEREF_P_CHAR(pc), g_cAmxPrintBuffer, (char) params[5]);
			break;
		case NXW_SPEECH_TALK_ALL_RUNIC : pc2->talkAllRunic( g_cAmxPrintBuffer, (char) params[5]);
			break;
		case NXW_SPEECH_EMOTE : pc2->emote(DEREF_P_CHAR(pc), g_cAmxPrintBuffer,(char) params[5]);
			break;
		case NXW_SPEECH_EMOTE_ALL : pc2->emoteall( g_cAmxPrintBuffer, (char) params[5]);
			break;
		default : WarnOut("_chr_speech called with invalid params[1] (%d)\n", params[1]);
			break;
	}
	return 0;
}


/*!
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

		int length = 44 + strlen(g_cAmxPrintBuffer) + 1;
		UI08 talk[14] = { 0x1C, 0x00, };

		// Block size
		ShortToCharPtr(length, talk +1);
		// char serial
		LongToCharPtr(pc2->getSerial32(), talk +3);
		// Model ID
		ShortToCharPtr(pc2->GetBodyType(), talk +7);
		// Speech Type
		talk[9]= 6;
		// Speech color
		ShortToCharPtr(params[4], talk +10);
		// Speech font
		ShortToCharPtr((UI16)pc1->fonttype, talk +12);

		UI08 sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

		Xsend(pc1->getClient()->toInt(), talk, 14);
		Xsend(pc1->getClient()->toInt(), sysname, 30);
		Xsend(pc1->getClient()->toInt(), g_cAmxPrintBuffer, length-44);
//AoS/		Network->FlushBuffer(pc1->getClient()->toInt());
		return 0;
	}
	return INVALID;
}

/*!
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

/*!
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

/*!
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

/*!
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

/*!
\brief Get the serial from an item
\author Luxor
\since 0.7
\param 1: item
\return serial of item or INVALID if not valid item
\note useless since i've changed the whole system to use serialz
*/
NATIVE(_getItemFromSerial)
{
    P_ITEM pi = pointers::findItemBySerial(params[1]);
    VALIDATEPIR(pi, INVALID);
    return pi->getSerial32();
}

/*!
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
	pc->npcWander=2;
	pc->setOwnerSerial32(-1);
	pc->tamed = false;
	return 0;
}

/*!
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


/*!
\brief Get the serial from an character
\author Luxor
\since 0.7
\param 1: character
\return serial of character or INVALID if not valid character
\note useless since i've changed the whole system to use serialz
*/
NATIVE(_getCharFromSer)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    return pc->getSerial32();
}

/*!
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

/*!
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

/*!
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
	return line_of_sight(params[1], params[2], params[3], params[4], params[5], params[6], params[7], params[8]);
}



/*!
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
/*!
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

/*!
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
/*!
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

/*!
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
/*!
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

/*!
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
/*!
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

/*!
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
/*!
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

/*!
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
/*!
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

/*!
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

/*!
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


/*!
\brief spawns an item
\author Luxor
\since 0.7
\param  1: socket
\param  2: character
\param  3: amount
\param  4: name of the item
\param  5: stackable
\param  6: cItemId1
\param  7: cItemId2
\param  8: cColorId1
\param  9: cColorId2
\param 10: pack
\param 11: send
\return item
*/
NATIVE(_itm_spawnItem)
{
    P_CHAR pc = pointers::findCharBySerial(params[2]);
    VALIDATEPCR(pc, INVALID);

    int nPackSerial = INVALID;
    P_ITEM pi = pointers::findItemBySerial(params[10]);
    if (ISVALIDPI(pi)) nPackSerial = pi->getSerial32();

    cell *cstr;
	int p[15];
	for (int i=0; i<=11; i++) p[i] = params[i];
	amx_GetAddr(amx,params[4],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
	P_ITEM pi_s = item::SpawnItem(p[1],DEREF_P_CHAR(pc),p[3],g_cAmxPrintBuffer,p[5],(p[6]<<8)+p[7], (p[8]<<8)+p[9],nPackSerial,p[11]);
	VALIDATEPIR(pi_s,INVALID);
	return pi_s->getSerial32();
}


NATIVE(_itm_spawnNecroItem)
{/*
    int nPackSerial = INVALID;
    P_ITEM pi = pointers::findItemBySerial(params[2]);
    if (ISVALIDPI(pi)) nPackSerial = pi->getSerial32();
	cell *cstr;
	amx_GetAddr(amx,params[3],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;
	return Necro::SpawnRandomItem(params[1],nPackSerial,"ITEMLIST",g_cAmxPrintBuffer);*/
	return INVALID;
}

/*!
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
\param  8: backup??
\param  9: ??
\return 0 or INVALID if not valid character
*/
NATIVE(_chr_morph)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);

    bool bBackup;
    cell *cstr;
    if (params[9] < 0) return 0;
    amx_GetAddr(amx,params[8],&cstr);
    printstring(amx,cstr,params+10,(int)(params[0]/sizeof(cell))-1);
    g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';

    (params[8] > 0) ? bBackup = true : bBackup = false;
    pc->morph(params[2], params[3], params[4], params[5], params[6], params[7], g_cAmxPrintBuffer, bBackup);
    g_nAmxPrintPtr=0;
    return 0;
}

/*!
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

    pc->morph();
    return 0;
}

/*!
\brief Speech
\author Anthalir
\since 0.7
\param 1: sock
\param 2: item
\param 3: text
\return 0 or INVALID if not valid character
*/
NATIVE(_itm_speech)
{
	P_ITEM cur = pointers::findItemBySerial(params[2]);
	VALIDATEPIR(cur,INVALID);

	int tl;

	cell *cstr;
	amx_GetAddr(amx,params[3],&cstr);
	printstring(amx,cstr,params+4,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	g_nAmxPrintPtr=0;

	if (params[1] == -1)
	{
		itemtalk(cur,g_cAmxPrintBuffer);	//Numbersix: if socket = -1
		return 0;							// =>item speaks to all in range
	}
	tl=44+strlen( g_cAmxPrintBuffer )+1;

	UI08 talk[15] = { 0x1C, 0x00, };

	// Block size
	ShortToCharPtr(tl, talk +1);

	// Item ID
	LongToCharPtr(cur->getSerial32(), talk +3);

	// Model ID
	ShortToCharPtr(0x0101, talk +7);

	// Speech Type
	talk[9]= 6;

	// Speech color
	ShortToCharPtr(0x0481, talk +10);

	// Speech font
	ShortToCharPtr(0x0003, talk +12); //cur->fonttype;

	Xsend(params[1], talk, 14);
	unsigned char sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	Xsend(params[1], sysname, 30);
	Xsend(params[1], g_cAmxPrintBuffer, strlen( g_cAmxPrintBuffer ) + 1 );
//AoS/	Network->FlushBuffer(params[1]);

	return 0;
}

/*!
\brief given character equip specific item
\author Anthalir
\since 0.7
\param 1: character
\param 2: item
\return equip or INVALID if not valid character or item
*/
NATIVE(_ItemEquip)
{
    P_CHAR pc = pointers::findCharBySerial(params[1]);
    VALIDATEPCR(pc, INVALID);
    P_ITEM pi = pointers::findItemBySerial(params[2]);
    VALIDATEPIR(pi, INVALID);

    return pc->Equip(pi);
}


/*!
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


/*!
\brief Print a translated message to a given socket
\author Luxor
\since 0.82
\param 1: socket
\param 2: text
\return 0
*/
NATIVE(_ntprintf)
{
    cell *cstr;
	int i = params[1];
    if (i<0) return 0;
    NXWCLIENT s = getClientFromSocket(params[1]);
	if (s == NULL) return 0;
	amx_GetAddr(amx,params[2],&cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';
	s->sysmsg(TRANSLATE(g_cAmxPrintBuffer));
	g_nAmxPrintPtr=0;
	return 0;
}

/*!
\brief Launchs the web browser
\author Luxor
\since 0.82
\param 1: socket
\param 2: text
\return 0
*/
NATIVE(_weblaunch)
{
	cell *cstr;
	int s = params[1];
	if (s<0) return 0;
	amx_GetAddr(amx,params[2],&cstr);
	printstring(amx,cstr,params+3,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[g_nAmxPrintPtr] = '\0';

	weblaunch(s, g_cAmxPrintBuffer);
	g_nAmxPrintPtr=0;
	return 0;
}

/*!
\brief System Broadcast
\author Luxor
\since 0.82
\param 1: text
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

/*!
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

/*!
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

/*!
\brief Opens a skill makemenu
\author Luxor
\since 0.82
\param 1: the character
\param 2: the number of the make menu to be opened
\param 3: the skill that's using the makemenu
\return 1 if the makemenu was opened, -1 if not.
*/
NATIVE(_chr_skillMakeMenu)
{
        P_CHAR pc = pointers::findCharBySerial(params[1]);
        VALIDATEPCR(pc, INVALID);
        if (pc->getClient() == NULL) return INVALID;
        Skills::MakeMenu(pc->getClient()->toInt(),params[2],params[3]);
        return 1;
}

/*!
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
			pc->teleport();
			break;
		case 2:
			pc->hideBySpell();
			break;
		default: return 0;
	}
	return 0;
}

/*!
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

/*!
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
	setrandomname(pc, g_cAmxPrintBuffer);
	g_nAmxPrintPtr=0;
	return 1;
}

/*!
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

/*!
\brief Get local variable error
\author Sparhawk
\since 0.82
\return the error
*/
NATIVE(_chr_getLocalVarErr)
{
	return amxVS.getError();
}

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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


/*!
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

NATIVE( _var_get )
{
	//
	//	Params[1]	serial of object or character
	//	Params[2]	variable identifier
	//
	switch( amxVS.typeOfVariable( params[1], params[2] ) )
	{
		case AMXVARSRV_INTEGER		:
			{
				SI32 value;
				amxVS.selectVariable( params[1], params[2], value );
				return value;
			}
			break;
		case AMXVARSRV_STRING		:
			{
				//
				// Params[3]	- string reference
				//
				g_cAmxPrintBuffer[0] = '\0';
				cell *cptr;
				std::string str;
				if( amxVS.selectVariable( params[1], params[2], str ) )
				{
					strcpy( g_cAmxPrintBuffer, str.c_str() );
					amx_GetAddr( amx, params[3], &cptr );
					amx_SetString( cptr, g_cAmxPrintBuffer, g_nStringMode);
					return str.size();
				}
				return INVALID;
			}
			break;
		case AMXVARSRV_INTEGERVECTOR	:
			{
				//
				//	Params[3]	- index into vector
				//
				SI32 value;
				amxVS.selectVariable( params[1], params[2], params[3], value );
				return value;
			}
			break;
	}
	return INVALID;
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

/*!
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

/*!
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

/*!
\brief Get local varible error
\author Sparhawk
\since 0.82
\return the error
*/
NATIVE(_itm_getLocalVarErr)
{
  return amxVS.getError();
}

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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


/*!
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

/*!
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

/*!
\brief Check if NPC can move in given location
\author Sparhawk
\since 0.82
\param 1: x location
\param 2: y location
\return true or false
*/
NATIVE(_map_canMoveHere)
{
	return Map->CanMonsterMoveHere( static_cast<short int>(params[1]), static_cast<short int>(params[2]), illegal_z );
}

/*!
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

/*!
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
    MapStaticIterator msi(params[1], params[2]);

    staticrecord *stat;
    tile_st tile;
    if((stat = msi.Next()) != NULL) {
        msi.GetTile(&tile);

        char str[100];
  		cell *cptr;
	  	strcpy(str, (char*)(tile.name));

  		amx_GetAddr(amx,params[3],&cptr);
	  	amx_SetString(cptr,str, g_nStringMode);

  		return strlen(str);
    }
    return INVALID;
}

/*! 
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

   MapStaticIterator msi(params[1], params[2]); 
   staticrecord *stat; 
   int loopexit=0; 
   while ( ((stat = msi.Next())!=0) && (++loopexit < MAXLOOPS) ) 
   { 
      tile_st tile; 
      msi.GetTile(&tile); 

      if ((tile.height+stat->zoff)>params[3]) // a roof  must be higher than player's z ! 
      { 
         return true; 
      } 
   } 
   return false; 

}

/*!
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
    MapStaticIterator msi(params[1], params[2]);

    staticrecord *stat;
    while((stat = msi.Next()) != NULL)
   {
        if (stat->zoff == params[3])
         return stat->itemid;
      
    }
    return INVALID;
}

/*!
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
    map_st map = Map->SeekMap0(params[1], params[2]);
    return map.id;
}

////////////////////////////////////////////////////////////
/////////////////////// GUILD //////////////////////////////
////////////////////////////////////////////////////////////


/*!
\brief Make a guildstone ( and a guild :D )
\author Endymion
\since 0.82
\param 1: the player
\param 2: the deed
\return the stone
*/
NATIVE(_guild_placeStone)
{
	P_CHAR pChar = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pChar, INVALID);
	P_ITEM pDeed = pointers::findItemBySerial(params[2]);
	VALIDATEPIR(pDeed, INVALID);
	P_ITEM pStone = PlaceGuildStoneDeed( pChar, pDeed );
	if(ISVALIDPI( pStone ))
		return pStone->getSerial32();
	else
		return INVALID;
}

/*!
\brief Add a member to a guild
\author Endymion
\since 0.82
\param 1: the guild
\param 2: the new member
\return true or false
*/
NATIVE(_guild_addMember)
{
	LOGICAL success = false;

	if( Guildz.find( params[1] ) )
	{
		cGuild *pGuild = Guildz.fetch();
		P_CHAR pc = pointers::findCharBySerial( params[2] );
		if( ISVALIDPC( pc ) )
			if( pGuild->addMember( pc ) )
				if( Guildz.update( pGuild ) )
					success = true;
	}
	return success;
}

/*!
\brief A member resign from his guild
\author Endymion
\since 0.82
\param 1: the guild
\param 2: the member
\todo Handle situation if member is guildmaster & if guild is empty after resign
*/
NATIVE(_guild_resignMember)
{
	LOGICAL success = false;

	if( Guildz.find( params[1] ) )
	{
		cGuild *pGuild = Guildz.fetch();
		P_CHAR pc= pointers::findCharBySerial( params[2] );
		if( ISVALIDPC( pc ) )
			if( pGuild->resignMember( pc ) )
				if( Guildz.update( pGuild ) )
					success = true;
	}
	return success;
}

/*!
\brief return the number of members in a guild
\author Sparhawk
\since 0.82
\param 1: the guild
\return number of members or invalid when guild does not exist
*/
NATIVE(_guild_countMember)
{
	if( Guildz.find( params[1] ) )
		return Guildz.fetch()->members.size();
	return INVALID;
}

/*!
\brief Get the member by given index
\author Endymion
\since 0.82
\param 1: the guild
\param 2: the member index
\return the member or INVALID
*/
NATIVE(_guild_getMemberByIdx)
{
	if( !Guildz.find( params[1] ) )
		return INVALID;
	cGuild *pGuild = Guildz.fetch();
	if( (params[2]<0) || ((UI32)params[2] >= pGuild->members.size() ) )
		return INVALID;

	GUILDMEMBERMAPITER iter( pGuild->members.begin() );
	for( int i = params[2]; i > 0; --i, ++iter );
		return iter->first;
}

/*!
\brief Add a recuit to a guild
\author Endymion
\since 0.82
\param 1: the guild
\param 2: the new recruit
\return true or false
*/
NATIVE(_guild_addRecruit)
{
	if( !Guildz.find( params[1] ) )
		return false;

	P_CHAR pc= pointers::findCharBySerial( params[2] );
	VALIDATEPCR(pc,false);
	cGuild *pGuild = Guildz.fetch();
	if( pGuild->addNewRecruit(pc) )
		return Guildz.update( pGuild );
	return false;
}

/*!
\brief Given recruit are refuse
\author Endymion
\since 0.82
\param 1: the guild
\param 2: the recruit
*/
NATIVE(_guild_refuseRecruit)
{
	if( !Guildz.find( params[1] ) )
		return false;
	P_CHAR pc= pointers::findCharBySerial( params[2] );
	VALIDATEPCR(pc,false);
	cGuild *pGuild = Guildz.fetch();
	if( pGuild->refuseRecruit(pc) )
		return Guildz.update( pGuild );
	return false;
}

/*!
\brief return the number of recruits in a guild
\author Sparhawk
\since 0.82
\param 1: the guild
\return number of recruits or invalid
*/
NATIVE(_guild_countRecruit)
{
	if( !Guildz.find( params[1] ) )
		return INVALID;
	else
		return Guildz.fetch()->recruits.size();
}

/*!
\brief Get the recruit by given index
\author Endymion
\since 0.82
\param 1: the guild
\param 2: the recruit index
\return the recruit or INVALID
*/
NATIVE(_guild_getRecruitByIdx)
{
	if( !Guildz.find( params[1] ) )
		return INVALID;
	cGuild *pGuild = Guildz.fetch();
	if( (params[2]<0) || (UI32)params[2] >= pGuild->recruits.size() )
		return INVALID;
	SERIALVECTORITER iter( pGuild->recruits.begin() );
	for( int i = params[2]; i > 0; --i, ++iter );
	return *iter;
}

/*!
\brief Get the Guild master for a specified guild
\author Sparhawk
\since 0.82
\param 1: guild id
\return the serial of the guildmaster or invalid
*/
NATIVE(_guild_getMaster)
{
	if( !Guildz.find( params[1] ) )
		return INVALID;
	return Guildz.fetch()->getGuildMaster();
}

/*!
\brief check if guild exists
\author Sparhawk
\since 0.82
\param 1: the guild
\return true or false
*/
NATIVE(_guild_exists)
{
	return Guildz.find( params[1] );
}

/*!
\brief Add a new timer
\author Endymion
\since 0.82
\param 1: the object ( item or char )
\param 2: secs
\param 3: check event
\param 4: flags
\param 5: n
\param 6: more1
\param 7: more2
\return none
*/
NATIVE(_timer_add)
{	
	cTimer timer( params[1], params[2], params[4], params[5], params[6], params[7]  );
	timer.amxevent=new AmxEvent( params[3] );
	timers::addTimer( params[1], timer );
	return 0;
}

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
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

/*!
\brief Call an AMX function without parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction )
{
	if (params[1] < 0) return INVALID;
	return g_prgOverride->CallFn(params[1]);
}

/*!
\brief Call an AMX function with 1 parameter
\author Luxor
\since 0.82
*/
NATIVE( _callFunction1P )
{
	if (params[1] < 0) return INVALID;
	return g_prgOverride->CallFn(params[1], params[2]);
}

/*!
\brief Call an AMX function with 2 parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction2P )
{
	if (params[1] < 0) return INVALID;
	return g_prgOverride->CallFn(params[1], params[2], params[3]);
}

/*!
\brief Call an AMX function with 3 parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction3P )
{
	if (params[1] < 0) return INVALID;
	return g_prgOverride->CallFn(params[1], params[2], params[3], params[4]);
}

/*!
\brief Call an AMX function with 4 parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction4P )
{
	if (params[1] < 0) return INVALID;
	return g_prgOverride->CallFn(params[1], params[2], params[3], params[4], params[5]);
}

/*!
\brief Call an AMX function with 5 parameters
\author Luxor
\since 0.82
*/
NATIVE( _callFunction5P )
{
	if (params[1] < 0) return INVALID;
	return g_prgOverride->CallFn(params[1], params[2], params[3], params[4], params[5], params[6]);
}

/*!
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

/*!
\brief Check if a character is frozen
\author Luxor
\since 0.82
*/
NATIVE( _chr_isFrozen )
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);
	VALIDATEPCR(pc, 0);
	return pc->IsFrozen();
}

/*!
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

/*!
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

/*!
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
// New Menu API -- still highly experimental
//

/*!
\brief Create a new menu
\author Endymion
\since 0.82
\param 1 the menu id
\param 2 x
\param 3 y
\param 2 is moveable
\param 2 is closeable
\param 3 is disposeable
\return the menu serial
*/
NATIVE ( _menu_create )
{
	return ( menus.createMenu( params[1], params[2], params[3], (params[4]?true:false), (params[5]?true:false), (params[6]?true:false) ) )? 1 : 0; 
}

/*!
\brief Delete a menu
\author Endymion
\since 0.82
\param 1 the menu serial
\return true if is deleted or false if error
*/
NATIVE ( _menu_delete )
{
	return ( menus.deleteMenu( params[1] ) )? 1 : 0;
}

/*!
\brief Delete a menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 the character
\return true if is show or false if error
*/
NATIVE ( _menu_show )
{
	P_MENU menu = menus.selectMenu( params[1] );
	if ( menu != NULL ) {
		P_CHAR pc = pointers::findCharBySerial( params[2] );
		VALIDATEPCR( pc, 0 );
		return ( menus.showMenu( params[1], pc ) ? 1 : 0 );
	}
	return 0;
}

/*!
\brief Add a new button at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 up
\param 5 down
\param 6 return code
\param 7 page ( default 1 )
\return false if error, true else
*/
NATIVE ( _menu_addButton )
{
	P_MENU menu = menus.selectMenu( params[1] );
	if ( menu != NULL )
	{
			menu->addButton( params[2], params[3], params[4], params[5], params[6], params[7] );
			return 1;
	}
	return 0;
}

/*!
\brief Add a new button at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 page number
\return false if error, true else
*/
NATIVE ( _menu_addPage )
{
	P_MENU menu = menus.selectMenu( params[1] );
	if ( menu != NULL )
	{
		menu->addPage( params[2] );
		return 1;
	}
	return 0;
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
NATIVE (  _menu_addResizeGump )
{
	P_MENU menu = menus.selectMenu( params[1] );
	if ( menu != NULL )
	{
		menu->addResizeGump( params[2], params[3], params[4], params[5], params[6] );
		return 1;
	}
	return 0;
}

/*!
\brief Add a page button at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 up gump
\param 5 down gump
\param 6 page
\return false if error, true else
*/
NATIVE ( _menu_addPageButton )
{
	P_MENU menu = menus.selectMenu( params[1] );
	if ( menu != NULL )
	{
		menu->addPageButton( params[2], params[3], params[4], params[5], params[6] );
		return 1;
	}
	return 0;
}

/*!
\brief Add text at given menu
\author Endymion
\since 0.82
\param 1 the menu serial
\param 2 x
\param 3 y
\param 4 text
\param 5 color
\return false if error, true else
*/
NATIVE ( _menu_addText )
{
	P_MENU menu = menus.selectMenu( params[1] );
	if ( menu != NULL )
	{
		cell *cstr;
		amx_GetAddr(amx,params[4],&cstr);
		wstring s;
		amx_GetStringUnicode( &s, cstr );
		menu->addText( params[2], params[3], s, params[5] );
		return 1;
	}
	return 0;
}



//
//
//















//API standard syntax :
// for generic system functions : functionName
// for other functions xxxx_functionName where
// xxxx is :

//		chr - for charachter related functions
//		itm - for item related functions
//		mnu - for menu access (0.20s)
//		magic - for magic access (0.50s)
//		send - for packet send functions (0.50s)
//		direct - for direct access functions (0.53b)
//		tempfx - for temp effects access functions (0.53b)
//		rgn - for region access functions (0.54b)
//		trig - for triggers functions (0.54b)
//		set - for sets (0.55x)
//		cal - for calendar access (Sparhawk 2001-09-15}
//		map - for map access (Sparhawk 082.)
//		guild - for guild acces ( Endymion 0.82 )
//		log - for logging messages ( Sparhawk 0.82 )

AMX_NATIVE_INFO nxw_API[] = {
// Standard commands :
 { "callFunction", _callFunction },
 { "callFunction1P", _callFunction1P },
 { "callFunction2P", _callFunction2P },
 { "callFunction3P", _callFunction3P },
 { "callFunction4P", _callFunction4P },
 { "callFunction5P", _callFunction5P },
 { "nprintf", _nprintf },
 { "ncprintf", _ncprintf },
 { "ntprintf", _ntprintf },
 { "sprintf", _sprintf },
 { "bypass", _bypass },
 { "setWindowTitle", _setWindowTitle },
 { "getFrameStatus", _getFrameStatus },
 { "addTimer", addTimer },
 { "getNXWVersion", _getNXWVersion },
 { "getNXWVersionType", _getNXWVersionType },
 { "getNXWPlatform", _getNXWPlatform },
 { "getCharTarget", _getCharTarget },
 { "getItemTarget", _getItemTarget },
 { "getCurrentSocket", _getCurrentSocket },
 { "getStringMode", _getStringMode },
 { "setStringMode", _setStringMode },
 { "getSocketFromChar", _getSocketFromChar },
 { "getCharFromSocket", _getCharFromSocket },
 { "getCharFromSerial", _getCharFromSer },
 { "getItemFromSerial", _getItemFromSerial },
 { "getTarget", _getTarget },
 { "getCurrentTime", _getCurrentTime },
 { "getSystemTime", _getSystemTime },
 { "getSocketCount", _getSocketCount },
 { "cfgServerOption", _cfgServerOption },
 { "weblaunch", _weblaunch },
 { "broadcast", _sysbroadcast },
// Direct-access functions :
 { "direct_target", _nativeTarget },
 { "direct_castSpell", _direct_castSpell },
 { "direct_setSpellType", _direct_setSpellType },
// Charachter functions :
 { "chr_addNPC",  _chr_addNPC},
 { "chr_canBroadcast", _canBroadcast },
 { "chr_canSeeSerials", _canSeeSerials},
 { "chr_canSnoop", _canSnoop},
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
 { "chr_spawnIteminBank", _chr_spawnIteminBank },
 { "chr_unhide", _unhide},
 { "chr_calcAtt", _chr_calcAtt },
 { "chr_calcDef", _chr_calcDef },
 { "chr_fish", _chr_fish },
 { "chr_isOnline", _chr_isOnline },
 { "chr_isFrozen", _chr_isFrozen },
 { "chr_doCombatAction", _chr_doCombatAction },
 { "chr_stable", _chr_stable },
 { "chr_unStable", _chr_unStable },
//
// Sparhawk: 	Guild related functions to be ported to new guild system
//		To maintain backward compatability these will be implemented as stock functions that map to the new guild functions
//
 { "chr_guildCompare", _chr_guildCompare },
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
// Sparhawk:	New guild functions
//
 { "chr_getGuild",		_chr_getGuild },
 { "chr_setGuild",		_chr_setGuild },
//
// End of new character guild functions
//
 { "chr_attackStuff", _chr_attackStuff },
 { "chr_helpStuff", _chr_helpStuff },
 { "chr_action", _chr_action },
 { "chr_checkSkill", _chr_checkSkill },
 { "chr_unmountHorse", _chr_unmountHorse },
 { "chr_mountHorse", _chr_mountHorse },
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
 { "chr_setCreationDay", _chr_setCreationDay },
 { "chr_getCreationDay", _chr_getCreationDay },
 { "chr_morph", _chr_morph },
 { "chr_unmorph", _chr_unmorph },
 { "chr_skillMakeMenu", _chr_skillMakeMenu },
 { "chr_possess", _chr_possess },
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
 { "itm_getCharBackPack", _itm_getCharBackPack },
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
 { "itm_spawnBank", _itm_spawnBank },
 { "itm_checkDecay", _itm_checkDecay },
 { "itm_remove", _itm_remove },
 { "itm_spawnBackpack", _createiteminbackpack },
 { "itm_getEventHandler", _itm_getEventHandler },
 { "itm_setEventHandler", _itm_setEventHandler  },
 { "itm_delEventHandler", _itm_delEventHandler  },
 { "itm_sound", _itm_sound },
 { "itm_spawnItem", _itm_spawnItem},
 { "itm_spawnNecroItem", _itm_spawnNecroItem},
 { "itm_refresh", _itm_refresh},
 { "itm_speech", _itm_speech},
 { "itm_equip", _ItemEquip},
 { "itm_BounceToPack", _ItemBounceToPack},
 { "itm_getCombatSkill", _itm_getCombatSkill},
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
//
// New local property interface
//
 { "var_get", _var_get },
//
// Gui functions
//
 { "gui_create", _gui_create },
 { "gui_delete", _gui_delete },
 { "gui_show", _gui_show },
 { "gui_addButton", _gui_addButton },
 { "gui_addGump", _gui_addGump },
 { "gui_addPage", _gui_addPage },
 { "gui_addResizeGump", _gui_addResizeGump },
 { "gui_addPageButton", _gui_addPageButton },
 { "gui_addRadioButton", _gui_addRadioButton },
 { "gui_addText", _gui_addText },
 { "gui_addCroppedText", _gui_addCroppedText },
 { "gui_addTilePic", _gui_addTilePic },
 { "gui_addTiledGump", _gui_addTiledGump },
 { "gui_addHtmlGump", _gui_addHtmlGump },
 { "gui_addInputField", _gui_addInputField },
 { "gui_getInputField", _gui_getInputField },
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
//
// Receive functions
//
 { "rcve_statsRequest", _rcve_statsRequest },
 { "rcve_skillsRequest", _rcve_skillsRequest },
// Temp Effects functions :
 { "tempfx_activate", _tempfx_activate },
// Old Menu functions :
 { "mnu_prepare", _menu_Prepare },
 { "mnu_setStyle", _menu_SetStyle },
 { "mnu_setTitle", _menu_SetTitle },
 { "mnu_setColor", _menu_SetColor },
 { "mnu_addItem", _menu_AddItem },
 { "mnu_show", _menu_Show },
 { "mnu_setCallback", _menu_SetCallback },
// New Menu functions :

 { "menu_create", _menu_create },
 { "menu_delete", _menu_delete },
 { "menu_show", _menu_show },
 { "menu_addButton", _menu_addButton },
 { "menu_addPage", _menu_addPage },
 { "menu_addResizeGump", _menu_addResizeGump },
 { "menu_addPageButton", _menu_addPageButton },
 { "menu_addText", _menu_addText },

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
// Trigger functions :
 { "trig_getTItem", _getTriggeredItem },
 { "trig_getTrigType", _getTriggeredItemTrigType },
// Set functions :
 { "set_create", _set_create },
 { "set_delete", _set_delete },
 
 { "set_rewind", _set_rewind },
 { "set_next", _set_next },
 { "set_end", _set_end },
 { "set_size", _set_size },

 { "set_get", _set_get },
 { "set_add", _set_add },
 
 { "set_addOwnedNpcs", _set_addOwnedNpcs },
 { "set_addCharsNearXYZ", _set_addCharsNearXYZ },
 { "set_addPartyFriend", _set_addPartyFriend },
 { "set_addItemsInContainer", _set_addItemsInContainer },
 { "set_addItemWeared", _set_addItemWeared },
 { "set_addItemsAtXY", _set_addItemsAtXY },
 { "set_addItemsNearXYZ", _set_addItemsNearXYZ },
 { "set_addAllOnlinePlayers", _set_addAllOnlinePlayers },
 { "set_addOnlinePlayersNearChar", _set_addOnlinePlayersNearChar },
 { "set_addOnlinePlayersNearItem", _set_addOnlinePlayersNearItem },
 { "set_addOnlinePlayersNearXY", _set_addOnlinePlayersNearXY },
// calendar properties - [Sparhawk] 2001-09-15
 { "cal_getProperty"		,	_getCalProperty			},
// Map functions - for experimental small npc ai Sparhawk
 { "map_canMoveHere", _map_canMoveHere },
 { "map_distance", _map_distance },
 { "map_getTileName", _map_getTileName},
 { "map_isUnderStatic", _map_isUnderStatic}, //Keldan 2003/02/09
 { "map_getTileID", _map_getTileID}, //Keldan, posted 2003/01/27 - added 2003/03/01
 { "map_getFloorTileID", _map_getFloorTileID}, // Keldan, posted 2003/01/27 - added 2003/03/01
// Guild function and properties - Endymion
 { "guild_setProperty", _setGuildProperty },
 { "guild_getProperty", _getGuildProperty },
 { "guild_placeStone", _guild_placeStone },
 { "guild_addMember", _guild_addMember },
 { "guild_resignMember", _guild_resignMember },
 { "guild_countMember", _guild_countMember },
 { "guild_getMbrByIdx", _guild_getMemberByIdx },
/*
 { "guild_getFstMember", _guild_getFstMember },
 { "guild_getNxtMember", _guild_getNxtMember },
*/
 { "guild_addRecruit", _guild_addRecruit },
 { "guild_refuseRecruit", _guild_refuseRecruit },
 { "guild_getRecByIdx", _guild_getRecruitByIdx },
 { "guild_countRecruit", _guild_countRecruit },
 { "guild_getMaster", _guild_getMaster },
 { "guild_exists",	_guild_exists },
// Timer function - Endymion
 { "timer_add", _timer_add },
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
// Terminator :
 { NULL, NULL }
};
