/*!
 ***********************************************************************************
 *  file    : cmdtable.h
 *  Project : Nox-Wizard
 *  Author  : Riekr
 *  Purpose : Privileges levels administration commands
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
	19/01/2003				Riekr				    Creation
 \*********************************************************************************/

#include "../nxwcommn.h"
#include "../cmdtable.h"
#include "../sndpkg.h"
#include "../client.h"
#include "privlevels.h"

// Register admin commands
void cPrivLevels::init(cCommandTable* commands) {
    commands->addGmCommand(new cCommand("SHOWPRIV3", 5, 13, cCommand::buildSteps(&SHOWPRIV3_SelectTarget, &SHOWPRIV3_ShowClearance), true));
}

// SHOWPRIV3 Command implementation
TARGETFUNC(cPrivLevels::SHOWPRIV3_SelectTarget, 0, NULL, "Select char to show command clearance:");
void cPrivLevels::SHOWPRIV3_ShowClearance(NXWCLIENT client) {
    /*TargetLocation* target= client->getLastTarget();
	P_CHAR pc = target->getChar();
	if (ISVALIDPC(pc)) {
        string priv_info(pc->getCurrentNameC());
		priv_info += " can execute the following commands:\n";
        for(td_cmditer iter= commands->getIteratorBegin(); iter!= commands->getIteratorEnd(); iter++) {
            P_COMMAND cmd = (*iter).second;
            if(cmd->isValid(pc)) {
				priv_info+= ' ';
                priv_info+= cmd->cmd_name;
            }
        }
		SndUpdscroll(client->toInt(), priv_info.size(), priv_info.c_str());
    } else
		client->sysmsg("You cannot retrieve privileges information on that.");
	*/
}
