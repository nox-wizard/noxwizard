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

/*
\brief endian-independent unsigned int 16 bits
\author Endymion
*/
class eUI16 {
	private:
		UI16 a;
	public:
		eUI16() { a=0; };
		eUI16(UI16 v);
		eUI16& operator =( UI16 v );
} PACK_NEEDED;


/*
\brief endian-independent unsigned int 32 bits
\author Endymion
*/
class eUI32 {
	private:
		UI32 a;
	public:
		eUI32() { a=0; };
		eUI32(UI32 v);
		eUI32& operator =( UI32 v );
} PACK_NEEDED;

typedef UI08 PACK_NEEDED eUI08;		//!< fake endian-independent UI08
typedef bool PACK_NEEDED ebool;		//!< fake endian-independent bool
typedef eUI16 PACK_NEEDED eCOLOR;	//!< endian-independent color
typedef eUI32 PACK_NEEDED eSERIAL;	//!< endian-independent serial

#endif
