  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "item_array.h"
#include "debug.h"

namespace ItemArray
{
	LOGICAL error = false;

	P_ITEM MakeItemref(ITEM i)
	{
		/*error=0; // clear previous state, 0=no error

		if ( i<0 || i>=itemcount)
		{

			error=1;
			return NULL;
		}

		return &realitems[i+I_W_O_1]; // plus wrapping offset, see charwrap.cpp for details
		*/
		P_ITEM pi = pointers::findItemBySerial(i);

		//  v---- stupid stuff
		error = (pi==NULL);
		return pi;
	}

}

