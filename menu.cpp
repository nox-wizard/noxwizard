  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "menu.h"


MENU_MAP cMenu::menuMap;
map<UI32, std::string > cMenu::responseMap;


#define OPTIONS2BITSET( MOVE, CLOSE, DISPOSE ) \
	( MOVE*MOVEABLE | CLOSE*CLOSEABLE | DISPOSE*DISPOSEABLE  ) \


LOGICAL cMenu::createGump(UI32 gump, UI32 x, UI32 y, UI08 options, UI32 serial)
{
	if( menuMap.find( gump ) == menuMap.end() )
	{
		menuMap.insert( make_pair( gump, new cMenu( gump, x, y, options, serial ) ) );
		return true;
	}
	return false;
}

LOGICAL	cMenu::createGump(UI32 gump, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose, UI32 serial)
{
	return createGump( gump, x, y, OPTIONS2BITSET(canMove, canClose, canDispose), serial );
}


LOGICAL	cMenu::deleteGump( UI32 gump )
{
	MENU_MAP::iterator iter( menuMap.find( gump ) );
	if( iter != menuMap.end() )
	{
		menuMap.erase( iter );
		return true;
	}
	return false;
}

cMenu* cMenu::selectGump( UI32 gump )
{
	MENU_MAP::iterator iter( menuMap.find( gump ) );
	if( iter != menuMap.end() )
		return iter->second;
	return NULL;	
}

LOGICAL cMenu::showGump( UI32 gump, P_CHAR pc )
{
	MENU_MAP::iterator iter( menuMap.find( gump ) );
	if( iter != menuMap.end() )
	{
		iter->second->show( pc );
		return true;
	}
	return false;	
}

LOGICAL cMenu::handleGump( const P_CHAR pc, const UI08 *data )
{
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
		return false;
}

LOGICAL cMenu::selectResponse( UI32 fieldId, std::string &value )
{
	map<UI32, std::string >::iterator iter( responseMap.find( fieldId ) );
	if( iter != responseMap.end() )
	{
		value = iter->second;
		return true;
	}
	else
		return false;
}

LOGICAL	cMenu::addResponse( UI32 fieldId, std::string value )
{
	if( responseMap.find( fieldId ) != responseMap.end() )
		return false;
	else
	{
		responseMap.insert( make_pair( fieldId, value ) );
		return true;
	}
}

cMenu::cMenu()
{
	cMenu( 0, 0, 0, MOVEABLE|CLOSEABLE|DISPOSEABLE, 0 );
}

cMenu::cMenu( UI32 gump, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose, UI32 serial )
{
	cMenu( gump, x, y, OPTIONS2BITSET(canMove, canClose, canDispose), serial );
}

cMenu::cMenu( UI32 gump, UI32 x, UI32 y, UI08 options, UI32 serial )
{
	setGumpId( gump );
	setX( x );
	setY( y );
	setOptions( options );
	setSerial( serial );
}

cMenu::~cMenu()
{
	
}

void cMenu::addCommand( const std::string& command )
{
	commands.push_back( command );
}

void cMenu::addCommand( char const *formatStr, ... )
{
	std::string s("");
	
	va_list vargs;
	va_start(vargs, formatStr );
	vsnprintf( (char*)s.c_str(), s.size()-1, formatStr, vargs);
	va_end(vargs);

	addCommand( s );
}


UI32 cMenu::addString( cUnicodeString& u )
{
	texts.push_back( u );
	return texts.size()-1;
}



void cMenu::addButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 returnCode )
{
	addCommand( "{button %d %d %d %d 1 0 %d}", x, y, up, down, returnCode );
}


void cMenu::addPageButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 page )
{
	addCommand( "{button %d %d %d %d 0 %d 0}", x, y, up, down, page );
}

void cMenu::addGump( UI32 x, UI32 y, UI32 gump, UI32 hue )
{
	addCommand( "{gumppic %d %d %d hue=%d}", x, y, gump, hue );
}

void cMenu::addTiledGump( UI32 x, UI32 y, UI32 width, UI32 height, UI32 gump, UI32 hue )
{
	addCommand( "{gumppictiled %d %d %d %d %d %d}", x, y, width, height, gump, hue );
}

void cMenu::addHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, cUnicodeString& html, UI32 hasBack, UI32 canScroll )
{
	addCommand( "{htmlgump %d %d %d %d %d %d %d}", x, y, width, height, addString(html), hasBack, canScroll );
}

void cMenu::addXmfHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, cUnicodeString& clilocid, UI32 hasBack , UI32 canScroll )
{
	addCommand( "{xmfhtmlgump %d %d %d %d %s %d %d}", x, y, width, height, addString(clilocid), hasBack, canScroll );
}

void cMenu::addCheckertrans( UI32 x, UI32 y, UI32 width, UI32 height )
{
	addCommand( "{checkertrans %d %d %d %d}", x, y, width, height );
}

void cMenu::addCroppedText( UI32 x, UI32 y, UI32 width, UI32 height, cUnicodeString& text, UI32 hue )
{
	addCommand( "{croppedtext %d %d %d %d %d %d}", x, y, width, height, hue, addString(text) );
}

void cMenu::addPage( UI32 page )
{
	if ( page < 256 )
	{
		addCommand( "{page %d}", page );
	}
}

void cMenu::addGroup( UI32 group )
{
	addCommand( "{group %d}", group );
}

void cMenu::addText( UI32 x, UI32 y, cUnicodeString& data, UI32 hue )
{
	addCommand( "{text %d %d %d %d}", x, y, hue, addString(data) );
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

void cMenu::addInputField( UI32 x, UI32 y, UI32 width, UI32 height, UI32 textId, cUnicodeString& data, UI32 hue )
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



void cMenu::setSerial( const UI32 arg )
{
	serial = arg;
}

void cMenu::handleButton( const NXWSOCKET socket, const UI32 button )
{
	callback->Call( socket, button );
}

void cMenu::setCallBack( const std::string& arg )
{
	callback = newAmxEvent( const_cast< char* >( arg.c_str() ) );
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





void cMenu::setGumpId( const UI32 arg )
{
	gumpId = arg ;
}

void cMenu::setX( const UI32 arg )
{
	x = arg;
}

void cMenu::setY( const UI32 arg )
{
	y = arg;
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
	packet.gump=gumpId;
	packet.x = x;
	packet.y = y;

	packet.commands = &commands;
	packet.texts = &texts;
	
	packet.send( ps );
}
