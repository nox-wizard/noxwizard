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


cTiledata* tiledata=NULL;
cMap* mappa=NULL;
cStatics* statics=NULL;
cVerdata* verdata=NULL;
cMULFile<multi_st>* multi=NULL;



cFile::cFile( std::string path ) {
	this->path=path;
	file.open( path.c_str(),ios::in|ios::binary );
}

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
cMULFile<T>::cMULFile( std::string idx, std::string data, bool cache, class cVerdata* verdata ) {
	this->idx = new cFile( idx );
	this->data = new cFile( data );
	isCached = false;
	if( cache ) 
		loadCache();
	if( verdata!=NULL )
		loadVerdata( verdata );
}

template <typename T>
cMULFile<T>::~cMULFile() {
	delete idx;
	delete data;
}

template <typename T>
bool cMULFile<T>::getData( UI32 i, std::vector< T >* data ) {
	
	//ndEndy need because can be into verdata
	std::map< SERIAL, std::vector<T>>::iterator iter( cache.find( i ) );
	if( iter!=cache.end() ) {
		data=iter->second;
		return false;
	}

	if( (i==INVALID) || ( isCached ) || ( i*sizeof(TINDEX) >= idx->file.width() ) ) {
		data=NULL;
		return false;
	}

	TINDEX index;
	idx->file.seekg( i*sizeof(TINDEX) );
	idx->file.read( (char*)&index, sizeof(TINDEX) );
	if( index.start==INVALID || index.size==INVALID ) {
		data=NULL;
		return false;
	}

	if( ( index.size % sizeof(T) ) != 0  ) {
		ErrOut( "data corrupted ( index=%i ) in %s ", i, idx.path.c_str() );
		data=NULL;
		return false;
	}

	data = new std::vector<T>;
	data.seekg( index.start );
	T buffer;
	for( int s=0; s< (index.size % sizeof(T)); ++s ) {
		data.read( (char*)&buffer, sizeof(T));
		data.push_back( buffer );
	}

	return true;

}

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
		TINDEX index;
		idx->file.read( (char*)&index, sizeof(TINDEX) );
		++i;

		if( index.start==INVALID || index.size==INVALID )
			continue;

		if( ( index.size % sizeof(T) ) != 0  ) {
			ErrOut( "data corrupted ( index=%i ) in %s ", i, idx.path.c_str() );
			continue;
		}

		data.seekg( index.start );
		for( int s=0; s< (index.size % sizeof(T)); ++s ) {
			T baffer;
			data.read( (char*)&baffer, sizeof(T));
			cache[i].push_back( baffer );
		}

	}

	isCached=true;

}

/*!
\brief Cache data
\author Endymion
*/
template <typename T> 
void cMULFile<T>::loadVerdata() {
}

template <typename T>
NxwMulWrapper<T>::NxwMulWrapper( cMULFile<T>* mul, UI32 i ) {
	idx=i;
	this->mul=mul;
	needFree=false;
	data=NULL;
}

template <typename T>
NxwMulWrapper<T>::NxwMulWrapper( cStatics* statics, UI32 x, UI32 y ) {
	idx=statics->idFromXY(x,y);
	this->mul=mul;
	needFree=false;
	data=NULL;
}


template <typename T>
NxwMulWrapper<T>::~NxwMulWrapper() {
	if( needFree )
		delete data;
}

template <typename T>
void NxwMulWrapper<T>::rewind() {
	needFree = mul->getData( i, data );
}

template <typename T>
UI32 NxwMulWrapper<T>::size() {
	return (data!=NULL)? data->size() : 0;
}

template <typename T>
bool NxwMulWrapper<T>::end() {
	return (data==NULL) || (current==data->end());
}

template <typename T>
bool NxwMulWrapper<T>::isEmpty() {
	return size()<=0;
}

template <typename T>
NxwMulWrapper<T>& NxwMulWrapper<T>::operator++(int) {
	current++;
}

template <typename T>
T NxwMulWrapper<T>::get() {
	return *current;
}
















