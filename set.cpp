  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "race.h"
#include "set.h"
#include "layer.h"

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Purpose : define standard container for a set of objects which responds to common
  queried sets, like items worn by a char or chars near a point. Access available
  both to AMX and native (C++) code. Last 16 sets are reserved to C++ code, so
  unclosed AMX sets will make nxw unable to create new AMX sets, but will keep C++
  code depending on sets working.

  Author : Xanathar.
  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


namespace nxwset {

AMX_WRAPPER_DB g_oSet;
static SERIAL currentIndex=INVALID;

AmxWrapper::AmxWrapper() {
	this->obsolete=uiCurrentTime;
	this->p_set=NULL;
}

AmxWrapper::~AmxWrapper() {
	if( this->p_set!=NULL )
		safedelete(p_set);
}

SERIAL open ()
{
	currentIndex++;

	SERIAL iSet=currentIndex;
	g_oSet[iSet].obsolete=uiCurrentTime+15*60*MY_CLOCKS_PER_SEC; //obsolete after 15min

	return iSet;
}


void close( SERIAL iSet )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		g_oSet.erase( iter );
}

bool end( SERIAL iSet )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second.p_set!=NULL ) {
			return iter->second.p_set->isEmpty();
		}
	return true;
}

void rewind( SERIAL iSet )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second.p_set!=NULL ) {
			iter->second.p_set->rewind();
		}
}

void next( SERIAL iSet )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second.p_set!=NULL ) {
			(*iter->second.p_set)++;
		}
}


SERIAL get( SERIAL iSet)
{
	SERIAL ser = INVALID;
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second.p_set!=NULL ) {
			ser = iter->second.p_set->get();
			(*iter->second.p_set)++;
		}
	return ser;
}


void insert( SERIAL iSet, SERIAL nVal )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second.p_set!=NULL )
			iter->second.p_set->insert(nVal);
}

bool isEmpty( SERIAL iSet )
{
	bool empty = true;
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second.p_set!=NULL )
			empty = iter->second.p_set->isEmpty();
	return empty;
}


UI32 size( SERIAL iSet)
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second.p_set!=NULL )
			return iter->second.p_set->size();
	return 0;
}


void fillItemsInContainer ( SERIAL iSet, P_ITEM pi, bool bIncludeSubContained, bool bIncludeOnlyFirstSubcont)
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter==g_oSet.end() )
		return;

	iter->second.p_set = new NxwItemWrapper;
	((NxwItemWrapper*)(iter->second.p_set))->fillItemsInContainer( pi, bIncludeSubContained, bIncludeOnlyFirstSubcont );
	((NxwItemWrapper*)(iter->second.p_set))->rewind();

}

void fillOwnedNpcs ( SERIAL iSet, P_CHAR pc, bool bIncludeStabled, bool bOnlyFollowing)
{

	VALIDATEPC(pc);
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter==g_oSet.end() )
		return;

	iter->second.p_set = new NxwCharWrapper;
	((NxwCharWrapper*)(iter->second.p_set))->fillOwnedNpcs( pc, bIncludeStabled, bOnlyFollowing );
	((NxwCharWrapper*)(iter->second.p_set))->rewind();

}


void fillCharsNearXYZ (SERIAL iSet, UI16 x, UI16 y, int nDistance, bool bExcludeOfflinePlayers)
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter==g_oSet.end() )
		return;

	iter->second.p_set = new NxwCharWrapper;
	((NxwCharWrapper*)(iter->second.p_set))->fillCharsNearXYZ( x, y, nDistance, bExcludeOfflinePlayers );
	((NxwCharWrapper*)(iter->second.p_set))->rewind();

}


void fillItemsAtXY( SERIAL iSet, UI16 x, UI16 y, int type, int id)
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter==g_oSet.end() )
		return;

	iter->second.p_set = new NxwItemWrapper;
	((NxwItemWrapper*)(iter->second.p_set))->fillItemsAtXY( x, y, type, id );
	((NxwItemWrapper*)(iter->second.p_set))->rewind();

}

