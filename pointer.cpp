  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "set.h"
#include "debug.h"

#define ALLOCATING_UNIT 1024
/*!
\brief Sets an item into the array, reallocating space if needed
\note can be used to set other pointer arrays too
\param ptr pointer
\param item item number
*/
void setptr(lookuptr_st *ptr, int item) //set item in pointer array
{
	int i;
	for (i=0;i<(ptr->max);i++)
	{
		if (ptr->pointer[i]==-1)
		{
			ptr->pointer[i]=item;
			return;
		} else if (ptr->pointer[i]==item) return;
	}

	//resize(ptr->pointer, ptr->max, ptr->max+ALLOCATING_UNIT);
	// Must be out of slots, so reallocate some and set item
	if ((ptr->pointer = (int *)realloc(ptr->pointer, (ptr->max+ALLOCATING_UNIT)*sizeof(int)))==0)
	{
		PanicOut("Can't reallocate memory!\n");
		error=1;
		keeprun=false;  //shutdown
		return;
	}

	for (i=ptr->max;i<(ptr->max+ALLOCATING_UNIT);i++) ptr->pointer[i]=-1;
	ptr->pointer[ptr->max]=item;
	ptr->max+=ALLOCATING_UNIT;
	return;

}

/*!
\brief Finds a specific item/char by serial number
\param ptr pointer to vector
\param nSerial serial to find
\param nType 0 if items, 1 if chars
*/
int findbyserial(lookuptr_st *ptr, int nSerial, int nType)
{
	if (nSerial < 0) 
		return INVALID;  //prevent errors from some clients being slower than the server clicking on nolonger valid items

	if (ptr == 0 || ptr->pointer == 0) // Blackwind / Crashfix
		return INVALID;

	for (int i=0;i<(ptr->max);i++)
	{
		//cnt++; //ubncommentt if you use the debugging print
		if (nType==0) {
			P_ITEM pi=MAKE_ITEM_REF(ptr->pointer[i]);
			if( ISVALIDPI(pi) && pi->getSerial32()==nSerial )
			{
				//ConOut("Found item %d out of %d in %d hits. max: %i\n",ptr->pointer[i],itemcount,cnt,ptr->max);
				return DEREF_P_ITEM(pi);
			}
		}
		else if (nType==1) {

			P_CHAR pc=MAKE_CHAR_REF(ptr->pointer[i]);
			if( ISVALIDPC(pc) && pc->getSerial32() == nSerial )
			{
				// ConOut("Found char %d out of %d in %d hits.\n",ptr->pointer[i],charcount,cnt);
				return DEREF_P_CHAR(pc);
			}
		}

	}
	
	return INVALID;

}

/*!
\brief remove an item/char from a pointer array
\note really it just mark it as a free slot
*/
void removefromptr(lookuptr_st *ptr, int nItem)
{
	int i;
	for (i=0;i<(ptr->max);i++)
	{
		if (ptr->pointer[i]==nItem)
		{
			ptr->pointer[i]=INVALID;
		//	return;		// don't return here, let's remove ALL the occurences (Duke)
		}
	}

}

