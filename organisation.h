#include "nxwcommn.h"
#include "gamedate.h"

namespace Organisation
{
	namespace Guild
	{
		LOGICAL	Create( SERIAL creatorSerial, SERIAL deedSerial );
		LOGICAL	Disband( SERIAL guildSerial, SERIAL adminSerial );

		LOGICAL EnlistRecruit( SERIAL guildSerial, SERIAL recruiterSerial, SERIAL recruiteeSerial );
		LOGICAL ResignAsRecruit( SERIAL guildSerial, SERIAL recruiteeSerial );
		LOGICAL FireRecruit( SERIAL guildSerial, SERIAL adminSerial, SERIAL recruiteeSerial, std::string message );

		LOGICAL EnlistMember( SERIAL adminSerial, SERIAL recruitSerial, std::string message );
		LOGICAL ResignAsMember( SERIAL memberSerial, std::string message );
		LOGICAL FireMember( SERIAL guildSerial, SERIAL adminSerial, SERIAL memberSerial, std::string message );
	}
}

