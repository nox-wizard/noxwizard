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

class cCallCommand;



typedef cCommand* P_COMMAND;

typedef cCallCommand* P_CALLCOMMAND;

typedef std::map<string, P_COMMAND> CMDMAP;

typedef std::map<SERIAL, P_CALLCOMMAND> CALLCMDMAP;



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
//a cCallCommand object is created every time a command is executed and goes in CALLCMDMAP. 
//it is destroyed when the command has finished
//All parameters given by the char who call the command goes in this obj.
 

class cCallCommand {

public:

	cCallCommand(std::string par1=NULL, std::string par2=NULL, std::string par3=NULL, std::string par4=NULL, std::string par5=NULL, std::string par6=NULL, std::string par7=NULL, std::string par8=NULL);

	std::string param1;
	std::string param2;
	std::string param3;
	std::string param4;
	std::string param5;
	std::string param6;
	std::string param7;
	std::string param8;


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



//brief Declaration of cCallCommandMap Class


class cCallCommandMap {

public:
	
	void addCallCommand(SERIAL cmdserial, std::string par1=NULL, std::string par2=NULL, std::string par3=NULL, std::string par4=NULL, std::string par5=NULL, std::string par6=NULL, std::string par7=NULL, std::string par8=NULL);
	P_CALLCOMMAND remCallCommand(SERIAL cmdserial);
	

private:

	CALLCMDMAP callCommand_map;
};
