  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "client.h"
#include "basics.h"
#include "amx/amxcback.h"
#include "cmdtable.h"
#include "itemid.h"
#include "sndpkg.h"
#include "debug.h"
#include "srvparms.h"
#include "nxw_utils.h"
#include "party.h"
#include "set.h"
#include "trigger.h"
#include "magic.h"
#include "house.h"
#include "targeting.h"
#include "npcai.h"
#include "layer.h"
#include "commands.h"
#include "tmpeff.h"
#include "jail.h"
#include "weight.h"
#include "data.h"
#include "boats.h"
#include "scp_parser.h"
#include "archive.h"
#include "fishing.h"
#include "rcvpkg.h"
#include "map.h"
#include "chars.h"
#include "items.h"
#include "skills.h"
#include "classes.h"
#include "inlines.h"
#include "basics.h"
#include "range.h"
#include "scripts.h"
#include "nox-wizard.h"
#include "utils.h"


cTargets* Targ=NULL;

//extern void tweakmenu(NXWSOCKET  s, SERIAL serial);
extern void targetParty(NXWSOCKET  s);

#define INVALID_SERIAL -1

/*!
\author Luxor
\param s socket to attack
\brief Manages all attack command
*/
void target_allAttack( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

    P_CHAR pc_target = pointers::findCharBySerial( t->getClicked() );
    VALIDATEPC(pc_target);


	NxwCharWrapper sc;
    sc.fillOwnedNpcs( pc, false, true );
    pc->attackStuff(pc_target);
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pet=sc.getChar();
		if( ISVALIDPC(pet))
			npcattacktarget(pet, pc_target);
    }

}


/*!
\author Magius
\param s socket
*/

void cTargets::BanTarg(NXWSOCKET s)
{

}




void target_xTeleport( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	SERIAL serial = t->getClicked();
	if( isCharSerial( serial ) ) {
		P_CHAR pc_i = pointers::findCharBySerial( serial );
		if( ISVALIDPC( pc_i ) )
		{
			pc_i->MoveTo( pc->getPosition() );
			pc_i->teleport();
		}
	}
	else if( isItemSerial( serial ) ) {
		P_ITEM pi = pointers::findItemBySerial( serial );
		if( ISVALIDPI( pi ) ) {
			pi->MoveTo( pc->getPosition() );
			pi->Refresh();
		}
	}
}


void /*cTargets::*/TargIdTarget(NXWSOCKET s) // Fraz
{
    char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
    const P_ITEM pi = pointers::findItemBySerPtr(buffer[s] + 7);
    if (pi)
    {
        if (buffer[s][7] >= 0x40)
        {
            if (pi && pi->magic != 4)
            {
                if (pi->getSecondaryNameC() &&(strncmp(pi->getSecondaryNameC(), "#", 1)))
                    pi->setCurrentName(pi->getSecondaryNameC());

                if ( strncmp(pi->getCurrentNameC(), "#", 1) )
                    pi->getName(temp2);
                else
                    strcpy(temp2, pi->getCurrentNameC());

                sprintf(temp, TRANSLATE("You found that this item appears to be called: %s"), temp2);
                sysmessage(s,  temp);
            }
        }
        if (pi->type != 15)
        {
            if (pi->type != 404)
            {
                sysmessage(s, TRANSLATE("This item has no hidden magical properties."));
            }
            else
            {
                sprintf(temp, TRANSLATE("It is enchanted with item identification, and has %d charges remaining."), pi->morex);
                sysmessage(s, temp);
            }
        }
        else
        {
            sprintf(temp, TRANSLATE("It is enchanted with the spell %s, and has %d charges remaining."), spellname[(8*(pi->morex - 1)) + pi->morey - 1], pi->morez);
            sysmessage(s, temp);
        }
    }
}

/*!
\todo MoveBelongingsToBp is written stupid and for what?
*/
static void MoveBelongingsToBp(P_CHAR pc, P_CHAR pc_2)
{
    P_ITEM pPack= pc->getBackpack();
    if (!pPack)
    {
        P_ITEM pPack=item::CreateFromScript( "$item_backpack" );
        VALIDATEPI(pPack);
		pc->packitemserial=pPack->getSerial32();
		pPack->setContSerial(pc_2->getSerial32());
    }

    NxwItemWrapper si;
	si.fillItemWeared(pc, false );
	for( si.rewind(); !si.isEmpty(); si++ )
    {
        P_ITEM pi=si.getItem();
		if(!ISVALIDPI(pi)) continue;
		if (pi->layer!=LAYER_BACKPACK && pi->layer!=LAYER_BANKBOX &&
            pi->layer!=LAYER_BEARD && pi->layer!=LAYER_HAIR )
        {
            pi->setContSerial( pPack->getSerial32() );
			pi->SetRandPosInCont( pPack );
            pi->layer=0x00;
            SndRemoveitem( pi->getSerial32() );
            pi->Refresh();
        }
        else if (pc->IsWearing(pi) &&
            (pi->layer==LAYER_HAIR || pi->layer==LAYER_BEARD))
        {
            pi->Delete();
        }
    }
    pc_2->teleport();
}

static void GMTarget(NXWCLIENT ps, P_CHAR pc)
{
	VALIDATEPC(pc);

	if (ps == NULL) return;

	P_CHAR curr=ps->currChar();
	VALIDATEPC(curr);

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    int i;
    if (pc->dead) return;
    if (SrvParms->gm_log)
		WriteGMLog(curr, "%s as made %s a GM.\n", curr->getCurrentNameC(), pc->getCurrentNameC());

    pc->unmountHorse();    //AntiChrist bugfix
    pc->gmrestrict = 0;
    pc->SetBodyType(BODY_GMSTAFF);
    pc->SetOldBodyType(BODY_GMSTAFF);
    pc->setSkinColor(0x8021);
    pc->setOldSkinColor(0x8021);
    pc->SetPriv(0xF7);
    pc->priv2 = (unsigned char) (0xD9);

    for (i = 0; i < 7; i++) // this overwrites all previous settings !
    {
        pc->priv3[i]=metagm[0][i]; // gm defaults
        if (pc->account==0) pc->priv3[i]=0xffffffff;
    }

    for (i = 0; i < TRUESKILLS; i++)
    {
        pc->baseskill[i]=1000;
        pc->skill[i]=1000;
    }

    // All stats to 100
    pc->setStrength(100);
    pc->st2 = 100;
    pc->hp  = 100;
    pc->stm = 100;
    pc->in  = 100;
    pc->in2 = 100;
    pc->mn  = 100;
    pc->mn2 = 100;
    pc->dx  = 100;
    pc->dx2 = 100;

	if (strncmp(pc->getCurrentNameC(), "GM", 2))
    {
        sprintf(temp, "GM %s", pc->getCurrentNameC());
		pc->setCurrentName(temp);
    }
    MoveBelongingsToBp(pc,pc);
}

static void CnsTarget(NXWCLIENT ps, P_CHAR pc)
{
	if (ps == NULL) return;
	VALIDATEPC(pc);

	P_CHAR curr=ps->currChar();
	VALIDATEPC(curr);

    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    if (SrvParms->gm_log)
		WriteGMLog(curr, "%s as made %s a Counselor.\n", curr->getCurrentNameC(), pc->getCurrentNameC());

    pc->SetBodyType(BODY_GMSTAFF);
    pc->SetOldBodyType(BODY_GMSTAFF);
    pc->setSkinColor(0x8003);
    pc->setOldSkinColor(0x8002);
    pc->SetPriv(0xB6);
    pc->SetPriv2(0x8D);
    pc->gmrestrict = 0;
    if (strncmp(pc->getCurrentNameC(), "Counselor", 9))
    {
        sprintf(temp, "Counselor %s", pc->getCurrentNameC());
		pc->setCurrentName(temp);
    }
    for (int u=0;u<7;u++) // this overwrites all previous settigns !!!
    {
        pc->priv3[u]=metagm[1][u]; // counselor defaults
        if (pc->account==0) pc->priv3[u]=0xffffffff;
    }
    MoveBelongingsToBp(pc,pc);
}

void KillTarget(P_CHAR pc, int ly)
{
    VALIDATEPC(pc);

	P_ITEM pi=pc->GetItemOnLayer( ly );
	if(ISVALIDPI(pi))
		pi->Delete();
}

void cTargets::GhostTarget(NXWSOCKET s)
{
    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
    if(ISVALIDPC(pc))
    {
        if(!pc->dead)
        {
		pc->attackerserial=pc->getSerial32();
		bolteffect(DEREF_P_CHAR(pc), true);
		pc->playSFX( 0x0029);
		pc->Kill();
        }
        else
            sysmessage(s,TRANSLATE("That player is already dead."));
    }
}

#ifdef PDDAFARE

class cBoltTarget : public cCharTarget
{
public:
    cBoltTarget(NXWCLIENT pCli) : cCharTarget(pCli) {}
    void CharSpecific()
    {
        if (w_anim[0]==0 && w_anim[1]==0)
        {
            bolteffect(inx, true);
            P_CHAR pc_inx=MAKE_CHAR_REF(inx);
			if(ISVALIDPC(pc_inx))
				pc_inx->playSFX( 0x0029);
        }
        else
        {
            for (int j=0;j<=333;j++) bolteffect2(inx,w_anim[0],w_anim[1]);
        }
    }
};

class cSetAmountTarget : public cItemTarget
{
public:
    cSetAmountTarget(NXWCLIENT pCli) : cItemTarget(pCli) {}
    void ItemSpecific()
    {
        if (addx[s] > 64000) //Ripper..to fix a client bug for over 64k.
        {
            sysmessage(s, TRANSLATE("No amounts over 64k in a pile!"));
            return;
        }
		P_ITEM pi= MAKE_ITEM_REF(inx);
		VALIDATEPI(pi);

        pi->amount=addx[s];
        pi->Refresh();
    }
};

#endif

