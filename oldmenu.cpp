  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "oldmenu.h"
#include "inlines.h"
#include "chars.h"
#include "items.h"
#include "basics.h"


/* Classic menu
	mnu_prepare(s, 1, 3);
	mnu_setStyle(s, MENUSTYLE_SCROLL, 0x481);
	mnu_setTitle(s, "Almanacco dei Ricercati");
	mnu_addItem(s, 0, 0, "Condanna");
	mnu_addItem(s, 0, 1, "Assolvi");
	mnu_addItem(s, 0, 2, "Concedi la grazia divina");
	mnu_setCallback(s, funcidx("alm_chosen"));
	mnu_show(s);
*/
/* INCON LIST
	mnu_prepare(s, 1, lev);
	mnu_setStyle(s, 16, 0x481);
	mnu_setTitle(s, "Dragon Scales Tailoring");
	mnu_setCallback(s, funcidx("sewingdragon"));

	if (lev>=1)  mnu_addItem(s, 0, 0, "13d6 Dragon Scales Gorget"); 
	if (lev>=2)  mnu_addItem(s, 0, 1, "13dd Dragon Scales Gloves");		  
	if (lev>=3)  mnu_addItem(s, 0, 2, "13d4 Dragon Scales Sleeves");
	if (lev>=4)  mnu_addItem(s, 0, 3, "13e1 Dragon Scales Leggings");
	if (lev>=5)  mnu_addItem(s, 0, 4, "13e2 Dragon Scales Tunic");	
	mnu_show(s);
*/


#define MENUSTYLE_LARGE 128

/*
\brief Constructor
\author Endymion
\since 0.82
*/
cOldMenu::cOldMenu() : cBasicMenu( MENUTYPE_TRASPARENCY )
{
	width = 320;
	color=0;
	style=MENUTYPE_TRASPARENCY;
	title.clear();
	allPages.clear();
	type=NULL;
	setParameters( 10, 1 );
}

/*
\brief Destructor
\author Endymion
\since 0.82
*/
cOldMenu::~cOldMenu()
{
	if( type!=NULL )
		delete type;
}

/*
\brief Destructor
\author Endymion
\param numPerPage number of item for page
\param numpages the page count
\attention is useless now
\since 0.82
*/
void cOldMenu::setParameters( int rowForPage, int pageCount )
{
	this->rowForPage = rowForPage;
}

/*!
\brief adds an item at a given position of a correctly inizialized menu
\author Endymion
\since 0.82
\param page the page number
\param idx the index number
\param desc the text
*/
void cOldMenu::addMenuItem( int page, int idx, std::wstring desc )
{
//	mnu_addItem(s, 0, 2, "Concedi la grazia divina");
	std::map<UI32, wstring >& p= allPages[ page ];
	p.insert( make_pair( idx, desc ) );
}


void cOldMenu::handleButton( NXWCLIENT ps, cClientPacket* pkg  )
{
	
	UI32 button;
	if( isIconList( pkg->cmd ) )
		button = ((cPacketResponseToDialog*)pkg)->index.get()-1;
	else {
		button = ((cPacketMenuSelection*)pkg)->buttonId.get();
		if( button!=MENU_CLOSE )
			button = ((cMenu*)type)->rc_button[ button-1 ];
	}

	callback->Call( ps->toInt(), ps->currChar()->getSerial32(), button );

}


cServerPacket* cOldMenu::build()
{
	if( type==NULL ) { //need build
		if( style==MENUTYPE_ICONLIST ) {
			type = new cIconListMenu();
			buildIconList();
		}
		else {
			type = new cMenu( MENUTYPE_TRASPARENCY, 110, 70, true, true, true );
			buildClassic();
		}

		type->serial = serial;
		type->id= id;
		type->setCallBack( (callback!=NULL)? callback->getFuncIdx() : INVALID );
	}

	return type->createPacket();

}

