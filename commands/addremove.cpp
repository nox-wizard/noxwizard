  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "cmdtable.h"
#include "sndpkg.h"
#include "addremove.h"
#include "archive.h"
#include "inlines.h"
#include "client.h"

// Register admin commands
void cAddRemove::init(cCommandTable* commands) {
    commands->addGmCommand(new cCommand("WIPE",  1, 4, cCommand::buildSteps(&WIPE_SelectTarget1, &WIPE_SelectTarget2, &WIPE_Execute), false));
    commands->addGmCommand(new cCommand("IWIPE", 1, 5, cCommand::buildSteps(&IWIPE_SelectTarget1, &IWIPE_SelectTarget2, &IWIPE_Execute), false));
}

// WIPE Command implementation
// (d d d d / nothing) Deletes ALL NPC's and items inside a specified square.
// <UL><LI>With no arguments, /WIPE will ask you to click in opposing corners of
// the square.</LI>
// <LI>You may also specify coordinates - X1, Y1, X2, Y2.</LI>
// </UL>
void cAddRemove::WIPE_SelectTarget1(NXWCLIENT client) {
//	addid1[s]=0; // addid1[s]==0 is used to denote a true wipe
	switch(client->cmdParams.size()) {
		case 0:
			client->doTargeting("Please select first corner of wiping box:");
			break;
		case 1: {
			// check if we've got to wipe all..
			td_cmdparams::iterator it= std::find_if(client->cmdParams.begin(), client->cmdParams.end(), std::bind1st (std::equal_to<td_cmdparams::value_type>(), "ALL"));
			if(it!=client->cmdParams.end()) 
				WipeAll(client);
			else
				// ..otherwise error
				goto bailout;
			break;
			}
		case 4:
			// wipe from input coordinates
			WipeArea(client, false,
					 str2num(client->cmdParams[0]),
					 str2num(client->cmdParams[1]),
					 str2num(client->cmdParams[2]),
				     str2num(client->cmdParams[3]));
			break;
		default:
bailout:
			client->sysmsg("Syntax is: WIPE [<x1 y1 x2 y2>|<ALL>]");
	}
}
void cAddRemove::WIPE_SelectTarget2(NXWCLIENT client) {
	client->doTargeting("Please select second corner of wiping box:");
}
void cAddRemove::WIPE_Execute(NXWCLIENT client) {
	td_targets targets= client->getTargets();
	if(targets.size()==2) {
		int x1,x2,y1,y2,z;
		targets[0].getXYZ(x1, y1, z);
		targets[1].getXYZ(x2, y2, z);
		WipeArea(client, false, x1, y1, x2, y2);
	} else
		client->sysmsg("Server error.");
}

/*!
\brief wipes every wipeable object in an area
\author Riekr & Luxor
*/
void cAddRemove::WipeArea(NXWCLIENT client, bool inverse, UI32 x1, UI32 y1, UI32 x2, UI32 y2)
{
	if (client == NULL)
		return;
	P_CHAR pc = client->currChar();
	VALIDATEPC(pc);
	
	if (x1>x2) 
		swap(x1, x2);
	if (y1>y2)
		swap(y1, y2);
	if(inverse)
		InfoOut("%s has initiated an inverse item wipe (%i,%i-%i,%i)\n", pc->getCurrentNameC(), x1,y1,x2,y2);
	
	cAllObjectsIter objs;
	P_ITEM pi = NULL;
	for( objs.rewind(); !objs.IsEmpty(); objs++ ) {
        	pi = pointers::findItemBySerial( objs.getSerial() );
		if (ISVALIDPI(pi)) {
			bool del= (pi->getPosition().x >= x1) && (pi->getPosition().x <= x2) &&
				  (pi->getPosition().y >= y1) && (pi->getPosition().y <= y2);
			if(inverse)
				del= !del;
			if(del && pi->isInWorld() && pi->wipe==0)
				pi->Delete();
		}
    }
}

/*!
\brief wipes every wipeable object
\author Luxor
*/
void cAddRemove::WipeAll(NXWCLIENT client) {
	if (client == NULL)
		return;
	
	P_CHAR pc = client->currChar();
	VALIDATEPC(pc);
	InfoOut( "%s has initiated an item wipe\n", pc->getCurrentNameC() );
	cAllObjectsIter objs;
	P_ITEM pi = NULL;
	for( objs.rewind(); !objs.IsEmpty(); objs++ ) {
		pi = pointers::findItemBySerial( objs.getSerial() );
		if ( ISVALIDPI(pi) && pi->isInWorld() && pi->wipe == 0 )
			pi->Delete();
	}
	
	sysbroadcast( TRANSLATE("All items have been wiped") );
}


TARGETFUNC(cAddRemove::IWIPE_SelectTarget1, 0, NULL, "Select first corner where to keep items:");
TARGETFUNC(cAddRemove::IWIPE_SelectTarget2, 0, NULL, "Select second corner where to keep items:");
void cAddRemove::IWIPE_Execute(NXWCLIENT client) {
	td_targets targets= client->getTargets();
	if(targets.size()==2) {
		int x1,x2,y1,y2,z;
		targets[0].getXYZ(x1, y1, z);
		targets[1].getXYZ(x2, y2, z);
		WipeArea(client, true, x1, y1, x2, y2);
	} else
		client->sysmsg("Server error.");
}
