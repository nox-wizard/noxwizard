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
#include "magic.h"
#include "sregions.h"
#include "sndpkg.h"
#include "debug.h"
#include "amx/amxcback.h"
#include "layer.h"
#include "commands.h"
#include "npcai.h"

COLOR addrandomcolor(cObject* po, char *colorlist)
{
	
	if( (po!=NULL) && isCharSerial( po->getSerial32() ) )
		{ VALIDATEPCR((P_CHAR)po,0); }
	else 
		VALIDATEPIR((P_ITEM)po,0);
	
	char sect[512];
	int i,j,storeval = 0;
	i=0; j=0;
	cScpIterator* iter = NULL;
	char script1[1024];

	sprintf(sect, "SECTION RANDOMCOLOR %s", colorlist);
	iter = Scripts::Colors->getNewIterator(sect);
	if (iter==NULL) {
		WarnOut("Colorlist %s not found on character: %s\n", colorlist, po->getCurrentNameC());
		return 0;
	}

	int loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	safedelete(iter);

	if(i>0)
	{
		i=rand()%i;
		i++;
		iter = Scripts::Colors->getNewIterator(sect);
		if (iter==NULL) {
			WarnOut("Colorlist %s not found on character: %s\n", colorlist, po->getCurrentNameC());
			return 0;
		}
		loopexit=0;
		do
		{
			strcpy(script1, iter->getEntry()->getFullLine().c_str());
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				j++;
				if(j==i)
				{
					storeval=hex2num(script1);
				}
			}
		}
		while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
		safedelete(iter);
	}
	return (storeval);

}

static COLOR addrandomhaircolor(P_CHAR pc, char* colorlist)
{

	VALIDATEPCR(pc,0);

	char sect[512];
	int i,j,haircolor = 0;
	i=0; j=0;
	cScpIterator* iter = NULL;
	char script1[1024];

	sprintf(sect, "SECTION RANDOMCOLOR %s", colorlist);
	iter = Scripts::Colors->getNewIterator(sect);
	if (iter==NULL)
	{
		WarnOut("Colorlist %s not found on character: %s\n", colorlist, pc->getCurrentNameC());
		return 0;
	}

	int loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	safedelete(iter);

	if(i>0)
	{
		i=rand()%i;
		i++;
		iter = Scripts::Colors->getNewIterator(sect);
		if (iter==NULL)
		{
			WarnOut("Colorlist %s not found on character: %s\n", colorlist, pc->getCurrentNameC());
			return 0;
		}
		loopexit=0;
		do
		{
			strcpy(script1, iter->getEntry()->getFullLine().c_str());
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				j++;
				if(j==i)
				{
					haircolor=hex2num(script1);
				}
			}
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
		safedelete(iter);
	}
	return (haircolor);


}

char* getRandomName(char * namelist)
{
        char sect[512];
	static char script1[1024];
        int i=0,j=0;

		cScpIterator* iter = NULL;

        sprintf(sect, "SECTION RANDOMNAME %s", namelist);
		iter = Scripts::Npc->getNewIterator(sect);
        if (iter==NULL) {
                //sprintf(chars[s].name, "Error Namelist %s Not Found", namelist);
                return "I_am_a_bug";
        }
	
        int loopexit=0;
        do
        {
                strcpy(script1, iter->getEntry()->getFullLine().c_str());
                if ((script1[0]!='}')&&(script1[0]!='{'))
                {
                        i++;
                }
        }
        while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

        iter->rewind();

        if(i>0)
        {
                i=rand()%(i);
                loopexit=0;
                do
                {
                        strcpy(script1, iter->getEntry()->getFullLine().c_str());
                        if ((script1[0]!='}')&&(script1[0]!='{'))
                        {
                                if(j==i)
                                {
                                        //return((char*)(&script1));
					return(script1); // anthalir elcabesa bug fix
                                }
                                else j++;
                        }
                }
                while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
                safedelete(iter);
        }

	return "I_am_a_bug";
}

void setrandomname(P_CHAR pc, char * namelist)
{
	

	VALIDATEPC(pc);

	std::string sect( string("SECTION RANDOMNAME ") + namelist );
	cScpIterator*	iter = 0;
	int 		i=0,
			j=0;

	iter = Scripts::Npc->getNewIterator(sect);
	if ( iter != 0 )
	{
		std::string 	script1;
		int		loopexit=0;
		do
		{
			script1 = iter->getEntry()->getFullLine();
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				i++;
			}
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

		iter->rewind();

		if(i>0)
		{
			i=rand()%(i);
			loopexit=0;
			do
			{
				script1 = iter->getEntry()->getFullLine();
				if ((script1[0]!='}')&&(script1[0]!='{'))
				{
					if(j==i)
					{
						pc->setCurrentName( script1 );
						break;
					}
					else j++;
				}
			}
			while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
			safedelete(iter);
		}
	}
	else
	{
		pc->setCurrentName( string("Namelist not found: ") + string( namelist ).substr(0,10) );
	}

}

