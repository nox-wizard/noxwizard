  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/



#ifndef __BOATS_H__
#define __BOATS_H__


#include "nxwcommn.h"
#include "data.h"
#include "target.h"
#include "house.h"
/*!
\file
\author Elcabesa
\brief Boat System Stuff
\note over an existing old boat system
*/

//For iSizeShipOffsets->
#define PORT_PLANK 0
#define STARB_PLANK 1
#define HOLD 2
#define TILLER 3

//For iShipitem::
#define PORT_P_C 0//Port Plank Closed
#define PORT_P_O 1//Port Planl Opened
#define STAR_P_C 2//Starboard Plank Closed
#define STAR_P_O 3//Starb Plank Open
#define TILLERID 5//Tiller
#define HOLDID 4//Hold

extern signed short int iSmallShipOffsets[4][4][2];
extern signed short int iMediumShipOffsets[4][4][2];
extern signed short int iLargeShipOffsets[4][4][2];
extern char cShipItems[4][6];

//NEW BOAT SYSTEM

//bool check_boat_position(NXWSOCKET  s,P_ITEM pBoat);


class cBoat : public cMulti
{
private:
	SERIAL tiller_serial;
	SERIAL l_plank_serial;
	SERIAL r_plank_serial;
	SERIAL container;
	P_ITEM p_serial;
	P_ITEM p_tiller;
	P_ITEM p_l_plank;
	P_ITEM p_r_plank;
	P_ITEM p_container;
	protected:
		LOGICAL boat_collision(P_ITEM pBoat1,int x1, int y1,int dir,P_ITEM pBoat2);
		LOGICAL collision(P_ITEM pi, Location where,int dir);
		LOGICAL good_position(P_ITEM pBoat, Location where, int dir);
		LOGICAL tile_check(multi_st multi,P_ITEM pBoat,map_st map,int x, int y ,int dir);
		void LeaveBoat(P_CHAR pc, P_ITEM pi);
		void TurnStuff_i(P_ITEM, P_ITEM, int, int);

		void TurnStuff_c(P_ITEM, P_CHAR, int, int);
		void iMove(NXWSOCKET  s, int dir, P_ITEM pBoat, LOGICAL forced = true);

		void TurnShip( UI08 size, SI32 dir, P_ITEM pPort, P_ITEM pStarboard, P_ITEM pTiller, P_ITEM pHold );
	public:

		cBoat();
		P_ITEM GetBoat(Location pos);
		virtual ~cBoat();
		LOGICAL Speech(P_CHAR pc, NXWSOCKET socket, std::string &talk );
		void OpenPlank(P_ITEM pi);
		void PlankStuff(P_CHAR pc, P_ITEM pi);
		LOGICAL Build(NXWSOCKET s, P_ITEM pBoat, char);
		void Move(NXWSOCKET  s, int dir, P_ITEM pBoat);
		void Turn(P_ITEM, int);
		void remove();
		void setTiller(SERIAL tillerSerial);
		void setTiller(P_ITEM tiller);
		void setLeftPlank(SERIAL plankSerial);
		void setLeftPlank(P_ITEM plank);
		void setRightPlank(SERIAL plankSerial);
		void setRightPlank(P_ITEM plank);
		void setHold(SERIAL holdSerial);
		void setHold(P_ITEM hold);
		void setShipLink(P_ITEM ship);
		P_ITEM getTiller();
		P_ITEM getLeftPlank();
		P_ITEM getRightPlank();
		P_ITEM getHold();
		P_ITEM getShipLink();
		static LOGICAL makeBoatItems(P_ITEM pBoat);
		static void buildShip( P_CHAR builder, P_ITEM shipdeed);
		static void target_buildShip( NXWCLIENT ps, P_TARGET t);
		static P_ITEM findmulti(Location where);
		static bool inmulti(Location where,P_ITEM pi);
		static void cBoat::insert_boat(P_ITEM pi, cBoat* boat);
		static void cBoat::remove_boat(P_ITEM pi);
		static cBoat* search_boat(SI32 ser);
		static P_ITEM search_boat_by_plank(P_ITEM pl);

};

typedef cBoat* P_BOAT;
typedef std::map<int,P_BOAT> BOATS;
extern BOATS	s_boat;

extern cBoat* Boats;

#endif
