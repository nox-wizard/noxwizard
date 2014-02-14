  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "spawn.h"
#include "map.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "scripts.h"
#include "sregions.h"

cSpawns* Spawns=NULL;


cSpawnArea::cSpawnArea( AREA_ITER area )
{
	disabled = false;
	this->nextspawn=uiCurrentTime;
	this->where=area;
	this->current=0;
	this->items_spawned.clear();
	this->npcs_spawned.clear();
}

cSpawnArea::~cSpawnArea()
{

}

bool cSpawnArea::findValidLocation( Location& location )
{
	int a=0;
	do {

		location.x = (UI16)RandomNum( where->second.x1, where->second.x2 );
		location.y = (UI16)RandomNum( where->second.y1, where->second.y2 );
		location.z = mapElevation( location.x, location.y );

		if( canNpcWalkHere( location ) )
			return true;
	} while (++a < 100);
	return false;

}

bool cSpawnArea::needSpawn()
{
	return ( TIMEOUT( this->nextspawn ) ); // && ( this->current < this->ma
}




cSpawnScripted::cSpawnScripted( SERIAL serial )
{
	this->serial=serial;
}

cSpawnScripted::~cSpawnScripted()
{
}

void cSpawnScripted::safeCreate( P_CHAR npc, cSpawnArea& single  )
{
	VALIDATEPC(npc);

	Location location = { 0,0,0,0};
	if( single.findValidLocation( location ) ) {
		npc->npcWander = WANDER_FREELY_BOX;
		npc->fx1 = single.where->second.x1;
		npc->fy1 = single.where->second.y1;
		npc->fx2 = single.where->second.x2;
		npc->fy2 = single.where->second.y2;
		npc->fz1 = INVALID;
		single.current++;
		single.nextspawn=uiCurrentTime+ (60*RandomNum( mintime, maxtime)*MY_CLOCKS_PER_SEC);
		npc->setSpawnRegion(this->serial);
		single.npcs_spawned.insert( npc->getSerial32() );
		npc->MoveTo( location );
		npc->teleport();
	}
	else {
		WarnOut("[ERROR] Problem with scripted spawn %i [%s] found. Nothing will be spawned.\n", serial, name.c_str() );
		npc->Delete();
	}
}

void cSpawnScripted::safeCreate( P_ITEM pi, cSpawnArea& single  )
{
	VALIDATEPI(pi);
	
	Location location= { 0,0,0,0};
	if( single.findValidLocation( location ) ) {
		pi->MoveTo(location);
		single.current++;
		single.nextspawn=uiCurrentTime+ (60*RandomNum( mintime, maxtime)*MY_CLOCKS_PER_SEC);
		pi->setSpawnRegion(this->serial);
		single.items_spawned.insert( pi->getSerial32() );
		pi->MoveTo( location );
		pi->Refresh();
	}
	else {
		WarnOut("[ERROR] Problem with scripted spawn %i [%s] found. Nothing will be spawned.\n", serial, name.c_str() );
		pi->Delete();
	}
}

