  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "itemid.h"
#include "sndpkg.h"
#include "amx/amxcback.h"
#include "targeting.h"
#include "tmpeff.h"
#include "constants.h"
#include "npcai.h"
#include "debug.h"
#include "srvparms.h"
#include "addmenu.h"
#include "weight.h"
#include "set.h"
#include "archive.h"
#include "map.h"
#include "items.h"
#include "chars.h"
#include "skills.h"
#include "basics.h"
#include "inlines.h"
#include "range.h"
#include "nox-wizard.h"


//Luxor: for AMX skills implementation
#undef AMXINGOTMAKEMENU
#define AMXINGOTMAKEMENU "__nxw_ingot_mm"
#undef AMXSMELTORE
#define AMXSMELTORE "__nxw_smeltOre"
#undef AMXLUMBERJACKING
#define AMXLUMBERJACKING "__nxw_sk_lumber"
#undef AMXTAILORING
#define AMXTAILORING "__nxw_sk_tailoring"
#undef AMXTANNERING
#define AMXTANNERING "__nxw_sk_tannering"


P_ITEM Check4Pack(NXWSOCKET  s)
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
    P_ITEM packnum= pc->getBackpack();
    if (packnum==NULL)
    {
        sysmessage(s,TRANSLATE("Time to buy a backpack"));
        return NULL;
    }
    else
        return packnum;
}

bool CheckInPack(NXWSOCKET  s, P_ITEM pi)
{
    P_ITEM pPack=Check4Pack(s);
    VALIDATEPIR(pPack, false);
    if (pi->getContSerial()!=pPack->getSerial32())
    {
        sysmessage(s,TRANSLATE("You can't use material outside your backpack"));
        return false;
    }
    return true;
}


void Skills::target_removeTraps( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);
	P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);
	NXWSOCKET s = ps->toInt();

	if (pi->amxevents[EVENT_IONREMOVETRAP]==NULL)
	{
		sysmessage(s, TRANSLATE("There are no traps on this object"));
		if ((rand()%3)==0)
			pc->checkSkill( REMOVETRAPS, 0, 750); //ndEny is good?
	}
	else
		pi->amxevents[EVENT_IONREMOVETRAP]->Call(pi->getSerial32(), pc->getSerial32() );

}


AmxFunction* tailoring = NULL;
AmxFunction* tannering = NULL;


void Skills::target_tailoring( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	NXWSOCKET s = ps->toInt();

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,TAILORING,AMX_BEFORE);

    if( pi->magic==4)
		return;

	if( pi->IsCutLeather() || pi->IsCutCloth() || pi->IsHide() )
    {
		if (CheckInPack(s,pi))
        {
			if(pc->getAmount(pi->getId())<1)
			{
				pc->sysmsg(TRANSLATE("You don't have enough material to make anything."));
				return;
			}

            if(	pi->IsCutLeather()  ) {
				if( tannering == NULL )
					tannering = new AmxFunction( AMXTANNERING );
				if( tannering != NULL )
					tannering->Call( pc->getSerial32(), pi->getSerial32() );
			}
            else {
				if( tailoring == NULL )
					tailoring = new AmxFunction( AMXTAILORING );
				if( tailoring != NULL )
					tailoring->Call( pc->getSerial32(), pi->getSerial32() );
			}

        }

    }
	else pc->sysmsg(TRANSLATE("You cannot use that material for tailoring."));

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,TAILORING,AMX_AFTER);
}

void Skills::target_fletching( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	NXWSOCKET s = ps->toInt();

	P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,BOWCRAFT,AMX_BEFORE);
    if ( pi->magic!=4) // Ripper
    {
        if (CheckInPack(s,pi))
        {
            MakeMenu(pc,60,BOWCRAFT, pointers::findItemBySerial( t->buffer[0] ), pi );
        }
    }
	else
		pc->sysmsg(TRANSLATE("You cannot use that for fletching."));

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,BOWCRAFT,AMX_AFTER);
}

void Skills::target_bowcraft( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc_currchar = ps->currChar();
	VALIDATEPC(pc_currchar);

	NXWSOCKET s = ps->toInt();

	pc_currchar->playAction(pc_currchar->isMounting() ? 0x1C : 0x0D);

	const P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	AMXEXECSVTARGET(pc_currchar->getSerial32(),AMXT_SKITARGS,BOWCRAFT,AMX_BEFORE);

	pc_currchar->playAction(pc_currchar->isMounting() ? 0x1C : 0x0D);
	if ( pi->magic!=4) // Ripper
	{

		if( pi->IsLog() || pi->IsBoard() )
		{
			if (CheckInPack(s,pi))
			{
				MakeMenu(pc_currchar,65,BOWCRAFT,pi);
			}
		}
	}

	AMXEXECSVTARGET( pc_currchar->getSerial32(),AMXT_SKITARGS,BOWCRAFT,AMX_AFTER);
}

////////////////////
// name:    target_carpentry()
// history: unknown, rewritten by Duke, 25.05.2000
// purpose: sets up appropriate Makemenu when player targets logs or boards
//          after dclick on carpentry tool
//
//          If logs are targetted, Makemenu 19 is called to produce boards
//          If boards, MM 20 is called for furniture etc.
//
void Skills::target_carpentry( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,CARPENTRY,AMX_BEFORE);
    if ( pi->magic!=4)
    {
        if( pi->IsLog() || pi->IsBoard() ) // logs or boards
        {
           if (CheckInPack(ps->toInt(),pi))
           {
              short mm = pi->IsLog() ? 19 : 20; // 19 = Makemenu to create boards from logs
              MakeMenu(pc,mm,CARPENTRY,pi);
           }
        }
    }
    else
        pc->sysmsg(TRANSLATE("You cannot use that material for carpentry."));

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,CARPENTRY,AMX_AFTER);
}

/*!
\todo use or remove it!
*/
static bool ForgeInRange(NXWSOCKET s)
{
    P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pc, false);

	NxwItemWrapper si;
	si.fillItemsNearXYZ( pc->getPosition(), 3, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {
        P_ITEM pi=si.getItem();
		if( ISVALIDPI(pi) && pi->IsForge())
			return true;
    }
    return false;
}

static bool AnvilInRange(NXWSOCKET s)
{
    P_CHAR pc = MAKE_CHAR_REF( currchar[s]);
	VALIDATEPCR(pc, false);

	NxwItemWrapper si;
	si.fillItemsNearXYZ( pc->getPosition(), 3, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {
        P_ITEM pi=si.getItem();
        if(pi->IsAnvil())
			return true;
    }
    return false;
}

/*!
\author Duke
\date 28/03/2000
\brief Little helper function for cSkills::target_smith()
\param s socket number
\param pi pointer to material item
\param ma maximum amount
\param mm makemenu number to invoke from create.scp
\param matname name of the metal

checks for anvil in reach and enough material and invokes appropriate makemenu
*/
static void AnvilTarget( NXWSOCKET s, P_ITEM pi, int ma, int mm, char* matname)
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	VALIDATEPI(pi);

    if (!AnvilInRange(s))
        sysmessage(s,TRANSLATE("The anvil is too far away."));
    else
    {
        if (CheckInPack(s,pi))
        {
			Skills::MakeMenu(pc,mm,BLACKSMITHING,pi);
		}

    }
}

//////////////////////////
// Function:    Smith
// History:     unknown, colored ore added by Cork,
//              28 March 2000 revamped by Duke
//              16.9.2000 removed array access and revamped s a bit more (Duke)
//
// Purpose:     checks if targeted material is ore of some kind,
//              checks if anvil is in reach and invokes appropriate Makemenu
// Remarks:     the ingottype var is problematic in a multiplayer environment!!
//
extern int ingottype;

void Skills::target_smith( NXWCLIENT ps, P_TARGET t )
{
    P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

    if (pi->magic!=4) // Ripper
    {
        if (!CheckInPack(ps->toInt(),pi))
			return;

        if (pi->getId()==0x1BEF || pi->getId()==0x1BF2)   // is it an ingot ?
        {
			AnvilTarget( ps->toInt(), pi, 1, AmxFunction::g_prgOverride->CallFn( AmxFunction::g_prgOverride->getFnOrdinal(AMXINGOTMAKEMENU), pi->getColor()), NULL);
			return;
        }
    }
    ps->sysmsg( TRANSLATE("You cannot use that material for blacksmithing") );
}

struct Ore
{
    short color;
    short minskill; // minimum skill to handle that ore
    short quota;    // relative(!) chance to mine that ore (quota/sum of all quotas)
    char *name;
};
const struct Ore OreTable[] =   // MUST be sorted by minskill
{
//  {0x0000,  0,630,"Iron"},
    {0x0386,650,126,"Shadow"},
    {0x02C3,700,112,"Merkite"},
    {0x046E,750, 98,"Copper"},
    {0x0961,790, 84,"Silver"},
    {0x02E7,800, 70,"Bronze"},
    {0x0466,850, 56,"Golden"},
    {0x0150,900, 42,"Agapite"},
    {0x022F,950, 28,"Verite"},
    {0x0191,990, 14,"Mythril"}
};
const short NumberOfOres = sizeof(OreTable)/sizeof(Ore);

