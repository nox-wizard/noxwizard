  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef _SPAWN_H
#define _SPAWN_H

#include "area.h"

enum SPAWNFLAG_ENUM
{
	SPAWN_AMOUNT,
	SPAWN_RANDOM,
	SPAWN_MAX
} ;

enum SPAWNTYPE_ENUM
{
	SPAWN_ALL,
	SPAWN_DYNAMIC,
	SPAWN_STATIC
} ;

class cSpawnArea {

public:
	bool findValidLocation( Location& location );
	bool disabled;

public:

	cSpawnArea( AREA_ITER area );
	~cSpawnArea();

	UI32 current; //!< currente number of spawned
	TIMERVAL nextspawn; //!< next spawn

	SERIAL_SET items_spawned;
	SERIAL_SET npcs_spawned;
	AREA_ITER where;

	bool needSpawn();


};

typedef std::vector< cSpawnArea > SPAWNAREA_VECTOR;

class cSpawnScripted {

private:

	void safeCreate( P_CHAR npc, cSpawnArea& single );
	void safeCreate( P_ITEM pi, cSpawnArea& single  );


public:

	cSpawnScripted( SERIAL serial );
	~cSpawnScripted();

	SERIAL serial; //!< region serial

	TIMERVAL mintime; //!< min time value	
	TIMERVAL maxtime; //!< max time value
	SERIAL_VECTOR npclists; //!< npclist to spawn
	SERIAL_VECTOR npcs; //!< npc to spawn
	SERIAL_VECTOR itemlists; //!< itemlist to spawn
	SERIAL_VECTOR itms; //!< item to spawn
	std::string name; //!< region name
	UI32 max; //!< max number of spanwed

	SPAWNAREA_VECTOR singles; //!< spawn areas

	void doSpawn( cSpawnArea& c );
	void doSpawn( );
	void doSpawnAll();
	void doSpawn(SPAWNFLAG_ENUM spawnWhat, UI32 amount=0 );

	void doRegionClearSpawn(UI32 region, SPAWNTYPE_ENUM clearWhat);

	void removeObject( P_ITEM pi );
	void removeObject( P_CHAR pc );
	void addSpawned(P_OBJECT obj);
	void clear();
};

class cSpawnDinamic {
public:

	cSpawnDinamic( P_ITEM pi );
	~cSpawnDinamic();

	SERIAL item; //!< the spawner
	SERIAL_SET items_spawned; //!< spawned items
	SERIAL_SET npcs_spawned; //!< spawned npcs

	UI32 current;
	TIMERVAL nextspawn; //!< next spawn

	void doSpawn();
	bool needSpawn();
	void remove( SERIAL serial );
	void addSpawned(P_OBJECT obj);
	void clear();
	void doSpawn(SPAWNFLAG_ENUM spawnWhat, UI32 amount);


};

typedef std::map< SERIAL, cSpawnScripted > SPAWN_SCRIPTED_DB;
typedef std::map< SERIAL, cSpawnDinamic  > SPAWN_DINAMIC_DB;

class cSpawns {
private:

	SPAWN_SCRIPTED_DB scripted; //!< list of scripted spawn
	SPAWN_DINAMIC_DB dinamic; //!< list of dinamic spawn

public:
	cSpawns();
	~cSpawns();

	TIMERVAL check; //!< check respawn

	void loadFromScript();
	void loadFromItem( P_ITEM pi );

	void clearDynamic();
	void doSpawnScripted( SERIAL spawn );
	void doSpawn();
	void doSpawnAll( );
	void doSpawnAll( SERIAL spawn );

	void removeObject( SERIAL spawn, P_ITEM pi );
	void removeObject( SERIAL spawn, P_CHAR pc );
	void removeSpawnDinamic( P_ITEM pi );
	void removeSpawnDinamic( P_CHAR pc );
	cSpawnDinamic *getDynamicSpawn(SERIAL spawnerSerial);
	cSpawnScripted *getScriptedSpawn(SERIAL spawnerSerial);
	void resetSpawnTime();
	void doRegionSpawn(UI32 region, SPAWNFLAG_ENUM spawnWhat, UI32 amount=0);
	void doRegionClearSpawn(UI32 region, SPAWNTYPE_ENUM clearWhat);


};

extern cSpawns* Spawns;


#endif