void cSpawnScripted::doSpawn( cSpawnArea& c ) {
	if ( c.disabled ) {
		c.nextspawn=uiCurrentTime+ (60*RandomNum( mintime, maxtime)*MY_CLOCKS_PER_SEC);
		return;
	}
	if( c.current >= max ) {
		c.nextspawn=uiCurrentTime+ (60*RandomNum( mintime, maxtime)*MY_CLOCKS_PER_SEC);
		return;
	}

	if( npclists.size() > 0 )
	{
		UI32 counter = rand()%npclists.size();
		{
			Location location= { 0,0,0,0};
			if( c.findValidLocation( location ) )
			{
				P_CHAR npc = npcs::AddNPCxyz( INVALID, npclists[counter], location );
				if(ISVALIDPC(npc))
				{
					safeCreate( npc, c );
					return;
				}
			}
		}
	}

	if( itemlists.size() > 0 )
	{

		UI32 counter = rand()%itemlists.size();
		{
			Location location= { 0,0,0,0};
			if( c.findValidLocation( location) )
			{
				char list[512];
				sprintf( list, "%i", itemlists[counter] ); // morrolan
				int num = item::CreateRandomItem( list );
				P_ITEM item = item::CreateScriptItem( INVALID, num, 0 );
				if( ISVALIDPI( item ) ) {
					safeCreate( item, c );
					return;
				}
			}
		}
	}

	if( npcs.size()>0 )
	{
		UI32 counter = rand()%npcs.size();
		{
			Location location= { 0,0,0,0};
			location.x=location.y=location.z=location.dispz=0;
			if( c.findValidLocation( location ) )
			{
				P_CHAR npc = npcs::AddNPCxyz( INVALID, npcs[counter], location  );
				if (ISVALIDPC(npc))
				{
					safeCreate( npc, c );
					return;
				}
			}
		}
	}
	if( itms.size()>0 )
	{
		UI32 counter = rand()%itms.size();
		{
			Location location= { 0,0,0,0};
			location.x=location.y=location.z=location.dispz=0;
			if( c.findValidLocation( location ) )
			{
				P_ITEM item = item::CreateScriptItem( INVALID, itms[counter],  true, NULL );
				if (ISVALIDPC(item))
				{
					item->setPosition(location);
					safeCreate( item, c );
					return;
				}
			}
		}
	}


	c.disabled = true;
	WarnOut("Scripted spawn %i [%s] couldn't find anything to spawn, check scripts.\n",serial, name.c_str());

}

void cSpawnScripted::doSpawn( )
{

	if( max==0 )
		return;

	SPAWNAREA_VECTOR::iterator iter( this->singles.begin() ), end( this->singles.end() );
	for( ; iter!=end; iter++ ) {
		if( (*iter).needSpawn() )
			this->doSpawn( *iter );
	}

}

void cSpawnScripted::doSpawn(SPAWNFLAG_ENUM spawnWhat, UI32 amount )
{

	if( max==0 )
		return;
	SPAWNAREA_VECTOR::iterator iter( this->singles.begin() ), end( this->singles.end() );
	for( ; iter!=end; iter++ ) 
	{
		if( (*iter).current >= max ) 
		{
			(*iter).nextspawn=uiCurrentTime+ (60*RandomNum( mintime, maxtime)*MY_CLOCKS_PER_SEC);
			continue;
		}
		if( (*iter).needSpawn() )
		{
			cSpawnArea c = *iter;
			if( npclists.size() > 0 )
			{
				UI32 counter = rand()%npclists.size();
				{
					Location location= { 0,0,0,0};
					if( c.findValidLocation( location ) )
					{
						P_CHAR npc = npcs::AddNPCxyz( INVALID, npclists[counter], location );
						if(ISVALIDPC(npc))
						{
							safeCreate( npc, c );
							return;
						}
					}
				}
			}
			if( itemlists.size() > 0 )
			{

				UI32 counter = rand()%itemlists.size();
				{
					Location location= { 0,0,0,0};
					if( c.findValidLocation( location) )
					{
						char list[512];
						sprintf( list, "%i", itemlists[counter] ); // morrolan
						int num = item::CreateRandomItem( list );
						P_ITEM item = item::CreateScriptItem( INVALID, num, 0 );
						if( ISVALIDPI( item ) ) {
							safeCreate( item, c );
							return;
						}
					}
				}
			}
			if( npcs.size()>0 )
			{
				UI32 counter = rand()%npcs.size();
				{
					Location location= { 0,0,0,0};
					location.x=location.y=location.z=location.dispz=0;
					if( c.findValidLocation( location ) )
					{
						P_CHAR npc = npcs::AddNPCxyz( INVALID, npcs[counter], location  );
						if (ISVALIDPC(npc))
						{
							safeCreate( npc, c );
							return;
						}
					}
				}
			}
		}		
	}
}


void cSpawnScripted::doSpawnAll()
{
	if( max==0 )
		return;

	SPAWNAREA_VECTOR::iterator iter( this->singles.begin() ), end( this->singles.end() );
	for( ; iter!=end; iter++ ) {
		int loopexit=0;
		while( ( (*iter).current < max ) && ( ++loopexit<MAXLOOPS ) )
			this->doSpawn( *iter );
	}

}

