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
\author Riekr
\brief Add & Remove commands
\date 20/01/2003 created
*/

#ifndef __ADDREMOVE_H
#define __ADDREMOVE_H

class cAddRemove {
public:
    // Register administrative commands
    static void init(cCommandTable* commands);
    // WIPE Steps
    static void WIPE_SelectTarget1(NXWCLIENT client);
    static void WIPE_SelectTarget2(NXWCLIENT client);
    static void WIPE_Execute(NXWCLIENT client);
	// Specialized for WIPE
	static void WipeAll(NXWCLIENT client);
	// IWIPE Steps
    static void IWIPE_SelectTarget1(NXWCLIENT client);
    static void IWIPE_SelectTarget2(NXWCLIENT client);
    static void IWIPE_Execute(NXWCLIENT client);
	// IWIPE and WIPE commons
	static void WipeArea(NXWCLIENT client, bool inverse, UI32 x1, UI32 y1, UI32 x2, UI32 y2);
};

#endif
