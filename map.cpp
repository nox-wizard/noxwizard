  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "nxwcommn.h"

/*!
\file
\author Luxor
\brief Map related functions
*/


/*!
\author Luxor
\brief Constructs a line from a location A to a location B
*/
cLine::cLine( Location A, Location B )
{
	m_xDist = SI32( A.x - B.x );
	m_yDist = SI32( A.y - B.y );
	m_zDist = SI32( A.z - B.z );
	x1 = A.x;
	y1 = A.y;
	z1 = A.z;
}

/*!
\author Luxor
*/
SI08 cLine::calcZAtX( UI32 x )
{
	SI08 z;
	z = SI08( round( R32( ( SI32(x - x1) * m_zDist ) + (z1 * m_xDist) ) / R32( m_xDist ) ) );
	return z;
}

/*!
\author Luxor
*/
Location cLine::getPosAtX( UI32 x )
{
	Location pos = Loc( x, 0, 0 );
	pos.y = UI32( round( R32( ( SI32(x - x1) * m_yDist ) + ( SI32(y1) * m_xDist) ) / R32( m_xDist ) ) );
	pos.z = calcZAtX( pos.x );
	return pos;
}

/*!
\author Luxor
*/
Location cLine::getPosAtY( UI32 y )
{
	Location pos = Loc( 0, y, 0 );
	pos.x = UI32( round( R32( ( SI32(y - y1) * m_xDist ) + ( SI32(x1) * m_yDist) ) / R32( m_yDist ) ) );
	pos.z = calcZAtX( pos.x );
	return pos;
}



/*!
\author Luxor
\brief Looks if a char can walk on the given Location
\return The next z value of char position, illegal_z if the tile isn't walkable
*/
SI08 isWalkable( Location pos, UI08 flags )
{
	SI08 zRes = 0;
	UI32 height = 0;

	//
        // DYNAMIC ITEMS -- Check for dynamic items Z elevation and block flag
        //
	if ( flags & WALKFLAG_DYNAMIC ) {
		NxwItemWrapper si;
		P_ITEM pi = NULL;
		si.fillItemsAtXY( pos );

		for( si.rewind(); !si.isEmpty(); si++ )	{
			pi = si.getItem();
			if( !ISVALIDPI(pi) )
				continue;

			tile_st tile;
			Map->SeekTile( pi->id(), &tile );

			height = tile.height;
			if ( tile.flag2 & 0x4 ) // Stairs, ladders
				height = tile.height / 2;

                	if ( pi->getPosition().z < (pos.z + MaxZstep) ) { // We cannot walk under it
				if ( tile.flag1 & 0x40 ) // Block flag
					return illegal_z;

				if ( (pi->getPosition().z + height) <= (pos.z + 2) ) { // We can walk on it
	                                if ( (pi->getPosition().z + height) > zRes )
						zRes = pi->getPosition().z + height;
				} else// if ( pi->type != 12 ) // Doors can be opened or avoided by passing under them
					return illegal_z;
                	}
		}
	} // WALKFLAG_DYNAMIC

        //
        // MAP -- Check for map Z elevation and denied textures (as water, mountains etc)
        //
	if ( flags & WALKFLAG_MAP ) {
		SI32 mapid = 0;
		map_st map1 = Map->SeekMap0( pos.x, pos.y );
		mapid = map1.id;

		// Z elevation
		if ( map1.z == illegal_z || map1.z > (pos.z + 2) )
			return illegal_z;
		else if ( map1.z > zRes )
			zRes = map1.z;

		// Denied textures
		if (
			(mapid >= 0x00DC && mapid <= 0x00E7) ||
			(mapid >= 0x00EC && mapid <= 0x00F7) ||
			(mapid >= 0x00FC && mapid <= 0x0107) ||
			(mapid >= 0x010C && mapid <= 0x0117) ||
			(mapid >= 0x011E && mapid <= 0x0129) ||
			(mapid >= 0x0141 && mapid <= 0x0144) ||
			(mapid >= 0x01AF && mapid <= 0x01B0) ||
			(mapid >= 0x01DE && mapid <= 0x01DB) ||
			(mapid >= 0x021F && mapid <= 0x0244) ||
			(mapid >= 0x025A && mapid <= 0x026D) ||
			(mapid >= 0x02BC && mapid <= 0x02CB) ||
			(mapid >= 0x06CD && mapid <= 0x06DD) ||
			(mapid >= 0x06EB && mapid <= 0x073E) ||
			(mapid >= 0x0745 && mapid <= 0x075C) ||
			(mapid >= 0x07BD && mapid <= 0x07D4) ||
			(mapid >= 0x0834 && mapid <= 0x08C6)
			)
			return illegal_z;

		if ( mapid >= 0x0A8 && mapid <= 0x0AB) 	// Water
			return illegal_z;
	} // WALKFLAG_MAP

        //
        // STATIC TILES -- Check for static tiles Z elevation
        //
	if ( flags & WALKFLAG_STATIC ) {
        	MapStaticIterator msi( pos.x, pos.y );
		UI32 loopexit = 0;

		staticrecord *stat;
		while ( ( (stat = msi.Next()) != NULL ) && ( ++loopexit < MAXLOOPS ) ) {
                	tile_st tile;

			Map->SeekTile( stat->itemid, &tile );

			// Z elevation
			height = tile.height;
			if ( tile.flag2 & 0x4 ) // Stairs, ladders
				height = tile.height / 2;

			if ( (stat->zoff + height) < (pos.z + MaxZstep) ) { // We cannot walk under it
				if ( tile.flag1 & 0x40 ) // Block flag
					return illegal_z;

				if ( (stat->zoff + height) <= (pos.z + 2) ) { // We can walk on it
	                                if ( (stat->zoff + tile.height) > zRes )
						zRes = stat->zoff + tile.height;
				} else
					return illegal_z;
                	}
			loopexit++;
		}
	} // WALKFLAG_STATIC

	//
	// CHARACTERS -- Check for characters at given position
	//
	if ( flags & WALKFLAG_CHARS ) {
		NxwCharWrapper sc;
		P_CHAR pc = NULL;
		sc.fillCharsAtXY( pos );

		for( sc.rewind(); !sc.isEmpty(); sc++ )	{
			pc = sc.getChar();
			if ( !ISVALIDPC( pc ) )
				continue;

			if ( abs( pc->getPosition().z - zRes ) < MaxZstep )
				return illegal_z;
		}
	} // WALKFLAG_CHARS

	return zRes;
}



/*!
\author Luxor
\brief Tells if the line of sight between two locations is not interrupted
\return True if the line of sight is clean, false if not
*/
LOGICAL lineOfSight( Location A, Location B )
{
	cLine line( A, B );

	UI32 max_x = max( A.x, B.x );
	UI32 max_y = max( A.y, B.y );
	UI32 max_i = max( max_x, max_y );
	UI32 i = ( max_i == max_x ) ? min( A.x, B.x ) : min( A.y, B.y );

	Location pos;
	for ( ; i <= max_i; i++ ) {
		pos = ( max_i == max_x ) ? line.getPosAtX( i ) : line.getPosAtY( i );
		if ( isWalkable( pos, WALKFLAG_DYNAMIC+WALKFLAG_MAP+WALKFLAG_STATIC ) == illegal_z )
			return false;
	}
	return true;
}

/*!
\author Luxor
\brief Tells if an npc can move in the given position
\todo Implement special features based on the npc (fire elemental should walk on lava passages etc...)
*/
LOGICAL canNpcMoveHere( Location pos )
{
	return ( isWalkable( pos ) != illegal_z );
}