void cTargets::CloseTarget(NXWSOCKET s)
{
    if(s<=INVALID)
		return;
	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);
	P_CHAR pc=pointers::findCharBySerPtr(buffer[s]+7);
    if(ISVALIDPC(pc))
    {
		curr->sysmsg( TRANSLATE("Kicking player"));
        pc->kick();
    }
}

int cTargets::AddMenuTarget(NXWSOCKET s, int x, int addmitem)
{
    if ( (s>=0) && LongFromCharPtr(buffer[s]+11) == INVALID)
		return INVALID;

    P_ITEM pi = item::CreateScriptItem(s, addmitem, 0);
    VALIDATEPIR( pi, INVALID );

    if (x)
		pi->Refresh();

    return DEREF_P_ITEM(pi);
}

// public !!!

void cTargets::VisibleTarget (NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);

    if(isItemSerial(serial))//item
    {
        P_ITEM pi = pointers::findItemBySerial(serial);
        if(ISVALIDPI(pi))
        {
            pi->visible=addx[s];
            pi->Refresh();
        }
    } else
    {//char
        P_CHAR pc = pointers::findCharBySerial(serial);
        if(ISVALIDPC(pc))
        {
            pc->hidden=addx[s];
	    pc->teleport();
        }
    }
}

/*!
\author Duke
\date 29/12/2000
\brief used by the '#empty' user-command
*/
static void ContainerEmptyTarget1(NXWCLIENT ps, P_ITEM pi)
{
	if (ps == NULL) return;
	VALIDATEPI(pi);
    NXWSOCKET  s = ps->toInt();
    if (pi->getSerial32() >= 0x40000000  // a container ?
        && pi->type==1)
    {
        addx[s]= pi->getSerial32();
        target(s, 0, 1, 0, 72, TRANSLATE("Select container to fill:"));
    }
    else
        sysmessage(s,TRANSLATE("That is not a valid container!"));

}
static void ContainerEmptyTarget2(NXWCLIENT ps, P_ITEM pNewCont)
{
	if (ps == NULL) return;
	/*VALIDATEPI(pNewCont);
    NXWSOCKET  s = ps->toInt();
    if (pNewCont->type==1)
    {
        P_ITEM pi;  // item to move from old container
        int ci=0,loopexit=0;
        while ( ((pi=ContainerSearch(addx[s],&ci)) != NULL) && (++loopexit < MAXLOOPS) )
            pNewCont->AddItem(pi);
    }
    else
        sysmessage(s,TRANSLATE("That is not a valid container!"));
*/
	sysmessage( ps->toInt(), "DISABLED" );
}

static void OwnerTarget(NXWCLIENT ps, P_CHAR pc)
{
	if (ps == NULL) return;
	VALIDATEPC(pc);
    NXWSOCKET  s = ps->toInt();

    int addser=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
    pc->setOwnerSerial32(addser);
    if (addser==-1)
    {
        pc->tamed=false;
    }
    else
    {
        pc->tamed=true;
    }
}

static void OwnerTarget(NXWCLIENT ps, P_ITEM pi)
{
	if (ps == NULL) return;
	VALIDATEPI(pi);
    NXWSOCKET  s = ps->toInt();

    int os=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
    pi->setOwnerSerial32(os);

    if (pi->visible==1)
    {
        for (int j=0;j<now;j++) if (perm[j]) senditem(j,pi); // necassairy for items with visible value of 1
    }
}



static void InfoTarget(NXWSOCKET s, PKGx6C *pp) // rewritten to work also with map-tiles, not only static ones by LB
{

    if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
    int x=pp->TxLoc;
    int y=pp->TyLoc;
    signed char z=pp->TzLoc;

    if (pp->model==0)   // damn osi not me why the tilenum is only send for static tiles, LB
    {   // manually calculating the ID's if it's a maptype
        
		map_st map1;
	    land_st land;

		data::seekMap( x, y, map1 );
        data::seekLand(map1.id, land);
        ConOut("type: map-tile\n");
        ConOut("tilenum: %i\n",map1.id);
        ConOut("Flags:%x\n", land.flags);
        ConOut("Name:%s\n", land.name);
    }
    else
    {
        int tilenum=pp->model; // lb, bugfix
		tile_st tile;
        data::seekTile(tilenum, tile);
        ConOut("type: static-tile\n");
        ConOut("tilenum: %i\n",tilenum);
        ConOut("Flags:%x\n", tile.flags);
        ConOut("Weight:%x\n", tile.weight);
        ConOut("Layer:%x\n", tile.quality);
        ConOut("Anim:%lx\n", tile.animid);
        ConOut("Unknown1:%lx\n", tile.unknown);
        ConOut("Unknown2:%x\n", tile.unknown2);
        ConOut("Unknown3:%x\n", tile.unknown3);
        ConOut("Height:%x\n", tile.height);
        ConOut("Name:%s\n", tile.name);
    }
    sysmessage(s, TRANSLATE("Item info has been dumped to the console."));
    ConOut("\n");
}

/*
void cTargets::TweakTarget(NXWSOCKET s)//Lag fix -- Zippy
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if (serial == 0) //Client sends zero if invalid!
        return;
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
    VALIDATEPC( pc );

    if (isCharSerial(serial))//Char
    {
        if (pointers::findCharBySerial(serial) == NULL) // but let's make sure
            return;
		newAmxEvent("gui_charProps")->Call( serial, pc->getSerial32(), 1 );
        //tweakmenu(s, serial);
    }
    else
    {//item
        if (FindItemBySerial(serial) == NULL)
            return;
		newAmxEvent("gui_itemProps")->Call( serial, pc->getSerial32(), 1 );
        //tweakmenu(s, serial);
    }
}
*/


