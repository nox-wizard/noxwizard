  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "debug.h"
#include "sndpkg.h"
#include "custmenu.h"
#include "speech.h" //for unicode conversions
#include "collector.h"
#include "party.h"

namespace PartySystem {

	int g_nPartyShareKarma = 1;
	int g_nPartyShareFame = 1;
	int g_nPartyCanPMsg = 1;
	int g_nPartyCanBroadcast = 1;

// Description	     : sends the permission menu
// Argument	     : NXWSOCKET  s -> socket to send packet to
void askPartyPermission( NXWCLIENT ps )
{
	if( ps==NULL ) 
		return;

	P_CHAR p = ps->currChar();;
	VALIDATEPC(p);

	if (p->customMenu != NULL) {
		safedelete(p->customMenu);
	}

	p->customMenu = new cPartyMenu();
	p->customMenu->setParameters(2,1);
	p->customMenu->setStyle(MENUSTYLE_TRASPARENCY, 0x481);
	p->customMenu->setTitle("Party invitation");
	p->customMenu->addMenuItem(0,0,"Accept invitation");
	p->customMenu->addMenuItem(0,1,"Deny invitation");
	p->customMenu->buildMenu();
	p->customMenu->showMenu(ps->toInt());

}


// Description	 : target handler for add member
// Argument	     : NXWSOCKET  s -> socket which fired the target
void targetParty( NXWCLIENT ps )
{
	if( ps==NULL)
		return;
	
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);
	
	P_CHAR target=pointers::findCharBySerPtr(buffer[ps->toInt()]+7);
	VALIDATEPC(target);

	//if player has choosen a one-person party, exit from here :)
	if (target->getSerial32()==pc->getSerial32()) 
		return;

	
	SERIAL party_current = pc->party;

	if( party_current == INVALID ) {
		party_current= Partys->newParty();
		if (!Partys->addMember( party_current, pc )) {
			ps->sysmsg( TRANSLATE("Sorry, can't build a party now (internal server error)!"));
			return;
		}


		/////////FOR WHAT?
		pc->requestedParty = INVALID;
		//////////////

		if( Partys->isLeader( party_current, pc ))
			ps->sysmsg( TRANSLATE("You're the leader of this party!"));
	}

	//check if char is already in a party
	if( target->party != INVALID ) {
		ps->sysmsg( TRANSLATE("The selected person is already in a party. "));
		return;
	}

	if ( !target->IsOnline() ) {
		ps->sysmsg( TRANSLATE("The selected person must be online to give you permission. "));
		return;
	}

	//go on!! -- request the char permission
	target->requestedParty = party_current;

	P_CHAR leader=Partys->getLeader( party_current );
	if( !ISVALIDPC(leader) ) {
		target->requestedParty=INVALID;
		return;
	}
	target->reqPartySerial = leader->getSerial32();
	askPartyPermission( target->getClient() );
	target->sysmsg( TRANSLATE("You are invited in a party lead by %s. Would you accept ?"), leader->getCurrentNameC());

}