void cSpawnScripted::clear( )
{
	SPAWNAREA_VECTOR::iterator iter( this->singles.begin() ), end( this->singles.end() );
	for( ; iter!=end; iter++ ) 
	{
		SERIAL_SET::iterator itm( (*iter).items_spawned.begin() );
		if( itm!=(*iter).items_spawned.end() ) {
			(*iter).items_spawned.erase( itm );
			if( (*iter).current>0 )
				(*iter).current--;
		}
		SERIAL_SET::iterator npc( (*iter).npcs_spawned.begin() );
		if( npc!=(*iter).npcs_spawned.end() ) {
			(*iter).npcs_spawned.erase( npc );
			if( (*iter).current>0 )
				(*iter).current--;
		}
	}
}


void cSpawnScripted::removeObject( P_ITEM pi )
{
	SPAWNAREA_VECTOR::iterator iter( this->singles.begin() ), end( this->singles.end() );
	for( ; iter!=end; iter++ ) {
		SERIAL_SET::iterator itm( (*iter).items_spawned.find( pi->getSerial32() ) );
		if( itm!=(*iter).items_spawned.end() ) {
			(*iter).items_spawned.erase( itm );
			if( (*iter).current>0 )
				(*iter).current--;
			return;
		}
	}
}

void cSpawnScripted::removeObject( P_CHAR pc )
{
	SPAWNAREA_VECTOR::iterator iter( this->singles.begin() ), end( this->singles.end() );
	for( ; iter!=end; iter++ ) {
		SERIAL_SET::iterator npc( (*iter).npcs_spawned.find( pc->getSerial32() ) );
		if( npc!=(*iter).npcs_spawned.end() ) {
			(*iter).npcs_spawned.erase( npc );
			if( (*iter).current>0 )
				(*iter).current--;
			return;
		}
	}
}

void cSpawnScripted::addSpawned(P_OBJECT obj)
{
	LOGICAL found=false;
	if( isItemSerial( obj->getSerial32() ) ) 
	{
		// Search the area that contains the position
		SPAWNAREA_VECTOR::iterator iter( this->singles.begin() ), end( this->singles.end() );
		for( ; iter!=end; iter++ ) 
		{
			Location objPos;
			objPos=obj->getPosition();
			Area areaPos = (*iter).where->second;
			// iter is a cSpawnArea
			if (( areaPos.x1 <= objPos.x ) && ( areaPos.x2 > objPos.x ) )
				if (( areaPos.y1 <= objPos.y ) && ( areaPos.y2 > objPos.y ) )
				{
					(*iter).items_spawned.insert( obj->getSerial32() );
					(*iter).current++;
					found=true;
					break;
				}
		}
		if ( ! found )
		{
	// add them to first entry
			iter=this->singles.begin();
			(*iter).items_spawned.insert( obj->getSerial32() );
			(*iter).current++;
		}
	}
	else if( isCharSerial( obj->getSerial32() ) ) 
	{
		SPAWNAREA_VECTOR::iterator iter( this->singles.begin() ), end( this->singles.end() );
		for( ; iter!=end; iter++ ) 
		{
			Location objPos;
			objPos=obj->getPosition();
			Area areaPos = (*iter).where->second;
			// iter is a cSpawnArea
			if (( areaPos.x1 <= objPos.x ) && ( areaPos.x2 > objPos.x ) )
				if (( areaPos.y1 <= objPos.y ) && ( areaPos.y2 > objPos.y ) )
				{
					(*iter).npcs_spawned.insert( obj->getSerial32() );
					(*iter).current++;
					found=true;
					break;
				}
		}
		if ( ! found )
		{
	// add them to first entry
			iter=this->singles.begin();
			(*iter).npcs_spawned.insert( obj->getSerial32() );
			(*iter).current++;
		}
	}

}


