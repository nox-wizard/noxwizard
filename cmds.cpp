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



//The function that is called after the control done in speech.cpp
//This should be put in another file or in a namespace (?)

void Command(NXWSOCKET  s, char* speech) // Client entred a '/' command like /ADD
	{
		unsigned char *comm;
		unsigned char nonuni[512];

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
		if(cmd->notValid(pc_currchar)) {
			client->sysmsg("Access denied.");
			return;
		}

		switch(cmd->cmd_type) {
		// Single step commands
			case CMD_FUNC:
				(*((CMD_EXEC)cmd->cmd_extra)) (client);
				break;
			case CMD_TARGET: {
				P_TARGET targ = clientInfo[s]->newTarget( createTarget( ((target_st*)cmd->cmd_extra)->type ) );
				targ->code_callback=((target_st*)cmd->cmd_extra)->func;
				targ->send( client );
				client->sysmsg( ((target_st*)cmd->cmd_extra)->msg );
				break;
			}
			case CMD_TARGETN:
				if(tnum==2) {
					P_TARGET targ = clientInfo[s]->newTarget( createTarget( ((target_st*)cmd->cmd_extra)->type ) );
					targ->code_callback=((target_st*)cmd->cmd_extra)->func;
					targ->buffer[0]=strtonum(1);
					targ->send( client );
					client->sysmsg( ((target_st*)cmd->cmd_extra)->msg );
				} 
				else 
					client->sysmsg("This command takes one number as an argument.");
				break;
			case CMD_TARGETNNN:
				if(tnum==4) {
					P_TARGET targ = clientInfo[s]->newTarget( createTarget( ((target_st*)cmd->cmd_extra)->type ) );
					targ->code_callback=((target_st*)cmd->cmd_extra)->func;
					targ->buffer[0]=strtonum(1);
					targ->buffer[1]=strtonum(2);
					targ->buffer[2]=strtonum(3);
					targ->send( client );
					client->sysmsg( ((target_st*)cmd->cmd_extra)->msg );
				} 
				else
					sysmessage(s, "This command takes three numbers as arguments.");
				break;
			case CMD_TARGETS:
				if(tnum==2) {
					P_TARGET targ = clientInfo[s]->newTarget( createTarget( ((target_st*)cmd->cmd_extra)->type ) );
					targ->code_callback=((target_st*)cmd->cmd_extra)->func;
					targ->buffer_str[0]=&tbuffer[Commands::cmd_offset+strlen(cmd->cmd_name)];
					targ->send( client );
					client->sysmsg( ((target_st*)cmd->cmd_extra)->msg );
				} 
				else
					sysmessage(s, "This command takes a string as arguments.");
				break;
			case CMD_MANAGEDCMD:
				client->startCommand(cmd, speech);
				break;
			default:
				client->sysmsg("INTERNAL ERROR: Command has a bad command type set!");
				break;
		}

	}




/*///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//Implementations of classes declared in cmds.h
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////*/


//Implementation of cCommand Class

cCommand::cCommand(std::string name, SI32 priv, AmxFunction* callback) {

	cmd_name=name;
	cmd_priv=priv; //stonedz: should be a std::bitset (?)
	cmd_callback=callback;
}




//Implementation of cCommandMap Class

cCommandMap::cCommandMap() {

	// all addGmCommand(...); goes here.

}


P_COMMAND cCommandMap::addGmCommand(std::string name, SI32 priv, AmxFunction* callback) {

	P_COMMAND cmd= new cCommand(name, priv, callback);
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



cCommandMap* commands = new cCommandMap();




/*///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//Todo's
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////*/


/*

*******
A function to construct a cCallCommand object and call the small function
for the command
*******


void command_execute( NXWCLIENT ps, P_COMMAND cmd , cCallCommand* params )
{
 ...

}

*/





/*


*******
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
*******

*/



/* Frodo:	must add the following function in AMX_NATIVE_INFO nxw_API[] 
			{ "getCmdProperty", _getCmdProperty }
*/

NATIVE (_getCmdProperty) {		//this is only a copy of getCharProperty, waiting for list of properties

	P_CHAR pc = pointers::findCharBySerial(params[1]);

	if ( ISVALIDPC( pc ) )
	{
		VAR_TYPE tp = getPropertyType( params[2] );
		switch( tp ) {
			case T_INT: {
				int p = getCharIntProperty( pc, params[2], params[3], params[4]);
				cell i = p;
				return i;
			}
			case T_BOOL: {
				bool p = getCharBoolProperty( pc, params[2], params[3]);
				cell i = p;
				return i;
			}
			case T_SHORT: {
				short p = getCharShortProperty( pc, params[2], params[3]);
				cell i = p;
				return i;
			}
			case T_CHAR: {
				char p = getCharCharProperty( pc, params[2], params[3]);
				cell i = p;
				return i;
			}
			case T_STRING: {
				//we're here so we should pass a string, params[4] is a str ptr
	  			char str[100];	
	  			strcpy(str, getCharStrProperty( pc, params[2], params[3] ) );

	  			cell *cptr;
  				amx_GetAddr(amx,params[4],&cptr);
	  			amx_SetString(cptr,str, g_nStringMode);

  				return strlen(str);
			}
			case T_UNICODE: {
				wstring& w=getCharUniProperty( pc, params[2], params[3] );

				cell *cptr;
	  			amx_GetAddr(amx,params[4],&cptr);
				amx_SetStringUnicode(cptr, w );

				return w.length();
			}
		}

  	}
  	return INVALID;
}

