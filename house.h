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

#include "items.h"

extern std::map< SERIAL, P_CHAR > houses;

int 	add_hlist(int c, int h, int t);
void buildhouse( NXWCLIENT ps, P_TARGET t );
int 	del_hlist(int c, int h);
int		on_hlist(P_ITEM pi, unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4, int *li);
void 	mtarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt);



class cHouse : public cItem
{
private:
	SERIAL owner;
protected:

public:

	cHouse();
	void getCorners(unsigned int &x1, unsigned int &x2, unsigned int &y1, unsigned int &y2 );
	int getUpperYRange();
	int getLowerYRange();
	int getLeftXRange();
	int getRightXRange();
	SERIAL getOwner();
	void setOwner(SERIAL newOwner);
	void deedhouse(NXWSOCKET  s, P_ITEM pi);

	bool inHouse(P_ITEM pi);
	bool inHouse(Location where);
};

class cHouses
{
	static std::map< SERIAL, P_CHAR > houses;
public:
	static cHouse *findHouse(Location loc);
	static cHouse *findHouse(int x, int y, int z);
	static std::map< SERIAL, P_HOUSE >::iterator *findHouses(SERIAL owner);
	static void killkeys(SERIAL serial);
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

};

#endif
