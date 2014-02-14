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
	SERIAL fealty;

		cGuildMember( SERIAL serial );
		~cGuildMember();

		void load( cStringFile& file );
		void save( FILE* file );
		inline const SERIAL getSerial() const
		{ return serial; }
		inline void setSerial(SERIAL newSerial) 
		{ serial = newSerial; }
		inline const UI32 getRank() const
		{ return rank; }
		inline void setRank(UI32 newRank )
		{ rank= newRank; }
		inline const SERIAL getFealty() const
		{ return fealty; }
		inline void setFealty(SERIAL newFealty )
		{ fealty= newFealty; }
		
};


#define GUILD_POLITICS_WAR 0
#define	GUILD_POLITICS_PEACE 1
#define	GUILD_POLITICS_ALLIED 2

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
	GUILD_TYPE_CHAOS,
	GUILD_TYPE_ORDER,
	GUILD_TYPE_CITY
} GUILD_TYPE;


/*!
\brief a guild
\author Endymion
*/
class cGuild
{
private:
	SERIAL serial;	//!< guild serial, is equal to the guildstone serial
	GUILD_TYPE type;	//!< guild type

	public:

		void load( cStringFile& file );
		void save( FILE* file );

	public:



	private:
		std::string	name;	//!< guild name ( es Keeper of the Crimson Soul )
		std::string	abbreviation;	//!< abbreviation ( es KCS )
		SERIAL_VECTOR guildWar;
		SERIAL_VECTOR guildPeace;
		SERIAL_VECTOR guildAllies;

		SERIAL guildMaster;
	typedef enum  {
		RANK_GUILDMASTER,
		RANK_GUILDVICE,
		RANK_GUILDMEMBER,
		RANK_GUILDRECRUIT,
		ALL_RANKS
	} GUILD_RANK;	
	public:

		std::map< SERIAL, P_GUILD_MEMBER > members;	//!< all members of this guild
		std::wstring	charter;	//!< charter
		std::string webpage;	//!< web page

		cGuild( SERIAL guildstone );
		~cGuild();
		void load();
		SERIAL getSerial();

		void setName( std::string &newName );
		std::string getName();
		void setAbbreviation( std::string &newAbbr );
		std::string getAbbreviation();
		inline const	GUILD_TYPE	getGuildType() const { return type; };
		inline void		setGuildType(GUILD_TYPE newGuildType)  
		{ if ( newGuildType >= GUILD_TYPE_NORMAL && newGuildType <=  GUILD_TYPE_ORDER) type = newGuildType; };

		// Helper functions for 
		inline const	SERIAL	getGuildMaster() const { return guildMaster; };
		inline void		setGuildMaster(SERIAL newGuildMaster)  
		{ guildMaster=newGuildMaster; getMember(guildMaster)->setRank(RANK_GUILDMASTER); };
		void calculateFealty();
		void showTitle(P_CHAR pc, P_CHAR pc2);
	public:
		inline std::map< SERIAL, P_GUILD_MEMBER >getMembers()
		{ return members; }
		P_GUILD_MEMBER addMember( P_CHAR pc );
		void resignMember( P_CHAR pc );
		inline void removeMember(SERIAL member)
		{ members.erase(member); }
		P_GUILD_MEMBER getMember( SERIAL member );
		int getMemberPosition(SERIAL member);
		SERIAL getMemberByIndex(int index);
		int getRecruitIndex(SERIAL member);
		SERIAL getRecruitByIndex(int index);
		std::vector<SERIAL> *getGuildsInWar();
		std::vector<SERIAL> *getGuildsInPeace();
		std::vector<SERIAL> *getGuildsAllied();
		void addWar(SERIAL guild);
		void addPeace(SERIAL guild);
		void addAlly(SERIAL guild);
		bool hasWarWith(SERIAL guild);
		bool hasPeaceWith(SERIAL guild);
		bool hasAllianceWith(SERIAL guild);
		void makePeace(SERIAL guild);
	public:

		std::map< SERIAL, P_GUILD_RECRUIT > recruits;	//!< all who want become member of this guild
		
		P_GUILD_RECRUIT addNewRecruit( P_CHAR recruit, P_CHAR recruiter );
		void refuseRecruit( P_CHAR pc );
		P_GUILD_RECRUIT getRecruit( SERIAL recruit );
		

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
		
		void archive();
		bool load();
		bool save();

	public:

		std::map< SERIAL, P_GUILD > guilds;

		cGuildz();
		~cGuildz();

		P_GUILD getGuild( SERIAL guild );
		P_GUILD addGuild( SERIAL stone );
		void removeGuild( SERIAL guild );
		int compareGuilds(P_GUILD guild1,P_GUILD guild2);
		void checkConsistancy(void );
};

extern cGuildz Guildz;


#endif

