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
\brief Base Emulator's Functions
*/

#ifndef __NOXWIZARD_H__
#define __NOXWIZARD_H__

#include "typedefs.h"

void StartClasses();
void SetGlobalVars();
void BuildPointerArray();
void InitMultis();
void DeleteClasses();
void telltime(NXWSOCKET s);
void impaction(int s, int act);
bool npcattacktarget(int target2, int target);
void enlist(int s, int listnum);		//!< For enlisting in army
void gcollect();
void initque();
void addgold(int s, int totgold);
void StoreItemRandomValue(P_ITEM pi,int tmpreg);
void loadmetagm();

// Profiling
void StartMilliTimer(unsigned long &Seconds, unsigned long &Milliseconds);
unsigned long CheckMilliTimer(unsigned long &Seconds, unsigned long &Milliseconds);

void criminal(P_CHAR pc);
void callguards( int p );
void charcreate(NXWSOCKET  s);			//!< All the character creation stuff
void usepotion(int p, P_ITEM pi);

void setcharflag(P_CHAR pc);
LOGICAL setcharflag2(P_CHAR pc);

void newScriptsInit();
void deleteNewScripts();

void init_creatures();

#endif
