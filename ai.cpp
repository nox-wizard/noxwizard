  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "ai.h"


bool operator ==(path_node a, path_node b)
{
	return ( (a.pos == b.pos) && (a.parentNode == b.parentNode) && (a.cost == b.cost) );
}

/*!
\file
\author Luxor
\brief Artificial Intelligence implementation, with a pathfinding algorithm and, in the future, a combat AI
*/

/*!
\author Luxor
\brief Looks if a char can walk on the given Location, if false 
*/
SI08 isWalkable( Location pos )
{
	SI08 zRes = 0;
        //
        // DYNAMIC ITEMS -- Check for dynamic items Z elevation and block flag
        //
	NxwItemWrapper si;
	P_ITEM pi = NULL;
	UI32 height = 0;
	si.fillItemsAtXY( pos );

	for( si.rewind(); !si.isEmpty(); si++ )	{
		pi = si.getItem();
		if( !ISVALIDPI(pi) )
			continue;

		tile_st tile;
		Map->SeekTile( pi->id(), &tile );

		height = tile.height;
		if ( tile.flag2 & 0x4 ) // Stairs, ladders
			height = tile.height / 2;
		
                if ( pi->getPosition().z < (pos.z + MaxZstep) ) { // We cannot walk under it
			if ( tile.flag1 & 0x40 ) // Block flag
				return illegal_z;
				
			if ( (pi->getPosition().z + height) <= (pos.z + 4) ) { // We can walk on it
                                if ( (pi->getPosition().z + height) > zRes )
					zRes = pi->getPosition().z + height;
			} else// if ( pi->type != 12 ) // Doors can be opened or avoided by passing under them
				return illegal_z;
                }
	}

        //
        // MAP -- Check for map Z elevation and denied textures (as water, mountains etc)
        //
	SI32 mapid = 0;
	map_st map1 = Map->SeekMap0( pos.x, pos.y );
	mapid = map1.id;

	// Z elevation
	if ( map1.z == illegal_z || map1.z > (pos.z + 4) )
		return illegal_z;
	else if ( map1.z > zRes )
		zRes = map1.z;

	// Denied textures
	if (
		(mapid >= 0x00DC && mapid <= 0x00E7) ||
		(mapid >= 0x00EC && mapid <= 0x00F7) ||
		(mapid >= 0x00FC && mapid <= 0x0107) ||
		(mapid >= 0x010C && mapid <= 0x0117) ||
		(mapid >= 0x011E && mapid <= 0x0129) ||
		(mapid >= 0x0141 && mapid <= 0x0144) ||
		(mapid >= 0x01AF && mapid <= 0x01B0) ||
		(mapid >= 0x01DE && mapid <= 0x01DB) ||
		(mapid >= 0x021F && mapid <= 0x0244) ||
		(mapid >= 0x025A && mapid <= 0x026D) ||
		(mapid >= 0x02BC && mapid <= 0x02CB) ||
		(mapid >= 0x06CD && mapid <= 0x06DD) ||
		(mapid >= 0x06EB && mapid <= 0x073E) ||
		(mapid >= 0x0745 && mapid <= 0x075C) ||
		(mapid >= 0x07BD && mapid <= 0x07D4) ||
		(mapid >= 0x0834 && mapid <= 0x08C6)
		)
		return illegal_z;

	if ( mapid >= 0x0A8 && mapid <= 0x0AB) 	// Water
		return illegal_z;


        //
        // STATIC TILES -- Check for static tiles Z elevation
        //
        MapStaticIterator msi( pos.x, pos.y );
	UI32 loopexit = 0;

	staticrecord *stat;
	while ( ( (stat = msi.Next()) != NULL ) && ( ++loopexit < MAXLOOPS ) ) {
                tile_st tile;

		Map->SeekTile( stat->itemid, &tile );

		// Z elevation
		height = tile.height;
		if ( tile.flag2 & 0x4 ) // Stairs, ladders
			height = tile.height / 2;

		if ( (stat->zoff + height) < (pos.z + MaxZstep) ) { // We cannot walk under it
			if ( tile.flag1 & 0x40 ) // Block flag
				return illegal_z;

			if ( (stat->zoff + height) <= (pos.z + 4) ) { // We can walk on it
                                if ( (stat->zoff + tile.height) > zRes )
					zRes = stat->zoff + tile.height;
			} else
				return illegal_z;
                }
	}


	return zRes;
}

