  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


//file gmpages.h
//by Frodo & Stonedz

#ifndef __GMPAGES_H
#define __GMPAGES_H

#include "common_libs.h"
#include "nxwcommn.h"

typedef class cGmpage* P_GMPAGE;




class cGmpage{


private:

	SERIAL serial;
	std::string reason;
	std::string timeofcall;
	P_GMPAGE next_page;	//next page of the same char
	P_GMPAGE prev_page;	//previous page of the same char 
	UI08 page_number;		//page's number 

public:

	cGmpage(SERIAL serialg, std::string reasong);
	
	std::string getReason();
	std::string getTime();
	UI08 getPageNumber ();
	P_GMPAGE getNextPage ();
	P_GMPAGE getPrevPage ();
	SERIAL getSerial ();
	UI08 setPageNumber (UI08 new_page_number);
	P_GMPAGE setNextPage (P_GMPAGE new_next_page);
	P_GMPAGE setPrevPage (P_GMPAGE new_prev_page);



};


class cGmpagesMap{

private:

    static std::map< SERIAL, P_GMPAGE > gmpages_map; // SERIAL of the player. Contains all pages.

public:
	
	cGmpagesMap();
	P_GMPAGE addPage(P_GMPAGE page);
	P_GMPAGE findPage(SERIAL serial);
	P_GMPAGE findPage(SERIAL serial, UI08 page_num);
	bool deletePage(SERIAL serial, UI08 page_num);
	SERIAL_VECTOR getAllPagers();
	bool optimize_page_indexes(SERIAL serial);
	

};

#endif
