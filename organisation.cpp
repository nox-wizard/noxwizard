#include "organisation.h"
#include "gamedate.h"

namespace Organisation
{
	namespace Guild
	{

		class Guild
		{
			public:
						Guild( P_ITEM pStone, P_CHAR pCreator );
						~Guild();
			private:
				SERIAL		guildStoneSerial;
				SERIAL		creatorSerial;
				cGameDate	dateCreated;
				std::string	name;
				std::string	abbreviation;
				SERIAL		guildMasterSerial;
		};

		Guild::Guild( P_ITEM pStone, P_CHAR pCreator )
		{
			guildStoneSerial = pStone->getSerial32();
			creatorSerial = pCreator->getSerial32();
			guildMasterSerial = creatorSerial;
		}

		Guild::~Guild()
		{
		}

		typedef Guild* P_GUILD;
		typedef map< SERIAL, P_GUILD > guildMap;
		typedef guildMap::iterator guildMapIterator;
		typedef map< SERIAL, SERIAL > guildMemberMap;
		typedef guildMemberMap::iterator guildMemberMapIterator;

		static	guildMap		guilds;
		static	guildMemberMap		guildMembers;

		LOGICAL	Create( SERIAL creatorSerial, SERIAL deedSerial )
		{
			LOGICAL success = false;
			P_CHAR pCreator = pointers::findCharBySerial( creatorSerial );

			if( pCreator )
			{
				if( pCreator->IsOnline() )
				{
					if( guildMembers.find( creatorSerial ) == guildMembers.end() )
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
									success = true;
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

			return success;
		}

		LOGICAL	Disband( SERIAL guildSerial, SERIAL adminSerial )
		{
			P_CHAR pAdmin = pointers::findCharBySerial( adminSerial );
			if( pAdmin == 0 )
			{
				LogError("Organisation::Guild::Disband - Called using invalid char serial %d\n", adminSerial );
				return false;
			}
		}

		LOGICAL EnlistRecruit( SERIAL guildSerial, SERIAL recruiterSerial, SERIAL recruiteeSerial )
		{
		}

		LOGICAL ResignAsRecruit( SERIAL guildSerial, SERIAL recruiteeSerial )
		{
		}

		LOGICAL FireRecruit( SERIAL guildSerial, SERIAL adminSerial, SERIAL recruiteeSerial, std::string message )
		{
		}

		LOGICAL EnlistMember( SERIAL adminSerial, SERIAL recruitSerial, std::string message )
		{
		}

		LOGICAL ResignAsMember( SERIAL memberSerial, std::string message )
		{
		}

		LOGICAL FireMember( SERIAL guildSerial, SERIAL adminSerial, SERIAL memberSerial, std::string message )
		{
		}
	}
}
