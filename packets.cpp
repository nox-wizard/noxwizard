  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "packets.h"
#include "network.h"
#include "globals.h"
#include "chars.h"
#include "party.h"

wstring emptyUnicodeString;
char stringTerminator = 0x00;
char unicodeStringTerminator[2] = { 0x00, 0x00 };



/*!
\brief get pointer at first valid position in packet ( headerSize is used internal )
\author Endymion
\since 0.83
*/
char* cPacket::getBeginValid() {
	return (char*)(&this->cmd);
}

/*!
\brief get From Client
\author Endymion
\since 0.83a
\param socket the socket
\param b the data
\param size the number of byte to read
\param from offset
*/
inline void reciveFromSocket( NXWSOCKET socket, char* b, int size, int& from )
{
	memcpy( b, &buffer[socket][from], size );
	from+=size;
};

/*!
\brief read a string from socket buffer
\author Endymion
\since 0.83a
\param socket the socket
\param s the string
\param lenght the length of need to read
\param from offset
*/
inline void reciveStringFromSocket( NXWSOCKET socket, string& s, int lenght, int& from ) {
	from-=1;
	while( buffer[socket][++from]!=0 ) {
		s+=buffer[socket][from];
	}
}

/*!
\brief read an unicode string from socket buffer
\author Endymion
\since 0.83a
\param s the socket
\param c the string
\param from offset
\param size read until NULL termination if INVALID, else read size char
*/
inline void reciveUnicodeStringFromSocket( NXWSOCKET s, wstring& c, int& from, int size )
{
	SI32 chSize = sizeof( UI16 );
	UI16* w=(UI16*)( &buffer[s][from] );

	c.erase();

	SI32 i=0;
	if( size==INVALID ) {//until termination
		while ( w[i]!=0 ) {
			c+=ntohs( w[i++] );
		}
	}
	else { //until size
		while( i<size ) {
			c+=ntohs( w[i++] );
		}
	}
	if( size==INVALID )
		from+=c.size()*chSize+chSize;
	else
		from+=size*chSize;

}

/*!
\brief get pointer after the packet command ( it's read before )
\author Endymion
\since 0.83
*/
char* cClientPacket::getBeginValidForReceive() {
	return ( getBeginValid() +sizeof(cmd) );
}


/*!
\brief get From Client
\author Endymion
\since 0.83a
\param socket the socket
\param b the data
\param size the number of byte to read
\param from offset
*/
void cClientPacket::getFromSocket( NXWSOCKET socket, char* b, int size )
{
	reciveFromSocket( socket, b, size, offset );
};

/*!
\brief read a string from socket buffer
\author Endymion
\since 0.83a
\param socket the socket
\param s the string
\param lenght the length of need to read
\param from offset
*/
void cClientPacket::getStringFromSocket( NXWSOCKET socket, string& s, int length ) 
{
	reciveStringFromSocket( socket, s, length, offset );
}

/*!
\brief read an unicode string from socket buffer
\author Endymion
\since 0.83a
\param s the socket
\param c the string
\param from offset
\param size read until NULL termination if INVALID, else read size char
*/
void cClientPacket::getUnicodeStringFromSocket( NXWSOCKET socket, wstring& c, int size )
{
	reciveUnicodeStringFromSocket( socket, c, offset, size );
}

/*!
\brief Receive packet from client
\author Endymion
\since 0.83a
\param ps the client who send this packet
\attention NOT WRITE THE CMD, it's read before
*/
void cClientPacket::receive( NXWCLIENT ps ) {
	offset=1;
	if ( ps != NULL )
		getFromSocket( ps->toInt(), getBeginValidForReceive(), headerSize-1 );
};

/*!
\brief Send packet to client
\author Endymion, rewritten by Luxor
\since 0.83a
\param ps the client who send to
*/
void cServerPacket::send( NXWCLIENT ps ) {
	if( ps == NULL )
		return;

	Xsend( ps->toInt(), getBeginValid(), headerSize );
};


/*!
\brief Send packet to player
\author Endymion
\since 0.83a
\param pc the player who send to
*/
void cServerPacket::send( P_CHAR pc ) {
	VALIDATEPC( pc )
	send( pc->getClient() );
};








