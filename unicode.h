  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __UNICODE_H__
#define __UNICODE_H__

typedef UI16 UNI_TEXT;

/*!
\brief An unicode string
\author Endymion
*/
class cUnicodeString {
public:
	std::vector<UI08> s;
public:
	cUnicodeString();
	virtual ~cUnicodeString();

	UI32 size();
	UI32 length();

	void copy( std::string& s );

	void clear();
};

/*
\brief useful template
\author Endymion
\note endian now is not a problem
*/
template < class T > 
class u{
private: 	
	UI08 byte[sizeof(T)];
public:
	void set( T value );
	T get();
};

#endif