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
\name weights namespace
*/

#ifndef __WEIGHT_H
#define __WEIGHT_H
#include "typedefs.h"

/*!
\brief Weight related stuff
*/
namespace weights {

	int	CheckWeight(P_CHAR pc);
	float	RecursePacks(P_ITEM bp);
	int	CheckWeight2(P_CHAR pc);
	void	NewCalc(P_CHAR pc);
	float	LockeddownWeight(P_ITEM pItem, int *total);
}


#endif
