  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "sndpkg.h"
#include "rcvpkg.h"
#include "srvparms.h"
#include "amx/amxcback.h"
#include "debug.h"
#include "npcai.h"
#include "set.h"
#include "items.h"
#include "chars.h"
#include "basics.h"
#include "inlines.h"
#include "classes.h"
#include "skills.h"
#include "nox-wizard.h"

void deadattack (NXWSOCKET  s, CHARACTER i)
{
	CHARACTER cc = currchar[s];
	P_CHAR pc = MAKE_CHAR_REF( cc );
	VALIDATEPC( pc );
	P_CHAR pc_other = MAKE_CHAR_REF( i );
	VALIDATEPC( pc_other );

	if(pc_other->npc)
	{
		if(pc_other->npcaitype==NPCAI_HEALER)
		{
			if( pc->IsInnocent() )
			{
				if ( pc->distFrom( pc_other ) <= 3 )
				{//let's resurrect him!
					pc_other->playAction(0x10);
					pc->resurrect();
					staticeffect(cc, 0x37, 0x6A, 0x09, 0x06);
					switch(RandomNum(0, 4))
					{
					case 0: pc_other->talkAll( TRANSLATE("Thou art dead, but 'tis within my power to resurrect thee.  Live!"),0); break;
					case 1: pc_other->talkAll( TRANSLATE("Allow me to resurrect thee ghost.  Thy time of true death has not yet come."),0); break;
					case 2: pc_other->talkAll( TRANSLATE("Perhaps thou shouldst be more careful.  Here, I shall resurrect thee."),0); break;
					case 3: pc_other->talkAll( TRANSLATE("Live again, ghost!  Thy time in this world is not yet done."),0); break;
					case 4: pc_other->talkAll( TRANSLATE("I shall attempt to resurrect thee."),0); break;
					}
				}
				else
				{//if dist>3
					pc_other->talkAll( TRANSLATE("Come nearer, ghost, and i'll give you life!"),1);
				}
			}
			else
			{//if a bad guy
				pc_other->talkAll( TRANSLATE("I will not give life to a scoundrel like thee!"),1);
			}
		}
		else if( pc_other->npcaitype == NPCAI_EVILHEALER )
		{
			if( pc->IsMurderer())
			{
				if ( pc->distFrom( pc_other ) <=3 )
				{//let's resurrect him!
					pc_other->playAction(0x10);
					pc->resurrect();
					staticeffect(cc, 0x37, 0x09, 0x09, 0x19); //Flamestrike effect
					switch(rand()%5)
					{
						case 0: pc_other->talkAll( TRANSLATE("Fellow minion of Mondain, Live!!"),0); break;
						case 1: pc_other->talkAll( TRANSLATE("Thou has evil flowing through your vains, so I will bring you back to life."),0); break;
						case 2: pc_other->talkAll( TRANSLATE("If I res thee, promise to raise more hell!."),0); break;
						case 3: pc_other->talkAll( TRANSLATE("From hell to Britannia, come alive!."),0); break;
						case 4: pc_other->talkAll( TRANSLATE("Since you are Evil, I will bring you back to consciouness."),0); break;
					}
				}
				else
				{//if dist >3
					pc_other->talkAll( TRANSLATE("Come nearer, evil soul, and i'll give you life!"),1);
				}
			}
			else
			{//if player is a good guy
				pc_other->talkAll( TRANSLATE("I dispise all things good. I shall not give thee another chance!"),1);
			}
		}
		else
		{
			sysmessage(s,TRANSLATE("You are dead and cannot do that."));
		}//npcaitype check
	}
	else
	{//if this not a npc but a player
		if(SrvParms->persecute)
		{//start persecute stuff - AntiChrist
			pc->targserial = pc_other->getSerial32();
			Skills::Persecute(s);
		}
		else
		{
			sysmessage(s,TRANSLATE("You are dead and cannot do that."));
		}
	}//if npc

}

void RcvAttack(NXWCLIENT ps)
{

	if ( ps == NULL )
			return;
	NXWSOCKET  s=ps->toInt();
	P_CHAR pc = ps->currChar();
	VALIDATEPC( pc );
	P_CHAR victim=pointers::findCharBySerPtr(buffer[s] +1);
	VALIDATEPC( victim );

	if( pc->dead )
			deadattack( s, DEREF_P_CHAR(victim) );
	else
		if( pc->jailed )
			sysmessage(s,TRANSLATE("There is no fighting in the jail cells!"));
		else
			AttackStuff(s,victim);



}



/*** Xan : this function is critical, and *SHOULD* be used everytime
 *** an attack request is made, not only for dblclicks in war mode
 ***/

