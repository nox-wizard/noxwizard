  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "network.h"

OBJECT_MAP cAllObjects::all;
SERIAL cAllObjects::current_item_serial=0x40000000;
SERIAL cAllObjects::current_char_serial=0;


cAllObjects objects; //All objects

void cAllObjects::clear()
{
	OBJECT_MAP::iterator iter( all.begin() );
	for( ; iter!=all.end(); iter++ ) {
		if( iter->second!=NULL )
			safedelete(iter->second);
	}
}

P_OBJECT cAllObjects::findObject(SERIAL nSerial)
{
	if (nSerial < 0) return NULL;
    OBJECT_MAP::iterator iter( all.find(nSerial) );
    if (iter == all.end())
		return NULL;

	return iter->second;
}

void cAllObjects::insertObject( P_OBJECT obj )
{
	all.insert( make_pair( obj->getSerial32(), obj ) );
}

void cAllObjects::eraseObject( P_OBJECT obj )
{
	OBJECT_MAP::iterator iter( all.find( obj->getSerial32() ) );
	if( iter!=all.end() )
		all.erase( iter );
}

SERIAL cAllObjects::getNextCharSerial()
{
	return ++current_char_serial;
}

SERIAL cAllObjects::getNextItemSerial()
{
	return ++current_item_serial;
}

void cAllObjects::updateCharSerial( SERIAL ser )
{
	if( ser > current_char_serial )
		current_char_serial=ser;
}

void cAllObjects::updateItemSerial( SERIAL ser )
{
	if( ser > current_item_serial )
		current_item_serial=ser;
}



namespace archive {

P_ITEM getNewItem( ) 
{
	return new cItem( objects.getNextItemSerial() ); //oh yes
}

P_ITEM getItemForCopy()
{
	return new cItem( INVALID ); //oh yes
}

void DeleItem( P_ITEM pi )
{

	VALIDATEPI( pi );

	amxVS.deleteVariable( pi->getSerial32() );

	UI32 pi_serial = pi->getSerial32();

	if (pi->spawnregion!=INVALID )
	{
		spawns::removeObject( pi->spawnregion, pi );
	}

	if( pi->isSpawner() || pi->spawnserial!=INVALID )
	{
		spawns::removeSpawnDynamic( pi );
	}

	NxwSocketWrapper sw;
	sw.fillOnline( pi );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET j=sw.getSocket();			
		if (j!=INVALID)
			SendDeleteObjectPkt(j, pi_serial);
	}

	// - remove from pointer arrays
	pointers::delItem(pi);	//Luxor

	pi->setOwnerSerial32(-1);
	// - remove from mapRegions if a world item
	if (pi->isInWorld())
	{
	   	regions::remove(pi); // da==1 not added !!
	}

	if (pi->type==ITYPE_BOOK && (pi->morex==666 || pi->morey==999) && pi->morez)
		Books::books.erase( Books::books.find(pi->morez) );
        // if a new book gets deleted also delete the corresponding map element

	safedelete(pi);

}

void DeleItem( SERIAL i )
{
	DeleItem( MAKE_ITEM_REF(i) );
}


P_CHAR getNewChar()
{
	return new cChar( objects.getNextCharSerial() ); //oh yes
}

P_CHAR getCharForCopy()
{
	return new cChar( INVALID ); //oh yes
}

void DeleteChar( P_CHAR pc )
{
	VALIDATEPC( pc );


	pc->race = 0;

	amxVS.deleteVariable( pc->getSerial32() );

	UI32 pc_serial = pc->getSerial32();

	if( pc->spawnregion!=INVALID )
	{
		spawns::removeObject( pc->spawnregion, pc );
	}

	if( pc->spawnserial!=INVALID ) 
	{
		spawns::removeSpawnDynamic( pc );
	}

	pointers::delChar(pc);	//Luxor

	NxwSocketWrapper sw;
	sw.fillOnline( pc );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
			SendDeleteObjectPkt(j, pc_serial);
	}

	safedelete(pc);

}

void DeleteChar( SERIAL k )
{
	DeleteChar( MAKE_CHAR_REF( k ) );
}


}

void cAllObjectsIter::rewind()
{
	curr=objects.all.begin();
	next=curr;
	if ( next != objects.all.end() )
		next++;
}

bool cAllObjectsIter::IsEmpty()
{
	return ( curr==objects.all.end() );
}

P_OBJECT cAllObjectsIter::getObject()
{
	return curr->second;
}

SERIAL cAllObjectsIter::getSerial()
{
	return curr->first;
}


cAllObjectsIter& cAllObjectsIter::operator++(int)
{
	curr=next;
	next++;
	return (*this);
}




