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



cCommand::cCommand(std::string name, SI08 number ,std::string callback) {

	cmd_name=name;
	cmd_level=number; 
	cmd_callback=callback;
}



SI08 cCommand::getCommandLevel() {
	return this->cmd_level;
}



void cCommand::call(P_CHAR curr_char){

	char* callback = (char*)(this->cmd_callback).c_str();
	
	
	AmxFunction* CommandFunction = NULL;      
		
		if(CommandFunction == NULL ) {
	
		
	
	CommandFunction = new AmxFunction( callback ); 
		
		}
		else
			
			return;

		CommandFunction->Call( curr_char->getSerial32() ); 
}



//Implementation of cCommandMap Class



cCommandMap::cCommandMap() { 
 
	FILE * pFile;
	char str [150]; 
	std::string var1, var3;
	SI08 var2; 
      
   //ifstream in; 
      
   pFile = fopen ("small-scripts/commands.txt" , "r");

   if (pFile == NULL) return;
 
      
    while( !feof(pFile) ){  
      
     //in >> str ; 
     fgets (str , 80 , pFile);

	 if ( (str[0]=='/') || (str[0]=='\n') ) continue;
      
	if (str[strlen(str)-1]=='\n')
			str[strlen(str)-1]='\0';	
	 
	 
	 var1=strtok(str,","); 
		strupr(var1); 
		var2=atoi(strtok(0,","));      
		var3=strtok(0,","); 
		
		addGmCommand(var1,var2,var3); 
     } 

}




P_COMMAND cCommandMap::addGmCommand(std::string name, SI08 priv, std::string callback) {

	P_COMMAND cmd= new cCommand(name, priv, callback);
    command_map.insert(make_pair(name,cmd));
 	return cmd;
}





P_COMMAND cCommandMap::findCommand(char* name,NXWCLIENT client){


	std::map< std::string, P_COMMAND >::iterator iter;

	iter=command_map.find( name );


	if ( iter != command_map.end() )	//command exists
		return iter->second;
	else
		return NULL;					//command doesnt exist

}




cCommandMap* commands = new cCommandMap();





/*///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//Main Function
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////*/



//This function is called after the control done in speech.cpp


void Command(NXWSOCKET  s, char* speech) // Client entred a command like 'ADD
	{
	
	NXWCLIENT client= getClientFromSocket(s);
		
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

		P_COMMAND p_cmd= commands->findCommand((char*)comm,client);
		
		
		if(p_cmd==NULL) {
			client->sysmsg("Command %s doesn't exist!", (char*)comm);
			return;
		}
		
		
		//Control between cCommand privilege and cChar privilege.

		if( (p_cmd->getCommandLevel()) > pc_currchar->commandLevel){
		client->sysmsg("You can use this command!");
		return;	
		}
		
		
		//Let's call the Small Function
		
		p_cmd->call(pc_currchar);
   
		

	}
