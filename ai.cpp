  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "ai.h"


/*!
\file
\author Luxor
\brief Artificial Intelligence implementation, with a pathfinding algorithm and, in the future, a combat AI
*/

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
	// path_node startNode = { 0, startPos, NULL }; --> does not compile on win vc++

	path_node startNode;
	startNode.cost = 0;
	startNode.pos = startPos;
	startNode.parentNode = NULL;

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
                heuristic = UI32( dist( currNode->parentNode->pos, nextNode->pos, false ) );
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
	LOGICAL bWalkable[ 4 ];

	UI08 num = 0;
	SI08 zAdd = 0;
	Location loc;
	Location pos = node->pos;

	// North - 0
	loc = Loc( pos.x, pos.y - 1, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		bWalkable[ 0 ] = true;
		loc.z = zAdd;
		addToOpenList( loc, node, STRAIGHT_COST );
		num++;
	} else
		bWalkable[ 0 ] = false;

        // South - 1
	loc = Loc( pos.x, pos.y + 1, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		bWalkable[ 1 ] = true;
		loc.z = zAdd;
		addToOpenList( loc, node, STRAIGHT_COST );
		num++;
	} else
		bWalkable[ 1 ] = false;

	// East - 2
	loc = Loc( pos.x + 1, pos.y, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		bWalkable[ 2 ] = true;
		loc.z = zAdd;
		addToOpenList( loc, node, STRAIGHT_COST );
		num++;
	} else
		bWalkable[ 2 ] = false;


	// West - 3
	loc = Loc( pos.x - 1, pos.y, pos.z );
	if ( (zAdd = isWalkable( loc )) != illegal_z ) {
		bWalkable[ 3 ] = true;
		loc.z = zAdd;
		addToOpenList( loc, node, STRAIGHT_COST );
		num++;
	} else
		bWalkable[ 3 ] = false;

	// North-East
	loc = Loc( pos.x + 1, pos.y - 1, pos.z );
	if ( bWalkable[0] && bWalkable[2] ) { // Avoid to go near a tile angle
		if ( (zAdd = isWalkable( loc )) != illegal_z ) {
			loc.z = zAdd;
			addToOpenList( loc, node, OBLIQUE_COST );
			num++;
		}
	}

        // North-West
	loc = Loc( pos.x - 1, pos.y - 1, pos.z );
	if ( bWalkable[0] && bWalkable[3] ) { // Avoid to go near a tile angle
		if ( (zAdd = isWalkable( loc )) != illegal_z ) {
			loc.z = zAdd;
			addToOpenList( loc, node, OBLIQUE_COST );
			num++;
		}
	}

	// South-East
	loc = Loc( pos.x + 1, pos.y + 1, pos.z );
	if ( bWalkable[1] && bWalkable[2] ) { // Avoid to go near a tile angle
		if ( (zAdd = isWalkable( loc )) != illegal_z ) {
			loc.z = zAdd;
			addToOpenList( loc, node, OBLIQUE_COST );
			num++;
		}
	}

	// South-West
	loc = Loc( pos.x - 1, pos.y + 1, pos.z );
	if ( bWalkable[1] && bWalkable[3] ) { // Avoid to go near a tile angle
		if ( (zAdd = isWalkable( loc )) != illegal_z ) {
			loc.z = zAdd;
			addToOpenList( loc, node, OBLIQUE_COST );
			num++;
		}
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