/*static const Ore* getColorFound(short skill)
{
    int i,totalQuotas=0,minersQuota=0;

    for (i=0;i<NumberOfOres;i++)
    {
        if(OreTable[i].minskill<=skill)     // get his part of quotas depending on his skill
            minersQuota += OreTable[i].quota;
        else
            break;
    }
    if (minersQuota < 1) return NULL;
    int r = rand()%minersQuota;

    for (i=0;i<NumberOfOres;i++)
    {
        totalQuotas += OreTable[i].quota;   // summarize quotas until we find the ore
        if (r < totalQuotas)
            return &OreTable[i];
        if(OreTable[i].minskill > skill)
        {
            ErrOut("something went wrong with mining");
            break;
        }
    }
    return &OreTable[0];    // shouldn't get here, but return iron as default anyway
}*/

/////////////////
// name:    TryToMine
// history: by Duke, 31 March 2000
// Purpose: helper function for cSkills::Mine()
//          checks if the player's mining skill meets the requirements
//          if yes, he'll get the specified ore
//          There's a *second* CheckSkill done here. It was like this,
//          so I left it like this. That's a gameplay issue.
//
/*static bool TryToMine(    int s,                  // current char's socket #
                        int minskill,           // minimum skill required for ore color
                        unsigned char id1, unsigned char id2,       // item ID of ingot to be created
                        unsigned char col1,unsigned char col2,  // color
                        char *orename)          // first letter should be uppercase
{
    CHARACTER cc = currchar[s];
    P_CHAR pc = MAKE_CHARREF_LRV(cc, false);

    if(pc->skill[MINING] >= minskill)
    {
        char tmp[100];
        sprintf(tmp,"%s Ore",orename);
        item::SpawnItem(s,cc,1,tmp,1,id1,id2,col1,col2,1,1);

        sysmessage(s,"You place some %c%s ore in your pack.",tolower(*orename),orename+1);
        return true;
    }
    return false;
}
*/ // XAN : SEEMS UNUSED

#define max_res_x 610
#define max_res_y 410

void Skills::target_tree( NXWCLIENT ps, P_TARGET t )
{

    P_CHAR pc = ps->currChar();

	NXWSOCKET s = ps->toInt();

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,LUMBERJACKING,AMX_BEFORE);

    static TIMERVAL logtime[max_res_x][max_res_y];//see mine for values...they were 1000 also here
    static UI32 logamount[max_res_x][max_res_y];
    int a, b;
    unsigned int c;
    unsigned long int curtime=uiCurrentTime;

	if( pc->isMounting() ) {
		pc->sysmsg(TRANSLATE("You cannot do this on a horse"));
		return;
	}

	Location charpos= pc->getPosition();
	Location location = t->getLocation();


    if( dist( charpos, location )>2 )
    {
        pc->sysmsg(TRANSLATE("You are to far away to reach that"));
        return;
    }

	P_ITEM weapon = pc->getWeapon();
	if( !( ISVALIDPI(weapon) && ( weapon->IsAxe() || weapon->IsSword() ) ) )
	{
		pc->sysmsg(TRANSLATE("You must have a weapon in hand in order to chop."));
		return;
	}

    if (resource.logstamina<0 && abs(resource.logstamina)>pc->stm)
    {
        pc->sysmsg(TRANSLATE("You are too tired to chop."));
        return;
    }

    pc->stm+=resource.logstamina;
    if(pc->stm<0) pc->stm=0;
    if(pc->stm>pc->dx) pc->stm=pc->dx;
    pc->updateStats(2);

    if(resource.logarea<10) resource.logarea=10; //New -- Zippy

    if(logtime[0][0]==0)//First time done since server started
    {
        logtime[0][0]=17;//lucky number ;-)
        logamount[0][0]=resource.logs;
        LogMessage(TRANSLATE("Lumberjacking startup, setting tree values and times..."));

        //for(a=1;a<410;a++)
        for(a=1;a<max_res_x;a++)//AntiChrist bug fix
        {
            for(b=1;b<max_res_y;b++)
            {
                logamount[a][b]=resource.logs;
                SetTimerSec(&logtime[a][b],static_cast<short>(resource.logtime));
            }
        }
        LogMessage("[DONE]");
    }

	pc->facexy( location.x, location.y );

    a= charpos.x / resource.logarea; //Zippy
    b= charpos.y / resource.logarea;

    if(a>=max_res_x || b>=max_res_y) return;

    if(logtime[a][b]<=curtime)
    {
        for(c=0;c<resource.logs;c++)//Find howmany 10 min periods have been by, give 1 more for each period.
        {
            if((logtime[a][b]+(c*resource.logtime*MY_CLOCKS_PER_SEC))<=curtime && logamount[a][b]<resource.logs)
                logamount[a][b]+=resource.lograte;//AntiChrist
            else break;
        }
        SetTimerSec(&logtime[a][b],static_cast<short>(resource.logtime));
    }

    if(logamount[a][b]>resource.logs) logamount[a][b]=resource.logs;

    if(logamount[a][b]<=0)
    {
        pc->sysmsg(TRANSLATE("There is no more wood here to chop."));
        return;
    }

    P_ITEM packnum = pc->getBackpack();
    if( !ISVALIDPI(packnum) ) {
    	pc->sysmsg(TRANSLATE("No backpack to store logs"));
		return;
	}

    pc->playAction( pc->isMounting() ? 0x1C : 0x0D );
    pc->playSFX(0x013E);

    if (!pc->checkSkill(LUMBERJACKING, 0, 1000))
    {
        pc->sysmsg(TRANSLATE("You chop for a while, but fail to produce any usable wood."));
        if(logamount[a][b]>0 && rand()%2==1)
			logamount[a][b]--;//Randomly deplete resources even when they fail 1/2 chance you'll loose wood.
        return;
    }

    if(logamount[a][b]>0)
		logamount[a][b]--;

    AmxFunction::g_prgOverride->CallFn( AmxFunction::g_prgOverride->getFnOrdinal(AMXLUMBERJACKING), s);

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,LUMBERJACKING,AMX_AFTER);
}

void Skills::GraveDig(NXWSOCKET s) // added by Genesis 11-4-98
{
    P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    int nAmount, nFame;
    char iID=0;

    pc->IncreaseKarma(-2000); // Karma loss no lower than the -2 pier

    pc->playAction( pc->isMounting() ? 0x1A : 0x0b );
    pc->playSFX(0x0125);
    if(!pc->checkSkill(MINING, 0, 800))
    {
        sysmessage(s,TRANSLATE("You sifted through the dirt and found nothing."));
        return;
    }

    nFame = pc->GetFame();
    pc->playAction( pc->isMounting() ? 0x1A : 0x0b );
    pc->playSFX(0x0125);
    int nRandnum=rand()%13;
    switch(nRandnum)
    {
    case 2:
        npcs::SpawnRandomMonster(pc,"UNDEADLIST","1000"); // Low level Undead - Random
        pc->sysmsg(TRANSLATE("You have disturbed the rest of a vile undead creature."));
        break;
    case 4:
		{
			P_ITEM pi=item::SpawnRandomItem(s,"ITEMLIST","1001"); // Armor and shields - Random
			if(ISVALIDPI(pi))
				if((pi->getId()>=7026)&&(pi->getId()<=7035))
					pc->sysmsg(TRANSLATE("You unearthed an old shield and placed it in your pack"));
				else
		            pc->sysmsg(TRANSLATE("You have found an old piece armor and placed it in your pack."));
		}
        break;
    case 5:
        //Random treasure between gems and gold
        nRandnum=rand()%2;
        if(nRandnum)
        { // randomly create a gem and place in backpack
            P_ITEM pi=item::SpawnRandomItem(s,"ITEMLIST","999");
            if(ISVALIDPI(pi))
				pc->sysmsg(TRANSLATE("You place a gem in your pack."));
        }
        else
        { // Create between 1 and 15 goldpieces and place directly in backpack
            nAmount=1+(rand()%15);
            addgold(DEREF_P_CHAR(pc),nAmount);
            pc->playSFX( goldsfx(nAmount) );
            if (nAmount==1)
                pc->sysmsg(TRANSLATE("You unearthed %i gold coin."), nAmount);
            else
                pc->sysmsg(TRANSLATE("You unearthed %i gold coins."), nAmount);
        }
        break;
    case 6:
        if(nFame<500)
            npcs::SpawnRandomMonster(pc,"UNDEADLIST","1000"); // Low level Undead - Random
        else
            npcs::SpawnRandomMonster(pc,"UNDEADLIST","1001"); // Med level Undead - Random
        pc->sysmsg(TRANSLATE("You have disturbed the rest of a vile undead creature."));
        break;
    case 8:
		{
			P_ITEM pi=item::SpawnRandomItem(s,"ITEMLIST","1000");
			if(ISVALIDPI(pi))
				pc->sysmsg(TRANSLATE("You unearthed a old weapon and placed it in your pack."));
		}
        break;
    case 10:
        if(nFame<1000)
            npcs::SpawnRandomMonster(pc,"UNDEADLIST","1001"); // Med level Undead - Random
        else
            npcs::SpawnRandomMonster(pc,"UNDEADLIST","1002"); // High level Undead - Random
        pc->sysmsg(TRANSLATE("You have disturbed the rest of a vile undead creature."));
        break;
    case 12:
        if(nFame>1000)
            npcs::SpawnRandomMonster(pc,"UNDEADLIST","1002"); // High level Undead - Random
        else
            npcs::SpawnRandomMonster(pc,"UNDEADLIST","1001"); // Med level Undead - Random
        pc->sysmsg(TRANSLATE("You have disturbed the rest of a vile undead creature."));
        break;
    default:
        nRandnum=rand()%2;
        P_ITEM pBone = NULL;
        switch(nRandnum)
        {
            case 1:
                nRandnum=rand()%12;
                switch(nRandnum)
                {
                    case 0: iID=0x11; break;
                    case 1: iID=0x12; break;
                    case 2: iID=0x13; break;
                    case 3: iID=0x14; break;
                    case 4: iID=0x15; break;
                    case 5: iID=0x16; break;
                    case 6: iID=0x17; break;
                    case 7: iID=0x18; break;
                    case 8: iID=0x19; break;
                    case 9: iID=0x1A; break;
                    case 10: iID=0x1B; break;
                    case 11: iID=0x1C; break;
                }
                pBone = item::CreateFromScript( "$item_bone", pc->getBackpack() );
                pBone->setId( 0x1B00 + iID );
                pc->sysmsg(TRANSLATE("You have unearthed some old bones and placed them in your pack."));
                break;
            default: // found an empty grave
               	pc->sysmsg(TRANSLATE("This grave seems to be empty."));
        }
    }
}



