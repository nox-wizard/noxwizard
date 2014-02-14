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
\brief Debug related functions
*/
#ifndef DEBUG_H
#define DEBUG_H

#ifdef _MSC_VER
#pragma warning ( push , 3 )
#endif

#include <cstdio>
#include <string>
#include <stdarg.h>
#include <time.h>

#ifdef WIN32
 #include <windows.h>
#endif

#ifdef  _MSC_VER	// when compiling with borland or mingw, can't use asm
 bool canBreakpoint();
 bool newIsDebuggerPresent();
 //defined as macro so bkp happens locally to the prob :]
 #define BREAKPOINT { if (canBreakpoint()) { __asm { int 3 } } }
#else
 #define BREAKPOINT ;
#endif


//---------------------------------------------------------------------------
//     Prototypes of functions
//---------------------------------------------------------------------------
 
extern "C" void __CDECL ConOut(char *txt, ...);		//! Unformatted output


#define Xsend(sock,buff,length) \
	if( sock==INVALID || sock >=now) \
		ConOut("INVALID socket %d was used in Xsend in %s:%d \n", sock, __FILE__, __LINE__ ); \
	else	\
		Network->xSend(sock, buff, length) \

#define CRASH_IF_INVALID_SOCK(sock) if (sock<0 || sock>=MAXCLIENT) chars[-1000000].x=0

extern bool g_bExceptionCaught;



#ifdef _MSC_VER
#pragma warning ( pop )
#endif


//xan : for FUTURE parameter's validation
#ifdef _DEBUG
#define INVALIDATE assert(0);
#else 
#define INVALIDATE { nInvalidate = __LINE__; throw; }
#endif

#ifdef WIN32

#define DBG_INRANGE(A,B,C) if ((A<B)||(A>=C)) INVALIDATE;
#define DBG_WPTR(A,S) if ((A==NULL)||(IsBadWritePtr(A,S))) INVALIDATE;
#define DBG_STR(S) if ((A==NULL)||(IsBadStringPtr(A,0xFFFFFF))) INVALIDATE;

#else

#define DBG_INRANGE(A,B,C) if ((A<B)||(A>=C)) INVALIDATE;
#define DBG_WPTR(A,S) if ((A==NULL)) INVALIDATE;
#define DBG_STR(S) if ((A==NULL)) INVALIDATE;

#endif






#endif
