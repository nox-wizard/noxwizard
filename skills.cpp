  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "skills.h"
#include "basics.h"
#include "itemid.h"
#include "sndpkg.h"
#include "srvparms.h"
#include "amx/amxcback.h"
#include "race.h"
#include "magic.h"
#include "network.h"
#include "tmpeff.h"
#include "addmenu.h"
#include "scp_parser.h"
#include "set.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "nox-wizard.h"
#include "scripts.h"

//<Luxor>: for skills implementation by small
/*
#undef AMXTASTEID
#define AMXTASTEID "__nxw_sk_tasteid"
#undef AMXANATOMY
#define AMXANATOMY "__nxw_sk_anatomy"
#undef AMXEVALINT
#define AMXEVALINT "__nxw_sk_evint"
*/
#undef AMX_SKILLS_MAIN
#define AMX_SKILLS_MAIN "__nxw_sk_main"
//</Luxor>

#include "debug.h"

//int goldsmithing;
//1=iron, 2=golden, 3=agapite, 4=shadow, 5=mythril, 6=bronze, 7=verite, 8=merkite, 9=copper, 10=silver
int ingottype=0;//will hold number of ingot type to be deleted

inline void SetSkillDelay(CHARACTER cc) 
{ 
	P_CHAR pc_cc=MAKE_CHAR_REF(cc);
	VALIDATEPC(pc_cc);
	SetTimerSec(&pc_cc->skilldelay,SrvParms->skilldelay);
}


/*!
\author Luxor
\brief Implements the hiding skill
\param s socket requesting hiding
*/
void Skills::Hide(NXWSOCKET s)
{
	if ( s < 0 || s >= now )
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	NxwCharWrapper sc;
	P_CHAR pj = NULL;
	sc.fillCharsNearXYZ( pc->getPosition(), 4 );
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		pj = sc.getChar();
		if ( ISVALIDPC(pj) && pj->getSerial32() != pc->getSerial32() && !pj->IsHidden() && pc->losFrom(pj) ) {
			pc->sysmsg( TRANSLATE("There is someone nearby who prevents you to hide.") );
			return;
		}
	}

	pc->hideBySkill();
}

/*!
\author AntiChrist
\brief Stealths a char
\param s socket requesting stealthing
*/
void Skills::Stealth(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    if ( (pc->isMounting()) && (ServerScp::g_nStealthOnHorse==0) ) {
        sysmessage(s,TRANSLATE("You can't stealth on horse!"));
        return;
    }

    if (pc->hidden==0) {
        sysmessage(s,TRANSLATE("You must hide first."));
        return;
    }
    
    if (pc->skill[HIDING]<800)
    {
        sysmessage(s,TRANSLATE("You are not hidden well enough. Become better at hiding."));
        pc->stealth = INVALID;
	pc->unHide();
        return;
    }

    //XAN : more osi-like

    int def = pc->calcDef(0);
    int min = 0, max = 1000;
    if (def <= 5) { min = 0; max = 650; }
    if (def == 6) { min = 450; max = 700; }
    if (def == 7) { min = 500; max = 750; }
    if (def == 8) { min = 550; max = 800; }
    if (def == 9) { min = 600; max = 850; }
    if (def == 10) { min = 650; max = 900; }
    if (def == 11) { min = 650; max = 900; }
    if (def == 12) { min = 700; max = 950; }
    if (def == 13) { min = 700; max = 950; }
    if (def == 14) { min = 750; max = 1000; }
    if (def >= 15) { min = 800; max = 1100; }

    if (ServerScp::g_nStealthArLimit==0) {
        def = 0;
        min = 0;
        max = 1000;
    }
    // do not invert the two parts of the || operator otherwise
    // it stops raising skills when training with plate-armor ! :)
    if ((!pc->checkSkill(STEALTH, min, max))||(def>20))
    {
        pc->hidden = UNHIDDEN;
        pc->stealth = INVALID;
	pc->teleport( TELEFLAG_SENDWORNITEMS );
        return;
    }
    
    sysmessage(s,TRANSLATE("You can move %i steps unseen."), ((SrvParms->maxstealthsteps*pc->skill[STEALTH])/1000) );
    pc->stealth = 0; //AntiChrist -- init. steps already done
    pc->hideBySkill();
}

void Skills::PeaceMaking(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    int inst = Skills::GetInstrument( s );
    if( inst == INVALID )
    {
        pc->sysmsg( TRANSLATE("You do not have an instrument to play on!"));
        return;
    }

    if ( pc->checkSkill( PEACEMAKING, 0, 1000) && pc->checkSkill( MUSICIANSHIP, 0, 1000) )
    {
		Skills::PlayInstrumentWell(s, inst);
        pc->sysmsg( TRANSLATE("You play your hypnotic music, stopping the battle."));

		NxwCharWrapper sc;
		sc.fillCharsNearXYZ( pc->getPosition(), VISRANGE, true, false );
		for( sc.rewind(); !sc.isEmpty(); sc++ ) {
			P_CHAR pcm = sc.getChar();
			if( ISVALIDPC( pcm ) ) {
				if (pcm->war && pc->getSerial32()!=pcm->getSerial32())
                {
                    pcm->sysmsg(TRANSLATE("You hear some lovely music, and forget about fighting."));
					if (pcm->war) 
						pcm->toggleCombat();
                    pcm->targserial = INVALID;
					pcm->attackerserial = INVALID;
					pcm->ResetAttackFirst();
                }
            }
        }
    } 
	else
    {
		Skills::PlayInstrumentPoor(s, inst);
        pc->sysmsg( TRANSLATE("You attempt to calm everyone, but fail."));
    }

}

void Skills::PlayInstrumentWell(NXWSOCKET s, int i)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPI(pi);

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    switch(pi->getId())
    {
    case 0x0E9C:    pc->playSFX( 0x0038);  break;
    case 0x0E9D:
    case 0x0E9E:    pc->playSFX( 0x0052);  break;
    case 0x0EB1:
    case 0x0EB2:    pc->playSFX( 0x0045);  break;
    case 0x0EB3:
    case 0x0EB4:    pc->playSFX( 0x004C);  break;
    default:
        LogError("switch reached default");
    }

}

void Skills::PlayInstrumentPoor(NXWSOCKET s, int i)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPI(pi);

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    switch(pi->getId())
    {
    case 0x0E9C:    pc->playSFX( 0x0039);  break;
    case 0x0E9D:
    case 0x0E9E:    pc->playSFX( 0x0053);  break;
    case 0x0EB1:
    case 0x0EB2:    pc->playSFX( 0x0046);  break;
    case 0x0EB3:
    case 0x0EB4:    pc->playSFX( 0x004D);  break;
    default:
        LogError("switch reached default");
    }

}

int Skills::GetInstrument(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return INVALID;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pc,INVALID);
    
	P_ITEM pack= pc->getBackpack();
    VALIDATEPIR(pack,INVALID);

    NxwItemWrapper si;
	si.fillItemsInContainer( pack, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {
        P_ITEM pi=si.getItem();
		if ( ISVALIDPI(pi) && pi->IsInstrument() )
            return DEREF_P_ITEM(pi);
	}

    return INVALID;

}

/*!
\author Duke
\date 20/04/2000
\brief Helper function for DoPotion()
\param s socket of the crafter
\param regid reagent identifier
\param regamount amount of reagents
\param regname name of the reagent

checks if player has enough regs for selected potion and delets them
*/
static bool DoOnePotion(NXWSOCKET s, UI16 regid, UI32 regamount, char* regname)
{
	if ( s < 0 || s >= now ) //Luxor
		return false;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pc,false);
    
    bool success=false;
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    if (pc->getAmount(regid) >= regamount)
    {
        success=true;
        sprintf(temp, TRANSLATE("*%s starts grinding some %s in the mortar.*"), pc->getCurrentNameC(), regname);
        pc->emoteall( temp,1); // LB, the 1 stops stupid alchemy spam
        delequan(DEREF_P_CHAR(pc),regid,regamount);
    }
    else
        sysmessage(s, TRANSLATE("You do not have enough reagents for that potion."));

    return success;

}

