  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "network.h"
#include "calendar.h"
#include "set.h"
#include "chars.h"
#include "globals.h"


namespace Calendar {

#define HDT_NONE -3
#define HDT_SHOPCLOSED -4
#define HDT_HALFPRICE -5
#define HDT_DOUBLEPRICE -6
//other values are amxcbackz

#define MAXMONTHS 32
#define MAXWEEKDAY 32



int g_nWeekday = 1, g_nDay = 1, g_nMonth = 1, g_nYear = 1, g_nHour = 0, g_nMinute = 0;
int g_nMaxWeekday = 7, g_nMaxMonth = 12;
int g_nCurDawnHour = 7, g_nCurDawnMin = 30, g_nCurSunsetHour = 19, g_nCurSunsetMin = 30;
float g_fCurSnowMod = 1.0f, g_fCurRainMod = 1.0f, g_fCurDryMod = 1.0f;
int g_nWinterOverride = 0;

bool g_bInitOk = false;

cMonth g_Months[MAXMONTHS];
cWeekday g_Weekday[MAXWEEKDAY];


/*!
\brief returns the number of days in this month
\author Xanathar
\since 0.52a
*/
int cMonth::getDays()
{
	if (m_nCompensation<=0) return m_nDays;
	if ((g_nYear%m_nCompensation)==0) return m_nDays+1;
	return m_nDays;
}


/*!
\brief check if a charachter is in string
\return bool
\author Xanathar
\param c the char to be found
\param str the string to search in
\since 0.52a
*/
static bool chrInStr(char c, char *str)
{
	int i, ln = strlen(str);
	for (i=0; i<ln; i++) if (str[i]==c) return true;
	return false;
}


/*!
\brief sets a season globally
\author Xanathar
\param nSeason season number
\since 0.52a
*/
void setSeason(int nSeason)
{

	::season = nSeason;

	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if(ps!=NULL)
			commitSeason(ps->currChar());
	}
}


/*!
\brief commits season changes to a given player
\author Xanathar
\param pc poniter to char which will rcv season changes
\since 0.52a
*/
void commitSeason(P_CHAR pc)
{
	VALIDATEPC(pc);
	NXWSOCKET s = pc->getSocket();

	UI08 setseason[3]={ 0xBC, 0x00, 0x01 };
	
	if (region[pc->region].forcedseason>=0) 
		setseason[1] = region[pc->region].forcedseason;
	else {
		setseason[1] = ::season;
		if (::season==3) {
			setseason[1] = g_nWinterOverride;
		}
	}
	Xsend(s,setseason,3);
//AoS/	Network->FlushBuffer(s);
}



/*!
\brief advances time of a minute. returns true if date has changed
\return bool
\author Xanathar
\since 0.52a
*/
bool advanceMinute()
{
	bool bDateChanged = false;
	g_nMinute++;

	if (g_nMinute>=60) 
	{
		g_nMinute = 0;
		g_nHour++;
	}

	if (g_nHour>=24)
	{
		g_nHour = 0;
		g_nDay++;
		g_nWeekday++;
		g_nWeekday%=g_nMaxWeekday;
		if (g_nWeekday==0) g_nWeekday = g_nMaxWeekday;
		bDateChanged = true;
	}

	if (g_nDay>g_Months[g_nMonth].getDays()) 
	{
		g_nDay = 1;
		g_nMonth++;
		bDateChanged = true;
	}

	if (g_nMonth>g_nMaxMonth)
	{
		g_nMonth = 1;
		g_nYear++;
		bDateChanged = true;
	}

	if ((bDateChanged)||(!g_bInitOk))
	{
		g_nCurDawnHour = g_Months[g_nMonth].m_nDawnHour;
		g_nCurDawnMin = g_Months[g_nMonth].m_nDawnMin;
		g_nCurSunsetHour = g_Months[g_nMonth].m_nSunsetHour;
		g_nCurSunsetMin = g_Months[g_nMonth].m_nSunsetMin;
		g_fCurSnowMod = g_Months[g_nMonth].m_fSnowMod;
		g_fCurRainMod = g_Months[g_nMonth].m_fRainMod;
		g_fCurDryMod  = g_Months[g_nMonth].m_fDryMod;
		setSeason(g_Months[g_nMonth].m_nSeason);
		g_bInitOk = true;
	}
//	if ((g_nMinute==0)||(g_nMinute==30))
//		ConOut("HOUR : %d:%02d of %s %d-%s-%04d\n", g_nHour, g_nMinute, g_Weekday[g_nWeekday],
//			g_nDay, g_Months[g_nMonth].m_strName, g_nYear);

	return bDateChanged;
}

static void parseMonth(int m, FILE *F);
static void parseWeekday(int m, FILE *F);