/*!
\brief Build a classic menu
\author Endymion
\since 0.82
*/
void cOldMenu::buildClassic()
{

	cMenu* menu = (cMenu*)this->type;
	
	
	int pagebtny = 307;

	if( style&MENUSTYLE_LARGE ) 
		width = 512;

	MENU_TYPE curr_style = static_cast<MENU_TYPE>( style & ~MENUSTYLE_LARGE );

	id = curr_style;

	switch( curr_style ) {

	case MENUTYPE_STONE:
		menu->setCloseable( false );
		menu->addBackground( 2600, width, 340 );
		menu->addButton( 250, 17, 4017, 4017+1, INVALID, true );
		menu->addText( 30, 40, title, color );
		pagebtny = 300;
		break;
	
	case MENUTYPE_BLACKBOARD:
		menu->setCloseable( false );
		menu->addBackground( 2620, 320, 340 );
		menu->addButton( 250, 17, 4017, 4017+1, INVALID, true );
		menu->addText( 45, 17, title, color );
		pagebtny = 307;
		break;

	case MENUTYPE_PAPER:
		menu->setCloseable( false );
		menu->addBackground( 0x0DAC, 320, 340 );
		menu->addButton( 250, 7, 4017, 4017+1, INVALID, true );
		menu->addText( 45, 7, title, color );
		pagebtny = 307;
		break;

	case MENUTYPE_SCROLL:
		menu->setCloseable( false );
		menu->addBackground( 0x1432, 320, 340 );
		menu->addButton( 250, 27, 4017, 4017+1, INVALID, true );
		menu->addText( 45, 27, title, color );
		pagebtny = 290;
		break;

	case MENUTYPE_TRASPARENCY:
	default:
		menu->setCloseable( true );
		menu->addButton( 250, 27, 4017, 4017+1, INVALID, true );
		menu->addText( 45, 27, title, color );
		pagebtny = 290;
		break;
	}

	menu->addPage( 1 );

	int buttonnum=0; //button number
	int position = 80;

	std::map< UI32, std::map< UI32, std::wstring >  >::iterator curr_page( allPages.begin() ), last_page( allPages.end() );
	int page_count = allPages.size();

/*	int oldk = 0;



	for( int k=0; curr_page!=last_page; ++curr_page, ++k ) {

		std::map< UI32, std::wstring >::iterator iter( curr_page->second.begin() ), end( curr_page->second.end() );
	
		for( int i=0; iter!=end; ++iter, ++i )
		{
				if ( k > oldk )
				{
					position = 80;
					menu->addPage( menu->pageCurrent+1 );
					oldk = k;
				}

				menu->addText( 80, position, iter->second, color );

				menu->addButton( 50, position+3, 4005, 4005+1, buttonnum );

				position += 20;
				++buttonnum;
		}
	}


	curr_page = allPages.begin();

	for( int p=1; curr_page!=last_page; ++curr_page, ++p )
	{
		menu->addPage( p );
		if( p > 1 )
		{
			menu->addPageButton( 50, pagebtny, 4014, 4014+1, p-1 ); //back button
		}
		if( p < allPages.size() )
		{
			menu->addPageButton( 254, pagebtny, 4005, 4005+1, p+1 ); //next button
		}
	}
*/	

	for( ; curr_page!=last_page; ++curr_page ) {

		std::map< UI32, std::wstring >::iterator iter( curr_page->second.begin() ), end( curr_page->second.end() );
	
		if( menu->pageCurrent>1 )
			menu->addPage( menu->pageCurrent+1 );

		for( int b=1; iter!=end; ++iter, ++b )
		{
			if( b==rowForPage ) {
				position = 80;
				b=1;

				menu->addPageButton( 254, pagebtny, 4005, 4005+1, menu->pageCurrent+1 ); //next button

				menu->addPage( menu->pageCurrent+1 );

				menu->addPageButton(  50, pagebtny, 4014, 4014+1, menu->pageCurrent-1 ); //back button
			}

			menu->addText( 80, position, iter->second, color );

			menu->addButton( 50, position+3, 4005, 4005+1, buttonnum, true );

			position += 20;
			++buttonnum;
		}

		if( page_count>1 ) {

			if( menu->pageCurrent>1 )
				menu->addPageButton(  50, pagebtny, 4014, 4014+1, menu->pageCurrent-1 ); //back button

			if( menu->pageCurrent< page_count )
				menu->addPageButton( 254, pagebtny, 4005, 4005+1, menu->pageCurrent+1 ); //next button
		}

	}


}

/*!
\brief Show an icon list menu
\author Endymion
\since 0.82
*/
void cOldMenu::buildIconList()
{
	
	cIconListMenu* menu = (cIconListMenu*)this->type;
	
	wstring2string( title, menu->question );

	std::map< UI32, std::map< UI32, std::wstring >  >::iterator page( allPages.begin() ), last_page( allPages.end() );
	if( page!=last_page ) { //not support multiple pages
		
		std::map< UI32, std::wstring >::iterator iter( page->second.begin() ), end( page->second.end() );
		for( ; iter!=end; ++iter ) {

			string s;
			wstring2string( iter->second, s );
	
			char num[5] = { 0x00, };
			memcpy( num, s.c_str(), 4 );
			menu->addIcon( hex2num( num ), 0x0000, std::string( s.c_str() +5 ) );

		}
	}
	
}



