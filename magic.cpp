  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "targeting.h"
#include "magic.h"
#include "set.h"
#include "amx/amxcback.h"
#include "tmpeff.h"
#include "itemid.h"
#include "particles.h"
#include "sndpkg.h"
#include "npcai.h"

namespace magic {

static const char* g_szSpellName[] = {
	"CLUMSY", "CREATEFOOD", "FEEBLEMIND", "HEAL", "MAGICARROW", "NIGHTSIGHT", "REACTIVEARMOR", "WEAKEN",
	"AGILITY", "CUNNING", "CURE", "HARM", "MAGICTRAP", "MAGICUNTRAP", "PROTECTION", "STRENGHT",
	"BLESS", "FIREBALL", "LOCK", "POISON", "TELEKINESYS", "TELEPORT", "UNLOCK", "WALLOFSTONE",
	"ARCHCURE", "ARCHPROTECTION", "CURSE", "FIREFIELD", "GREATERHEAL", "LIGHTNING", "MANADRAIN", "RECALL",
	"BLADESPIRITS", "DISPELFIELD", "INCOGNITO", "REFLECTION", "MINDBLAST", "PARALYZE", "POISONFIELD", "SUMMON",
	"DISPEL", "ENERGYBOLT", "EXPLOSION", "INVISIBILITY", "MARK", "MASSCURSE", "PARALYZEFIELD", "REVEAL",
	"CHAINLIGHTNING", "ENERGYFIELD", "FLAMESTRIKE", "GATETRAVEL", "MANAVAMPIRE", "MASSDISPEL", "METEORSWARM", "POLYMORPH",
	"EARTHQUAKE", "ENERGYVORTEX", "RESURRECTION", "SUMMON_AIR_ELEMENTAL", "SUMMON_DEMON","SUMMON_EARTH_ELEMENTAL","SUMMON_FIRE_ELEMENTAL","SUMMON_WATER_ELEMENTAL",
};

g_Spell g_Spells[MAX_SPELLS];

///////////////////////////////////////////////////////////////////
// Function name	 : loadSpellsFromScript
// Description		 : load spells data from file
// Return type		 : static void
// Author			 : Xanathar
// Changes			 : none yet
void loadSpellsFromScript()
{
	cScpIterator* iter = NULL;
	char script1[1024], script2[1024];

	for (int curspell = 0; curspell < MAX_SPELLS; curspell++) {
		// in XSS script, numeration starts from 1 :[
		g_Spells[curspell].attackSpell = false;
		g_Spells[curspell].areasize = INVALID;
		sprintf(script1, "SECTION SPELL %s", g_szSpellName[curspell]);
		safedelete(iter);
		iter = Scripts::Spells->getNewIterator(script1);
		if (iter==NULL) continue;

		do
		{
			iter->parseLine(script1, script2);
			if (!strcmp("ENABLED", script1)) 		g_Spells[curspell].enabled = true;
			else if (!strcmp("DISABLED", script1))	g_Spells[curspell].enabled = false;
			else if (!strcmp("CIRCLE", script1)) 	g_Spells[curspell].circle = str2num(script2);
			else if (!strcmp("MANA", script1))		g_Spells[curspell].mana = str2num(script2);
			else if (!strcmp("HISKILL", script1))	g_Spells[curspell].hiskill = str2num(script2);
			else if (!strcmp("LOSKILL", script1))	g_Spells[curspell].loskill = str2num(script2);
			else if (!strcmp("SCLO", script1))		g_Spells[curspell].sclo = str2num(script2);
			else if (!strcmp("SCHI", script1))		g_Spells[curspell].schi = str2num(script2);
			else if (!strcmp("LODAMAGE", script1))	g_Spells[curspell].lodamage = str2num(script2);
			else if (!strcmp("HIDAMAGE", script1))	g_Spells[curspell].hidamage = str2num(script2);
			else if (!strcmp("DAMAGETYPE", script1)) g_Spells[curspell].damagetype = static_cast<DamageType>(str2num(script2));
			else if (!strcmp("ATTACKSPELL", script1))g_Spells[curspell].attackSpell = true;
			else if (!strcmp("ALWAYSFLAG", script1)) g_Spells[curspell].alwaysflag = str2num(script2);
			else if (!strcmp("AREASIZE", script1))	g_Spells[curspell].areasize = str2num(script2);
			else if (!strcmp("MANTRA", script1)) 	g_Spells[curspell].mantra += script2;
			else if (!strcmp("ACTION", script1)) 	g_Spells[curspell].action = hex2num(script2);
			else if (!strcmp("DELAY", script1))		g_Spells[curspell].delay = str2num(script2);
			else if (!strcmp("ASH", script1))		g_Spells[curspell].reagents.ash = str2num(script2);
			else if (!strcmp("DRAKE", script1))		g_Spells[curspell].reagents.drake = str2num(script2);
			else if (!strcmp("GARLIC", script1)) 	g_Spells[curspell].reagents.garlic = str2num(script2);
			else if (!strcmp("GINSING", script1))	g_Spells[curspell].reagents.ginseng = str2num(script2);
			else if (!strcmp("MOSS", script1))		g_Spells[curspell].reagents.moss = str2num(script2);
			else if (!strcmp("PEARL", script1))		g_Spells[curspell].reagents.pearl = str2num(script2);
			else if (!strcmp("SHADE", script1))		g_Spells[curspell].reagents.shade = str2num(script2);
			else if (!strcmp("SILK", script1))		g_Spells[curspell].reagents.silk = str2num(script2);
			else if (!strcmp("TARG", script1))		g_Spells[curspell].strToSay += script2;
			else if (!strcmp("REFLECTABLE", script1))	g_Spells[curspell].reflect = true;
			else if (!strcmp("UNREFLECTABLE", script1))	  g_Spells[curspell].reflect = false;
			else if (!strcmp("RUNIC", script1))		g_Spells[curspell].runic = str2num(script2)!=0;
		}
		while (script1[0]!='}');
	}
	safedelete(iter);
}


/*!
\author Luxor
\brief Checks for gate collisions
*/
bool checkGateCollision( P_CHAR pc )
{
	VALIDATEPCR( pc, false );

        if ( pc->npc )
		return false;

	P_ITEM pgate = NULL;

        Location charpos = pc->getPosition();
	
	NxwItemWrapper si;
	si.fillItemsNearXYZ( charpos, 1, false );
        for ( si.rewind(); !si.isEmpty(); si++ ) {
		pgate = si.getItem();
		if ( !ISVALIDPI( pgate ) )
			return false;
		
		if ( pgate->type != 51 )
			pgate = NULL;
	}

        if ( !ISVALIDPI( pgate ) )
		return false;

	Location gatepos = pgate->getPosition();
		
	if ( charpos.x != gatepos.x || charpos.y != gatepos.y || UI32(charpos.z - gatepos.z) > 2 )
		return false;

        P_CHAR pnpc = NULL;
        NxwCharWrapper sc;
        sc.fillOwnedNpcs( pc, false, true );
        for ( sc.rewind(); !sc.isEmpty(); sc++ ) {
		if ( !ISVALIDPC( (pnpc=sc.getChar()) ) )
			continue;
			
		pnpc->MoveTo( pgate->morex, pgate->morey, pgate->morez );
	}

	pc->MoveTo( pgate->morex, pgate->morey, pgate->morez );
	pc->teleport();
	pc->playSFX( 0x01FE );
	pc->staticFX( 0x372A, 0x09, 0x06 );

	return true;
}


///////////////////////////////////////////////////////////////////
// Function name	 : inline bool checkTownLimits
// Description		 : check if a spell can be casted (considering town limits)
// Return type		 : static
// Author			 : Xanathar
// Argument 		 : int spellnum -> the spell number
// Argument 		 : P_CHAR pa -> attacker, can be NULL
// Argument 		 : P_CHAR pd -> defender
// Argument 		 : int spellflags -> spell flags
// Argument 		 : int param -> spell param
// Argument 		 : bool areaspell = false -> is an area spell ?
// Changes			 : none yet
static inline bool checkTownLimits(SpellId spellnum, P_CHAR pa, P_CHAR pd, int spellflags, int param, bool areaspell = false)
{
	//VALIDATEPCR(pa, false);
	VALIDATEPCR(pd, false);

	if ((g_Spells[spellnum].attackSpell)&&(SrvParms->guardsactive)&&(region[pd->region].priv&RGNPRIV_GUARDED))
	{
		if (spellflags&SPELLFLAG_IGNORETOWNLIMITS) return false;
		if (areaspell) return false; // do *NOT* change order of these lines!! :]
		if ((pd->npc)&&(pd->npcaitype==NPCAI_EVIL)) return false;
		if (pa==pd) return false;
		if (ISVALIDPC(pa)) {
			if (Guilds->Compare(pa,pd)!=0) return false;
		}
		if (pd->IsCriminal()) return false;
		if (pd->IsGrey()) return false;
		return true;
	}
	return false;
}








///////////////////////////////////////////////////////////////////
// Function name	 : inline bool checkMana
// Description		 : checks if char has enough mana
// Return type		 : static
// Author			 : Xanathar
// Argument 		 : P_CHAR pc -> -- as default --
// Argument 		 : int num -> -- as default --
// Changes			 : none yet
static inline bool checkMana(P_CHAR pc, SpellId num)
{
	VALIDATEPCR(pc, false);

//	if( pc->IsGM() ) return true;
	if (pc->priv2&CHRPRIV2_DONTUSEUPMANA) return true;

	if (pc->mn >= g_Spells[num].mana) return true;

	pc->sysmsg(TRANSLATE("You have insufficient mana to cast that spell."));
	return false;
}


///////////////////////////////////////////////////////////////////
// Function name	 : inline void subtractMana
// Description		 : subtracts mana from char
// Return type		 : static
// Author			 : Xanathar
// Argument 		 : P_CHAR pc -> -- as default --
// Argument 		 : int mana -> -- as default --
// Changes			 : none yet
static inline void subtractMana(P_CHAR pc, SpellId spellnumber)
{
	VALIDATEPC(pc);

	if (pc->priv2&CHRPRIV2_DONTUSEUPMANA) return;

	if (pc->mn >= g_Spells[spellnumber].mana) pc->mn -= g_Spells[spellnumber].mana;
	else pc->mn = 0;

	pc->updateStats(STAT_MANA);//AntiChrist - bugfix
}



///////////////////////////////////////////////////////////////////
// Function name	 : checkReflection
// Description		 : checks (recursively) magic reflection and
//					   eventually changes attacker/defender roles
// Return type		 : bool (true if roles changed)
// Author			 : Xanathar
// Argument 		 : P_CHAR pa -> attacker
// Argument 		 : P_CHAR pd -> defender
// Changes			 : none yet
static bool checkReflection(P_CHAR &pa, P_CHAR &pd)
{
	VALIDATEPCR(pa, false);
	VALIDATEPCR(pd, false);
	if (pd->priv2 & CHRPRIV2_REFLECTION) {
		pd->priv2 &= ~CHRPRIV2_REFLECTION;
		pd->staticFX(0x373A, 0, 15);
		qswap(pa, pd);
		return !checkReflection(pa, pd);
	}
	return false;
}


///////////////////////////////////////////////////////////////////
// Function name	 : bool isFieldSpell, isBoxSpell, isAreaSpell
// Description		 : checks for spell size
// Author			 : Xanathar
static inline bool isFieldSpell(SpellId spell)
{
	switch (spell) {
		case SPELL_FIREFIELD:
		case SPELL_POISONFIELD:
		case SPELL_PARALYZEFIELD:
		case SPELL_ENERGYFIELD:
		case SPELL_WALLSTONE:
			return true;
		default:
			return false;
	}
}

static inline bool isBoxSpell(SpellId spell)
{
	return (g_Spells[spell].areasize==0);
}

static inline bool isAreaSpell(SpellId spell)
{
	return (g_Spells[spell].areasize>0);
}

///////////////////////////////////////////////////////////////////
// Function name	 : checkResist
// Description		 : check if defender can resist a spell
// Return type		 : bool
// Author			 : Xanathar
// Argument 		 : P_CHAR pa -> attacker
// Argument 		 : P_CHAR pd -> defender
// Argument 		 : int spellnumber -> spell
// Changes			 : none yet
static bool checkResist(P_CHAR pa, P_CHAR pd, SpellId spellnumber)
{	// This function uses informations found at http://uo.stratics.com !

	VALIDATEPCR(pd, false);
	int circle = (spellnumber) / 8;
	// just to give skill a chance to raise :)
	pd->checkSkill( MAGICRESISTANCE, 80*circle, 1000, !isFieldSpell(spellnumber));

	int resist = pd->skill[MAGICRESISTANCE] / 10;
	int resistchance = resist / 5;

	if (ISVALIDPC(pa)) {
		int magery = pa->skill[MAGERY]/10;
		int secondresist = (resist) - ( (magery/2) + (circle*5));
		resistchance = qmax(resistchance, secondresist);
	}

	if (chance(resistchance)) {
		pd->sysmsg("You feel yourself resisting magical energy!");
		return true;
	} else return false;
}




///////////////////////////////////////////////////////////////////
// Function name	 : spellFX
// Description		 : plays all kind of spell effects
// NOTE 			 : DOES *NOT* play some sfxs for some area spells
// Return type		 : void
// Author			 : Xanathar
// Argument 		 : int spellnum -> -- as default --
// Argument 		 : P_CHAR pcaster -> caster, NULL if none
// Argument 		 : P_CHAR pctarget = NULL -> char targetted, NULL otherwise
// Argument 		 : P_ITEM pitarget = NULL -> item targetted, NULL otherwise
// Changes			 : Luxor -- Added the effects :D
static void spellFX(SpellId spellnum, P_CHAR pcaster = NULL, P_CHAR pctarget = NULL, P_ITEM pitarget = NULL )
{
	P_CHAR pcfrom = pcaster;
	P_CHAR pcto = pctarget;
	if ((!(ISVALIDPC(pcfrom))) && (!(ISVALIDPC(pcto)))) return;
	if ((!(ISVALIDPC(pcfrom)))&&(ISVALIDPC(pcto))) pcfrom = pcto;
	if ((ISVALIDPC(pcfrom))&&(!(ISVALIDPC(pcto)))) pcto = pcfrom;

	ParticleFx spfx, mpfx/*, postfx*/;

	mpfx.initWithSpellMoveEffect(spellnum);
	spfx.initWithSpellStatEffect(spellnum);

	switch(spellnum) {
		case SPELL_FIREBALL:
			pcfrom->movingFX(pcto, 0x36D5, 5, 0, true, &mpfx);
			pcfrom->playSFX(0x15E);
			break;
		case SPELL_CLUMSY:
		case SPELL_FEEBLEMIND:
		case SPELL_WEAKEN:
		case SPELL_DISPEL:
		case SPELL_CURSE:
		case SPELL_POISON:
			pcto->playSFX(0x1DF);
			pcto->staticFX(0x374A, 0, 10, &spfx);
			break;
		case SPELL_PARALYZE:
			pcto->playSFX(0x204);
			pcto->staticFX(0x374A, 0, 10, &spfx);
			break;
		case SPELL_CUNNING:
		case SPELL_AGILITY:
		case SPELL_STRENGHT:
		case SPELL_BLESS:
		case SPELL_REFLECTION:
			pcto->playSFX(0x1DF);
			pcto->staticFX(0x373A, 0, 10, &spfx);
			break;
		case SPELL_GREATHEAL:
			pcto->staticFX(0x376A, 0, 10, &spfx);
			pcto->playSFX(0x202);
			break;
		case SPELL_HEAL:
			pcto->staticFX(0x376A, 0, 10, &spfx);
			pcto->playSFX(0x1F2);
			break;
		case SPELL_CURE:
		case SPELL_ARCHCURE:
			pcto->playSFX(0x1DF);
			pcto->staticFX(0x376A, 0, 10, &spfx);
			break;
		case SPELL_RESURRECTION:
			pcto->playSFX(0x214);
			break;
		case SPELL_MAGICARROW:
			pcfrom->movingFX(pcto, 0x36E4, 5, 0, true, &mpfx);
			pcfrom->playSFX(0x1E5);
			break;
		case SPELL_FLAMESTRIKE:
			pcto->staticFX(0x3709, 0, 10, &spfx);
			pcto->playSFX(0x208);
			break;
		case SPELL_EXPLOSION:
			pcto->staticFX(0x36B0, 0, 10, &spfx);
			pcto->playSFX(0x207);
			break;
		case SPELL_LIGHTNING:
			pcto->boltFX(false);
			pcto->playSFX(0x29);
			break;
		case SPELL_ENERGYBOLT:
			pcfrom->movingFX(pcto, 0x379F, 5, 0, true, &mpfx);
			pcfrom->playSFX(0x20A);
			break;
		case SPELL_HARM:
		case SPELL_MINDBLAST:
		case SPELL_MANADRAIN:
		case SPELL_MANAVAMPIRE:
			pcto->staticFX(0x374A, 0, 10, &spfx);
			pcto->playSFX(0x1F1);
			break;

		case SPELL_REACTIVEARMOUR:
			pcfrom->staticFX(0x373A, 0, 10, &spfx);
			break;
		case SPELL_PROTECTION:
		case SPELL_ARCHPROTECTION:
			pcfrom->staticFX(0x376A, 0, 10, &spfx);
			pcfrom->playSFX(0x1ED);
			break;
		case SPELL_NIGHTSIGHT:
			pcfrom->playSFX(0x1E3);
			break;
		case SPELL_INCOGNITO:
		case SPELL_INVISIBILITY:
			pcfrom->playSFX(0x203);
			break;
		case SPELL_SUMMON:
			pcfrom->staticFX(0x3735, 0, 10, &spfx);
			break;
		case SPELL_EARTHQUAKE:
			break;
		case SPELL_CREATEFOOD:
			pcfrom->playSFX(0x1E2);
			break;
		case SPELL_POLYMORPH:
			break;

		case SPELL_TELEKINESYS:
			break;

		//Luxor: Lock/Unlock & Trap/Untrap effects are done by applyspell function
		case SPELL_TRAP:
		case SPELL_UNTRAP:
		case SPELL_LOCK:
		case SPELL_UNLOCK:
			return ;
		//----------------------------------

		case SPELL_GATE:
			pcfrom->playSFX(0x20E);
			break;
		case SPELL_MARK:
			pcfrom->playSFX(0x1FA);
			break;
		case SPELL_RECALL:
			pcfrom->staticFX(0x374A, 0, 10, &spfx);
			pcfrom->playSFX(0x1FC);
			break;

		case SPELL_BLADESPIRITS:
		case SPELL_SUMMON_AIR:
		case SPELL_SUMMON_DEAMON:
		case SPELL_SUMMON_EARTH:
		case SPELL_SUMMON_FIRE:
		case SPELL_SUMMON_WATER:
		case SPELL_ENERGYVORTEX:
			pcfrom->staticFX(0x372A, 0, 10, &spfx);
			pcfrom->playSFX(0x212);
			break;
		case SPELL_WALLSTONE:
			pcfrom->playSFX(0x1F6);
			break;
		case SPELL_TELEPORT:
			pcfrom->staticFX(0x372A, 0, 10, &spfx);
			pcfrom->playSFX(0x1FE);
			break;
		case SPELL_MASSDISPEL:
			pcfrom->playSFX(0x209);
			break;
		case SPELL_MASSCURSE:
			pcfrom->playSFX(0x1FB);
			break;
		case SPELL_REVEAL:
			pcfrom->playSFX(0x1FD);
			break;
		case SPELL_FIREFIELD:
		case SPELL_DISPELFIELD:
		case SPELL_POISONFIELD:
		case SPELL_PARALYZEFIELD:
		case SPELL_ENERGYFIELD:
			pcfrom->playSFX(0x20C);
			break;
		case SPELL_CHAINLIGHTNING:
			pcto->boltFX(false);
			pcto->playSFX(0x29);
			break;
		case SPELL_METEORSWARM:
			pcto->staticFX(0x372A, 0, 10, &spfx);
			pcto->playSFX(0x160);
			return ;

		default:
			break;
	}

}



///////////////////////////////////////////////////////////////////
// Function name	 : damage
// Description		 : inflicts magic damage from a spell
// Return type		 : void
// Author			 : Xanathar
// Argument 		 : P_CHAR pa -> attacker
// Argument 		 : P_CHAR pd -> defender
// Argument 		 : int spellnum -> spell number
// Argument 		 : int spellflags = 0 -> spell flags
// Argument 		 : int param = 0 -> optional parameter for some spellflags
// Changes			 : none yet
static void damage(P_CHAR pa, P_CHAR pd, SpellId spellnum, int spellflags = 0, int param = 0)
{
	VALIDATEPC(pd);

	P_CHAR p_realattacker = pa;
	P_CHAR p_realdefender = pd;

	// check resistances :)
	if ((ISVALIDPC(pa))&&(!(spellflags&SPELLFLAG_DONTREFLECT)))
		if (g_Spells[spellnum].reflect)
			checkReflection(pa, pd);

	// early return for invulz
	if (pd->IsInvul()) return;

	// early return if in town and target should not be damaged
	if (checkTownLimits(spellnum, pa, pd, spellflags, param)) return;

	// check resistance for damage resistance
	bool bResists = checkResist(pa, pd, spellnum);
	if (spellflags&SPELLFLAG_IGNORERESISTANCE) bResists = false;

	// calculate basic spell damage
	double damage = static_cast<double>(RandomNum(g_Spells[spellnum].lodamage, g_Spells[spellnum].hidamage));
	if (spellflags&SPELLFLAG_PARAMISDAMAGE) damage = static_cast<double>(param);

	// calculates evint/resist modifier
	double resist = static_cast<double>(pd->skill[MAGICRESISTANCE]) / 10.0;
	double evint = resist; //no bonus/malus if no attacker

	if (ISVALIDPC(pa)) {
		evint = static_cast<double>(pd->skill[EVALUATINGINTEL]) / 10.0;
	}

	if (bResists) damage/=2.0;

	double mod;

	if (resist > evint) {
		mod = 1.0 + (evint - resist) / 200.0;
	} else if (resist == evint) {
		mod = 1.0 + (evint - resist) / 300.0;
	} else {
		mod = 1.0 + (evint - resist) / 500.0;
	}

	damage *= mod;
	int amount = static_cast<int>(damage);

	// now we have the damage to be done :]

	if (pd->npc) amount *= 4;		   // double damage against non-players

	if (ISVALIDPC(p_realattacker)) p_realattacker->attackStuff(p_realdefender);

	StatType stattodamage = STAT_HP;
	if ((spellnum==SPELL_MANADRAIN)||(spellnum==SPELL_MANAVAMPIRE)) stattodamage = STAT_MANA;
	pd->damage(amount, g_Spells[spellnum].damagetype, stattodamage);
}





///////////////////////////////////////////////////////////////////
// Function name	 : checkReagents
// Description		 : check reagents for presence
// Return type		 : bool
// Author			 : Xanathar
// Argument 		 : P_CHAR pc -> -- as default --
// Argument 		 : reag_st reagents -> -- as default --
// Changes			 : none yet
bool checkReagents(P_CHAR pc, reag_st reagents)
{
	VALIDATEPCR(pc, false);
	reag_st fail;

//	if( pc->IsGM() ) return true;

	if (pc->priv2&CHRPRIV2_DONTUSEREAGENTS) return true;
	if (pc->npc) return true;

	fail.ash=fail.drake=fail.garlic=fail.moss=fail.pearl=fail.shade=fail.silk = 0;

	if (reagents.ash!=0 && pc->CountItems(0x0F8C)<reagents.ash) fail.ash=1;
	if (reagents.drake!=0 && pc->CountItems(0x0F86)<reagents.drake) fail.drake=1;
	if (reagents.garlic!=0 && pc->CountItems(0x0F84)<reagents.garlic) fail.garlic=1;
	if (reagents.ginseng!=0 && pc->CountItems(0x0F85)<reagents.ginseng) fail.ginseng=1;
	if (reagents.moss!=0 && pc->CountItems(0x0F7B)<reagents.moss) fail.moss=1;
	if (reagents.pearl!=0 && pc->CountItems(0x0F7A)<reagents.pearl) fail.pearl=1;
	if (reagents.shade!=0 && pc->CountItems(0x0F88)<reagents.shade) fail.shade=1;
	if (reagents.silk!=0 && pc->CountItems(0x0F8D)<reagents.silk) fail.silk=1;

	string str(TRANSLATE("You do not have enough reagents to cast that spell.[ "));

	if (fail.ash)	  str += "Sa ";
	if (fail.drake)   str += "Mr ";
	if (fail.garlic)  str += "Ga ";
	if (fail.ginseng) str += "Gi ";
	if (fail.moss)	  str += "Bm ";
	if (fail.pearl)   str += "Bp ";
	if (fail.shade)   str += "Ns ";
	if (fail.silk)	  str += "Ss ";
	str += "]";

	int failure = fail.ash+fail.drake+fail.garlic+fail.moss+fail.pearl+fail.shade+fail.silk;

	if (failure!=0) {
		pc->sysmsg(const_cast<char*>(str.c_str()));
		return false;
	}

	return true;
}



///////////////////////////////////////////////////////////////////
// Function name	 : spellFailFX
// Description		 : plays failure effects
// Return type		 : void
// Author			 : Xanathar
// Argument 		 : P_CHAR pc -> wannabe caster
// Changes			 : none yet
void spellFailFX(P_CHAR pc)
{
	VALIDATEPC(pc);
	if ((pc->spell < 0)||( pc->spell>89)) return;

	pc->staticFX(0x3735, 0, 30);
	pc->playSFX(0x005C);
	pc->emote(pc->getSocket(), TRANSLATE("The spell fizzles."),1);
}



///////////////////////////////////////////////////////////////////
// Function name	 : castAreaAttackSpell
// Description		 : casts an area attack spell like eartquake or chain lightning
// Return type		 : void
// Author			 : Xanathar
// Argument 		 : int x -> x coord of epicenter
// Argument 		 : int y -> y coord of epicenter
// Argument 		 : int spellnum -> spell number
// Argument 		 : P_CHAR pcaster -> caster (can be NULL)
// Changes			 : none yet
void castAreaAttackSpell (int x, int y, SpellId spellnum, P_CHAR pcaster)
{
	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( x, y, VISRANGE -2 );

	int damagetobedone = RandomNum(g_Spells[spellnum].lodamage, g_Spells[spellnum].hidamage);
	int divider = (sc.size() / 4) + 1;
	if (divider!=0) damagetobedone /= divider;

	if (ISVALIDPC(pcaster)) {
		if (checkTownLimits(spellnum, pcaster, pcaster, 0, 0, true)) return;
	}

	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pd = sc.getChar();
		if (ISVALIDPC(pd)) {
			spellFX(spellnum, pcaster, pd);
			damage(pcaster, pd, spellnum, SPELLFLAG_PARAMISDAMAGE, damagetobedone);
		}
	}
}


























