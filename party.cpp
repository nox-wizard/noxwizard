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

/*!
 \brief Party related stuff, include cPartys functions
 */
namespace partySystem {

	int g_nPartyShareKarma = 1;
	int g_nPartyShareFame = 1;
	int g_nPartyCanPMsg = 1;
	int g_nPartyCanBroadcast = 1;
	PARTY_LIST partylist;
	UI32 nextparty = 0;

	/*!
	 \brief Sends the permission menu
	 \param ps client to send the packet to
	 */
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

	/*!
	 \brief Target handler for add member
	 \param ps client with fired the target
	 */
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
			party_current= newParty();
			if (!addMember( party_current, pc )) {
				ps->sysmsg( TRANSLATE("Sorry, can't build a party now (internal server error)!"));
				return;
			}


			/////////FOR WHAT?
			pc->requestedParty = INVALID;
			//////////////

			if( isLeader( party_current, pc ))
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

		P_CHAR leader=getLeader( party_current );
		if( !ISVALIDPC(leader) ) {
			target->requestedParty=INVALID;
			return;
		}
		target->reqPartySerial = leader->getSerial32();
		askPartyPermission( target->getClient() );
		target->sysmsg( TRANSLATE("You are invited in a party lead by %s. Would you accept ?"), leader->getCurrentNameC());

	}

	/*!
	 \brief Processes a party network packet
	 \author Xanathar
	 \param ps client which sent the packet
	 */
	void processInputPacket( NXWCLIENT ps )
	{

	 	if(ps==NULL)
			return;

		P_CHAR pc=ps->currChar();
		VALIDATEPC(pc);

		NXWSOCKET s=ps->toInt();
		
		UI08 subsubcommand = buffer[s][5];  // party system


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
						removeMember( kicked ); //autoremove
					else
						kickMember(kicked); //kick another player


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
				talkToOthers( pc, mex );
				return; }
			case NET_LOOTMODE:
				pc->partyCanLoot = buffer[s][6]!=0;
				if (pc->partyCanLoot)
					ps->sysmsg( TRANSLATE("Party can loot you!"));
				else
					ps->sysmsg( TRANSLATE("Party CANNOT loot you!"));
			}

	}

	/*!
	 \brief Menu handler for the join permission menu
	 \note contains join code
	 \author Xanathar
	 \param s socket of menu pressed
	 \param btn button pressed
	 \param seed menu seed (see cCustomMenu for all details)
	 */
	void cPartyMenu::buttonSelected(NXWSOCKET  s, UI16 btn, int seed )
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
				talkToOthers( req, str  );
				if( membersNumber( req ) == 1 ) // one player party is not valid
					kickMember( req );
			}
			pc->requestedParty = INVALID;
			pc->reqPartySerial = INVALID;
			pc->party = INVALID;
		}
		else 
		{

			if ( !addMember( pc->requestedParty, pc ) )
			{
				pc->party = pc->reqPartySerial = pc->requestedParty = INVALID;
				pc->sysmsg( TRANSLATE("This party is no more valid. Sorry."));
				return;
			}

			
			pc->reqPartySerial = pc->requestedParty = INVALID;

			std::string str(" ");
			str += pc->getCurrentNameC();
			str += " entered the party.";

			talkToOthers( pc, str );
			
			if (pc->partyCanLoot)
				pc->sysmsg( TRANSLATE("Party can loot you!"));
			else 
				pc->sysmsg( TRANSLATE("Party CANNOT loot you!"));
			
		}


	};

	cParty::cParty( SERIAL ser )
	{
		serial=ser;
	}

	bool cParty::isMember( P_CHAR pc )
	{
		VALIDATEPCR( pc, false );
		SERIAL_VECTOR::iterator iter( members.begin() ), end( members.end() );
		for( ; iter!=end; iter++ ) {
			if( (*iter)==pc->getSerial32() ) 
				return true;
		}
		return false;
	}


	void cParty::talkToOthers( std::string s )
	{
		SERIAL_VECTOR::iterator iter( members.begin() ), end( members.end() );
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

		members.push_back( pc->getSerial32() );
		pc->party=serial;
		sendPartyListAll();
		return true;
	}


	void cParty::removeMember( P_CHAR pc )
	{

		VALIDATEPC(pc);

		SERIAL_VECTOR::iterator iter( find( members.begin( ), members.end(), pc->getSerial32() ) );
		if( iter!=members.end() ) {
			members.erase( iter );
			pc->party=INVALID;
			sendEmptyList( pc->getClient() );
			sendPartyListAll();
		}

	}

	void cParty::kickMember( P_CHAR pc )
	{

		VALIDATEPC(pc);

		SERIAL_VECTOR::iterator iter( find( members.begin( ), members.end(), pc->getSerial32() ) );
		if( iter!=members.end() ) {
			pc->sysmsg(TRANSLATE("You've been kicked from the party"));
			removeMember( pc );
		}

	}

	/*!
	 \brief Refreshes the party list to a single member
	 */
	void cParty::sendEmptyList( NXWCLIENT ps )
	{
		if( ps==NULL )
			return;

		P_CHAR pc=ps->currChar();
		VALIDATEPC(pc);
		
		//0000: bf 0f 00 00 06 02 01 00 00 62 60 00 00 5f e8 -- : .........b`.._.
		UI08 packet[14]; // (3 bytes are a "anti-failure boundary" ;D)
		memset(packet, 0, 14);

		//assemble packet
		UI16 lenght = 11;

		packet[0] = '\xbf';
		ShortToCharPtr(lenght, packet+1);
		packet[3] = '\x00';
		packet[4] = '\x06';
		packet[5] = '\x02';
		packet[6] = '\x00';
		LongToCharPtr(pc->getSerial32(), packet+7);

		ps->send( packet, lenght);
	}

	void cParty::sendPartyList( NXWCLIENT ps )
	{

		if( ps==NULL )
			return;

		UI32 num=members.size();
		if( num == 0 )
			return;

		UI08 packet[50]; //7+10*4+3 = 50 (3 bytes are a "anti-failure boundary" ;D)

		memset(packet, 0, 50);

		//assemble packet
		UI16 lenght = (4*num)+7;

		packet[0] = '\xbf';
		ShortToCharPtr(lenght, packet+1);
		packet[3] = '\x00';
		packet[4] = '\x06';
		packet[5] = '\x01';
		packet[6] = num & 0xFF;

		int p = 7;
		SERIAL_VECTOR::iterator iter( members.begin() ), end( members.end() );
		for( ; iter!=end; iter++ ) {
			if( (*iter) ) {
				LongToCharPtr(*iter, packet+p);
				p += 4;
			}
		}

		ps->send( packet, lenght);
	}

	//! refreshes the party list to all members
	void cParty::sendPartyListAll()
	{
		SERIAL_VECTOR::iterator iter( members.begin() ), end( members.end() );
		for( ; iter!=end; iter++ ) {
			P_CHAR pc = pointers::findCharBySerial( (*iter) );
			if( ISVALIDPC(pc) )
				sendPartyList(pc->getClient());
		}
	}



	P_CHAR cParty::getLeader()
	{
		if( members.size() >0 )
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
		SERIAL_VECTOR::iterator iter( members.begin() ), end( members.end() );
		for( ; iter!=end; iter++ ) {
			P_CHAR pc = pointers::findCharBySerial( (*iter) );
			if( ISVALIDPC(pc) ) {
				pc->party=INVALID;
				pc->reqPartySerial=INVALID;
				pc->requestedParty=INVALID;
				sendEmptyList( pc->getClient() );
			}
		}

	}

	UI32 cParty::membersNumber()
	{
		return members.size();
	}

	SERIAL newParty()
	{
		SERIAL newp= nextparty;
		partylist.insert( make_pair( newp, cParty(newp) ) );
		nextparty++;
		return newp;
	}


	bool addMember( SERIAL party, P_CHAR pc )
	{
		VALIDATEPCR(pc,false);
		PARTY_LIST::iterator iter( partylist.find( party ) ), end( partylist.end() );
		if( iter!=end )
			return iter->second.addMember( pc );
		else
			return false;
		
	}

	void removeMember( P_CHAR pc )
	{
		VALIDATEPC(pc);
		PARTY_LIST::iterator iter( partylist.find( pc->party ) ), end( partylist.end() );
		if( iter!=end ) {
			if( iter->second.isLeader( pc ) ) { //disband party
				iter->second.eraseAllMembers();
				partylist.erase( iter );
			}
			else {
				iter->second.removeMember( pc );
				if( iter->second.membersNumber() ==1 ) {
					iter->second.eraseAllMembers();
					partylist.erase( iter );
				}	
			}
		}
	}

	void kickMember( P_CHAR pc )
	{
		VALIDATEPC(pc);
		PARTY_LIST::iterator iter( partylist.find( pc->party ) ), end( partylist.end() );
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

	bool isMember( SERIAL party, P_CHAR pc )
	{
		VALIDATEPCR(pc,false);
		PARTY_LIST::iterator iter( partylist.find( party ) ), end( partylist.end() );
		if( iter!=end )
			return iter->second.isMember( pc );
		else
			return false;
	}

	bool isLeader( SERIAL party, P_CHAR pc )
	{
		VALIDATEPCR(pc,false);
		PARTY_LIST::iterator iter( partylist.find( party ) ), end( partylist.end() );
		if( iter!=end )
			return iter->second.isLeader( pc );
		else
			return false;
	}

	P_CHAR getLeader( SERIAL party )
	{
		PARTY_LIST::iterator iter( partylist.find( party ) ), end( partylist.end() );
		if( iter!=end )
			return iter->second.getLeader( );
		else
			return NULL;
	}

	void talkToOthers( P_CHAR pc, std::string s )
	{
		VALIDATEPC(pc);
		PARTY_LIST::iterator iter( partylist.find( pc->party ) ), end( partylist.end() );
		if( iter!=end )
			iter->second.talkToOthers( s );
	}

	UI32 membersNumber( P_CHAR pc )
	{
		VALIDATEPCR(pc, 0);
		PARTY_LIST::iterator iter( partylist.find( pc->party ) ), end( partylist.end() );
		if( iter!=end )
			return iter->second.membersNumber();
		else
			return 0;
	}

} // namespace

