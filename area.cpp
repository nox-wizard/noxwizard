  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"

cAreas*	Areas;


cAreas::cAreas() 
{
	this->allareas.clear();
	this->currarea=INVALID;
};

cAreas::~cAreas() 
{
}


SERIAL cAreas::insert( Area& newarea, SERIAL index )
{
	AREA_DB::iterator iter( this->allareas.find( index ) );
	if( iter==this->allareas.end() && index!=INVALID ) {
		if( currarea<=index )
			currarea=index;
		allareas[index]=newarea;
		return index;
	}
	else {
		currarea++;
		allareas[currarea]=newarea;
		return currarea;
	}
}


void cAreas::loadareas()
{

    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];
	int idxarea=0;
	
	int loopexit=0;
	do
	{
		safedelete(iter);
		SERIAL current=idxarea;
		iter = Scripts::Areas->getNewIterator("SECTION AREA %i", idxarea++);
		if( iter==NULL ) continue;

		Area area;
		UI16 check=0;

		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if		(!strcmp("X1", script1)) 		  
					{ area.x1 = str2num(script2); check|=0x000F; }
				else if (!strcmp("Y1", script1))		  
					{ area.y1 = str2num(script2); check|=0x00F0; }
				else if (!strcmp("X2", script1))		  
					{ area.x2 = str2num(script2); check|=0x0F00; }
				else if (!strcmp("Y2", script1))		  
					{ area.y2 = str2num(script2); check|=0xF000; }
				else ConOut("[ERROR] on parse of areas.xss" );
			}

		}
        while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

		if( check==0xFFFF )
			this->insert( area, current );
    }
	while ( (strcmp("EOF", script1)) && (++loopexit < MAXLOOPS) );

    safedelete(iter);

}
