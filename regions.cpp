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

namespace regions
{
	WHERE_IS_NOW_MAP item_where_is_now;
	WHERE_IS_NOW_MAP char_where_is_now;
	region_db_st regions[REGION_X_CELLS][REGION_Y_CELLS];

	void initialize()
	{
		for( int x=0; x<REGION_X_CELLS; x++ )
			for ( int y=0; y<REGION_Y_CELLS; y++ ) {
				regions[x][y].charsInRegions.clear();
				regions[x][y].itemsInRegions.clear();
			}

	}

	void add( P_CHAR pc )
	{
		if(!ISVALIDPC(pc) )
			return;
		if(!isValidCoord( pc->getPosition() )) {
			//move to safe location? boh
			return;
		}

		removeNow( pc );
		addNow( pc );
	}

	void add( P_ITEM pi )
	{
		VALIDATEPI(pi);
		if( !ISVALIDPI(pi) || !pi->isInWorld() )
			return;
		if(!isValidCoord( pi->getPosition() )) {
			//move to safe location? boh
			return;
		}

		removeNow( pi );
		addNow( pi );
	}

	void remove( P_CHAR pc )
	{

		VALIDATEPC(pc);

		if(!isValidCoord( pc->getPosition() ) )
			return;

		removeNow( pc );

	}

	void remove( P_ITEM pi )
	{

		VALIDATEPI(pi);

		if(!isValidCoord( pi->getPosition() ) )
			return;

		removeNow( pi );
	}

	void removeNow( P_CHAR pc )
	{

		WHERE_IS_NOW_MAP::iterator iternow( char_where_is_now.find( pc->getSerial32() ) );
		if( iternow!=char_where_is_now.end() ) {
			RegCoordPoint* p = &iternow->second;
		
			SERIAL_SET* iter = &regions[p->a][p->b].charsInRegions;
			SERIAL_SET::iterator i( iter->find(pc->getSerial32()) );
			if( i!=iter->end() )
				iter->erase( i );
		}
	}

	void removeNow( P_ITEM pi )
	{
		WHERE_IS_NOW_MAP::iterator iternow( item_where_is_now.find( pi->getSerial32() ) );
		if( iternow!=item_where_is_now.end() ) {
			RegCoordPoint* p = &iternow->second;
		
			SERIAL_SET* iter = &regions[p->a][p->b].itemsInRegions;
			SERIAL_SET::iterator i( iter->find(pi->getSerial32()) );
			if( i!=iter->end() )
				iter->erase( i );
		}
	}

	bool isValidCoord( Location l )
	{
		return l.x < (MapTileWidth*8) && l.y < (MapTileHeight*8);
	}

	bool isValidCoord( UI16 x, UI16 y )
	{
		return x < (MapTileWidth*8) && y < (MapTileHeight*8);
	}

	void addNow( P_CHAR pc )
	{
		RegCoordPoint p( pc->getPosition() ); 
		char_where_is_now.insert( make_pair( pc->getSerial32(), p ) );
		regions[p.a][p.b].charsInRegions.insert( pc->getSerial32() );
	}

	void addNow( P_ITEM pi )
	{
		RegCoordPoint p( pi->getPosition() ); 
		item_where_is_now.insert( make_pair( pi->getSerial32(), p ) );
		regions[p.a][p.b].itemsInRegions.insert( pi->getSerial32() );
	}

	RegCoordPoint::RegCoordPoint( Location location )
	{
		 a=location.x/REGION_GRIDSIZE;
		 b=location.y/REGION_COLSIZE;
	}

} // namespace

