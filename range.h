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

#define VERY_VERY_FAR 50000;

double dist(Location a, Location b);
double dist(int xa, int ya, int za, int xb, int yb, int zb);
bool inRange(Location a, Location b, UI32 range);
bool inVisRange(Location a, Location b);
bool char_inVisRange(P_CHAR a, P_CHAR b);
bool item_inVisRange(P_CHAR a, P_ITEM b );
bool char_inRange(P_CHAR a, P_CHAR b, UI32 range);
bool item_inRange(P_CHAR a, P_ITEM b, UI32 range);
UI32 item_dist(P_CHAR a, P_ITEM b);
bool inbankrange(int i);

#endif
