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


#include "cmds.h"



//Implementation of cCommand Class

cCommand::cCommand(std::string name, SI32 priv, AmxFunction* callback()) {

	cmd_name=name;
	cmd_priv=priv; //stonedz: should be a std::bitset (?)
	callback();
}




//Implementation of cCommandMap Class

cCommandMap::cCommandMap() {

	// all addGmCommand(...); here

}


P_COMMAND cCommandMap::addGmCommand(std::string name, SI32 priv, AmxFunction* callback()) {

	P_COMMAND cmd= new cCommand(name, priv, callback());
    command_map[name]= cmd;
 	return cmd;
}



/*	Frodo: probably useless (not me, this function :P)

P_COMMAND cCommandMap::addGmCommand(P_COMMAND cmd) {
 	
	P_COMMAND old= command_map[cmd->cmd_name];
    command_map[cmd->cmd_name]= cmd;
 	return old;
} */



P_COMMAND cCommandMap::findCommand(std::string name) {
	
CMDMAP::iterator iter( command_map.find( "name" ) );

if ( iter != command_map.end() )	//command exists
    return iter->second.callback;
else
    return NULL;					//command doesnt exist
}






/*


*****
Must add a native function for AMX to get command property
for use it in small scripting.
Something like :

\param 1 char serial
\param 2 command serial
\param 3 property
\param ...  

  NATIVE (_getCmdProperty){
	...
  }
*****

*/



