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

#include "nxwcommn.h"

typedef UI16 PACK_NEEDED UNI_TEXT;

/*!
\brief An unicode string
\author Endymion
*/
class cUnicodeString {
public:
	std::vector<UI08> s;

	void addTerminator();

public:
	cUnicodeString();
	cUnicodeString( std::string& s );
	cUnicodeString( char* c, int size=INVALID );
	~cUnicodeString();

	UI32 size();
	UI32 length();

	void copy( std::string& s );
	void copy( char* c, int size=INVALID );
	void append( wchar_t c );
	cUnicodeString& operator+=( wchar_t c );

	void clear();
} PACK_NEEDED;

#endif
