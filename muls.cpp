  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "muls.h"
#include "inlines.h"


std::string verdata_path;

std::string statics_idx_path;
std::string statics_path;
bool statics_cache = false;

std::string map_path;
UI16 map_width = DEFAULTWIDTHMAP;
UI16 map_height = DEFAULTHEIGHTMAP;

std::string tiledata_path;
bool tiledata_cache = false;

std::string multi_idx_path;
std::string multi_path;
bool multi_cache = false;



namespace data {

cTiledata* tiledata=NULL;
cMap* maps=NULL;
cStatics* statics=NULL;
cVerdata* verdata=NULL;
cMulti* multi=NULL;

}



/*!
\brief Constructor
\author Endymion
*/
cFile::cFile( std::string path ) {
	this->path=path;
	file.open( path.c_str(),ios::in|ios::binary );
}

/*!
\brief Destructor
\author Endymion
*/
cFile::~cFile() {
	file.close();
}




/*!
\brief Constructor
\author Endymion
\param idx the path of index file
\param data the path of data file
\param cache if true are cached
\param verdata if valid pointer add verdata patches
*/
template <typename T>
cMULFile<T>::cMULFile( std::string idx, std::string data, bool cache ) {
	this->idx = new cFile( idx );
	this->data = new cFile( data );
	isCached = false;
	if( cache ) 
		loadCache();
}

/*!
\brief Destructor
\author Endymion
*/
template <typename T>
cMULFile<T>::~cMULFile() {
	delete idx;
	delete data;
}

/*!
\brief Get data from given id
\author Endymion
\param id the id
\param data the data
\return true if need delete of vector
*//*
template <typename T>
bool cMULFile<T>::getData( UI32 id, std::vector< T >* data ) {
	
	//ndEndy need because can be into verdata
	std::map< UI32, std::vector<T> >::iterator iter( cache.find( id ) );
	if( iter!=cache.end() ) {
		data=&iter->second;
		return false;
	}

	if( (id==INVALID) || ( isCached ) || ( id*sizeof(TINDEX) >= idx->file.width() ) ) {
		data=NULL;
		return false;
	}

	TINDEX index;
	idx->file.seekg( id*sizeof(TINDEX) );
	idx->file.read( (char*)&index, sizeof(TINDEX) );
	if( index.start==INVALID || index.size==INVALID ) {
		data=NULL;
		return false;
	}

	if( ( index.size % sizeof(T) ) != 0  ) {
		ErrOut( "data corrupted ( index=%i ) in %s ", id, idx->path.c_str() );
		data=NULL;
		return false;
	}

	data = new std::vector<T>;
	this->data->file.seekg( index.start );
	T buffer;
	for( int s=0; s< (index.size % sizeof(T)); ++s ) {
		this->data->file.read( (char*)&buffer, sizeof(T));
		data->push_back( buffer );
	}

	return true;

}*/

/*!
\brief Cache data
\author Endymion
*/
template <typename T> 
void cMULFile<T>::loadCache() {

	if(!isReady() || isCached )
		return;

	int i=INVALID;
	
	idx->file.seekg( 0 );
	while( !idx->file.eof() ) {
		mul_index_st index;
		idx->file.read( (char*)&index, sizeof(mul_index_st) );
		++i;

		if( index.start==INVALID || index.size==INVALID )
			continue;

		if( ( index.size % sizeof(T) ) != 0  ) {
			ErrOut( "data corrupted ( index=%i ) in %s ", i, idx->path.c_str() );
			continue;
		}

		data->file.seekg( index.start );
		int count = index.size / sizeof(T);
		for( int s=0; s<count; ++s ) {
			T buffer;
			data->file.read( (char*)&buffer, sizeof(T));
			cache[ getIndexForCache( i, buffer ) ].push_back( buffer );
		}

	}

	isCached=true;

}







