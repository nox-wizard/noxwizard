  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "racetype.h"
#include "racetokenizer.h"

string RaceType::raceTypeNames[RACETYPES] = { "PC", "NPC", "PCNPC" };

///////////////////////////////////////////////////////////////////
// Function name	: RaceType::RaceType
// Purpose		: construct new RaceType object
// Return type		: constructor
// Author            	: Sparhawk
// Argument          	: none
// Note			: we could introduce unknown as racetype
//			  and set value to this on construction
// History		:
RaceType::RaceType()
{
	value = PCNPCRACE;
}

RACETYPE RaceType::getValue()
{
	return value;
}

RaceType& RaceType::operator=( const char* newRaceType )
{
	string		str = newRaceType;
	RaceType	rt;
	rt = str;
	value = rt.value;
	return *this;
}

RaceType& RaceType::operator=( const RACETYPE newRaceType )
{
	value = newRaceType;
	return *this;
}

RaceType& RaceType::operator=( const string& newRaceType )
{
	int		index;

	value = DEFAULTRACETYPE;

	for( index = FIRSTRACE; index < LASTRACE; index++ )
		if ( raceTypeNames[index] == newRaceType )
		{
			value = static_cast< RACETYPE >(index);
			break;
		}

	return *this;
}

RaceType& RaceType::operator=( const int newRaceType )
{
	if ( newRaceType >= int( FIRSTRACE ) && newRaceType <= int( LASTRACE ) )
		value = static_cast< RACETYPE >( newRaceType );
	else
		value = static_cast< RACETYPE >( DEFAULTRACETYPE );
	return *this;
}

bool RaceType::operator==( RaceType a )
{
	return ( a.value == value );
}


bool RaceType::operator==( RACETYPE a )
{
	return ( a == value ) ;
}

bool RaceType::operator==( int a )
{
	return ( a == int( value ) );
}

bool RaceType::operator==( char* a )
{
	return ( strcmp( raceTypeNames[ value ].c_str(), a ) == 0 );
}

RaceType::operator int() const
{
	return value;
}

#if 0
RaceType::operator RACETYPE() const
{
	return value;
}
#endif

RaceType::operator char*() const
{
	return const_cast< char * >( raceTypeNames[ value ].c_str() ) ;
}


RaceType::operator string*() const
{
	return &raceTypeNames[ value ] ;
}
