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

//undefining this will force to use old-style toolbar buttons.
//if defined the program will check the system for flatbar support and use
//the appropriate toolbar type :]
#ifndef __GNUC__
#define USE_FLAT_TOOLBAR
#endif

bool g_bUseFlatToolbar = false;

#include "nxwgui.h"
#include "resource.h"
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <stdio.h>
#include <commctrl.h>
#include <shlwapi.h>

extern bool g_bDebuggingTriggers;

namespace ServerScp {
extern int g_nLoadDebugger;
extern int g_nCheckBySmall;
}

#define MNUDEBUG 5

void display(HDC hDC, LPRECT lpRect);
void initScr();
void printLn (char *s);
void __cdecl xwprintf(char *txt, ...); // System message (In lower left corner)
void __cdecl backspace();

#pragma comment( exestr , "Program by Marco Mastropaolo, aka Xanathar" )
#define GETSBUFLIMIT 4096
static CRITICAL_SECTION s_csXwgs;
static bool s_bGetsDone = false;
static char s_strBuffer[GETSBUFLIMIT];
static int s_iBufPtr = 0;
static bool s_bInGetS = false;


extern char INKEY;
#define MAX_LOADSTRING 100 

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
//TCHAR *szTitle = "NoX-Wizard UOL Server";
TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text
static CRITICAL_SECTION s_csXwpf;

HFONT g_Font;
HMENU g_Menu;
HMENU mnu, mnu2;
HWND	g_HWnd;
HWND	g_hToolbar;

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SplashWnd(HWND, UINT, WPARAM, LPARAM);
#define MAXX 256
#define MAXY 2048
static char screen[MAXY+1][MAXX+1];
static int crx = 0, cry = 0;
static char txt[MAXX+10];
static char s_strMsg[32768];

static int g_nHPos = 0;

extern "C" int g_nTraceMode;

void initToolbar(HWND hWnd, HINSTANCE hInstance);
void ToolTip (WPARAM wParam, LPARAM lParam);
void mnuShow (int style);
void enableAllMenu (int ID, bool enabled);
void enableMenuAndBtnID(int ID, bool enabled);


#define DISABLE_ALL 0


#define BACKCOLOR RGB(0x00,0x11,0x33) 
#define CURIPCOLOR RGB(0,0,192)
#define CURIPTEXT RGB(0xFF,0xFF,0xFF)
#define TEXTCOLOR ((ServerScp::g_nLoadDebugger!=0) ? RGB(192,192,192) : RGB(255,255,255))
#define INPUTCOLOR (RGB(255,128,255) )
#define DEBUGCOLOR (RGB(0xFF,0xC0,0))
#define TRACECOLOR (RGB(0,0xC0,255))

bool g_bScrLock = false;

//HBITMAP StretchBitmap (HBITMAP hBitmap1);
//void DeleteAllBitmaps (HWND hwnd);

extern "C" int win32_main(int argc, char **argv);

#define MAXARGV 12
char *gArgv[MAXARGV];
int gArgc = 0;

void __cdecl thr0 (void *p)
{
	win32_main(0,NULL);
	exit(0);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
//	parseArgCArgV(lpCmdLine);
	InitializeCriticalSection(&s_csXwpf);
	InitializeCriticalSection(&s_csXwgs);
	initScr();

	if (strstr(lpCmdLine, "-debug")) ServerScp::g_nLoadDebugger = 1;
	if (strstr(lpCmdLine, "-check")) ServerScp::g_nCheckBySmall = 1;
	
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_XWGUI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}


	mnuShow(DISABLE_ALL);

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_XWGUI);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}


	return msg.wParam;
}

#define ENABLE_RUNMODE

void updateMenus()
{
	enableAllMenu(0,true);
	enableAllMenu(1,true);
	enableAllMenu(2,true);
	enableAllMenu(3,true);
	enableAllMenu(4,true);
	enableMenuAndBtnID(ID_HELP_SHOWKEYBOARDCOMMANDS,true);
	if (ServerScp::g_nLoadDebugger) {
		enableAllMenu(5,true);
		enableMenuAndBtnID(ID_DEBUG_STEPIN,false);
		enableMenuAndBtnID(ID_DEBUG_STEPOUT,false);
		enableMenuAndBtnID(ID_DEBUG_GO,false);
		enableMenuAndBtnID(ID_DEBUG_SHOWVARIABLES,false);
		enableMenuAndBtnID(ID_DEBUG_SHOWWATCHES,false);
		enableMenuAndBtnID(ID_DEBUG_STEPOUTCF,false);
		enableMenuAndBtnID(ID_DEBUG_SHOWCALLSTACK,false);
		enableMenuAndBtnID(ID_DEBUG_LISTFUNCTIONS,false);
		enableMenuAndBtnID(ID_DEBUG_LISTBREAKPOINTS,false);
		enableMenuAndBtnID(ID_DEBUG_REMOVEALLBREAKPOINTS,false);
		enableMenuAndBtnID(ID_DEBUG_REMOVEALLWATCHES,false);
	}
	RedrawWindow(g_HWnd,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance) 
{
	WNDCLASSEX wcex;


	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_XWGUI);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)CreateSolidBrush(BACKCOLOR);//(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_XWGUI;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}


