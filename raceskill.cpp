  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "raceskill.h"
#include "basics.h"

//
// SkillAdvancePoint
//

RaceSkillAdvancePoint::RaceSkillAdvancePoint( void )
{
	base = success = failure = 0;
}

RaceSkillAdvancePoint& RaceSkillAdvancePoint::operator=( const string& that )
{
	sscanf( that.c_str(), "%d %d %d", &this->base, &this->success, &this->failure );
	return *this;
}

UI32 RaceSkillAdvancePoint::getBase( void )
{
	return this->base;
}

UI32 RaceSkillAdvancePoint::getSuccess( void )
{
	return this->success;
}

UI32 RaceSkillAdvancePoint::getFailure( void )
{
	return this->failure;
}

//
// RaceSkillAdvancePoints
//

RaceSkillAdvancePoints::RaceSkillAdvancePoints( void )
{
}

UI32 RaceSkillAdvancePoints::getStrength( void )
{
	return this->strength;
}

UI32 RaceSkillAdvancePoints::getDexterity( void )
{
	return this->dexterity;
}

UI32 RaceSkillAdvancePoints::getIntelligence( void )
{
	return this->intelligence;
}

bool RaceSkillAdvancePoints::getUnhideOnUse( void )
{
	return this->unhideOnUse;
}

RaceSkillAdvancePoint& RaceSkillAdvancePoints::getSkillAdvancePoint( UI32 baseSkill )
{
	if(skillAdvancePoints.empty()) return dummy;

	map< UI32, RaceSkillAdvancePoint >::iterator it( skillAdvancePoints.end() ), begin( skillAdvancePoints.begin() );
	do it--; while( it != begin && it->second.getBase() > baseSkill );
	
	return it->second;
}

void RaceSkillAdvancePoints::show( void )
{
	ConOut("  Advance       :\n");
	map< UI32, RaceSkillAdvancePoint >::iterator it( skillAdvancePoints.begin() ), end( skillAdvancePoints.end() );
	for(; it != end; ++it )
		ConOut("  %4d %4d %4d\n", it->second.getBase(), it->second.getSuccess(), it->second.getFailure());
}

RaceSkillAdvancePoints& RaceSkillAdvancePoints::operator+=( RaceSkillAdvancePoint& that )
{
	skillAdvancePoints[ that.getBase() ] = that;
	return *this;
}

RaceSkillAdvancePoints& RaceSkillAdvancePoints::operator=( RaceScriptEntry& that )
{
	cScpIterator*	iter = NULL;
	iter = that.script->getNewIterator( that.section );
    	if ( iter == NULL )
	{
		ErrOut( "RaceSkillAdvancePoints() no %s\n", that.section.c_str() );
	}
	else
	{
		UI32 			loopexit = 0;
		string			lha,
					rha;
		RaceSkillAdvancePoint	rsap;
		do
		{
			iter->parseLine( lha, rha );
			switch( lha[0] )
			{
				case '{' :
				case '}' :
					break;
				case 'D' :
					if( lha == "DEX" )
						this->dexterity = str2num( rha );
					break;
				case 'I' :
					if( lha == "INT" )
						this->intelligence = str2num( rha );
					break;
				case 'S' :
					if( lha == "SKILLPOINT" )
					{
						rsap = rha;
						*this += rsap;
					}
					else if ( lha == "STR" )
						this->strength = str2num( rha );
					break;
				case 'U' :
					if ( lha == "UNHIDEONUSE" )
						this->unhideOnUse = str2num( rha );
					break;
				default:
					WarnOut("RaceSkillAdvancePoint unknown tag %s %s\n", lha.c_str(), rha.c_str() );
					break;
			}

		} while ( ( lha[0] != '}' ) && ( ++loopexit < MAXLOOPS ) );
	}
	return *this;
}

//
// RaceSkillModifier
//

RaceSkillModifier::RaceSkillModifier( void )
{
}

RaceSkillModifier& RaceSkillModifier::operator=( const string& that )
{
	sscanf( that.c_str(), "%d %f", &this->base, &this->modifier );
	return *this;
}

UI32 RaceSkillModifier::getBase( void )
{
	return this->base;
}

R32 RaceSkillModifier::getModifier( void )
{
	return this->modifier;
}

//
// RaceSkillModifiers
//

RaceSkillModifiers::RaceSkillModifiers( void )
{
}

RaceSkillModifiers& RaceSkillModifiers::operator+=( RaceSkillModifier& that )
{
	this->skillModifiers[ that.getBase() ] = that;
	return *this;
}

RaceSkillModifiers& RaceSkillModifiers::operator=( RaceScriptEntry& that )
{
	cScpIterator*	iter = NULL;
	iter = that.script->getNewIterator( that.section );
    	if ( iter == NULL )
	{
		ErrOut( "RaceSkillModifier() no %s\n", that.section.c_str() );
	}
	else
	{
		UI32			loopexit = 0;
		string 			parms;
		RaceSkillModifier	rsm;
		do
		{
			parms = iter->getEntry()->getFullLine();
			if ( parms[0] != '{' && parms[0] != '}' )
			{
				rsm	= parms;
				*this  += rsm;
			}
		} while ( ( parms[0] != '}' ) && ( ++loopexit < MAXLOOPS ) );
	}
	return *this;
}

//
// RaceSkill
//

