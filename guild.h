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

class cGuildMember ; //forward declaration

#include "constants.h"
#include "typedefs.h"
 
/*
\note a candidate for membership of a guild
\author Sparhawk
*/
class cGuildRecruit
{
	public:
		cGuildRecruit();
		cGuildRecruit( P_CHAR recruit, P_CHAR recruiter );
		~cGuildRecruit();
	private:
		SERIAL		serial;
	public:
		SERIAL		getSerial();
		void		setSerial( const SERIAL serial );
	private:
		SERIAL		recruiter;
	public:
		SERIAL		getRecuiter();
		void		setRecruiter( const P_CHAR pChar );
		void		setRecruiter( const SERIAL serial );
	public:
		cGameDate	recruitementDate;
		cPoll		poll;
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
		cGuildMember();
		~cGuildMember();
	private:
		std::string	title;	//!< title
	public:
		void setTitle( const std::string& newTitle );
		std::string	getTitle();
	private:
		GUILD_TITLE_TOGGLE toggle; //!< title toggle
	public:
		void setToggle( GUILD_TITLE_TOGGLE newToggle );
		GUILD_TITLE_TOGGLE getToggle();
	public:
		cGameDate membershipDate; //!< membership date
};

typedef std::map< SERIAL, cGuildMember > GUILDMEMBERMAP;
typedef std::map< SERIAL, cGuildRecruit > GUILDRECRUITMAP;

typedef enum {
	GUILD_NORMAL,
	GUILD_CHAOS,
	GUILD_ORDER
} GUILD_TYPE;

/*
\class cGuild
\note a guild
\author Endymion
*/
typedef class cGuild* P_GUILD;

class cGuild
{

	public:
		static void	archive();
		static void	safeoldsave();

	public:
		cGuild( const P_ITEM pGuildStone, const P_CHAR pc );
		cGuild( const SERIAL );
		~cGuild();

	private:
		SERIAL serial;	//!< Guild Serial = equal to the guildstone serial
	public:
		void setSerial( SERIAL newSerial );
		SERIAL getSerial();
		void create( SERIAL newSerial );

	private:
		std::string	name;	//!< Guild Name ( es Keeper of the Crimson Soul )
	public:
		void setName( const std::string& newName );
		std::string	getName();

	private:
		std::string	abbreviation;	//!< Guild Abbreviation ( es KCS )
	public:
		void setAbbreviation( const std::string& newAbbr );
		std::string	getAbbreviation();

	private:
		std::string	charter;	//!< Guild Charter
	public:
		void setCharter( const std::string& newCharter );
		std::string	getCharter();

	private:
		GUILD_TYPE type;	//!< guild type
	public:
		void setType( GUILD_TYPE newType );
		GUILD_TYPE getType();

	private:
		std::string webpage;	//!< Web Page
	public:
		void setWebPage( const std::string &newWebPage );
		std::string getWebPage();

	private:
		SERIAL guildmaster;	//!< the guild master
	public:
		GUILDMEMBERMAP members;	//!< All member of this guild
		void loadMembers();
		LOGICAL isMember( P_CHAR pChar );
		void addMember( P_CHAR pc );
		void resignMember( P_CHAR pc );
		void setGuildMaster( P_CHAR pc );
		SERIAL getGuildMaster();

	public:
		GUILDRECRUITMAP recruits;	//!< All who want become member of this guild
		void addNewRecruit( P_CHAR recruit, P_CHAR recruiter );
		void refuseRecruit( P_CHAR pc );

	public:
		std::vector<SERIAL> war;	//!< All war guild with this
		void declareWar( SERIAL guild );
		bool isInWar( SERIAL guild );
		void declarePeace( SERIAL guild );
	public:
		std::vector<SERIAL> allied;	//!< All allied guild with this
		void declareAllied( SERIAL guild );
		bool isAllied( SERIAL guild );
		void declareNeutral( SERIAL guild );

};

typedef std::map< SERIAL, cGuild > GUILDMAP;

typedef enum
{
	GUILD_NO_ERROR,
} GUILD_ERROR;


class cGuildz
{
	public:
		cGuildz();
		~cGuildz();

	public:

		void archive();
		void safeoldsave();
		void load();
		void save( FILE *worldfile );

	private:
		GUILDMAP guilds;

	public:

		P_GUILD getGuild( SERIAL guild );
		P_GUILD addGuild( P_ITEM stone, P_CHAR master );

};

extern cGuildz Guildz;

//Changed by Sparhawk
#define ISVALIDGUILD(guild) ( (guild>INVALID) && (guildes.find(guild) != guildes.end()) )

#endif

