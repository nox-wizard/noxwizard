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

enum {
	MAP_HEADER_SIZE = 4, MAP_BLOCK_SIZE = 196,
	TILE_HEADER_SIZE = 4, TILEDATA_LAND_SIZE = 0x68800,
	VERDATA_HEADER_SIZE = 4
};

enum VerFile {
	VerMap = 0x00,
	VerStaIdx,
	VerStatics,
	VerArtIdx,
	VerArt,
	VerAnimIdx,
	VerAnim,
	VerSoundIdx,
	VerSound,
	VerTexIdx,
	VerTexMaps,
	VerGumpIdx,
	VerGumpArt,
	VerMultiIdx,
	VerMulti,
	VerSkillsIdx,
	VerSkills,
	VerTileData = 0x1E,
	VerAnimData
};

enum MulFileId {
	Map_File = 0, StaIdx_File, Statics_File, Multi_File, MultiIdx_File, TileData_File, VerData_File
};

/*struct map_st {
	UI16 id;
	SI08 z;
} PACK_NEEDED;

struct staticIdx_st {
	SI32 start;
	SI32 length;
	SI32 unknown;
} PACK_NEEDED;

struct static_st {
	UI16 id;
	UI08 xoff;
	UI08 yoff;
	SI08 z;
	UI16 unknown;
} PACK_NEEDED;

typedef std::vector< static_st > staticVector;

struct land_st
{
	UI32 flags;
	UI16 id;
	TEXT name[20];
} PACK_NEEDED;

struct tile_st
{
	UI32 flags;
	UI08 weight;
	UI08 quality;
	UI16 unknown;
	UI08 unknown1;
	UI08 quantity;
	UI16 animid;
	UI08 unknown2;
	UI08 hue;
	UI16 unknown3;
	UI08 height;
	TEXT name[20];
} PACK_NEEDED;


struct multiIdx_st {
	SI32 start;
	SI32 length;
	SI32 unknown;
} PACK_NEEDED;

struct multi_st {
	SI16 block;
	SI16 x;
	SI16 y;
	SI16 height;
	UI32 flags;
} PACK_NEEDED;

typedef std::vector< multi_st > multiVector;

struct verdata_st
{
	SI32 fileid;
	SI32 block;
	SI32 pos;
	SI32 size;
	SI32 various;
} PACK_NEEDED;*/


const UI08 verdata_st_size = sizeof( verdata_st );
const UI08 multi_st_size = sizeof( multi_st );
const UI08 multiIdx_st_size = sizeof( multiIdx_st );
const UI08 tile_st_size = sizeof( tile_st );
const UI08 land_st_size = sizeof( land_st );
const UI08 static_st_size = sizeof( static_st );
const UI08 staticIdx_st_size = sizeof( staticIdx_st );
const UI08 map_st_size = sizeof( map_st );


/*!
\author Luxor
*/
namespace data {

void init();
void shutdown();
void setPath( MulFileId id, std::string path );

LOGICAL seekMap( UI32 x, UI32 y, map_st& m, UI08 nMap = 0 ); //<! Luxor: nMap will be used for future multiple maps support.
LOGICAL collectStatics( UI32 x, UI32 y, staticVector& s_vec );
LOGICAL seekLand( UI16 id, land_st& land );
LOGICAL seekTile( UI16 id, tile_st& tile );
LOGICAL seekMulti( UI16 id, multiVector& m_vec );
LOGICAL seekVerTile( UI16 id, tile_st& tile );
LOGICAL seekVerLand( UI16 id, land_st& land );

/*!
\author Luxor
*/
template <typename T> class cMULFile {
public:
	cMULFile( std::string path, std::string mode );
	~cMULFile() {
		if ( m_file != NULL )
			fclose( m_file );
		if ( m_cache != NULL )
			safedelete( m_cache );
	}
	LOGICAL getData( UI32 index, T& data );
	LOGICAL getData( UI32 index, BYTE* ptr, UI32 size );
	void setCache( std::map< UI32, T > *cache );
	LOGICAL eof();
	inline LOGICAL isReady() { return ( m_file != NULL ); }
	inline LOGICAL isCached() { return ( m_cache != NULL ); }
private:
	FILE	*m_file;
	std::map< UI32, T > *m_cache;
};

} // namespace data

#endif // __DATA_H__

