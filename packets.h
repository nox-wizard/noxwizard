  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


/*!
\file packets.h
\brief Declaration of class cPackets and derived, all the Uo Packets know
\note All these information are from Wolfpack ( very thx to )
\note client version 3.0.
\todo Send skill pkg 0x3a not implemented yet
\todo Bulletin board message [0x71]
*/

#ifndef __PACKETS_H
#define __PACKETS_H

#include "constants.h"
#include "typedefs.h"
#include <string>
#include "abstraction/endian.h"

using namespace std;

/*!
\brief Base class of all packets
\author Endymion
\since 0.83a
*/
class cPacket
{

protected:

	int headerSize; //!< header size ( NOT SENDED OR RECEIVED. USE INTERNAL )
	int offset; //!< current baffer offset ( NOT SENDED OR RECEIVED. USE INTERNAL )
	char* getBeginValid();

public:
	UI08 cmd;		//!< packet id


} PACK_NEEDED;


/*!
\brief Base class of all packets send from client to server
\author Endymion
\since 0.83a
*/
class cClientPacket : public cPacket {
protected:
	char* getBeginValidForReceive();
	void getFromSocket( NXWSOCKET socket, char* b, int size );
	void getStringFromSocket( NXWSOCKET socket, string& s, int lenght );
	void getUnicodeStringFromSocket( NXWSOCKET s, wstring& c, int size=INVALID ); 

public:
	virtual void receive( NXWCLIENT ps );
	virtual ~cClientPacket() { };
} PACK_NEEDED;

/*!
\brief Base class of all packets send to server to client
\author Endymion
\since 0.83a
*/
class cServerPacket : public cPacket {
public:
	virtual void send( NXWCLIENT ps );
	virtual void send( P_CHAR pc );
	virtual ~cServerPacket() { };

} PACK_NEEDED;


typedef cPacket* P_PACKET;	//!< pointer to cPacket
typedef cClientPacket* P_CLIENT_PACKET;	//!< pointer to cClientPacket
typedef cServerPacket* P_SERVER_PACKET;	//!< pointer to cServerPacket




/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////////// PACKETS NOT WORK ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#define PKG_CREATE_CHARACTER 0x00;
/*!
\brief User selects to create a new character
\author Endymion
\since 0.83
\note 0x00
\note pattern3 is set to 0xFF to ealy signal a Krioss client user, where it expects a 0xF0| x00
		before login confim
\warning check for consistency before using information contained
\attention Str, dex and int should always sum to 65
\attention Str, dex and int should always be between 10 and 45, inclusive
\attention Skill1, skill2, and skill3 should never be the same value
\attention Skill1value, skill2value, and skill3value should always sum to 100
\attention Skill1value, skill2value, and skill3value should always be between 0 and 50, inclusive
\attention SkinColor should always be between 0x3EA and 0x422, exclusive
\attention HairColor and facialHairColor should always be between 0x44E and 0x4AD, exclusive
\attention Shirt color and Pants color need bounds checking too
\todo Add a validate function for check consistecy
*/
class cPacketCreateCharacter : public cClientPacket {

public:

	UI32	pattern1; //!< is 0xEDEDEDED 
	UI32	pattern2; //!< is 0xFFFFFFFF 
	UI08	pattern3; //!< is 0x00
	string	name; //!< name ( 30 character )
	string	passwd; //!< password ( 30 character )
	UI08	sex; //!< sex
	UI08	st; //!< strenght
	UI08	dx; //!< dexterity
	UI08	in; //!< intelligence
	UI08	skill1; //!< first skill
	UI08	skill1value; //!< value of first skill
	UI08	skill2; //!< second skill
	UI08	skill2value; //!< value of second skill
	UI08	skill3; //!< third skill
	UI08	skill3value; //!< value of third skill
	eCOLOR	skin; //!< skin color
	UI16	hairstyle; //!< hair style
	eCOLOR	hair; //!< hair color
	UI16	facialhairstyle; //!< facial hair style
	eCOLOR	facialhair; //!< facial hair color
	UI16	location; //!<// starting location from starting list
	UI16	unknown; //!< unknow
	UI16	slot; //!< account slot
	UI32	clientIP; //!< the IP of client
	eCOLOR	shirt; //!< shirt color
	eCOLOR	pants; //!< pants color

	cPacketCreateCharacter();
	void receive( NXWCLIENT ps  );

} PACK_NEEDED;

#define PKG_DISCONNECT_NOTIFY 0x01;
/*!
\brief User Special Disconnect
\author Endymion
\since 0.83
\note 0x01
\note Sent when user chooses to return to main menu grom the character selection
\note Send in 3.0.x clients when player does a "perperdoll logout"
*/
class cPacketDisconnectNotification : public cClientPacket {

public:

	UI32	pattern; //!< is 0xFFFFFFFF

	cPacketDisconnectNotification();

} PACK_NEEDED;

#define PKG_TALK_REQUEST 0x03;
/*!
\brief User is talking
\author Endymion
\since 0.83a
\note 0x03
*/
class cPacketTalkRequest : public cClientPacket {

private:

	UI16	size; //!< size of the packet

public:

	UI08	type; //!< Text type
	eCOLOR	color; //!< Text color
	UI16	font; //!< Text font
	string	msg; //!< Message ( Null Terminated String )

	cPacketTalkRequest();
	void receive( NXWCLIENT ps );

};

#define PKG_GODMODE_TOGGLE 0x04;
/*!
\brief Used in God client
\author Endymion
\since 0.83
\note 0x04
*/
class cPacketGodModeToggle : public cClientPacket {
public:

	UI08	mode; //!< 0=off, 1=on

	cPacketGodModeToggle();

};

#define PKG_ATTACK_REQUEST 0x05;
/*!
\brief User tries to attack someone
\author Endymion
\since 0.83
\note 0x05
*/
class cPacketAttackRequest : public cClientPacket {

public:

	eSERIAL	att;	//!< serial of to be attacked

	cPacketAttackRequest();	

} PACK_NEEDED;

#define PKG_DB_CLICK 0x06;
/*!
\brief User do double click
\author Endymion
\since 0.83
\note 0x06
*/
class cPacketDoubleClick : public cClientPacket {

public:

	Serial	obj;	//!< serial of double clicked object

	cPacketDoubleClick();	

} PACK_NEEDED;


#define PKG_PICK_UP 0x07;
/*!
\brief User pick up an item
\author Endymion
\since 0.83
\note 0x07
*/
class cPacketPickUpItem : public cClientPacket {

public:

	Serial	item;	//!< serial of the item
	UI16	num;	//!< number of items in stack

	cPacketPickUpItem();

} PACK_NEEDED;


