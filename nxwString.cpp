  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "stdio.h"
#include "stdarg.h"

#include "nxwString.h"


UI32	nxwString::defaultMaxLength		= 256;
UI32	nxwString::formatBufferLength = 1024;
//char* nxwString::formatBuffer				= new char[nxwString::formatBufferLength];

char* nxwString::formatBuffer				= new char[1024];

void nxwString::setDefaultMaxLength( UI32 value )
{
	defaultMaxLength = value;
}

UI32 nxwString::getDefaultMaxLength()
{
	return defaultMaxLength;
}

nxwString nxwString::format( char const *formatStr, ... )
{
	va_list vargs;
	va_start(vargs, formatStr );
	vsnprintf( formatBuffer, formatBufferLength-1, formatStr, vargs);
	va_end(vargs);
	formatBuffer[formatBufferLength-1] = '\0';
	return nxwString( formatBuffer );
}


nxwString::nxwString()
{
	maxLength = nxwString::defaultMaxLength;
}

nxwString::nxwString( char const *arg )
{
	UI32 length	  = strlen( arg );
	UI32 defaultLength = nxwString::getDefaultMaxLength();
	if( length < defaultLength )
		maxLength = defaultLength;
	else
		maxLength = length;
	value = arg;
}

nxwString::nxwString( std::string const &arg )
{
	UI32 length	  = arg.size();
	UI32 defaultLength = nxwString::getDefaultMaxLength();
	if( length < defaultLength )
		maxLength = defaultLength;
	else
		maxLength = length;
	value = arg;
}

nxwString::nxwString( nxwString const &arg )
{
	*this = arg;
}

LOGICAL nxwString::empty()
{
	return value.empty();
}

std::string nxwString::getValue()
{
	return value;
}

void nxwString::print()
{
	ConOut( "nxwString = %s\n", value.c_str() );
}

UI32 nxwString::size()
{
	return value.size();
}

//nxwString const &nxwString::operator = ( char const *rvalue )
//{
//}
