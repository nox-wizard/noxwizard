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


/*!
\brief Contructor of cGuild
*/
cGuild::cGuild( SERIAL guildstone )
{
	serial	= guildstone;
	type = (GUILD_TYPE) 0;
	
}

/*!
\brief Destructor of cGuild
*/
cGuild::~cGuild()
{
	std::map< SERIAL, P_GUILD_MEMBER >::iterator member( members.begin() ), member_end( members.end() );
	for( ; member!=member_end; ++member )
		delete member->second;

	std::map< SERIAL, P_GUILD_RECRUIT >::iterator recruit( recruits.begin() ), recruit_end( recruits.end() );
	for( ; recruit!=recruit_end; ++recruit )
		delete recruit->second;
}

/*!
\brief Constructor for when loading from worldfile
*/
void cGuild::load( cStringFile& file )
{

	do
	{
		std::string l, r;
		file.read( l, r );

		if( l[0]=='{' )
			continue;

		if( l[0]=='}' )
			break;

		switch( l[0] )
		{
			case 'A':
			case 'a':
				if ( l=="ABBR" )
					this->abbreviation = r;
				else if ( l=="ALIGN" )
					this->type = (GUILD_TYPE) str2num(r);
				else if ( l=="ALLY" )
					this->addAlly((SERIAL) str2num(r));
				break;
			case 'C':
			case 'c':
				if ( l=="CHARTER" )
					this->charter = HexVector2UnicodeString( (char*)r.c_str() );
				break;
			case 'M':
			case 'm':
				if( l=="MEMBER" )
				{
					SERIAL memberSerial = str2num( r );
					P_CHAR pChar = pointers::findCharBySerial( memberSerial );
					if( ISVALIDPC( pChar ) )
					{
						P_GUILD_MEMBER member = addMember( pChar );
						member->load( file );
						if ( member->getRank() == RANK_GUILDMASTER )
							this->guildMaster=member->getSerial();
					}
					else
					{
						do file.read( l, r ); while( l!="}" );
						LogError("Guild worldfile: Member %d of guild %d is not a valid character", memberSerial, serial );
					}
				}
				break;
			case 'N':
			case 'n':
				if ( l=="NAME" )
					this->name = r;
				break;
			case 'P':
			case 'p':
				if ( l=="PEACE" )
					this->addPeace((SERIAL) str2num(r));
				break;
			case 'R':
			case 'r':
				if( l=="RECRUIT" )
				{
					std::string f, s;
					split( r, f, s );
					P_CHAR re = pointers::findCharBySerial( str2num( f ) );
					P_CHAR by = pointers::findCharBySerial( str2num( s ) ); 

					if( ISVALIDPC( re ) && ISVALIDPC( by ) )
					{
						P_GUILD_RECRUIT recruit = this->addNewRecruit( re, by );
						recruit->load( file );
					}
					else // Skip recruit properties
					{
						do file.read( l, r ); while( l!="}" );
						LogError("Guild worldfile: Recruit %d or Recruiter %d of guild %d is not a valid character", re, by, serial );
					}
				}
				break;
			case 'W':
			case 'w':
				if ( l=="WAR" )
					this->addWar((SERIAL) str2num(r));
				else if ( l=="WEBPAGE" )
					this->webpage = r;
				break;
		}

	}  while( !file.eof() );
}

void cGuild::save( FILE* file )
{
	fprintf( file, "SECTION GUILD %i\n", serial );
	fprintf( file, "{\n");
	fprintf( file, "NAME %s\n", getName().c_str());
	fprintf( file, "ABBR %s\n", getAbbreviation().c_str());
	fprintf( file, "ALIGN %d\n", getGuildType());
	fprintWstring(  file, "CHARTER", charter );
	fprintf( file, "WEBPAGE %s\n", webpage.c_str());

	std::map< SERIAL, P_GUILD_MEMBER >::iterator member( members.begin() ), member_end( members.end() );
	for( ; member!=member_end; ++member )
	{
		member->second->save( file );
	}

	std::map< SERIAL, P_GUILD_RECRUIT >::iterator recruit( recruits.begin() ), recruit_end( recruits.end() );
	for( ; recruit!=recruit_end; ++recruit )
	{
		recruit->second->save( file );
	}
	std::vector<SERIAL>::iterator wars (this->getGuildsInWar()->begin()), wars_end(this->getGuildsInWar()->end());
	for( ; wars!=wars_end; ++wars)
	{
		fprintf( file, "WAR %d\n", *wars);
	}
	std::vector<SERIAL>::iterator peace (this->getGuildsInPeace()->begin()), peace_end(this->getGuildsInPeace()->end());
	for( ; peace!=peace_end; ++peace)
	{
		fprintf( file, "PEACE %d\n", *peace);
	}
	std::vector<SERIAL>::iterator allies (this->getGuildsAllied()->begin()), allies_end(this->getGuildsAllied()->end());
	for( ; allies!=allies_end; ++allies)
	{
		fprintf( file, "ALLY %d\n", *allies);
	}
	fprintf( file, "}\n\n");
}