#define PKG_DROP_ITEM 0x08;
/*!
\brief User drop an item
\author Endymion
\since 0.83
\note 0x08
\warning 3D client send 2 of them ( burst ) for ONE drop action ( the last hav -1 in X/Y locs )
*/
class cPacketDropItem : public cClientPacket {

public:

	Serial	item;	//!< serial of the item
	UI16	x;	//!< x location
	UI16	y;	//!< y location
	SI08	z;	//!< z location
	Serial	cont;	//!< moved into this container ( 0xFFFFFFFF if normal world ) 

	cPacketDropItem();	

} PACK_NEEDED;

#define PKG_SINGLE_CLICK 0x09;
/*!
\brief User do single click on an object
\author Endymion
\since 0.83
\note 0x09
*/
class cPacketSingleClick : public cClientPacket {

public:

	Serial	obj;		//!< serial of the object

	cPacketSingleClick();

} PACK_NEEDED;

#define PKG_STAT_WINDOW 0x11;
/*!
\brief Stat window info
\author Endymion
\since 0.83
\note 0x11
\note For characters other than the player, currentHitpoints and maxHitpoints
are not the actual values. MaxHitpointsis a fixed value, and currentHitpoints works
like a percentage
*/
class cPacketStatWindow : public cServerPacket {

private:
	UI16	size;			//!< lenght packet
public:
	Serial	player;			//!< serial of player
	string	name;			//!< name ( 30 characters )
	UI16	currhp;			//!< current Hit points
	UI16	maxhp;			//!< max Hit points
	UI08	canchangename;	//!< name change flag ( 0xFF = allowed, 0 = not allowed )
private:
	UI08	flag;			//!< flag ( 0x00 = no more data following ( end of packet here ), 0x01 = more data follow, 0x03 like 0x01 with extended info)
public:
	UI08	sex;			//!< sex ( 0=male, 1=female)
	UI16	st;				//!< strenght
	UI16	dx;				//!< dexterity
	UI16	in;				//!< intelligence
	UI16	currstamina;	//!< current Stamina
	UI16	maxstamina;		//!< max Stamina
	UI16	currmana;		//!< current Mana
	UI16	maxmana;		//!< max Mana
	UI32	gold;			//!< gold 
	UI16	ac;				//!< armor class
	UI16	weight;			//!< weight

	//extended info flah==0x03
	UI16	statcap;		//!< stat cap
	UI08	currpets;		//!< current pets
	UI08	maxpets;		//!< max pets

	cPacketStatWindow();
	void send( NXWCLIENT ps );
	void sendBasic( NXWCLIENT ps, UI08 flag = 0x00 );
	void sendStat( NXWCLIENT ps );
	void sendExtended( NXWCLIENT ps );

} PACK_NEEDED;

#define PKG_DROP_OR_WEAR_ITEM 0x13;
/*!
\brief User drop - wear and item
\author Endymion
\since 0.83
\note 0x13
\remarks don't trust layer
*/
class cPacketWear : public cClientPacket {
public:

	Serial	item;		//!< serial of item 
	UI08	layer;		//!< layer
	Serial	player;		//!< serial of the player ( ? who wear or where the item are weared? )

	cPacketWear();	

} PACK_NEEDED;

#define PKG_CHAR_INFO 0x1B;
/*!
\brief Char Location and body type ecc..
\author Endymion
\since 0.83
\note 0x1B
\note only send once after login. it's mandatory to send it once
\note OSI call this packet Login confim
*/
class cPacketCharInfo : public cServerPacket {

public:

	Serial	player;		//!< serial of player 
	UI32	unk1;		//!< Unknow 1 ( always 0 ) ( for boat?? )
	UI16	bodyType;	//!< Body type
	UI16	x;			//!< x Location
	UI16	y;			//!< y Location
	UI16	z;			//!< z Location
	UI08	dir;		//!< direction
	UI08	unk2;		//!< Unknow 2
	UI32	unk3;		//!< Unknow 3
	UI16	sbx;		//!< Server Boundry X
	UI16	sby;		//!< Server Boundry Y
	UI16	sbwidth;	//!< Server Boundry Width
	UI16	sbheigth;	//!< Server Boundry Height
	UI08	ignored[6];	//!< Ignored by standard client

	cPacketCharInfo();

} PACK_NEEDED;

#define PKG_SPEECH 0x1C;
/*!
\brief Send speech
\author Endymion
\since 0.83
\note 0x1C
*/
class cPacketSpeech : public cServerPacket {

private:
	UI16	size; 
public:
	Serial	obj;		//!< from ( 0xFFFFFFFF=system message)
	UI16	model;		//!< item hex number | 0xFFFF=system
	UI08	type;		//!< text type
	eCOLOR	color;		//!< text color
	UI16	font;		//!< text font
	string	name;		//!< name of who speech ( 30 character )
	string*	msg;		//!< message  Null-Terminated Message (? = blockSize - 44)

	cPacketSpeech();
	void send( NXWCLIENT ps );

} PACK_NEEDED;

#define PKG_DELETE 0x1D;
/*!
\brief Delete object
\author Endymion
\since 0.83
\note 0x1D
*/
class cPacketDelete : public cServerPacket {
public:

	Serial	obj; //!< serial of object

	cPacketDelete();

} PACK_NEEDED;

#define PKG_DRAW_PLAYER 0x20;
/*!
\brief Draw Game Player
\author Endymion
\since 0.83
\note 0x20
\note Only used with the character being played by the client
*/
class cPacketDrawPlayer : public cServerPacket {

public:

	Serial	chr;		//!< serial of the character
	UI16	body;		//!< body type
	UI08	unk1;		//!< unknow 1 ( is 0 )
	eCOLOR	skin;		//!< skin color
	UI08	flag;		//!< flag
	UI16	x;			//!< x Location
	UI16	y;			//!< y Location
	UI16	unk2;		//!< unknow 2 ( is 0 )
	UI08	dir;		//!< direction
	SI08	z;			//!< z Location

	cPacketDrawPlayer();	

} PACK_NEEDED;

#define PKG_REJECT_MOVEMENT 0x21;
/*!
\brief A movement request is rejected
\author Endymion
\since 0.83
\note 0x21
*/
class cPacketMoveReject : public cServerPacket {

public:

	UI08	seq;	//!< sequence number 
	UI16	x;		//!< x Location
	UI16	y;		//!< y Location
	UI08	dir;	//!< direction
	SI08	z;		//!< z Location

	cPacketMoveReject();	

} PACK_NEEDED;

#define PKG_MOVE_ACK 0x22;
/*!
\brief Character Move ACK/ Resync Request
\author Endymion
\since 0.83
\note 0x22
\note server side is move ack for 0x02 packet
\note whenever client thinks it's out of synch ( basically: sequence doesen't fit )
	it sends a 0x22 0 0 / Resync request ( server should resend items / npc /ect around
*/
class cPacketMoveACK : public cServerPacket {

public:

