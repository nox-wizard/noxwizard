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
#include "npcai.h"
#include "nxw_utils.h"
#include "weight.h"
#include "set.h"
#include "chars.h"
#include "items.h"
#include "skills.h"
#include "classes.h"
#include "inlines.h"
#include "range.h"


/*!
\brief Snoop into container
\author Unknow, completly rewritten by Endymion
\param snooper the snooper
\param cont the contanier
*/
void snooping( P_CHAR snooper, P_ITEM cont )
{
	VALIDATEPC(snooper);
	NXWCLIENT ps = snooper->getClient();
	if( ps == NULL ) return;
	NXWSOCKET s = ps->toInt();
	VALIDATEPI(cont);
	P_CHAR owner = cont->getPackOwner();
	VALIDATEPC(owner);
	char temp[TEMP_STR_SIZE];

	if (snooper->getSerial32() == owner->getSerial32())
		snooper->showContainer(cont);
	else if (snooper->IsGMorCounselor())
		snooper->showContainer(cont);
	else
	if ((char_inRange(snooper, owner, 2)) ||(item_inRange(snooper, cont, 2)))
	{
		if ( owner->HasHumanBody() && ( owner->getOwnerSerial32()==snooper->getSerial32()))
			snooper->showContainer(cont);
		else if ( owner->npcaitype == NPCAI_PLAYERVENDOR)
				snooper->showContainer(cont);
		else
		{
			if ((cont->getContSerial()>1) && (cont->getContSerial() != snooper->getSerial32()) )
			{
				
				if ( owner->amxevents[EVENT_CHR_ONSNOOPED])
				{
					g_bByPass = false;
					owner->amxevents[EVENT_CHR_ONSNOOPED]->Call( owner->getSerial32(), snooper->getSerial32());
					if (g_bByPass==true) return;
				}
				/*
				owner->runAmxEvent( EVENT_CHR_ONSNOOPED, owner->getSerial32(), s);
				if (g_bByPass==true)
					return;
				*/
				snooper->objectdelay=SrvParms->snoopdelay * MY_CLOCKS_PER_SEC + uiCurrentTime;
				if ( owner->IsGMorCounselor())
				{
					snooper->sysmsg( TRANSLATE("You can't peek into that container or you'll be jailed."));// AntiChrist
					sprintf( temp, TRANSLATE("%s is trying to snoop you!"), snooper->getCurrentNameC());
					owner->sysmsg(temp);
					return;
				}
				else if (snooper->checkSkill( SNOOPING, 0, 1000))
				{
					snooper->showContainer(cont);
					snooper->sysmsg( TRANSLATE("You successfully peek into that container."));
				}
				else
				{
					snooper->sysmsg( TRANSLATE("You failed to peek into that container."));
					if ( owner->npc )
						owner->talk(s, TRANSLATE("Art thou attempting to disturb my privacy?"), 0);
					else {
						sprintf( temp, TRANSLATE("You notice %s trying to peek into your pack!"), snooper->getCurrentNameC());
						owner->sysmsg( temp );
					}
					snooper->IncreaseKarma(-ServerScp::g_nSnoopKarmaLoss);//AntiChrist
					snooper->modifyFame(-ServerScp::g_nSnoopFameLoss);//AntiChrist
					setCrimGrey(snooper, ServerScp::g_nSnoopWillCriminal);
				}
			}
		}
	}
	else {
		snooper->sysmsg(TRANSLATE("You are too far away!"));
	}
}

