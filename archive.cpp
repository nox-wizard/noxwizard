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

	UI08 removeitem[5]={ 0x1D, 0x00, };
	LongToCharPtr(pi->getSerial32(), removeitem +1);

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
		{
			Xsend(j, removeitem, 5);
///			Network->FlushBuffer(j);
		}
	}

	// - remove from pointer arrays
	pointers::delItem(pi);	//Luxor

	pi->setOwnerSerial32(-1);
	// - remove from mapRegions if a world item
	if (pi->isInWorld())
	{
	   	mapRegions->remove(pi); // da==1 not added !!
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

	UI08 removeitem[5]={ 0x1D, 0x00, };
	LongToCharPtr(pc->getSerial32(), removeitem +1);

	if( pc->spawnregion!=INVALID )
	{
		Spawns->removeObject( pc->spawnregion, pc );
	}

	if( pc->spawnserial!=INVALID ) 
	{
		Spawns->removeSpawnDinamic( pc );
	}

	pointers::delChar(pc);	//Luxor

	NxwSocketWrapper sw;
	sw.fillOnline( pc );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
		{
			Xsend(j, removeitem, 5);
///			Network->FlushBuffer(j);
		}
	}

	safedelete(pc);

}

void DeleteChar( SERIAL k )
{
	DeleteChar( MAKE_CHAR_REF( k ) );
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
	this->next++;
	return (*this);
}




