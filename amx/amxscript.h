/*!
 ***********************************************************************************
 *  file    : amxscript.h
 *
 *  Project : Nox-Wizard
 *
 *  Author  : 
 *
 *  Purpose : Definition of methods Amx for Script
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
 *//*! \file amxscript.h
	\brief Definition of methods Amx for Script
 */
#ifndef __AMXSCRIPT_H__
#define __AMXSCRIPT_H__
//#include "amx_vm.h"
//<Luxor>
//typedef long int cell;
#include "amx.h"
//</Luxor>


/*!
\brief Class AmxProgram
\author Xanathar
*/
class AmxProgram {
private:
	void *m_ptrCode;
	int  m_nSize;
	bool _init(void *program);
	struct __amx *m_AMX;
public:
	void Load (const char *filename);
	cell CallFn (const char *fn);
	cell CallFn (int fn);
	cell CallFn (int fn, int param);
	cell CallFn (int fn, int param1, int param2);
	cell CallFn (int fn, int param1, int param2, int param3, int param4 = -1);
	cell CallFn (int fn, int param1, int param2, int param3, int param4, int param5); //Luxor
    cell CallFn (int fn, int param1, int param2, int param3, int param4, int param5, int param6); //Luxor
	int getFnOrdinal (char *fn);
	explicit AmxProgram (const char *filename) { Load(filename); };
	AmxProgram () { m_ptrCode = NULL; }
	~AmxProgram () { if (m_ptrCode!= NULL) delete static_cast<char*>(m_ptrCode); }
};


#ifdef  __cplusplus
extern  "C" {
#endif


	extern  char g_cAmxPrintBuffer[2000];
	extern  int  g_nAmxPrintPtr;

#ifdef  __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////////
// HERE THE OVERRIDE PART OF CODE :) - as always by xan :)
////////////////////////////////////////////////////////////////////////////////

#define AMX_NONE 0
#define AMX_BEFORE 1
#define AMX_AFTER 2
#define AMX_FRAME 3
#define AMX_SKIP 4
#define AMX_BYPASS 5


/*!
\brief Class AmxOverride
\author Xanathar
*/
class AmxOverride {
	char *m_function;
	int   m_ordinal;
	int	  m_mode;
public: 
	bool Exec (int moment);
	bool Exec (int moment, int sock);
	int  getOrdinal (void) {return m_ordinal; };
	AmxOverride(); 
	~AmxOverride(); 
	void Load (char *fname);
	void Parse (char *line);
};

#define AMX_MAXTYPE 10
#define AMX_MAXNUMBER 8192

extern AmxOverride g_Scripts[AMX_MAXTYPE][AMX_MAXNUMBER];

#define AMXEXEC(TYPE,NUMBER,RET,MOMENT) { if (g_Scripts[TYPE][NUMBER].Exec(MOMENT)) return RET; }
#define AMXEXECV(TYPE,NUMBER,MOMENT) { if (g_Scripts[TYPE][NUMBER].Exec(MOMENT)) return; }
#define AMXEXECS(SOCKET,TYPE,NUMBER,RET,MOMENT) { if (g_Scripts[TYPE][NUMBER].Exec(MOMENT,SOCKET)) return RET; }
#define AMXEXECSV(SOCKET,TYPE,NUMBER,MOMENT) { if (g_Scripts[TYPE][NUMBER].Exec(MOMENT,SOCKET)) return; }
#define AMXEXECSVNR(SOCKET,TYPE,NUMBER,MOMENT) { g_Scripts[TYPE][NUMBER].Exec(MOMENT,SOCKET); }

#define AMXT_SKITARGS 0
#define AMXT_SKILLS 1
#define AMXT_SPEECH 2
#define AMXT_NETRCV 3
#define AMXT_TRIGGERS 4
#define AMXT_MAGICSELECT 5
#define AMXT_MAGICCAST 6
#define AMXT_SPECIALS 7
#define AMXT_GMCMD 8

void LoadOverrides (void);

extern AmxProgram *g_prgOverride;

extern bool g_bByPass;


#endif //__AMXSCRIPT_H__