void CarveTarget(NXWSOCKET s, int feat, int ribs, int hides, int fur, int wool, int bird)
{
        if ( s < 0 || s >= now )
		return;
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC( pc );

	P_ITEM pi1 = item::CreateFromScript( "$item_blood_puddle" );
	VALIDATEPI(pi1);
	pi1->setId( 0x122A );
	P_ITEM pi2=MAKE_ITEM_REF(npcshape[0]);
	VALIDATEPI(pi2);
#ifdef SPAR_I_LOCATION_MAP
	pi1->setPosition( pi2->getPosition() );
	pointers::updateLocationMap(pi1);
#else
	mapRegions->remove(pi1);
	pi1->setPosition( pi2->getPosition() );
	mapRegions->add(pi1); // lord Binary
#endif
	pi1->magic=2;//AntiChrist - makes the item unmovable
	pi1->setDecayTime();
	pi1->Refresh();

	if(feat>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_feathers", pc->getBackpack(), feat );
		VALIDATEPI(pi);
		pi->Refresh();
		sysmessage(s,TRANSLATE("You pluck the bird and get some feathers."));
	}
	if(ribs>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_cuts_of_raw_ribs", pc->getBackpack(), ribs );
		VALIDATEPI(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You carve away some meat."));
	}

	if(hides>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_hide", pc->getBackpack(), hides );
		VALIDATEPI(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You skin the corpse and get the hides."));
	}
	if(fur>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_hide", pc->getBackpack(), fur );
		VALIDATEPI(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You skin the corpse and get the hides."));
	}
	if(wool>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_piles_of_wool", pc->getBackpack(), wool );
		VALIDATEPI(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You skin the corpse and get some unspun wool."));
	}
	if(bird>0)
	{
		P_ITEM pi = item::CreateFromScript( "$item_raw_bird", pc->getBackpack(), bird );
		VALIDATEPI(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You carve away some raw bird."));
	}

	weights::NewCalc(pc);
}

/*!
\author AntiChrist
\date 03/11/1999
\brief New Carving System
\note Human-corpse carving code added
\note Scriptable carving product added
*/
static void newCarveTarget(NXWSOCKET  s, ITEM i)
{
	bool deletecorpse=false;
	char sect[512];
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	P_ITEM pi1 = item::CreateFromScript( "$item_blood_puddle" );
	VALIDATEPI(pi1);
	pi1->setId( 0x122A );
	P_ITEM pi2=MAKE_ITEM_REF(npcshape[0]);
	VALIDATEPI(pi2);
	P_ITEM pi3=MAKE_ITEM_REF(i);
	VALIDATEPI(pi3);
#ifdef SPAR_I_LOCATION_MAP
	pi1->setPosition( pi2->getPosition() );
	pointers::updateLocationMap(pi1);
#else
	mapRegions->remove(pi1);
	pi1->setPosition( pi2->getPosition() );
	mapRegions->add(pi1); // lord Binary
#endif
	pi1->magic=2;//AntiChrist - makes the item unmovable
	pi1->setDecayTime();

	pi1->Refresh();

	//if it's a human corpse
	if(pi3->morey)
	{
		pc->modifyFame(ServerScp::g_nChopFameLoss); // Ripper..lose fame and karma and criminal.
		pc->IncreaseKarma(+ServerScp::g_nChopKarmaLoss);
		pc->sysmsg(TRANSLATE("You lost some fame and karma!"));
		setCrimGrey(pc, ServerScp::g_nChopWillCriminal);//Blue and not attacker and not guild

		//create the Head
		sprintf(temp,"the head of %s",pi3->getSecondaryNameC());
               	P_ITEM pi = item::CreateFromScript( "$item_hardcoded" );
                VALIDATEPI(pi);
                pi->setId( 0x1DA0 );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		//AntiChrist & Magius(CHE) - store item's owner, so that lately
		//if we want to know the owner we can do it
		pi->setSameOwnerAs(pi3);
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Heart
		sprintf(temp,"the heart of %s",pi3->getSecondaryNameC());
		pi=item::CreateFromScript( "$item_a_heart" );
		VALIDATEPI(pi);
		pi->setCurrentName( temp );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Body
		sprintf(temp,"the body of %s",pi3->getSecondaryNameC());
               	pi = item::CreateFromScript( "$item_hardcoded" );
                VALIDATEPI(pi);
                pi->setId( 0x1DAD );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Left Arm
		sprintf(temp,"the left arm of %s",pi3->getSecondaryNameC());
               	pi = item::CreateFromScript( "$item_hardcoded" );
                VALIDATEPI(pi);
                pi->setId( 0x1DA1 );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Right Arm
		sprintf(temp,"the right arm of %s",pi3->getSecondaryNameC());
               	pi = item::CreateFromScript( "$item_hardcoded" );
                VALIDATEPI(pi);
                pi->setId( 0x1DA2 );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Left Leg
		sprintf(temp,"the left leg of %s",pi3->getSecondaryNameC());
               	pi = item::CreateFromScript( "$item_hardcoded" );
                VALIDATEPI(pi);
                pi->setId( 0x1DAE );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//create the Rigth Leg
		sprintf(temp,"the right leg of %s",pi3->getSecondaryNameC());
               	pi = item::CreateFromScript( "$item_hardcoded" );
                VALIDATEPI(pi);
                pi->setId( 0x1DA4 );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->setCurrentName( temp ); //Luxor
		pi->setDecay();
		pi->Refresh();

		//human: always delete corpse!
		deletecorpse=true;
	}
		else
	{
		cScpIterator* iter = NULL;
		char script1[1024];
		char script2[1024];
		sprintf(sect,"SECTION CARVE %i",pi3->carve);
		iter = Scripts::Carve->getNewIterator(sect);
		if (iter==NULL) return;


		int loopexit=0;
		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if (!(strcmp("ADDITEM",script1)))
				{
					std::string itemnum, amount;
					splitLine( script2, itemnum, amount );
					int amt = str2num( amount );
					if( amt == 0 )
						amt=INVALID;
					P_ITEM pi = item::CreateFromScript( (char*)itemnum.c_str(), pi3, amt );
					if( ISVALIDPI(pi) ) {
						pi->layer=0;
						pi->Refresh();//let's finally refresh the item
					}
                }
            }
        }
        while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

        safedelete(iter);
    }


    if(deletecorpse)//if corpse has to be deleted
    {

		NxwItemWrapper si;
		si.fillItemsInContainer( pi3, false );
		for( si.rewind(); !si.isEmpty(); si++ )
        {
            P_ITEM pj=si.getItem();
			if(ISVALIDPI(pj)) {
				pj->setContSerial(INVALID);
				pj->MoveTo( pi3->getPosition() );
				pj->setDecayTime();
				pj->Refresh();
			}
        }
        pi3->Delete();
    }
}

static void CorpseTarget(const NXWCLIENT pC)
{
	if (pC == NULL) return;
    int n=0;
    NXWSOCKET  s = pC->toInt();

    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcItemFromSer(serial);
    P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPI(pi);
    P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
    if(ISVALIDPI(pi))
    {
        if(item_inRange(pc,pi,1))
        {
            npcshape[0]=i;
            pc->playAction(0x20);
            n=1;
            if(pi->more1==0)
            {
                pi->more1=1;//corpse being carved...can't carve it anymore

                if(pi->morey || pi->carve>-1)
                {//if specified, use enhanced carving system!
                    newCarveTarget(s, i);//AntiChrist
                } else
                {//else use standard carving
                    switch(pi->amount) {
                    case 0x01: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Ogre
                    case 0x02: CarveTarget(s, 0, 5, 0, 0, 0, 0); break; //Ettin
                    case 0x03: break;   //Zombie
                    case 0x04: break;   //Gargoyle
                    case 0x05: CarveTarget(s,36, 0, 0, 0, 0, 1); break; //Eagle
                    case 0x06: CarveTarget(s,25, 0, 0, 0, 0, 1); break; //Bird
                    case 0x07: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Orc w/axe
                    case 0x08: break;   //Corpser
                    case 0x09: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Deamon
                    case 0x0A: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Deamon w/sword
                    case 0x0B: break;   //-NULL-
                    case 0x0C: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Dragon (green)
                    case 0x0D: break;   //Air Elemental
                    case 0x0E: break;   //Earth Elemental
                    case 0x0F: break;   //Fire Elemental
                    case 0x10: break;   //Water Elemental
                    case 0x11: CarveTarget(s, 0, 3, 0, 0, 0, 0); break; //Orc
                    case 0x12: CarveTarget(s, 0, 5, 0, 0, 0, 0); break; //Ettin w/club
                    case 0x13: break; //-NULL-
                    case 0x14: break; //-NULL-
                    case 0x15: CarveTarget(s, 0, 4,20, 0, 0, 0); break; //Giant Serpent
                    case 0x16: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Gazer
                    case 0x17: break;   //-NULL-
                    case 0x18: break;   //Liche
                    case 0x19: break;   //-NULL-
                    case 0x1A: break;   //Ghoul
                    case 0x1B: break;   //-NULL-
                    case 0x1C: break;   //Spider
                    case 0x1D: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Gorilla
                    case 0x1E: CarveTarget(s,50, 0, 0, 0, 0, 1); break; //Harpy
                    case 0x1F: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Headless
                    case 0x20: break;   //-NULL-
                    case 0x21: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman
                    case 0x0122: CarveTarget(s, 0,10, 0, 0, 0, 0); break; // Boar
                    case 0x23: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman w/spear
                    case 0x24: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman w/mace
                    case 0x25: break;   //-NULL-
                    case 0x26: break;   //-NULL-
                    case 0x27: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Mongbat
                    case 0x28: break;   //-NULL-
                    case 0x29: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Orc w/club
                    case 0x2A: break;   //Ratman
                    case 0x2B: break;   //-NULL-
                    case 0x2C: break;   //Ratman w/axe
                    case 0x2D: break;   //Ratman w/dagger
                    case 0x2E: break;   //-NULL-
                    case 0x2F: break;   //Reaper
                    case 0x30: break;   //Scorpion
                    case 0x31: break;   //-NULL-
                    case 0x32: break;   //Skeleton
                    case 0x33: break;   //Slime
                    case 0x34: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Snake
                    case 0x35: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll w/axe
                    case 0x36: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll
                    case 0x37: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll w/club
                    case 0x38: break;   //Skeleton w/axe
                    case 0x39: break;   //Skeleton w/sword
                    case 0x3A: break;   //Wisp
                    case 0x3B: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Dragon (red)
                    case 0x3C: CarveTarget(s, 0,10,20, 0, 0, 0); break; //Drake (green)
                    case 0x3D: CarveTarget(s, 0,10,20, 0, 0, 0); break; //Drake (red)
                    case 0x46: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen Matriarche - t2a
                    case 0x47: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen drone - t2a
                    case 0x48: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen warrior, Terathen Avenger - t2a
                    case 0x4B: CarveTarget(s, 0,4, 0, 0, 0, 0); break; //Titan - t2a
                    case 0x4C: CarveTarget(s, 0, 4, 0, 0, 0, 0); break; //Cyclopedian Warrior - t2a
                    case 0x50: CarveTarget(s, 0,10, 2, 0, 0, 0); break; //Giant Toad - t2a
                    case 0x51: CarveTarget(s, 0, 4, 1, 0, 0, 0); break; //Bullfrog - t2a
                    case 0x55: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian apprentice, Ophidian Shaman - t2a
                    case 0x56: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian warrior, Ophidian Enforcer, Ophidian Avenger - t2a
                    case 0x57: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian Matriarche - t2a
                    case 0x5F: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Kraken - t2a
                        //case 0x3E-case 0x95: break; //-NULL-
                    case 0x96: CarveTarget(s, 0,10, 0, 0, 0, 0); break; //Sea Monster
                    case 0x97: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Dolphin
                        //case 0x98-case 0xC7: break; //-NULL-
                    case 0xC8: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (tan)
                    case 0xC9: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Cat
                    case 0xCA: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Alligator
                    case 0xCB: CarveTarget(s, 0, 6, 0, 0, 0, 0); break; //Pig
                    case 0xCC: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (dark)
                    case 0xCD: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Rabbit
                    case 0xCE: CarveTarget(s, 0, 1,12, 0 ,0, 0); break; //Lava Lizard - t2a
                    case 0xCF: CarveTarget(s, 0, 3, 0, 0, 1, 0); break; //Sheep
                    case 0xD0: CarveTarget(s,25, 0, 0, 0, 0, 1); break; //Chicken
                    case 0xD1: CarveTarget(s, 0, 2, 8, 0, 0, 0); break; //Goat
                    case 0xD2: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Desert Ostarge - t2a
                    case 0xD3: CarveTarget(s, 0, 1, 12, 0, 0, 0); break; //Bear
                    case 0xD4: CarveTarget(s, 0, 1, 0, 2, 0, 0); break; //Grizzly Bear
                    case 0xD5: CarveTarget(s, 0, 2, 0, 3, 0, 0); break; //Polar Bear
                    case 0xD6: CarveTarget(s, 0, 1, 10, 0, 0, 0); break; //Cougar
                    case 0xD7: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Giant Rat
                    case 0xD8: CarveTarget(s, 0, 8,12, 0, 0, 0); break; //Cow (black)
                    case 0xD9: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Dog
                    case 0xDA: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Frenzied Ostard - t2a
                    case 0xDB: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Forest Ostard - t2a
                    case 0xDC: CarveTarget(s, 0, 1, 12,0, 0, 0); break; //Llama
                    case 0xDD: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Walrus
                    case 0xDE: break;   //-NULL-
                    case 0xDF: CarveTarget(s, 0, 3, 0, 0, 0, 0); break; //Sheep (BALD)
                    case 0xE1: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Timber Wolf
                    case 0xE2: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (Silver)
                    case 0xE3: break;   //-NULL-
                    case 0xE4: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (tan)
                    case 0xE5: break;   //-NULL-
                    case 0xE6: break;   //-NULL-
                    case 0xE7: CarveTarget(s, 0, 8,12, 0, 0, 0); break; //Cow (brown)
                    case 0xE8: CarveTarget(s, 0,10,15, 0, 0, 0); break; //Bull (brown)
                    case 0xE9: CarveTarget(s, 0,10,15, 0, 0, 0); break; //Bull (d-brown)
                    case 0xEA: CarveTarget(s, 0, 6,15, 0, 0, 0); break; //Great Heart
                    case 0xEB: break;   //-NULL-
                    case 0xEC: break;   //-NULL-
                    case 0xED: CarveTarget(s, 0, 5, 8, 0, 0, 0); break; //Hind
                    case 0xEE: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Rat
                        //case 0xEF-case 0xFF: break; //-NULL-
                    default:
                        LogError("Fallout of switch statement, corpsetarget()");
                    }// switch
                }//if morey || carve>-1
            } else {
                 sysmessage(s, TRANSLATE("You carve the corpse but find nothing usefull."));
            }// if more1==0
        //break;
        }
    }// if i!=-1
    if (!n) sysmessage(s, TRANSLATE("That is too far away."));
}

void cTargets::NpcTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s] +7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
    {
        addid1[s]= pc->getSerial().ser1;
        addid2[s]= pc->getSerial().ser2;
        addid3[s]= pc->getSerial().ser3;
        addid4[s]= pc->getSerial().ser4;
        target(s, 0, 1, 0, 57, TRANSLATE("Select NPC to follow this player."));
    }
}

void cTargets::NpcTarget2(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
        if (pc->npc==1)
        {
            pc->ftargserial=calcserial(addid1[s], addid2[s], addid3[s], addid4[s]);
            pc->npcWander=WANDER_FOLLOW;
        }
}

void cTargets::NpcRectTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
        if ((pc->npc==1))
        {
            pc->fx1=addx[s];
            pc->fy1=addy[s];
            pc->fz1=-1;
            pc->fx2=addx2[s];
            pc->fy2=addy2[s];
            pc->npcWander=WANDER_FREELY_BOX;
        }
}

