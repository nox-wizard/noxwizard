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
#include "chars.h"
#include "items.h"
#include "range.h"
#include "network.h"

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Purpose : define standard container for a set of objects which responds to common
  queried sets, like items worn by a char or chars near a point. Access available
  both to AMX and native (C++) code. Last 16 sets are reserved to C++ code, so
  unclosed AMX sets will make nxw unable to create new AMX sets, but will keep C++
  code depending on sets working.

  Author : Xanathar.
  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


namespace amxSet {

AMX_WRAPPER_DB g_oSet;
static SERIAL currentIndex=INVALID;

SERIAL create( )
{
	++currentIndex;

	SERIAL iSet=currentIndex;
	g_oSet.insert( make_pair( iSet, new NxwWrapper() ) );
	return iSet;
}


void deleteSet( SERIAL iSet )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		g_oSet.erase( iter );
}

bool end( SERIAL iSet )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second!=NULL ) {
			return iter->second->isEmpty();
		}
	return true;
}

void rewind( SERIAL iSet )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second!=NULL ) {
			iter->second->rewind();
		}
}

void next( SERIAL iSet )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second!=NULL ) {
			(*iter->second)++;
		}
}


SERIAL get( SERIAL iSet)
{
	SERIAL ser = INVALID;
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second!=NULL ) {
			ser = iter->second->get();
			(*iter->second)++;
		}
	return ser;
}


void add( SERIAL iSet, SERIAL nVal )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second!=NULL )
			iter->second->insert(nVal);
}

void copy( SERIAL iSet, const NxwWrapper& from )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second!=NULL )
			iter->second->copyQ( from );
}

UI32 size( SERIAL iSet)
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter!=g_oSet.end() )
		if( iter->second!=NULL )
			return iter->second->size();
	return 0;
}


void addOwnedNpcs( SERIAL iSet, P_CHAR pc, bool includeStabled, bool onlyFollowing )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwCharWrapper* sc=static_cast<NxwCharWrapper*>(iter->second);
		sc->fillOwnedNpcs( pc, includeStabled, onlyFollowing );
		sc->rewind();
	}
}

void addNpcsNearXY( SERIAL iSet, UI16 x, UI16 y, int distance )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwCharWrapper* sc=static_cast<NxwCharWrapper*>(iter->second);
		sc->fillNpcsNearXY( x, y, distance );
		sc->rewind();
	}
}

void addNpcsNearObj( SERIAL iSet, P_CHAR pc, int nDistance )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwCharWrapper* sc=static_cast<NxwCharWrapper*>(iter->second);
		sc->fillNpcsNear( pc, nDistance );
		sc->rewind();
	}
}

void addNpcsNearObj( SERIAL iSet, P_ITEM pi, int nDistance )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwCharWrapper* sc=static_cast<NxwCharWrapper*>(iter->second);
		sc->fillNpcsNear( pi, nDistance );
		sc->rewind();
	}
}

void addPartyFriend( SERIAL iSet, P_CHAR pc, int distance, bool excludeThis )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwCharWrapper* sc=static_cast<NxwCharWrapper*>(iter->second);
		sc->fillPartyFriend( pc, distance, excludeThis );
		sc->rewind();
	}
}

void addItemsInContainer( SERIAL iSet, P_ITEM pi, bool includeSubCont, bool includeOnlyFirstSubCont )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwItemWrapper* si=static_cast<NxwItemWrapper*>(iter->second);
		si->fillItemsInContainer( pi, includeSubCont, includeOnlyFirstSubCont );
		si->rewind();
	}
}

void addItemWeared( SERIAL iSet, P_CHAR pc, bool includeLikeHair, bool includeProtectedLayer, bool excludeIllegalLayer )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwItemWrapper* si=static_cast<NxwItemWrapper*>(iter->second);
		si->fillItemWeared( pc, includeLikeHair, includeProtectedLayer, excludeIllegalLayer );
		si->rewind();
	}
}

void addItemsAtXY( SERIAL iSet, UI16 x, UI16 y, UI32 type )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwItemWrapper* si=static_cast<NxwItemWrapper*>(iter->second);
		si->fillItemsAtXY( x, y, type );
		si->rewind();
	}
}

void addItemsNearXY( SERIAL iSet, UI16 x, UI16 y, int distance, bool excludeNotMovable )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwItemWrapper* si=static_cast<NxwItemWrapper*>(iter->second);
		si->fillItemsNearXYZ( x, y, distance, excludeNotMovable );
		si->rewind();
	}
}