///////////////////////////////////////////////////////////////////
// Function name     : processInputPacket
// Description	     : processes a party network packet
// Return type	     : void
// Author	     : Xanathar
// Argument	     : NXWSOCKET  s -> socket which sent the packet
// Changes	     : none yet
void processInputPacket( NXWCLIENT ps )
{

 	if(ps==NULL)
		return;

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	NXWSOCKET s=ps->toInt();
	
	int subsubcommand = buffer[s][5];  // party system


	switch(subsubcommand) {
		case NET_ADDMEMBER: 
			target(s, 0, 1, 0, PARTYSYSTARGET, 	TRANSLATE("Select the one to add to the party..") );
			return;
		case NET_DELMEMBER: {
			if (pc->party==INVALID) {
				ps->sysmsg( TRANSLATE("You're not in a party!"));
				return;
			}
			P_CHAR kicked=pointers::findCharBySerPtr(buffer[s]+6);
			if( ISVALIDPC(kicked) && pc->party==kicked->party )
				if( pc->getSerial32()==kicked->getSerial32() )
					Partys->removeMember( kicked ); //autoremove
				else
					Partys->kickMember(kicked); //kick another player


			}
			return;
		case NET_MESSAGE: {
			if (pc->party==INVALID) {
				ps->sysmsg( TRANSLATE("You're not in a party!"));
				return;
			}
			P_CHAR to=pointers::findCharBySerPtr(buffer[s]+6);
			VALIDATEPC(to);
			wchar2char(const_cast<const char *>(reinterpret_cast<char*>(buffer[s]+0xf)));
			to->sysmsg( TRANSLATE("Private message from %s : %s"), pc->getCurrentNameC(), Unicode::temp);
			return; }
		case NET_BROADCAST: {
			if (pc->party==INVALID) {
				ps->sysmsg( TRANSLATE("You're not in a party!"));
				return;
			}
			std::string mex;
			wchar2char(const_cast<const char *>(reinterpret_cast<char*>(buffer[s]+0x9)));
			mex="Party message from ";
			mex+= pc->getCurrentName();
			mex+=" : ";
			mex+= Unicode::temp;
			Partys->talkToOthers( pc, mex );
			return; }
		case NET_LOOTMODE:
			pc->partyCanLoot = buffer[s][6]!=0;
			if (pc->partyCanLoot)
				ps->sysmsg( TRANSLATE("Party can loot you!"));
			else
				ps->sysmsg( TRANSLATE("Party CANNOT loot you!"));
		}

}

};



cPartyMenu::cPartyMenu() { }

///////////////////////////////////////////////////////////////////
// Function name :buttonSelected
// Description   : Menu handler for the join permission menu -- contains join code
// Return type	 : void
// Author	     : Xanathar
// Argument	     : NXWSOCKET  s -> socket of menu pressed
// Argument	     : unsigned short int btn -> button pressed
// Argument	     : int seed -> menu seed (see cCustomMenu for all details)
// Changes	     : none yet
void cPartyMenu::buttonSelected(NXWSOCKET  s, unsigned short int btn, int seed )
{
		
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
		
		
	int page = (btn-10)/m_nNumPerPage;
	int item = (btn-10)%m_nNumPerPage;
	if (btn < 10) page = item = -1;

	if (seed!=m_nSeed) {
		pc->sysmsg( TRANSLATE("You should be faster when choosing an option in your life.."));
		return;
	}

	if (item != 0) {
		P_CHAR req=pointers::findCharBySerial( pc->reqPartySerial );
		if( ISVALIDPC( req ) ) {
			std::string str= pc->getCurrentName();
			str+=" refuse the party";
			Partys->talkToOthers( req, str  );
			if( Partys->membersNumber( req ) == 1 ) // one player party is not valid
				Partys->kickMember( req );
		}
		pc->requestedParty = INVALID;
		pc->reqPartySerial = INVALID;
		pc->party = INVALID;
	}
	else 
	{

		if ( !Partys->addMember( pc->requestedParty, pc ) )
		{
			pc->party = pc->reqPartySerial = pc->requestedParty = INVALID;
			pc->sysmsg( TRANSLATE("This party is no more valid. Sorry."));
			return;
		}

		
		pc->reqPartySerial = pc->requestedParty = INVALID;

//		std::string str = ( " %s entered the party ", pc->getCurrentNameC() );
		std::string str(" ");
		str += pc->getCurrentNameC();
		str += " entered the party.";

		Partys->talkToOthers( pc, str );
		
		if (pc->partyCanLoot)
			pc->sysmsg( TRANSLATE("Party can loot you!"));
		else 
			pc->sysmsg( TRANSLATE("Party CANNOT loot you!"));
		
	}


};




cParty::cParty( SERIAL ser )
{
	this->members.clear();
	this->serial=ser;
}


cParty::~cParty() { }



bool cParty::isMember( P_CHAR pc )
{
	VALIDATEPCR( pc, false );
	MEMBER_LIST::iterator iter( members.begin() ), end( members.end() );
	for( ; iter!=end; iter++ ) {
		if( (*iter)==pc->getSerial32() ) 
			return true;
	}
	return false;
}


void cParty::talkToOthers( std::string s )
{
	MEMBER_LIST::iterator iter( members.begin() ), end( members.end() );
	for( ; iter!=end; iter++ ) {
		P_CHAR pc = pointers::findCharBySerial( (*iter) );
		if( ISVALIDPC(pc) ) {
			pc->sysmsg( (char*)s.c_str() );
		}
	}
}