///////////////////////////////////////////////////////////////////
// Function name	 : spellTargetType
// Description		 : returns target type
// Return type		 : int
// Author			 : Xanathar
// Argument 		 : int spellnum -> -- as default --
// Changes			 : none yet
static inline int spellTargetType(SpellId spellnum)
{
	switch(spellnum) {
		case SPELL_FIREBALL:
		case SPELL_CLUMSY:
		case SPELL_FEEBLEMIND:
		case SPELL_WEAKEN:
		case SPELL_PARALYZE:
		case SPELL_DISPEL:
		case SPELL_CURSE:
		case SPELL_POISON:
		case SPELL_CUNNING:
		case SPELL_AGILITY:
		case SPELL_STRENGHT:
		case SPELL_BLESS:
		case SPELL_HEAL:
		case SPELL_GREATHEAL:
		case SPELL_CURE:
		case SPELL_ARCHCURE:
		case SPELL_RESURRECTION:
		case SPELL_MAGICARROW:
		case SPELL_FLAMESTRIKE:
		case SPELL_EXPLOSION:
		case SPELL_LIGHTNING:
		case SPELL_ENERGYBOLT:
		case SPELL_HARM:
		case SPELL_MINDBLAST:
		case SPELL_MANADRAIN:
		case SPELL_MANAVAMPIRE:
			return TARGTYPE_CHAR;

		case SPELL_REACTIVEARMOUR:
		case SPELL_PROTECTION:
		case SPELL_ARCHPROTECTION:
		case SPELL_INCOGNITO:
		case SPELL_REFLECTION:
		case SPELL_NIGHTSIGHT:
		case SPELL_INVISIBILITY:
		case SPELL_SUMMON:
		case SPELL_SUMMON_AIR:
		case SPELL_SUMMON_DEAMON:
		case SPELL_SUMMON_EARTH:
		case SPELL_SUMMON_FIRE:
		case SPELL_SUMMON_WATER:
		case SPELL_EARTHQUAKE:
		case SPELL_CREATEFOOD:
		case SPELL_POLYMORPH:
			return TARGTYPE_NONE;

		case SPELL_TELEKINESYS:
			return TARGTYPE_ITEM;

		case SPELL_TRAP:
		case SPELL_UNTRAP:
			return TARGTYPE_CONTAINER;

		case SPELL_LOCK:
		case SPELL_UNLOCK:
			return TARGTYPE_CONTAINERORDOOR;

		case SPELL_GATE:
		case SPELL_MARK:
		case SPELL_RECALL:
			return TARGTYPE_RUNE;

		case SPELL_BLADESPIRITS:
		case SPELL_ENERGYVORTEX:
		case SPELL_WALLSTONE:
		case SPELL_TELEPORT:
		case SPELL_MASSDISPEL:
		case SPELL_MASSCURSE:
		case SPELL_REVEAL:
		case SPELL_FIREFIELD:
		case SPELL_DISPELFIELD:
		case SPELL_POISONFIELD:
		case SPELL_PARALYZEFIELD:
		case SPELL_ENERGYFIELD:
		case SPELL_CHAINLIGHTNING:
		case SPELL_METEORSWARM:
			return TARGTYPE_XYZ;

		default:
			return TARGTYPE_NONE;
	}
}