	UI08	seq;		//!< sequence number (matches sent sequence)
/*!
\brief notoriety flag

can be one of this:
<ul>
<li>0 = invalid/across server line</li>
<li>1 = innocent ( blue )</li>
<li>2 = guilded/ally ( green )</li>
<li>3 = attackable but not criminal ( gray )</li>
<li>4 = criminal ( gray )</li>
<li>5 = enemy ( orange )</li>
<li>6 = murdered ( red )</li>
<li>7 = unknow, translucent like hue 0x4000 )</li>
</ul>
*/
	UI08	notoriety;	//!< notoriety

	cPacketMoveACK();

} PACK_NEEDED;

#define PKG_DRAGGING 0x23;
/*!
\brief Dragging of Items
\author Endymion
\since 0.83
\note 0x23
*/
class cPacketDragging : public cClientPacket {

public:

	UI16	model;		//!< model number
	UI08	unk[3];		//!< unknown 1
	UI16	stack;		//!< stack count
	Serial	from;		//!< serial of source
	UI16	fromx;		//!< x Location of source
	UI16	fromy;		//!< y Location of source
	SI08	fromz;		//!< z Location of source
	Serial	to;			//!<  serial of target
	UI16	tox;		//!< x Location of target
	UI16	toy;		//!< y Location of target
	SI08	toz;		//!< z Location of target

	cPacketDragging();	

} PACK_NEEDED;

#define PKG_DRAW_CONTAINER 0x24;
/*!
\brief Draw Container
\author Endymion
\since 0.83
\note 0x24
*/
class cPacketDrawContainer : public cServerPacket {

public:

	Serial	item;		//!< serial of container
	UI08	model;		//!< Gump of the container

	cPacketDrawContainer();	

} PACK_NEEDED;

#define PKG_ADD_ITEM_INTO_CONTAINER 0x25;
/*!
\brief Add Item into Container
\author Endymion
\since 0.83
\note 0x25
*/
class cPacketAddItemIntoCont : public cClientPacket {

public:

	Serial	item;		//!< serial of item to add
	UI16	model;		//!< model
	UI08	unk;		//!< unknown ( is 0 )
	UI16	n;			//!< number of items
	UI16	x;			//!< x Location in container
	UI16	y;			//!< y Location in container
	Serial	cont;		//!< serial of container
	eCOLOR	color;		//!< color

	cPacketAddItemIntoCont();	

} PACK_NEEDED;

#define PKG_KICK 0x26;
/*!
\brief Kick Player
\author Endymion
\since 0.83
\note 0x26
*/
class cPacketKick : public cServerPacket {

public:

	Serial	chr;	//!< serial of GM who issued kick?

	cPacketKick();	

} PACK_NEEDED;

#define PKG_REJECT_MOVE_ITEM 0x27;
/*!
\brief Reject Request to move Items
\author Endymion
\since 0.83
\note 0x27
*/
class cPacketRejectMoveItem : public cServerPacket {

public:

	UI08 unk;	//!< unknow (0x00)

	cPacketRejectMoveItem();	

} PACK_NEEDED;

#define PKG_CLEAR_SQUARE 0x28;
/*!
\brief Clear Square
\author Endymion
\since 0.83
\note 0x28
*/
class cPacketClearSquare : public cServerPacket {

public:

	UI16	x;	//!< x Location
	UI16	y;	//!< y Location

	cPacketClearSquare();	

} PACK_NEEDED;

#define PKG_PD_CLOTHING 0x29;
/*!
\brief Paperdoll Clothing Added
\author Endymion
\since 0.83
\note 0x29
*/
class cPacketPDClothing : public cServerPacket {

public:

	cPacketPDClothing();	

} PACK_NEEDED;

#define PKG_RESS_MENU 0x2C;
/*!
\brief Resurrection Menu Choice
\author Endymion
\since 0.83
\note 0x2C
\note Resurrection menu has been removed from UO, so now is unused
*/
class cPacketRessMenu : public cClientPacket {

public:

	UI08	mode;		//!< action ( 2=ghost, 1=resurrect, 0=from server )

	cPacketRessMenu();

} PACK_NEEDED;

#define PKG_MOBILE_ATTRIBUTES 0x2D;
/*!
\brief Mobile attributes
\author Endymion
\since 0.83
\note 0x2D
*/
class cPacketMobileAttributes : public cServerPacket {

public:

	Serial chr;		//!< the charatcer
	UI16 maxhp;		//!< max hp
	UI16 currhp;	//!< current hp
	UI16 maxmana;	//!< max mana
	UI16 currmana;	//!< current mana
	UI16 maxstam;	//!< max stamina
	UI16 currstam;	//!< current stamina

	cPacketMobileAttributes();	

} PACK_NEEDED;

#define PKG_WORN_ITEM 0x2E;
/*!
\brief Worn Item
\author Endymion
\since 0.83
\note 0x2E
*/
class cPacketWornItem : public cClientPacket {

public:

	Serial	item;		//!< item id
	UI16	model;		//!< item hex number
	UI08	unk ;		//!< unknow (0x00)
	UI08	layer;		//!< layer
	Serial	player;		//!< player id
	eCOLOR	color;		//!< color

	cPacketWornItem();	

} PACK_NEEDED;

#define PKG_FIGHT_OCCURING 0x2F;
/*!
\brief There is a fight going on somewhere on screen
\author Endymion
\since 0.83
\note 0x2F
*/
class cPacketFightOccuring : public cServerPacket {

public:

	UI08	unk;	//!< unknown1 (0)
	Serial	att;	//!< serial of attacker
	Serial	def;	//!< serial of defender

	cPacketFightOccuring();	

} PACK_NEEDED;

#define PKG_PAUSE_RESUME 0x33;
/*!
\brief Pause/Resume Client
\author Endymion
\since 0.83
\note 0x33
\todo can be use while world data saving?
\todo use misanderstood
\warning not sure from who
*/
class cPacketPauseResumeClient : public cServerPacket {
public:

	UI08	mode;	//!< action ( 0=pause, 1=resume )

	cPacketPauseResumeClient();	

} PACK_NEEDED;

#define PKG_PLAYER_STATUS 0x34;
/*!
\brief Get Player Status
\author Endymion
\since 0.83
\note 0x34
*/
class cPacketPlayerStatus : public cClientPacket {

public:

	UI32	pattern;	//!< pattern ( 0xEDEDEDED )
	BYTE	type;		//!< get type ( 4=Basic Stats (Packet 0x11 Response), 5=Request Skills (Packet 0x3A Response) )
	Serial	player;		//!< serial of player

	cPacketPlayerStatus();	

} PACK_NEEDED;


