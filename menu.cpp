  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "menu.h"

cMenus menus;

#define OPTIONS2BITSET( MOVE, CLOSE, DISPOSE ) \
	( MOVE*MOVEABLE | CLOSE*CLOSEABLE | DISPOSE*DISPOSEABLE  ) \


/*!
\brief Constructor
\author Endymion
*/
cMenus::cMenus()
{
	current_serial=INVALID;
}


/*!
\brief Destructor
\author Endymion
*/
cMenus::~cMenus()
{
}


/*!
\brief Create a new menu
\author Endymion
\param id the menu id
\param x the x location on screen
\param y the y location on screen
\param options the options
\return the serial of new menu
*/
SERIAL cMenus::createMenu( UI32 id, UI32 x, UI32 y, UI08 options )
{
	menuMap.insert( make_pair( current_serial++, new cMenu( current_serial++, id, x, y, options ) ) );
	return current_serial -1;
}

/*!
\brief Create a new menu
\author Endymion
\param id the menu id
\param x the x location on screen
\param y the y location on screen
\param canMove true if can move
\param canClose true if can close
\param canDispose true if can dispose
\return the serial of new menu
*/
SERIAL	cMenus::createMenu( UI32 id, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose )
{
	return createMenu( id, x, y, OPTIONS2BITSET(canMove, canClose, canDispose) );
}


LOGICAL	cMenus::deleteMenu( SERIAL menu )
{
	MENU_MAP::iterator iter( menuMap.find( menu ) );
	if( iter != menuMap.end() )
	{
		menuMap.erase( iter );
		return true;
	}
	return false;
}

P_MENU cMenus::selectMenu( SERIAL menu )
{
	MENU_MAP::iterator iter( menuMap.find( menu ) );
	if( iter != menuMap.end() )
		return iter->second;
	return NULL;	
}

LOGICAL cMenus::showMenu( SERIAL menu, P_CHAR pc )
{
	MENU_MAP::iterator iter( menuMap.find( menu ) );
	if( iter != menuMap.end() )
	{
		iter->second->show( pc );
		return true;
	}
	return false;	
}

LOGICAL cMenus::handleMenu( NXWCLIENT ps )
{

	if( ps==NULL )
		return false;

	P_CHAR pc=ps->currChar();
	VALIDATEPCR( pc, false );

	cPacketMenuSelection p;
	p.receive( ps );

	newAmxEvent("gui_handleResponse")->Call( p.id.get(), p.gump.get(), p.buttonId.get(), pc->getSerial32() );
	return true;
/*	if( data[0] == 0xB1 )
	{
		char		text[256];
		UI32		textIndex;
//		UI32		dataSize		= static_cast<int>(data[1]<<8)|static_cast<int>(data[2]);
		UI32 		gumpSerial	= static_cast<int>(data[3]<<24)|static_cast<int>(data[4]<<16)|static_cast<int>(data[5]<<8)|static_cast<int>(data[6]);
		UI32		gump				= static_cast<int>(data[7]<<24)|static_cast<int>(data[8]<<16)|static_cast<int>(data[9]<<8)|static_cast<int>(data[10]);
		UI32 		button			= static_cast<int>(data[13]<<8)|static_cast<int>(data[14]);
		LOGICAL	hasRadio		= data[18] == 1 ? true : false;
		SI32		radioButton;
		UI32		fieldCount;
		UI32		fieldOffset;
		UI32		fieldLength;
		UI32		fieldId;
		if( hasRadio )
		{
			radioButton = static_cast<int>(data[22]); // probably data[19]...data[22]
			fieldCount	= static_cast<int>(data[26]); // probably data[23]...data[26]
			fieldOffset	= 27;
		}
		else
		{
			radioButton	= INVALID;
			fieldCount	= static_cast<int>(data[22]);
			fieldOffset	= 23;
		}
		responseMap.clear();
		while( fieldCount )
		{
			fieldId	  = static_cast<int>(data[fieldOffset]<<8) | static_cast<int>(data[++fieldOffset]);
			//fieldLength = static_cast<int>(data[++fieldOffset]<<8) | static_cast<int>(data[++fieldOffset]);
			//FIX by Riekr
			fieldLength = static_cast<int>(data[++fieldOffset])<<8; 
			fieldLength |= static_cast<int>(data[++fieldOffset]); 

			//
			// Transform unicode text to text
			//
			for (textIndex = 0; textIndex < fieldLength; ++textIndex)
			{
				fieldOffset += 2;
				text[textIndex] = data[fieldOffset];
			}
			text[textIndex] = 0;
			addResponse( fieldId, text );
			++fieldOffset;
			--fieldCount;
		}
	newAmxEvent("gui_handleResponse")->Call( gump, gumpSerial, button, pc->getSerial32() );
	return true;
	}
	else*/
}

