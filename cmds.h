  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


//file cmds.h
//by Frodo & Stonedz
//Work in progress...


//#include <bitset>
//#include "common_libs.h"
#include "nxwcommn.h"		//for std Class

class cCommand;

typedef cCommand* P_COMMAND;

typedef std::map<string, P_COMMAND> CMDMAP;



//brief Declaration of cCommand Class


class cCommand {

public:

	cCommand(std::string cmd_name, SI32 cmd_priv, AmxFunction* callback());
	
	std::string cmd_name;
	SI32 cmd_priv;  //stonedz: this should be a std::bitset (?) //frodo: bitset later defined in cChar
	AmxFunction* callback();
};



//brief Declaration of cCommandMap Class


class cCommandMap {

public:
	
	cCommandMap();
	P_COMMAND addGmCommand(std::string name, SI32 priv, AmxFunction* callback());
	P_COMMAND findCommand(std::string name);
	

private:

	CMDMAP command_map;
};