//////////////////////////
// Function:    SmeltOre
// History:     unknown, colored ore by Cork,
//              31 March 2000 totally revamped by Duke
//              16.9.2000 removed array access and revamped a bit more (Duke)
//
// Purpose:     checks if targeted item is a forge and in range
//              then executes the smelting function with the appropriate
//              minskill and ingot type
// Remarks:     NOTE: ingot color is different from ore color for gold, silver & copper!
//
void Skills::target_smeltOre( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);
    P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

    if ( pi->magic!=4) // Ripper
    {
        if( pi->IsForge() )
        {
            if(!item_inRange(pc,pi,3))        //Check if the forge is in range
                pc->sysmsg(TRANSLATE("You cant smelt here."));
            else
            {
                P_ITEM pix=pointers::findItemBySerial( t->buffer[0] );
				VALIDATEPI( pix );

                AmxFunction::g_prgOverride->CallFn( AmxFunction::g_prgOverride->getFnOrdinal(AMXSMELTORE), pc->getSerial32(), pix->getColor(), pix->getSerial32());
            }
        }
    }

	weights::NewCalc(pc);   // Ison 2-20-99
    statwindow(pc,pc);      // Ison 2-20-99
}

void Skills::target_wheel( NXWCLIENT ps, P_TARGET t )	//Spinning wheel
{
    P_CHAR pc_currchar = ps->currChar();
	VALIDATEPC(pc_currchar);
    P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
    VALIDATEPI(pi);

	int mat = t->buffer[0];

    int tailme=0;


    if( pi->getId() >= 0x10A4 || pi->getId() <= 0x10A6 )
    {
        if(item_inRange(pc_currchar,pi,3))
        {
            if (!pc_currchar->checkSkill(TAILORING, 0, 1000))
            {
                pc_currchar->sysmsg(TRANSLATE("You failed to spin your material."));
                return;
            }

            P_ITEM pti=pointers::findItemBySerial( t->buffer[1] );   // on error return
			VALIDATEPI(pti);

            pc_currchar->sysmsg(TRANSLATE("You have successfully spun your material."));

            if (mat==YARN)
            {
                pti->setCurrentName("#");
                pti->setId( 0x0E1D );
                pti->amount=pti->amount*3;
            }
            else if (mat==THREAD)
            {
                pti->setCurrentName("#");
                pti->setId( 0x0FA0 );
                pti->amount=pti->amount*3;
            }

            pti->priv |= ITMPRIV_DECAY;
            pti->Refresh();
            tailme=1;
        }
    }

    if(!tailme)
		pc_currchar->sysmsg(TRANSLATE("You cant tailor here."));
}

void Skills::target_loom( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc_currchar = ps->currChar();
	VALIDATEPC(pc_currchar);
	P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	int tailme=0;

	if (pi->magic!=4) // Ripper
	{
		if ( pi->getId() >= 0x105F && pi->getId() <= 0x1066 )
		{
			if(item_inRange(pc_currchar,pi,3))
			{
				P_ITEM pti=pointers::findItemBySerial( t->buffer[0] );
				VALIDATEPI(pti);

				if(pti->amount<5)
				{
					pc_currchar->sysmsg(TRANSLATE("You do not have enough material to make anything!"));
					return;
				}

				if (!pc_currchar->checkSkill(TAILORING, 300, 1000))
				{
					pc_currchar->sysmsg(TRANSLATE("You failed to make cloth."));
					pc_currchar->sysmsg(TRANSLATE("You have broken and lost some material!"));
					pti->ReduceAmount( 1+(rand() % (pti->amount)));
					pti->Refresh();
					return;
				}

				switch( pti->getId() )
				{
					case 0x0E1E: // Yarn
					case 0x0E1D:
					case 0x0E1F:
						pc_currchar->sysmsg(TRANSLATE("You have made your cloth."));
						pti->setCurrentName("#");
						pti->setId( 0x175D );
						pti->priv |= ITMPRIV_DECAY;
						pti->amount=(pti->amount-1)*10;
						break;
					case 0x0FA0: // Thread
					case 0x0FA1:
						pc_currchar->sysmsg(TRANSLATE("You have made a bolt of cloth."));
						pti->setCurrentName("#");
						pti->setId( 0x0F95 );
						pti->priv |= ITMPRIV_DECAY;
						pti->amount=(UI16)(pti->amount*0.25);
						break;
				}

				pti->Refresh();

				tailme=1;
			}
		}
	}

	if(!tailme)
		pc_currchar->sysmsg(TRANSLATE("You can't tailor here."));
}

////////////
// Name:    CookOnFire
// By:      Ripper & Duke, 07/20/00
// Purpose: so you can use raw meat on fire
//
void Skills::target_cookOnFire( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	short id = t->buffer[0];
	std::string matname = t->buffer_str[0];

    if (pi->magic!=4) // Ripper
    {
        P_ITEM piRaw=MAKE_ITEM_REF( t->buffer[1] );
		VALIDATEPI(piRaw);
        if (pc->isInBackpack(piRaw))
        {
            if(pi->IsCookingPlace() )
            {
                if(item_inRange(pc,pi,3))
                {
                    pc->playSFX(0x01DD);   // cooking sound
                    if (!pc->checkSkill(COOKING, 0, 1000))
                    {
                        piRaw->ReduceAmount(1+(rand() %(piRaw->amount)));
                        pc->sysmsg(TRANSLATE("You failed to cook the %s and drop some into the ashes."),matname.c_str());
                    }
                    else
                    {
                        P_ITEM pi=item::CreateFromScript( "$item_raw_fish", pc->getBackpack(), piRaw->amount );
                        VALIDATEPI(pi);

                        pi->setCurrentName( "#" );
                        pi->setId( id );
                        pi->type=ITYPE_FOOD;
                        pi->Refresh();
                        piRaw->Delete();
                        pc->sysmsg(TRANSLATE("You have cooked the %s,and it smells great."),matname.c_str());
                    }
                }
            }
        }
    }
}


/*
* I decided to base this on how OSI will be changing detect hidden.
* The greater your skill, the more of a range you can detect from target position.
* Hiders near the center of the detect circle are easier to detect than ones near
* the edges of the detect circle. Also low skill detecters cannot find high skilled
* hiders as easily as low skilled hiders.
*/

#include "set.h"

