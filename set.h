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
\brief Set Related Stuff
*/

#ifndef __SET_H__
#define __SET_H__

#include "nxwcommn.h"
#include "guild.h"

class NxwWrapper {

private:
	SERIAL_VECTOR vect;
	SERIAL_VECTOR::iterator current;

protected:
	void next();

public:
	NxwWrapper();
	~NxwWrapper();

	bool isEmpty();
	void clear();
	UI32 size();
	
	void rewind();
	NxwWrapper& operator++(int);


	SERIAL get();
	void insert( SERIAL s );
	
	void copyQ( const NxwWrapper& from );
};

class NxwSerialWrapper : public NxwWrapper {

protected: 
	void fillSerialInContainer( SERIAL serial, bool bIncludeSubContained = true, bool bIncludeOnlyFirstSubcont=false );
	void fillSerialInContainer( cObject* obj, bool bIncludeSubContained = true, bool bIncludeOnlyFirstSubcont=false );

public:
	NxwSerialWrapper();
	~NxwSerialWrapper();

	SERIAL getSerial();
	void insertSerial( SERIAL s );
	void insertSerial( cObject* obj );



};

class NxwCharWrapper : public NxwSerialWrapper {

public:
	NxwCharWrapper();
	NxwCharWrapper( const NxwCharWrapper& that );
	~NxwCharWrapper();

	P_CHAR getChar();
	void insertChar( P_CHAR pc );

	void fillOwnedNpcs( P_CHAR pc, bool bIncludeStabled = true, bool bOnlyFollowing = false );
	void fillCharsAtXY( Location location, bool bExcludeOfflinePlayers = true, bool bOnlyPlayer = false );
	void fillCharsAtXY( UI16 x, UI16 y, bool bExcludeOfflinePlayers = true, bool bOnlyPlayer = false );
	void fillCharsNearXYZ ( UI16 x, UI16 y, int nDistance = VISRANGE, bool bExcludeOfflinePlayers = true, bool bOnlyPlayer = false );
	void fillCharsNearXYZ ( Location location, int nDistance = VISRANGE, bool bExcludeOfflinePlayers = true, bool bOnlyPlayer = false );
	void fillPartyFriend( P_CHAR pc, UI32 nDistance = VISRANGE, bool bExcludeThis = false );
	void fillNpcsNearXY( UI16 x, UI16 y, int nDistance = VISRANGE );
	void fillNpcsNear( P_CHAR pc, int nDistance = VISRANGE );
	void fillNpcsNear( P_ITEM pi, int nDistance = VISRANGE );
	void fillGuildMembers( SERIAL guild );
	void fillGuildRecruits( SERIAL guild );

};

class NxwItemWrapper : public NxwSerialWrapper {

public:
	NxwItemWrapper();
	NxwItemWrapper( const NxwItemWrapper& that );
	~NxwItemWrapper();

	P_ITEM getItem();
	void insertItem( P_ITEM pi );

	void fillItemsInContainer ( P_ITEM pi, bool bIncludeSubContained = true, bool bIncludeOnlyFirstSubcont=false);
	void fillItemWeared( P_CHAR pc, bool bIncludeLikeHair = false, bool bIncludeProtectedLayer = false, bool bExcludeIllegalLayer=true );
	void fillItemsAtXY( UI16 x, UI16 y, SI32 type = INVALID, SI32 id = INVALID );
	void fillItemsAtXY( Location location, SI32 type = INVALID, SI32 id = INVALID );
	void fillItemsNearXYZ ( UI16 x, UI16 y, int nDistance = VISRANGE, bool bExcludeNotMovableItems = true);
	void fillItemsNearXYZ ( Location location, int nDistance = VISRANGE, bool bExcludeNotMovableItems = true);
	void fillGuilds( SERIAL guild );

};

class NxwSocketWrapper : public NxwWrapper {

public:
	NxwSocketWrapper();
	~NxwSocketWrapper();

	SERIAL getSocket();
	NXWCLIENT getClient();
	void insertSocket( NXWSOCKET s );
	void insertClient( NXWCLIENT ps );

	void fillOnline( P_CHAR onlyNearThis, bool bExcludeThis = true, UI32 nDistance = VISRANGE );
	void fillOnline( Location location, int nDistance = VISRANGE );
	void fillOnline( P_ITEM onlyNearThis,int nDistance = VISRANGE );
	void fillOnline( );

};


const int RS_BACKPACK_ITEM = 1;
const int RS_BANK_ITEM = 2;
const int RS_EQUIP_ITEM = 3;


namespace amxSet {

	typedef std::map< SERIAL, NxwWrapper* > AMX_WRAPPER_DB;

	
	SERIAL create(  );
	void deleteSet( SERIAL iSet );
	bool end( SERIAL iSet );
	void rewind( SERIAL iSet );
	void next( SERIAL iSet );
	UI32 size( SERIAL iSet);

	SERIAL get( SERIAL iSet );
	void add( SERIAL iSet, SERIAL nVal );
	void copy( SERIAL iSet, const NxwWrapper& from );

	void addOwnedNpcs( SERIAL iSet, P_CHAR pc, bool includeStabled, bool onlyFollowing );
 	void addPartyFriend( SERIAL iSet, P_CHAR pc, int distance, bool excludeThis );
 	void addNpcsNearXY( SERIAL iSet, UI16 x, UI16 y, int distance );
	void addNpcsNearObj( SERIAL iSet, P_CHAR pc, int nDistance );
	void addNpcsNearObj( SERIAL iSet, P_ITEM pi, int nDistance );
 	void addItemsInContainer( SERIAL iSet, P_ITEM pi, bool includeSubCont, bool includeOnlyFirstSubCont );
 	void addItemWeared( SERIAL iSet, P_CHAR pc, bool includeLikeHair, bool includeProtectedLayer, bool excludeIllegalLayer );
 	void addItemsAtXY( SERIAL iSet, UI16 x, UI16 y, UI32 type );
 	void addItemsNearXY( SERIAL iSet, UI16 x, UI16 y, int distance, bool excludeNotMovable );
 	void addAllOnlinePlayers( SERIAL iSet );
	void addOnlinePlayersNearChar( SERIAL iSet, P_CHAR pc, bool excludeThis, int distance );
	void addOnlinePlayersNearItem( SERIAL iSet, P_ITEM pi, int distance );
	void addOnlinePlayersNearXY( SERIAL iSet, UI16 x, UI16 y, int distance );

	void addGuildMembers( SERIAL iSet, SERIAL guild );
	void addGuildRecruits( SERIAL iSet, SERIAL guild );
	void addGuilds( SERIAL iSet, SERIAL guild );

}

#endif //__SET_H__