HRESULT GetComCtlVersion(LPDWORD pdwMajor, LPDWORD pdwMinor)
{
HINSTANCE   hComCtl;

if(   IsBadWritePtr(pdwMajor, sizeof(DWORD)) || 
      IsBadWritePtr(pdwMinor, sizeof(DWORD)))
   return E_INVALIDARG;

//load the DLL
hComCtl = LoadLibrary(TEXT("comctl32.dll"));

if(hComCtl)
   {
   HRESULT           hr = S_OK;
   DLLGETVERSIONPROC pDllGetVersion;
   
   /*
   You must get this function explicitly because earlier versions of the DLL 
   don't implement this function. That makes the lack of implementation of the 
   function a version marker in itself.
   */
   pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hComCtl, TEXT("DllGetVersion"));
   
   if(pDllGetVersion)
      {
      DLLVERSIONINFO    dvi;
      
      ZeroMemory(&dvi, sizeof(dvi));
      dvi.cbSize = sizeof(dvi);
   
      hr = (*pDllGetVersion)(&dvi);
      
      if(SUCCEEDED(hr))
         {
         *pdwMajor = dvi.dwMajorVersion;
         *pdwMinor = dvi.dwMinorVersion;
         }
      else
         {
         hr = E_FAIL;
         }   
      }
   else
      {
      /*
      If GetProcAddress failed, then the DLL is a version previous to the one 
      shipped with IE 3.x.
      */
      *pdwMajor = 4;
      *pdwMinor = 0;
      }
   
   FreeLibrary(hComCtl);

   return hr;
   }

return E_FAIL;
}


// This function compares the passed in "suite name" string
// to the product suite information stored in the registry.
// This only works on the Terminal Server 4.0 platform.

BOOL ValidateProductSuite (LPSTR SuiteName)
{
    BOOL rVal = FALSE;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPSTR ProductSuite = NULL;
    LPSTR p;

    Rslt = RegOpenKeyA(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Control\\ProductOptions",
        &hKey
        );

    if (Rslt != ERROR_SUCCESS)
        goto exit;

    Rslt = RegQueryValueExA( hKey, "ProductSuite", NULL, &Type, NULL, &Size );
    if (Rslt != ERROR_SUCCESS || !Size)
        goto exit;

    ProductSuite = (LPSTR) LocalAlloc( LPTR, Size );
    if (!ProductSuite)
        goto exit;

    Rslt = RegQueryValueExA( hKey, "ProductSuite", NULL, &Type,
        (LPBYTE) ProductSuite, &Size );
     if (Rslt != ERROR_SUCCESS || Type != REG_MULTI_SZ)
        goto exit;

    p = ProductSuite;
    while (*p)
    {
        if (lstrcmpA( p, SuiteName ) == 0)
        {
            rVal = TRUE;
            break;
        }
        p += (lstrlenA( p ) + 1);
    }

exit:
    if (ProductSuite)
        LocalFree( ProductSuite );

    if (hKey)
        RegCloseKey( hKey );

    return rVal;
}
// This function performs the basic check to see if
// the platform on which it is running is Terminal
// services enabled.  Note, this code is compatible on
// all Win32 platforms.  For the Windows 2000 platform
// we perform a "lazy" bind to the new product suite
// APIs that were first introduced on that platform.

BOOL IsTerminalServicesEnabled( VOID )
{
    BOOL    bResult = FALSE;    // assume Terminal Services is not enabled
    DWORD   dwVersion;
  
	dwVersion = GetVersion();

    // are we running NT ?
    if (!(dwVersion & 0x80000000))
    {
        // Is it Windows 2000 (NT 5.0) or greater ?
        if (LOBYTE(LOWORD(dwVersion)) > 4)
        {
			bResult = GetSystemMetrics( 0x1000 );
        }
        else
        {
            // This is NT 4.0 or older
            bResult = ValidateProductSuite( "Terminal Server" );
        }
    }

    return bResult;
}


//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
int g_nPos = MAXY;
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable
	InitCommonControls();

   DWORD Major, Minor;

   GetComCtlVersion(&Major, &Minor);

/*   char szz[800];
   sprintf(szz, "CommCtl32 version %d.%d", Major, Minor);
*///   MessageBox(NULL, szz, "CommCtl32", 0);

   if ((Major>4)||(Minor>71)) g_bUseFlatToolbar = true;
   else g_bUseFlatToolbar = false;

   bool bWts = false;
   
   if (IsTerminalServicesEnabled()) {
	int btn = MessageBox(NULL, "You're running NoX-Wizard on a Windows NT 4.0 Server Terminal Server Edition on any console/terminal, or on a Windows 2000 Server or Windows XP Server with Terminal Services enabled and on a remote terminal.\n\nIt's not reccomended to run NoX-Wizard GUI on a terminal based system due to performances reasons. You should use the Console/Service version of NoX-Wizard.\n\nDo you want to continue anyway ?", 
		"Error - Terminal Session Detected",MB_YESNO|MB_ICONSTOP);
	if (btn != IDYES) return FALSE;
   }

   
   hWnd = CreateWindowEx(WS_EX_APPWINDOW|WS_EX_WINDOWEDGE
	   ,szWindowClass, szTitle, WS_CLIPCHILDREN|WS_OVERLAPPEDWINDOW|WS_THICKFRAME|WS_BORDER|WS_VSCROLL|WS_HSCROLL,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   initToolbar(hWnd, hInstance);


	if (g_hToolbar==NULL) return FALSE;
 

   g_Font = (HFONT)GetStockObject(ANSI_FIXED_FONT); 

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   g_HWnd = hWnd;
  // InitializeFlatSB(hWnd);


   g_Menu = GetMenu(hWnd);
	mnu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_XWPOPUP));
	mnu2 = GetSubMenu(mnu,0);

   long l = _beginthread(thr0, 0, NULL);
 
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
// CreateToolbarEx

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	g_HWnd = hWnd;

	SCROLLINFO si;
	int nVert = 0;

