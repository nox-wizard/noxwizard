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




void Command( NXWSOCKET , char * );



/*
\brief Declaration of cCommand Class
*/


class cCommand {

	private:

	    std::string cmd_name;
		SI08 cmd_level;  
		std::string cmd_callback;

	public:

		cCommand( std::string cmd_name, SI08 cmd_number, std::string callback );
		SI08 getCommandLevel();
		void call( NXWSOCKET s, P_CHAR current );

};




/*
\brief Declaration of cCommandMap Class
*/


class cCommandMap {

private:

	static std::map< std::string, P_COMMAND > command_map; //!< all commands

public:
	
	cCommandMap();
	P_COMMAND addGmCommand(std::string name, SI08 number ,std::string callback);
	P_COMMAND findCommand(char* name,NXWCLIENT client);

};




#endif

