  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "racegender.h"

class RaceStartItem
{
//
// Quick & incomplete solution to add wearable/equipable items to char when starting as a race member & keeping protected items on death
//
	public:
		bool					itemlist;
		UI32					itemReference;
		bool					skinColor;
		bool					protectedItem;
		Gender				gender;
		unsigned char	layer;			// needed to prevent items from disappearing on death
	public:
				RaceStartItem( void );
};
