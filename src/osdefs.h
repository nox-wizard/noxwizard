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
\brief header regarding os definitions
\author ITB CompuPhase <info@compuphase.com>
\note Copyright 1998-2000, ITB CompuPhase, The Netherlands.
\note 2003/03/10 Added doxygen documentation - Akron

Every compiler uses different "default" macros to indicate the mode
it is in. Throughout the source, we use the Borland C++ macros, so
the macros of Watcom C/C++ and Microsoft Visual C/C++ are mapped to
those of Borland C++.

<ul>
<li><pre>__MSDOS__</pre>    set when compiling for DOS (not Windows)</li>
<li><pre>_Windows</pre>     set when compiling for any version of Microsoft Windows</li>
<li><pre>__WIN32__</pre>    set when compiling for Windows95 or WindowsNT (32 bit mode)</li>
<li><pre>__32BIT__</pre>    set when compiling in 32-bit "flat" mode (DOS or Windows)</li>
</ul>
*/

#ifndef _OSDEFS_H
#define _OSDEFS_H

#if defined(__WATCOMC__)
#  if defined(__WINDOWS__) || defined(__NT__)
#    define _Windows
#  endif
#  ifdef __386__
#    define __32BIT__
#  endif
#  if defined(_Windows) && defined(__32BIT__)
#    define __WIN32__
#  endif
#elif defined(_MSC_VER)
#  if defined(_WINDOWS) || defined(_WIN32)
#    define _Windows
#  endif
#  ifdef _WIN32
#    define __WIN32__
#    define __32BIT__
#  endif
#endif

/* Linux already has these */
#if !defined BIG_ENDIAN
  #define BIG_ENDIAN 4321
#endif
#if !defined LITTLE_ENDIAN
  #define LITTLE_ENDIAN 1234
#endif
/* educated guess, BYTE_ORDER is undefined, i386 is common => little endian */
#if !defined BYTE_ORDER
  #if defined UCLINUX
    #define BYTE_ORDER BIG_ENDIAN
  #else
    #define BYTE_ORDER LITTLE_ENDIAN
  #endif
#endif


#if defined(_Windows) && !defined(_INC_WINDOWS) && !defined(__WINDOWS_H) && !defined(NOWINDOWS)
#  include <windows.h>
#  include <windowsx.h>
#endif

#endif  /* _OSDEFS_H */
