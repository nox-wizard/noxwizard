  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


//file cmds.cpp
//by Frodo & Stonedz
//Work in progress...



#include "nxwcommn.h"
#include "network.h"
#include "cmds.h"
#include "sregions.h"
#include "bounty.h"
#include "sndpkg.h"
#include "debug.h"
#include "amx/amxcback.h"
#include "calendar.h"
#include "race.h"
#include "scp_parser.h"
#include "commands.h"
#include "addmenu.h"
#include "telport.h"
#include "accounts.h"
#include "worldmain.h"
#include "data.h"
#include "spawn.h"
#include "trade.h"
#include "chars.h"
#include "items.h"
#include "basics.h"
#include "inlines.h"
#include "nox-wizard.h"
#include "archive.h"
#include "map.h"
#include "jail.h"
#include "skills.h"
#include "layer.h"
#include "scripts.h"




/*///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//Implementations of classes declared in cmds.h
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////*/


//Implementation of cCommand Class

cCommand::cCommand(std::string name, SI08 number ,AmxFunction* callback) {

	cmd_name=name;
	cmd_number=number; 
	cmd_callback=callback;
}

//Implementation of cCallCommand Class

cCallCommand::cCallCommand (std::string par1, std::string par2, std::string par3, std::string par4, std::string par5, std::string par6, std::string par7, std::string par8){

	param1=par1;
	param2=par2;
	param3=par3;
	param4=par4;
	param5=par5;
	param6=par6;
	param7=par7;
	param8=par8;


}


cCallCommand::~cCallCommand() {

	
}


//Implementation of cCommandMap Class

cCommandMap::cCommandMap() {

	// all addGmCommand(...); goes here.

}


P_COMMAND cCommandMap::addGmCommand(std::string name, SI08 number, AmxFunction* callback) {

	P_COMMAND cmd= new cCommand(name, priv, number,callback);
    command_map[name]= cmd;
 	return cmd;
}



P_COMMAND cCommandMap::findCommand(std::string name) {
	
CMDMAP::iterator iter( command_map.find( "name" ) );

if ( iter != command_map.end() )	//command exists
    return iter->second.callback;
else
    return NULL;					//command doesnt exist
}



cCommandMap* commands = new cCommandMap();






/*///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//Todo's
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////*/


/*
*******
A function that controls if the char can do the specified command and
prepare a cCallCommand object to be given at Small function.
This function is called after the control in speech.cpp
*******

*/



//The function that is called after the control done in speech.cpp
//This should be put in another file or in a namespace (?)

void Command(NXWSOCKET  s, char* speech) // Client entred a '/' command like /ADD
	{
		unsigned char *comm;
		unsigned char nonuni[512];
		cCallCommand* command; 

		cmd_offset = 1;

		P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC( pc_currchar );

		strcpy((char*)nonuni, speech);
		strcpy((char*)tbuffer, (char*)nonuni);

		strupr((char*)nonuni);
		cline = (char*)&nonuni[0];
		splitline();

		if (tnum<1)	return;
		// Let's ignore the command prefix;
		comm = nonuni + 1;

		P_COMMAND cmd= commands->findCommand((char*)comm);
		NXWCLIENT client= getClientFromSocket(s);

		if(cmd==NULL) {
		client->sysmsg("Unrecognized command: %s", comm);
			return;
		}
		
		
		//Here there must be a control between cCommand privilege and cChar privilege.

		//Here there must be the construction of command's parameters that will be put
		//in cCallCommand object(If the char has the permission to xecute command).

		called_command=new cCallCommand(comm[1], comm[2], comm[3], comm[4); 
		
		//Here there must be the call to small function specified in cCommand.cmd_callback.

		delete called_command; 			
			  
			
			  //To be finished...
		

	}






/*

*******
Must complete a native function for AMX to get command property
for use it in small scripting.

*******


*/

static CP_PARAM=0;
static CP_ALLPARAMS=1;


/* Frodo:	must add the following function in AMX_NATIVE_INFO nxw_API[] 
			{ "getCmdProperty", _getCmdProperty } */
	// params[1] = char serial
	// params[2] = cCallCommand Serial
	// params[3] = property
	// params[4] = 1st param given
	// params[5] = 2nd param given
	// params[6] = 3rd param given
	// params[7] = 4th param given



NATIVE (_getCmdProperty) {		//this is only a copy of getCharProperty, waiting for list of properties

	if ( !params[2] )
	{
		switch( params[3] ) {
			
			case CP_PARAM: {
				}
			case CP_ALLPARAMS: {
				}
		}

  	}
  	return INVALID;
}

