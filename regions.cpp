  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "debug.h"
#include "items.h"
#include "chars.h"

cRegion::cRegion()
{
	for( int x=0; x<REGION_X_CELLS; x++ )
		for ( int y=0; y<REGION_Y_CELLS; y++ ) {
			this->regions[x][y].charsInRegions.clear();
			this->regions[x][y].itemsInRegions.clear();
		}

}

cRegion::~cRegion()
{


}

bool cRegion::isValidCoord( UI16 x, UI16 y )
{
	return ( x<MAP_WIDTH && y<MAP_HEIGHT );

}


void cRegion::add( P_CHAR pc )
{

	if(!ISVALIDPC(pc) )
		return;
	if(!isValidCoord( pc->getPosition().x, pc->getPosition().y )) {
		//move to safe location? boh
		return;
	}

	this->removeNow( pc );
	this->addNow( pc );

}

void cRegion::add( P_ITEM pi )
{

	VALIDATEPI(pi);
	if( !ISVALIDPI(pi) || !pi->isInWorld() )
		return;
	if(!isValidCoord( pi->getPosition().x, pi->getPosition().y )) {
		//move to safe location? boh
		return;
	}

	this->removeNow( pi );
	this->addNow( pi );
}

void cRegion::remove( P_CHAR pc )
{

	VALIDATEPC(pc);

	if(!isValidCoord( pc->getPosition().x, pc->getPosition().y ) )
		return;

	this->removeNow( pc );

}

void cRegion::remove( P_ITEM pi )
{

	VALIDATEPI(pi);

	if(!isValidCoord( pi->getPosition().x, pi->getPosition().y ) )
		return;

	this->removeNow( pi );
}

void cRegion::removeNow( P_CHAR pc )
{

	WHERE_IS_NOW_MAP::iterator iternow( this->char_where_is_now.find( pc->getSerial32() ) );
	if( iternow!=this->char_where_is_now.end() ) {
		RegCoordPoint* p = &iternow->second;
	
		SERIAL_SET* iter = &this->regions[p->a][p->b].charsInRegions;
		SERIAL_SET::iterator i( iter->find(pc->getSerial32()) );
		if( i!=iter->end() )
			iter->erase( i );
	}
}

void cRegion::removeNow( P_ITEM pi )
{

	WHERE_IS_NOW_MAP::iterator iternow( this->item_where_is_now.find( pi->getSerial32() ) );
	if( iternow!=this->item_where_is_now.end() ) {
		RegCoordPoint* p = &iternow->second;
	
		SERIAL_SET* iter = &this->regions[p->a][p->b].itemsInRegions;
		SERIAL_SET::iterator i( iter->find(pi->getSerial32()) );
		if( i!=iter->end() )
			iter->erase( i );
	}
}

void cRegion::addNow( P_CHAR pc )
{
	RegCoordPoint p( pc->getPosition() ); 
	this->char_where_is_now.insert( make_pair( pc->getSerial32(), p ) );
	this->regions[p.a][p.b].charsInRegions.insert( pc->getSerial32() );
}

void cRegion::addNow( P_ITEM pi )
{
	RegCoordPoint p( pi->getPosition() ); 
	this->item_where_is_now.insert( make_pair( pi->getSerial32(), p ) );
	this->regions[p.a][p.b].itemsInRegions.insert( pi->getSerial32() );
}

RegCoordPoint::RegCoordPoint( Location location )
{
	 this->a=location.x/REGION_GRIDSIZE;
	 this->b=location.y/REGION_COLSIZE;
}


