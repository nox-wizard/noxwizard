  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __TRADE_H__
#define __TRADE_H__

typedef struct {
	int layer;
	P_ITEM item;
	int amount;
} buyeditem;


#define RESTOCK_PER_TIME 20
#define CHECK_RESTOCK_EVERY 3

#include <queue>

class cRestockMng {

private:

	TIMERVAL timer;
	std::queue< SERIAL > needrestock;
	std::queue< SERIAL > restocked;

	void rewindList();
	void updateTimer();


public:
	
	cRestockMng();


	void doRestock();
	void doRestockAll();

	void addNewRestock( P_ITEM pi );

};

extern cRestockMng* Restocks;


#endif
