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
\brief Bounty System Stuff
\author Dupois
\since July 17, 2000
*/

#ifndef __BOUNTY_H__
#define __BOUNTY_H__

#include "msgboard.h"

// Ask victim if they want to post a bounty on the
// murderer, and if so, then return the amount of the
// reward they want to place on the murderers head.
void BountyAskVictim  ( SERIAL nVictimSerial,
                        SERIAL nMurdererSerial );

// Create the bounty on the murderer
bool BountyCreate     ( P_CHAR pc,
                        int nRewardAmount );

// Remove the bounty from the murderer
bool BountyDelete     ( P_CHAR pc );

// Check and then withdraw the bounty amount
bool BountyWithdrawGold( P_CHAR pVictim, int nAmount );

#endif
