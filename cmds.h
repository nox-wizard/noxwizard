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

//#ifndef __CMDS_H
//#define __CMDS_H

#include "common_libs.h"
#include "nxwcommn.h"		//for std classes

class cCommand;

typedef cCommand* P_COMMAND;

typedef std::map<string, P_COMMAND> CMDMAP;

static SERIAL CmdNextSerial=0;

//brief Declaration of cCommand Class


class cCommand {

public:

	cCommand(std::string cmd_name, SI08 cmd_number, AmxFunction* callback);
	
	std::string cmd_name;
	SI08 cmd_number;  
	AmxFunction* cmd_callback;

};



//brief Declaration of cCallCommand Class
//a cCallCommand object is created every time a command is executed. 
//and destroyed when the command has finished
//All parameters given by the char who call the command goes here.

//stonedz :Maybe, if Small supports it, we can use an array dynamically allocated(?).
 

class cCallCommand {

public:

	cCallCommand(SERIAL cmdSerial);

	SERIAL CmdNextSerial;

}
 


//brief Declaration of cCommandMap Class


class cCommandMap {

public:
	
	cCommandMap();
	P_COMMAND addGmCommand(std::string name, SI08 number ,AmxFunction* callback);
	P_COMMAND findCommand(std::string name);
	

private:

	CMDMAP command_map;
};