/*!
\brief Constructor
\author Endymion
\param mul the mul file
\param id the id
*/
template <typename T>
NxwMulWrapper<T>::NxwMulWrapper( cMULFile<T>* mul, UI32 i ) {
	idx=i;
	this->mul=mul;
	needFree=false;
	data=NULL;
}


/*
\brief Destructor
\author Endymion
*//*
template <class T>
NxwMulWrapper<T>::~NxwMulWrapper() {
	if( needFree )
		delete data;
}*/

/*
\brief Rewind the set
\author Endymion
*//*
template <typename T>
void NxwMulWrapper<T>::rewind() {
	needFree = mul->getData( i, data );
}*/

/*
\brief Get the size
\author Endymion
\return the size
*//*
template <typename T>
UI32 NxwMulWrapper<T>::size() {
	return (data!=NULL)? data->size() : 0;
}*/

/*
\brief Check if set is at end
\author Endymion
*/
/*template< typename T > 
bool NxwMulWrapper<T>::end() {
	return (data==NULL) || (current==data->end());
}*/

/*
\brief Check if set is empty
\author Endymion
*//*
template <typename T>
bool NxwMulWrapper<T>::isEmpty() {
	return size()<=0;
}*/

/*
\brief Advance set
\author Endymion
*//*
template <typename T>
NxwMulWrapper<T>& NxwMulWrapper<T>::operator++(int) {
	current++;
}*/

/*
\brief Get the value
\author Endymion
*//*
template <typename T>
T NxwMulWrapper<T>::get() {
	return *current;
}*/




NxwMulWrapperStatics::NxwMulWrapperStatics( UI32 x, UI32 y ) : NxwMulWrapper<statics_st>( data::statics, data::statics->getHash(x,y) ) {

};

NxwMulWrapperMulti::NxwMulWrapperMulti( UI32 id ) : NxwMulWrapper<multi_st>( data::multi, id ) {

};



/*!
\brief Constructor
\author Endymion
\param path the path of tiledata.mul
\param cache true cache the tiledata
\param verdata if valid pointer are added verdata infos to tiledata
*/
cTiledata::cTiledata( std::string path, bool cache ) : cFile( path )
{
	isCached=false;
	if( cache ) {
		ConOut("\n    Caching TILEDATA..");
		loadCache();
		ConOut("[DONE]\n");
	}
};

/*!
\brief Destructor
\author Endymion
*/
cTiledata::~cTiledata(  )
{
};

/*!
\brief Check if Tiledata is ready for read
\author Endymion
\return true if ready
*/
bool cTiledata::isReady()
{
	return file.is_open();
};

/*!
\brief Get the land info at given id
\author Endymion
\return bool, true if valid land info
\param id the id
\param land the land info
*/
bool cTiledata::getLand( SERIAL id, land_st& land )
{
	if( !(id>INVALID && id<LANDSINFOCOUNT)) 
		return false;
	
	//need because can be verdata info
	LANDINFOMAP::iterator iter = landsCached.find(id);
	if(iter!=landsCached.end()) {
		land=iter->second;
		return true;
	}
	else { 	
		if( isCached )
			return false;

		UI32 pos= ( id / LANDSINGROUP ) *sizeof( land_group_st ) +
			sizeof( UI32 ) + // TLANDGROUP.header
			( id % LANDSINGROUP ) *sizeof( land_st );
		
		file.seekg( pos );
		file.read((char*)&land, sizeof(land_st));
		return true;
	};
};