/*!
\author Duke
\brief Determines regs and quantity, creates working sound
indirectly calls CreatePotion() on success
*/
void Skills::DoPotion(NXWSOCKET s, SI32 type, SI32 sub, P_ITEM pi_mortar)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	VALIDATEPI(pi_mortar);

	bool success=false;

	switch((type*10)+sub)
	{
		case 11: success=DoOnePotion(s,0x0F7B, 1,"blood moss");     break;//agility
		case 12: success=DoOnePotion(s,0x0F7B, 3,"blood moss");     break;//greater agility
		case 21: success=DoOnePotion(s,0x0F84, 1,"garlic");         break;//lesser cure
		case 22: success=DoOnePotion(s,0x0F84, 3,"garlic");         break;//cure
		case 23: success=DoOnePotion(s,0x0F84, 6,"garlic");         break;//greater cure
		case 31: success=DoOnePotion(s,0x0F8C, 3,"sulfurous ash");  break;//lesser explosion
		case 32: success=DoOnePotion(s,0x0F8C, 5,"sulfurous ash");  break;//explosion
		case 33: success=DoOnePotion(s,0x0F8C,10,"sulfurous ash");  break;//greater explosion
		case 41: success=DoOnePotion(s,0x0F85, 1,"ginseng");        break;//lesser heal
		case 42: success=DoOnePotion(s,0x0F85, 3,"ginseng");        break;//heal
		case 43: success=DoOnePotion(s,0x0F85, 7,"ginseng");        break;//greater heal
		case 51: success=DoOnePotion(s,0x0F8D, 1,"spider's silk");  break;//night sight
		case 61: success=DoOnePotion(s,0x0F88, 1,"nightshade");     break;//lesser poison
		case 62: success=DoOnePotion(s,0x0F88, 2,"nightshade");     break;//poison
		case 63: success=DoOnePotion(s,0x0F88, 4,"nightshade");     break;//greater poison
		case 64: success=DoOnePotion(s,0x0F88, 8,"nightshade");     break;//deadly poison
		case 71: success=DoOnePotion(s,0x0F7A, 1,"black pearl");    break;//refresh
		case 72: success=DoOnePotion(s,0x0F7A, 5,"black pearl");    break;//total refreshment
		case 81: success=DoOnePotion(s,0x0F86, 2,"mandrake");       break;//strength
		case 82: success=DoOnePotion(s,0x0F86, 5,"mandrake");       break;//greater strength
		default:
			 LogError("switch reached default",(type*10)+sub);
			 return;
	}
    
	if (success)
	{
		tempfx::add(pc, pc, tempfx::ALCHEMY_GRIND, 0, 0, 0); // make grinding sound for a while
		tempfx::add(pc, pc, tempfx::ALCHEMY_GRIND, 0, 3, 0);
		tempfx::add(pc, pc, tempfx::ALCHEMY_GRIND, 0, 6, 0);
		tempfx::add(pc, pc, tempfx::ALCHEMY_GRIND, 0, 9, 0);
		tempfx::add(pc, pi_mortar, tempfx::ALCHEMY_END, type, sub, 0);  // this will indirectly call CreatePotion()
	}
}

/*!
\author Duke
\brief Does the appropriate skillcheck for the potion, creats it
in the mortar on success and tries to put it into a bottle
\param s pointer to the character crafter
\param type type of potion
\param sub subtype of potion
\param mortar serial of the mortar
*/
void Skills::CreatePotion(CHARACTER s, char type, char sub, int mortar)
{
	P_CHAR pc=MAKE_CHAR_REF(s);
	VALIDATEPC(pc);

	NXWCLIENT ps=pc->getClient();
	if( ps==NULL )	return;

	P_ITEM pi_mortar=MAKE_ITEM_REF(mortar);
	VALIDATEPI(pi_mortar);


	int success=0;

	switch((10*type)+sub)
	{
		case 11:success=pc->checkSkill( ALCHEMY,151, 651);break;//agility
		case 12:success=pc->checkSkill( ALCHEMY,351, 851);break;//greater agility
		case 21:success=pc->checkSkill( ALCHEMY,  0, 500);break;//lesser cure
		case 22:success=pc->checkSkill( ALCHEMY,251, 751);break;//cure
		case 23:success=pc->checkSkill( ALCHEMY,651,1151);break;//greater cure
		case 31:success=pc->checkSkill( ALCHEMY, 51, 551);break;//lesser explosion
		case 32:success=pc->checkSkill( ALCHEMY,351, 851);break;//explosion
		case 33:success=pc->checkSkill( ALCHEMY,651,1151);break;//greater explosion
		case 41:success=pc->checkSkill( ALCHEMY,  0, 500);break;//lesser heal
		case 42:success=pc->checkSkill( ALCHEMY,151, 651);break;//heal
		case 43:success=pc->checkSkill( ALCHEMY,551,1051);break;//greater heal
		case 51:success=pc->checkSkill( ALCHEMY,  0, 500);break;//night sight
		case 61:success=pc->checkSkill( ALCHEMY,  0, 500);break;//lesser poison
		case 62:success=pc->checkSkill( ALCHEMY,151, 651);break;//poison
		case 63:success=pc->checkSkill( ALCHEMY,551,1051);break;//greater poison
		case 64:success=pc->checkSkill( ALCHEMY,901,1401);break;//deadly poison
		case 71:success=pc->checkSkill( ALCHEMY,  0, 500);break;//refresh
		case 72:success=pc->checkSkill( ALCHEMY,251, 751);break;//total refreshment
		case 81:success=pc->checkSkill( ALCHEMY,251, 751);break;//strength
		case 82:success=pc->checkSkill( ALCHEMY,451, 951);break;//greater strength

		default:
			LogError("switch reached default");
			return;
	}

	if (success==0 && !pc->IsGM()) // AC bugfix
	{
		pc->emoteall(TRANSLATE("*%s tosses the failed mixture from the mortar, unable to create a potion from it.*"),0, pc->getCurrentNameC());
		return;
	}

	pi_mortar->type=17;
	pi_mortar->more1=type;
	pi_mortar->more2=sub;
	pi_mortar->morex=pc->skill[ALCHEMY];

	if (pc->getAmount(0x0F0E)<1)
	{
		P_TARGET targ = clientInfo[s]->newTarget( new cItemTarget() );
		targ->code_callback=Skills::target_bottle;
		targ->buffer[0]=pi_mortar->getSerial32();
		targ->send( ps );
		ps->sysmsg( TRANSLATE("Where is an empty bottle for your potion?"));
	}
	else
	{
		pc->playSFX(0x0240); // Liquid sfx
		pc->emoteall(TRANSLATE("*%s pours the completed potion into a bottle.*"), 0, pc->getCurrentNameC());
		delequan(DEREF_P_CHAR(pc), 0x0F0E, 1);
		Skills::PotionToBottle(pc, pi_mortar);
	}
}

/*!
\author Duke
\brief Uses the targeted potion bottle <b>outside</b> the backpack to
pour in the potion from the mortar
\param s socket of the crafter
*/
void Skills::target_bottle( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	NXWSOCKET s = ps->toInt();
    
	if(pi->magic==4) 
		return;    // Ripper

	if (pi->getId()==0x0F0E)   // an empty potion bottle ?
	{
		pi->ReduceAmount(1);
		
		P_ITEM pi_mortar=pointers::findItemBySerial( t->buffer[0] );
		VALIDATEPI(pi_mortar);

		if (pi_mortar->type==17)
		{
			pc->emoteall(TRANSLATE("*%s pours the completed potion into a bottle.*"), 0, pc->getCurrentNameC());
			Skills::PotionToBottle(pc, pi_mortar);
		}
	}
	else
		sysmessage(s,TRANSLATE("This is not an appropriate container for a potion."));
}

#define CREATEINBACKPACK( ITEM ) pi = item::CreateFromScript( ITEM, pc->getBackpack() );

