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
\author Endymion
\brief MUL and UO Client related stuff
*/
#ifndef _MULS_H
#define _MULS_H

#include "nxwcommn.h"

namespace tiledata {

extern std::string path; //!< path

const int LENGTHNAMESTRING = 20;

const UI32 LANDGROUPCOUNT = 512;
const int LANDINGROUP = 32;
const int LANDSINFOCOUNT = LANDGROUPCOUNT*LANDINGROUP;

const UI32 STATICGROUPCOUNT = 512;
const int STATICINGROUP = 32;
const int STATICSINFOCOUNT = STATICGROUPCOUNT*STATICINGROUP;
const int FIRSTSTATICSINFO = LANDGROUPCOUNT*LANDINGROUP;

struct staticinfo_st {
	UI32 flag;
	UI08 weight;
	UI08 quality;
	UI16 unk;
	UI08 unk2;
	UI08 quantity;	//!< (weapon=Weapon Class; armour=Armour Class)
	UI16 animation;	//!< The bodyID of the the item's animation
	UI08 unk3;
	UI08 hue;
	UI08 unk4;
	UI08 value;
	UI08 height;	//!< Carries
	char name[ LENGTHNAMESTRING ];
} PACK_NEEDED;
typedef staticinfo_st TSTATICINFO;
typedef TSTATICINFO* P_STATICINFO;

struct staticgroup_st {
	UI32 header;
	TSTATICINFO statics[STATICINGROUP];
} PACK_NEEDED;
typedef staticgroup_st TSTATICGROUP;


struct landinfo_st {
	UI32 flag;
	UI16 textureID;
	char name[ LENGTHNAMESTRING ];
} PACK_NEEDED;
typedef landinfo_st TLANDINFO;
typedef TLANDINFO* P_LANDINFO;

struct landgroup_st {
   	UI32 header;
	TLANDINFO lands[ LANDINGROUP ];
} PACK_NEEDED;
typedef landgroup_st TLANDGROUP;

const int BASESTATICINFO = LANDGROUPCOUNT *sizeof( TLANDGROUP );

typedef std::map<SERIAL, TSTATICINFO> STATICINFOMAP;
typedef std::map<SERIAL, TLANDINFO> LANDINFOMAP;



/*!
\brief Tiledata
*/
class cTiledata {

private:

	std::ifstream file;	//!< file stream
	bool isCached;	//!< true if cached on memory
	std::string path;	//!< file path

	STATICINFOMAP staticsCached;	//!< all static info cached
	LANDINFOMAP landsCached;	//!< all land info cached

public:

	cTiledata( const char* path, bool cache=false, class cVerdata* verdata=NULL );
	~cTiledata();
	bool isReady();
	

	void loadForCaching();
	void addVerdata(  );
	bool getLand( SERIAL id, TLANDINFO& land );
	bool getStatic( SERIAL id, TSTATICINFO& stat );

};

extern cTiledata* tiledata; //!<the tiledata class istance

}









namespace map_uo {

extern std::string path; //!< path

const int DEFAULTHEIGHTMAP = 512;
const int DEFAULTWIDTHMAP = 768;

const int XCELLSINBLOCK = 8;
const int YCELLSINBLOCK = 8;
const int CELLSINBLOCK = XCELLSINBLOCK * YCELLSINBLOCK;

struct tcella_st {
	UI16 id;
	SI08 altitudine;
} PACK_NEEDED;
typedef tcella_st TCELLA;
typedef TCELLA* P_CELLA;

struct tblocco_st {
	UI32 header;
	TCELLA celle[XCELLSINBLOCK][YCELLSINBLOCK];
}  PACK_NEEDED;

typedef tblocco_st TBLOCCO;

/*!
\brief Map
*/
class cMap {

private:

	std::ifstream file;	//!< file stream
	bool isCached;	//!< true if cached on memory ( 75 MEGA? very cool :P but not implemented yet )
	std::string path;	//!< file path

	UI16 width;	//!< width of the map
    UI16 height;	//!< height of the map

public:

	cMap( const char* path, UI16 width, UI16 height, bool cache=false );
	~cMap();
	bool isReady();

	bool getMap( UI16 x, UI16 y, TCELLA& cella );


};

extern cMap* mappa; //!<the map class istance

}


namespace multi {

extern std::string pathidx; //!< path index
extern std::string pathdata; //!< path data

struct multi_st {
	UI16 id;
	UI16 x;
	UI16 y;
	UI16 alt;
	UI32 flags;
} PACK_NEEDED;
typedef multi_st TMULTI;

typedef std::vector<TMULTI> MULTISVEC;
typedef std::map< UI32, MULTISVEC > MULTISMAP;

class cMulti {

private:

	std::ifstream idx;	//!< file stream
	std::ifstream data;
	bool isCached;	//!< true if cached on memory
	std::string pathidx;	//!< file path of index file
	std::string pathdata;	//!< file path of data file

	MULTISMAP multisCached;	//!< all multi cached

public:

	cMulti( const char* pathidx, const char* pathdata, bool cache=false, class cVerdata* verdata=NULL );
	~cMulti();
	bool isReady();

	void loadForCaching();
	void addVerdata( );
	bool getMulti( UI32 id, MULTISVEC& multi );

};


extern cMulti* multi;

}

namespace verdata {

extern std::string path; //!< path

enum VERFILE {
	MAP = 0x00,
	STAIDX,
	STATICS,
	ARTIDX,
	ART,
	ANIMIDX,
	ANIM,
	SOUNDIDX,
	SOUND,
	TEXIDX,
	TEXMAPS,
	GUMPIDX,
	GUMPART,
	MULTIIDX,
	MULTI,
	SKILLSIDX,
	SKILLS,
	TILEDATA = 0x1E,
	ANIMDATA 
};

struct index_st {
	SI32 start;
	SI32 size;
	UI16 height;
	UI16 width;
} PACK_NEEDED;
typedef index_st TINDEX;

struct patch_st {
	UI32 file;
	UI32 id;
	TINDEX info;
} PACK_NEEDED;
typedef patch_st TPATCH;

/*!
\brief Verdata
\attention CAN'T contain statics or mul records
*/
class cVerdata {

private:

	std::ifstream file;	//!< file stream
	bool isCached;	//!< true if cached on memory
	std::string path;	//!< file path

public:

	tiledata::STATICINFOMAP staticsCached;	//!< all static info cached
	tiledata::LANDINFOMAP landsCached;	//!< all land info cached
	multi::MULTISMAP multisCached;


	cVerdata( const char* path, bool cache=true );
	~cVerdata();
	bool isReady();

	void loadForCaching();
};

typedef cVerdata* P_VERDATA;

extern cVerdata* verdata; //!<the verdata class istance

}


namespace statics {

extern std::string pathidx; //!< path index
extern std::string pathdata; //!< path data

struct statics_st {
	UI16 id;
	UI08 x;
	UI08 y;
	SI08 altitudine;
	COLOR color;
} PACK_NEEDED;
typedef statics_st TSTATICS;

typedef std::vector< TSTATICS > STATICSVET;
typedef std::map< UI32, STATICSVET > STATICSMAP;
typedef STATICSVET* P_STATICSVET;

/*!
\brief Statics
*/
class cStatics {

private:

	std::ifstream idx;	//!< file stream
	std::ifstream data;
	bool isCached;	//!< true if cached on memory
	std::string pathidx;	//!< file path of index file
	std::string pathdata;	//!< file path of data file

	UI16 width;	//!< width of map
	UI16 height; //!< height of map

	STATICSMAP staticsCached;	//!< all statics cached

public:

	cStatics( const char* pathidx, const char* pathdata, UI16 width, UI16 height, bool cache=false );
	~cStatics();
	bool isReady();

	void loadForCaching();
	bool getStatics( UI16 x, UI16 y, STATICSVET& stats );

};

extern cStatics* statics; //!<the statics class istance

}


namespace mulmanager {

//stupid
	bool IsTileWet(int tilenum);
	bool TileWalk(int tilenum);
	bool DoesTileBlock(int tilenum);
	bool DoesStaticBlock(short int x, short int y, signed char oldz);
	bool IsRoofOrFloorTile( tile_st *tile );
	bool IsRoofOrFloorTile( unitile_st *tile );
//can be
	bool IsUnderRoof(short int x, short int y, signed char z);
	bool CanMonsterMoveHere( short int x, short int y, signed char z );

	int DynTile( short int x, short int y, signed char oldz );

	void MultiArea(P_ITEM pi, int *x1, int *y1, int *x2, int *y2);

//usefull

	inline SI16 StaticTop(Location where);	// added by Anthalir
	SI16 StaticTop( UI16 x, UI16 y, SI16 oldz );
	
	inline SI16 DynamicElevation(Location where);	// added by Anthalir
	SI16 DynamicElevation( UI16 x, UI16 y, SI16 oldz );
	
	SI16 MapElevation( UI16 x, UI16 y );
	
	inline SI16 AverageMapElevation(Location where, int &id);	// added by Anthalir
	SI16 AverageMapElevation( UI16 x, UI16 y, int &id);
	
	inline SI16 Height(Location where);	// added by Anthalir
	SI16 Height( UI16 x, UI16 y, SI16 oldz );
	
	SI16 TileHeight( UI32 tilenum );


}



#endif

