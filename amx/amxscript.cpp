/*!
 ***********************************************************************************
 *  file    : amxscript.cpp
 *
 *  Project : Nox-Wizard
 *
 *  Author  : 
 *
 *  Purpose : Implementation of methods Amx for Script
 *
 ***********************************************************************************
 *//*
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW)		   [http://www.noxwizard.com]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums or mail staff@noxwizard.com  ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    CHANGELOG:
    -----------------------------------------------------------------------------
    DATE					DEVELOPER				DESCRIPTION
    -----------------------------------------------------------------------------

 ***********************************************************************************
 */

#ifdef __BEOS__
#ifndef __unix__
#define __unix__
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>     /* for memset() (on some compilers) */
#include <time.h>
#include <assert.h>
#ifdef WIN32
#include <windows.h>
#endif
#define CONSOLE
//#include "amx_api.h"
#include "console.h"

// MUST BE *AFTER* ANY OTHER INCLUDE!!!
#include "amx.h"
#include "amx_comp.h"
#include "common_libs.h"
#include "srvparms.h"
#include "amx_api.h"
#include "constants.h"
#include "target.h"
#include "globals.h"
#include "inlines.h"



extern "C" int g_nTraceMode; //by xan -> trace mode


extern "C" {
	int AMXAPI amx_InternalDebugProc(AMX *amx);
}


static char g_strAmxSpeeches[256][256];
static int g_nAmxSpeechPtr = 0;



#include "amxscript.h"

extern AMX_NATIVE_INFO nxw_API[];

extern "C" {
    extern AMX_NATIVE_INFO console_Natives[];
	extern AMX_NATIVE_INFO core_Natives[];
	void core_Init(void);   /* two functions from AMX_CORE.C */
	void core_Exit(void);
}

AmxProgram* AmxFunction::g_prgOverride = NULL;
extern int g_nCurrentSocket;
static int s_nTmpEnableAMXScripts = 1;

int g_nMoment=0;

#define CHECKAMXV { if ((ServerScp::g_nEnableAMXScripts==0)||(s_nTmpEnableAMXScripts==0)) return; }
#define CHECKAMX { if ((ServerScp::g_nEnableAMXScripts==0)||(s_nTmpEnableAMXScripts==0)) return 0; }



int AMXAPI amx_SetStringUnicode( cell *dest, wstring& source )
{
	wstring::iterator iter( source.begin() ), end( source.end() );

	for( int i=0; iter!=end; ++iter, ++i ) {
		dest[i]=(*iter);
	}
    /* On Big Endian machines, the characters are well aligned in the
     * cells; on Little Endian machines, we must swap all cells.
    */
    if (!amx_getLittleEndian()) {
		int len = source.size();
		while (len>=0)
			swapcell((ucell *)&dest[len--]);
    }

	return AMX_ERR_NONE;
}

int AMXAPI amx_GetStringUnicode( wstring& dest, cell* source )
{

	dest.clear();
	cell temp;
	int i=0;
	while( (temp=source[i++])!=0 ) {
	    if (!amx_getLittleEndian()) 
			swapcell( (ucell *)&temp );
		dest+=static_cast<wchar_t>(temp);
	}

	return AMX_ERR_NONE;
}





/*!
\brief Uppercases a string
\author Xanathar
\return static
\param s the string
*/
static void strupcase (char *s)
{
	int i, ln= strlen(s);
	for(i=0; i<ln; i++)
		if ((s[i]>='a')&&(s[i]<='z')) s[i] = static_cast<char>(s[i] - 'a' + 'A');
}


/*!
\brief Lowercases a string
\author Xanathar
\return static
\param s the string
*/
static void strlocase (char *s)
{
	int i, ln= strlen(s);
	for(i=0; i<ln; i++)
		if ((s[i]>='A')&&(s[i]<='Z')) s[i] = static_cast<char>(s[i] - 'A' + 'a');
}




/*!
\brief signal handler for amx debug interfaces
\author Xanathar
\return static
\param sig signal recvd
*/
int abortflagged = 0;
static void sigabort(int sig)
{
  abortflagged=1;
  signal(sig,sigabort); /* re-install the signal handler */
}


/*!
\brief abort procedure for amx
\author Xanathar
\return static
\param amx -- as default --
*/
static int AMXAPI amx_AbortProc(AMX *amx)
{
  switch (amx->dbgcode) {
	  case DBG_INIT:
		return AMX_ERR_NONE;
	  case DBG_LINE:
		return abortflagged ? AMX_ERR_EXIT : AMX_ERR_NONE;
  default:
		return AMX_ERR_DEBUG;
  }
}

