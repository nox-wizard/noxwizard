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
\brief Header for conversion functions
\note documentation ported to doxygen by Akron
*/
#ifndef __BASICS_H__
#define __BASICS_H__

#include "typedefs.h"

/*!
\author Duke
\brief Calculates a long int from 4 subsequent bytes pointed to by p
\param p pointer to the 4 subsequent bytes
\return the value of the long found
\note assuming 'normal' (big endian, ndAkron) byte order (NOT intel style)
*/
inline SI32 LongFromCharPtr(const unsigned char *p)
{
	return (*p<<24) | (*(p+1)<<16) | (*(p+2)<<8) | *(p+3);
}

/*!
\author Duke
\brief Calculates a short int from 2 subsequent bytes pointed to by p
\param p pointer to the 2 subsequent bytes
\return the value of the short found
\note assuming 'normal' (big endian, ndAkron) byte order (NOT intel style)
*/
inline SI16 ShortFromCharPtr(const unsigned char *p)
{
	return static_cast<short>((*p<<8) | *(p+1));
}

/*!
\author Duke
\brief Stores a long int into 4 subsequent bytes pointed to by 'p'
\param i value to store
\param p pointer to the char array
\note assuming 'normal' (big endian, ndAkron) byte order (NOT intel style)
*/
inline void LongToCharPtr(const UI32 i, unsigned char *p)
{
	*p=static_cast<unsigned char>(i>>24); 
	*(p+1)=static_cast<unsigned char>(i>>16); 
	*(p+2)=static_cast<unsigned char>(i>>8); 
	*(p+3)=static_cast<unsigned char>(i);
}

/*!
\author Duke
\brief Stores a short int into 2 subsequent bytes pointed to by 'p'
\param i value to store
\param p pointer to the char array
\note assuming 'normal' (big endian, ndAkron) byte order (NOT intel style)
*/
inline void ShortToCharPtr(const UI16 i, unsigned char *p)
{
	*p=static_cast<unsigned char>(i>>8);
	*(p+1)=static_cast<unsigned char>(i);	// no %256 for 2nd byte, truncation will occur anyway
}

//@{
/*!
\author Lord Binary
\name Wrappers for stdlib num-2-str functions
*/

/*!
\brief convert the integer into a string in decimal base
\param i integer to convert
\param ourstring string to write the number to
*/
inline void numtostr(UI32 i, char *ourstring)
{
	sprintf(ourstring,"%d",i);
}

/*!
\brief convert the integer into a string in hexadecimal base
\param i integer to convert
\param ourstring string to write the number to
*/
inline void hextostr(UI32 i, char *ourstring)
{
	sprintf (ourstring, "%x",i);
}
//@}

//@{
/*!
\name Bases
\brief bases for conversion from string to number
*/
#define BASE_INARRAY -1
#define BASE_AUTO 0
#define BASE_BIN 2
#define BASE_OCT 8
#define BASE_DEC 10
#define BASE_HEX 16
//@}


/*!
\author Xanathar
\brief Conversion from sz to numbers
\param sz string that represent the number
\param base base in which the number is (see Bases)
\return the number represented by the string
*/
int str2num(char* sz, int base = BASE_AUTO);

/*!
\brief Conversion from sz to numbers
\author Endymion
\param sz string that represent the number
\param base base in which the number is (see Bases)
\return the number represented by the string
*/
int str2num( wchar_t* sz, int base = BASE_AUTO);

/*!
\author Xanathar
\brief New style hexstring to number
\param sz the hexstring
\return the number represented by the string
*/
inline int hex2num (char *sz)
{
	return str2num(sz, BASE_HEX);
}

//@{
/*!
\name Wrappers
\author Sparhawk
\brief Wrappers for c++ strings
*/

/*!
\author Sparhawk
\brief wrapper to str2num function
\param s string that represent the number
\param base base in which the number is (see Bases)
\return the number represented by the string
*/
inline int str2num ( std::string& s, int base = BASE_AUTO )
{
	return str2num( const_cast< char* >( s.c_str() ), base );
}

/*!
\brief Wrapper to str2num function
\author Endymion
\param s wstring that represent the number
\param base base in which the number is (see Bases)
\return the number represented by the string
*/
inline int str2num ( std::wstring& s, int base = BASE_AUTO )
{
	return str2num( const_cast< wchar_t* >(s.c_str()), base );
}


/*!
\author Sparhawk
\brief wrapper to str2num function
\param s the hexstring
\return the number represented by the string
*/
inline int hex2num ( std::string& s )
{
	return str2num( const_cast< char* >( s.c_str() ), BASE_HEX );
}
//@}

int fillIntArray(char* str, int *array, int maxsize, int defval = -1, int base = BASE_AUTO);
void readSplitted(FILE* F, char* script1, char* script2);
int RandomNum(int nLowNum, int nHighNum);
char *RealTime(char *time_str);
Location Loc(SI32 x, SI32 y, SI08 z, SI08 dispz=0);

#endif //__BASICS_H__
