  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "oldmenu.h"


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
cOldMenu::cOldMenu()
{
	setWidth( 320 );
}

/*
\brief Destructor
\author Endymion
\since 0.82
*/
cOldMenu::~cOldMenu()
{
}

/*
\brief Destructor
\author Endymion
\param numPerPage number of item for page
\param numpages the page count
\attention is useless now
\since 0.82
*/
void cOldMenu::setParameters( int numPerPage, int numpages )
{
}

/*!
\brief adds an item at a given position of a correctly inizialized menu
\author Endymion
\since 0.82
\param page the page number
\param idx the index number
\param desc the text
*/
void cOldMenu::addMenuItem( int page, int idx, std::wstring& desc )
{
//	mnu_addItem(s, 0, 2, "Concedi la grazia divina");
	std::map<UI32, wstring >& p= allPages[ page ];
	p.insert( make_pair( idx, desc ) );
}

/*!
\brief Set title
\author Endymion
\since 0.82
\param str the title
*/
void cOldMenu::setTitle( wstring& str )
{
	title = str;
}

/*!
\brief Set width
\author Endymion
\since 0.82
\param width the width
*/
void cOldMenu::setWidth( int width )
{
	this->width=width;
}

/*!
\brief Set style
\author Endymion
\since 0.82
\param style the style
\param color the color
*/
void cOldMenu::setStyle( int style, int color )
{
	this->style=style;
	setColor( color );
}

/*!
\brief Set color
\author Endymion
\since 0.82
\param color the color
*/
void cOldMenu::setColor( int color )
{
	this->color=color;
}

/*!
\brief Handle selection of a button
\author Endymion
\since 0.82
\attention this function is pure virtual.
*/
void cOldMenu::buttonSelected( NXWSOCKET s, unsigned short int buttonPressed, int type )
{
	//do nothing
}

void cOldMenu::show( P_CHAR pc )
{
}


cOldMenuClassic::cOldMenuClassic() : cMenu()
{
}

cOldMenuClassic::~cOldMenuClassic()
{
}

/*!
\brief Build a classic menu
\author Endymion
\since 0.82
*/
void cOldMenuClassic::buildOldMenu()
{

	int pagebtny = 307;

	if( style&MENUSTYLE_LARGE ) 
		setWidth( 512 );

	UI32 curr_style = style & ~MENUSTYLE_LARGE;

	setId( curr_style );
	setX( 0x6E );
	setY( 0x46 );

	//--static pages
	if( curr_style==MENUSTYLE_STONE ) {
		setCloseAble( false );
		addPage( 0 );
		addResizeGump( 0, 0, 2600, width, 340 );
		addButton( 250, 17, 4017, 4017+1, 1 );
		addText( 30, 40, title, color );
		pagebtny = 300;
	}
	else if( curr_style==MENUSTYLE_BLACKBOARD ) {
		setCloseAble( false );
		addPage( 0 );
		addResizeGump( 0, 0, 2620, 320, 340 );
		addButton( 250, 17, 4017, 4017+1, 1 );
		addText( 45, 17, title, color );
		pagebtny = 307;
	}
	else if( curr_style==MENUSTYLE_PAPER ) {
		setCloseAble( false );
		addPage( 0 );
		addResizeGump( 0, 0, 0x0DAC, 320, 340 );
		addButton( 250, 7, 4017, 4017+1, 1 );
		addText( 45, 7, title, color );
		pagebtny = 307;
	}
	else if( curr_style==MENUSTYLE_SCROLL ) {
		setCloseAble( false );
		addPage( 0 );
		addResizeGump( 0, 0, 0x1432, 320, 340 );
		addButton( 250, 27, 4017, 4017+1, 1 );
		addText( 45, 27, title, color );
		pagebtny = 290;
	}
	else if (curr_style==MENUSTYLE_TRASPARENCY) {
		setCloseAble( true );
		addButton( 250, 27, 4017, 4017+1, 1 );
		addText( 45, 27, title, color );
		pagebtny = 290;
	}



	int pagenum = 1;
	addPage( pagenum );

	//this i don't think what are
	//sprintf( temp, m_strTitle );
	//m_lstLabels.push_back( new string( temp ) );
	//

	int oldk = 0;
	int buttonnum=10; //button number
	int position = 80, linenum = 1;

	std::map< UI32, std::map< UI32, std::wstring >  >::iterator curr_page( allPages.begin() ), last_page( allPages.end() );

	for( int k=0; curr_page!=last_page; ++curr_page, ++k ) {

		std::map< UI32, std::wstring >::iterator iter( curr_page->second.begin() ), end( curr_page->second.end() );
	
		for( int i=0; iter!=end; ++iter, ++i )
		{
				if ( k > oldk )
				{
					position = 80;
					pagenum++;
					addPage( pagenum );
					oldk = k;
				}

				addText( 80, position, iter->second, color );

				addButton( 50, position+3, 4005, 4005+1, buttonnum, pagenum );

				position += 20;
				++linenum;
				++buttonnum;
		}
	}


	curr_page = allPages.begin();

	//there is not this check in old code
	//if( allPages.size()==1 )
	//	return; //not need back and forward buttons with only a page

	//now add back and forward buttons
	for( int p=1; curr_page!=last_page; ++curr_page, ++p )
	{
		addPage( p );
		if( p > 1 )
		{
			addPageButton( 50, pagebtny, 4014, 4014+1, p-1 ); //back button
		}
		if( p < allPages.size() )
		{
			addPageButton( 254, pagebtny, 4005, 4005+1, p+1 ); //next button
		}
	}
}

void cOldMenuClassic::buttonSelected( NXWSOCKET s, unsigned short int buttonPressed, int type )
{
}

void cOldMenuClassic::show( P_CHAR pc )
{
	buildOldMenu();
	cMenu::show( pc );
}


cOldMenuIconList::cOldMenuIconList()
{
}

cOldMenuIconList::~cOldMenuIconList()
{
}

/*!
\brief Show an icon list menu
\author Endymion
\since 0.82
*/
void cOldMenuIconList::show( P_CHAR pc )
{
	
	VALIDATEPC(pc);

	cPacketIconListMenu p;
	
	p.gump = serial;
	p.id= id;
	wstring2string( title, p.question );

	std::vector< pkg_icon_list_menu_st > icons;
	std::map< UI32, std::map< UI32, std::wstring >  >::iterator iter( allPages.begin() ), end( allPages.end() );
	if( iter!=end ) { //not support multiple pages
		
		std::map< UI32, std::wstring >& pagina = iter->second;

		std::map< UI32, std::wstring >::iterator itp( pagina.begin() ), endp( pagina.end() );
		for( ; itp!=endp; ++itp ) {

			string s;
			wstring2string( itp->second, s );
	
			pkg_icon_list_menu_st icon;

			icon.color=0x0000;	
			char num[5] = { 0x00, };
			memcpy( num, s.c_str(), 4 );
			icon.model=hex2num( num );
			icon.response += (s.c_str() +5);
			icons.push_back( icon );

		}
	}
	p.icons = &icons;
	
	p.send( pc->getClient() );
}

void cOldMenuIconList::buttonSelected( NXWSOCKET s, unsigned short int buttonPressed, int type )
{
	ConOut( "chiamata" );
}
