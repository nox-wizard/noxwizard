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
\brief Declaration of CWorldMain class
*/

#ifndef __WORLDMAIN_H_
#define __WORLDMAIN_H_

class CWorldMain  
{
private:

	FILE* iWsc;
	FILE* cWsc;
	FILE* jWsc;
	FILE* gWsc;

	bool isSaving;

	UI32 itm_curr, chr_curr;

	void SaveChar( P_CHAR pc );
	void loadChar();

	void SaveItem( P_ITEM pi );
	void loadItem();

	void realworldsave();

	void loadGuilds();
	
	void loadjailed();
	void loadPrison();
	void savePrison();

public:

	CWorldMain();
	~CWorldMain();

	void loadNewWorld();
	void saveNewWorld();

	bool Saving();
};

void fprintWstring( FILE* f, char* name, wstring c );
wstring HexVector2UnicodeString( char* s );

extern class CWorldMain* cwmWorldState;


#endif // WORLDMAIN
