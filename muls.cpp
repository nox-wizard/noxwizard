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

namespace tiledata {

std::string path; //!< path


/*!
\brief Constructor
\author Endymion
\param path the path of tiledata.mul
\param cache true cache the tiledata
\param verdata if valid pointer are added verdata infos to tiledata
*/
cTiledata::cTiledata( const char* path, bool cache, class cVerdata* verdata )
{
	this->path=path;
	file.open(this->path.c_str(),ios::in|ios::binary);
	this->isCached=false;
	if( cache )
		loadForCaching();
	addVerdata();
};

/*!
\brief Destructor
\author Endymion
*/
cTiledata::~cTiledata(  )
{
	file.close();
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
bool cTiledata::getLand( SERIAL id, TLANDINFO& land )
{
	if( !(id>INVALID && id<LANDSINFOCOUNT)) 
		return false;
	LANDINFOMAP::iterator iter = landsCached.find(id);
	if(iter!=landsCached.end()) {
		land=iter->second;
		return true;
	}
	else
		if(isCached)
			return false;
		else {
			UI32 pos= ( id / LANDINGROUP ) *sizeof( TLANDGROUP ) +
				sizeof( UI32 ) + // TLANDGROUP.header
				( id % LANDINGROUP ) *sizeof( TLANDINFO );
		
			file.seekg( pos );
			file.read((char*)&land, sizeof(TLANDINFO));
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
bool cTiledata::getStatic( SERIAL id, TSTATICINFO& stat )
{
	if( !( (id>(INVALID +FIRSTSTATICSINFO)) && (id<FIRSTSTATICSINFO+STATICSINFOCOUNT))) 
		return false;

	//need because can be verdata info
	STATICINFOMAP::iterator iter = staticsCached.find(id);
	if(iter!=staticsCached.end()) {
		stat=iter->second;
		return true;
	}
	else
		if( isCached )
			return false;
		else {
			UI32 pos= LANDGROUPCOUNT *sizeof(TLANDGROUP)+
					( id / STATICINGROUP ) *sizeof( TSTATICGROUP ) +
					sizeof( UI32 ) + // TSTATICGROUP.header
					( id % STATICINGROUP ) *sizeof( TSTATICINFO );
		
			file.seekg( pos );
			file.read((char*)&stat, sizeof(TSTATICINFO));
			return true;
		};
};

/*!
\brief Cache Tiledata
\author Endymion
*/
void cTiledata::loadForCaching() {

	if(!isReady() || isCached )
		return;

	UI32 i;
	TLANDGROUP landg;
	for ( i=0; i<LANDGROUPCOUNT; i++ ) {
		file.read( (char*)&landg, sizeof(TLANDGROUP) );
		for( int j=0; j<LANDINGROUP; j++ )
			landsCached[i*LANDINGROUP+j]=landg.lands[j];
	}

	TSTATICGROUP staticg;
	for ( i=0; i<STATICGROUPCOUNT; i++ ) {
		file.read( (char*)&staticg, sizeof(TSTATICGROUP) );
		for( int j=0; j<STATICINGROUP; j++ )
			staticsCached[i*STATICINGROUP+j]=staticg.statics[j];
	}

	isCached=true;

};

/*!
\brief Add all tiledata records contained in verdata
\author Endymion
*/
void cTiledata::addVerdata() {
	
	if(!verdata::verdata)
		return;

	LANDINFOMAP::iterator land( verdata::verdata->landsCached.begin() ), landend( verdata::verdata->landsCached.end() );
	for( ; land!=landend; land++ ) {
		this->landsCached[land->first]=land->second;
	}
	verdata::verdata->landsCached.clear(); //why cache it two times?


	STATICINFOMAP::iterator stat( verdata::verdata->staticsCached.begin() ), statend( verdata::verdata->staticsCached.end() );
	for( ; stat!=statend; stat++ ) {
		staticsCached[stat->first]=stat->second;
	}
	verdata::verdata->staticsCached.clear(); //why cache it two times?
};


cTiledata* tiledata=NULL;


}

namespace map_uo {

std::string path; //!< path

cMap* mappa=NULL;

/*!
\brief Constructor
\author Endymion
\param path the path of tiledata.mul
\param width the width of the map
\param height the height of the map
\param cache if true are cached
*/
cMap::cMap( const char* path, UI16 width, UI16 height, bool cache )
{
	this->width=width;
	this->height=height;
	this->path=path;
	file.open(this->path.c_str(),ios::in|ios::binary);
	this->isCached=false;
}

/*!
\brief Destructor
\author Endymion
*/
cMap::~cMap()
{
	file.close();
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
bool cMap::getMap( UI16 x, UI16 y, TCELLA& cella )
{
	if( !isReady() )
		return false;

	if( x>=this->width || y>=this->height )
		return false;

	file.seekg( x*this->height +y );
	file.read( (char*)&cella, sizeof(TCELLA) );
	return true;
}


}


namespace verdata {

std::string path; //!< path

cVerdata* verdata=NULL;

/*!
\brief Constructor
\author Endymion
\param path the path of tiledata.mul
\param cache true cache the tiledata
*/
cVerdata::cVerdata( const char* path, bool cache )
{
	this->path=path;
	file.open(this->path.c_str(),ios::in|ios::binary);
	this->isCached=false;
	if( cache )
		loadForCaching();
}

/*!
\brief Destructor
\author Endymion
*/
cVerdata::~cVerdata()
{
	file.close();
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
\return void
*/
void cVerdata::loadForCaching() {

	if(!isReady() || isCached )
		return;

	SI32 nblocchi=INVALID;
	file.seekg( 0 );
	file.read( (char*)&nblocchi, sizeof( SI32 ) );
	
	for( int i=0; i<nblocchi; i++ ) {
		file.seekg( sizeof(UI32) + i*sizeof(TPATCH) );
		TPATCH patch;
		file.read( (char*)&patch, sizeof(patch) );
		switch( patch.file ) {
			case MAP:
			case STAIDX:
			case STATICS:
				ErrOut("VERDATA contains statics/map data. Ignoring version record.\n");
				break;
			case MULTIIDX:
			case MULTI:
				file.seekg( patch.info.start );
				if((patch.info.size % sizeof(multi::TMULTI))==0) {
					multi::TMULTI multi;
					for( UI32 j=0; j<(patch.info.size % sizeof(multi::TMULTI)); j++ ) {
						file.read( (char*)&multi, sizeof(multi::TMULTI) );
						multisCached[patch.id+j].push_back(multi);
					}
				}
				else 
					ErrOut("VERDATA contains multi data with wrong lenght. Ignoring version record.\n");
				break;
			case TILEDATA:
				if( patch.id<tiledata::LANDGROUPCOUNT ) {
					file.seekg( patch.info.start );
					if(patch.info.size==sizeof(tiledata::TLANDGROUP)) {
						tiledata::TLANDGROUP landg;
						file.read( (char*)&landg, sizeof(tiledata::TLANDGROUP) );
						for( int j=0; j<tiledata::LANDINGROUP; j++ )
							landsCached[(patch.id*tiledata::LANDINGROUP)+j]=landg.lands[j];
					}
					else 
						ErrOut("VERDATA contains tiledata.land data with wrong lenght. Ignoring version record.\n");
				}
				else if( (patch.id>=tiledata::LANDGROUPCOUNT) && (patch.id<(tiledata::LANDGROUPCOUNT+tiledata::STATICGROUPCOUNT)) ) {
					file.seekg( patch.info.start );
					if(patch.info.size==sizeof(tiledata::TSTATICGROUP)) {
						tiledata::TSTATICGROUP staticg;
						file.read( (char*)&staticg, sizeof(tiledata::TSTATICGROUP) );
						for( int j=0; j<tiledata::STATICINGROUP; j++ )
							staticsCached[(patch.id*tiledata::STATICINGROUP)+j]=staticg.statics[j];
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


}

namespace statics {

std::string pathidx; //!< path index
std::string pathdata; //!< path data


cStatics* statics=NULL;

/*!
\brief Constructor
\author Endymion
\param pathidx the path of statXidx.mul
\param pathdata the path of staticsX.mul
\param width the width of the map
\param height the height of the map
\param cache if true are cached
*/
cStatics::cStatics( const char* pathidx, const char* pathdata, UI16 width, UI16 height, bool cache )
{
	this->width=width;
	this->height=height;
	this->pathidx=pathidx;
	idx.open(this->pathidx.c_str(),ios::in|ios::binary);
	this->pathdata=pathdata;
	data.open(this->pathdata.c_str(),ios::in|ios::binary);
	this->isCached=false;
	if( cache )
		loadForCaching();
}

/*!
\brief Destructor
\author Endymion
*/
cStatics::~cStatics()
{
	idx.close();
	data.close();
}

/*!
\brief Check of Statics are ready for read
\author Endymion
\return true if ready
*/
bool cStatics::isReady()
{
	return ( idx.is_open() && data.is_open() );
}

/*!
\brief Cache Statics
\author Endymion
\return void
*/
void cStatics::loadForCaching() {

	if(!isReady() || isCached )
		return;

	verdata::TINDEX index; TSTATICS buffer;
	for( int x=0; x<width; x++ ) {
		for( int y=0; y<height; y++ ) {
			idx.read( (char*)&index, sizeof(verdata::TINDEX) );
			if( index.start==INVALID || index.size==INVALID )
				continue;
			else {
				if( ( index.size % sizeof(TSTATICS) ) != 0 /*|| index.start>data.size()*/ ) {
					ErrOut( "Static data corrupted in %s, skipping.. ",pathdata.c_str());
					continue;
				}
				else {
					data.seekg( index.start );
					this->staticsCached[x*this->height+y].clear();
					for( UI32 s=0; s< (index.size % sizeof(TSTATICS)); s++ ) {
						
						data.read( (char*)&buffer, sizeof(TSTATICS));
						this->staticsCached[x*this->height+y].push_back( buffer );
						
					}
				}				
			}
		}
	}

	isCached=true;

}

/*!
\brief Get the Statics at given location
\author Endymion
\return true if valid statics
\param x the x location
\param y the y location
\param stats the statics vector
*/
bool cStatics::getStatics( UI16 x, UI16 y, STATICSVET& stats )
{ 
	stats.clear();
	if( this->isCached ) { 
		STATICSMAP::iterator iter( staticsCached.find(x*height+y) );
		if( iter!=staticsCached.end() ) {
			stats = iter->second;
			return true;
		}
		else
			return false;
	}
	else {
		if( x>=width || y>=height ) {
			ErrOut( "Bad static ( x=%i y=%i ) search in map %s ( width=%i height=%i )",x,y,pathidx.c_str(),width,height );
			return false;
		}
		if( !isReady() )
			return false;

		verdata::TINDEX index;
		idx.seekg( (x*height+y)*sizeof(verdata::TINDEX) );
		idx.read( (char*)&index, sizeof(verdata::TINDEX) );
		if( index.start==INVALID || index.size==INVALID )
			return false;
		else {
			if( ( index.size % sizeof(TSTATICS) ) != 0 /*|| index.start>data.size()*/ ) {
				ErrOut( "static data corrupted ( x=%i y=%i ) search in map %s ( width=%i height=%i )",x,y,pathdata.c_str(),width,height );
				return false;
			}
			else {
				data.seekg( index.start );
				TSTATICS buffer;
				for( UI32 s=0; s< (index.size % sizeof(TSTATICS)); s++ ) {
					data.read( (char*)&buffer, sizeof(TSTATICS));
					stats.push_back( buffer );
				}
				return true;
			}				
		}
	}

}

}


namespace multi {

std::string pathidx; //!< path index
std::string pathdata; //!< path data

cMulti* multi=NULL;

/*!
\brief Constructor
\author Endymion
\param pathidx the path of multi.idx
\param pathdata the path of multi.mul
\param cache if true are cached
\param verdata if valip pointer add verdata multi
*/
cMulti::cMulti( const char* pathidx, const char* pathdata, bool cache, class cVerdata* verdata )
{
	this->pathidx=pathidx;
	idx.open(this->pathidx.c_str(),ios::in|ios::binary);
	this->pathdata=pathdata;
	data.open(this->pathdata.c_str(),ios::in|ios::binary);
	this->isCached=false;
	if( cache )
		loadForCaching();
	if( verdata!=NULL )
		addVerdata();
}

/*!
\brief Destructor
\author Endymion
*/
cMulti::~cMulti()
{
	idx.close();
	data.close();
}

/*!
\brief Check of Multis are ready for read
\author Endymion
\return true if ready
*/
bool cMulti::isReady()
{
	return ( idx.is_open() && data.is_open() );
}

/*!
\brief Cache Multis
\author Endymion
\return void
*/
void cMulti::loadForCaching() {

	if(!isReady() || isCached )
		return;

	verdata::TINDEX index; TMULTI buffer; int id=0;
	while( !idx.eof() ) {
		idx.read( (char*)&index, sizeof(verdata::TINDEX) );
		if( index.start==INVALID || index.size==INVALID )
			continue;
		else {
			if( ( index.size % sizeof(TMULTI) ) != 0 /*|| index.start>data.size()*/ ) {
				ErrOut( "Multis data corrupted in %s, skipping.. ",pathdata.c_str());
				continue;
			}
			else {
				data.seekg( index.start );
				this->multisCached[id].clear();
				for( UI32 s=0; s< (index.size % sizeof(TMULTI)); s++ ) {
				
					data.read( (char*)&buffer, sizeof(TMULTI));
					this->multisCached[id].push_back( buffer );
					
				}
			}				
		}

		id++;
		
	}

	isCached=true;

}

void cMulti::addVerdata( )
{
	if(verdata::verdata==NULL)
		return;
	MULTISMAP::iterator iter( verdata::verdata->multisCached.begin() ), end(verdata::verdata->multisCached.end() );
	for( ; iter!=end; iter++ ) {
		multisCached[iter->first]=iter->second;
	}
	verdata::verdata->multisCached.clear();
}

/*!
\brief Get the multi at given id
\author Endymion
\return true if valid multi
\param id the id
\param multi the multi vector
*/
bool cMulti::getMulti( UI32 id, MULTISVEC& multi )
{ 
	multi.clear();

	MULTISMAP::iterator iter( multisCached.find(id) );
	if( iter!=multisCached.end() ) {
		multi = iter->second;
		return true;
	}
	else
		if( isCached )
			return false;
		else {

			if( !isReady() )
				return false;

			verdata::TINDEX index;
			idx.seekg( id *sizeof(verdata::TINDEX) );
			idx.read( (char*)&index, sizeof(verdata::TINDEX) );
			if( index.start==INVALID || index.size==INVALID )
				return false;
			else {
				if( ( index.size % sizeof(TMULTI) ) != 0 /*|| index.start>data.size()*/ ) {
					ErrOut( "Multi data corrupted ( id=%i ) in %s",id,pathdata.c_str() );
					return false;
				}
				else {
					data.seekg( index.start );
					TMULTI buffer;
					for( UI32 s=0; s< (index.size % sizeof(TMULTI)); s++ ) {
						data.read( (char*)&buffer, sizeof(TMULTI));
						multi.push_back( buffer );
					}
					return true;
				}				
			}
		}

}

}

namespace mulmanager {

inline SI16 StaticTop( Location where )	
{
	return StaticTop( where.x, where.y, where.z );
}

SI16 StaticTop( UI16 x, UI16 y, SI16 oldz )
{

	statics::STATICSVET v;

	if(!statics::statics->getStatics( x, y, v ))
		return illegal_z;

	SI16 top=illegal_z;

	statics::STATICSVET::iterator iter( v.begin() ), end( v.end() );

	for( ; iter!=end; iter++ ) {
		SI16 temp = iter->altitudine + TileHeight( iter->id );
		if( ( temp <= oldz + MaxZstep ) && temp > top )
			top=temp;
	}

	return top;

}

inline SI16 DynamicElevation(Location where)
{
	return DynamicElevation( where.x, where.y, where.z );
}

SI16 DynamicElevation( UI16 x, UI16 y, SI16 oldz )
{
	return illegal_z;
}
	
SI16 MapElevation( UI16 x, UI16 y )
{
	return illegal_z;
}
	
inline SI16 AverageMapElevation(Location where, int &id)
{
	return AverageMapElevation( where.x, where.y, id );
}

SI16 AverageMapElevation( UI16 x, UI16 y, int &id)
{
	return illegal_z;
}

inline SI16 Height(Location where)
{
	return Height( where.x, where.y, where.z );
}

SI16 Height( UI16 x, UI16 y, SI16 oldz )
{
	SI16 dynz = DynamicElevation(x, y, oldz);
	if (illegal_z != dynz)
		return dynz;

	SI16 staticz = StaticTop(x, y, oldz);
	if (illegal_z != staticz)
		return staticz;

	return MapElevation(x, y);

}

SI16 TileHeight( UI32 tilenum )
{
	/*tiledata::TT tile;

	SeekTile(tilenum, &tile);

	// For Stairs+Ladders
	if (tile.flag2 & 4)
		return (signed char)(tile.height/2);	// hey, lets just RETURN half!
	return (tile.height);*/
	return 15;

}



}