void cTargets::NpcCircleTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
        if ((pc->npc==1))
        {
            pc->fx1=addx[s];
            pc->fy1=addy[s];
            pc->fz1=-1;
            pc->fx2=addx2[s];
            pc->npcWander=WANDER_FREELY_CIRCLE; // bugfix, LB
        }
}

void cTargets::NpcWanderTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
        if ((pc->npc==1)) pc->npcWander=npcshape[0];
}

//taken from 6904t2(5/10/99) - AntiChrist
void cTargets::NpcAITarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
    {
        pc->npcaitype=addx[s];
        sysmessage(s, "Npc AI changed.");//AntiChrist
    }
}

void cTargets::xBankTarget(NXWSOCKET s)
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC(pc);
	P_CHAR pc2 = pointers::findCharBySerPtr(buffer[s] +7);
	VALIDATEPC(pc2);

	pc->openBankBox(pc2);
}

void cTargets::xSpecialBankTarget(NXWSOCKET s)
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC(pc);
	P_CHAR pc2 = pointers::findCharBySerPtr(buffer[s] +7);
	VALIDATEPC(pc2);

    pc->openSpecialBank(pc2);
}

void cTargets::SellStuffTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    if (i!=-1)
    {
        sellstuff(s, i);
    }
}

void cTargets::ReleaseTarget(NXWSOCKET s, int c)
{

	P_CHAR pc = NULL;
	if (c==INVALID)
		pc = pointers::findCharBySerPtr(buffer[s] + 7);
	else
		pc = pointers::findCharBySerial(c);
	P_CHAR rel = MAKE_CHAR_REF(currchar[s]);

	prison::release( rel, pc  );
}

/*!
\brief opens a creature's backpack
\author Luxor
*/
void cTargets::GmOpenTarget(NXWSOCKET s)
{
	P_ITEM pi;
	P_CHAR pc= MAKE_CHAR_REF(currchar[s]);
	P_CHAR pc_target = pointers::findCharBySerPtr(buffer[s]+7);
	VALIDATEPC(pc);
	VALIDATEPC(pc_target);

	NxwItemWrapper si;
	si.fillItemWeared(pc_target);
	for (si.rewind(); !si.isEmpty(); si++) {
        	pi = si.getItem();
		if (pi->layer == addmitem[s]) {
			pc->showContainer(pi);
			return;
		}
	}

	pc->sysmsg(TRANSLATE("No object was found at that layer on that character"));
}

void cTargets::StaminaTarget(NXWSOCKET s)
{
    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
    if (ISVALIDPC(pc))
    {
        pc->playSFX( 0x01F2);
        staticeffect(DEREF_P_CHAR(pc), 0x37, 0x6A, 0x09, 0x06);
        pc->stm=pc->dx;
        pc->updateStats(2);
        return;
    }
    sysmessage(s,TRANSLATE("That is not a person."));
}

void cTargets::ManaTarget(NXWSOCKET s)
{
    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
    if (ISVALIDPC(pc))
    {
        pc->playSFX( 0x01F2);
        staticeffect(DEREF_P_CHAR(pc), 0x37, 0x6A, 0x09, 0x06);
        pc->mn=pc->in;
        pc->updateStats(1);
        return;
    }
    sysmessage(s,TRANSLATE("That is not a person."));
}

void cTargets::MakeShopTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
	P_CHAR pc=MAKE_CHAR_REF(i);
    if (pc!=NULL)
    {
        Commands::MakeShop(i);
        pc->teleport();
        sysmessage(s, TRANSLATE("The buy containers have been added."));
        return;
    }
    sysmessage(s, TRANSLATE("Target character not found..."));
}

void cTargets::BuyShopTarget(NXWSOCKET s)
{
    P_CHAR pc =pointers::findCharBySerPtr(buffer[s]+7);
    if (ISVALIDPC(pc))
        if ((pc->getSerial32() == pc->getSerial32()))
        {
            Targ->BuyShop(s, DEREF_P_CHAR(pc));
            return;
        }
	sysmessage(s, TRANSLATE("Target shopkeeper not found..."));
}

int cTargets::BuyShop(NXWSOCKET s, CHARACTER c)
{
    P_ITEM buyRestockContainer=NULL, buyNoRestockContainer=NULL;

    P_CHAR pc = MAKE_CHAR_REF(c);
	VALIDATEPCR(pc,0);
	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(curr,0);


	NxwItemWrapper si;
	si.fillItemWeared( pc, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ )
    {
        P_ITEM pi=si.getItem();
		if(!ISVALIDPI(pi)) continue;

		if( pi->layer==LAYER_TRADE_RESTOCK )
            buyRestockContainer=pi;

        if( pi->layer==LAYER_TRADE_NORESTOCK )
            buyNoRestockContainer=pi;

		if( ISVALIDPI(buyRestockContainer) && ISVALIDPI(buyNoRestockContainer) )
			break;
    }

    if (!ISVALIDPI(buyRestockContainer) || !ISVALIDPI(buyNoRestockContainer) )
        return 0;

    impowncreate(s, pc, 0); // Send the NPC again to make sure info is current. (OSI does this we might not have to)

    sendshopinfo(s, DEREF_P_CHAR(pc), buyRestockContainer); // Send normal shop items
//  sendshopinfo(s, c, buyNoRestockContainer); // Send items sold to shop by players
    SndShopgumpopen(s,pc->getSerial32());
    statwindow(curr,curr); // Make sure the gold total has been sent.
    return 1;
}

//////////////////////////////////
// Changed hideing to make flamestrike and hide work better
//
//
void cTargets::permHideTarget(NXWSOCKET s)
{
	SERIAL serial = LongFromCharPtr(buffer[s] + 7);
	int i = calcCharFromSer(serial);
	P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
	Location pcpos= pc->getPosition();

	if (i!=-1)
	{
		if (pc->hidden == HIDDEN_BYSKILL)
		{
			if (i == currchar[s])
				sysmessage(s, TRANSLATE("You are already hiding."));
			else
				sysmessage(s, TRANSLATE("He is already hiding."));
			return;
		}

		pc->priv2 |= CHRPRIV2_PERMAHIDDEN;
		staticeffect3(pcpos.x + 1, pcpos.y + 1, pcpos.z + 10, 0x37, 0x09, 0x09, 0x19, 0);
		pc->playSFX(0x0208);
		tempfx::add(pc, pc, tempfx::GM_HIDING, 1, 0, 0);

		return;
    }
}
//
//
// Aldur
//////////////////////////////////

//////////////////////////////////
// Changed unhideing to make flamestrike and unhide work better
//
//
void cTargets::unHideTarget(NXWSOCKET s)
{
	SERIAL serial = LongFromCharPtr(buffer[s] + 7);
	int i = calcCharFromSer(serial);
	P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);

	Location pcpos= pc->getPosition();

	if (i!=-1)
	{
		if (pc->hidden == UNHIDDEN)
		{
			if (i == currchar[s])
				sysmessage(s, TRANSLATE("You are not hiding."));
			else
				sysmessage(s, TRANSLATE("He is not hiding."));
			return;
		}

		pc->priv2 &= ~CHRPRIV2_PERMAHIDDEN;

		staticeffect3(pcpos.x + 1, pcpos.y + 1, pcpos.z + 10, 0x37, 0x09, 0x09, 0x19, 0);
		pc->playSFX(0x0208);
		tempfx::add(pc, pc, tempfx::GM_UNHIDING, 1, 0, 0);

		return;
	}
}
//
//
// Aldur
//////////////////////////////////

void cTargets::SetSpeechTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    if (i!=-1)
    {
        P_CHAR pc = MAKE_CHAR_REF(i);
		VALIDATEPC(pc);
        if (pc->npc==0)
        {
            sysmessage(s,"You can only change speech for npcs.");
            return;
        }
        pc->speech=addx[s];
    }
}

static void SetSpAttackTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
    {
        pc->spattack=tempint[s];
    }
}

void cTargets::SetSpaDelayTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
    {
        pc->spadelay=tempint[s];
    }
}

void cTargets::SetPoisonTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
    {
        pc->poison=tempint[s];
    }
}