/*!
\author Endymion
\brief This really creates the potion
\param pc pointer to the crafter's character
\param pi_mortar pointer to the mortar's item
*/
void Skills::PotionToBottle( P_CHAR pc, P_ITEM pi_mortar )
{
	VALIDATEPC(pc);

	NXWCLIENT ps=pc->getClient();
	if( ps==NULL ) return;

	VALIDATEPI(pi_mortar);

	P_ITEM pi=NULL;

	int potionType= (10*pi_mortar->more1)+pi_mortar->more2;

	switch( potionType )    {
		case 11: CREATEINBACKPACK( "$normal_agility_potion" )		break;
		case 12: CREATEINBACKPACK( "$greater_agility_potion" )		break;

		case 21: CREATEINBACKPACK( "$lesser_cure_potion" )		break;
		case 22: CREATEINBACKPACK( "$normal_cure_potion" )		break;
		case 23: CREATEINBACKPACK( "$greater_cure_potion" )		break;

		case 31: CREATEINBACKPACK( "$lesser_explosion_potion" )		break;
		case 32: CREATEINBACKPACK( "$normal_explosion_potion" )		break;
		case 33: CREATEINBACKPACK( "$greater_explosion_potion" )	break;

		case 41: CREATEINBACKPACK( "$lesser_healing_potion" )		break;
		case 42: CREATEINBACKPACK( "$normal_healing_potion" )		break;
		case 43: CREATEINBACKPACK( "$greater_healing_potion" )		break;

		case 51: CREATEINBACKPACK( "$normal_nightsight_potion" )	break;

		case 61: CREATEINBACKPACK( "$lesser_poison_potion" )		break;
		case 62: CREATEINBACKPACK( "$normal_poison_potion" )		break;
		case 63: CREATEINBACKPACK( "$greater_poison_potion" )		break;
		case 64: CREATEINBACKPACK( "$deadly_poison_potion" )		break;

		case 71: CREATEINBACKPACK( "$normal_energy_potion" )		break;
		case 72: CREATEINBACKPACK( "$greater_energy_potion" )		break;

		case 81: CREATEINBACKPACK( "$normal_strength_potion" )		break;
		case 82: CREATEINBACKPACK( "$greater_strength_potion" )		break;
		default: 
			LogError("switch reached default into PotionToBottle");
			return;
	}

	VALIDATEPI(pi);

    
	if(!pc->IsGM())
    
	{
		pi->creator = pc->getCurrentName();

		if (pc->skill[ALCHEMY]>950) 
			pi->madewith=ALCHEMY+1;
		else 
			pi->madewith=0-ALCHEMY-1;
    
	} else  {
		pi->creator = "";
		pi->madewith=0;
	}

    pi->Refresh();
    pi_mortar->type=0;
}

char Skills::CheckSkillSparrCheck(int c, unsigned short int sk, int low, int high, P_CHAR pcd)
{
    P_CHAR pc=MAKE_CHAR_REF(c);
	VALIDATEPCR(pc, 0);

    bool bRaise = false;
    if (pcd->npc) bRaise = true;
    if (ServerScp::g_nLimitPlayerSparring==0) bRaise = true;
    if (pcd->skill[sk] > (pc->skill[sk]+ServerScp::g_nLimitPlayerSparring)) bRaise = true;

    return pc->checkSkill( static_cast<Skill>(sk), low, high, bRaise);
}

char Skills::AdvanceSkill(CHARACTER s, int sk, char skillused)
{
	if ( sk < 0 || sk >= TRUESKILLS ) //Luxor
		return 0;

    P_CHAR pc = MAKE_CHAR_REF(s);
	VALIDATEPCR(pc,0);

    int a,ges=0,d=0;
    unsigned char lockstate;
    int skillcap = SrvParms->skillcap;
    UI32 incval;
    int atrophy_candidates[ALLSKILLS+1];


    
    if (pc->amxevents[EVENT_CHR_ONGETSKILLCAP])
        skillcap = pc->amxevents[EVENT_CHR_ONGETSKILLCAP]->Call(pc->getSerial32(), pc->getSocket() );
	/*
	if ( pc->getAmxEvent(EVENT_CHR_ONGETSKILLCAP) != NULL )
		skillcap = pc->runAmxEvent( EVENT_CHR_ONGETSKILLCAP, pc->getSerial32(), pc->getSocket() );
	*/

    lockstate=pc->lockSkill[sk];
    if (pc->IsGM()) lockstate=0;
    // for gms no skill cap exists, also ALL skill will be interperted as up, no matter how they are set

    if (lockstate==2 || lockstate==1) return 0;// skill is locked -> forget it

    // also NOthing happens if you train a skill marked for atrophy !!!
    // skills only fall if others raise, ( osi quote ), so my interpretion
    // is that those marked as falling cant fall if you use them directly
    // exception: if you are gm its ignored!

    int c=0;

    for (int b=0;b<(ALLSKILLS+1);b++)
    {
        if (pc->lockSkill[b]==1 && pc->baseskill[b]!=0) // only count atrophy candidtes if they are above 0 !!!
        {
            atrophy_candidates[c]=b;
            c++;
        }
    }

    if (!pc->IsGM())
    {
        for (a=0;a<ALLSKILLS;a++)
        {
            ges+=pc->baseskill[a];
        }
        ges=ges/10;

        if (ges>skillcap && c==0) // skill capped and no skill is marked as fall down.
        {
            pc->sysmsg(TRANSLATE("You have reached the skill-cap of %i and no skill can fall!"), skillcap);
            return 0;
        }

    } else ges=0;

    //
    // Skill advance parameter
    //
    // 1. If the race system is active get them from there
    // 2. Else use the standard server tables
    //
    if( Race::isRaceSystemActive() )
    {
		if ( skillused )
		{
			Race* r = Race::getRace( (UI32) pc->race );
			if( r!=NULL ) 
				incval = r->getSkillAdvanceSuccess( (UI32) sk, (UI32) pc->baseskill[sk] );
			else 
				incval=0;
			//SDbgOut("Race advance success for skill %d with base %d is %d\n", sk, pc->baseskill[sk], incval * 10 );
		}
		else
		{
			Race* r = Race::getRace( (UI32) pc->race );
			if( r!=NULL )
				incval = r->getSkillAdvanceFailure( (UI32) sk, (UI32) pc->baseskill[sk] );
			else 
				incval =0;

			//SDbgOut("Race advance failure for skill %d with base %d is %d\n", sk, pc->baseskill[sk], incval * 10 );
		}
		incval *= 10;
    }
    else {
    
	int i = 0;
	int loopexit=0;
    	while ( (wpadvance[1+i+skillinfo[sk].advance_index].skill == sk &&
      	         wpadvance[1+i+skillinfo[sk].advance_index].base  <= pc->baseskill[sk] ) &&
    	     	  (++loopexit < MAXLOOPS) ) ++i;
	if(skillused)
   		incval=(wpadvance[i+skillinfo[sk].advance_index].success);
	else
		incval=(wpadvance[i+skillinfo[sk].advance_index].failure);
	incval *= 10;
    }

    int retval=0;
    if (incval>rand()%SrvParms->skilladvancemodifier)
    {
        retval=1;
    }

    
    if(pc->amxevents[EVENT_CHR_ONADVANCESKILL]!=NULL) {
        g_bByPass = false;
        retval = pc->amxevents[EVENT_CHR_ONADVANCESKILL]->Call(pc->getSerial32(), sk, skillused, retval);
        if (g_bByPass==true) return retval;
    }
	/*
	if ( pc->getAmxEvent(EVENT_CHR_ONADVANCESKILL) != NULL ) {
		retval = pc->runAmxEvent( EVENT_CHR_ONADVANCESKILL, pc->getSerial32(), sk, skillused, retval);
		if (g_bByPass==true)
			return retval;
	}
	*/
    if (retval)
    {
		pc->baseskill[sk]++;
        // no atrophy for gm's !!
        if (ges>skillcap) // atrophy only if cap is reached !!!
        // if we are above the skill cap -> we have to let the atrophy candidates fall
        // important: we have to let 2 skills fall, or we'll never go down to cap
        // (especially if we are far above the cap from previous verisons)
        {
            if (c==1)
            {
                d = (pc->baseskill[atrophy_candidates[0]]>=2)? 2 : 1; // avoid value below 0 (=65535 cause unsigned)
                {
                    if (d==1 && pc->baseskill[atrophy_candidates[0]]==0) 
						d=0; // should never happen ...
                    pc->baseskill[atrophy_candidates[0]]-=d;
					Skills::updateSkillLevel(pc, atrophy_candidates[0]);         // we HAVE to correct the skill-value
                    updateskill(pc->getSocket(), atrophy_candidates[0]); // and send changed skill values packet so that client can re-draw correctly
                }
            // this is very important cauz this is ONLY done for the calling skill value automatically .
            }
            else
            {
                for( int vol=0; vol<2; vol++ ) {

					d = ( c != 0)? rand()%c : 0;

					if (pc->baseskill[atrophy_candidates[d]]>=1)
					{
	                    pc->baseskill[atrophy_candidates[d]]--;
						Skills::updateSkillLevel(pc, atrophy_candidates[d]);
						updateskill(pc->getSocket(), atrophy_candidates[d]);
					}

                }
            }
        }
        if (ServerScp::g_nStatsAdvanceSystem == 0)
			Skills::AdvanceStats(s, sk);
    }
    if (ServerScp::g_nStatsAdvanceSystem == 1)
		Skills::AdvanceStats(s, sk);
    //AMXEXECS(s,AMXT_SPECIALS, 5, retval, AMX_AFTER);
    return retval;


}