//@{
/*!
\brief packet macro
\author Endymion
\since 0.83a
*/

#define CREATE( NAME, CMD, SIZE ) \
cPacket##NAME::cPacket##NAME() { \
	cmd = CMD; \
	headerSize = SIZE; \
};

#define SEND( NAME ) void cPacket##NAME::send( NXWCLIENT ps )

#define RECEIVE( NAME ) void cPacket##NAME::receive( NXWCLIENT ps )
//@}



CREATE( CreateCharacter, PKG_CREATE_CHARACTER, 0x0A )
RECEIVE( CreateCharacter ) {
	/*if( ps == NULL ) return; //after error here
	getFromSocket( ps->toInt(), this->getBeginValidForReceive(), this->headerSize -1 );
	getStringFromSocket( ps->toInt(), this->name, 30 ); 	
	getStringFromSocket( ps->toInt(), this->passwd, 30 );
	getFromSocket( ps->toInt(), (char*)(&this->sex), 30 );*/
};

CREATE( DisconnectNotification, PKG_DISCONNECT_NOTIFY, 0x05 )

CREATE( TalkRequest, PKG_TALK_REQUEST, 0x08 )
RECEIVE( TalkRequest ) {
	/*if( ps == NULL ) return; //after error here
	offset=1;
	getFromSocket( ps->toInt(), this->getBeginValidForReceive(), this->headerSize -1 );
	getStringFromSocket( ps->toInt(), this->msg, this->size-0x08 ); 	*/
};

CREATE( GodModeToggle, PKG_GODMODE_TOGGLE, 0x02  )

CREATE( AttackRequest, PKG_ATTACK_REQUEST, 0x05 )

CREATE( DoubleClick, PKG_DB_CLICK, 0x05 )

CREATE( PickUpItem, PKG_PICK_UP, 0x07 )

CREATE( DropItem, PKG_DROP_ITEM, 0x0E )

CREATE( SingleClick, PKG_SINGLE_CLICK, 0x05 )

CREATE( StatWindow, PKG_STAT_WINDOW, 0x2B )
SEND( StatWindow ) {
	this->sendBasic( ps );
};

void cPacketStatWindow::sendBasic( NXWCLIENT ps, UI08 flag  ) {
	if( ps == NULL ) return; //after error here
	this->flag = flag;
	Xsend( ps->toInt(), this->getBeginValid(), this->headerSize );
};

void cPacketStatWindow::sendStat( NXWCLIENT ps ) {
	if( ps == NULL ) return; //after error here
	sendBasic( ps, 0x01 );
	Xsend( ps->toInt(), (char*)(&this->sex), 0x17 );
};

void cPacketStatWindow::sendExtended( NXWCLIENT ps ) {
	if( ps == NULL ) return; //after error here
	sendBasic( ps, 0x03 );
	Xsend( ps->toInt(), (char*)(&this->sex), 0x1B );
};

CREATE( Wear, PKG_DROP_OR_WEAR_ITEM, 0x0A )

CREATE( CharInfo, PKG_CHAR_INFO, 0x25 )

CREATE( Speech, PKG_SPEECH, 0x0E )
SEND( Speech ) {
	if( ps == NULL ) return; //after error here
	this->size=this->headerSize +30 +msg->length()+1;
	Xsend( ps->toInt(), this->getBeginValid(), this->headerSize );
	this->name.resize( 30 );
	Xsend( ps->toInt(), this->name.c_str(), 30 );
	Xsend( ps->toInt(), this->msg->c_str(), this->msg->length()+1 );
};

CREATE( Delete, PKG_DELETE, 0x05 )

CREATE( DrawPlayer, PKG_DRAW_PLAYER, 0x13 )

CREATE( MoveReject, PKG_REJECT_MOVEMENT, 0x08 )

CREATE( MoveACK, PKG_MOVE_ACK, 0x03 )

CREATE( Dragging, PKG_DRAGGING, 0x1A )

CREATE( DrawContainer, PKG_DRAW_CONTAINER, 0x07 )

CREATE( AddItemIntoCont, PKG_ADD_ITEM_INTO_CONTAINER, 0x14 )

CREATE( Kick, PKG_KICK, 0x05 )

CREATE( RejectMoveItem, PKG_REJECT_MOVE_ITEM, 0x02 )