///////////////////////////////////////////////////////////////////
// Function name	 : spellRequiresTarget
// Description		 : returns if spell requires a target
// Return type		 : bool
// Author			 : Xanathar
// Argument 		 : int spellnum -> -- as default --
// Changes			 : none yet
bool spellRequiresTarget(SpellId spellnum)
{
	return (spellTargetType(spellnum)!=TARGTYPE_NONE);
}



///////////////////////////////////////////////////////////////////
// Function name	 : checkDistance
// Description		 : check if the target is too far from the caster
// Return type		 : bool
// Author			 : Luxor
// Argument 		 : P_CHAR caster -> the caster
// Argument 		 : P_CHAR target -> the target
// Changes			 : none yet
static bool checkDistance(P_CHAR caster, P_CHAR target)
{
	VALIDATEPCR(caster, false);
	VALIDATEPCR(target, false);
	if (caster->distFrom(target) > 15) {
		caster->sysmsg(TRANSLATE("You are too far away from the target."));
		return false;
	}
	if ( target->IsHidden() ) {
		caster->sysmsg(TRANSLATE("You cannot see your target."));
		return false;
	}
	return true;
}


///////////////////////////////////////////////////////////////////
// Function name	 : checkLos
// Description		 : check if there's something between the caster and the target
// Return type		 : bool
// Author                : Luxor
// Changes		 : none yet
static bool checkLos(P_CHAR caster, Location destpos)
{
	VALIDATEPCR(caster, false);
        if (!line_of_sight(INVALID, caster->getPosition(), destpos, INVALID)) {
		caster->sysmsg(TRANSLATE("There is something between you and your target that makes the casting impossible."));
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////
// Function name	 : checkRequiredTargetType
// Description		 : check if target is correct
// Return type		 : bool
// Author			 : Xanathar & Luxor
// Argument 		 : int spellnum -> spell number
// Argument 		 : TargetLocation& t -> target location
// Changes			 : none yet
bool checkRequiredTargetType(SpellId spellnum, TargetLocation& t)
{
			// 0:none,1:xyz,2:item,3:char,4:container or door,6:rune,5:container
	P_ITEM pi = t.getItem();
	int x,y,z;
	t.getXYZ(x,y,z);

	switch(spellTargetType(spellnum)) {
		case TARGTYPE_NONE :
			return true;
		case TARGTYPE_CONTAINER:
			if (!(ISVALIDPI(pi))) return false;
			return pi->isContainer();
		case TARGTYPE_CONTAINERORDOOR:
			if (!(ISVALIDPI(pi))) return false;
			return( (pi->isContainer() || ( pi->type == ITYPE_DOOR ) || ( pi->type == ITYPE_LOCKED_DOOR ) ) );
		case TARGTYPE_XYZ :
			return ((x>0)&&(y>0));
		case TARGTYPE_CHAR:
			return (t.getChar()!=NULL);
		case TARGTYPE_ITEM:
			return (pi!=NULL);
		case TARGTYPE_RUNE:
			if (!(ISVALIDPI(pi))) return false;
			return (pi->type == ITYPE_RUNE);
		default:
			return (ISVALIDPI(pi)); // needz to be changed
	}
}




///////////////////////////////////////////////////////////////////
// Function name	 : consumeReagents
// Description		 : delete reagents
// Return type		 : void
// Author			 : Xanathar
// Argument 		 :	P_CHAR pc -> -- as default --
// Argument 		 : reag_st reags -> -- as default --
// Changes			 : none yet
void consumeReagents( P_CHAR pc, reag_st reags )
{
	VALIDATEPC(pc);
	if (pc->priv2&CHRPRIV2_DONTUSEREAGENTS) return;
	pc->delItems(0x0F7A, reags.pearl);
	pc->delItems(0x0F7B, reags.moss);
	pc->delItems(0x0F84, reags.garlic);
	pc->delItems(0x0F85, reags.ginseng);
	pc->delItems(0x0F86, reags.drake);
	pc->delItems(0x0F88, reags.shade);
	pc->delItems(0x0F8C, reags.ash);
	pc->delItems(0x0F8D, reags.silk);
}



///////////////////////////////////////////////////////////////////
// Function name	 : spellNumberFromScrollId
// Description		 : get spell number from scroll item id
// Return type		 : int
// Author			 : Xanathar
// Argument 		 : int id -> item id of a scroll
// Changes			 : none yet
SpellId spellNumberFromScrollId(int id)
{
	if (id==0x1F2D) 				return SPELL_REACTIVEARMOUR;		   // Reactive Armor
	if (id>=0x1F2E && id<=0x1F33)	return static_cast<SpellId>(id-0x1F2D-1); // first circle without weaken
	if (id>=0x1F34 && id<=0x1F6C)	return static_cast<SpellId>(id-0x1F2D);    // 2 to 8 circle spell scrolls plus weaken
	return SPELL_INVALID;
}




///////////////////////////////////////////////////////////////////
// Function name	 : castStatPumper
// Description		 : real casting function for stat pumping spells
// Return type		 : void
// Author			 : Xanathar
// Changes			 : none yet
static void castStatPumper(SpellId spellnumber, TargetLocation& dest, P_CHAR pa, int flags, int param)
{
	int bonus = 10; //default
	P_CHAR pd = NULL;
	int duration = 60; // one minute default

	if (ISVALIDPC(pa)) {
		int skilltouse = MAGERY;
		if (flags&SPELLFLAG_PARAMISSKILLTOUSE) skilltouse = param;
		bonus = pa->skill[skilltouse] / 50;
		duration = pa->skill[skilltouse] / 10;
	}

	if (flags&SPELLFLAG_PARAMISBONUS) bonus = param;
	if (flags&SPELLFLAG_PARAMISDURATION) duration = param;

	if ((pd = dest.getChar())==NULL) return;

	P_CHAR p_realAttacker = pa;
	P_CHAR p_realDefender = pd;


	if ((pa!=NULL)&&(!(flags&SPELLFLAG_DONTREFLECT)))
		if (g_Spells[spellnumber].reflect) checkReflection(pa, pd);

	if (pd == NULL) return; //paranoia >:]

	// early return for invulz : no bonus, no malus
	if ((g_Spells[spellnumber].attackSpell)&&(pd->IsInvul())) return;

	// check resistance for damage resistance
	bool bResists = checkResist(pa, pd, spellnumber);
	if (flags&SPELLFLAG_IGNORERESISTANCE) bResists = false;

	// early return if in town and target should not be damaged
	if (checkTownLimits(spellnumber, pa, pd, flags, param)) return;

	if ((g_Spells[spellnumber].attackSpell)&&(pa!=NULL)&&(!(flags&SPELLFLAG_DONTCRIMINAL)))
		p_realAttacker->attackStuff(p_realDefender);

	if ((!g_Spells[spellnumber].attackSpell)&&(pa!=NULL)&&(!(flags&SPELLFLAG_DONTCRIMINAL)))
		p_realAttacker->helpStuff(p_realDefender);

	if ((g_Spells[spellnumber].attackSpell)&&(bResists)) return;

	switch (spellnumber)
	{
		case SPELL_CLUMSY:
			tempfx::add(pa, pd, tempfx::SPELL_CLUMSY, bonus, 0, 0, duration);
			break;
		case SPELL_FEEBLEMIND:
			tempfx::add(pa, pd, tempfx::SPELL_FEEBLEMIND, bonus, 0, 0, duration);
			break;
		case SPELL_WEAKEN:
			tempfx::add(pa, pd, tempfx::SPELL_WEAKEN, bonus, 0, 0, duration);
			break;
		case SPELL_CURSE:
			tempfx::add(pa, pd, tempfx::SPELL_CURSE, bonus, bonus, bonus, duration);
			break;
		case SPELL_CUNNING:
			tempfx::add(pa, pd, tempfx::SPELL_CUNNING, bonus, 0, 0, duration);
			break;
		case SPELL_AGILITY:
			tempfx::add(pa, pd, tempfx::SPELL_AGILITY, bonus, 0, 0, duration);
			break;
		case SPELL_STRENGHT:
			tempfx::add(pa, pd, tempfx::SPELL_STRENGHT, bonus, 0, 0, duration);
			break;
		case SPELL_BLESS:
			tempfx::add(pa, pd, tempfx::SPELL_BLESS, bonus, bonus, bonus, duration);
			break;
		default :
			ErrOut("Switch fallout. magic::castStatPumper\n");
	}
}


///////////////////////////////////////////////////////////////////
// Function name	 : summon
// Description		 : summons an npc for the caster :]
// Return type		 : P_CHAR
// Author			 : Xanathar & Luxor
// Changes			 : Luxor -> added code for uncontrollable npcs. added code for xyz target.
P_CHAR summon (P_CHAR owner, int npctype, int duration, bool bTamed, int x, int y, int z)
{
	VALIDATEPCR(owner, NULL);
	if (x == INVALID || y == INVALID || z == INVALID)
	{
		Location charpos= owner->getPosition();
		x = charpos.x;
		y = charpos.y;
		z = charpos.z;
	}
	P_CHAR pc = npcs::addNpc(npctype, x, y, z);
	VALIDATEPCR(pc, NULL);
	if (bTamed) {
		pc->setOwner(owner);
		pc->tamed = true;
	} else {
		pc->npcaitype = NPCAI_MADNESS; //Blade spirit, E-Vortex
	}
	pc->summontimer = uiCurrentTime + duration * MY_CLOCKS_PER_SEC;
	return pc;
}









void castFieldSpell( P_CHAR pc, int x, int y, int z, int spellnumber)
{
	VALIDATEPC(pc);
	int /*snr,*/ j = 0, fieldLen = 4/*, i*/;
	int fx[5], fy[5]; // bugfix LB, was fx[4] ...
	short id;

	if (pc!=NULL) j=fielddir(DEREF_P_CHAR(pc), x, y, z); // lb bugfix, socket char# confusion

	if (j)
	{	fx[0]=fx[1]=fx[2]=fx[3]=fx[4]=x; fy[0]=y; fy[1]=y+1; fy[2]=y-1; fy[3]=y+2; fy[4]=y-2;}
	else
	{	fy[0]=fy[1]=fy[2]=fy[3]=fy[4]=y; fx[0]=x; fx[1]=x+1; fx[2]=x-1; fx[3]=x+2; fx[4]=x-2;}	// end else


	switch(spellnumber)
	{
		case SPELL_WALLSTONE:
			id = 0x0080; fieldLen = 2; break;
		case SPELL_FIREFIELD:
			id = (j) ? 0x3996 : 0x398c; break;
		case SPELL_POISONFIELD:
			id = (j) ? 0x3920 : 0x3915; break;
		case SPELL_PARALYZEFIELD:
			id = (j) ? 0x3979 : 0x3967; break;
		case SPELL_ENERGYFIELD:
			id = (j) ? 0x3956 : 0x3946; break;
		default:
			ErrOut("Switch fallout in magic::castFieldSpell\n");
			return;
	}

	int R;
	if (pc!=NULL) R = pc->region;

	if ((R<=INVALID)||(R>255)) R = 255;


	if ((region[R].priv&RGNPRIV_GUARDED)&&(SrvParms->guardsactive)) return;

	for( j=0; j<=fieldLen; j++ )
	{
		SI08 nz=getHeight( Loc( fx[j], fy[j], z ) );
		P_ITEM pi = item::addByID(id, 1, "#", 0, fx[j], fy[j], nz);

		if (pi!=NULL)
		{
			pi->setDecay();
			pi->setDispellable();
			pi->setDecayTime( uiCurrentTime+((pc->skill[MAGERY]/15)*MY_CLOCKS_PER_SEC) );
			pi->morex=pc->skill[MAGERY]; // remember casters magery skill for damage, LB
			pi->dir=29;
			pi->magic=2;
			pi->Refresh();
		}
	}
}



///////////////////////////////////////////////////////////////////
// Function name	 : void applySpell
// Description		 : the function which really does spell effects :]
// Return type		 : static
// Author			 : Xanathar & Luxor
// Changes			 : none yet.
#define CHECKDISTANCE(A,B) if(!checkDistance(A,B) || !A->losFrom(B)) return; //Luxor

static void applySpell(SpellId spellnumber, TargetLocation& dest, P_CHAR src, int flags, int param)
{
	VALIDATEPC(src);

	P_CHAR pd = dest.getChar();
	P_ITEM pi = dest.getItem();
	int x,y,z;
	dest.getXYZ(x,y,z);

        Location srcpos= src->getPosition();

	//
        //Luxor:	Line of sight check
	//
	//Sparhawk:	Don't check los for spells on items in containers
	//
	if( ( ( pi != 0 ) && pi->isInWorld() ) || pd != 0 )
		if (!checkLos(src, Loc(x,y,z)))
			return;

	int nSkill = MAGERY;
	int nValue = INVALID;
	int nTime = INVALID;

	if (flags&SPELLFLAG_PARAMISSKILLTOUSE) nSkill = param;
	if (flags&SPELLFLAG_PARAMISDAMAGE) nValue = param;
	if (flags&SPELLFLAG_PARAMISDURATION) nTime = param;



	switch (spellnumber)
	{
		case SPELL_CLUMSY:
		case SPELL_FEEBLEMIND:
		case SPELL_WEAKEN:
		case SPELL_CUNNING:
		case SPELL_AGILITY:
		case SPELL_STRENGHT:
		case SPELL_BLESS:
		case SPELL_CURSE:
			if (pd!=NULL) {
				CHECKDISTANCE(src, pd);
				spellFX(spellnumber, src, pd);
				castStatPumper(spellnumber, dest, src, flags, param);
			}
			break;

		case SPELL_PARALYZE:
			if (pd != NULL) {
				CHECKDISTANCE(src, pd);
				spellFX(spellnumber, src, pd);
				tempfx::add(src, pd, tempfx::SPELL_PARALYZE);
			}
			break;
		case SPELL_POISON:
			if (pd != NULL) {
				CHECKDISTANCE(src, pd);
				spellFX(spellnumber, src, pd);
				if ( src->skill[MAGERY] < 700 )
					pd->applyPoison(POISON_WEAK);
				else if ( src->skill[MAGERY] < 900 )
					pd->applyPoison(POISON_NORMAL);
				else
					pd->applyPoison(POISON_GREATER);
			}
			break;
		case SPELL_MAGICARROW:
		case SPELL_FIREBALL:
		case SPELL_FLAMESTRIKE:
		case SPELL_LIGHTNING:
		case SPELL_EXPLOSION:
		case SPELL_ENERGYBOLT:
		case SPELL_HARM:
		case SPELL_METEORSWARM:
		case SPELL_CHAINLIGHTNING:
		case SPELL_EARTHQUAKE:
			if (pd!=NULL) {
				if (g_Spells[spellnumber].areasize<=0)
				{
					CHECKDISTANCE(src, pd);
					spellFX(spellnumber, src, pd);
					damage(src, pd, spellnumber, flags, param);
				} 
				else
				{
					if (spellnumber == SPELL_EARTHQUAKE)
					{
						x = srcpos.x;
						y = srcpos.y;
					}
					castAreaAttackSpell(x, y, spellnumber, src);
				}
			}
			break;

		case SPELL_MINDBLAST:
			if ( pd != 0 ) {
				CHECKDISTANCE(src, pd);
				spellFX(spellnumber, src, pd);
				if ((src->in2+10)>(pd->in2)) damage(src, pd, spellnumber, flags, param);
				else {
					damage(pd, src, spellnumber, flags|SPELLFLAG_PARAMISDAMAGE, param);
					src->attackStuff(pd);
				}
			}
			break;

		case SPELL_MANADRAIN:
			if ( pd != 0 ) {
				CHECKDISTANCE(src, pd);
				spellFX(spellnumber, src, pd);
				damage(src, pd, spellnumber, flags, param);
			}
			break;

		case SPELL_MANAVAMPIRE:
			if ( pd != 0 ) {
				CHECKDISTANCE(src, pd);
				spellFX(spellnumber, src, pd);
				int manabogus = pd->mn;
				damage(src, pd, spellnumber, flags, param);
				if (src!=NULL) {
					manabogus -= pd->mn;
					src->mn+=manabogus/2;
					if (src->mn > src->in) src->mn = src->in;
					src->updateStats(STAT_MANA);
				}
			}
			break;
		case SPELL_LOCK:
			if (pi!=NULL) {
				if(pi->isContainer() && !pi->isSecureContainer()) {
				switch(pi->type)
				{
				case ITYPE_CONTAINER: pi->type=ITYPE_LOCKED_ITEM_SPAWNER; break;
				case ITYPE_UNLOCKED_CONTAINER: pi->type=ITYPE_LOCKED_CONTAINER; break;
				}
				if (src!=NULL) {
				src->playSFX(0x0200);
				src->sysmsg(TRANSLATE("It's locked!"));
				}
			}
			else if (src!=NULL) src->sysmsg(TRANSLATE("You cannot lock this!!!"));
			}
			break;
		case SPELL_UNLOCK:
			if ( ( pi != 0 )&&(pi->more1==0)&&(pi->more2==0)&&(pi->more3==0)&&(pi->more4==0)) {
				if(pi->isSecureContainer()) {
					switch(pi->type)
					{
					case ITYPE_LOCKED_ITEM_SPAWNER: pi->type=ITYPE_CONTAINER; break;
					case ITYPE_LOCKED_CONTAINER: pi->type=ITYPE_UNLOCKED_CONTAINER; break;
					}
					if (src!=NULL) {
					src->playSFX(0x01FF);
					src->sysmsg(TRANSLATE("You unlocked it!"));
					}
				}
				else src->sysmsg(TRANSLATE("You cannot unlock this!!!"));
			} else src->sysmsg(TRANSLATE("You cannot unlock this!!!"));
			break;




		case SPELL_TRAP:
        if (ISVALIDPI(pi)) {
            if((pi->type==ITYPE_CONTAINER || pi->type==ITYPE_LOCKED_ITEM_SPAWNER ||
               pi->type==ITYPE_LOCKED_CONTAINER || pi->type==ITYPE_UNLOCKED_CONTAINER) && pi->id()!=0x0E75)
            {
                pi->moreb1=1;
                if (nValue!=-1) {
                    pi->moreb2=nValue/2;
                    pi->moreb3=nValue;
                } else if (src!=NULL) {
                    pi->moreb2=src->skill[nSkill]/20;
                    pi->moreb3=src->skill[nSkill]/10;
                    src->playSFX(0x1F0);
                    src->sysmsg(TRANSLATE("It's trapped!"));
                } else {
                    pi->moreb2=13;
                    pi->moreb3=26;
                }
            } else if (src!=NULL) src->sysmsg(TRANSLATE("You cannot trap this!!!"));
        }
        break;



         case SPELL_UNTRAP:
         if (pi!=NULL) {
             if((pi->type==ITYPE_CONTAINER || pi->type==ITYPE_LOCKED_ITEM_SPAWNER ||
                pi->type==ITYPE_LOCKED_CONTAINER || pi->type==ITYPE_UNLOCKED_CONTAINER))
         	{
               if(pi->moreb1==1) {
                   pi->moreb1=0;
                   pi->moreb2=0;
                   pi->moreb3=0;
                   src->playSFX(0x1F1);
                   src->sysmsg(TRANSLATE("You successfully untrap this item!"));
               } else if (src!=NULL) src->sysmsg(TRANSLATE("This item doesn't seem to be trapped!"));
           } else if (src!=NULL) src->sysmsg(TRANSLATE("This item cannot be untrapped!"));
        }
        break;





		case SPELL_REACTIVEARMOUR:
			if (nTime==INVALID) nTime = src->skill[nSkill]/50;
			spellFX(spellnumber, src, src);
			tempfx::add(src,src, tempfx::SPELL_REACTARMOR, 0, 0, 0, nTime);
			break;
		case SPELL_DISPEL:	//Luxor
			if ( ISVALIDPC(pd) && pd->summontimer > 0 ) { //Only if it's a summoned creature
				pd->emoteall( "%s begins disappearing", true, pd->getCurrentNameC() );
				//3 seconds left
				if ( pd->summontimer > (uiCurrentTime + 3*MY_CLOCKS_PER_SEC) )
					pd->summontimer = uiCurrentTime + 3*MY_CLOCKS_PER_SEC;
			}
			break;
		case SPELL_MASSDISPEL:
		case SPELL_TELEKINESYS:
		case SPELL_POLYMORPH:
			src->sysmsg("Sorry, spell not yet implemented in this version :(");
			break;

		case SPELL_GATE: //Luxor
			if ( ISVALIDPC( src ) && ISVALIDPI( pi ) ) {
				if ( pi->type == ITYPE_RUNE ) {
					if ((pi->morex < 10)&&(pi->morey < 10)) {
						src->sysmsg("The rune is not marked yet.");
					} else {
						P_ITEM pgate = item::CreateFromScript( "$item_a_blue_moongate" );
						VALIDATEPI( pgate );
						pgate->MoveTo( srcpos );
						pgate->morex = pi->morex;
						pgate->morey = pi->morey;
						pgate->morez = pi->morez;
						pgate->type = 51;
						pgate->setDecay( true );
						pgate->setDecayTime( uiCurrentTime + 30*MY_CLOCKS_PER_SEC );
						pgate->Refresh();

						P_ITEM pgate2 = item::CreateFromScript( "$item_a_blue_moongate" );
						VALIDATEPI( pgate2 );
						pgate2->MoveTo( pi->morex, pi->morey, pi->morez );
						pgate2->morex = srcpos.x;
						pgate2->morey = srcpos.y;
						pgate2->morez = srcpos.z;
						pgate2->type = 51;
						pgate2->setDecay( true );
						pgate2->setDecayTime( uiCurrentTime + 30*MY_CLOCKS_PER_SEC );
						pgate2->Refresh();

						spellFX( spellnumber, src );
					}
				} else
					src->sysmsg("That is not a rune!!");
			}
			break;
			
		case SPELL_MASSCURSE: {
			
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( x, y, 2, true);

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {
				P_CHAR pd = sc.getChar();
				if ( ISVALIDPC(pd) && pd->getSerial32()!=src->getSerial32()) {
					spellFX(spellnumber, src, pd);
					castStatPumper(SPELL_CURSE, dest, src, flags, param);
				}
			} 
			}
			break;

		case SPELL_REVEAL: {
			spellFX(spellnumber, src, pd);
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( x, y, 2, true);
			for( sc.rewind(); !sc.isEmpty(); sc++ ) {
				P_CHAR pd = sc.getChar();
				if (ISVALIDPC(pd) && pd->IsHiddenBySpell() && !checkResist(src, pd, SPELL_REVEAL))
					pd->unHide();
			}
			}
			break;



		case SPELL_PROTECTION:
			spellFX(spellnumber, src, pd);
			if (nTime==INVALID) nTime = src->skill[nSkill]/50;
			if (nValue==INVALID) nValue = src->skill[nSkill]/10;
			tempfx::add(src,src, tempfx::SPELL_PROTECTION, nValue, 0, 0, nTime);
			break;

		case SPELL_ARCHPROTECTION: {
			spellFX(spellnumber, src, pd);
			if (src!=NULL) {
			  if (nTime==INVALID) nTime = 12;
			  if (nValue==INVALID) nValue = 80;
			} else {
			  if (nTime==INVALID) nTime = src->skill[nSkill]/50;
			  if (nValue==INVALID) nValue = src->skill[nSkill]/10;
			}
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( x, y, 2, true);

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {
				P_CHAR pd = sc.getChar();
				if(ISVALIDPC(pd))
					tempfx::add(pd,pd, tempfx::SPELL_PROTECTION, nValue, 0, 0, nTime);
			}
			}
		break;



		case SPELL_INCOGNITO:
			spellFX(spellnumber, src, pd);
			if ((pd==NULL)&&(src!=NULL)) pd = src;
			if (pd!=NULL) {
				if (nTime==INVALID) nTime = 90;
				tempfx::add(pd,pd, tempfx::SPELL_INCOGNITO, 0,0,0, nTime);
			}
			break;

		case SPELL_REFLECTION:
			spellFX(spellnumber, src, pd);
			if ((pd==NULL)&&(src!=NULL)) pd = src;
			if (pd!=NULL) pd->priv2|=CHRPRIV2_REFLECTION;
			break;


		case SPELL_INVISIBILITY:
			spellFX(spellnumber, src, pd);
			if (nTime==INVALID) nTime = 90;
			src->hideBySpell(nTime);
			break;

		case SPELL_HEAL:
		case SPELL_GREATHEAL:
			if (pd==NULL) pd = src;
			if (pd!=NULL) {
                                CHECKDISTANCE(src, pd);
                                spellFX(spellnumber, src, pd);
				if (pd->holydamaged) {
					damage(src, pd, spellnumber, flags|SPELLFLAG_DONTCRIMINAL, param);
				} else {
					if (nValue==INVALID) {
						(spellnumber==SPELL_HEAL) ? nValue = (1+src->skill[nSkill]/100) : nValue = (src->skill[nSkill]/30);
					/*} else if ((nValue==INVALID)&&(src==NULL)) {
						nValue = (spellnumber==SPELL_HEAL) ? 5 : 15;*/
					}
					pd->hp = min(pd->hp+nValue, pd->getStrength());
					src->helpStuff(pd);
					pd->updateStats(STAT_HP);
				} // if !should damage
			} //pd!=NULL
			break;

		case SPELL_CURE:
			if (pd==NULL) pd = src;
                        CHECKDISTANCE(src, pd);
                        if (pd!=NULL) pd->curePoison();
			spellFX(spellnumber, src, pd);
			break;

		case SPELL_ARCHCURE: {
			CHECKDISTANCE(src, pd);
            		spellFX(spellnumber, src, pd);
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( x, y, 2, true);

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {
				P_CHAR pd = sc.getChar();
				if(ISVALIDPC(pd))
					pd->curePoison();
			}
			}
			break;

		case SPELL_RESURRECTION:
			if (pd==NULL) pd = src;
                        CHECKDISTANCE(src, pd);
                        if (pd!=NULL) {
				spellFX(spellnumber, src, pd);
				if (pd->holydamaged) {
					damage(src, pd, spellnumber, flags|SPELLFLAG_DONTCRIMINAL, param);
				} else {
					if ((pd->dead)&&(pd->IsOnline())) pd->resurrect();
					else if ((!pd->dead)&&(src!=NULL)) src->sysmsg(TRANSLATE("That player isn't dead!"));
					else if ((!pd->IsOnline())&&(src!=NULL)) src->sysmsg(TRANSLATE("That player isn't online!"));
				}
			}
			break;

		case SPELL_NIGHTSIGHT:
			if (pd==NULL) pd = src;
                        CHECKDISTANCE(src, pd);
                        if (pd!=NULL) {
				CHECKDISTANCE(src, pd);
				spellFX(spellnumber, src, pd);
				if ((nTime==INVALID)) {
					if (src!=NULL) nTime = src->skill[nSkill] / 2;
					else nTime = 300; // 5' default
				}
				tempfx::add(src, pd, tempfx::SPELL_LIGHT, 0,0,0, nTime);
			}
			break;


		case SPELL_FIREFIELD:
		case SPELL_POISONFIELD:
		case SPELL_PARALYZEFIELD:
		case SPELL_ENERGYFIELD:
		case SPELL_WALLSTONE:
			spellFX(spellnumber, src, src);
			castFieldSpell( src, x, y, z, spellnumber);
			break;

		case SPELL_DISPELFIELD:
			if ( pi!=NULL )
			{
				if( pi->isDispellable() )
				{
					spellFX(spellnumber, src, pd);
					pi->Delete();
				}
			}
			break;



		case SPELL_SUMMON:
			if (src!=NULL) {
				spellFX(spellnumber, src, pd);
				nTime = (nTime==INVALID) ? src->skill[nSkill] / 10 : nTime;
				nValue = (nValue==INVALID) ? xss::getIntFromDefine("$npclist_summon_list") : nValue;
				char buffer_list[20];
				//itoa( nValue, buffer_list, 10 ); // Only works on win os Sparhawk so let's sprintf it
				sprintf( buffer_list, "%d", nValue );
				P_CHAR p_monster = MAKE_CHAR_REF(npcs::AddRandomNPC( src->getSocket(), buffer_list, -1 ));
				if (ISVALIDPC(p_monster)) {
					p_monster->setOwner(src);
					p_monster->tamed = true;
                                        p_monster->summontimer = uiCurrentTime + nTime * MY_CLOCKS_PER_SEC;
					p_monster->MoveTo( x,y,z );
					p_monster->teleport();
				}
			}
			break;

		case SPELL_SUMMON_AIR:
			if (src!=NULL) {
				spellFX(spellnumber, src, pd);
				nTime = (nTime==INVALID) ? (int)(src->skill[nSkill] / 10) : nTime;
				summon (src, xss::getIntFromDefine("$npc_summoned_air_elemental"), nTime, true, x, y, z);
			}
			break;
		case SPELL_SUMMON_DEAMON:
			if (src!=NULL) {
				spellFX(spellnumber, src, pd);
				if (nTime==INVALID) nTime = (src->skill[nSkill] / 10);
				summon (src, xss::getIntFromDefine("$npc_summoned_deamon"), nTime, true, x, y, z);
			}
			break;
		case SPELL_SUMMON_EARTH:
			if (src!=NULL) {
				spellFX(spellnumber, src, pd);
				if (nTime==INVALID) nTime = (src->skill[nSkill] / 10);
    			summon (src, xss::getIntFromDefine("$npc_summoned_earth_elemental"), nTime, true, x, y, z);
			}
			break;
		case SPELL_SUMMON_FIRE:
			if (src!=NULL) {
				spellFX(spellnumber, src, pd);
				if (nTime==INVALID) nTime = (src->skill[nSkill] / 10);
    			summon (src, xss::getIntFromDefine("$npc_summoned_fire_elemental"), nTime, true, x, y, z);
			}
			break;
		case SPELL_SUMMON_WATER:
			if (src!=NULL) {
				spellFX(spellnumber, src, pd);
				if (nTime==INVALID) nTime = (src->skill[nSkill] / 10);
				summon (src, xss::getIntFromDefine("$npc_summoned_water_elemental"), nTime, true, x, y, z);
			}
			break;
		case SPELL_BLADESPIRITS:
			if (src!=NULL) {
				spellFX(spellnumber, src, pd);
				if (nTime==INVALID) nTime = (src->skill[nSkill] / 10);
				summon (src, xss::getIntFromDefine("$npc_summoned_blade_spirit"), nTime, false, x, y, z);
			}
			break;
		case SPELL_ENERGYVORTEX:
			if (src!=NULL) {
				spellFX(spellnumber, src, pd);
				if (nTime==INVALID) nTime = (src->skill[nSkill] / 10);
				summon (src, xss::getIntFromDefine("$npc_summoned_energy_vortex"), nTime, false, x, y, z);
			}
			break;


		case SPELL_MARK:
			if ((src!=NULL)&&(pi!=NULL)) {
				if (pi->type==ITYPE_RUNE) {
					pi->morex = srcpos.x;
					pi->morey = srcpos.y;
					pi->morez = srcpos.z;
					src->sysmsg(TRANSLATE("Recall rune marked."));
					spellFX(spellnumber, src, pd);
				} else {
					src->sysmsg("That is not a rune!!");
				}// if a rune
			}
			break;

		case SPELL_RECALL:
			if ((src!=NULL)&&(pi!=NULL)) {
				if (src->IsOverWeight()) {
					src->sysmsg("You're too heavy!");
				} else {
					if (pi->type==ITYPE_RUNE) {
						if ((pi->morex < 10)&&(pi->morey < 10)) {
							src->sysmsg("The rune is not marked yet.");
						} else {
							src->MoveTo( pi->morex, pi->morey, pi->morez );
							src->teleport();
							spellFX(spellnumber, src, pd);
						} // if rune marked ok
					} else {
						src->sysmsg("That is not a rune!!");
					}// if a rune
				} // if not overweight
			} // if src & pi valids
			break;

		case SPELL_CREATEFOOD:
			if (src!=NULL) {
				if (nValue == INVALID)
					nValue = xss::getIntFromDefine("$item_french_bread");
				item::CreateFromScript( nValue, src->getBackpack() );
				spellFX(spellnumber, src, pd);
			}
		break;




		case SPELL_TELEPORT:
			//Luxor: now a mage cannot teleport to water
			bool isWater = false;
			map_st map;
			data::seekMap(x, y, map);
			switch(map.id)
			{
				//water tiles:
				case 0x00A8:
				case 0x00A9:
				case 0x00AA:
				case 0x00Ab:
				case 0x0136:
				case 0x0137:
				case 0x3FF0:
				case 0x3FF1:
				case 0x3FF2:
				case 0x2FF3:
					isWater = true;
					break;
				default:
					break;
			}
			land_st land;
			data::seekLand(map.id, land);
			if (land.flags&TILEFLAG_WET) isWater = true;
			int s;
			if (src->getClient()!=NULL) {
				s = src->getClient()->toInt();
				if(buffer[s][0x11]==0x17 && buffer[s][0x12]==0x98 || buffer[s][0x11]==0x17 && buffer[s][0x12]==0x9B || buffer[s][0x11]==0x17 && buffer[s][0x12]==0x9C || buffer[s][0x11]==0x17 && buffer[s][0x12]==0x99 || buffer[s][0x11]==0x17 && buffer[s][0x12]==0x97 || buffer[s][0x11]==0x17 && buffer[s][0x12]==0x9A) {
					isWater = true;
				}
			}
			if (!isWater) {
				src->MoveTo( x,y,z );
	                        src->teleport();
                        	spellFX(spellnumber, src, pd);
			}
		break;
	}
	if (g_Spells[spellnumber].attackSpell) src->attackStuff(pd);	//Luxor
}











///////////////////////////////////////////////////////////////////
// Function name	 : castSpell
// Description		 : casts a spell (direct action)
// Return type		 : void
// Author			 : Xanathar
// Argument 		 : int spellnumber -> the spell number
// Argument 		 : TargetLocation& dest -> the target
// Argument 		 : P_CHAR src = NULL -> the caster, if any
// Argument 		 : int flags = 0 -> spell flags, if any
// Argument 		 : int param = 0 -> optional parameters, if any
// Changes			 : Luxor -> some checks to improve stability
void castSpell(SpellId spellnumber, TargetLocation& dest, P_CHAR src, int flags, int param)
{

	if (!checkRequiredTargetType(spellnumber, dest)) return;

	VALIDATEPC(src);

	// initial checks and unhide/unfreeze/disturbmed
	if (src->dead) return;
	src->unfreeze();
	src->unHide();
	src->disturbMed();
	src->spell=spellnumber;
	if ( src->spelltype !=CASTINGTYPE_ITEM && src->spelltype !=CASTINGTYPE_NOMANAITEM )
	{
		src->talkAll((char*)g_Spells[src->spell].mantra.c_str());
		if (src->isMounting()) { //Luxor
			src->playAction(0x1B); // General Lee
		} else {
			src->playAction(g_Spells[src->spell].action);
		}
	}
	// do the event :]
	
	if (src->amxevents[EVENT_CHR_ONCASTSPELL]) {
		g_bByPass = false;
		src->amxevents[EVENT_CHR_ONCASTSPELL]->Call(src->getSerial32(), spellnumber, src->spelltype, INVALID);
		if (g_bByPass==true) return;
	}
	/*
	src->runAmxEvent( EVENT_CHR_ONCASTSPELL, src->getSerial32(), spellnumber, src->spelltype, INVALID );
	if (g_bByPass==true)
		return;
	*/

	// check mana, don't bother the rest if no mana
	if ((!(flags&SPELLFLAG_NOUSEMANA)) && (!checkMana(src, spellnumber))) return;

	// check regs and consume them
	if (!(flags&SPELLFLAG_DONTREQREAGENTS)) {
		if (!checkReagents(src, g_Spells[spellnumber].reagents)) return;
		consumeReagents( src, g_Spells[spellnumber].reagents );
	}

	// if a skill needs to be checked.. check it :]
	// Sparhawk let's check this for pc's and npc's both
	//if ((!src->npc)&&(!(flags&SPELLFLAG_DONTCHECKSKILL))) {
	if ( !( flags & SPELLFLAG_DONTCHECKSKILL ) )
	{
		int skilltobechecked = MAGERY;
		if ( flags&SPELLFLAG_PARAMISSKILLTOUSE )
			skilltobechecked = param;
		int loskill, hiskill;
		if ( ( flags & SPELLFLAG_BONUSCHANCE ) && ( SrvParms->cutscrollreq ) )
		{
			loskill=g_Spells[spellnumber].sclo;
			hiskill=g_Spells[spellnumber].schi;
		}
		else
		{
			loskill=g_Spells[spellnumber].loskill;
			hiskill=g_Spells[spellnumber].hiskill;
		}
		if (!src->checkSkill(static_cast<Skill>(skilltobechecked), loskill, hiskill)) {
			spellFailFX(src);
			return;
		}
	}

	// and now consume that fucking mana
	if (!(flags&SPELLFLAG_NOUSEMANA)) subtractMana(src, spellnumber);

	applySpell(spellnumber, dest, src, flags, param);
}




///////////////////////////////////////////////////////////////////
// Function name	 : beginCasting
// Description		 : prepares spell casting from a char :)
// Return type		 : bool
// Author			 : Xanathar
// Argument 		 : int num -> the spell number
// Argument 		 : NXWCLIENT s -> client which is casting
// Argument 		 : int type -> type of spell casting
// Changes			 : none yet
bool beginCasting (SpellId num, NXWCLIENT s, CastingType type)
{

	if (s == NULL) return true;
	// override for spellcasting (?)
	P_CHAR pc = s->currChar();
	VALIDATEPCR(pc, false);
	if (pc->dead) return false;

	// caster jailed ?
	if ((pc->jailed) && (!pc->IsGM()))
	{
		s->sysmsg(TRANSLATE("You are in jail and cannot cast spells"));
		return false;
	}

	// spell disabled ?
	if( g_Spells[num].enabled != true )
	{
		s->sysmsg(TRANSLATE("Unseen forces make thou unable to cast that spell."));
		return false;
	}

	if ((type!=CASTINGTYPE_ITEM)&&(type!=CASTINGTYPE_NOMANAITEM)&&(!pc->CanDoGestures())) {
		pc->sysmsg(TRANSLATE("You cannot cast with a weapon equipped."));
		return false;
	}

	
	if (pc->amxevents[EVENT_CHR_ONCASTSPELL]) {
		g_bByPass = false;
		pc->amxevents[EVENT_CHR_ONCASTSPELL]->Call(pc->getSerial32(), num, type, s->toInt());
		if (g_bByPass==true) return false;
	}
	/*
	pc->runAmxEvent( EVENT_CHR_ONCASTSPELL, pc->getSerial32(), num, type, s->toInt() );
	if (g_bByPass==true)
		return false;
	*/

	pc->unHide();
	pc->disturbMed();

	if (type==CASTINGTYPE_SPELL && (!checkReagents(pc, g_Spells[num].reagents))) return false;

	if ((type != CASTINGTYPE_ITEM) && (!checkMana(pc, num))) return false;

	pc->spelltype = type;
	pc->spell = num;
	pc->casting = 1;
	pc->nextact = 75;
	pc->spellaction = g_Spells[num].action;

	if ((type==CASTINGTYPE_SPELL)&&(!pc->IsGM())) {
		pc->spelltime = ((g_Spells[num].delay/10)*MY_CLOCKS_PER_SEC)+uiCurrentTime;
		pc->freeze();
	} else {
		pc->spelltime = 0;
	}

	return true;
}







} // namespace
