  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "network.h"
#include "amx/amxcback.h"
#include "nxwGump.h"

map<UI32, class nxwGump* > nxwGump::gumpMap;
map<UI32, std::string > nxwGump::responseMap;

//LOGICAL nxwGump::createGump(UI32 gump, UI32 x, UI32 y, UI32 noMove, UI32 noClose, UI32 noDispose, std::string amxCallBack)
//{
//	return nxwGump( gump, x, y, noMove == 0 ? false : true, noClose == 0 ? false : true, noDispose == 0 ? false : true, amxCallBack );
//}

LOGICAL nxwGump::createGump(UI32 gump, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose, UI32 serial)
{
	if( gumpMap.find( gump ) == gumpMap.end() )
	{
		nxwGump::gumpMap[ gump ] = new nxwGump( gump, x, y, canMove, canClose, canDispose, serial );
		return true;
	}

	map< UI32, std::string >::iterator it( responseMap.find( gump ) );
	if ( it != responseMap.end() )
		responseMap.erase( it );

	return false;
}

LOGICAL	nxwGump::deleteGump( UI32 gump )
{
	if( gumpMap.find( gump ) != gumpMap.end() )
	{
		gumpMap.erase( gump );
		return true;
	}

	return false;
}

nxwGump* nxwGump::selectGump( UI32 gump )
{
	if( gumpMap.find( gump ) != gumpMap.end() )
		return gumpMap[ gump ];
	return NULL;	
}

LOGICAL nxwGump::showGump( UI32 gump, P_CHAR pc )
{
	nxwGump* pGump = selectGump( gump );
	if( pGump )
	{
		pGump->show( pc );
		return 1;
	}
	return 0;
}

LOGICAL nxwGump::handleGump( const P_CHAR pc, const UI08 *data )
{
	if( data[0] == 0xB1 )
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
                if ( gump < 1000 ) { // Luxor: NoX-Wizard small gumps
			switch( gump )
			{
				case   1	:	// character properties
				case   2	:	// item properties
				case  20	:	// region choice list
				case  21	:	// region properties
				case  50	:	// guild stone menu
				case  51	:	// guild members
				case  52	:	// guild recruits
				case  53	:	// guild properties
				case 999	:
					newAmxEvent("gui_handleResponse")->Call( gump, gumpSerial, button, pc->getSerial32() );
					break;
				default:
					return false;
					break;
			}
		} else { // Luxor: user defined small gumps
                        map< UI32, std::string >::iterator it( responseMap.find( gump ) );
                        if ( it == responseMap.end() )
				return false;
			string sCallback( it->second );
			AmxEvent *callback;
			callback = newAmxEvent( const_cast< char* >( sCallback.c_str() ) );
			callback->Call( gump, gumpSerial, button, pc->getSerial32() );

			responseMap.erase( it );
		}			
		return true;
	}
	else
		return false;
}

LOGICAL nxwGump::selectResponse( UI32 fieldId, std::string &value )
{
	if( responseMap.find( fieldId ) != responseMap.end() )
	{
		value = responseMap[ fieldId ];
		return true;
	}
	else
		return false;
}

LOGICAL	nxwGump::addResponse( UI32 fieldId, std::string value )
{
	if( responseMap.find( fieldId ) != responseMap.end() )
		return false;
	else
	{
		responseMap[ fieldId ] = value;
		return true;
	}
}

nxwGump::nxwGump()
{
	setGumpId( 0 );
	setX( 0 );
	setY( 0 );
	setMoveAble( true );
	setCloseAble( true );
	setDisposeAble( true );
	setSerial( 0 );
	setDirty();
	packet = NULL;
}

nxwGump::nxwGump( UI32 gump, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose, UI32 serial )
{
	setGumpId( gump );
	setX( x );
	setY( y );
	setMoveAble( canMove );
	setCloseAble( canClose );
	setDisposeAble( canDispose );
	setSerial( serial );
	setDirty();
	packet = 0;
}

nxwGump::~nxwGump()
{
	dropPacket();
}

/*!
\author Luxor
*/
void nxwGump::setCallBack( const std::string& arg )
{
	responseMap[ gumpId ] = string( arg );
}

void nxwGump::addCommand( const std::string& command )
{
	setDirty();
	commands.append( command );
}