/*LOGICAL cMenu::selectResponse( UI32 fieldId, std::string &value )
{
	map<UI32, std::string >::iterator iter( responseMap.find( fieldId ) );
	if( iter != responseMap.end() )
	{
		value = iter->second;
		return true;
	}
	else
		return false;
}*/

/*LOGICAL	cMenu::addResponse( UI32 fieldId, std::string value )
{
	if( responseMap.find( fieldId ) != responseMap.end() )
		return false;
	else
	{
		responseMap.insert( make_pair( fieldId, value ) );
		return true;
	}
}*/




















cMenu::cMenu()
{
	cMenu( INVALID, 0, 0, 0, MOVEABLE|CLOSEABLE|DISPOSEABLE );
}

cMenu::cMenu( SERIAL menu, UI32 id, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose )
{
	cMenu( menu, id, x, y, OPTIONS2BITSET(canMove, canClose, canDispose) );
}

cMenu::cMenu( SERIAL menu, UI32 id, UI32 x, UI32 y, UI08 options )
{
	setId( id );
	setX( x );
	setY( y );
	setOptions( options );
	this->serial = serial;
}

cMenu::~cMenu()
{
	
}

void cMenu::setId( const UI32 arg )
{
	id = arg ;
}

void cMenu::setX( const UI32 arg )
{
	x = arg;
}

void cMenu::setY( const UI32 arg )
{
	y = arg;
}

void cMenu::setOptions( const UI08 options )
{
	this->options=options;
}

void cMenu::setOptions( const UI08 options, const bool value )
{
	if( value )
		this->options|=options;
	else 
		this->options&=~options;
}







void cMenu::addCommand( const std::string& command )
{
	commands.push_back( command );
}

void cMenu::addCommand( char const *formatStr, ... )
{
	char temp[TEMP_STR_SIZE];
	
	va_list vargs;
	va_start(vargs, formatStr );
	vsnprintf( temp, sizeof(temp)-1, formatStr, vargs);
	va_end(vargs);

	addCommand( std::string( temp ) );
}


UI32 cMenu::addString( wstring& u )
{
	texts.push_back( u );
	return texts.size()-1;
}



void cMenu::addButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 returnCode, UI32 page )
{
	addCommand( "{button %d %d %d %d %d 0 %d}", x, y, up, down, page, returnCode );
}


