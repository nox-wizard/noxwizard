  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __GUILD__
#define __GUILD__

#include "nxwcommn.h"
#include "scp_parser.h"

namespace Organisation
{

	class 	Guild;
	typedef SERIAL 	GUILDID;
	typedef Guild*	P_GUILD;

	class Guild
	{
		typedef map< SERIAL, P_GUILD >		GUILD_TBL		;
		typedef GUILD_TBL::iterator		GUILD_TBL_IT		;

		typedef SERIAL 				MEMBERID		;
		class	Member;
		typedef Member*				P_MEMBER		;

		typedef map< MEMBERID, GUILDID >	MEMBER_OF_GUILD_IDX	;
		typedef MEMBER_OF_GUILD_IDX::iterator 	MEMBER_OF_GUILD_IDX_IT	;

		public:
			class Member
			{
				friend class Guild;

				typedef map< MEMBERID, P_MEMBER >	MEMBER_TBL		;
				typedef MEMBER_TBL::iterator		MEMBER_TBL_IT		;
				//
				//	Public class methods
				//
				public:
					static LOGICAL		load()				;
					static LOGICAL		save()				;
					static LOGICAL		exists( MEMBERID who )		;
					static LOGICAL		add( P_MEMBER pNewMember )	;
				//
				//	Private class methods
				//
				private:
					static LOGICAL		parse()				;
				//
				//	Private class properties
				//
				private:
					static MEMBER_TBL	members				;
				//
				//	Object creators and destructors
				//
				public:
								Member()			;
								~Member()			;
				//
				//	Public object methods
				//
				public:
					MEMBERID		getId()				;
					GUILDID			getGuildId()			;
				//
				//	Private object methods
				//
				private:
					MEMBERID		id				;
					GUILDID			guildId				;
					cGameDate		dateCreated			;
			};
		//
		//	Public class methods
		//
		public:
			static	LOGICAL		load()			;
			static	LOGICAL		save()			;
			static	LOGICAL		exists( GUILDID guild )	;
		//
		//	Private class methods
		//
		private:
			static	LOGICAL		parse()			;
		//
		//	Private class properties
		//
		private:
			static	GUILD_TBL	guilds			;
			static	cScpScript*	wsc			;
			static	std::string	wscName			;
		//
		//	Object creators and destructors
		//
		public:
						Guild()			;
						~Guild()		;
		//
		//	Public object methods
		//
		public:
			GUILDID			getId()			;
		private:
			GUILDID			id			;
			std::string		name			;
			std::string		abbreviation		;
			cGameDate		dateCreated		;
	};

}

#endif
