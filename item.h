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
\brief Header defining item namespace
\author Akron
\date 18/03/2003
\since 0.82r3
\todo move here eLayer and item type enums
*/
#ifndef __ITEM_H__
#define __ITEM_H__

#include "nxwcommn.h"

/*!
\brief Item related stuff
*/
namespace item
{

	P_ITEM		CreateScriptRandomItem(SI32 s, char *sItemList, cObject* cont=NULL );
	void		GetScriptItemSetting(P_ITEM pi); // by Magius(CHE)
	SI32		getname(SI32 i, char* itemname);

	P_ITEM		CreateFromScript(NXWSOCKET so, char *itemname);
	P_ITEM		CreateFromScript(NXWSOCKET s, SI32 itemnum, cObject* cont=NULL );
	P_ITEM		CreateScriptItem(NXWSOCKET s, SI32 itemnum, LOGICAL nSpawned, cObject* cont=NULL );
	SI32		CreateRandomItem(char *sItemList);
	P_ITEM		SpawnItem(NXWSOCKET  nSocket,
						SI32 nAmount, char* cName, LOGICAL nStackable,
						SI16 cItemId, SI16 cColorId,
						LOGICAL nPack, LOGICAL nSend);

	P_ITEM		SpawnItem(NXWSOCKET  nSocket, CHARACTER ch,
						SI32 nAmount, char* cName, LOGICAL nStackable,
						SI16 cItemId, SI16 cColorId,
						LOGICAL nPack, LOGICAL nSend);

	P_ITEM		SpawnItem(CHARACTER ch,
						SI32 nAmount, char* cName, LOGICAL pileable,
						SI16 id, SI16 color, LOGICAL nPack);

	P_ITEM		SpawnItemBank(CHARACTER ch, int nItem);
	P_ITEM		SpawnItemBackpack2(NXWSOCKET  s, int nItem, int nDigging);

	void		AddRespawnItem(int s, int x, int y);
	P_ITEM		add (int itemid, int x=INVALID, int y = INVALID, int z = INVALID);
	P_ITEM		addByID(int id, UI32 nAmount, const char *cName, int color, Location where);	// added by Anthalir
	P_ITEM		addByID(int id, UI32 nAmount = 1, const char* cName = NULL, int color = 0x0000, int x=INVALID, int y = INVALID, int z = INVALID);
	P_ITEM		SpawnRandomItem(NXWSOCKET s, char* cList, char* cItemID);
};

#endif
