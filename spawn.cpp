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

cSpawns*	Spawns;


cSpawnArea::cSpawnArea( areas::AREA_ITER area )
{
	nextspawn=uiCurrentTime;
	where=area;
	current=0;
	items_spawned.clear();
	npcs_spawned.clear();
}

bool cSpawnArea::findValidLocation( Location& location )
{
	int a=0;
	do {

		location.x = RandomNum( where->second.x1, where->second.x2 );
		location.y = RandomNum( where->second.y1, where->second.y2 );
		location.z = Map->MapElevation( location.x, location.y );

		if( Map->CanMonsterMoveHere( location.x, location.y, location.z ) )
			return true;
	} while (++a < 100);
	return false;

}

bool cSpawnArea::needSpawn()
{
	return ( TIMEOUT( nextspawn ) ); // && ( current < ma
}

cSpawnScripted::cSpawnScripted( SERIAL serial )
{
	serial=serial;
}

void cSpawnScripted::safeCreate( P_CHAR npc, cSpawnArea& single  )
{
	VALIDATEPC(npc);

	Location location;
	if( single.findValidLocation( location ) ) {
		npc->npcWander = 3;
		npc->fx1 = single.where->second.x1;
		npc->fy1 = single.where->second.y1;
		npc->fx2 = single.where->second.x2;
		npc->fy2 = single.where->second.y2;
		npc->fz1 = INVALID;
		single.current++;
		single.nextspawn=uiCurrentTime+ (60*RandomNum( mintime, maxtime)*MY_CLOCKS_PER_SEC);
		npc->spawnregion=serial;
		single.npcs_spawned.insert( npc->getSerial32() );
		npc->MoveTo( location );
		npc->teleport();
	}
	else {
		WarnOut("[ERROR] Problem with scripted spawn %i [%s] found. Nothing will be spawned.\n", serial, name.c_str() );
		npc->deleteChar();
	}
}

void cSpawnScripted::safeCreate( P_ITEM pi, cSpawnArea& single  )
{
	VALIDATEPI(pi);
	
	Location location;
	if( single.findValidLocation( location ) ) {
		pi->MoveTo(location);
		single.current++;
		single.nextspawn=uiCurrentTime+ (60*RandomNum( mintime, maxtime)*MY_CLOCKS_PER_SEC);
		pi->spawnregion=serial;
		single.items_spawned.insert( pi->getSerial32() );
		pi->MoveTo( location );
		pi->Refresh();
	}
	else {
		WarnOut("[ERROR] Problem with scripted spawn %i [%s] found. Nothing will be spawned.\n", serial, name.c_str() );
		pi->deleteItem();
	}
}

