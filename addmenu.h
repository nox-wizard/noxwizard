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
\brief Addmenu from MakeItem stuff
*/


#ifndef __ADDMENU_H
#define __ADDMENU_H


#include "menu.h"
#include "oldmenu.h"
#include "utils.h"
#include "chars.h"
#include "client.h"


class cRawItem {

public:

    SI32 id;
    COLOR color;
    UI32 number;

	cRawItem( std::string& s );
	cRawItem( SI32 id=0, COLOR color=0, UI32 number=0 );
	~cRawItem();

};


/*!
\brief Class MakeItem
*/
class cMakeItem {

public:

    cScriptCommand* command;
    SI32 skillToCheck;
    SI32 minskill;
    SI32 maxskill;
	cRawItem reqitems[2];

    
	cMakeItem();
	~cMakeItem();
    bool checkReq( P_CHAR pc, bool inMenu = false, cRawItem* def = NULL );
};

class cMakeMenu : public cBasicMenu 
{

	private:
		std::vector<cMakeItem*>	makeItems;

	protected:

		SERIAL section;
		P_OLDMENU oldmenu;

		virtual void loadFromScript( P_CHAR pc );
		std::string cleanString( std::string s );
		void execMake( NXWCLIENT ps, UI32 item );

	protected:
		virtual cServerPacket* build();

	public:

		int skill;
		cRawItem mat[2];
		
		cMakeMenu( SERIAL section );
		cMakeMenu( SERIAL section, P_CHAR pc, int skill, UI16 firstId=0, COLOR firstColor=0, UI16 secondId=0, COLOR secondColor=0 );

		~cMakeMenu();

		virtual void handleButton( NXWCLIENT ps, cClientPacket* pkg  );

};

class cAddMenu : public cMakeMenu 
{

	private:
		std::vector< cScriptCommand > commands;
		virtual void loadFromScript( P_CHAR pc );

	public:
		cAddMenu( SERIAL section, P_CHAR pc );
		~cAddMenu();

		virtual void handleButton( NXWCLIENT ps, cClientPacket* pkg  );

};

void showAddMenu( P_CHAR pc, int menu );

/*!
\brief Open a scripted menu
\author Endymion
*/
inline void itemmenu( NXWSOCKET s, SI32 m )
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	if( ISVALIDPC( pc ) )
	{
		showAddMenu( pc, m ); 
	}
}

/*!
\brief Open one of the gray GM Call menus
\author Endymion
*/
inline void gmmenu( NXWSOCKET s, SI32 m )
{
	itemmenu( s, 7009+m );
}



#endif
