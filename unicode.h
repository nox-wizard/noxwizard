  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file
\brief Unicode Stuff
\author Endymion
*/

#ifndef __UNICODE_H__
#define __UNICODE_H__

#include <string>

/*!
\brief An unicode string
\author Endymion
\note Fixed for GCC by Akron
*/
class cUnicodeString : public wstring{
public:
	cUnicodeString(std::string &str)
	{
		*this = str;
	}

	cUnicodeString &operator=(std::string &str);

	inline const char* sc_str()
	{
		return (const char*)c_str();
	}

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
