  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwStringList.h"

nxwStringList::nxwStringList()
{
	listItemSerial = INVALID;
}

UI32 nxwStringList::append( const char* what )
{
	return append( nxwString( what ) );
}

UI32 nxwStringList::append( const std::string& what )
{
	return append( nxwString( what ) );
}

UI32 nxwStringList::append( const nxwString& what )
{
	list[++listItemSerial] = what;
	return listItemSerial;
}

UI32NXWSTRINGMAP::iterator nxwStringList::begin()
{
	return list.begin();
}

UI32 nxwStringList::count()
{
	return list.size();
}

LOGICAL nxwStringList::empty()
{
	return list.empty();
}

UI32NXWSTRINGMAP::iterator nxwStringList::end()
{
	return list.end();
}

void nxwStringList::print()
{
	UI32NXWSTRINGMAP::iterator it( list.begin() ), end( list.end() );

	for(; it != end; ++it )
	{
		it->second.print();
		ConOut("\n");
	}
}

UI32 nxwStringList::size()
{
	UI32 size = 0;
	UI32NXWSTRINGMAP::iterator it( list.begin() ), end( list.end() );

	for(; it != end; ++it )
		size += it->second.size();
	
	return size;
}
