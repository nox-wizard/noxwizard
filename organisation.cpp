#include "organisation.h"
#include "gamedate.h"

namespace Organisation
{
	class Member
	{
		public:
					Member();
					Member( P_CHAR pChar );
					~Member();
		private:
			SERIAL		serial;
			cGameDate	dateJoined;
		public:
			SERIAL		getSerial();
	};

	Member::Member()
	{
		serial = INVALID;
	}

	Member::Member( P_CHAR pChar )
	{
		serial = pChar->getSerial32();
	}

	Member::~Member()
	{
	}

	SERIAL Member::getSerial()
	{
		return serial;
	}

	typedef Member*			P_MEMBER;
	typedef map< SERIAL, P_MEMBER > memberMap;
	typedef memberMap::iterator	memberMapIterator;

	class Organisation
	{
		public:
					Organisation();
					Organisation( P_ITEM pFocus );
			virtual		~Organisation();
		private:
			SERIAL		focusSerial;
			std::string	name;
			Member*		leader;
			memberMap	members;
			cGameDate	dateCreated;
		public:
			LOGICAL		isMember( SERIAL who );
			SI32		numberOfMembers();
			virtual LOGICAL	addMember( P_MEMBER pWho );
			virtual LOGICAL	setLeader( P_MEMBER pWho );
			LOGICAL		isLeader( Member& who );
			LOGICAL		isLeader( SERIAL who );
	};

	Organisation::Organisation()
	{
		focusSerial 	= INVALID;
	}

	Organisation::Organisation( P_ITEM pFocus )
	{
		focusSerial 	= pFocus->getSerial32();
	}

	Organisation::~Organisation()
	{
	}

	LOGICAL Organisation::isMember( SERIAL who )
	{
		return members.find( who ) != members.end();
	}

	/*!
	\brief	Add a member to an organisation
	\author Sparhawk
	\param	pWho pointer to member instance.
	\return boolean true on success, otherwise false
	\since	0.82a
	*/
	LOGICAL Organisation::addMember( P_MEMBER pWho )
	{
		SERIAL serial = pWho->getSerial();

		if( !isMember( serial ) )
		{
			members[ serial ] = pWho;
			return true;
		}
		else
			return false;
	}

	/*!
	\brief	Set the leader of an organisation
	\note	A leader has to be a member of an organisation.
	\author Sparhawk
	\param	pWho pointer to member instance.
	\return boolean true on success, otherwise false
	\since	0.82a
	*/
	LOGICAL Organisation::setLeader( P_MEMBER pWho )
	{
		if( isMember( pWho->getSerial() ) )
		{
			leader = pWho;
			return true;
		}
		else
			return false;
	}

	/*!
	\brief	Get the number of members in an organisation
	\author Sparhawk
	\return number of members
	\since	0.82a
	*/
	SI32 Organisation::numberOfMembers()
	{
		return members.size();
	}

	LOGICAL	Organisation::isLeader( Member &who )
	{
		return isLeader( who.getSerial() );
	}

	LOGICAL	Organisation::isLeader( SERIAL who )
	{
		if( leader )
			if( leader->getSerial() == who )
				return true;
		return false;
	}
}

namespace Organisation
{
	namespace Guild
	{
		class GuildMember;

		class GuildMember : public Member
		{
			public:
						GuildMember( P_CHAR pWho );
				virtual		~GuildMember();
			private:
				GuildMember*	fealty;
				UI32		loyaltyBase;
		};

		GuildMember::GuildMember( P_CHAR pWho ) : Member( pWho )
		{
			fealty		= this;
			loyaltyBase	= 1;
		}

		GuildMember::~GuildMember()
		{
		}

		class GuildRecruit : public Member
		{
			public:
						GuildRecruit( P_CHAR pWho );
				virtual		~GuildRecruit();
		};

		GuildRecruit::GuildRecruit( P_CHAR pWho ) : Member( pWho )
		{
		}

		GuildRecruit::~GuildRecruit()
		{
		}

		class Guild : public Organisation
		{
			public:
						Guild( P_ITEM pStone, P_CHAR pCreator );
						~Guild();
			private:
				char		abbreviation[4];
				memberMap	recruits;
			public:
				LOGICAL		isRecruit( SERIAL who );
				SI32		numberOfRecruits();
				LOGICAL		addRecruit( P_CHAR pWho );

				virtual LOGICAL	addMember( P_MEMBER pWho );
		};

		Guild::Guild( P_ITEM pStone, P_CHAR pCreator ) : Organisation( pStone )
		{
			GuildMember *member = new GuildMember( pCreator );

			Organisation::addMember( member );
			setLeader( member );
		}

		Guild::~Guild()
		{
		}

		LOGICAL Guild::isRecruit( SERIAL who )
		{
			return recruits.find( who ) != recruits.end();
		}

		SI32 Guild::numberOfRecruits()
		{
			return recruits.size();
		}

