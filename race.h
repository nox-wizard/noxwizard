  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __race__
#define __race__

#include "nxwcommn.h"
#include "scp_parser.h"

#include "raceability.h"
#include "racegender.h"
#include "racerequiredtype.h"
#include "raceskill.h"
#include "racestartitem.h"
#include "racetype.h"

//#include "cPoison.h"

/*!
Racial relations, constants to define relationship between races
*/
typedef enum 
{ 
	RACE_ENEMY, 
	RACE_NEUTRAL, 
	RACE_FRIEND 
} RACIALRELATION;

/*!
Beard styles
*/
typedef enum 
{ 
	NOBEARD,	
	SHORTBEARD, 
	LONGBEARD, 
	GOATEE, 
	MOUSTACHE, 
	SHORTBEARDWITHMOUSTACHE, 
	LONGBEARDWITHMOUSTACHE, 
	VANDYKE 
} BEARDSTYLE;

typedef enum 
{ 
	NOPOISON, 
	LESSERPOISON, 
	REGULARPOISON, 
	GREATERPOISON, 
	DEADLYPOISON, 
	LETHALPOISON, 
	POISONSTRENGHTS = 6 
} POISONSTRENGTH;

/*!
\brief Race class

\todo port all static members into a wrapper class cAllRaces. Comments in the class decalration explain wich are to be ported.
*/

class Race
{
//Here I tried to subdivide members into 2 groups, cAllRaces members are thos Race members to be ported to a wrapper class.
//cRace members are to be left into this class (eventually rename class to cRace)


//======== cAllRaces members =========================//
private:

	static	std::map<UI32, class Race*> raceMap; //!< map with all available races
	static	cScpScript* script; //!< race script
	static	bool activeRaceSystem; //!< race system switch

	//these are for parsing scripts
	static void parseGlobalSection( void );
	static void parseRaceSection( void );
	static void scriptConOut( void );
	void parseAbilityModifiers( const RACIALABILITY ability, const string& sectionName );
	void parseBeardColor( const string& sectionName );
	void parseHairColor( const string& sectionName );
	void parsePoisonResistance( const string& sectionName );
	void parseRaceDescription( const string& sectionName );
	void parseSkinColor( const string& sectionName );
	void parseSkill( const string& sectionName );
	void parseStartItem( const string& itemReference );
	void parseStartItemDetails( const string& itemReference );

	//these functions are for race gui interface, they are to be removed when
	//Small interface is done
	static void enlistDialog1( const NXWSOCKET socket );//!< Main enlist menu
	static void dialogRaceInfo( const NXWSOCKET socket );//!< Information about races
	static void dialogNotImplemented( const NXWSOCKET socket );//<! General race information
	static void dialogPlayerRaceInfo( const NXWSOCKET socket );//!<	Player race list
	static void dialogNonPlayerRaceInfo( const NXWSOCKET socket );//!<Non player race list
	static void dialogAnyRaceInfo( const NXWSOCKET socket );//!< Any race list
	static void dialogChoosePlayerRace( const NXWSOCKET socket );//!< Select race
	static void enlistDialog121( const NXWSOCKET socket );//!< Race parameter dialog
	static void showRaceDescription( const NXWSOCKET socket, UI32 raceId );//!< static void	
	//showRaceDescription( const NXWSOCKET socket, const UI32 raceId, short int gumpId, bool canClose, bool canMove, bool withOk );
	static void showRaceDescription( const NXWSOCKET socket, const UI32 raceId, BYTE gumpId, bool canClose, bool canMove, bool withOk );
	//static void showRaceNameList( const NXWSOCKET socket, const RACETYPE raceType, short int gumpId, bool canClose, bool canMove, bool withOk ); // race list pc | npc | both
	static void showRaceNameList( const NXWSOCKET socket, const RACETYPE raceType, BYTE gumpId, bool canClose, bool canMove, bool withOk ); // race list pc | npc | both
	
public:

	static bool teleportOnEnlist; //!< true if character has to be teleported somewhere when joins a race
	static bool withWebInterface; //!< true if race has a web interface
	static string globalWebRoot; //!< global http root for races
	static string globalWebLink; //!< 
	
	//these are for race loading
	static void load( const char* scriptName );
	static void reload( const char* scriptName );
	static void parse( void );