bool cParty::addMember( P_CHAR pc )
{
	VALIDATEPCR( pc, false );
	if( pc->party!=INVALID )
		return false;

	this->members.push_back( pc->getSerial32() );
	pc->party=this->serial;
	sendPartyListAll();
	return true;
}


void cParty::removeMember( P_CHAR pc )
{

	VALIDATEPC(pc);

	MEMBER_LIST::iterator iter( find( members.begin( ), members.end(), pc->getSerial32() ) );
	if( iter!=members.end() ) {
		this->members.erase( iter );
		pc->party=INVALID;
		this->sendEmptyList( pc->getClient() );
		sendPartyListAll();
	}

}

void cParty::kickMember( P_CHAR pc )
{

	VALIDATEPC(pc);

	MEMBER_LIST::iterator iter( find( members.begin( ), members.end(), pc->getSerial32() ) );
	if( iter!=members.end() ) {
		pc->sysmsg(TRANSLATE("You've been kicked from the party"));
		this->removeMember( pc );
	}

}



// Description	     : refreshes the party list to a single member
void cParty::sendEmptyList( NXWCLIENT ps )
{
	if( ps==NULL )
		return;

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);
	
	//0000: bf 0f 00 00 06 02 01 00 00 62 60 00 00 5f e8 -- : .........b`.._.
	//m_serOldLeader
	char packet[50]; //7+10*4+3 = 50 (3 bytes are a "anti-failure boundary" ;D)
	memset(packet, 0, 49);

	//assemble packet
	short lenght = +7+4;

	packet[0] = '\xbf';
	packet[1] = static_cast<char>(lenght >> 8);
	packet[2] = static_cast<char>(lenght & 0xFF);
	packet[3] = '\x00';
	packet[4] = '\x06';
	packet[5] = '\x02';
	packet[6] = '\x00'; //static_cast<char>(num & 0xFF);
	packet[7] = pc->getSerial().ser1;
	packet[8] = pc->getSerial().ser2;
	packet[9] = pc->getSerial().ser3;
	packet[10] = pc->getSerial().ser4;

	ps->send( packet, lenght);
}

void cParty::sendPartyList( NXWCLIENT ps )
{

	if( ps==NULL )
		return;

	UI32 num=this->members.size();
	if( num == 0 )
		return;

	char packet[50]; //7+10*4+3 = 50 (3 bytes are a "anti-failure boundary" ;D)

	memset(packet, 0, 49);

	//assemble packet
	short lenght = (4*num)+7;

	packet[0] = '\xbf';
	packet[1] = static_cast<char>(lenght >> 8);
	packet[2] = static_cast<char>(lenght & 0xFF);
	packet[3] = '\x00';
	packet[4] = '\x06';
	packet[5] = '\x01';
	packet[6] = static_cast<char>(num & 0xFF);

	int p = 7;
	MEMBER_LIST::iterator iter( members.begin() ), end( members.end() );
	for( ; iter!=end; iter++ ) {
		if( (*iter) ) {
			packet[p++] = static_cast<char>(((*iter)>>24) & 0xFF);
			packet[p++] = static_cast<char>(((*iter)>>16) & 0xFF);
			packet[p++] = static_cast<char>(((*iter)>> 8) & 0xFF);
			packet[p++] = static_cast<char>( (*iter)      & 0xFF);
		}
	}

	ps->send( packet, lenght);
}

//! refreshes the party list to all members
void cParty::sendPartyListAll()
{
	MEMBER_LIST::iterator iter( members.begin() ), end( members.end() );
	for( ; iter!=end; iter++ ) {
		P_CHAR pc = pointers::findCharBySerial( (*iter) );
		if( ISVALIDPC(pc) )
			sendPartyList(pc->getClient());
	}
}



P_CHAR cParty::getLeader()
{
	if( this->members.size() >0 )
		return pointers::findCharBySerial(members[0]);
	else
		return NULL;
}