/*
void setserial(int nChild, int nParent, int nType)
{ // Sets the serial #'s and adds to pointer arrays
  // types: 1-item container, 2-item spawn, 3-Item's owner 4-container is PC/NPC
  //        5-NPC's owner, 6-NPC spawned

	if (nChild == INVALID || nParent == INVALID )
		return;
	
	switch(nType)
	{
	case 1:
		{
			P_ITEM child=MAKE_ITEM_REF(nChild);
			VALIDATEPI(child);
			P_ITEM parent=MAKE_ITEM_REF(nParent);
			VALIDATEPI(parent);
			child->setContSerial(parent->getSerial32());	//Luxor
		    //items[nChild].setContSerial(items[nParent].getSerial32());	//Luxor
			//setptr(&contsp[items[nChild].contserial%HASHMAX], nChild);
		}
		break;
	case 2:				// Set the Item's Spawner
		{
			P_ITEM child=MAKE_ITEM_REF(nChild);
			VALIDATEPI(child);
			P_ITEM parent=MAKE_ITEM_REF(nParent);
			VALIDATEPI(parent);
		    child->spawnserial=parent->getSerial32();
			//items[nChild].spawnserial=items[nParent].getSerial32();
			setptr(&spawnsp[child->spawnserial%HASHMAX], DEREF_P_ITEM(child));
			//setptr(&spawnsp[items[nChild].spawnserial%HASHMAX], nChild);
		}
		break;
	case 3:				// Set the Item's Owner
		{
			P_ITEM child=MAKE_ITEM_REF(nChild);
			VALIDATEPI(child);
			P_CHAR parent=MAKE_CHAR_REF(nParent);
			VALIDATEPC(parent);
			child->setOwnerSerial32Only(parent->getSerial32());
		    //items[nChild].setOwnerSerial32Only(chars[nParent].getSerial32());
			setptr(&ownsp[child->getOwnerSerial32()%HASHMAX], DEREF_P_ITEM(child));
			//setptr(&ownsp[items[nChild].getOwnerSerial32()%HASHMAX], nChild);
		}
		break;
	case 4:
		{
			P_ITEM child=MAKE_ITEM_REF(nChild);
			VALIDATEPI(child);
			P_CHAR parent=MAKE_CHAR_REF(nParent);
			VALIDATEPC(parent);
			child->setContSerial(parent->getSerial32());	//Luxor
			//items[nChild].setContSerial(chars[nParent].getSerial32());	//Luxor
			//setptr(&contsp[items[nChild].contserial%HASHMAX], nChild);
		}
		break;
	case 5:				// Set the Character's Owner
		{
			P_CHAR child = MAKE_CHAR_REF(nChild);
			VALIDATEPC(child);
			P_CHAR parent=MAKE_CHAR_REF(nParent);
			VALIDATEPC(parent);
			child->setOwnerSerial32Only(parent->getSerial32());
			//chars[nChild].setOwnerSerial32Only(chars[nParent].getSerial32());
			setptr(&cownsp[child->getOwnerSerial32()%HASHMAX], DEREF_P_CHAR(child));
			//setptr(&cownsp[chars[nChild].getOwnerSerial32()%HASHMAX], nChild);

			if( child->getSerial32() != parent->getSerial32() )
				child->tamed = true;
			else
				child->tamed = false;
		}
		break;
	case 6:				// Set the character's spawner
		{
			P_CHAR child = MAKE_CHAR_REF(nChild);
			VALIDATEPC(child);
			P_ITEM parent=MAKE_ITEM_REF(nParent);
			VALIDATEPI(parent);
			child->spawnserial=parent->getSerial32();
			//chars[nChild].spawnserial=items[nParent].getSerial32();
			setptr(&cspawnsp[child->spawnserial%HASHMAX], DEREF_P_CHAR(child));
			//setptr(&cspawnsp[chars[nChild].spawnserial%HASHMAX], nChild);
		}
		break;
	case 7:				// Set the Item in a multi
		{
			P_ITEM child=MAKE_ITEM_REF(nChild);
			VALIDATEPI(child);
			P_ITEM parent=MAKE_ITEM_REF(nParent);
			VALIDATEPI(parent);
			child->setMultiSerial32Only( parent->getSerial32() );
			//items[nChild].setMultiSerial32Only( items[nParent].getSerial32() );
			setptr(&imultisp[child->getMultiSerial32()%HASHMAX], DEREF_P_ITEM(child));
			//setptr(&imultisp[items[nChild].getMultiSerial32()%HASHMAX], nChild);
		}
		break;
	case 8:				//Set the CHARACTER in a multi
		{
			P_CHAR child = MAKE_CHAR_REF(nChild);
			VALIDATEPC(child);
			P_ITEM parent=MAKE_ITEM_REF(nParent);
			VALIDATEPI(parent);
			child->setMultiSerial32Only(parent->getSerial32());
			//chars[nChild].setMultiSerial32Only(items[nParent].getSerial32());
			setptr(&cmultisp[child->getMultiSerial32()%HASHMAX], DEREF_P_CHAR(child));
			//setptr(&cmultisp[chars[nChild].getMultiSerial32()%HASHMAX], nChild);
		}
		break;
	default:
		WarnOut("No handler for nType (%08x) in setserial()", nType);
		break;
	}

}
*/

/*!
\brief gets world coordinates from a serial
\author Xanathar
\param sr serial
\param px x coordinate
\param py y coordinate
\param pz z coordinate
\param ch eventual index to char
\param it eventual index to item
*/
void getWorldCoordsFromSerial (int sr, int& px, int& py, int& pz, int& ch, int& it)
{
    int serial = sr;
    int loop = 0;
    it = ch = INVALID;

	P_CHAR pc=0;
	P_ITEM pi=0;

    while ((++loop) < 500)
    {

		pc=pointers::findCharBySerial(serial);
		ch = DEREF_P_CHAR(pc);
		pi=pointers::findItemBySerial(serial);
        it = DEREF_P_ITEM(pi);
        if (ISVALIDPI(pi)) {
            if (pi->getContSerial()!=INVALID) {
                serial = pi->getContSerial();
                continue;
            }
        }
        break;
    }

    if (ISVALIDPC(pc)) {
		Location charpos= pc->getPosition();
        px = charpos.x;
        py = charpos.y;
        pz = charpos.z;
    } else if ( ISVALIDPI(pi) && (pi->getContSerial()==INVALID)) {
        px = pi->getPosition("x");
        py = pi->getPosition("y");
        pz = pi->getPosition("z");
    } else {
        px = 0;
        py = 0;
        pz = 0;
    }
}




