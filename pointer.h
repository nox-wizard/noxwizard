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
#ifdef SPAR_LOCATION_MAP
	//
	// Sparhawk:	mapRegion replacement (work in progress)
	//
	enum
	{
		NONE		=   0,
		ONLINE		=   1,
		OFFLINE 	=   2,
		NPC		=   4,
		EXCLUDESELF	=   8,
		DEAD		=  16,
		PARTYMEMBER	=  32,
		GUILDMEMBER	=  64,
		COMBATTARGET	= 128
	};

	void 		addToLocationMap( const P_OBJECT pObject );
	void 		updateLocationMap( const P_OBJECT pObject );
	void 		delFromLocationMap( const P_OBJECT pObject );

	void		showCharLocationMap();
	void		addCharToLocationMap( const P_CHAR who );
	void 		delCharFromLocationMap( const P_CHAR who );
	PCHAR_VECTOR*	getNearbyChars( P_OBJECT pObject, UI32 range, UI32 flags = 0 );
	PCHAR_VECTOR*	getNearbyChars( UI32 x, UI32 y, UI32 range, UI32 flags = 0, P_CHAR pSelf = 0 );

	void		showItemLocationMap();
	void		addItemToLocationMap( const P_ITEM what );
	void		delItemFromLocationMap( const P_ITEM what );
	PITEM_VECTOR*	getNearbyItems( cObject* pObject, UI32 range, UI32 flags = 0 );
	PITEM_VECTOR*	getNearbyItems( UI32 x, UI32 y, UI32 range, UI32 flags = 0, P_ITEM pSelf = 0 );
#endif
}

#endif
