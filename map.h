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
\brief Header of Map related functions
*/

#ifndef __MAP_H__
#define __MAP_H__


/*!
\author Luxor
\brief Check flags for isWalkable function
*/
enum {
	WALKFLAG_NONE = 0x0, WALKFLAG_MAP = 0x1, WALKFLAG_STATIC = 0x2, WALKFLAG_DYNAMIC = 0x4, WALKFLAG_CHARS = 0x8, WALKFLAG_ALL = 0xF
};

class cLine {
public:
	cLine( Location A, Location B );
	Location getPosAtX( UI32 x );
	Location getPosAtY( UI32 y );
	SI08 calcZAtX( UI32 x );
private:
	UI32 x1, y1;
	SI08 z1;
	SI32 m_xDist, m_yDist, m_zDist;
};

SI08 isWalkable( Location pos, UI08 flags = WALKFLAG_ALL );
LOGICAL lineOfSight( Location pos1, Location pos2 );
LOGICAL canNpcWalkHere( Location pos );

inline SI32 line_of_sight( SI32 s, Location a, Location b, SI32 checkfor )
{ return lineOfSight( a, b ); }

inline SI32 line_of_sight( SI32 s, SI32 x1, SI32 y1, SI32 z1, SI32 x2, SI32 y2, SI32 z2, SI32 checkfor )
{ return lineOfSight( Loc( x1, y1, z1 ), Loc( x2, y2, z2 ) ); }


#endif //__MAP_H__