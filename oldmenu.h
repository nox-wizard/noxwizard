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

#include "nxwcommn.h"
#include "packets.h"
#include "menu.h"

#ifndef _OLDMENU_H_
#define _OLDMENU_H_

/*!
\brief an Old type menu
*/
class cOldMenu 
{

protected:

	UI32 style; //!< style
	UI32 color; //!< color
	UI32 width; //!< width
	wstring title; //!< title

	std::map< UI32, std::map< UI32, std::wstring >  > allPages; //!< all pages

	virtual void buildOldMenu();

public:
	cOldMenu();
	virtual ~cOldMenu();

	void setParameters( int numPerPage, int numpages );
	void addMenuItem( int page, int idx, char *desc );
	void showMenu( NXWSOCKET s );
	void setTitle( char *str );
	void setWidth( int width );
	void setStyle( int style, int color = 0 );
	void setColor( int color );
	virtual void buttonSelected( NXWSOCKET s, unsigned short int buttonPressed, int type );
	virtual void show( P_CHAR pc );

};

/*!
\brief an Old type menu [ CLASSIC ]
*/
class cOldMenuClassic : public cOldMenu , public cMenu
{

private:

	void buildOldMenu();

public:
	cOldMenuClassic();
	virtual ~cOldMenuClassic();

	virtual void buttonSelected( NXWSOCKET s, unsigned short int buttonPressed, int type );

};

/*!
\brief an Old type menu [ ICONLIST ]
*/
class cOldMenuIconList : public cOldMenu, public cBasicMenu
{

private:

	void buildOldMenu();

public:
	cOldMenuIconList();
	virtual ~cOldMenuIconList();

	virtual void buttonSelected( NXWSOCKET s, unsigned short int buttonPressed, int type );
	virtual void show( P_CHAR pc );

};


#endif