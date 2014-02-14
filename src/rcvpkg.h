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
\brief Receive Packets stuff
*/

#ifndef __RCVPKG_H
#define __RCVPKG_H

#include "packets.h"

void RcvAttack(NXWCLIENT ps);
void AttackStuff (NXWSOCKET  s, P_CHAR victim);
void profileStuff( NXWCLIENT ps, cPacketCharProfileReq& p );

#endif