/********************************************************************************
 HERE START XAN AMX CODE WITH A TINY NICE C++ CLASS :)
 ********************************************************************************/
//Declares the Global Abstract Machine


/*!
\brief Loads an amx program in memory
\author Xanathar
\param filename *.amx file name
*/
#pragma pack(1)
void AmxProgram::Load(const char *filename)
{
  FILE *fp; AMX_HEADER hdr; void *program;

  CHECKAMXV;

  m_AMX = new __amx;

  if ( (fp = fopen( filename, "rb" )) != NULL )
  {
    fread(&hdr, sizeof hdr, 1, fp);
    if ( (program = new char[( (int)hdr.stp )]) != NULL )
    {
      rewind( fp );
      fread( program, 1, (int)hdr.size, fp );
      fclose( fp );
	  m_nSize = hdr.stp;
	  m_ptrCode = program;

//#define FIND_NATIVE_NOT_FOUND
#ifdef FIND_NATIVE_NOT_FOUND
	  if( !findNativeNotFound(program) ) { //native not found
		  ConOut("[FAIL]\n");
	  }
#else
	  _init(program);
#endif
	  core_Init();
	  ConOut("[ OK ]\n");
	  return;
    }
  }
  m_ptrCode = NULL; m_nSize = 0;
  ConOut("[FAIL]\n");
}


/*!
\brief do all needed initializations to have program running
\author Xanathar
\return bool
\param program -> -- as default --
*/
bool AmxProgram::_init (void *program)
{
  AMX *amx = m_AMX;

  CHECKAMX;

  memset(amx, 0, sizeof *amx);

  if (ServerScp::g_nDeamonMode!=0) ServerScp::g_nLoadDebugger = 0;

  if (ServerScp::g_nLoadDebugger==0) {
	amx_SetDebugHook(amx, amx_AbortProc);
  } else {
	amx_SetDebugHook(amx, amx_InternalDebugProc);
  }

  if ( amx_Init( amx, program ) != AMX_ERR_NONE )
  {
     ConOut ("AMX Init FAILED!\n"); return false;
  }
  signal(SIGINT,sigabort);
  amx_Register(amx, console_Natives, -1);
  amx_Register(amx, nxw_API, -1);
  amx_Register(amx, core_Natives, -1);
  return true;
}

/*!
\brief Check natives and find what function is not present
\author Endymion
*/
bool AmxProgram::findNativeNotFound( void* program )
{

	
  AMX *amx = m_AMX;

  CHECKAMX;

  memset(amx, 0, sizeof *amx);

  if (ServerScp::g_nDeamonMode!=0) ServerScp::g_nLoadDebugger = 0;

  if (ServerScp::g_nLoadDebugger==0) {
	amx_SetDebugHook(amx, amx_AbortProc);
  } else {
	amx_SetDebugHook(amx, amx_InternalDebugProc);
  }

  if ( amx_Init( amx, program ) != AMX_ERR_NONE )
  {
     ConOut ("AMX Init FAILED!\n"); return false;
  }

  AMX_FUNCSTUB *func;
  AMX_HEADER *hdr;
  int i,numnatives,err;
  AMX_NATIVE funcptr;
  AMX_NATIVE_INFO *list;

  hdr=(AMX_HEADER *)amx->base;
  assert(hdr!=NULL);
  assert(hdr->natives<=hdr->libraries);
  
  numnatives=(int)(((*hdr).libraries - (*hdr).natives)/sizeof(AMX_FUNCSTUB));

  err=AMX_ERR_NONE;
  func=(AMX_FUNCSTUB *)(amx->base+(int)hdr->natives);
  for (i=0; i<numnatives; i++) {
    if (func->address==0) {
      /* this function is not yet located */
		bool found=false;
		for( int l=0; (l<=3) && !found; ++l ) {
			
			switch( l ) {
				case 0 : list = console_Natives; break;
				case 1 : list = nxw_API; break;
				case 2 : list = core_Natives; break;
				case 3 :
					ConOut( "\n[ERROR] native function not found [ %s ]   [ERROR]\n", func->name );
					return false;
			}						

			funcptr= NULL;
			for (int n=0; list[n].name!=NULL; n++)
				if (strcmp(func->name,list[n].name)==0)
					funcptr= list[n].func;
			found = (funcptr!=NULL);
			if (funcptr!=NULL)
				func->address=(cell)funcptr;
			else
				err=AMX_ERR_NOTFOUND;
		}
    } /* if */
    func++;
  } /* for */
  return true;
}