/*!
\author Duke
\date 21/03/2000
\brief Little helper function for cSkills::AdvanceStats()

finds the appropriate line for the used skill in advance table
and uses the value of that skill (!) to increase the stat
and cuts it down to 100 if necessary

\param sk skill identifier
\param stat stat identifier
\param pc pointer to character to advance the stats to
\todo document missing paramteres
*/
static int AdvanceOneStat(UI32 sk, int i, char stat, bool *update, int type, P_CHAR pc)
{
	if ( sk < 0 || sk >= TRUESKILLS ) //Luxor
		return 0;

	int loopexit=0, limit=1000;
	*update = false;
	SI32 tmp;

	int stat2update1, stat2update2;
	int stat2update;

	switch( stat )
	{
		case 'S': tmp= pc->st3;	break;
		case 'D': tmp= pc->dx3;	break;
		case 'I': tmp= pc->in3;	break;
	}

	if( Race::isRaceSystemActive() )
		stat2update1 = Race::getRace( (UI32) pc->race )->getSkillAdvanceSuccess( sk, tmp*10 );

	while ((wpadvance[i+1].skill==sk) &&     // if NEXT line is for same skill and is not higher than our stat then proceed to it !
     		(wpadvance[i+1].base<=(tmp*10)) && (++loopexit < MAXLOOPS) )
  	      	i++;

	stat2update2 = wpadvance[i].success;         // gather small increases

	//SDbgOut("AdvanceOneStat() skill %d base %d succes %d %d\n", sk, (*stat)*10, stat2update1, stat2update2);

	if( Race::isRaceSystemActive() )
		stat2update = stat2update1;
	else
		stat2update = stat2update2;

	switch( stat )
	{
		case 'S':	pc->st2+= stat2update;
					tmp= pc->st2;
					break;

		case 'D':	pc->dx2+= stat2update;
					tmp= pc->dx2;
					break;

		case 'I':	pc->in2+= stat2update;
					tmp= pc->in2;
					break;
	}

	if ( tmp >= 1000)           // until they reach 1000
	{
		switch( stat )
		{
			case 'S':	pc->st2-= 1000;	if (pc->st2 < 0) pc->st2 = 0; tmp= pc->st3; break;
			case 'D':	pc->dx2-= 1000; if (pc->dx2 < 0) pc->dx2 = 0; tmp= pc->dx3; break;
			case 'I':	pc->in2-= 1000;	if (pc->in2 < 0) pc->in2 = 0; tmp= pc->in3; break;
		}
 //       *stat2 -= 1000;                     // then change it

 	
    if (pc->amxevents[EVENT_CHR_ONADVANCESTAT]) {
        g_bByPass = false;
        pc->amxevents[EVENT_CHR_ONADVANCESTAT]->Call(pc->getSerial32(), type, sk, tmp);
        if (g_bByPass==true) return false;
	}
	
	//pc->runAmxEvent( EVENT_CHR_ONADVANCESTAT, pc->getSerial32(), type, sk, tmp);
 	if (g_bByPass==true)
		return false;

		if( Race::isRaceSystemActive() )
		{
			switch( type )
			{
				case STATCAP_CAP:
					limit = Race::getRace( (UI32) pc->race )->getStatCap();
					SDbgOut("AdvanceOneStat() race %d %s statcap %d\n", pc->race, Race::getName( pc->race )->c_str(), limit );
					break;
				case STATCAP_STR:
					limit = Race::getRace( (UI32) pc->race )->getStrCap();
					SDbgOut("AdvanceOneStat() race %d %s strcap %d\n", pc->race, Race::getName( pc->race )->c_str(), limit );
					break;
				case STATCAP_DEX:
					limit = Race::getRace( (UI32) pc->race )->getDexCap();
					SDbgOut("AdvanceOneStat() race %d %s dexcap %d\n", pc->race, Race::getName( pc->race )->c_str(), limit );
					break;
				case STATCAP_INT:
					limit = Race::getRace( (UI32) pc->race )->getIntCap();
					SDbgOut("AdvanceOneStat() race %d %s intcap %d\n", pc->race, Race::getName( pc->race )->c_str(), limit );
					break;
			}
		}
		else
		{
			limit = 100;
		}

    if (pc->amxevents[EVENT_CHR_ONGETSTATCAP]!=NULL)
       	limit = pc->amxevents[EVENT_CHR_ONGETSTATCAP]->Call(pc->getSerial32(), type, limit);
	
	/*
	if ( pc->getAmxEvent(EVENT_CHR_ONGETSTATCAP) != NULL )
		limit = pc->runAmxEvent( EVENT_CHR_ONGETSTATCAP, pc->getSerial32(), type, limit);
	*/

		switch( stat )
		{
			case 'S':	pc->modifyStrength(1);
						pc->st3++;
						break;

			case 'D':	pc->dx++;
						pc->dx3++;						
						break;

			case 'I':	pc->in++;
						pc->in3++;
						break;
		}
       	*update=true;
    }

	if( !pc->IsGM() )
	{
		switch( stat )
		{
		case 'S':	if(pc->st3 > limit)	{
						pc->st3=limit;
						pc->setStrength(limit);
					}
					break;

		case 'D':	if(pc->dx3 > limit) {
						pc->dx3=limit;
						pc->dx= limit;
					}
					break;

		case 'I':	if(pc->in3 > limit) {
						pc->in3=limit;
						pc->in= limit;
					}
					break;
		}
		*update= true;
	}
	return *update;
}

/*!
\author Duke
\date 21/03/2000
\brief Advance STR, DEX and INT after use of a skill
\param s crafter character
\param sk skill identifier

checks if STR+DEX+INT are higher than statcap from server.cfg
gives all three stats the chance (from skills.scp & server.cfg) to rise
and reduces the two other stats if necessary
*/
void Skills::AdvanceStats(CHARACTER s, int sk)
{
	if ( sk < 0 || sk >= TRUESKILLS ) //Luxor
		return;


	P_CHAR pc = MAKE_CHAR_REF(s);
	VALIDATEPC(pc);

    	// Begin: Determine statcap
    	// 1. get statcap as defined in server.cfg
	// 2. get statcap from racesystem
	// 3. get statcap from char event
	//
	int statcap = SrvParms->statcap;

	if( Race::isRaceSystemActive() )
		statcap = Race::getRace( pc->race )->getStatCap();


	
	if (pc->amxevents[EVENT_CHR_ONGETSTATCAP]!=NULL)
		statcap = pc->amxevents[EVENT_CHR_ONGETSTATCAP]->Call(pc->getSerial32(), STATCAP_CAP, statcap);
	/*
	if ( pc->getAmxEvent(EVENT_CHR_ONGETSTATCAP) != NULL )
		statcap = pc->runAmxEvent( EVENT_CHR_ONGETSTATCAP, pc->getSerial32(), STATCAP_CAP, statcap);
	*/
	// End: Determine statcap

    bool atCap = (pc->st3 + pc->dx3 + pc->in3) > statcap;

    int	i = skillinfo[sk].advance_index;
    int mod	= SrvParms->statsadvancemodifier;
//  int	*pi; // ptr to stat to be decreased
	bool 	update 	= false;

	if ( pc->statGainedToday <= ServerScp::g_nStatDailyLimit )
	{
		bool strCheck = ( Race::isRaceSystemActive() ? Race::getRace( pc->race )->getSkillAdvanceStrength( sk ) : skillinfo[sk].st ) > (UI32)(rand() % mod);
    	bool dexCheck = ( Race::isRaceSystemActive() ? Race::getRace( pc->race )->getSkillAdvanceDexterity( sk ) : skillinfo[sk].dx ) > (UI32)(rand() % mod);
    	bool intCheck = ( Race::isRaceSystemActive() ? Race::getRace( pc->race )->getSkillAdvanceIntelligence( sk ) : skillinfo[sk].in ) > (UI32)(rand() % mod);

       	if ( strCheck )
       		if ( AdvanceOneStat( sk, i, 'S', &update, STATCAP_STR, pc ) && atCap && !pc->IsGM() )
			{
				if( rand()%2 ) {
					pc->dx3--;
					pc->dx--;
				}
				else {
					pc->in3--;
					pc->in--;
				}
			}

		if ( dexCheck )
	       	if ( AdvanceOneStat(sk, i, 'D', &update, STATCAP_DEX, pc ) && atCap && !pc->IsGM() )
			{
				if( rand()%2 ) {
					pc->st3--;
					pc->modifyStrength(-1);
				}
				else {
					pc->in3--;
					pc->in--;
				}
			}

	    if ( intCheck )
	       	if ( AdvanceOneStat(sk, i, 'I', &update, STATCAP_INT, pc ) && atCap && !pc->IsGM() )
			{
				if( rand()%2 ) {
					pc->st3--;
					pc->modifyStrength(-1);
				}
				else {
					pc->dx3--;
					pc->dx--;
				}
			}


    	
		if ( update )
		{
  			
			NXWSOCKET socket = pc->getSocket();

			++pc->statGainedToday;
  
			if ( socket != INVALID )
				statwindow( pc, pc);              // update client's status window

        	
			for ( i = 0;  i < ALLSKILLS; i++ )
				updateSkillLevel(pc,i );     // update client's skill window

			if ( atCap && !pc->IsGM() )
				pc->sysmsg(TRANSLATE("You have reached the stat-cap of %i!") ,statcap );
	        
		}
	}
}

