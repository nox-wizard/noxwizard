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

static int nextbestskill(P_CHAR pc, int bstskll)  // Which skill is the second highest
{

	VALIDATEPCR(pc,0);
	
	int i, a = 0, b = 0;

	for (i = 0; i < TRUESKILLS; i++)
	{
		if (pc->baseskill[i] > b && pc->baseskill[i] < pc->baseskill[bstskll] && bstskll != i )
		{
			a = i;
			b = pc->baseskill[i];
		}
		if ( pc->baseskill[i] == pc->baseskill[bstskll] && bstskll != i)
		{
			a = i;
			b = pc->baseskill[i];
		}
	}
	return a;

}

void newbieitems(P_CHAR pc)
{

	VALIDATEPC(pc);
	
	NXWCLIENT ps=pc->getClient();
	if(ps==NULL)
		return;

	NXWSOCKET s=ps->toInt();
	
	int first, second, third, storeval, itemaddperskill, loopexit = 0;
	char sect[512];
	char whichsect[105];
	cScpIterator* iter = NULL;

	first = bestskill(pc);
	second = nextbestskill(pc, first);
	third = nextbestskill(pc, second);
	if (pc->baseskill[third] < 190)
		third = 46;

	for (itemaddperskill = 1; itemaddperskill <= 5; itemaddperskill++)
	{
		switch (itemaddperskill)
		{
			// first of all the general section with the backpack, else where we put items?
			case 1: strcpy(whichsect, "SECTION ALLNEWBIES");		break;
			case 2:
				if ( (pc->GetBodyType() == BODY_MALE) && (pc->GetOldBodyType() == BODY_MALE) )
					strcpy(whichsect, "SECTION MALENEWBIES");
				else
					strcpy(whichsect, "SECTION FEMALENEWBIES");
				break;
			case 3: sprintf(whichsect, "SECTION BESTSKILL %i", first);	break;
			case 4: sprintf(whichsect, "SECTION BESTSKILL %i", second);	break;
			case 5: sprintf(whichsect, "SECTION BESTSKILL %i", third);	break;
			default:
				ErrOut("Switch fallout. newbie.cpp, newbieitems()/n"); // Morrolan
		}

		sprintf(sect, whichsect);
		char script1[1000], script2[1000];
		safedelete(iter);
		iter = Scripts::Newbie->getNewIterator(sect);
		if (iter==NULL) return;

		do
		{
			iter->parseLine(script1,script2);

			if (script1[0] == '@') pc->loadEventFromScript(script1, script2); 	// Sparhawk: Huh loading character events 
												// from newbie item scripts????

			if (script1[0] != '}')
			{
				if (!(strcmp("PACKITEM", script1)))
				{
					storeval = str2num(script2);
					P_ITEM pi_n = item::SpawnItemBackpack2(s, storeval, 0); // Tauriel 11-24-98
					if (ISVALIDPI(pi_n)) {
						pi_n->priv |= 0x02; // Mark as a newbie item
						if( strcmp( script3, "" ) ) //ndEndy defined amount
							pi_n->amount=str2num( script3 );
					}
					strcpy(script1, "DUMMY");
				}
				else if (!strcmp("BANKITEM", script1))
				{
					storeval = str2num(script2);
					P_ITEM pi = item::SpawnItemBank(DEREF_P_CHAR(pc), storeval); // Tauriel 11-24-98
					if (ISVALIDPI(pi)) {
						pi->priv |= 0x02; // Mark as a newbie item
						if( strcmp( script3, "" ) ) //ndEndy defined amount
							pi->amount=str2num( script3 );
					}
					strcpy(script1, "DUMMY");
				}
				else if (!strcmp("EQUIPITEM", script1))
				{
					P_ITEM pi = item::CreateFromScript( script2 );
					if (ISVALIDPI(pi))
					{
						pi->priv |= 0x02; // Mark as a newbie item
						pi->setCont(pc);
						storeval = pi->getScriptID();
					}
					strcpy(script1, "DUMMY");
				}
			}
		}
		while ((script1[0] != '}') &&(++loopexit < MAXLOOPS));
	
		safedelete(iter);
	}
	
	// Give the character some gold
	if ( goldamount > 0 )
	{
		P_ITEM pi = item::SpawnItemBackpack2(s, 2000, 0);	// gold coin
		if(ISVALIDPI(pi))
			pi->setAmount(goldamount);
	}


}



