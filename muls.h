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

protected:

	friend class cVerdata;

	cFile* idx;	//!< index file
	cFile* data;	//!< data file

	void loadCache();

	std::map< UI32, std::vector< T > > cache;

	virtual bool checkValidity( UI32 id, T b ) {	return true; }
	virtual UI32 getIndex( UI32 id ) {	return id;	}
	virtual UI32 getIndexForCache( UI32 id, T b ) {	return id;	}

public:

	bool isCached;	//!< true if cached on memory

	cMULFile( std::string idx, std::string data, bool cache  );
	~cMULFile();


	bool isReady() { return idx->file.is_open() && data->file.is_open(); }
	std::string getPath() { return idx->path; }
	virtual bool getData( UI32 id, std::vector< T >*& data )	{

		//ndEndy need because can be into verdata
		std::map< UI32, std::vector<T> >::iterator iter( cache.find( id ) );
		if( iter!=cache.end() ) {
			data=&iter->second;
			return false;
		}

		UI32 i = getIndex( id );
		if( (i==INVALID) || ( isCached ) ) {
			data=NULL;
			return false;
		}

		mul_index_st index;
		idx->file.seekg( i*sizeof(mul_index_st) );
		idx->file.read( (char*)&index, sizeof(mul_index_st) );
		if( index.start==INVALID || index.size==INVALID ) {
			data=NULL;
			return false;
		}

		if( ( index.size % sizeof(T) ) != 0  ) {
			ErrOut( "data corrupted ( index=%i ) in %s ", i, idx->path.c_str() );
			data=NULL;
			return false;
		}

		data = new std::vector<T>;
		this->data->file.seekg( index.start );
		T buffer;
	
		int count = index.size / sizeof(T);
		for( int s=0; s< count; ++s ) {
			this->data->file.read( (char*)&buffer, sizeof(T));
			if( checkValidity( id, buffer ) )
				data->push_back( buffer );
		}

		return true;
	}

};


template <class T> class NxwMulWrapper {

protected:
	std::vector< T >* data; 
	std::vector< T >::iterator current;
	bool needFree;
	UI32 idx;
	cMULFile<T>* mul;

public:

	NxwMulWrapper( cMULFile<T>* mul, UI32 id );
	~NxwMulWrapper()	{	if( needFree )	delete data;	}

	virtual void rewind()	{ 	if( data==NULL)	needFree = mul->getData( idx, data ); if( data!=NULL )	current=data->begin();	}
	UI32 size()	{	return (data!=NULL)? data->size() : 0;	}
	bool end() { 	return (data==NULL) || (current==data->end()); }
	bool isEmpty()	{	return size()<=0;	}
	NxwMulWrapper<T>& operator++(int)	{	current++;	return (*this);	}
	T get()	{	return *current;	}


};









extern std::string tiledata_path;
extern bool tiledata_cache;

enum {
	TILEFLAG_BACKGROUND	=	0x00000001,
	TILEFLAG_WEAPON		=	0x00000002,
	TILEFLAG_TRANSPARENT	=	0x00000004,
	TILEFLAG_TRANSLUCENT	=	0x00000008,
	TILEFLAG_WALL		=	0x00000010,
	TILEFLAG_DAMAGING	=	0x00000020,
	TILEFLAG_IMPASSABLE	=	0x00000040,
	TILEFLAG_WET		=	0x00000080,
	TILEFLAG_SURFACE	=	0x00000200,
	TILEFLAG_BRIDGE		=	0x00000400,
	TILEFLAG_STACKABLE	=	0x00000800,
	TILEFLAG_WINDOW		=	0x00001000,
	TILEFLAG_NOSHOOT	=	0x00002000,
	TILEFLAG_PREFIX_A	=	0x00004000,
	TILEFLAG_PREFIX_AN	=	0x00008000,
	TILEFLAG_INTERNAL	=	0x00010000,
	TILEFLAG_FOLIAGE	=	0x00020000,
	TILEFLAG_PARTIAL_HUE	=	0x00040000,
	TILEFLAG_MAP		=	0x00100000,
	TILEFLAG_CONTAINER	=	0x00200000,
	TILEFLAG_WEARABLE	=	0x00400000,
	TILEFLAG_LIGHTSOURCE	=	0x00800000,
	TILEFLAG_ANIMATED	=	0x01000000,
	TILEFLAG_NODIAGONAL	=	0x02000000,
	TILEFLAG_ARMOR		=	0x08000000,
	TILEFLAG_ROOF		=	0x10000000,
	TILEFLAG_DOOR		=	0x20000000,
	TILEFLAG_STAIR_BACK	=	0x40000000,
	TILEFLAG_STAIR_RIGHT	=	0x80000000
};

const int LENGTHNAMESTRING = 20;

const UI32 LANDGROUPCOUNT = 512;
const int LANDSINGROUP = 32;
const int LANDSINFOCOUNT = LANDGROUPCOUNT*LANDSINGROUP;

const UI32 TILEGROUPCOUNT = 512;
const int TILESINGROUP = 32;
const int TILESINFOCOUNT = TILEGROUPCOUNT*TILESINGROUP;
const int FIRSTTILEINFO = LANDGROUPCOUNT*LANDSINGROUP;

