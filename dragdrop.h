  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file dragdrop.h
\brief DragDrop related stuff
*/

#ifndef __DRAGDROP_H_
#define __DRAGDROP_H_

void get_item(NXWCLIENT ps);	//!< Client grabs an item
void wear_item(NXWCLIENT ps);	//!< Item is dropped on paperdoll
void drop_item(NXWCLIENT ps);	//!< Item is dropped on ground, char or item

#endif