/*!
\author Luxor
*/
UI32 cPath::getLocDist( Location loc1, Location loc2, LOGICAL zEnable )
{
        SI16 x1 = loc1.x, y1 = loc1.y, x2 = loc2.x, y2 = loc2.y;
	double distance = hypot( abs(x2-x1), abs(y2-y1) );
	if ( !zEnable )
		return UI32( distance );
	else {
		SI16 z1 = loc1.z, z2 = loc2.z;
		UI32 dz = abs( z1 - z2 );
                if ( dz == 0 )
			return UI32( distance );
		distance = hypot( distance, dz );
		return UI32( distance );
	}
}

/*!
\author Luxor
*/
path_node* cPath::create_node( Location pos, path_node* parentNode, UI32 cost )
{
	path_node *node, newnode;

	newnode.cost = parentNode->cost + cost;
	newnode.pos = pos;
	newnode.parentNode = parentNode;
	nodes_vector.push( newnode );

	node = &(nodes_vector.back());
	return node;
}

/*!
\author Luxor
*/
cPath::cPath( Location startPos, Location finalPos )
{
	open_list.clear();
	closed_list.clear();
	path_list.clear();

	m_finalPos = finalPos;

	NODE_LIST::iterator it;
	UI32 min_cost, curr_cost, heuristic, loops = 0;
	LOGICAL bFound = false;
	path_node *currNode, *nextNode;

        // Create the start node and put it in the open list
	path_node startNode = { 0, startPos, NULL };

	startNode.parentNode = &startNode;
	
	addToOpenList( &startNode );
		
	currNode = &startNode;

	while( loops < MAX_PATH_LOOPS && bFound == false ) {
                if ( currNode->pos == finalPos ) {
			bFound = true;
			break;
		}
		// Look for tiles reachable by currNode, add them to the open list
		addReachableNodes( currNode );
		
		// Drop the current node in the closed list
		dropToClosedList( currNode );

		if ( open_list.empty() )
			break;

		for ( it = open_list.begin(); it != open_list.end(); it++ ) {
			if ( it == open_list.begin() ) {
				min_cost = (*it)->cost + ( UI32( dist( (*it)->pos, finalPos ) ) * OBLIQUE_COST );
				if ( (*it)->pos.z != finalPos.z )
					min_cost += Z_COST * (abs( SI16((*it)->pos.z - finalPos.z )) / 2);
				nextNode = (*it);
				continue;
			}

			curr_cost = (*it)->cost + UI32( dist( (*it)->pos, finalPos ) ) * OBLIQUE_COST;
			if ( (*it)->pos.z != finalPos.z )
				curr_cost += Z_COST * (abs( SI16((*it)->pos.z - finalPos.z) ) / 2);
			if ( curr_cost < min_cost ) {
				min_cost = curr_cost;
				nextNode = (*it);
			}
			loops++;
		}

		//
                // Heuristic and possible path improvement
                //
                heuristic = getLocDist( currNode->parentNode->pos, nextNode->pos );
                if ( heuristic == 1 ) { // The nodes are adjacent
			heuristic = 0;
                        Location parent = currNode->parentNode->pos;
                        Location next = nextNode->pos;
                        LOGICAL bOk = false;

			if ( abs( SI16(parent.x - next.x) ) + abs( SI16(parent.y - next.y) ) == 1 )
				heuristic = STRAIGHT_COST;
			else
				heuristic = OBLIQUE_COST;

			if ( heuristic < abs( SI16(nextNode->cost - currNode->parentNode->cost) ) )
				bOk = true;
                        if ( parent.z != next.z ) {
				next.z = parent.z;
				if ( isWalkable( next ) == illegal_z ) // nextNode is not walkable by parentNode
					bOk = false;
			}
                        if ( bOk )
				nextNode->parentNode = currNode->parentNode;
		}
		currNode = nextNode;
		loops++;
	}

	while( currNode->pos != startPos && currNode->cost != 0 && loops >= 0 ) {
		path_list.push_front( currNode->pos );
		currNode = currNode->parentNode;
		loops--;
	}
	path_list.push_front( currNode->pos );
}

