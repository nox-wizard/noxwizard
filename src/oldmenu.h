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
\brief Old Menu related stuff
*/

#ifndef _OLDMENU_H_
#define _OLDMENU_H_


#include "nxwcommn.h"
#include "packets.h"
#include "menu.h"


/*!
\brief an Old type menu
*/
class cOldMenu : public cBasicMenu
{

	friend class cMakeMenu;
	friend class cPartyMenu;

protected:

	std::map< UI08, std::map< UI32, std::wstring >  > allPages; //!< all pages

	virtual cServerPacket* build();
	void buildClassic();
	void buildIconList();

public:

	P_MENU type;
	
	wstring title; //!< title
	UI32 style; //!< style
	UI32 color; //!< color
	UI32 width; //!< width
	UI32 rowForPage; //!< row for page

	cOldMenu();
	~cOldMenu();

	void setParameters( int rowForPage, int pageCount );
	virtual void addMenuItem( int page, int idx, std::wstring desc );

	virtual void handleButton( NXWCLIENT ps, cClientPacket* pkg  );

};

typedef cOldMenu*	P_OLDMENU;



#endif
