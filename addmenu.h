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
	cRawItem( SI32 id, COLOR color, UI32 number );
	~cRawItem();

};


/*!
\brief Class MakeItem
*/
class cMakeItem {

public:

    class cScriptCommand command;
    SI32 skillToCheck;
    SI32 minskill;
    SI32 maxskill;
    SI32 reqspell;
	std::vector<cRawItem> reqitems;
    SI32 mana, stam, hit;

    
	cMakeItem();
	~cMakeItem();
    bool checkReq( P_CHAR pc, bool inMenu = false );
};

cMakeItem* getcMakeItem( SERIAL n );

class cMakeMenu : public cBasicMenu 
{

	private:
		int skill;
		std::vector<cMakeItem>*	makeItems;

	protected:

		SERIAL section;
		P_OLDMENU oldmenu;
		std::vector< cScriptCommand > commands;

		virtual void loadFromScript( P_CHAR pc, P_ITEM first, P_ITEM second );
		bool checkShouldAdd( class cScpEntry* entry, P_CHAR pc);
		std::string cleanString( std::string s );

	protected:
		virtual cServerPacket* build();

	public:
		cMakeMenu( SERIAL section, P_CHAR pc, int skill, P_ITEM first, P_ITEM second=NULL );
		~cMakeMenu();

		virtual void handleButton( NXWCLIENT ps, cClientPacket* pkg  );

};

class cAddMenu : public cMakeMenu 
{

	private:
		virtual void loadFromScript( P_CHAR pc = NULL );

	protected:
		virtual cServerPacket* build();

	public:
		cAddMenu( SERIAL section, P_CHAR pc );
		~cAddMenu();

		virtual void handleButton( NXWCLIENT ps, cClientPacket* pkg  );

};

void execMake( P_CHAR pc, int n );
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
