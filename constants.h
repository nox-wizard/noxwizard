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
\brief Declarations of extern extern constants and enumerated types
*/

#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__
#include "typedefs.h"

// Miscellaneous constants
enum
{
	MAXLOOPS=66666,		//!< max cycles in a loop
	MaxZstep=9,

	MAX_GUILDTYPE = 2,	//!< Code to support the Chaos/order system
	MAXIMUM = 1024,		//!< Maximum open connections to server
	MAXCLIENT = MAXIMUM +1,
				//!< Maximum clients allowed
	MAXSERV = 15,		//!< Maximum servers in login listing
	MAXBUFFER = 2560,	//!< Buffer Size (For socket operations)
	MAXSTART = 15,		//!< Maximum starting locations
	MAXPAGES = 75,		//!< Maximum number of pages in the GM queue
	VISRANGE = 18,		//!< Visibility for normal items & chars
	BUILDRANGE = 90,	//!< Visibility for castles and keeps
	MAP0CACHE = 300,
	PATHNUM = 2,

	XYMAX = 256,		//!< Maximum items NoX-Wizard can handle on one X/Y square
	MAXLAYERS = 50,	//!< Maximum number of Layers in paperdolls (still not sure how many)
	CMAX = 40,		//!< Maximum parameters in one line (Only for memory reasons)
	SPIRITSPEAKTIMER = 30,
				//!< spiritspeak lasts at least 30 seconds, additional time is calculated by Skill & INT

	HASHMAX = 8191,		//!< lord binary, hashmax must be a prime for maximum performce
	MY_CLOCKS_PER_SEC = 1000,

	DOORTYPES = 17,
	WEIGHT_PER_STR = 4,
	POLYMORPHMENUOFFSET = 8196,
				//!< polymorph spell menu offset
	MAKEMENUOFFSET = 2048,

}; //enum

#define INVALID -1
#define UINVALID 0xFFFFFFFF
#define UINVALID08 0xFF
#define UINVALID16 0xFFFF

//@{
/*!
\name enumerated extern extern constants
*/

//! Verdata patches index
enum
{
	VERFILE_MAP = 0, VERFILE_STAIDX, VERFILE_STATICS, VERFILE_ARTIDX, VERFILE_ART,
	VERFILE_ANIMIDX, VERFILE_ANIM, VERFILE_SOUNDIDX, VERFILE_SOUND, VERFILE_TEXIDX,
	VERFILE_TEXMAPS, VERFILE_GUMPIDX, VERFILE_GUMPART, VERFILE_MULTIIDX, VERFILE_MULTI,
	VERFILE_SKILLSIDX, VERFILE_SKILLS, VERFILE_TILEDATA, VERFILE_ANIMDATA,
	TILEDATA_TILES = 0x68800
};

//! List of monster sounds (see cChar::playMonsterSound())
typedef enum { SND_STARTATTACK = 0, SND_IDLE, SND_ATTACK, SND_DEFEND, SND_DIE } MonsterSound;
#define ALL_MONSTER_SOUND 5

//! Skills
enum Skill {
	ALCHEMY = 0, ANATOMY, ANIMALLORE, ITEMID, ARMSLORE, PARRYING, BEGGING, BLACKSMITHING,
	BOWCRAFT, PEACEMAKING, CAMPING, CARPENTRY, CARTOGRAPHY, COOKING, DETECTINGHIDDEN,
	ENTICEMENT, EVALUATINGINTEL, HEALING, FISHING, FORENSICS, HERDING, HIDING, PROVOCATION,
	INSCRIPTION, LOCKPICKING, MAGERY, MAGICRESISTANCE, TACTICS, SNOOPING, MUSICIANSHIP,
	POISONING, ARCHERY, SPIRITSPEAK, STEALING, TAILORING, TAMING, TASTEID, TINKERING,
	TRACKING, VETERINARY, SWORDSMANSHIP, MACEFIGHTING, FENCING, WRESTLING, LUMBERJACKING,
	MINING, MEDITATION, STEALTH, REMOVETRAPS, TRUESKILLS, INVALID_SKILL = -1
};

enum { ALLSKILLS = 49, STR , DEX, INTEL, FAME, KARMA,
	NXWFLAG0, NXWFLAG1, NXWFLAG2, NXWFLAG3,
	AMXFLAG0, AMXFLAG1, AMXFLAG2, AMXFLAG3, AMXFLAG4, AMXFLAG5, AMXFLAG6, AMXFLAG7,
	AMXFLAG8, AMXFLAG9, AMXFLAGA, AMXFLAGB, AMXFLAGC, AMXFLAGD, AMXFLAGE, AMXFLAGF,
	I_ACCOUNT, SKILLS
};