/*!
\brief Constructor
\author Endymion
\param path the path of tiledata.mul
\param cache true cache the tiledata
\param verdata if valid pointer are added verdata infos to tiledata
*/
cTiledata::cTiledata( std::string path, bool cache, class cVerdata* verdata ) : cFile( path )
{
	isCached=false;
	if( cache )
		loadForCaching();
	if( verdata!=NULL )
		addVerdata(verdata);
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
bool cTiledata::getLand( SERIAL id, TLANDINFO& land )
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
	else {
		if( isCached )
			return false;
			
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

	int i;
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
void cTiledata::addVerdata( cVerdata* verdata ) {
	
	if(verdata==NULL)
		return;

	LANDINFOMAP::iterator land( verdata->landsCached.begin() ), landend( verdata->landsCached.end() );
	for( ; land!=landend; land++ ) {
		landsCached[land->first]=land->second;
	}
	verdata->landsCached.clear(); //why cache it two times?


	STATICINFOMAP::iterator stat( verdata->staticsCached.begin() ), statend( verdata->staticsCached.end() );
	for( ; stat!=statend; stat++ ) {
		staticsCached[stat->first]=stat->second;
	}
	verdata->staticsCached.clear(); //why cache it two times?
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
bool cMap::getMap( UI16 x, UI16 y, TCELLA& cella )
{
	if( !isReady() )
		return false;

	if( x>=width || y>=height )
		return false;

	file.seekg( x*height +y );
	file.read( (char*)&cella, sizeof(TCELLA) );
	return true;
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
	if( cache )
		loadForCaching();
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
			case VF_MAP:
			case VF_STAIDX:
			case VF_STATICS:
				ErrOut("VERDATA contains statics/map data. Ignoring version record.\n");
				break;
			case VF_MULTIIDX:
			case VF_MULTI:
				file.seekg( patch.info.start );
				if((patch.info.size % sizeof(TMULTI))==0) {
					TMULTI multi;
					for( UI32 j=0; j<(patch.info.size % sizeof(TMULTI)); j++ ) {
						file.read( (char*)&multi, sizeof(TMULTI) );
						multisCached[patch.id+j].push_back(multi);
					}
				}
				else 
					ErrOut("VERDATA contains multi data with wrong lenght. Ignoring version record.\n");
				break;
			case VF_TILEDATA:
				if( patch.id<LANDGROUPCOUNT ) {
					file.seekg( patch.info.start );
					if(patch.info.size==sizeof(TLANDGROUP)) {
						TLANDGROUP landg;
						file.read( (char*)&landg, sizeof(TLANDGROUP) );
						for( int j=0; j<LANDINGROUP; j++ )
							landsCached[(patch.id*LANDINGROUP)+j]=landg.lands[j];
					}
					else 
						ErrOut("VERDATA contains tiledata.land data with wrong lenght. Ignoring version record.\n");
				}
				else if( (patch.id>=LANDGROUPCOUNT) && (patch.id<(LANDGROUPCOUNT+STATICGROUPCOUNT)) ) {
					file.seekg( patch.info.start );
					if(patch.info.size==sizeof(TSTATICGROUP)) {
						TSTATICGROUP staticg;
						file.read( (char*)&staticg, sizeof(TSTATICGROUP) );
						for( int j=0; j<STATICINGROUP; j++ )
							staticsCached[(patch.id*STATICINGROUP)+j]=staticg.statics[j];
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
\param width the width of the map
\param height the height of the map
\param cache if true are cached
*/
cStatics::cStatics( std::string pathidx, std::string pathdata, UI16 width, UI16 height, bool cache, cVerdata* verdata ) : cMULFile<statics_st>( pathidx, pathdata, cache, verdata )
{
	width=width;
	height=height;
}

/*!
\brief Destructor
\author Endymion
*/
cStatics::~cStatics()
{
}

/*!
\brief Get the Statics at given location
\author Endymion
\return true if valid statics
\param x the x location
\param y the y location
\param stats the statics vector
*/
bool cStatics::getData( UI16 x, UI16 y, std::vector<statics_st>* stats )
{ 

	SERIAL id=idFromXY( x, y );
	if( id==INVALID ) {
		stats = NULL;
		return false;
	}

	return cMULFile<statics_st>::getData( id, stats );

}


SERIAL cStatics::idFromXY( UI16 x, UI16 y ) {

	if( x>=width || y>=height ) {
		ErrOut( "Bad static ( x=%i y=%i ) search in map %s ( width=%i height=%i )",x,y,getPath().c_str(),width,height );
		return INVALID;
	}

	return x*height+y;
}