void fillItemsNearXYZ ( SERIAL iSet, UI16 x, UI16 y, int nDistance, bool bExcludeNotMovableItems)
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter==g_oSet.end() )
		return;

	iter->second.p_set = new NxwItemWrapper;
	((NxwItemWrapper*)(iter->second.p_set))->fillItemsAtXY( x, y, bExcludeNotMovableItems );
	((NxwItemWrapper*)(iter->second.p_set))->rewind();

}

void fillOnlineSockets( SERIAL iSet, SERIAL onlyNearThis, UI32 range )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter==g_oSet.end() )
		return;

	P_CHAR pc = pointers::findCharBySerial( onlyNearThis );
	iter->second.p_set = new NxwSocketWrapper;
	if( ISVALIDPC( pc ) )
		((NxwSocketWrapper*)(iter->second.p_set))->fillOnline( pc, range );
	else
		((NxwSocketWrapper*)(iter->second.p_set))->fillOnline( );
	
	((NxwSocketWrapper*)(iter->second.p_set))->rewind();

}


} //namespace


/*!
\brief Constructor
*/
NxwWrapper::NxwWrapper() { 
	this->rewind();
};

/*!
\brief Destructor
*/
NxwWrapper::~NxwWrapper() {
};


/*!
\brief Check if empty
\author Endymion
\return true if empty
*/
bool NxwWrapper::isEmpty()
{
	return ( ( this->vect.size()==0 ) || ( current==end ) );	
};

/*!
\brief Move to next
\author Endymion
\return the object itself (standard)
*/
NxwWrapper& NxwWrapper::operator++(int)
{
	next();
	return (*this);
}

/*!
\brief Clear all list
\author Endymion
*/
void NxwWrapper::clear()
{
	vect.clear();
	this->rewind();
}

/*!
\brief Get size of list
\author Endymion
\return size
*/
UI32 NxwWrapper::size()
{
	return vect.size();
}

/*!
\brief Move to next value
\author Endymion
*/
void NxwWrapper::next()
{
	current++;
}

/*!
\brief Reset internal counter
\author Endymion
*/
void NxwWrapper::rewind()
{
	this->current=vect.begin();
	this->end=vect.end();
}

/*!
\brief Get the current value and after move to next
\author Endymion
\return the current value
*/
SERIAL NxwWrapper::get()
{
	if( isEmpty() )
		return INVALID;
	else {
		return (*current);
	}

};

/*!
\brief Return a copy of the queue
\author Sparhawk
\param from the origin of the queue
*/
void NxwWrapper::copyQ( const NxwWrapper& from )
{
	vect = from.vect;
}

/*!
\brief Insert a new value
\author Endymion
\param s the value
*/
void NxwWrapper::insert( SERIAL s )
{
	vect.insert( s );

};


/*!
\brief Constructor
*/
NxwSerialWrapper::NxwSerialWrapper() { };

/*!
\brief Destructor
*/
NxwSerialWrapper::~NxwSerialWrapper() { };


/*!
\brief Get the current serial
\author Endymion
\return the current serial
*/
SERIAL NxwSerialWrapper::getSerial()
{
	return get();
};

/*!
\bref Insert a new serial
\author Endymion
\param s the serial
*/
void NxwSerialWrapper::insertSerial( SERIAL s )
{
	if( s!=INVALID )
		insert( s );
};

/*!
\bref Insert a new serial
\author Endymion
\param obj the object
*/
void NxwSerialWrapper::insertSerial( cObject* obj )
{
	if( (obj!=NULL) )
		this->insertSerial( obj->getSerial32() );
};


/*!
\bref Fills with serial in a container
\author Endymion
\param serial the serial
\param bIncludeSubContained if true recurse subcontainers
\param bIncludeOnlyFirstSubcont if true only recurse first sub container
*/
void NxwSerialWrapper::fillSerialInContainer( SERIAL serial, bool bIncludeSubContained, bool bIncludeOnlyFirstSubcont )
{
	
	std::map< SERIAL , vector<P_ITEM> >::iterator cont( pointers::pContMap.find( serial ) );
	if( cont==pointers::pContMap.end() || cont->second.empty() )
		return;

	std::vector<P_ITEM>::iterator iter( cont->second.begin() );
	for( ; iter!=cont->second.end(); iter++ )
	{
	
		P_ITEM pi=(*iter);

		insertSerial(pi);
		if (pi->type==ITYPE_CONTAINER)		// a subcontainer ?
		{
			if (bIncludeSubContained ) {
				fillSerialInContainer( pi, bIncludeOnlyFirstSubcont ? false : bIncludeSubContained);
			} //Endymion bugfix, becuase not added to set before :[
		}
	}
}

