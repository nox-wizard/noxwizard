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
	serial	= ((pGuildStone) ? pGuildStone->getSerial32() : INVALID );
	name	= "unnamed guild";
	abbreviation = "";
	charter		= DEFAULTCHARTER;
	type		= 0;
	webpage		= DEFAULTWEBPAGE;
	if( pChar )
	{
		addMember( pChar );
		setGuildMaster( pChar );
		if ( pChar->GetBodyType() == BODY_FEMALE )
			members[pChar->getSerial32()].setTitle("Guildmistress");
		else
			members[pChar->getSerial32()].setTitle("Guildmaster");
	}
}

/*!
\brief Copy constructor of cGuild
*/
cGuild::cGuild( const cGuild &copy )
{
	serial		= copy.serial;
	name		= copy.name;
	abbreviation	= copy.abbreviation;
	charter		= copy.charter;
	type		= copy.type;
	webpage		= copy.webpage;
	guildmaster	= copy.guildmaster;
	members		= copy.members;
	recruits	= copy.recruits;
	war		= copy.war;
	allied		= copy.allied;
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
				if (!strcmp(script1, "CHARTER"))
					setCharter( script2 );
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
								members[ memberSerial ].setToggle( str2num( script2 ) );
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
					recruits.push_back( str2num( script2 ) );
					P_CHAR pChar = pointers::findCharBySerial( recruitSerial );
					if( ISVALIDPC( pChar ) )
					{
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
					}
					else
					{
						//
						// Skip recruit properties
						//
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
					setType(str2num(script2));
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
	this->name = newName.substr( 0, MAX_NAME_LENGTH );
}

/*!
\brief Get the guild name
\author Endymion
\return the guild name
*/
std::string cGuild::getName()
{
	return this->name;
}

/*!
\brief Set the new guild abbreviation
\author Endymion
\param newAbbr the new guild abbreviation
*/
void cGuild::setAbbreviation( const std::string &newAbbr )
{
	this->abbreviation = newAbbr.substr( 0, MAX_ABBR_LENGTH );
}

/*!
\brief Get the guild abbreviation
\author Endymion
\return the guild abbreviation
*/
std::string cGuild::getAbbreviation()
{
	return this->abbreviation;
}

/*!
\brief Set the new guild charter
\author Endymion
\param newCharter the new guild charter
*/
void cGuild::setCharter( const std::string &newCharter )
{
	this->charter = newCharter;
}

/*!
\brief Get the guild charter
\author Endymion
\return the guild charter
*/
std::string cGuild::getCharter()
{
	return this->charter;
}

/*!
\brief Set the new guild type
\author Endymion
\param newType the new guild type
\return success
*/
LOGICAL cGuild::setType( UI08 newType )
{
	LOGICAL success = false;

	if( newType > 2 )
		this->type = 0;
	else
	{
		this->type = newType;
		success = true;
	}
	
	return success;
}

/*!
\brief Get the guild type
\author Endymion
\return the guild type
*/
UI08 cGuild::getType()
{
	return this->type;
}

/*!
\brief Set the new guild web page
\author Endymion
\param newWebPage the new web page
*/
void cGuild::setWebPage( const std::string &newWebPage )
{
	this->webpage = newWebPage;
}

/*!
\brief Get the guild web page
\author Endymion
\return the guild web page
*/
std::string cGuild::getWebPage()
{
	return this->webpage;
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
\return true/false
\param pc the player
*/
LOGICAL cGuild::addMember( P_CHAR pc )
{
	VALIDATEPCR(pc, false);
	LOGICAL success = false;
	if( !pc->npc )
	{
		if( members.find( pc->getSerial32() ) == members.end() )
		{
			if( !pc->isGuilded() )
			{
				pc->setGuild( this->serial );
				members[pc->getSerial32()].setTitle("Novice");
				success = true;
			}
			else
				LogError( "Rejected pc %d as member for guild %d. Pc is still a member of another guild", pc->getSerial32(), serial );
		}
		else
			LogError( "Rejected pc %d as member for guild %d. Pc is allready a member of this guild", pc->getSerial32(), serial );
	}
	else
		LogError("Rejected npc %d as member for guild %d. Feature not implemented", pc->getSerial32(), serial );
	return success;
}

/*!
\brief Remove a guild member
\author Endymion
\param pc the player
\note if are guildmaster can't be removed.. before change guildmaster :D
*/
LOGICAL cGuild::resignMember( P_CHAR pc )
{
	LOGICAL success = false;

	if( ISVALIDPC( pc ) )
	{
		if( pc->getGuild() == this->serial )
		{
			if( this->getGuildMaster()!=pc->getSerial32() )
			{
				pc->setGuild( INVALID );
				members.erase(pc->getSerial32());
				success = true;
			}
		}
	}
	return success;
}


/*!
\brief Set the guildmaster
\author Endymion
\param pc the player
*/
void cGuild::setGuildMaster( P_CHAR pc )
{
	VALIDATEPC(pc);
	if( pc->getGuild() == this->serial ) { 
		this->guildmaster = pc->getSerial32();
	}
}

/*!
\brief Get the guildmaster
\author Endymion
\return the serial of guildmaster
*/
SERIAL cGuild::getGuildMaster()
{
	return this->guildmaster;
}

/*!
\brief Add a new recruit
\author Endymion
\return true if pc accepted as recruit, false if not
*/
LOGICAL cGuild::addNewRecruit( P_CHAR pc )
{
	if( !pc->isGuilded() )
	{
		//
		// Sparhawk: Let's make sure the pc is not allready a recruit
		//
		std::vector<SERIAL>::iterator iter( this->recruits.begin() ), end( this->recruits.end() );
		while ( iter != end )
		{
			if( *iter == pc->getSerial32() )
			{
				ConOut("addNewRecruit: %s is allready a recruit\n", pc->getCurrentNameC());
				return false;
			}
			else
				++iter;
		}
		this->recruits.push_back( pc->getSerial32() );
		return true;
	}
	ConOut("addNewRecuit: %s allready member of guild\n", pc->getCurrentNameC());
	return false;
}

/*!
\brief refuse a new recruit
\author Endymion
\return pc the player
*/
LOGICAL cGuild::refuseRecruit( P_CHAR pc )
{
	std::vector<SERIAL>::iterator iter( this->recruits.begin() ), end( this->recruits.end() );
	while ( iter != end ) {
		if( *iter == pc->getSerial32() ) {
			this->recruits.erase( iter );
			return true;
		}
		++iter;
	}
	return false;
}

/*!
\brief Declare War ( :] ndEndymion )
\author Endymion
\return guild the guild
*/
void cGuild::declareWar( SERIAL guild )
{
	if( !this->isInWar( guild ) )	{
		this->war.push_back( guild );		
	}
}

/*!
\brief Check if this guild is i war with iven guild
\author Endymion
\return guild the guild
*/
bool cGuild::isInWar( SERIAL guild )
{
	std::vector<SERIAL>::iterator iter( this->war.begin() ), end( this->war.end() );
	while ( iter!= end ) {
		if( *iter == guild ) {
			return true;
		}
		++iter;
	}
	return false;
}

/*!
\brief Declare Peace ( :[ ndEndymion )
\author Endymion
\return guild the guild
*/
void cGuild::declarePeace( SERIAL guild )
{
	std::vector<SERIAL>::iterator iter( this->war.begin() ), end( this->war.end() );
	while ( iter!= end ) {
		if( *iter == guild ) {
			this->war.erase( iter );
			return;
		}
	}
}

/*!
\brief Declare Allied
\author Endymion
\return guild the guild
*/
void cGuild::declareAllied( SERIAL guild )
{
	if(!this->isAllied( guild ) )  {
		this->allied.push_back( guild );
	}
}

/*!
\brief Check if Allied
\author Endymion
\return guild the guild
*/
bool cGuild::isAllied( SERIAL guild )
{
	std::vector<SERIAL>::iterator iter( this->allied.begin() ), end( this->allied.end() );
	while ( iter!= end ) {
		if( *iter == guild ) {
			return true;
		}
	}
	return false;
}

/*!
\brief Declare Allied
\author Endymion
\return guild the guild
*/
void cGuild::declareNeutral( SERIAL guild )
{
	std::vector<SERIAL>::iterator iter( this->allied.begin() ), end( this->allied.end() );
	while ( iter!= end ) {
		if( *iter == guild ) {
			this->allied.erase( iter );
			return;
		}
	}
}






////////////////////////////////////////////
////////////////GUILDED/////////////////////
////////////////////////////////////////////

/*!
\brief Contructor of cGuilded
*/
cGuilded::cGuilded()
{
	setToggle( 0 );
	setTitle("");
}

/*!
\brief Destructor of cGuild
*/
cGuilded::~cGuilded()
{
}

/*!
\brief Set the new guilded title
\author Endymion
\param newTitle the new guilded title
\todo define title length and code the check.
*/
void cGuilded::setTitle( const std::string &newTitle )
{
	this->title=newTitle;
}

/*!
\brief Get the guilded title
\author Endymion
\return the guilded title
*/
std::string cGuilded::getTitle()
{
	return this->title;
}

/*!
\brief Set the guilded title show mode
\author Endymion
\param newToggle the new show title mode
*/
void cGuilded::setToggle( UI08 newToggle )
{
	if( newToggle > 2 )
		this->toggle = 0;
	else
		this->toggle = newToggle;
}

/*!
\brief Get the guilded title show mode 
\author Endymion
\return the guilded title show mode
*/
UI08 cGuilded::getToggle()
{
	return this->toggle;
}



//////////////////////////////////////////////
///////////////GUILDSTONE STUFF///////////////
//////////////////////////////////////////////

P_ITEM PlaceGuildStoneDeed( P_CHAR pChar, P_ITEM pDeed )
{
	VALIDATEPCR(pChar, 0 );
	VALIDATEPIR(pDeed, 0 );
	NXWCLIENT ps = pChar->getClient();
	if( !ps )
		return 0;
	NXWSOCKET s = ps->toInt();

	if (pChar->isGuilded())
	{
		pChar->sysmsg(TRANSLATE("Resign from your guild before creating a new guild"));
		pChar->objectdelay=0;
		return 0;
	}

	P_ITEM pStone = item::CreateFromScript( pDeed->morex );
	if (!ISVALIDPI(pStone))
	{
		pChar->sysmsg(TRANSLATE("Cannot create guildstone"));
		pChar->objectdelay=0;
		return 0;
	}

	SERIAL serial = pStone->getSerial32();

	if( Guildz.find( serial ) )
	{
		pChar->sysmsg(TRANSLATE("Error: guild already exists"));
		LogError("Duplicate guild serial %d in guild creation by player [%d] %s\n", serial, pChar->getSerial32(), pChar->getCurrentNameC());
		return 0;
	}

	cGuild guild( pStone, pChar );
	if( Guildz.insert( guild ) )
	{
		pStone->setPosition( pChar->getPosition() );
		pStone->priv  = 0;
		pStone->magic = 3;
		pStone->Refresh();
		pDeed->ReduceAmount(1);
		return pStone;
	}
	else
	{
		pStone->ReduceAmount(1);
		return 0;
	}
}


cGuildz::cGuildz()
{
	setIndex( 0 );
}

cGuildz::~cGuildz()
{
}

UI32 cGuildz::getError()
{
	return (UI32) error;
}

LOGICAL cGuildz::insert( cGuild &guild )
{
	SERIAL		key	= guild.getSerial();
	std::string	name( guild.getName() );

	if( !find( key ) )
	{
		if( !find( name ) )
		{
			guilds[ key ] = guild;
			guildNames[ name ] = key;
			error = GUILD_ERROR_NO_ERROR;
		}
		else
			error = GUILD_ERROR_DUPLICATE_NAME;
	}
	else
		error = GUILD_ERROR_DUPLICATE_SERIAL;

	return ( error == GUILD_ERROR_NO_ERROR );
}

LOGICAL cGuildz::erase( SERIAL guild )
{
	if( find( guild ) )
	{
		guildNames.erase( fetch()->getName() );
		guilds.erase( guild );
		error = GUILD_ERROR_NO_ERROR;
	}
	else
		error = GUILD_ERROR_INVALID_SERIAL;

	return ( error == GUILD_ERROR_NO_ERROR );
}

LOGICAL cGuildz::erase( cGuild &guild )
{
	return erase( guild.getSerial() );
}

LOGICAL cGuildz::erase( cGuild *guild )
{
	return erase( guild->getSerial() );
}

LOGICAL cGuildz::update( cGuild &guild )
{
	SERIAL		key	= guild.getSerial();
	std::string	name( guild.getName() );

	if( find( key ) )
	{
		if( name != fetch()->getName() )
		{
			//
			// The Guild name has changed
			//
			if( !find( name ) )
			{
				//
				// The Guild name is unique
				//
				setIndex( 0 );
				//
				// Remove index entry for the old name
				//
				guildNames.erase( fetch()->getName() );
				//
				// Add index entry for the new name
				//
				guildNames[ name ] = key;
				error = GUILD_ERROR_NO_ERROR;
			}
			else
				//
				// New guild name allready in use
				//
				error = GUILD_ERROR_DUPLICATE_NAME;
		}
		else
			error = GUILD_ERROR_NO_ERROR;
	}
	else
		error = GUILD_ERROR_INVALID_SERIAL;

	return ( error == GUILD_ERROR_NO_ERROR );
}

LOGICAL cGuildz::update( cGuild *guild )
{
	return update( *guild );
}

LOGICAL cGuildz::find( SERIAL guild )
{
	setIndex( GUILD_INDEX_SERIAL );
	guildsIterator = guilds.find( guild );
	if( guildsIterator != guilds.end() )
		error = GUILD_ERROR_NO_ERROR;
	else
		error = GUILD_ERROR_SERIAL_NOT_FOUND;

	return ( error == GUILD_ERROR_NO_ERROR );
}

LOGICAL cGuildz::find( std::string &name )
{
	setIndex( GUILD_INDEX_NAME );
	guildNamesIterator = guildNames.find( name );

	if( guildNamesIterator != guildNames.end() )
		error = GUILD_ERROR_NO_ERROR;
	else
		error = GUILD_ERROR_NAME_NOT_FOUND;

	return ( error == GUILD_ERROR_NO_ERROR );
}

UI32 cGuildz::getIndex()
{
	error = GUILD_ERROR_NO_ERROR;
	return (UI32) currentIndex;
}

LOGICAL cGuildz::setIndex( UI32 index )
{
	switch( index )
	{
		case GUILD_INDEX_SERIAL :
		case GUILD_INDEX_NAME	:
			currentIndex = (eGuildzIndex) index;
			error = GUILD_ERROR_NO_ERROR;
			break;
		default			:
			error = GUILD_ERROR_INVALID_INDEX;
	}

	return ( error == GUILD_ERROR_NO_ERROR );
}

void cGuildz::first()
{
	error = GUILD_ERROR_NO_ERROR;

	switch( currentIndex )
	{
		case GUILD_INDEX_SERIAL :
			if( ( guildsIterator = guilds.begin() ) == guilds.end() )
				error = GUILD_ERROR_END_OF_SET;
			break;
		case GUILD_INDEX_NAME :
			if( ( guildNamesIterator = guildNames.begin() ) == guildNames.end() )
				error = GUILD_ERROR_END_OF_SET;
			break;
	}
}

void cGuildz::last()
{
	error = GUILD_ERROR_NO_ERROR;

	switch( currentIndex )
	{
		case GUILD_INDEX_SERIAL :
			guildsIterator = guilds.end();
			previous();
			if( guildsIterator == guilds.end() )
				error = GUILD_ERROR_END_OF_SET;
			break;
		case GUILD_INDEX_NAME :
			guildNamesIterator = guildNames.end();
			previous();
			if( guildNamesIterator == guildNames.end() )
				error = GUILD_ERROR_END_OF_SET;
			break;
	}
}

void cGuildz::next()
{
	error = GUILD_ERROR_NO_ERROR;

	switch( currentIndex )
	{
		case GUILD_INDEX_SERIAL :
			if( ++guildsIterator == guilds.end() )
				error = GUILD_ERROR_END_OF_SET;
			break;
		case GUILD_INDEX_NAME :
			if( ++guildsIterator == guilds.end() )
				error = GUILD_ERROR_END_OF_SET;
			break;
	}
}

void cGuildz::previous()
{
	error = GUILD_ERROR_NO_ERROR;

	switch( currentIndex )
	{
		case GUILD_INDEX_SERIAL :
			if( ++guildsIterator == guilds.end() )
				error = GUILD_ERROR_END_OF_SET;
			break;
		case GUILD_INDEX_NAME :
			if( ++guildsIterator == guilds.end() )
				error = GUILD_ERROR_END_OF_SET;
			break;
	}
}

cGuild* cGuildz::fetch()
{
	cGuild* guild = 0;
	error = GUILD_ERROR_NO_ERROR;

	switch( currentIndex )
	{
		case GUILD_INDEX_SERIAL :
			if( guildsIterator != guilds.end() )
				guild = new cGuild( guildsIterator->second );
			else
				error = GUILD_ERROR_NO_DATA;
			break;
		case GUILD_INDEX_NAME :
			if( guildNamesIterator != guildNames.end() )
				guild = new cGuild( guilds[ guildNamesIterator->second ] );
			else
				error = GUILD_ERROR_NO_DATA;
			break;
	}
	return guild;
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
	GUILDMEMBERMAPITER	mIter, mEnd;
	SERIALVECTORITER	rIter, rEnd;

	Guildz.first();
	cGuild *pGuild;
	while( (pGuild = Guildz.fetch()) != 0 )
	{
		fprintf(gWsc, "SECTION GUILD %i\n", 	pGuild->getSerial() );
		fprintf(gWsc, "{\n");
		fprintf(gWsc, "NAME %s\n",		pGuild->getName().c_str());
		fprintf(gWsc, "ABBR %s\n",		pGuild->getAbbreviation().c_str());
		fprintf(gWsc, "CHARTER %s\n",		pGuild->getCharter().c_str());
		fprintf(gWsc, "WEBPAGE %s\n",		pGuild->getWebPage().c_str());
		fprintf(gWsc, "TYPE %i\n",		pGuild->getType());
		fprintf(gWsc, "MASTER %i\n",		pGuild->getGuildMaster());

		mIter = pGuild->members.begin();
		mEnd  = pGuild->members.end();
		while( mIter != mEnd )
		{
			fprintf(gWsc, "MEMBER %d\n",	mIter->first );
			fprintf(gWsc, "{\n");
			fprintf(gWsc, "TITLE %s\n",	mIter->second.getTitle().c_str() );
			fprintf(gWsc, "TOGGLE %d\n",	mIter->second.getToggle() );
			fprintf(gWsc, "}\n");
			++mIter;
		}
		rIter = pGuild->recruits.begin();
		rEnd  = pGuild->recruits.end();
		while( rIter != rEnd )
		{
			fprintf(gWsc, "RECRUIT %d\n", *rIter );
			fprintf(gWsc, "{\n" );
			fprintf(gWsc, "DATERECRUITED\n" );
			fprintf(gWsc, "RECRUITER %d\n",	pGuild->getGuildMaster() );
			fprintf(gWsc, "}\n" );
			++rIter;
		}
		fprintf(gWsc, "}\n\n");
		Guildz.next();
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
		cGuild *guild = 0;
		do
		{
			readw3();
			if (!(strcmp(script1, "SECTION")))
			{
				if (!(strcmp(script2, "GUILD")))
				{
					guild = new cGuild( (SERIAL) str2num( script3 ) );
					if( guild )
						insert( *guild );
				}
			}
		}	while (strcmp(script1,"EOF") && !feof(wscfile) );
		fclose(wscfile);
		ConOut("[DONE]\n"); // Magius(CHE)
	}
}

LOGICAL	cGuildz::isGuildMember( SERIAL charSerial )
{
	LOGICAL success = false;

	miGuild iter( guilds.begin() ), end( guilds.end() );

	while( iter != end && !success )
	{
		if( iter->second.members.find( charSerial ) != iter->second.members.end() )
			success = true;
		++iter;
	}
	return success;
}

LOGICAL	cGuildz::isGuildMember( P_CHAR pChar )
{
	if( ISVALIDPC( pChar ) )
	{
		return isGuildMember( static_cast<SERIAL>( pChar->getSerial32() ) );
	}
	return false;
}

LOGICAL	cGuildz::isGuildMember( P_CHAR pChar, SERIAL guildSerial )
{
	if( ISVALIDPC( pChar ) )
	{
		return isGuildMember( pChar->getSerial32(), guildSerial );
	}
	return false;
}

LOGICAL	cGuildz::isGuildMember( SERIAL charSerial, SERIAL guildSerial )
{
	if( guilds.find( guildSerial ) != guilds.end() )
		if( guilds[guildSerial].members.find( charSerial ) != guilds[guildSerial].members.end() )
			return true;
	return false;
}


cGuildRecruit::cGuildRecruit( const P_CHAR pChar )
{
	if( ISVALIDPC( pChar ) )
		serial = pChar->getSerial32();
	else
		serial = INVALID;
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
	if( ISVALIDPC( pChar ) )
		setRecruiter( pChar->getSerial32() );
	else
		setRecruiter( INVALID );
}

//
// cGuildMember implementation
//
//	Class properties and methods
//
UI32 cGuildMember::membershipMaximum = 1;
UI32 cGuildMember::getMembershipMaximum()
{
	return membershipMaximum;
}

void cGuildMember::setMembershipMaximum( UI32 maximum )
{
	membershipMaximum = maximum;
}

LOGICAL cGuildMember::multiMembershipAllowed()
{
	return ( membershipMaximum > 1 );
}

//
//	Object methods
//
cGuildMember::cGuildMember()
{
}

cGuildMember::~cGuildMember()
{
}

