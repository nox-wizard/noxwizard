  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "menu.h"
#include "amx/amxwraps.h"
#include "chars.h"
#include "items.h"
#include "inlines.h"

cMenus Menus;

/*!
\brief Constructor
\author Endymion
*/
cMenus::cMenus()
{
	current_serial=0;
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
\param menu the menu
\return the serial of new menu
*/
P_MENU cMenus::insertMenu( P_MENU menu )
{
	menu->serial=++current_serial;
	menuMap.insert( make_pair( menu->serial, menu ) );
	return menu;
}


P_MENU cMenus::getMenu( SERIAL menu )
{
	MENU_MAP::iterator iter( menuMap.find( menu ) );
	if( iter != menuMap.end() )
		return iter->second;
	return NULL;	
}

bool cMenus::showMenu( SERIAL menu, P_CHAR pc )
{
	MENU_MAP::iterator iter( menuMap.find( menu ) );
	if( iter != menuMap.end() )
	{
		iter->second->show( pc );
		return true;
	}
	return false;	
}

inline bool isIconList( NXWSOCKET s ) 
{
	return isIconList( buffer[s][0] );
}

inline bool isIconList( UI08 cmd )
{
	return cmd==PKG_RESPONSE_TO_DIALOG;
}

/*!
\brief 
\author Endymion
\return true if menu need delete
*/
bool cMenus::removeFromView( P_MENU menu, SERIAL chr )
{
	
	menu->whoSeeThis.erase( chr );

	std::map<SERIAL, std::set<SERIAL> >::iterator i( whoSeeWhat.find( menu->serial ) );
	if( i!=whoSeeWhat.end() ) {
		i->second.erase( chr );

		if( i->second.empty() )
			whoSeeWhat.erase( i );
	}

	return menu->whoSeeThis.empty();
}

SERIAL cMenus::removeMenu( SERIAL menu, P_CHAR pc )
{

	MENU_MAP::iterator iter( menuMap.find( menu ) );
	if( iter != menuMap.end() )
	{		
		bool needDelete = removeFromView( iter->second, pc->getSerial32() );

		if( needDelete ) {
			delete iter->second;
			menuMap.erase( iter );
			return INVALID;
		}
		else
			return menu;
	}
	return INVALID;
}

LOGICAL cMenus::handleMenu( NXWCLIENT ps )
{

	if( ps==NULL )
		return false;

	P_CHAR pc=ps->currChar();
	VALIDATEPCR( pc, false );

	
	cClientPacket* p =  NULL;

	SERIAL serial;
	if( isIconList( ps->toInt() ) ) {
		p = new cPacketResponseToDialog();
		p->receive( ps );
		serial = ((cPacketResponseToDialog*)p)->serial.get();
	}
	else {
		p = new cPacketMenuSelection();
		p->receive( ps );
		serial = ((cPacketMenuSelection*)p)->serial.get();
	}


	P_MENU menu = Menus.getMenu( serial );
	VALIDATEPMR( menu, false );

	SERIAL menu_serial = menu->serial;

	menu->handleButton( ps, p );

	removeMenu( menu_serial, pc );		
	
	return true;

}







cBasicMenu::cBasicMenu( MENU_TYPE id )
{
	serial=INVALID;
	this->id=id;
	callback=NULL;
}

cBasicMenu::~cBasicMenu()
{
	if( callback!=NULL )
		delete callback;
}


void cBasicMenu::setCallBack( std::string& arg )
{
	if( callback!=NULL )
		safedelete( callback );

	callback = new AmxFunction( const_cast< char* >( arg.c_str() ) );
}

void cBasicMenu::setCallBack( int fn )
{
	if( callback!=NULL )
		safedelete( callback );

	callback = new AmxFunction( fn );
}

void cBasicMenu::handleButton( NXWCLIENT ps, cClientPacket* pkg  )
{
}

void cBasicMenu::show( P_CHAR pc )
{
	VALIDATEPC( pc );

	NXWCLIENT ps=pc->getClient();
	if( ps==NULL ) return;

	cServerPacket* packet = build();
	packet->send( ps );
	delete packet;

	Menus.whoSeeWhat[ pc->getSerial32() ].insert( serial );
	whoSeeThis.insert( pc->getSerial32() );
}

cServerPacket* cBasicMenu::createPacket()
{
	return NULL;
}

cServerPacket* cBasicMenu::build()
{
	return createPacket();
}









cMenu::cMenu( MENU_TYPE id, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose ) : cBasicMenu( id )
{
	this->x = x;
	this->y = y;

	moveable=closeable=disposeable=true;

	setMoveable( canMove );
	setCloseable( canClose );
	setDisposeable( canDispose );

	buttonCurrent = 1;
	pageCount=1;
	pageCurrent=1;

	for( int i=0; i<4; ++i ) {
		buffer[i] = INVALID;
	}

}

cMenu::~cMenu()
{
}

void cMenu::addCommand( std::string& command )
{
	commands.push_back( command );
}

void cMenu::addCommand( char* formatStr, ... )
{
	char temp[TEMP_STR_SIZE];
	
	va_list vargs;
	va_start(vargs, formatStr );
	vsnprintf( temp, sizeof(temp)-1, formatStr, vargs);
	va_end(vargs);

	addCommand( std::string( temp ) );
}

void cMenu::removeCommand( std::string& command )
{
	std::vector< std::string >::iterator iter( commands.begin() ), end( commands.end() );
	for( ; iter!=end; iter++ ) {
		if( (*iter)==command ) {
			commands.erase( iter );
			return;
		}
	}
}


UI32 cMenu::addString( wstring& u )
{
	texts.push_back( u );
	return texts.size()-1;
}

void cMenu::clear()
{
	setCloseable( true );
	setMoveable( true );
	setDisposeable( true );

	buttonCurrent = 1;
	buttonCallbacks.clear();

	switchs = NULL;
	propEditInfo.clear();
	commands.clear();
	texts.clear();

	x = y = 0;

	pageCurrent=0;
	pageCount=1;

	for( int i=0; i<4; ++i ) {
		buffer[i] = INVALID;
		buffer_str[i].erase();
	}

}

void cMenu::addButton( UI32 x, UI32 y, UI32 up, UI32 down, SERIAL returnCode, bool pressable )
{
	addCommand( "{button %d %d %d %d %d %d %d}", x, y, up, down, pressable, pageCurrent, returnCode );
	buttonCurrent++;
}

 //needed for remove possibility of two return code equal
#define AUTO_BUTTONID_OFFSET 0xFFFFF000

void cMenu::addButtonFn( UI32 x, UI32 y, UI32 up, UI32 down, SI32 returnCode, bool pressable, FUNCIDX fn )
{
	buttonCallbacks_st info = { fn, returnCode };
	buttonCallbacks.insert( make_pair(  AUTO_BUTTONID_OFFSET + buttonCurrent, info ) );
	addButton( x, y, up, down, AUTO_BUTTONID_OFFSET + buttonCurrent, pressable );
}

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

void cMenu::addText( UI32 x, UI32 y, wstring& data, UI32 hue )
{
	addCommand( "{text %d %d %d %d}", x, y, hue, addString(data) ); //text <Spaces from Left> <Space from top> <Length, Color?> <# in order>
}

void cMenu::addBackground( UI32 gump, UI32 width, UI32 height )
{
	addResizeGump( 0, 0, gump, width, height );
}

void cMenu::addResizeGump( UI32 x, UI32 y, UI32 gump, UI32 width, UI32 height )
{
	addCommand( "{resizepic %d %d %d %d %d}", x, y, gump, width, height );
}

void cMenu::addTilePic( UI32 x, UI32 y, UI32 tile, UI32 hue )
{
	addCommand( "{tilepic %d %d %d %d}", x, y, tile, hue );
}

void cMenu::addInputField( UI32 x, UI32 y, UI32 width, UI32 height, UI32 textId, wstring& data, UI32 hue )
{
	addCommand( "{textentry %d %d %d %d %d %d %d}", x, y, width, height, hue, textId, addString(data) );
}

void cMenu::addPropertyField( UI32 x, UI32 y, UI32 width, UI32 height, UI32 property, UI32 subProperty, UI32 hue, UI32 subProperty2 )
{
	
	VAR_TYPE t = getPropertyType( property );
	if( t==T_BOOL ) 
	{
		addCheckbox( x, y, 0x00D2, 0x00D3, getPropertyFieldBool( buffer[0], buffer[1], property, subProperty, subProperty2 ), property );
		propEditInfo.insert( make_pair( property, subProperty ) );
	}
	else
	{
		addInputField( x, y, width, height, property, getPropertyField( buffer[0], buffer[1], property, subProperty, subProperty2 ), hue );
		propEditInfo.insert( make_pair( property, subProperty ) );
	}
	
}

void cMenu::addCheckbox( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result )
{
	addCommand( "{checkbox %d %d %d %d %d %d}", x, y, off, on, checked, result );
}

void cMenu::addRadioButton( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result  )
{
	addCommand( "{radio %d %d %d %d %d %d}", x, y, off, on, checked, result );
}


void cMenu::addPageButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 page )
{
	addCommand( "{button %d %d %d %d 0 %d 0}", x, y, up, down, page );
}

