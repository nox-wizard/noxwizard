/*!
 ***********************************************************************************
 *  file    : amxcback.h
 *
 *  Project : Nox-Wizard
 *
 *  Author  : 
 *
 *  Purpose : Definition of Functions for AMX Callbacks and AMX Events
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
 *//*! \file amxcback.h
	\brief Definition of Functions for AMX Callbacks and AMX Events
 */
#ifndef __AMXCBACK_H__
#define __AMXCBACK_H__

#include <string>
#include <vector>
using namespace std ;

#include "client.h"

void initAmxEvents(void);
bool checkItemUsability(P_CHAR pc, P_ITEM pi, int type);



/*!
\brief Definition of AmxEvent
\author Xanathar
*/
class AmxEvent {
private:
	bool dynamic;	//!< means : saved on worldsave
	bool valid;
	int  function;
	char *funcname;	//!< function name
public:
	class AmxEvent *listNext;
	class AmxEvent *hashNext;
	cell Call (int param1, int param2=-1, int param3=-1, int param4=-1);
	char *getFuncName (void);
	bool shouldBeSaved(void) { return !dynamic; }
	AmxEvent(char *fnname, bool dyn);
	AmxEvent(int funidx);
};

AmxEvent* newAmxEvent(char *funcname, bool dynamic = false);




#endif //__AMXCBACK_H__