typedef struct {
	UI08	flag;	//!< flag ( 0x1A )
	Serial	item;	//!< serial of item (from 3C packet)
	UI16	n;		//!< number of bought item
} buyitem_st;

#define PKG_BUY 0x3B;
/*!
\brief Buy Item(s)
\author Endymion
\since 0.83a
\note 0x3B
*/
class cPacketBuy : public cServerPacket {

private:
	UI16	size;		//!< packet size
public:
	Serial	vendor;		//!< serial of vendor
private:
	UI08	flag;		//!< flag ( 0=no items following, 2=items following ) 1=onlyoneitem?
public:
	vector<buyitem_st> list;

	void send( NXWCLIENT ps );
	cPacketBuy();

} PACK_NEEDED;

/*!
\brief struct with informations about items in containers
*/
typedef struct {
	Serial	item;		//!< serial of item
	UI32	model;		//!< model
	UI08	unk;		//!< unknown (0x00)
	UI16	n;		//!< number of items in stack
	UI16	x;		//!< x Location in container
	UI16	y;		//!< y Location in container
	Serial	cont;		//!< serial of container
	eCOLOR	color;		//!< color of item
} itemincont_st;

#define PKG_CONTAINER 0x3C;
/*!
\brief Items in Container
\author Endymion
\since 0.83
\note 0x3C
*/
class cPacketContainer : public cServerPacket {

private: 
	UI16	size;		//!< packet size
	UI16	n;			//!< number of itemincont_st
public:
	vector<itemincont_st> list;
	
	void send( NXWCLIENT ps );
	cPacketContainer();	

} PACK_NEEDED;

#define PKG_PERSONAL_LIGHT 0x4E;
/*!
\brief Personal Light Level
\author Endymion
\since 0.83
\note 0x4E
*/
class cPacketPersonalLight : public cServerPacket {
public:

	Serial	chr;		//!< serial of character
	UI08	level;		//!< light level

	cPacketPersonalLight();	

} PACK_NEEDED;


#define PKG_OVERALL_LIGHT 0x4F;
/*!
\brief Overall Light Level
\author Endymion
\since 0.83
\note 0x4F
*/
class cPacketOverallLight : public cServerPacket {
public:

	UI08	level;		//!< light level ( 0x00=day, 0x09=OSI night, 0x1F=Black )

	cPacketOverallLight();

} PACK_NEEDED;

#define PKG_IDLE_WARNING 0x53;
/*!
\brief Displays a pre-defined message in client
\author Endymion
\since 0.83
\note 0x53
\note type are:
 1=no character
 2=char exists
 3=can't connect
 4=can't connect
 5=character already in world
 6=login problem
 7=idle
 8=can't connect
*/
class cPacketIdleWarning : public cServerPacket {

public:

	UI08	type;		//!< type ( see note )

	cPacketIdleWarning();	

} PACK_NEEDED;

#define PKG_SOUND 0x54;
/*!
\brief Play Sound Effect
\author Endymion
\since 0.83
\note 0x54
*/
class cPacketSound : public cServerPacket {

public:

	UI08	mode;		//!< mode ( 0=quiet repeating, 1=single normally played sound effect )
	UI16	model;		//!< sound model
	UI16	unk;		//!< unknown ( speed/volume modifier? Line of sight stuff? )
	UI16	x;			//!< x Location
	UI16	y;			//!< y Location
	UI16	z;			//!< z Location

	cPacketSound();

} PACK_NEEDED;

#define PKG_LOGIN_COMPLETE 0x55;
/*!
\brief Login Complete, start game
\author Endymion
\since 0.83
\note 0x55
*/
class cPacketLoginComplete : public cServerPacket {

	cPacketLoginComplete();	

} PACK_NEEDED;

#define PKG_PLOT_COURSE 0x56;
/*!
\brief Plot course for ships
\author Endymion
\since 0.83
\note 0x56
\note type
 1= add map point
 2= add new pin with pin number ( insertion, other pins after the number are pushed back )
 3= change pin
 4= remove pin
 5= remove all pin on the map
 6= toggle the "editable" state of the map
 7= return msg from the server to the request 6 of the client
*/
class cPacketPlotCourse : public cServerPacket {

	Serial	serial;		//!< serial
	UI08	type;		//!< type 
	UI08	state;		//!< plotting state ( 1=on, 0=off, valid only if type 7 )
	UI16	x;			//!< x pin ( relative to upper left corner of the map, inpixel, for points )
	UI16	y;			//!< y pin ( relative to upper left corner of the map, inpixel, for points )
	
	cPacketPlotCourse();	

} PACK_NEEDED;

#define PKG_TIME 0x5B;
/*!
\brief Time
\author Endymion
\since 0.83
\note 0x5B
*/
class cPacketTime : public cServerPacket {

	UI08	hour;		//!< hour
	UI08	minute;		//!< minute
	UI08	second;		//!< second

	cPacketTime();	

} PACK_NEEDED;

/*!
\brief Login Character
\author Endymion
\since 0.83
\note 0x5D
\remarks ClientIP should not be trusted.
*/
#define PKG_LOGIN 0x5D;
class cPacketLogin : public cClientPacket {

	UI32	pattern;	//!< pattern ( 0xEDEDEDED )
	string	name;		//!< name ( 30 characters )
	string	passwd;		//!< password ( 30 characters )
	UI32	slot;		//!< slot choosen ( 0 based )
	UI32	ip;			//!< client ip
	
	cPacketLogin();	
	void receive( NXWCLIENT ps );

} PACK_NEEDED;

#define PKG_WEATHER 0x65;
/*!
\brief Set Weather
\author Endymion
\since 0.83
\note 0x65
\note Temperature has no effect at present
\remarks maximum number of weather effects on screen is 70
\remarks If it is raining, you can add snow by setting the num
to the num of rain currently going, plus the number of snow you want
\attention Weather messages are only displayed when weather starts
\note Weather will end automatically after 6 minutes without any weather change packets
\remarks You can totally end weather (to display a new message) by teleporting
\note weather type:
0x00="It starts to rain"
0x01="A fierce storm approaches."
0x02="It begins to snow"
0x03="A storm is brewing.",
0xFF=None (turns off sound effects),
0xFE=(no effect?? Set temperature?)
0x78,0x20,0x4F,0x4E = reset, never tested
*/
class cPacketWeather : public cServerPacket {

	BYTE	type;		//!< type ( see note )
	BYTE	n;			//!< number of weather effects on screen
	BYTE	temp;		//!< temperature
	
	
	cPacketWeather();	

} PACK_NEEDED;

#define PKG_MIDI 0x6D;
/*!
\brief Play Midi Music
\author Endymion
\since 0.83
\note 0x6D
*/
class cPacketMidi : public cServerPacket {

public:

	UI16	music;		//!< music id

	cPacketMidi();	

} PACK_NEEDED;

