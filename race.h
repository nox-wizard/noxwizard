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

typedef enum { ENEMY, NEUTRAL, FRIEND } RACIALRELATION;

typedef enum { NOBEARD,	SHORTBEARD, LONGBEARD, GOATEE, MOUSTACHE, SHORTBEARDWITHMOUSTACHE, LONGBEARDWITHMOUSTACHE, VANDYKE } BEARDSTYLE;

typedef enum { NOPOISON, LESSERPOISON, REGULARPOISON, GREATERPOISON, DEADLYPOISON, LETHALPOISON, POISONSTRENGHTS = 6 } POISONSTRENGTH;

class Race
{
	private:
		//
		//	private static data
		//
		static	std::map<UI32, class Race*>
					raceMap;
		static	cScpScript* 	script;
		static	bool		activeRaceSystem;

	private:
		//
		//	private static methods
		//
		static	void		enlistDialog1( const NXWSOCKET socket );	// Main enlist menu
		static	void		dialogRaceInfo( const NXWSOCKET socket );		// 	Information about races
		static	void		dialogNotImplemented( const NXWSOCKET socket );	// 		General race information
		static	void		dialogPlayerRaceInfo( const NXWSOCKET socket );	//		Player race list
		static	void		dialogNonPlayerRaceInfo( const NXWSOCKET socket );	//		Non player race list
		static	void		dialogAnyRaceInfo( const NXWSOCKET socket );	//		Any race list
		static	void		dialogChoosePlayerRace( const NXWSOCKET socket );	//	Select race
		static	void		enlistDialog121( const NXWSOCKET socket );	//		Race parameter dialog

		static	void		showRaceDescription( const NXWSOCKET socket, UI32 raceId );
//		static	void		showRaceDescription( const NXWSOCKET socket, const UI32 raceId, short int gumpId, bool canClose, bool canMove, bool withOk );
		static	void		showRaceDescription( const NXWSOCKET socket, const UI32 raceId, BYTE gumpId, bool canClose, bool canMove, bool withOk );
//		static	void		showRaceNameList( const NXWSOCKET socket, const RACETYPE raceType, short int gumpId, bool canClose, bool canMove, bool withOk ); // race list pc | npc | both
		static	void		showRaceNameList( const NXWSOCKET socket, const RACETYPE raceType, BYTE gumpId, bool canClose, bool canMove, bool withOk ); // race list pc | npc | both

		static	void		parseGlobalSection( void );
		static	void		parseRaceSection( void );
		static	void		scriptConOut( void );

	public:

		static	bool		teleportOnEnlist;
		static	bool		withWebInterface;
		static	string		globalWebRoot;
		static	string		globalWebLink;
		static	short		startLocation[3];
		
		//
		//	public static methods
		//
		static	void		load( const char* scriptName );
		static	void		reload( const char* scriptName );
		static	void		parse( void );

		static	void		enlist( const NXWSOCKET socket );
		//static	void		resurrect( const P_CHAR pc );

		static	bool		isRaceSystemActive( void );
		static	const 	string*	getName( const UI32 raceId );
		static	const 	string*	getPluralName( const UI32 raceId );
		static	void		setName( const UI32 raceId, const char *newName )	;

		static	R32			getPoisonResistance( const UI32 raceId, PoisonType poisonStrength )	;
		static	bool		isPlayerRace( const UI32 raceId );
		static	RACIALRELATION	getRacialRelation( UI32 raceId1, UI32 raceId2 );
		static	Race*		getRace( UI32 raceId );
		static	RequiredType	getBeardPerm( UI32 raceId );
		static	RequiredType	getHairPerm( UI32 raceId );

		static	bool		isProtectedLayer( UI32 raceId, unsigned char layer );

		//static	UI32		getSkillCap( UI32 raceId );
		//static	UI32		setSkillCap( UI32 raceId, newSkillCap );

	private:
			//
			// race id
			//
			UI32				id;
			//
			// race name
			//
			string				pluralName;
			//
			// race mode
			//
			bool				activeRace;
			//
			// pc or npc race
			//
			//
			// males or females only or mixed
			//
			Gender				gender;
			//
			// -- race description
			//
			//
			// -- appearance
			//
			//
			// -- abilities
			//
			UI32				statCap;

			UI32				strStart;
			UI32				strCap;
			RaceAbilityModifiers		strModifiers;

			UI32				dexStart;
			UI32				dexCap;
			RaceAbilityModifiers		dexModifiers;

			UI32				intStart;
			UI32				intCap;
			RaceAbilityModifiers		intModifiers;
			//
			// -- resistances
			//
			bool				magicResistant;
			R32				poisonResistance[6];
			//
			// -- relations
			//
			std::map<UI32, RACIALRELATION>	m_mapRacialRelations;
			//
			// -- items gained when pc becomes race member
			//
			vector< class RaceStartItem >   startItems;
			//
			// -- skills
			//
			UI32				skillCap;
			RaceSkills			skills;
			//
			// -- miscelaneous
			//
			UI32				tileId;
			UI32				font;
			UI32				say;
			UI32				emote;

	private:
			void				parseAbilityModifiers( const RACIALABILITY ability, const string& sectionName );
			void				parseBeardColor( const string& sectionName );
			void				parseHairColor( const string& sectionName );
			void				parsePoisonResistance( const string& sectionName );
			void				parseRaceDescription( const string& sectionName );
			void				parseSkinColor( const string& sectionName );
			void 				parseSkill( const string& sectionName );
			void 				parseStartItem( const string& itemReference );
			void 				parseStartItemDetails( const string& itemReference );

	public:
							Race( void );
			virtual				~Race( void );

			bool				getCanUseSkill( UI32 skillId );
			UI32				getDexCap( void );
			int				getDexModifier( UI32 baseDex );
			UI32				getDexStart( void );
			UI32				getIntCap( void );
			int				getIntModifier( UI32 baseDex );
			UI32				getIntStart( void );
			const string*			getPluralName( void );
			RACIALRELATION			getRacialRelation( UI32 raceId );
			UI32				getSkillAdvanceDexterity( UI32 skillId );
			UI32				getSkillAdvanceIntelligence( UI32 skillId );
			UI32				getSkillAdvanceStrength( UI32 skillId );
			UI32				getSkillAdvanceSuccess( UI32 skillId, UI32 baseSkill );
			UI32				getSkillAdvanceFailure( UI32 skillId, UI32 baseSkill );
			UI32				getSkillCap( void );
			UI32				getStatCap( void );
			UI32				getStrCap( void );
			int				getStrModifier( UI32 baseDex );
			UI32				getStrStart( void );

			bool				isPlayerRace( void );
			bool				isProtectedLayer( unsigned char layer );
			bool				isRaceActive( void );

			void				setRacialRelation( UI32 raceId, RACIALRELATION relation );
			void				setSkillCap( UI32 newSkillCap );
			void				show( void );

			vector< UI32 >			skinColor;
			string				webLink;
			std::vector< string >		description;
			RaceType			raceType;
			UI32				skin;
			//UI32		skinColor;
			RequiredType			hairPerm;
			UI32				hairStyle;
			vector< UI32>			hairColor;
			RequiredType			beardPerm;
			vector< BEARDSTYLE >		beardStyles;
			vector< UI32 >			beardColor;
			string				name;
};



#endif