/*!
\brief calls an amx function with 6 integer parameters
\author Luxor
\return cell
\param idx function index (as returned by funcidx())
\param param1 parameter 1
\param param2 parameter 2
\param param3 parameter 3
\param param4 parameter 4
\param param5 parameter 5
\param param6 parameter 6
*/
cell AmxProgram::CallFn (int idx, int param1, int param2, int param3, int param4, int param5, int param6)
{
	CHECKAMX;
	try {
	  cell ret = 0;
	  cell par1 = param1;
	  cell par2 = param2;
	  cell par3 = param3;
	  cell par4 = param4;
	  cell par5 = param5;
   	  cell par6 = param6;
	  int err;

	  if (m_nSize==0) return -2;

	  err = amx_Exec(m_AMX, &ret, idx, 6, par1, par2, par3, par4, par5, par6);
  	  while (err == AMX_ERR_SLEEP) err = amx_Exec(m_AMX, &ret, AMX_EXEC_CONT, 6, par1, par2, par3, par4, par5, par6);

	  return ret;

	} catch(...) {
		ConOut("Exception Handled during AMX Script Execution : skipping script\n");
		return -1;
	}
}

/*!
\brief calls an amx function with 7 integer parameters
\author Endymion
\return cell
\param idx function index (as returned by funcidx())
\param param1 parameter 1
\param param2 parameter 2
\param param3 parameter 3
\param param4 parameter 4
\param param5 parameter 5
\param param6 parameter 6
\param param7 parameter 7
*/
cell AmxProgram::CallFn (int idx, int param1, int param2, int param3, int param4, int param5, int param6, int param7)
{
	CHECKAMX;
	try {
	  cell ret = 0;
	  cell par1 = param1;
	  cell par2 = param2;
	  cell par3 = param3;
	  cell par4 = param4;
	  cell par5 = param5;
   	  cell par6 = param6;
   	  cell par7 = param7;
	  int err;

	  if (m_nSize==0) return -2;

	  err = amx_Exec(m_AMX, &ret, idx, 7, par1, par2, par3, par4, par5, par6, par7);
  	  while (err == AMX_ERR_SLEEP) err = amx_Exec(m_AMX, &ret, AMX_EXEC_CONT, 7, par1, par2, par3, par4, par5, par6, par7);

	  return ret;

	} catch(...) {
		ConOut("Exception Handled during AMX Script Execution : skipping script\n");
		return -1;
	}
}

/*!
\brief calls an amx function with 7 integer parameters
\author Endymion
\return cell
\param idx function index (as returned by funcidx())
\param param1 parameter 1
\param param2 parameter 2
\param param3 parameter 3
\param param4 parameter 4
\param param5 parameter 5
\param param6 parameter 6
\param param7 parameter 7
\param param8 parameter 8
*/
cell AmxProgram::CallFn (int idx, int param1, int param2, int param3, int param4, int param5, int param6, int param7, int param8)
{
	CHECKAMX;
	try {
	  cell ret = 0;
	  cell par1 = param1;
	  cell par2 = param2;
	  cell par3 = param3;
	  cell par4 = param4;
	  cell par5 = param5;
   	  cell par6 = param6;
   	  cell par7 = param7;
   	  cell par8 = param8;
	  int err;

	  if (m_nSize==0) return -2;

	  err = amx_Exec(m_AMX, &ret, idx, 8, par1, par2, par3, par4, par5, par6, par7, par8);
  	  while (err == AMX_ERR_SLEEP) err = amx_Exec(m_AMX, &ret, AMX_EXEC_CONT, 8, par1, par2, par3, par4, par5, par6, par7, par8);

	  return ret;

	} catch(...) {
		ConOut("Exception Handled during AMX Script Execution : skipping script\n");
		return -1;
	}
}

/*!
\brief calls an amx function with 5 integer parameters
\author Luxor
\return cell
\param idx function index (as returned by funcidx())
\param param1 parameter 1
\param param2 parameter 2
\param param3 parameter 3
\param param4 parameter 4
\param param5 parameter 5
*/
cell AmxProgram::CallFn (int idx, int param1, int param2, int param3, int param4, int param5)
{
	CHECKAMX;
	try {
	  cell ret = 0;
	  cell par1 = param1;
	  cell par2 = param2;
	  cell par3 = param3;
	  cell par4 = param4;
      cell par5 = param5;
	  int err;

	  if (m_nSize==0) return -2;

	  err = amx_Exec(m_AMX, &ret, idx, 5, par1, par2, par3, par4, par5);
  	  while (err == AMX_ERR_SLEEP) err = amx_Exec(m_AMX, &ret, AMX_EXEC_CONT, 5, par1, par2, par3, par4, par5);

	  return ret;

	} catch(...) {
		ConOut("Exception Handled during AMX Script Execution : skipping script\n");
		return -1;
	}
}


