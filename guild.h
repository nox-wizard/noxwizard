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
 
/*
\class cGuildRecruit
\note a candidate for membership of a guild
\author Sparhawk
*/
class cGuildRecruit
{
	public:
				cGuildRecruit();
				cGuildRecruit( const P_CHAR pChar );
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

//typedef std::multimap< SERIAL, cGuildMember > mmGuildMember;
//typedef mmGuildMember::iterator mmiGuildMember;


/*
\class cGuildMember
\note a member of a guild
\author Sparhawk
*/
class cGuildMember
{

	//
	// Class properties and methods
	//
	private:
		//
		// Maximum guilds a pc join as member
		//
		static UI32			membershipMaximum;
	public:
		static UI32			getMembershipMaximum();
		static void			setMembershipMaximum( UI32 maximum );
		static LOGICAL			multiMembershipAllowed();
	public:
		//
		// Worldfile save and load methods
		//
		//static	void		load();
		//static	void		save( FILE *worldfile );
	private:
//		static	mmGuildMember	members;

	public:
				cGuildMember();
				//cGuildMember( const cGuildRecruit &recruit );
				~cGuildMember();
	private:
		SERIAL		serial;
	public:
		SERIAL		getSerial();
		void		setSerial( SERIAL );
	private:
		std::string	title;	//!< Guild Title Name
	public:
		void		setTitle( const std::string &newTitle );
		std::string	getTitle();
	private:
		UI08		toggle;	//!< 0=All / 1=Only GuildName / 2=None
	public:
		void		setToggle( UI08 newToggle );
		UI08		getToggle();
	public:
		cGameDate	membershipDate;
};

/*
\class cGuilded
\note the menber of a guild
\author Endymion
*/
class cGuilded
{
	public:
				cGuilded();
				~cGuilded();
	private:
		std::string	title;	//!< Guild Title Name
	public:
		void		setTitle( const std::string &newTitle );
		std::string	getTitle();
	private:
		UI08		toggle;	//!< 0=All / 1=Only GuildName / 2=None
	public:
		void		setToggle( UI08 newToggle );
		UI08		getToggle();
	/*
	private:
		SERIAL		acceptedBy;
	public:
		SERIAL		getAcceptedBy;
		void		setAcceptedBy;
	*/
};

typedef std::map< SERIAL, cGuilded > GUILDMEMBERMAP;
typedef std::map< SERIAL, cGuilded >::iterator GUILDMEMBERMAPITER;
typedef std::vector< SERIAL > SERIALVECTOR;
typedef std::vector< SERIAL >::iterator SERIALVECTORITER;


/*
\class cGuild
\note a guild
\author Endymion
*/
typedef class cGuild* P_GUILD;

class cGuild
{
	//
	// Class properties and methods
	//
	public:
		static void	archive();
		static void	safeoldsave();
	//
	// Object properties and methods
	//
	public:
				cGuild( const P_ITEM pGuildStone = 0, const P_CHAR pChar = 0 );
				cGuild( const cGuild &copy );
				cGuild( const SERIAL );
				~cGuild();
	private:
		SERIAL		serial;	//!< Guild Serial = equal to the guildstone serial
	public:
		void		setSerial( SERIAL newSerial );
		SERIAL		getSerial();
		void		create( SERIAL newSerial );
	/*
	// Failsafe for when someone accidentally deletes the guildstone
	//
	private:
		UI32		guildStoneScriptId;
		Location	guildStoneLocation;
	public:
		UI32		getGuildStoneScriptId();
		void		setGuildStoneScriptId( UI32 newScript );
		Location	getGuildStoneLocation();
		void		setGuildStoneLocation( Location newLoc );
	*/
	private:
		std::string	name;	//!< Guild Name ( es Keeper of the Crimson Soul )
	public:
		void		setName( const std::string &newName );
		std::string	getName();

	private:
		std::string	abbreviation;	//!< Guild Abbreviation ( es KCS )
	public:
		void		setAbbreviation( const std::string &newAbbr );
		std::string	getAbbreviation();

	private:
		std::string	charter;	//!< Guild Charter
	public:
		void		setCharter( const std::string &newCharter );
		std::string	getCharter();

	private:
		UI08		type;	//!< 0=standard / 1=chaos / 2=order
	public:
		LOGICAL		setType( UI08 newType );
		UI08		getType();

