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
				if ( l=="WEBPAGE" )
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
	fprintf( file, "}\n\n");
}


#define MAX_NAME_LENGTH 41

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
	members.insert( make_pair( member->serial, member ) );
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

	}
	while( true );
}

void cGuildMember::save( FILE* file )
{
	fprintf( file, "MEMBER %d\n", serial );
	fprintf( file, "{\n");
	fprintf( file, "TITLE %s\n",	title.c_str() );
	fprintf( file, "TOGGLE %d\n",	toggle );
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
	fprintf( file, "RECRUIT %d\n", serial );
	fprintf( file, "{\n" );
	fprintf( file, "RECRUITER %d\n",	recruiter->serial );
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


bool cGuildz::archive()
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


	if( rename( saveFileName.c_str(), archiveFileName.c_str() ) != 0 )
	{
		LogWarning("Could not rename/move file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
		return false;
	}
	else
	{
		InfoOut("Renamed/moved file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
		return true;
	}
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
				guilds.insert( make_pair( guild->serial, guild ) );
			}
		}
	} 
	while ( !file.eof() );

	ConOut("[DONE]\n");
	return true;

}

P_GUILD cGuildz::addGuild( SERIAL stone ) {

	P_GUILD guild = new cGuild( stone );
	guilds.insert( make_pair( guild->serial, guild ) );
	return guild;

}

P_GUILD cGuildz::getGuild( SERIAL guild )
{
	std::map< SERIAL, P_GUILD >::iterator iter( guilds.find( guild ) );
	if( iter!=guilds.end() )
		return iter->second;
	else
		return NULL;
}