/*!
\brief Spirit speack time on a base of 30 seconds + skill[SPIRITSPEAK]/50 + INT
\param s socket to the character
*/
void Skills::SpiritSpeak(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
    //  Unsure if spirit speaking should they attempt again?
    //  Suggestion: If they attempt the skill and the timer is !0 do not have it raise the skill

    if(!pc->checkSkill(SPIRITSPEAK, 0, 1000))
    {
        sysmessage(s,TRANSLATE("You fail your attempt at contacting the netherworld."));
        return;
    }

    impaction(s,0x11);          // I heard there is no action...but I decided to add one
    pc->playSFX(0x024A);   // only get the sound if you are successful
    sysmessage(s,TRANSLATE("You establish a connection to the netherworld."));
    SetTimerSec(&(pc->spiritspeaktimer),spiritspeak_data.spiritspeaktimer+pc->in);
}

/*!
\brief Skill is clicked on the skill list
\param s socket to the character that used skill
\param x skill identifier
*/
void Skills::SkillUse(NXWSOCKET s, int x)
{
	if ( s < 0 || s >= now || x < 0 || x >= TRUESKILLS) //Luxor
		return;

    NXWCLIENT ps=getClientFromSocket(s);
	if( ps==NULL )
		return;
    
	P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);

	if( (pc->skilldelay>uiCurrentTime) && (!pc->IsGM()) )
	{
		sysmessage(s, TRANSLATE("You must wait a few moments before using another skill."));
		return;
	}

	if ( pc->jailed )
	{
		sysmessage(s,TRANSLATE("you are in jail and cant gain skills here!"));
		return;
	}

	if ( pc->dead )
	{
		sysmessage(s,TRANSLATE("You cannot do that as a ghost."));
		return;
	}
    
	if (pc->IsHiddenBySpell())
		return; //Luxor: cannot use skills if under invisible spell
    /*  chars[cc].unHide();*/
    
	//<Luxor> 7 dec 2001
	if (skillinfo[x].unhide_onuse == 1)
		pc->unHide();
	//</Luxor>
    
	
	if( pc->casting )
	{
		sysmessage( s, TRANSLATE("You can't do that while you are casting" ));
		return;
	}

	pc->disturbMed(); // Meditation
	
	AMXEXECSV( pc->getSerial32(),AMXT_SKILLS, x, AMX_BEFORE);
	
	bool setSkillDelay = true;

	if( Race::isRaceSystemActive() && !(Race::getRace( pc->race )->getCanUseSkill( (UI32) x )) )
	{
		sysmessage(s, TRANSLATE("Your race cannot use that skill") );
		setSkillDelay = false;
	}
	else 
	{
		P_TARGET targ=NULL;
		switch(x)
		{
			case ARMSLORE:
				targ=clientInfo[s]->newTarget( new cItemTarget() );
				targ->code_callback=target_armsLore;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("What item do you wish to get information about?"));
				break;

			case ANATOMY:
				break;

			case ITEMID:
				targ=clientInfo[s]->newTarget( new cItemTarget() );
				targ->code_callback=Skills::target_itemId;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("What do you wish to appraise and identify?"));
				break;
			
			case EVALUATINGINTEL:
				break;
			
			case TAMING:
				targ=clientInfo[s]->newTarget( new cCharTarget() );
				targ->code_callback=target_tame;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("Tame which animal?"));
				break;
			
			case HIDING:
				Skills::Hide(s);
				break;
			
			case STEALTH:
				Skills::Stealth(s);
				break;
			
			case DETECTINGHIDDEN:
				targ=clientInfo[s]->newTarget( new cLocationTarget() );
				targ->code_callback=target_detectHidden;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("Where do you wish to search for hidden characters?"));
				break;
			
			case PEACEMAKING:
				Skills::PeaceMaking(s);
				break;
			
			case PROVOCATION:
				targ=clientInfo[s]->newTarget( new cCharTarget() );
				targ->code_callback=target_provocation1;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("Whom do you wish to incite?"));
				break;
			
			case ENTICEMENT:
				targ=clientInfo[s]->newTarget( new cCharTarget() );
				targ->code_callback=target_enticement1;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("Whom do you wish to entice?"));
				break;
			
			case SPIRITSPEAK:
				Skills::SpiritSpeak(s);
				break;
			
			case STEALING:
				if (SrvParms->rogue) {
					targ=clientInfo[s]->newTarget( new cObjectTarget() );
					targ->code_callback=target_stealing;
					targ->send( ps );
					ps->sysmsg( TRANSLATE("What do you wish to steal?"));
				}
				else
				{
					sysmessage(s, TRANSLATE("That skill has been disabled."));
					setSkillDelay = false;
				}
				break;
			
			case INSCRIPTION:
				break;
			
			case TRACKING:
				break;
			
			case BEGGING:
				targ=clientInfo[s]->newTarget( new cCharTarget() );
				targ->code_callback=Skills::target_begging;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("Whom do you wish to annoy?"));
				break;
			
			case ANIMALLORE:
				targ=clientInfo[s]->newTarget( new cCharTarget() );
				targ->code_callback=Skills::target_animalLore;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("What animal do you wish to get information about?"));
				break;
			
			case FORENSICS:
				targ=clientInfo[s]->newTarget( new cItemTarget() );
				targ->code_callback=Skills::target_forensics;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("What corpse do you want to examine?"));
				break;
			
			case POISONING:
				targ=clientInfo[s]->newTarget( new cItemTarget() );
				targ->code_callback=Skills::target_poisoning;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("What poison do you want to apply?"));
				break;
			
			case TASTEID:
				break;
			
			case MEDITATION:  //Morrolan - Meditation
				//if(SrvParms->armoraffectmana)
					Skills::Meditation(s);
				/*else
				{
					sysmessage(s, TRANSLATE("Meditation is turned off.  Tell your GM to enable ARMOR_AFFECT_MANA_REGEN in server.cfg to enable it."));
					setSkillDelay = false;
				}*/
				break;
			
			case REMOVETRAPS:
				targ=clientInfo[s]->newTarget( new cItemTarget() );
				targ->code_callback=target_removeTraps;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("What do you want to untrap?"));
				break;
			
			case CARTOGRAPHY:
				Skills::Cartography(s);
				break;
			
			default:
				sysmessage(s, TRANSLATE("That skill has not been implemented yet."));
				setSkillDelay = false;
				break;
		}
        //<Luxor>: Complete skills handling by AMX
		AmxFunction::g_prgOverride->CallFn( AmxFunction::g_prgOverride->getFnOrdinal(AMX_SKILLS_MAIN), s, x);
        //</Luxor>
	}
	
	if ( setSkillDelay )
		SetSkillDelay(pc->getSerial32());

	AMXEXECSV( pc->getSerial32(),AMXT_SKILLS, x, AMX_AFTER);
}


