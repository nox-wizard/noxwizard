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
\brief NPC Stuff
*/

#ifndef NPCS_INCLUDE
#define NPCS_INCLUDE

char* getRandomName(char * namelist);
void setrandomname(P_CHAR pc, char * namelist);
COLOR addrandomcolor(cObject* po, char *colorlist);

namespace npcs
{
	enum {
		MAGIC_SPHERE_CLASSIC = 0,
		MAGIC_SPHERE_WATER,
		MAGIC_SPHERE_FIRE,
		MAGIC_SPHERE_EARTH,
		MAGIC_SPHERE_AIR,
		MAGIC_SPHERE_MENTAL,
		MAGIC_SPHERE_CLERICAL,
		MAGIC_SPHERE_NECROMANCY,
		MAGIC_SPHERE_POISON,
		MAGIC_MAXSPHERES
	};

	void SpawnGuard( P_CHAR pc, P_CHAR pc_i, Location where);		// added by Anthalir
	void SpawnGuard( P_CHAR pc, P_CHAR pc_i, int x, int y, signed char z);
	void initNpcSpells();
	void npcCastSpell(P_CHAR pc_att, P_CHAR pc_def);
	void npcMagicAttack(P_CHAR pc_att, P_CHAR pc_def);

	P_CHAR AddNPC(NXWSOCKET s, P_ITEM pi, int npcNum, UI16 x1, UI16 y1, SI08 z1);
	P_CHAR AddNPC(NXWSOCKET s, P_ITEM pi, int npcNum, Location where);	// added by Anthalir
	P_CHAR addNpc(int npcNum, int x, int y, int z);

	P_CHAR AddRespawnNPC(P_ITEM pi, int npcNum);
	P_CHAR AddRespawnNPC(P_CHAR pc, int npcNum);

	P_CHAR AddNPCxyz(NXWSOCKET s, int npcNum, Location where);		// added by anthalir
	P_CHAR AddNPCxyz(NXWSOCKET s, int npcNum, int x1, int y1, signed char z1);

	P_ITEM AddRandomLoot(P_ITEM pack, char * lootlist);

	int AddRandomNPC(int s, char *npclist, int spawnpoint);
	
	void checkAI(P_CHAR pc );

	P_CHAR SpawnRandomMonster(P_CHAR pc, char* cList, char* cNpcID);


}	//namespace npcs


#endif