void cTargets::SetPoisonedTarget(NXWSOCKET s)
{
    P_CHAR pc = pointers::findCharBySerPtr(buffer[s] +7);
    if (ISVALIDPC(pc))
    {
        pc->poisoned=(PoisonType)tempint[s];
        pc->poisonwearofftime=uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParms->poisontimer); // lb, poison wear off timer setting
        impowncreate(pc->getSocket(), pc, 1); //Lb, sends the green bar !
    }
}

void cTargets::FullStatsTarget(NXWSOCKET s)
{
    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
    if (ISVALIDPC(pc))
    {
        pc->playSFX( 0x01F2);
        staticeffect(DEREF_P_CHAR(pc), 0x37, 0x6A, 0x09, 0x06);
        pc->mn=pc->in;
        pc->hp=pc->getStrength();
        pc->stm=pc->dx;
        pc->updateStats(0);
        pc->updateStats(1);
        pc->updateStats(2);
        return;
    }
    sysmessage(s,TRANSLATE("That is not a person."));
}

void cTargets::SetAdvObjTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
    {
        pc->advobj=tempint[s];
    }
}

////////////////
// name:        CanTrainTarget
// history:     by Antrhacks 1-3-99
// Purpose:     Used for training by NPC's
//
void cTargets::CanTrainTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
    {
        if (pc->npc==0)
        {
            sysmessage(s, "Only NPC's may train.");
            return;
        }
        pc->cantrain = !pc->cantrain;   //turn on if off, off if on
    }
}

void cTargets::SetSplitTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
    {
        pc->split=tempint[s];
    }
}

void cTargets::SetSplitChanceTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if (i!=-1)
    {
        pc->splitchnc=tempint[s];
    }
}

void cTargets::SetDirTarget(NXWSOCKET s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	if (serial == -1)
		return;

	if (isItemSerial(serial))
	{
		P_ITEM pi = pointers::findItemBySerial(serial);
		if (pi)
		{
			pi->dir=addx[s];
			pi->Refresh();
			return;
		}
	}
	else
	{
		P_CHAR pc = pointers::findCharBySerial(serial);
		if (pc)
		{
			pc->dir=addx[s];
			pc->teleport();
			return;
		}
	}
}

void cTargets::NewXTarget(NXWSOCKET s) // Notice a high similarity to th function above? Wonder why. - Gandalf
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcItemFromSer(serial);
    P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPI(pi);
    if (i!=-1)
    {
        //item::MoveTo(i,addx[s],pi->y,pi->z);
		pi->MoveTo( addx[s], pi->getPosition("y"), pi->getPosition("z"));
        pi->Refresh();
    }

    i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
	Location pcpos= pc->getPosition();

    if (i!=-1)
    {
        //Char_MoveTo(i, addx[s], pcpos.y, pcpos.z);
		pc->MoveTo( addx[s], pcpos.y, pcpos.z );
        pc->teleport();
    }
}

void cTargets::NewYTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcItemFromSer(serial);
    P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPI(pi);
    if (i!=-1)
    {
        //item::MoveTo(i,pi->x,addx[s],pi->z);
		pi->MoveTo( pi->getPosition("x"), addx[s], pi->getPosition("z"));
        pi->Refresh();
    }

    i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
	Location pcpos= pc->getPosition();

    if (i!=-1)
    {
        //Char_MoveTo(i, pcpos.x, addx[s], pi->z);
		pc->MoveTo( pcpos.x, addx[s], pcpos.z );
        pc->teleport();
    }
}

void cTargets::IncXTarget(NXWSOCKET s)
{
	Location position;
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	int i=calcItemFromSer(serial);
	P_ITEM pi=MAKE_ITEM_REF(i);
	if( ISVALIDPI( pi ) )
	{
		position = pi->getPosition();
		position.x += addx[s];
		pi->MoveTo( position );
		pi->Refresh();
	}
	else
	{
		i=calcCharFromSer(serial);
		P_CHAR pc = MAKE_CHAR_REF(i);
		if ( ISVALIDPC( pc ) )
		{
			position = pc->getPosition();
			position.x += addx[s];
			pc->MoveTo( position );
			pc->teleport();
		}
	}
}

void cTargets::IncYTarget(NXWSOCKET s)
{
	Location position;
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	int i=calcItemFromSer(serial);
	P_ITEM pi=MAKE_ITEM_REF(i);
	if( ISVALIDPI( pi ) )
	{
		position = pi->getPosition();
		position.y += addx[s];
		pi->MoveTo( position );
		pi->Refresh();
	}
	else
	{
		i=calcCharFromSer(serial);
		P_CHAR pc = MAKE_CHAR_REF(i);
		if ( ISVALIDPC( pc ) )
		{
			position = pc->getPosition();
			position.y += addx[s];
			pc->MoveTo( position );
			pc->teleport();
		}
	}
}

/*
void cTargets::IncZTarget(NXWSOCKET s)
{
	Location position;
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	int i=calcItemFromSer(serial);
	P_ITEM pi=MAKE_ITEM_REF(i);
	if( ISVALIDPI( pi ) )
	{
		position = pi->getPosition();
		position.z += addx[s];
		pi->MoveTo( position );
		pi->Refresh();
	}
	else
	{
		i=calcCharFromSer(serial);
		P_CHAR pc = MAKE_CHAR_REF(i);
		if ( ISVALIDPC( pc ) )
		{
			position = pi->getPosition();
			position.z += addx[s];
			pc->MoveTo( position );
			pc->teleport();
		}
	}
}
*/


/*
void cTargets::ShowPriv3Target(NXWSOCKET s) // crackerjack, jul 25/99
{
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int p=calcCharFromSer(serial);
     P_CHAR pc = MAKE_CHARREF_LR(p);
    if (p!=-1)
    {
        char priv_info[10248];
        int i;
        sprintf(priv_info, "%s can execute the following commands:\n", pc->getCurrentNameC());
        i=0; int loopexit=0;
        while(command_table[i].cmd_name && (++loopexit < MAXLOOPS) )
        {
            if(command_table[i].cmd_priv_m==255||
                (pc->priv3[command_table[i].cmd_priv_m]&
                (0-0xFFFFFFFF<<command_table[i].cmd_priv_b)))
            {
                sprintf((char*)temp, " %s", command_table[i].cmd_name);
                strcpy(&priv_info[strlen(priv_info)], (char*)temp);
            }
            i++;
        }
        SndUpdscroll(s, strlen(priv_info), priv_info);
    }
    else
        sysmessage(s, "You cannot retrieve privilige information on that.");
}
*/

void cTargets::ShowSkillTarget(NXWSOCKET s) // LB's showskills
{
    int a,j,k,b=0,c,z,zz,ges=0;
    char skill_info[(ALLSKILLS+1)*40];
    char sk[25];
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);

    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
	if(ISVALIDPC(pc)) {

		z=addx[s];
        if (z<0 || z>3) z=0;
        if (z==2 || z==3)
            sprintf(skill_info, "%s's skills:", pc->getCurrentNameC());
        else
            sprintf(skill_info, "%s's baseskills:", pc->getCurrentNameC());

        //b=strlen(pc->name)+11;
		b= strlen(pc->getCurrentNameC()) + 11;
        if (b>23) b=23;

        for (c=b;c<=26;c++)
            strcpy(&skill_info[strlen(skill_info)], " ");

        numtostr(ges,sk);
        sprintf(temp,"sum: %s",sk);
        strcpy(&skill_info[strlen(skill_info)],temp);

        for (a=0;a<ALLSKILLS;a++)
        {
            if (z==0 || z==1) k=pc->baseskill[a]; else k=pc->skill[a];
            if (z==0 || z==2) zz=9; else zz=-1;

            if (k>zz) // show only if skills >=1
            {
                if (z==2 || z==3) j=pc->skill[a]/10; else j=pc->baseskill[a]/10;    // get skill value
                numtostr(j,sk);     // skill-value string in sk
                ges+=j;
                sprintf(temp, "%s %s", skillname[a],sk);
                strcpy(&skill_info[strlen(skill_info)],temp);

                b=strlen(skillname[a])+strlen(sk)+1; // it doesnt like \n's, so insert spaces till end of line
                if (b>23) b=23;
                for (c=b;c<=26;c++)
                     strcpy(&skill_info[strlen(skill_info)], " ");
            }
        }
        numtostr(ges,sk);
        sprintf(temp,"sum: %s  ",sk);
        strcpy(&skill_info[strlen(skill_info)],temp);

        SndUpdscroll(s, strlen(skill_info), skill_info);
    }
    else
        sysmessage(s,TRANSLATE("no valid target"));
}

void cTargets::ResurrectionTarget( NXWSOCKET  s )
{
	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);

    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
	if(ISVALIDPC(pc)) {

        if (pc->dead)
        {
            pc->resurrect();
            return;
        }
    }
}

//AntiChrist - shows the COMMENT line in the account section of player current acct.
void cTargets::ShowAccountCommentTarget(NXWSOCKET s)
{
//TO BE REMOVED
}

void cTargets::SetHome(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if(serial==-1) return;
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if(i!=-1)
    {
        pc->homeloc.x=addx[s];
        pc->homeloc.y=addy[s];
        pc->homeloc.z=addz[s];
    }
}

void cTargets::SetWork(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if(serial==-1) return;
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if(i!=-1)
    {
        pc->workloc.x=addx[s];
        pc->workloc.y=addy[s];
        pc->workloc.z=addz[s];
    }
}

void cTargets::SetFood(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if(serial==-1) return;
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPC(pc);
    if(i!=-1)
    {
        pc->foodloc.x=addx[s];
        pc->foodloc.y=addy[s];
        pc->foodloc.z=addz[s];
    }
}


void cTargets::MoveToBagTarget(NXWSOCKET s)
{
    P_ITEM pi=pointers::findItemBySerPtr(buffer[s]+7);
	VALIDATEPI(pi);

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    P_ITEM pack= pc->getBackpack();
	VALIDATEPI(pack);

    //setserial(DEREF_P_ITEM(pi),DEREF_P_ITEM(pack),1);
	pi->setContSerial(pack->getSerial32());
	pi->setPosition( 50+rand()%80, 50+rand()%80, 9);
    pi->layer=0x00;
    pi->setDecayTime(0);

    SndRemoveitem( pi->getSerial32() );
    pi->Refresh();
}

