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

void eUI16::operator =( UI16 v )
{
	a=htons( v );
}

void eUI16::operator=( eUI16 v )
{
	this->a=v.a;
}


eUI16::eUI16(UI16 v)
{
	a=htons( v );
}

void eUI32::operator =( UI32 v )
{
	a=htonl( v );
}

void eUI32::operator=( eUI32 v )
{
	this->a=v.a;
}

eUI32::eUI32(UI32 v)
{
	a=htonl( v );
}