		LOGICAL Guild::addRecruit( P_CHAR pWho )
		{
			LOGICAL added = false;

			if( ISVALIDPC( pWho ) )
			{
				SERIAL who = pWho->getSerial32();
				if( !isMember( who ) && !isRecruit( who ) )
				{
					recruits[ who ] = new GuildRecruit( pWho );
					added = true;
				}
			}
			return added;
		}

		LOGICAL Guild::addMember( P_MEMBER pWho )
		{
			return false;
		}

		typedef Guild* 			P_GUILD;
		typedef map< SERIAL, P_GUILD >	guildMap;
		typedef guildMap::iterator	guildMapIterator;

		static	guildMap		guilds;

		LOGICAL	Create( SERIAL creatorSerial, SERIAL deedSerial )
		{
			LOGICAL created = false;
			P_CHAR pCreator = pointers::findCharBySerial( creatorSerial );

			if( pCreator )
			{
				if( pCreator->IsOnline() )
				{
					guildMapIterator it( guilds.begin() ), end( guilds.end() );
					LOGICAL isGuildMember = false;
					while( it != end && !isGuildMember )
					{
						isGuildMember = it->second->isMember( creatorSerial ) || it->second->isRecruit( creatorSerial );
						++it;
					}
					if( !isGuildMember )
					{
						P_ITEM pDeed = pointers::findItemBySerial( deedSerial );
						if( pDeed )
						{
							P_ITEM pStone = item::CreateFromScript( pDeed->morex );
							if ( pStone )
							{
								if( guilds.find( pStone->getSerial32() ) == guilds.end() )
								{
									guilds[ pStone->getSerial32() ] = new Guild( pStone, pCreator );

									pStone->setPosition( pCreator->getPosition() );
									pStone->priv  = 0;
									pStone->magic = 3;
									pStone->Refresh();
									pDeed->ReduceAmount(1);
									created = true;
								}
								else
								{
									LogError("Organisation::Guild::Create - Duplicate guild serial %d in guild creation by player [%d] %s\n", pStone->getSerial32(), creatorSerial, pCreator->getCurrentNameC());
									pCreator->sysmsg(TRANSLATE("Error: Cannot create guild from guildstone. Notify a GM"));
								}
							}
							else
							{
								LogError("Organisation::Guild::Create - Guildstone cannot be created from deed %d by player [%d]\n", deedSerial, creatorSerial, pCreator->getCurrentNameC() );
								pCreator->sysmsg(TRANSLATE("Error: Cannot create guildstone from deed. Notify a GM"));
								pCreator->objectdelay=0;
							}
						}
						else
						{
							LogError("Organisation::Guild::Create - invalid deed %d\n", deedSerial );
							pCreator->sysmsg(TRANSLATE("Error: Invalid guildstone deed. Notify a GM"));
							pCreator->objectdelay=0;
						}
					}
					else
					{
						pCreator->sysmsg(TRANSLATE("You are still associated with another guild."));
						pCreator->sysmsg(TRANSLATE("Resign from that guild before creating a new guild"));
						pCreator->objectdelay=0;
					}
				}
				else
				{
					if( pCreator->npc == 1)
					{
						LogError("Organisation::Guild::Create - npc %d tried to create a guild\n", creatorSerial );
					}
					else
					{
						LogError("Organisation::Guild::Create - offline pc %d tried to create a guild\n", creatorSerial );
					}
				}
			}
			else
				LogError("Organisation::Guild::Create - Called using invalid char serial %d\n", creatorSerial );

			return created;
		}

		LOGICAL	Disband( SERIAL guildSerial, SERIAL adminSerial )
		{
			LOGICAL disbanded = false;

			P_CHAR pAdmin = pointers::findCharBySerial( adminSerial );
			if( pAdmin )
			{
				guildMapIterator it( guilds.find( guildSerial ) );
				if( it != guilds.end() )
				{
					if( it->second->isLeader( adminSerial ) )
					{
						if( it->second->numberOfMembers() == 1 )
						{
							guilds.erase( it );
							pAdmin->sysmsg(TRANSLATE("Guild has been disbanded."));
						}
						else
						{
							pAdmin->sysmsg(TRANSLATE("Guild with members cannot be disbanded."));
						}
					}
				}
			}
			else
			{
				LogError("Organisation::Guild::Disband - Called using invalid char serial %d\n", adminSerial );
			}
			return disbanded;
		}

		LOGICAL EnlistRecruit( SERIAL guildSerial, SERIAL recruiterSerial, SERIAL recruiteeSerial )
		{
			return true;
		}

		LOGICAL ResignAsRecruit( SERIAL guildSerial, SERIAL recruiteeSerial )
		{
			return true;
		}

		LOGICAL FireRecruit( SERIAL guildSerial, SERIAL adminSerial, SERIAL recruiteeSerial, std::string message )
		{
			return true;
		}

		LOGICAL EnlistMember( SERIAL adminSerial, SERIAL recruitSerial, std::string message )
		{
			return true;
		}

		LOGICAL ResignAsMember( SERIAL memberSerial, std::string message )
		{
			return true;
		}

		LOGICAL FireMember( SERIAL guildSerial, SERIAL adminSerial, SERIAL memberSerial, std::string message )
		{
			return true;
		}
	}
}