void cTargets::MultiTarget(NXWCLIENT ps) // If player clicks on something with the targetting cursor
{
#ifdef PDDAFARE
	if (ps == NULL)
		return;
	NXWSOCKET  s=ps->toInt();
	if (s < 0) return;
	P_CHAR curr=ps->currChar();
	VALIDATEPC(curr);


	P_TARGET target = clientInfo[s]->getTarget();
	if( target==NULL )
		return;

	target->execute( ps );

	return;

	targetok[s]=0;

	PKGx6C tbuf, *pt=&tbuf;
	pt->type=buffer[s][1];
	pt->Tnum=buffer[s][5];
	pt->Tserial=LongFromCharPtr(buffer[s]+7);
	pt->TxLoc=ShortFromCharPtr(buffer[s]+11);
	pt->TyLoc=ShortFromCharPtr(buffer[s]+13);
	pt->TzLoc=buffer[s][16];
	pt->model=ShortFromCharPtr(buffer[s]+17);

	if (pt->TxLoc==-1 && pt->TyLoc==-1) // do nothing if user cancelled
		if (pt->Tserial==0 && pt->model==0) // this seems to be the complete 'cancel'-criteria (Duke)
			return;

//  cClient cli(s), *ps = &cli;

    if ((buffer[s][2]==0)&&(buffer[s][3]==1)&&(buffer[s][4]==0))
    {
        bool Iready=false; //valid item
		bool Cready=false; //valid char
		bool Lready=false; //valid location

        P_ITEM pi = NULL;
        P_CHAR pc = NULL;
		if(buffer[s][1]==0) { //selected object
			if(buffer[s][7]>=0x40) // an item's serial ?
			{
	            pi=pointers::findItemBySerial(pt->Tserial);
				if (ISVALIDPI(pi))
	                Iready=true;
			}
			else
			{
	            pc=pointers::findCharBySerial(pt->Tserial);
				if (ISVALIDPC(pc))
	                Cready=true;
		    }
		}
		else if( buffer[s][1]==1 ) { //selected location
			Lready=true;
		}

        switch(pt->Tnum)
        {
        //case 0: if (Lready) AddTarget(s,pt); break;
        //case 1: /* { cRenameTarget     T(ps);      T.process();} */ break;
        //case 2: if (Lready) TeleTarget(s,pt); break; // LB, bugfix, we need it for the /tele command
        //case 3: { cRemoveTarget     T(ps);      T.process();} break;
        //case 4: DyeTarget(s); break;
        //case 5: { cNewzTarget       T(ps);      T.process();} break;
        //case 6: if (Iready) pi->type=addid1[s]; break; //Typetarget
        case 7: Targ->IDtarget(s); break;
//        case 8: target_xgo XgoTarget(s); break;
        case 9: if (Cready) PrivTarget(s,pc); break;
//        case 10: ItemTarget(ps,pt); break;//MoreTarget
//        case 11: if (Iready) KeyTarget(s,pi); break;
        case 14: target_makegm if (Cready) GMTarget(ps,pc); break;
        case 15: target_makecns if (Cready) CnsTarget(ps,pc); break;
        case 16: target_killhair if (Cready) KillTarget(pc, 0x0b); break;
        case 17: target_killbeard if (Cready) KillTarget(pc, 0x10); break;
//        case 18: if (Cready) KillTarget(pc, 0x15); break;
        case 19: if (Cready) pc->fonttype=addid1[s]; break;
        case 20: target_kill if (Cready) Targ->GhostTarget(s); break;
        case 21: target_resurrect if (Cready) Targ->ResurrectionTarget(s); break; // needed for /resurrect command
        case 22: target_bolt { cBoltTarget      T(ps);  T.process();} break;
        case 23: target_setamount { cSetAmountTarget T(ps);  T.process();} break;
//       case 24:
//            {
//            }
        case 25: target_kick Targ->CloseTarget(s); break;
        case 26: Targ->AddMenuTarget(s, 1, addmitem[s]); break;
        case 27: Targ->NpcMenuTarget(s); break;
//        case 28: ItemTarget(ps,pt); break;//MovableTarget
//        case 29: Skills::ArmsLoreTarget(s); break;
        case 30: if (Cready)
				    OwnerTarget(ps,pc);
				 else if (Iready)
				 	OwnerTarget(ps,pi);
				 break;
//        case 31: ItemTarget(ps,pt); break;//ColorsTarget
        case 32: Targ->DvatTarget(s); break;
        case 33: if (Lready) AddNpcTarget(s,pt); break;
        case 34: target_freeze if (Cready) { pc->priv2|=2; pc->teleport(); } break;
        case 35: target_unfreeze if (Cready) { pc->priv2&=0xfd; pc->teleport(); } break; // unfreeze, AntiChris used LB bugfix
        case 36: Targ->AllSetTarget(s); break;
        case 37: Skills::AnatomyTarget(s); break;
//        case 38: target_recall /*Magic->Recall(s); break;*/
//        case 39: target_mark /*Magic->Mark(s); break;*/
//        case 40: Skills::ItemIdTarget(s); break;
        case 41: Skills::Evaluate_int_Target(s); break;
        case 42: Skills::TameTarget(s); break;
//        case 43: /*Magic->Gate(s); break;*/
        case 44: target_heal	//Luxor
		{
        	P_CHAR pc_toheal = pointers::findCharBySerial(LongFromCharPtr(buffer[s]+7));
        	VALIDATEPC(pc_toheal);
			pc_toheal->hp = pc_toheal->getStrength();
        	pc_toheal->updateStats(STAT_HP);
        }
        break;
//        case 45: Fishing->FishTarget(ps); break;
        case 46: target_tiledata InfoTarget(s,pt); break;
        case 47: /* if (Cready) strcpy(pc->title,xtext[s]); */ break;//TitleTarget
        case 48: break; //XAN : THIS *IS* FREE
//        case 49: Skills::CookOnFire(s,0x097B,"fish steaks"); break;
//        case 50: Skills::Smith(s); break;
//        case 51: Skills::Mine(s); break;
//        case 52: Skills::SmeltOre(s); break;
        case 53: target_npcaction npcact(s); break;
//        case 54: Skills::CookOnFire(s,0x09B7,"bird"); break;
 //       case 55: Skills::CookOnFire(s,0x160A,"lamb"); break;
        case 56: Targ->NpcTarget(s); break;
        case 57: Targ->NpcTarget2(s); break;
        case 58: VALIDATEPC(curr); curr->resurrect(); break;
        case 59: Targ->NpcCircleTarget(s); break;
        case 60: Targ->NpcWanderTarget(s); break;
        case 61: Targ->VisibleTarget(s); break;
        case 62: /* Targ->TweakTarget(s); */ break;
//mo        case 63: target_morex //MoreXTarget
//mo        case 64: target_morey//MoreYTarget
//no        case 65: target_morez //MoreZTarget
//no        case 66: ItemTarget(ps,pt); break;//MoreXYZTarget
        case 67: Targ->NpcRectTarget(s); break;
//        case 68: Skills::CookOnFire(s,0x09F2,"ribs"); break;
//        case 69: Skills::CookOnFire(s,0x1608,"chicken legs"); break;
        case 70: Skills::TasteIDTarget(s); break;
        case 71: if (Iready) ContainerEmptyTarget1(ps,pi); break;
        case 72: if (Iready) ContainerEmptyTarget2(ps,pi); break;
        case 73: CodedNpcRectangle(s,pt); break;
//        case 76: AxeTarget(ps,pt); break;
//        case 77: Skills::DetectHidden(s); break;

//        case 79: Skills::ProvocationTarget1(s); break;
//        case 80: Skills::ProvocationTarget2(s); break;
        case 81: Skills::EnticementTarget1(s); break;
        case 82: Skills::EnticementTarget2(s); break;

//        case 86: Targ->SwordTarget(ps); break;
        case 87: /*Magic->SbOpenContainer(s);*/ break;
        case 88: Targ->SetDirTarget(s); break;
//        case 89: ItemTarget(ps,pt); break;//ObjPrivTarget

        case 100: /*Magic->NewCastSpell( s );*/ break;  // we now have this as our new spell targeting location

        case 105: target_xsbank Targ->xSpecialBankTarget(s); break;//AntiChrist
        case 106: target_setnpcai Targ->NpcAITarget(s); break;
        case 107: target_xbank Targ->xBankTarget(s); break;
//        case 108: Skills::AlchemyTarget(s); break;
//        case 109: Skills::BottleTarget(s); break;
        case 110: Targ->DupeTarget(s); break;
//        case 111: target_movetobag ItemTarget(ps,pt); break;//MovableTarget
        case 112: Targ->SellStuffTarget(s); break;
        case 113: target_mana Targ->ManaTarget(s); break;
        case 114: target_stamina Targ->StaminaTarget(s); break;
        case 115: Targ->GmOpenTarget(s); break;
        case 116: Targ->MakeShopTarget(s); break;
//        case 117: Targ->FollowTarget(s); break;
//        case 118: Targ->AttackTarget(s); break;
//        case 119: Targ->TransferTarget(s); break;
        case 120: Targ->GuardTarget( s ); break;
        case 121: Targ->BuyShopTarget(s); break;
        case 122: ItemTarget(ps,pt); break;//SetValueTarget
//        case 123: ItemTarget(ps,pt); break;//SetRestockTarget
//        case 124: Targ->FetchTarget(s); break;

//        case 126: target_jail Targ->JailTarget(s,-1); break;
        case 127: target_release Targ->ReleaseTarget(s,-1); break;
//        case 129: ItemTarget(ps,pt); break;//SetAmount2Target
//        case 130: Skills::HealingSkillTarget(s); break;
        case 131: target_hide VALIDATEPC(curr); if (curr->IsGM()) Targ->permHideTarget(s); break; /* not used */
        case 132: target_unhide VALIDATEPC(curr); if (curr->IsGM()) Targ->unHideTarget(s); break; /* not used */
        case 133: ItemTarget(ps,pt); break;//SetWipeTarget
//        case 134: Skills::Carpentry(s); break;
        case 135: Targ->SetSpeechTarget(s); break;
//        case 136: Targ->XTeleport(s,0); break;

        case 150: SetSpAttackTarget(s); break;
        case 151: target_fullstats Targ->FullStatsTarget(s); break;
        case 152: Skills::BeggingTarget(s); break;
        case 153: Skills::AnimalLoreTarget(s); break;
        case 154: Skills::ForensicsTarget(s); break;
//        case 155:
//            {
//                curr->poisonserial=LongFromCharPtr(buffer[s]+7);
//                target(s, 0, 1, 0, 156, TRANSLATE("What item do you want to poison?"));
///                return;
//            }
//        case 156: Skills::PoisoningTarget(ps); break;

//        case 160: Skills::Inscribe(s); break;

//        case 162: Skills::LockPick(ps); break;

//        case 164: Skills::Wheel(s, YARN); break;
//        case 165: Skills::Loom(s); break;
//        case 166: Skills::Wheel(s, THREAD); break;
//        case 167: Skills::Tailoring(s); break;

//        case 170: Targ->LoadCannon(s); break;
//        case 171: /*Magic->BuildCannon(s);*/ break;
//        case 172: Skills::Fletching(s); break;
//        case 173: Skills::MakeDough(s); break;
//        case 174: Skills::MakePizza(s); break;
        case 175: Targ->SetPoisonTarget(s); break;
        case 176: Targ->SetPoisonedTarget(s); break;
        case 177: Targ->SetSpaDelayTarget(s); break;
        case 178: Targ->SetAdvObjTarget(s); break;
        case 179: if (Cready) SetInvulFlag(ps,pc); break;
//        case 180: Skills::Tinkering(s); break;
        case 181: Skills::PoisoningTarget(ps); break;

//        case 183: Skills::TinkerAxel(s); break;
//        case 184: Skills::TinkerAwg(s); break;
//        case 185: Skills::TinkerClock(s); break;
        //case 186: Necro::vialtarget(s); break;
        case 187: Skills::RemoveTraps(s); break;
        /* -- BEGIN Custom NoX-Wizard targets */
/*        case 191:
        { //<Luxor>: AMX Target Callback
            TargetLocation TL(pt);
            targetCallback(s, TL);
            break;
        } //</Luxor>*/
//        case 192: PartySystem::targetParty(ps); break;   // Xan Party System
//        case 193: Targ->AllAttackTarget(s); break;      // Luxor All Attack
		case 194:
		{
		     TargetLocation TL(pt);
		     magic::castSpell(curr->spell, TL, curr);
		     break;
        }
        /* -- END Custom NoX-Wizard targets */

//        case 198: Tiling(s,pt); break;
        case 199: /* Targ->Wiping(s); */ break;
//no        case 200: Commands::SetItemTrigger(s); break;
//no        case 201: Commands::SetNPCTrigger(s); break;
//no		case 202: Commands::SetTriggerType(s); break;
//no        case 203: Commands::SetTriggerWord(s); break;
        case 204: triggertarget(s); break; // Fixed by Magius(CHE)
        case 205: Skills::StealingTarget(ps); break;
        case 206: Targ->CanTrainTarget(s); break;
        case 207: ExpPotionTarget(s,pt); break;
        case 209: Targ->SetSplitTarget(s); break;
        case 210: Targ->SetSplitChanceTarget(s); break;
        case 212: target_possess Commands::Possess(s); break;
        case 213: Skills::PickPocketTarget(ps); break;

//        case 220: Guilds->Recruit(s); break;
//        case 221: Guilds->TargetWar(s); break;
        case 222: target_telestuff TeleStuff(s,pt); break;
//        case 223: Targ->SquelchTarg(s); break;//Squelch
//        case 224: Targ->PlVBuy(s); break;//PlayerVendors
//        case 225: Targ->Priv3XTarget(s); break; // SETPRIV3 +/- target
        case 226: /* Targ->ShowPriv3Target(s); */ break; // SHOWPRIV3
//        case 227: Targ->HouseOwnerTarget(s); break; // cj aug11/99
//        case 228: Targ->HouseEjectTarget(s); break; // cj aug11/99
//        case 229: Targ->HouseBanTarget(s); break; // cj aug12/99
//        case 230: Targ->HouseFriendTarget(s); break; // cj aug 12/99
//        case 231: Targ->HouseUnlistTarget(s); break; // cj aug 12/99
//        case 232: Targ->HouseLockdown( s ); break; // Abaddon 17th December 1999
//        case 233: Targ->HouseRelease( s ); break; // Abaddon 17th December 1999
//        case 234: Targ->HouseSecureDown( s ); break; // Ripper
//        case 235: target_ban Targ->BanTarg(s); break;
//        case 236: Skills::RepairTarget(s); break; //Ripper..Repairing item
        //case 237: Skills->SmeltItemTarget(s); break; Ripper..Smelting item
        //taken from 6904t2(5/10/99) - AntiChrist
        case 240: Targ->SetMurderCount( s ); break; // Abaddon 13 Sept 1999

        case 245: buildhouse(s,addid3[s]);   break;

        case 247: target_spy {
			if( Cready && ( pc->getSerial32()!=curr->getSerial32() ) ) {
				NXWCLIENT victim = pc->getClient();
				if( victim!=NULL ) {
					clientInfo[victim->toInt()]->spyTo=curr->getSerial32();
				}
			}
			else {
				for( int s=0; s<now; s++ )
					if( clientInfo[s]->spyTo=curr->getSerial32() )
						clientInfo[s]->spyTo=INVALID;
			}
			}
			break; //showskill target
//        case 248: Targ->MenuPrivTarg(s);break; // menupriv target
        //case 249: Targ->UnglowTaget(s);break; // unglow
//        case 250: if ((Cready)&&(ISVALIDPC(pc))) Priv3Target(s,pc); break; // meta gm target
        case 251: Targ->NewXTarget(s); break; // NEWX
        case 252: Targ->NewYTarget(s); break; // NEWY
        case 253: Targ->IncXTarget(s); break; // INCX
        case 254: Targ->IncYTarget(s); break; // INCY
//        case 255: Targ->IncZTarget(s); break; // INCZ
        // 255 Specially handled for managed commands (Rik)
        case 255: {
            TargetLocation target(pt);
            ps->receiveTarget(target);
            ps->continueCommand();
            break;
        }
        case 256: Targ->SetHome(s); break;
        case 257: Targ->SetWork(s); break;
        case 258: Targ->SetFood(s); break;

        default:
            LogError("Fallout of switch statement, multitarget(), value=(%i)",pt->Tnum);
        }
    }
#endif
}




