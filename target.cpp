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
#include "archive.h"


SERIAL cTarget::serial_current = 0;

cTarget::cTarget( bool selectLocation )
{
	this->type = selectLocation;
	this->serial= ++serial_current;
}

cTarget::~cTarget()
{
}

void cTarget::send( NXWCLIENT ps )
{
	cPacketTargetingCursor< cServerPacket > pkg;
	pkg.type = this->type;
	pkg.cursor = this->serial;
	pkg.send( ps );
}

void cTarget::receive( NXWCLIENT ps )
{
	cPacketTargetingCursor<cClientPacket> pkg;
	pkg.receive( ps );

	this->clicked = pkg.clicked.get();
	this->model= pkg.model.get();
	this->loc = Loc( pkg.x.get(), pkg.y.get(), pkg.z );
}

bool cTarget::isValid()
{
	if( type=1 && ( ( loc.x==UINVALID16 ) || ( loc.y==UINVALID16 ) ) )
		return false;
	if( type=0 && ( ( clicked==0 ) && ( model== 0 ) ) )
		return false;
	return true;
}

void cTarget::error( NXWCLIENT ps )
{
}

Location cTarget::getLocation()
{
	return loc;
}

SERIAL cTarget::getClicked()
{
	return clicked;
}

UI16 cTarget::getModel()
{
	return model;
}


cObjectTarget::cObjectTarget() : cTarget( false )
{
}

cObjectTarget::~cObjectTarget()
{
}

bool cObjectTarget::isValid()
{
	return ( type==0 );
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
	return ( type==0 ) && ( isCharSerial( clicked ) && ( MAKE_CHAR_REF( clicked )!=NULL ) );
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
	return ( type==0 ) && ( isItemSerial( clicked ) && MAKE_ITEM_REF( clicked )!=NULL );
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
	return ( type==1 ) && ( ( loc.x!=UINVALID16 ) && ( loc.y!=UINVALID16 ) );
}

void cLocationTarget::error( NXWCLIENT ps )
{
	ps->sysmsg( "Invalid location" );
}


/*
void amxCallbackOld( NXWCLIENT ps, P_TARGET t )
{
	if( t->amx_callback==NULL) 
		return;
	
	P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
	if( ISVALIDPC(pc) ) {
        t->amx_callback->Call( ps->currCharIdx(), pc->getSerial32(), INVALID, INVALID, INVALID, INVALID );
        return;
    }

    P_ITEM pi = pointers::findItemBySerial( t->getClicked() );
    if( ISVALIDPI(pi) ) {
		t->amx_callback->Call( ps->currCharIdx(), INVALID, pi->getSerial32(), INVALID, INVALID, INVALID );
        return;
    }

    Location loc = t->getLocation();
	t->amx_callback->Call( ps->currCharIdx(), INVALID, INVALID, loc.x, loc.y, loc.z );
}*/

void amxCallback( NXWCLIENT ps, P_TARGET t )
{
	if( t->amx_callback==NULL) 
		return;

	SI16 model = t->getModel();
	if( model == 0 )
		model = INVALID;

	/// targ_serial, chr, obj, x, y, z, model, param

	P_OBJECT po = objects.findObject( t->getClicked() );
	if( ISVALIDPO(po) ) {
        t->amx_callback->Call( t->serial, ps->currCharIdx(), po->getSerial32(), INVALID, INVALID, INVALID, model, t->buffer[0] );
        return;
    }
	else {
	    Location loc = t->getLocation();
        t->amx_callback->Call( t->serial, ps->currCharIdx(), INVALID, loc.x, loc.y, loc.z, model, t->buffer[0] );
	}
}


P_TARGET createTarget( TARG_TYPE type )
{
	switch( type ) {
		case TARG_CHAR:
			return new cCharTarget();
		case TARG_ITEM:
			return new cItemTarget();
		case TARG_OBJ:
			return new cObjectTarget();
		case TARG_LOC:
			return new cLocationTarget();
		case TARG_ALL:
		default:
			return new cTarget();
	}

}











