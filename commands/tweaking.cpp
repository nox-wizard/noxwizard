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

// Register tweaking commands
void cTweaking::init(cCommandTable* commands) {
    commands->addGmCommand(new cCommand("INCZ", 5, 14, cCommand::buildSteps(&INCZ_SelectTarget, &INCZ_Apply), true));
    commands->addGmCommand(new cCommand("TWEAK", 3, 15, cCommand::buildSteps(&TWEAK_SelectTarget, &TWEAK_Apply), true));
    commands->addGmCommand(new cCommand("RENAME", 1, 8, cCommand::buildSteps(&RENAME_SelectTarget, &RENAME_Apply), true));
    commands->addGmCommand(new cCommand("RENAME2", 5, 5, cCommand::buildSteps(&RENAME2_SelectTarget, &RENAME2_Apply), true));
    commands->addGmCommand(new cCommand("TITLE", 1, 9, cCommand::buildSteps(&TITLE_SelectTarget, &TITLE_Apply), true));
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

// TWEAK Command implementation
TARGETFUNC(cTweaking::TWEAK_SelectTarget, 0, "Please select a char or item to tweak.", "Select char or item to tweak:");
void cTweaking::TWEAK_Apply(NXWCLIENT client) { //Lag fix -- Zippy
    TargetLocation* target= client->getLastTarget();
	P_CHAR pc_user = MAKE_CHAR_REF( currchar[client->toInt()] );
    P_CHAR pc= target->getChar();
    if(ISVALIDPC(pc)) {
		newAmxEvent("gui_charProps")->Call( pc->getSerial32(), pc_user->getSerial32(), 1 );
    } else { //item
        P_ITEM pi= target->getItem();
        if(ISVALIDPI(pi)) {
			newAmxEvent("gui_itemProps")->Call( pi->getSerial32(), pc_user->getSerial32(), 1 );
		}
    }
}

// RENAME Command implementation
TARGETFUNC(cTweaking::RENAME_SelectTarget, 1, "Please specify new name.", "Select object to rename:");
void cTweaking::RENAME_Apply(NXWCLIENT client) {
	TargetLocation* target= client->getLastTarget();
	string text= client->getParamsAsString();
	P_CHAR pc= target->getChar();
	if(ISVALIDPC(pc)) {
		pc->setCurrentName(text);
	}
	P_ITEM pi= target->getItem();
	if(ISVALIDPI(pi)) {
		pi->setCurrentName(text);
	}
}

// RENAME2 Command implementation
TARGETFUNC(cTweaking::RENAME2_SelectTarget, 1, "Please specify new name.", "Select object to rename:");
void cTweaking::RENAME2_Apply(NXWCLIENT client) {
	TargetLocation* target= client->getLastTarget();
	string text= client->getParamsAsString();
	P_CHAR pc= target->getChar();
	if(ISVALIDPC(pc)) {
		pc->setCurrentName(text);
	}
	P_ITEM pi= target->getItem();
	if(ISVALIDPI(pi)) {
		pi->setSecondaryName(text.c_str());
	}
}

// TITLE Command implementation
TARGETFUNC(cTweaking::TITLE_SelectTarget, 1, "Please specify new title.", "Select char for title change:");
void cTweaking::TITLE_Apply(NXWCLIENT client) {
	TargetLocation* target= client->getLastTarget();
	string text= client->getParamsAsString();
	P_CHAR pc= target->getChar();
	if(ISVALIDPC(pc)) {
		pc->title = text;
	} else
		client->sysmsg("Please select a character.");
}
