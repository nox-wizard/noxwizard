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
\author Akron
\brief Commands namespace
*/

#ifndef __COMMANDS__
#define __COMMANDS__

#include "typedefs.h"

/*!
\brief Commands related stuff
\author Akron (to namespace)
\since 0.82r3
\date 18/03/2003
\note ported from cCommands class
*/
namespace Commands
{
	extern SI32 cmd_offset;					//!< Command Offset

	void NextCall(NXWSOCKET s, int type);
	void KillSpawn(NXWSOCKET s, int r);
	void RegSpawnMax(NXWSOCKET s, int r);
	void RegSpawnNum(NXWSOCKET s, int r, int n);
	void KillAll(NXWSOCKET s, int percent, char * sysmsg);
	void AddHere(NXWSOCKET s, UI16 id, char z);
	void ShowGMQue(NXWSOCKET s, int type);
	void Wipe(NXWSOCKET s);
	void CPage(NXWSOCKET s, std::string reason);
	void GMPage(NXWSOCKET s, std::string reason);
	void Command( NXWSOCKET , char * );
	void MakeShop(int c);
	void DyeItem(NXWSOCKET s);
	void DupeItem(NXWSOCKET s, int i, int amount);
	void Possess(NXWSOCKET s);
};

#endif
