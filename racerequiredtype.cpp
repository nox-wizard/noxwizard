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
	value = RT_OPTIONAL;
}

RequiredType& RequiredType::operator=( const char* newRequiredType )
{
	string		str = newRequiredType;
	RequiredType	rt;
	rt = str;
	value = rt.value;
	return *this;
}

RequiredType& RequiredType::operator=( const REQUIREDTYPE newRequiredType )
{
	value = newRequiredType;
	return *this;
}

RequiredType& RequiredType::operator=( const string& newRequiredType )
{
	int		index;

	value = RT_OPTIONAL;

	for( index = RT_FIRST; index < RT_LAST; index++ )
		if ( requiredTypeNames[index] == newRequiredType )
		{
			value = static_cast< REQUIREDTYPE >(index);
			break;
		}

	return *this;
}

RequiredType& RequiredType::operator=( const int newRequiredType )
{
	if ( newRequiredType >= int( RT_FIRST ) && newRequiredType <= int( RT_LAST ) )
		value = static_cast< REQUIREDTYPE >( newRequiredType );
	else
		value = RT_OPTIONAL;
	return *this;
}

bool RequiredType::operator==( RequiredType a )
{
	return ( a.value == value );
}


bool RequiredType::operator==( REQUIREDTYPE a )
{
	return ( a == value ) ;
}

bool RequiredType::operator==( int a )
{
	return ( a == int( value ) );
}

bool RequiredType::operator==( char* a )
{
	return ( strcmp( requiredTypeNames[ value ].c_str(), a ) == 0 );
}

RequiredType::operator int() const
{
	return value;
}

#if 0
RequiredType::operator REQUIREDTYPE() const
{
	return value;
}
#endif

RequiredType::operator char*() const
{
	return const_cast< char * >( requiredTypeNames[ value ].c_str() ) ;
}


RequiredType::operator string*() const
{
	return &requiredTypeNames[ value ] ;
}

