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

