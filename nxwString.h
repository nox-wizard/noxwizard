  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __NXWSTRING_H__
#define __NXWSTRING_H__

#include "nxwcommn.h"

class nxwString
{
	private:
		static UI32				defaultMaxLength;
		static char*			formatBuffer;
		static UI32				formatBufferLength;
	public:
		static void				setDefaultMaxLength( UI32 value );
		static UI32				getDefaultMaxLength();
		static nxwString	format( char const *formatStr, ... );
	private:
		std::string				value;
		UI32							maxLength;
	public:
											nxwString();
											nxwString( char const *arg );
											nxwString( std::string const &arg );
											nxwString( nxwString const &arg );
		virtual	~nxwString() {}

		std::string				getValue();
		LOGICAL						empty();
		void							print();
		UI32							size();

		//nxwString const 	&operator=( char const *rvalue );
		//nxwString const 	&operator=( std::string const *rvalue );
};

typedef map< UI32, nxwString>	UI32NXWSTRINGMAP;
typedef UI32NXWSTRINGMAP::iterator UI32NXWSTRINGMAPITERATOR;
#endif
