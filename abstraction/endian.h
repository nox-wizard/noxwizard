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
\brief Endian Abstraction level
\author Endymion
\note splitted from unicode.cpp by Akron
*/

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include "nxwcommn.h"

/*
\brief endian-independent unsigned int 16 bits
\author Endymion
*/
class eUI16 {
	private:
		UI16 a;
	public:
		eUI16();
		eUI16(UI16 v);
		eUI16& operator =( UI16 v );
		eUI16& operator=( eUI16 v );
		UI16 get( );

} PACK_NEEDED;


/*
\brief endian-independent unsigned int 32 bits
\author Endymion
*/
class eUI32 {
	private:
		UI32 a;
	public:
		eUI32();
		eUI32(UI32 v);
		eUI32& operator =( UI32 v );
		eUI32& operator=( eUI32 v );
		UI32 get( );

} PACK_NEEDED;

typedef UI08 eUI08;		//!< fake endian-independent UI08
typedef SI08 eSI08;		//!< fake endian-independent SI08
typedef bool eBool;		//!< fake endian-independent bool
typedef eUI16 eCOLOR;	//!< endian-independent color
typedef eUI32 eSERIAL;	//!< endian-independent serial

#endif