//! cChar::teleport flags
enum {
	TELEFLAG_NONE = 0x0, TELEFLAG_SENDWORNITEMS = 0x1, TELEFLAG_SENDNEARCHARS = 0x2,
	TELEFLAG_SENDNEARITEMS = 0x4, TELEFLAG_SENDLIGHT = 0x8, TELEFLAG_SENDWEATHER = 0x10,
	TELEFLAG_SENDALL = 0x1F
};

//! Directions
enum Direction { NORTH = 0, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST };

//! for spinning wheel -vagrant
enum { YARN, THREAD };

//! Wrestling - Luxor
enum { WRESDISARM = 1, WRESSTUNPUNCH };

//! type of item use
enum { ITEM_USE_UNKNOWN = 0, ITEM_USE_WEAR, ITEM_USE_CHECKEQUIP, ITEM_USE_DBLCLICK };

enum { spawn_script = 20, NUM_SCRIPTS = 32};

//! Item privs
enum {	ITMPRIV_DECAY = 0x01, ITMPRIV_NEWBIE = 0x02, ITMPRIV_DISPELLABLE = 0x04 };

//! Region privs
enum {	RGNPRIV_GUARDED = 0x01, RGNPRIV_MARKALLOWED = 0x02, RGNPRIV_GATEALLOWED = 0x04,
	RGNPRIV_RECALLALLOWED = 0x08
};

//! Stat types
enum StatType { STAT_HP = 0, STAT_MANA, STAT_STAMINA, ALL_STATS };

//! Type of damage
enum DamageType {
	DAMAGE_PURE = 0, DAMAGE_SLASH, DAMAGE_PIERCE, DAMAGE_BLUDGEON, DAMAGE_BACKSTAB,
	DAMAGE_FIRE, DAMAGE_ELECTRICITY, DAMAGE_MENTAL, DAMAGE_POISON, DAMAGE_COLD,
	DAMAGE_FORCE, DAMAGE_HOLY, DAMAGE_MAGIC, MAX_RESISTANCE_INDEX
};

//! Type of poison
enum PoisonType { POISON_NONE, POISON_WEAK, POISON_NORMAL, POISON_GREATER, POISON_DEADLY };

//! Type of item
enum ItemType {
	ITYPE_CONTAINER = 1, ITYPE_ORDER_GATES = 2, ITYPE_ORDER_GATES_OPENER = 3, ITYPE_CHAOS_GATES = 4,
	ITYPE_CHAOS_GATES_OPENER = 5, ITYPE_TELEPORTRUNE = 6,  ITYPE_KEY = 7, ITYPE_LOCKED_ITEM_SPAWNER = 8,
	ITYPE_SPELLBOOK = 9, ITYPE_MAP = 10, ITYPE_BOOK = 11, ITYPE_DOOR = 12, ITYPE_LOCKED_DOOR = 13,
	ITYPE_FOOD = 14, ITYPE_WAND = 15, ITYPE_RESURRECT = 16,ITYPE_MANAREQ_WAND = 17, ITYPE_POTION = 19,
	ITYPE_RUNE = 50, ITYPE_ITEM_SPAWNER = 61, ITYPE_NPC_SPAWNER = 62, ITYPE_UNLOCKED_CONTAINER = 63,
	ITYPE_LOCKED_CONTAINER = 64, ITYPE_NODECAY_ITEM_SPAWNER = 65, ITYPE_DECAYING_ITEM_SPAWNER = 66,
	ITYPE_RAND_NPC_SPAWNER = 69, ITYPE_ADVENCEMET_GATE1 = 80, ITYPE_ADVENCEMET_GATE2 = 81,
	ITYPE_ADVENCEMET_GATE_SEX = 82, ITYPE_TRASH = 87, ITYPE_BOATS = 117, ITYPE_FIREWORKS_WAND = 181,
	ITYPE_SMOKE = 185, ITYPE_RENAME_DEED = 186, ITYPE_POLYMORPH = 101, ITYPE_POLYMORPH_BACK = 102,
	ITYPE_ARMY_ENLIST = 103, ITYPE_TELEPORT = 104, ITYPE_DRINK = 105, ITYPE_ESCORTSPAWN = 125,
	ITYPE_GUILDSTONE = 202, ITYPE_GUMPMENU = 203, ITYPE_SLOTMACHINE = 204,
	ITYPE_PLAYER_VENDOR_DEED = 217, ITYPE_TREASURE_MAP = 301, ITYPE_DECIPHERED_MAP = 302,
	ITYPE_JAIL_BALL = 401, ITYPE_ITEMID_WAND = 404
};

