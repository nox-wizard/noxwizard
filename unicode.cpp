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
	addTerminator();
}

cUnicodeString::cUnicodeString( std::string& s )
{
	this->copy(s);
}

cUnicodeString::~cUnicodeString() { }

UI32 cUnicodeString::size()
{
	return (s.size()-2)/2;
}

void cUnicodeString::addTerminator()
{
	s.push_back(0); // 0 0 termination
	before_term=s.begin();
	s.push_back(0);
}


UI32 cUnicodeString::length()
{
	return size();
}

void cUnicodeString::clear()
{
	s.clear();
	addTerminator();
}

void cUnicodeString::copy( std::string& s )
{
	this->s.clear();
	std::string::iterator iter( s.begin() ), end( s.end() );
	for( ; iter!=end; iter++ ) {
		this->s.push_back( 0 );
		this->s.push_back( (*iter) );
	}
	addTerminator();
}

cUnicodeString& cUnicodeString::operator+=( wchar_t c )
{
	this->s.insert( before_term, c >> 8   );
	this->s.insert( before_term, c & 0xFF );
	return (*this);
}







void eUI16::operator =( UI32 v )
{
	this->a=v >>  8;
	this->b=v & 0xFF;
}


void eUI32::operator =( UI32& v )
{
	this->a= v >>  16;
	this->b= v & 0xFFFF;
}