void cMenu::addPageButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 page )
{
	addCommand( "{button %d %d %d %d 0 %d 0}", x, y, up, down, page );
}
/*
void cMenu::addGump( UI32 x, UI32 y, UI32 gump, UI32 hue )
{
	addCommand( "{gumppic %d %d %d hue=%d}", x, y, gump, hue );
}

void cMenu::addTiledGump( UI32 x, UI32 y, UI32 width, UI32 height, UI32 gump, UI32 hue )
{
	addCommand( "{gumppictiled %d %d %d %d %d %d}", x, y, width, height, gump, hue );
}

void cMenu::addHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, wstring& html, UI32 hasBack, UI32 canScroll )
{
	addCommand( "{htmlgump %d %d %d %d %d %d %d}", x, y, width, height, addString(html), hasBack, canScroll );
}

void cMenu::addXmfHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, wstring& clilocid, UI32 hasBack , UI32 canScroll )
{
	addCommand( "{xmfhtmlgump %d %d %d %d %s %d %d}", x, y, width, height, addString(clilocid), hasBack, canScroll );
}

void cMenu::addCheckertrans( UI32 x, UI32 y, UI32 width, UI32 height )
{
	addCommand( "{checkertrans %d %d %d %d}", x, y, width, height );
}

void cMenu::addCroppedText( UI32 x, UI32 y, UI32 width, UI32 height, wstring& text, UI32 hue )
{
	addCommand( "{croppedtext %d %d %d %d %d %d}", x, y, width, height, hue, addString(text) );
}
*/
void cMenu::addPage( UI32 page )
{
	if ( page < 256 )
	{
		addCommand( "{page %d}", page );
	}
}
/*
void cMenu::addGroup( UI32 group )
{
	addCommand( "{group %d}", group );
}
*/
void cMenu::addText( UI32 x, UI32 y, wstring& data, UI32 hue )
{
	addCommand( "{text %d %d %d %d}", x, y, hue, addString(data) ); //text <Spaces from Left> <Space from top> <Length, Color?> <# in order>
}
/*
void cMenu::addBackground( UI32 gump, UI32 width, UI32 height )
{
	addResizeGump( 0, 0, gump, width, height );
}
*/
void cMenu::addResizeGump( UI32 x, UI32 y, UI32 gump, UI32 width, UI32 height )
{
	addCommand( "{resizepic %d %d %d %d %d}", x, y, gump, width, height );
}
/*
void cMenu::addTilePic( UI32 x, UI32 y, UI32 tile, UI32 hue )
{
	addCommand( "{tilepic %d %d %d %d}", x, y, tile, hue );
}

void cMenu::addInputField( UI32 x, UI32 y, UI32 width, UI32 height, UI32 textId, wstring& data, UI32 hue )
{
	addCommand( "{textentry %d %d %d %d %d %d %d}", x, y, width, height, hue, textId, addString(data) );
}

void cMenu::addCheckbox( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result )
{
	addCommand( "{checkbox %d %d %d %d %d %d}", x, y, off, on, result, checked );
}

void cMenu::addRadioButton( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result  )
{
	addCommand( "{radio %d %d %d %d %d %d}", x, y, off, on, checked, result );
}

*/

void cMenu::handleButton( const NXWSOCKET socket, const UI32 button )
{
	callback->Call( socket, button );
}

void cMenu::setCallBack( const std::string& arg )
{
	callback = newAmxEvent( const_cast< char* >( arg.c_str() ) );
}



void cMenu::setMoveAble( const bool arg )
{
	setOptions( MOVEABLE, arg );
}

void cMenu::setCloseAble( const bool arg )
{
	setOptions( CLOSEABLE, arg );
}

void cMenu::setDisposeAble( const bool arg )
{
	setOptions( DISPOSEABLE, arg );
}









void cMenu::show( P_CHAR pc )
{
	VALIDATEPC( pc );

	NXWCLIENT ps=pc->getClient();
	if( ps==NULL ) return;

	if( !(options&MOVEABLE) )
		addCommand("{nomove}");

	if( !(options&CLOSEABLE) )
		addCommand("{noclose}");

	if( !(options&DISPOSEABLE) )
		addCommand("{nodispose}");


	cPacketMenu	packet; //!< menu packet with new packet system


	packet.id=serial;
	packet.gump=id;
	packet.x = x;
	packet.y = y;

	packet.commands = &commands;
	packet.texts = &texts;
	
	packet.send( ps );
}




/* menu exaple
  mnu_prepare(s, 1, 3);
  mnu_setStyle(s, MENUSTYLE_SCROLL, 0x481);
  mnu_setTitle(s, "Almanacco dei Ricercati");
  mnu_addItem(s, 0, 0, "Condanna");
  mnu_addItem(s, 0, 1, "Assolvi");
  mnu_addItem(s, 0, 2, "Concedi la grazia divina");
  mnu_setCallback(s, funcidx("alm_chosen"));
  mnu_show(s);
*/

