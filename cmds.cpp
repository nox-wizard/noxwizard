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


#include "cmds.h"



//Implementation of cCommand Class

cCommand::cCommand(char* name, long int priv) {

	this->cmd_name=name;
	this->cmd_priv=priv;
}




//Implementation of cCommandMap Class


P_COMMAND cCommandMap::addGmCommand(char* name, int long priv, void callback()) {

	P_COMMAND cmd= new cCommand(name, priv, callback());
	P_COMMAND old= command_map[name];
    command_map[name]= cmd;
 	return old;
}



P_COMMAND cCommandMap::addGmCommand(P_COMMAND cmd) {
 	
	P_COMMAND old= command_map[cmd->cmd_name];
    command_map[cmd->cmd_name]= cmd;
 	return old;
}



P_COMMAND cCommandMap::findCommand(char* name) {
    return command_map[name];
}



td_cmditer cCommandMap::getIteratorBegin() {			// Not sure if necessary 
    return command_map.begin();
}


td_cmditer cCommandMap::getIteratorEnd() {			// Not sure if necessary
    return command_map.end();
}