/*!
\brief Calculate the skill of this character based on the characters baseskill and stats
\param pc pointer to the character
\param s skill identifier
*/
void Skills::updateSkillLevel(P_CHAR pc, int s)
{
	VALIDATEPC(pc);
	if ( s < 0 || s >= TRUESKILLS ) //Luxor
		return;


    int temp = (((skillinfo[s].st * pc->getStrength()) / 100 +
        (skillinfo[s].dx * pc->dx) / 100 +
        (skillinfo[s].in * pc->in) / 100)
        *(1000-pc->baseskill[s]))/1000+pc->baseskill[s];


    pc->skill[s] = qmax(static_cast<short>(pc->baseskill[s]), static_cast<short>(temp));


}

void Skills::TDummy(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int hit;

	P_ITEM pWeapon = pc->getWeapon();

	if (ISVALIDPI(pWeapon))
	{
		if (pc->getWeapon()->IsBowType())
		{
			sysmessage(s, TRANSLATE("Practice archery on archery buttes !"));
			return;
		}
	}

	int skillused = pc->getCombatSkill();
	
	if (pc->isMounting())
		pc->combatOnHorse();
	else
		pc->combatOnFoot();

	hit=rand()%3;

	switch(hit)
	{
		case 0: pc->playSFX(0x013B);
			break;
    
		case 1: pc->playSFX(0x013C);
			break;

		case 2: pc->playSFX(0x013D);
			break;

		default:
			LogError("switch reached default");
			return;
	}
	
	P_ITEM pj = pointers::findItemBySerial( LongFromCharPtr(buffer[s] +1) & 0x7FFFFFFF );

	if (ISVALIDPI(pj))
	{
		if (pj->getId()==0x1070)
			pj->setId( 0x1071 );
        
		if (pj->getId()==0x1074)
			pj->setId( 0x1075 );
	
		tempfx::add(pc, pj, tempfx::TRAINDUMMY, 0, 0, 0); //Luxor
		pj->Refresh();
	}
	
	if(pc->skill[skillused] < 300)
	{
		pc->checkSkill( static_cast<Skill>(skillused), 0, 1000);

		if(pc->skill[TACTICS] < 300)
			pc->checkSkill(TACTICS, 0, 250);  //Dupois - Increase tactics but only by a fraction of the normal rate
	}
	else
		sysmessage(s, TRANSLATE("You feel you would gain no more from using that."));

}

void Skills::AButte(NXWSOCKET s1, P_ITEM pButte)
{
	if ( s1 < 0 || s1 >= now ) //Luxor
		return;
	P_CHAR pc = MAKE_CHAR_REF( currchar[s1] );
	VALIDATEPC(pc);
	


	int v1;
    if(pButte->getId()==0x100A)
    { // East Facing Butte
        if ((pButte->getPosition("x") > pc->getPosition("x"))||(pButte->getPosition("y") != pc->getPosition("y")))
            v1= INVALID;
        else v1= pc->getPosition("x") - pButte->getPosition("x");
    }
    else
    { // South Facing Butte
        if ((pButte->getPosition("y") > pc->getPosition("y"))||(pButte->getPosition("x") != pc->getPosition("x")))
            v1= INVALID;
        else v1= pc->getPosition("y") - pButte->getPosition("y");
    }

    int arrowsquant=0;
    if(v1==1)
    {

        if(pButte->more1>0)
        {
            
			P_ITEM pi = item::CreateFromScript( "$item_arrow", pc->getBackpack(), pButte->more1/2 );
			VALIDATEPI(pi);
            pi->Refresh();
        }

        if(pButte->more2>0)
        {
			P_ITEM pi = item::CreateFromScript( "$item_crossbow_bolt", pc->getBackpack(), pButte->more2/2 );
			VALIDATEPI(pi);
            pi->Refresh();
        }

        int i=0;
        if(pButte->more1>0) i++;
        if(pButte->more2>0) i+=2;

        char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

        switch(i)
        {
        case 0:
            sprintf(temp,TRANSLATE("This target is empty"));
            break;
        case 1:
            sprintf(temp,TRANSLATE("You pull %d arrows from the target"),pButte->more1/2);
            break;
        case 2:
            sprintf(temp,TRANSLATE("You pull %d bolts from the target"),pButte->more2/2);
            break;
        case 3:
            sprintf(temp,TRANSLATE("You pull %d arrows and %d bolts from the target"),pButte->more1,pButte->more2/2);
            break;
        default:
            LogError("switch reached default");
            return;
        }
        pc->sysmsg(temp);
        pButte->more1=0;
        pButte->more2=0;
    }

    if( (v1>=5) && (v1<=8) )
    {
        if (!pc->getWeapon()->IsBowType())
        {
            pc->sysmsg( TRANSLATE("You need to equip a bow to use this."));
            return;
        }
        if ((pButte->more1+pButte->more2)>99)
        {
            pc->sysmsg( TRANSLATE("You should empty the butte first!"));
            return;
        }
		if (pc->getWeapon()->IsBow()) 
			arrowsquant=pc->getAmount(0x0F3F); // Sabrewulf
        else 
			arrowsquant=pc->getAmount(0x1BFB);

        if (arrowsquant==0)
        {
            pc->sysmsg( TRANSLATE("You have nothing to fire!"));
            return;
        }
		
		if (pc->getWeapon()->IsBow()) 
        {
            pc->delItems( 0x0F3F, 1);
            pButte->more1++;
            //add moving effect here to item, not character
        }
        else
        {
            pc->delItems( 0x1BFB, 1);
            pButte->more2++;
            //add moving effect here to item, not character
        }
        if (pc->isMounting()) 
			pc->combatOnHorse();
        else 
			pc->combatOnFoot();

        if( pc->skill[ARCHERY] < 350 )
            pc->checkSkill( ARCHERY, 0, 1000 );
        else
            pc->sysmsg( TRANSLATE("You learn nothing from practicing here") );

        switch( ( pc->skill[ARCHERY]+ ( (rand()%200) -100) ) /100 )
        {
		case -1:
		case 0:
		case 1:
			pc->sysmsg( TRANSLATE("You miss the target"));
			pc->playSFX(0x0238);
			break;
		case 2:
		case 3:
			pc->sysmsg( TRANSLATE("You hit the outer ring!"));
			pc->playSFX(0x0234);
			break;
		case 4:
		case 5:
		case 6:
			pc->sysmsg( TRANSLATE("You hit the middle ring!"));
			pc->playSFX(0x0234);
			break;
		case 7:
		case 8:
		case 9:
			pc->sysmsg( TRANSLATE("You hit the inner ring!"));
			pc->playSFX(0x0234);
			break;
		case 10:
		case 11:
			pc->sysmsg( TRANSLATE("You hit the bullseye!!"));
			pc->playSFX(0x0234);
			break;
		default:
			break;
		}
    }
    if ( (v1>1)&&(v1<5) || (v1>8))
		pc->sysmsg( TRANSLATE("You cant use that from here."));

}

/*!
\author Luxor
\brief Implements Meditation skill
*/
void Skills::Meditation (NXWSOCKET  s)
{
	if ( s < 0 || s >= now )
		return;

	P_CHAR pc = pointers::findCharBySerial(currchar[s]);
	VALIDATEPC(pc);

	P_ITEM pi = NULL;

	pc->med = 0;

	if ( pc->war ) {
		pc->sysmsg( TRANSLATE("Your mind is too busy with the war thoughts.") );
		return;
	}

	if ( SrvParms->armoraffectmana && Skills::GetAntiMagicalArmorDefence(pc->getSerial32()) > 15 ) {
		pc->sysmsg( TRANSLATE("Regenerative forces cannot penetrate your armor.") );
		return;
	}

	pi = pc->getWeapon();
	if ( (ISVALIDPI(pi) && !pi->IsStave()) || pc->getShield() ) {
		pc->sysmsg( TRANSLATE("You cannot meditate with a weapon or shield equipped!") );
		return;
	}

	if ( pc->mn == pc->in ) {
		pc->sysmsg( TRANSLATE("You are at peace.") );
		return;
	}


	//
	// Meditation check
	//
	if ( !pc->checkSkill(MEDITATION, 0, 1000) ) {
		pc->sysmsg( TRANSLATE("You cannot focus your concentration.") );
		return;
	}

	pc->sysmsg( TRANSLATE("You enter a meditative trance.") );
	pc->med = 1;
	pc->playSFX(0x00F9);
}

