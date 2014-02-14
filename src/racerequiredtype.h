  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __racerequiredtype__
#define __racerequiredtype__

#include "nxwcommn.h"

typedef enum { RT_OPTIONAL = 0, RT_PROHIBITED = 1, RT_MANDATORY = 2 } REQUIREDTYPE;

class RequiredType
{
	private:
		static	string			requiredTypeNames[];
	private:
			REQUIREDTYPE		value;
	public:
						RequiredType( void );

			RequiredType&	operator=( const char* newRequiredType );
			RequiredType&	operator=( const REQUIREDTYPE newRequiredType );
			RequiredType&	operator=( const string& newRequiredType );
			RequiredType&	operator=( const int newRequiredType );
			bool	   	operator==( RequiredType a );
			bool	   	operator==( REQUIREDTYPE a );
			bool	   	operator==( int a );
			bool	   	operator==( char* a );
				   	operator int() const;
				   	operator char*() const;
				   	operator string*() const;
					/*
				   	operator REQUIREDTYPE() const;
					*/
};

#endif