/*!
\author Luxor
*/
void Skills::target_detectHidden( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);

	AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,DETECTINGHIDDEN,AMX_BEFORE);

	Location location = t->getLocation();

	SI32 nSkill = pc->skill[DETECTINGHIDDEN];
	SI32 nRange = SI32( VISRANGE * nSkill/2000.0 );
	SI32 nLow = 0;
	Location lCharPos = pc->getPosition();

	if ( SI32(dist(lCharPos, location)) > 15 ) {
		pc->sysmsg( TRANSLATE("You cannot see for hidden objects so far.") );
		return;
	}

	NxwCharWrapper sw;
	LOGICAL bFound = false;
	P_CHAR pc_curr = NULL;
	SI32 nDist = 0;
	sw.fillCharsNearXYZ( location, nRange, true, true );

	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		pc_curr = sw.getChar();
		if ( !ISVALIDPC(pc_curr) )
			continue;

		if ( pc_curr->IsHiddenBySkill() && !pc->isPermaHidden() )
		{
			nDist = SI32( dist(lCharPos, pc_curr->getPosition()) );
			nLow = SI32( (nDist * 20.0) + (pc_curr->skill[HIDING] / 2.0) );
			if ( nLow < 0 )
				nLow = 0;
			else if ( nLow > 999 )
				nLow = 999;
			if ( pc->checkSkill(DETECTINGHIDDEN, nLow, 1000) ) {
				pc_curr->unHide();
				pc_curr->sysmsg( TRANSLATE("You have been revealed!") );
				pc->sysmsg( TRANSLATE("You revelaled %s"), pc_curr->getCurrentNameC() );
				bFound = true;
			}
		}
	}

	if( !bFound )
		pc->sysmsg( TRANSLATE("You fail to find anyone.") );

	AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,DETECTINGHIDDEN,AMX_AFTER);
}

void target_enticement2( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

	P_CHAR pc_ftarg=pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc_ftarg);

	NXWSOCKET s= ps->toInt();

	ITEM inst = Skills::GetInstrument(s);
	if (inst==INVALID)
	{
		sysmessage(s, TRANSLATE("You do not have an instrument to play on!"));
		return;
	}

	if (pc->checkSkill( ENTICEMENT, 0, 1000) && pc->checkSkill( MUSICIANSHIP, 0, 1000) )
	{
		P_CHAR pc_target = pointers::findCharBySerial( t->buffer[0] );
		VALIDATEPC(pc_target);
		pc_target->ftargserial = pc_ftarg->getSerial32();
		pc_target->npcWander = WANDER_FOLLOW;
		sysmessage(s, TRANSLATE("You play your hypnotic music, luring them near your target."));
		Skills::PlayInstrumentWell(s, inst);
	}
	else
	{
		sysmessage(s, TRANSLATE("Your music fails to attract them."));
		Skills::PlayInstrumentPoor(s, inst);
	}
}

void Skills::target_enticement1( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR current=ps->currChar();
	VALIDATEPC(current);

	P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc);

	NXWSOCKET s = ps->toInt();

	ITEM inst = Skills::GetInstrument(s);
	if (inst==INVALID)
	{
		sysmessage(s, TRANSLATE("You do not have an instrument to play on!"));
		return;
	}
	if ( pc->IsInvul() || pc->shopkeeper )
	{
		sysmessage(s,TRANSLATE(" You cant entice that npc!"));
		return;
	}
	if (pc->InGuardedArea())
	{
		sysmessage(s,TRANSLATE(" You cant do that in town!"));
		return;
	}
	if (!pc->npc)
		sysmessage(s, TRANSLATE("You cannot entice other players."));
	else
	{
		P_TARGET targ= clientInfo[s]->newTarget( new cCharTarget() );
		targ->code_callback = target_enticement2;
		targ->buffer[0]= pc->getSerial32();
		targ->send( ps );
		ps->sysmsg(TRANSLATE("You play your music, luring them near. Whom do you wish them to follow?"));
		PlayInstrumentWell(s, inst);
	}
}


void target_provocation2( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR Victim2 = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(Victim2);

	P_CHAR Player = ps->currChar();
	VALIDATEPC(Player);
	Location charpos= Player->getPosition();

	P_CHAR Victim1 = pointers::findCharBySerial( t->buffer[0] );
	VALIDATEPC(Victim1);

	NXWSOCKET s =ps->toInt();

	if (Victim2->InGuardedArea())
	{
		sysmessage(s,"You cant do that in town!");
		return;
	}
	if (Victim1->isSameAs(Victim2))
	{
		sysmessage(s, "Silly bard! You can't get something to attack itself.");
		return;
	}

	int inst = Skills::GetInstrument(s);
	if (inst==INVALID)
	{
		sysmessage(s, "You do not have an instrument to play on!");
		return;
	}
	if (Player->checkSkill( MUSICIANSHIP, 0, 1000))
	{
		Skills::PlayInstrumentWell(s, inst);
		if (Player->checkSkill( PROVOCATION, 0, 1000))
		{
			if (Player->InGuardedArea() && ServerScp::g_nInstantGuard == 1) //Luxor
				npcs::SpawnGuard(Player, Player, charpos.x+1, charpos.y, charpos.z); //ripper
			sysmessage(s, "Your music succeeds as you start a fight.");
		}
		else
		{
			sysmessage(s, TRANSLATE("Your music fails to incite enough anger."));
			Victim2 = Player;		// make the targeted one attack the Player
		}

		Victim1->fight(Victim2);
		Victim1->SetAttackFirst();

		Victim2->fight(Victim1);
		Victim2->ResetAttackFirst();

		char temp[500];
		sprintf(temp, TRANSLATE("* You see %s attacking %s *"), Victim1->getCurrentNameC(), Victim2->getCurrentNameC());

		NxwSocketWrapper sw;
		sw.fillOnline( Victim1, false );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWSOCKET i=sw.getSocket();
			if( i!=INVALID )
			{
				itemmessage(i, temp, Victim1->getSerial32());
			}
		}
	}
	else
	{
		Skills::PlayInstrumentPoor(s, inst);
		sysmessage(s, TRANSLATE("You play rather poorly and to no effect."));
	}
}

void Skills::target_provocation1( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR current=ps->currChar();
	VALIDATEPC(current);

	P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc);

	NXWSOCKET s =ps->toInt();

	int inst = Skills::GetInstrument(s);
	if (inst==INVALID)
	{
		sysmessage(s, TRANSLATE("You do not have an instrument to play on!"));
		return;
	}

	if (!pc->npc)
		sysmessage(s, TRANSLATE("You cannot provoke other players."));
	else
	{
		P_TARGET targ=clientInfo[s]->newTarget( new cCharTarget() );
		targ->code_callback=target_provocation2;
		targ->buffer[0]=pc->getSerial32();
		targ->send( ps );
		ps->sysmsg( TRANSLATE("You play your music, inciting anger, and your target begins to look furious. Whom do you wish it to attack?"));
		PlayInstrumentWell(s, inst);
	}
}



//////////////////////////
// name:    AlchemyTarget
// history: unknown, revamped by Duke,21.04.2000
// Purpose: checks for valid reg and brings up gumpmenu to select potion
//          This is called after the user dblclicked a mortar and targeted a reg
//
void Skills::target_alchemy( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc_currchar = ps->currChar();
	VALIDATEPC(pc_currchar);

	P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	P_ITEM pack= pc_currchar->getBackpack();    // Get the packitem
	VALIDATEPI(pack);

	NXWSOCKET s = ps->toInt();


	P_ITEM pfbottle=NULL; //candidate of the bottle

	NxwItemWrapper si;
	si.fillItemsInContainer( pack, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM piii=si.getItem();
		if( ISVALIDPI(piii) && piii->type==0) {
			pfbottle=pi;
			break;
		}
	}

	if (!ISVALIDPI(pfbottle))
	{
		sysmessage(s,TRANSLATE("There is no bottle in your pack"));
		return;
	}

    switch (pi->getId())
    {
		case 0x0F7B: itemmenu( s, 7021 ); break;   // Agility,
		case 0x0F84: itemmenu( s, 7022 ); break;   // Cure, Garlic
		case 0x0F8C: itemmenu( s, 7023 ); break;   // Explosion, Sulfurous Ash
		case 0x0F85: itemmenu( s, 7024 ); break;   // Heal, Ginseng
		case 0x0F8D: itemmenu( s, 7025 ); break;   // Night sight
		case 0x0F88: itemmenu( s, 7026 ); break;   // Poison, Nightshade
		case 0x0F7A: itemmenu( s, 7027 ); break;   // Refresh,
		case 0x0F86: itemmenu( s, 7028 ); break;   // Strength,
		case 0x0E9B: break; // Mortar
		default:
            sysmessage(s,TRANSLATE("That is not a valid reagent."));
    }
}


