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
\brief Magic Related stuff
*/

#ifndef __MAGIC_H__
#define __MAGIC_H__

#include "nxwcommn.h"
#include "targeting.h"
#include "packets.h"
#include "menu.h"


//! dont use up mana
#define SPELLFLAG_NOUSEMANA 0x1
//! dont check criminal stuff
#define SPELLFLAG_DONTCRIMINAL 0x2
//! dont use up reagents
#define SPELLFLAG_DONTREQREAGENTS 0x4
//! dont check spellbook for spell presence
#define SPELLFLAG_DONTCHECKSPELLBOOK 0x8

//! optional parameter is the skill to be used instead of magery
#define SPELLFLAG_PARAMISSKILLTOUSE 0x10
//! optional parameter is the damage to be done or healed
#define SPELLFLAG_PARAMISDAMAGE 0x20
//! optional parameter is the bonus/malus to be applied
#define SPELLFLAG_PARAMISBONUS 0x20
//! optional parameter is the npc to be invoked
#define SPELLFLAG_PARAMISNPCNUMBER 0x20
//! optional parameter is the duration of some tempfx
#define SPELLFLAG_PARAMISDURATION 0x80

//! dont use UO:3D particle effects
 #define SPELLFLAG_DONTUSEPARTICLES 0x100
//! dont check magic reflection
#define SPELLFLAG_DONTREFLECT 0x200
//! dont check magic resistance
#define SPELLFLAG_IGNORERESISTANCE 0x400
//! dont check if in town
#define SPELLFLAG_IGNORETOWNLIMITS 0x800

//! do not check skill
#define SPELLFLAG_DONTCHECKSKILL 0x1000
//! spell is silent
 #define SPELLFLAG_SILENTSPELL	0x2000
//! spell is immoble
 #define SPELLFLAG_IMMOBLESPELL 0x4000
//! spell has a great chance to hit (scrolls)
#define SPELLFLAG_BONUSCHANCE  0x8000

//! this is the default for NPC casting :]
#define SPELLFLAGS_NPCDEFAULT (SPELLFLAG_DONTCRIMINAL|SPELLFLAG_DONTREQREAGENTS|SPELLFLAG_DONTCHECKSPELLBOOK|SPELLFLAG_IGNORETOWNLIMITS|SPELLFLAG_DONTCHECKSKILL)

/*!
\brief Magic related stuff
*/
namespace magic {

	/*!
	\author Luxor
	\brief Polymorph menu
	*/
	class cPolymorphMenu : public cIconListMenu {
	public:
		cPolymorphMenu( P_CHAR pc );
		virtual void handleButton( NXWCLIENT ps, cClientPacket* pkg  );
	};

	/*!
	\author Luxor
	\brief Create food menu
	*/
	class cCreateFoodMenu : public cIconListMenu {
	public:
		cCreateFoodMenu( P_CHAR pc );
		virtual void handleButton( NXWCLIENT ps, cClientPacket* pkg  );
	};

	/*!
	\author Luxor
	\brief Summon creature menu
	*/
	class cSummonCreatureMenu : public cIconListMenu {
	public:
		cSummonCreatureMenu( P_CHAR pc );
		virtual void handleButton( NXWCLIENT ps, cClientPacket* pkg  );
	};