void target_playerVendorBuy( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc = MAKE_CHAR_REF(t->buffer[0]);
	VALIDATEPC(pc);
    P_CHAR pc_currchar = ps->currChar();
	VALIDATEPC(pc_currchar);

	NXWSOCKET s = ps->toInt();

    P_ITEM pBackpack= pc_currchar->getBackpack();
    if (!pBackpack) {sysmessage(s,TRANSLATE("Time to buy a backpack")); return; } //LB

    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    P_ITEM pi=pointers::findItemBySerial(serial);     // the item
    if (pi==NULL) return;
    if (pi->isInWorld()) return;
    int price=pi->value;


	P_ITEM thepack=(P_ITEM)pi->getContainer();
	VALIDATEPI(thepack);
	P_CHAR pNpc= thepack->getPackOwner();               // the vendor

    if(DEREF_P_CHAR(pNpc)!=pc->getSerial32() || pc->npcaitype!=NPCAI_PLAYERVENDOR) return;

    if (pc_currchar->isOwnerOf(pc))
    {
        pc->talk(s, TRANSLATE("I work for you, you need not buy things from me!"),0);
        return;
    }

    int gleft=pc_currchar->CountGold();
    if (gleft<pi->value)
    {
        pc->talk(s, TRANSLATE("You cannot afford that."),0);
        return;
    }
    pBackpack->DeleteAmount(price,0x0EED);  // take gold from player

    pc->talk(s, TRANSLATE("Thank you."),0);
    pc->holdg+=pi->value; // putting the gold to the vendor's "pocket"

    // sends item to the proud new owner's pack
    pi->setContSerial( pBackpack->getSerial32() );
    pi->Refresh();

}


void target_envoke( NXWCLIENT ps, P_TARGET t )
{
	
	P_CHAR curr=ps->currChar();
	
	SERIAL serial=t->getClicked();
	if( isItemSerial( serial ) )
	{
        P_ITEM pi = MAKE_ITEM_REF(serial);
		VALIDATEPI( pi );
        triggerItem( ps->toInt(),pi, TRIGTYPE_ENVOKED );
        curr->envokeid1=0x00;
        curr->envokeid2=0x00;
    }
	else if( isCharSerial( serial ) )
	{
        P_CHAR pc = MAKE_CHAR_REF(serial);
		VALIDATEPC(pc);
        triggerNpc( ps->toInt(), pc, TRIGTYPE_NPCENVOKED );
        curr->envokeid1=0x00;
        curr->envokeid2=0x00;
    }
	else {
        triggerTile( ps->toInt() );
        curr->envokeid1=0x00;
        curr->envokeid2=0x00;
	}
}