///////////////////////////////////////////////////////////////////////
//
// TARGETLOCATION CLASS
//
///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Function name     : TargetLocation::TargetLocation
// Author            : Xanathar
// Changes           : none yet
void TargetLocation::init(P_CHAR pc)
{
	Location pcpos= pc->getPosition();

	m_pc = pc;
	m_x = pcpos.x;
	m_y = pcpos.y;
	m_z = pcpos.z;
	m_pi = NULL;
	m_piSerial = INVALID;
	m_pcSerial = pc->getSerial32();
}
///////////////////////////////////////////////////////////////////
// Function name     : void TargetLocation::init
// Author            : Xanathar
// Changes           : none yet
void TargetLocation::init(P_ITEM pi)
{
	m_pc = NULL;
	if (pi->isInWorld()) {
		m_x = pi->getPosition("x");
		m_y = pi->getPosition("y");
		m_z = pi->getPosition("z");
	} else {
		m_x = m_y = m_z = 0;
	}
	m_pi = pi;
	m_piSerial = pi->getSerial32();
	m_pcSerial = INVALID;
}

///////////////////////////////////////////////////////////////////
// Function name     : TargetLocation::init
// Author            : Xanathar
// Changes           : none yet
void TargetLocation::init(int x, int y, int z)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_pi = NULL;
	m_pc = NULL;
	m_piSerial = m_pcSerial = INVALID;
}


///////////////////////////////////////////////////////////////////
// Function name     : TargetLocation::revalidate
// Description       : recalculates item & char from their serial :)
// Return type       : void
// Author            : Xanathar
// Changes           : none yet
void TargetLocation::revalidate()
{
	m_pi=pointers::findItemBySerial(m_piSerial);

	m_pc=pointers::findCharBySerial(m_piSerial);

	if (ISVALIDPI(m_pi)) {
		m_pi = NULL;
		m_piSerial = INVALID;
	}

	if (ISVALIDPC(m_pc)) {
		m_pc = NULL;
		m_pcSerial = INVALID;
	}
}


///////////////////////////////////////////////////////////////////
// Function name     : TargetLocation::extendItemTarget
// Description       : extends item data to x,y,z and eventually owner char
// Return type       : void
// Author            : Xanathar
// Changes           : none yet
void TargetLocation::extendItemTarget()
{
	if (m_pc!=NULL)
		return;
	if (m_pi==NULL)
		return;
	if (m_pi->isInWorld()) {
		m_x = m_pi->getPosition("x");
		m_y = m_pi->getPosition("y");
		m_z = m_pi->getPosition("z");
	}
	else {
		int it, ch;
		getWorldCoordsFromSerial (m_pi->getSerial32(), m_x, m_y, m_z, ch, it);
		m_pc=MAKE_CHAR_REF(ch);
		m_pcSerial = (ISVALIDPC(m_pc))? m_pc->getSerial32() : INVALID;
	}
	revalidate();
}



///////////////////////////////////////////////////////////////////
// Function name     : TargetLocation::TargetLocation
// Author            : Xanathar
// Changes           : none yet
TargetLocation::TargetLocation( P_TARGET pp )
{
	Location loc = pp->getLocation();
	init( loc.x, loc.y, loc.z );
	if( pp->type==0 ) {

		P_CHAR pc= pointers::findCharBySerial( pp->getClicked() );
		if(ISVALIDPC(pc)) {
			init(pc);
			return;
		}

		P_ITEM pi= pointers::findItemBySerial( pp->getClicked() );
		if (ISVALIDPI(pi))  {
			init(pi);
			return;
		}
	}
	else if( pp->type==1 ) {
		return;
	}
// Wintermute: Always return a valid location, or spells can't target ground
	this->m_pc=NULL;
	this->m_pcSerial=INVALID;
	this->m_pi=NULL;
	this->m_piSerial=INVALID;
/*
	this->m_x=0;
	this->m_y=0;
	this->m_z=0;
*/
}


