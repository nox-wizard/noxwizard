#include "organisation.h"

namespace Organisation
{

Guild::GUILD_TBL		Guild::guilds	;
cScpScript*			Guild::wsc	;
std::string			Guild::wscName = "save/organisation/guild.wsc";
Guild::Member::MEMBER_TBL	Guild::Member::members;

LOGICAL Guild::load()
{
	LOGICAL success = false;

	wsc = new cScpScript( const_cast<char*>(wscName.c_str()) );

	if( wsc )
	{
		success = parse();
	}

	return success;
}

LOGICAL Guild::save()
{
	LOGICAL success = false;

	return success;
}

LOGICAL Guild::exists( GUILDID guild )
{
	return ( guilds.find( guild ) != guilds.end() );
}

LOGICAL Guild::parse()
{
	LOGICAL 	success		= false;
	std::string	sectionName	= "SECTION GUILD ";
	UI32 		guildCount	= wsc->countSectionInStr( sectionName );

	if ( guildCount > 0 )
	{
		cScpIterator*	iter		= 0;
		std::string*	sectionParms	= wsc->getFirstSectionParmsInStr( sectionName );
		std::string	section,
				lha,
				rha;
		LOGICAL		error		= false;
		P_GUILD		pGuild		= 0;

		do
		{
			section = sectionName + *sectionParms;
			iter = wsc->getNewIterator( section );
			if ( iter )
			{
				/*
				pGuild = new Guild();

				pGuild->id = str2num( *sectionParms );

				do
				{
					iter->parseLine( lha, rha );
					switch( lha[0] )
					{
						case '{':
						case '}':
							break;
						case '@':
							break;
						case 'A':
							if ( lha == "ABBR" )
							{
								pGuild->abbreviation = rha;
							}
							break;
						case 'D':
							if ( lha == "DATE" )
							{
							}
							break;
						case 'N':
							if ( lha == "NAME" )
							{
								pGuild->name = rha;
							}
							break;
						case 'T':
							if ( lha == "TYPE" )
							{
							}
							break;
					}
				}
				while ( lha[0] != '}' );
				//guilds[ pGuild->getid ] = pGuild;
				iter = NULL;
				sectionParms = wsc->getNextSectionParmsInStr( sectionName );
				*/
			}
			else
			{
				error = true;
				ErrOut( "Guild::load() no %s\n", section.c_str() );
			}
		}
		while ( --guildCount && !error );

		if (!error)
			success = true;
	}
	else
	{
		WarnOut("Guild::load() no guilds in worldfile\n");
	}
	return success;
}

Guild::GUILDID	Guild::getId()
{
	return id;
}


LOGICAL Guild::Member::load()
{
	LOGICAL success = false;

	return success;
}

LOGICAL Guild::Member::save()
{
	LOGICAL success = false;

	return success;
}

LOGICAL Guild::Member::exists( MEMBERID who )
{
	return ( members.find( who ) != members.end() );
}

LOGICAL	Guild::Member::add( P_MEMBER pNewMember )
{
	LOGICAL 	success = false			;
	MEMBERID	id 	= pNewMember->getId()	;

	//
	//	Rule 1:	a new member cannot already be a guildmember
	//
	if( !exists( id ) )
	{
		members[ id ] = pNewMember;
		success = true;
	}
	return success;
}

LOGICAL Guild::Member::parse()
{
	LOGICAL success = false;

	return success;
}

Guild::MEMBERID	Guild::Member::getId()
{
	return id;
}

}