#define PKG_ANIM 0x6E;
/*!
\brief Character Animation
\author Endymion
\since 0.83
\note 0x6E
\note movement type
0x00 = walk
0x01 = walk faster
0x02 = run
0x03 = run (faster?)
0x04 = nothing
0x05 = shift shoulders
0x06 = hands on hips
0x07 = attack stance (short)
0x08 = attack stance (longer)
0x09 = swing (attack with knife)
0x0a = stab (underhanded)
0x0b = swing (attack overhand with sword)
0x0c = swing (attack with sword over and side)
0x0d = swing (attack with sword side)
0x0e = stab with point of sword
0x0f = ready stance
0x10 = magic (butter churn!)
0x11 = hands over head (balerina)
0x12 = bow shot
0x13 = crossbow
0x14 = get hit
0x15 = fall down and die (backwards)
0x16 = fall down and die (forwards)
0x17 = ride horse (long)
0x18 = ride horse (medium)
0x19 = ride horse (short)
0x1a = swing sword from horse
0x1b = normal bow shot on horse
0x1c = crossbow shot
0x1d = block #2 on horse with shield
0x1e = block on ground with shield
0x1f = swing, and get hit in middle
0x20 = bow (deep)
0x21 = salute
0x22 = scratch head
0x23 = 1 foot forward for 2 secs
0x24 = same
*/
class cPacketAnim : public cServerPacket {

public:

	Serial	chr;		//!< serial of the character
	UI16	mov;		//!< movement model ( see note )
	UI08	unk;		//!< unknown ( 0x00 )
	UI08	dir;		//!< direction
	UI16	repeat;		//!<  ( 0=repeat forever, 1=once, 2=twice, ?n=n times? )
	UI08	wards;		//!< forward/backwards ( 0=forward, 1=backwards )
	UI08	flag;		//!< repeat flag ( 0=Don't repeat, 1=repeat )
	UI08	deelay;		//!< frame delay ( 0x00=fastest, 0xFF=too slow to watch )

	cPacketAnim();	

} PACK_NEEDED;

/*|
\brief Secure Trading
\author Endymion
\since 0.83
\note 0x6F
*//*not implemented yet
class cPacketSecureTrading : public cServerPacket {

private:

	UI16 size; //!< packet size

public:

	BYTE action
	BYTE[4] id1
	BYTE[4] id2
	BYTE[4] id3
	BYTE nameFollowing (0 or 1)
	If (nameFollowing = 1)
	BYTE[?] charName

	cPacketSecureTrading();	

} PACK_NEEDED;
*/

#define PKG_EFFECT 0x70;
/*!
\brief Graphical Effect
\author Endymion
\since 0.83
\note 0x70
\note direction type
0=go from source to dest
1=lightning strike at source
2=stay at current x,y,z
3=stay with current source character id
*/
class cPacketEffect : public cServerPacket {

public:

	UI08	dir;		//!< direction type ( see note )
	Serial	chr;		//!< serial of source character
	Serial	target;		//!< serial of target character
	UI16	model;		//!< model of the first frame of the effect
	UI16	sx;			//!< x Location of source
	UI16	sy;			//!< y Location of source
	SI08	sz;			//!< z Location of source
	UI16	tx;			//!< x Location of target
	UI16	ty;			//!< y Location of target
	SI08	tz;			//!< z Location of target
	UI08	speed;		//!< speed of the animation
	UI08	duration;	//!< duration ( 0=really long, 1=shortest )
	UI16	unk;		//!< unknown (0 works)
	UI08	direct;		//!< no adjust direction during animation ( 0=yes, else no ) ( ?is bool?)
	UI08	explode;	//!< explode on impact

	cPacketEffect();	

} PACK_NEEDED;

#define PKG_WAR_MODE 0x72;
/*!
\brief Request War Mode Change/Send War Mode status
\author Endymion
\since 0.83
\note From Server,Client
\note 0x72
\note server replies with 0x77 packet
*/
class cPacketWarMode : public cServerPacket /*CLIENTALSOPD*/ {

public:

	BYTE	flag;		//!< flag ( 0=Normal, 1=Fighting )
	UI08	unk[3];		//!< unknown (always 00 32 00 in testing)

	cPacketWarMode();

} PACK_NEEDED;


/*!
\brief Ping message
\author Endymion
\since 0.83a
\note From ??
\note 0x73
\note server replies with 0x77 packet
*//*
class cPacketPing : public cPackets {

public:

	BYTE seq; //!<  

	cPacketPing();	

} PACK_NEEDED;*/

/*
typedef struct {
	UI32 price;
	UI08 length; of text description
	BYTE[text length] item description
} openbuyitem_st ;

*//*
\brief Open Buy Window 
\author Endymion
\since 0.83
\note 0x74
\note This packet is always preceded by a describe contents packet (0x3c) with the
container id as the (vendorID | 0x40000000) and then an open container packet
(0x24?) with the vendorID only and a model number of 0x0030 (probably the model
number for the buy screen)
*//*
#define PKG_OPEN_BUY 0x74;
class cPacketOpenBuy : public cServerPacket {

private:
	UI16 size; //!< packet size
public:
	Serial vendor; //!< serial of verndor | 0x40000000
private:
	UI08 n; //!< number of items of items
public:
	vector<openbuyitem_st> list;

	cPacketOpenBuy();	
	void send( NXWCLIENT ps );

} PACK_NEEDED;

*/

#define PKG_RENAME 0x75;
/*!
\brief Rename Character
\author Endymion
\since 0.83
\note 0x75
*/
class cPacketRename : public cServerPacket {

public:

	Serial	chr;		//!< serial of chr
	string	newname;	//!< the new name ( 30 characters )

	cPacketRename();
	void send( NXWCLIENT ps );

} PACK_NEEDED;

#define PKG_NEW_SUBSERVER 0x76;
/*!
\brief New subserver
\author Endymion
\since 0.83
\note 0x76
*/
class cPacketNewSubserver : public cServerPacket {

public:

	UI16	x;			//!< x location
	UI16	y;			//!< y location
	UI16	z;			//!< z location
	UI08	unk;		//!< unknow ( always 0 )
	UI16	sbx;		//!< server boundry x
	UI16	sby;		//!< server boundry y
	UI16	sbwithd;	//!< server boundry Width
	UI16	sbheight;	//!< server boundry Height

	cPacketNewSubserver();

} PACK_NEEDED;

#define PKG_UPDATE_PLAYER 0x77;
/*!
\brief Update player
\author Endymion
\since 0.83
\note 0x77
*/
class cPacketUpdatePlayer : public cServerPacket {

public:

	Serial	player;		//!< the serial of player
	UI16	model;		//!< the model id
	UI16	x;			//!< x location
	UI16	y;			//!< y location
	SI08	z;			//!< z location
	UI08	dir;		//!< direction
	eCOLOR	color;		//!< color
	UI08	flag;		//!< flag ( bitset )
	UI08	highcolor;	//!< highlight color

	cPacketUpdatePlayer();

} PACK_NEEDED;

//0x78 not yet



#define PKG_RESPONSE_TO_DIALOG 0x7D;
/*!
\brief Client response to dialog
\author Endymion
\since 0.83
\note 0x7D
*/
class cPacketResponseToDialog : public cClientPacket {

public:

	eSERIAL	serial;		//!< the dialog serial ( echoed back from 7C packet )
	eUI16	id;		//!< the model id ( echoed back from 7C packet )
	eUI16	index;		//!< index of choice ( 1 based )
	eUI16	model;		//!< model of choice
	eCOLOR	color;		//!< color

	cPacketResponseToDialog();

} PACK_NEEDED;

#define PKG_LOGIN_DENIED 0x82;
/*!
\brief Login denied
\author Endymion
\since 0.83
\note 0x82
\note why
 0x00 = unknow user
 0x01 = account already in use
 0x02 = account disabled
 0x03 = password bad
 0x04 and higer = communication failed
*/
class cPacketLoginDenied : public cServerPacket {

public:

	UI08	why;		//!< why ( see note )

	cPacketLoginDenied();

} PACK_NEEDED;

#define PKG_DELETE_CHARACHTER 0x83;
/*!
\brief Delete Character
\author Endymion
\since 0.83
\note 0x83
\note used on login not in game :)
*/
class cPacketDeleteCharacter : public cClientPacket {

public:

	string	passwd;		//!< the password
	UI32	idx;		//!< the char index
	UI32	ip;			//!< the client ip

	cPacketDeleteCharacter();
	void receive( NXWCLIENT ps );

} PACK_NEEDED;

/*
typedef struct resaftdel_st {
	string	name;		//!< the name
	string	passwd;		//!< the password
}

*//*!
\brief Resend Character After delete
\author Endymion
\since 0.83
\note 0x86
\note used on login not in game :)
*//*
#define PKG_RESEND_CHARACHTER_AFTER_DELETE 0x86;
class cPacketResendCharacterAfterDelete : public cServerPacket {
private:
	UI16	size;		//!< the size of packet
	UI08	n;			//!< number of character resend

public:

	vector<resaftdel_st> chr;	//!< the characters to resend

	cPacketResendCharacterAfterDelete();
	void send( NXWCLIENT ps );

};*/





//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////////////////////ONLY WORKING PACKETS/////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////




#define PKG_UNICODE_SPEECH 0xAE;
/*!
\brief Send speech ( unicode mode )
\author Endymion
\since 0.83
\note 0xAE
*/
class cPacketUnicodeSpeech : public cServerPacket {

private:
	eUI16	size; 
public:
	eSERIAL	obj;		//!< from ( 0xFFFFFFFF=system message)
	eUI16 model;		//!< item hex number | 0xFFFF=system
	eUI08	type;		//!< text type
	eCOLOR	color;		//!< text color
	eUI16	font;		//!< text font
	eSERIAL	language;   //!< language
	string	name;		//!< name of who speech ( 30 character )
	wstring*	msg;	//!< message

	cPacketUnicodeSpeech();
	void send( NXWCLIENT ps );

} PACK_NEEDED;


//@{
/*!
\name map packets
\brief map relatede packets
*/

#define PKG_MAP 0x56;
/*!
\brief Map related
\author Endymion
\since 0.83
\note 0x56
\note
command:
	1 = add map point, 
	2 = add new pin with pin number. (insertion. other pins after the number are pushed back.) 
	3 = change pin
	4 = remove pin
	5 = remove all pins on the map 
	6 = toggle the 'editable' state of the map. 
	7 = return msg from the server to the request 6 of the client. 
*/
class cPacketMap : public cServerPacket {

public:
	eSERIAL	obj;		//!< the map
	eUI08	command;	//!< command see note
	eUI08	plotting;	//!< plotting state ( 1=on, 0=off only valid if command 7)
	eUI16	x;		//!< x location (relative to upper left corner of the map, in pixels)
	eUI16	y;		//!< y location (relative to upper left corner of the map, in pixels)

	cPacketMap();

} PACK_NEEDED;

#define PKG_MAP_MESSAGE 0x90;
/*!
\brief Map Message
\author Endymion
\since 0.83
\note 0x90
*/
class cPacketMapMessage : public cServerPacket {

public:
	eSERIAL	key;	//!< the key used
	eUI16	id;		//!< gump art id (0x139D)
	eUI16	xa;		//!< x location (relative to upper left corner of the map, in pixels)
	eUI16	ya;		//!< y location (relative to upper left corner of the map, in pixels)
	eUI16	xb;		//!< x location (relative to upper left corner of the map, in pixels)
	eUI16	yb;		//!< y location (relative to upper left corner of the map, in pixels)
	eUI16   width;	//!< gump width in pixels
	eUI16   height;	//!< gump height in pixels

	cPacketMapMessage();

} PACK_NEEDED;

//@}

//@{
/*!
\name walking packets
\brief walking relatede packets
*/

#define PKG_WALK 0x97;
/*!
\brief Moves player played by client one step in requested direction.
\author Endymion
\since 0.83
\note 0x97
\note Doesn't move if there are obstacles
*/
class cPacketWalk : public cServerPacket {

public:
	eUI08   direction;	//!< direction
	cPacketWalk();

} PACK_NEEDED;


#define PKG_WALK_ACK 0x22;
/*!
\brief Character Move ACK/ Resync Request
\author Endymion
\since 0.83
\note 0x22
\note for notoriety see NOTORIETY
\note server packet: move ack for 0x02.
\note client packet: Whenever client thinks it’s out of synch (basicly: sequence doesn’t fit) it sends a 0x22 0 0 / Resync request. (server should resend items / npcs / etc around)
*/
class cPacketWalkAck : public cServerPacket {

public:
	eUI08   sequence;	//!< sequence
	eUI08   notoriety;	//!< notoriety ( see note and NOTORIETY )
	cPacketWalkAck();

} PACK_NEEDED;

typedef enum {
	NOT_VALID =0, //!< invalid/across server line
	INNOCENT, //!< innocent (blue)
	GUILDED, //!< guilded/ally (green)
	ATTACKABLE, //!< attackable but not criminal (gray)
	CRIMINAL, //!< criminal (gray)
	ENEMYY, //!< enemy (orange)
	MURDERER, //!< murderer (red)
	TRASLUCENT //!< unknown use (translucent (like 0x4000 hue))
} NOTORIETY;

