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
#include "books.h"
#include "spawn.h"
#include "set.h"
#include "archive.h"
#include "sndpkg.h"
#include "inlines.h"

OBJECT_MAP cAllObjects::all;
SERIAL cAllObjects::current_item_serial=0x40000000;
SERIAL cAllObjects::current_char_serial=1; // ndEndy not 0 because target return 0 if invalid target, so OSI think 0 like INVALID


cAllObjects objects; //All objects

cAllObjects::cAllObjects()
{

}

cAllObjects::~cAllObjects()
{
}

void cAllObjects::clear()
{
	OBJECT_MAP::iterator iter( this->all.begin() );
	for( ; iter!=all.end(); iter++ ) {
		if( iter->second!=NULL )
			safedelete(iter->second);
	}
}

P_OBJECT cAllObjects::findObject(SERIAL nSerial)
{
	if (nSerial < 0) return NULL;
    OBJECT_MAP::iterator iter( this->all.find(nSerial) );
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
	return ++this->current_char_serial;
}

SERIAL cAllObjects::getNextItemSerial()
{
	return ++this->current_item_serial;
}

void cAllObjects::updateCharSerial( SERIAL ser )
{
	if( ser > this->current_char_serial )
		this->current_char_serial=ser;
}

void cAllObjects::updateItemSerial( SERIAL ser )
{
	if( ser > this->current_item_serial )
		this->current_item_serial=ser;
}



namespace archive {

	namespace item
	{
		P_ITEM New( LOGICAL forCopyPurpose )
		{
			return new cItem( forCopyPurpose ? INVALID : objects.getNextItemSerial() ); //oh yes
		}
	}

void deleteItem( P_ITEM pi )
{
	VALIDATEPI( pi );

	SI32 serial = pi->getSerial32();

	amxVS.deleteVariable( serial );

	if (pi->spawnregion!=INVALID )
	{
		Spawns->removeObject( pi->spawnregion, pi );
	}

	if( pi->isSpawner() || pi->spawnserial!=INVALID )
	{
		Spawns->removeSpawnDinamic( pi );
	}

	NxwSocketWrapper sw;
	sw.fillOnline( pi );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET j=sw.getSocket();			
		if (j!=INVALID)
			SendDeleteObjectPkt( j, serial );
	}

	// - remove from pointer arrays
	pointers::delItem(pi);	//Luxor

	pi->setOwnerSerial32(INVALID);

	if (pi->type==ITYPE_BOOK && (pi->morex==666 || pi->morey==999) && pi->morez)
		// make sure that the book is really there
		if ( Books::books.find(pi->morez) != Books::books.end() )
			Books::books.erase( Books::books.find(pi->morez) );
        // if a new book gets deleted also delete the corresponding map element

	safedelete(pi);

}

void deleteItem( SERIAL i )
{
	deleteItem( MAKE_ITEM_REF(i) );
}

namespace character
{
	P_CHAR New()
	{
		return new cChar( objects.getNextCharSerial() ); //oh yes
	}

	P_CHAR Instance()
	{
		return new cChar( INVALID ); //oh yes
	}

	void Delete( P_CHAR pc )
	{
		VALIDATEPC( pc );

		pc->race = 0;

		amxVS.deleteVariable( pc->getSerial32() );

		UI32 pc_serial = pc->getSerial32();

		if( pc->spawnregion!=INVALID )
			Spawns->removeObject( pc->spawnregion, pc );

		if( pc->spawnserial!=INVALID )
			Spawns->removeSpawnDinamic( pc );

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

	void Delete( SERIAL k )
	{
		Delete( MAKE_CHAR_REF( k ) );
	}

}

}





cAllObjectsIter::cAllObjectsIter(  )
{
}

cAllObjectsIter::~cAllObjectsIter()
{
}


void cAllObjectsIter::rewind()
{
	this->curr=objects.all.begin();
	this->next=this->curr;
	if ( this->next != objects.all.end() )
		this->next++;
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
	this->curr=this->next;
	if ( this->next != objects.all.end() )
		this->next++;
	return (*this);
}




