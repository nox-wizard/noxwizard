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
\brief Header that defines pointers namespace
*/

#ifndef __POINTERS_H__
#define __POINTERS_H__

void setptr(lookuptr_st *ptr, int item);
void removefromptr(lookuptr_st *ptr, int nItem);// - remove item from pointer array
int findbyserial(lookuptr_st *ptr, int nSerial, int nType);// - find item in a pointer array
void getWorldCoordsFromSerial (int sr, int& px, int& py, int& pz, int& ch, int& it);


/*!
\brief Pointer related stuff
*/
namespace pointers {

	//@{
	/*!
	\name Maps declarations
	\author Luxor
	*/
	extern std::map<SERIAL, vector <P_CHAR> > pStableMap;
	extern std::map<SERIAL, P_CHAR > pMounted;

	extern std::map<SERIAL, vector <P_CHAR> > pOwnCharMap;
	extern std::map<SERIAL, vector <P_ITEM> > pOwnItemMap;

	extern std::map<SERIAL, vector <P_ITEM> > pContMap;

	extern std::map<SERIAL, vector <P_CHAR> > pMultiCharMap;
	extern std::map<SERIAL, vector <P_ITEM> > pMultiItemMap;
	//@}

	void init();
	void delChar(P_CHAR pc);
	void delItem(P_ITEM pi);
	
	void updContMap(P_ITEM pi);
	
	void addToStableMap(P_CHAR pet);
	void delFromStableMap(P_CHAR pet);

	void addToOwnerMap( P_CHAR pet );
	void delFromOwnerMap( P_CHAR pet );
	void addToOwnerMap( P_ITEM pi );
	void delFromOwnerMap( P_ITEM pi );

	void addToMultiMap( P_CHAR pc );
	void delFromMultiMap( P_CHAR pc );
	void addToMultiMap( P_ITEM pi );
	void delFromMultiMap( P_ITEM pi );
	
	P_CHAR findCharBySerial(int serial);
	P_ITEM findItemBySerial(int serial);
	P_CHAR findCharBySerPtr(unsigned char *p);
	P_ITEM findItemBySerPtr(unsigned char *p);
	P_ITEM containerSearch(int serial, int *index);
	P_ITEM containerSearchFor(const int serial, int *index, short id, short color);
	UI32 containerCountItems(SERIAL serial, short id, short color, LOGICAL bAddAmounts = true, LOGICAL recurseSubpack=true);
	UI32 containerCountItemsByID(SERIAL serial, UI32 scriptID, LOGICAL bAddAmounts);
	P_CHAR stableSearch(int serial, int *index);

	//
	// Sparhawk:	mapRegion replacement (work in progress)
	//
	typedef std::vector< P_CHAR > pCharVector;
	typedef pCharVector::iterator pCharVectorIt;
	typedef std::vector< P_ITEM > pItemVector;
	typedef pItemVector::iterator pItemVectorIt;

	enum
	{
		NONE		=  0,
		ONLINE		=  1,
		OFFLINE 	=  2,
		NPC		=  4,
		SELF		=  8
	};
	
	void showCharLocationMap();
	void addCharToLocationMap( const P_CHAR who );
	void delCharFromLocationMap( const P_CHAR who );
	pCharVector* getCharsNearLocation( cObject* pObject, SI32 range, UI32 flags );
	pCharVector* getCharsNearLocation( SI32 x, SI32 y, SI32 range, UI32 flags = 0 );
	void addItemToLocationMap( const P_ITEM what );
	void delItemFromLocationMap( const P_ITEM what );
	pItemVector getItemFromLocationMap( SI32 x, SI32 y, SI32 range, UI32 flags = 0 );
}

#endif