#define PKG_WALK_REJECT 0x21;
/*!
\brief Character Move Reject
\author Endymion
\since 0.83
\note 0x21
*/
class cPacketWalkReject : public cServerPacket {

public:
	eUI08   sequence;	//!< sequence
	eUI16   x;	//!< x location
	eUI16   y;	//!< y location
	eUI08   direction;	//!< direction
	eUI08   z;	//!< z location
	cPacketWalkReject();

} PACK_NEEDED;



//@}

//@{
/*!
\name char profile packets
\brief char profile related packets
*/

#define PKG_CHAR_PROFILE 0xB8;
/*!
\brief Char Profile Request
\author Endymion
\since 0.83
\note 0xB8
*/
class cPacketCharProfileReqOnly : public cClientPacket {
private:
	eUI16	size; //!< size
public:
	eBool	update; //!< update
	eSERIAL chr; //!< character

	cPacketCharProfileReqOnly();

};

/*!
\brief Char Profile Request ( maybe an update )
\author Endymion
\since 0.83
\note 0xB8
*/
class cPacketCharProfileReq : public cClientPacket {
private:
	eUI16	size;	//!< size
public:
	eBool	update; //!< update
	eSERIAL	chr;	//!< character
    eUI16	type;	//!< type (0x0001 – Update)
private:
	eUI16	len;	//!< # of unicode characters
public:
	wstring profile;	//!< new profile, in unicode, not null terminated.

	cPacketCharProfileReq();
	void receive( NXWCLIENT PS );

};

/*!
\brief Char Profile
\author Endymion
\since 0.83
\note 0xB8
*/
class cPacketCharProfile : public cServerPacket {
private:
	eUI16 size;	//!< size
public:
	eSERIAL chr;	//!< character

	std::string title;	//!< title

	wstring* staticProfile;	//!< static profile, cant be edited 
	wstring* profile;	//!< profile, can be edited

	cPacketCharProfile();
	void send( NXWCLIENT ps );

};

//@}

//@{
/*!
\name client options packets
\brief client options related packets
*/



#define PKG_FEATURES 0xB9;

/*!
\brief Features
\author Endymion
\since 0.83
\note 0xB9
*/
class cPacketFeatures : public cServerPacket {

public:
	eUI16 feature;

	cPacketFeatures();
};

#define FEATURE_T2A 0x0001 //!< enable chat button
#define FEATURE_LBR 0x0002 //!< plays MP3 instead of midis, 2D LBR client shows new LBR monsters
#define FEATURE_LBR_WITHOUT_CHAT 0x0004 //!< lbr without chat

/*if (MSB not set)
Bit# 1 T2A upgrade, enables chatbutton, 
Bit# 2 enables LBR update.  (of course LBR installation is required)
(plays MP3 instead of midis, 2D LBR client shows new LBR monsters,…)

if (MSB set)
Bit# 3 T2A upgrade, enables chatbutton, 
Bit# 4 enables LBR update.  
Bit#5  enables AOS update (Aos monsters/map (AOS installation required for that) , 
AOS skills/necro/paladin/fight book stuff  – works for ALL clients 4.0 and above)

Note1: this message is send immediately after login.
Note2: on OSI  servers this controls features OSI enables/disables via “upgrade codes.”
Note3: a 3 doesn’t seem to “hurt” older (NON LBR) clients.
*/


//@}

//@{
/*!
\name utilitys packets
\brief utilitys related packets
*/

#define PKG_WEB_BROWSER 0xA5;

/*!
\brief Launch web browser
\author Endymion
\since 0.83
\note 0xA5
*/
class cPacketWebBrowser : public cServerPacket {
private:
	eUI16 size; //<! size
public:

	std::string link;

	cPacketWebBrowser();
	void send( NXWCLIENT ps );
};

//@}

//@{
/*!
\name menu packets
\brief menu related packets
*/


#define PKG_MENU 0xB0;

/*!
\brief Show an menu
\author Endymion
\since 0.83
\note 0xB0
*/
class cPacketMenu : public cServerPacket {
private:
	eUI16 size; //<! size
public:

	eSERIAL serial; //!< the serial
	eSERIAL id; //!< gump serial
	eUI32	x; //!< x location
	eUI32	y; //!< x location

private:
	eUI16 cmd_length; //!< command section length
public:
	std::vector<std::string>* commands; //!< commands ( zero terminated )

private:
	 eUI16 numTextLines; //!<text lines number
public:


//	for everty vector item
	eUI16 len; //!< text lenth for every unicode string	
	std::vector< wstring >* texts; //!< text ( every string is NOT null terminated )
//end

	cPacketMenu();
	void send( NXWCLIENT ps );
};


typedef struct {
	eUI16	id; //!< textentries id
	eUI16	textlength; //!< text length
	wstring text; //!< text ( not nullterminated )
} text_entry_st;


#define PKG_MENU_SELECTION 0xB1;

/*!
\brief A menu item is selected
\author Endymion
\since 0.83
\note 0xB1
*/

class cPacketMenuSelection : public cClientPacket {

private:
	eUI16	size;	//!< size
public:

	eSERIAL serial; //!< the serial ( first Id in PKG_MENU )
	eSERIAL id; //!< gump serial (second Id in PKG_MENU )
	eUI32	buttonId; //!< which button pressed or 0 if closed

private:
	eUI32	switchcount; //!<  response info for radio buttons and checkboxes, any switches listed here are switched on
public:
	std::vector<SERIAL> switchs; //!< switch ids
private:
	eUI32 textcount; //!< response info for textentries
public:
//	std::vector<text_entry_st> text_entries; //!< text entries
	std::map< SERIAL, std::wstring> text_entries; //!< text entries

	cPacketMenuSelection();
	void receive( NXWCLIENT PS );

};


typedef struct  {

	eUI16	model; //!< model id number of shown icon ( if grey menu then always 0x00 as msb )
	eCOLOR	color; //!< icon color
//	eUI08 resp_length, //needed but managed into send, so not need var
	std::string response;	

} pkg_icon_list_menu_st;

#define PKG_ICON_LIST_MENU 0x7C;

/*!
\brief Show an icon list menu
\author Endymion
\since 0.83
\note 0x7C
*/
class cPacketIconListMenu : public cServerPacket {
private:
	eUI16 size; //<! size
public:

	eSERIAL serial; //!< the serial
	eUI16 id; //!< the gump
private:
	eUI08 question_length; //!< question length
public:
	std::string question; //!< question text

private:
	eUI08 icon_count; //!< icon number
public:
	std::vector< pkg_icon_list_menu_st >* icons; //!< icons

	cPacketIconListMenu();
	void send( NXWCLIENT ps );
};

//@}

//@{
/*!
\name target packets
\brief target related packets
*/

#define PKG_QUEST_ARROW 0xBA;

/*!
\brief Show a quest arrow
\author Endymion
\since 0.83
\note 0xBA
*/
class cPacketQuestArrow : public cServerPacket {

public:

	eBool active; //<! active
	eUI16 x; //!< x location
	eUI16 y; //!< y location

	cPacketQuestArrow();

};


#define PKG_TARGETING 0x6C;
/*!
\brief Targeting Cursor Commands
\author Endymion
\since 0.83
\note 0x6C
\remarks the model number shouldn’t be trusted
*/
template< class T >
class cPacketTargetingCursor : public T {

public:

	eUI08	type;		//!< type ( 0=Select Object, 1=Select X, Y, Z )
	eUI32	cursor;		//!< cursor id
	eUI08	cursorType;	//!< cursor type

	//The following are always sent but are only valid if sent by client
	eUI32	clicked;	//!<  clicked on id
	eUI16	x ;			//!< click x Location
	eUI16	y ;			//!< click y Location
	eUI08	unk;		//!< nown (0x00)
	eSI08	z ;			//!< click z Location
	eUI16	model;		//!< model number ( 0=map/landscape tile, else static tile )
	
	cPacketTargetingCursor();

} PACK_NEEDED;

//@}

//@{
/*!
\name party packets
\brief party related packets
*/


#define PKG_GENERAL_INFO 0xBF;

/*!
\brief General info packet
\author Endymion
\since 0.83
\note 0xBF
*/
template< class T >
class cPacketGeneralInfo : public T {

protected:
	eUI16 size;	//<! size
	eUI16 subcmd;	//!< the subcmd
public:
	cPacketGeneralInfo();

};

#define GEN_INFO_SUBCMD_PARTY 6

/*!
\brief Party packets
\author Endymion
\since 0.83
\note 0xBF - 6
*/
template< class T >
class cSubPacketParty : public cPacketGeneralInfo<T> {
public:
	eUI08 subsubcommand;

	cSubPacketParty();
};

#define PARTY_SUBCMD_ADD 1

/*!
\brief Add a new member to party
\author Endymion
\since 0.83
\note 0xBF - 6 - 1
*/
class clPacketAddPartyMember : public cSubPacketParty< cClientPacket > {
public:
	eUI32 member; //!< the member, if 0 the targeting cursor appears

	clPacketAddPartyMember();
	void receive( NXWCLIENT ps );
};

/*!
\brief Send all party members after an add
\author Endymion
\since 0.83
\note 0xBF - 6 - 1
*/
class csPacketAddPartyMembers : public cSubPacketParty< cServerPacket > {
private:
	eUI08 count;	//!< members count
public:
	std::vector<P_PARTY_MEMBER>* members;
	void send( NXWCLIENT ps );

	csPacketAddPartyMembers();

};

#define PARTY_SUBCMD_REMOVE 2

/*!
\brief Remove a member to party
\author Endymion
\since 0.83
\note 0xBF - 6 - 2
*/
class clPacketRemovePartyMember : public cSubPacketParty< cClientPacket > {
public:
	eUI32 member; //!< the member, if 0 the targeting cursor appears

	clPacketRemovePartyMember();
	void receive( NXWCLIENT ps );
};

/*!
\brief Remove a member to party, and resend list
\author Endymion
\since 0.83
\note 0xBF - 6 - 2
*/
class csPacketRemovePartyMembers : public cSubPacketParty< cServerPacket > {
private:
	eUI08 count;	//!< members count
public:
	eUI32 member; //!< the member removed
	std::vector<P_PARTY_MEMBER>* members; //!< all members

	csPacketRemovePartyMembers();
	void send( NXWCLIENT ps );

};

#define PARTY_SUBCMD_MESSAGE 3

/*!
\brief Tell to a party member a message
\author Endymion
\since 0.83
\note 0xBF - 6 - 3
\note message is NULL terminated
*/
class clPacketPartyTellMessage : public cSubPacketParty< cClientPacket > {
public:
	eSERIAL member; //!< to member
	std::wstring message; //!< the message

	clPacketPartyTellMessage();
	void receive( NXWCLIENT ps );

};

/*!
\brief Tell to a party member a message
\author Endymion
\since 0.83
\note 0xBF - 6 - 3
\note message is NULL terminated
*/
class csPacketPartyTellMessage : public cSubPacketParty< cServerPacket > {
public:
	eSERIAL member; //!< from member
	std::wstring* message; //!< the message

	csPacketPartyTellMessage();
	void send( NXWCLIENT ps );

};

#define PARTY_SUBCMD_BROADCAST 4

/*!
\brief Tell to all party members a message
\author Endymion
\since 0.83
\note 0xBF - 6 - 4
\note message is NULL terminated
*/
class csPacketPartyTellAllMessage : public cSubPacketParty< cServerPacket > {
public:
	eSERIAL from; //!< from member
	std::wstring* message; //!< the message

	csPacketPartyTellAllMessage();
	void send( NXWCLIENT ps );

};

/*!
\brief Tell to all party members a message
\author Endymion
\since 0.83
\note 0xBF - 6 - 4
\note message is NULL terminated
*/
class clPacketPartyTellAllMessage : public cSubPacketParty< cClientPacket > {
public:
	std::wstring message; //!< the message

	clPacketPartyTellAllMessage();
	void receive( NXWCLIENT ps );

};

#define PARTY_SUBCMD_CANLOOT 6

/*!
\brief Other party memeber can loot this
\author Endymion
\since 0.83
\note 0xBF - 6 - 6
*/
class clPacketPartyCanLoot : public cSubPacketParty< cClientPacket > {
public:
	eBool canLoot; //!< other member can loot this

	clPacketPartyCanLoot();
	void receive( NXWCLIENT ps );
};

#define PARTY_SUBCMD_INVITE 7

/*!
\brief Accept party invitation
\author Endymion
\since 0.83
\note 0xBF - 6 - 7
*/
class csPacketPartyInvite : public cSubPacketParty< cServerPacket > {
public:
	eSERIAL leader; //!< party leader

	csPacketPartyInvite();
	void send( NXWCLIENT ps );
};

#define PARTY_SUBCMD_ACCEPT 8

/*!
\brief Accept party invitation
\author Endymion
\since 0.83
\note 0xBF - 6 - 8
*/
class clPacketPartyAccept : public cSubPacketParty< cClientPacket > {
public:
	eSERIAL leader; //!< party leader

	clPacketPartyAccept();
	void receive( NXWCLIENT ps );
};

#define PARTY_SUBCMD_DECLINE 9

/*!
\brief Decline party invitation
\author Endymion
\since 0.83
\note 0xBF - 6 - 9
*/
class clPacketPartyDecline : public cSubPacketParty< cClientPacket > {
public:
	eSERIAL leader; //!< party leader

	clPacketPartyDecline();
	void receive( NXWCLIENT ps );
};







//@}


#endif