inline void NxwSocketWrapper2NxwCharWrapper( NxwSocketWrapper& sw, NxwCharWrapper* sc )
{
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps = sw.getClient();
		if( ps != 0 ) {
			P_CHAR pc=ps->currChar();
			if(ISVALIDPC(pc))
				sc->insert( pc->getSerial32() );
		}
	}
}

void addAllOnlinePlayers( SERIAL iSet )
{
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwSocketWrapper sw;
		sw.fillOnline();

		NxwSocketWrapper2NxwCharWrapper( sw, static_cast<NxwCharWrapper*>(iter->second) );
		iter->second->rewind();
	}
}

void addOnlinePlayersNearChar( SERIAL iSet, P_CHAR pc, bool excludeThis, int distance )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwSocketWrapper sw;
		sw.fillOnline( pc, excludeThis, distance );

		NxwSocketWrapper2NxwCharWrapper( sw, static_cast<NxwCharWrapper*>(iter->second) );
		iter->second->rewind();
	}
}

void addOnlinePlayersNearItem( SERIAL iSet, P_ITEM pi, int distance )
{

	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwSocketWrapper sw;
		sw.fillOnline( pi, distance );

		NxwSocketWrapper2NxwCharWrapper( sw, static_cast<NxwCharWrapper*>(iter->second) );
		iter->second->rewind();
	}
}

void addOnlinePlayersNearXY( SERIAL iSet, UI16 x, UI16 y, int distance )
{
 
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwSocketWrapper sw;
		Location loc; loc.x=x; loc.y=y;
		sw.fillOnline( loc, distance );

		NxwSocketWrapper2NxwCharWrapper( sw, static_cast<NxwCharWrapper*>(iter->second) );
		iter->second->rewind();
	}
}

void addGuildMembers( SERIAL iSet, SERIAL guild )
{
 
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwCharWrapper* sc = static_cast<NxwCharWrapper*>(iter->second);
		sc->fillGuildMembers( guild );
		sc->rewind();
	}
}

void addGuildRecruits( SERIAL iSet, SERIAL guild )
{
 
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwCharWrapper* sc = static_cast<NxwCharWrapper*>(iter->second);
		sc->fillGuildRecruits( guild );
		sc->rewind();
	}
}