////////////////////////
// name:    HealingSkillTarget
// history: unknown, revamped by Duke, 4.06.2000
//
void Skills::target_healingSkill( NXWCLIENT ps, P_TARGET t )
{

    P_CHAR ph = ps->currChar();   // points to the healer
	VALIDATEPC(ph);
    P_CHAR pp = pointers::findCharBySerial( t->getClicked() );; // pointer to patient
	VALIDATEPC(pp);

    int j;
    P_ITEM pib=MAKE_ITEM_REF( t->buffer[0] );    // item index of bandage
	VALIDATEPI(pib);

    AMXEXECSVTARGET( ph->getSerial32(),AMXT_SKITARGS,HEALING,AMX_BEFORE);

    if (!SrvParms->bandageincombat ) {
		//P_CHAR pc_att=pointers::findCharBySerial(ph->attackerserial);
		if( ph->war/* || pp->war || ( ISVALIDPC(pc_att) && pc_att->war)*/)
		{
			ph->sysmsg(TRANSLATE("You can't heal while in a fight!"));
			return;
		}
	}

	if(!char_inRange(ph,pp,1))
	{
		ph->sysmsg(TRANSLATE("You are not close enough to apply the bandages."));
		return;
	}

	if( ((pp->getId() != BODY_MALE) || (pp->getId() != BODY_FEMALE)) && pp->tamed==false) //Used on non-human and controls if tamed

        if ((ph->IsInnocent()) &&(ph->getSerial32() != pp->getSerial32()))
        {
			ph->helpStuff(pp);
        }

	if (pp->dead)
	{
		if (ph->baseskill[HEALING] < 800 || ph->baseskill[ANATOMY]<800)
			ph->sysmsg(TRANSLATE("You are not skilled enough to resurrect"));
		else
		{
			if(ph->checkSkill(HEALING,800,1000) && ph->checkSkill(ANATOMY,800,1000) ) {
				pp->resurrect();
				ph->sysmsg(TRANSLATE("Because of your skill, you were able to resurrect the ghost."));
			}
            else
				ph->sysmsg(TRANSLATE("You failed to resurrect the ghost"));

			SetTimerSec(&ph->objectdelay,SrvParms->objectdelay+SrvParms->bandagedelay);
			pib->ReduceAmount(1);
            return;
        }

	}

	if (pp->poisoned>0)
	{
		if (ph->baseskill[HEALING]<=600 || ph->baseskill[ANATOMY]<=600)
		{
			ph->sysmsg(TRANSLATE("You are not skilled enough to cure poison."));
			ph->sysmsg(TRANSLATE("The poison in your target's system counters the bandage's effect."));
		}
		else
		{
			if (ph->checkSkill( HEALING,600,1000) && ph->checkSkill(ANATOMY,600,1000))
			{
				pp->poisoned=POISON_NONE;
				ph->sysmsg(TRANSLATE("Because of your skill, you were able to counter the poison."));
			}
			else
				ph->sysmsg(TRANSLATE("You fail to counter the poison"));

		}

		pib->ReduceAmount(1);
		SetTimerSec(&ph->objectdelay,SrvParms->objectdelay+SrvParms->bandagedelay);
		return;
	}

	if(pp->hp >= pp->getStrength())
	{
		ph->sysmsg(TRANSLATE("That being is not damaged"));
		return;
	}

	if(pp->HasHumanBody()) //Used on human
	{
		if (!ph->checkSkill(HEALING,0,1000))
		{
			ph->sysmsg(TRANSLATE("You apply the bandages, but they barely help!"));
			pp->hp++;
		}
		else
		{
			j=ph->skill[HEALING]/100*2 + 1 + rand()%2;      // a GM healer gives 42-44,
			j+=ph->skill[ANATOMY]/100*2 + 1 + rand()%2;     // a 20.0 healer 10-12. Ok ?
			//pp->hp = qmin(pp->st, j+pp->hp);
			//updatestats(i, 0);
			//sysmessage(s,"You apply the bandages and the patient looks a bit healthier.");
			int iMore1 = 0;
			(j+pp->hp > pp->getStrength()) ? iMore1 = pp->getStrength() - pp->hp : iMore1 = j;
			if(pp->getSerial32()==ph->getSerial32())
				tempfx::add(ph, ph, tempfx::HEALING_DELAYHEAL, iMore1/2,0,10);//allow a delay
			else
				tempfx::add(ph, pp, tempfx::HEALING_DELAYHEAL, iMore1/2,0,4);// added suggestion by Ramases //-Fraz- must be checked

		}

	}
    else //Bandages used on a non-human
	{
		if (!ph->checkSkill(VETERINARY,0,1000))
			ph->sysmsg(TRANSLATE("You are not skilled enough to heal that creature."));
		else
		{
			j=((3*ph->skill[VETERINARY])/100) + rand()%6;
			pp->hp=qmin(pp->getStrength(), j+pp->hp);
			pp->updateStats(0);
			ph->sysmsg(TRANSLATE("You apply the bandages and the creature looks a bit healthier."));
		}
	}

	SetTimerSec(&ph->objectdelay,SrvParms->objectdelay+SrvParms->bandagedelay);
    pib->ReduceAmount(1);


    AMXEXECSVTARGET( ph->getSerial32(),AMXT_SKITARGS,HEALING,AMX_AFTER);
}

void Skills::target_armsLore( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);

    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    P_ITEM pi = pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

    int total;
    float totalhp;
    char p2[100];

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,ARMSLORE,AMX_BEFORE);

    if ( (pi->def==0 || pi->pileable)
        && ((pi->lodamage==0 && pi->hidamage==0) && (pi->rank<1 || pi->rank>9)))
    {
        pc->sysmsg(TRANSLATE("That does not appear to be a weapon."));
        return;
    }
    if(pc->IsGM())
    {
        pc->sysmsg(TRANSLATE("Attack [%i] Defense [%i] Lodamage [%i] Hidamage [%i]"), pi->att, pi->def, pi->lodamage, pi->hidamage);
        return;
    }

    if (!pc->checkSkill( ARMSLORE, 0, 250))
        pc->sysmsg(TRANSLATE("You are not certain..."));
    else
    {
        if( pi->maxhp==0)
            pc->sysmsg(TRANSLATE(" Sorry this is a old item and it doesn't have maximum hp"));
        else
        {
            totalhp= (float) pi->hp/pi->maxhp;
            strcpy(temp,TRANSLATE("This item "));
            if  (totalhp>0.9) strcpy(p2, TRANSLATE("is brand new."));
            else if (totalhp>0.8) strcpy(p2, TRANSLATE("is almost new."));
            else if (totalhp>0.7) strcpy(p2, TRANSLATE("is barely used, with a few nicks and scrapes."));
            else if (totalhp>0.6) strcpy(p2, TRANSLATE("is in fairly good condition."));
            else if (totalhp>0.5) strcpy(p2, TRANSLATE("suffered some wear and tear."));
            else if (totalhp>0.4) strcpy(p2, TRANSLATE("is well used."));
            else if (totalhp>0.3) strcpy(p2, TRANSLATE("is rather battered."));
            else if (totalhp>0.2) strcpy(p2, TRANSLATE("is somewhat badly damaged."));
            else if (totalhp>0.1) strcpy(p2, TRANSLATE("is flimsy and not trustworthy."));
            else              strcpy(p2, TRANSLATE("is falling apart."));
            strcat(temp,p2);
            char temp2[33];
            sprintf(temp2," [%.1f %%]",totalhp*100);
            strcat(temp,temp2);  // Magius(CHE) §
        }
        if (pc->checkSkill(ARMSLORE, 250, 510))
        {
            if (pi->hidamage)
            {
                total = (pi->hidamage + pi->lodamage)/2;
                if  ( total > 26) strcpy(p2, TRANSLATE(" Would be extraordinarily deadly."));
                else if ( total > 21) strcpy(p2, TRANSLATE(" Would be a superior weapon."));
                else if ( total > 16) strcpy(p2, TRANSLATE(" Would inflict quite a lot of damage and pain."));
                else if ( total > 11) strcpy(p2, TRANSLATE(" Would probably hurt your opponent a fair amount."));
                else if ( total > 6)  strcpy(p2, TRANSLATE(" Would do some damage."));
                else if ( total > 3)  strcpy(p2, TRANSLATE(" Would do minimal damage."));
                else              strcpy(p2, TRANSLATE(" Might scratch your opponent slightly."));
                strcat(temp,p2);

                if (pc->checkSkill( ARMSLORE, 500, 1000))
                {
                    if  (pi->spd > 35) strcpy(p2, TRANSLATE(" And is very fast."));
                    else if (pi->spd > 25) strcpy(p2, TRANSLATE(" And is fast."));
                    else if (pi->spd > 15) strcpy(p2, TRANSLATE(" And is slow."));
                    else               strcpy(p2, TRANSLATE(" And is very slow."));
                    strcat(temp,p2);
                }
            }
            else
            {
                if  (pi->def> 12) strcpy(p2, TRANSLATE(" Is superbly crafted to provide maximum protection."));
                else if (pi->def> 10) strcpy(p2, TRANSLATE(" Offers excellent protection."));
                else if (pi->def> 8 ) strcpy(p2, TRANSLATE(" Is a superior defense against attack."));
                else if (pi->def> 6 ) strcpy(p2, TRANSLATE(" Serves as a sturdy protection."));
                else if (pi->def> 4 ) strcpy(p2, TRANSLATE(" Offers some protection against blows."));
                else if (pi->def> 2 ) strcpy(p2, TRANSLATE(" Provides very little protection."));
                else if (pi->def> 0 ) strcpy(p2, TRANSLATE(" Provides almost no protection."));
                else              strcpy(p2, TRANSLATE(" Offers no defense against attackers."));
                strcat(temp,p2);
            }
        }
        pc->sysmsg(temp);

        if (!(pi->rank<1 || pi->rank>10 || SrvParms->rank_system==0))
        {
            if (pc->checkSkill(ARMSLORE, 250, 500))
            {
                switch(pi->rank)
                {
                    case 1: strcpy(p2, TRANSLATE("It seems an item with no quality!"));             break;
                    case 2: strcpy(p2, TRANSLATE("It seems an item very below standard quality!"));     break;
                    case 3: strcpy(p2, TRANSLATE("It seems an item below standard quality!"));      break;
                    case 4: strcpy(p2, TRANSLATE("It seems a weak quality item!"));                     break;
                    case 5: strcpy(p2, TRANSLATE("It seems a standard quality item!"));             break;
                    case 6: strcpy(p2, TRANSLATE("It seems a nice quality item!"));                     break;
                    case 7: strcpy(p2, TRANSLATE("It seems a good quality item!"));                     break;
                    case 8: strcpy(p2, TRANSLATE("It seems a great quality item!"));                break;
                    case 9: strcpy(p2, TRANSLATE("It seems a beautiful quality item!"));            break;
                    case 10:strcpy(p2, TRANSLATE("It seems a perfect quality item!"));              break;
                }
                pc->sysmsg(p2);
            }
        }
    }
    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,ARMSLORE,AMX_AFTER);

}