/*!
\brief calls an amx function with 4 integer parameters
\author Xanathar
\return cell
\param idx function index (as returned by funcidx())
\param param1 parameter 1
\param param2 parameter 2
\param param3 parameter 3
\param param4 parameter 4
*/
cell AmxProgram::CallFn (int idx, int param1, int param2, int param3, int param4)
{
	CHECKAMX;
	try {
	  cell ret = 0;
	  cell par1 = param1;
	  cell par2 = param2;
	  cell par3 = param3;
	  cell par4 = param4;
	  int err;

	  if (m_nSize==0) return -2;

	  err = amx_Exec(m_AMX, &ret, idx, 4, par1, par2, par3, par4);
  	  while (err == AMX_ERR_SLEEP) err = amx_Exec(m_AMX, &ret, AMX_EXEC_CONT, 4, par1, par2, par3, par4);

	  return ret;

	} catch(...) {
		ConOut("Exception Handled during AMX Script Execution : skipping script\n");
		return -1;
	}
}

/*!
\brief calls an amx function with 3 integer parameters
\author Endymion
\return cell
\param idx function index (as returned by funcidx())
\param param1 parameter 1
\param param2 parameter 2
\param param3 parameter 3
*/
cell AmxProgram::CallFn (int idx, int param1, int param2, int param3 )
{
	CHECKAMX;
	try {
	  cell ret = 0;
	  cell par1 = param1;
	  cell par2 = param2;
	  cell par3 = param3;
	  int err;

	  if (m_nSize==0) return -2;

	  err = amx_Exec(m_AMX, &ret, idx, 4, par1, par2, par3 );
  	  while (err == AMX_ERR_SLEEP) err = amx_Exec(m_AMX, &ret, AMX_EXEC_CONT, 4, par1, par2, par3 );

	  return ret;

	} catch(...) {
		ConOut("Exception Handled during AMX Script Execution : skipping script\n");
		return -1;
	}
}



/*!
\brief calls an amx function with 2 integer parameters
\author Xanathar
\return cell
\param idx function index (as returned by funcidx())
\param param1 parameter 1
\param param2 parameter 2
*/
cell AmxProgram::CallFn (int idx, int param1, int param2)
{
	CHECKAMX;
	try {
	  cell ret = 0;
	  cell par1 = param1;
	  cell par2 = param2;
	  int err;

	  if (m_nSize==0) return -2;

	  err = amx_Exec(m_AMX, &ret, idx, 2, par1, par2);
  	  while (err == AMX_ERR_SLEEP) err = amx_Exec(m_AMX, &ret, AMX_EXEC_CONT, 2, par1, par2);

	  return ret;

	} catch(...) {
		ConOut("Exception Handled during AMX Script Execution : skipping script\n");
		return -1;
	}
}


/*!
\brief calls an amx function with 1 integer parameter
\author Xanathar
\param idx function index (as returned by funcidx())
\param param parameter 1
*/
cell AmxProgram::CallFn (int idx, int param)
{
	CHECKAMX;

	try {
	  cell ret = 0;
	  cell par = param;
	  int err;

	  if (m_nSize==0) return -2;

	  err = amx_Exec(m_AMX, &ret, idx, 1, par);
  	  while (err == AMX_ERR_SLEEP) err = amx_Exec(m_AMX, &ret, AMX_EXEC_CONT, 1, par);

	  return ret;

	} catch(...) {
		ConOut("Exception Handled during AMX Script Execution : skipping script\n");
		return -1;
	}
}

/*!
\brief calls an amx function with no parameters
\author Xanathar
\param fn function index (as returned by funcidx())
*/
cell AmxProgram::CallFn (int fn)
{
  	  CHECKAMX;
	  if (m_nSize==0) return -2;
	  return CallFn(fn, 0);
}



/*!
\brief calls an amx function with no parameters, by name
\author Xanathar
\return cell
\param fn name of function to be called, if invalid it calls main()
*/
cell AmxProgram::CallFn (const char *fn)
{
	CHECKAMX;

	try {
	  int err = -1, idx = AMX_EXEC_MAIN;
	  if (m_nSize==0) return -2;

	  if (fn!=NULL) err = amx_FindPublic (m_AMX, (char*)fn, &idx);
	  if ((err!=0)||(fn==NULL)) idx = AMX_EXEC_MAIN;

	  return CallFn(idx);

	} catch(...) {
		ConOut("Exception Handled during AMX Script Execution : skipping script\n");
		return -1;
	}
}



