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
#include "globals.h"

#include <stdio.h>
#include <stdarg.h>
#include "console.h"
#ifdef _WINDOWS
#include "nxwgui.h"
#endif

static FILE *s_fileStdOut = NULL;
static char s_strSrvcMsg[8192];


void clearscreen( void )
{
	ConOut("__________________________________________________________________________\n\n");
}

void setWinTitle(char *str, ...)
{
	if (ServerScp::g_nDeamonMode!=0) return;

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	va_list argptr;

	va_start( argptr, str );
	vsnprintf( temp, sizeof(temp)-1, str, argptr );
	va_end( argptr );
	
	#ifdef __unix__
	    ConOut("\033]0;%s\007", temp); // xterm code
	#endif
	#ifdef WIN32
    	#ifdef _WINDOWS
    		SetWindowText(g_HWnd, temp);
    	#endif
    	#ifdef _CONSOLE
    		SetConsoleTitle(temp);
    	#endif
	#endif
}

void constart( void )
{
	if (ServerScp::g_nLoadDebugger!=0) {
		setWinTitle("%s %s - [debug mode] [run]", PRODUCT, VERNUMB);
	} else {
		setWinTitle("%s %s", PRODUCT, VERNUMB);
	}
	#ifndef __unix__
	#ifndef _WINDOWS
	if (ServerScp::g_nDeamonMode==0) {
		HANDLE Buff = GetStdHandle(STD_OUTPUT_HANDLE);
		COORD coord; coord.X = 80; coord.Y = (short)ServerScp::g_nLineBuffer;
		WORD arr[80];
		DWORD  w;

		SetConsoleScreenBufferSize(Buff, coord);

		unsigned short color;

		color=FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_GREEN;


		
		SetConsoleTextAttribute(Buff,color);

		coord.X = coord.Y = 0;
		for (int i = 0; i<80; i++)
			arr[i] = color;
		
		for (coord.Y = 0; coord.Y<1024; coord.Y++) WriteConsoleOutputAttribute(Buff, (CONST WORD *)arr,80,coord,(LPDWORD )&w);       // actual number written

	}
	#endif
	#endif
}


void initConsole()
{
	if ((ServerScp::g_nRedirectOutput)||(ServerScp::g_nDeamonMode)) {
		if(s_fileStdOut==NULL) s_fileStdOut = fopen(ServerScp::g_szOutput,"wt");
		if(s_fileStdOut==NULL) s_fileStdOut = fopen("nxwout","wt");
		if(s_fileStdOut==NULL) exit(1);
	} else s_fileStdOut = stdout;
}


extern "C" void ConOut(char *txt, ...)
{
	va_list argptr;

	va_start( argptr, txt );
	vsnprintf( s_strSrvcMsg, sizeof(s_strSrvcMsg)-1, txt, argptr );
	va_end( argptr );

#ifndef _WINDOWS
	fprintf(s_fileStdOut, s_strSrvcMsg);
	fflush(s_fileStdOut);
#else
	xwprintf("%s", s_strSrvcMsg);
#endif
}

extern void setWinTitle(char *str, ...);

extern "C" void SDbgIn(char *s, int n) 
{
	if (ServerScp::g_nDeamonMode!=0) {
		ConOut("ALERT : INPUT REQUESTED IN SERVICE");
		sprintf(s, "--Unknown Input--");
	}
	
	setWinTitle("%s %s - [debug mode] [break]", PRODUCT, VERNUMB);

#ifndef _WINDOWS

	if (ServerScp::g_nDeamonMode==0) {
		#ifdef _CONSOLE
				HANDLE Buff = GetStdHandle(STD_OUTPUT_HANDLE);
				unsigned short color;
				color=FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY;
				SetConsoleTextAttribute(Buff,color);
		#endif //Win-CONSOLE
		fgets(s,n,stdin);
		#ifdef _CONSOLE
				color=FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
				SetConsoleTextAttribute(Buff,color);
		#endif //Win-CONSOLE
	}
#else //Win-GUI
	xwgets(s, n);
#endif //Win-GUI
	
	setWinTitle("%s %s - [debug mode] [run]", PRODUCT, VERNUMB);
}


extern "C" void SDbgOut(char *txt, ...)
{
	va_list argptr;

	va_start( argptr, txt );
	vsnprintf( s_strSrvcMsg, sizeof(s_strSrvcMsg)-1, txt, argptr );
	va_end( argptr );

#ifndef _WINDOWS

	#ifdef _CONSOLE
		if (ServerScp::g_nDeamonMode==0) 
		{
			HANDLE Buff = GetStdHandle(STD_OUTPUT_HANDLE);
			unsigned short color;
			color=FOREGROUND_GREEN|FOREGROUND_RED;
			SetConsoleTextAttribute(Buff,color);
		}
	#endif

	fprintf(s_fileStdOut, "%s", s_strSrvcMsg);
	fflush(s_fileStdOut);

	#ifdef _CONSOLE
		if (ServerScp::g_nDeamonMode==0) {
			HANDLE Buff = GetStdHandle(STD_OUTPUT_HANDLE);
			unsigned short color;
			color=FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
			SetConsoleTextAttribute(Buff,color);
		}
	#endif


#else
	xwprintf("\x80%s", s_strSrvcMsg);
#endif

}