/*!
\brief Get the Static info at given id
\author Endymion
\return true if valid static info
\param id the id
\param stat the static info
*/
bool cTiledata::getStatic( SERIAL id, tile_st& stat )
{
	//if( !( (id>(INVALID +FIRSTSTATICSINFO)) && (id<FIRSTSTATICSINFO+STATICSINFOCOUNT))) 
	//	return false;

	//need because can be verdata info
	STATICINFOMAP::iterator iter = tilesCached.find(id);
	if(iter!=tilesCached.end()) {
		stat=iter->second;
		return true;
	}
	else {
		if( isCached )
			return false;
			
		UI32 pos= BASESTATICINFO +
				( id / TILESINGROUP ) *sizeof( tile_group_st ) +
				sizeof( UI32 ) + // TSTATICGROUP.header
				( id % TILESINGROUP ) *sizeof( tile_st );
		
		file.seekg( pos );
		file.read((char*)&stat, sizeof(tile_st));
		return true;
	};
};

/*!
\brief Cache Tiledata
\author Endymion
*/
void cTiledata::loadCache() {

	if(!isReady() || isCached )
		return;

	int i;
	land_group_st landg;
	for ( i=0; i<LANDGROUPCOUNT; i++ ) {
		file.read( (char*)&landg, sizeof(land_group_st) );
		for( int j=0; j<LANDSINGROUP; j++ )
			landsCached[i*LANDSINGROUP+j]=landg.land[j];
	}

	tile_group_st staticg;
	for ( i=0; i<TILEGROUPCOUNT; i++ ) {
		file.read( (char*)&staticg, sizeof(tile_group_st) );
		for( int j=0; j<TILESINGROUP; j++ )
			tilesCached[i*TILESINGROUP+j]=staticg.tile[j];
	}

	isCached=true;

};



/*!
\brief Constructor
\author Endymion
\param path the path of tiledata.mul
\param width the width of the map
\param height the height of the map
\param cache if true are cached
*/
cMap::cMap( std::string path, UI16 width, UI16 height, bool cache ) : cFile( path )
{
	this->width=width;
	this->height=height;
	isCached=false;
}

/*!
\brief Destructor
\author Endymion
*/
cMap::~cMap()
{
}

/*!
\brief Check if map is ready for read
\author Endymion
\return true if ready for read
*/
bool cMap::isReady()
{
	return file.is_open();
}

/*!
\brief Get the map cell to given position
\author Endymion
\return true if map cell
\param x the x location
\param y the y location
\param cella the map cell
*/
bool cMap::getMap( UI16 x, UI16 y, map_st& m )
{
	if( !isReady() )
		return false;

	UI16 blockX = x / 8, blockY = y / 8;
	if( (blockX>=width) || (blockY>=height) )
		return false;

	UI16 cellX = x%8, cellY = y%8;
	UI32 pos =
		// Block position - A block contains 8x8 cells. Blocks are registered in file by top to bottom columns from left to right.
		( blockX * this->height * sizeof(mapblock_st) ) + ( blockY * sizeof(mapblock_st) ) +
		// Header of the block, it doesn't interest us.
		sizeof( UI32 ) +
		// Cell position in block - A cell is a map_st. Cells are registered in blocks by left to right rows from top to bottom.
		( cellY * 8 * sizeof( map_st ) ) + ( cellX * sizeof( map_st ) );

	file.seekg( pos );
	file.read( (char*)&m, sizeof(map_st) );
	return true;
/////////////////////////////
}



/*!
\brief Constructor
\author Endymion
\param path the path of tiledata.mul
\param cache true cache the tiledata
*/
cVerdata::cVerdata( std::string path, bool cache ) : cFile( path )
{
	this->path=path;
	isCached=false;
}

/*!
\brief Destructor
\author Endymion
*/
cVerdata::~cVerdata()
{
}

/*!
\brief Check if ready for read
\author Endymion
\return true if if ready
*/
bool cVerdata::isReady()
{
	return file.is_open();
}

