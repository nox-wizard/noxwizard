  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "npcai.h"

//return distances between 2 point
double dist(Location a, Location b)
{
	return dist( a.x, a.y, a.z, b.x, b.y, b.z );
}

double dist(int xa, int ya, int za, int xb, int yb, int zb) {
	double c= hypot(abs(xa-xb), abs(ya-yb) );
	if( server_data.disable_z_checking==1 )  
		return c;
	else {
		double dz= abs(za-zb);
		if(dz == 0) 
			return c;
		return hypot(c, dz);
	}
}

bool inRange(Location a, Location b, UI32 range)
{
	return (dist(a,b)<= range);
}

bool inVisRange(Location a, Location b)
{
	return (dist(a,b)<=VISRANGE);
}

bool char_inVisRange(P_CHAR a, P_CHAR b)
{
	 return char_inRange( a, b, VISRANGE );
}

bool item_inVisRange(P_CHAR a, P_ITEM b )
{
	 return item_inRange( a, b, VISRANGE );
}

bool char_inRange(P_CHAR a, P_CHAR b, UI32 range)
{
	VALIDATEPCR(a, false);
	return ( a->distFrom( b ) <= range );
}

bool item_inRange(P_CHAR a, P_ITEM b, UI32 range)
{
	VALIDATEPCR(a, false);
	return ( a->distFrom( b ) <= range );
}

UI32 item_dist(P_CHAR a, P_ITEM b)
{
	VALIDATEPCR(a, VERY_VERY_FAR);
	return a->distFrom( b );
}

bool inbankrange(int i)
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

