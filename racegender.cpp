  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "racegender.h"
#include "racetokenizer.h"

string Gender::genderNames[] = { "female", "male", "female or male" };

Gender::Gender( void )
{
	setValue( FEMALE_OR_MALE );
}

Gender::Gender( const char* initialGender )
{
	setValue( initialGender );
}

GENDER Gender::setValue( const GENDER newGender )
{
	value = newGender;
	return newGender;
}

GENDER Gender::setValue( const char* newGender )
{
	string*		genderParm;
	bool		isMale = false,
			isFemale = false;

	Tokenizer	genderLine( newGender );

	genderParm = genderLine.first();
	while ( !genderParm->empty() )
	{
		if ( *genderParm == "MALE" ) isMale = true;
		else if ( *genderParm == "FEMALE" ) isFemale = true;
		genderParm = genderLine.next();
	}
	if ( isMale )
		if ( isFemale )
			setValue( FEMALE_OR_MALE );
		else
			setValue( MALE );
	else
		if ( isFemale )
			setValue( FEMALE );
		else
			setValue( FEMALE_OR_MALE ); // default value

	return value;
}

GENDER Gender::setValue( const string& newGender )
{
	return setValue( newGender.c_str() );
}

GENDER Gender::getValue( void )
{
	return value;
}

Gender& Gender::operator=( const char* newGender )
{
	setValue( newGender );
	return *this;
}

Gender& Gender::operator=( const string& newGender )
{
	setValue( newGender );
	return *this;
}

Gender& Gender::operator=( const GENDER newGender )
{
	setValue( newGender );
	return *this;
}

bool Gender::operator==( Gender that )
{
	return ( that.value == this->value );
}

bool Gender::operator==( GENDER that )
{
	return ( that == this->value ) ;
}

bool Gender::operator==( int that )
{
	return ( that == int( this->value ) );
}

bool Gender::operator==( char* that )
{
	return ( strcmp( genderNames[ this->value ].c_str(), that ) == 0 );
}

bool Gender::operator==( const string& that )
{
	return ( genderNames[ this->value ] == that );
}

Gender::operator int() const
{
	return this->value;
}

Gender::operator char*() const
{
	return const_cast< char * >( genderNames[ this->value ].c_str() ) ;
}

Gender::operator string*() const
{
	return ( &genderNames[ this->value ] ) ;
}
