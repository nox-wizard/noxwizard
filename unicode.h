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
	cUnicodeString( char* c );
	virtual ~cUnicodeString();

	UI32 size();
	UI32 length();

	void copy( std::string& s );
	void append( wchar_t c );
	cUnicodeString& operator+=( wchar_t c );

	void clear();
};

/*
\brief endian buster
\author Endymion
\note used to remove problem with big-little endian 
*/
class endian {
public:
	endian() {}
	~endian() {}
};

typedef UI08 eUI08;


/*
\brief endian buster 16Bit
\author Endymion
*/
class eUI16 : public endian {
private:
	UI08 a, b;
public:
	eUI16() { a=0; b=0; };
	~eUI16() { };
	void operator =( UI32 v );
};

typedef eUI16 eCOLOR;

/*
\brief endian buster 32Bit
\author Endymion
*/
class eUI32 : public endian {
private:
	eUI16 a, b;
public:
	eUI32() { a=0; b=0; };
	~eUI32() { };
	void operator =( UI32 v );
};

typedef eUI32 eSERIAL;


#endif
