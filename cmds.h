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


typedef cCommand* P_COMMAND;

typedef std::map< string, P_COMMAND> td_cmdmap;
typedef td_cmdmap::iterator td_cmditer;



//brief Declaration of cCommand Class


class cCommand {

public:

	cCommand(char* cmd_name, long int cmd_priv, void callback());
	
	void (*cmd_extra) ();
    char* cmd_name;
	long int cmd_priv;  //stonedz: this should be a std::bitset (?)
};



//brief Declaration of cCommandMap Class


class cCommandMap {

public:
	
	cCommandMap();
	P_COMMAND addGmCommand(char* name, int long priv, void callback());
	P_COMMAND addGmCommand(P_COMMAND cmd);
	P_COMMAND findCommand(char* name);
	
	td_cmditer getIteratorBegin();
	td_cmditer getIteratorEnd();

private:

	td_cmdmap command_map;
};
