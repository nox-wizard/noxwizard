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

class cAddMenu : public cBasicMenu 
{

	private:

		SERIAL section;
		std::vector< cScriptCommand > commands;
		P_OLDMENU oldmenu;

		void loadFromScript( P_CHAR pc = NULL );
		bool checkShouldAdd ( class cScpEntry* entry, P_CHAR pc);
		std::string cleanString( std::string s );

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



/*



struct RawItem {
    int id;
    int color;
    UI32 number;
    void parse(char *sz);
};

/*!
\brief Class MakeItem
*//*
class MakeItem {
public:
	//static const int MAXREQITEM =8;
	enum { MAXREQITEM = 8 };
    string cmd1, cmd2;
    int skillToCheck;
    int minskill;
    int maxskill;
    int reqspell;
    RawItem reqitem[8]; //no more than 8 items
    int mana, stam, hit;

    
	MakeItem();
    bool checkReq(P_CHAR pc, bool inMenu = false);
};


MakeItem* getMakeItem(int n);*/
/*void execMake(P_CHAR pc, int n);
void showAddMenu (P_CHAR pc, int menu);
*/
#endif