cSpawns::cSpawns()
{
	this->dinamic.clear();
	this->scripted.clear();
	this->check=uiCurrentTime;
}

cSpawns::~cSpawns()
{
}

void cSpawns::loadFromScript()
{

	cScpIterator*	iter = NULL;
	std::string script1;
	std::string script2;

	int idxspawn=0;

	int loopexit=0;
	do
	{
		safedelete(iter);
		SERIAL current = idxspawn;
		iter = Scripts::Spawn->getNewIterator("SECTION REGIONSPAWN %i", idxspawn++);
		if( iter==NULL ) continue;

		Area area;
		UI16 check=0;

		cSpawnScripted* dummy=new cSpawnScripted(current);

		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if("NLIST"==script1) 
					dummy->npclists.push_back(str2num(script2));
				else if("NAME"==script1) 
					dummy->name=script2;
				else if("NPC"==script1) 
					dummy->npcs.push_back(str2num(script2));
				else if("ILIST"==script1)
					dummy->itemlists.push_back(str2num(script2));
				else if("ITEM"==script1)
					dummy->itms.push_back(str2num(script2));
				else if("MAX"==script1) 
					dummy->max=str2num(script2);
				else if("X1"==script1) 
					{ area.x1=(unsigned short)str2num(script2); check|=0x000F; }
				else if("Y1"==script1)
					{ area.y1=(unsigned short)str2num(script2); check|=0x00F0; }
				else if("X2"==script1)
					{ area.x2=(unsigned short)str2num(script2); check|=0x0F00; }
				else if("Y2"==script1)
					{ area.y2=(unsigned short)str2num(script2); check|=0xF000; }
				else if("MINTIME"==script1) 
					dummy->mintime=str2num(script2);
				else if("MAXTIME"==script1) 
					dummy->maxtime=str2num(script2);
				else if("AREA"==script1) {
					AREA_ITER newarea = Areas->allareas.find( str2num(script2) );
					if( newarea!=Areas->allareas.end() )
						dummy->singles.push_back( cSpawnArea( newarea ) );
					else 
						ConOut("[ERROR] on parse of spawn.xss, can't add a new area\n" );
				}
				else ConOut("[ERROR] on parse of spawn.xss [%s]\n", script1.c_str() );
			}

		}
        while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

		if( check==0xFFFF ) { //use old method x1, x2 so need to add a new region
			SERIAL insarea = Areas->insert( area );
			if( insarea!=INVALID ) {
				AREA_ITER newarea = Areas->allareas.find( insarea );
				if( newarea!=Areas->allareas.end() )
					dummy->singles.push_back( cSpawnArea( newarea ) );
				else 
					ConOut("[ERROR] on parse of spawn.xss, can't add a new area\n" );
			}
			else 
				ConOut("[ERROR] on parse of spawn.xss, can't add a new area\n" );
		}
		else if( check!=0 ) {
			ConOut("[ERROR] on parse of spawn.xss [%i], incomplete X1 Y1 X2 Y2\n", current );
			continue;
		}

		Spawns->scripted.insert( make_pair( current, (*dummy) ) );

    }
	while ( (strcmp("EOF", script1.c_str())) && (++loopexit < MAXLOOPS) );


    safedelete(iter);

}

/*!
\author Luxor
\brief Clears the dynamic spawners map.
*/
void cSpawns::clearDynamic()
{
	this->dinamic.clear();
}

/*!
\author Luxor
\brief Creates a new dynamic spawner getting data from an item.
*/
void cSpawns::loadFromItem( P_ITEM pi )
{
	VALIDATEPI(pi);

	//
	// If it's not a spawner, return.
	//
	if ( !pi->isSpawner() )
		return;

	//
	// Insert the spawner in the map.
	//
	cSpawnDinamic dynSpawner(pi);
	dynSpawner.current=pi->amount2;
	Spawns->dinamic.insert( make_pair( pi->getSerial32(), dynSpawner ) );
}

