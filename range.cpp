  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "range.h"
#include "npcai.h"
#include "set.h"
#include "items.h"
#include "chars.h"

/*!
\author Luxor
\brief Returns distance between two points.
*/
R64 dist( Location a, Location b, LOGICAL countZ )
{
        SI32 xDiff = a.x - b.x;
        SI32 yDiff = a.y - b.y;
	R64 distance = hypot( abs( xDiff ), abs( yDiff ) );
	if ( !countZ || a.z == b.z )
		return distance;

	R64 distZ = abs( a.z - b.z );
	return hypot( distance, distZ );
}

LOGICAL inRange(Location a, Location b, UI32 range)
{
	return (dist(a,b)<= range);
}

LOGICAL inVisRange(Location a, Location b)
{
	return (dist(a,b)<=VISRANGE);
}

LOGICAL char_inVisRange(P_CHAR a, P_CHAR b)
{
	 return char_inRange( a, b, VISRANGE );
}

LOGICAL item_inVisRange(P_CHAR a, P_ITEM b )
{
	 return item_inRange( a, b, VISRANGE );
}

LOGICAL char_inRange(P_CHAR a, P_CHAR b, UI32 range)
{
	VALIDATEPCR(a, false);
	return ( a->distFrom( b ) <= range );
}

LOGICAL item_inRange(P_CHAR a, P_ITEM b, UI32 range)
{
	VALIDATEPCR(a, false);
	return ( a->distFrom( b ) <= range );
}

UI32 item_dist(P_CHAR a, P_ITEM b)
{
	VALIDATEPCR(a, VERY_VERY_FAR);
	return a->distFrom( b );
}

LOGICAL inbankrange(int i)
{
	P_CHAR pc=MAKE_CHAR_REF(i);
	VALIDATEPCR(pc,false);

	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( pc->getPosition(), 6, true, false );
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pcm=sc.getChar();
	
		if (ISVALIDPC(pcm) && pcm->npcaitype==NPCAI_BANKER)
		{
			return true;
		}
	}
	return false;
}