/*!
\brief Cache verdata
\author Endymion
*/
void cVerdata::load( cTiledata* tiledata, cMULFile<multi_st>* multi ) {

	if(!isReady() )
		return;

	SI32 nblocchi=0;
	file.seekg( 0 );
	file.read( (char*)&nblocchi, sizeof( SI32 ) );
	
	for( int i=0; i<nblocchi; i++ ) {
		file.seekg( sizeof(UI32) + i*sizeof(mul_patch_st) );
		mul_patch_st patch;
		file.read( (char*)&patch, sizeof(mul_patch_st) );
		switch( patch.file ) {
			case MUL_MAP:
			case MUL_STATIDX:
			case MUL_STATICS:
				ErrOut("VERDATA contains statics/map data. Ignoring version record.\n");
				break;
			case MUL_MULTIIDX:
			case MUL_MULTI:
				file.seekg( patch.info.start );
				if((patch.info.size % sizeof(multi_st))==0) {
					multi_st m;
					multi->cache.erase( patch.id );
					int count = patch.info.size / sizeof(multi_st);
					for( UI32 j=0; j<count; j++ ) {
						file.read( (char*)&m, sizeof(multi_st) );
						multi->cache[patch.id].push_back(m);
					}
				}
				else 
					ErrOut("VERDATA contains multi data with wrong lenght. Ignoring version record.\n");
				break;
			case MUL_TILEDATA:
				if( patch.id<LANDGROUPCOUNT ) {
					file.seekg( patch.info.start );
					if(patch.info.size==sizeof(land_group_st)) {
						land_group_st landg;
						file.read( (char*)&landg, sizeof(land_group_st) );
						for( int j=0; j<LANDSINGROUP; j++ )
							tiledata->landsCached[(patch.id*LANDSINGROUP)+j]=landg.land[j];
					}
					else 
						ErrOut("VERDATA contains tiledata.land data with wrong lenght. Ignoring version record.\n");
				}
				else if( (patch.id>=LANDGROUPCOUNT) && (patch.id<(LANDGROUPCOUNT+TILEGROUPCOUNT)) ) {
					file.seekg( patch.info.start );
					if(patch.info.size==sizeof(tile_group_st)) {
						tile_group_st staticg;
						file.read( (char*)&staticg, sizeof(tile_group_st) );
						for( int j=0; j<TILESINGROUP; j++ )
							tiledata->tilesCached[(patch.id-LANDGROUPCOUNT)*TILESINGROUP+j]=staticg.tile[j];
					}
					else 
						ErrOut("VERDATA contains tiledata.statics data with wrong lenght. Ignoring version record.\n");
				}
				else ErrOut("VERDATA contains alfa data. Ignoring version record.\n");
			default:
				break;
								
		}
	}

	isCached=true;

}


/*!
\brief Constructor
\author Endymion
\param pathidx the path of statXidx.mul
\param pathdata the path of staticsX.mul
\param cache if true are cached
*/
cMulti::cMulti( std::string pathidx, std::string pathdata, bool cache ) : cMULFile<multi_st>( pathidx, pathdata, false )
{
	if( cache ) {
		ConOut("\n    Caching MULTI..");
		loadCache();
		ConOut("[DONE]\n");
	}
}

/*!
\brief Destructor
\author Endymion
*/
cMulti::~cMulti() 
{
}



/*!
\brief Constructor
\author Endymion
\param pathidx the path of statXidx.mul
\param pathdata the path of staticsX.mul
\param width the width of the map
\param height the height of the map
\param cache if true are cached
*/
cStatics::cStatics( std::string pathidx, std::string pathdata, UI16 width, UI16 height, bool cache ) : cMULFile<statics_st>( pathidx, pathdata, false )
{
	this->width=width;
	this->height=height;
	if( cache ) {
		ConOut("\n    Caching STATICS..");
		loadCache();
		ConOut("[DONE]\n");
	}
}

/*!
\brief Destructor
\author Endymion
*/
cStatics::~cStatics()
{
}

SERIAL cStatics::blockFromXY( UI16 x, UI16 y ) {

	UI16 blockX = x/8, blockY = y/8;
	if( (blockX>=width) || (blockY>=height) ) {
		ErrOut( "Bad static ( x=%i y=%i ) search in map %s ( width=%i height=%i )",x,y,getPath().c_str(),width,height );
		return INVALID;
	}

	return blockX*height+blockY;
}

