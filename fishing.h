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
\brief Fishing Functions
*/

#ifndef __FISHING_H__
#define __FISHING_H__

/*!
 * \brief Fishing Related stuff
 */
namespace fishing
{
	void FishTarget(NXWCLIENT ps);
	void Fish(CHARACTER c);

	int SpawnFishingMonster(P_CHAR pc, char* cScript, char* cList, char* cNpcID);
	int SpawnFishingItem(NXWSOCKET nCharID,int nInPack, char* cScript, char* cList, char* cItemID);
}

#endif
