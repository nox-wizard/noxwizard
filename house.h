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
\brief House Related Stuff
*/
#ifndef _HOUSE_H_
#define _HOUSE_H_

#include "worldmain.h"
#include "items.h"

int 	add_hlist(int c, int h, int t);
void buildhouse( NXWCLIENT ps, P_TARGET t );
int 	del_hlist(int c, int h);
int		on_hlist(P_ITEM pi, unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4, int *li);
void 	mtarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt);


class cMulti
{
protected:
	SERIAL serial;
	SERIAL owner;
	SI32	keycode;
	LOGICAL norealmulti, nokey, boat;
	SI08 spacex1, spacey1,spacex2, spacey2; // how many tiles are used for the house zone, x1 north elongation, y1 west
	SI32 char_x, char_y, char_z;
	SI32 deed;
	SI16 lockedItems, securedItems;
	SI16 maxLockedItems, maxSecuredItems;

public:
	cMulti();
	SERIAL getSerial();
	void setSerial(SERIAL itemLink);
	SERIAL getOwner();
	void setOwner(SERIAL newOwner);
	SI32 getKeycode();
	void setKeycode(SI32 keycode);
	void getCorners(SI32 &x1, SI32 &x2, SI32 &y1, SI32 &y2 );
	void setCorners(SI32 x1, SI32 x2, SI32 y1, SI32 y2 );
	void getCharPos( int &x,  int &y,  int &z);
	void setCharPos( int x,  int y,  int z);
	int getUpperYRange();
	int getLowerYRange();
	int getLeftXRange();
	int getRightXRange();
	void changeLocks();
	SI32 getDeed();
	void setDeed(SI32 newID);
	void createMulti(UI32 multinumber, P_ITEM multiItem);
	bool isRealMulti();
	static void makeKeys(cMulti* pMulti, P_CHAR pc);
	static void cMulti::target_buildmulti( NXWCLIENT ps, P_TARGET t );
	static void cMulti::buildmulti( P_CHAR builder, P_ITEM deed);
	static cMulti* findMulti(Location position);
	bool increaseLockedItems(unsigned int amount=1);
	bool decreaseLockedItems(unsigned int amount=1);
	void setLockedItems(unsigned int amount);
	unsigned int getLockedItems();
	unsigned int getMaxLockedItems();
	void setMaxLockedItems(unsigned int amount);
	bool increaseSecuredItems(unsigned int amount=1);
	bool decreaseSecuredItems(unsigned int amount=1);
	void setSecuredItems(unsigned int amount);
	unsigned int getSecuredItems();
	unsigned int getMaxSecuredItems();
	void setMaxSecuredItems(unsigned int amount);
	SI32 getCurrentZPosition(P_CHAR pc);
	LOGICAL noKey() { return nokey;}
};

extern std::map< SERIAL, P_HOUSE > houses;

class cHouse :public cMulti
{
private:
	LOGICAL publicHouse;
	std::vector<SERIAL> friends;
	std::vector<SERIAL> coowners;
	std::vector<SERIAL> banned;
protected:

public:

	cHouse();
	void deedhouse(NXWSOCKET  s, P_ITEM pi);
	void deedhouse(P_CHAR owner);
	bool isInsideHouse(P_ITEM pi);
	bool isInsideHouse(P_CHAR pc);
	bool isInsideHouse(Location where);
	bool isInsideHouse(int x, int y, int z);
	bool inHouse(P_ITEM pi);
	bool inHouse(P_CHAR pc);
	bool inHouse(Location where);
	bool inHouse(int x, int y);
	void remove();
	bool isPublicHouse();
	void setPublicState(bool state);
	void togglePublicState();
	std::vector<SERIAL>::iterator getHouseFriends();
	std::vector<SERIAL>::iterator getHouseCoOwners();
	std::vector<SERIAL>::iterator getHouseBans();
	void addFriend(P_CHAR newfriend);
	void removeFriend(P_CHAR newfriend);
	bool isFriend(P_CHAR pc);
	std::vector<SERIAL> getHouseFriendsList();
	void addCoOwner(P_CHAR newCoOwner);
	void removeCoOwner(P_CHAR newCoOwner);
	bool isCoOwner(P_CHAR pc);
	std::vector<SERIAL> getHouseCoOwnerList();
	void addBan(P_CHAR newBanned);
	void removeBan(P_CHAR newBanned);
	bool isBanned(P_CHAR pc);
	std::vector<SERIAL> getHouseBannedList();

	void createHouse(UI32 houseNumber);
	void transfer(SERIAL newOwner);
	void save(ofstream  *output);
	void load(cStringFile& input);

};


class cHouses
{

	static std::map< SERIAL, P_HOUSE > houses;
	static std::map< int, UI32VECTOR> houseitems;
public:
	static cHouse *findHouse(Location loc);
	static cHouse *findHouse(int x, int y);
	static cHouse *cHouses::findHouse(SERIAL houseSerial);
	static std::map< SERIAL, P_HOUSE >::iterator findHouses(SERIAL owner);
	static void killkeys(SERIAL serial);
	static void buildhouse( P_CHAR builder, P_ITEM housedeed );
	static void target_buildhouse( NXWCLIENT ps, P_TARGET t );
	static void target_houseOwner( NXWCLIENT ps, P_TARGET t );
	static void target_houseEject( NXWCLIENT ps, P_TARGET t );
	static void target_houseBan( NXWCLIENT ps, P_TARGET t );
	static void target_houseFriend( NXWCLIENT ps, P_TARGET t );
	static void target_houseUnlist( NXWCLIENT ps, P_TARGET t );
	static void target_houseLockdown( NXWCLIENT ps, P_TARGET t );
	static void target_houseRelease( NXWCLIENT ps, P_TARGET t );
	static void target_houseSecureDown( NXWCLIENT ps, P_TARGET t );
	static LOGICAL house_speech( P_CHAR pc, NXWSOCKET socket, std::string &talk);
	static int 	check_house_decay();
	static void cHouses::Delete(SERIAL houseserial);
	static void cHouses::makeKeys(P_HOUSE phouse, P_CHAR pc);
	static void cHouses::safeoldsave();
	static bool cHouses::save( );
	static bool cHouses::load();
	static void cHouses::addHouseItem(int housenumber, int itemnumber);
	static UI32VECTOR  getHouseItems(int housenumber);
	static void cHouses::addHouse(P_HOUSE newHouse );
	static void cHouses::makeHouseItems(int housenumber, P_CHAR owner, P_ITEM multi, LOGICAL key);
	static std::map< SERIAL, P_HOUSE > cHouses::findOwnedHouses(SERIAL owner);
	static void cHouses::archive();

};

#endif
