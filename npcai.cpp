  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "amx/amxcback.h"
#include "magic.h"
#include "npcai.h"
#include "set.h"
#include "chars.h"
#include "items.h"
#include "basics.h"
#include "utils.h"
#include "inlines.h"
#include "nox-wizard.h"
#include "scripts.h"

namespace npcs {


/*!
\author Luxor
*/
static void npcBeginCasting( P_CHAR pc, P_CHAR target, magic::SpellId spell )
{
	VALIDATEPC( pc );
	VALIDATEPC( target );
	if ( pc->spellTL != NULL )
		safedelete( pc->spellTL );
	pc->spellTL = new TargetLocation( target );
	pc->spelltype = magic::CASTINGTYPE_NPC;
	pc->spell = spell;
	pc->casting = 1;
	pc->nextact = 1;
	pc->spellaction = 6;
	pc->spelltime = magic::getCastingTime( spell );
	pc->addTempfx( *pc, tempfx::SPELL_PARALYZE, 0, 0, 0, ( pc->spelltime - uiCurrentTime ) / MY_CLOCKS_PER_SEC );
	pc->emoteall("*Begins casting a spell*", false);
}

///////////////NPC MAGIC STUFF
///////////////BY LUXOR & XANATHAR
#define NPCMAGIC_FLAGS (SPELLFLAG_DONTCRIMINAL+SPELLFLAG_DONTREQREAGENTS+SPELLFLAG_DONTCHECKSPELLBOOK+SPELLFLAG_IGNORETOWNLIMITS+SPELLFLAG_DONTCHECKSKILL)
//#define NPC_CASTSPELL(A,B) pc_att->castSpell(A, TargetLocation DUMMYTMP(B), NPCMAGIC_FLAGS);
//#define NPC_CASTSPELL(A,B) { TargetLocation INSTANCETEMP(B); pc_att->castSpell(A, INSTANCETEMP , NPCMAGIC_FLAGS); }
#define NPC_CASTSPELL(A,B) { npcBeginCasting( pc_att, B, A ); }

int spherespells[256][256];

void npcMagicAttack(P_CHAR pc_att, P_CHAR pc_def)
{
	VALIDATEPC(pc_att);
	VALIDATEPC(pc_def);
	int spattackbit, spattacks, currenttime = uiCurrentTime;
	// early return if prerequisites for spellcasting aren't true!
	// dirty,but helps losing some KG of code later :)

	if ((pc_att->spatimer > (TIMERVAL)currenttime)) return;

	//note : if magicsphere!=0, spattack has a different meaning!!
	spattacks = numbitsset( pc_att->spattack );

	if (pc_def->dead || pc_att->distFrom(pc_def)>=10 || spattacks <= 0 || pc_att->spattack==0) return;

	pc_att->spatimer=currenttime+(pc_att->spadelay*MY_CLOCKS_PER_SEC); //LB bugkilling


	if ( pc_def->summontimer && pc_att->baseskill[MAGERY] > 700 ) {
		pc_att->facexy( pc_def->getPosition().x, pc_def->getPosition().y );
		NPC_CASTSPELL( magic::SPELL_DISPEL, pc_def );
		return;
	}
	// We're here.. let's spellcast ;)
	if (pc_att->magicsphere!=0) npcCastSpell(pc_att, pc_def);
	else
	{
		spattackbit=rand()%(spattacks) + 1;
		pc_att->playAction(6);

		switch(whichbit(pc_att->spattack, spattackbit))
		{
			case 1:
				NPC_CASTSPELL(magic::SPELL_MAGICARROW, pc_def);
				break;
			case 2:
				NPC_CASTSPELL(magic::SPELL_HARM, pc_def);
				break;
			case 3:
				NPC_CASTSPELL(magic::SPELL_CLUMSY, pc_def);
				break;
			case 4:
				NPC_CASTSPELL(magic::SPELL_FEEBLEMIND, pc_def);
				break;
			case 5:
				NPC_CASTSPELL(magic::SPELL_WEAKEN, pc_def);
				break;
			case 6:
				NPC_CASTSPELL(magic::SPELL_FIREBALL, pc_def);
				break;
			case 7:
				NPC_CASTSPELL(magic::SPELL_CURSE, pc_def);
				break;
			case 8:
				NPC_CASTSPELL(magic::SPELL_LIGHTNING, pc_def);
				break;
			case 9:
				NPC_CASTSPELL(magic::SPELL_PARALYZE, pc_def);
				break;
			case 10:
				NPC_CASTSPELL(magic::SPELL_MINDBLAST, pc_def);
				break;
			case 11:
				NPC_CASTSPELL(magic::SPELL_ENERGYBOLT, pc_def);
				break;
			case 12:
				NPC_CASTSPELL(magic::SPELL_EXPLOSION, pc_def);
				break;
			case 13:
				NPC_CASTSPELL(magic::SPELL_FLAMESTRIKE, pc_def);
				break;
			case 14:
				NPC_CASTSPELL(magic::SPELL_MINDBLAST, pc_def);
				break;
			case 15:
				NPC_CASTSPELL(magic::SPELL_MINDBLAST, pc_def);
				break;
			case 16:
				NPC_CASTSPELL(magic::SPELL_MINDBLAST, pc_def);
				break;
			default:
				break;
		}
	}
}

#define CHECKSPELL(NAME,VALUE) { if ( script1 == NAME ) { spherespells[section][ptr++] = VALUE; continue;	} }
void initNpcSpells ()
{
	int ptr = 0;

	ConOut("Loading custom NPC grimoires (npcmagic.xss)...");

	for ( int i=0; i<256; ++i )
		for ( int j=0; j<256; ++j )
			spherespells[i][j] = 0;

	cScpIterator*	iter = NULL;
	std::string	script1,
			script2;

	for (int section=0; section<256; ++section )
	{
		safedelete(iter);
		iter = Scripts::NpcMagic->getNewIterator( "SECTION SPHERE %d", section );
		if (iter != 0 )
		{
			ptr = 0;
			do {
				iter->parseLine(script1, script2);
				if ((script1[0]!='}')&&(script1[0]!='{')) {
					CHECKSPELL("SUMMON", (-atoi(script2.c_str())));
					CHECKSPELL("MAGICARROW", 1);
					CHECKSPELL("HARM", 2);
					CHECKSPELL("CLUMSY", 3);
					CHECKSPELL("FEEBLEMIND", 4);
					CHECKSPELL("WEAKEN", 5);
					CHECKSPELL("FIREBALL", 6);
					CHECKSPELL("CURSE", 7);
					CHECKSPELL("LIGHTNING", 8);
					CHECKSPELL("PARALYZE", 9);
					CHECKSPELL("MINDBLAST", 10);
					CHECKSPELL("ENERGYBOLT", 11);
					CHECKSPELL("EXPLOSION", 12);
					CHECKSPELL("FLAMESTRIKE", 13);
					CHECKSPELL("FIREFIELD", 14);
					CHECKSPELL("POISONFIELD", 15);
					CHECKSPELL("PARALYZEFIELD", 16);
					CHECKSPELL("HEAL", 17);
					CHECKSPELL("GREATHEAL", 18);
					CHECKSPELL("GREATCURE", 19);
					CHECKSPELL("PIROMANCY", 20);
					CHECKSPELL("POISOMANCY", 21);
					CHECKSPELL("POISON", 22);
					CHECKSPELL("MANADRAIN", 23);
					CHECKSPELL("MANAVAMPIRE", 24);
					CHECKSPELL("BLADESPIRITS", 25);
					CHECKSPELL("ENERGYVORTEX", 26);
					CHECKSPELL("MAGICREFLECTION", 27);
					CHECKSPELL("CHAINLIGHTNING", 28);
					CHECKSPELL("METEORSWARM", 29);
					CHECKSPELL("EARTHQUAKE", 30);
				}
			} while (script1[0]!='}');
		}

	}
	safedelete(iter);
	ConOut("[DONE]\n");
}
#undef CHECKSPELL

void npcCastSpell(P_CHAR pc_att, P_CHAR pc_def)
{
	VALIDATEPC(pc_att);
	VALIDATEPC(pc_def);
	int sphere = (pc_att->magicsphere) % 255;
	int spell = (rand()%pc_att->spattack) % 255;


	if (spherespells[sphere][spell]==0) return;

	if (pc_att->amxevents[EVENT_CHR_ONCASTSPELL]) {
		g_bByPass = false;
		pc_att->amxevents[EVENT_CHR_ONCASTSPELL]->Call(pc_att->getSerial32(), spell, -1, sphere);
		if (g_bByPass==true) return;
	}
	/*
	pc_att->runAmxEvent( EVENT_CHR_ONCASTSPELL, pc_att->getSerial32(), spell, -1, sphere );
	if (g_bByPass==true)
		return;
	*/

	pc_att->playAction(6);


	if (spherespells[sphere][spell]<0) {
		//summon an NPC
		if (pc_att->mn>=40) {
			pc_att->emoteall(TRANSLATE("*Doing a summoning ritual.*"),1);
			AddNPC(INVALID, NULL, -spherespells[sphere][spell], pc_def->getPosition());
			pc_att->mn -= 40;
		}
		return;
	}

	switch(spherespells[sphere][spell])
	{
		case 1:
			NPC_CASTSPELL(magic::SPELL_MAGICARROW, pc_def);
			break;
		case 2:
			NPC_CASTSPELL(magic::SPELL_HARM, pc_def);
			break;
		case 3:
			NPC_CASTSPELL(magic::SPELL_CLUMSY, pc_def);
			break;
		case 4:
			NPC_CASTSPELL(magic::SPELL_FEEBLEMIND, pc_def);
			break;
		case 5:
			NPC_CASTSPELL(magic::SPELL_WEAKEN, pc_def);
			break;
		case 6:
			NPC_CASTSPELL(magic::SPELL_FIREBALL, pc_def);
			break;
		case 7:
			NPC_CASTSPELL(magic::SPELL_CURSE, pc_def);
			break;
		case 8:
			NPC_CASTSPELL(magic::SPELL_LIGHTNING, pc_def);
			break;
		case 9:
			NPC_CASTSPELL(magic::SPELL_PARALYZE, pc_def);
			break;
		case 10:
			NPC_CASTSPELL(magic::SPELL_MINDBLAST, pc_def);
			break;
		case 11:
			NPC_CASTSPELL(magic::SPELL_ENERGYBOLT, pc_def);
			break;
		case 12:
			NPC_CASTSPELL(magic::SPELL_EXPLOSION, pc_def);
			break;
		case 13:
			NPC_CASTSPELL(magic::SPELL_FLAMESTRIKE, pc_def);
			break;
		case 14:
			NPC_CASTSPELL(magic::SPELL_FIREFIELD, pc_def);
			break;
		case 15:
			NPC_CASTSPELL(magic::SPELL_POISONFIELD, pc_def);
			break;
		case 16:
			NPC_CASTSPELL(magic::SPELL_PARALYZEFIELD, pc_def);
			break;
		case 17:
			NPC_CASTSPELL(magic::SPELL_HEAL, pc_att);
			break;
		case 18:
			NPC_CASTSPELL(magic::SPELL_GREATHEAL, pc_att);
			break;
		case 19:
			NPC_CASTSPELL(magic::SPELL_CURE, pc_att);
			break;
		case 22:
			NPC_CASTSPELL(magic::SPELL_POISON, pc_def);
			break;
		case 23:
			NPC_CASTSPELL(magic::SPELL_MANADRAIN, pc_def);
			break;
		case 24:
			NPC_CASTSPELL(magic::SPELL_MANAVAMPIRE, pc_def);
			break;
		case 27:
			NPC_CASTSPELL(magic::SPELL_REFLECTION, pc_att);
			break;
		default :
			WarnOut("NPC-Spell %d not yet implemented, sorry :(\n", spherespells[sphere][spell]);
			break;
	}

}
//END MAGIG STUFF



/*!
\author Luxor
*/
void checkAI(P_CHAR pc) //Lag Fix -- Zippy
{
	VALIDATEPC(pc);
	P_CHAR pc_att = pc;	//Dirty... but now we can use NPC_CASTSPELL macro :P
	if ( !pc->npc ) return;
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	if ( !TIMEOUT( pc->nextAiCheck ) )
		return;

	pc->nextAiCheck = (UI32) ((R64)uiCurrentTime + (speed.npcaitime*MY_CLOCKS_PER_SEC));

	if( pc->amxevents[ EVENT_CHR_ONCHECKNPCAI ]!=NULL ) {

		NxwCharWrapper sc;
		sc.fillCharsNearXYZ( pc->getPosition(), VISRANGE, true, false );
		SERIAL set = amxSet::create();
		amxSet::copy( set, sc );

		g_bByPass = false;
		pc->amxevents[ EVENT_CHR_ONCHECKNPCAI ]->Call( pc->getSerial32(), set, uiCurrentTime );

		amxSet::deleteSet( set );

		if (g_bByPass==true)
			return;
	}

	switch(pc->npcaitype)
	{
		case NPCAI_GOOD:
			if( pc->war )
				return;

			if( pc->npcWander == WANDER_FLEE )
				return;

			if( pc->shopkeeper )
			{
				NxwCharWrapper sc;
				sc.fillCharsNearXYZ( pc->getPosition(), 3, true, true );
				for( sc.rewind(); !sc.isEmpty(); sc++ ) {
					P_CHAR pj=sc.getChar();
					if (pj->getSerial32() == pc->getSerial32()) continue; //Luxor

					if( pj->dead )
						continue;

					if( pj->hidden )
						continue;
					// Stop talking npcs to each other
					if( pj->IsInnocent() && !pj->npc )
					{
						sprintf( temp,TRANSLATE("Hello %s, Welcome to my shop, How may i help thee?."), pj->getCurrentNameC());
						pc->talkAll( temp, 1);
					}
				}
			}
			break;
		case NPCAI_HEALER: // good healers
		{
			if ( pc->war )
				return;

			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc->getPosition(), 3, true, true );

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {

				P_CHAR pj=sc.getChar();

				if ( !ISVALIDPC(pj) || !pj->dead )
			  		continue;

				if (pj->getSerial32() == pc->getSerial32()) continue; //Luxor
			  	if ( !pj->IsInnocent() || pj->IsCriminal() || pj->IsMurderer())
			  	{
			  		if (pj->IsMurderer())
			  			pc->talkAll(TRANSLATE("I will nay give life to a scoundrel like thee!"), 1);
			  		else if (pj->IsCriminal())
						pc->talkAll(TRANSLATE("I will nay give life to thee for thou art a criminal!"), 1);

			  		continue;
			  	}

				pc->playAction(0x10);
				pj->resurrect();
				pj->staticFX(0x376A, 1, 0, NULL);
				switch(RandomNum(0, 4))
				{
					case 0: pc->talkAll(TRANSLATE("Thou art dead, but 'tis within my power to resurrect thee.	Live!"), 1); break;
					case 1: pc->talkAll(TRANSLATE("Allow me to resurrect thee ghost.  Thy time of true death has not yet come."), 1); break;
					case 2: pc->talkAll(TRANSLATE("Perhaps thou shouldst be more careful.	Here, I shall resurrect thee."), 1); break;
					case 3: pc->talkAll(TRANSLATE("Live again, ghost!	Thy time in this world is not yet done."), 1); break;
					case 4: pc->talkAll(TRANSLATE("I shall attempt to resurrect thee."), 1); break;
				}

			}
		}
		break;
		case NPCAI_EVIL:
		{
			if ( pc->war )
				return;
			if ( pc->npcWander == WANDER_FLEE )
				return;

			if (pc->baseskill[MAGERY] > 400)
			{
				if ( chance( 50 ) )
				{
					if (pc->hp < pc->getStrength()/2)
						NPC_CASTSPELL(magic::SPELL_GREATHEAL, pc);
				}
				else
					if (pc->poisoned > 0)
						NPC_CASTSPELL(magic::SPELL_CURE, pc);
			}
#ifdef SPAR_NEW_WR_SYSTEM
			pointers::pCharVector *pcv = pointers::getCharsNearLocation( pc, VISRANGE, pointers::NPC );
			pointers::pCharVectorIt it( pcv->begin() ), end( pcv->end() );
			P_CHAR	pj = 0;
			P_CHAR	pc_target = NULL;
			SI32	att_value = 0, curr_value = 0;
			while( it != end )
			{
				pj = (*it);
				if ( 	!(
					pc->getSerial32() == pj->getSerial32() ||
					pj->IsInvul() ||
					pj->hidden > 0 ||
					pj->dead ||
					pj->npcaitype == NPCAI_EVIL ||
					pj->npcaitype == NPCAI_HEALER ||
					( SrvParms->monsters_vs_animals == 0 && ((pj->title.size() == 0) && !pj->IsOnline()) ) ||
					( SrvParms->monsters_vs_animals == 1 && chance( SrvParms->animals_attack_chance ) )
					)
				   )
				{
					if( pc->losFrom( pj ) )
					{
						if ( pc_target != 0 )
						{
                                        		curr_value = pc->distFrom( pj ) + pj->hp/3;
							if ( curr_value < att_value )
								pc_target = pj;
						}
						else
						{
							att_value = curr_value = pc->distFrom( pj ) + pj->hp/3;
							pc_target = pj;
						}
					}
				}
				++it;
			}
			if ( pc_target != NULL )
				pc->fight( pc_target );
#else
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc->getPosition(), VISRANGE, true, false );
			P_CHAR pc_target = NULL;
			SI32 att_value = 0, curr_value = 0;
			for( sc.rewind(); !sc.isEmpty(); sc++ ) {

				P_CHAR pj=sc.getChar();
				if (!ISVALIDPC(pj) || pc->getSerial32()==pj->getSerial32() )
					continue;

				if (	pj->IsInvul() ||
						pj->hidden > 0 ||
						pj->dead ||
						pj->npcaitype == NPCAI_EVIL ||
						pj->npcaitype == NPCAI_HEALER ||
						( SrvParms->monsters_vs_animals == 0 && ((pj->title.size() == 0) && !pj->IsOnline()) ) ||
						( SrvParms->monsters_vs_animals == 1 && chance( SrvParms->animals_attack_chance ) )
					)
					continue;

				if ( !pc->losFrom( pj ) )
					continue;

				if ( pc_target != NULL ) {
                                        curr_value = pc->distFrom( pj ) + pj->hp/3;
					if ( curr_value < att_value )
						pc_target = pj;
				} else {
					att_value = curr_value = pc->distFrom( pj ) + pj->hp/3;
					pc_target = pj;
				}
			}

                        if ( pc_target != NULL )
				pc->fight( pc_target );
#endif
		}
		break;
		case NPCAI_EVILHEALER:
		{
			if (pc->war) return;

			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc->getPosition(), 3, true, true );

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {
				P_CHAR pj=sc.getChar();
				if ( !ISVALIDPC( pj ) || !pj->dead )
					continue;
				if (pj->getSerial32() == pc->getSerial32()) continue; //Luxor
				if ( pj->IsInnocent() ) {
					pc->talkAll(TRANSLATE("I despise all things good. I shall not give thee another chance!"), 1);
					continue;
				}
				pc->playAction(0x10);
				pj->resurrect();
				pj->staticFX(0x3709, 1, 0, NULL);
				switch (RandomNum(0,4))
				{
					case 0:	pc->talkAll(TRANSLATE("Fellow minion of Mondain, Live!!"), 1); break;
					case 1:	pc->talkAll(TRANSLATE("Thou has evil flowing through your vains, so I will bring you back to life."), 1); break;
					case 2:	pc->talkAll(TRANSLATE("If I res thee, promise to raise more hell!."), 1); break;
					case 3:	pc->talkAll(TRANSLATE("From hell to Britannia, come alive!."), 1); break;
					case 4:	pc->talkAll(TRANSLATE("Since you are Evil, I will bring you back to consciouness."), 1); break;

				}
			}
		}
		break;

		case NPCAI_BEGGAR:
		{
			if (pc->war) return;

			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc->getPosition(), 3, true, true );

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {

				P_CHAR pj=sc.getChar();
				if (!ISVALIDPC(pj)) continue;

				if (pj->getSerial32() == pc->getSerial32()) continue; //Luxor
				if ( pj->dead || !pj->IsInnocent() || pj->hidden > 0)
					continue;

				switch (RandomNum(0,2))
				{
					case 0: pc->talkAll(TRANSLATE("Could thou spare a few coins?"), 1); break;
					case 1: pc->talkAll(TRANSLATE("Hey buddy can you spare some gold?"), 1); break;
					case 2: pc->talkAll(TRANSLATE("I have a family to feed, think of the children."), 1); break;
					default: break;
				}
			}
		}
		break;

		case NPCAI_ORDERGUARD: break;
		case NPCAI_CHAOSGUARD: break;
		case NPCAI_BANKER: break;
		case NPCAI_TELEPORTGUARD:
		case NPCAI_GUARD : 		// in world guards, they dont teleport out...Ripper
		{
			if (pc->war || !(region[pc->region].priv & RGNPRIV_GUARDED))	// this region is not guarded
				return;
			//
			// Sparhawk	Check wether a guard should continue walking towards the character who called him
			//
			int x, y;
			if ( pc->npcWander == WANDER_FOLLOW )
			{
				P_CHAR guardcaller = pointers::findCharBySerial( pc->ftargserial );

				if ( (!(ISVALIDPC( guardcaller ))) || TIMEOUT( pc->antiguardstimer ) || pc->distFrom( guardcaller ) <= 1 )
				{
					pc->npcWander = pc->oldnpcWander;
					pc->oldnpcWander = WANDER_NOMOVE;
					pc->ftargserial = INVALID;
				}
				else
				{
					//
					//	Guard is still walking towards char who called
					//	In that case check whether guard can see targets within visual range of caller
					//
					x = guardcaller->getPosition("x");
					y = guardcaller->getPosition("y");
				}
			}
			else
			{
				x = pc->getPosition("x");
				y = pc->getPosition("y");
			}

			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc->getPosition(), VISRANGE, true, false );

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {

				P_CHAR character=sc.getChar();
				if ( ISVALIDPC( character ) )
				{
					if ( character->getSerial32() != pc->getSerial32() &&
					     !character->dead &&
					     !character->IsHidden() &&
					     pc->losFrom( character )
					   )
					{
						if ( 	character->npcaitype == NPCAI_EVIL ||
							character->npcaitype == NPCAI_MADNESS ||
							character->IsMurderer() ||
							character->IsCriminal()
						   )
						{
							if ( pc->npcWander == WANDER_FOLLOW )
							{
								pc->npcWander = pc->oldnpcWander;
								pc->oldnpcWander = WANDER_NOMOVE;
								pc->ftargserial = INVALID;
							}

							pc->talkAll(TRANSLATE("Thou shalt regret thine actions, swine!"), 1);
							pc->fight( character );
							return;
						}
					}
				}
			}
		}
		break;
		case NPCAI_TAMEDDRAGON: // Tamed Dragons ..not white wyrm..Ripper
		// so regular dragons attack reds on sight while tamed.
		{
			if (!pc->tamed) return;
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc->getPosition(), 10, true, false );

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {

				P_CHAR pj=sc.getChar();

				if (pj->getSerial32() == pc->getSerial32()) continue; //Luxor
				if( ISVALIDPC(pj) && pj->npc && pj->npcaitype==NPCAI_EVIL)
				{
					npcattacktarget(pc, pj);
					return;
				}
			}
		}
		break;
		case NPCAI_ATTACKSRED:
		{
			if (pc->war) return;
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc->getPosition(), 10, true, false );

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {

				P_CHAR pj=sc.getChar();
				if (!ISVALIDPC(pj)) continue;
				if (pj->getSerial32() == pc->getSerial32()) continue; //Luxor
				if ( pj->IsInvul() || pj->dead || (pj->npcaitype != NPCAI_EVIL && !pj->IsCriminal() && !pj->IsMurderer())) continue;

				npcattacktarget(pc, pj);
			}
		}
		break;
		case NPCAI_PLAYERVENDOR: break;
		case NPCAI_PETGUARD:
		{
			if (!pc->tamed) return;
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc->getPosition(), VISRANGE, true, false );

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {

				P_CHAR pj=sc.getChar();
				if (!(ISVALIDPC(pj))) continue;
				if (pj->npc || pj->dead || pj->guarded == false) continue;
				if (pc->getOwnerSerial32() == pj->getSerial32()) {
					if (pj->IsOnline()) {
						P_CHAR pc_attacker = pointers::findCharBySerial(pj->attackerserial);
						VALIDATEPC(pc_attacker);
						if (pc->distFrom(pc_attacker) <= 10) {
							npcattacktarget(pc, pc_attacker);
							return;
						}
					}
				}
			}
		}
		break;
		case 30: // why is this the same as case 50???..Ripper
		case NPCAI_MADNESS://Energy Vortex/Blade Spirit
		{
			if (pc->war) return;
			if (pc->attackerserial == INVALID) {
				NxwCharWrapper sc;
				sc.fillCharsNearXYZ( pc->getPosition(), 10, true, false );

				for( sc.rewind(); !sc.isEmpty(); sc++ ) {

					P_CHAR pj=sc.getChar();
					if ( !pj->IsInvul() && !pj->dead && pj->hidden == UNHIDDEN && pj->npcaitype != NPCAI_MADNESS) {
						pc->fight(pj);
						return;
					}
				}
			}
		}
		break;
		// Case 60-70 is Skyfires new AI
		case NPCAI_DRAGON1:
		case NPCAI_DRAGON2: //Dragon AI (Skyfire and araknesh)
		{
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc->getPosition(), VISRANGE, true, false );

			for( sc.rewind(); !sc.isEmpty(); sc++ ) {

				P_CHAR pj=sc.getChar();
				if (!(ISVALIDPC(pj))) continue;
				if (pj->dead ||
						pj->IsInvul() ||
						pj->IsGMorCounselor() ||
						pj->npcaitype == NPCAI_EVIL ||
						pj->npcaitype == NPCAI_HEALER ||
						pj->npcaitype == NPCAI_DRAGON1 ||
						pj->npcaitype == NPCAI_DRAGON2 || pj == pc) continue;

				if (pj->hidden == UNHIDDEN) {
					switch(RandomNum(0, 6))
					{
						case 0:
							NPC_CASTSPELL(magic::SPELL_CURSE, pj);
							pc->talkAll(TRANSLATE("You are ridiculous"), 1);
							break;
						case 1:
							NPC_CASTSPELL(magic::SPELL_FLAMESTRIKE, pj);
							pc->talkAll(TRANSLATE("Die unusefull mortal!"), 1);
							break;
						case 2:
							NPC_CASTSPELL(magic::SPELL_PARALYZE, pj);
							pc->talkAll(TRANSLATE("What are you doing? Come here and Die!"), 1);
							break;
						case 3:
							NPC_CASTSPELL(magic::SPELL_LIGHTNING, pj);
							pc->talkAll(TRANSLATE("Stupid Mortal I'll crush you as a fly"), 1);
							break;
						case 4:
							NPC_CASTSPELL(magic::SPELL_LIGHTNING, pj);
							pc->talkAll(TRANSLATE("Stupid Mortal I'll crush you as a fly"), 1);
							break;
						case 5:
							NPC_CASTSPELL(magic::SPELL_EXPLOSION, pj);
							pc->talkAll(TRANSLATE("Die unusefull mortal!"), 1);
							break;
						case 6:
							NPC_CASTSPELL(magic::SPELL_EXPLOSION, pj);
							pc->talkAll(TRANSLATE("Die unusefull mortal!"), 1);
							break;
					}
				}
				if (pc->hp < pc->getStrength()/2) {
					pc->talkAll("In Vas Mani", 1);
					NPC_CASTSPELL(magic::SPELL_GREATHEAL, pc);
				}
				if (pc->poisoned > 0) {
					pc->talkAll("An Nox", 1);
					NPC_CASTSPELL(magic::SPELL_CURE, pc);
				}
				if ( pj->isDispellable() ) {
					pc->talkAll("An Ort", 1);
					NPC_CASTSPELL(magic::SPELL_DISPEL, pj);
				}

				if ( !pj->IsHidden() ) npcattacktarget(pc, pj);
				return;
			}
		}
		break;
		default:
			WarnOut("cCharStuff::CheckAI-> Error npc %i ( %08x ) has invalid AI type %i\n", pc->getSerial32(), pc->getSerial32(), pc->npcaitype);
			return;
	}	//switch(pc->npcaitype)
} //void checkAI(unsigned int currenttime, P_CHAR pc)


} //namespace


