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
\brief Line of Sight related stuff
*/
#ifndef __LOS_H__
#define __LOS_H__

#include "constants.h"
#include "typedefs.h"
#include <string>

using namespace std;

/*!
\brief need for Line of Sight checking
\author Duke ( 26/12/01 )
*/
class cRectangle
{
public:
	SI16 lowerLeftX;
	SI16 lowerLeftY;
	SI16 upperRightX;
	SI16 upperRightY;
	cRectangle(SI16 x1, SI16 y1, SI16 x2, SI16 y2);
	~cRectangle() { }
	bool includes(SI16 x, SI16 y);
	bool overlaps(cRectangle& other);
};

/*!
\brief Coordinates
*/
struct Coord_cl {
	SI32 x;
	SI32 y; 
	SI16 z;
};

/*!
\brief Line of Sight
\author Duke ( 26/12/01 )
*/
class cLoS
{
protected:
	Coord_cl pos1;		//!< coord source
	Coord_cl pos2;		//!< coord dest
	cRectangle lineRect;	//!< rectangle bounding the line
	bool noXdelta, noYdelta, noZdelta;	//!< delta options
	SI32 Xdir, Ydir, Zdir;	
	R32 incX, incY, incZ;
	R32 incYhalf;
	SI16 minZ,maxZ;	//!< just needed for vertikal lines

	bool isInsideRectangle(SI16 x, SI16 y)	{return lineRect.includes(x,y);}
	SI16 getZatXY(SI16 x, SI16 y);
	bool isBlocking(SI16 id);
	bool crossesTile(SI16 x, SI16 y);
	bool isBlockedByHeight(SI16 id, SI16 baseZ, SI16 lineZ);
	bool isBlockedByMulti(P_ITEM pi);
	bool isBlockedByTile(P_ITEM pi);
public:
	cLoS(Coord_cl& posA,Coord_cl& posB);
	~cLoS() { }
	UI08 flags;		//!< check for
	bool isBlockedByItem(P_ITEM pi);
	bool isBlockedByLand(SI16 x, SI16 y);
	bool isBlockedByStatic(SI16 x, SI16 y);
	void firstTile(SI16* x, SI16* y);
	bool nextTile(SI16* x, SI16* y);
};


#endif