	private:
		std::string 	webpage;	//!< Web Page
	public:
		void 		setWebPage( const std::string &newWebPage );
		std::string 	getWebPage();

	private:
		SERIAL		guildmaster;	//!< the guild master
	public:
		GUILDMEMBERMAP	members;	//!< All member of this guild
		void		loadMembers();
		LOGICAL		isMember( P_CHAR pChar );
		LOGICAL		addMember( P_CHAR pc );
		LOGICAL		resignMember( P_CHAR pc );
		void 		setGuildMaster( P_CHAR pc );
		SERIAL		getGuildMaster();
	public:
		SERIALVECTOR 	recruits;	//!< All who want become member of this guild
		LOGICAL 	addNewRecruit( P_CHAR pc );
		LOGICAL 	refuseRecruit( P_CHAR pc );
	public:
		SERIALVECTOR	war;	//!< All war guild with this
		void		declareWar( SERIAL guild );
		bool		isInWar( SERIAL guild );
		void		declarePeace( SERIAL guild );
	public:
		SERIALVECTOR	allied;	//!< All allied guild with this
		void		declareAllied( SERIAL guild );
		bool		isAllied( SERIAL guild );
		void		declareNeutral( SERIAL guild );
};


typedef std::map< SERIAL, cGuild > GUILDMAP; //Changed by Sparhawk
typedef std::map< SERIAL, cGuild >::iterator GUILDMAPITER; //Changed by Sparhawk

extern GUILDMAP guildes;


class cGuildz
{
	public:
						cGuildz();
						~cGuildz();

	public:
		//
		// Worldfile save and load methods
		//
		void				archive();
		void				safeoldsave();
		void				load();
		void				save( FILE *worldfile );

	enum eGuildzError
	{
		GUILD_ERROR_NO_ERROR,
		GUILD_ERROR_END_OF_SET,
		GUILD_ERROR_NO_DATA,
		GUILD_ERROR_INVALID_INDEX,
		GUILD_ERROR_INVALID_SERIAL,
		GUILD_ERROR_DUPLICATE_SERIAL,
		GUILD_ERROR_SERIAL_NOT_FOUND,
		GUILD_ERROR_NO_NAME,
		GUILD_ERROR_DUPLICATE_NAME,
		GUILD_ERROR_NAME_NOT_FOUND,
		GUILD_ERROR_NO_ABBREVIATION,
		GUILD_ERROR_DUPLICATE_ABBREVIATION
	};

	private:
		eGuildzError			error;
	public:
		UI32				getError();
	
	typedef std::map< SERIAL, cGuild > 	mGuild;
	typedef mGuild::iterator		miGuild;
	typedef std::map< std::string, SERIAL > mGuildName;
	typedef mGuildName::iterator 		miGuildName;

	enum eGuildzIndex
	{
		GUILD_INDEX_SERIAL,
		GUILD_INDEX_NAME
	};

	private:
		mGuild				guilds;
		miGuild				guildsIterator;
		mGuildName			guildNames;
		miGuildName			guildNamesIterator;
		eGuildzIndex			currentIndex;

	public:
		//
		// Guild manipulation methods
		//
		LOGICAL				insert( cGuild &guild );
		LOGICAL				update( cGuild &guild );
		LOGICAL				update( cGuild *guild );
		LOGICAL				erase( SERIAL guild );
		LOGICAL				erase( cGuild &guild );
		LOGICAL				erase( cGuild *guild );
		LOGICAL				find( SERIAL guild );
		LOGICAL				find( std::string &name );
		void				first();
		void				last();
		void				next();
		void				previous();
		cGuild*				fetch();
		//
		// Guild membership info
		//
		LOGICAL				isGuildMember( P_CHAR pChar );
		LOGICAL				isGuildMember( SERIAL charSerial );
		LOGICAL				isGuildMember( P_CHAR pChar, SERIAL guildSerial );
		LOGICAL				isGuildMember( SERIAL charSerial, SERIAL guildSerial );
		//
		// Direct guild index manipulation methods
		//
		UI32				getIndex();
		LOGICAL				setIndex( UI32 index );
};

extern cGuildz Guildz;

//Changed by Sparhawk
#define ISVALIDGUILD(guild) ( (guild>0) && (guildes.find(guild) != guildes.end()) )

P_ITEM PlaceGuildStoneDeed( P_CHAR pc, P_ITEM deed );

#endif