namespace pointers {

	std::map<SERIAL, vector <P_CHAR> > pStableMap;
	std::map<SERIAL, P_CHAR > pMounted; 

	std::map<SERIAL, vector <P_CHAR> > pOwnCharMap;
	std::map<SERIAL, vector <P_ITEM> > pOwnItemMap;

	std::map<SERIAL, vector <P_ITEM> > pContMap;

	std::map<SERIAL, vector <P_CHAR> > pMultiCharMap;
	std::map<SERIAL, vector <P_ITEM> > pMultiItemMap;

#ifdef SPAR_NEW_WR_SYSTEM
	typedef std::multimap< UI32, P_CHAR >	PCHARLOCATIONMAP;
	typedef PCHARLOCATIONMAP::iterator	PCHARLOCATIONMAPIT;

	PCHARLOCATIONMAP	pCharLocationMap;

	static UI32 location2key( const Location& l );
	static UI32 location2key( const UI32 x, const UI32 y );

	static UI32 location2key( const Location& l )
	{
		return location2key( l.x, l.y );
	}

	static UI32 location2key( const UI32 x, const UI32 y )
	{
		return (x << 16) + y;
	}

	struct XY
	{
		UI32	x;
		UI32	y;
	};

	static XY upperLeft	;
	static XY lowerRight	;

	void addCharToLocationMap( const P_CHAR who )
	{
		VALIDATEPC(who);
		UI32 key  = (who->getPosition().x << 16) +  who->getPosition().y;
		//ConOut("addCharToWorldMap serial=%d name=%s key=%i\n", who->getSerial32(), who->getCurrentNameC(), key );
		pCharLocationMap.insert( pair< UI32, P_CHAR >( key, who ) );
	}

	void delCharFromLocationMap( const P_CHAR who )
	{
		VALIDATEPC(who);
		UI32 key  = (who->getPosition().x << 16) +  who->getPosition().y;

		//ConOut("delCharFromWorldMap serial=%d name=%s key=%i\n", who->getSerial32(), who->getCurrentNameC(), key );

		pair< PCHARLOCATIONMAPIT, PCHARLOCATIONMAPIT > it = pCharLocationMap.equal_range( key );

		for( ; it.first != it.second; ++it.first )
			if( it.first->second->getSerial32() == who->getSerial32() )
			{
				pCharLocationMap.erase( it.first );
				break;
			}
		/*
		if( it.first == it.second )
			WarnOut("delCharFromWorldMap: could not delete char %i at x %i y %i from location map\n", who->getSerial32(), who->getPosition().x, who->getPosition().y );
		*/
	}

	void showCharLocationMap()
	{
		PCHARLOCATIONMAPIT it( pCharLocationMap.begin() ), end( pCharLocationMap.end() );

		ConOut( "--------------------------------\n" );
		ConOut( "|      CHAR LOCATION MAP       |\n" );
		ConOut( "--------------------------------\n" );
		ConOut( "|   Key   | X  | Y  |  SERIAL  |\n" );
		ConOut( "--------------------------------\n" );

		UI32 	invalidCount	=  0;
		SI32 	x	  	=  0;
		SI32 	y		=  0;
		SERIAL	serial		= INVALID;
		for( ; it != end; ++it )
		{
			x = it->first >> 16;
			y = it->first & 0x0000FFFF;
			if( ISVALIDPC( it->second ) )
				serial = it->second->getSerial32();
			else
			{
				++invalidCount;
				serial = INVALID;
			}
			ConOut( "|%10i|%4i|%4i|%10i|\n", it->first, x, y, serial );
		}
		ConOut( "--------------------------------\n" );
		ConOut( "| entries in map : %10i  |\n", pCharLocationMap.size());
		ConOut( "| invalid entries: %10i  |\n", invalidCount );
		ConOut( "--------------------------------\n" );
	}

