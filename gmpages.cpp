  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

//compilable but not yet fully implemented
//file gmpages.cpp
//by Frodo & Stonedz


#include "gmpages.h"
#include "basics.h"
#include "typedefs.h"


std::map< SERIAL, P_GMPAGE > cGmpagesMap::gmpages_map;


cGmpage::cGmpage(SERIAL serialg, std::string reasong) {

	serial=serialg;
	reason=reasong;
	page_number=1;
	next_page=NULL;
	prev_page=NULL;
	
	
	time_t current_time = time(0);
	struct tm *local = localtime(&current_time);
	sprintf( (char *)timeofcall.c_str(), "%02d/%02d/%04d at %02d:%02d:%02d", local->tm_mday, local->tm_mon, local->tm_year, local->tm_hour, local->tm_min, local->tm_sec);
	
}


std::string cGmpage::getReason(){

	return reason;
}

std::string cGmpage::getTime(){

	return timeofcall;

}



UI08 cGmpage::getPageNumber (){
	
	return page_number;
}

P_GMPAGE cGmpage::getNextPage (){

	return next_page;
}

P_GMPAGE cGmpage::getPrevPage (){

	return prev_page;
}

SERIAL cGmpage::getSerial (){

	return serial;
}

UI08 cGmpage::setPageNumber (UI08 new_page_number){

	page_number=new_page_number;
	return page_number;
}

P_GMPAGE cGmpage::setNextPage (P_GMPAGE new_next_page){

	next_page=new_next_page;
	return next_page;
}

P_GMPAGE cGmpage::setPrevPage (P_GMPAGE new_prev_page){

	prev_page=new_prev_page;
	return prev_page;
}


cGmpagesMap::cGmpagesMap() {


}

P_GMPAGE cGmpagesMap::addPage(P_GMPAGE page){

		
		P_GMPAGE page1 = NULL;
		
		if (findPage(page->getSerial())==NULL)		//first page for a player
		{
			gmpages_map.insert(make_pair(page->getSerial(),page));
			return page;
		}
		
		else							//appends further pages
		{
			page1=findPage(page->getSerial());		

			while(page1->getNextPage() != NULL)
				page1=page1->getNextPage();
			
			if (page1->getPageNumber() < 4) // allows a maximum of 3 pages for a single player (first page_number is 1)
				{
					page->setPrevPage(page1);
					page->setPageNumber(page1->getPageNumber() + 1);
					page1->setNextPage(page);
					return page;
				}

			else return NULL; //no page can be added, first a gm should solve one.
			
		}

	

}

bool cGmpagesMap::deletePage(SERIAL serial,UI08 page_num){
	
	P_GMPAGE page_del = NULL;
	
	page_del=findPage(serial, page_num);
	
	if ( page_del==NULL)
		return false;
		
		if ( page_del->getNextPage() == NULL && page_del->getPageNumber()==1)	//only one page for the given player -> erase map entry
		{
			gmpages_map.erase(serial);
			return true;
		}
		else 
		{
		
			page_del->getPrevPage()->setNextPage(page_del->getNextPage());
		
			if (page_del->getNextPage() != NULL )
				page_del->getNextPage()->setPrevPage(page_del->getPrevPage());

			delete page_del;
			optimize_page_indexes(serial);
			return true;
		
		}
	
}


P_GMPAGE cGmpagesMap::findPage(SERIAL serial, UI08 page_num){ //finds a specified page for a char

	std::map<SERIAL, P_GMPAGE>::iterator iter;
	P_GMPAGE page = NULL;

	iter=gmpages_map.find( serial );

	if(iter!=gmpages_map.end())
		
		page=iter->second;
	else
		return NULL;
	
		while ( page->getPageNumber() != page_num && page->getNextPage() != NULL )
			page->setNextPage(page->getNextPage());
		
		if (page->getPageNumber() == page_num)
			return page;
		else 
			return NULL;

	

}



P_GMPAGE cGmpagesMap::findPage(SERIAL serial){

	std::map< SERIAL, P_GMPAGE >::iterator iter;

	iter=gmpages_map.find( serial );
	
	if ( iter != gmpages_map.end() )
		return iter->second;	//first page for specified char
	else
		return NULL;			//no pages for specified char

}



SERIAL_VECTOR cGmpagesMap::getAllPagers(){
	
	SERIAL_VECTOR vect;

	
	std::map< SERIAL, P_GMPAGE >::iterator iter;
	

	iter = gmpages_map.begin();
	int i = 0;
	
	while(iter != gmpages_map.end())
		{
			vect.push_back(iter->first);
			iter++;
		}
	
	return vect;
}

bool cGmpagesMap::optimize_page_indexes(SERIAL serial){ //rearranges pages indexes (page_number) not used for the moment
	
	P_GMPAGE tmp_page = NULL;
	
	if ((tmp_page = findPage (serial))== NULL) //No need to rearrange pages id.
			return false;

	do{
		if( tmp_page->getNextPage()->getPageNumber() != (tmp_page->getPageNumber() + 1))
			{
				tmp_page->getNextPage()->setPageNumber(tmp_page->getPageNumber() + 1);
			}
		
		tmp_page = tmp_page->getNextPage();

	}while(tmp_page->getNextPage() != NULL);
	return true;
}


   

cGmpagesMap* pages = new cGmpagesMap(); // Creates the page map. 