struct tile_st {
	UI32 flags;
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

struct tile_group_st {
	UI32 header;
	tile_st tile[ TILESINGROUP ];
} PACK_NEEDED;


struct land_st {
	UI32 flags;
	UI16 textureID;
	char name[ LENGTHNAMESTRING ];
} PACK_NEEDED;

struct land_group_st {
   	UI32 header;
	land_st land[ LANDSINGROUP ];
} PACK_NEEDED;

const int BASESTATICINFO = LANDGROUPCOUNT *sizeof(land_group_st);

typedef std::map<SERIAL, tile_st> STATICINFOMAP;
typedef std::map<SERIAL, land_st> LANDINFOMAP;


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
	bool getLand( SERIAL id, land_st& land );
	bool getStatic( SERIAL id, tile_st& stat );

};











extern std::string map_path;
extern UI16	map_width;
extern UI16 map_height;

const int DEFAULTHEIGHTMAP = 512;
const int DEFAULTWIDTHMAP = 768;

const int XCELLSINBLOCK = 8;
const int YCELLSINBLOCK = 8;
const int CELLSINBLOCK = XCELLSINBLOCK * YCELLSINBLOCK;

struct map_st {
	UI16 id;
	SI08 z;
} PACK_NEEDED;

struct mapblock_st {
	UI32 header;
	map_st celle[XCELLSINBLOCK][YCELLSINBLOCK];
}  PACK_NEEDED;


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
	bool getMap( UI16 x, UI16 y, map_st& m );

};





extern std::string multi_idx_path;
extern std::string multi_path;
extern bool multi_cache;

struct multi_st {
	UI16 id;
	UI16 x;
	UI16 y;
	UI16 alt;
	UI32 flags;
} PACK_NEEDED;

#define MULTISVEC std::vector<multi_st>
#define MULTISMAP std::map< UI32, MULTISVEC >
typedef MULTISVEC* P_MULTISVEC;


typedef cMULFile<multi_st> cMulti;







extern std::string statics_idx_path;
extern std::string statics_path;
extern bool statics_cache;


struct statics_st {
	UI16 id;
	UI08 x;
	UI08 y;
	SI08 z;
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

	friend class NxwMulWrapperStatics;

	UI16 width;	//!< width of map
	UI16 height; //!< height of map

protected:
	
	virtual bool checkValidity( UI32 id, statics_st b ) {	return (((id>>16)%8)==b.x) && (((id&0xFFFF)%8)==b.y); }
	virtual UI32 getIndex( UI32 id ) {	return blockFromXY( id>>16, id&0xFFFF );	}
	virtual UI32 getIndexForCache( UI32 id, statics_st b );
	UI32 getHash( UI32 x, UI32 y )	{	return (x<<16) +y;	}
	SERIAL blockFromXY( UI16 x, UI16 y ); 

public:

	cStatics( std::string pathidx, std::string pathdata, UI16 width, UI16 height, bool cache );
	~cStatics();

};








extern std::string verdata_path;

enum MUL_FILES {
	MUL_MAP = 0x00,
	MUL_STATIDX,
	MUL_STATICS,
	MUL_ARTIDX,
	MUL_ART,
	MUL_ANIMIDX,
	MUL_ANIM,
	MUL_SOUNDIDX,
	MUL_SOUND,
	MUL_TEXIDX,
	MUL_TEXMAPS,
	MUL_GUMPIDX,
	MUL_GUMPART,
	MUL_MULTIIDX,
	MUL_MULTI,
	MUL_SKILLSIDX,
	MUL_SKILLS,
	MUL_TILEDATA = 0x1E,
	MUL_ANIMDATA,
	MUL_VERDATA
};

struct mul_index_st {
	SI32 start;
	SI32 size;
	UI16 height;
	UI16 width;
} PACK_NEEDED;

struct mul_patch_st {
	UI32 file;
	UI32 id;
	mul_index_st info;
} PACK_NEEDED;

/*!
\brief Verdata
\attention CAN'T contain statics or mul records
*/
class cVerdata : private cFile {

private:

	bool isCached;	//!< true if cached on memory

public:

	cVerdata( std::string path, bool cache=true );
	~cVerdata();

	bool isReady();
	void load( cTiledata* tiledata, cMULFile<multi_st>* multi );
};


/*!
\author Luxor
*/
namespace data {

extern cTiledata* tiledata; //!<the tiledata class istance
extern cMap* maps; //!<the map class istance
extern cStatics* statics; //!<the statics class istance
extern cVerdata* verdata; //!<the verdata class istance
extern cMulti* multi;

void init();
void shutdown();
void setPath( MUL_FILES id, std::string path );
std::string getPath( MUL_FILES id );

LOGICAL seekMap( UI32 x, UI32 y, map_st& m, UI08 nMap = 0 ); //<! Luxor: nMap will be used for future multiple maps support.
LOGICAL seekLand( UI16 id, land_st& land );
LOGICAL seekTile( UI16 id, tile_st& tile );

} // namespace data







class NxwMulWrapperStatics : public NxwMulWrapper<statics_st> {
public:
	NxwMulWrapperStatics( UI32 x, UI32 y );

};

class NxwMulWrapperMulti : public NxwMulWrapper<multi_st> {
public:
	NxwMulWrapperMulti( UI32 id );

};



#endif