extern "C" void STraceOut(char *txt, ...)
{
	va_list argptr;

	va_start( argptr, txt );
	vsnprintf( s_strSrvcMsg, sizeof(s_strSrvcMsg)-1, txt, argptr );
	va_end( argptr );

#ifndef _WINDOWS

#ifdef _CONSOLE
	if (ServerScp::g_nDeamonMode==0) {
		HANDLE Buff = GetStdHandle(STD_OUTPUT_HANDLE);
		unsigned short color;
		color=FOREGROUND_BLUE|FOREGROUND_INTENSITY;
		SetConsoleTextAttribute(Buff,color);
	}
#endif

	fprintf(s_fileStdOut, "%s", s_strSrvcMsg);
	fflush(s_fileStdOut);

#ifdef _CONSOLE
	if (ServerScp::g_nDeamonMode==0) {
		HANDLE Buff = GetStdHandle(STD_OUTPUT_HANDLE);
		unsigned short color;
		color=FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
		SetConsoleTextAttribute(Buff,color);
	}
#endif


#else
	xwprintf("\x81%s", s_strSrvcMsg);
#endif

}





static char s_szErrMsg[2048];
static char s_szNoXDate[128];

char* getNoXDate()
{
    time_t TIME;
    tm* T;
    time(&TIME);
    T = localtime(&TIME);

//	sprintf(s_szNoXDate, "%02d%02d%02d%02d%02d%02d",  T->tm_year%100,T->tm_mon+1, T->tm_mday,
//		T->tm_hour, T->tm_min, T->tm_sec);

	sprintf(s_szNoXDate, "[%02d/%02d/%04d %02d:%02d:%02d]", T->tm_mday, T->tm_mon+1, T->tm_year+1900, 
													  T->tm_hour, T->tm_min, T->tm_sec);

	return s_szNoXDate;
}



/*******************************************************
 ** NEW GEN Console Functions                         **
 *******************************************************/
#define USEANSICODES
extern "C" void AnsiOut(char *txt)
{
    #ifdef __unix__
    #ifdef USEANSICODES
        ConOut(txt);
    #endif
    #endif
}


extern "C" void ErrOut(char *txt, ...)
{
	va_list argptr;

	va_start( argptr, txt );
	vsnprintf( s_szErrMsg, sizeof(s_szErrMsg)-1, txt, argptr );
	va_end( argptr );

    AnsiOut("\x1B[1;31m");
	ConOut("E %s - %s", getNoXDate(), s_szErrMsg);
    AnsiOut("\x1B[0m");
}


extern "C" void WarnOut(char *txt, ...)
{
	va_list argptr;

	va_start( argptr, txt );
	vsnprintf( s_szErrMsg, sizeof(s_szErrMsg)-1, txt, argptr );
	va_end( argptr );

    AnsiOut("\x1B[1;33m");
	ConOut("W %s - %s", getNoXDate(), s_szErrMsg);
    AnsiOut("\x1B[0m");
}


extern "C" void InfoOut(char *txt, ...)
{
	va_list argptr;

	va_start( argptr, txt );
	vsnprintf( s_szErrMsg, sizeof(s_szErrMsg)-1, txt, argptr );
	va_end( argptr );
    AnsiOut("\x1B[1;34m");
	ConOut("i %s - %s", getNoXDate(), s_szErrMsg);
    AnsiOut("\x1B[0m");
}

extern "C" void PanicOut(char *txt, ...)
{
	va_list argptr;

	va_start( argptr, txt );
	vsnprintf( s_szErrMsg, sizeof(s_szErrMsg)-1, txt, argptr );
	va_end( argptr );

    AnsiOut("\x1B[1;31m");
	ConOut("! %s - %s", getNoXDate(), s_szErrMsg);
    AnsiOut("\x1B[0m");
}


extern "C" void DmpOut(char *txt, ...)
{
	va_list argptr;

	va_start( argptr, txt );
	vsnprintf( s_szErrMsg, sizeof(s_szErrMsg)-1, txt, argptr );
	va_end( argptr );

    AnsiOut("\x1B[33m");
	ConOut("--> %s", s_szErrMsg);
    AnsiOut("\x1B[0m");
}

