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
	std::vector<UI08>::iterator before_term;
	std::vector<UI08> s;

	void addTerminator();

public:
	cUnicodeString();
	cUnicodeString( std::string& s );
	virtual ~cUnicodeString();

	UI32 size();
	UI32 length();

	void copy( std::string& s );
	cUnicodeString& operator+=( wchar_t c );

	void clear();
};

/*
\brief endian buster 16bit
\author Endymion
*/

template< class T >
class endian {
protected:
	T a;
	T b;
public:
	endian() { a=0; b=0; }
	virtual ~endian( ) { };
};

typedef UI08 eUI08;


class eUI16 : endian<UI08> {
public:
	eUI16() : endian<UI08>() { };
	virtual ~eUI16() { };
	void operator =( UI32 v );
};

typedef eUI16 eCOLOR;

/*
\brief endian buster 16bit
\author Endymion
*/
class eUI32 : endian<eUI16> {
public:
	eUI32() : endian<eUI16>() { };
	virtual ~eUI32() { };
	void operator =( UI32 v );
};

typedef eUI32 eSERIAL;


#endif
