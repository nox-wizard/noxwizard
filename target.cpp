  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "target.h"
#include "client.h"
#include "inlines.h"
#include "network.h"

cTarget::cTarget( bool selectLocation )
{
	pkg.type = selectLocation;
}

cTarget::~cTarget()
{
}

void cTarget::send( NXWCLIENT ps )
{
	pkg.send( ps );
}

bool cTarget::isValid()
{
	if( pkg.type=1 && ( ( pkg.x.get()==INVALID ) || ( pkg.y.get()==INVALID ) ) )
		return false;
	if( pkg.type=0 && ( ( pkg.clicked.get()==0 ) && ( pkg.model.get()== 0 ) ) )
		return false;
	return true;
}

void cTarget::error( NXWCLIENT ps )
{
}


cObjectTarget::cObjectTarget() : cTarget( false )
{
}

cObjectTarget::~cObjectTarget()
{
}

bool cObjectTarget::isValid()
{
	return ( pkg.type==0 );
}

void cObjectTarget::error( NXWCLIENT ps )
{
	ps->sysmsg( "Invalid object" );
}

cCharTarget::cCharTarget() : cObjectTarget()
{
}

cCharTarget::~cCharTarget()
{
}

bool cCharTarget::isValid()
{
	return ( pkg.type==0 ) && ( isCharSerial( pkg.clicked.get() ) && ( MAKE_CHAR_REF( pkg.clicked.get() )!=NULL ) );
}

void cCharTarget::error( NXWCLIENT ps )
{
	ps->sysmsg( "Invalid character" );
}

cItemTarget::cItemTarget() : cObjectTarget()
{
}

cItemTarget::~cItemTarget()
{
}

bool cItemTarget::isValid()
{
	return ( pkg.type==0 ) && ( isItemSerial( pkg.clicked.get() ) && MAKE_ITEM_REF( pkg.clicked.get() )!=NULL );
}

void cItemTarget::error( NXWCLIENT ps )
{
	ps->sysmsg( "Invalid item" );
}

cLocationTarget::cLocationTarget() : cTarget( true )
{
}

cLocationTarget::~cLocationTarget()
{
}

bool cLocationTarget::isValid()
{
	return ( pkg.type==1 ) && ( ( pkg.x.get()!=INVALID ) && ( pkg.y.get()!=INVALID ) );
}

void cLocationTarget::error( NXWCLIENT ps )
{
	ps->sysmsg( "Invalid location" );
}



void amxCallbackOld( NXWCLIENT ps, P_TARGET t )
{
	if( t->amx_callback==NULL) 
		return;
	
	P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
	if( ISVALIDPC(pc) ) {
        t->amx_callback->Call( ps->toInt(), pc->getSerial32(), INVALID, INVALID, INVALID, INVALID );
        return;
    }

    P_ITEM pi = pointers::findItemBySerial( t->getClicked() );
    if( ISVALIDPI(pi) ) {
		t->amx_callback->Call( ps->toInt(), INVALID, pi->getSerial32(), INVALID, INVALID, INVALID );
        return;
    }

    Location loc = t->getLocation();
	t->amx_callback->Call( ps->toInt(), INVALID, INVALID, loc.x, loc.y, loc.z );
}