/*!
\brief gets the function index from a function name (same as funcidx()) in Small
\author Xanathar
\return int
\param fn function name
*/
int AmxProgram::getFnOrdinal (char *fn)
{
	
	int idx = AMX_EXEC_MAIN, err = 0;
	CHECKAMX;

	if (fn==NULL) 
		return AMX_EXEC_MAIN;
	else
		err = amx_FindPublic (m_AMX, fn, &idx);
	  
	if(err!=AMX_ERR_NONE) 
		return INVALID;
	else
		return idx;
}



/******************************************************************************************
   AMX Scripting Stuff - Override part of the things :D
   by Xanathar, June 2001
 *****************************************************************************************/

bool g_bByPass = false;



/*!
\brief Constructor
\author Xanathar
*/
AmxOverride::AmxOverride()
{
	m_mode = AMX_NONE;
	m_function = NULL;
}


/*!
\brief Destructor
\author Xanathar
*/
AmxOverride::~AmxOverride()
{
	if (m_function!=NULL) delete m_function;
}


/*!
\brief executes an override - no sockets passed
\author Xanathar
\return bool
\param moment the phase of this override (BEFORE/AFTER)
*/
bool AmxOverride::Exec (int moment)
{
	CHECKAMX;
	return Exec(moment, -1);
}


/*!
\brief executes an override
\author Xanathar
\return bool
\param moment the phase of this override (BEFORE/AFTER)
\param socket the socket which directly or indirectly invoked this override
*/
bool AmxOverride::Exec (int moment, int socket)
{
	CHECKAMX;
	bool bypassmode = (m_mode&0x04) ? true : false;

	if (AmxFunction::g_prgOverride==NULL) return false;
	if (m_mode==AMX_NONE) return false;
	if (m_mode==AMX_SKIP) return true;
	if ((moment&m_mode)==0) return false;

	g_bByPass = false;

	g_nMoment = moment;

	g_nCurrentSocket = socket;
	AmxFunction::g_prgOverride->CallFn(m_ordinal, socket);
	g_nCurrentSocket = -1;

	return ((g_bByPass) || (bypassmode));
}

/*!
\brief executes an target override
\author Xanathar
\return bool
\param moment the phase of this override (BEFORE/AFTER)
\param socket the socket which directly or indirectly invoked this override
*/
bool AmxOverride::ExecTarget(int moment, int socket)
{
	CHECKAMX;
	bool bypassmode = (m_mode&0x04) ? true : false;

	if (AmxFunction::g_prgOverride==NULL) return false;
	if (m_mode==AMX_NONE) return false;
	if (m_mode==AMX_SKIP) return true;
	if ((moment&m_mode)==0) return false;

	g_bByPass = false;

	g_nMoment = moment;

	P_TARGET targ_amx =clientInfo[socket]->getTarget();
	if( targ_amx==NULL ) {
		return false;
	}

	SERIAL obj_serial = targ_amx->getClicked();
	if( isCharSerial( obj_serial) ) {
		obj_serial = ISVALIDPC( MAKE_CHAR_REF( obj_serial) )? obj_serial : INVALID;
	} else if( isItemSerial( obj_serial ) ) {
		obj_serial = ISVALIDPI( MAKE_ITEM_REF( obj_serial) )? obj_serial : INVALID;
	}
	else 
		obj_serial = INVALID;
		
	g_nCurrentSocket = socket;
	AmxFunction::g_prgOverride->CallFn(m_ordinal, socket, targ_amx->serial, obj_serial );
	g_nCurrentSocket = INVALID;

	return ((g_bByPass) || (bypassmode));

} 

/*!
\brief converts a mode to a string
\author Xanathar
\return static
\param a the moment
*/
static char *strMode(int a)
{
	if (a == AMX_BEFORE) return "\"Before\"";
	if (a == AMX_BYPASS) return "\"Bypass\"";
	if (a == AMX_AFTER) return "\"After\"";
	if (a == AMX_FRAME) return "\"Frame\"";
	return "\"Unknown\"";
}


