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

namespace partySystem
{
	class cParty
	{
		protected:
			SERIAL_VECTOR members;
		public:
			cParty( SERIAL ser );

			SERIAL serial;	//!< party serial
			
			bool addMember( P_CHAR pc );
			void removeMember( P_CHAR pc );
			void kickMember( P_CHAR pc );
			void eraseAllMembers();
			bool isMember( P_CHAR pc );
			bool isLeader( P_CHAR pc );
			P_CHAR getLeader();
			UI32 membersNumber();

			void talkToOthers( std::string s );
			
			void sendPartyList( NXWCLIENT ps );
			void sendPartyListAll();
			void sendEmptyList( NXWCLIENT ps );
	};

	class cPartyMenu : public cCustomMenu {
	public:
		void buttonSelected(NXWSOCKET  s, unsigned short int btn, int seed );
	};

	typedef std::map< SERIAL, cParty> PARTY_LIST;

	extern int g_nPartyShareKarma;
	extern int g_nPartyShareFame;
	extern int g_nPartyCanPMsg;
	extern int g_nPartyCanBroadcast;
	extern PARTY_LIST partylist;	//!< All party
	extern UI32 nextparty;		//!< next party free

	#define NET_ADDMEMBER 1
	#define NET_DELMEMBER 2
	#define NET_MESSAGE   3
	#define NET_BROADCAST 4
	#define NET_LOOTMODE  6

	#define PARTYSYSTARGET 192

	void targetParty( NXWCLIENT ps );
	void processInputPacket( NXWCLIENT ps );
	void askPartyPermission( NXWCLIENT ps );

	SERIAL newParty();

	bool addMember( SERIAL party, P_CHAR pc );
	void removeMember( P_CHAR pc );
	void kickMember( P_CHAR pc );

	bool isMember( SERIAL party, P_CHAR pc );
	bool isLeader( SERIAL party, P_CHAR pc );

	P_CHAR getLeader( SERIAL party );

	void talkToOthers( P_CHAR pc, std::string s );
	UI32 membersNumber( P_CHAR pc );

}

#endif //__PARTY_H__