/*    OSVERSIONINFO vi = { sizeof(vi) };
    GetVersionEx(&vi);
*/	//skip splash screen under Win9x [don't know why, but it freezes the system! :(]

	switch (message) 
	{
	case WM_NOTIFY:
		ToolTip(wParam, lParam);

	case WM_SIZE:
			SendMessage(g_hToolbar,WM_SIZE, wParam,lParam); 
			return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_CREATE:
			    SetScrollRange(g_HWnd, SB_VERT, 0, MAXY, TRUE);
				si.cbSize = sizeof(si);
				si.fMask  = SIF_ALL;
				GetScrollInfo(g_HWnd, SB_VERT, &si);
				si.nPos = MAXY;
				si.fMask = SIF_POS;
				SetScrollInfo(g_HWnd, SB_VERT, &si, TRUE);
		    return 0;
	case WM_VSCROLL:
			{
				si.cbSize = sizeof(si);
				si.fMask  = SIF_ALL;
				GetScrollInfo(g_HWnd, SB_VERT, &si);
				nVert = si.nPos ;
				switch(LOWORD(wParam))
				{
					case SB_TOP:
						si.nPos = si.nMin;
						break;
					case SB_BOTTOM:
						si.nPos = si.nMax;
						break;
					case SB_LINEUP:
						si.nPos -= 1;
						break;
					case SB_LINEDOWN:
						si.nPos += 1;
						break;
					case SB_PAGEUP:
						si.nPos -= 25;
						break;
					case SB_PAGEDOWN:
						si.nPos += 25;
						break;
					case SB_THUMBTRACK:
						si.nPos = si.nTrackPos;
						break;
					default : 
						break;
				}
				si.fMask = SIF_POS;
				SetScrollInfo(g_HWnd, SB_VERT, &si, TRUE);
				GetScrollInfo(g_HWnd, SB_VERT, &si);
				if (si.nPos != nVert) {
					g_nPos = si.nPos;
					RedrawWindow(g_HWnd,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_INTERNALPAINT);
				}
				return 0;
			}

	case WM_HSCROLL:
			{
				si.cbSize = sizeof(si);
				si.fMask  = SIF_ALL;
				GetScrollInfo(g_HWnd, SB_HORZ, &si);
				nVert = si.nPos ;
				switch(LOWORD(wParam))
				{
					case SB_TOP:
						si.nPos = si.nMin;
						break;
					case SB_BOTTOM:
						si.nPos = si.nMax;
						break;
					case SB_LINEUP:
						si.nPos -= 1;
						break;
					case SB_LINEDOWN:
						si.nPos += 1;
						break;
					case SB_PAGEUP:
						si.nPos -= 5;
						break;
					case SB_PAGEDOWN:
						si.nPos += 5;
						break;
					case SB_THUMBTRACK:
						si.nPos = si.nTrackPos;
						break;
					default : 
						break;
				}
				si.fMask = SIF_POS;
				SetScrollInfo(g_HWnd, SB_HORZ, &si, TRUE);
				GetScrollInfo(g_HWnd, SB_HORZ, &si);
				if (si.nPos != nVert) {
					g_nHPos = si.nPos;
					RedrawWindow(g_HWnd,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_INTERNALPAINT);
				}
				return 0;
			}
		case WM_CHAR:
			{
				char chCharCode = (char) wParam;    // character code 
				if(!s_bInGetS) {
					if ((chCharCode!='\r')&&(chCharCode!='\n')&&(chCharCode!='\0')
						&&(chCharCode!='s')&&(chCharCode!='S')&&(chCharCode!='n')&&(chCharCode!='N')) {
							INKEY = chCharCode;
					}
					return 0;
				}
				EnterCriticalSection(&s_csXwgs);
				if ((chCharCode=='\r')||(chCharCode=='\n')||(chCharCode=='\0')) {
					s_bGetsDone = true;
					chCharCode='\0';
				}
				if (chCharCode=='\x8') {
					if (s_iBufPtr!=0) {
						s_iBufPtr--;
						backspace();
					}
				} else {
					if(s_iBufPtr<(GETSBUFLIMIT-2)) s_strBuffer[s_iBufPtr++] = chCharCode;
					if (chCharCode!='\0') xwprintf("%c", chCharCode);
					else xwprintf("\n", chCharCode); 
				}
				LeaveCriticalSection(&s_csXwgs);
			}
			return 0;

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
			case ID_VIEW_LOCKSCREEN:
				g_bScrLock = !g_bScrLock;

				CheckMenuItem(g_Menu, ID_VIEW_LOCKSCREEN, MF_BYCOMMAND|((g_bScrLock) ? MF_CHECKED : MF_UNCHECKED));
				SendMessage(g_hToolbar,TB_SETSTATE, ID_VIEW_LOCKSCREEN,
					MAKELONG(((g_bScrLock) ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED),0)); 

				break;
			case ID_DEBUG_BREAKONNEXTAMXCALL:
				INKEY = 'B';
				break;
			case ID_DEBUG_BREAKONNEXTLEGACYTRIGGER:
				INKEY = 'C';
				break;
			case ID_DEBUG_TRACENATIVEFUNCTIONS:
					g_nTraceMode = 1-g_nTraceMode;
					CheckMenuItem(g_Menu,ID_DEBUG_TRACENATIVEFUNCTIONS, 
						MF_BYCOMMAND|((g_nTraceMode!=0) ? MF_CHECKED : MF_UNCHECKED));
					SendMessage(g_hToolbar,TB_SETSTATE, ID_DEBUG_TRACENATIVEFUNCTIONS,
						MAKELONG(((g_nTraceMode!=0) ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED),0)); 
					break;


				case ID_DEBUG_STEPOUTCF:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "GO RET");
					s_bGetsDone = true;
					xwprintf("\n");
					LeaveCriticalSection(&s_csXwgs);
				   break;
				case ID_DEBUG_SHOWCALLSTACK:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "CALLS");
					s_bGetsDone = true;
					xwprintf("\n");
					LeaveCriticalSection(&s_csXwgs);
				   break;
				case ID_DEBUG_LISTFUNCTIONS:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "L FUNCS");
					s_bGetsDone = true;
					xwprintf("\n");
					LeaveCriticalSection(&s_csXwgs);
				   break;
				case ID_DEBUG_LISTBREAKPOINTS:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "BREAK");
					s_bGetsDone = true;
					xwprintf("\n");
					LeaveCriticalSection(&s_csXwgs);
				   break;
				case ID_DEBUG_REMOVEALLBREAKPOINTS:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "CBREAK *");
					s_bGetsDone = true;
					xwprintf("\n");
					LeaveCriticalSection(&s_csXwgs);
				   break;
				case ID_DEBUG_REMOVEALLWATCHES:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "CW *");
					s_bGetsDone = true;
					LeaveCriticalSection(&s_csXwgs);
				   break;

				case ID_DEBUG_GO:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "G");
					s_bGetsDone = true;
					xwprintf("\n");
					LeaveCriticalSection(&s_csXwgs);
				   break;
				case ID_DEBUG_STEPIN:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "S");
					s_bGetsDone = true;
					xwprintf("\n");
					LeaveCriticalSection(&s_csXwgs);
				   break;
				case ID_DEBUG_STEPOUT:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "N");
					s_bGetsDone = true;
					xwprintf("\n");
					LeaveCriticalSection(&s_csXwgs);
				   break;
				case ID_DEBUG_SHOWVARIABLES:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "D");
					s_bGetsDone = true;
					xwprintf("\n");
					LeaveCriticalSection(&s_csXwgs);
				   break;
				case ID_DEBUG_SHOWWATCHES:
					EnterCriticalSection(&s_csXwgs);
					sprintf(s_strBuffer, "W");
					s_bGetsDone = true;
					xwprintf("\n");
					LeaveCriticalSection(&s_csXwgs);
				   break;
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
					INKEY = 'Q'; //ReadFile
				   break;
				case ID_FILE_SAVEWORLD:
					INKEY = '#';
				   break;
				case ID_ADMIN_SHUTDOWNIN10MINUTES:
					INKEY = 'T';
				   break;
				case ID_ADMIN_WHOISONLINE:
					INKEY = 'W';
				   break;
				case ID_HELP_SHOWKEYBOARDCOMMANDS:
					INKEY = '?';
				   break;
				case ID_ADMIN_PERFORMANCEDUMP:
					INKEY = 'P';
				   break;
				case ID_RELOAD_RELOADACCOUNTS:
					INKEY = 'A';
				   break;
				case ID_TOOLS_DISCONNECTACCOUNT0:
					INKEY = 'D';
				   break;
   				case ID_TOOLS_TOGGLEHEARTBEAT:
					INKEY = 'H';
				   break;
   				case ID_TOOLS_TOGGLELAYERDISPLAY:
					INKEY = 'L';
				   break;
   				case ID_RELOAD_RELOADCLASSFILES:
					INKEY = 'X';
				   break;

				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			RECT rt;
			GetClientRect(hWnd, &rt);
			display( hdc, &rt);

			EndPaint(hWnd, &ps);
			break;
		case WM_CONTEXTMENU:
			{
				POINT pt;
				pt.x = GET_X_LPARAM(lParam); 
				pt.y = GET_Y_LPARAM(lParam); 
				TrackPopupMenuEx(mnu2,TPM_LEFTALIGN | TPM_LEFTBUTTON,pt.x,pt.y,g_HWnd,NULL);
			}
			break;
			
		case WM_DESTROY:
			PostQuitMessage(0);
			exit(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;

	}
    return FALSE;
}