//AntiChrist - 5/11/99
//
//If you are a ghost and attack a player, you can PERSECUTE him
//and his mana decreases each time you try to persecute him
//decrease=3+(your int/10)
//
/*!
\author AntiChrist
\date 05/11/1999
\param s socket of the persecuter

If you are a ghost and attack a player, you can PERSECUTE him
and his mana decreases each time you try to persecute him
decrease=3+(your int/10)
*/
void Skills::Persecute (NXWSOCKET  s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	P_CHAR pc_targ=pointers::findCharBySerial(pc->targserial);
	VALIDATEPC(pc_targ);

    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    if (pc_targ->IsGM()) return;

    int decrease=(pc->in/10)+3;

    if((pc->skilldelay<=uiCurrentTime) || pc->IsGM())
    {
        if(((rand()%20)+pc->in)>45) //not always
        {
            if( pc_targ->mn <= decrease )
                pc_targ->mn = 0;
            else
                pc_targ->mn-=decrease;//decrease mana
            pc_targ->updateStats(1);//update
		pc->sysmsg(TRANSLATE("Your spiritual forces disturb the enemy!"));
		pc_targ->sysmsg(TRANSLATE("A damned soul is disturbing your mind!"));
            SetSkillDelay(DEREF_P_CHAR(pc));

            sprintf(temp, TRANSLATE("%s is persecuted by a ghost!!"), pc_targ->getCurrentNameC());

            // Dupois pointed out the for loop was changing i which would drive stuff nuts later

			pc_targ->emoteall( temp, 1);
            
        } else
        {
		pc->sysmsg(TRANSLATE("Your mind is not strong enough to disturb the enemy."));
        }
    } else
    {
        
	pc->sysmsg(TRANSLATE("You are unable to persecute him now...rest a little..."));
    }

}

void loadskills()
{
    int i, noskill, l=0;
    char sect[512];
    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];

    for (i=0;i<SKILLS;i++) // lb
    {
        skillinfo[i].st=0;
        skillinfo[i].dx=0;
        skillinfo[i].in=0;
        skillinfo[i].advance_index=l;
        skillinfo[i].unhide_onuse = 1;
        skillinfo[i].unhide_onfail = 0;
        noskill=0;

        sprintf(sect, "SECTION SKILL %i", i);
        safedelete(iter);
        iter = Scripts::Skills->getNewIterator(sect);
        if (iter==NULL) continue;

        int loopexit=0;
        do
        {
            iter->parseLine(script1, script2);
            if ((script1[0]!='}')&&(script1[0]!='{'))
            {
                if (!(strcmp("STR", script1)))
                {
                    skillinfo[i].st=str2num(script2);
                }
                else if (!(strcmp("DEX", script1)))
                {
                    skillinfo[i].dx=str2num(script2);
                }
                else if (!(strcmp("INT", script1)))
                {
                    skillinfo[i].in=str2num(script2);
                }
                else if (!(strcmp("SKILLPOINT", script1)))
                {
                    wpadvance[l].skill=i;
                    gettokennum(script2, 0);
                    wpadvance[l].base=str2num(gettokenstr);
                    gettokennum(script2, 1);
                    wpadvance[l].success=str2num(gettokenstr);
                    gettokennum(script2, 2);
                    wpadvance[l].failure=str2num(gettokenstr);
                    l++;
                }
                else if (!(strcmp("UNHIDEONUSE", script1)))  //Luxor 7 dec 2001
                {
                    if (str2num(script2) == 0)
                        skillinfo[i].unhide_onuse = 0;
                    else
                        skillinfo[i].unhide_onuse = 1;
                }
                else if (!(strcmp("UNHIDEONFAIL", script1))) //Luxor 7 dec 2001
                {
                    if (str2num(script2) > 0)
                        skillinfo[i].unhide_onfail = 1;
                    else
                        skillinfo[i].unhide_onfail = 0;
                }
            }
        }
        while ( (script1[0]!='}') && (!noskill) && (++loopexit < MAXLOOPS) );
    }
    safedelete(iter);
}

void SkillVars()
{
    strcpy(skillinfo[ALCHEMY].madeword,"mixed");
    strcpy(skillinfo[ANATOMY].madeword,"made");
    strcpy(skillinfo[ANIMALLORE].madeword,"made");
    strcpy(skillinfo[ITEMID].madeword,"made");
    strcpy(skillinfo[ARMSLORE].madeword,"made");
    strcpy(skillinfo[PARRYING].madeword,"made");
    strcpy(skillinfo[BEGGING].madeword,"made");
    strcpy(skillinfo[BLACKSMITHING].madeword,"forged");
    strcpy(skillinfo[BOWCRAFT].madeword,"bowcrafted");
    strcpy(skillinfo[PEACEMAKING].madeword,"made");
    strcpy(skillinfo[CAMPING].madeword,"made");
    strcpy(skillinfo[CARPENTRY].madeword,"made");
    strcpy(skillinfo[CARTOGRAPHY].madeword,"wrote");
    strcpy(skillinfo[COOKING].madeword,"cooked");
    strcpy(skillinfo[DETECTINGHIDDEN].madeword,"made");
    strcpy(skillinfo[ENTICEMENT].madeword,"made");
    strcpy(skillinfo[EVALUATINGINTEL].madeword,"made");
    strcpy(skillinfo[HEALING].madeword,"made");
    strcpy(skillinfo[FISHING].madeword,"made");
    strcpy(skillinfo[FORENSICS].madeword,"made");
    strcpy(skillinfo[HERDING].madeword,"made");
    strcpy(skillinfo[HIDING].madeword,"made");
    strcpy(skillinfo[PROVOCATION].madeword,"made");
    strcpy(skillinfo[INSCRIPTION].madeword,"wrote");
    strcpy(skillinfo[LOCKPICKING].madeword,"made");
    strcpy(skillinfo[MAGERY].madeword,"envoked");
    strcpy(skillinfo[MAGICRESISTANCE].madeword,"made");
    strcpy(skillinfo[TACTICS].madeword,"made");
    strcpy(skillinfo[SNOOPING].madeword,"made");
    strcpy(skillinfo[MUSICIANSHIP].madeword,"made");
    strcpy(skillinfo[POISONING].madeword,"made");
    strcpy(skillinfo[ARCHERY].madeword,"made");
    strcpy(skillinfo[SPIRITSPEAK].madeword,"made");
    strcpy(skillinfo[STEALING].madeword,"made");
    strcpy(skillinfo[TAILORING].madeword,"sewn");
    strcpy(skillinfo[TAMING].madeword,"made");
    strcpy(skillinfo[TASTEID].madeword,"made");
    strcpy(skillinfo[TINKERING].madeword,"made");
    strcpy(skillinfo[TRACKING].madeword,"made");
    strcpy(skillinfo[VETERINARY].madeword,"made");
    strcpy(skillinfo[SWORDSMANSHIP].madeword,"made");
    strcpy(skillinfo[MACEFIGHTING].madeword,"made");
    strcpy(skillinfo[FENCING].madeword,"made");
    strcpy(skillinfo[WRESTLING].madeword,"made");
    strcpy(skillinfo[LUMBERJACKING].madeword,"made");
    strcpy(skillinfo[MINING].madeword,"smelted");
    strcpy(skillinfo[MEDITATION].madeword,"envoked");
    strcpy(skillinfo[STEALTH].madeword,"made");
    strcpy(skillinfo[REMOVETRAPS].madeword,"made");

}

