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

cUnicodeString::cUnicodeString()
{
	s.push_back(0); // 0 0 termination
	s.push_back(0);
}

cUnicodeString::~cUnicodeString() { }

UI32 cUnicodeString::size()
{
	return (s.size()-2)/2;
}

UI32 cUnicodeString::length()
{
	return size();
}

void cUnicodeString::clear()
{
	s.clear();
	s.push_back(0); // 0 0 termination
	s.push_back(0);
}

void cUnicodeString::copy( std::string& s )
{
	this->s.clear();
	std::string::iterator iter( s.begin() ), end( s.end() );
	for( ; iter!=end; iter++ ) {
		this->s.push_back( 0 );
		this->s.push_back( (*iter) );
	}
	this->s.push_back(0); //terminator
	this->s.push_back(0);

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

