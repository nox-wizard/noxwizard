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
	07/01/2003				Riekr				    Creation
 \*********************************************************************************/

class cTweaking {
public:
    // Register tweaking commands
    static void init(cCommandTable* commands);
    // INCZ Steps
    static void INCZ_SelectTarget(NXWCLIENT client);
    static void INCZ_Apply(NXWCLIENT client);
    // TWEAK Steps
    static void TWEAK_SelectTarget(NXWCLIENT client);
    static void TWEAK_Apply(NXWCLIENT client);
    // RENAME Steps
	static void RENAME_SelectTarget(NXWCLIENT client);
	static void RENAME_Apply(NXWCLIENT client);
    // RENAME2 Steps
	static void RENAME2_SelectTarget(NXWCLIENT client);
	static void RENAME2_Apply(NXWCLIENT client);
    // TITLE Steps
	static void TITLE_SelectTarget(NXWCLIENT client);
	static void TITLE_Apply(NXWCLIENT client);
};