#define MAX_NAME_LENGTH 41

/*!
\brief Get the guild serial/stone id
\author Wintermute
\return the guild serial which is also the serial of the guildstone
*/
SERIAL cGuild::getSerial()
{
	return serial;
}

/*!
\brief Set the new guild name
\author Endymion
\param newName the new name
\note Maximun MAX_NAME_LENGTH ( 41 now ) characters
*/
void cGuild::setName( std::string &newName )
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

#define MAX_ABBR_LENGTH 4

/*!
\brief Set the new guild abbreviation
\author Endymion
\param newAbbr the new guild abbreviation
*/
void cGuild::setAbbreviation( std::string &newAbbr )
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
\brief Add a new guild member
\author Endymion
\param pc the player
*/
P_GUILD_MEMBER cGuild::addMember( P_CHAR pc )
{

	P_GUILD_MEMBER member = new cGuildMember( pc->getSerial32() );
	members.insert( make_pair( member->getSerial(), member ) );
	pc->setGuild( this, member );
	return member;

}

/*!
\brief Remove a guild member
\author Endymion
\param pc the player
*/
void cGuild::resignMember( P_CHAR pc )
{
	pc->setGuild( NULL, NULL );
	members.erase( pc->getSerial32() );
}

/*!
\brief Get the given guild member
\author Endymion
\param member the member
*/
P_GUILD_MEMBER cGuild::getMember( SERIAL member )
{

	std::map< SERIAL, P_GUILD_MEMBER >::iterator iter( members.find( member ) );
	return ( iter!=members.end() )? iter->second: NULL;

}


/*!
\brief Get the index of the guild member in the guild 
\author Wintermute
\param member the member
*/
int cGuild::getMemberPosition(SERIAL member)
{
	std::map< SERIAL, P_GUILD_MEMBER >::iterator iter( members.find( member ) );
	return ( iter!=members.end() )? distance(members.begin(), iter) : INVALID;
}

/*!
\brief Get  the guild member at the given index in the guild 
\author Wintermute
\param index, position of member
*/
SERIAL cGuild::getMemberByIndex(int index)
{
	std::map< SERIAL, P_GUILD_MEMBER >::iterator iter;
	int count = 0;
	for ( iter = members.begin();iter != members.end();iter++)
	{
		if ( count == index)
		{
			P_GUILD_MEMBER guildmember = iter->second;
			
			return guildmember->getSerial();
		}
	}
	return INVALID;
}

/*!
\brief Get the index of the guild member in the guild 
\author Wintermute
\param member the member
*/
int cGuild::getRecruitIndex(SERIAL member)
{
	std::map< SERIAL, P_GUILD_RECRUIT >::iterator iter( recruits.find( member ) );
	return ( iter!=recruits.end() )? distance(recruits.begin(), iter) + 1 : INVALID;
}

/*!
\brief Get  the guild member at the given index in the guild 
\author Wintermute
\param index, position of member
*/
SERIAL cGuild::getRecruitByIndex(int index)
{
	std::map< SERIAL, P_GUILD_RECRUIT >::iterator iter;
	int count = 0;
	for ( iter = recruits.begin();iter != recruits.end();iter++)
	{
		if ( count == index - 1)
		{
			P_GUILD_RECRUIT guildrecruit = iter->second;
			
			return guildrecruit->serial;
		}
	}
	return INVALID;
}