	/*!
	\brief Spells ID
	*/
	enum SpellId {
		// Level 1
		SPELL_CLUMSY = 0,
		SPELL_CREATEFOOD,
		SPELL_FEEBLEMIND,
		SPELL_HEAL,
		SPELL_MAGICARROW,
		SPELL_NIGHTSIGHT,
		SPELL_REACTIVEARMOUR,
		SPELL_WEAKEN,
		// Level 2
		SPELL_AGILITY,
		SPELL_CUNNING,
		SPELL_CURE,
		SPELL_HARM,
		SPELL_TRAP,
		SPELL_UNTRAP,
		SPELL_PROTECTION,
		SPELL_STRENGHT,
		// Level 3
		SPELL_BLESS,
		SPELL_FIREBALL,
		SPELL_LOCK,
		SPELL_POISON,
		SPELL_TELEKINESYS,
		SPELL_TELEPORT,
		SPELL_UNLOCK,
		SPELL_WALLSTONE,
		// Level 4
		SPELL_ARCHCURE,
		SPELL_ARCHPROTECTION,
		SPELL_CURSE,
		SPELL_FIREFIELD,
		SPELL_GREATHEAL,
		SPELL_LIGHTNING,
		SPELL_MANADRAIN,
		SPELL_RECALL,
		// Level 5
		SPELL_BLADESPIRITS,
		SPELL_DISPELFIELD,
		SPELL_INCOGNITO,
		SPELL_REFLECTION,
		SPELL_MINDBLAST,
		SPELL_PARALYZE,
		SPELL_POISONFIELD,
		SPELL_SUMMON,
		// Level 6
		SPELL_DISPEL,
		SPELL_ENERGYBOLT,
		SPELL_EXPLOSION,
		SPELL_INVISIBILITY,
		SPELL_MARK,
		SPELL_MASSCURSE,
		SPELL_PARALYZEFIELD,
		SPELL_REVEAL,
		// Level 7
		SPELL_CHAINLIGHTNING,
		SPELL_ENERGYFIELD,
		SPELL_FLAMESTRIKE,
		SPELL_GATE,
		SPELL_MANAVAMPIRE,
		SPELL_MASSDISPEL,
		SPELL_METEORSWARM,
		SPELL_POLYMORPH,
		// Level 8
		SPELL_EARTHQUAKE,
		SPELL_ENERGYVORTEX,
		SPELL_RESURRECTION,
		SPELL_SUMMON_AIR,
		SPELL_SUMMON_DEAMON,
		SPELL_SUMMON_EARTH,
		SPELL_SUMMON_FIRE,
		SPELL_SUMMON_WATER,

		MAX_SPELLS,
		SPELL_INVALID = -1
	};

	enum CastingType {
		CASTINGTYPE_SPELL = 0,
		CASTINGTYPE_SCROLL,
		CASTINGTYPE_ITEM,
		CASTINGTYPE_NOMANAITEM,
		CASTINGTYPE_NPC
	};

	struct reag_st
	{
		UI32 ginseng;
		UI32 moss;
		UI32 drake;
		UI32 pearl;
		UI32 silk;
		UI32 ash;
		UI32 shade;
		UI32 garlic;
	};
	
	struct g_Spell
	{
		bool enabled;		// spell enabled?
		int circle; 		// circle number
		int mana;			// mana requirements
		int loskill;		// low magery skill req.
		int hiskill;		// high magery skill req.
		int sclo;			// low magery skill req. if using scroll
		int schi;			// high magery skill req. if using scroll
		//char mantra[27];	// words of power
		std::string mantra;	// words of power
		int action; 		// character action
		int delay;			// spell delay
		reag_st reagents;	// reagents req.
		//char strToSay[102]; // string visualized with targ. system
		std::string strToSay; // string visualized with targ. system
		bool reflect;		// 1=spell reflectable, 0=spell not reflectable
		bool runic;
		int lodamage;
		int hidamage;
		bool attackSpell;
		int areasize;
		int alwaysflag;
		DamageType damagetype;
	};

	enum {	TARGTYPE_NONE = 0, TARGTYPE_XYZ, TARGTYPE_ITEM, TARGTYPE_CHAR,
		TARGTYPE_CONTAINERORDOOR, TARGTYPE_CONTAINER, TARGTYPE_RUNE };

	extern g_Spell g_Spells[MAX_SPELLS];

	LOGICAL checkMagicalSpeech( P_CHAR pc, char* speech );
	bool beginCasting (SpellId num, NXWCLIENT s, CastingType type);
	SpellId spellNumberFromScrollId(int id);
	void castSpell(SpellId spellnumber, TargetLocation& dest, P_CHAR src = NULL, int flags = 0, int param = 0);
	void castAreaAttackSpell (int x, int y, SpellId spellnum, P_CHAR pcaster = NULL);
	void loadSpellsFromScript();
	bool spellRequiresTarget(SpellId spellnum);
	P_CHAR summon (P_CHAR owner, int npctype, int duration, bool bTamed = true, int x = INVALID, int y = INVALID, int z = INVALID);
	bool checkGateCollision( P_CHAR pc );
	UI32 getCastingTime( SpellId spell );


} // namespace

#endif //__MAGIC_H__
