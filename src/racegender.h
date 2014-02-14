  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __racegender__
#define __racegender__

#include "nxwcommn.h"

typedef enum { FEMALE, MALE, FEMALE_OR_MALE } GENDER;

class Gender
{
	private:
		static	string genderNames[];
	private:
			GENDER			value;
	public:
						Gender( void );
						Gender( const char* initialGender );
			GENDER			getValue( void );
			GENDER			setValue( const GENDER newGender );
			GENDER			setValue( const char* newGender );
			GENDER			setValue( const string& newGender );

			Gender& operator=( const char* newGender );
			Gender& operator=( const string& newGender );
			Gender& operator=( const GENDER newGender );

			bool	 operator==( Gender that );
			bool	 operator==( GENDER that );
			bool	 operator==( int that );
			bool	 operator==( char* that );
			bool	 operator==( const string& that );
			operator int() const;
			operator char*() const;
			operator string*() const;
};


#endif
