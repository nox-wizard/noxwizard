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
\brief Area Management stuff (used by spawn, region ecc ecc)
\author Endymion
*/

#ifndef _AREA_H
#define _AREA_H

namespace areas
{

	typedef struct {
		UI16 x1;
		UI16 y1;
		UI16 x2;
		UI16 y2;
	} Area;

	typedef std::map< SERIAL, Area > AREA_DB;
	typedef AREA_DB::iterator AREA_ITER;

	extern AREA_DB allareas;
	extern SERIAL currarea;

	SERIAL insert( Area& newarea, SERIAL index = INVALID );
	void loadareas();
}
#endif
