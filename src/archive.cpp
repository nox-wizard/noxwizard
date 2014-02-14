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

/*!
\brief deletes all objects
*/
void cAllObjects::clear()
{
	OBJECT_MAP::iterator iter(all.begin() );
	
	for( ; iter!=all.end(); iter++ ) 
		if( iter->second!=NULL )
			safedelete(iter->second);
}

/*!
\brief get a pointer to the object from the object's serial
\param nSerial the object's serial
\return P_OBJECT pointer, or NULL if invalid serial
*/
P_OBJECT cAllObjects::findObject(SERIAL nSerial)
{
	if (nSerial < 0) 
		return NULL;
		
    OBJECT_MAP::iterator iter(all.find(nSerial) );
    if (iter == all.end())
		return NULL;

	return iter->second;
}

/*!
\brief inserts an object in the list
\param obj the object
*/
void cAllObjects::insertObject( P_OBJECT obj )
{
	all.insert( make_pair( obj->getSerial32(), obj ) );
}

/*!
\brief deletes an object
\param obj the object
*/
void cAllObjects::eraseObject( P_OBJECT obj )
{
	OBJECT_MAP::iterator iter( all.find( obj->getSerial32() ) );
	if( iter!=all.end() )
		all.erase( iter );
}

/*!
\brief gets a new character serial
\return the serial
*/
SERIAL cAllObjects::getNextCharSerial()
{
	return ++current_char_serial;
}

/*!
\brief gets a new item serial
\return the serial
*/
SERIAL cAllObjects::getNextItemSerial()
{
	return ++current_item_serial;
}

/*!
\brief updates current_char_serial

\param ser the serial
\todo why this function? char serials should already be < current_char_serial
*/
void cAllObjects::updateCharSerial( SERIAL ser )
{
	if( ser > current_char_serial )
		current_char_serial = ser;
}

/*!
\brief updates current_item_serial

\param ser the serial
\todo why this function? item serials should already be < current_item_serial
*/
void cAllObjects::updateItemSerial( SERIAL ser )
{
	if( ser > current_item_serial )
		current_item_serial=ser;
}


/*!
\todo is this namespace heirarchy really necessary? why don't put all in cAllObjects?
*/
namespace archive 
{

	namespace item
	{
		/*!
		\brief creates a new item
		\param forCopyPurpose if true current_item_serial is not increased
		\return pointer to the item
		*/
		P_ITEM New( LOGICAL forCopyPurpose )
		{
			return new cItem( forCopyPurpose ? INVALID : objects.getNextItemSerial() );
		}
	}

/*!
\brief deletes an item

\param pi pointer to the item to delete
\todo check comments inside function body
*/
void deleteItem( P_ITEM pi )
{
	VALIDATEPI( pi );

	SI32 serial = pi->getSerial32();

	amxVS.deleteVariable( serial );
	
	//remove from spawn region if item was spawned
	if (pi->getSpawnRegion() != INVALID )
		Spawns->removeObject( pi->getSpawnRegion(), pi );
	
	//remove spawn if it was a spawner
	if( pi->isSpawner() || pi->getSpawnSerial()!=INVALID )
		Spawns->removeSpawnDinamic( pi );
	
	//update clients
	//possible network bug: shouldn't the packet be sent only to characters in visrange? check protocol	
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
	
	//why this?
	pi->setOwnerSerial32(INVALID);

	if (pi->type==ITYPE_BOOK && (pi->morex==666 || pi->morey==999) && pi->morez)
		// make sure that the book is really there
		if ( Books::books.find(pi->morez) != Books::books.end() )
			Books::books.erase( Books::books.find(pi->morez) );
        // if a new book gets deleted also delete the corresponding map element

	safedelete(pi);
}

/*!
\brief deletes an item

Overloaded version of deleteItem(P_ITEM)
\param i item serial
*/
void deleteItem( SERIAL i )
{
	deleteItem( MAKE_ITEM_REF(i) );
}

namespace character
{
	/*!
	\brief creates a new character
	\todo why not make this like archive::item::New() and remove Instance() ? 
	*/
	P_CHAR New()
	{
		return new cChar( objects.getNextCharSerial() ); //oh yes
	}
	
	/*!
	\brief creates a new character without increasing char serial
	*/
	P_CHAR Instance()
	{
		return new cChar( INVALID ); //oh yes
	}
	
	/*!
	\brief deletes a character
	\param pc the character to be deleted
	*/
	void Delete( P_CHAR pc )
	{
		VALIDATEPC( pc );

		pc->setRace(0);


		amxVS.deleteVariable( pc->getSerial32() );

		UI32 pc_serial = pc->getSerial32();
		
		//remove from spawn region
		if( pc->getSpawnRegion()!=INVALID )
			Spawns->removeObject( pc->getSpawnRegion(), pc );
		
		if( pc->getSpawnSerial()!=INVALID )
			Spawns->removeSpawnDinamic( pc );

		pointers::delChar(pc);	//Luxor
		
		//update clients
		//same problem as with archive::item::Delete()
		NxwSocketWrapper sw;
		sw.fillOnline( pc );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWSOCKET j=sw.getSocket();
			if( j!=INVALID )
				SendDeleteObjectPkt(j, pc_serial);
		}
		
		//remove items weared
		NxwItemWrapper si;
		P_ITEM pi;
		si.fillItemWeared( pc, true, true, false );
		for( si.rewind(); !si.isEmpty(); si++ ) 
		{
			pi = si.getItem();
			if ( !ISVALIDPI(pi) )
				continue;
			pi->Delete();
		}

		safedelete(pc);
	}
	
	/*!
	\brief removes a character
	
	Overloaded version of archive::character::Delete()
	\param k char serial serial
	*/
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
	curr = next;
	if ( next != objects.all.end() )
		next++;
	return (*this);
}




