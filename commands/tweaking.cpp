/*!
 ***********************************************************************************
 *  file    : cmdtable.h
 *  Project : Nox-Wizard
 *  Author  : Riekr
 *  Purpose : Tweaking commands implementations
 ***********************************************************************************
 */                                                                               /*
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW)		   [http://www.noxwizard.com]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums or mail staff@noxwizard.com  ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    CHANGELOG:-------------------------------------------------------------------
    DATE					DEVELOPER				DESCRIPTION
    -----------------------------------------------------------------------------
	06/01/2003				Riekr				    Creation
 \*********************************************************************************/
 

#include "../nxwcommn.h"
#include "../cmdtable.h"
#include "tweaking.h"
#include "../client.h"
#include "../chars.h"
#include "../items.h"
#include "../basics.h"

// Register tweaking commands
void cTweaking::init(cCommandTable* commands) {
    commands->addGmCommand(new cCommand("INCZ", 5, 14, cCommand::buildSteps(&INCZ_SelectTarget, &INCZ_Apply), true));
}

// INCZ Command implementation
TARGETFUNC(cTweaking::INCZ_SelectTarget, 1, "Please specify Z increase.", "Select object to modify Z coordinate:");
void cTweaking::INCZ_Apply(NXWCLIENT client) {
    const int amount= str2num(client->cmdParams[0]);
    TargetLocation* target= client->getLastTarget();
    P_ITEM pi= target->getItem();
    if(ISVALIDPI(pi)) {
        Location pipos= pi->getPosition();
		pi->MoveTo(pipos.x, pipos.y, pipos.z+amount);
        pi->Refresh();
    } else {
        P_CHAR pc= target->getChar();
        if(ISVALIDPC(pc)) {
            Location pcpos= pc->getPosition();
    		pc->MoveTo(pcpos.x, pcpos.y, pcpos.z+amount);
            pc->teleport();
        }
    }
    client->sysmsg("Z coordinate modified by %d", amount);
}

