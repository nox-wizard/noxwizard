  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "debug.h"
#include <signal.h>
#include "version.h"
#include "nxw_utils.h"
#include "inlines.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef _WINDOWS
#include "nxwgui.h"
#endif

//for Linux exception handling
//we use it to emulate asynch exceptions in a synch system :)
bool g_bExceptionCaught = false;

// Akron
// newIsDebuggerPresent and canBreakpoint are used only when compiling with MSVC++
// so them need to be compiled only if _MSC_VER is defined

#ifdef _MSC_VER

/*!
\brief Returns if the process is running in the context of a win32 debugger
\return true if the process is running in the context of a win32 debugger obv ;)
\author Lord Binary, Xanathar
\note Under Windows 95 returns <b>always</b> false
*/
bool newIsDebuggerPresent()
{
	static bool bChecked = false;
	static bool bPresent = false;

	if (bChecked) return bPresent;
	bChecked = true;

	BOOL  (WINAPI *lpfIsDebuggerPresent)   (void  ) = NULL;
	HINSTANCE hInstLib = LoadLibraryA ("Kernel32.DLL" );
	if (hInstLib==NULL) FreeLibrary(hInstLib);
	else lpfIsDebuggerPresent = (BOOL(WINAPI*)(void))  GetProcAddress(hInstLib, "IsDebuggerPresent");

	if (lpfIsDebuggerPresent!=NULL) bPresent = lpfIsDebuggerPresent()!=FALSE;

	if (hInstLib!=NULL) FreeLibrary(hInstLib);

	return bPresent;

}



///////////////////////////////////////////////////////////////////
// Function name     : canBreakpoint
// Description       : check if it can do hard breakpoint for debugging purposes
// Return type       : bool
// Author            : Xanathar
// Changes           : none yet
bool canBreakpoint()
{
	static bool bChecked = false;
	static bool bBreak = false;
	if (bChecked) return bBreak;
	bChecked = true;
	if (newIsDebuggerPresent()) {
		char *p = getHKLMRegistryString("SOFTWARE\\NoXWizard", "ThrowBreakpoints");
		if (p==NULL) return false;
		int i = atoi(p);
		safedelete(p);
		bBreak = (i!=0);
		return bBreak;
	}
	return false; //for no debug and or no win32
}

#endif

void signalhandler (int x)
{ g_bExceptionCaught = true; }

void interrupthandler (int x)
{ keeprun = false; }


void initSignalHandlers()
{
	g_bExceptionCaught = false;
    start_signal_thread();
/*
#ifdef __unix__
	signal(SIGSEGV, signalhandler);
	signal(SIGILL, signalhandler);
	signal(SIGFPE, signalhandler);
	signal(SIGABRT, signalhandler);
	signal(SIGINT, interrupthandler);
	signal(SIGTERM, interrupthandler);
#endif
*/
}

#define BOUNDCHECKING
//Xan : when boundchecking, we often must use release rtl in debug builds
#ifdef BOUNDCHECKING
extern "C" {
int _CrtDbgReport( int reportType, const char *filename, int linenumber, const char *moduleName, const char *format, ... )
{
	return 0;
}
}
#endif







