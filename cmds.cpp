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

// Reference to static member
std::map< std::string, P_COMMAND > cCommandMap::command_map;

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


AmxFunction* cCommand::getCommandCallback(P_COMMAND cmd) {
	return cmd->cmd_callback;
}


/*



//Implementation of cCallCommand Class

SERIAL cCallCommand::current_serial = 0;



cCallCommand::cCallCommand(std::string params){

	all_params=params;
//	single_param=param;
}


cCallCommand::~cCallCommand() {
}


*/

//Implementation of cCommandMap Class

cCommandMap::cCommandMap() {

	// all addGmCommand(...); goes here.

}


P_COMMAND cCommandMap::addGmCommand(std::string name, SI08 priv, AmxFunction* callback) {

	P_COMMAND cmd= new cCommand(name, priv, callback);
    command_map[name]= cmd;
 	return cmd;
}


/*

bool cCommandMap::Check( string& text ){
	std::map< std::string, P_COMMAND >::iterator iter( command_map.find( text ) );

	if ( iter == command_map.end() )	//command not exists
		return false;

	//other checks..
	return true;
}

*/


P_COMMAND cCommandMap::findCommand(std::string name){

	std::map< std::string, P_COMMAND >::iterator iter( command_map.find( "name" ) );

	if ( iter != command_map.end() )	//command exists
		return iter->second;
	else
		return NULL;					//command doesnt exist
}


/*


//Implementation of cCallCommandMap Class


cCallCommand* cCallCommand::findCallCommand(SERIAL cmd){

	std::map< SERIAL, cCallCommand* >::iterator iter( callcommand_map.find( cmd ) );

	if ( iter != callcommand_map.end() )	//command exists
		return iter->second;
	else
		return NULL;					//command doesnt exist
}




SERIAL cCallCommand::addCallCommand(cCallCommand* called){

	callcommand_map[++current_serial]=called;
	return current_serial;

}



void cCallCommand::delCommand(SERIAL cmd){

	callcommand_map.erase(cmd);
}


*/
cCommandMap* commands = new cCommandMap();





/*///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//Main Function
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////*/



//The function that is called after the control done in speech.cpp


void Command(NXWSOCKET  s, char* speech) // Client entred a command like 'ADD
	{
	
	NXWCLIENT client= getClientFromSocket(s);
	//client->sysmsg("No Commands Avaible. Works in Progress, sorry :( !");
	
		unsigned char *comm;
		unsigned char nonuni[512];

		//cmd_offset = 1;

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
		
		
		if(p_cmd==NULL) {
			client->sysmsg("Command %s doesn't exist!", (char*)comm);
			return;
		}
		
		
		//Control between cCommand privilege and cChar privilege.

		if( (p_cmd->getCommandLevel(p_cmd)) > (pc_currchar->commandLevel) ){
		client->sysmsg("You can't use this command!");
			return;
		}

		
		// cCallCommand* called= new cCallCommand(speech);
    

		//SERIAL cmd_serial=called->addCallCommand(called);

		AmxFunction* CommandFunction = NULL;      
		if(CommandFunction == NULL ) 
			CommandFunction = new AmxFunction("p_cmd->getCommandCallback"); 
		CommandFunction->Call( pc_currchar->getSerial32() ); 
		
		
		//Let's delete the temp object		
			  
		//called->delCommand(cmd_serial);     
		

	}
