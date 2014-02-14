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
	string str( newRaceType );
	RaceType rt;
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

	if( newRaceType == "PC" )
		this->value = PCRACE;
	else if( newRaceType == "NPC" )
		this->value = NPCRACE;
	else if( newRaceType == "PCNPC" )
		this->value = PCNPCRACE;
	else
		this->value = DEFAULTRACETYPE;

	return *this;
}

RaceType& RaceType::operator=( const int newRaceType )
{
	if( ( newRaceType>=0 )  && ( newRaceType<=3 ) )
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
	switch( this->value ) {
		case PCRACE:
			return ( strcmp( "PC", a ) == 0 );
		case NPCRACE:
			return ( strcmp( "NPC", a ) == 0 );
		default:
		case PCNPCRACE:
			return ( strcmp( "PCNPC", a ) == 0 );
	}
;
}

RaceType::operator int() const
{
	return this->value;
}

RaceType::operator char*() const
{
	switch( this->value ) {
		case PCRACE:
			return "PC";
		case NPCRACE:
			return "NPC";
		default:
		case PCNPCRACE:
			return "PCNPC";
	}
}

