  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "area.h"
#include "scp_parser.h"
#include "globals.h"
#include "basics.h"
#include "inlines.h"
#include "scripts.h"


cAreas*	Areas=NULL;


cAreas::cAreas() 
{
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

	cScpIterator*	iter = 0;
	std::string	rha,
			lha;
	int idxarea=0;

	int loopexit=0;
	do
	{
		safedelete(iter);
		SERIAL current=idxarea;
		iter = Scripts::Areas->getNewIterator("SECTION AREA %i", idxarea++);
		if( iter )
		{

			Area area;
			UI16 check=0;

			do
			{
				iter->parseLine( lha, rha );
				if ( lha[0] != '}' && lha[0] !='{' )
				{
					if	( lha == "X1")
					{
						area.x1 = str2num( rha );
						check|=0x000F;
					}
					else if ( lha == "Y1" )
					{
						area.y1 = str2num( rha );
						check|=0x00F0;
					}
					else if ( lha == "X2" )
					{
						area.x2 = str2num( rha );
						check|=0x0F00;
					}
					else if ( lha == "Y2" )
					{
						area.y2 = str2num( rha );
						check|=0xF000;
					}
					else
						WarnOut("[ERROR] on parse of areas.xss" );
				}
			}
			while ( lha[0] !='}' && ++loopexit < MAXLOOPS );

			if( check==0xFFFF )
				this->insert( area, current );
		}
    }
	while (  lha != "EOF" && ++loopexit < MAXLOOPS );

    safedelete(iter);

}
