  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "racerequiredtype.h"
#include "racetokenizer.h"

string RequiredType::requiredTypeNames[3] = { "OPTIONAL", "PROHIBITED", "MANDATORY" };

///////////////////////////////////////////////////////////////////
// Function name	: RequiredType::RequiredType
// Purpose		: construct new CRequiredType object
// Return type		: constructor
// Author            	: Sparhawk
// Argument          	: none
// Note			: we could introduce unknown as racetype
//			  and set value to this on construction
// History		:
RequiredType::RequiredType( void )
{
	this->value = RT_OPTIONAL;
}

RequiredType& RequiredType::operator=( const char* newRequiredType )
{
	string		str = newRequiredType;
	RequiredType	rt;
	rt = str;
	this->value = rt.value;
	return *this;
}

RequiredType& RequiredType::operator=( const REQUIREDTYPE newRequiredType )
{
	this->value = newRequiredType;
	return *this;
}

RequiredType& RequiredType::operator=( const string& newRequiredType )
{
	int		index;

	this->value = RT_OPTIONAL;

	for( index = 0; index < 2; index++ )
		if ( requiredTypeNames[index] == newRequiredType )
		{
			this->value = static_cast< REQUIREDTYPE >(index);
			break;
		}

	return *this;
}

RequiredType& RequiredType::operator=( const int newRequiredType )
{
	if ( newRequiredType >= int( 0 ) && newRequiredType <= int( 2 ) )
		this->value = static_cast< REQUIREDTYPE >( newRequiredType );
	else
		this->value = RT_OPTIONAL;
	return *this;
}

bool RequiredType::operator==( RequiredType a )
{
	return ( a.value == this->value );
}


bool RequiredType::operator==( REQUIREDTYPE a )
{
	return ( a == this->value ) ;
}

bool RequiredType::operator==( int a )
{
	return ( a == int( this->value ) );
}

bool RequiredType::operator==( char* a )
{
	return ( strcmp( requiredTypeNames[ this->value ].c_str(), a ) == 0 );
}

RequiredType::operator int() const
{
	return this->value;
}

/*
RequiredType::operator REQUIREDTYPE() const
{
	return this->value;
}
*/

RequiredType::operator char*() const
{
	return const_cast< char * >( requiredTypeNames[ this->value ].c_str() ) ;
}


RequiredType::operator string*() const
{
	return &requiredTypeNames[ this->value ] ;
}
