  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#if !defined(AFX_SPEECH_H__93559DDF_35B6_4DCC_BC04_4F7CECA54BB3__INCLUDED_)
#define AFX_SPEECH_H__93559DDF_35B6_4DCC_BC04_4F7CECA54BB3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

void responsevendor(NXWSOCKET  s, int vendor);
void talking(int s, string);


void wchar2char (const char* str);
void char2wchar (const char* str);


#endif // !defined(AFX_SPEECH_H__93559DDF_35B6_4DCC_BC04_4F7CECA54BB3__INCLUDED_)
