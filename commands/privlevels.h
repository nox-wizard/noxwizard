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

class cPrivLevels {
public:
    // Register administrative commands
    static void init(cCommandTable* commands);
    // SHOWPRIV3 Steps
    static void SHOWPRIV3_SelectTarget(NXWCLIENT client);
    static void SHOWPRIV3_ShowClearance(NXWCLIENT client);
};