	pCharVector* getCharsNearLocation( cObject* pObject, SI32 range, UI32 flags )
	{
		pCharVector* 	pvCharsInRange	= 0;
		LOGICAL		validCall	= false;
		SERIAL		self		= INVALID;

		if( pObject != 0 )
		{
			if( isItemSerial( pObject->getSerial32() ) )
			{
				if( static_cast<P_ITEM>(pObject)->isInWorld() )
					validCall = true;
			}
			else
			{
				self = pObject->getSerial32();
				validCall = true;
			}
			if( validCall )
				pvCharsInRange = getCharsNearLocation( pObject->getPosition().x, pObject->getPosition().y, range, flags, self );
		}
		return pvCharsInRange;
	}

	pCharVector* getCharsNearLocation( SI32 x, SI32 y, SI32 range, UI32 flags, SERIAL serial )
	{
		pCharVector* pvCharsInRange = 0;

		if( x > 0 && x < 6145 && y > 0 && y < 4097 && range > -1 )
		{
			pvCharsInRange = new pCharVector();

			if( x - range < 1 )
				upperLeft.x = 1;
			else
				upperLeft.x = x - range;

			if( x + range > 6144 )
				lowerRight.x = 6144;
			else
				lowerRight.x = x + range;

			if( y - range < 1 )
				upperLeft.y = 1;
			else
				upperLeft.y = y - range;

			if( y + range > 4096 )
				lowerRight.y = 4096;
			else
				lowerRight.y = y + range;

			//ConOut( "getCharFromLocationMap: x=%i, y=%i range=%i upperLeft.x=%i upperLeft.y=%i lowerRight.x=%i lowerRight.y=%i\n",
			//	x, y, range, upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);

			PCHARLOCATIONMAPIT it(  pCharLocationMap.lower_bound( ( upperLeft.x << 16 ) + upperLeft.y ) ),
					   end( pCharLocationMap.upper_bound( ( lowerRight.x << 16 ) + lowerRight.y ));

			LOGICAL select = false;

			for( ; it != end; ++it )
			{
				if( ISVALIDPC( it->second ) )
				{
					if( flags )
					{
						if ( !(flags & EXCLUDESELF) ||
						      ((flags & EXCLUDESELF) && serial != it->second->getSerial32() ) )
						{
							if ( it->second->npc )
							{ //Pc is an npc
								if ( (flags & NPC) )
									select = true;
							}
							else
							{  //Pc is a player
								if ( (flags & ONLINE) && it->second->IsOnline() )
									select = true;
								if ( (flags & OFFLINE) && !it->second->IsOnline() )
									select = true;
								if ( (flags & DEAD) && it->second->dead )
									select = true;
							}
						}
					}
					if( select )
						pvCharsInRange->push_back( it->second );
				}
				else	// cleanup
				{
					pCharLocationMap.erase( it );
					WarnOut( "getCharFromLocationMap removed invalid entry\n" );
				}
				select = false;
			}
		}
		return pvCharsInRange;
	}

	typedef std::multimap< UI32, P_ITEM >	PITEMLOCATIONMAP;
	typedef PITEMLOCATIONMAP::iterator	PITEMLOCATIONMAPIT;

	PITEMLOCATIONMAP	pItemLocationMap;


	void addItemToLocationMap( const P_ITEM what )
	{
		VALIDATEPI(what);
		UI32 key  = (what->getPosition().x << 16) +  what->getPosition().y;
		//ConOut("addItemToWorldMap serial=%d name=%s key=%i\n", what->getSerial32(), what->getCurrentNameC(), key );
		pItemLocationMap.insert( pair< UI32, P_ITEM >( key, what ) );
	}

	void delItemFromLocationMap( const P_ITEM what )
	{
		VALIDATEPI(what);
		UI32 key  = (what->getPosition().x << 16) +  what->getPosition().y;
		//ConOut("delItemFromWorldMap serial=%d name=%s key=%i\n", what->getSerial32(), what->getCurrentNameC(), key );
		pair< PITEMLOCATIONMAPIT, PITEMLOCATIONMAPIT > it = pItemLocationMap.equal_range( key );

		for( ; it.first != it.second; ++it.first )
			if( it.first->second->getSerial32() == what->getSerial32() )
			{
				pItemLocationMap.erase( it.first );
				break;
			}
		/*
		if( it.first == it.second )
			WarnOut("delCharFromWorldMap: could not delete char %i at x %i y %i from location map\n", who->getSerial32(), who->getPosition().x, who->getPosition().y );
		*/
	}