//! Colors
enum {	COLOR_LIGHTBLUE = 0x489, COLOR_VIOLET = 0xD, COLOR_WHITE = 0x481, COLOR_BLACK = 0x1,
	COLOR_NONE = 0
}; //!< Colors

//! Human bodies
enum { BODY_MALE = 0x190, BODY_FEMALE = 0x191, BODY_DEADMALE = 0x192, BODY_DEADFEMALE = 0x193, BODY_GMSTAFF = 0x03DB };

//! Bank Type
enum { BANK_GOLD = 1, BANK_ITEM = 0, MOREY_GOLDONLYBANK = 123, MOREX_BANK = 1 };

//! Items (?)
enum {	ITEMID_DEATHSHROUD = 0x204E, ITEMID_CORPSEBASE = 0x2006, ITEMID_BANKBOX = 0x09AB,
	ITEMID_GOLD = 0x0EED
};

//! Type of hiding
enum { UNHIDDEN = 0, HIDDEN_BYSKILL, HIDDEN_BYSPELL };

//! House list type
enum { H_FRIEND = 1, H_BAN };

//! Line of sight
enum {	ITEM_TYPE_CHOICES = 6,
	TREES_BUSHES = 1,		//!< Trees and other large vegetaion in the way
	WALLS_CHIMNEYS = 2,		//!< Walls, chimineys, ovens, etc... in the way
	DOORS = 4,			//!< Doors in the way
	ROOFING_SLANTED = 8,		//!< So can't tele onto slanted roofs, basically
	FLOORS_FLAT_ROOFING = 16,	//!< So can't tele onto slanted roofs, basically
	LAVA_WATER = 32			//!< Don't know what all to use this for yet
};
//@}

//@{
	/*!
	\name Guilds related
	\author DasRaetsels'
	*/
	enum
	{
		MAXGUILDS=100,
		MAXGUILDMEMBERS=30,
		MAXGUILDRECRUITS=30,
		MAXGUILDWARS=10,
		MAXMEMRECWAR=30, //!< max of max guildmembers, max recruits and max war
	};
	#define DEFAULTCHARTER "NoX-Wizard Guildstone"
	#define DEFAULTWEBPAGE "http://www.noxwizard.com"
//@}

/*!
\brief temp effects
\author Luxor
*/
namespace tempfx {
	enum {
		SPELL_PARALYZE = 1,
		SPELL_LIGHT,
		SPELL_CLUMSY,
		SPELL_FEEBLEMIND,
		SPELL_WEAKEN,
		SPELL_AGILITY,
		SPELL_CUNNING,
		SPELL_STRENGHT,
		ALCHEMY_GRIND,
		ALCHEMY_END,
		SPELL_BLESS,
		SPELL_CURSE,
		AUTODOOR,
		TRAINDUMMY,
		SPELL_REACTARMOR,
		EXPLOTIONMSG,
		EXPLOTIONEXP,
		SPELL_POLYMORPH,
		SPELL_INCOGNITO,
		LSD,
		SPELL_PROTECTION,
		HEALING_HEAL,
		HEALING_RESURRECT,
		HEALING_CURE,
		POTION_DELAY,
		SPELL_INVISIBILITY,
		DRINK_EMOTE,
		DRINK_FINISHED,
		GM_HIDING = 33,
		GM_UNHIDING,
		HEALING_DELAYHEAL,
		COMBAT_PARALYZE	= 44,
		COMBAT_CONCUSSION = 45,
		AMXCUSTOM = 121,	//!< custom amx effects
		GREY,			//!< mark grey
		CRIMINAL,		//!< mark criminal
		FIELD_DAMAGE,		//!< magic fields damage
		SPELL_TELEKINESYS,	//!< telekinesys spell
		MAX_TEMPFX_INDEX
	};
};

/*!
\brief Types of variaibles
\author Endymion
*/
enum VarType { VAR_EFF =0, VAR_DEC, VAR_REAL, VAR_ACT, };

#endif //__extern extern constANTS_H__
