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
		};

		Guild::Guild( P_ITEM pStone, P_CHAR pCreator )
		{
			guildStoneSerial = pStone->getSerial32();
			creatorSerial = pCreator->getSerial32();
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
			P_CHAR pCreator = pointers::findCharBySerial( creatorSerial );
			if( pCreator == 0 )
				return false;
			NXWCLIENT ps = pCreator->getClient();
			if( !ps )
				return false;
			NXWSOCKET s = ps->toInt();

			if( guildMembers.find( creatorSerial ) != guildMembers.end() )
			{
				pCreator->sysmsg(TRANSLATE("You are still associated with another guild."));
				pCreator->sysmsg(TRANSLATE("Resign from that guild before creating a new guild"));
				pCreator->objectdelay=0;
				return false;
			}

			P_ITEM pDeed = pointers::findItemBySerial( deedSerial );
			if( pDeed == 0 )
				return false;

			P_ITEM pStone = item::CreateFromScript( pDeed->morex );
			if ( pStone == 0 )
			{
				LogError("Organisation::Guild::Create - Guildstone cannot be created from deed %d by player [%d]\n", deedSerial, creatorSerial, pCreator->getCurrentNameC() );
				pCreator->sysmsg(TRANSLATE("Error: Cannot create guildstone from deed. Notify a GM"));
				pCreator->objectdelay=0;
				return false;
			}

			if( guilds.find( pStone->getSerial32() ) != guilds.end() )
			{
				LogError("Organisation::Guild::Create - Duplicate guild serial %d in guild creation by player [%d] %s\n", pStone->getSerial32(), creatorSerial, pCreator->getCurrentNameC());
				pCreator->sysmsg(TRANSLATE("Error: Cannot create guild from guildstone. Notify a GM"));
				return false;
			}

			guilds[ pStone->getSerial32() ] = new Guild( pStone, pCreator );

			pStone->setPosition( pCreator->getPosition() );
			pStone->priv  = 0;
			pStone->magic = 3;
			pStone->Refresh();
			pDeed->ReduceAmount(1);
		}

		LOGICAL	Disband( SERIAL guildSerial, SERIAL adminSerial )
		{
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