/*!
\brief Steal something
\author Unknow, completly rewritten by Endymion
\param ps the client
*/
void Skills::target_stealing( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR thief = ps->currChar();
	VALIDATEPC(thief);
	SERIAL target_serial = t->getClicked();

	AMXEXECSVTARGET( thief->getSerial32(),AMXT_SKITARGS,STEALING,AMX_BEFORE);

	//steal a char
	if ( isCharSerial(target_serial) )
	{
		Skills::target_randomSteal(ps,t);
        	return;
	}

	const P_ITEM pi = pointers::findItemBySerial( target_serial );
	VALIDATEPI(pi);

	//steal a pickpocket, a steal training dummy
	if( pi->getId() == 0x1E2D || pi->getId() == 0x1E2C )
	{
		Skills::PickPocketTarget(ps);
        	return;
	}

	//no stealing for items on layers other than 0 (equipped!) , newbie items, and items not being in containers allowed !
	if ( pi->layer!=0 || pi->isNewbie() || pi->isInWorld() )
	{
       	thief->sysmsg(TRANSLATE("You cannot steal that."));
       	return;
	}

	P_CHAR victim = pi->getPackOwner();
	VALIDATEPC(victim);

	if (victim->npcaitype == NPCAI_PLAYERVENDOR)
	{
		thief->sysmsg(TRANSLATE("You cannot steal from player vendors."));
       	return;
	}

	if ( (thief->getSerial32() == victim->getSerial32()) || (thief->getSerial32()==victim->getOwnerSerial32()) )
	{
		thief->sysmsg(TRANSLATE("You catch yourself red handed."));
		return;
	}

	if (thief->distFrom( victim ) == 1)
	{

		int result;

		R32 we = pi->getWeightActual();
		int bonus= (int)( (1800 - we)/5 );
		if ( thief->checkSkill( STEALING,0,(1000-bonus)) )
		{
			// 0 stealing 2 stones, 10  3 stones, 99.9 12 stones, 100 17 stones !!!
			int cansteal = thief->skill[STEALING] > 999 ? 1700 : thief->skill[STEALING] + 200;

			if ( we > cansteal )
			{
        		thief->sysmsg(TRANSLATE("That is too heavy."));
        		return;
			}

			
			if (pi->amxevents[EVENT_IONSTOLEN]!=NULL)
			{
				g_bByPass = false;
				pi->amxevents[EVENT_IONSTOLEN]->Call(pi->getSerial32(), thief->getSerial32(), victim->getSerial32());
				if (g_bByPass==true)
					return;
			}

			if (victim->amxevents[EVENT_CHR_ONSTOLEN])
			{
				g_bByPass = false;
				victim->amxevents[EVENT_CHR_ONSTOLEN]->Call(victim->getSerial32(), thief->getSerial32());
				if (g_bByPass==true)
					return;
			}
			/*

			pi->runAmxEvent( EVENT_IONSTOLEN, pi->getSerial32(), s, victim->getSerial32() );
			if (g_bByPass==true)
				return;

			victim->runAmxEvent( EVENT_CHR_ONSTOLEN, victim->getSerial32(), s );
			if (g_bByPass==true)
				return;
			*/

			P_ITEM pack= thief->getBackpack();
			VALIDATEPI(pack);

			pi->setContSerial( pack->getSerial32() );

			thief->sysmsg(TRANSLATE("You successfully steal the item."));
			pi->Refresh();
			
			result=+200;
			//all_items(s); why all item?
		}
		else
		{
			thief->sysmsg( TRANSLATE("You failed to steal the item."));
			result=-200;
			//Only onhide when player is caught!
		}

		if ( rand()%1000 > ( thief->skill[STEALING] + result )  )
		{
			thief->unHide();
			thief->sysmsg(TRANSLATE("You have been caught!"));
			thief->IncreaseKarma(ServerScp::g_nStealKarmaLoss);
			thief->modifyFame(ServerScp::g_nStealFameLoss);

			if ( victim->IsInnocent() && thief->attackerserial != victim->getSerial32() && Guilds->Compare(thief,victim)==0)
				setCrimGrey(thief, ServerScp::g_nStealWillCriminal); //Blue and not attacker and not same guild


			std::string itmname ( "" );
			char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
			char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
			if ( pi->getCurrentName() != "#" )
				itmname = pi->getCurrentName();
			else
			{
				pi->getName( temp );
				itmname = temp;
			}
			sprintf(temp,TRANSLATE("You notice %s trying to steal %s from you!"), thief->getCurrentNameC(), itmname.c_str());
			sprintf(temp2,TRANSLATE("You notice %s trying to steal %s from %s!"), thief->getCurrentNameC(), itmname.c_str(), victim->getCurrentNameC());

			if ( victim->npc )
				if( victim->HasHumanBody() )
					victim->talkAll(TRANSLATE( "Guards!! A thief is amoung us!"),0);
			else
				victim->sysmsg(temp);

			//send to all player temp2 = thief are stealing victim if are more intelligent and a bonus of luck :D
			NxwSocketWrapper sw;
			sw.fillOnline( thief, true );
			for( sw.rewind(); !sw.isEmpty(); sw++ ) {
				
				NXWCLIENT ps_i=sw.getClient();
				if(ps_i==NULL ) continue;

				P_CHAR pc_i=ps_i->currChar();
				if ( ISVALIDPC(pc_i) )
					if( (rand()%10+10==17) || ( (rand()%2==1) && (pc_i->in>=thief->in)))
						pc_i->sysmsg(temp2);
			}
		}
	}
	else 
	{
		thief->sysmsg(TRANSLATE("You are too far away to steal that item."));
	}

	AMXEXECSVTARGET( thief->getSerial32(),AMXT_SKITARGS,STEALING,AMX_AFTER);
}

