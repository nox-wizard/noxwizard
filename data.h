  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
#ifndef __DATA_H__
#define __DATA_H__

/*!
\file
\author Luxor
\brief Header of data files handling system
*/

#include "nxwcommn.h"


/*!
\author Luxor
*/
namespace data {

void init( LOGICAL cache );

LOGICAL seekMap( UI32 x, UI32 y, map_st& m, UI08 nMap = 0 ); //<! Luxor: nMap will be used for future multiple maps support.

/*!
\author Luxor
*/
template <typename T> class cMULFile {
public:
	cMULFile( std::string path, std::string mode );
	LOGICAL getData( UI32 index, T& data );
	void setCache( std::map< UI32, T > *cache );
	inline LOGICAL isReady() { return ( m_file != NULL ); }
	inline LOGICAL isCached() { return ( m_cache != NULL ); }
private:
	FILE	*m_file;
	std::map< UI32, T > *m_cache;
};

} // namespace data

#endif // __DATA_H__

