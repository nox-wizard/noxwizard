  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file
\author Luxor
\brief Header of Artificial Intelligence implementation
*/

//@{
/*!
\name Pathfinding method implementation
*/

#define MAX_PATH_LOOPS 2000
#define Z_COST 1
#define STRAIGHT_COST 10
#define OBLIQUE_COST 14

/*!
\author Luxor
*/
struct path_node {
	UI32 cost;
	Location pos;
	path_node *parentNode;
};

typedef slist<path_node*> NODE_LIST;
typedef slist<Location> LOCATION_LIST;

SI08 isWalkable( Location pos );

/*!
\brief Class implementation of a pathfinding algorithm similar to an A* one
\author Luxor
*/
class cPath {
public:
	cPath( Location startPos, Location finalPos );
	Location getNextPos();
	LOGICAL targetReached();
private:
	UI32 getLocDist( Location loc1, Location loc2, LOGICAL zEnable = false );
	path_node* create_node( Location pos, path_node* parentNode, UI32 cost );
        queue<path_node> nodes_vector;
	UI08 addReachableNodes( path_node* node );
	void dropToClosedList( path_node* node );
	void addToOpenList( Location pos, path_node* parentNode, UI32 cost = 10 );
	void addToOpenList( path_node* node );
	void addToClosedList( path_node* node );
	NODE_LIST open_list;
	NODE_LIST closed_list;
	LOCATION_LIST path_list;
};

void testAI(); // Will be removed soon, test purpose only.


//@}
