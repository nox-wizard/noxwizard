  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __NXWSTRINGLIST_H__
#define __NXWSTRINGLIST_H__

#include "nxwcommn.h"
#include "nxwString.h"

class nxwStringList
{
	private:
		UI32NXWSTRINGMAP					list;
		SERIAL										listItemSerial;
	public:
															nxwStringList();

		UI32											append( const char*					what );
		UI32											append( const std::string&	what );
		UI32											append( const nxwString&		what );
		UI32NXWSTRINGMAPITERATOR	begin();
		UI32											count();
		bool											empty();
		UI32NXWSTRINGMAPITERATOR	end();
		void											print();
		UI32											size();
};

#endif
