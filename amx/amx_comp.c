/************************************************************************************
 by Xanathar :

  Interface to the Small compiler


  This module allows one not to have to retouch the compiler too much :).

  To integrate the Compiler :
               - simply change files in the compiler directory with those from 
			     Small package :)
			   - remove main, sc_error and sc_printf functions from sclib.c
			   - find in files indent_nowarn everywhere and set it to TRUE
			   - remove all #include <sclinux.h>
			   - test, test, test



  To integrate a new amx vm :
			   - remember to make changes for trace mode in amx.c

 **********************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "console.h"

#ifdef _MSC_VER
	#pragma warning(disable: 4244)
	#pragma warning(disable: 4018)
	#pragma warning(disable: 4761)
	#pragma warning(disable: 4100)
	#pragma warning(disable: 4125)
	#pragma warning(disable: 4115)
#endif

int sc_printf(const char *txt, ...)
{
	static char buffy[2*1024];

	va_list argptr;

	va_start( argptr, txt );
	vsprintf( buffy, txt, argptr );
	va_end( argptr );

	SDbgOut("%s", buffy);
	return 0;
}


int sc_error(int number,char *message,char *filename,int firstline,int lastline,va_list argptr)
{
static char *prefix[3]={ "Error", "Fatal", "Warning" };
static char buffy[2048];

  if (number!=0) {
    char *pre;

    pre=prefix[number/100];
    if (firstline>=0)
      sc_printf("%s(%d -- %d) %s [%03d]: ",filename,firstline,lastline,pre,number);
    else
      sc_printf("%s(%d) %s [%03d]: ",filename,lastline,pre,number);
  } /* if */

  //va_list v = argptr;
  vsprintf(buffy,message,argptr);
  SDbgOut("%s", buffy);
  //fflush(stdout);
  return 0;
}

#ifdef __unix__
#include "sclinux.h"
#define LINUX
#endif
#include "compiler/sclib.c"


int compiler_main(int argc, char **argv)
{
	int ret; 
	ConOut("\x80\x80\t__________________________________________________________________________\n\n");
	ret = sc_compile(argc, argv);
	ConOut("\t__________________________________________________________________________\n\n");
	return ret;
}