/*!
\bref Fills with serial in a container
\author Endymion
\param obj the object
\param bIncludeSubContained if true recurse subcontainers
\param bIncludeOnlyFirstSubcont if true only recurse first sub container
*/
void NxwSerialWrapper::fillSerialInContainer( cObject* obj, bool bIncludeSubContained, bool bIncludeOnlyFirstSubcont )
{
	if( obj!=NULL )
		fillSerialInContainer( obj->getSerial32(), bIncludeSubContained, bIncludeOnlyFirstSubcont );
}


/*!
\brief Constructor
*/
NxwCharWrapper::NxwCharWrapper() { };

NxwCharWrapper::NxwCharWrapper( const NxwCharWrapper& that )
{
	copyQ( that );
}

/*!
\brief Destructor
*/
NxwCharWrapper::~NxwCharWrapper() { };

/*!
\brief Get the current char and after move to next
\author Endymion
\return ptr to the current char
*/
P_CHAR NxwCharWrapper::getChar()
{
	return pointers::findCharBySerial( getSerial() );
};

/*!
\brief Insert a new char
\author Endymion
\param pc the char
*/
void NxwCharWrapper::insertChar( P_CHAR pc )
{
	VALIDATEPC(pc);
	insertSerial( pc->getSerial32() );
};

/*!
\brief Fills a set with a list of npcs owned by a char
\author Endymion
\param pc the char
\param bIncludeStabled if true stabled pets should be included
\param bOnlyFollowing if true only following pets should be included
\warning this function ADD new char to current list
*/
void NxwCharWrapper::fillOwnedNpcs( P_CHAR pc, bool bIncludeStabled, bool bOnlyFollowing )
{

	VALIDATEPC( pc );

	std::map< SERIAL, std::vector< P_CHAR > >::iterator vect( pointers::pOwnCharMap.find( pc->getSerial32() ) );
	if( ( vect!=pointers::pOwnCharMap.end() ) && !vect->second.empty() ) {

		std::vector< P_CHAR >::iterator iter( vect->second.begin() );
		for( ; iter!=vect->second.end(); iter++ ) {
			P_CHAR poOwnedChr = (*iter);
		
			if(ISVALIDPC(poOwnedChr))
			{
				if ((poOwnedChr->ftargserial==pc->getSerial32()) || 
					( !bOnlyFollowing && bIncludeStabled && ( poOwnedChr->isStabled() ) ) ) {
					insertSerial(poOwnedChr->getSerial32());
				} 
			}
		}
	} 
}

/*!
\brief Fills a set with a list of char near x, y
\author Endymion
\param x the x location
\param y the y location
\param nDistance the distance requested
\param bExcludeOfflinePlayers if true exclude offline players from search
\warning this function ADD new char to current list
*/
void NxwCharWrapper::fillCharsNearXYZ ( UI16 x, UI16 y, int nDistance, bool bExcludeOfflinePlayers, bool bOnlyPlayer )
{

	if(!mapRegions->isValidCoord( x, y ))
		return;

	for( SI32 ix=x-REGION_GRIDSIZE; ix<=x+REGION_GRIDSIZE; ix+=REGION_GRIDSIZE ) {
		if( ix>=0 ) {
			for( SI32 iy=y-REGION_COLSIZE; iy<=y+REGION_COLSIZE; iy+=REGION_COLSIZE ) {
				if( iy>=0 && mapRegions->isValidCoord( x, y ) ) {
					UI16 nowx = ix/REGION_GRIDSIZE, nowy= iy/REGION_COLSIZE;

					if( mapRegions->regions[nowx][nowy].charsInRegions.empty() )
						continue;
					
					SERIAL_SET::iterator iter( mapRegions->regions[nowx][nowy].charsInRegions.begin() );
					for( ; iter!=mapRegions->regions[nowx][nowy].charsInRegions.end(); iter++ ) {
						P_CHAR pc=pointers::findCharBySerial( *iter );
						if( !ISVALIDPC( pc ) )
							continue;
						if(  !pc->isStabled() && !pc->mounted ) {
							int iDist=(int)dist(x,y,0,pc->getPosition().x,pc->getPosition().y,0);
							if (iDist <= nDistance)
								if ( ( !bOnlyPlayer && pc->npc ) ||
									( !bExcludeOfflinePlayers || pc->IsOnline() ) )
									this->insertSerial(pc->getSerial32());
						}
					}
				}
			}
		}
	}

	
}


