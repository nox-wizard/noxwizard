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


class cFile {

public:
	std::string path;
	std::ifstream file;

	cFile( std::string path );
	~cFile();

};

template <typename T> class cMULFile {

private:

	friend class cVerdata;

	cFile* idx;	//!< index file
	cFile* data;	//!< data file

	std::map< UI32, std::vector< T > > cache;
	void loadCache();

public:

	bool isCached;	//!< true if cached on memory

	cMULFile( std::string idx, std::string data, bool cache  );
	~cMULFile();

	virtual bool is_open() { return idx->file.is_open() && data->file.is_open(); }
	virtual bool getData( UI32 i, std::vector< T >* data );
	std::string getPath() { return idx->path; }

};


template <typename T> class NxwMulWrapper {

private:
	std::vector< T >* data; 
	std::vector< T >::iterator current;
	bool needFree;
	UI32 idx;
	cMULFile<T>* mul;

public:

	NxwMulWrapper( cMULFile<T>* mul, UI32 id );
	NxwMulWrapper( class cStatics* statics, UI32 x, UI32 y );
	~NxwMulWrapper();

	void rewind();
	UI32 size();
	bool end();
	bool isEmpty();
	NxwMulWrapper<T>& operator++(int);
	T get();

};

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
class cTiledata : private cFile {

private:

	friend class cVerdata;

	bool isCached;	//!< true if cached on memory

	STATICINFOMAP staticsCached;	//!< all static info cached
	LANDINFOMAP landsCached;	//!< all land info cached

public:

	cTiledata( std::string path, bool cache=false );
	~cTiledata();
	bool isReady();
	
	void loadForCaching();
	bool getLand( SERIAL id, TLANDINFO& land );
	bool getStatic( SERIAL id, TSTATICINFO& stat );

};


const int DEFAULTHEIGHTMAP = 512;
const int DEFAULTWIDTHMAP = 768;

const int XCELLSINBLOCK = 8;
const int YCELLSINBLOCK = 8;
const int CELLSINBLOCK = XCELLSINBLOCK * YCELLSINBLOCK;

struct tcella_st {
	UI16 id;
	SI08 z;
} PACK_NEEDED;
typedef tcella_st TCELLA;
typedef TCELLA* P_CELLA;

typedef tcella_st map_st;

struct tblocco_st {
	UI32 header;
	TCELLA celle[XCELLSINBLOCK][YCELLSINBLOCK];
}  PACK_NEEDED;

typedef tblocco_st TBLOCCO;

/*!
\brief Map
*/
class cMap : private cFile {

private:

	bool isCached;	//!< true if cached on memory ( 75 MEGA? very cool :P but not implemented yet )

	UI16 width;	//!< width of the map
    UI16 height;	//!< height of the map

	bool isReady();

public:

	cMap( std::string path, UI16 width, UI16 height, bool cache=false );
	~cMap();

	bool getMap( UI16 x, UI16 y, TCELLA& cella );


};



struct multi_st {
	UI16 id;
	UI16 x;
	UI16 y;
	UI16 alt;
	UI32 flags;
} PACK_NEEDED;
typedef multi_st TMULTI;

#define MULTISVEC std::vector<TMULTI>
#define MULTISMAP std::map< UI32, MULTISVEC >
typedef MULTISVEC* P_MULTISVEC;


typedef cMULFile<multi_st> cMulti;

struct statics_st {
	UI16 id;
	UI08 x;
	UI08 y;
	SI08 altitudine;
	COLOR color;
} PACK_NEEDED;

#define STATICSVET std::vector< statics_st >
#define STATICSMAP std::map< UI32, STATICSVET >
typedef STATICSVET* P_STATICSVET;

/*!
\brief Statics
*/
class cStatics : public cMULFile< statics_st >{

private:

	UI16 width;	//!< width of map
	UI16 height; //!< height of map

public:

	cStatics( std::string pathidx, std::string pathdata, UI16 width, UI16 height, bool cache );
	~cStatics();

	SERIAL idFromXY( UI16 x, UI16 y ); 
	virtual bool getData( UI16 x, UI16 y, std::vector<statics_st>* stats );

};



enum VERFILE {
	VF_MAP = 0x00,
	VF_STAIDX,
	VF_STATICS,
	VF_ARTIDX,
	VF_ART,
	VF_ANIMIDX,
	VF_ANIM,
	VF_SOUNDIDX,
	VF_SOUND,
	VF_TEXIDX,
	VF_TEXMAPS,
	VF_GUMPIDX,
	VF_GUMPART,
	VF_MULTIIDX,
	VF_MULTI,
	VF_SKILLSIDX,
	VF_SKILLS,
	VF_TILEDATA = 0x1E,
	VF_ANIMDATA 
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
class cVerdata : private cFile {

private:

	bool isCached;	//!< true if cached on memory
	bool isReady();

public:

	cVerdata( std::string path, bool cache=true );
	~cVerdata();

	void load( cTiledata* tiledata, cMULFile<multi_st>* multi );
};



namespace mul {
	extern class cTiledata* tiledata; //!<the tiledata class istance
	extern class cMap* mappa; //!<the map class istance
	extern class cStatics* statics; //!<the statics class istance
	extern class cVerdata* verdata; //!<the verdata class istance
	extern cMulti* multi;
}




#endif