void nxwGump::addString( const std::string& text )
{
	setDirty();
	strings.append( text );
}

void nxwGump::addButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 returnCode )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{button %d %d %d %d 1 0 %d}", x, y, up, down, returnCode )  ) );
}


void nxwGump::addPageButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 page )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{button %d %d %d %d 0 %d 0}", x, y, up, down, page )  ) );
}
void nxwGump::addGump( UI32 x, UI32 y, UI32 gump, UI32 hue )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{gumppic %d %d %d hue=%d}", x, y, gump, hue )  ) );
}

/*
void nxwGump::addGump( UI32 x, UI32 y, UI32 gump, SI32 hue )
{
	setDirty();
	if ( hue >= 0 )
		commands.append( nxwString( nxwString::format( "{gumppic %d %d %d hue=%d}", x, y, gump, hue )  ) );
	else
		commands.append( nxwString( nxwString::format( "{gumppic %d %d %d}", x, y, gump )  ) );
}
*/
/*
void nxwGump::addTiledGump( UI32 x, UI32 y, UI32 gump, SI32 hue, SI32 width, SI32 height )
{
	setDirty();
	if( hue >= 0 )
		commands.append( nxwString( nxwString::format( "{gumppictiled %d %d %d %d %d %d}", x, y, gump, width, height, hue )  ) );
	else
		commands.append( nxwString( nxwString::format( "{gumppictiled %d %d %d %d %d}", x, y, gump, width, height )  ) );
}
*/

void nxwGump::addTiledGump( UI32 x, UI32 y, UI32 width, UI32 height, UI32 gump, UI32 hue )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{gumppictiled %d %d %d %d %d %d}", x, y, width, height, gump, hue )  ) );
}

void nxwGump::addHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, const char* html, UI32 hasBack, UI32 canScroll )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{htmlgump %d %d %d %d %d %d %d}", x, y, width, height, strings.append(html), hasBack, canScroll )  ) );
}

void nxwGump::addXmfHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, const char* clilocid, UI32 hasBack , UI32 canScroll )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{xmfhtmlgump %d %d %d %d %s %d %d}", x, y, width, height, strings.append(clilocid), hasBack, canScroll )  ) );
}

void nxwGump::addCheckertrans( UI32 x, UI32 y, UI32 width, UI32 height )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{checkertrans %d %d %d %d}", x, y, width, height )  ) );
}

void nxwGump::addCroppedText( UI32 x, UI32 y, UI32 width, UI32 height, const char* text, UI32 hue )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{croppedtext %d %d %d %d %d %d}", x, y, width, height, hue, strings.append(text) )  ) );
}

void nxwGump::addPage( UI32 page )
{
	if ( page < 256 )
	{
		setDirty();
		commands.append( nxwString( nxwString::format( "{page %d}", page )  ) );
	}
}

void nxwGump::addGroup( UI32 group )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{group %d}", group )  ) );
}

void nxwGump::addText( UI32 x, UI32 y, const char* data, UI32 hue )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{text %d %d %d %d}", x, y, hue, strings.append(data) )  ) );
}

void nxwGump::addBackground( UI32 gump, UI32 width, UI32 height )
{
	setDirty();
	addResizeGump( 0, 0, gump, width, height );
}

void nxwGump::addResizeGump( UI32 x, UI32 y, UI32 gump, UI32 width, UI32 height )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{resizepic %d %d %d %d %d}", x, y, gump, width, height )  ) );
}

void nxwGump::addTilePic( UI32 x, UI32 y, UI32 tile, UI32 hue )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{tilepic %d %d %d %d}", x, y, tile, hue )  ) );
}

void nxwGump::addInputField( UI32 x, UI32 y, UI32 width, UI32 height, UI32 textId, const char* data, UI32 hue )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{textentry %d %d %d %d %d %d %d}", x, y, width, height, hue, textId, strings.append(data) )  ) );
}

void nxwGump::addCheckbox( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{checkbox %d %d %d %d %d %d}", x, y, off, on, result, checked )  ) );
}

void nxwGump::addRadioButton( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result  )
{
	setDirty();
	commands.append( nxwString( nxwString::format( "{radio %d %d %d %d %d %d}", x, y, off, on, checked, result )  ) );
}

