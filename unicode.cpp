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

/*!
\brief Constructor
\author Endymion
*/
cUnicodeString::cUnicodeString()
{
	addTerminator();
}

/*!
\brief Constructor
\author Endymion
\param s the string to copy
*/
cUnicodeString::cUnicodeString( std::string& s )
{
	this->copy(s);
}

/*!
\brief Constructor
\author Endymion
\param u the unicode string to copy 
\note id u is NULL an empty unicode string is created
*/
cUnicodeString::cUnicodeString( cUnicodeString* u )
{
	this->copy(u);
}

/*
\brief Constructor
\author Endymion
\param c is an array ( Big Endian Format ) double null terminated
\param size if INVALID copy until 00 termination else until size
*/
cUnicodeString::cUnicodeString( char* c, int size )
{
	this->copy( c, size );
}

/*
\brief Destructor
\author Endymion
*/
cUnicodeString::~cUnicodeString() { }

/*
\brief Get the string size ( in byte )
\author Endymion
\return string size
*/
UI32 cUnicodeString::size()
{
	return s.size();
}

/*
\brief add 00 termination
\author Endymion
*/
void cUnicodeString::addTerminator()
{
	s.push_back(0); // 0 0 termination
	s.push_back(0);
}


/*
\brief Get the string size length, number of character
\author Endymion
\return string length
*/
UI32 cUnicodeString::length()
{
	return (s.size()-2)/2;
}

/*
\brief Clear string
\author Endymion
*/
void cUnicodeString::clear()
{
	s.clear();
	addTerminator();
}

/*
\brief Copy from another string
\author Endymion
\param s the string
*/
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

/*
\brief Copy from another unicode string
\author Endymion
\param u the unicode string to copy 
\note id u is NULL an empty unicode string is created
*/
void cUnicodeString::copy( cUnicodeString* u )
{
	this->s.clear();
	if( u==NULL ) {
		addTerminator();
	}
	else {
		std::vector<UI08>::iterator iter( u->s.begin() ), end( u->s.end() );
		for( ; iter!=end; iter++ ) {
			this->s.push_back( (*iter) );
		}
	}
}


/*
\brief Copy from another unicode string
\author Endymion
\param c is an array ( Big Endian Format ) double null terminated
\param size if INVALID copy until 00 termination else until size
*/
void cUnicodeString::copy( char* c, int size )
{
	this->s.clear();
	if( size==INVALID ) {//until termination
		int wasZero=0;
		int i=0;
		do
		{
			this->s.push_back( c[i] );
			if( c[i]==0 )
				++wasZero;
			else 
				wasZero=0;
			++i;
		} while( wasZero!=2 );
	}
	else { //until size
		for( int i=0; i<size; ++i ) {
			this->s.push_back( c[i] );
		}
		addTerminator();
	}

}

/*
\brief Append an uncode character
\author Endymion
\param c the character
*/
cUnicodeString& cUnicodeString::operator+=( wchar_t c )
{
	append( c );
	return (*this);
}

/*
\brief Append an uncode character
\author Endymion
\param c the character
*/
void cUnicodeString::append( wchar_t c )
{
}