/*!
\brief Fills a set with a list of char near location
\author Endymion
\param location the location
\param nDistance the distance requested
\param bExcludeOfflinePlayers if true exclude offline players from search
\warning this function ADD new char to current list
*/
void NxwCharWrapper::fillCharsNearXYZ ( Location location, int nDistance, bool bExcludeOfflinePlayers , bool bOnlyPlayer )
{
	fillCharsNearXYZ( location.x, location.y, nDistance, bExcludeOfflinePlayers );
}

/*!
\brief Fills a set with a list of char in same party of given char
\author Endymion
\param pc the player
\param nDistance the distance requested
\param bExcludeThis if true exclude this
\param nDistance maximum distance from the player
\warning this function ADD new char to current list
\note offline player are not added
*/
void NxwCharWrapper::fillPartyFriend( P_CHAR pc, UI32 nDistance, bool bExcludeThis )
{
	VALIDATEPC(pc);
	if( pc->party==INVALID )
		return;
	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( pc->getPosition(), nDistance, true, true );
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pj=sc.getChar();
		if( ISVALIDPC(pj) && pc->party==pj->party ) {
			if( pc->distFrom( pj ) <= nDistance ) {
				if( !bExcludeThis || ( pc->getSerial32()!=pj->getSerial32() ) )
					this->insert( pj->getSerial32() );
			}
		}
	}	
}

/*!
\brief Constructor
*/
NxwItemWrapper::NxwItemWrapper() { };

NxwItemWrapper::NxwItemWrapper( const NxwItemWrapper& that )
{
	copyQ( that );
}

/*!
\brief Destructor
*/
NxwItemWrapper::~NxwItemWrapper() { };

/*!
\brief Get the current item
\author Endymion
\return the current item
*/
P_ITEM NxwItemWrapper::getItem()
{
	return pointers::findItemBySerial( getSerial() );
};

/*!
\brief Insert a new item
\author Endymion
\param pi the item
*/
void NxwItemWrapper::insertItem( P_ITEM pi )
{
	VALIDATEPI(pi);
	insertSerial( pi->getSerial32() );
};

/*!
\bref Fills with item in a container
\author Endymion
\param pi the container
\param bIncludeSubContained if true recurse subcontainers
\param bIncludeOnlyFirstSubcont if true only recurse first sub container
*/
void NxwItemWrapper::fillItemsInContainer( P_ITEM pi, bool bIncludeSubContained, bool bIncludeOnlyFirstSubcont )
{
	VALIDATEPI(pi);
	fillSerialInContainer( pi, bIncludeSubContained, bIncludeOnlyFirstSubcont );
}


/*!
\brief Fills with a list of item at given location
\author Endymion
\param x the x location
\param y the y location
\param type if not INVALID only add item with this type
\param id if not INVALID only add item with this id
\warning this function ADD new char to current list
*/
void NxwItemWrapper::fillItemsAtXY( UI16 x, UI16 y, UI32 type, SI32 id )
{
	
	if(!mapRegions->isValidCoord( x, y ))
		return;

	UI16 nowx = x/REGION_GRIDSIZE, nowy= x/REGION_COLSIZE;

	if( mapRegions->regions[nowx][nowy].itemsInRegions.empty() )
		return;

	SERIAL_SET::iterator iter( mapRegions->regions[nowx][nowy].itemsInRegions.begin() );
	for( ; iter!= mapRegions->regions[nowx][nowy].itemsInRegions.end(); iter++ ) {
		P_ITEM pi=pointers::findItemBySerial( *iter );
		if( ISVALIDPI( pi ) && pi->isInWorld() )
		{
			if ( type==(UI32)INVALID || pi->type==type )
				if ( id == INVALID || pi->id() == id )
					insertItem(pi);
		}
	}
}

