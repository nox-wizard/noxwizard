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
#include "addmenu.h"
#include "worldmain.h"



std::map< std::string, P_COMMAND > cCommandMap::command_map;  // Reference to static member




/*///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//Implementations of classes declared in cmds.h
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////*/


//Implementation of cCommand Class



cCommand::cCommand(std::string name, UI08 number ,std::string callback) {

	cmd_name=name;
	cmd_level=number; 
	cmd_callback=callback;
}



UI08 cCommand::getCommandLevel() {
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
	UI08 var2; 
      
         
	pFile = fopen ("small-scripts/commands.txt" , "r");		//Open commands.txt files

	if (pFile == NULL) return;
 
      
    while( !feof(pFile) ){  
      
		str[0]='\0';
		fgets (str , 80 , pFile);

		if ( (str[0]=='/') || (str[0]=='\n') || str[0]=='\r' || str[0]=='\0') continue;    //Ignore comments and blank lines
      
		if (str[strlen(str)-2]=='\r')
			
			str[strlen(str)-2]='\0';						//Delete eventually added \r
		
		if (str[strlen(str)-1]=='\n')
		
			str[strlen(str)-1]='\0';	                    //Delete final \n of the strings
	 
	 
		var1=strtok(str,","); 
		strupr(var1); 
		var2=atoi(strtok(0,","));      
		var3=strtok(0,","); 
		
		addGmCommand(var1,var2,var3);						//Put the command in command_map
     } 

}




P_COMMAND cCommandMap::addGmCommand(std::string name, UI08 priv, std::string callback) {

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





	 cCommandMap* commands = new cCommandMap(); // Creates the map.






/*///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//Main Function
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////*/



//This function is called after the control done in speech.cpp


void Command(NXWSOCKET  s, char* speech) // Client entered a command like 'ADD
	{
	
	NXWCLIENT client= getClientFromSocket(s);
		
		unsigned char *comm;
		unsigned char nonuni[512];
		char command_line[512];
		char param[30];
		int n=1;
		std::string par;

		//cmd_offset = 1;

		P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC( pc_currchar );

		strcpy((char*)nonuni, speech);
		strcpy((char*)tbuffer, (char*)nonuni);

		strupr((char*)nonuni);
		cline = (char*)&nonuni[0];
		splitline();

		if (tnum<1)	return;
		
		comm = nonuni + 1;  // Let's ignore the command prefix;

		P_COMMAND p_cmd= commands->findCommand((char*)comm,client);
		
		
		if(p_cmd==NULL) {
			client->sysmsg("Command %s doesn't exist!", (char*)comm);
			return;
		}
		
		//Control between cCommand privilege and cChar privilege.
		if( (p_cmd->getCommandLevel()) > pc_currchar->getPrivLevel()){
		client->sysmsg("You can't use this command! %d %d",p_cmd->getCommandLevel(),pc_currchar->getPrivLevel());
		return;	
		}
		
		pc_currchar->resetCommandParams();
		
		strcpy(command_line, speech);
		
		strlwr(command_line);
		
		char* pch;
  
		pch = strtok (command_line," ");
		pch = strtok (NULL, " ");
		
		while (pch != NULL && n<9)
		{
			if( strcmp( pch, "\0" )!=0 ){
				
				strcpy(param, pch);
				par=param;
				InfoOut("PARMA %s\n",(char*)par.c_str());
				pc_currchar->setCommandParams(n, par);
				n++;
			}
			
			pch = strtok (NULL, " ");
		}

		while(n<9){
			pc_currchar->setCommandParams(n, "_");
			n++;
		}
  		
		par=pc_currchar->getCommandParams(8);
		InfoOut("PARMA %s\n",(char*)par.c_str());
		p_cmd->call(pc_currchar); //Let's call the Small Function
   
		

	}