/*!
\brief Add a new page
\author Endymion
\param page the page num, if INVALID is used automatic page count
*/
void cMenu::addPage( UI32 page )
{
	pageCurrent=page;
	pageCount=page+1;

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


void cMenu::handleButton( NXWCLIENT ps, cClientPacket* pkg  )
{
	
	cPacketMenuSelection* p = (cPacketMenuSelection*)pkg;

	SERIAL button = p->buttonId.get();
	switchs = &p->switchs;
	
	if( button!=MENU_CLOSE ) { 

		std::map< SERIAL, buttonCallbacks_st >::iterator iter( buttonCallbacks.find( button ) );
		if( iter!=buttonCallbacks.end() ) {

			AmxFunction func( iter->second.fn );
			func.Call( ps->toInt(), serial, iter->second.returnCode );
			return;

		}
	}

	//set property if there are
	textResp = &p->text_entries;

	if( button==buffer[3] ) {
		std::map< SERIAL, SERIAL >::iterator propIter( propEditInfo.begin() ), lastProp( propEditInfo.end() );
		for( ; propIter!=lastProp; ++propIter ) {
			if( getPropertyType( propIter->first )!=T_BOOL ) {
				std::wstring* data = getText( propIter->first );
				if( data!=NULL )
					setPropertyField( buffer[0], buffer[1], propIter->first, propIter->second, 0, *data );
			}
			else 
				setPropertyField( buffer[0], buffer[1], propIter->first, propIter->second, 0, getCheckBox( propIter->first ) );
		}
	}
	
	callback->Call( ps->toInt(), serial, button );
}


bool cMenu::getPropertyFieldBool( SERIAL type, SERIAL obj, SERIAL prop, SERIAL subProp, SERIAL subProp2 )
{
	
	switch( type ) {
	
		case PROP_CHARACTER: {
			P_CHAR pc = pointers::findCharBySerial( obj );
			VALIDATEPCR( pc, false );

			return getCharBoolProperty( pc, prop, subProp );

		}
		case PROP_ITEM : {
			P_ITEM pi = pointers::findItemBySerial( obj );
			VALIDATEPIR( pi, false );

			return getItemBoolProperty( pi, prop, subProp );

		}
		case PROP_CALENDAR:
		case PROP_GUILD:
		default:
			return false;
	}

}

void cMenu::setPropertyField( SERIAL type, SERIAL obj, SERIAL prop, SERIAL subProp, SERIAL subProp2, bool data )
{
	switch( type ) {
	
		case PROP_CHARACTER: {
			P_CHAR pc = pointers::findCharBySerial( obj );
			VALIDATEPC(pc);
			if( data!=getCharBoolProperty( pc, prop, subProp ) )
				setCharBoolProperty( pc, prop, subProp, subProp2, data );
			}
			break;
		case PROP_ITEM : {
			P_ITEM pi = pointers::findItemBySerial( obj );
			VALIDATEPI(pi);
			if( data!=getItemBoolProperty( pi, prop, subProp ) )
				setItemBoolProperty( pi, prop, subProp, data );
			}
			break;
		case PROP_CALENDAR:
		case PROP_GUILD:
		default:
			return;
	}
		
}

void cMenu::setPropertyField( SERIAL type, SERIAL obj, SERIAL prop, SERIAL subProp, SERIAL subProp2, std::wstring& data )
{
	VARTYPE t = getPropertyType( prop );
	switch( type ) {
	
		case PROP_CHARACTER: {
			P_CHAR pc = pointers::findCharBySerial( obj );
			VALIDATEPC(pc);

			switch( t ) {
				case T_CHAR : {
					char value = str2num( data );
					if( value!=getCharCharProperty( pc, prop, subProp ) )
						setCharCharProperty( pc, prop, subProp, subProp2, value );
					}
					break;
				case T_INT: {
					int value = str2num( data );
					if( value!=getCharIntProperty( pc, prop, subProp ) )
						setCharIntProperty( pc, prop, subProp, subProp2, value );
					}
					break;
				case T_SHORT: {
					short value = str2num( data );
					if( value!=getCharShortProperty( pc, prop, subProp ) )
						setCharShortProperty( pc, prop, subProp, subProp2, value );
					}
					break;
				case T_STRING: {
					std::string value;
					wstring2string( data, value );
					if( value!=std::string( getCharStrProperty( pc, prop, subProp ) ) )
						setCharStrProperty( pc, prop, subProp, subProp2, const_cast<char*>(value.c_str()) );
					}
					break;
				case T_UNICODE: {
					if( data!=getCharUniProperty( pc, prop, subProp ) )
						setCharUniProperty( pc, prop, subProp, subProp2, data );
					}
					break;
			}
		}
		break;
		case PROP_ITEM : {
			P_ITEM pi = pointers::findItemBySerial( obj );
			VALIDATEPI(pi);

			switch( t ) {
				case T_CHAR : {
					char value = str2num( data );
					if( value!=getItemCharProperty( pi, prop, subProp ) )
						setItemCharProperty( pi, prop, subProp, value );
					}
					break;
				case T_INT: {
					int value = str2num( data );
					if( value!=getItemIntProperty( pi, prop, subProp ) )
						setItemIntProperty( pi, prop, subProp, value );
					}
					break;
				case T_SHORT: {
					short value = str2num( data );
					if( value!=getItemShortProperty( pi, prop, subProp ) )
						setItemShortProperty( pi, prop, subProp, value );
					}
					break;
				case T_STRING: {
					std::string value;
					wstring2string( data, value );
					if( value!=std::string( getItemStrProperty( pi, prop, subProp ) ) )
						setItemStrProperty( pi, prop, subProp, const_cast<char*>(value.c_str()) );
					}
					break;
				case T_UNICODE:
					if( data!=getItemUniProperty( pi, prop, subProp ) )
						setItemUniProperty( pi, prop, subProp, data );
					break;
			}
		}
		break;
		case PROP_CALENDAR:
		case PROP_GUILD:
		default:
			return;
	}


}

template< typename T >
std::wstring toWstr( T num )
{
	wchar_t buffer[TEMP_STR_SIZE];
	swprintf( buffer, L"%i", num );
	return std::wstring( buffer );
}

std::wstring toWstr( const char* s )
{
	std::wstring data;
	string2wstring( std::string(s), data );
	return data;
}


std::wstring cMenu::getPropertyField( SERIAL type, SERIAL obj, SERIAL prop, SERIAL subProp, SERIAL subProp2 )
{
	
	VARTYPE t = getPropertyType( prop );
	switch( type ) {
	
		case PROP_CHARACTER: {
			P_CHAR pc = pointers::findCharBySerial( obj );
			VALIDATEPCR(pc, std::wstring());

			switch( t ) {
				case T_CHAR :
					return toWstr( getCharCharProperty( pc, prop, subProp ) );
				case T_INT:
					return toWstr( getCharIntProperty( pc, prop, subProp ) );
				case T_SHORT:
					return toWstr( getCharShortProperty( pc, prop, subProp ) );
				case T_STRING:
					return toWstr( getCharStrProperty( pc, prop, subProp ) );
				case T_UNICODE: 
					return getCharUniProperty( pc, prop, subProp );
			}
		}
		break;
		case PROP_ITEM : {
			P_ITEM pi = pointers::findItemBySerial( obj );
			VALIDATEPIR(pi, std::wstring());

			switch( t ) {
				case T_CHAR :
					return toWstr( getItemCharProperty( pi, prop, subProp ) );
				case T_INT:
					return toWstr( getItemCharProperty( pi, prop, subProp ) );
				case T_SHORT:
					return toWstr( getItemCharProperty( pi, prop, subProp ) );
				case T_STRING:
					return toWstr( getItemStrProperty( pi, prop, subProp ) );
				case T_UNICODE:
					return getItemUniProperty( pi, prop, subProp );
			}
		}
		break;
		case PROP_CALENDAR:
		case PROP_GUILD:
		default:
			return std::wstring(L"Not implemented yet");
	}


	return std::wstring( L"Error" );
}

void cMenu::setMoveable( bool canMove )
{
	if( !canMove )
		addCommand("{nomove}");
	else 
		if( !moveable )
			removeCommand( std::string("{nomove}") );
	moveable=canMove;
}

bool cMenu::getMoveable()
{
	return moveable;
}

void cMenu::setCloseable( bool canClose )
{
	if( !canClose )
		addCommand("{noclose}");
	else
		if( !closeable )
			removeCommand( std::string("{noclose}") );
	closeable=canClose;
}

bool cMenu::getCloseable()
{
	return closeable;
}

void cMenu::setDisposeable( bool canDispose )
{
	if( !canDispose )
		addCommand("{nodispose}");
	else
		if( !disposeable )
			removeCommand( std::string("{nodispose}") );
	disposeable=canDispose;
}

bool cMenu::getDisposeable()
{
	return disposeable;
}

bool cMenu::getCheckBox( SERIAL checkbox )
{
	return find( switchs->begin(), switchs->end(), checkbox )!=switchs->end();
}

bool cMenu::getRadio( SERIAL radio )
{
	return find( switchs->begin(), switchs->end(), radio )!=switchs->end();
}

std::wstring* cMenu::getText( SERIAL text )
{
	std::map< SERIAL, std::wstring >::iterator iter( textResp->find( text ) );
	if( iter!=textResp->end() )
		return &iter->second;
	else
		return NULL;
}



cServerPacket* cMenu::createPacket()
{
	cPacketMenu* packet = new cPacketMenu; //!< menu packet with new packet system

	packet->serial =serial;
	packet->id=id;
	packet->x = x;
	packet->y = y;

	packet->commands = &commands;
	packet->texts = &texts;

	return packet;
}





cIconListMenu::cIconListMenu() : cBasicMenu( MENUTYPE_ICONLIST )
{
}

cIconListMenu::~cIconListMenu()
{
}

cServerPacket* cIconListMenu::createPacket() 
{
	cPacketIconListMenu* p = new cPacketIconListMenu;
	
	p->serial = serial;
	p->id = id;

	p->question = question;
	p->icons = &icons;

	return p;
}

void cIconListMenu::addIcon( UI16 model, COLOR color, std::string response )
{
	pkg_icon_list_menu_st icon;
	icon.color=color;
	icon.model=model;
	icon.response=response;

	icons.push_back( icon );
}

void cIconListMenu::handleButton( NXWCLIENT ps,  cClientPacket* pkg  )
{
	cPacketResponseToDialog* p = (cPacketResponseToDialog*)pkg;

	callback->Call( ps->toInt(), serial, p->index.get(), p->model.get(), p->color.get() );

}

