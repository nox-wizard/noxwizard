  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "char_array.h"
#include "debug.h"

namespace CharArray
{
	LOGICAL error = false;

	/*!
	\brief Make P_CHAR from CHARACTE
	\param i the character index
	\return the pointer to the char
	*/
	P_CHAR MakeCharref(CHARACTER i)
	{
		/*error=0; // clear previous state, 0=no error

		if ( i<0 || i>=charcount)
		{
			error=1;
			return NULL;
		}

		return &realchars[i+C_W_O_1]; // plus wrapping offset, see charwrap.cpp for details*/

		P_CHAR pc = pointers::findCharBySerial(i); //Luxor

		// v-- Stupid stuff.. we don't need it
		error = (pc == NULL);
		return pc;
	}

}