void Skills::target_itemId( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc=ps->currChar();
	VALIDATEPC(pc);

    const P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

    char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,ITEMID,AMX_BEFORE);
    if( pi->magic!=4) // Ripper
    {
        if (!pc->checkSkill( ITEMID, 0, 250))
        {
            pc->sysmsg(TRANSLATE("You can't quite tell what this item is..."));
        }
        else
        {
            if(pi->corpse)
            {
                pc->sysmsg(TRANSLATE("You have to use your forensics evalutation skill to know more on this corpse."));
                return;
            }

            // Identify Item by Antichrist // Changed by MagiusCHE)
            if (pc->checkSkill( ITEMID, 250, 500))
                if (pi->getSecondaryNameC() && (strcmp(pi->getSecondaryNameC(),"#"))) pi->setCurrentName(pi->getSecondaryNameC()); // Item identified! -- by Magius(CHE)

            if( ! strncmp(pi->getCurrentNameC(), "#", 1) )
				pi->getName(temp2);
            else
				strcpy(temp2, pi->getCurrentNameC());

            pc->sysmsg(TRANSLATE("You found that this item appears to be called: %s"), temp2);

            // Show Creator by Magius(CHE)
            if (pc->checkSkill( ITEMID, 250, 500))
            {
                if (!pi->creator.empty())
                {
                    if (pi->madewith>0) pc->sysmsg(TRANSLATE("It is %s by %s"),skillinfo[pi->madewith-1].madeword,pi->creator.c_str()); // Magius(CHE)
                    else if (pi->madewith<0) pc->sysmsg(TRANSLATE("It is %s by %s"),skillinfo[0-pi->madewith-1].madeword,pi->creator.c_str()); // Magius(CHE)
                    else pc->sysmsg(TRANSLATE("It is made by %s"),pi->creator.c_str()); // Magius(CHE)
                } else pc->sysmsg(TRANSLATE("You don't know its creator!"));
            } else pc->sysmsg(TRANSLATE("You can't know its creator!"));
            // End Show creator

            if (!pc->checkSkill( ITEMID, 250, 500))
            {
                pc->sysmsg(TRANSLATE("You can't tell if it is magical or not."));
            }
            else
            {
                if(pi->type != ITYPE_WAND)
                {
                    pc->sysmsg(TRANSLATE("This item has no hidden magical properties."));
                }
                else
                {
                    if (!pc->checkSkill( ITEMID, 500, 1000))
                    {
                        pc->sysmsg(TRANSLATE("This item is enchanted with a spell, but you cannot determine which"));
                    }
                    else
                    {
                        if (!pc->checkSkill( ITEMID, 750, 1100))
                        {
                            pc->sysmsg(TRANSLATE("It is enchanted with the spell %s, but you cannot determine how many charges remain."),spellname[(8*(pi->morex-1))+pi->morey-1]);
                        }
                        else
                        {
                            pc->sysmsg(TRANSLATE("It is enchanted with the spell %s, and has %d charges remaining."),spellname[(8*(pi->morex-1))+pi->morey-1],pi->morez);
                        }
                    }
                }
            }
        }
    }

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,ITEMID,AMX_AFTER);
}


void Skills::target_tame( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);
	P_CHAR target = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(target);

	NXWSOCKET s=ps->toInt();

	int tamed=0;

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	if(line_of_sight(INVALID, pc->getPosition(), target->getPosition(), WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)==0)
		return;

	AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,TAMING,AMX_BEFORE);

	if(buffer[s][7]==0xFF) return;

	if (target->npc==1 && ( pc->distFrom(target) <= 3))
	{
		if ( (target->taming > 1000) || (target->taming ==0) )//Morrolan default is now no tame
		{
			pc->sysmsg(TRANSLATE("You can't tame that creature."));
			return;
		}

		if( (target->tamed) && pc->isOwnerOf(target) )
		{
			pc->sysmsg(TRANSLATE("You already control that creature!" ));
			return;
		}

		if( target->tamed )
		{
			pc->sysmsg(TRANSLATE("That creature looks tame already." ));
			return;
		}

		for(int a=0;a<3;a++)
		{
			switch(rand()%4)
			{
				case 0: pc->talkAll( TRANSLATE("I've always wanted a pet like you."),0); break;
				case 1: pc->talkAll( TRANSLATE("Will you be my friend?"),0); break;
				case 2: sprintf(temp, TRANSLATE("Here %s."), target->getCurrentNameC()); pc->talkAll( temp,0); break;
				case 3: sprintf(temp, TRANSLATE("Good %s."), target->getCurrentNameC()); pc->talkAll( temp,0); break;
				default:
					LogError("switch reached default");
			}
		}

		if ( (!pc->checkSkill(TAMING, 0, 1000)) || (pc->skill[TAMING] < target->taming) )
		{
			pc->sysmsg(TRANSLATE("You were unable to tame it."));
			return;
		}

		pc->talk(s, TRANSLATE("It seems to accept you as it's master!"),0);
		tamed=1;
		target->setOwner(pc);

		if((target->getId()==0x000C) || (target->getId()==0x003B))
		{
			if(target->getColor() != 0x0481)
			{
				target->npcaitype=NPCAI_TAMEDDRAGON;
			}
		}
	}

	if (tamed==0) pc->sysmsg(TRANSLATE("You can't tame that!"));

	AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,TAMING,AMX_AFTER);
}


void Skills::target_begging( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pcc=ps->currChar();
	VALIDATEPC(pcc);

    P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc);

	NXWSOCKET s = ps->toInt();

    int gold,x,y,realgold;
    char abort;

    AMXEXECSVTARGET( pcc->getSerial32(),AMXT_SKITARGS,BEGGING,AMX_BEFORE);

    if(pc->IsOnline())
    {
        sysmessage(s,TRANSLATE("Maybe you should just ask."));
        return;
    }

        if( pc->distFrom(pcc)>=begging_data.range)
        {
            sysmessage(s,TRANSLATE("You are not close enough to beg."));
            return;
        }

        if(pc->HasHumanBody() && (pc->in != 0)) //Used on human
        {
            if (pc->begging_timer>=uiCurrentTime)
            {
                pc->talk(s,TRANSLATE("Annoy someone else !"),1);
                return;
            }

            pcc->talkAll( begging_data.text[rand()%3],0); // npcemoteall?
            if (!pcc->checkSkill( BEGGING, 0, 1000))
                sysmessage(s,TRANSLATE("They seem to ignore your begging plees."));
            else
            {
                SetTimerSec(&pc->begging_timer,begging_data.timer);
                x=pc->skill[BEGGING]/50;

                if (x<1) x=1;
                y=rand()%x;
                y+=RandomNum(1,4);
                if (y>25) y=25;
                // pre-calculate the random amout of gold that is "targeted"

                P_ITEM pack= pc->getBackpack();
                gold=0;
                realgold=0;
                abort=0;

                // check for gold in target-npc pack

                if (ISVALIDPI(pack))
                {
					NxwItemWrapper si;
					si.fillItemsInContainer( pack, false );
					for( si.rewind(); !si.isEmpty(); si++ ) {

						P_ITEM pj= si.getItem();

                        if (ISVALIDPI(pj))
                        {
                            if (pj->getId()==0x0EED  )
                            {
                                gold+=pj->amount; // calc total gold in pack

                                int k=pj->amount;
                                if(k>=y) // enough money in that pile in pack to satisfy pre-aclculated amount
                                {
                                    pj->ReduceAmount(y);
                                    realgold+=y; // calc gold actually given to player
                                    pj->Refresh(); // resend new amount
                                    abort=1;
                                }
                                else // not enough money in this pile -> only delete it
                                {
                                    realgold+=pj->amount;
                                    pj->Delete();
                                    pj->Refresh();
                                }
                            }
                        } // end of if j!=-1
                        if (abort) break;
                    }
                }

                if (gold<=0)
                {
                    pc->talk(s,TRANSLATE("Sorry, I'm poor myself"),1);
                    return;
                }

                pc->talkAll(TRANSLATE("Ohh thou lookest so poor, Here is some gold I hope this will assist thee."),0); // zippy
                addgold(s,realgold);
                sysmessage(s,TRANSLATE("Some gold is placed in your pack."));
            }
        }
        else
            sysmessage(s, TRANSLATE("That would be foolish."));


    AMXEXECSVTARGET( pcc->getSerial32(),AMXT_SKITARGS,BEGGING,AMX_AFTER);
}

