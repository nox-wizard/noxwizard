  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __RANGE_H__
#define __RANGE_H__

#include "globals.h"
#include "basics.h"

#define VERY_VERY_FAR 50000;

R64 dist( Location a, Location b, LOGICAL countZ = (server_data.disable_z_checking == 0) );
inline R64 dist( UI32 xa, UI32 ya, SI08 za, UI32 xb, UI32 yb, SI08 zb, LOGICAL countZ = (server_data.disable_z_checking == 0) )
{
	return dist( Loc( xa, ya, za ), Loc( xb, yb, zb ), countZ );
}

LOGICAL inRange(Location a, Location b, UI32 range);
LOGICAL inVisRange(Location a, Location b);
LOGICAL char_inVisRange(P_CHAR a, P_CHAR b);
LOGICAL item_inVisRange(P_CHAR a, P_ITEM b );
LOGICAL char_inRange(P_CHAR a, P_CHAR b, UI32 range);
LOGICAL item_inRange(P_CHAR a, P_ITEM b, UI32 range);
UI32 item_dist(P_CHAR a, P_ITEM b);
LOGICAL inbankrange(int i);

#endif
