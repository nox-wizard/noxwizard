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
RaceType::RaceType( void )
{
	this->value = PCNPCRACE;
}

RACETYPE RaceType::getValue( void )
{
	return this->value;
}

RaceType& RaceType::operator=( const char* newRaceType )
{
	string		str = newRaceType;
	RaceType	rt;
	rt = str;
	this->value = rt.value;
	return *this;
}

RaceType& RaceType::operator=( const RACETYPE newRaceType )
{
	this->value = newRaceType;
	return *this;
}

RaceType& RaceType::operator=( const string& newRaceType )
{
	int		index;

	this->value = DEFAULTRACETYPE;

	for( index = FIRSTRACE; index < LASTRACE; index++ )
		if ( raceTypeNames[index] == newRaceType )
		{
			this->value = static_cast< RACETYPE >(index);
			break;
		}

	return *this;
}

RaceType& RaceType::operator=( const int newRaceType )
{
	if ( newRaceType >= int( FIRSTRACE ) && newRaceType <= int( LASTRACE ) )
		this->value = static_cast< RACETYPE >( newRaceType );
	else
		this->value = static_cast< RACETYPE >( DEFAULTRACETYPE );
	return *this;
}

bool RaceType::operator==( RaceType a )
{
	return ( a.value == this->value );
}


bool RaceType::operator==( RACETYPE a )
{
	return ( a == this->value ) ;
}

bool RaceType::operator==( int a )
{
	return ( a == int( this->value ) );
}

bool RaceType::operator==( char* a )
{
	return ( strcmp( raceTypeNames[ this->value ].c_str(), a ) == 0 );
}

RaceType::operator int() const
{
	return this->value;
}

/*
RaceType::operator RACETYPE() const
{
	return this->value;
}
*/

RaceType::operator char*() const
{
	return const_cast< char * >( raceTypeNames[ this->value ].c_str() ) ;
}


RaceType::operator string*() const
{
	return &raceTypeNames[ this->value ] ;
}