	static void enlist( const NXWSOCKET socket );
	//static void resurrect( const P_CHAR pc );

	static bool isRaceSystemActive( void );
	
	static Race* getRace( UI32 raceId ); //!< gets a pointer from a race serial
	
	//these functions are redundant, they are for getting race properties from race index
	//but this function list is not complete, and moreover you can always do the same work
	//by getting a race pointer with getRace() and then using cRace member functions (see below)
	//So I would remove these.
	static const string* getName( const UI32 raceId );
	static const string* getPluralName( const UI32 raceId );
	static void setName( const UI32 raceId, const char *newName )	;
	static R32 getPoisonResistance( const UI32 raceId, PoisonType poisonStrength )	;
	static bool isPlayerRace( const UI32 raceId );
	static RACIALRELATION getRacialRelation( UI32 raceId1, UI32 raceId2 );
	static RequiredType getBeardPerm( UI32 raceId );
	static RequiredType getHairPerm( UI32 raceId );
	static bool isProtectedLayer( UI32 raceId, unsigned char layer );
	//static UI32 getSkillCap( UI32 raceId );
	//static UI32 setSkillCap( UI32 raceId, newSkillCap );


//===== cRace members ==============================//
private:

	UI32 id; //!< race serial
	string pluralName; //!< plural of race name
	bool activeRace; //!< true if race is active
	Gender gender; //!< tells if race has male/female or they are mixed
	
	UI32 statCap; //!< custom race statcap
	
	UI32 strStart; //!< starting strenght
	UI32 strCap; //!< strenght cap
	RaceAbilityModifiers strModifiers; //!< strenght modifier

	UI32 dexStart; //!< starting dex
	UI32 dexCap; //!< dex cap
	RaceAbilityModifiers dexModifiers; //!< dex modifier

	UI32 intStart; //!< starting int
	UI32 intCap; //!< int cap
	RaceAbilityModifiers intModifiers; //!< int modifier
			
	bool magicResistant; //!< true if race is magic resistant
	R32 poisonResistance[6]; //!< poison resistenca table
	std::map<UI32, RACIALRELATION>	m_mapRacialRelations; //!< relations with other races
	vector< class RaceStartItem >   startItems; //!< items given to character when it joins race
			
	UI32 skillCap; //!< skill cap
	RaceSkills skills; //!< ??
			
	UI32 tileId; //!< race symbol
	UI32 font; //!< ??
	UI32 say; //!< ??
	UI32 emote; //!< ??

public:
	Race( void );
	virtual	~Race( void );

	//these are getters for getting private members
	bool getCanUseSkill( UI32 skillId );
	UI32 getDexCap( void );
	int getDexModifier( UI32 baseDex );
	UI32 getDexStart( void );
	UI32 getIntCap( void );
	int getIntModifier( UI32 baseDex );
	UI32 getIntStart( void );
	const string* getPluralName( void );
	RACIALRELATION getRacialRelation( UI32 raceId );
	UI32 getSkillAdvanceDexterity( UI32 skillId );
	UI32 getSkillAdvanceIntelligence( UI32 skillId );
	UI32 getSkillAdvanceStrength( UI32 skillId );
	UI32 getSkillAdvanceSuccess( UI32 skillId, UI32 baseSkill );
	UI32 getSkillAdvanceFailure( UI32 skillId, UI32 baseSkill );
	UI32 getSkillCap( void );
	UI32 getStatCap( void );
	UI32 getStrCap( void );
	UI32 getStrModifier( UI32 baseDex );
	UI32 getStrStart( void );
	UI32 getRaceGender( void );
	bool isPlayerRace( void );
	bool isProtectedLayer( unsigned char layer );
	bool isRaceActive( void );
	void setRacialRelation( UI32 raceId, RACIALRELATION relation );
	void setSkillCap( UI32 newSkillCap );
	void show( void );
	
	static short startLocation[3]; //!< start location, if teleportOnEnlist is true
	vector< UI32 > skinColor;
	string webLink;
	std::vector< string > description;
	RaceType raceType;
	UI32 skin;
	//UI32		skinColor;
	RequiredType hairPerm;
	UI32 hairStyle;
	vector<UI32> hairColor;
	RequiredType beardPerm;
	vector<BEARDSTYLE> beardStyles;
	vector<UI32> beardColor;
	string name;
};

#endif