void AttackStuff(NXWSOCKET  s, P_CHAR victim)
{
	if ( s < 0 || s >= now )
		return;

	P_CHAR attacker = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC( attacker );
	VALIDATEPC( victim );

	if( attacker->getSerial32() == victim->getSerial32() )
		return;

	
	if ( attacker->amxevents[EVENT_CHR_ONBEGINATTACK]) {
		g_bByPass = false;
		attacker->amxevents[EVENT_CHR_ONBEGINATTACK]->Call( attacker->getSerial32(), victim->getSerial32() );
		if (g_bByPass==true) return;
	}

	if ( victim->amxevents[EVENT_CHR_ONBEGINDEFENSE]) {
		g_bByPass = false;
		victim->amxevents[EVENT_CHR_ONBEGINDEFENSE]->Call( victim->getSerial32(), attacker->getSerial32() );
		if (g_bByPass==true) return;
	}
	/*
	attacker->runAmxEvent( EVENT_CHR_ONBEGINATTACK, attacker->getSerial32(), victim->getSerial32() );
	if (g_bByPass==true)
		return;
	victim->runAmxEvent( EVENT_CHR_ONBEGINDEFENSE, victim->getSerial32(), attacker->getSerial32() );
	if (g_bByPass==true)
		return;
	*/
	attacker->targserial=victim->getSerial32();
	attacker->unHide();
	attacker->disturbMed();

	if( victim->dead || victim->hp <= 0 )//AntiChrist
	{
		attacker->sysmsg( TRANSLATE("That person is already dead!") );
		return;
	}

	if ( victim->npcaitype==NPCAI_PLAYERVENDOR)
	{
		attacker->sysmsg( TRANSLATE("%s cannot be harmed."), victim->getCurrentNameC() );
		return;
	}

	SndAttackOK(s, victim->getSerial32());	//keep the target highlighted
	if (!( victim->targserial== INVALID))
	{
		victim->attackerserial=attacker->getSerial32();
		victim->ResetAttackFirst();
	}
	attacker->SetAttackFirst();
	attacker->attackerserial=victim->getSerial32();

	if( victim->guarded )
	{
		NxwCharWrapper sc;
		sc.fillOwnedNpcs( victim, false, false );
		for ( sc.rewind(); !sc.isEmpty(); sc++ )
		{
			P_CHAR guard = sc.getChar();
			if ( ISVALIDPC( guard ) )
				if ( guard->npcaitype == NPCAI_PETGUARD && ( attacker->distFrom( guard )<= 10 ) )
					npcattacktarget(attacker, guard);
		}
	}

	if ((region[ victim->region].priv & RGNPRIV_GUARDED) && (SrvParms->guardsactive))
	{
		if (victim->IsGrey())
			attacker->SetGrey();

		if (victim->npc==0 && victim->IsInnocent() && (!victim->IsGrey()) && Guilds->Compare( attacker, victim )==0) //REPSYS
		{
			criminal( attacker );
			if (ServerScp::g_nInstantGuard==1)
				npcs::SpawnGuard(attacker, victim,attacker->getPosition() );
		}
		else if( victim->npc && victim->IsInnocent() && !victim->HasHumanBody() && victim->npcaitype!=NPCAI_TELEPORTGUARD )
		{
			criminal( attacker );
			if (ServerScp::g_nInstantGuard==1)
				npcs::SpawnGuard(attacker, victim, attacker->getPosition() );
		}
		else if( victim->npc && victim->IsInnocent() && victim->HasHumanBody() && victim->npcaitype!=NPCAI_TELEPORTGUARD )
		{
			victim->talkAll( TRANSLATE("Help! Guards! I've been attacked!"), 1);
			criminal( victim );
			callguards(DEREF_P_CHAR(victim)); // Sparhawk must check if npcs can call guards
		}
		else if( victim->npc && victim->npcaitype==NPCAI_TELEPORTGUARD)
		{
			criminal( attacker );
			npcattacktarget(victim, attacker);
		}
		else if ((victim->npc || victim->tamed) && !victim->war && victim->npcaitype!=NPCAI_TELEPORTGUARD)
		{
			victim->fight( attacker );
		}
		else
		{
			victim->setNpcMoveTime();
		}
		//attacker->emoteall( "You see %s attacking %s!", 1, attacker->getCurrentNameC(), victim->getCurrentNameC() );
	}
	else	// not a guarded area
	{
		if ( victim->IsInnocent())
		{
			if ( victim->IsGrey())
				attacker->SetGrey();
			if (!victim->npc && (!victim->IsGrey()) && Guilds->Compare(attacker, victim )==0)
			{
				criminal( attacker );
			}
			else if (victim->npc && victim->tamed)
			{
				criminal( attacker );
				npcattacktarget(victim, attacker);
			}
			else if (victim->npc)
			{
				criminal( attacker );
				npcattacktarget(victim, attacker);
				if (victim->HasHumanBody() )
				{
					victim->talkAll(TRANSLATE("Help! Guards! Tis a murder being commited!"), 1);
				}
			}
		}
	}

}

void profileStuff( NXWCLIENT ps, cPacketCharProfileReq& p )
{
	
	if( ps==NULL ) return;
	P_CHAR pc= ps->currChar();
	VALIDATEPC( pc );
	
	P_CHAR who=pointers::findCharBySerial( p.chr.get() );
	VALIDATEPC( who );

	if( p.update ) { //update profile
		if( ( who->getSerial32()!=pc->getSerial32() ) && !pc->IsGMorCounselor() ) 
			return; //lamer fix
		who->profile = p.profile;
	}
	else { //only send
		cPacketCharProfile resp;
		resp.chr=p.chr;
		resp.title+= who->getCurrentName();
		resp.staticProfile = who->staticProfile;
		resp.profile = &who->profile;
		resp.send( ps );

	}
					
}

