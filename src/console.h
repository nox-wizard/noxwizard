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
\brief Console Handling Functions
*/
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef WIN32
#define __CDECL __cdecl
#else
#define __CDECL
#endif


#ifdef __cplusplus

extern "C" void __CDECL ConOut(char *txt, ...);		//!< Unformatted output
extern "C" void __CDECL ErrOut(char *txt, ...);		//!< Formatted as error
extern "C" void __CDECL WarnOut(char *txt, ...);	//!< Formatted as warning
extern "C" void __CDECL InfoOut(char *txt, ...);	//!< Formatted as info
extern "C" void __CDECL PanicOut(char *txt, ...);	//!< Formatted as panic
extern "C" void __CDECL DmpOut(char *txt, ...);		//!< Formatted as internal-development debug info (should not be in releases)
extern "C" void __CDECL SDbgOut(char *txt, ...);	//!< Formatted as amx debug output
extern "C" void __CDECL SDbgIn(char *s, int n);		//!< Formatted as amx debug info
extern "C" void __CDECL STraceOut(char *txt, ...);
void clearscreen( void );
void setWinTitle(char *str, ...);
void constart( void );

#define ALERT_TYPE_OK 0
#define ALERT_TYPE_YESNO 1

#define ALERT_MESSAGE 0
#define ALERT_INFO 1
#define ALERT_WARNING 2
#define ALERT_ERROR 3
#define ALERT_FATAL 4

bool alertBox(char *msg, char *title, int gravity = 0, int type = 0);

#else
void __CDECL ConOut(char *txt, ...);
void __CDECL STraceOut(char *txt, ...);
void __CDECL SDbgOut(char *txt, ...);
void __CDECL SDbgIn(char *s, int n);
#endif

void initConsole();
char* getNoXDate();

#endif //__CONSOLE_H__