/*!
\brief Steal from a pickpocket, a stealing training dummy
\author Ripper, updated by Endymion
\param ps the client
*/
void Skills::PickPocketTarget(NXWCLIENT ps)
{
	if( ps == 0 ) return;
	P_CHAR Me = ps->currChar();
	VALIDATEPC(Me);

	if (Me->skill[STEALING] < 300)
	// check if under 30 in stealing
	{
		Me->checkSkill( STEALING, 0, 1000);
		// check their skill
		Me->playSFX(0x0249);
		// rustling sound..dont know if right but it works :)
	}
	else
        Me->sysmsg(TRANSLATE("You learn nothing from practicing here"));
        	// if over 30 Stealing..dont learn.
}

/*!
\brief Steal random
\author Unknow, updated by Endymion
\param ps the client
\todo add string because it's locked contanier into translate
*/
void Skills::target_randomSteal( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR thief=ps->currChar();
	VALIDATEPC(thief);
	P_CHAR victim = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(victim);
	

	if (thief->getSerial32() == victim->getSerial32() || thief->getSerial32()==victim->getOwnerSerial32())
	{
		thief->sysmsg(TRANSLATE("You catch yourself red handed."));
		return;
	}

	if (victim->npcaitype == NPCAI_PLAYERVENDOR)
	{
		thief->sysmsg(TRANSLATE("You cannot steal from player vendors."));
		return;
	}

	if (victim->IsGMorCounselor() )
	{
		thief->sysmsg( TRANSLATE("You can't steal from gods."));
		return;
	}

	P_ITEM pack= victim->getBackpack();
	if ( !ISVALIDPI(pack))
	{
		thief->sysmsg(TRANSLATE("bad luck, your victim doesn't have a backpack"));
		return;
	}

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	sprintf(temp, TRANSLATE("You reach into %s's pack to steal something ..."), victim->getCurrentNameC() );
	thief->sysmsg(temp);

	if ( char_inRange(thief,victim,1) )
	{
		P_ITEM pi = NULL;

		NxwItemWrapper si;
		si.fillItemsInContainer( pack, false );
		if( si.size()>0 ) {
			int ra=rand()%si.size();
			int c=0;
			for( si.rewind(); !si.isEmpty(); si++ ) {
				c++;
				if( c==ra ) {
					pi=si.getItem();
					break;
				}
			}
		}

		if( pi==NULL ) {
			thief->sysmsg(TRANSLATE("... and discover your victim doesn't have any posessions"));
			return;
		}

		
		//Endy can't be not valid after this -^ loop, else error
		VALIDATEPI(pi);

		if( pi->isNewbie() ) 
		{//newbie
			thief->sysmsg(TRANSLATE("... and fail because it is of no value to you."));
			return;
		}

		if(pi->isSecureContainer())
		{
			thief->sysmsg(TRANSLATE("... and fail because it's a locked container."));
			return;
		}

		if ( thief->checkSkill( STEALING,0,999) )
		{
			// 0 stealing 2 stones, 10  3 stones, 99.9 12 stones, 100 17 stones !!!
			int cansteal = thief->skill[STEALING] > 999 ? 1700 : thief->skill[STEALING] + 200;

			if ( ((pi->getWeightActual())>cansteal) && !pi->isContainer())//Containers
				thief->sysmsg(TRANSLATE("... and fail because it is too heavy."));
        		else
				if(pi->isContainer() && (weights::RecursePacks(pi)>cansteal))
					thief->sysmsg(TRANSLATE("... and fail because it is too heavy."));
				else
				{
					
					if (victim->amxevents[EVENT_CHR_ONSTOLEN])
					{
						g_bByPass = false;
						victim->amxevents[EVENT_CHR_ONSTOLEN]->Call(victim->getSerial32(), thief->getSerial32());
						if (g_bByPass==true)
							return;
					}
					/*
					victim->runAmxEvent( EVENT_CHR_ONSTOLEN, victim->getSerial32(), s);
					if (g_bByPass==true)
						return;
					*/
					P_ITEM thiefpack = thief->getBackpack();
					VALIDATEPI(thiefpack);
					pi->setContSerial( thiefpack->getSerial32() );
					thief->sysmsg(TRANSLATE("... and you succeed."));
					pi->Refresh();
					//all_items(s);
				}
		}
		else
			thief->sysmsg(TRANSLATE(".. and fail because you're not good enough."));

		if ( thief->skill[STEALING] < rand()%1001 )
		{
			thief->unHide();
			thief->sysmsg(TRANSLATE("You have been caught!"));
			thief->IncreaseKarma( ServerScp::g_nStealKarmaLoss);
			thief->modifyFame( ServerScp::g_nStealFameLoss);

			if (victim->IsInnocent() && thief->attackerserial!=victim->getSerial32() && Guilds->Compare(thief,victim)==0)//AntiChrist
				setCrimGrey(thief, ServerScp::g_nStealWillCriminal);//Blue and not attacker and not guild

			std::string itmname = "";
			if ( pi->getCurrentName() != "#" )
				itmname = pi->getCurrentName();
			else 
			{
				pi->getName( temp );
				itmname = temp;
			}

			sprintf(temp,TRANSLATE("You notice %s trying to steal %s from you!"), thief->getCurrentNameC(), itmname.c_str());
			sprintf(temp2,TRANSLATE("You notice %s trying to steal %s from %s!"), thief->getCurrentNameC(), itmname.c_str(), victim->getCurrentNameC());

			if ( victim->npc)
				victim->talkAll(TRANSLATE( "Guards!! A thief is amoung us!"),0);
			else
				victim->sysmsg(temp);

			//send to all player temp2 = thief are stealing victim if are more intelligent and a bonus of luck :D
			//
			//
			NxwSocketWrapper sw;
			sw.fillOnline( thief, true );
			for( sw.rewind(); !sw.isEmpty(); sw++ ) {
				
				NXWCLIENT ps_i=sw.getClient();
				if( ps_i==NULL ) continue;

				P_CHAR pc_i=ps_i->currChar();
				if ( ISVALIDPC(pc_i) )
					if( (rand()%10+10==17) || ( (rand()%2==1) && (pc_i->in>=thief->in)))
						sysmessage(ps_i->toInt(),temp2);
			}
		}
	}
	else 
	{
		thief->sysmsg( TRANSLATE("... and realise you're too far away."));
	}

}