UI32 cStatics::getIndexForCache( UI32 id, statics_st b ) {

	UI16 blockX = id/height, blockY = id%height;
	return getHash( (blockX*8)+b.x, (blockY*8)+b.y );

}





namespace data {

#define CHECKMUL( A, B ) \
	if ( !A->isReady() ) { \
		LogError( "[ERROR] file not found %s ...\n", B.c_str() ); \
		return; \
	} \
	else ConOut( "[DONE]\n" ); \
		

void init()
{
	//
	// If MULs loading fails, stop the server!
	//
	keeprun = false;

	ConOut("Preparing to open *.mul files...\n(If they don't open, fix your paths in server.cfg)\n");

	ConOut( "Loading TILEDATA ... ");
	tiledata = new cTiledata( tiledata_path, tiledata_cache );
	CHECKMUL( tiledata, tiledata_path );

	ConOut( "Loading MAP ... ");
	maps = new cMap( map_path, map_width, map_height );
	CHECKMUL( maps, map_path );

	ConOut( "Loading STATICS ... ");
	statics = new cStatics( statics_idx_path, statics_path, map_width, map_height, statics_cache );
	CHECKMUL( statics, std::string( statics_idx_path + " or " + statics_path ) );

	ConOut( "Loading MULTI ... ");
	multi = new cMulti( multi_idx_path, multi_path, multi_cache );
	CHECKMUL( multi, std::string( multi_idx_path + " or " + multi_path ) );
	
	ConOut( "Loading VERDATA ... ");
	verdata = new cVerdata( verdata_path );
	CHECKMUL( verdata, verdata_path );
	ConOut("    Caching VERDATA..");
	verdata->load( tiledata, multi );
	ConOut("[DONE]\n");

	ConOut("All *.mul files opened\n\n");
	//
	// MULs loaded, let's keep the server running
	//
	keeprun = true;
}

/*!
\author Luxor
*/
void shutdown()
{
	if ( maps != NULL )
		safedelete( maps );
	if ( statics != NULL )
		safedelete( statics );
	if ( multi != NULL )
		safedelete( multi );
	if ( tiledata != NULL )
		safedelete( tiledata );
	if ( verdata!= NULL )
		safedelete( verdata );
}

/*!
\author Luxor
*/
void setPath( MUL_FILES id, std::string path )
{
	switch ( id )
	{
		case MUL_MAP:
			map_path = path;
			break;
		case MUL_STATIDX:
			statics_idx_path = path;
			break;
		case MUL_STATICS:
			statics_path = path;
			break;
		case MUL_MULTIIDX:
			multi_idx_path = path;
			break;
		case MUL_MULTI:
			multi_path = path;
			break;
		case MUL_TILEDATA:
			tiledata_path = path;
			break;
		case MUL_VERDATA:
			verdata_path = path;
			break;
		default:
			break;
	}
}

/*!
\author Luxor
*/
std::string getPath( MUL_FILES id )
{
	switch ( id )
	{
		case MUL_MAP:
			return map_path;
		case MUL_STATIDX:
			return statics_idx_path;
		case MUL_STATICS:
			return statics_path;
		case MUL_MULTIIDX:
			return multi_idx_path;
		case MUL_MULTI:
			return multi_path;
		case MUL_TILEDATA:
			return tiledata_path;
		case MUL_VERDATA:
			return verdata_path;
		default:
			return std::string("");
	}
}


/*!
\author Luxor
*/
LOGICAL seekMap( UI32 x, UI32 y, map_st& m, UI08 nMap )
{
	
	return maps->getMap( x, y, m );
	
/*	if ( nMap >= maps.size() )
		return false;
	if ( !maps[ nMap ]->isReady() )
		return false;

	UI32 pos;
	UI16 blockX = x / 8, blockY = y / 8, cellX = x % 8, cellY = y % 8;
	pos =
		// Block position - A block contains 8x8 cells. Blocks are registered in file by top to bottom columns from left to right.
		( blockX * ServerScp::g_nMapHeight * MAP_BLOCK_SIZE ) + ( blockY * MAP_BLOCK_SIZE ) +
		// Header of the block, it doesn't interest us.
		MAP_HEADER_SIZE +
		// Cell position in block - A cell is a map_st. Cells are registered in blocks by left to right rows from top to bottom.
		( cellY * 8 * map_st_size ) + ( cellX * map_st_size );

	return maps[ nMap ]->getData( pos, m );*/
}

/*!
\author Luxor
*/
LOGICAL seekLand( UI16 id, land_st& land )
{
	
	return tiledata->getLand( id, land );
	
/*	if ( seekVerLand( id, land ) )
		return true;

	if ( !tdLand->isReady() )
		return false;

	UI16 block = id / 32;

	UI32 pos =
		// Each block contains 32 land_st.
		( (block + 1) * TILE_HEADER_SIZE ) + ( land_st_size * id );

	return tdLand->getData( pos, land );*/
}

/*!
\author Luxor
*/
LOGICAL seekTile( UI16 id, tile_st& tile )
{
	
	return tiledata->getStatic( id, tile );

/*
	if ( seekVerTile( id, tile ) )
		return true;

	if ( !tdTile->isReady() )
		return false;

	UI16 block = id / 32;

	UI32 pos =
		// Go beyond the land_st dedicated space.
		TILEDATA_LAND_SIZE +
		// Each block contains 32 tile_st.
		( (block + 1) * TILE_HEADER_SIZE ) + ( tile_st_size * id );

	return tdTile->getData( pos, tile );*/
}

/*!
\author Luxor
*//*
LOGICAL collectStatics( UI32 x, UI32 y, NxwMulWrapperStatics& s_vec )
{
	if ( !staticIdx->isReady() || !statics->isReady() )
		return false;


	UI16 blockX = x / 8, blockY = y / 8;
	UI32 pos =
		// Block position - A block contains (staticIdx_st.length / static_st_size ) statics.
		// Blocks are registered in file by top to bottom columns from left to right.
		( blockX * ServerScp::g_nMapHeight * staticIdx_st_size ) + ( blockY * staticIdx_st_size );

	staticIdx_st staidx;
	if ( !staticIdx->getData( pos, staidx ) || staidx.start == 0xFFFFFFFF || staidx.start < 0 || staidx.length <= 0 )
		return false;

	UI32 num = staidx.length / static_st_size;
	static_st s;
	UI08 xOffset = x % 8, yOffset = y % 8;
	for ( UI32 i = 0; i < num; i++ ) {
		pos = staidx.start + ( i * static_st_size );
		if ( !statics->getData( pos, s ) )
			continue;
		if ( s.xoff == xOffset && s.yoff == yOffset )
			s_vec.push_back( s );
	}
	return ( s_vec.size() > 0 );
}*/

/*!
\author Luxor
*//*
LOGICAL seekMulti( UI16 id, NxwMulWrapper<multi_st>& m_vec )
{
	
	

	if ( !multiIdx->isReady() || !multi->isReady() )
		return false;

	multiIdx_st idx;
	UI32 pos = id * multiIdx_st_size;
	if ( !multiIdx->getData( pos, idx ) || idx.start == (0xFFFFFFFF - 1) || idx.start < 0 || idx.length <= 0 )
		return false;

	multi_st m;
	UI32 num = idx.length / multi_st_size;
	for ( UI32 i = 0; i < num; i++ ) {
		pos = idx.start + ( i * multi_st_size );
		if ( !multi->getData( pos, m ) )
			continue;
		m_vec.push_back( m );
	}
	return ( m_vec.size() > 0 );
}*/


}