/*!
\brief parses a line of override.scp
\author Xanathar
\param line the line to be parsed
*/
void AmxOverride::Parse(char *line)
{
	char *b = line;
	int i;
	int ln = strlen(line);
	char *file, *fn;
	int mode = AMX_NONE;

	CHECKAMXV;

	try {
		// skips the first part
		for (i = 0; i<ln; i++){
			if (line[i]=='=') {
				b = line+i;
				break;
		}
		}


		fn = b+1;
		ln = strlen(fn);

		// search for function name
		for (i = 0; i<ln; i++){
			if (fn[i]==',') {
				fn[i]='\0';
				b = fn+i+1;
				break;
		}
		}

		// understand the override mode :)
		strupcase(b);

		if ((b[0]=='B')&&(b[1]=='E')) mode = AMX_BEFORE;
		if ((b[0]=='A')&&(b[1]=='F')) mode = AMX_AFTER;
		if ((b[0]=='F')&&(b[1]=='R')) mode = AMX_FRAME;
		if ((b[0]=='B')&&(b[1]=='Y')) mode = AMX_BYPASS;
		if ((b[0]=='S')&&(b[1]=='K')) mode = AMX_SKIP;

		m_function = new char[strlen(fn)+4];
		strcpy(m_function, fn);
		if (mode!=AMX_SKIP)
			 ConOut("    Installing %s override for %s()...", strMode(mode), fn);
		else ConOut("    Installing Skip override...[ OK ]\n");

		m_mode = mode;
		if (mode!=AMX_SKIP) {
			m_ordinal = AmxFunction::g_prgOverride->getFnOrdinal(m_function);
			if (m_ordinal==-3) ConOut("[FAIL]\n");
				else ConOut("[ OK ]\n");
		}


	} catch(...) {
		ConOut("Error parsing override.scp, line : %s", line);
		file = NULL;
		m_function = NULL;
		m_mode = AMX_NONE;
	}
}


//---------------------------------------------------------------------

AmxOverride g_Scripts[AMX_MAXTYPE][AMX_MAXNUMBER];

#define ISNUMERICTYPE(A) ((A<7)&&(A!=AMXT_SPEECH))

#define FAIL { ConOut ("[FAIL]\n"); return; }

/*!
\brief check the integrity of exported natives apis
\author Xanathar
*/
void check_Natives ()
{
	char *str;
	static char buffer[30];
	int i = 0;
	bool bFailed = false;

	for (str = nxw_API[0].name; str!=NULL; str = nxw_API[++i].name)
	{
		if (strlen(str)>19) {
			if (!bFailed) ConOut("[FAIL]\n");
			bFailed = true;
			ConOut("\n%s() -> exceed 19 chars name len!", str);
		}
	}

	if (bFailed) {
		#ifndef WIN32
			ConOut("\n\nPress <return> to exit...");
			if (ServerScp::g_nDeamonMode==0) {
				fgets(buffer, 29, stdin);
			}
		#else
		if (ServerScp::g_nDeamonMode==0) {
			MessageBox(NULL, "API Callback name lenght exceeded. Press OK to abort.", "NoX-Wizard Internal Error", MB_ICONSTOP);
		}
		#endif
		exit(1);
	}
}