void Skills::target_animalLore( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);

	P_CHAR target = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(target);

	NXWSOCKET s = ps->toInt();

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    // blackwind distance fix
	if( target->distFrom(pc) >= 10 )
	{
		pc->sysmsg(TRANSLATE("You need to be closer to find out more about them" ));
		return;
	}

	AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,ANIMALLORE,AMX_BEFORE);

	if (target->IsGMorCounselor())
	{
		pc->sysmsg(TRANSLATE( "Little is known of these robed gods."));
		return;
	}
	else if( target->HasHumanBody() ) // Used on human
	{
		pc->sysmsg(TRANSLATE("The human race should use dvorak!"));
	}
	else // Lore used on a non-human
	{
        	if (target->checkSkill( ANIMALLORE, 0, 1000))
        	{
			P_CHAR target_owner = pointers::findCharBySerial( target->getOwnerSerial32() );
			VALIDATEPC(target_owner);

			sprintf(temp, TRANSLATE("Attack [%i] Defense [%i] Taming [%i] Hit Points [%i] Is Loyal to: [%s]"), target->att, target->def, target->taming/10, target->hp, (target->tamed)? target_owner->getCurrentNameC() : "himself" );
			target->emote(s,temp,1);
        	}
        	else
        	{
            		pc->sysmsg(TRANSLATE("You can not think of anything relevant at this time."));
        	}
	}

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,ANIMALLORE,AMX_AFTER);
}

void Skills::target_forensics( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);
	P_ITEM pi = pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,FORENSICS,AMX_BEFORE);

	int curtim=uiCurrentTime;


	if (!pi->corpse) {
    		pc->sysmsg(TRANSLATE("That does not appear to be a corpse."));
    		return;
	}

	if(pc->IsGM()) {
    		pc->sysmsg(TRANSLATE("The %s is %i seconds old and the killer was %s."), pi->getCurrentNameC(), (curtim-pi->murdertime)/MY_CLOCKS_PER_SEC, pi->murderer.c_str());
	} else {
		if (!pc->checkSkill( FORENSICS, 0, 500)) pc->sysmsg(TRANSLATE("You are not certain about the corpse.")); else
    		{
			char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

			if(((curtim-pi->murdertime)/MY_CLOCKS_PER_SEC)<=60) strcpy(temp2,TRANSLATE("few"));
			if(((curtim-pi->murdertime)/MY_CLOCKS_PER_SEC)>60) strcpy(temp2,TRANSLATE("many"));
			if(((curtim-pi->murdertime)/MY_CLOCKS_PER_SEC)>180) strcpy(temp2,TRANSLATE("many many"));

			pc->sysmsg(TRANSLATE("The %s is %s seconds old."), pi->getCurrentNameC(), temp2);

			if (!pc->checkSkill( FORENSICS, 500, 1000) || pi->murderer.empty())
				pc->sysmsg(TRANSLATE("You can't say who was the killer."));
			else
				pc->sysmsg(TRANSLATE("The killer was %s."), pi->murderer.c_str());
		}
	}

	AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,FORENSICS,AMX_AFTER);
}



/*!
\brief Poison target
\author AntiChrist, rewritten by Endymion
\param ps the client
\note pi->morez is the poison type
*/
void target_poisoning2( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc = ps->currChar();
    VALIDATEPC(pc);
	NXWSOCKET s = ps->toInt();

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,POISONING,AMX_BEFORE);
    P_ITEM poison=pointers::findItemBySerial(t->buffer[0]);
    VALIDATEPI(poison);

    if(poison->type!=ITYPE_POTION || poison->morey!=6)
    {
        pc->sysmsg(TRANSLATE("That is not a valid poison!"));
		pc->objectdelay = 0;
        return;
    }

    if ( ! CheckInPack( s, poison) )
    {
		pc->objectdelay = 0;
		return;
    }


    const P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
    if( !ISVALIDPI(pi) ) {
        pc->sysmsg(TRANSLATE("You can't poison that item."));
		pc->objectdelay = 0;
        return;
    }

    if ( ! CheckInPack( s, pi) )
    {
	pc->objectdelay = 0;
	return;
    }

	if (!pi->IsFencing1H() && !pi->IsSword() && !pi->IsArrow() && !pi->IsBolt())
	{
		pc->sysmsg(TRANSLATE("You cannot poison that item"));
		pc->objectdelay = 0;
		return;
	}

	if ( (pi->IsArrow()||pi->IsBolt()) && pi->amount>5)
	{
		pc->sysmsg(TRANSLATE("Too many items you can poison at least 5 items"));
		pc->objectdelay = 0;
		return;
	}

	if (pi->amxevents[EVENT_IONPOISONED]!=NULL) {
		g_bByPass = false;
		poison->morez = pi->amxevents[EVENT_IONPOISONED]->Call(pi->getSerial32(), s, poison->morez);
		if (g_bByPass==true) return;
	}
	/*
	if ( pi->getAmxEvent(EVENT_IONPOISONED) != NULL ) {
		poison->morez = pi->runAmxEvent( EVENT_IONPOISONED, pi->getSerial32(), s, poison->morez );
		if (g_bByPass==true)
			return;
	}
	*/

	int success=0;
    switch(poison->morez)
	{
        case 0: break;
        case 1: success=pc->checkSkill( POISONING, 0, 500);	break; //lesser poison
        case 2:
			{
				success=pc->checkSkill( POISONING, 251, 651);
				if (pc->skill[POISONING]<650) success=0;
				break;//poison
			}
        case 3:
			{
				success=pc->checkSkill( POISONING, 851, 1051);
				if (pc->skill[POISONING]<850) success=0;
				break;//greater poison
			}
        case 4:
			{
				success=pc->checkSkill( POISONING, 1201, 1401);
				if (pc->skill[POISONING]<950) success=0;
				break;//deadly poison
			}
        default:
            LogError("switch reached default");
            return;
	}

	if (poison->morez!=0) {
		pc->playSFX( 0x0247);
		if(success!=0)
        {
			if(pi->poisoned<(PoisonType)poison->morez) pi->poisoned=(PoisonType)poison->morez;
			pc->sysmsg(TRANSLATE("You successfully poison that item."));
		}
		else
		{
			pc->sysmsg(TRANSLATE("You fail to apply the poison."));
			pi->hp-=poison->morez;
			pi->poisoned=POISON_NONE;
			if(pi->hp<=0)
			{
				pc->sysmsg(TRANSLATE("Your weapon has been destroyed"));
                                //<Luxor>
				ps->sendRemoveObject( static_cast<P_OBJECT>(pi) );
				pi->Delete();
			}
		}
	}

	poison->Delete();
	//spawn an empty bottle after poisoning
	P_ITEM emptybottle=archive::item::New();

	emptybottle->setId( 0x0F0E );
	emptybottle->pileable=1;
	emptybottle->MoveTo( pc->getPosition() );
	emptybottle->priv|=0x01;
	emptybottle->Refresh();

    AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,POISONING,AMX_AFTER);
}


void Skills::target_poisoning( NXWCLIENT ps, P_TARGET t )
{
	P_ITEM poison = pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(poison);

	P_TARGET targ = clientInfo[ps->toInt()]->newTarget( new cItemTarget() );
	targ->code_callback=target_poisoning2;
	targ->buffer[0]=poison->getSerial32();
	targ->send( ps );
	ps->sysmsg( TRANSLATE("What item do you want to poison?") );

}


void Skills::target_tinkering( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc_currchar = ps->currChar();
	VALIDATEPC(pc_currchar);
    P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	NXWSOCKET s = ps->toInt();

    AMXEXECSVTARGET( pc_currchar->getSerial32(),AMXT_SKITARGS,TINKERING,AMX_BEFORE);

    if ( pi->magic!=4) // Ripper
    {
        if(pi->getId()==0x1BEF || pi->getId()==0x1BF2 || pi->IsLog() )
        {
            if (CheckInPack(s,pi))
            {
                int amt;
                amt=pc_currchar->CountItems(pi->getId(), pi->getColor());
                if(amt<2)
                {
                    sysmessage(s,TRANSLATE("You don't have enough ingots to make anything."));
                    return;
                }
                if ( pi->IsLog() )
                {
                    if (amt<4)
                    {
                        sysmessage(s,TRANSLATE("You don't have enough log's to make anything."));
                        return;
                    }
                    else Skills::MakeMenu(pc_currchar,70,TINKERING,pi);
                }
                else
                {
                    Skills::MakeMenu(pc_currchar,80,TINKERING,pi);
                }
            }
            return;
        }
    }
    sysmessage(s,TRANSLATE("You cannot use that material for tinkering."));

    AMXEXECSVTARGET(pc_currchar->getSerial32(),AMXT_SKITARGS,TINKERING,AMX_AFTER);
}