namespace npcs {	//Luxor



//<Anthalir>
void SpawnGuard(P_CHAR pc, P_CHAR pc_i, Location where)
{
	SpawnGuard(pc, pc_i, where.x, where.y, where.z);
}
//</Anthalir>

void SpawnGuard(P_CHAR pc, P_CHAR pc_i, int x, int y, signed char z)
{

	VALIDATEPC(pc);

	VALIDATEPC(pc_i);

	int t;
	if (!(region[pc_i->region].priv&0x01)) return;
	if( pc->dead || pc_i->dead ) return; //AntiChrist

	if(SrvParms->guardsactive /*&& !chars[s].isInvul()*/)
	{
		t=RandomNum(1000,1001);
		t=region[pc_i->region].guardnum[(rand()%10)+1];
		P_CHAR pc_c = npcs::AddNPCxyz(pc->getSocket(), t, x, y, z);

		if (ISVALIDPC(pc_c))
		{
		  pc_c->npcaitype=NPCAI_TELEPORTGUARD;
		  pc_c->SetAttackFirst();
		  pc_c->attackerserial=pc->getSerial32();
		  pc_c->targserial=pc->getSerial32();
		  pc_c->npcWander=2;  //set wander mode Tauriel
		  pc_c->toggleCombat();
		  pc_c->setNpcMoveTime();
		  pc_c->summontimer=(getclock()+(MY_CLOCKS_PER_SEC*25));

		  pc_c->playSFX(0x01FE);
		  staticeffect(DEREF_P_CHAR(pc_c), 0x37, 0x2A, 0x09, 0x06);

		  pc_c->teleport();
		  pc_c->talkAll(TRANSLATE("Thou shalt regret thine actions, swine!"),1);
		}
	}

}

P_ITEM AddRandomLoot(P_ITEM pack, char * lootlist)
{
	VALIDATEPIR(pack, NULL);

	char sect[512];
	char script1[1024];
	int i,j, loopexit=0;
	cScpIterator* iter = NULL;

	i=0; j=0;

	sprintf(sect, "SECTION LOOTLIST %s", lootlist);
	iter = Scripts::Npc->getNewIterator(sect);
	if (iter==NULL) return NULL;

	loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			i++; // Count number of entries on list.
		}
	} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	iter->rewind();

	P_ITEM pi=NULL;
	if(i>0)
	{
		i=rand()%(i);

		loopexit=0;
		do
		{
			strcpy(script1, iter->getEntry()->getFullLine().c_str());
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if(j==i)
				{
					int storeval=str2num(script1);	//script1 = ITEM#
					pi=item::CreateFromScript( storeval, pack );

					break;
				}
				else j++;
			}
		}	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	}
	safedelete(iter);
	return pi;

}

/*** s: socket ***/
//Xan : upgraded to new scripts
int AddRandomNPC(NXWSOCKET s, char * npclist, int spawnpoint)
{
	//This function gets the random npc number from the list and recalls
	//addrespawnnpc passing the new number
	char sect[512];
	char script1[1024];
	cScpIterator* iter = NULL;
	unsigned int uiTempList[100];
	int i=0,k=0;
	sprintf(sect, "SECTION NPCLIST %s", npclist);

	iter = Scripts::Npc->getNewIterator(sect);
	if (iter==NULL) return -1;

	int loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			uiTempList[i]=str2num(script1);
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS));

	safedelete(iter);

	if(i>0)
	{
		i=rand()%(i);
		k=uiTempList[i];
	}
	if(k!=0)
	{
		if (spawnpoint==-1)
		{
			addmitem[s]=k;
			return targets::NpcMenuTarget(s);
			//return -1;
		}
		else
		{
			return k; //addrespawnnpc(spawnpoint,k,1);
		}
	}
	return -1;

}

//o---------------------------------------------------------------------------o
//| Function   : AddNPC (3 interfaces)
//| Programmer : Duke, 23.05.2000
//o---------------------------------------------------------------------------o
//| Purpose    : creates the scripted NPC given by npcNum
//| 			 The position of the NPC can be given in three different ways:
//| 			 1. by parms x1 y1 z1 (trigger)
//| 			 2. by passing a socket (GM add)
//| 			 3. by passing an item index (spawn rune)
//|
//| Remarks    : This function was created from the former AddRespawnNPC() and
//| 			 AddNPCxyz() that were 95% identical
//o---------------------------------------------------------------------------o
P_CHAR AddRespawnNPC(P_ITEM pi, int npcNum)
{
        //type 1 remember 
        VALIDATEPIR(pi, NULL);
        return AddNPC(INVALID, pi, npcNum, 0,0,0);
}

P_CHAR AddRespawnNPC(P_CHAR pc, int npcNum)
{
        VALIDATEPCR(pc, NULL);
        return AddNPC(pc->getSocket(), NULL, npcNum, 0,0,0);
}

//<Anthalir>
P_CHAR AddNPCxyz(NXWSOCKET s, int npcNum, Location where)
{
	return AddNPCxyz(s, npcNum, where.x, where.y, where.dispz);
}
//</Anthalir>

P_CHAR AddNPCxyz(NXWSOCKET s, int npcNum, int x1, int y1, signed char z1) //Morrolan - replacement for old npcs::AddNPCxyz(), fixes a LOT of problems.
{

	return AddNPC(s, NULL, npcNum, x1,y1,z1); 

}


//<Anthalir>
P_CHAR AddNPC(NXWSOCKET s, P_ITEM pi, int npcNum, Location where)
{
	return AddNPC(s, pi, npcNum, where.x, where.y, where.z);
}
//</Anthalir>