	pItemVector* getItemsNearLocation( cObject* pObject, SI32 range, UI32 flags )
	{
		pItemVector* 	pvItemsInRange	= 0;
		LOGICAL		validCall	= false;
		SERIAL		self		= INVALID;

		if( pObject != 0 )
		{
			if( isItemSerial( pObject->getSerial32() ) )
			{
				if( static_cast<P_ITEM>(pObject)->isInWorld() )
				{
					self = pObject->getSerial32();
					validCall = true;
				}
			}
			else
			{
				validCall = true;
			}
			if( validCall )
				pvItemsInRange = getItemsNearLocation( pObject->getPosition().x, pObject->getPosition().y, range, flags, self );
		}
		return pvItemsInRange;
	}

	pItemVector* getItemsNearLocation( SI32 x, SI32 y, SI32 range, UI32 flags, SERIAL serial )
	{
		pItemVector* pvItemsInRange;

		if( x > 0 && x < 6145 && y > 0 && y < 4097 && range > -1 )
		{
			pvItemsInRange = new pItemVector();

			if( x - range < 1 )
				upperLeft.x = 1;
			else
				upperLeft.x = x - range;

			if( x + range > 6144 )
				lowerRight.x = 6144;
			else
				lowerRight.x = x + range;

			if( y - range < 1 )
				upperLeft.y = 1;
			else
				upperLeft.y = y - range;

			if( y + range > 4096 )
				lowerRight.y = 4096;
			else
				lowerRight.y = y + range;

			//ConOut( "getItemFromLocationMap: x=%i, y=%i range=%i upperLeft.x=%i upperLeft.y=%i lowerRight.x=%i lowerRight.y=%i\n",
			//	x, y, range, upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);

			PITEMLOCATIONMAPIT it(  pItemLocationMap.lower_bound( ( upperLeft.x << 16 ) + upperLeft.y ) ),
					   end( pItemLocationMap.upper_bound( ( lowerRight.x << 16 ) + lowerRight.y ));

			LOGICAL select = false;

			for( ; it != end; ++it )
			{
				if( ISVALIDPI( it->second ) )
				{
					if( flags )
					{
						if ( !(flags & EXCLUDESELF) ||
						      ((flags & EXCLUDESELF) && serial != it->second->getSerial32() ) )
						{
							select = true;
						}
					}
					if( select )
						pvItemsInRange->push_back( it->second );
				}
				else	// cleanup
				{
					pItemLocationMap.erase( it );
					WarnOut( "getItemFromLocationMap removed invalid entry\n" );
				}
				select = false;
			}
		}
		return pvItemsInRange;
	}
#endif
	/*!
	\brief initializes pointer maps
	\author Luxor
	*/
	void init()
	{


		pStableMap.clear();
		pContMap.clear();
		pMounted.clear();
		pOwnCharMap.clear();
		pOwnItemMap.clear();
		//Chars and Stablers
		P_CHAR pc = 0;

		cAllObjectsIter objs;

		for( objs.rewind(); !objs.IsEmpty(); objs++ )
		{
			if( isCharSerial( objs.getSerial() ) ) {
				pc=(P_CHAR)objs.getObject();
				{
					if( pc->isStabled() ) {
						pStableMap[pc->getStablemaster()].push_back(pc);
					}
					if( pc->mounted )
						pMounted[pc->getOwnerSerial32()]=pc;

					P_CHAR own=pointers::findCharBySerial(pc->getOwnerSerial32());
					pc->setOwnerSerial32( DEREF_P_CHAR(own), true );
				}
			}
			else {

				P_ITEM pi = (P_ITEM)objs.getObject();

				updContMap(pi);

				P_CHAR own=pointers::findCharBySerial(pi->getOwnerSerial32());
				pi->setOwnerSerial32( DEREF_P_CHAR(own), true );
			}
		}

		std::map< SERIAL, P_CHAR >::iterator iter( pMounted.begin() ), end( pMounted.end() );
		for( ; iter!=end; iter++)
		{
			pc = pointers::findCharBySerial(iter->first);
			if(ISVALIDPC(pc))
				pc->setOnHorse();
		}
	}

