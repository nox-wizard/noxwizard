  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\author Xanathar
\brief Manages the entire Windows GUI, application startup and control
*/
#ifdef _WINDOWS

	#if !defined(AFX_XWGUI_H__3D2AFD80_7D78_4BF9_8725_00EA93BE01E5__INCLUDED_)
	#define AFX_XWGUI_H__3D2AFD80_7D78_4BF9_8725_00EA93BE01E5__INCLUDED_
	
	#if _MSC_VER > 1000
	#pragma once
	#endif // _MSC_VER > 1000
	
	#include "resource.h"
	
	
	#endif // !defined(AFX_XWGUI_H__3D2AFD80_7D78_4BF9_8725_00EA93BE01E5__INCLUDED_)
	
	#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
	#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_
	
	#if _MSC_VER > 1000
	#pragma once
	#endif // _MSC_VER > 1000
	
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	
	
	// Windows Header Files:
	#include <windows.h>
	
	// C RunTime Header Files
	#include <stdlib.h>
	#include <malloc.h>
	#include <memory.h>
	#include <tchar.h>
	void __cdecl xwprintf(char *txt, ...); // System message (In lower left corner)
	void __cdecl xwgets(char *s, int n); // gets from xwconsole
	#ifdef __cplusplus
	extern "C" {
	#endif
	void __cdecl xwprintf_C(char *txt, ...); // System message (In lower left corner)
	#ifdef __cplusplus
	}
	#endif
	extern HWND	g_HWnd;
	
	extern bool g_bDebuggingTriggers;
	
	// Local Header Files
	
	// TODO: reference additional headers your program requires here
	
	//{{AFX_INSERT_LOCATION}}
	// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	
	#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)

#endif //_WINDOWS