CREATE( ClearSquare, PKG_CLEAR_SQUARE, 0x05 )

CREATE( PDClothing, PKG_PD_CLOTHING, 0x01 )

CREATE( RessMenu, PKG_RESS_MENU, 0x02 )

CREATE( MobileAttributes, PKG_MOBILE_ATTRIBUTES, 0x11 )

CREATE( WornItem, PKG_WORN_ITEM, 0x0F )

CREATE( FightOccuring, PKG_FIGHT_OCCURING, 0x0A )

CREATE( PauseResumeClient, PKG_PAUSE_RESUME, 0x02 )

CREATE( PlayerStatus, PKG_PLAYER_STATUS, 0x0A )

CREATE( Buy, PKG_BUY, 0x08 )
SEND( Buy ) {
	if( ps == NULL ) return; //after error here
	this->size=this->headerSize +list.size()*sizeof( buyitem_st );
	if( list.size() == 0 )
		this->flag=0x00;
	else
		this->flag=0x02;
	Xsend( ps->toInt(), this->getBeginValid(), this->headerSize );
	for( vector<buyitem_st>::iterator iter = list.begin(); iter!=list.end(); iter++ )
		Xsend( ps->toInt(), (char *)&(*iter), sizeof( buyitem_st ) );
}

CREATE( Container, PKG_CONTAINER, 0x05 )
SEND( Container ) {
	if( ps == NULL ) return; //after error here
	this->size=this->headerSize +list.size()*sizeof( itemincont_st );
	this->n=list.size();
	Xsend( ps->toInt(), this->getBeginValid(), this->headerSize );
	for( vector<itemincont_st>::iterator iter = list.begin(); iter!=list.end(); iter++ )
		Xsend( ps->toInt(), (char *)&(*iter), sizeof( itemincont_st ) );
}

CREATE( PersonalLight, PKG_PERSONAL_LIGHT, 0x06 )

CREATE( OverallLight, PKG_OVERALL_LIGHT, 0x02 )

CREATE( IdleWarning, PKG_IDLE_WARNING, 0x02 )

CREATE( Sound, PKG_SOUND, 0x0C )

CREATE( LoginComplete, PKG_LOGIN_COMPLETE, 0x01 )

CREATE( PlotCourse, PKG_PLOT_COURSE, 0x0B )

CREATE( Time, PKG_TIME, 0x04 )

CREATE( Login, PKG_LOGIN, 0x05 )
RECEIVE( Login ) {
/*	if( ps == NULL ) return; //after error here
	offset=1;
	getFromSocket( ps->toInt(), this->getBeginValidForReceive(), this->headerSize -1 );
	getStringFromSocket( ps->toInt(), this->name, 30 ); 	
	getStringFromSocket( ps->toInt(), this->passwd, 30 );
	getFromSocket( ps->toInt(), (char*)(&this->slot), 8 );*/
}

CREATE( Weather, PKG_WEATHER, 0x04 )

CREATE( Midi, PKG_MIDI, 0x03 )

CREATE( Anim, PKG_ANIM, 0x0E )

CREATE( Effect, PKG_EFFECT, 0x1C )

CREATE( WarMode, PKG_WAR_MODE, 0x05 )
/*
CREATE( OpenBuy, PKG_OPEN_BUY, 0x08 )
void cPacketOpenBuy::send( NXWCLIENT ps ) {
	if( ps == NULL ) return; //after error here
	this->size=this->headerSize +list.size()*sizeof( openbuyitem_st );
	this->n=list.size();
	Xsend( ps->toInt(), this->getBeginValid(), this->headerSize );
	for( vector<openbuyitem_st>::iterator iter = list.begin(); iter!=list.end(); iter++ )
		Xsend( ps->toInt(), (char*)iter, sizeof( openbuyitem_st ) );
}
*/

CREATE( Rename, PKG_RENAME, 0x05 )
SEND( Rename ) {
	if( ps == NULL ) return; //after error here
	Xsend( ps->toInt(), this->getBeginValid(), this->headerSize );
	this->newname.resize(30);
	Xsend( ps->toInt(), this->newname.c_str(), 30 );
}

CREATE( NewSubserver, PKG_NEW_SUBSERVER, 0x10 )

