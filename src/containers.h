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
\brief All containers related stuff
*/

#ifndef _CONTAINERS_H
#define _CONTAINERS_H

void loadcontainers();

typedef struct {
	SI16 x;
	SI16 y;
} BasicPosition;


typedef struct {
	UI32 gump;
	BasicPosition upperleft;
	BasicPosition downright;
} cont_gump_st;

typedef std::map< UI32, cont_gump_st > CONTINFOGUMPMAP;
typedef std::map< UI32, CONTINFOGUMPMAP::iterator > CONTINFOMAP;

extern CONTINFOGUMPMAP contInfoGump;
extern CONTINFOMAP contInfo;

#endif
