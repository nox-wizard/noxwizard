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

/*
0x6C Packet

Targeting Cursor Commands (19 bytes) 

	BYTE cmd 
    BYTE type 
		0x00 = Select Object
		0x01 = Select X, Y, Z 

	BYTE[4] cursorID 
	BYTE Cursor Type 
		Always 0 now  
	
	The following are always sent but are only valid if sent by client 

	BYTE[4] Clicked On ID
	BYTE[2] click xLoc 
	BYTE[2] click yLoc 
	BYTE unknown (0x00) 
	BYTE click zLoc 
	BYTE[2] model # (if a static tile, 0 if a map/landscape tile)
		Note: the model # shouldn’t be trusted.

*/

cTargeT::cTargeT(  ) 
{
}

void cTargeT::Error( NXWCLIENT ps, char* txt )
{
	if( ps!=NULL )
		ps->sysmsg( txt );
}

void cTargeT::Do( NXWCLIENT ps )
{
}

cTargetSerial::cTargetSerial( ) : cTargeT(  ) 
{
	this->call=NULL;
}

cTargetSerial::cTargetSerial( processSerialTarget callThis ) : cTargeT(  ) 
{
	this->call=callThis;
}

bool cTargetSerial::isObjectTarget( NXWCLIENT ps )
{
	return ( ps!=NULL && ( buffer[ps->toInt()][1]==0 ) );
}

SERIAL cTargetSerial::makeSerial( NXWCLIENT ps ) 
{
	return LongFromCharPtr( buffer[ps->toInt()]+7 );
}

void cTargetSerial::Do( NXWCLIENT ps )
{
	if( this->isObjectTarget( ps ) ) {
		this->call( ps, makeSerial( ps ) );
	}
	else
		this->Error( ps, "Invalid Object selected" );

}

cTargetItem::cTargetItem( processItemTarget callThis ) : cTargetSerial( ) 
{
	this->call=callThis;
};

bool cTargetItem::isItemTarget( NXWCLIENT ps )
{
	if( this->isObjectTarget( ps ) ) {
		return isItemSerial( makeSerial( ps ) );
	}
	else
		return false;
}

void cTargetItem::Do( NXWCLIENT ps )
{
	if( this->isItemTarget( ps ) ) {
		this->call( ps, pointers::findItemBySerial( makeSerial( ps ) ) );
	}
	else
		this->Error( ps, "Invalid Item selected" );
}


cTargetChar::cTargetChar( processCharTarget callThis ) : cTargetSerial( ) 
{
	this->call=callThis;
}

bool cTargetChar::isCharTarget( NXWCLIENT ps )
{
	if( this->isObjectTarget( ps ) ) {
		return isCharSerial( makeSerial( ps ) );
	}
	else
		return false;
}

void cTargetChar::Do( NXWCLIENT ps )
{
	if( this->isCharTarget( ps ) )
		this->call( ps, pointers::findCharBySerial( makeSerial( ps ) ) );
	else
		this->Error( ps, "Invalid Char selected" );
}


bool cTargetLocation::isLocationTarget( NXWCLIENT ps )
{
	return ( ps!=NULL && ( buffer[ps->toInt()][1]==1 ) );
}

cTargetLocation::cTargetLocation( processLocationTarget callThis ) : cTargeT( ) 
{
	this->call=callThis;
}

Location cTargetLocation::makeLocation( NXWCLIENT ps ) 
{
	Location location;
	location.x=DBYTE2WORD( buffer[ps->toInt()][11], buffer[ps->toInt()][12] );
	location.y=DBYTE2WORD( buffer[ps->toInt()][13], buffer[ps->toInt()][14] );
	location.z=DBYTE2WORD( buffer[ps->toInt()][15], buffer[ps->toInt()][16] );
	return location;
}

void cTargetLocation::Do( NXWCLIENT ps )
{
	if( this->isLocationTarget( ps ) )
		this->call( ps, makeLocation( ps ) );
	else
		this->Error( ps, "Invalid Location selected" );
}

