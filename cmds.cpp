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

cCommand::cCommand(std::string& name, SI08 number ,AmxFunction* callback) {

	cmd_name=name;
	cmd_level=number; 
	cmd_callback=callback;
}


SI08 cCommand::getCommandLevel(P_COMMAND cmd) {
	return cmd->cmd_level;
}



//Implementation of cCallCommand Class

SERIAL cCallCommand::current_serial = 0;

cCallCommand::cCallCommand( SERIAL cmd_serial, std::string params, std::vector<string>* param ){

	cmd_serial=++current_serial;
	all_params=params;
	single_param=param;
}


cCallCommand::~cCallCommand() {
}




//Implementation of cCommandMap Class

cCommandMap::cCommandMap() {

	// all addGmCommand(...); goes here.

}


P_COMMAND cCommandMap::addGmCommand(std::string name, SI08 priv, AmxFunction* callback) {

	P_COMMAND cmd= new cCommand(name, priv, callback);
    command_map[name]= cmd;
 	return cmd;
}




bool cCommandMap::Check( string& text ){

	std::map< std::string, P_COMMAND >::iterator iter( command_map.find( text ) );

	if ( iter == command_map.end() )	//command not exists
		return false;

	//other checks..

	return true;
}




P_COMMAND cCommandMap::findCommand(std::string name){

		
	std::map< std::string, P_COMMAND >::iterator iter( command_map.find( "name" ) );

	if ( iter != command_map.end() )	//command exists
		return iter->second;
	else
		return NULL;					//command doesnt exist
}





//Implementation of cCallCommandMap Class


cCallCommand* cCallCommand::findCallCommand(SERIAL cmd){

	std::map< SERIAL, cCallCommand* >::iterator iter( callcommand_map.find( cmd ) );

	if ( iter != callcommand_map.end() )	//command exists
		return iter->second;
	else
		return NULL;					//command doesnt exist
}
	


void cCallCommand::delCommand(SERIAL cmd){

	callcommand_map.erase(cmd);
}



cCommandMap* commands = new cCommandMap();





/*///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//Todo's
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////*/




//The function that is called after the control done in speech.cpp


void Command(NXWSOCKET  s, char* speech) // Client entred a command like 'ADD
	{
		unsigned char *comm;
		unsigned char nonuni[512];
		//cCallCommand* command; 

		//cmd_offset = 1;

		//cCallCommandMap* callcommands = new cCallCommandMap()
		
		
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

		P_COMMAND p_cmd= commands->findCommand((char*)comm);
		
		
		NXWCLIENT client= getClientFromSocket(s);

		

		if(p_cmd==NULL) {
			return;
		}
		
		
		//Control between cCommand privilege and cChar privilege.

		if(p_cmd->getCommandLevel(p_cmd)==pc_currchar->commandLevel){
		client->sysmsg("You can't use this command!");
			return;
		}

		
		//Here there must be the call to small function specified in cCommand.cmd_callback.
		//callcommands->remCallCommand(*serial*);		
			  
			
			  //To be finished...
		

	}







/*******
Must complete a native function for AMX to get command property
for use it in small scripting.

*******/



/*


static CP_PARAM=0;
static CP_ALLPARAMS=1;


//Frodo:	must add the following function in AMX_NATIVE_INFO nxw_API[] 
//			{ "getCmdProperty", _getCmdProperty } 
	
	// params[1] = cCallCommand Serial
	// params[2] = property
	// params[3] = 1st param given
	// params[4] = 2nd param given
	// params[5] = 3rd param given
	// params[6] = 4th param given



NATIVE (_getCmdProperty) {		//this is only a copy of getCharProperty, waiting for list of properties

	if (cCallCommand->findCallCommand(param[1])==NULL)
		return NULL;
	  
		
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
*/