void target_key( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	P_ITEM pi = pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	NXWSOCKET s = ps->toInt();

    if ((pi->more1==0)&&(pi->more2==0)&&
            (pi->more3==0)&&(pi->more4==0))
        {
            if ( pi->type==ITYPE_KEY && (item_inRange(MAKE_CHAR_REF(currchar[s]),pi,2) || (!pi->isInWorld()) ) )
            {
                if (!pc->checkSkill(TINKERING, 400, 1000))
                {
                    sysmessage(s,TRANSLATE("You fail and destroy the key blank."));
                    // soundeffect3( pi, <whatever> );
                    pi->Delete();
                }
                else
                {
                    pi->more1=t->buffer[0];
                    pi->more2=t->buffer[1];
                    pi->more3=t->buffer[2];
                    pi->more4=t->buffer[3];
                    // soundeffect3( pi, <whatever> );
                    sysmessage(s, TRANSLATE("You copy the key.")); //Morrolan can copy keys
                }
            }
            return;
        }//if
        else if (((pi->more1==t->buffer[0])&&(pi->more2==t->buffer[1])&&
            (pi->more3==t->buffer[2])&&(pi->more4==t->buffer[3]))||
            (t->buffer[0]==(unsigned char) 0xFF))
        {
            if (((pi->type==ITYPE_CONTAINER)||(pi->type==ITYPE_UNLOCKED_CONTAINER))&&(item_inRange(pc,pi,2)))
            {
                if(pi->type==ITYPE_CONTAINER) pi->type=ITYPE_LOCKED_ITEM_SPAWNER;
                if(pi->type==ITYPE_UNLOCKED_CONTAINER) pi->type=ITYPE_LOCKED_CONTAINER;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You lock the container."));
                return;
            }
            else if ((pi->type==ITYPE_KEY)&&(item_inRange(pc,pi,2)))
            {
                pc->keyserial=pi->getSerial32();
                sysmessage(s,TRANSLATE("Enter new name for key."));//morrolan rename keys
                return;
            }
            else if ((pi->type==ITYPE_LOCKED_ITEM_SPAWNER)||(pi->type==ITYPE_LOCKED_CONTAINER)&&(item_inRange(pc,pi,2)))
            {
                if(pi->type==ITYPE_LOCKED_ITEM_SPAWNER) pi->type=ITYPE_CONTAINER;
                if(pi->type==ITYPE_LOCKED_CONTAINER) pi->type=ITYPE_UNLOCKED_CONTAINER;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You unlock the container."));
                return;
            }
            else if ((pi->type==ITYPE_DOOR)&&(item_inRange(pc,pi,2)))
            {
                pi->type=ITYPE_LOCKED_DOOR;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You lock the door."));
                return;
            }
            else if ((pi->type==ITYPE_LOCKED_DOOR)&&(item_inRange(pc,pi,2)))
            {
                pi->type=ITYPE_DOOR;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You unlock the door."));
                return;
            }
            else if (pi->id()==0x0BD2)
            {
                sysmessage(s, TRANSLATE("What do you wish the sign to say?"));
                pc->keyserial=pi->getSerial32(); //Morrolan sign kludge
                return;
            }

            //Boats ->
            else if(pi->type==ITYPE_BOATS && pi->type2==3)
            {
                Boats->OpenPlank(pi);
                pi->Refresh();
            }
            //End Boats --^
        }//else if
        else
        {
            if (pi->type==ITYPE_KEY) sysmessage (s, TRANSLATE("That key is not blank!"));
            else if (pi->more1==0x00) sysmessage(s, TRANSLATE("That does not have a lock."));
            else sysmessage(s, TRANSLATE("The key does not fit into that lock."));
            return;
        }//else
}

void target_attack( NXWCLIENT ps, P_TARGET t ) 
{

    P_CHAR pc_t1= pointers::findCharBySerial( t->buffer[0] );
    VALIDATEPC(pc_t1);
	P_CHAR pc_t2=pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc_t2);

	NXWSOCKET s = ps->toInt();

    AttackStuff(s,pc_t2); //xan : flag them all!
    npcattacktarget(pc_t1, pc_t2);
}

void target_follow( NXWCLIENT ps, P_TARGET t ) 
{

    P_CHAR pc = pointers::findCharBySerial( t->buffer[0] );
    VALIDATEPC(pc);

	P_CHAR pc2 = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc2);

    pc->ftargserial=pc2->getSerial32();
    pc->npcWander=WANDER_FOLLOW;
}

void target_axe( NXWCLIENT ps, P_TARGET t )
{
    NXWSOCKET  s = ps->toInt();

    UI16 id=t->getModel();
    if (itemById::IsTree(id))
		Skills::TreeTarget(s);
    else if (itemById::IsCorpse(id))
        CorpseTarget(ps);
    else if (itemById::IsLog(id)) //Luxor bug fix
		Skills::BowCraft(s);
}


void target_sword( NXWCLIENT ps, P_TARGET t )
{
	NXWSOCKET  s = ps->toInt();
    P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);

	UI16 id = t->getModel();
	if (itemById::IsTree2(id))
	{
		Location pcpos= pc->getPosition();
		Location location = t->getLocation();

		if( dist( location, pcpos )>5 )
		{
			pc->sysmsg(TRANSLATE("You are to far away to reach that"));
			return;
		}

		pc->playAction( pc->isMounting() ? 0x0D : 0x01D );
		pc->playSFX(0x013E);

		P_ITEM pi=item::CreateFromScript( "$item_kindling" );
		VALIDATEPI(pi);

		pi->setPosition( pcpos );
		mapRegions->add(pi);

		pi->Refresh();
		pc->sysmsg(TRANSLATE("You hack at the tree and produce some kindling."));
	}
	else if(itemById::IsLog(id)) // vagrant
	{
		Skills::BowCraft(s);
	} 
	else if(itemById::IsCorpse(id))
	{
		CorpseTarget(ps);
	} 
	else
		pc->sysmsg(TRANSLATE("You can't think of a way to use your blade on that."));
}

void target_fetch( NXWCLIENT ps, P_TARGET t )
{
    ps->sysmsg( TRANSLATE("Fetch is not available at this time.") );
}

void target_guard( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	P_CHAR pPet = pointers::findCharBySerial(t->buffer[0]);
    VALIDATEPC(pPet);

    P_CHAR pToGuard = pointers::findCharBySerial( t->getClicked() );
    if( !ISVALIDPC(pToGuard) || pToGuard->getSerial32() != pPet->getOwnerSerial32() )
    {
        ps->sysmsg( TRANSLATE("Currently can't guard anyone but yourself!" ));
        return;
    }
    pPet->npcaitype = NPCAI_PETGUARD;
    pPet->ftargserial=pc->getSerial32();
    pPet->npcWander=WANDER_FOLLOW;
    ps->sysmsg( TRANSLATE("Your pet is now guarding you."));
    pc->guarded = true;
}

void target_transfer( NXWCLIENT ps, P_TARGET t )
{

    P_CHAR pc1 = pointers::findCharBySerial( t->buffer[0] );
	VALIDATEPC(pc1);
    P_CHAR pc2 = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc2);

	//Araknesh Call OnTransfer Event Passing Animal,NewOwner

	if (pc1->amxevents[EVENT_CHR_ONTRANSFER])
	{
		g_bByPass = false;
		pc1->amxevents[EVENT_CHR_ONTRANSFER]->Call(pc1->getSerial32(),pc2->getSerial32());
		if (g_bByPass==true) return ;
	}
	/*
	pc1->runAmxEvent( EVENT_CHR_ONTRANSFER, pc1->getSerial32(),pc2->getSerial32());
	if (g_bByPass==true)
		return ;
	*/
    char bb[120];
    sprintf(bb,TRANSLATE("* %s will now take %s as his master *"), pc1->getCurrentNameC(), pc2->getCurrentNameC());
    pc1->talkAll(bb,0);

    pc1->setOwner( pc2 );
    pc1->npcWander=WANDER_FOLLOW;
    pc1->ftargserial=INVALID;
    pc1->npcWander=WANDER_NOMOVE;
}

 //Throws the potion and places it (unmovable) at that spot
void target_expPotion( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

    Location loc=t->getLocation();

	NXWSOCKET s=ps->toInt();

    if(line_of_sight(s, pc->getPosition(), loc, WALLS_CHIMNEYS + DOORS + ROOFING_SLANTED))
    {
        P_ITEM pi=pointers::findItemBySerial( t->buffer[0] );
        if (ISVALIDPI(pi)) // crashfix LB
        {
            pi->MoveTo( loc );
            pi->setContSerial(INVALID);
            pi->magic=2; //make item unmovable once thrown
            movingeffect2(DEREF_P_CHAR(pc), DEREF_P_ITEM(pi), 0x0F, 0x0D, 0x11, 0x00, 0x00);
            pi->Refresh();
        }
    }
    else 
		pc->sysmsg(TRANSLATE("You cannot throw the potion there!"));
}


void target_trigger( NXWCLIENT ps, P_TARGET t )
{

	P_ITEM pi = MAKE_ITEM_REF(t->getClicked());
	VALIDATEPI(pi);
    
	triggerItem(ps->toInt(), pi, TRIGTYPE_TARGET);

}

void target_npcMenu( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

    P_CHAR npc = npcs::AddRespawnNPC(pc,t->buffer[0]);

}

/*!
\brief implements the 'telestuff GM command
\author Endymion
*/
void target_telestuff( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);

	NXWSOCKET s = ps->toInt();

	P_OBJECT po = objects.findObject( t->getClicked() );

	if( ISVALIDPO(po) ) { //clicked on obj to move
		P_TARGET targ=clientInfo[s]->newTarget( new cLocationTarget() );
		targ->code_callback=target_telestuff;
		targ->buffer[0]=po->getSerial32();
		targ->send(ps);
		ps->sysmsg( TRANSLATE("Select location to put this object.") );
	} else { //on ground.. so move it
		
		Location loc=t->getLocation();
		loc.z+=tileHeight( t->getModel() );

		SERIAL serial = t->buffer[0];
		if( isCharSerial(serial) ) {
			P_CHAR pt = pointers::findCharBySerial( serial );
			VALIDATEPC(pt);

			pt->MoveTo( loc );
			pt->teleport();
		} else if ( isItemSerial(serial) ) {
			P_ITEM pi = pointers::findItemBySerial( serial );
			VALIDATEPI(pi);

			pi->MoveTo(loc);
			pi->Refresh();
		}
	}
}


