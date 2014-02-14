  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __REGIONS_H
#define __REGIONS_H

#define MAX_MAP_CELLS 33000

#define REGION_GRIDSIZE 32
#define REGION_COLSIZE  32

#define MAP_WIDTH 6144
#define MAP_HEIGHT 4096

#define REGION_X_CELLS MAP_WIDTH/REGION_GRIDSIZE
#define REGION_Y_CELLS MAP_HEIGHT/REGION_COLSIZE

struct region_st
{
	LOGICAL inUse;
 	TEXT name[50];
 	SI32 midilist;
 	UI08 priv;	// 0x01 guarded, 0x02, mark allowed, 0x04, gate allowed, 0x08, recall
			// 0x10 raining, 0x20, snowing, 0x40 magic damage reduced to 0
	TEXT guardowner[50];
	UI08 snowchance;
	UI08 rainchance;
	UI08 drychance;
	UI08 keepchance;
	UI08 wtype;
	SI32 guardnum[10];
	SI32 goodsell[256]; // Magius(CHE)
	SI32 goodbuy[256]; // Magius(CHE)
	SI32 goodrnd1[256]; // Magius(CHE) (2)
	SI32 goodrnd2[256]; // Magius(CHE) (2)
	SI32 forcedseason;
	LOGICAL ignoreseason;
};

extern region_st region[256];

typedef struct {
	SERIAL_SET charsInRegions;
	SERIAL_SET itemsInRegions;
} region_db_st;

class RegCoordPoint : public cPoint< UI16 > {
public:
	
	RegCoordPoint( Location location );
};

typedef std::map< SERIAL, RegCoordPoint > WHERE_IS_NOW_MAP;

class cRegion
{
private:

	WHERE_IS_NOW_MAP item_where_is_now;
	WHERE_IS_NOW_MAP char_where_is_now;
	void removeNow( P_CHAR pc );
	void removeNow( P_ITEM pi );
	void addNow( P_CHAR pc );
	void addNow( P_ITEM pi );

	region_db_st regions[REGION_X_CELLS][REGION_Y_CELLS];

	bool isValidCoord( UI16 x, UI16 y );

public:

	friend class NxwItemWrapper;
	friend class NxwCharWrapper;

	cRegion(); //constructor
	~cRegion(); //destructor
	
	void add( P_CHAR pc );
	void add( P_ITEM pi );
	void remove( P_CHAR pc );
	void remove( P_ITEM pi );


};


#endif

