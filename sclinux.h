  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __SC_LINUX_H
#define __SC_LINUX_H


#define getch               getchar
#define stricmp(a,b)        strcasecmp(a,b)
#define strnicmp strncasecmp

/*
 * WinWorld wants '\'. Unices do not.
 */
#define DIRECTORY_SEP_CHAR  '/'
#define DIRECTORY_SEP_STR       "/"

/*
 * Sc considers that a computer is Big Enddian if BIG_ENDIAN is defined.
 * Instead, Linux (well glibc) gives the order of bytes '4321' (see endian.h) for
 * BIG_ENDIAN, LITTLE_ENDIAN, and even PDP_ENDIAN.
 *
 * So we must define BIG_ENDIAN for SC only if the compiler is really
 * Big Endian. __BYTE_ORDER is designed for that.
 */

#if defined(__OpenBSD__)
	#define __BYTE_ORDER BYTE_ORDER
	#define __LITTLE_ENDIAN LITTLE_ENDIAN
	#define __BIG_ENDIAN BIG_ENDIAN
#endif

#if defined(__FreeBSD__)
	#include <sys/types.h>
	#define __BYTE_ORDER BYTE_ORDER
	#define __LITTLE_ENDIAN LITTLE_ENDIAN
	#define __BIG_ENDIAN BIG_ENDIAN
//    typedef unsigned char  u_char;
//    typedef unsigned short u_short;
//    typedef unsigned long  u_long;
#endif

#ifndef __BEOS__

#undef  BIG_ENDIAN
#ifndef __BYTE_ORDER
#include <stdlib.h>
#endif
#ifndef __BYTE_ORDER
// Xan : default to x86
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#if  __BYTE_ORDER ==  __BIG_ENDIAN
#ifndef BIG_ENDIAN
#define BIG_ENDIAN
#endif
#endif

#endif

#ifdef __BEOS__
#undef BIG_ENDIAN
#endif


#endif