void cSpawns::doSpawn()
{
	SPAWN_SCRIPTED_DB::iterator iter_scr( this->scripted.begin() );
	for( ; iter_scr!=this->scripted.end(); iter_scr++ ) {
		iter_scr->second.doSpawn();
	}

	SPAWN_DINAMIC_DB::iterator iter_din( this->dinamic.begin() );
	for( ; iter_din!=this->dinamic.end(); iter_din++ ) {
		if( iter_din->second.needSpawn() )
			iter_din->second.doSpawn();
	}

	if( speed.srtime!=UINVALID )
		check = uiCurrentTime+ speed.srtime*MY_CLOCKS_PER_SEC; //Don't check them TOO often (Keep down the lag)
	else
		check = uiCurrentTime+ 30*MY_CLOCKS_PER_SEC;
}

void cSpawns::doRegionClearSpawn(UI32 region, SPAWNTYPE_ENUM clearWhat)
{
	if (( clearWhat == SPAWN_ALL )|| ( clearWhat == SPAWN_STATIC ))
	{	
		SPAWN_SCRIPTED_DB::iterator iter_scr( this->scripted.begin() );
		for( ; iter_scr!=this->scripted.end(); iter_scr++ ) 
		{
			// Spawn for all regions
			LOGICAL found=false;
			SPAWNAREA_VECTOR spawnAreas=iter_scr->second.singles;
			SPAWNAREA_VECTOR::iterator areaIter=spawnAreas.begin();
			for ( ; areaIter!=spawnAreas.end();areaIter++)
			{
				short areaRegionNumber = calcRegionFromXY(areaIter->where->second.x1, areaIter->where->second.y1);
				// test the corners of the spawn area if they are within the region, as soon as we find one
				// break out of the for
				// upper left
				if ( region == areaRegionNumber )
				{
					found=true;
					break;
				}
				// lower left
				areaRegionNumber = calcRegionFromXY(areaIter->where->second.x1, areaIter->where->second.y2);
				if ( region == areaRegionNumber )
				{
					found=true;
					break;
				}
				// upper right
				areaRegionNumber = calcRegionFromXY(areaIter->where->second.x2, areaIter->where->second.y1);
				if ( region == areaRegionNumber )
				{
					found=true;
					break;
				}
				// lower right
				areaRegionNumber = calcRegionFromXY(areaIter->where->second.x2, areaIter->where->second.y2);
				if ( region == areaRegionNumber )
				{
					found=true;
					break;
				}

			}
			if ( found)
				// the spawn region is at least partially inside the region
				iter_scr->second.clear();
		}
	}
	if (( clearWhat == SPAWN_ALL )|| ( clearWhat == SPAWN_DYNAMIC ))
	{
		SPAWN_DINAMIC_DB::iterator iter_din( this->dinamic.begin() );
		for( ; iter_din!=this->dinamic.end(); iter_din++ ) 
		{
			P_ITEM spawn=pointers::findItemBySerial( iter_din->second.item );
			if( region == calcRegionFromXY(spawn->getPosition() ))
				iter_din->second.clear();
		}
	}
}