// Xan -> compatible with new style scripts!! :D
P_CHAR AddNPC(NXWSOCKET s, P_ITEM pi, int npcNum, UI16 x1, UI16 y1, SI08 z1)
{
        int tmp, z, lovalue, hivalue;
	P_ITEM pi_n;
	int storeval;
	int k=0, xos=0, yos=0, lb;
	char sect[512];
	char script1[1024], script2[1024];
	int haircolor;				//(we need this to remember the haircolor)
	haircolor=-1;
	short postype;				// determines how xyz of the new NPC are set, see below

	int fx1,fx2,fy1,fy2,fz1;
	cScpIterator* iter = NULL;
	fx1=fx2=fy1=fy2=fz1=0;

	if (x1 > 0 && y1 > 0)
		postype = 3;	// take position from parms
	else if ( s > INVALID && !ISVALIDPI(pi) )
		postype = 2;	// take position from socket's buffer
	else if ( s == INVALID && ISVALIDPI(pi) )
		postype = 1;	// take position from items[i]
	else
	{
		ErrOut("bad parms in call to AddNPC (socket [%d], item[%d], npcNum[%d], x1[%d], y1[%d]\n", 
			s, ((ISVALIDPI(pi))? pi->getSerial32() : INVALID), npcNum, x1, y1);
		return NULL;
	}

	P_ITEM mypack=NULL;
	pi_n=NULL;
	storeval=-1;
	ITEM buyRestockContainer 	= INVALID;
	ITEM buyNoRestockContainer= INVALID;
	ITEM sellContainer				= INVALID;
	//
	// First things first...lets find out what NPC# we should spawn
	//
	sprintf(sect, "SECTION NPC %i", npcNum);
	iter = Scripts::Npc->getNewIterator(sect);
	if (iter==NULL) return NULL;

	int loopexit=0;
	do
	{
		iter->parseLine(script1, script2);
		if (script1[0]!='}')
		{
			if (!(strcmp("NPCLIST", script1)))
			{
					npcNum=npcs::AddRandomNPC(s,script2,1);
					if (npcNum==-1)
					{
						safedelete(iter);
						return NULL;
					}
				break;	//got the NPC number to add stop reading
			}
		}
	} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	safedelete(iter);

	sprintf(sect, "SECTION NPC %i", npcNum);
	iter = Scripts::Npc->getNewIterator(sect);
	if (iter==NULL) return NULL;

	//
	// Now lets spawn him/her
	//

	P_CHAR pc=archive::getNewChar();
	if(!ISVALIDPC(pc))
	{
		safedelete(iter);
		return NULL;
	}

	pc->SetPriv(0x10);
	pc->npc=1;
	pc->att=1;
	pc->def=1;

	loopexit=0;

	amxVS.setServerMode();
	do
	{
		iter->parseLine(script1, script2);

		if ((script1[0]!='}')&&(script1[0]!='{')) 
		{
			switch(script1[0])
			{
			case '@':
				pc->loadEventFromScript(script1,script2);
				break;
			case 'A':
				if	( !strcmp( "ALCHEMY", script1 ) ) 		pc->baseskill[ALCHEMY] = getRangedValue(script2);
				else if ( !strcmp( "AMOUNT",  script1 ) )
				{
					if( ISVALIDPI( pi_n ) )
						pi_n->amount = str2num( script2 );
				}
				//
				// Old style user variables
				//
				else if	( !strcmp( "AMXFLAG0", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 0, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAG1", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 1, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAG2", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 2, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAG3", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 3, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAG4", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 4, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAG5", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 5, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAG6", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 6, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAG7", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 7, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAG8", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 8, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAG9", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 9, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAGA", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 10, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAGB", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 11, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAGC", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 12, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAGD", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 13, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAGE", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 14, str2num( script2 ) );
				else if ( !strcmp( "AMXFLAGF", script1 ) )		amxVS.insertVariable( pc->getSerial32(), 15, str2num( script2 ) );
				//
				// New style user variables
				//
				else if ( !strcmp( "AMXINT", script1 ) )
				{
					splitLine( script2, script1, script3 );
					amxVS.insertVariable( pc->getSerial32(), str2num( script1 ), str2num( script3 ) );
				}
				else if ( !strcmp( "AMXINTVEC", script1 ) )
				{
					splitLine( script2, script1, script3 );
					SI32 vectorId = str2num( script1 );
					SI32 vectorSize = str2num( script3 );
					amxVS.insertVariable( pc->getSerial32(), vectorId, vectorSize, 0 );
					SI32 vectorIndex = 0;
					do
					{
						iter->parseLine(script1, script2);
						switch( script1[0] )
						{
							case '[' :
							case ']' :
								break;
							default	 :
								if( vectorIndex < vectorSize )
								{
									amxVS.updateVariable( pc->getSerial32(), vectorId, vectorIndex, str2num( script1 ) );
									++vectorIndex;
								}
								break;
						}
					}
					while( script1[0] != ']' );
				}
				else if ( !strcmp( "AMXSTR", script1 ) )
				{
					splitLine( script2, script1, script3 );
					amxVS.insertVariable( pc->getSerial32(), str2num( script1 ), script3 );
				}
				else if ( !strcmp( "ANATOMY", script1 ) ) 		pc->baseskill[ANATOMY] = getRangedValue(script2);
				else if ( !strcmp( "ANIMALLORE", script1 ) )		pc->baseskill[ANIMALLORE] = getRangedValue(script2);
				else if ( !strcmp( "ARCHERY", script1 ) ) 		pc->baseskill[ARCHERY] = getRangedValue(script2);
				else if ( !strcmp( "ARMSLORE", script1 ) )		pc->baseskill[ARMSLORE] = getRangedValue(script2);
				break;
			case 'B':
				if	( !strcmp( "BACKPACK", script1 ) )
				{
					if (!ISVALIDPI(mypack))
					{
						pi_n=item::CreateFromScript( "$item_backpack" );
						if( ISVALIDPI(pi_n) )
						{
							pc->packitemserial=pi_n->getSerial32();
							pi_n->setPosition(0, 0, 0);
							pi_n->setContSerial(pc->getSerial32());
							mypack=pi_n;
						}
						else
							WarnOut("AddNPC: cannot spawn item 0x0E75\n");
						strcpy(script1, "DUMMY");
					}
				}
				else if ( !strcmp( "BEGGING", script1 ) ) 		pc->baseskill[BEGGING] = getRangedValue(script2);
				else if ( !strcmp( "BLACKSMITHING", script1 ) )		pc->baseskill[BLACKSMITHING] = getRangedValue(script2);
				else if ( !strcmp( "BOWCRAFT", script1 ) )		pc->baseskill[BOWCRAFT] = getRangedValue(script2);
				break;
			case 'C':
				if	( !strcmp( "COLOR", script1 ) )
				{
					if( ISVALIDPI(pi_n) )
					{
						pi_n->setColor( hex2num(script2) );
					}
				}
				else if ( !strcmp( "CAMPING", script1 ) )		pc->baseskill[CAMPING] = getRangedValue(script2);
				else if ( !strcmp( "CANTRAIN", script1 ) )		pc->cantrain=true;
				else if ( !strcmp( "CARVE", script1 ) )			pc->carve=str2num(script2);
				else if ( !strcmp( "CARPENTRY", script1 ) )		pc->baseskill[CARPENTRY] = getRangedValue(script2);
				else if ( !strcmp( "CARTOGRAPHY", script1 ) )		pc->baseskill[CARTOGRAPHY] = getRangedValue(script2);
				else if ( !strcmp( "COLORMATCHHAIR", script1 ) )
				{
					if (ISVALIDPI(pi_n) && haircolor!=-1)
					{
						pi_n->setColor( haircolor );
					}
				}
				else if ( !strcmp( "COLORLIST", script1 ) )
				{
					storeval=addrandomcolor(pc,script2);
					if (ISVALIDPI(pi_n))
					{
						pi_n->setColor( storeval );
					}
					strcpy(script1, "DUMMY");
				}
				else if ( !strcmp( "COOKING", script1 ) )		pc->baseskill[COOKING] = getRangedValue(script2);
				break;
			case 'D':
				if	(!strcmp( "DAMAGE", script1 ) || !strcmp( "ATT", script1 ) )
				{
					gettokennum(script2, 0);
					lovalue=str2num(gettokenstr);
					gettokennum(script2, 1);
					hivalue=str2num(gettokenstr);
					pc->lodamage = lovalue;
					pc->hidamage = lovalue;
					if(hivalue)
						pc->hidamage = hivalue;
				}
				else if ( !strcmp( "DAMAGETYPE", script1 ) )		pc->damagetype = static_cast<DamageType>(str2num(script2));
				else if ( !strcmp( "DEF", script1 ) )			pc->def = getRangedValue(script2);
				else if ( !strcmp( "DETECTINGHIDDEN", script1 ) )	pc->baseskill[DETECTINGHIDDEN] = getRangedValue(script2);
				else if ( !strcmp( "DEX", script1 ) || !strcmp( "DEXTERITY", script1 ) )
				{
					pc->dx  = getRangedValue(script2);
					pc->dx2 = pc->dx;
					pc->dx3 = pc->dx;
					pc->stm = pc->dx;
				}
				else if ( !strcmp( "DIRECTION", script1 ) )
				{
					if	( !strcmp( "NE", script2 ) )		pc->dir=1;
					else if ( !strcmp( "E", script2 ) )		pc->dir=2;
					else if ( !strcmp( "SE", script2 ) )		pc->dir=3;
					else if ( !strcmp( "S", script2 ) )		pc->dir=4;
					else if ( !strcmp( "SW", script2 ) )		pc->dir=5;
					else if ( !strcmp( "W", script2 ) )		pc->dir=6;
					else if ( !strcmp( "NW", script2 ) )		pc->dir=7;
					else if ( !strcmp( "N", script2 ) )		pc->dir=0;
				}
				else if (!strcmp("DOORUSE",script1)) pc->doorUse = str2num( script2 );
				break;
			case 'E':
				if	( !strcmp( "EMOTECOLOR", script1 ) )
				{
					pc->emotecolor = hex2num(script2);
				}
				else if ( !strcmp("ENTICEMENT",script1 ) )		pc->baseskill[ENTICEMENT] = getRangedValue(script2);
				else if ( !strcmp("EVALUATINGINTEL",script1 ) )		pc->baseskill[EVALUATINGINTEL] = getRangedValue(script2);
				break;
			case 'F':
				if	( !strcmp( "FAME", script1 ) )			pc->SetFame(str2num(script2));
				else if ( !strcmp( "FENCING", script1 ) )		pc->baseskill[FENCING] = getRangedValue(script2);
				else if ( !strcmp( "FISHING", script1 ) )		pc->baseskill[FISHING] = getRangedValue(script2);
				else if ( !strcmp( "FLEEAT", script1 ) )		pc->fleeat=str2num(script2);
				else if ( !strcmp( "FOLLOWSPEED",script1) )		pc->npcFollowSpeed = (float) atof( script2 );
				else if ( !strcmp( "FORENSICS", script1 ) )		pc->baseskill[FORENSICS] = getRangedValue(script2);
				else if ( !strcmp( "FX1", script1 ) )			fx1=str2num(script2);
				else if ( !strcmp( "FX2", script1 ) )			fx2=str2num(script2);
				else if ( !strcmp( "FY1", script1 ) )			fy1=str2num(script2);
				else if ( !strcmp( "FY2", script1 ) )			fy2=str2num(script2);
				else if ( !strcmp( "FZ1", script1 ) ) 			fz1=str2num(script2);
				break;
			case 'G':
				if	(!strcmp( "GOLD", script1 ) )
				{
					if (ISVALIDPI(mypack))
					{
						char lo[1024], hi[1024];
						splitLine( script2, lo, hi );
						int amt = RandomNum( str2num(lo), str2num(hi) );

						P_ITEM pi_sp = item::CreateFromScript( "$item_gold_coin", mypack, amt );
						if( ISVALIDPI( pi_sp ) )
						{
							pi_sp->priv|=0x01;
						}
						else
							WarnOut("AddNPC: cannot spawn item 0x0EED\n");
					} 
					else
						WarnOut("Bad NPC Script %d with problem no backpack for gold.\n", npcNum);
				}
				break;
			case 'H':
				if	( !strcmp( "HAIRCOLOR", script1 ) )
				{
					if (ISVALIDPI(pi_n))
					{
						haircolor=addrandomhaircolor(pc,script2);
						if (haircolor!=-1)
						{
							pi_n->setColor( haircolor );
						}
					}
					strcpy(script1, "DUMMY");
				}
				else if	( !strcmp( "HEALING", script1 ) )		pc->baseskill[HEALING] = getRangedValue(script2);
				else if ( !strcmp( "HERDING", script1 ) )		pc->baseskill[HERDING] = getRangedValue(script2);
				else if ( !strcmp( "HIDAMAGE", script1 ) )		pc->hidamage=str2num(script2);
				else if ( !strcmp( "HIDING", script1 ) )		pc->baseskill[HIDING] = getRangedValue(script2);
				else if ( !strcmp( "HOLYDAMAGED", script1 ) )		pc->holydamaged = true;
				break;
			case 'I':
				if	( !strcmp( "ID", script1 ) )
				{
					tmp=hex2num(script2);
					pc->SetBodyType(tmp);
					pc->SetOldBodyType(tmp);
				}
				else if ( !strcmp( "INSCRIPTION", script1 ) )		pc->baseskill[INSCRIPTION] = getRangedValue(script2);
				else if ( !strcmp( "INT", script1 ) || !strcmp( "INTELLIGENCE", script1 ) )
				{
					pc->in  = getRangedValue(script2);
					pc->in2 = pc->in;
					pc->in3 = pc->in;
					pc->mn  = pc->in;
				}
				else if ( !strcmp( "INVULNERABLE", script1 ) )		pc->MakeInvulnerable();
				else if ( !strcmp( "ITEM", script1 ) )
				{
					storeval=str2num(script2);
					pi_n=item::CreateScriptItem( INVALID, storeval, 0, pc );
					if (ISVALIDPI(pi_n))
					{
						if (pi_n->layer==0)
							WarnOut("Bad NPC Script %d with problem item %d executed!\n", npcNum, storeval);
					}
					strcpy(script1, "DUMMY");
				}
				else if ( !strcmp( "ITEMID", script1 ) )		pc->baseskill[ITEMID] = getRangedValue(script2);
				break;
			case 'K':
				if	( !strcmp( "KARMA", script1 ) )			pc->SetKarma(str2num(script2));
				break;
			case 'L':
				if	( !strcmp( "LIGHTDAMAGED", script1 ) )		pc->lightdamaged = true;
				else if ( !strcmp( "LOCKPICKING", script1 ) )		pc->baseskill[LOCKPICKING] = getRangedValue(script2);
				else if ( !strcmp( "LODAMAGE", script1 ) )		pc->lodamage=str2num(script2);
				else if ( !strcmp( "LOOT", script1 ) )
				{
					//
					// Sparhawk:	new Just In Time Loot handling
					//
					pc->lootVector.push_back( str2num( script2 ) );
					/*
					if (ISVALIDPI(mypack))
					{
						pi_n=npcs::AddRandomLoot(mypack, script2);
						strcpy(script1, "DUMMY");
					}
					else
						WarnOut("Bad NPC Script %d with problem no backpack for loot.\n", npcNum);
					*/
				}
				else if ( !strcmp( "LUMBERJACKING", script1 ) ) 	pc->baseskill[LUMBERJACKING] = getRangedValue(script2);
				break;
			case 'M':
				if	( !strcmp( "MACEFIGHTING", script1 ) )		pc->baseskill[MACEFIGHTING] = getRangedValue(script2);
				else if ( !strcmp( "MAGERY", script1 ) )		pc->baseskill[MAGERY] = getRangedValue(script2);
				else if ( !strcmp( "MAGICRESISTANCE", script1 ) )	pc->baseskill[MAGICRESISTANCE] = getRangedValue(script2);
				else if	( !strcmp( "MAGICSPHERE", script1 ) )		pc->magicsphere = str2num(script2);
				else if ( !strcmp( "MAGICLEVEL", script1 ) )
				{
					if (ServerScp::g_nUseNewNpcMagic!=0)
						pc->spattack=str2num(script2);
				}
				else if ( !strcmp( "MEDITATION", script1 ) )		pc->baseskill[MEDITATION] = getRangedValue(script2);
				else if ( !strcmp( "MINING", script1 ) )		pc->baseskill[MINING] = getRangedValue(script2);
				else if ( !strcmp( "MOVESPEED", script1) )		pc->npcMoveSpeed = (float) atof( script2 );
				else if ( !strcmp( "MUSICIANSHIP", script1 ) )		pc->baseskill[MUSICIANSHIP] = getRangedValue(script2);
				break;
			case 'N':
				if	( !strcmp( "NAME", script1 ) )
				{
					pc->setCurrentName( script2 );
					pc->setRealName( script2 );
				}
				else if ( !strcmp( "NAMELIST", script1 ) )
				{
					setrandomname(pc,script2);
					pc->setRealName( pc->getCurrentNameC() );
					strcpy(script1, "DUMMY");
				}
				else if ( !strcmp( "NOTRAIN", script1 ) )		pc->cantrain=false;
				else if ( !strcmp( "NPCAI",script1 ) )			pc->npcaitype=str2num(script2);
				else if ( !strcmp( "NPCWANDER",script1) )		pc->npcWander=str2num(script2);
				else if ( !strcmp( "NXWFLAG0", script1 ) )		pc->nxwflags[0] = str2num(script2);
				else if ( !strcmp( "NXWFLAG1", script1 ) )		pc->nxwflags[1] = str2num(script2);
				else if ( !strcmp( "NXWFLAG2", script1 ) )		pc->nxwflags[2] = str2num(script2);
				else if ( !strcmp( "NXWFLAG3", script1 ) )		pc->nxwflags[3] = str2num(script2);
				break;
			case 'O':
				if	( !strcmp( "ONHORSE", script1 ) )		pc->setOnHorse();
				break;
			case 'P':
				if	( !strcmp( "PACKITEM", script1) )
				{
					if (ISVALIDPI(mypack))
					{
						storeval=str2num(script2);
						pi_n=item::CreateFromScript( storeval, mypack);
						strcpy(script1, "DUMMY"); // Prevents unexpected matchups...
					}
					else
						WarnOut("Bad NPC Script %d with problem no backpack for packitem.\n", npcNum);
				}
				else if ( !strcmp( "PARRYING", script1) )		pc->baseskill[PARRYING] = getRangedValue(script2);
				else if ( !strcmp( "PEACEMAKING", script1 ) )		pc->baseskill[PEACEMAKING] = getRangedValue(script2);
				else if ( !strcmp( "POISON", script1) )			pc->poison=str2num(script2);
				else if ( !strcmp( "POISONING", script1 ) )		pc->baseskill[POISONING] = getRangedValue(script2);
				else if ( !strcmp( "PRIV1", script1 ) )			pc->SetPriv(str2num(script2));
				else if ( !strcmp( "PRIV2", script1) )			pc->priv2=str2num(script2);
				else if ( !strcmp( "PROVOCATION", script1 ) )		pc->baseskill[PROVOCATION] = getRangedValue(script2);
				break;
			case 'R':
				if	( !strcmp( "RACE", script1 ) )			pc->race=str2num(script2);
				else if ( !strcmp( "REATTACKAT", script1 ) )		pc->reattackat=str2num(script2);
				else if ( !strcmp( "REGEN_HP", script1 ) )	{ UI32 v=str2num(script2);	pc->setRegenRate( STAT_HP, v, VAR_REAL );	pc->setRegenRate( STAT_HP, v, VAR_EFF ); }
				else if ( !strcmp( "REGEN_ST", script1 ) )	{ UI32 v=str2num(script2);	pc->setRegenRate( STAT_STAMINA, v, VAR_REAL );	pc->setRegenRate( STAT_STAMINA, v, VAR_EFF ); }
				else if ( !strcmp( "REGEN_MN", script1 ) )	{ UI32 v=str2num(script2);	pc->setRegenRate( STAT_MANA, v, VAR_REAL );	pc->setRegenRate( STAT_MANA, v, VAR_EFF ); }
				else if ( !strcmp( "REMOVETRAPS", script1 ) )		pc->baseskill[REMOVETRAPS] = getRangedValue(script2);
				else if ( !strcmp( "RESISTS", script1 ) )
				{
					int params[2];
					fillIntArray(script2, params, 2, 0, 10);
					if (params[0] < MAX_RESISTANCE_INDEX)
						pc->resists[params[0]] = params[1];
				}
				else if ( !strcmp( "RSHOPITEM", script1 ) )
				{
					if ( buyRestockContainer == INVALID )
					{
						
						P_ITEM pi_z=pc->GetItemOnLayer(LAYER_TRADE_RESTOCK);
						if (ISVALIDPI(pi_z))
						{
							buyRestockContainer = pi_z->getSerial32();
						}
					}
					if ( buyRestockContainer != INVALID )
					{
						char itmnum[1024], amount[1024];
						splitLine( script2, itmnum, amount );
						
						int amt=str2num( amount );
						if( amt==0 )
							amt=server_data.defaultSelledItem;
						
						pi_n=item::CreateFromScript( str2num(itmnum), MAKE_ITEM_REF( buyRestockContainer ), amt );
						if (ISVALIDPI(pi_n))
						{
							if (pi_n->getSecondaryNameC() && (strcmp(pi_n->getSecondaryNameC(),"#")))
								pi_n->setCurrentName(pi_n->getSecondaryNameC()); // Item identified! -- by Magius(CHE) 				}
						}
						strcpy(script1, "DUMMY");
					}
					else
						WarnOut("Bad NPC Script %d with problem no buyRestockContainer for item %s.\n", npcNum, script2);
				}
				break;
			case 'S':
				if	( !strcmp( "SAYCOLOR", script1 ) )
				{
					pc->saycolor = hex2num(script2);
				}
				else if ( !strcmp( "SELLITEM", script1 ) )
				{
					if ( sellContainer == INVALID )
					{
						P_ITEM pi_z=pc->GetItemOnLayer(0x1C);
						{
							sellContainer = DEREF_P_ITEM(pi_z);
						}
					}
					if ( sellContainer != INVALID )
					{
						storeval=str2num(script2);
						pi_n=item::CreateFromScript( storeval, MAKE_ITEM_REF(sellContainer));
						if (ISVALIDPI(pi_n))
						{
							pi_n->value=pi_n->value/2;
							if (pi_n->getSecondaryNameC() && (strncmp(pi_n->getSecondaryNameC(),"#", 1)))
								pi_n->setCurrentName(pi_n->getSecondaryNameC()); // Item identified! -- by Magius(CHE) 				}
						}
						strcpy(script1, "DUMMY"); // Prevents unexpected matchups...
					}
					else
						WarnOut("Bad NPC Script %d with problem no sellContainer for item %s.\n", npcNum, script2);
				}
				else if ( !strcmp( "SHOPITEM", script1 ) )
				{
					if ( buyNoRestockContainer == INVALID )
					{
						P_ITEM pi_z=pc->GetItemOnLayer(0x1B);
						if (ISVALIDPI(pi_z))
						{
							buyNoRestockContainer = DEREF_P_ITEM(pi_z);
						}
					}
					if ( buyNoRestockContainer != INVALID )
					{
						storeval=str2num(script2);
						pi_n=item::CreateFromScript( storeval, MAKE_ITEM_REF(buyNoRestockContainer));
						if (ISVALIDPI(pi_n))
						{
							if (pi_n->getSecondaryNameC() && (strcmp(pi_n->getSecondaryNameC(),"#")))
								pi_n->setCurrentName(pi_n->getSecondaryNameC()); // Item identified! -- by Magius(CHE) 				}
						}
						strcpy(script1, "DUMMY"); // Prevents unexpected matchups...
					}
					else
						WarnOut("Bad NPC Script %d with problem no buyNoRestockContainer for item %s.\n", npcNum, script2);
				}
				else if ( !strcmp( "SHOPKEEPER", script1 ) )		Commands::MakeShop(DEREF_P_CHAR(pc));
				else if ( !strcmp( "SKILL", script1 ) )
				{
					gettokennum(script2, 0);
					z=str2num(gettokenstr);
					gettokennum(script2, 1);
					pc->baseskill[z]=str2num(gettokenstr);
				}
				else if ( !strcmp( "SKIN", script1 ) )
				{
					tmp=hex2num(script2);
					pc->setSkinColor(tmp);
					pc->setOldSkinColor(tmp);
				}
				else if ( !strcmp( "SKINLIST",script1 ) )
				{
					storeval=addrandomcolor(pc,script2);
					pc->setSkinColor(storeval);
					pc->setOldSkinColor(storeval);
					strcpy(script1, "DUMMY"); // To prevent accidental exit of loop.
				}
				else if ( !strcmp( "SNOOPING",script1 ) )		pc->baseskill[SNOOPING] = getRangedValue(script2);
				else if ( !strcmp( "SPADELAY",script1 ) )		pc->spadelay=str2num(script2);
				else if ( !strcmp( "SPATTACK", script1 ) )
				{
					if (ServerScp::g_nUseNewNpcMagic==0) pc->spattack=str2num(script2);
				}
				else if ( !strcmp( "SPEECH", script1 ) )		pc->speech=str2num(script2);
				else if ( !strcmp( "SPIRITSPEAK",script1 ) )		pc->baseskill[SPIRITSPEAK] = getRangedValue(script2);
				else if ( !strcmp( "SPLIT", script1 ) )			pc->split=str2num(script2);
				else if ( !strcmp( "SPLITCHANCE", script1 ) )		pc->splitchnc=str2num(script2);
				else if ( !strcmp( "STABLEMASTER",script1 ) )		pc->npc_type=1;
				else if ( !strcmp( "STEALING",script1 ) )		pc->baseskill[STEALING] = getRangedValue(script2);
				else if ( !strcmp( "STEALTH",script1 ) )		pc->baseskill[STEALTH] = getRangedValue(script2);
				else if ( !strcmp( "STR", script1 ) || !strcmp( "STRENGTH",script1 ) )
				{
					pc->setStrength( getRangedValue(script2) );
					pc->st2 = pc->getStrength();
					pc->st3 = pc->getStrength(); //Luxor
					pc->hp  = pc->getStrength();
				}
				else if ( !strcmp( "SWORDSMANSHIP",script1 ) )		pc->baseskill[SWORDSMANSHIP] = getRangedValue(script2);
				break;
			case 'T':
				if	( !strcmp( "TACTICS", script1 ) )		pc->baseskill[TACTICS] = getRangedValue(script2);
				else if ( !strcmp( "TAILORING", script1 ) )		pc->baseskill[TAILORING] = getRangedValue(script2);
				else if ( !strcmp( "TAMING", script1 ) )		pc->baseskill[TAMING] = getRangedValue(script2);
				else if ( !strcmp( "TASTEID", script1 ) )		pc->baseskill[TASTEID] = getRangedValue(script2);
				else if ( !strcmp( "TINKERING", script1 ) )		pc->baseskill[TINKERING] = getRangedValue(script2);
				else if ( !strcmp( "TITLE", script1 ) )			pc->title = script2;
				else if ( !strcmp( "TOTAME", script1 ) || !strcmp("TAMING", script1 ) )
											pc->taming=str2num(script2);
				else if ( !strcmp( "TRACKING", script1 ) )		pc->baseskill[TRACKING] = getRangedValue(script2);
				else if ( !strcmp( "TRIGGER", script1 ) )		pc->trigger=str2num(script2);
				else if ( !strcmp( "TRIGWORD", script1 ) )		pc->trigword=script2;
				break;
			case 'V':
				if	( !strcmp( "VALUE", script1 ) )
				{
					if ( ISVALIDPI( pi_n ) )
						pi_n->value=(str2num(script2));
				}
				else if ( !strcmp( "VETERINARY", script1 ) )		pc->baseskill[VETERINARY] = getRangedValue(script2);
				else if ( !strcmp( "VULNERABLE", script1 ) )		pc->MakeVulnerable();
				break;
			case 'W':
				if	( !strcmp( "WATERWALK", script1 ) )		pc->nxwflags[0]|=NCF0_WATERWALK;
				else if ( !strcmp( "WRESTLING", script1 ) )		pc->baseskill[WRESTLING] = getRangedValue(script2);
				break;
			default:
				ErrOut("Switch fallout in npcs.cpp AddNPC( %d ), invalid script param [ %s ]\n", npcNum, script1);
			}
		}
   	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	safedelete(iter);

	amxVS.setUserMode();

	pc->setScriptID( npcNum );


   // Now that we have created the NPC, lets place him
   switch (postype)
   {
   case 1:					// take position from (spawning) item
	   {
	   /*Zippy's Code chages for area spawns --> (Type 69) xos and yos (X OffSet, Y OffSet)
	   are used to find a random number that is then added to the spawner's x and y (Using
	   the spawner's z) and then place the NPC anywhere in a square around the spawner.
	   This square is random anywhere from -10 to +10 from the spawner's location (for x and
	   y) If the place chosen is not a valid position (the NPC can't walk there) then a new
	   place will be chosen, if a valid place cannot be found in a certain # of tries (50),
		   the NPC will be placed directly on the spawner and the server op will be warned. */

		   P_ITEM pi_i=pi;
		   if (ISVALIDPI(pi_i) && ((pi_i->type==69 || pi_i->type==125)&& pi_i->isInWorld()) )
		   {
			   if (pi_i->more3==0) pi_i->more3=10;
			   if (pi_i->more4==0) pi_i->more4=10;
			   //signed char z, ztemp, found;

			   k=0;

			   do
			   {
				   if (k>=50) //this CAN be a bit laggy. adjust as nessicary
				   {
					   WarnOut("Problem area spawner found at [%i,%i,%i]. NPC placed at default location.\n",pi_i->getPosition().x, pi_i->getPosition().y, pi_i->getPosition().z);
					   xos=0;
					   yos=0;
					   break;
				   }
				   xos=RandomNum(-pi_i->more3,pi_i->more3);
				   yos=RandomNum(-pi_i->more4,pi_i->more4);
				   //ConOut("AddNPC Spawning at Offset %i,%i (%i,%i,%i) [-%i,%i <-> -%i,%i]. [Loop #: %i]\n",xos,yos,items[i].x+xos,items[i].y+yos,items[i].z,items[i].more3,items[i].more3,items[i].more4,items[i].more4,k); /** lord binary, changed %s to %i, crash when uncommented ! **/
				   k++;

				   if ((pi_i->getPosition().x+xos<1) || (pi_i->getPosition().y+yos<1))
				   	lb=0; /* lord binary, fixes crash when calling npcvalid with negative coordiantes */
				   else
				   	lb=validNPCMove(pi_i->getPosition().x+xos,pi_i->getPosition().y+yos,pi_i->getPosition().z,pc);

				   //Bug fix Monsters spawning on water:
				   MapStaticIterator msi(pi_i->getPosition().x + xos, pi_i->getPosition().y + yos);
				   staticrecord *stat;
				   loopexit=0;
				   while ( ((stat = msi.Next())!=NULL) && (++loopexit < MAXLOOPS) )
				   {
					   tile_st tile;
					   msi.GetTile(&tile);
					   if(!(strcmp((char *) tile.name, "water")))//Water
					   {//Don't spawn on water tiles... Just add other stuff here you don't want spawned on.
						   lb=0;
					   }
				   }
			   } while (!lb);
		   } // end Zippy's changes (exept for all the +xos and +yos around here....)

		   /*
		   pc->x= items[i].x + xos;
		   pc->y= items[i].y + yos;
		   pc->dispz= chars[c].z= items[i].z;
		   */
		   pc->MoveTo( pi_i->getPosition() );

		} // end of if !triggerx
		break;
	case 2: // take position from Socket
		if (s > INVALID)
		{
			/*
			pc->x=(buffer[s][11]<<8)+buffer[s][12];
			pc->y=(buffer[s][13]<<8)+buffer[s][14];
			pc->dispz=chars[c].z=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]);
			*/
			pc->MoveTo( (buffer[s][11]<<8)+buffer[s][12], (buffer[s][13]<<8)+buffer[s][14],
							buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]) );
		}
		break;
	case 3: // take position from Parms
		/*
		pc->x=x1;
		pc->y=y1;
		pc->dispz=pc->z=z1;
		*/
		pc->MoveTo( x1, y1, z1 );
		break;
	} // no default coz we tested on entry to function

	Location charpos= pc->getPosition();
	// now we have a position, let's set the borders
	switch (pc->npcWander)
	{
	case 2: 	// circle
		pc->fx1= charpos.x;
		pc->fy1= charpos.y;
		pc->fz1= charpos.z;
		pc->fx2= (fx2>=0) ? fx2 : 2;	// radius; if not given from script,default=2
		break;
	case 3: 	// box
		if (fx1 || fx2 || fy1 || fy2)	// any offset for rectangle given from script ?
		{
			pc->fx1= charpos.x+fx1;
			pc->fx2= charpos.x+fx2;
			pc->fy1= charpos.y+fy1;
			pc->fy2= charpos.y+fy2;
			pc->fz1= -1;			// irrelevant for box
		}
		break;
	default:
		pc->fx1=fx1;
		pc->fx2=fx2;
		pc->fy1=fy1;
		pc->fy2=fy2;
		pc->fz1=fz1;
		break;
	}

	pc->region=static_cast<unsigned char>(calcRegionFromXY( pc->getPosition() ));

	//Now find real 'skill' based on 'baseskill' (stat modifiers)
	for(z=0;z<TRUESKILLS;z++)
	{
		Skills::updateSkillLevel(pc,z);
	}

	if (donpcupdate==0)
	{
		pc->teleport();
	}

   // Dupois - Added April 4, 1999
   // After the NPC has been fully initialized, then post the message (if its a quest spawner) type==125
   if (postype==1) // lb crashfix
   {
		P_ITEM pi_i=pi;
		if ( ISVALIDPI(pi_i) && pi_i->type == 125 )
		{
			MsgBoards::MsgBoardQuestEscortCreate( DEREF_P_CHAR(pc) );
		}
   }
   // End - Dupois

   //Char mapRegions
   regions::add(pc);
   safedelete(iter);
   pc->teleport();
   return pc;

} 

P_CHAR addNpc(int npcNum, int x, int y, int z) {
	return AddNPC(INVALID, NULL, npcNum, x, y, z);
}

P_CHAR SpawnRandomMonster(P_CHAR pc, char* cList, char* cNpcID)
{


    char sect[512];
    int i=0,item[256]={0};
    char script1[1024];
    cScpIterator* iter = NULL;

    sprintf(sect, "SECTION %s %s", cList, cNpcID);
    iter = Scripts::Necro->getNewIterator(sect);
    if (iter==NULL) return NULL;

    int loopexit=0;
    do
    {
        strcpy(script1, iter->getEntry()->getFullLine().c_str());       
        if ((script1[0]!='}')&&(script1[0]!='{'))
        {
            item[i]=str2num(script1);
            i++;
        }
    }
    while(script1[0]!='}' && (++loopexit < MAXLOOPS) );
 
    safedelete(iter);

    if(i>0)
    {
        i=rand()%(i);
        if(item[i]!=INVALID)
        {
            return npcs::AddRespawnNPC(pc,item[i]);
        }
    }
    return NULL;

}



} // namespace
