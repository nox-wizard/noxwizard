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

	cCommand(std::string cmd_name, SI32 cmd_priv, SI08 params_number ,AmxFunction* callback);
	
	std::string cmd_name;
	SI32 cmd_priv;  //stonedz: this should be a std::bitset (?) //frodo: bitset later defined in cChar
	AmxFunction* cmd_callback;
	SI08 cmd_params_number ; //Max number of parameters the command should have.
							 //There is a problem if a command uses more than one 
							 //multi-word parameter (e.g. 'COMMAND param one param two).
};



//brief Declaration of cCallCommand Class
//a cCallCommand object is created every time a command is executed. 
//and destroyed whn the command has finished
//All parameters given by the char who call the command goes here.
//stonedz :Maybe, if Small supports it, we can use an array dynamically allocated.
 


class cCallCommand : public cCommand {

public:

	cCallCommand(/*...*/);
	~cCallCommand();

	string param1;
	string param2;
	/* .... */
	string paramN;
	//N is the Max number of parameters any command in nox could have

}
 


//brief Declaration of cCommandMap Class


class cCommandMap {

public:
	
	cCommandMap();
	P_COMMAND addGmCommand(std::string name, SI32 priv,SI08 params_number ,AmxFunction* callback);
	P_COMMAND findCommand(std::string name);
	

private:

	CMDMAP command_map;
};