/*!
\brief lockpicking skill
\author Unknow, rewrite by Endymion
\since 0.53
\param ps the client
*/
void Skills::target_lockpick( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);
	P_ITEM chest=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(chest);
	P_ITEM pick=MAKE_ITEM_REF( t->buffer[0] );
	VALIDATEPI(pick);

	AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,LOCKPICKING,AMX_BEFORE);


	if (chest->amxevents[EVENT_IONLOCKPICK]!=NULL)
	{
		g_bByPass = false;
		chest->amxevents[EVENT_IONLOCKPICK]->Call(chest->getSerial32(), pc->getSerial32());
		if (g_bByPass==true)
			return;
	}
	
	/*
	chest->runAmxEvent( EVENT_IONLOCKPICK, chest->getSerial32(), s );
	if (g_bByPass==true)
		return;
	*/

	if( !item_inRange(pc,pick,1) )
	{
		pc->sysmsg(TRANSLATE("You are too far away!"));
	}

	if (chest->magic==4)
	{
		return;
	}

	if(!chest->isSecureContainer())
	{
		pc->sysmsg(TRANSLATE("That is not locked."));
		return;
	}

	if(chest->more1==0 && chest->more2==0 && chest->more3==0 && chest->more4==0)
	{ //Make sure it isn't an item that has a key (i.e. player house, chest..etc)
		if(pc->checkSkill( LOCKPICKING, 0, 1000))
		{
			switch(chest->type)
			{
				case 8: chest->type=1; break;
				case 13: chest->type=12; break;
				case 64: chest->type=63; break;
				default:
					LogError("switch reached default");
					return;
			}
			soundeffect3(chest, 0x0241);
			pc->sysmsg(TRANSLATE("You manage to pick the lock."));
		}
		else
		{
			if((rand()%100)>50)
			{
				pc->sysmsg( TRANSLATE("You broke your lockpick!"));
				pick->ReduceAmount(1);
			}
			else 
				pc->sysmsg(TRANSLATE( "You fail to open the lock."));
		}
	}
	else 
		pc->sysmsg(TRANSLATE("That cannot be unlocked without a key."));


	AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,LOCKPICKING,AMX_AFTER);
}
