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

cCallCommand::cCallCommand (SERIAL cmdSerial){

	SERIAL ++cmdSerial;
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

		called_command=new cCallCommand(); 
		
		//Here there must be the call to small function specified in cCommand.cmd_callback.

		delete called_command; 			
			  
			
			  //To be finished...
		

	}






/*

*******
Must add a native function for AMX to get command property
for use it in small scripting.
Something like :

\param 1 char serial
\param 2 cCallCommand serial
\param ...  

  NATIVE (_getCmdProperty){
	...
  }
*******

*/



/* Frodo:	must add the following function in AMX_NATIVE_INFO nxw_API[] 
			{ "getCmdProperty", _getCmdProperty }


NATIVE (_getCmdProperty) {		//this is only a copy of getCharProperty, waiting for list of properties

	if ( !params[1] )
	{
		switch( params[2] ) {
			case T_INT: {
				int p = getCharIntProperty( pc, params[2], params[3], params[4]);
				cell i = p;
				return i;
			
		}

  	}
  	return INVALID;
}

*/