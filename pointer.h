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
	
	// Sparhawk
	
#ifdef SPAR_NEW_WR_SYSTEM
	enum eCharWorldMap
	{
		NONE	=  0,
		ONLINE	=  1,
		OFFLINE =  2,
		NPC	=  4
	};

	class NxwCharWrapper;
	class NxwItemWrapper;

	void addCharToWorldMap( const P_CHAR who );
	void delCharFromWorldMap( const P_CHAR who );
	NxwCharWrapper getCharFromWorldMap( SI32 x, SI32 y, SI32 range, UI32 flags = 0 );
	void addItemToWorldMap( const P_ITEM what );
	void delItemFromWorldMap( const P_ITEM what );
	NxwItemWrapper getItemFromWorldMap( SI32 x, SI32 y, SI32 range, UI32 flags = 0 );
#endif
}

#endif
