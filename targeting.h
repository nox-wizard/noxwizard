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
\brief Header for targeting handling class
*/

#ifndef _TARGETING_INCLUDED
#define _TARGETING_INCLUDED

#include "typedefs.h"
#include "target.h"

int BuyShop(NXWSOCKET s, SERIAL c);

void target_envoke( NXWCLIENT ps, P_TARGET t );
void target_key( NXWCLIENT ps, P_TARGET t );
void target_axe( NXWCLIENT ps, P_TARGET t );
void target_sword( NXWCLIENT ps, P_TARGET t );

void target_expPotion( NXWCLIENT ps, P_TARGET t );
void target_npcMenu( NXWCLIENT ps, P_TARGET t );
void target_trigger( NXWCLIENT ps, P_TARGET t );

void target_follow( NXWCLIENT ps, P_TARGET t );
void target_attack( NXWCLIENT ps, P_TARGET t );
void target_playerVendorBuy( NXWCLIENT ps, P_TARGET t );
void target_allAttack( NXWCLIENT ps, P_TARGET t );
void target_fetch( NXWCLIENT ps, P_TARGET t );
void target_guard( NXWCLIENT ps, P_TARGET t );
void target_transfer( NXWCLIENT ps, P_TARGET t );






#endif	// _TARGETING_INCLUDED