/*!
\brief Fills with a list of item at given location
\author Endymion
\param location the location
\param type if not INVALID only add item with this type
\param id if not INVALID only add item with this id
\warning this function ADD new char to current list
*/
void NxwItemWrapper::fillItemsAtXY( Location location, UI32 type, SI32 id )
{
	fillItemsAtXY( location.x, location.y, type, id ); 
}

/*!
\brief Fills with a list of item near given location
\author Endymion
\param x the x location
\param y the y location
\param nDistance only add item in distance range
\param bExcludeNotMovableItems if true exluce not movable items
\warning this function ADD new char to current list
*/
void NxwItemWrapper::fillItemsNearXYZ ( UI16 x, UI16 y, int nDistance, bool bExcludeNotMovableItems )
{
	if(!mapRegions->isValidCoord( x, y ))
		return;

	for( SI32 ix=x-REGION_GRIDSIZE; ix<=x+REGION_GRIDSIZE; ix+=REGION_GRIDSIZE ) {
		if( ix>=0 ) {
			for( SI32 iy=y-REGION_COLSIZE; iy<=y+REGION_COLSIZE; iy+=REGION_COLSIZE ) {
				if( iy>=0 && mapRegions->isValidCoord( x, y ) ) {
					UI16 nowx = ix/REGION_GRIDSIZE, nowy= iy/REGION_COLSIZE;

					if( mapRegions->regions[nowx][nowy].itemsInRegions.empty() )
						continue;
					
					SERIAL_SET::iterator iter( mapRegions->regions[nowx][nowy].itemsInRegions.begin() );
					for( ; iter!=mapRegions->regions[nowx][nowy].itemsInRegions.end(); iter++ ) {
						P_ITEM pi=pointers::findItemBySerial( *iter );
						if(ISVALIDPI(pi) && pi->isInWorld() ) {

							int iDist=(int)dist(x,y,0, pi->getPosition("x"), pi->getPosition("y"), 0 );
							if (iDist <= nDistance) {
								if ((!bExcludeNotMovableItems) || (pi->magic != 2 && pi->magic != 3))
								{
									insertItem(pi);
								} //if Distance
							}
						}
					}
				}
			}
		}
	}
}

/*!
\brief Fills with a list of item near given location
\author Endymion
\param location the location
\param nDistance only add item in distance range
\param bExcludeNotMovableItems if true exluce not movable items
\warning this function ADD new char to current list
*/
void NxwItemWrapper::fillItemsNearXYZ ( Location location, int nDistance, bool bExcludeNotMovableItems )
{
	fillItemsNearXYZ( location.x, location.y, nDistance, bExcludeNotMovableItems );
}

/*!
\brief Fills a set with a list of item weared by given char
\author Endymion
\param pc the char
\param bExcludeIllegalLayer if true layer like backpack, trade are excluded
\param bIncludeLikeHair if true add also hair, beard ecc
\param bIncludeProtectedLayer if true add also protected layer
\warning this function ADD new char to current list
*/
void NxwItemWrapper::fillItemWeared( P_CHAR pc, bool bIncludeLikeHair, bool bIncludeProtectedLayer, bool bExcludeIllegalLayer )
{
	
	VALIDATEPC(pc);

	std::map< SERIAL , vector<P_ITEM> >::iterator cont( pointers::pContMap.find( pc->getSerial32() ) );
	if( cont==pointers::pContMap.end() || cont->second.empty() )
		return;

	std::vector<P_ITEM>::iterator iter( cont->second.begin() );
	for( ; iter!=cont->second.end(); iter++ )
	{
	
		P_ITEM pi_j=(*iter);
		if(!ISVALIDPI(pi_j) )			// just to be sure ;-)
			continue;
		//is trade studd or bank ( ndEndy not need to remove it? )
		if(bExcludeIllegalLayer && ( pi_j->layer==LAYER_TRADE_RESTOCK || pi_j->layer==LAYER_TRADE_NORESTOCK || pi_j->layer==LAYER_TRADE_BOUGHT || pi_j->layer==LAYER_BANKBOX ) )  
			continue;
		//beard and hair not moving to corpse
		if(!bIncludeLikeHair && ( pi_j->layer == LAYER_BEARD || pi_j->layer==LAYER_HAIR ) )
			continue;
		//race protected
		if( Race::isRaceSystemActive() )
			if (!bIncludeProtectedLayer && ( Race::isProtectedLayer( (UI32) pc->race, pi_j->layer ) ) ) 
				continue;
		this->insertSerial( pi_j->getSerial32() );
	}

}