	/*!
	\brief updates containers map
	\author Luxor
	\param pi the item which the function will update in the containers map
	*/
	void updContMap(P_ITEM pi)
	{
		VALIDATEPI(pi);
		vector<P_ITEM>::iterator contIter;
		SI32 ser;

		ser= pi->getContSerial(true);
		if( ser > INVALID ) 
		{
			contIter = find(pContMap[ser].begin(), pContMap[ser].end(), pi);

			if ( !pContMap[ser].empty() && (contIter!=pContMap[ser].end()) )
				pContMap[ser].erase(contIter);
		}

		ser= pi->getContSerial();
		if( ser > INVALID) 
		{
			contIter = find(pContMap[ser].begin(), pContMap[ser].end(), pi);

			if (!pContMap[ser].empty() && contIter != pContMap[ser].end())
				pContMap[ser].erase(contIter);

			pContMap[ser].push_back(pi);
		}
	}

	/*!
	\author Luxor
	*/
	void addToStableMap(P_CHAR pet)
	{
		VALIDATEPC(pet);
		SERIAL stablemaster=pet->getStablemaster();
		if( stablemaster !=INVALID ) {

			delFromStableMap( pet );

			pStableMap[stablemaster].push_back(pet);
		}
	}

	/*!
	\author Luxor
	*/
	void delFromStableMap(P_CHAR pet)
	{
		VALIDATEPC(pet);
		SERIAL stablemaster=pet->getStablemaster();
		if( stablemaster != INVALID ) {

			std::map<SERIAL, vector <P_CHAR> >::iterator vect( pStableMap.find( stablemaster ) );
			if( ( vect!=pStableMap.end() ) && !vect->second.empty() ) {

				vector<P_CHAR>::iterator stableIter;
				stableIter = find( vect->second.begin(), vect->second.end(), pet);
				if( stableIter != vect->second.end())
					vect->second.erase(stableIter);
			}
		}
	}

	/*!
	\author Endymion
	*/
	void addToOwnerMap(P_CHAR pet)
	{
		VALIDATEPC(pet);
		SERIAL own=pet->getOwnerSerial32();
		if ( own!=INVALID ) {

			delFromOwnerMap( pet );

			pOwnCharMap[own].push_back(pet);
		}
	}

	/*!
	\author Endymion
	*/
	void delFromOwnerMap(P_CHAR pet)
	{
		VALIDATEPC(pet);
		SERIAL own=pet->getOwnerSerial32();
		if ( own!=INVALID ) {

			std::map<SERIAL, vector <P_CHAR> >::iterator vect( pOwnCharMap.find( own ) );
			if( ( vect!=pOwnCharMap.end() ) && !vect->second.empty() )
			{

				vector<P_CHAR>::iterator iter;
				iter = find(vect->second.begin(), vect->second.end(), pet);
				if( iter != vect->second.end() )
					vect->second.erase(iter);
			}

		}
	}


	/*!
	\author Endymion
	*/
	void addToOwnerMap(P_ITEM pi)
	{
		VALIDATEPI(pi);
		SERIAL own=pi->getOwnerSerial32();
		if ( own!=INVALID ) {

			delFromOwnerMap( pi );

			pOwnItemMap[own].push_back(pi);
		}
	}

	/*!
	\author Endymion
	*/
	void delFromOwnerMap(P_ITEM pi)
	{
		VALIDATEPI(pi);
		SERIAL own=pi->getOwnerSerial32();
		if ( own!=INVALID ) {

			std::map<SERIAL, vector <P_ITEM> >::iterator vect( pOwnItemMap.find( own ) );
			if( ( vect!=pOwnItemMap.end() ) && !vect->second.empty() )
			{

				vector<P_ITEM>::iterator iter;
				iter = find(vect->second.begin(), vect->second.end(), pi);
				if( iter != vect->second.end() )
					vect->second.erase(iter);
			}

		}
	}

	/*!
	\author Endymion
	*/
	void addToMultiMap( P_CHAR pc )
	{
		VALIDATEPC(pc);
		SERIAL multi=pc->getMultiSerial32();
		if ( multi!=INVALID ) {

			delFromMultiMap( pc );

			pMultiCharMap[multi].push_back(pc);
		}

	}

	/*!
	\author Endymion
	*/
	void delFromMultiMap( P_CHAR pc )
	{
		VALIDATEPC(pc);
		SERIAL multi=pc->getMultiSerial32();
		if ( multi!=INVALID ) {

			std::map<SERIAL, vector <P_CHAR> >::iterator vect( pMultiCharMap.find( multi ) );
			if( ( vect!=pMultiCharMap.end() ) && !vect->second.empty() )
			{

				vector<P_CHAR>::iterator iter;
				iter = find(vect->second.begin(), vect->second.end(), pc);
				if( iter != vect->second.end() )
					vect->second.erase(iter);
			}

		}

	}