int Skills::GetAntiMagicalArmorDefence(CHARACTER p)
{// blackwind

	P_CHAR pc= MAKE_CHAR_REF( p );
	VALIDATEPCR( pc, 0 );

    int ar = 0;
    if (pc->HasHumanBody())
    {
        NxwItemWrapper si;
		si.fillItemWeared( pc, false, true, true );
		for( si.rewind(); !si.isEmpty(); si++ )
        {
            P_ITEM pi=si.getItem();
			if( ISVALIDPI(pi) && pi->layer>1 && pi->layer < 25)
            {
                if (!(strstr(pi->getCurrentNameC(), "leather") || strstr(pi->getCurrentNameC(), "magic") ||
                    strstr(pi->getCurrentNameC(), "boot")|| strstr(pi->getCurrentNameC(), "mask")))
                    ar += pi->def;
            }
        }
    }
    return ar;
}
/*!
\author Polygon
\brief Builds the cartography menu
\param s socket of the crafter

Function is called when clicked on the <i>Cartography</i> button
*/
void Skills::Cartography(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
    
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    if( Skills::HasEmptyMap(pc->getSerial32()) )
    {
        //itemmake[s].has = 1;
		//Skills::MakeMenu(s, 1200, CARTOGRAPHY);
    }
    else
        sysmessage(s, TRANSLATE("You don't have an empty map to draw on"));
}

/*!
\author Polygon
\brief Check if the player carries an empty map
\param cc character to check if has empty map
\return always false (?)
\todo write it
*/
bool Skills::HasEmptyMap(CHARACTER cc)
{

	/*P_CHAR pc=MAKE_CHAR_REF(cc);
	VALIDATEPCR(pc,false);
    
    P_ITEM pack = pc->getBackpack();    // Get the packitem
	VALIDATEPIR(pack,false);
	
	int ci = 0, loopexit = 0;
	P_ITEM pi;
	while (((pi = ContainerSearch(pack->getSerial32(), &ci)) != NULL) &&(++loopexit < MAXLOOPS))
	{
        if(!ISVALIDPI(pi))
			continue;
		
        if (pi->type == 300)  // Is it the right type
            return true;    // Yay, go on with carto
    }
*/
    return false;
}

/*!
\author Polygon
\brief Delete an empty map from the player's backpack, use HasEmptyMap before!
\return always false (?)
\todo write it
*/
bool Skills::DelEmptyMap(CHARACTER cc)
{

 	/*P_CHAR pc=MAKE_CHAR_REF(cc);
	VALIDATEPCR(pc,false);
    
    P_ITEM pack = pc->getBackpack();    // Get the packitem
	VALIDATEPIR(pack,false);

    int ci=0;       // Stores the last found item
    int loopexit=0; // Avoids the loop to take too much time
	P_ITEM cand=NULL;
	while (((cand = ContainerSearch(pack->getSerial32(), &ci)) != NULL) &&(++loopexit < MAXLOOPS))
	{
        if(!ISVALIDPI(cand))
			continue;

        if (cand->type == 300)  // Is it the right type
        {
            cand->deleteItem();    // Delete it
            return true;        // Go on with cartography
        }
    }
*/
    return false;   // Search lasted too long, abort (shouldn't happen, abort if ya get this)
}

/*!
\author Polygon
\brief Attempt to decipher a tattered map
\param tmap item pointer to the map
\param s socket of the decipher

Called when double-click such a map
*/
void Skills::Decipher(P_ITEM tmap, NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
 	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    char sect[512];         // Needed for script search
    int regtouse;           // Stores the region-number of the TH-region
    int i;                  // Loop variable
    int btlx, btly, blrx, blry; // Stores the borders of the tresure region (topleft x-y, lowright x-y)
    int tlx, tly, lrx, lry;     // Stores the map borders
    int x, y;                   // Stores the final treasure location
    cScpIterator* iter = NULL;
    char script1[1024];

    if(pc->skilldelay<=uiCurrentTime || pc->IsGM()) // Char doin something?
    {
        if (pc->checkSkill( CARTOGRAPHY, tmap->morey * 10, 1000)) // Is the char skilled enaugh to decipher the map
        {
            // Stores the new map
            P_ITEM nmap=item::CreateFromScript( 70025, pc->getBackpack() );
            if (!ISVALIDPI(nmap))
            {
                LogWarning("bad script item # 70025(Item Not found).");
                return; //invalid script item
            }

			nmap->setCurrentName("a deciphered lvl.%d treasure map", tmap->morez);   // Give it the correct name
            nmap->morez = tmap->morez;              // Give it the correct level
            nmap->creator = pc->getCurrentName();  // Store the creator


            sprintf(sect, "SECTION TREASURE %i", nmap->morez);

            iter = Scripts::Regions->getNewIterator(sect);

            if (iter == NULL) {
                LogWarning("Treasure hunting cSkills::Decipher : Unable to find 'SECTION TREASURE %d' in regions-script", nmap->morez);
                return;
            }
            strcpy(script1, iter->getEntry()->getFullLine().c_str());               // skip the {
            strcpy(script1, iter->getEntry()->getFullLine().c_str());               // Get the number of areas
            regtouse = rand()%str2num(script1); // Select a random one
            for (i = 0; i < regtouse; i++)      // Skip the ones before the correct one
            {
                strcpy(script1, iter->getEntry()->getFullLine().c_str());
                strcpy(script1, iter->getEntry()->getFullLine().c_str());
                strcpy(script1, iter->getEntry()->getFullLine().c_str());
                strcpy(script1, iter->getEntry()->getFullLine().c_str());
            }
            strcpy(script1, iter->getEntry()->getFullLine().c_str());
            btlx = str2num(script1);
            strcpy(script1, iter->getEntry()->getFullLine().c_str());
            btly = str2num(script1);
            strcpy(script1, iter->getEntry()->getFullLine().c_str());
            blrx = str2num(script1);
            strcpy(script1, iter->getEntry()->getFullLine().c_str());
            blry = str2num(script1);

            safedelete(iter);

            if ((btlx < 0) || (btly < 0) || (blrx > 0x13FF) || (blry > 0x0FFF)) // Valid region?
            {
                sprintf(sect, "Treasure Hunting cSkills::Decipher : Invalid region borders for lvl.%d , region %d", nmap->morez, regtouse+1);   // Give out detailed warning :D
                LogWarning(sect);
                return;
            }
            x = btlx + (rand()%(blrx-btlx));    // Generate treasure location
            y = btly + (rand()%(blry-btly));
            tlx = x - 250;      // Generate map borders
            tly = y - 250;
            lrx = x + 250;
            lry = y + 250;
            // Check if we are over the borders and correct errors
            if (tlx < 0)    // Too far left?
            {
                lrx -= tlx; // Add the stuff too far left to the right border (tlx is neg. so - and - gets + ;)
                tlx = 0;    // Set tlx to correct value
            }
            else if (lrx > 0x13FF) // Too far right?
            {
                tlx -= lrx - 0x13FF;    // Subtract what is to much from the left border
                lrx = 0x13FF;   // Set lrx to correct value
            }
            if (tly < 0)    // Too far top?
            {
                lry -= tly; // Add the stuff too far top to the bottom border (tly is neg. so - and - gets + ;)
                tly = 0;    // Set tly to correct value
            }
            else if (lry > 0x0FFF) // Too far bottom?
            {
                tly -= lry - 0x0FFF;    // Subtract what is to much from the top border
                lry = 0x0FFF;   // Set lry to correct value
            }
            nmap->more1 = tlx>>8;   // Store the map extends
            nmap->more2 = tlx%256;
            nmap->more3 = tly>>8;
            nmap->more4 = tly%256;
            nmap->moreb1 = lrx>>8;
            nmap->moreb2 = lrx%256;
            nmap->moreb3 = lry>>8;
            nmap->moreb4 = lry%256;
            nmap->morex = x;        // Store the treasure's location
            nmap->morey = y;
            tmap->Delete();    // Delete the tattered map
        }
        else
            pc->sysmsg(TRANSLATE("You fail to decipher the map"));      // Nope :P
        // Set the skill delay, no matter if it was a success or not
        SetTimerSec(&pc->skilldelay,SrvParms->skilldelay);
        pc->playSFX(0x0249); // Do some inscription sound regardless of success or failure
        pc->sysmsg(TRANSLATE("You put the deciphered tresure map in your pack"));       // YAY
    }
    else
        pc->sysmsg(TRANSLATE("You must wait to perform another action"));       // wait a bit
}