void cSpawnScripted::doSpawn( cSpawnArea& c ) {
		
	if( c.current >= max ) 
		return;

	if( npclists.size() > 0 )
	{
		UI32 counter = rand()%npclists.size();
		{
			Location location;
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
			Location location;
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
			Location location;
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

	WarnOut("Scripted spawn %i [%s] couldn't find anything to spawn, check scripts.\n",serial, name.c_str());

}

void cSpawnScripted::doSpawn( )
{

	if( max==0 )
		return;

	SPAWNAREA_VECTOR::iterator iter( singles.begin() ), end( singles.end() );
	for( ; iter!=end; iter++ ) {
		if( (*iter).needSpawn() )
			doSpawn( *iter );
	}

}


void cSpawnScripted::doSpawnAll()
{
	if( max==0 )
		return;

	SPAWNAREA_VECTOR::iterator iter( singles.begin() ), end( singles.end() );
	for( ; iter!=end; iter++ ) {
		int loopexit=0;
		while( ( (*iter).current < max ) && ( ++loopexit<MAXLOOPS ) )
			doSpawn( *iter );
	}

}

void cSpawnScripted::removeObject( P_ITEM pi )
{
	SPAWNAREA_VECTOR::iterator iter( singles.begin() ), end( singles.end() );
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
	SPAWNAREA_VECTOR::iterator iter( singles.begin() ), end( singles.end() );
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

cSpawns::cSpawns()
{
	check=uiCurrentTime;
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

		areas::Area area;
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
					{ area.x1=str2num(script2); check|=0x000F; }
				else if("Y1"==script1)
					{ area.y1=str2num(script2); check|=0x00F0; }
				else if("X2"==script1)
					{ area.x2=str2num(script2); check|=0x0F00; }
				else if("Y2"==script1)
					{ area.y2=str2num(script2); check|=0xF000; }
				else if("MINTIME"==script1) 
					dummy->mintime=str2num(script2);
				else if("MAXTIME"==script1) 
					dummy->maxtime=str2num(script2);
				else if("AREA"==script1) {
					areas::AREA_ITER newarea = areas::allareas.find( str2num(script2) );
					if( newarea!=areas::allareas.end() )
						dummy->singles.push_back( cSpawnArea( newarea ) );
					else 
						ConOut("[ERROR] on parse of spawn.xss, can't add a new area\n" );
				}
				else ConOut("[ERROR] on parse of spawn.xss [%s]\n", script1.c_str() );
			}

		}
        while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

		if( check==0xFFFF ) { //use old method x1, x2 so need to add a new region
			SERIAL insarea = areas::insert( area );
			if( insarea!=INVALID ) {
				areas::AREA_ITER newarea = areas::allareas.find( insarea );
				if( newarea!=areas::allareas.end() )
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
	dynamic.clear();
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
	Spawns->dynamic.insert( make_pair( pi->getSerial32(), cSpawnDynamic(pi) ) );
}

void cSpawns::doSpawn()
{
	SPAWN_SCRIPTED_DB::iterator iter_scr( scripted.begin() );
	for( ; iter_scr!=scripted.end(); iter_scr++ ) {
		iter_scr->second.doSpawn();
	}

	SPAWN_DYNAMIC_DB::iterator iter_din( dynamic.begin() );
	for( ; iter_din!=dynamic.end(); iter_din++ ) {
		if( iter_din->second.needSpawn() )
			iter_din->second.doSpawn();
	}

	if( speed.srtime!=UINVALID )
		check = uiCurrentTime+ speed.srtime*MY_CLOCKS_PER_SEC; //Don't check them TOO often (Keep down the lag)
	else
		check = uiCurrentTime+ 30*MY_CLOCKS_PER_SEC;
}

void cSpawns::doSpawnAll()
{
	SPAWN_SCRIPTED_DB::iterator iter( scripted.begin() ), end( scripted.end() );
	for( ; iter!=end; iter++ ) {
		iter->second.doSpawnAll();
	}

	SPAWN_DYNAMIC_DB::iterator iter_din( dynamic.begin() );
	for( ; iter_din!=dynamic.end(); iter_din++ ) {
		iter_din->second.doSpawn();
	}

	if( speed.srtime!=UINVALID )
		check = uiCurrentTime+ speed.srtime*MY_CLOCKS_PER_SEC; //Don't check them TOO often (Keep down the lag)
	else
		check = uiCurrentTime+ 30*MY_CLOCKS_PER_SEC;
}

void cSpawns::doSpawnAll( SERIAL spawn )
{
	SPAWN_SCRIPTED_DB::iterator iter( scripted.find( spawn) );
	if( iter!= scripted.end() )
		iter->second.doSpawnAll();

}


void cSpawns::removeObject( SERIAL spawn, P_ITEM pi )
{
	SPAWN_SCRIPTED_DB::iterator iter( scripted.find( spawn) );
	if( iter!= scripted.end() )
		iter->second.removeObject( pi );
}

void cSpawns::removeObject( SERIAL spawn, P_CHAR pc )
{
	SPAWN_SCRIPTED_DB::iterator iter( scripted.find( spawn) );
	if( iter!= scripted.end() )
		iter->second.removeObject( pc );
}

void cSpawns::removeSpawnDynamic( P_ITEM pi )
{
	VALIDATEPI(pi);
	SPAWN_DYNAMIC_DB::iterator iter( dynamic.find( pi->getSerial32() ) );
	if( iter!=dynamic.end() ) {
		dynamic.erase( iter );
	}
	else {
		if( pi->spawnserial!=INVALID ) {
			iter= dynamic.find( pi->spawnserial );
			if( iter!=dynamic.end() ) {
				iter->second.remove( pi->getSerial32() );
			}
		}
	}

}

void cSpawns::removeSpawnDynamic( P_CHAR pc )
{
	VALIDATEPC(pc);
	if( pc->spawnserial!=INVALID ) {
		SPAWN_DYNAMIC_DB::iterator iter= dynamic.find( pc->spawnserial );
		if( iter!=dynamic.end() ) {
			iter->second.remove( pc->getSerial32() );
		}
	}

}

cSpawnDynamic::cSpawnDynamic( P_ITEM pi )
{
	item=pi->getSerial32();
	item_spawned.clear();
	npc_spawned.clear();
	current=0;
	nextspawn=uiCurrentTime+ (60*RandomNum( pi->morey, pi->morez)*MY_CLOCKS_PER_SEC);
}

void cSpawnDynamic::doSpawn()
{
	P_ITEM spawn=pointers::findItemBySerial( item );
	VALIDATEPI(spawn);

	if( current>=spawn->amount )
		return;
	if( !spawn->isInWorld() )
		return; //npc spawned in container? ahhah i have a gremlin in backpack :P

	if( spawn->type == ITYPE_ITEM_SPAWNER ) {
		
		P_ITEM pi=item::CreateFromScript( spawn->morex );
		if( ISVALIDPI( pi ) ) {
			current++;
			item_spawned.insert( pi->getSerial32() );
			pi->spawnserial=item;
			pi->MoveTo( spawn->getPosition() );
			pi->Refresh();
		}

		nextspawn=uiCurrentTime+ (60*RandomNum( spawn->morey, spawn->morez)*MY_CLOCKS_PER_SEC);
	}
	else if( spawn->type == ITYPE_NPC_SPAWNER ) {
		P_CHAR npc=npcs::addNpc( spawn->morex, spawn->getPosition().x, spawn->getPosition().y, spawn->getPosition().z );
		if(ISVALIDPC( npc )) {
			current++;
			npc_spawned.insert( npc->getSerial32() );
			npc->spawnserial=item;
			npc->MoveTo( spawn->getPosition() );
			npc->teleport();
		}

		nextspawn=uiCurrentTime+ (60*RandomNum( spawn->morey, spawn->morez)*MY_CLOCKS_PER_SEC);
	}

}

void cSpawnDynamic::remove( SERIAL serial )
{
	
	if( isCharSerial( serial ) ) {
		SERIAL_SET::iterator iter( npc_spawned.find( serial ) );
		if( iter!=npc_spawned.end() ) {
			npc_spawned.erase( iter );
			if( current>0 )
				current--;
		}
	}
	else {
		SERIAL_SET::iterator iter( item_spawned.find( serial ) );
		if( iter!=item_spawned.end() ) {
			item_spawned.erase( iter );
			if( current>0 )
				current--;
		}
	}

		
}

bool cSpawnDynamic::needSpawn()
{
	return ( TIMEOUT( nextspawn ) );//&& ( current<max
}