/*!
\brief Calculate the guild master after a member has changed his fealty
\author Wintermute
*/
void cGuild::calculateFealty()
{
	std::map< SERIAL, P_GUILD_MEMBER >::iterator iter;
	std::map< SERIAL, int> votes;
	std::map< SERIAL, int>::iterator voteIndex;
	int voteCount = 0;
	for ( iter = members.begin();iter != members.end();iter++)
	{
		P_GUILD_MEMBER guildmember = iter->second;
		P_GUILD_MEMBER votedGuildMember=getMember(guildmember->getFealty());
		voteIndex=votes.find(votedGuildMember->getSerial());
		if ( voteIndex != votes.end() )
		{
			voteCount=voteIndex->second+1;
		}
		else
		{
			voteCount=1;
		}
		votes.insert(make_pair(votedGuildMember->getSerial(), voteCount));
	}
	int highest;
	SERIAL master;
	for ( voteIndex = votes.begin();voteIndex != votes.end();voteIndex++)
	{
		if ( highest < voteIndex->second )
		{
			highest=voteIndex->second;
			master=voteIndex->first;
		}
	}
	if ( master != getGuildMaster() )
	{
		P_GUILD_MEMBER oldMaster=getMember(getGuildMaster());
		oldMaster->setRank(RANK_GUILDMEMBER);
		setGuildMaster(master);
	}
}


/*!
\brief Add a new recruit
\author Endymion
*/
P_GUILD_RECRUIT cGuild::addNewRecruit( P_CHAR recruit, P_CHAR recruiter )
{
	P_GUILD_RECRUIT re = new cGuildRecruit( recruit->getSerial32() );
	re->recruiter = this->getMember( recruiter->getSerial32() );
	recruits.insert( make_pair( re->serial, re ) );
	return re;
}

/*!
\brief refuse a new recruit
\author Endymion
\return pc the player
*/
void cGuild::refuseRecruit( P_CHAR pc )
{
	std::map< SERIAL, P_GUILD_RECRUIT>::iterator iter( recruits.find( pc->getSerial32() ) ), end( recruits.end() );
	if( iter != end ) {
		recruits.erase( iter );
	}
}

/*!
\brief displays players title string, over the name of clicked character, name color gets calculated from the guild relationship of both players
\author Unknown, adapted by Wintermute
\parameters (viewing character socket, clicked character) 
*/

void cGuild::showTitle(P_CHAR pc, P_CHAR pc2)
{
	char title[150];
	char abbreviation[5];
	char guildtype[10];

	if (pc->getGuild() == NULL )
		return;

	if (pc->HasGuildTitleToggle() )
	{
		strcpy(abbreviation,pc->getGuild()->getAbbreviation().c_str());
		if (!(strcmp(abbreviation,"")))
			strcpy(abbreviation,"none");

		if (pc->getGuild()->getGuildType() > 0 )
		{
			if (pc->getGuild()->getGuildType()== GUILD_TYPE_ORDER) strcpy(guildtype,"Order");
			else if (pc->getGuild()->getGuildType()==GUILD_TYPE_CHAOS) strcpy(guildtype,"Chaos");

			if (strcmp(pc->GetGuildTitle(),"")) sprintf(title,"[%s, %s] [%s]",pc->GetGuildTitle(),abbreviation,guildtype);
			else sprintf(title,"[%s] [%s]",abbreviation, guildtype);
		}
		else
		{
			if (strcmp(pc->GetGuildTitle(),"")) sprintf(title,"[%s, %s]",pc->GetGuildTitle(),abbreviation);
			else sprintf(title,"[%s]",abbreviation);
		}

		UI08 sysname[30]={ 0x00, };
		strcpy((char *)sysname, "System");

		SendSpeechMessagePkt(pc2->getClient()->getRealSocket(), pc->getSerial32(), 0x0101, 0, pc->emotecolor, 0x0003, sysname, title);
	}
}

bool cGuild::hasWarWith(SERIAL guild)
{
	std::vector< SERIAL>::iterator iter;
	for ( iter = guildWar.begin();iter != guildWar.end(); iter++ )
	{
	    if (*iter == guild )
			return true;
	}
	return false;
}

