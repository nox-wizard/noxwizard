  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "racescriptentry.h"

typedef enum { STRENGTH, DEXTERITY, INTELLIGENCE, CONSTITUTION, WISDOM, CHARISMA } RACIALABILITY;
#define RACIALABILITIES	6

class RaceAbilityModifier
{
	public:
														RaceAbilityModifier( void );
		UI32										getBase( void );
		int											getModifier( void );
		RaceAbilityModifier& 		operator=( const string& );
	private:
		UI32										base;
		int											modification;
};

typedef map< UI32, class RaceAbilityModifier > RaceAbilityModifierMap;

class RaceAbilityModifiers
{
	private:
		RaceAbilityModifierMap	modifiers;
		RaceAbilityModifier			dummy;
	public:
														RaceAbilityModifiers( void );
		RaceAbilityModifier&		getAbilityModifier( UI32 baseAbility );
		RaceAbilityModifiers&		operator+=( RaceAbilityModifier& that );
		RaceAbilityModifiers&		operator=( RaceScriptEntry& rse );
};