/*!
\brief loads calendar.scp file
\author Xanathar
\since 0.52a
\todo should be changed when we'll use new scripts with stl maps
*/
void loadCalendarScp ()
{
#define SEC_NONE -1
#define SEC_MONTH 0 
#define SEC_WEEKDAY 1
#define SEC_HOLYDAY 2

	char buffer[2048];
	int i, ln;
	char *p;

	ConOut("Loading calendar information...");

	FILE *F;

	F = fopen("scripts/calendar.scp", "rt");
	if (F==NULL) {
		ConOut(" [FAIL]\n");
		return;
	}

	while(!feof(F))
	{ 
		fgets(buffer, 2000, F);
		if (feof(F)) break;
		if (chrInStr(buffer[0], ";#/\'")) continue;

		ln = strlen(buffer);
		for (i=0; i<ln; i++) if ((buffer[i]<='z')&&(buffer[i]>='a')) buffer[i]+= static_cast<char>('A'-'a');

		if ((p=strstr(buffer, "MONTH"))!=NULL) 
		{
			p = strstr(p, " ");
			parseMonth(atoi(p), F);
		}

		if ((p=strstr(buffer, "WEEKDAY"))!=NULL) 
		{
			p = strstr(p, " ");
			parseWeekday(atoi(p), F);
		}

/*		if ((p=strstr(buffer, "HOLYDAY"))!=NULL) 
		{
			p = strstr(p, " ");
//			parseHolyday(atoi(p));
		}
*///XAN : future 

	}
	fclose(F);
	ConOut(" [ OK ]\n");
	return;
	
}


/*!
\brief parses an hour  from the file
\return static
\author Xanathar
\param str the string containing the hour
\param h the integer which'll contain the hour
\param m the integer which'll contain the minutes
\since 0.52a
*/
static void parseHour (char *str, int &h, int &m)
{
	char *p = strstr(str, ":");
	if (p==NULL) return;
	*p = '\0';
	p++;
	h = atoi(str);
	m = atoi(p);
}
	


/*!
\brief parses a month from the calendar.scp file
\return static
\author Xanathar
\param m the current month we're parsing
\param F the opened calendar.scp
\since 0.52a
*/
static void parseMonth(int m, FILE *F)
{
	char buffer[2048];
	char *par;
	int i,ln;

	if (g_nMaxMonth<m) g_nMaxMonth = m;

	while (!feof(F))
	{
		fgets(buffer, 2000, F);
		if (feof(F)) break;

		if (strstr(buffer, "}")!=NULL) return;
		
		par = strstr(buffer, " ");
		
		if (par!=NULL) {
			*par = '\0';
			par++;
		}

		ln = strlen(buffer);
		for (i=0; i<ln; i++) if ((buffer[i]<='z')&&(buffer[i]>='a')) buffer[i]+= static_cast<char>('A'-'a');

		if (!strcmp(buffer,"SUNMODIFIER")) g_Months[m].m_fDryMod = (float)atof(par); 
		else if (!strcmp(buffer,"SNOWMODIFIER")) g_Months[m].m_fSnowMod = (float)atof(par); 
		else if (!strcmp(buffer,"RAINMODIFIER")) g_Months[m].m_fRainMod = (float)atof(par); 
		else if (!strcmp(buffer,"DRYMODIFIER")) g_Months[m].m_fDryMod = (float)atof(par); 
		else if (!strcmp(buffer,"COMPENSATION")) g_Months[m].m_nCompensation = atoi(par); 
		else if (!strcmp(buffer,"DAYS")) g_Months[m].m_nDays = atoi(par); 
		else if (!strcmp(buffer,"DAWN")) parseHour(par, g_Months[m].m_nDawnHour, g_Months[m].m_nDawnMin); 
		else if (!strcmp(buffer,"SUNSET")) parseHour(par, g_Months[m].m_nSunsetHour, g_Months[m].m_nSunsetMin); 
		else if (!strcmp(buffer,"SEASON")) g_Months[m].m_nSeason  = atoi(par); 
		else if (!strcmp(buffer,"NAME")) {
			g_Months[m].m_strName = new char[strlen(par)+1];
			strcpy(g_Months[m].m_strName, par);
			g_Months[m].m_strName[strlen(par)-1] = '\0';
		}
	}
}


/*!
\brief parses a weekday from calendar.scp
\return static
\author Xanathar
\param m current weekday we're parsing
\param F the opened calendar.scp
\since 0.52a
*/
static void parseWeekday(int m, FILE *F)
{
	char buffer[2048];
	char *par;
	int i,ln;

	if (g_nMaxWeekday<m) g_nMaxWeekday = m;

	while (!feof(F))
	{
		fgets(buffer, 2000, F);
		if (feof(F)) break;

		if (strstr(buffer, "}")!=NULL) return;
		
		par = strstr(buffer, " ");
		
		if (par!=NULL) {
			*par = '\0';
			par++;
		}

		ln = strlen(buffer);
		for (i=0; i<ln; i++) if ((buffer[i]<='z')&&(buffer[i]>='a')) buffer[i]+= static_cast<char>('A'-'a');

		if (!strcmp(buffer,"NAME")) {
			g_Weekday[m].m_strName = new char[strlen(par)+1];
			strcpy(g_Weekday[m].m_strName, par);
			g_Weekday[m].m_strName[strlen(par)-1] = '\0';
			int ln = strlen(g_Weekday[m].m_strName)-1;
			if (ln>0) {
				if ((g_Weekday[m].m_strName[ln]=='\r')||(g_Weekday[m].m_strName[ln]=='\n')) {
					g_Weekday[m].m_strName[ln]='\0';
				}
			}
		}
	}
}





} //NAMESPACE :)