bool cGuild::hasPeaceWith(SERIAL guild)
{
	std::vector< SERIAL>::iterator iter;
	for ( iter = guildPeace.begin();iter != guildPeace.end(); iter++ )
	{
	    if (*iter == guild )
			return true;
	}
	return false;

}

bool cGuild::hasAllianceWith(SERIAL guild)
{
	std::vector< SERIAL>::iterator iter;
	for ( iter = guildAllies.begin();iter != guildAllies.end(); iter++ )
	{
	    if (*iter == guild )
			return true;
	}
	return false;

}

/*!
\brief Return array of guild wars
\author Wintermute
\return pointer to std::vector of serials of the guilds this guild is warring with
*/


std::vector<SERIAL> *cGuild::getGuildsInWar()
{
	return &guildWar;
}

/*!
\brief Return array of guild peace offers
\author Wintermute
\return pointer to std::vector of serials of the guilds this guild has made a peace offer to
*/


std::vector<SERIAL> *cGuild::getGuildsInPeace()
{
	return &guildPeace;

}

/*!
\brief Return array of guild allies
\author Wintermute
\return pointer to std::vector of serials of the guilds this guild is allied with
*/


std::vector<SERIAL> *cGuild::getGuildsAllied()
{
	return &guildAllies;

}

/*!
\brief Add a guild to declare war on
\author Wintermute
\param guild, the guild to declare war on
*/


void cGuild::addWar(SERIAL guild)
{
	guildWar.push_back(guild);
}

/*!
\brief Add a guild to make a peace offer to
\author Wintermute
\param guild, the guild to make an peace offer to
*/


void cGuild::addPeace(SERIAL guild)
{
	P_GUILD otherGuild=Guildz.getGuild(guild);
	if ( otherGuild != NULL && otherGuild->hasPeaceWith(this->getSerial()))
	{
		makePeace(guild);
		otherGuild->makePeace(this->getSerial());
	}
	else
		guildPeace.push_back(guild);
}

/*!
\brief Add a guild as an ally to your guild
\author Wintermute
\param guild, the guild to make an alliance with
*/

void cGuild::addAlly(SERIAL guild)
{
	guildAllies.push_back(guild);
}

/*!
\brief Remove all war entries from the current guild
\author Wintermute
\param guild, the guild to make peace with
*/
void cGuild::makePeace(SERIAL guild)
{
	vector<int>::iterator itRemove = guildPeace.begin();
	for ( ; itRemove < guildPeace.end(); itRemove++)
		if ( *itRemove == guild ) guildPeace.erase(itRemove);
	itRemove = guildWar.begin();
	for ( ; itRemove < guildWar.end(); itRemove++) guildWar.erase(itRemove);
}
/*!
\brief Get the given guild recruit
\author Endymion
\param recruit the recruit
*/
P_GUILD_RECRUIT cGuild::getRecruit( SERIAL recruit )
{

	std::map< SERIAL, P_GUILD_RECRUIT >::iterator iter( recruits.find( recruit ) );
	return ( iter!=recruits.end() )? iter->second: NULL;

}





/*!
\brief Contructor of cGuilded
*/
cGuildMember::cGuildMember( SERIAL serial )
{
	toggle = GUILD_TOGGLE_ALL;
	title = "Novice";
	rank=0;
	this->serial = serial;
}

/*!
\brief Destructor of cGuild
*/
cGuildMember::~cGuildMember()
{
}

void cGuildMember::load( cStringFile& file )
{
	do {
		std::string l, r;

		file.read( l, r );
		if( l.size()==0 )
			continue;

		if( l[0]=='{' )
			continue;
		if( l[0]=='}' )
			return;

		if( l=="TITLE" )
			title=r;
		else if( l=="TOGGLE" )
			toggle=static_cast<GUILD_TITLE_TOGGLE>( str2num(r) );
		else if( l=="RANK" )
			rank=str2num(r);	
		else if ( l=="FEALTY" )
			fealty=str2num(r);	
	}
	while( true );
}

void cGuildMember::save( FILE* file )
{
	fprintf( file, "MEMBER %d\n", serial );
	fprintf( file, "{\n");
	fprintf( file, "TITLE %s\n",	title.c_str() );
	fprintf( file, "TOGGLE %d\n",	toggle );
	fprintf( file, "RANK %d\n",		rank );
	fprintf( file, "FEALTY %d\n",		fealty );
	fprintf( file, "}\n");
}