void addGuilds( SERIAL iSet, SERIAL guild )
{
 
	AMX_WRAPPER_DB::iterator iter( g_oSet.find( iSet ) );
	if( iter != g_oSet.end() )
	{
		NxwItemWrapper* si = static_cast<NxwItemWrapper*>( iter->second );
		si->fillGuilds( guild );
		si->rewind();
	}
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
	return ( ( this->vect.size()==0 ) || ( current==vect.end() ) );	
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
	current=vect.begin();
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
	SERIAL_VECTOR::const_iterator i = std::find( vect.begin(), vect.end(), s);
	if( i==vect.end() ) //unique
		vect.push_back( s );
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
	if( cont != pointers::pContMap.end() && !cont->second.empty() )
	{
		std::vector<P_ITEM>::iterator iter( cont->second.begin() ), end( cont->second.end() );
		P_ITEM pi;
		for( ; iter != end; ++iter )
		{

			pi=(*iter);

			insertSerial(pi);
			if (pi->type==ITYPE_CONTAINER)		// a subcontainer ?
			{
				if (bIncludeSubContained ) {
					fillSerialInContainer( pi, bIncludeOnlyFirstSubcont ? false : bIncludeSubContained);
				} //Endymion bugfix, becuase not added to set before :[
			}
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
	if( obj != 0 )
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

		std::vector< P_CHAR >::iterator iter( vect->second.begin() ), end( vect->second.end() );
		P_CHAR poOwnedChr;
		for( ; iter != end; ++iter ) {
			poOwnedChr = (*iter);

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
\brief Fills with a list of chars at given location
\author Luxor
\param location the location
*/
void NxwCharWrapper::fillCharsAtXY( UI16 x, UI16 y, bool bExcludeOfflinePlayers, bool bOnlyPlayer )
{

	if( mapRegions->isValidCoord( x, y ) )
	{
		UI16 	nowx = x / REGION_GRIDSIZE,
			nowy = y / REGION_COLSIZE;

		if( !mapRegions->regions[nowx][nowy].charsInRegions.empty() )
		{
			SERIAL_SET::iterator	it( mapRegions->regions[nowx][nowy].charsInRegions.begin() ),
						end( mapRegions->regions[nowx][nowy].charsInRegions.end() );
			for( ; it != end; ++it ) {
				P_CHAR pc = pointers::findCharBySerial( *it );
				if ( pc == 0 )
					continue;
				if ( pc->getPosition().x != x || pc->getPosition().y != y )
					continue;
				if ( bExcludeOfflinePlayers && !pc->npc && !pc->IsOnline() )
					continue;
				if ( bOnlyPlayer && pc->npc )
					continue;
				if ( pc->mounted )
					continue;
				insertSerial( pc->getSerial32() );
			}
		}
	}
}

/*!
\brief Fills with a list of chars at given location
\author Luxor
\param location the location
*/
void NxwCharWrapper::fillCharsAtXY( Location location, bool bExcludeOfflinePlayers, bool bOnlyPlayer )
{
	fillCharsAtXY( location.x, location.y, bExcludeOfflinePlayers, bOnlyPlayer );
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
	Location pos = Loc( x, y, 0 );
	if( mapRegions->isValidCoord( x, y ) )
		for( SI32 ix=x-REGION_GRIDSIZE; ix<=x+REGION_GRIDSIZE; ix+=REGION_GRIDSIZE ) {
			if( ix>=0 ) {
				for( SI32 iy=y-REGION_COLSIZE; iy<=y+REGION_COLSIZE; iy+=REGION_COLSIZE ) {
					if( iy>=0 && mapRegions->isValidCoord( x, y ) ) {
						UI16 nowx = ix/REGION_GRIDSIZE, nowy= iy/REGION_COLSIZE;

						if( mapRegions->regions[nowx][nowy].charsInRegions.empty() )
							continue;

						SERIAL_SET::iterator	iter( mapRegions->regions[nowx][nowy].charsInRegions.begin() ),
									end( mapRegions->regions[nowx][nowy].charsInRegions.end() );
						for( ; iter != end; ++iter ) {
							P_CHAR pc=pointers::findCharBySerial( *iter );
							if( pc == 0 )
								continue;
							if( pc->isStabled() || pc->mounted )
								continue;
							UI32 uiDist = UI32( dist( pos, pc->getPosition(), false ) );
							if ( uiDist > nDistance )
								continue;
							if ( bOnlyPlayer && pc->npc )
								continue;
							if ( bExcludeOfflinePlayers && !pc->npc && !pc->IsOnline() )
								continue;
							insertSerial( pc->getSerial32() );
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
\brief Fills a set with a list of npcs near location
\author Endymion
\param x the x location
\param y the y location
\param nDistance the distance requested
\warning this function ADD new char to current list
*/
void NxwCharWrapper::fillNpcsNearXY( UI16 x, UI16 y, int nDistance )
{
	if( mapRegions->isValidCoord( x, y ) )
		for( SI32 ix=x-REGION_GRIDSIZE; ix<=x+REGION_GRIDSIZE; ix+=REGION_GRIDSIZE ) {
			if( ix>=0 ) {
				for( SI32 iy=y-REGION_COLSIZE; iy<=y+REGION_COLSIZE; iy+=REGION_COLSIZE ) {
					if( iy>=0 && mapRegions->isValidCoord( x, y ) ) {
						UI16	nowx = ix/REGION_GRIDSIZE,
							nowy= iy/REGION_COLSIZE;

						if( mapRegions->regions[nowx][nowy].charsInRegions.empty() )
							continue;

						SERIAL_SET::iterator iter( mapRegions->regions[nowx][nowy].charsInRegions.begin() ),
									end( mapRegions->regions[nowx][nowy].charsInRegions.end() );
						for( ; iter != end; ++iter ) {
							P_CHAR pc=pointers::findCharBySerial( *iter );
							if( pc == 0 )
								continue;
							if( !pc->npc )
								continue;
							if(  !pc->isStabled() && !pc->mounted ) {
								int iDist=(int)dist(x,y,0,pc->getPosition().x,pc->getPosition().y,0);
								if (iDist <= nDistance)
									this->insertSerial(pc->getSerial32());
							}
						}
					}
				}
			}
		}
}

/*!
\brief Fills a set with a list of npcs near char
\author Endymion
\param pc the char
\param nDistance the distance requested
\warning this function ADD new char to current list
*/
void NxwCharWrapper::fillNpcsNear( P_CHAR pc, int nDistance )
{
	VALIDATEPC(pc)
	fillNpcsNearXY(pc->getPosition().x, pc->getPosition().y, nDistance );
}

/*!
\brief Fills a set with a list of npcs near item
\author Endymion
\param pi the item
\param nDistance the distance requested
\warning this function ADD new char to current list
\note location is true, of the in world container
*/
void NxwCharWrapper::fillNpcsNear( P_ITEM pi, int nDistance )
{
	VALIDATEPI(pi)

	P_ITEM out=pi->getOutMostCont();
	if( out->isInWorld() ) {
		fillNpcsNearXY( out->getPosition().x, out->getPosition().y, nDistance );
	}
	else {
		fillNpcsNear( pointers::findCharBySerial( out->getContSerial() ), nDistance );
	}

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
	if( pc->party != INVALID )
	{
		NxwCharWrapper sc;
		sc.fillCharsNearXYZ( pc->getPosition(), nDistance, true, true );
		P_CHAR pj;
		for( sc.rewind(); !sc.isEmpty(); sc++ ) {
			pj=sc.getChar();
			if( ISVALIDPC(pj) )
				if( pc->party==pj->party ) {
					if( pc->distFrom( pj ) <= nDistance ) {
						if( !bExcludeThis || ( pc->getSerial32()!=pj->getSerial32() ) )
							this->insert( pj->getSerial32() );
				}
			}
		}
	}
}


/*!
\brief Fills a set with a list of member in given guild
\author Endymion
\param guild the guild
\warning this function ADD new char to current list
*/
void NxwCharWrapper::fillGuildMembers( SERIAL guild )
{
	P_GUILD pGuild = Guildz.getGuild( guild );
	if( pGuild != 0 )
	{
		std::map< SERIAL, P_GUILD_MEMBER >::iterator iter( pGuild->members.begin() ), end( pGuild->members.end() );
		for( ; iter!=end; iter++ ) {
			insertSerial( iter->first );
		}
	}
}

/*!
\brief Fills a set with a list of recruit in given guild
\author Endymion
\param guild the guild
\warning this function ADD new char to current list
*/
void NxwCharWrapper::fillGuildRecruits( SERIAL guild )
{
	P_GUILD pGuild = Guildz.getGuild( guild );
	if( pGuild != NULL )
	{
		std::map< SERIAL, P_GUILD_RECRUIT >::iterator iter( pGuild->recruits.begin() ), end( pGuild->recruits.end() );
		for( ; iter!=end; iter++ ) {
			insertSerial( iter->first );
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
void NxwItemWrapper::fillItemsAtXY( UI16 x, UI16 y, SI32 type, SI32 id )
{
	
	if(!mapRegions->isValidCoord( x, y ))
		return;

	UI16 nowx = x / REGION_GRIDSIZE, nowy = y / REGION_COLSIZE;

	if( mapRegions->regions[nowx][nowy].itemsInRegions.empty() )
		return;

	SERIAL_SET::iterator	iter( mapRegions->regions[nowx][nowy].itemsInRegions.begin() ),
				end( mapRegions->regions[nowx][nowy].itemsInRegions.end() );\

	for( ; iter != end; ++iter ) {
		// <Luxor bug fix>
		P_ITEM pi=pointers::findItemBySerial( *iter );
		if ( pi == 0 )
			continue;
		if ( !pi->isInWorld() )
			continue;
		if ( pi->getPosition().x != x || pi->getPosition().y != y )
			continue;
		// </Luxor>
		if ( type == INVALID || pi->type==(UI32)type )
			if ( id == INVALID || pi->getId() == id )
				insertItem(pi);
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
void NxwItemWrapper::fillItemsAtXY( Location location, SI32 type, SI32 id )
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
	if( mapRegions->isValidCoord( x, y ) )
		for( SI32 ix=x-REGION_GRIDSIZE; ix<=x+REGION_GRIDSIZE; ix+=REGION_GRIDSIZE ) {
			if( ix>=0 ) {
				for( SI32 iy=y-REGION_COLSIZE; iy<=y+REGION_COLSIZE; iy+=REGION_COLSIZE ) {
					if( iy>=0 && mapRegions->isValidCoord( x, y ) ) {
						UI16	nowx = ix/REGION_GRIDSIZE,
							nowy= iy/REGION_COLSIZE;

						if( mapRegions->regions[nowx][nowy].itemsInRegions.empty() )
							continue;

						SERIAL_SET::iterator	iter( mapRegions->regions[nowx][nowy].itemsInRegions.begin() ),
									end( mapRegions->regions[nowx][nowy].itemsInRegions.end() );
						for( ; iter != end; ++iter ) {
							P_ITEM pi=pointers::findItemBySerial( *iter );
							if( pi != 0 )
								if( pi->isInWorld() ) {
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
\brief Fills a set with a list of item worn by given char
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

	std::vector<P_ITEM>::iterator	iter( cont->second.begin() ),
					end( cont->second.end() );
	for( ; iter != end; ++iter )
	{
	
		P_ITEM pi_j=(*iter);
		if(!ISVALIDPI(pi_j) )			// just to be sure ;-)
			continue;
		
		switch( pi_j->layer ) {
			case LAYER_BACKPACK:
			case LAYER_MOUNT:
			case LAYER_TRADE_RESTOCK:
			case LAYER_TRADE_NORESTOCK:
			case LAYER_TRADE_BOUGHT:
			case LAYER_BANKBOX:
				if( bExcludeIllegalLayer )
					continue;
			case LAYER_BEARD:
			case LAYER_HAIR:
				if( !bIncludeLikeHair )
					continue;
		}

		//race protected
		if( Race::isRaceSystemActive() )
			if (!bIncludeProtectedLayer && ( Race::isProtectedLayer( (UI32) pc->race, pi_j->layer ) ) ) 
				continue;

		this->insertSerial( pi_j->getSerial32() );
	}

}

/*!
\brief Fills a set with a list of all guilds or guild political
\author Endymion
\param guild INVALID for all or specific for use guild options
\param options 
\warning this function ADD new char to current list
*/
void NxwItemWrapper::fillGuilds( SERIAL guild )
{
	if( guild == INVALID ) { //all guilds
		std::map< SERIAL, P_GUILD >::iterator iter( Guildz.guilds.begin() ), end( Guildz.guilds.end() );
		for( ; iter!=end; ++iter ) {
			insertSerial( iter->first );
		}
	}/*
	else {

		P_GUILD pGuild = Guildz.getGuild( guild );
		if( pGuild != 0 )
			if( options == GUILD_WAR ) {
				std::vector<SERIAL>::iterator iter( pGuild->war.begin() ), end( pGuild->war.end() );
				for( ; iter!=end; ++iter ) {
					insertSerial( *iter );
				}
			}
			else if( options == GUILD_ALLIED ) {
				std::vector<SERIAL>::iterator iter( pGuild->allied.begin() ), end( pGuild->allied.end() );
				for( ; iter!=end; ++iter ) {
					insertSerial( *iter );
				}
			}
	}*/

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
	if( ps != 0 )
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
	P_CHAR	pc;
	LOGICAL	validOnlyNearThis = ISVALIDPC(onlyNearThis);

	for (SI32 i = 0; i < now; ++i )
	{
		pc = pointers::findCharBySerial( currchar[i] );
		//
		//	Sparhawk ISVALIDPC(pc) checking unnecessary here, is done by findCharBySerial
		//
		if ( pc != 0 )
			if ( !validOnlyNearThis )
				insertSocket(i);
			else
				if ( onlyNearThis->distFrom(pc) <= nDistance )
					if( !(bExcludeThis && ( pc->getSerial32()==onlyNearThis->getSerial32() ) ) )
						insertSocket(i);
	}
}

/*!
\brief Fills with a list of socket
\author Endymion, rewritten by Luxor
\param location only socket near given location are added
\param nDistance the distance requested
\warning this function ADD new char to current list
*/
void NxwSocketWrapper::fillOnline( Location location, int nDistance )
{
	P_CHAR pc;
	for (SI32 i = 0; i < now; ++i )
	{
		pc = pointers::findCharBySerial( currchar[i] );
		//
		//	Sparhawk ISVALIDPC(pc) checking unnecessary here, is done by findCharBySerial
		//
		if ( pc != 0 )
			if (dist(location, pc->getPosition()) <= nDistance)
				insertSocket(i);
	}
}

/*!
\brief Fills with a list of socket
\author Endymion
\param onlyNearThis only socket near given item are added
\param nDistance the distance requested
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

	for (SI32 i = 0; i < now; ++i ) {
		if (currchar[i] != INVALID ) 
			insertSocket(i);
	}

}
