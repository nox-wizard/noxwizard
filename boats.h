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
extern UI08 cShipItems[4][6];

#ifndef __Boats_h
#define __Boats_h

struct boat_db
{
	int serial;
	int tiller_serial;
	int l_plank_serial;
	int r_plank_serial;
	int container;
	P_ITEM p_serial;
	P_ITEM p_tiller;
	P_ITEM p_l_plank;
	P_ITEM p_r_plank;
	P_ITEM p_container;

};


P_ITEM findmulti(Location where);
bool inmulti(Location where,P_ITEM pi);
void insert_boat(P_ITEM pi);
boat_db* search_boat(SI32 ser);
P_ITEM search_boat_by_plank(P_ITEM pl);

namespace boats
{
	LOGICAL boat_collision(P_ITEM pBoat1,UI16 x1, UI16 y1,SI08 dir,P_ITEM pBoat2);
	LOGICAL collision(P_ITEM pi, Location where,SI08 dir);
	LOGICAL good_position(P_ITEM pBoat, Location where, SI08 dir);
	LOGICAL tile_check(st_multi multi,P_ITEM pBoat,map_st map,UI16 x, UI16 y ,SI08 dir);
	void LeaveBoat(P_CHAR pc, P_ITEM pi);
	void TurnStuff_i(P_ITEM, P_ITEM, SI08, int);

	void TurnStuff_c(P_ITEM, P_CHAR, SI08, int);
	void iMove(NXWSOCKET s, SI08 dir, P_ITEM pBoat, LOGICAL forced = true);

	P_ITEM GetBoat(Location pos);
	LOGICAL Speech(P_CHAR pc, NXWSOCKET socket, std::string &talk );
	void OpenPlank(P_ITEM pi);
	void PlankStuff(P_CHAR pc, P_ITEM pi);
	LOGICAL Build(NXWSOCKET s, P_ITEM pBoat, char);
	void Move(NXWSOCKET  s, SI08 dir, P_ITEM pBoat);
	void Turn(P_ITEM, int);
}

#endif
