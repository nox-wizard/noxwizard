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


extern class cTiledata* tiledata; //!<the tiledata class istance
extern class cMap* mappa; //!<the map class istance
extern class cMulti* multi;
extern class cStatics* statics; //!<the statics class istance
extern class cVerdata* verdata; //!<the verdata class istance


class cFile {

public:
	std::string path;
	std::ifstream file;

	cFile( std::string path );
	~cFile();

};

template <typename T> class cMULFile {

private:

	cFile* idx;	//!< index file
	cFile* data;	//!< data file

public:

	bool isCached;	//!< true if cached on memory

	cMULFile( std::string idx, std::string data );
	~cMULFile();

	virtual bool is_open() { return idx->is_open() && data->is_open(); }
	virtual bool getData( UI32 i, std::vector< T >* data );
	std::string getPath() { return idx->path; }

};

template <typename T> class cMULFileCached : public cMULFile<T> {

private:

	std::map< UI32, std::vector< T > > cache;
	void loadCache();

public:
	
	cMULFileCached( std::string idx, std::string data );
	~cMULFileCached();

	virtual bool is_open() { return true; }
	virtual bool getData( UI32 i, std::vector< T >* data );

};

template <typename T, typename M> class NxwMulWrapper {

private:
	std::vector< T >* data; 
	std::vector< T >::iterator current;
	bool needFree;
	UI32 idx;
	cMULFile<M>* mul;

public:

	NxwMulWrapper( cMULFile<M>* mul, UI32 i );
	~NxwMulWrapper();

	void rewind();
	UI32 size();
	bool end();
	bool isEmpty();
	NxwMulWrapper<T,M>& operator++(int);
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

	bool isCached;	//!< true if cached on memory

	STATICINFOMAP staticsCached;	//!< all static info cached
	LANDINFOMAP landsCached;	//!< all land info cached

public:

	cTiledata( std::string path, bool cache=false, class cVerdata* verdata=NULL );
	~cTiledata();
	bool isReady();
	

	void loadForCaching();
	void addVerdata(  );
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
class cMap : private cFile {

private:

	bool isCached;	//!< true if cached on memory ( 75 MEGA? very cool :P but not implemented yet )

	UI16 width;	//!< width of the map
    UI16 height;	//!< height of the map

public:

	cMap( std::string path, UI16 width, UI16 height, bool cache=false );
	~cMap();
	bool isReady();

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

class cMulti {

private:

	cMULFile<multi_st>* file;

public:

	cMulti( std::string pathidx, std::string pathdata, bool cache=false, class cVerdata* verdata=NULL );
	~cMulti();
	bool isReady();

	void addVerdata( );
	bool getMulti( UI32 id, P_MULTISVEC multi );

};



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
class cStatics {

private:

	cMULFile<statics_st>* file;

	UI16 width;	//!< width of map
	UI16 height; //!< height of map


public:

	cStatics( std::string pathidx, std::string pathdata, UI16 width, UI16 height, bool cache=false );
	~cStatics();
	bool isReady();

	bool getStatics( UI16 x, UI16 y, P_STATICSVET stats );

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

public:

	STATICINFOMAP staticsCached;	//!< all static info cached
	LANDINFOMAP landsCached;	//!< all land info cached
	MULTISMAP multisCached;


	cVerdata( std::string path, bool cache=true );
	~cVerdata();
	bool isReady();

	void loadForCaching();
};






#endif


