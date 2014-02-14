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
\brief Walking Functions
*/

#ifndef __WALKING_H__
#define __WALKING_H__

/*!
\author Luxor
*/
//namespace walking {
	void handleCharsAtNewPos( P_CHAR pc );
	bool handleItemsAtNewPos(P_CHAR pc, int oldx, int oldy, int newx, int newy);
	void sendToPlayers( P_CHAR pc, SI08 dir );
//} //namespace walking

void walking(P_CHAR pc, int dir, int sequence);
void walking2(P_CHAR pc_s);
void npcwalk( P_CHAR pc_i, int newDirection, int type);


#endif

