  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "sndpkg.h"
#include "debug.h"
#include "worldmain.h"
#include "guild.h"
#include "items.h"
#include "chars.h"
#include "basics.h"
#include "scripts.h"


cGuildz Guildz;

//
// Class methods
//

void cGuild::archive()
{
	std::string saveFileName( SrvParms->savePath + SrvParms->guildWorldfile + SrvParms->worldfileExtension );
	std::string timeNow( getNoXDate() );
	for( int i = timeNow.length() - 1; i >= 0; --i )
		switch( timeNow[i] )
		{
			case '/' :
			case ' ' :
			case ':' :
				timeNow[i]= '-';
		}
	std::string archiveFileName( SrvParms->archivePath + SrvParms->guildWorldfile + timeNow + SrvParms->worldfileExtension );


	if( rename( saveFileName.c_str(), archiveFileName.c_str() ) != 0 )
	{
		LogWarning("Could not rename/move file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
	}
	else
	{
		InfoOut("Renamed/moved file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
	}
}

void cGuild::safeoldsave()
{
	std::string oldFileName( SrvParms->savePath + SrvParms->guildWorldfile + SrvParms->worldfileExtension );
	std::string newFileName( SrvParms->savePath + SrvParms->guildWorldfile + SrvParms->worldfileExtension + "$" );
	remove( newFileName.c_str() );
	rename( oldFileName.c_str(), newFileName.c_str() );
}

/*!
\brief Contructor of cGuild
*/
cGuild::cGuild( P_ITEM pGuildStone, P_CHAR pChar )
{
	serial	= pGuildStone->getSerial32();
/*	name = "unnamed guild";
	abbreviation = "UNK";
	charter = DEFAULTCHARTER;
	type = GUILD_NORMAL;
	webpage		= DEFAULTWEBPAGE;
*/
	addMember( pChar );
	setGuildMaster( pChar );
/*	if ( pChar->GetBodyType() == BODY_FEMALE )
		members[pChar->getSerial32()].setTitle("Guildmistress");
	else
		members[pChar->getSerial32()].setTitle("Guildmaster");
*/
}

/*!
\brief Constructor for when loading from worldfile
*/

cGuild::cGuild( SERIAL serial )
{

	do
	{
		readw2();
		switch(script1[0])
		{
			case 'A':
			case 'a':
				if (!strcmp(script1, "ABBR") )
					setAbbreviation( script2 );
				break;
			case 'C':
			case 'c':
				if (!strcmp(script1, "CHARTER")) {
					setCharter( HexVector2UnicodeString( script2 ) );
				}
				break;
			case 'M':
			case 'm':
				if (!strcmp(script1, "MASTER"))
				{
					SERIAL masterSerial = str2num(script2);
					P_CHAR pChar = pointers::findCharBySerial( masterSerial );
					if( ISVALIDPC( pChar ) )
					{
							setGuildMaster( pChar );
							pChar->setGuild( serial );
					}
					else
					{
						LogError("Guild worldfile: Guild master %i of guild %i is not a valid character", masterSerial, serial );
					}
				}
				else if( !strcmp( script1, "MEMBER" ) )
				{
					SERIAL memberSerial = str2num( script2 );
					P_CHAR pChar = pointers::findCharBySerial( memberSerial );
					if( ISVALIDPC( pChar ) )
					{
						//
						// Process member properties
						//
						do
						{
							readw2();
							if( !strcmp( script1, "TITLE" ) )
								members[ memberSerial ].setTitle( script2 );
							else if( !strcmp( script1, "TOGGLE" ) )
								members[ memberSerial ].setToggle( static_cast<GUILD_TITLE_TOGGLE>(str2num( script2 )) );
						}
						while( strcmp( script1, "}" ) );
						pChar->setGuild( serial );
					}
					else
					{
						do
						{
							readw2();
						}
						while( strcmp( script1, "}" ) );
						LogError("Guild worldfile: Member %d of guild %d is not a valid character", memberSerial, serial );
					}
				}
				break;
			case 'N':
			case 'n':
				if (!strcmp(script1, "NAME"))
					setName(script2);
				break;
			case 'R':
			case 'r':
				if( !strcmp( script1, "RECRUIT" ) )
				{
					SERIAL recruitSerial = str2num( script2 );
					P_CHAR pChar = pointers::findCharBySerial( recruitSerial );
					if( ISVALIDPC( pChar ) )
					{
						cGuildRecruit* recruit = new cGuildRecruit();

						//
						// Process recruit properties
						//
						do
						{
							readw2();
							//
							// TODO implement recuit properties
							//
						}
						while( strcmp( script1, "}" ) );

						recruits.insert( make_pair( recruitSerial, *recruit ) );

					}
					else // Skip recruit properties
					{
						do
						{
							readw2();
						}
						while( strcmp( script1, "}" ) );
						LogError("Guild worldfile: Recruit %d of guild %d is not a valid character", recruitSerial, serial );
					}
				}
				break;
			case 'T':
			case 't':
				if (!strcmp(script1, "TYPE"))
					setType( static_cast<GUILD_TYPE>(str2num(script2)));
				break;
			case 'W':
			case 'w':
				if (!strcmp(script1, "WEBPAGE"))
					setWebPage(script2);
				break;
		}

	}  while( strcmp( script1, "}" ) );
}

/*!
\brief Destructor of cGuild
*/
cGuild::~cGuild()
{
}

void cGuild::create( SERIAL newSerial )
{
	this->serial=newSerial;
}

void cGuild::setSerial( SERIAL newSerial )
{
	this->serial=newSerial;
}

SERIAL cGuild::getSerial()
{
	return this->serial;
}

#define MAX_NAME_LENGTH 41
#define MAX_ABBR_LENGTH 4

/*!
\brief Set the new guild name
\author Endymion
\param newName the new name
\note Maximun MAX_NAME_LENGTH ( 41 now ) characters
*/
void cGuild::setName( const std::string &newName )
{
	name = newName.substr( 0, MAX_NAME_LENGTH );
}

/*!
\brief Get the guild name
\author Endymion
\return the guild name
*/
std::string cGuild::getName()
{
	return name;
}

/*!
\brief Set the new guild abbreviation
\author Endymion
\param newAbbr the new guild abbreviation
*/
void cGuild::setAbbreviation( const std::string &newAbbr )
{
	abbreviation = newAbbr.substr( 0, MAX_ABBR_LENGTH );
}

/*!
\brief Get the guild abbreviation
\author Endymion
\return the guild abbreviation
*/
std::string cGuild::getAbbreviation()
{
	return abbreviation;
}

/*!
\brief Set the new guild charter
\author Endymion
\param newCharter the new guild charter
*/
void cGuild::setCharter( const std::wstring &newCharter )
{
	charter = newCharter;
}

/*!
\brief Get the guild charter
\author Endymion
\return the guild charter
*/
std::wstring cGuild::getCharter() const
{
	return charter;
}

/*!
\brief Set the new guild type
\author Endymion
\param newType the new guild type
\return success
*/
void cGuild::setType( GUILD_TYPE newType )
{
	type = newType;
}

/*!
\brief Get the guild type
\author Endymion
\return the guild type
*/
GUILD_TYPE cGuild::getType()
{
	return type;
}

/*!
\brief Set the new guild web page
\author Endymion
\param newWebPage the new web page
*/
void cGuild::setWebPage( const std::string &newWebPage )
{
	webpage = newWebPage;
}

/*!
\brief Get the guild web page
\author Endymion
\return the guild web page
*/
std::string cGuild::getWebPage()
{
	return webpage;
}

/*!
\brief Check is character is a member of this guild
\author Sparhawk
\return true if the char is member of the guild
\param pChar the player
*/
LOGICAL cGuild::isMember( P_CHAR pChar )
{
	if( members.find( pChar->getSerial32() ) != members.end() )
		return true;
	else
		return false;
}

/*!
\brief Add a new guild member
\author Endymion
\param pc the player
*/
void cGuild::addMember( P_CHAR pc )
{

	pc->setGuild( serial );
	members.insert( make_pair( pc->getSerial32(), cGuildMember() ) );

}

/*!
\brief Remove a guild member
\author Endymion
\param pc the player
*/
void cGuild::resignMember( P_CHAR pc )
{
	pc->setGuild( INVALID );
	members.erase( pc->getSerial32() );
}


/*!
\brief Set the guildmaster
\author Endymion
\param pc the player
*/
void cGuild::setGuildMaster( P_CHAR pc )
{
	VALIDATEPC(pc);
	guildmaster = pc->getSerial32();
}

/*!
\brief Get the guildmaster
\author Endymion
\return the serial of guildmaster
*/
SERIAL cGuild::getGuildMaster()
{
	return guildmaster;
}

/*!
\brief Add a new recruit
\author Endymion
*/
void cGuild::addNewRecruit( P_CHAR recruit, P_CHAR recruiter )
{
	refuseRecruit( recruit );
	recruits.insert( make_pair( recruit->getSerial32(), cGuildRecruit( recruit, recruiter ) ) );
}

/*!
\brief refuse a new recruit
\author Endymion
\return pc the player
*/
void cGuild::refuseRecruit( P_CHAR pc )
{
	GUILDRECRUITMAP::iterator iter( recruits.find( pc->getSerial32() ) ), end( recruits.end() );
	if( iter != end ) {
		recruits.erase( iter );
	}
}

/*!
\brief Declare War ( :] ndEndymion )
\author Endymion
\return guild the guild
*/
void cGuild::declareWar( SERIAL guild )
{
	if( !isInWar( guild ) )	{
		war.push_back( guild );
	}
}

/*!
\brief Check if this guild is i war with iven guild
\author Endymion
\return guild the guild
*/
bool cGuild::isInWar( SERIAL guild )
{
	return ( find( war.begin(), war.end(), guild )!= war.end() );
}

/*!
\brief Declare Peace ( :[ ndEndymion )
\author Endymion
\return guild the guild
*/
void cGuild::declarePeace( SERIAL guild )
{
	std::vector<SERIAL>::iterator iter( find( war.begin(), war.end(), guild ) );
	if( iter != war.end() )
		war.erase( iter );
}

/*!
\brief Declare Allied
\author Endymion
\return guild the guild
*/
void cGuild::declareAllied( SERIAL guild )
{
	if(!isAllied( guild ) )  {
		allied.push_back( guild );
	}
}

/*!
\brief Check if Allied
\author Endymion
\return guild the guild
*/
bool cGuild::isAllied( SERIAL guild )
{
	return ( find( allied.begin(), allied.end(), guild ) != allied.end() );
}

/*!
\brief Declare Allied
\author Endymion
\return guild the guild
*/
void cGuild::declareNeutral( SERIAL guild )
{
	std::vector<SERIAL>::iterator iter( find( allied.begin(), allied.end(), guild ) );
	if( iter != allied.end() )
		war.erase( iter );
}






////////////////////////////////////////////
////////////////GUILDED/////////////////////
////////////////////////////////////////////

/*!
\brief Contructor of cGuilded
*/
cGuildMember::cGuildMember()
{
	setToggle( GUILD_TOGGLE_ALL );
	setTitle( "Novice" );
}

/*!
\brief Destructor of cGuild
*/
cGuildMember::~cGuildMember()
{
}

/*!
\brief Set the new guilded title
\author Endymion
\param newTitle the new guilded title
\todo define title length and code the check.
*/
void cGuildMember::setTitle( const std::string &newTitle )
{
	title=newTitle;
}

/*!
\brief Get the guilded title
\author Endymion
\return the guilded title
*/
std::string cGuildMember::getTitle()
{
	return title;
}

/*!
\brief Set the guilded title show mode
\author Endymion
\param newToggle the new show title mode
*/
void cGuildMember::setToggle( GUILD_TITLE_TOGGLE newToggle )
{
	toggle = newToggle;
}

/*!
\brief Get the guilded title show mode
\author Endymion
\return the guilded title show mode
*/
GUILD_TITLE_TOGGLE cGuildMember::getToggle()
{
	return toggle;
}



cGuildz::cGuildz()
{
}

cGuildz::~cGuildz()
{
}

void cGuildz::archive()
{
	std::string saveFileName( SrvParms->savePath + SrvParms->guildWorldfile + SrvParms->worldfileExtension );
	std::string timeNow( getNoXDate() );
	for( int i = timeNow.length() - 1; i >= 0; --i )
		switch( timeNow[i] )
		{
			case '/' :
			case ' ' :
			case ':' :
				timeNow[i]= '-';
		}
	std::string archiveFileName( SrvParms->archivePath + SrvParms->guildWorldfile + timeNow + SrvParms->worldfileExtension );


	if( rename( saveFileName.c_str(), archiveFileName.c_str() ) != 0 )
	{
		LogWarning("Could not rename/move file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
	}
	else
	{
		InfoOut("Renamed/moved file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
	}
}

void cGuildz::safeoldsave()
{
	std::string oldFileName( SrvParms->savePath + SrvParms->guildWorldfile + SrvParms->worldfileExtension );
	std::string newFileName( SrvParms->savePath + SrvParms->guildWorldfile + SrvParms->worldfileExtension + "$" );
	remove( newFileName.c_str() );
	rename( oldFileName.c_str(), newFileName.c_str() );
}

void cGuildz::save( FILE *gWsc )
{

	GUILDMAP::iterator iter( guilds.begin() ), end( guilds.end() );
	for( ; iter!=end; iter++ )
	{

		P_GUILD pGuild = (P_GUILD)&iter->second;

		fprintf(gWsc, "SECTION GUILD %i\n", 	pGuild->getSerial() );
		fprintf(gWsc, "{\n");
		fprintf(gWsc, "NAME %s\n",		pGuild->getName().c_str());
		fprintf(gWsc, "ABBR %s\n",		pGuild->getAbbreviation().c_str());
		fprintWstring( gWsc, "CHARTER", pGuild->getCharter() );
		fprintf(gWsc, "WEBPAGE %s\n",		pGuild->getWebPage().c_str());
		fprintf(gWsc, "TYPE %i\n",		pGuild->getType());
		fprintf(gWsc, "MASTER %i\n",		pGuild->getGuildMaster());

		GUILDMEMBERMAP::iterator mIter( pGuild->members.begin() ), mEnd( pGuild->members.end() );
		for( ; mIter!=mEnd; mIter++ )
		{
			fprintf(gWsc, "MEMBER %d\n", mIter->first );
			fprintf(gWsc, "{\n");
			fprintf(gWsc, "TITLE %s\n",	mIter->second.getTitle().c_str() );
			fprintf(gWsc, "TOGGLE %d\n", mIter->second.getToggle() );
			fprintf(gWsc, "}\n");
			++mIter;
		}

		GUILDRECRUITMAP::iterator rIter( pGuild->recruits.begin() ), rEnd( pGuild->recruits.end() );
		for( ; rIter!=rEnd; rIter++ )
		{
			fprintf(gWsc, "RECRUIT %d\n", rIter->first );
			fprintf(gWsc, "{\n" );
			fprintf(gWsc, "DATERECRUITED\n" );
			fprintf(gWsc, "RECRUITER %d\n",	pGuild->getGuildMaster() );
			fprintf(gWsc, "}\n" );
			++rIter;
		}
		fprintf(gWsc, "}\n\n");
	}
}

void cGuildz::load()
{
	std::string fileName( SrvParms->savePath + SrvParms->guildWorldfile + SrvParms->worldfileExtension );
	wscfile = fopen( fileName.c_str(), "r");
	if(!wscfile)
	{
		WarnOut("%s not found.\n", fileName.c_str());
	}
	else
	{
		ConOut("Loading guilds ");
		do
		{
			readw3();
			if (!(strcmp(script1, "SECTION")))
			{
				if (!(strcmp(script2, "GUILD")))
				{
					SERIAL sGuild = str2num( script3 );
					guilds.insert( make_pair( sGuild, cGuild( sGuild ) ) );
				}
			}
		} while (strcmp(script1,"EOF") && !feof(wscfile) );
		fclose(wscfile);
		ConOut("[DONE]\n"); // Magius(CHE)
	}
}

P_GUILD cGuildz::addGuild( P_ITEM stone, P_CHAR master ) {

	P_GUILD guild = new cGuild( stone, master );
	guilds.insert( make_pair( stone->getSerial32(), *guild ) );
	return guild;

}

P_GUILD cGuildz::getGuild( SERIAL guild )
{
	GUILDMAP::iterator iter( guilds.find( guild ) );
	if( iter!=guilds.end() )
		return &iter->second;
	else
		return NULL;
}


cGuildRecruit::cGuildRecruit( )
{
	setSerial( INVALID );
	setRecruiter( INVALID );
}

cGuildRecruit::cGuildRecruit( P_CHAR recruit, P_CHAR recruiter )
{
	setSerial( recruit->getSerial32() );
	setRecruiter( recruiter );
}

cGuildRecruit::~cGuildRecruit()
{
}

SERIAL cGuildRecruit::getSerial()
{
	return serial;
}

void cGuildRecruit::setSerial( const SERIAL serial )
{
	this->serial = serial;
}

SERIAL cGuildRecruit::getRecuiter()
{
	return recruiter;
}

void cGuildRecruit::setRecruiter( const SERIAL serial )
{
	recruiter = serial;
}

void cGuildRecruit::setRecruiter( const P_CHAR pChar )
{
	setRecruiter( pChar->getSerial32() );
}


