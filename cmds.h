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

#ifndef __CMDS_H
#define __CMDS_H

#include "common_libs.h"
#include "nxwcommn.h"		//for std classes


enum PrivLevel
{
	PRIVLEVEL_ADMIN = 255,
	PRIVLEVEL_GM	= 200,
	PRIVLEVEL_SEER	= 150,
	PRIVLEVEL_CNS	= 100,
	PRIVLEVEL_PLAYER = 50,
	PRIVLEVEL_GUEST = 0,
};


typedef class cCommand* P_COMMAND;

/*
\brief Declaration of cCommand Class
*/

class cCommand {

	private:
	
		std::string cmd_name;
		SI08 cmd_level;  
		AmxFunction* cmd_callback;

	public:

		cCommand( std::string& cmd_name, SI08 cmd_number, AmxFunction* callback );
		SI08 getCommandLevel(P_COMMAND cmd);
		void call( std::string params );

};




/*
\brief Declaration of cCallCommand Class
 cCallCommand object is created every time a command is executed and goes in CALLCMDMAP. 
 it is destroyed when the command has finished
 All parameters given by the char who call the command goes in this obj.
*/
class cCallCommand 
{

private:

	static SERIAL current_serial;
	std::map< SERIAL, cCallCommand* > callcommand_map;

public:

	cCallCommand(std::string all_params);
	~cCallCommand();
	std::string all_params;
	cCallCommand* findCallCommand(SERIAL cmd);
	SERIAL addCallCommand(cCallCommand* called_command);	
	void delCommand(SERIAL cmd);
	//std::vector< string >* single_param;

};
 



/*
\brief Declaration of cCommandMap Class
*/
class cCommandMap {

private:

	static std::map< std::string, P_COMMAND > command_map; //!< all commands
public:
	
	cCommandMap();
	P_COMMAND addGmCommand(std::string name, SI08 number ,AmxFunction* callback);
	static bool Check(string& text);
	P_COMMAND findCommand(std::string name);

};



#endif

