  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "nxwcommn.h"
#include "debug.h"
#include "tmpeff.h"
#include "sndpkg.h"
#include "set.h"
#include "srvparms.h"
#include "nxw_utils.h"
#include "archive.h"
#include "chars.h"
#include "items.h"
#include "inlines.h"
#include "skills.h"
#include "utils.h"
#include "nox-wizard.h"


namespace tempfx {

SERIAL_SLIST tempfxCheck;

/*!
\author Luxor
*/
void tempeffectson()
{
	if ( tempfxCheck.empty() )
		return;

	P_OBJECT po = NULL;

	SERIAL_SLIST::iterator it( tempfxCheck.begin() );
	for ( ; it != tempfxCheck.end(); ) {
		po = objects.findObject( (*it) );

                if ( po == NULL ) {
			it = tempfxCheck.erase( it );
			continue;
	        }

	        if ( !po->hasTempfx() ) {
			it = tempfxCheck.erase( it );
			continue;
	        }

	        po->tempfxOn();

	        it++;
	}
}

/*!
\author Luxor
*/
void tempeffectsoff()
{
        if ( tempfxCheck.empty() )
		return;

	P_OBJECT po = NULL;

	SERIAL_SLIST::iterator it( tempfxCheck.begin() );
        for ( ; it != tempfxCheck.end(); ) {
                po = objects.findObject( (*it) );

                if ( po == NULL ) {
			it = tempfxCheck.erase( it );
			continue;
	        }

	        if ( !po->hasTempfx() ) {
			it = tempfxCheck.erase( it );
			continue;
	        }

	        po->tempfxOff();
	        it++;
	}
}

/*!
\author Luxor
*/
void checktempeffects()
{
        if ( tempfxCheck.empty() )
		return;

	P_OBJECT po = NULL;

	SERIAL_SLIST::iterator it( tempfxCheck.begin() );
        for ( ; it != tempfxCheck.end(); ) {
                po = objects.findObject( (*it) );

                if ( po == NULL ) {
			it = tempfxCheck.erase( it );
			continue;
	        }

	        if ( !po->hasTempfx() ) {
			it = tempfxCheck.erase( it );
			continue;
	        }

	        po->checkTempfx();
	        it++;
	}
}

/*!
\author Luxor
\brief Calls a custom tempfx defined in small code
*/
static void callCustomTempFx(P_OBJECT poSrc, P_OBJECT poDest, int mode, int amxcallback, int more1, int more2, int more3)
{
	VALIDATEPO(poSrc);
	VALIDATEPO(poDest);
	if (amxcallback <= -2) return;

	int more = (more3<<16) + (more2<<8) + more1;
	AmxFunction::g_prgOverride->CallFn(amxcallback, poSrc->getSerial32(), poDest->getSerial32(), more, mode);
}

/*!
\author Luxor
*/
LOGICAL isSrcRepeatable(int num)
{
	if ( num < 0 || num >= MAX_TEMPFX_INDEX )
		return false;

	switch(num)
	{
		case SPELL_INCOGNITO:
		case SPELL_INVISIBILITY:
		case SPELL_POLYMORPH:
		case DRINK_FINISHED:
			return false;

		default:
			return true;
	}

}

/*!
\author Luxor
*/
LOGICAL isDestRepeatable(int num)
{
	if ( num < 0 || num >= MAX_TEMPFX_INDEX )
		return false;

	switch(num)
	{
		case SPELL_CLUMSY:
		case SPELL_FEEBLEMIND:
		case SPELL_WEAKEN:
		case SPELL_AGILITY:
		case SPELL_STRENGHT:
		case SPELL_CUNNING:
		case SPELL_BLESS:
		case SPELL_CURSE:
		case SPELL_INCOGNITO:
		case SPELL_POLYMORPH:
		case SPELL_INVISIBILITY:
		case CRIMINAL:
		case FIELD_DAMAGE:
			return false;

		default:
			return true;
	}

}


/*!
\author Luxor
*/
SI32 getTempFxTime(P_CHAR src, int num, int more1, int more2, int more3)
{
	int dur = 0;

	if ( num < 0 || num >= MAX_TEMPFX_INDEX )
		return 0;

	switch (num)
	{
		case FIELD_DAMAGE:
			dur = 2;
			break;
		case SPELL_PARALYZE:
			VALIDATEPCR(src, 0);
			dur = src->skill[MAGERY]/100;
			break;

		case SPELL_LIGHT:
			VALIDATEPCR(src, 0);
			dur = src->skill[MAGERY]*10;
			break;

		case SPELL_CLUMSY:
		case SPELL_FEEBLEMIND:
		case SPELL_WEAKEN:
		case SPELL_AGILITY:
		case SPELL_STRENGHT:
		case SPELL_CUNNING:
		case SPELL_BLESS:
		case SPELL_CURSE:
		case SPELL_REACTARMOR:
		case SPELL_PROTECTION:
			VALIDATEPCR(src, 0);
			dur = src->skill[MAGERY]/10;
			break;

		case SPELL_POLYMORPH:
			dur = polyduration;
			break;

		case SPELL_INCOGNITO:
			dur = 90;
			break;

		case SPELL_INVISIBILITY:
			dur = 90;
			break;

		case ALCHEMY_GRIND:
			dur = more2;
			break;

		case ALCHEMY_END:
			dur = 12;
			break;

		case AUTODOOR:
			dur = 10;
			break;

		case TRAINDUMMY:
			dur = 5;
			break;

		case EXPLOTIONMSG:
			dur = more2;
			break;

		case EXPLOTIONEXP:
			dur = 4;
			break;

		case HALLUCINATE:
			dur = 90;
			break;

		case HEALING_HEAL:
			dur = 0;
			break;

		case HEALING_CURE:
			dur = 0;
			break;

		case HEALING_RESURRECT:
			dur = 0;
			break;

		case POTION_DELAY:
			dur = 0;
			break;

		case GM_HIDING:
			dur = more1;
			break;

		case GM_UNHIDING:
			dur = more1;
			break;

		case HEALING_DELAYHEAL:
			dur = more3;
			break;

		case COMBAT_PARALYZE:
			dur = 5;
			break;

		case COMBAT_CONCUSSION:
			dur = 30;
			break;

		case AMXCUSTOM:
			dur = 0;
			break; //Luxor's note: AMXCUSTOM will never get into this function :P

		case GREY:
			dur = 240;
			break;

		case CRIMINAL:
			dur = repsys.crimtime;
			break;

		case DRINK_FINISHED:
			dur = 3;
			break;

		default:
			dur = 0;
			break;
	}

	return dur;
}

/*!
\author Luxor
*/
void cTempfx::start()
{
	P_CHAR src = pointers::findCharBySerial(m_nSrc);
	P_CHAR dest = pointers::findCharBySerial(m_nDest);

	if ( !ISVALIDPC(dest) )
		return;

	switch(m_nNum)
	{
		case FIELD_DAMAGE:
			dest->damage(m_nMore1, static_cast<DamageType>(m_nMore2));
			dest->playSFX(0x0208);
			break;
		case SPELL_PARALYZE:
			if (dest->resistsFreeze())
				return;
			dest->freeze();
			break;

		case SPELL_LIGHT:
			dest->fixedlight = worldbrightlevel;
			if (dest->getClient())
				dolight(dest->getClient()->toInt(), worldbrightlevel);
			break;

		case SPELL_CLUMSY:
			if (dest->dx < m_nMore1)
				m_nMore1 = dest->dx;
			dest->dx -= m_nMore1;
			dest->stm = min (dest->dx, dest->stm);
			if (dest->getClient())
				statwindow(dest, dest);
			break;

		case SPELL_FEEBLEMIND:
			if (dest->in < m_nMore1)
				m_nMore1 = dest->in;
			dest->in -= m_nMore1;
			if (dest->getClient())
				statwindow(dest, dest);
			break;

		case SPELL_WEAKEN:
			if (dest->getStrength() < m_nMore1)
				m_nMore1 = dest->getStrength();
			dest->modifyStrength(-m_nMore1);
			if (dest->getClient())
				statwindow(dest, dest);
			break;

		case SPELL_AGILITY:
			dest->dx += m_nMore1;
			dest->stm += m_nMore1;
			if (dest->getClient())
				statwindow(dest, dest);
			break;

		case SPELL_STRENGHT:
			dest->modifyStrength(m_nMore1);
			dest->hp += m_nMore1;
			if (dest->getClient())
				statwindow(dest, dest);
			break;

		case SPELL_CUNNING:
			dest->in += m_nMore1;
			dest->mn += m_nMore1;
			if (dest->getClient())
				statwindow(dest, dest);
			break;

		case SPELL_BLESS:
			dest->modifyStrength(m_nMore1);
			dest->dx += m_nMore2;
			dest->in += m_nMore3;

			dest->hp += m_nMore1;
			dest->stm += m_nMore2;
			dest->mn += m_nMore3;
			if (dest->getClient())
				statwindow(dest, dest);
			break;

		case SPELL_CURSE:
			if (dest->getStrength() < m_nMore1)
				m_nMore1 = dest->getStrength();
			if (dest->dx < m_nMore2)
				m_nMore2 = dest->dx;
			if (dest->in < m_nMore3)
				m_nMore3 = dest->in;
			dest->modifyStrength(-m_nMore1);
			dest->dx -= m_nMore2;
			dest->in -= m_nMore3;
			if (dest->getClient())
				statwindow(dest, dest);
			break;

		case SPELL_INVISIBILITY:
			dest->hidden = HIDDEN_BYSPELL;
			dest->morph(0);
			break;

		case ALCHEMY_GRIND:
			break;

		case ALCHEMY_END:
			break;

		case AUTODOOR:
			break;

		case TRAINDUMMY:
			break;

		case SPELL_REACTARMOR:
			dest->ra = 1;
			break;

		case EXPLOTIONMSG:
			break;

		case EXPLOTIONEXP:
			break;

		case SPELL_POLYMORPH:
			if (dest->morphed)
				dest->morph();  //if the char is morphed, unmorph him
			setCrimGrey(dest, ServerScp::g_nPolymorphWillCriminal);
			// dest->morph( m_nMore1, -1, -1, -1, -1, -1, NULL, true );
			dest->morph( m_nMore1, true );
			dest->polymorph = true;
			break;

		case SPELL_INCOGNITO:
		{
			//Luxor's incognito code :)
			if (dest->morphed)
				dest->morph();	//if the char is morphed, unmorph him
			dest->incognito = true;
			UI16 body=0, skincolor=0, hairstyle=0, haircolor=0, beardstyle=0, beardcolor=0, x=0;

			//--Sex--
			(chance(50) == true)? body = BODY_MALE : body = BODY_FEMALE;
			//--Skin color--
			x=(SI16)(rand()%6);
			switch(x)
			{
				case 0:	skincolor = 0x83EA; break;
				case 1: skincolor = 0x8405; break;
				case 2:	skincolor = 0x83EF; break;
				case 3: skincolor = 0x83F5; break;
				case 4:	skincolor = 0x841C; break;
				case 5:	skincolor = 0x83FB; break;
				default: skincolor = 0x83FB;break;
			}
			//--Hair Style--
			x=(SI16)(rand()%10);
			switch(x)
			{
				case 0:	hairstyle = 0x203B; break;
				case 1: hairstyle = 0x203C; break;
				case 2:	hairstyle = 0x203D; break;
				case 3: hairstyle = 0x2044; break;
				case 4:	hairstyle = 0x2045; break;
				case 5:	hairstyle = 0x2046; break;
				case 6:	hairstyle = 0x2047; break;
				case 7:	hairstyle = 0x2048; break;
				case 8:	hairstyle = 0x2049; break;
				case 9:	hairstyle = 0x204A; break;
				default: hairstyle = 0x204A;break;
			}
			//--Beard Style--
			if (body == BODY_MALE)
			{
				x=(SI16)(rand()%7);
				switch(x)
				{
					case 0:	beardstyle = 0x203E; break;
					case 1: beardstyle = 0x203F; break;
					case 2:	beardstyle = 0x2040; break;
					case 3: beardstyle = 0x2041; break;
					case 4:	beardstyle = 0x204B; break;
					case 5:	beardstyle = 0x204C; break;
					case 6:	beardstyle = 0x204D; break;
					default: beardstyle = 0x204D;break;
				}
			}
			//--Hair color--
			x=(SI16)(rand()%6);
			switch(x)
			{
				case 0:	haircolor = 0x83EA; break;
				case 1: haircolor = 0x8405; break;
				case 2:	haircolor = 0x83EF; break;
				case 3: haircolor = 0x83F5; break;
				case 4:	haircolor = 0x841C; break;
				case 5:	haircolor = 0x83FB; break;
				default: haircolor = 0x83FB;break;
			}
			//--Beard color--
			x=(SI16)(rand()%6);
			switch(x)
			{
				case 0:	beardcolor = 0x83EA; break;
				case 1: beardcolor = 0x8405; break;
				case 2:	beardcolor = 0x83EF; break;
				case 3: beardcolor = 0x83F5; break;
				case 4:	beardcolor = 0x841C; break;
				case 5:	beardcolor = 0x83FB; break;
				default: beardcolor = 0x83FB;break;
			}
			std::string* newname;
			if( body == BODY_MALE )
			{
				std::string value("1");
				newname = new std::string( cObject::getRandomScriptValue( std::string("RANDOMNAME"), value ) );
			}
			else
			{
				std::string value("2");
				newname = new std::string( cObject::getRandomScriptValue( std::string("RANDOMNAME"), value ) );
			}
			dest->morph(body, skincolor, hairstyle, haircolor, beardstyle, beardcolor, newname->c_str(), true);
		}
			break;

		case HALLUCINATE:
		{
			if (dest->getClient() == NULL) return;
			int index = dest->getClient()->toInt();
			clientInfo[index]->lsd = true;
			impowncreate(index, dest, 0);
		}
			break;

		case SPELL_PROTECTION:
			dest->nxwflags[0] |= cChar::flagSpellProtection;
			break;

		case GM_HIDING:
			break;

		case GM_UNHIDING:
			break;

		case HEALING_DELAYHEAL:
			if (ISVALIDPC(src)) {
				if (src->war)
					src->sysmsg("You cannot heal while you are in a fight.");
				if (!m_nMore2)
					src->sysmsg(TRANSLATE("You start healing..."));
				else
					src->sysmsg(TRANSLATE("You continue to heal..."));
			}
			break;

		case AMXCUSTOM:
			callCustomTempFx(src, dest, MODE_START, m_nAmxcback, m_nMore1, m_nMore2, m_nMore3);
			break;

		case GREY:
			dest->nxwflags[0] |= cChar::flagGrey;
			break;

		case COMBAT_PARALYZE:
			dest->freeze();
			break;

		case COMBAT_CONCUSSION:
			if (dest->in < m_nMore1)
				m_nMore1 = dest->in;
			dest->in -= m_nMore1;
			if (dest->getClient())
				statwindow(dest, dest);
			break;

		case CRIMINAL:
			dest->SetCriminal();
			dest->sysmsg( TRANSLATE("You are now a criminal!") );
			break;

		case SPELL_TELEKINESYS:
			dest->nxwflags[0] |= cChar::flagSpellTelekinesys;
			break;
		default:
			break;
	}

	//if (ISVALIDPC(dest)) item::CheckEquipment(DEREF_P_CHAR(dest));
}

/*!
\author Luxor
*/
SI08 cTempfx::checkForExpire()
{
	if ( !TIMEOUT(m_nExpireTime) )
		return 0;

	executeExpireCode();

        P_OBJECT po = objects.findObject( m_nDest );
	if ( !ISVALIDPO( po ) )
		return INVALID;

	return 1;
}

/*!
\author Luxor
*/
void cTempfx::executeExpireCode()
{
	P_CHAR pc_src = pointers::findCharBySerial(m_nSrc);
	P_CHAR pc_dest = pointers::findCharBySerial(m_nDest);
	P_ITEM pi_src = pointers::findItemBySerial(m_nSrc);
	P_ITEM pi_dest = pointers::findItemBySerial(m_nDest);

	switch(m_nNum)
	{
		case SPELL_PARALYZE:
			VALIDATEPC(pc_dest);
			if (pc_dest->isFrozen())
				pc_dest->unfreeze( true );
			break;

		case SPELL_LIGHT:
			VALIDATEPC(pc_dest);
			pc_dest->fixedlight = 0xFF;
			if (pc_dest->getClient())
				dolight(pc_dest->getClient()->toInt(), worldbrightlevel);
			break;

		case SPELL_CLUMSY:
			VALIDATEPC(pc_dest);
			pc_dest->dx += m_nMore1;
			if (pc_dest->getClient())
				statwindow(pc_dest, pc_dest);
			break;

		case SPELL_FEEBLEMIND:
			VALIDATEPC(pc_dest);
			pc_dest->in += m_nMore1;
			if (pc_dest->getClient())
				statwindow(pc_dest, pc_dest);
			break;

		case SPELL_WEAKEN:
			VALIDATEPC(pc_dest);
			pc_dest->modifyStrength(m_nMore1);
			if (pc_dest->getClient())
				statwindow(pc_dest, pc_dest);
			break;

		case SPELL_AGILITY:
			VALIDATEPC(pc_dest);
			pc_dest->dx -= m_nMore1;
			pc_dest->stm = min(pc_dest->stm, pc_dest->dx);
			if (pc_dest->getClient())
				statwindow(pc_dest, pc_dest);
			break;

		case SPELL_STRENGHT:
			VALIDATEPC(pc_dest);
			pc_dest->modifyStrength(-m_nMore1);
			pc_dest->hp = min(pc_dest->hp, (SI32)pc_dest->getStrength());
			if (pc_dest->getClient())
				statwindow(pc_dest, pc_dest);
			break;

		case SPELL_CUNNING:
			VALIDATEPC(pc_dest);
			pc_dest->in -= m_nMore1;
			pc_dest->mn = min(pc_dest->mn, pc_dest->in);
			if (pc_dest->getClient())
				statwindow(pc_dest, pc_dest);
			break;

		case SPELL_BLESS:
			VALIDATEPC(pc_dest);
			pc_dest->modifyStrength(-m_nMore1);
			pc_dest->dx -= m_nMore2;
			pc_dest->in -= m_nMore3;
			pc_dest->hp = min(pc_dest->hp, (SI32)pc_dest->getStrength());
			pc_dest->stm = min(pc_dest->stm, pc_dest->dx);
			pc_dest->mn = min(pc_dest->mn, pc_dest->in);
			if (pc_dest->getClient())
				statwindow(pc_dest, pc_dest);
			break;

		case SPELL_CURSE:
			VALIDATEPC(pc_dest);
			pc_dest->modifyStrength(m_nMore1);
			pc_dest->dx += m_nMore2;
			pc_dest->in += m_nMore3;
			if (pc_dest->getClient())
				statwindow(pc_dest, pc_dest);
			break;

		case SPELL_INVISIBILITY:
			VALIDATEPC(pc_dest);
			if (pc_dest->IsHiddenBySpell()) {
				pc_dest->hidden = UNHIDDEN;
				pc_dest->morph();
				pc_dest->playSFX(0x203);
			}
			break;


		case ALCHEMY_GRIND:
			VALIDATEPC(pc_dest);
			if (m_nMore1 == 0)
			{
				if (m_nMore2 != 0)
					pc_dest->emoteall(TRANSLATE("*%s continues grinding.*"), 1, pc_dest->getCurrentNameC());

				pc_dest->playSFX(0x242);
			}
			break;
		case ALCHEMY_END:
			VALIDATEPC(pi_src);
			VALIDATEPI(pi_dest);
			Skills::CreatePotion(DEREF_P_CHAR(pi_src),(SI08) m_nMore1,(SI08) m_nMore2, DEREF_P_ITEM(pi_dest));
			break;

		case AUTODOOR:
			VALIDATEPI(pi_dest);
			if (pi_dest->dooropen == 0)
				break;
			pi_dest->dooropen = 0;
			dooruse(INVALID, pi_dest);
			break;

		case TRAINDUMMY:
			VALIDATEPI(pi_dest);
			if (pi_dest->getId() == 0x1071)
			{
				pi_dest->setId(0x1070);
				pi_dest->gatetime = 0;
				pi_dest->Refresh();
			}
			else if (pi_dest->getId()==0x1075)
			{
				pi_dest->setId(0x1074);
				pi_dest->gatetime = 0;
				pi_dest->Refresh();
			}
			break;

		case SPELL_REACTARMOR:
			VALIDATEPC(pc_dest);
			pc_dest->ra = 0;
			break;

		case EXPLOTIONMSG:
			VALIDATEPC(pc_dest);
			pc_dest->sysmsg("%i", m_nMore3);
			break;

		case EXPLOTIONEXP:
			VALIDATEPC(pc_src);
			VALIDATEPI(pi_dest);
			if (pc_src->getClient())
				pi_dest->explode(pc_src->getClient()->toInt());
			break;

		case SPELL_POLYMORPH:
			VALIDATEPC(pc_dest);
			pc_dest->morph();
			pc_dest->polymorph = false;
			break;

		case SPELL_INCOGNITO:
			VALIDATEPC(pc_dest);
			pc_dest->morph();
			pc_dest->incognito = false;
			break;

		case HALLUCINATE:
			if (pc_dest->getClient() == NULL) return;
			pc_dest->sysmsg(TRANSLATE("The effect wears off."));
			pc_dest->stm = 3;
			pc_dest->mn = 3;
			pc_dest->hp /= 7;
			pc_dest->teleport();
			break;

		case SPELL_PROTECTION:
			VALIDATEPC(pc_dest);
			pc_dest->nxwflags[0] &= ~cChar::flagSpellProtection;
			break;

		case DRINK_EMOTE:
			VALIDATEPC(pc_src);
			pc_src->emote(pc_src->getSocket(),"*glu*",1);
			break;

		case DRINK_FINISHED:
			VALIDATEPC(pc_src);
			VALIDATEPI(pi_dest);
			usepotion(pc_src, pi_dest);
			break;

		case GM_HIDING:
			VALIDATEPC(pc_dest);
			pc_dest->sysmsg(TRANSLATE("You have hidden yourself well."));
			//pc_dest->hideBySkill();
			pc_dest->hidden = HIDDEN_BYSKILL;
			pc_dest->teleport( TELEFLAG_NONE );
			break;

		case GM_UNHIDING:
			VALIDATEPC(pc_dest);
			pc_dest->unHide();
			pc_dest->sysmsg(TRANSLATE("You are now visible."));
			break;

		case HEALING_DELAYHEAL:
			VALIDATEPC(pc_src);
			VALIDATEPC(pc_dest);
			if (pc_src->war) {
				pc_src->sysmsg("You cannot heal while you are in a fight.");
				return;
			}
			pc_dest->hp = min(pc_dest->hp + m_nMore1, (SI32)pc_dest->getStrength());
			pc_dest->sysmsg(TRANSLATE("After receiving some healing, you feel better."));
			pc_dest->updateStats(STAT_HP);
			if (!m_nMore2)
				add(pc_src, pc_dest, m_nNum, m_nMore1 +1, 1, m_nMore3);
			break;

		case AMXCUSTOM:
			{
			cObject *src, *dest;
			if ( ISVALIDPC(pc_dest) ) dest=pc_dest;
			else if ( ISVALIDPC(pi_dest) ) dest=pi_dest;
			if ( ISVALIDPC(pc_src) ) src=pc_src;
			else if ( ISVALIDPC(pi_src) ) src=pi_src;

			callCustomTempFx(src, dest, MODE_END, m_nAmxcback, m_nMore1, m_nMore2, m_nMore3);
			}
			break;

		case GREY:
			VALIDATEPC(pc_dest);
			pc_dest->nxwflags[0] &= ~cChar::flagGrey;
			break;

		case CRIMINAL:
			VALIDATEPC( pc_dest );
			pc_dest->SetInnocent();
			pc_dest->sysmsg(TRANSLATE("You are no longer a criminal."));
			break;

		case SPELL_TELEKINESYS:
			VALIDATEPC( pc_dest );
			pc_dest->nxwflags[0] &= ~cChar::flagSpellTelekinesys;
			break;
		case NPC_HIRECOST:
			// src is hired noc, pc_dest is employer
			if ( pc_dest->CountItems(ITEMID_GOLD) < pc_src->getHireFee() )
			{
				// release npc
				pc_src->talkAll("You don't have enough money to employ me anymore!");
				pc_src->setOwner(NULL);
				tempfx::add(pc_src, 
					pc_dest,
					tempfx::NPC_REMOVE,
					0,
					0,
					0,
					MY_CLOCKS_PER_SEC*secondsperuominute*60
				);
			}
			else
			{
				UI32 amount = pc_dest->delItems(ITEMID_GOLD, pc_src->getHireFee());
				if ( amount > 0 )
				{
					// release npc
					pc_src->talkAll("You don't have enough money to employ me anymore!");
					pc_src->setOwner(NULL);
					// remove npc after one hour without payment
					tempfx::add(pc_src, 
						pc_dest,
						tempfx::NPC_REMOVE,
						0,
						0,
						0,
						MY_CLOCKS_PER_SEC*secondsperuominute*60
					);

				}
				tempfx::add(pc_src, 
					pc_dest,
					tempfx::NPC_HIRECOST, 
					0, 
					0, 
					0, 
					MY_CLOCKS_PER_SEC*secondsperuominute*60*24 
				); // call callback every uo day

			}
			break;
		case NPC_REMOVE:
			if ( pc_src->getOwnerSerial32() == INVALID)
			{
				pc_src->Delete();
			}
		default:
			break;
	}



	if (ISVALIDPC(pc_dest))
		pc_dest->checkEquipement();
}


/*!
\author Luxor
*/
void cTempfx::activate()
{
	cObject *src, *dest;
	if ( isCharSerial(m_nSrc))
	{
		P_CHAR pc_src = pointers::findCharBySerial(m_nSrc);
		src=pc_src;
	}
	else
	{
		P_ITEM pi_src = pointers::findItemBySerial(m_nSrc);
		src=pi_src;
	}
	if ( isCharSerial(m_nDest))
	{
		P_CHAR pc_dest = pointers::findCharBySerial(m_nDest);
		dest=pc_dest;
	}
	else
	{
		P_ITEM pc_dest = pointers::findItemBySerial(m_nDest);
		dest=pc_dest;
	}

	switch(m_nNum)
	{
		case SPELL_PARALYZE:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->freeze();
			break;

		case SPELL_LIGHT:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->fixedlight = worldbrightlevel;
			break;

		case SPELL_CLUMSY:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->dx -= m_nMore1;
			break;

		case SPELL_FEEBLEMIND:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->in -= m_nMore1;
			break;

		case SPELL_WEAKEN:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->modifyStrength(-m_nMore1);
			break;

		case SPELL_AGILITY:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->dx += m_nMore1;
			break;

		case SPELL_STRENGHT:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->modifyStrength(m_nMore1);
			break;

		case SPELL_CUNNING:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->in += m_nMore1;
			break;

		case SPELL_BLESS:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->modifyStrength(m_nMore1);
			((P_CHAR)dest)->dx += m_nMore2;
			((P_CHAR)dest)->in += m_nMore3;
			break;

		case SPELL_CURSE:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->modifyStrength(-m_nMore1);
			((P_CHAR)dest)->dx -= m_nMore2;
			((P_CHAR)dest)->in -= m_nMore3;
			break;

		case SPELL_INVISIBILITY:
                        break;

		case AMXCUSTOM:
			callCustomTempFx(src, dest, MODE_ON, m_nAmxcback, m_nMore1, m_nMore2, m_nMore3);
			break;

		case GREY:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->setGreyFlag(false);
			break;

		case CRIMINAL:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->SetCriminal();
			break;

		case SPELL_TELEKINESYS:
			if ( !ISVALIDPC(dest) ) return;
			((P_CHAR)dest)->setTelekinesisFlag(false);
			break;

		default:
			break;
	}


}

/*!
\author Luxor
*/
void cTempfx::deactivate()
{
	cObject *src, *dest;
	if ( isCharSerial(m_nSrc))
	{
		P_CHAR pc_src = pointers::findCharBySerial(m_nSrc);
		src=pc_src;
	}
	else
	{
		P_ITEM pi_src = pointers::findItemBySerial(m_nSrc);
		src=pi_src;
	}
	if ( isCharSerial(m_nDest))
	{
		P_CHAR pc_dest = pointers::findCharBySerial(m_nDest);
		dest=pc_dest;
	}
	else
	{
		P_ITEM pc_dest = pointers::findItemBySerial(m_nDest);
		dest=pc_dest;
	}

	switch(m_nNum)
	{
		case SPELL_PARALYZE:
			if (((P_CHAR)dest)->isFrozen())
				((P_CHAR)dest)->unfreeze( true );
			break;

		case SPELL_LIGHT:
			((P_CHAR)dest)->fixedlight = 0xFF;
			break;

		case SPELL_CLUMSY:
			((P_CHAR)dest)->dx += m_nMore1;
			break;

		case SPELL_FEEBLEMIND:
			((P_CHAR)dest)->in += m_nMore1;
			break;

		case SPELL_WEAKEN:
			((P_CHAR)dest)->modifyStrength(m_nMore1);
			break;

		case SPELL_AGILITY:
			((P_CHAR)dest)->dx -= m_nMore1;
			break;

		case SPELL_STRENGHT:
			((P_CHAR)dest)->modifyStrength(-m_nMore1);
			break;

		case SPELL_CUNNING:
			((P_CHAR)dest)->in -= m_nMore1;
			break;

		case SPELL_BLESS:
			((P_CHAR)dest)->modifyStrength(-m_nMore1);
			((P_CHAR)dest)->dx -= m_nMore2;
			((P_CHAR)dest)->in -= m_nMore3;
			break;

		case SPELL_CURSE:
			((P_CHAR)dest)->modifyStrength(m_nMore1);
			((P_CHAR)dest)->dx += m_nMore2;
			((P_CHAR)dest)->in += m_nMore3;
			break;

		case SPELL_INVISIBILITY:
			break;

		case AMXCUSTOM:
			callCustomTempFx(src, dest, MODE_OFF, m_nAmxcback, m_nMore1, m_nMore2, m_nMore3);
			break;

		case GREY:
			((P_CHAR)dest)->setGreyFlag(false);
			break;

		case CRIMINAL:
			((P_CHAR)dest)->SetInnocent();
			break;
		case SPELL_TELEKINESYS:
			((P_CHAR)dest)->setTelekinesisFlag(false);
			break;

		default:
			break;
	}


}


/*!
\author Luxor
\brief Tells if a tempfx is valid
*/
bool cTempfx::isValid()
{
	if ( m_nNum < 0 || m_nNum >= MAX_TEMPFX_INDEX )
		return false;

	if ( m_nNum == AMXCUSTOM && m_nAmxcback <= INVALID )
		return false;

	P_OBJECT src = objects.findObject(m_nSrc);
	P_OBJECT dest = objects.findObject(m_nDest);

	if ( !ISVALIDPO(src) || !ISVALIDPO(dest) )
		return false;

	return true;
}

/*!
\author Luxor
\brief cTempfx constructor
*/
cTempfx::cTempfx( SERIAL nSrc, SERIAL nDest, SI32 num, SI32 dur, SI32 more1, SI32 more2, SI32 more3, SI32 amxcback )
{
	m_nSrc = INVALID;
	m_nDest = INVALID;
	m_nNum = INVALID;
	m_nMode = INVALID;
	m_nExpireTime = 0;
	m_nAmxcback = INVALID;
	m_nMore1 = INVALID;
	m_nMore2 = INVALID;
	m_nMore3 = INVALID;
	m_bDispellable = false;
	m_bSrcRepeatable = isSrcRepeatable( num );
	m_bDestRepeatable = isDestRepeatable( num );

	//
	//	Set serials
	//
	if ( isCharSerial(nSrc) ) {
		if ( !ISVALIDPC(pointers::findCharBySerial(nSrc)) )
			return;
	}

	if ( isItemSerial(nSrc) ) {
		if ( !ISVALIDPI(pointers::findItemBySerial(nSrc)) )
			return;
	}

	if ( isCharSerial(nDest) ) {
		if ( !ISVALIDPC(pointers::findCharBySerial(nDest)) )
			return;
	}

	if ( isItemSerial(nDest) ) {
		if ( !ISVALIDPI(pointers::findItemBySerial(nDest)) )
			return;
	}

	m_nSrc = nSrc;
	m_nDest = nDest;

	if ( num < 0 || num >= MAX_TEMPFX_INDEX )
		return;

	m_nNum = num;

	//
	//	If a duration is given, use it. Otherwise, use the standard value.
	//
	if ( dur > 0 )
		m_nExpireTime = uiCurrentTime + (dur*MY_CLOCKS_PER_SEC);
	else
		m_nExpireTime = uiCurrentTime + (getTempFxTime(pointers::findCharBySerial(m_nSrc), num, more1, more2, more3)*MY_CLOCKS_PER_SEC);

	if ( m_nNum == AMXCUSTOM && amxcback <= INVALID )
		return;

	m_nMore1 = more1;
	m_nMore2 = more2;
	m_nMore3 = more3;
	m_nAmxcback = amxcback;
}

/*!
\author Luxor
\brief	Adds a temp effect
*/
bool add(P_OBJECT src, P_OBJECT dest, int num, unsigned char more1, unsigned char more2, unsigned char more3, short dur, int amxcback)
{
	VALIDATEPOR(src, false);
	VALIDATEPOR(dest, false);

	return dest->addTempfx( *src, num, (int)more1, (int)more2, (int)more3, (int)dur, amxcback );
}

/*!
\author Luxor
*/
void addTempfxCheck( SERIAL serial )
{
	P_OBJECT po = objects.findObject( serial );
	VALIDATEPO( po );

	if ( find( tempfxCheck.begin(), tempfxCheck.end(), serial ) != tempfxCheck.end() )
		return;

	tempfxCheck.push_front( serial );
}

} //namespace