//////////////////////////////////
// name:    cTinkerCombine
// history: by Duke, 3.06.2000
// Purpose: handles the combining of two (tinkering-)items after the user
//          dclicked one and then targeted the second item
//
//          It's a base class for three derived classes that handle the
//          old interfaces
//
class cTinkerCombine    // Combining tinkering items
{
protected:
    char* failtext;
//    short badsnd1;
//    short badsnd2;
    short badsnd;
    short itembits;
    short minskill;
    short id2;
public:
    cTinkerCombine(short badsnd=0x0051, char *failmsg=TRANSLATE("You break one of the parts."))
    {
        this->badsnd=badsnd;
        failtext=failmsg;
        itembits=0;
        minskill=100;
    }
    /*
    virtual void delonfail(SOCK s)      {deletematerial(s, itemmake[s].needs/2);}
    virtual void delonsuccess(SOCK s)   {deletematerial(s, itemmake[s].needs);}
    virtual void failure(SOCK s)        {delonfail(s);playbad(s);failmsg(s);}
    */
    virtual void failmsg(NXWSOCKET s)         {sysmessage(s,failtext);}
    virtual void playbad(NXWSOCKET s)         {soundeffect(s, badsnd);}
    virtual void playgood(NXWSOCKET s)        {soundeffect(s, 0x002A);}
    virtual void checkPartID(short id)  {;}
    virtual bool decide()               {return (itembits == 3) ? true : false;}
    virtual void createIt(NXWSOCKET s)        {;}
    static cTinkerCombine* factory(short combinetype);
    virtual void DoIt( NXWCLIENT ps, P_TARGET t )
    {
        NXWSOCKET s = ps->toInt();

		P_ITEM piClick = pointers::findItemBySerial( t->buffer[0] );
        if( piClick == NULL )
        {
            sysmessage( s,TRANSLATE("Original part no longer exists" ));
            return;
        }

        const P_ITEM piTarg=pointers::findItemBySerial( t->getClicked() );
        if (piTarg==NULL || piTarg->magic==4)
        {
            sysmessage(s,TRANSLATE("You can't combine these."));
            return;
        }

        // make sure both items are in the player's backpack
        P_ITEM pPack=Check4Pack(s);
        if (pPack==NULL) return;
        if ( piTarg->getContSerial()!=pPack->getSerial32()
            || piClick->getContSerial()!=pPack->getSerial32())
        {
            sysmessage(s,TRANSLATE("You can't use material outside your backpack"));
            return;
        }

        // make sure the parts are of correct IDs AND they are different
        checkPartID( piClick->getId() );
        checkPartID( piTarg->getId() );
        if (!decide())
            sysmessage(s,TRANSLATE("You can't combine these."));
        else
        {
            P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);

            if (pc_currchar->skill[TINKERING]<minskill)
            {
                sysmessage(s,TRANSLATE("You aren't skilled enough to even try that!"));
                return;
            }
            if( !pc_currchar->checkSkill( TINKERING, minskill, 1000 ) )
            {
                failmsg(s);
                P_ITEM piLoser= rand()%2 ? piTarg : piClick;
                piLoser->ReduceAmount(1);
                playbad(s);
            }
            else
            {
                sysmessage(s,TRANSLATE("You combined the parts"));
                piClick->ReduceAmount(1);
                piTarg->ReduceAmount(1);        // delete both parts
                createIt(s);                        // spawn the item
                playgood(s);
            }
        }
    }
};

class cTinkCreateAwG : public cTinkerCombine
{
public:
    cTinkCreateAwG() : cTinkerCombine() {}
    virtual void checkPartID(short id)
    {
        if (id==0x105B || id==0x105C) itembits |= 0x01; // axles
        if (id==0x1053 || id==0x1054) itembits |= 0x02; // gears
    }
    virtual void createIt(NXWSOCKET s)
    {
		P_CHAR pc=pointers::findCharBySerial( currchar[s] );
		VALIDATEPC(pc);
		item::CreateFromScript( "$item_axles_with_gears", pc->getBackpack() );
    }
};

class cTinkCreateParts : public cTinkerCombine
{
public:
    cTinkCreateParts() : cTinkerCombine() {}
    virtual void checkPartID(short id)
    {
        if (id==0x1051 || id==0x1052) itembits |= 0x01; // axles with gears
        if (id==0x1055 || id==0x1056) itembits |= 0x02; // hinge
        if (id==0x105D || id==0x105E) itembits |= 0x04; // springs
    }
    virtual bool decide()
    {
        if (itembits == 3) {id2=0x59; minskill=300; return true;}   // sextant parts
        if (itembits == 5) {id2=0x4F; minskill=400; return true;}   // clock parts
        return false;
    }
    virtual void createIt(NXWSOCKET s)
    {
	            if ( s < 0 || s >= now )
		return;
	P_CHAR pc = pointers::findCharBySerial( currchar[s] );
	VALIDATEPC( pc );


    if (id2 == 0x4F)
          item::CreateFromScript( "$item_clock_parts", pc->getBackpack() );
        else
          item::CreateFromScript( "$item_sextant_parts", pc->getBackpack() );
    }
};

class cTinkCreateClock : public cTinkerCombine
{
public:
    cTinkCreateClock() : cTinkerCombine() {}
    virtual void checkPartID(short id)
    {
        if (id==0x104D || id==0x104E) itembits |= 0x01; // clock frame
        if (id==0x104F || id==0x1050) itembits |= 0x02; // clock parts
    }
    virtual bool decide()   {minskill=600; return cTinkerCombine::decide();}
    virtual void createIt(NXWSOCKET s)
    {
        if ( s < 0 || s >= now )
		return;
	P_CHAR pc = pointers::findCharBySerial( currchar[s] );
	VALIDATEPC( pc );
        item::CreateFromScript( "$item_clock", pc->getBackpack() );
    }
};

#define cTC_AwG     11
#define cTC_Parts   22
#define cTC_Clock   33

cTinkerCombine* cTinkerCombine::factory(short combinetype)
{
    switch (combinetype)
    {
        case cTC_AwG:   return new cTinkCreateAwG();
        case cTC_Parts: return new cTinkCreateParts();    // clock/sextant parts
        case cTC_Clock: return new cTinkCreateClock();    // clock
        default:        return new cTinkerCombine();      // a generic handler
    }
}

void Skills::target_tinkerAxel( NXWCLIENT ps, P_TARGET t )
{
    cTinkerCombine *ptc = cTinkerCombine::factory(cTC_AwG);
    ptc->DoIt(ps, t);
}

void Skills::target_tinkerAwg( NXWCLIENT ps, P_TARGET t )
{
    cTinkerCombine *ptc = cTinkerCombine::factory(cTC_Parts);
    ptc->DoIt(ps, t);
}

void Skills::target_tinkerClock( NXWCLIENT ps, P_TARGET t )
{
    cTinkerCombine *ptc = cTinkerCombine::factory(cTC_Clock);
    ptc->DoIt(ps, t);
}

void Skills::target_repair( NXWCLIENT ps, P_TARGET t )
{

    P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);

    P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	NXWSOCKET s = ps->toInt();

    short smithing=pc->baseskill[BLACKSMITHING];

    if (smithing < 500)
    {
        sysmessage(s,TRANSLATE("* Your not skilled enough to repair items.*"));
        return;
    }

    if ( pi->magic!=4)
    {
        if (!CheckInPack(s,pi))
            return;
        if (!pi->hp)
        {
            sysmessage(s,TRANSLATE(" That item cant be repaired."));
            return;
        }
        if(!AnvilInRange(s))
        {
            sysmessage(s,TRANSLATE(" Must be closer to the anvil."));
            return;
        }
        if (pi->hp>=pi->maxhp)
        {
            sysmessage(s,TRANSLATE(" That item is at full strength."));
            return;
        }
        short dmg=4;    // damage to maxhp
        if      ((smithing>=900)) dmg=1;
        else if ((smithing>=700)) dmg=2;
        else if ((smithing>=500)) dmg=3;

        if (pc->checkSkill(BLACKSMITHING, 0, 1000))
        {
            pi->maxhp-=dmg;
            pi->hp=pi->maxhp;
            sysmessage(s,TRANSLATE(" * the item has been repaired.*"));
        }
        else
        {
            pi->hp-=2;
            pi->maxhp-=1;
            sysmessage(s,TRANSLATE(" * You fail to repair the item. *"));
            sysmessage(s,TRANSLATE(" * You weaken the item.*"));
        }
    }
}

