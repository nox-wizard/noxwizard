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
cMulti* multi=NULL;



cFile::cFile( std::string path ) {
	this->path=path;
	file.open( path.c_str(),ios::in|ios::binary );
}

cFile::~cFile() {
	file.close();
}




template <typename T>
cMULFile<T>::cMULFile( std::string idx, std::string data ) {
	this->idx = new cFile( idx );
	this->data = new cFile( data );
	isCached = false;
}

template <typename T>
cMULFile<T>::~cMULFile() {
	delete idx;
	delete data;
}

template <typename T>
bool cMULFile<T>::getData( UI32 i, std::vector< T >* data ) {
	
	data = new std::vector<T>;
	if( i==INVALID ) 
		return true;

	if( i*sizeof(TINDEX) >= idx->file.width() )
		return true;

	TINDEX index;
	idx->file.seekg( i*sizeof(TINDEX) );
	idx->file.read( (char*)&index, sizeof(TINDEX) );
	if( index.start==INVALID || index.size==INVALID )
		return true;

	if( ( index.size % sizeof(T) ) != 0  ) {
		ErrOut( "data corrupted ( index=%i ) in %s ", i, idx.path.c_str() );
		return true;
	}

	data.seekg( index.start );
	T buffer;
	for( int s=0; s< (index.size % sizeof(T)); ++s ) {
		data.read( (char*)&buffer, sizeof(T));
		data.push_back( buffer );
	}

	return true;

}

template <typename T> 
cMULFileCached<T>::cMULFileCached( std::string idx, std::string data ) {
	loadCache();
}

template <typename T> 
cMULFileCached<T>::~cMULFileCached() {
}


/*!
\brief Cache data
\author Endymion
*/
template <typename T> 
void cMULFileCached<T>::loadCache() {

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

template <typename T> 
bool cMULFileCached<T>::getData( UI32 i, std::vector< T >* data ) {
	
	if( i==INVALID )
		return true;

	std::vector<T>::iterator iter( cache.find( i ) );
	if( iter!=cache.end() ) {
	}


}

template <typename T, typename M>
NxwMulWrapper<T,M>::NxwMulWrapper( cMULFile<M>* mul, UI32 i ) {
	idx=i;
	this->mul=mul;
	needFree=false;
	data=NULL;
}

template <typename T, typename M>
NxwMulWrapper<T,M>::~NxwMulWrapper() {
	if( needFree )
		delete data;
}

template <typename T, typename M>
void NxwMulWrapper<T,M>::rewind() {
	needFree = mul->getData( i );
}

template <typename T, typename M>
UI32 NxwMulWrapper<T,M>::size() {
	return (data!=NULL)? data->size() : 0;
}

template <typename T, typename M>
bool NxwMulWrapper<T,M>::end() {
	return current==data->end();
}

template <typename T, typename M>
bool NxwMulWrapper<T,M>::isEmpty() {
	return size()<=0;
}

template <typename T, typename M>
NxwMulWrapper<T,M>& NxwMulWrapper<T,M>::operator++(int) {
	current++;
}

template <typename T, typename M>
T NxwMulWrapper<T,M>::get() {
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
	addVerdata();
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
void cTiledata::addVerdata() {
	
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
cStatics::cStatics( std::string pathidx, std::string pathdata, UI16 width, UI16 height, bool cache )
{
	width=width;
	height=height;
	if( cache )
		file = new cMULFileCached<statics_st>( pathidx, pathdata );
	else 
		file = new cMULFile<statics_st>( pathidx, pathdata );
}

/*!
\brief Destructor
\author Endymion
*/
cStatics::~cStatics()
{
}

/*!
\brief Check of Statics are ready for read
\author Endymion
\return true if ready
*/
bool cStatics::isReady()
{
	return file->is_open();
}

/*!
\brief Get the Statics at given location
\author Endymion
\return true if valid statics
\param x the x location
\param y the y location
\param stats the statics vector
*/
bool cStatics::getStatics( UI16 x, UI16 y, P_STATICSVET stats )
{ 

	if( x>=width || y>=height ) {
		ErrOut( "Bad static ( x=%i y=%i ) search in map %s ( width=%i height=%i )",x,y,file->getPath().c_str(),width,height );
		return file->getData( INVALID, stats );
	}

	bool needFree = file->getData( x*height+y, stats );


	/*	TINDEX index;
		idx.seekg( (x*height+y)*sizeof(TINDEX) );
		idx.read( (char*)&index, sizeof(TINDEX) );
		if( index.start==INVALID || index.size==INVALID )
			return false;
		else {
			if( ( index.size % sizeof(statics_st) ) != 0 /*|| index.start>data.size()*//* ) {
				ErrOut( "static data corrupted ( x=%i y=%i ) search in map %s ( width=%i height=%i )",x,y,pathdata.c_str(),width,height );
				return false;
			}
			else {
				data.seekg( index.start );
				statics_st buffer;
				for( UI32 s=0; s< (index.size % sizeof(statics_st)); s++ ) {
					data.read( (char*)&buffer, sizeof(statics_st));
					stats.push_back( buffer );
				}
				return true;
			}				
		}
	}*/

}


/*!
\brief Constructor
\author Endymion
\param pathidx the path of multi.idx
\param pathdata the path of multi.mul
\param cache if true are cached
\param verdata if valip pointer add verdata multi
*/
cMulti::cMulti( std::string pathidx, std::string pathdata, bool cache, class cVerdata* verdata )
{
	if( cache )
		file = new cMULFileCached<multi_st>( pathidx, pathdata );
	else
		file = new cMULFile<multi_st>( pathidx, pathdata );

	if( verdata!=NULL )
		addVerdata();
}

/*!
\brief Destructor
\author Endymion
*/
cMulti::~cMulti()
{
}

/*!
\brief Check of Multis are ready for read
\author Endymion
\return true if ready
*/
bool cMulti::isReady()
{
	return file->is_open();
}

void cMulti::addVerdata( )
{
	if(verdata==NULL)
		return;
}

/*!
\brief Get the multi at given id
\author Endymion
\return true if valid multi
\param id the id
\param multi the multi vector
*/
bool cMulti::getMulti( UI32 id, P_MULTISVEC multi )
{ 

	return file->getData( id, multi );
/*	multi.clear();

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

			TINDEX index;
			idx.seekg( id *sizeof(TINDEX) );
			idx.read( (char*)&index, sizeof(TINDEX) );
			if( index.start==INVALID || index.size==INVALID )
				return false;
			else {
				if( ( index.size % sizeof(TMULTI) ) != 0 /*|| index.start>data.size()*/ /*) {
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
		}*/

}


