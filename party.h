  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __PARTY_H__
#define __PARTY_H__

#include "nxwcommn.h"
#include "oldmenu.h"
#include "target.h"

#include <list>

#define PARTY_BROADCAST_COLOR 0
#define PARTY_PRIVATE_COLOR 0

class cParty
{
	
	private:
		void sendToAllMember( P_SERVER_PACKET pkg );
	
	public:
		std::vector< P_PARTY_MEMBER >	members;	//!< all members
		std::vector< SERIAL > candidates;	//!< all candidates

		SERIAL serial;	//!< party serial

		cParty();
		~cParty();
			
		void addMember( P_CHAR member );
		void removeMember( P_CHAR member );
		P_PARTY_MEMBER getMember( SERIAL member );

		SERIAL getLeader();

		void addCandidate( P_CHAR leader, P_CHAR candidate );
		void removeCandidate( SERIAL serial );
		bool isCandidate( SERIAL serial );

		void privateMessage( SERIAL from, SERIAL to, std::wstring& s, COLOR color=PARTY_PRIVATE_COLOR );
		void talkToOthers( SERIAL from, std::wstring& s, COLOR color=PARTY_BROADCAST_COLOR );
		void talkToAll( std::wstring& s, COLOR color=PARTY_BROADCAST_COLOR );
			
};

class cPartyMember {
	
public:

	SERIAL serial;
	bool canLoot;

	cPartyMember( SERIAL member );

};

class cPartys {

	private:
		std::map< SERIAL, P_PARTY > partys;	//!< all partys
		SERIAL currentSerial;	//!< current serial
	public:
		bool shareKarma;
		bool shareFame;
		bool canPMsg;
		bool canBroadcast;

		P_PARTY	createParty( );
		P_PARTY	getParty( SERIAL serial );
		void removeParty( SERIAL serial );
		void recive( NXWCLIENT ps );

		cPartys();
		~cPartys();

};

extern cPartys Partys;

#endif //__PARTY_H__
