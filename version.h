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
\brief header
*/

#ifndef __VERSION_H__
#define __VERSION_H__

//NXW versions number are :
// 
//MajorNumber.MinorNumber(Letter)
//
//Letter is :
//		i = internal version (includes internal tests, and releases to official testing shards)
//	    b = public released betas
//      x = special release
//      s = supported release
//      u = unsupported release
//
//		public releases usually fall in s or u categories

#define VER "**Oxygene**"
//Note : vernumb MUST always be 5 char lenght and WSCREQVER also must.
#define VERNUMB "0.82s"
//This is the requested version for the WSC to be read correctly
#define WSCREQVER "0.60s"
//This is the requested version for the server.cfg to be read correctly
#define SCPREQVER "0.67b"
#define HOTFIX "11AF"
#define VERHEXCODE 0x081

#define VTF_SUPPORTED 0x1
#define VTF_PRIVATE	  0x2
#define VTF_BETA      0x4
#define VTF_SPECIAL   0x8
#define VTF_GUI		  0x40

#define VT_S (VTF_SUPPORTED)
#define VT_U (0)
#define VT_I (VTF_PRIVATE|VTF_BETA)
#define VT_B (VTF_BETA)
#define VT_X (VTF_SPECIAL|VTF_SUPPORTED)

#define VERTYPE2 VT_X

//#define EXTREMELY_UNTESTED

#ifdef _WINDOWS
#define VERTYPE ((VERTYPE2|VTF_GUI))
#else
#define VERTYPE (VERTYPE2)
#endif




#define OS    "UNKNOWN OS"
#define OSFIX "[UNKNOWN OS]   "
#define PLATFORMID 0

#ifdef _CONSOLE
#undef OS
#undef OSFIX
#undef PLATFORMID
#define OS    ((ServerScp::g_nDeamonMode==0) ? ("Win32-Console") : ("WinNT-Service"))
#define OSFIX ((ServerScp::g_nDeamonMode==0) ? ("[Win32-Console]") : ("[WinNT-Service]"))
#define PLATFORMID 1                            
#endif 

#ifdef _WINDOWS
#undef OS
#undef OSFIX
#undef PLATFORMID
#define OS    (("Win32-GUI"))
#define OSFIX (("[Win32-Gui]    "))
#define PLATFORMID 2
#endif

#ifdef __unix__
#undef OS
#undef OSFIX
#undef PLATFORMID
#define OS    ((ServerScp::g_nDeamonMode==0) ? ("Linux-Console") : ("Linux-Deamon"))
#define OSFIX ((ServerScp::g_nDeamonMode==0) ? ("[GNU/Linux tty]") : ("[GNU/Linux dmn]"))
#define PLATFORMID 4                            
#endif

#if defined(__OpenBSD__)
#undef OS
#undef OSFIX
#undef PLATFORMID
#define OS    ((ServerScp::g_nDeamonMode==0) ? ("OpenBSD-Console") : ("OpenBSD-Deamon"))
#define OSFIX ((ServerScp::g_nDeamonMode==0) ? ("[OpenBSD tty]  ") : ("[OpenBSD dmn]  "))
#define PLATFORMID 6
#endif

#if defined(__FreeBSD__)
#undef OS
#undef OSFIX
#undef PLATFORMID
#define OS    ((ServerScp::g_nDeamonMode==0) ? ("FreeBSD-Console") : ("FreeBSD-Deamon"))
#define OSFIX ((ServerScp::g_nDeamonMode==0) ? ("[FreeBSD tty]  ") : ("[FreeBSD dmn]  "))
#define PLATFORMID 7
#endif


#ifdef __BEOS__
#undef OS
#undef OSFIX
#undef PLATFORMID
#define OS    "BeOS"
#define OSFIX "[BeOS 5]        "
#define PLATFORMID 5
#endif

#define PRODUCT (("NoX-Wizard"))


#define SUPPORTED_CLIENT "3.0.3a" 
// #define SUPPORTED_CLIENT_T3D "3.0.0 UO:Third Dawn Build 61"

// SUPPORTED_CLIENT is part of the client version control system.
// this is NOT necassairily the lastest client.
// don't change it unless you are sure that its packets structure is in synch with network.cpp etc.
// until I (LB) don't leave the wolfpack project, I want to be the only(!) one who
// changes that value. thx for your understanding =:)

// Your timezone
#define TIMEZONE "GMT+1"
// Information about the person who compiled this (Put your info here!)

#define NAME "Official Dev Team"

//NOTE: In order to modify this, please ask to the nox-wizard staff. Remember to do not remove any developer from PROGRAMMERS without nox-wizard staff permission
#define PROGRAMMERS "Xanathar, Luxor, Sparhawk, Elcabesa, Araknesh, Anthalir, Endymion, Akron and Riekr"

#define EMAIL "staff@noxwizard.com"


#endif //__VERSION_H__