bool cParty::isLeader( P_CHAR pc )
{
	VALIDATEPCR( pc, false );
	P_CHAR leader = getLeader();
	if(ISVALIDPC(leader)) 
		return (leader->getSerial32()==pc->getSerial32());
	else 
		return false;

}

void cParty::eraseAllMembers()
{
	MEMBER_LIST::iterator iter( members.begin() ), end( members.end() );
	for( ; iter!=end; iter++ ) {
		P_CHAR pc = pointers::findCharBySerial( (*iter) );
		if( ISVALIDPC(pc) ) {
			pc->party=INVALID;
			pc->reqPartySerial=INVALID;
			pc->requestedParty=INVALID;
			this->sendEmptyList( pc->getClient() );
		}
	}

}

UI32 cParty::membersNumber()
{
	return members.size();
}






cPartys::cPartys() {
	this->partylist.clear();
	this->nextparty=0;
}

cPartys::~cPartys() {
}

SERIAL cPartys::newParty()
{
	SERIAL newp= this->nextparty;
	this->partylist.insert( make_pair( newp, cParty(newp) ) );
	nextparty++;
	return newp;
}


bool cPartys::addMember( SERIAL party, P_CHAR pc )
{
	VALIDATEPCR(pc,false);
	PARTY_LIST::iterator iter( this->partylist.find( party ) ), end( this->partylist.end() );
	if( iter!=end )
		return iter->second.addMember( pc );
	else
		return false;
	
}

void cPartys::removeMember( P_CHAR pc )
{
	VALIDATEPC(pc);
	PARTY_LIST::iterator iter( this->partylist.find( pc->party ) ), end( this->partylist.end() );
	if( iter!=end ) {
		if( iter->second.isLeader( pc ) ) { //disband party
			iter->second.eraseAllMembers();
			this->partylist.erase( iter );
		}
		else {
			iter->second.removeMember( pc );
			if( iter->second.membersNumber() ==1 ) {
				iter->second.eraseAllMembers();
				this->partylist.erase( iter );
			}	
		}
	}
}

void cPartys::kickMember( P_CHAR pc )
{
	VALIDATEPC(pc);
	PARTY_LIST::iterator iter( this->partylist.find( pc->party ) ), end( this->partylist.end() );
	if( iter!=end ) {
		if( iter->second.isLeader( pc ) ) { //disband party
			iter->second.talkToOthers( "The Glorious Adventures relinquish their Swords, longing for the quiet fireside's home." );
			iter->second.removeMember(pc);
		}
		else {
			iter->second.kickMember( pc );
			if( iter->second.membersNumber() ==1 ) {
				iter->second.talkToOthers( "The Glorious Adventures relinquish their Swords, longing for the quiet fireside's home." );
				iter->second.removeMember(pc);
			}	
		}
	}
}

bool cPartys::isMember( SERIAL party, P_CHAR pc )
{
	VALIDATEPCR(pc,false);
	PARTY_LIST::iterator iter( this->partylist.find( party ) ), end( this->partylist.end() );
	if( iter!=end )
		return iter->second.isMember( pc );
	else
		return false;
}

bool cPartys::isLeader( SERIAL party, P_CHAR pc )
{
	VALIDATEPCR(pc,false);
	PARTY_LIST::iterator iter( this->partylist.find( party ) ), end( this->partylist.end() );
	if( iter!=end )
		return iter->second.isLeader( pc );
	else
		return false;
}

P_CHAR cPartys::getLeader( SERIAL party )
{
	PARTY_LIST::iterator iter( this->partylist.find( party ) ), end( this->partylist.end() );
	if( iter!=end )
		return iter->second.getLeader( );
	else
		return NULL;
}

void cPartys::talkToOthers( P_CHAR pc, std::string s )
{
	VALIDATEPC(pc);
	PARTY_LIST::iterator iter( this->partylist.find( pc->party ) ), end( this->partylist.end() );
	if( iter!=end )
		iter->second.talkToOthers( s );
}

UI32 cPartys::membersNumber( P_CHAR pc )
{
	VALIDATEPCR(pc, 0);
	PARTY_LIST::iterator iter( this->partylist.find( pc->party ) ), end( this->partylist.end() );
	if( iter!=end )
		return iter->second.membersNumber();
	else
		return 0;
}