void cSpawns::doRegionSpawn(UI32 region, SPAWNFLAG_ENUM spawnWhat, UI32 amount)
{
	SPAWN_SCRIPTED_DB::iterator iter_scr( this->scripted.begin() );
	for( ; iter_scr!=this->scripted.end(); iter_scr++ ) 
	{
		// Spawn for all regions
		if ( region == -1 )
		{
			iter_scr->second.doSpawn(spawnWhat, amount);
		}
		else
		{
			LOGICAL found=false;
			SPAWNAREA_VECTOR spawnAreas=iter_scr->second.singles;
			SPAWNAREA_VECTOR::iterator areaIter=spawnAreas.begin();
			for ( ; areaIter!=spawnAreas.end();areaIter++)
			{
				short areaRegionNumber = calcRegionFromXY(areaIter->where->second.x1, areaIter->where->second.y1);
				// test the corners of the spawn area if they are within the region, as soon as we find one
				// break out of the for
				// upper left
				if ( region == areaRegionNumber )
				{
					found=true;
					break;
				}
				// lower left
				areaRegionNumber = calcRegionFromXY(areaIter->where->second.x1, areaIter->where->second.y2);
				if ( region == areaRegionNumber )
				{
					found=true;
					break;
				}
				// upper right
				areaRegionNumber = calcRegionFromXY(areaIter->where->second.x2, areaIter->where->second.y1);
				if ( region == areaRegionNumber )
				{
					found=true;
					break;
				}
				// lower right
				areaRegionNumber = calcRegionFromXY(areaIter->where->second.x2, areaIter->where->second.y2);
				if ( region == areaRegionNumber )
				{
					found=true;
					break;
				}

			}
			if ( found)
				// the spawn region is at least partially inside the region
				iter_scr->second.doSpawn(spawnWhat, amount);
		}
	}

	SPAWN_DINAMIC_DB::iterator iter_din( this->dinamic.begin() );
	for( ; iter_din!=this->dinamic.end(); iter_din++ ) 
	{
		P_ITEM spawn=pointers::findItemBySerial( iter_din->second.item );
		if( region == calcRegionFromXY(spawn->getPosition() ))
			iter_din->second.doSpawn(spawnWhat, amount);
	}

	if( speed.srtime!=UINVALID )
		check = uiCurrentTime+ speed.srtime*MY_CLOCKS_PER_SEC; //Don't check them TOO often (Keep down the lag)
	else
		check = uiCurrentTime+ 30*MY_CLOCKS_PER_SEC;
}


void cSpawns::doSpawnAll()
{
	SPAWN_SCRIPTED_DB::iterator iter( this->scripted.begin() ), end( this->scripted.end() );
	for( ; iter!=end; iter++ ) {
		iter->second.doSpawnAll();
	}

	SPAWN_DINAMIC_DB::iterator iter_din( this->dinamic.begin() );
	for( ; iter_din!=this->dinamic.end(); iter_din++ ) {
		iter_din->second.doSpawn();
	}

	if( speed.srtime!=UINVALID )
		check = uiCurrentTime+ speed.srtime*MY_CLOCKS_PER_SEC; //Don't check them TOO often (Keep down the lag)
	else
		check = uiCurrentTime+ 30*MY_CLOCKS_PER_SEC;
}

void cSpawns::doSpawnAll( SERIAL spawn )
{
	SPAWN_SCRIPTED_DB::iterator iter( this->scripted.find( spawn) );
	if( iter!= this->scripted.end() )
		iter->second.doSpawnAll();

}


void cSpawns::removeObject( SERIAL spawn, P_ITEM pi )
{
	SPAWN_SCRIPTED_DB::iterator iter( this->scripted.find( spawn) );
	if( iter!= this->scripted.end() )
		iter->second.removeObject( pi );
}

void cSpawns::removeObject( SERIAL spawn, P_CHAR pc )
{
	SPAWN_SCRIPTED_DB::iterator iter( this->scripted.find( spawn) );
	if( iter!= this->scripted.end() )
		iter->second.removeObject( pc );
}

void cSpawns::removeSpawnDinamic( P_ITEM pi )
{
	VALIDATEPI(pi);
	SPAWN_DINAMIC_DB::iterator iter( this->dinamic.find( pi->getSerial32() ) );
	if( iter!=this->dinamic.end() ) {
		this->dinamic.erase( iter );
	}
	else {
		if( pi->getSpawnSerial()!=INVALID ) {
			iter= this->dinamic.find( pi->getSpawnSerial() );
			if( iter!=this->dinamic.end() ) {
				iter->second.remove( pi->getSerial32() );
			}
		}
	}

}

void cSpawns::removeSpawnDinamic( P_CHAR pc )
{
	VALIDATEPC(pc);
	if( pc->getSpawnSerial()!=INVALID ) {
		SPAWN_DINAMIC_DB::iterator iter= this->dinamic.find( pc->getSpawnSerial() );
		if( iter!=this->dinamic.end() ) {
			iter->second.remove( pc->getSerial32() );
		}
	}

}