/*!
\brief Loads override.amx (eventually building it) and override.scp
\author Xanathar
*/
void LoadOverrides (void)
{
	int curType = AMXT_SPECIALS;
	char buf[1024], *b = NULL;
	int id, ln, count=0, i;
	char *argv[5];

	ConOut("\n\nPerforming natives internal check... ");
	check_Natives();
	ConOut("[ OK ]\n");


	argv[0] = "noxwizard.internal.compiler";
	argv[1] = new char[50];
	argv[2] = new char[50];
	argv[3] = new char[50];
	argv[4] = new char[50];

	g_nTraceMode = ServerScp::g_nLoadDebugger;

	strcpy(argv[1], "small-scripts/override.sma");
	strcpy(argv[2], "-osmall-scripts/override.amx");
	if (ServerScp::g_nLoadDebugger==0) {
		strcpy(argv[3], "-d1");
	} else {
		strcpy(argv[3], "-d3");
	}
	strcpy(argv[4], "-i./small-scripts/include");

	//strcpy(argv[4], "-t0");


	if (ServerScp::g_bEnableInternalCompiler)
	{
		ConOut("\nCompiling override.sma...\n");
		int ret = compiler_main(5, (char **)argv);

		if (ret>1) {
			if (ServerScp::g_nDeamonMode!=0) exit(1);

			ConOut("\n\n");
			ConOut("Compilation errors occured!!\n");
			#ifndef WIN32
				ConOut("Type C to continue or anything else to abort.\n");
				fgets(buf, 1000 ,stdin);
				if ((buf[0]!='C')&&(buf[0]!='c')) exit(1);
				s_nTmpEnableAMXScripts = 0;
				return;
			#endif
			#ifdef WIN32
				int ret = MessageBox(NULL, "Compilation errors have occurred.\nIf you continue, AMX scripts and programs will be disabled.\nDo you want to continue?", "Compilation errors", MB_YESNO|MB_SETFOREGROUND|MB_TOPMOST|MB_ICONERROR);
				if (ret!=IDYES) exit(1);
				s_nTmpEnableAMXScripts = 0;
				return;
			#endif

		}
	}

	delete argv[1];
	delete argv[2];
	delete argv[3];
	delete argv[4];

	ConOut("Loading override.amx...");
	AmxFunction::g_prgOverride = new AmxProgram("small-scripts/override.amx");

	ConOut("Loading overrides...\n");



	try {
		FILE *F = fopen("small-scripts/override.scp", "rt");

		if (F==NULL) FAIL;

		while (!feof(F))
		{
			id = -1;
			fgets(buf,1020,F);
			ln = strlen(buf);

			if(buf[0]=='[') {
				strlocase(buf);
				for (int i = 0; i<ln; i++) {
					if (buf[i]==']') { buf[i] = '\0'; break;}
				}

				if (!strcmp(buf,"[skills")) curType = AMXT_SKILLS;
				if (!strcmp(buf,"[skilltargets")) curType = AMXT_SKITARGS;
				if (!strcmp(buf,"[speech")) curType = AMXT_SPEECH;
				if (!strcmp(buf,"[triggers")) curType = AMXT_TRIGGERS;
				if (!strcmp(buf,"[magicselect")) curType = AMXT_MAGICSELECT;
				if (!strcmp(buf,"[magiccast")) curType = AMXT_MAGICCAST;
				if (!strcmp(buf,"[special")) curType = AMXT_SPECIALS;
				if (!strcmp(buf,"[networkrcv")) curType = AMXT_NETRCV;
				//if (!strcmp(buf,"[gmcommands")) curType = AMXT_GMCMD; //not used now
				continue;
			}

			if ((buf[0]==' ')||(buf[0]=='#')||(buf[0]=='/')||(buf[0]=='\n')) continue;
			if ((buf[0]=='{')||(buf[0]=='.')||(buf[0]==';')||(buf[0]=='\r')) continue;

			// if here : we are parsing an override :)

			for (i = 0; i<ln; i++) {
				if (buf[i]=='=') {
						buf[i] = '\0';
						b = buf+i;
						break;
				}
				}


			if (ISNUMERICTYPE(curType))	
				id = atoi(buf);
			else 
				if (curType==AMXT_SPECIALS)
				{
					if (!strcmp(buf,"OnStart")) id = 0;
					if (!strcmp(buf,"OnExit")) id = 1;
					if (!strcmp(buf,"OnLoop")) id = 2;
					if (!strcmp(buf,"InitChar")) id = 3;
					if (!strcmp(buf,"StartChar")) id = 4;
					if (!strcmp(buf,"AdvanceSkill")) id = 5;
					if (!strcmp(buf,"AdvanceStat")) id = 6;
					if (!strcmp(buf,"VerifyItemUsability")) id = 7;
					if (!strcmp(buf,"OnLogOut")) id = 8;
				}

			if (curType==AMXT_SPEECH) {
				/** all speeches containing the given phrase will kick up the script */
				for (i = 0; i<(signed)strlen(buf); i++) {
					if ((buf[i]>='a')&&(buf[i]<='z')) buf[i] += static_cast<char>('A'-'a');
				}
				strcpy(g_strAmxSpeeches[g_nAmxSpeechPtr], buf);
				id = g_nAmxSpeechPtr;
				g_nAmxSpeechPtr++;
			}

			if (b==NULL) return; //xan

			*b = '=';
			if (id>=0) {
				g_Scripts[curType][id].Parse(buf);
				count++;
			}
			else {
				ConOut("\nError parsing %s", buf);
			}
		}

	} catch(...) { ConOut("[FATAL!]\n"); return ; }

	ConOut("%d overrides loaded\n", count);



}


/*!
\brief checks speech from socket s for override invoking
\author Xanathar
\param chr player who spoken
\param speech speech of socket s
*/
void checkAmxSpeech( SERIAL chr, char *speech)
{
	if( ServerScp::g_css_override_case_sensitive==0 )
		strupr( speech );
	for (int i=0; i<g_nAmxSpeechPtr; i++)
	{
		if (strstr(speech,g_strAmxSpeeches[i])!=NULL)
		{
			extern char script1[512];
			extern char script2[512];
			strcpy( script1, g_strAmxSpeeches[i] );
			strcpy( script2, speech );
			AMXEXECSV( chr,AMXT_SPEECH, i, AMX_BEFORE);
		}
	}
}


