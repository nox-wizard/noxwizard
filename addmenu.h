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


struct RawItem {
    int id;
    int color;
    UI32 number;
    void parse(char *sz);
};

/*!
\brief Class MakeItem
*/
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


MakeItem* getMakeItem(int n);
void execMake(P_CHAR pc, int n);




#endif

