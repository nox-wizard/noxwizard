  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "unicode.h"

#define FIRSTBYTE( C ) C & 0xFF
#define SECONDBYTE( C ) C >> 8

cUnicodeString& cUnicodeString::operator=( std::string& str )
{
	erase(begin(), end()); // GCC2 workaround to missing clear
	for(iterator iter = begin(); iter != end(); iter++ ) {
		push_back( (wchar_t)*iter );
	}

	return *this;
}

template < class T >
  void u < T >::set( T value )
{
	for( int i=0; i<sizeof(T); ++i ) {
		this->byte[i]=value%256;
		value/=256;
	}
}

template < class T >
  T u < T >::get()
{
	T v=0;
	for( int i=sizeof(T)-1; i>0; --i )
		v+=this->byte[i]<< (8*i);
}