///////////////////////////////////////////////////////////////////
// Function name     : initScr
// Description       : initializes the screen buffer
// Return type       : void 
// Author            : Xanathar 
// Changes           : none yet
void initScr()
{
	for (int x=0; x<MAXX; x++)
		for (int y=0; y<MAXY; y++)
			screen[y][x] = '\0';
}


///////////////////////////////////////////////////////////////////
// Function name     : printLn 
// Description       : formats a string to be printed in the screen buffer
// Return type       : void 
// Author            : Xanathar 
// Argument          : char *s -> string to be printed
// Changes           : none yet
void printLn (char *s)
{
	bool colored = false;
	char clrchar = '\x80';
	int i;
	int l = strlen(s);

	if( (s[0]==(char)0x80) || (s[0]==(char)0x81) )
	{
		colored= true;
		clrchar= s[0];
		s++;

		if( crx==0 )
		{
			screen[cry][0]= s[0];
			crx++;
		}
	}

	for(i=0; i<l; i++) 
	{
		if (crx >= MAXX) 
		{
			screen[cry][crx] = '\0';
			crx %= MAXX;
			cry++;
			cry %= MAXY;
			if (colored) 
				screen[cry][crx++] = clrchar;
		}

		cry %= MAXY;

		if ((s[i]=='\n')) 
		{
			screen[cry][crx] = '\0';
			crx = 0; 
			cry++;
			cry %= MAXY;
			if (colored&&(i<l-1)) screen[cry][crx++] = clrchar;
			continue;
		}

		if ((s[i]!=clrchar) && (s[i]!='\0')) 
			screen[cry][crx++] = s[i];
	}
	screen[cry][crx] = '\0';

	if (!g_bScrLock) 
	{
		g_nPos = MAXY;
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask  = SIF_ALL;
		GetScrollInfo(g_HWnd, SB_VERT, &si);
		si.nPos = MAXY;
		si.fMask = SIF_POS;
		SetScrollInfo(g_HWnd, SB_VERT, &si, TRUE);
	}

	RedrawWindow(g_HWnd,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_INTERNALPAINT);
}