void cSpawns::resetSpawnTime()
{
	SPAWN_DINAMIC_DB::iterator iter_din( this->dinamic.begin() );
	for( ; iter_din!=this->dinamic.end(); iter_din++ ) {
		iter_din->second.nextspawn=uiCurrentTime+(5*MY_CLOCKS_PER_SEC);;
	}

}

cSpawnDinamic *cSpawns::getDynamicSpawn(SERIAL spawnerSerial)
{
	if( spawnerSerial !=INVALID ) 
	{
		SPAWN_DINAMIC_DB::iterator iter = this->dinamic.find( spawnerSerial );
		if( iter!=this->dinamic.end() ) 
		{
			return &iter->second;
		}
	}
	else
		return NULL;
	return NULL;
}

cSpawnScripted *cSpawns::getScriptedSpawn(SERIAL spawnerSerial)
{
	if( spawnerSerial !=INVALID ) 
	{
		SPAWN_SCRIPTED_DB::iterator iter = this->scripted.find( spawnerSerial );
		if( iter!=this->scripted.end() ) 
		{
			return &iter->second;
		}
	}
	else
		return NULL;
	return NULL;
}


cSpawnDinamic::cSpawnDinamic( P_ITEM pi )
{
	this->item=pi->getSerial32();
	this->items_spawned.clear();
	this->npcs_spawned.clear();
	this->current=0;
	this->nextspawn=uiCurrentTime+ (60*RandomNum( pi->morey, pi->morez)*MY_CLOCKS_PER_SEC);
}

cSpawnDinamic::~cSpawnDinamic()
{

}

/*!
\author Wintermute
\brief delete all spawned items or npcs in the world
*/

void cSpawnDinamic::clear()
{
	SERIAL_SET::iterator iter=items_spawned.begin();
	for( ; iter!=items_spawned.end(); iter++ ) 
	{
		P_ITEM itm=pointers::findItemBySerial(*iter);
		itm->Delete();
	}
	iter=npcs_spawned.begin();
	for( ; iter!=npcs_spawned.end(); iter++ ) 
	{
		P_CHAR pc=pointers::findCharBySerial(*iter);
		pc->Delete();
	}

}

/*!
\author Endymion, modified by Wintermute
\brief spawn a npc/item from a dynamic spawner
\note spawn is the item of the spawn in the world
\note amount holds the maximum number of spawnables
\note morey the minimum delay, morez the maximum delay
\note morex is the xss number to be spawned
*/

void cSpawnDinamic::doSpawn()
{
	P_ITEM spawn=pointers::findItemBySerial( this->item );
	VALIDATEPI(spawn);

	if( this->current>=spawn->amount || spawn->morex == 0)
		return;
	if( !spawn->isInWorld() )
		return; //npc spawned in container? ahhah i have a gremlin in backpack :P

	if( spawn->type == ITYPE_ITEM_SPAWNER ) {
		if ( spawn->morex == 0 )
			return;
		UI16 min=(UI16)(spawn->moreb1+(spawn->moreb2<<8));
		UI16 max=(UI16)(spawn->moreb3+(spawn->moreb4<<8));
		UI16 maxSpawn=(UI16)RandomNum(min,max);
		if ( (SI16)(spawn->amount - this->current - maxSpawn) < 0 )
			maxSpawn=(UI16)(spawn->amount - this->current);
		for ( int i = 0; i < maxSpawn;++i)
		{
			P_ITEM pi=item::CreateFromScript( spawn->morex );
			if( ISVALIDPI( pi ) ) {
				this->current++;
				spawn->amount2=(unsigned short)this->current;
				this->items_spawned.insert( pi->getSerial32() );
				pi->setSpawnSerial(this->item);
				pi->MoveTo( spawn->getPosition() );
				pi->Refresh();
				#ifdef SPAR_I_LOCATION_MAP
						pointers::addToLocationMap(pi);
				#else
						mapRegions->add(pi);
				#endif
			}
		}
		this->nextspawn=uiCurrentTime+ (60*RandomNum( spawn->morey, spawn->morez)*MY_CLOCKS_PER_SEC);
	}
	else if( spawn->type == ITYPE_NPC_SPAWNER ) {
		if ( spawn->morex == 0 )
			return;
		UI16 min=(UI16)(spawn->moreb1+(spawn->moreb2<<8));
		UI16 max=(UI16)(spawn->moreb3+(spawn->moreb4<<8));
		UI16 maxSpawn=(UI16)RandomNum(min,max);
		if ( maxSpawn == 0 )
			maxSpawn = 1;
		if ( (SI16)(spawn->amount - this->current - maxSpawn) < 0 )
			maxSpawn=(UI16)(spawn->amount - this->current);
		for ( int i = 0; i < maxSpawn;++i)
		{
			P_CHAR npc=npcs::addNpc( spawn->morex, spawn->getPosition().x, spawn->getPosition().y, spawn->getPosition().z );
			if(ISVALIDPC( npc )) {
				this->current++;
				spawn->amount2=(unsigned short)this->current;
				this->npcs_spawned.insert( npc->getSerial32() );
				npc->setSpawnSerial(this->item);
				npc->MoveTo( spawn->getPosition() );
				npc->teleport();
				#ifdef SPAR_I_LOCATION_MAP
						pointers::addToLocationMap(npc);
				#else
						mapRegions->add(npc);
				#endif
			}
		}
		this->nextspawn=uiCurrentTime+ (60*RandomNum( spawn->morey, spawn->morez)*MY_CLOCKS_PER_SEC);
	}

}

