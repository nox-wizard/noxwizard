  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __raceskill__
#define __raceskill__

#include "nxwcommn.h"
#include "scp_parser.h"

#include "racescriptentry.h"

class RaceSkillAdvancePoint
{
	private:
		UI32			base;
		UI32			success;
		UI32			failure;
	public:
		RaceSkillAdvancePoint( void );
		UI32			getBase( void );
		UI32			getSuccess( void );
		UI32			getFailure( void );
		RaceSkillAdvancePoint& 	operator=( const string& );
};

class RaceSkillAdvancePoints
{
	private:
		UI32	strength,
			dexterity,
			intelligence;
		bool	unhideOnUse;	// also in RaceSkill, but here so scripts/skill.xss can be used
		map< UI32, RaceSkillAdvancePoint > skillAdvancePoints; // Hmmm this could just be a vector
		RaceSkillAdvancePoint dummy;
	public:
		RaceSkillAdvancePoints( void );
		UI32	getStrength( void );
		UI32	getDexterity( void );
		UI32	getIntelligence( void );
		bool	getUnhideOnUse( void );
		void	show( void );
		RaceSkillAdvancePoint& getSkillAdvancePoint( UI32 baseSkill );
		RaceSkillAdvancePoints& operator+=( RaceSkillAdvancePoint& rsap );
		RaceSkillAdvancePoints&	operator=( RaceScriptEntry& rse );
};


class RaceSkillModifier
{
	private:
		UI32			base;
		R32				modifier;
	public:
		RaceSkillModifier( void );
		UI32			getBase( void );
		R32				getModifier( void );
		RaceSkillModifier&	operator=( const string& );
};

class RaceSkillModifiers
{
	private:
		map< UI32, RaceSkillModifier > skillModifiers;
	public:
		RaceSkillModifiers( void );
		RaceSkillModifiers&	operator+=( RaceSkillModifier& rsm );
		RaceSkillModifiers&	operator=( RaceScriptEntry& rse );
};


class RaceSkill
{
	private:
		static string		names[];
	private:
		UI32			id;
		bool			canUseSkill;
		bool			canUseModifier;
		bool			canUseAdvance;
		bool			unhideOnUse;
		UI32			startValue;
		RaceSkillModifiers	modifier;
		RaceSkillAdvancePoints	advance;
	public:
		RaceSkill( void );
		UI32			getId( void );
		const string&		getName( void );
		bool			getCanUseSkill( void );
		bool			getCanUseModifier( void );
		bool			getCanUseAdvance( void );
		bool			getUnhideOnUse( void );
		UI32			getStartValue( void );
		void			show( void );
		RaceSkillAdvancePoint&	getAdvance( UI32 baseSkill );
		UI32			getAdvanceStrength( void );
		UI32			getAdvanceDexterity( void );
		UI32			getAdvanceIntelligence( void );
		RaceSkill&		operator=( RaceScriptEntry& rse );
};

class RaceSkills
{
	private:
		map< UI32, RaceSkill > 	skills;
		RaceSkill								dummy;
	public:
									RaceSkills( void );
		void					show( void );
		RaceSkill&		getSkill( UI32 skillId );
		RaceSkills&		operator+=( RaceSkill& );
		RaceSkills&		operator+=( RaceScriptEntry& );
};


#endif
