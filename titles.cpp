  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "titles.h"
#include "chars.h"
#include "items.h"
#include "inlines.h"
#include "scripts.h"


int bestskill(P_CHAR p) // Which skill is the highest for character p
{

	VALIDATEPCR(p, 0);
	int i,a=0,b=0;

	for (i=0;i<TRUESKILLS;i++)
		if (p->baseskill[i]>b)
		{
			a=i;
			b=p->baseskill[i];
		}
	return a;
}


void loadcustomtitle() // for custom titles
{
	int titlecount=0;
	char sect[512];
    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];

	for (int a=0; a<ALLSKILLS; a++)
	{
		title[a].fame[0] = 0;
		title[a].other[0] = 0;
		title[a].prowess[0] = 0;
		title[a].skill[0] = 0;
	}

	strcpy(sect,"SECTION SKILL");
    iter = Scripts::Titles->getNewIterator(sect);
	if (iter==NULL) return;

	int loopexit=0;
	do
	{
		iter->parseLine(script1, script2);
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			if ( !strcmp(script1, "TITLE") ) {
				strcpy(title[titlecount].skill,script2);
				titlecount++;
			} else if ( !strcmp(script1, "SKIP") ) {
				strcpy(title[titlecount].skill," ");
				titlecount++;
			}
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	safedelete(iter);

	script1[0]=0;
	titlecount=0;

	strcpy(sect,"SECTION PROWESS");
    iter = Scripts::Titles->getNewIterator(sect);
	if (iter==NULL) return;

	loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			if ( !strcmp(script1, "NONE") ) ;
			else strcpy(title[titlecount].prowess,script1);
			titlecount++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

	safedelete(iter);

	script1[0]=0;
	titlecount=0;
	strcpy(sect,"SECTION FAME");

	iter = Scripts::Titles->getNewIterator(sect);
	if (iter==NULL) return;

	loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			if ( !strcmp(script1, "NONE") ) ;
			else strcpy(title[titlecount].fame, script1);

			if (titlecount==23)
			{
				title[titlecount].fame[0] = '\0';
				strcpy(title[++titlecount].fame, script1);
			}

			titlecount++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	safedelete(iter);

	script1[0]=0;
	titlecount=0;
	strcpy(sect,"SECTION OTHER");

	iter = Scripts::Titles->getNewIterator(sect);
	if (iter==NULL) return;

	loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			if ( !strcmp(script1, "NONE") ) ;
			else strcpy(title[titlecount].other,script1);
			titlecount++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	safedelete(iter);

}

char *title1(P_CHAR pc) // Paperdoll title for character p (1)
{
	VALIDATEPCR(pc, 0);
	int titlenum = 0;
	int x = pc->baseskill[bestskill(pc)];

	//if (x>=1000) titlenum=10;
	//else if (x>=960) titlenum=9;
	if (x>=1000) titlenum=8;
	else if (x>=900) titlenum=7;
	else if (x>=800) titlenum=6;
	else if (x>=700) titlenum=5;
	else if (x>=600) titlenum=4;
	else if (x>=500) titlenum=3;
	else if (x>=400) titlenum=2;
	else if (x>=300) titlenum=1;

	strcpy(prowesstitle,title[titlenum].prowess);
	return prowesstitle;
}

char *title2(P_CHAR pc) // Paperdoll title for character p (2)
{
	VALIDATEPCR(pc, 0);

	int titlenum=0;

	int x=bestskill(pc);
	titlenum=x+1;

	strcpy(skilltitle,title[titlenum].skill);

	return skilltitle;
}