CREATE( UpdatePlayer, PKG_UPDATE_PLAYER, 0x11 )

CREATE( ResponseToDialog, PKG_RESPONSE_TO_DIALOG, 0x0D )

CREATE( LoginDenied, PKG_LOGIN_DENIED, 0x02 )

CREATE( DeleteCharacter, PKG_DELETE_CHARACHTER, 0x01 )
RECEIVE( DeleteCharacter ) {
/*	if( ps == NULL ) return; 
	offset=1;
	getFromSocket( ps->toInt(), this->getBeginValidForReceive(), this->headerSize -1 ); // nothing.. remove?
	getStringFromSocket( ps->toInt(), this->passwd, 30 ); 	
	getFromSocket( ps->toInt(), (char*)(&this->idx), 8 );*/
}


/// v----- only working packets


CREATE( UnicodeSpeech, PKG_UNICODE_SPEECH, 0x12 )
SEND( UnicodeSpeech ) {
	if( ps == NULL || msg == NULL ) return;
	SI32 nSize = sizeof( UI16 );
	this->size=this->headerSize +30 + msg->size()*nSize+nSize;
	Xsend( ps->toInt(), this->getBeginValid(), this->headerSize );
	this->name.resize( 30 );
	Xsend( ps->toInt(), this->name.c_str(), 30 );
	Xsend( ps->toInt(), *msg, true );
};

CREATE( Map, PKG_MAP, 0x0B )
CREATE( MapMessage, PKG_MAP_MESSAGE, 0x13 )

CREATE( Walk, PKG_WALK, 0x02 )
CREATE( WalkAck, PKG_WALK_ACK, 0x03 )
CREATE( WalkReject, PKG_WALK_REJECT, 0x08 )

CREATE( CharProfileReqOnly, PKG_CHAR_PROFILE, 0x08 )
CREATE( CharProfileReq, PKG_CHAR_PROFILE, 0x08 )
RECEIVE( CharProfileReq ) {
	if( ps == NULL ) return; 
	NXWSOCKET s=ps->toInt();

	offset=1;
	getFromSocket( s, this->getBeginValidForReceive(), this->headerSize-1 );
	if( update ) { //complete packet so
		getFromSocket( s, (char*)&this->type, sizeof(type)+sizeof(len) );
		getUnicodeStringFromSocket( s, this->profile, len.get() );
	}
}

CREATE( CharProfile, PKG_CHAR_PROFILE, 0x07 )
SEND( CharProfile ) {
	if( ps == NULL ) return; 
	if( profile==NULL ) profile=&emptyUnicodeString;
	if( staticProfile==NULL ) staticProfile=&emptyUnicodeString;
	this->size=this->headerSize +(title.size()+1) + profile->size()*2+2 + staticProfile->size()*2+2;
	Xsend( ps->toInt(), this->getBeginValid(), this->headerSize );
	Xsend( ps->toInt(), this->title.c_str(), title.size()+1 );
	Xsend( ps->toInt(), *staticProfile, true );
	Xsend( ps->toInt(), *profile, true );
}

CREATE( Features, PKG_FEATURES, 0x03 )

CREATE( WebBrowser, PKG_WEB_BROWSER, 0x03 )
SEND( WebBrowser ) {
	if( ps == NULL ) return; 
	this->size=this->headerSize + (link.size()+1 );
	Xsend( ps->toInt(), this->getBeginValid(), this->headerSize );
	Xsend( ps->toInt(), this->link.c_str(), link.size()+1 );
}