std::string InvalidFunction( "InvalidFunction" );

/*
\brief Constructor
\author Endymion
\param funName the function name
\note if funName is NULL, main is called
*/
AmxFunction::AmxFunction( char* funName )
{
	function = g_prgOverride->getFnOrdinal( funName );
	funcName.copy( funName, strlen(funName) );
}

AmxFunction::AmxFunction( FUNCIDX fn )
{
	function = fn;
}

AmxFunction::AmxFunction( class AmxFunction& af )
{
	function = af.function;
	funcName = af.funcName;
}

/*
\brief Get function name
\author Endymion
\return the function name or "" if invalid func
*/
char* AmxFunction::getFuncName()
{
    return ( function != INVALID ) ? (char*)funcName.c_str() : (char*)InvalidFunction.c_str();
}

FUNCIDX AmxFunction::getFuncIdx()
{
	return function;
}

void AmxFunction::copy( class AmxFunction& af )
{
	this->funcName=af.funcName;
	this->function=af.function;
}


/*
\brief Call the function with no params
\author Endymion
\return the function return
*/
cell AmxFunction::Call( )
{
	return g_prgOverride->CallFn( function );
}

/*
\brief Call the function wih 1 param
\author Endymion
\return the function return
\param param the param
*/
cell AmxFunction::Call( int param )
{
	return g_prgOverride->CallFn( function, param );
}

/*
\brief Call the function wih 2 params
\author Endymion
\return the function return
\param param1 the 1 param
\param param2 the 2 param
*/
cell AmxFunction::Call( int param1, int param2 )
{
	return g_prgOverride->CallFn( function, param1, param2 );
}


/*
\brief Call the function wih 3 params
\author Endymion
\return the function return
\param param1 the 1 param
\param param2 the 2 param
\param param3 the 3 param
*/
cell AmxFunction::Call( int param1, int param2, int param3 )
{
	return g_prgOverride->CallFn( function, param1, param2, param3 );
}


/*
\brief Call the function wih 4 params
\author Endymion
\return the function return
\param param1 the 1 param
\param param2 the 2 param
\param param3 the 3 param
\param param4 the 4 param
*/
cell AmxFunction::Call( int param1, int param2, int param3, int param4 )
{
	return g_prgOverride->CallFn( function, param1, param2, param3, param4 );
}


/*
\brief Call the function wih 5 params
\author Endymion
\return the function return
\param param1 the 1 param
\param param2 the 2 param
\param param3 the 3 param
\param param4 the 4 param
\param param5 the 5 param
*/
cell AmxFunction::Call( int param1, int param2, int param3, int param4, int param5 )
{
	return g_prgOverride->CallFn( function, param1, param2, param3, param4, param5 );
}


/*
\brief Call the function wih 6 params
\author Endymion
\return the function return
\param param1 the 1 param
\param param2 the 2 param
\param param3 the 3 param
\param param4 the 4 param
\param param5 the 5 param
\param param6 the 6 param
*/
cell AmxFunction::Call( int param1, int param2, int param3, int param4, int param5, int param6 )
{
	return g_prgOverride->CallFn( function, param1, param2, param3, param4, param5, param6 );
}

/*
\brief Call the function wih 7 params
\author Endymion
\return the function return
\param param1 the 1 param
\param param2 the 2 param
\param param3 the 3 param
\param param4 the 4 param
\param param5 the 5 param
\param param6 the 6 param
\param param7 the 7 param
*/
cell AmxFunction::Call( int param1, int param2, int param3, int param4, int param5, int param6, int params7 )
{
	return g_prgOverride->CallFn( function, param1, param2, param3, param4, param5, param6, params7 );
}

/*
\brief Call the function wih 8 params
\author Endymion
\return the function return
\param param1 the 1 param
\param param2 the 2 param
\param param3 the 3 param
\param param4 the 4 param
\param param5 the 5 param
\param param6 the 6 param
\param param7 the 7 param
\param param8 the 8 param
*/
cell AmxFunction::Call( int param1, int param2, int param3, int param4, int param5, int param6, int params7, int params8 )
{
	return g_prgOverride->CallFn( function, param1, param2, param3, param4, param5, param6, params7, params8 );
}