	/*!
	\author Endymion
	*/
	void addToMultiMap( P_ITEM pi )
	{
		VALIDATEPI(pi);
		SERIAL multi=pi->getMultiSerial32();
		if ( multi!=INVALID ) {

			delFromMultiMap( pi );

			pMultiItemMap[multi].push_back(pi);
		}

	}


	/*!
	\author Endymion
	*/
	void delFromMultiMap( P_ITEM pi )
	{
		VALIDATEPI(pi);
		SERIAL multi=pi->getMultiSerial32();
		if ( multi!=INVALID ) {

			std::map<SERIAL, vector <P_ITEM> >::iterator vect( pMultiItemMap.find( multi ) );
			if( ( vect!=pMultiItemMap.end() ) && !vect->second.empty() )
			{

				vector<P_ITEM>::iterator iter;
				iter = find(vect->second.begin(), vect->second.end(), pi);
				if( iter != vect->second.end() )
					vect->second.erase(iter);
			}

		}

	}






	void eraseContainerInfo( SERIAL ser )
	{

		std::map<SERIAL, vector <P_ITEM> >::iterator vect( pContMap.find( ser ) );
		if( vect!=pContMap.end() ) {

			if( !vect->second.empty() ) {

				vector<P_ITEM>::iterator iter( vect->second.begin() );
				for( ; iter!=vect->second.end(); iter++ ) {

					(*iter)->setContSerial( INVALID, true, false);
					(*iter)->setContSerial( INVALID, false, false);

				}
			}

			pContMap.erase( vect );
		}

	}


	/*!
	\brief removes a char from chars pointer map and cleares its vector in pContMap
	\param pc the character
	\author Luxor
	*/
	void delChar(P_CHAR pc)
	{
		VALIDATEPC(pc);

		delFromStableMap(pc);
		delFromOwnerMap(pc);
#ifdef SPAR_NEW_WR_SYSTEM
		delCharFromLocationMap(pc);
#endif
		objects.eraseObject( pc );

		eraseContainerInfo( pc->getSerial32() );


	}

	/*!
	\brief removes an item from items pointer and from the vector of his container
	\author Luxor
	\param pi the item
	*/
	void delItem(P_ITEM pi)
	{
		VALIDATEPI(pi);

		if (pi->isInWorld())
		{
#ifdef SPAR_NEW_WR_SYSTEM
			pointers::delItemFromLocationMap(pi);
#else
			mapRegions->remove(pi);
#endif
		}

		objects.eraseObject(pi);

		eraseContainerInfo( pi->getSerial32() );

		SERIAL cont=pi->getContSerial();
		if ( cont > INVALID ) {

			std::map<SERIAL, vector <P_ITEM> >::iterator vect( pContMap.find( cont ) );
			if( ( vect!=pContMap.end() ) && !vect->second.empty() ) {

				vector<P_ITEM>::iterator contIter;
				contIter = find( vect->second.begin(), vect->second.end(), pi);
				if( contIter != vect->second.end() )
					vect->second.erase(contIter);
			}

		}
	}

	/*!
	\brief returns the corrispondent char of a serial
	\author Luxor
	\return the corrispondent char of the serial passed in the params
	\param serial the serial of the char which we're searching for
	*/
	P_CHAR findCharBySerial(int serial)
	{
		if (serial < 0 || !isCharSerial(serial)) return 0;

		return static_cast<P_CHAR>( objects.findObject(serial) );

	}

	/*!
	\brief returns the corrispondent item of a serial
	\author Luxor
	\return the corrispondent item of the serial passed in the params
	\param serial the serial of the item which we're searching for
	*/
	P_ITEM findItemBySerial(SERIAL serial)
	{
		if (serial < 0 || !isItemSerial(serial)) return 0;

		return static_cast<P_ITEM>( objects.findObject(serial) );
	}

	/*!
	\brief returns the corrispondent char of a serial
	\param p the pointer to serial
	\author Luxor
	\return the char we're looking for
	*/
	P_CHAR findCharBySerPtr(UI08 *p)
	{
		int serial=LongFromCharPtr(p);
		if (serial < 0) return 0;
		return findCharBySerial(serial);
	}

	/*!
	\brief returns the corrispondent item of a serial
	\author Luxor
	\param p the pointer to serial
	\return P_ITEM of the found item
	\author Luxor
	*/
	P_ITEM findItemBySerPtr(unsigned char *p)
	{
		int serial=LongFromCharPtr(p);
		if(serial < 0) return 0;
		return findItemBySerial(serial);
	}