CREATE( Menu, PKG_MENU, 0x15 )
SEND( Menu ) {

	if( ps == NULL ) return; 
	NXWSOCKET s = ps->toInt();

	//calc of packet size
	UI32 temp=this->headerSize;

	//command string
	UI32 size_of_commands=0;
	std::vector< std::string >::iterator its( commands->begin() ), ends( commands->end() );
	for( ; its!=ends; its++ ) {
		size_of_commands += its->length();
	}
	++size_of_commands; // terminator of command string
	this->cmd_length=size_of_commands;

	temp+=size_of_commands;
	temp+=sizeof( numTextLines );

	std::vector< wstring >::iterator itu( texts->begin() ), endu( texts->end() );
	for( ; itu!=endu; itu++ ) {
		temp += itu->size()*2 +sizeof( len );
	}
	
	this->size=temp;
	
	this->numTextLines=texts->size();

	//send of header
	Xsend( s, this->getBeginValid(), this->headerSize );
	
	//send of command string
	its = commands->begin();
	for( ; its!=ends; its++ ) {
		Xsend( s, its->c_str(), its->size() );
	}
	Xsend( s, &stringTerminator, sizeof( stringTerminator ) );


	//send of text
	numTextLines=texts->size();
	Xsend( s, (char*)&numTextLines, sizeof( numTextLines ) );

	itu = texts->begin();
	for( ; itu!=endu; itu++ ) {
		len=itu->length();
		Xsend( s, (char*)&len, sizeof( len ) );
		Xsend( s, *itu, false ); //not send null terminator
	}

}

CREATE( MenuSelection, PKG_MENU_SELECTION, 0x13 )
RECEIVE( MenuSelection ) {
	if( ps == NULL ) return; 
	NXWSOCKET s=ps->toInt();
	
	offset=1;
	getFromSocket( s, this->getBeginValidForReceive(), this->headerSize-1 );
	
	int si = switchcount.get();
	while( si-- ) {
		eSERIAL sw;
		getFromSocket( s, (char*)&sw, sizeof(sw) );
		switchs.push_back( sw.get() );
	}

	getFromSocket( s, (char*)&textcount, sizeof(textcount) );

	int ti=textcount.get();
	while( ti-- ) {
		text_entry_st te;
		getFromSocket( s, (char*)&te, sizeof(te.id)+sizeof(te.textlength) );
		
		std::wstring entry;
		getUnicodeStringFromSocket( s, entry, te.textlength.get() );

		text_entries.insert( make_pair( te.id.get(), entry ) );
	}
}


CREATE( IconListMenu, PKG_ICON_LIST_MENU, 0x0A )
SEND( IconListMenu ) {

	if( ps == NULL ) return; 
	NXWSOCKET s = ps->toInt();

	//calc of packet size
	UI32 temp=this->headerSize;

	//question string
	temp+=question.size();
	this->question_length=question.size();

	
	temp+=sizeof( icon_count );

	std::vector< pkg_icon_list_menu_st >::iterator iter( icons->begin() ), end( icons->end() );
	for( ; iter!=end; iter++ ) {
		temp += sizeof( iter->model );
		temp += sizeof( iter->color );
		temp += sizeof( eUI08 ) + iter->response.size();
	}
	
	this->size=temp;
	
	//send of header
	Xsend( s, this->getBeginValid(), this->headerSize );
	

	Xsend( s, question.c_str(), question.size() );

	icon_count = icons->size();
	Xsend( s, (char*)&icon_count, sizeof(icon_count) );

	iter = icons->begin();
	for( ; iter!=end; iter++ ) {
		Xsend( s, (char*)&iter->model, ( sizeof(iter->model) +sizeof(iter->color) ) );
		eUI08 rl=iter->response.size();
		Xsend( s, (char*)&rl, sizeof(rl) );
		Xsend( s, iter->response.c_str(), iter->response.size() ); //not send null terminator
	}

}

CREATE( QuestArrow, PKG_QUEST_ARROW, 0x06 )

template< class T >
cPacketTargetingCursor<T>::cPacketTargetingCursor() {
	cmd = PKG_TARGETING;
	headerSize = 0x13;
}

template< class T >
cPacketGeneralInfo<T>::cPacketGeneralInfo() {
	cmd = PKG_GENERAL_INFO;
	headerSize = 0x05;
}

cSubPacketParty<cServerPacket>::cSubPacketParty() : cPacketGeneralInfo<cServerPacket>() {
	this->subcmd = 6;
	headerSize += sizeof( eUI08 );
}

cSubPacketParty<cClientPacket>::cSubPacketParty() : cPacketGeneralInfo<cClientPacket>() {
	this->subcmd = 6;
	headerSize += sizeof( eUI08 );
}

clPacketAddPartyMember::clPacketAddPartyMember()
{
	subsubcommand = 1;
}

void clPacketAddPartyMember::receive( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();

	offset=headerSize;
	this->getFromSocket( s, (char*)&this->member, sizeof( eSERIAL ) );
}

