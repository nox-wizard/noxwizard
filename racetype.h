  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __RaceType__
#define __RaceType__

#include "nxwcommn.h"

typedef enum { PCRACE = 0, NPCRACE = 1, PCNPCRACE = 4 } RACETYPE;
const RACETYPE DEFAULTRACETYPE = PCNPCRACE;

class RaceType
{
	private:
			RACETYPE		value;
	public:
						RaceType( void );
			RACETYPE		getValue( void );

			RaceType&	operator=( const char* newRaceType );
			RaceType&	operator=( const RACETYPE newRaceType );
			RaceType& 	operator=( const string& newRaceType );
			RaceType& 	operator=( const int newRaceType );
			bool	   	operator==( RaceType a );
			bool	   	operator==( RACETYPE a );
			bool	   	operator==( int a );
			bool	   	operator==( char* a );
				   	operator int() const;
				   	operator char*() const;
};

#endif
