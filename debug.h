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

#ifdef WIN32
#pragma warning ( push , 3 )
#endif

#include <cstdio>
#include <string>
#include <stdarg.h>
#include <time.h>

#ifdef WIN32
 #include <windows.h>
#endif

namespace ServerScp {
extern int g_nExceptionHandling;
}

#ifdef  _MSC_VER	// when compiling with borland or mingw, can't use asm
 bool canBreakpoint(void);
 bool newIsDebuggerPresent(void);
 //defined as macro so bkp happens locally to the prob :]
 #define BREAKPOINT { if (canBreakpoint()) { __asm { int 3 } } }
#else
 #define BREAKPOINT ;
#endif


//---------------------------------------------------------------------------
//     Prototypes of functions
//---------------------------------------------------------------------------

#define _ ,

//#define Xsend(sock,buff,length) if (sock<0 || sock>=MAXCLIENT) LogCritical("Socket/Index confusion @ Xsend %i\n" _ sock);  else Network->xSend(sock, buff, length, 0)

// Usefull to leave some FIXME comments in source (will show in the warnings window (Original from UOX3) :)
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

#define NOTE( x )  message( x )
#define FILE_LINE  message( __FILE__LINE__ )

#define TODO( x )  message( __FILE__LINE__"\n""+------------------------------------------------\n""|  TODO :   " #x "\n""+-------------------------------------------------\n" )
#define FIXME( x ) message( __FILE__LINE__"\n""+------------------------------------------------\n""|  FIXME :  " #x "\n""+-------------------------------------------------\n" )
#define todo( x )  message( __FILE__LINE__" TODO :   " #x "\n" ) 
#define fixme( x ) message( __FILE__LINE__" FIXME:   " #x "\n" ) 
#define note( x )  message( __FILE__LINE__" NOTE :   " #x "\n" ) 

//Endy needed for log Xsend with INVALID socket
extern "C" void __CDECL ConOut(char *txt, ...);		//! Unformatted output


#define Xsend(sock,buff,length) \
	if( sock==INVALID || sock >=now) \
		ConOut("INVALID socket %d was used in Xsend in %s:%d \n", sock, __FILE__, __LINE__ ); \
	else	\
		Network->xSend(sock, buff, length, 0) \

#define CRASH_IF_INVALID_SOCK(sock) if (sock<0 || sock>=MAXCLIENT) chars[-1000000].x=0

extern bool g_bExceptionCaught;



#ifdef WIN32
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