///////////////////////////////////////////////////////////////////
// Function name     : __cdecl xwgets
// Description       : manages gets mode (used for debugger only)
// Return type       : void 
// Author            : Xanathar 
// Argument          : char *s -> string to be filled
// Argument          : int n -> max number of chars (like in fgets)
// Changes           : none yet
void __cdecl xwgets(char *s, int n) // gets from xwconsole
{
	int i = 0;
	bool done = false;
	static bool first = false;

	if (!first) {
		first = true;
		strcpy(s, "LIST ON");
		return;
	}

	if (!g_bDebuggingTriggers) {
		enableMenuAndBtnID(ID_DEBUG_STEPOUTCF,true);
		enableMenuAndBtnID(ID_DEBUG_SHOWCALLSTACK,true);
		enableMenuAndBtnID(ID_DEBUG_LISTFUNCTIONS,true);
		enableMenuAndBtnID(ID_DEBUG_LISTBREAKPOINTS,true);
		enableMenuAndBtnID(ID_DEBUG_REMOVEALLBREAKPOINTS,true);
		enableMenuAndBtnID(ID_DEBUG_REMOVEALLWATCHES,true);
		enableMenuAndBtnID(ID_DEBUG_STEPIN,true);
		enableMenuAndBtnID(ID_DEBUG_SHOWVARIABLES,true);
		enableMenuAndBtnID(ID_DEBUG_SHOWWATCHES,true);
	}
	enableMenuAndBtnID(ID_DEBUG_STEPOUT,true);
	enableMenuAndBtnID(ID_DEBUG_GO,true);
	enableMenuAndBtnID(ID_DEBUG_BREAKONNEXTAMXCALL, false);
	enableMenuAndBtnID(ID_DEBUG_BREAKONNEXTLEGACYTRIGGER, false);
	enableAllMenu(0,false);
	enableAllMenu(2,false);
	enableAllMenu(4,false);
	enableAllMenu(3,false);
	enableAllMenu(MNUDEBUG,true);
	enableMenuAndBtnID(ID_HELP_SHOWKEYBOARDCOMMANDS,false);
	enableMenuAndBtnID(ID_DEBUG_TRACENATIVEFUNCTIONS,true);

	RedrawWindow(g_HWnd,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
	CheckMenuItem(g_Menu,ID_DEBUG_TRACENATIVEFUNCTIONS, 
		MF_BYCOMMAND|((g_nTraceMode!=0) ? MF_CHECKED : MF_UNCHECKED));
	SendMessage(g_hToolbar,TB_SETSTATE, ID_DEBUG_TRACENATIVEFUNCTIONS,
		MAKELONG(((g_nTraceMode!=0) ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED),0)); 

	EnterCriticalSection(&s_csXwgs);
	s_bInGetS = true;
	LeaveCriticalSection(&s_csXwgs);
	printLn ("");
	while (!done) 
	{
		EnterCriticalSection(&s_csXwgs);
		done = s_bGetsDone;
		LeaveCriticalSection(&s_csXwgs);
		Sleep(1);
	}
	EnterCriticalSection(&s_csXwgs);
	strncpy(s, s_strBuffer, n); 
	s_bGetsDone = false;
	s_bInGetS = false;
	s_iBufPtr = 0;
	LeaveCriticalSection(&s_csXwgs);
	enableMenuAndBtnID(ID_DEBUG_STEPIN,false);
	enableMenuAndBtnID(ID_DEBUG_STEPOUT,false);
	enableMenuAndBtnID(ID_DEBUG_GO,false);
	enableMenuAndBtnID(ID_DEBUG_SHOWVARIABLES,false);
	enableMenuAndBtnID(ID_DEBUG_SHOWWATCHES,false);
	enableMenuAndBtnID(ID_DEBUG_STEPOUTCF,false);
	enableMenuAndBtnID(ID_DEBUG_SHOWCALLSTACK,false);
	enableMenuAndBtnID(ID_DEBUG_LISTFUNCTIONS,false);
	enableMenuAndBtnID(ID_DEBUG_LISTBREAKPOINTS,false);
	enableMenuAndBtnID(ID_DEBUG_REMOVEALLBREAKPOINTS,false);
	enableMenuAndBtnID(ID_DEBUG_REMOVEALLWATCHES,false);
	enableAllMenu(0,true);
	enableAllMenu(1,true);
	enableAllMenu(2,true);
	enableAllMenu(3,true);
	enableAllMenu(4,true);
	enableMenuAndBtnID(ID_DEBUG_BREAKONNEXTAMXCALL, true);
	enableMenuAndBtnID(ID_DEBUG_BREAKONNEXTLEGACYTRIGGER, true);
//	EnableMenuItem(g_Menu,5,MF_ENABLED|MF_BYPOSITION);
	enableMenuAndBtnID(ID_HELP_SHOWKEYBOARDCOMMANDS,true);
	CheckMenuItem(g_Menu,ID_DEBUG_TRACENATIVEFUNCTIONS, 
		MF_BYCOMMAND|((g_nTraceMode!=0) ? MF_CHECKED : MF_UNCHECKED));
	SendMessage(g_hToolbar,TB_SETSTATE, ID_DEBUG_TRACENATIVEFUNCTIONS,
		MAKELONG(((g_nTraceMode!=0) ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED),0)); 
	RedrawWindow(g_HWnd,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
	printLn("");
}


///////////////////////////////////////////////////////////////////
// Function name     : __cdecl backspace
// Description       : backspaces when in edit (gets) mode
// Return type       : void 
// Author            : Xanathar 
// Changes           : none yet
void __cdecl backspace()
{
	EnterCriticalSection(&s_csXwpf);
	crx--;
	printLn ("");
	LeaveCriticalSection(&s_csXwpf);

}


///////////////////////////////////////////////////////////////////
// Function name     : __cdecl xwprintf
// Description       : printf stub to printLn (for C++ code)
// Return type       : void 
// Author            : Xanathar 
// Argument          : char *txt -> printf format 
// Argument          : ... -> printf parameters
// Changes           : none yet
void __cdecl xwprintf(char *txt, ...) // Printf to xwconsole
{
	va_list argptr;
	EnterCriticalSection(&s_csXwpf);

	va_start( argptr, txt );
	vsprintf( s_strMsg, txt, argptr );
	va_end( argptr );
	s_strMsg[32000] = '\0'; //ensure we are under the line max lenght :), Xan
	
	printLn(s_strMsg);
	LeaveCriticalSection(&s_csXwpf);
}


///////////////////////////////////////////////////////////////////
// Function name     : "C" void __cdecl xwprintf_C
// Description       : xwprintf for C code (see xwprintf, above)
// Return type       : extern 
// Author            : Xanathar 
// Argument          : char *txt -> -- as default --
// Argument          : ... -> -- as default --
// Changes           : none yet
extern "C" void __cdecl xwprintf_C(char *txt, ...) // Printf to xwconsole
{
	va_list argptr;
	EnterCriticalSection(&s_csXwpf);

	va_start( argptr, txt );
	vsprintf( s_strMsg, txt, argptr );
	va_end( argptr );
	s_strMsg[32000] = '\0'; //ensure we are under the line max lenght :), Xan
	
	printLn(s_strMsg);
	LeaveCriticalSection(&s_csXwpf);
}



///////////////////////////////////////////////////////////////////
// Function name     : display
// Description       : redraws the text (handles WM_PAINT)
// Return type       : void 
// Author            : Xanathar 
// Argument          : HDC hDC -> device context where we draw
// Argument          : LPRECT lpRect -> rect to be redrawn
// Changes           : none yet
void display(HDC hDC, LPRECT lpRect)
{
	int y, ry;
	#ifndef __GNUC__
	__try {
	#else
	try {
	#endif
		txt[0] = '\0';
		char *ttt = txt;
		static int calib;
		static int corr = 0;
		
		if (calib==0) 
		{
			SetBkColor(hDC, BACKCOLOR);
			SetTextColor(hDC, BACKCOLOR);
			SelectObject(hDC, g_Font);
			calib = DrawText(hDC, "NoX-Wizard is initializing\nplease wait...\n.....",-1,lpRect,0);
			return;
		}

		int h = calib/3;

		int delta = (lpRect->bottom - lpRect->top) % h ;
		lpRect->left += 2-8*g_nHPos;
		lpRect->top+=12+delta;

		SetBkColor(hDC, BACKCOLOR);
		SetTextColor(hDC, TEXTCOLOR);
		
		if (calib!=0) 
		{
			int hg = lpRect->bottom - lpRect->top ;
			int hx = hg / h;
			hx+=corr;
			
			//hx -> numero di righe visibili
			int scrollmax = MAXY-hx;
			int newpos = (g_nPos*scrollmax)/MAXY;

			SelectObject(hDC, g_Font);

			for (y = 1; y <= hx; y++)
			{
				ry = ((cry - hx + y + MAXY)+newpos-scrollmax) % MAXY;
				//strcat(txt,screen[ry]);
				if ((y==hx)&&(s_bInGetS)) {
					//we're in gets -> change the line to be the last 
					lpRect->left -= 5;
					lpRect->right += 5;
					FillRect(hDC, lpRect, (HBRUSH)(GetStockObject(BLACK_BRUSH)));
					SetBkColor(hDC, RGB(0,0,0));
					ry = ((cry - hx + y + MAXY)) % MAXY;
					strcpy(txt,screen[ry]);
					strcat(txt,"\x7F");
					//int ln = DrawText(hDC, txt, strlen(txt), lpRect, DT_NOCLIP|DT_EXPANDTABS);
	
					if (screen[ry][0]=='\x80') 
					{
						SetTextColor(hDC, INPUTCOLOR);
						int ln = DrawText(hDC, txt+1, strlen(txt)-1, lpRect, DT_EXPANDTABS);
						SetTextColor(hDC, TEXTCOLOR);
					} 
					else if (screen[ry][0]=='\x81') {
						SetTextColor(hDC, INPUTCOLOR);
						int ln = DrawText(hDC, txt+1, strlen(txt)-1, lpRect, DT_EXPANDTABS);
						SetTextColor(hDC, TEXTCOLOR);
					} 
					else 
					{
						int ln = DrawText(hDC, txt, strlen(txt), lpRect, DT_EXPANDTABS);
					}

					SetBkColor(hDC, BACKCOLOR);
					FrameRect(hDC, lpRect, 
						static_cast<struct HBRUSH__ *>(GetStockObject(GRAY_BRUSH)));
					lpRect->left += 5;
 
					lpRect->top += h;
				} 
				else 
				{
					if (screen[ry][0]=='\x80') 
					{
						SetTextColor(hDC, DEBUGCOLOR);
						if (screen[ry][1]=='=') { SetBkColor(hDC, CURIPCOLOR);SetTextColor(hDC, CURIPTEXT); }
						int ln = DrawText(hDC, screen[ry]+1, strlen(screen[ry])-1, lpRect, DT_EXPANDTABS);
						if (screen[ry][1]=='=') SetBkColor(hDC, BACKCOLOR);
						SetTextColor(hDC, TEXTCOLOR);
					} 
					else if (screen[ry][0]=='\x81') 
					{
						SetTextColor(hDC, TRACECOLOR);
						int ln = DrawText(hDC, screen[ry]+1, strlen(screen[ry])-1, lpRect, DT_EXPANDTABS);
						SetTextColor(hDC, TEXTCOLOR);
					} 
					else 
					{
						int ln = DrawText(hDC, screen[ry], strlen(screen[ry]), lpRect, DT_EXPANDTABS);
					}

					lpRect->top += h;
				}
//					strcat(txt,"\n");
			}

            // XAN -> we'll do it row by row now :)
			//int ln = DrawText(hDC, ttt, strlen(txt), lpRect, DT_NOCLIP|DT_EXPANDTABS);
		}
	#ifndef __GNUC__
	} __except(1)
	#else
	} catch(...)
	#endif
		{/*An exception here should NEVER crash the app! */}

	//Paint
}


enum Bottoni
{
	SAVE,
	SEP1,
	SHUT2MIN,
	EXIT,
	SEP2,
	LOCKVIEW,
	SEP3,
	WHO,
	PDUMP,
	SEP4,
	RELACC,
	RELSCP,
	SEP5,
	DISC0,
	SEP6,
	GO,
	SEP7,
	BRKNEXT,
	SEP8,
	STEPIN,
	STEPOVER,
	STEPOUT,
	SEP9,
	TRACE,
	SEP10,
	SHWATCH,
	SHVAR,
	SHSTACK,
	SHFNS,
	SHBPS,
	SEP11,
	DELBPS,
	DELWATCH,
	SEP12,
	ABOUT,
	ENDOFBOTTONI
};

bool isCheck (int b)
{
	switch (b)
	{
	case LOCKVIEW:
	case TRACE:
//	case BRKNEXT:
		return true;
	default :
		return false;
	}
}


	
bool isSeparator (int b)
{
	switch (b)
	{
	case SEP1:
	case SEP2:
	case SEP3:
	case SEP4:
	case SEP5:
	case SEP6:
	case SEP7:
	case SEP8:
	case SEP9:
	case SEP10:
	case SEP11:
	case SEP12:
		return true;
	default :
		return false;
	}
}

int getCommand(int b)
{
	switch (b)
	{
	case SAVE: return ID_FILE_SAVEWORLD;
	case SHUT2MIN: return ID_ADMIN_SHUTDOWNIN10MINUTES;
	case EXIT: return IDM_EXIT;
	case LOCKVIEW: return ID_VIEW_LOCKSCREEN;
	case WHO: return ID_ADMIN_WHOISONLINE;
	case PDUMP: return ID_ADMIN_PERFORMANCEDUMP;
	case RELACC: return ID_RELOAD_RELOADACCOUNTS;
	case RELSCP: return ID_RELOAD_RELOADCLASSFILES;
	case DISC0: return ID_TOOLS_DISCONNECTACCOUNT0;
	case GO: return ID_DEBUG_GO;
	case BRKNEXT: return ID_DEBUG_BREAKONNEXTAMXCALL;
	case STEPIN: return ID_DEBUG_STEPIN;
	case STEPOVER: return ID_DEBUG_STEPOUT;
	case STEPOUT: return ID_DEBUG_STEPOUTCF;
	case TRACE: return ID_DEBUG_TRACENATIVEFUNCTIONS;
	case SHWATCH: return ID_DEBUG_SHOWWATCHES;
	case SHVAR: return ID_DEBUG_SHOWVARIABLES;
	case SHSTACK: return ID_DEBUG_SHOWCALLSTACK;
	case SHFNS: return ID_DEBUG_LISTFUNCTIONS;
	case SHBPS: return ID_DEBUG_LISTBREAKPOINTS;
	case DELBPS: return ID_DEBUG_REMOVEALLBREAKPOINTS;
	case DELWATCH: return ID_DEBUG_REMOVEALLWATCHES;
	case ABOUT: return IDM_ABOUT;
	default :
		return 0;
	}
}

bool isDebuggerBtn (int b)
{
	if (g_bDebuggingTriggers) {
		if (b==GO) return true;
		if (b==STEPOVER) return true;
		return false;
	}
	if ((b>=GO)&&(b<ABOUT)) return true;
	return false;
}

TBBUTTON array[50]; 
#define DBG(A) (((ServerScp::g_nLoadDebugger!=0)||(!isDebuggerBtn(A))) ? 0 : TBSTATE_HIDDEN)


void initToolbar(HWND hWnd, HINSTANCE hInstance)
{
   int BTNS = ENDOFBOTTONI;
   int sep = 0;

	for(int i=0;i<BTNS;i++) 
	{ 
        array[i].iBitmap=i-sep; 
        array[i].idCommand=getCommand(i); 
        array[i].fsState=DBG(i)|TBSTATE_ENABLED | ((i==TRACE) ? TBSTATE_CHECKED : 0); 
//		if (isDebuggerBtn(i)) array[i].fsState=TBSTATE_HIDDEN|((i==TRACE) ? TBSTATE_CHECKED : 0);
		if (!isSeparator(i))
			array[i].fsStyle= (isCheck(i) ? TBSTYLE_CHECK : TBSTYLE_BUTTON); 
		else {
			sep++;
			array[i].fsStyle= TBSTYLE_SEP; 
			array[i].iBitmap=0; 
			array[i].idCommand=0; 
			array[i].fsState=DBG(i);
		}
        array[i].dwData=0L; 
        array[i].iString=0; 
	} 
	g_hToolbar = CreateToolbarEx(hWnd,TBSTYLE_TOOLTIPS|WS_BORDER|WS_CHILD|WS_VISIBLE ,ID_TOOLBAR, 
        BTNS,hInstance,IDR_TOOLBAR,array,BTNS,16, 
        15,(BTNS+1)*16, 15, sizeof(TBBUTTON)); 
	
#ifdef USE_FLAT_TOOLBAR

	if (g_bUseFlatToolbar) {
		LONG uToolType = (UINT)GetWindowLong(g_hToolbar, GWL_STYLE);
		SetWindowLong(g_hToolbar, GWL_STYLE, uToolType|TBSTYLE_FLAT);
		InvalidateRect(g_hToolbar, NULL, TRUE);
	}
#endif




}



void ToolTip (WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *tem=(NMHDR FAR *)lParam; 
    if (tem->code!=TTN_NEEDTEXT)  return;
	LPTOOLTIPTEXT Tooltip= LPTOOLTIPTEXT(tem); 
    switch(wParam) 
	{
		case ID_FILE_SAVEWORLD: 
        Tooltip->lpszText="Saves the world file [Ctrl+S]"; 
        break; 
		case ID_ADMIN_SHUTDOWNIN10MINUTES: 
        Tooltip->lpszText="Shutsdown the server in 2 minutes, alerting online players [Ctrl+T]"; 
        break; 
		case IDM_EXIT: 
        Tooltip->lpszText="Shutsdown the server, saving the world [Ctrl+X]"; 
        break; 
		case ID_VIEW_LOCKSCREEN: 
        Tooltip->lpszText="Disables autoscroll [Ctrl+Alt+L]"; 
        break; 
		case ID_ADMIN_WHOISONLINE: 
        Tooltip->lpszText="Lists who is online [Ctrl+W]"; 
        break; 
		case ID_ADMIN_PERFORMANCEDUMP: 
        Tooltip->lpszText="Shows performances informations [Ctrl+P]"; 
        break; 
		case ID_RELOAD_RELOADACCOUNTS: 
        Tooltip->lpszText="Reloads player accounts [Ctrl+A]"; 
        break; 
		case ID_RELOAD_RELOADCLASSFILES: 
        Tooltip->lpszText="Reloads some script files -- Use with caution [Ctrl+R]"; 
        break; 
		case ID_TOOLS_DISCONNECTACCOUNT0: 
        Tooltip->lpszText="Disconnects the account 0 (admin) if connected. [Ctrl+Alt+D]"; 
        break; 
		case ID_DEBUG_GO: 
        Tooltip->lpszText="Go ahead executing AMX code [F5]"; 
        break; 
		case ID_DEBUG_BREAKONNEXTAMXCALL: 
        Tooltip->lpszText="Schedules a break on next AMX call [Ctrl+B]"; 
        break; 
		case ID_DEBUG_STEPIN: 
        Tooltip->lpszText="Steps the code entering any amx function [F11]"; 
        break; 
		case ID_DEBUG_STEPOUT: 
        Tooltip->lpszText="Steps the code, passing over amx functions [F10]"; 
        break; 
		case ID_DEBUG_STEPOUTCF: 
        Tooltip->lpszText="Go directly to the end of this function [Shift+F11]"; 
        break; 
		case ID_DEBUG_TRACENATIVEFUNCTIONS: 
        Tooltip->lpszText="Traces execution of native functions [F5]"; 
        break; 
		case ID_DEBUG_SHOWWATCHES: 
        Tooltip->lpszText="Shows watches [F7]"; 
        break; 
		case ID_DEBUG_SHOWVARIABLES: 
        Tooltip->lpszText="Shows in-scope variables [Shift+F7]"; 
        break; 
		case ID_DEBUG_SHOWCALLSTACK: 
        Tooltip->lpszText="Shows calls stack [F8]"; 
        break; 
		case ID_DEBUG_LISTFUNCTIONS: 
        Tooltip->lpszText="Lists amx functions [Shift+F8]"; 
        break; 
		case ID_DEBUG_LISTBREAKPOINTS: 
        Tooltip->lpszText="Lists current breakpoints [F9]"; 
        break; 
		case ID_DEBUG_REMOVEALLBREAKPOINTS: 
        Tooltip->lpszText="Removes all breakpoints [Ctrl+F9]"; 
        break; 
		case ID_DEBUG_REMOVEALLWATCHES: 
        Tooltip->lpszText="Removes all watches [Ctrl+F9]"; 
        break; 
		case IDM_ABOUT: 
        Tooltip->lpszText="Click here to know more about NoX-Wizard [Alt+?]"; 
        break; 
 		default: 
        Tooltip->lpszText="?"; 
        break; 
   } 


}



void mnuShow (int style)
{   int i;

	if (style==DISABLE_ALL) {
		EnableMenuItem(g_Menu,0,MF_GRAYED|MF_BYPOSITION);
		EnableMenuItem(g_Menu,2,MF_GRAYED|MF_BYPOSITION);
		EnableMenuItem(g_Menu,3,MF_GRAYED|MF_BYPOSITION);
		EnableMenuItem(g_Menu,4,MF_GRAYED|MF_BYPOSITION);
		EnableMenuItem(g_Menu,MNUDEBUG,MF_GRAYED|MF_BYPOSITION);
		EnableMenuItem(g_Menu,ID_HELP_SHOWKEYBOARDCOMMANDS,MF_GRAYED|MF_BYCOMMAND);
		for (i=0; i<ABOUT; i++) {
			if(i!=LOCKVIEW) 
			SendMessage(g_hToolbar,TB_SETSTATE, array[i].idCommand,
				MAKELONG(DBG(i)|0,0)); 
		}
	}

	if (ServerScp::g_nLoadDebugger) {
		enableMenuAndBtnID(ID_DEBUG_BREAKONNEXTAMXCALL, true);
		enableMenuAndBtnID(ID_DEBUG_BREAKONNEXTLEGACYTRIGGER, true);
		enableMenuAndBtnID(ID_DEBUG_TRACENATIVEFUNCTIONS, true);
	} else {
		DeleteMenu(g_Menu, 5, MF_BYPOSITION);
	}


	RedrawWindow(g_HWnd,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
}


void enableMenuAndBtnID(int ID, bool enabled)
{
	EnableMenuItem(g_Menu,ID,(enabled ? MF_ENABLED : MF_GRAYED)|MF_BYCOMMAND);
	SendMessage(g_hToolbar,TB_SETSTATE, ID,
		MAKELONG((enabled ? TBSTATE_ENABLED : 0),0)); 
}

void enableBtn(int i, bool enabled)
{
	SendMessage(g_hToolbar,TB_SETSTATE, array[i].idCommand,
		MAKELONG((enabled ? TBSTATE_ENABLED : 0),0)); 
}

void enableAllMenu (int ID, bool enabled)
{	int i;
	EnableMenuItem(g_Menu,ID,(enabled ? MF_ENABLED : MF_GRAYED)|MF_BYPOSITION);

	switch (ID) {
	case 0:
		enableBtn(SAVE, enabled);
		enableBtn(SHUT2MIN, enabled);
		enableBtn(EXIT, enabled);
		break;
	case 2:
		enableBtn(WHO, enabled);
		enableBtn(PDUMP, enabled);
		break;
	case 3:
		enableBtn(RELACC, enabled);
		enableBtn(RELSCP, enabled);
		break;
	case 4:
		enableBtn(DISC0, enabled);
		break;
	case MNUDEBUG:
		for (i=0; i<ENDOFBOTTONI; i++)
		{
			if (isDebuggerBtn(i)) enableBtn(i, enabled);
		}
		break;
	}
}





















#endif