#define MENUSTYLE_LARGE 128

cOldMenu::cOldMenu()
{
	setWidth( 320 );
}

cOldMenu::~cOldMenu()
{
}

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
void cOldMenu::addMenuItem( int page, int idx, char* desc )
{
//	mnu_addItem(s, 0, 2, "Concedi la grazia divina");
	wstring s;
	string2wstring( string( desc ), s );
	std::map<UI32, wstring >& p= allPages[ page ];
	p.insert( make_pair( idx, s ) );
}

/*void cOldMenu::setCallback( int cback ) 
{ 
}*/

void cOldMenu::buildMenu ()
{
	switch( style&0x7F )
	{
		case MENUSTYLE_STONE:
		case MENUSTYLE_SCROLL:
		case MENUSTYLE_PAPER:
		case MENUSTYLE_BLACKBOARD:
		case MENUSTYLE_TRASPARENCY:
			buildClassicMenu();
			return;
		case MENUSTYLE_ICONLIST:
			buildIconList();
			return;
		case MENUSTYLE_ICONMENU:
			buildIconMenu();
			return;
		default:
			style = MENUSTYLE_STONE;
			buildClassicMenu();
			WarnOut("cCustomMenu::buildMenu() : unsupported menu style was used\n");
	}

}

void cOldMenu::showMenu( NXWSOCKET s )
{
    NXWCLIENT ps= getClientFromSocket( s );
	if( ps==NULL ) return;
	this->show( ps->currChar() );
}

void cOldMenu::setTitle( char* str )
{
	string2wstring( string( str ), title );
}

void cOldMenu::setWidth( int width )
{
	this->width=width;
}

void cOldMenu::setStyle( int style, int color )
{
	this->style=style;
	setColor( color );
}

void cOldMenu::setColor( int color )
{
	this->color=color;
}

void cOldMenu::buttonSelected( NXWSOCKET s, unsigned short int buttonPressed, int type )
{
}

void cOldMenu::buildClassicMenu()
{

	int pagebtny = 307;

	if( style&MENUSTYLE_LARGE ) 
		setWidth( 512 );

	UI32 curr_style = style & ~MENUSTYLE_LARGE;

	//--static pages
	if( curr_style==MENUSTYLE_STONE ) {
		setCloseAble( false );
		addPage( 0 );
		addResizeGump( 0, 0, 2600, width, 340 );
		addPageButton( 250, 17, 4017, 4017+1, 1 );
		addText( 30, 40, title, color );
		pagebtny = 300;
	}
	else if( curr_style==MENUSTYLE_BLACKBOARD ) {
		setCloseAble( false );
		addPage( 0 );
		addResizeGump( 0, 0, 2620, 320, 340 );
		addPageButton( 250, 17, 4017, 4017+1, 1 );
		addText( 45, 17, title, color );
		pagebtny = 307;
	}
	else if( curr_style==MENUSTYLE_PAPER ) {
		setCloseAble( false );
		addPage( 0 );
		addResizeGump( 0, 0, 0x0DAC, 320, 340 );
		addPageButton( 250, 7, 4017, 4017+1, 1 );
		addText( 45, 7, title, color );
		pagebtny = 307;
	}
	else if( curr_style==MENUSTYLE_SCROLL ) {
		setCloseAble( false );
		addPage( 0 );
		addResizeGump( 0, 0, 0x1432, 320, 340 );
		addPageButton( 250, 27, 4017, 4017+1, 1 );
		addText( 45, 27, title, color );
		pagebtny = 290;
	}
	else if (curr_style==MENUSTYLE_TRASPARENCY) {
		setCloseAble( true );
		addPageButton( 250, 27, 4017, 4017+1, 1 );
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

	if( allPages.size()==1 )
		return; //not need back and forward buttons with only a page

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

void cOldMenu::buildIconList()
{
}

void cOldMenu::buildIconMenu()
{
}