/*!
\author Luxor
\brief Looks for every tile reachable walking by pos, and adds them to the open list
*/
UI08 cPath::addReachableNodes( path_node* node )
{       
	UI08 num = 0;
	SI08 zAdd = 0;
	Location loc;
	Location pos = node->pos;
	
	// East
	loc = Loc( pos.x + 1, pos.y, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
                loc.z = zAdd;
		addToOpenList( loc, node, STRAIGHT_COST );
		num++;
	}


	// West
	loc = Loc( pos.x - 1, pos.y, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		loc.z = zAdd;
		addToOpenList( loc, node, STRAIGHT_COST );
		num++;
	}

        // South
	loc = Loc( pos.x, pos.y + 1, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		loc.z = zAdd;
		addToOpenList( loc, node, STRAIGHT_COST );
		num++;
	}
	
	// North
	loc = Loc( pos.x, pos.y - 1, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		loc.z = zAdd;
		addToOpenList( loc, node, STRAIGHT_COST );
		num++;
	}

	// North-East
	loc = Loc( pos.x + 1, pos.y - 1, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		loc.z = zAdd;
		addToOpenList( loc, node, OBLIQUE_COST );
		num++;
	}
        	
        // North-West
	loc = Loc( pos.x - 1, pos.y - 1, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		loc.z = zAdd;
		addToOpenList( loc, node, OBLIQUE_COST );
		num++;
	}

	// South-East
	loc = Loc( pos.x + 1, pos.y + 1, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		loc.z = zAdd;
		addToOpenList( loc, node, OBLIQUE_COST );
		num++;
	}

	// South-West
	loc = Loc( pos.x - 1, pos.y + 1, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		loc.z = zAdd;
		addToOpenList( loc, node, OBLIQUE_COST );
		num++;
	}

	return num;
}

/*!
\author Luxor
\brief Adds an element to the open list
*/
void cPath::addToOpenList( Location pos, path_node* parentNode, UI32 cost )
{
	path_node* node = create_node( pos, parentNode, cost );
	addToOpenList( node );
}

/*!
\author Luxor
\brief Adds an element to the open list
*/
void cPath::addToOpenList( path_node* node )
{
	// Don't add it if it's already present in one of the two lists
        if ( find( open_list.begin(), open_list.end(), node ) != open_list.end() )
		return;

	if ( find( closed_list.begin(), closed_list.end(), node ) != closed_list.end() )
		return;

	open_list.push_front( node );
}

/*!
\author Luxor
\brief Adds an element to the closed list
*/
void cPath::addToClosedList( path_node* node )
{
        // Don't add it if it's already present
        if ( find( closed_list.begin(), closed_list.end(), node ) != closed_list.end() )
		return;

	closed_list.push_front( node );
}

/*!
\author Luxor
\brief Insert an element in the closed list, and removes it from the open list if present.
*/
void cPath::dropToClosedList( path_node* node )
{
	addToClosedList( node );
        
	NODE_LIST::iterator it( find( open_list.begin(), open_list.end(), node ) );
	if ( it != open_list.end() )
		open_list.erase( it );
}

/*!
\author Luxor
*/
Location cPath::getNextPos()
{
	if( path_list.empty() )
		return Loc( 0, 0, 0 );
	Location pos = *(path_list.begin());
	path_list.pop_front();
	return pos;
}

/*!
\author Luxor
*/
LOGICAL cPath::targetReached()
{
	return path_list.empty();
}