/*!
\author Wintermute
\brief spawn a npc/item from a dynamic spawner
\note spawn is the item of the spawn in the world
\note amount holds the maximum number of spawnables
\note morey the minimum delay, morez the maximum delay
\note morex is the xss number to be spawned
*/

void cSpawnDinamic::doSpawn(SPAWNFLAG_ENUM spawnWhat, UI32 amount)
{
	P_ITEM spawn=pointers::findItemBySerial( this->item );
	VALIDATEPI(spawn);
	UI32 amountToSpawn=0;
	if ( spawnWhat == SPAWN_AMOUNT )
		amountToSpawn=amount;
	else if ( spawnWhat == SPAWN_RANDOM )
		amountToSpawn=rand()%(spawn->amount-this->current);
	else if ( spawnWhat == SPAWN_MAX )
		amountToSpawn=spawn->amount-this->current;
	while (amountToSpawn>= 0 && this->current<=spawn->amount )
		doSpawn();

}

void cSpawnDinamic::addSpawned(P_OBJECT obj)
{
	P_ITEM spawn=pointers::findItemBySerial( this->item );
	VALIDATEPI(spawn);
	if( spawn->type == ITYPE_ITEM_SPAWNER ) 
	{
		this->items_spawned.insert( obj->getSerial32() );
		this->current++;
	}
	else if( spawn->type == ITYPE_NPC_SPAWNER ) 
	{
		this->npcs_spawned.insert( obj->getSerial32() );
		this->current++;
	}
}

void cSpawnDinamic::remove( SERIAL serial )
{
	
	if( isCharSerial( serial ) ) {
		SERIAL_SET::iterator iter( this->npcs_spawned.find( serial ) );
		if( iter!=this->npcs_spawned.end() ) 
		{
			this->npcs_spawned.erase( iter );
			if( current>0 )
				current--;
			P_ITEM spawnerItem=pointers::findItemBySerial(this->item);
			if ( ISVALIDPI(spawnerItem))
			{
				spawnerItem->amount2=(UI16)current;
			}

		}
	}
	else {
		SERIAL_SET::iterator iter( this->items_spawned.find( serial ) );
		if( iter!=this->items_spawned.end() ) {
			this->items_spawned.erase( iter );
			if( current>0 )
				current--;
			P_ITEM spawnerItem=pointers::findItemBySerial(this->item);
			if ( ISVALIDPI(spawnerItem))
			{
				spawnerItem->amount2=(UI16)current;
			}
		}
	}

		
}

bool cSpawnDinamic::needSpawn()
{
	return ( TIMEOUT( this->nextspawn ) );//&& ( this->current<this->max
}