cGuildRecruit::cGuildRecruit( SERIAL recruit )
{
	this->serial = recruit;
	this->recruiter = NULL;
}

cGuildRecruit::~cGuildRecruit()
{
}

void cGuildRecruit::load( cStringFile& file )
{
	do {

		std::string l, r;
		file.read( l, r );

		if( l.size()==0 )
			continue;

		if( l[0]=='{' )
			continue;
		if( l[0]=='}' )
			return;
		
		//if( l=="RECRUITER" )
		//	recruiter=str2num(r);

	}
	while( true );

}

void cGuildRecruit::save( FILE* file )
{
	fprintf( file, "RECRUIT %d %d\n", serial,	recruiter->getSerial() );
	fprintf( file, "{\n" );
//	fprintf( file, "RECRUITER %d\n",	recruiter->serial );
	fprintf( file, "}\n" );
}






cGuildPolitics::cGuildPolitics()
{
	forever=false;
}

cGuildPolitics::~cGuildPolitics()
{
}










cGuildz::cGuildz()
{
}

cGuildz::~cGuildz()
{
	std::map< SERIAL, P_GUILD >::iterator iter( guilds.begin() ), end( guilds.end() );
	for( ; iter!=end; iter++ )
	{
		delete iter->second;
	}
}

std::string cGuildz::getFullAdress()
{
	return std::string( SrvParms->savePath + SrvParms->guildWorldfile + SrvParms->worldfileExtension );
}