csPacketAddPartyMembers::csPacketAddPartyMembers()
{
	subsubcommand = 1;
	headerSize += sizeof( eUI08 );
}

void csPacketAddPartyMembers::send( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();

	count = members->size();
	size = headerSize + count*sizeof(eSERIAL);
	Xsend( s, getBeginValid(), headerSize );

	std::vector<P_PARTY_MEMBER>::iterator iter( members->begin() ), end( members->end () );
	for( ; iter!=end; ++iter ) {
		eSERIAL b = (*iter)->serial;
		Xsend( s, &b, sizeof( eSERIAL ) );
	}
}

clPacketRemovePartyMember::clPacketRemovePartyMember()
{
	subsubcommand = 2;
}

void clPacketRemovePartyMember::receive( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();

	offset=headerSize;
	this->getFromSocket( s, (char*)&this->member, sizeof( eSERIAL ) );
}


csPacketRemovePartyMembers::csPacketRemovePartyMembers()
{
	subsubcommand = 2;
	headerSize += sizeof( eUI08 ) +sizeof(eSERIAL);
}

void csPacketRemovePartyMembers::send( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();

	count = members->size();
	size = headerSize + count*sizeof(eSERIAL);
	Xsend( s, getBeginValid(), headerSize );

	std::vector<P_PARTY_MEMBER>::iterator iter( members->begin() ), end( members->end () );
	for( ; iter!=end; ++iter ) {
		eSERIAL b = (*iter)->serial;
		Xsend( s, &b, sizeof( eSERIAL ) );
	}
}

clPacketPartyTellMessage::clPacketPartyTellMessage()
{
	subsubcommand = 3;
}

void clPacketPartyTellMessage::receive( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();

	offset=headerSize;
	getFromSocket( s, (char*)&this->member, sizeof(eSERIAL) );
	this->offset+=4;
	getUnicodeStringFromSocket( s, message );

}

csPacketPartyTellMessage::csPacketPartyTellMessage()
{
	subsubcommand = 3;
	headerSize += sizeof( eSERIAL );
}

void csPacketPartyTellMessage::send( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();

	size = headerSize + message->size()*sizeof(UI16)+sizeof(UI16);
	Xsend( s, getBeginValid(), headerSize );

	Xsend( s, *message, true );
}

csPacketPartyTellAllMessage::csPacketPartyTellAllMessage()
{
	subsubcommand = 4;
	headerSize += sizeof( eSERIAL );
}

void csPacketPartyTellAllMessage::send( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();

	size = headerSize + message->size()*sizeof(UI16)+sizeof(UI16);
	Xsend( s, getBeginValid(), headerSize );

	Xsend( s, *message, true );
}


clPacketPartyTellAllMessage::clPacketPartyTellAllMessage()
{
	subsubcommand = 4;
}

void clPacketPartyTellAllMessage::receive( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();
	
	this->offset = headerSize;
	getUnicodeStringFromSocket( s, message );
}

clPacketPartyCanLoot::clPacketPartyCanLoot()
{
	subsubcommand = 6;
}

void clPacketPartyCanLoot::receive( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();
	
	this->offset = headerSize;
	this->getFromSocket( s, (char*)&this->canLoot, sizeof(eBool) );
}

csPacketPartyInvite::csPacketPartyInvite()
{
	subsubcommand = 7;
	headerSize += sizeof(eSERIAL);
}

void csPacketPartyInvite::send( NXWCLIENT ps )
{
	if( ps==NULL )
		return;

	size=headerSize;
	Xsend( ps->toInt(), getBeginValid(), headerSize );
}

clPacketPartyAccept::clPacketPartyAccept()
{
	subsubcommand = 8;
}

void clPacketPartyAccept::receive( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();
	
	this->offset = headerSize;
	this->getFromSocket( s, (char*)&this->leader, sizeof(eSERIAL) );
}

clPacketPartyDecline::clPacketPartyDecline()
{
	subsubcommand = 9;
}

void clPacketPartyDecline::receive( NXWCLIENT ps )
{
	if( ps==NULL ) return;
	NXWSOCKET s = ps->toInt();
	
	this->offset = headerSize;
	this->getFromSocket( s, (char*)&this->leader, sizeof(eSERIAL) );
}





