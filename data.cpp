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
\author Luxor
\brief Data files handling system
\todo finish it - experimental yet
*/

#include "data.h"

namespace data {


cMULFile< map_st > *Map;

/*!
\author Luxor
*/
static void cacheData()
{
	//
	// Map caching
	//
	Map->setCache( NULL );
	map_st m;
	std::map< UI32, map_st > *cache = new std::map< UI32, map_st >;
	UI16 x1, y1, x2, y2;
	UI32 x, y, pos;
	for ( x = 0; x < ( ServerScp::g_nMapWidth * 8 ); x++ ) {
		x1 = x / 8;
		x2 = x % 8;
		for ( y = 0; y < ( ServerScp::g_nMapHeight * 8 ); y++ ) {
			y1 = y / 8;
			y2 = y % 8;
			pos = ( x1 * ServerScp::g_nMapHeight * 196 ) + ( y1 * 196 ) + ( y2 * 24 ) + ( x2 * 3 ) + 4;
			if ( Map->getData( pos, m ) )
				cache->insert( make_pair( pos, m ) );
		}
	}
	Map->setCache( cache );
}


/*!
\author Luxor
*/
void init( LOGICAL cache )
{

	Map = new cMULFile< map_st > ( "./mul/map0.mul", "rb" );
	if ( cache )
		cacheData();
}



/*!
\author Luxor
*/
LOGICAL seekMap( UI32 x, UI32 y, map_st& m, UI08 nMap )
{
	if ( !Map->isReady() )
		return false;

	UI16 x1 = x / 8, y1 = y / 8, x2 = x % 8, y2 = y % 8;
	UI32 pos = ( x1 * ServerScp::g_nMapHeight * 196 ) + ( y1 * 196 ) + ( y2 * 24 ) + ( x2 * 3 ) + 4;
	return Map->getData( pos, m );
}



/*!
\author Luxor
*/
template <typename T>
cMULFile<T>::cMULFile( std::string path, std::string mode )
{
	m_file = NULL;
	m_cache = NULL;
	m_file = fopen( path.c_str(), mode.c_str() );
}

/*!
\author Luxor
*/
template <typename T>
LOGICAL cMULFile<T>::getData( UI32 index, T& data )
{
	if ( !isReady() )
		return false;
	if ( !isCached() ) {
		fseek( m_file, index, SEEK_SET );
		return ( fread( &data, sizeof( T ), 1, m_file ) == 0 );
	}

	typename std::map< UI32, T >::iterator it = m_cache->find( index );
	if ( it == m_cache->end() )
		return false;

	data = it->second;
	return true;
}

/*!
\author Luxor
*/
template <typename T>
void cMULFile<T>::setCache( typename std::map< UI32, T > *cache )
{
	if ( m_cache != NULL )
		safedelete( m_cache );
	m_cache = cache;
}


} // namespace data