	/*!
	\brief returns the *index element of the vector of a container (identified by serial)
	\author Luxor
	\param serial the serial of the container
	\index the pointer to the integer which we're using for the search
	\note *index should be 0 at the beginning of the search
	\return P_ITEM of the item found
	*/
	P_ITEM containerSearch(int serial, int *index)
	{
		if (serial < 0 || (*index) < 0)
			return 0;

		P_ITEM pi = 0;

		vector<P_ITEM> &pcm = pContMap[serial];

		for (pi = 0; pi == 0; (*index)++)
		{
			if ( pcm.empty())
				return 0;

			if ((UI32)(*index) >= pcm.size())
				return 0;

			pi = pcm[*index];

			if (!(ISVALIDPI(pi)))
			{
				if ((UI32)(*index)+1 < pcm.size() && !pcm.empty())
					pcm[*index] = pcm[pcm.size()-1];
			}
			pi = 0;
			pcm.pop_back();
		}

		if ( !ISVALIDPI(pi) )
			return 0;

		return pi;
	}


	P_CHAR stableSearch(int serial, int *index)
	{
		if (serial < 0 || (*index) < 0)
			return 0;
		if (pStableMap[serial].empty()) return 0;
		if ((UI32)*index >= pStableMap[serial].size()) return 0;
		P_CHAR pet = 0;
		pet = pStableMap[serial][*index];
		(*index)++;
		VALIDATEPCR(pet, 0);
		return pet;
	}

	/*!
	\author Luxor
	\brief returns the *index element with the given id and color of the vector of a container(identified by serial)
	\param serial the serial of the container
	\param index the pointer to the integer which we're using for the search
	\param id the id which we're searching for
	\param color the color which we're searching for
	\return the item we're searching for
	\note *index should be 0 at the beginning of the search
	*/
	P_ITEM containerSearchFor(const int serial, int *index, short id, short color)
	{
		P_ITEM pi;
		int loopexit=0;
		while ( ((pi = containerSearch(serial,index)) != 0) && (++loopexit < MAXLOOPS) )
		{
			if (pi->id()==id  &&
				(color==-1 || pi->color()==color) && ISVALIDPI(pi))
			return pi;
		}
		return 0;
	}

	/*!
	\brief returns the number of elements with the given id and color of the vector of a container (identified by serial)
	\author Luxor
	\return the number of elements found
	\param serial the serial of the container
	\param id the id which we're searching for
	\param color the color which we're searching for
	\param bAddAmounts if true we want to add the amount of the items to the return value
	\param recurseSubpack if true we search also in subpack
	*/
	UI32 containerCountItems(SERIAL serial, short id, short color, LOGICAL bAddAmounts, LOGICAL recurseSubpack)
	{

		std::map< SERIAL , vector<P_ITEM> >::iterator cont( pointers::pContMap.find( serial ) );
		if( cont==pointers::pContMap.end() || cont->second.empty() )
			return 0;

		UI32 total=0;

		std::vector<P_ITEM>::iterator iter( cont->second.begin() );
		for( ; iter!=cont->second.end(); iter++ )
		{

			P_ITEM pi=(*iter);
			if (pi->isContainer() && recurseSubpack) {
				total += containerCountItems(pi->getSerial32(), id, color, bAddAmounts, true);
				continue;
			}
			if ((pi->id()==id && (color==-1 || pi->color()==color))||(id==-1)) {
				if (bAddAmounts) total += pi->amount;
				else total++;
			}
		}
		return total;
	}

	/*!
	\brief returns the number of elements with the given scriptid of the vector of a container (identified by serial)
	\author Luxor
	\return the number of elements found
	\param serial the serial of the container
	\param scriptID the scriptID which we're searching for
	\param bAddAmounts if true we want to add the amount of the items to the return value
	*/
	UI32 containerCountItemsByID(SERIAL serial, UI32 scriptID, LOGICAL bAddAmounts)
	{
		std::map< SERIAL , vector<P_ITEM> >::iterator cont( pointers::pContMap.find( serial ) );
		if( cont==pointers::pContMap.end() || cont->second.empty() )
			return 0;

		UI32 total=0;

		std::vector<P_ITEM>::iterator iter( cont->second.begin() );
		for( ; iter!=cont->second.end(); iter++ )
		{

			P_ITEM pi=(*iter);
			if (pi->type == 1)	// container
			{
				total += containerCountItemsByID(pi->getSerial32(), scriptID, bAddAmounts);
				continue;
			}
			if ( pi->getScriptID() == scriptID ) {
				if (bAddAmounts) total += pi->amount;
				else total++;
			}
		}

		return total;
	}
} //namespace
