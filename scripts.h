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
\brief Script related functions
*/

#ifndef __SCRIPTS_H__
#define __SCRIPTS_H__

void splitLine( char* source, char *head, char *tail );
void splitLine( const std::string& source, std::string& head, std::string& tail );
void gettokennum(std::string s, int num);
void readFullLine ();
int getRangedValue(char *stringguy);
int getRangedValue( std::string str );
void readw2();
void readw3();

namespace Scripts {
	extern class cScpScript* Advance;
	extern class cScpScript* Calendar;
	extern class cScpScript* Carve;
	extern class cScpScript* Colors;
	extern class cScpScript* Create;
	extern class cScpScript* CronTab;
	extern class cScpScript* Envoke;
	extern class cScpScript* Fishing;
	extern class cScpScript* HardItems;
	extern class cScpScript* House;
	extern class cScpScript* Creatures;
	extern class cScpScript* HostDeny;
	extern class cScpScript* HtmlStrm;
	extern class cScpScript* Items;
	extern class cScpScript* Location;
	extern class cScpScript* MenuPriv;
	extern class cScpScript* Menus;
	extern class cScpScript* MetaGM;
	extern class cScpScript* Misc;
	extern class cScpScript* MList;
	extern class cScpScript* MsgBoard;
	extern class cScpScript* Necro;
	extern class cScpScript* Newbie;
	extern class cScpScript* Npc;
	extern class cScpScript* NpcMagic;
	extern class cScpScript* Override;
	extern class cScpScript* Polymorph;
	extern class cScpScript* Regions;
	extern class cScpScript* Skills;
	extern class cScpScript* Spawn;
	extern class cScpScript* Speech;
	extern class cScpScript* Spells;
	extern class cScpScript* Teleport;
	extern class cScpScript* Titles;
	extern class cScpScript* Triggers;
	extern class cScpScript* WTrigrs;
	extern class cScpScript* Mountable;
	extern class cScpScript* WeaponInfo;
	extern class cScpScript* Containers;
	extern class cScpScript* Areas;
};


#endif