void nxwGump::createPacket()
{
	const int generalHeaderLength = 19;
	const int commandHeaderLength =  2;
	const int commandFooterLength	=  1;
	const int textHeaderLength		=  2;
	const int stringHeaderLength	=  2;
	const int unicodeModifier			=  2;

	int commandLength = commands.size() + commandFooterLength;
	int textLines			= strings.count();
	int textLength		= strings.size() * unicodeModifier + textLines * stringHeaderLength;
	int packetLength 	= generalHeaderLength + commandHeaderLength + commandLength + textHeaderLength + textLength;
	packet = new BYTE[ packetLength ];
	// packet id
	packet[ 0] = 0xB0;
	// packet length
	packet[ 1] = packetLength >> 8;
	packet[ 2] = packetLength % 256;
	// serial
	packet[ 3] = serial >> 24;
	packet[ 4] = serial >> 16;
	packet[ 5] = serial >>  8;
	packet[ 6] = serial % 256;
	// gump id
	packet[ 7] = gumpId >> 24;
	packet[ 8] = gumpId >> 16;
	packet[ 9] = gumpId >>  8;
	packet[10] = gumpId % 256;
	// x offset
	packet[11] = x >> 24;
	packet[12] = x >> 16;
	packet[13] = x >>  8;
	packet[14] = x % 256;
	// y offset
	packet[15] = y >> 24;
	packet[16] = y >> 16;
	packet[17] = y >>  8;
	packet[18] = y % 256;
	// command length
	packet[19] = commandLength >> 8;
	packet[20] = commandLength % 256;
	// commands
	int index = 21;
	UI32NXWSTRINGMAP::iterator it( commands.begin() ), end( commands.end() );
	for(; it != end; ++it)
	{
		memcpy( packet + index, it->second.getValue().c_str(), it->second.size() );
		index += it->second.size();
	}
	// command section end
	packet[index] = 0;
	// text length
	packet[++index] = textLines >> 8;
	packet[++index] = textLines % 256;

	// texts in unicode format

	int ssize = 0;
	it = strings.begin();
	end = strings.end();
	for(; it != end; ++it)
	{
		ssize = it->second.size();
		packet[++index] = ssize >> 8;
		packet[++index] = ssize % 256;
		for( int strIndex = 0; strIndex < ssize; ++strIndex )
		{
			packet[++index] = 0;
			packet[++index] = it->second.getValue()[strIndex];
		}
	}

}

void nxwGump::dropPacket()
{
	safedeletearray(packet);
}

bool nxwGump::isDirty()
{
	return dirty;
}

void nxwGump::resetDirty()
{
	dirty = false;
}

void nxwGump::setDirty()
{
	dirty = true;
}

void nxwGump::setSerial( const UI32 arg )
{
	serial = arg;
}

void nxwGump::handleButton( const NXWSOCKET socket, const UI32 button )
{
//	callback->Call( socket, button );
}

void nxwGump::print()
{
	commands.print();
	strings.print();
}

void nxwGump::setMoveAble( const bool arg )
{
	moveable = arg;
	if (!moveable)
		addCommand("{nomove}");
}

void nxwGump::setCloseAble( const bool arg )
{
	closeable = arg;
	if (!closeable)
		addCommand("{noclose}");
}

void nxwGump::setDisposeAble( const bool arg )
{
	disposeable = arg;
	if (!disposeable)
		addCommand("{nodispose}");
}

void nxwGump::setGumpId( const UI32 arg )
{
	gumpId = arg ;
}

void nxwGump::setX( const UI32 arg )
{
	x = arg;
}

void nxwGump::setY( const UI32 arg )
{
	y = arg;
}

void nxwGump::show( P_CHAR pc )
{
	// <Luxor>
	NXWSOCKET socket = pc->getSocket();
	if ( socket == INVALID )
		return;
	// </Luxor>
	if( isDirty() )
	{
		if( packet != 0 )
			dropPacket();
		createPacket();
	}
	SI32 packetLength = ((packet[1] << 8) + packet[2]);
	UI32 packetIndex	= 0;
	while( packetLength > 0 )
	{		
		Xsend( socket, (packet + packetIndex), packetLength > MAXBUFFER ? MAXBUFFER : packetLength );
		packetLength -= MAXBUFFER;
		packetIndex += MAXBUFFER;
	}
	//Xsend( pc->getSocket(), packet, ((packet[1] << 8) + packet[2]) );
}