string RaceSkill::names[] = 	{
					"ALCHEMY",
					"ANATOMY",
					"ANIMALLORE",
					"ITEMID",
					"ARMSLORE",
					"PARRYING",
					"BEGGING",
					"BLACKSMITHING",
					"BOWCRAFT",
					"PEACEMAKING",
					"CAMPING",
					"CARPENTRY",
					"CARTOGRAPHY",
					"COOKING",
					"DETECTINGHIDDEN",
					"ENTICEMENT",
					"EVALUATINGINTEL",
					"HEALING",
					"FISHING",
					"FORENSICS",
					"HERDING",
					"HIDING",
					"PROVOCATION",
					"INSCRIPTION",
					"LOCKPICKING",
					"MAGERY",
					"MAGICRESISTANCE",
					"TACTICS",
					"SNOOPING",
					"MUSICIANSHIP",
					"POISONING",
					"ARCHERY",
					"SPIRITSPEAK",
					"STEALING",
					"TAILORING",
					"TAMING",
					"TASTEID",
					"TINKERING",
					"TRACKING",
					"VETERINARY",
					"SWORDSMANSHIP",
					"MACEFIGHTING",
					"FENCING",
					"WRESTLING",
					"LUMBERJACKING",
					"MINING",
					"MEDITATION",
					"STEALTH",
					"REMOVETRAPS",
					"INVALID_SKILL"
				};

RaceSkill::RaceSkill( void )
{
	canUseSkill = canUseModifier = canUseAdvance = unhideOnUse = false;
	startValue = 0;
}

UI32 RaceSkill::getId( void )
{
	return this->id;
}

const string& RaceSkill::getName( void )
{
	return names[ this->id ];
}

bool RaceSkill::getCanUseSkill( void )
{
	return this->canUseSkill;
}

bool RaceSkill::getCanUseModifier( void )
{
	return this->canUseModifier;
}

bool RaceSkill::getCanUseAdvance( void )
{
	return this->canUseAdvance;
}

bool RaceSkill::getUnhideOnUse( void )
{
	return this->unhideOnUse;
}

RaceSkillAdvancePoint& RaceSkill::getAdvance( UI32 baseSkill )
{
	return advance.getSkillAdvancePoint( baseSkill );
}

UI32 RaceSkill::getAdvanceStrength( void )
{
	return advance.getStrength();
}

UI32 RaceSkill::getAdvanceDexterity( void )
{
	return advance.getDexterity();
}

UI32 RaceSkill::getAdvanceIntelligence( void )
{
	return advance.getIntelligence();
}

void RaceSkill::show( void )
{
	ConOut("Skill %d %s\n", id, names[id].c_str());
	ConOut("  canUseSkill   : %s\n", canUseSkill ? "yes" : "no" );
	ConOut("  canUseModifier: %s\n", canUseModifier ? "yes" : "no" );
	ConOut("  canUseAdvance : %s\n", canUseAdvance ? "yes" : "no" );
	ConOut("  unhideOnuse   : %s\n", unhideOnUse ? "yes" : "no" );
	ConOut("  startValue    : %d\n", startValue);
	advance.show();
}

RaceSkill& RaceSkill::operator=( RaceScriptEntry& that )
{
	cScpIterator*	iter = NULL;
	iter = that.script->getNewIterator( that.section );
    	if ( iter == NULL )
	{
		ErrOut( "RaceSkill() no %s\n", that.section.c_str() );
	}
	else
	{
		UI32		loopexit = 0;
		string  lha, rha;
		bool		unhideOnUseSet = false;
		do
		{
			iter->parseLine( lha, rha );
			switch( lha[0] )
			{
				case '{' :
				case '}' :
					break;
				case 'A' :
					if( lha == "ADVANCE" )
					{
						std::string section("SECTION RACESKILLADVANCE ");
						section += rha;
						RaceScriptEntry rse( that.script, section );
						this->advance = rse;
					}
					break;
				case 'C' :
					if ( lha == "CANUSEADVANCE" )
					{
						if ( rha == "" || rha == "YES" )
							this->canUseAdvance = true;
						else
							this->canUseAdvance = false;
					}
					else if ( lha == "CANUSEMODIFIER" )
					{
						if ( rha == "" || rha == "YES" )
							this->canUseModifier = true;
						else
							this->canUseModifier = false;
					}
					else if ( lha == "CANUSESKILL" )
					{
						if ( rha == "" || rha == "YES" )
							this->canUseSkill = true;
						else
							this->canUseSkill = false;
					}
					break;
				case 'I' :
					if( lha == "ID" )
						this->id = str2num( rha );
					break;
				case 'M' :
					if( lha == "MODIFIER" )
					{
						std::string section("SECTION RACESKILLMODIFIER ");
						section += rha;
						RaceScriptEntry rse( that.script, section );
						this->modifier = rse;
					}
					break;
				case 'U' :
					if ( lha == "UNHIDEONUSE" )
					{
						this->unhideOnUse = str2num( rha );
					}	unhideOnUseSet = true;
					break;
				default:
					WarnOut("RaceSkill unknown tag %s %s\n", lha.c_str(), rha.c_str() );
					break;
			}

		} while ( ( lha[0] != '}' ) && ( ++loopexit < MAXLOOPS ) );
		if ( !unhideOnUseSet )
			this->unhideOnUse = advance.getUnhideOnUse();
	}
	return *this;
}

//
// RaceSkills
//

RaceSkills::RaceSkills( void )
{
}

RaceSkill&	RaceSkills::getSkill( UI32 skillId )
{
	map< UI32, RaceSkill >::iterator it( skills.find( skillId ) );

	if ( it != skills.end() )
		return it->second;
	else
		return dummy;
}

void RaceSkills::show( void )
{
	map< UI32, RaceSkill >::iterator it( skills.begin() ), end( skills.end() );
	for(; it != end; ++it )
		it->second.show();
}

RaceSkills& RaceSkills::operator+=( RaceSkill& that )
{
	this->skills[that.getId()] = that;
	return *this;
}

RaceSkills& RaceSkills::operator+=( RaceScriptEntry& that )
{
	RaceSkill skill;
	skill = that;
	*this += skill;
	return *this;
}