void cGuildz::archive()
{
	std::string saveFileName( getFullAdress() );
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
	char tempBuf[60000]; // copy files in 60k chunks
	ifstream oldSave;
	ofstream archiveSave;
	oldSave.open(saveFileName.c_str(), ios::binary );
	if ( ! oldSave.is_open() )
	{
		InfoOut("No books available for saving\n");
		return;
	}
	archiveSave.open(archiveFileName.c_str(), ios::binary);
	if ( ! archiveSave.is_open() )
	{
		LogWarning("Could not copy file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
		return ;
	}
	while ( ! oldSave.eof() )
	{
		int byteCount;
		oldSave.read(&tempBuf[0], sizeof(tempBuf)); 
		byteCount = oldSave.gcount();
		archiveSave.write(&tempBuf[0], byteCount);
	}
	
	InfoOut("Copied file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );

}

void cGuildz::safeoldsave()
{
	std::string oldFileName( getFullAdress() );
	std::string newFileName( getFullAdress() + "$" );
	remove( newFileName.c_str() );
	rename( oldFileName.c_str(), newFileName.c_str() );
}

bool cGuildz::save( )
{
	safeoldsave();

	std::string filename( getFullAdress() );
	FILE* file =fopen( filename.c_str(), "w");
	if( file==NULL )
	{
		ErrOut("Error, couldn't open %s for writing. Check file permissions.\n", filename.c_str() );
		tempfx::tempeffectson();
		return false;
	}

	fprintf( file, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf( file, "// || NoX-Wizard guild save (nxwguild.wsc)                                ||\n");
	fprintf( file, "// || Automatically generated on worldsaves                               ||\n");
	fprintf( file, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf( file, "// || Generated by NoX-Wizard version %s %s               ||\n", VERNUMB, OSFIX);
	fprintf( file, "// || Requires NoX-Wizard version 0.82 to be read correctly               ||\n");
	fprintf( file, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n\n");

	std::map< SERIAL, P_GUILD >::iterator iter( guilds.begin() ), end( guilds.end() );
	for( ; iter!=end; iter++ )
	{
		iter->second->save( file );
	}

	fclose( file );

	return true;
}

bool cGuildz::load()
{
	std::string fulladress( getFullAdress() );
	cStringFile file( fulladress, "r" );

	if( file.eof() )
	{
		WarnOut("Guild file [ %s ] not found.\n", fulladress.c_str());
		return false;
	}

	ConOut("Loading guilds ");
	do
	{
		std::string a, b, c;
		file.read( a, b, c );
		if ( a=="SECTION" )
		{
			if( b =="GUILD" )
			{
				P_GUILD guild = new cGuild( str2num( c ) );
				guild->load( file );
				guilds.insert( make_pair( guild->getSerial(), guild ) );
			}
		}
	} 
	while ( !file.eof() );

	ConOut("[DONE]\n");
	return true;

}

/*!
\brief add a new guild to the list of guilds
\author Endymion
\param stone, the serial of the guildstone
\return pointer to guild object
*/

P_GUILD cGuildz::addGuild( SERIAL stone ) {

	P_GUILD guild = new cGuild( stone );
	guilds.insert( make_pair( guild->getSerial(), guild ) );
	return guild;

}

/*!
\brief get a guild pointer for a given guild
\author Endymion
\param guild, the serial of the guildstone
\return pointer to guild object
*/


P_GUILD cGuildz::getGuild( SERIAL guild )
{
	std::map< SERIAL, P_GUILD >::iterator iter( guilds.find( guild ) );
	if( iter!=guilds.end() )
		return iter->second;
	else
		return NULL;
}

/*!
\brief remove a guild from the list of guilds
\author Wintermute
\param guild, the serial of the guildstone

*/


void cGuildz::removeGuild( SERIAL guild )
{
	guilds.erase(guild);
}


/*!
\brief this is for highlighting/guards and other stuff
\note Computes the relation player 1 and player 2 have. 
results are:
1= both in same guild (so fighting is okay, green hightlighting)
2= both in opposite guilds/guildtypes (so fighting is okay, orange highlighting)
0= no guildwarfare, or no guild relation (so no fighting, normal highlighting)
Order/Order or Chaos/Chaos guilds (in different guilds) may not war eachother!
\author Unknown, adapted by Wintermute
\parameters (viewing character socket, clicked character) 
*/

int cGuildz::compareGuilds(P_GUILD guild1,P_GUILD guild2)
{
	// one of both not in a guild -> no guildwarfare

	if (guild1 == NULL || guild2 == NULL ) return 0;

	if ((guild1 != NULL)&&(guild2 != NULL))
	{
		if (guild1==guild2)
			return 1;

		if (((guild1->getGuildType()== GUILD_TYPE_ORDER) && (guild2->getGuildType()== GUILD_TYPE_CHAOS))
			|| ((guild2->getGuildType()== GUILD_TYPE_ORDER)&&(guild1->getGuildType()== GUILD_TYPE_CHAOS)))
			return 2;

		if ( guild1->hasWarWith(guild2->getSerial()) )
			return 2;
	}
	return 0;
}

void cGuildz::checkConsistancy(void )
{
	int ok=1,error=0;

	P_ITEM stone;
	P_CHAR pc;

	ConOut("Checking guild data consistancy...");

	//////////// check the guilds
	for ( std::map< SERIAL, P_GUILD >::iterator iter = guilds.begin();iter != guilds.end();iter++)
	{
		P_GUILD guild = iter->second;
		// is the guildmaster still alive ?
			
		ok=1;
		
		pc = pointers::findCharBySerial(guild->getGuildMaster());
		if (!pc) // if not, erase the guild !
		{
			ok=0;
			LogWarning("guild: %s ereased because guildmaster vanished",guild->getName());
			Guildz.removeGuild(guild->getSerial());
		}
		// guildstone deleted ? yes -> erase guild !
		if (ok) // don't erease twice ;)
		{
			stone = pointers::findItemBySerial(guild->getSerial());
			if (!stone)
			{
				ok=0;
				LogWarning("guild: %s ereased because guildstone vanished",guild->getName().c_str());
				Guildz.removeGuild(guild->getSerial());
			}
		}
		if (ok)
		{
		 // check for guildmembers that don't exist anymore and remove from guild structure if so
			for ( std::map< SERIAL, P_GUILD_MEMBER>::iterator iter = guild->getMembers().begin(); iter!= guild->getMembers().end();iter ++ )
			{
				P_GUILD_MEMBER member = iter->second;
				
				pc = pointers::findCharBySerial(member->getSerial());
				if (!ISVALIDPC( pc ) )
				{
					ok=0;
					LogWarning("guild: %s had an member that didnt exist anymore, removed\n",guild->getName());
					guild->removeMember(member->getSerial());
				}
			}
		}
		
		if (!ok) error=1;
	}

	if (error) ConOut("[DONE] - errors found, check logs\n"); else ConOut("[ OK ]\n");
}