char *title3(P_CHAR pc) // Paperdoll title for character p (3)
{
	VALIDATEPCR(pc, 0);
	char thetitle[50];
	int titlenum=0;
	int k;
	unsigned int f;

	k= pc->GetKarma();
	f= pc->GetFame();
	thetitle[0] = 0;

	if (k>=10000)
	{
		titlenum=3;
		if (f>=5000) titlenum=0;
		else if (f>=2500) titlenum=1;
		else if (f>=1250) titlenum=2;
	}
	else if ((5000<=k)&&(k<9999))
	{
		titlenum=7;
		if (f>=5000) titlenum=4;
		else if (f>=2500) titlenum=5;
		else if (f>=1250) titlenum=6;
	}
	else if ((2500<=k)&&(k<5000))
	{
		titlenum=11;
		if (f>=5000) titlenum=8;
		else if (f>=2500) titlenum=9;
		else if (f>=1250) titlenum=10;
	}
	else if ((1250<=k)&&(k<2500))
	{
		titlenum=15;
		if (f>=5000) titlenum=12;
		else if (f>=2500) titlenum=13;
		else if (f>=1250) titlenum=14;
	}
	else if ((625<=k)&&(k<1250))
	{
		titlenum=19;
		if (f>=5000) titlenum=16;
		else if (f>=1000) titlenum=17;
		else if (f>=500) titlenum=18;
	}
	else if ((-635<k)&&(k<625))
	{
		titlenum=23;
		if (f>=5000) titlenum=20;
		else if (f>=2500) titlenum=21;
		else if (f>=1250) titlenum=22;
	}
	else if ((-1250<k)&&(k<=-625))
	{
		titlenum=24;
		if (f>=10000) titlenum=28;
		else if (f>=5000) titlenum=27;
		else if (f>=2500) titlenum=26;
		else if (f>=1250) titlenum=25;
	}
	else if ((-2500<k)&&(k<=-1250))
	{
		titlenum=29;
		if (f>=5000) titlenum=32;
		else if (f>=2500) titlenum=31;
		else if (f>=1250) titlenum=30;
	}
	else if ((-5000<k)&&(k<=-2500))
	{
		titlenum=33;
		if (f>=10000) titlenum=37;
		else if (f>=5000) titlenum=36;
		else if (f>=2500) titlenum=35;
		else if (f>=1250) titlenum=34;
	}
	else if ((-10000<k)&&(k<=-5000))
	{
		titlenum=38;
		if (f>=5000) titlenum=41;
		else if (f>=2500) titlenum=40;
		else if (f>=1250) titlenum=39;
	}
	else if (k<=-10000)
	{
		titlenum=42;
		if (f>=5000) titlenum=45;
		else if (f>=2500) titlenum=44;
		else if (f>=1250) titlenum=43;
	}
	sprintf(thetitle,"%s ",title[titlenum].fame);
	if (titlenum==24) thetitle [0] = 0;

	if (f>=10000) // Morollans bugfix for repsys
	{
		if (pc->kills >= (unsigned)repsys.maxkills)
		{
			if (pc->getId()==BODY_FEMALE) strcpy(fametitle,TRANSLATE("The Murderous Lady "));//Morrolan rep
			else strcpy(fametitle,TRANSLATE("The Murderer Lord "));
		}
		else if (pc->getId()==BODY_FEMALE) sprintf(fametitle,TRANSLATE("The %sLady "),thetitle);
		else sprintf(fametitle,TRANSLATE("The %sLord "),thetitle);
	}
	else
	{
		if (pc->kills >= (unsigned)repsys.maxkills)
		{
			strcpy(fametitle,TRANSLATE("The Murderer ")); //Morrolan rep
		}
		else if (!(strcmp(thetitle," ")==0)) sprintf(fametitle,TRANSLATE("The %s"),thetitle);
		else fametitle[0] = 0;
	}
	return fametitle;
}


char *complete_title(P_CHAR pc) // generates the ENTIRE title plus criminal stuff
{
	VALIDATEPCR(pc, 0);
	char tempstr[1024];
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	if (pc->account==0 && pc->IsGM()) // Ripper..special titles for admins :)
	{
		sprintf(tempstr, "%s %s", pc->getCurrentNameC(), pc->title.c_str());
	}
	else
	if (pc->IsGM() && pc->account!=0)
	{//GM.
		sprintf(tempstr, "%s %s", pc->getCurrentNameC(), pc->title.c_str());
	}
	// ripper ..rep stuff
	else if ((pc->IsCriminal()) && (!(pc->dead) && (pc->kills<4)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[0].other, pc->getCurrentNameC(), pc->title.c_str(), title1(pc), title2(pc));
	}
	else if ((pc->kills>=5) && (!(pc->dead) && (pc->kills<10)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[1].other, pc->getCurrentNameC(), pc->title.c_str(), title1(pc), title2(pc));
	}
	else if ((pc->kills>=10) && (!(pc->dead) && (pc->kills<20)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[2].other, pc->getCurrentNameC(), pc->title.c_str(), title1(pc), title2(pc));
	}
	else if ((pc->kills>=20) && (!(pc->dead) && (pc->kills<50)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[3].other, pc->getCurrentNameC(), pc->title.c_str(), title1(pc), title2(pc));
	}
	else if ((pc->kills>=50) && (!(pc->dead) && (pc->kills<100)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[4].other, pc->getCurrentNameC(), pc->title.c_str(), title1(pc), title2(pc));
	}
	else if ((pc->kills>=100) && (!(pc->dead)))
	{
		sprintf(tempstr, "%s %s, %s%s %s", title[5].other, pc->getCurrentNameC(), pc->title.c_str(), title1(pc), title2(pc));
	} // end of rep stuff
	else
	{//Player.
		sprintf(tempstr, "%s%s", title3(pc), pc->getCurrentNameC());		//Repuation + Name
		{//NoTownTitle
			strcpy(temp,tempstr);
			if (pc->title.length()>0)
			{//Titled & Skill
				sprintf(tempstr, "%s %s, %s %s", temp, pc->title.c_str(), title1(pc), title2(pc));
			}
			else
			{//Just skilled
				sprintf(tempstr, "%s, %s %s", temp, title1(pc), title2(pc));
			}
		}
	}

	strcpy(completetitle, tempstr);
	return completetitle;

}