/*!
\brief Constructor
*/
NxwSocketWrapper::NxwSocketWrapper() { };

/*!
\brief Destructor
*/
NxwSocketWrapper::~NxwSocketWrapper() { };

/*!
\brief Get the current socket
\author Endymion
\return the current socket
*/
SERIAL NxwSocketWrapper::getSocket()
{
	return get();
}

/*!
\brief Get the current client
\author Endymion
\return the current client
*/
NXWCLIENT NxwSocketWrapper::getClient()
{
	if( isEmpty() )
		return NULL;
	else {
		return getClientFromSocket( get() );
	}
}

/*!
\brief Insert a new socket
\author Endymion
\param s the socket
*/
void NxwSocketWrapper::insertSocket( NXWSOCKET s )
{
	if( s!=INVALID )
		insert( s );
}

/*!
\brief Insert a new client
\author Endymion
\param ps the client
*/
void NxwSocketWrapper::insertClient( NXWCLIENT ps )
{
	if( ps!=NULL )
		insertSocket( ps->toInt() );
}


/*!
\brief Fills with a list of socket
\author Endymion, rewritten by Luxor
\param onlyNearThis if true only socket near given char are added
\param bExcludeThis if true given char is not added to list
\param nDistance maximum distance from the character
\warning this function ADD new char to current list
\note Akron - Changed nDistance to UI32 and doxygen documented it
*/
void NxwSocketWrapper::fillOnline( P_CHAR onlyNearThis, bool bExcludeThis, UI32 nDistance )
{

	for (SI32 i = 0; i < now; i++) {
		SERIAL serial = currchar[i];
		if (serial < 0) continue;
		P_CHAR pc = pointers::findCharBySerial(serial);
		if (!ISVALIDPC(pc)) continue;

		if (!ISVALIDPC(onlyNearThis)) {
			insertSocket(i);
		} else {
			if (onlyNearThis->distFrom(pc) <= nDistance )
				if( !(bExcludeThis && ( pc->getSerial32()==onlyNearThis->getSerial32() ) ) )
					insertSocket(i);
		}
	}
}

/*!
\brief Fills with a list of socket
\author Endymion, rewritten by Luxor
\param location only socket near given location are added
\warning this function ADD new char to current list
*/
void NxwSocketWrapper::fillOnline( Location location, int nDistance )
{

	SERIAL serial = INVALID;
	P_CHAR pc = NULL;
	for (SI32 i = 0; i < now; i++) {
		serial = currchar[i];
		if (serial < 0) continue;
		pc = pointers::findCharBySerial(serial);
		if (!ISVALIDPC(pc)) continue;
		
		if (dist(location, pc->getPosition()) <= nDistance)
			insertSocket(i);
	}
}

/*!
\brief Fills with a list of socket
\author Endymion
\param onlyNearThis only socket near given item are added
\warning this function ADD new char to current list
*/
void NxwSocketWrapper::fillOnline( P_ITEM onlyNearThis, int nDistance )
{
	VALIDATEPI(onlyNearThis);

	P_ITEM out=onlyNearThis->getOutMostCont();
	if( out->isInWorld() ) {
		fillOnline(onlyNearThis->getPosition(), nDistance );
	}
	else {
		P_CHAR own=pointers::findCharBySerial( out->getContSerial() );
		if( ISVALIDPC( own ) )
			fillOnline( own->getPosition(), nDistance );
		else
			fillOnline( );
	}

}

/*!
\brief Fills with a list of socket
\author Endymion, rewritten by Luxor
\warning this function ADD new char to current list
*/
void NxwSocketWrapper::fillOnline(  )
{

	for (SI32 i = 0; i < now; i++) {
		if (currchar[i] != INVALID ) 
			insertSocket(i);
	}

}
