  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file
\brief Class Related stuff
*/
#ifndef __GUILD_H__
#define __GUILD_H__

#include "constants.h"
#include "typedefs.h"
#include "worldmain.h"

/*!
\brief A candidate for membership of a guild
\author Endymion
*/
class cGuildRecruit
{
	public:

		SERIAL	serial; //!< the recruit
		P_GUILD_MEMBER	recruiter; //!< the recruiter
		
		cGuildRecruit( SERIAL recruit );
		~cGuildRecruit();

		void load( cStringFile& file );
		void save( FILE* file );
};

typedef enum {
	GUILD_TOGGLE_ALL,
	GUILD_TOGGLE_SAMEGUILD,
	GUILD_TOGGLE_NONE
} GUILD_TITLE_TOGGLE;

/*
\note the menber of a guild
\author Endymion
*/
class cGuildMember
{
	public:
		
		SERIAL serial;	//!< the member
		std::string	title;	//!< title
		GUILD_TITLE_TOGGLE toggle; //!< title toggle
		UI32 rank; //!< the rank

		cGuildMember( SERIAL serial );
		~cGuildMember();

		void load( cStringFile& file );
		void save( FILE* file );

};


#define GUILD_POLITICS_WAR 0
#define	GUILD_POLITICS_ALLIED 1

/*!
\brief this color are used into draw char packet
\author Endymion
*/
typedef enum {
	GUILD_POLITICS_BLUE = 1,
	GUILD_POLITICS_GREEN,
	GUILD_POLITICS_ORANGE,
	GUILD_POLITICS_RED,
	GUILD_POLITICS_TRANSPARENT
} GUILD_POLITICS_COLOR;



/*!
\brief Guild politics info
\author Endymion
*/
class cGuildPolitics {

	public:

		SERIAL serial; //!< the guild
		GUILD_POLITICS_COLOR color; //!< guild member see member of this in this color
		UI08 type; //!< the type, war or allied
		bool forever;	//!< used for chaos or order guild

		cGuildPolitics();
		~cGuildPolitics();
};


typedef enum {
	GUILD_TYPE_NORMAL,
	GUILD_TYPE_ORDER,
	GUILD_TYPE_CHAOS,
	GUILD_TYPE_CITY
} GUILD_TYPE;


/*!
\brief a guild
\author Endymion
*/
class cGuild
{

	public:

		void load( cStringFile& file );
		void save( FILE* file );

	public:

		SERIAL serial;	//!< guild serial, is equal to the guildstone serial
		GUILD_TYPE type;	//!< guild type


	private:
		std::string	name;	//!< guild name ( es Keeper of the Crimson Soul )
		std::string	abbreviation;	//!< abbreviation ( es KCS )
	
	public:

		std::wstring	charter;	//!< charter
		std::string webpage;	//!< web page

		cGuild( SERIAL guildstone );
		~cGuild();
		void load();

		void setName( std::string &newName );
		std::string getName();
		void setAbbreviation( std::string &newAbbr );
		std::string getAbbreviation();

	public:

		std::map< SERIAL, P_GUILD_MEMBER > members;	//!< all members of this guild

		P_GUILD_MEMBER addMember( P_CHAR pc );
		void resignMember( P_CHAR pc );
		P_GUILD_MEMBER getMember( SERIAL member );

	public:

		std::map< SERIAL, P_GUILD_RECRUIT > recruits;	//!< all who want become member of this guild
		
		P_GUILD_RECRUIT addNewRecruit( P_CHAR recruit, P_CHAR recruiter );
		void refuseRecruit( P_CHAR pc );
		P_GUILD_RECRUIT getRecruit( SERIAL recruit );

	public:

		std::map< SERIAL, cGuildPolitics > political_to_guild;	//!< all guild politics related to other guild

};


/*!
\brief Guild Manager
\author Endymion
*/
class cGuildz
{
	private:

		void safeoldsave();
		std::string getFullAdress();

	public:
		
		bool archive();
		bool load();
		bool save();

	public:

		std::map< SERIAL, P_GUILD > guilds;

		cGuildz();
		~cGuildz();

		P_GUILD getGuild( SERIAL guild );
		P_GUILD addGuild( SERIAL stone );

};

extern cGuildz Guildz;


#endif

