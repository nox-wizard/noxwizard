  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "abstraction/endian.h"

eUI16::eUI16()
{
	a=0;
}

eUI16::eUI16(UI16 v)
{
	a=htons( v );
}

eUI16& eUI16::operator =( UI16 v )
{
	a=htons( v );
	return (*this);
}

eUI16& eUI16::operator=( eUI16 v )
{
	this->a=v.a;
	return (*this);
}


UI16 eUI16::get(  )
{
	return ntohs( this->a );
	//return this->a;
}






eUI32::eUI32()
{
	a=0;
}

eUI32::eUI32(UI32 v)
{
	a=htonl( v );
}

eUI32& eUI32::operator =( UI32 v )
{
	a=htonl( v );
	return (*this);
}

eUI32& eUI32::operator=( eUI32 v )
{
	this->a=v.a;
	return (*this);
}

UI32 eUI32::get(  )
{
	return ntohl( this->a );
}

