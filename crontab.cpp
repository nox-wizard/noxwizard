  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


/*****************************************************************************************

  NXW-Crontab 0.9 (27-december-2000)
  ---------------- ------  -----  ---  --  -

  Simple crontab-like scheduler for NXW servers.
  [originally designed by Xanathar for UOX3, 27-december-2000]

  CODER : Xanathar (gm.xanathar@libero.it - ICQ : 91330853)

  All the cron-jobs are in crontab.scp under the NXW directory

  NOTES :
	o The entries are checked in the same order entered
	o If you enables the DEBUGMODE macro commands are not executed but a system string is
	  broadcasted to everyone with the command line at execution time
	o Note that the "resolution" is 2 minutes  so the schedule "* * * * * SAVE"
	  is a save executed every 2 minutes
	o any line not beginning with a number is considered a comment

    o On NoX-Wizard 0.50 and later, this will become the main scheduling engine for all
	  realworld time scheduled tasks, like weather changes, world saves, backups, and more

    o Can be edited with the not pratical :D text editor of remote administration, and
	  eventually reloaded. New tasks can be added at runtime with no save by amx fns with

			addCronTask (const format[], ...);
     
	   which simply adds a schedule with that ConOut like syntax command.
	o Also the commands ADDCRON and ADDCRONW of remote administration takes a string
	  and insert it directly in the task list, the second updating the file also.

******************************************************************************************/

#include "nxwcommn.h"
#include "network.h"
#include "cmdtable.h"
#include "sregions.h"
#include "bounty.h"
#include "sndpkg.h"
#include "debug.h"
#include "amx/amxcback.h"
#include "srvparms.h"
#include "jail.h"
#include "worldmain.h"
#include "books.h"
#include "set.h"
#include "archive.h"
#include "trade.h"
#include "globals.h"
#include "inlines.h"
#include "chars.h"
#include "items.h"
#include "nox-wizard.h"



//@{
/*!
\name Useful macros
*/
#define CHKCRONTABINIT { if (!s_bCronTabOK) return; }
#define CRONTABFAIL(M) { ConOut(M); s_bCronTabOK = false; return; }
//@}

//Constants
#define MAXLINELEN 256
//## HEADER FILE FOR NXW-CRONTAB ##
//forward function for calls dispatching
void dispatchCommand(char *cmd, char *params);


//@{
/*!
\name Prototypes for internal use
*/
static UI32 calcMask (char *str, bool isMinutes);
static void addTaskToCrontab(char *str);
static char getFirstValidChar (char *str);
static bool getToken (char **pstrSource, char **pstrRes);
static bool isCharInStr (char *str, char chr);
static void exec_isave(char *dummy);
//@}

//static void parseCommand (char *str, struct tm *now);

// Internal data structures
struct CronTab
{
	UI32				mskHour;
	UI32				mskMinute;
	UI32				mskMDay;
	UI32				mskMonth;
	UI32				mskWDay;
	char*				strCommand;
	char*				strParams;
	struct CronTab*		next;
};

//Internal data
static bool s_bCronTabOK = false;

static CronTab *TaskH = NULL, *TaskT = NULL;	//!< manages the Crontab list as a linked List
static int s_nTasks = 0;			//!< just for statistics ;)
static UI32 s_nSaveTime = 0;


/*!
\brief Kills the scheduler

Frees the memory and kills the scheduler [maybe for a reload, not needed for termination]
not really deeply debugged (ok neither the rest is really deeply debugged! eheh ;) )

\author Xanathar
*/
void killCronTab()
{
	CHKCRONTABINIT;
	InfoOut("nxw-crontab : killing the task list...\n");

	s_bCronTabOK = false;
	CronTab *ptr = TaskH;
	CronTab *ptr2;

	while (ptr!=NULL)
	{
		safedelete(ptr->strCommand);
		ptr2 = ptr->next;
		safedelete(ptr);
		ptr = ptr2;
	}

	TaskH = TaskT = NULL;

	InfoOut("nxw-crontab : crontab completely killed. Services will be unavailable until next initialization\n");
}


/*!
\brief Inits the scheduler

 Function called to init the Crontab management code

\author Xanathar
\remark must be called right before the main loop
*/
void initCronTab()
{
	FILE *File;
	char strBuffer[MAXLINELEN];

	srand( (unsigned)time( NULL ) );

	ConOut("\nLoading crontab entries ... ");

	File = fopen("config/crontab.cfg","rt");
	
	if (File == NULL) CRONTABFAIL("[FAIL]\n");

	//Loads the CronTab
	while (!feof(File))
	{
		fgets(strBuffer, MAXLINELEN-1, File);
		if ((getFirstValidChar(strBuffer)!='#')&&(getFirstValidChar(strBuffer)!='/')&&(getFirstValidChar(strBuffer)!='\0'))
			addTaskToCrontab(strBuffer);
	}

	fclose(File);
	ConOut("[ OK ] (%d schedules in task list)\n", s_nTasks);
	s_bCronTabOK = true;
}


/*!
\brief checks for ready schedules

Function to be called every now and then... check to tasks ready to be scheduled

\author Xanathar
*/
void checkCronTab()
{
	CronTab *Cron = TaskH;
	UI32 min,hour,month,wday,mday;
	time_t ltime;
	struct tm *today;
	static UI32 lastMinuteMask;

	CHKCRONTABINIT; //if the test is ok then the data structures are safe

	time(&ltime);

	today = localtime( &ltime );

	if ((s_nSaveTime!=0)&&((uiCurrentTime>=s_nSaveTime)||(overflow))) {
		exec_isave(NULL);
		s_nSaveTime = 0;
	}


	// The last minute's mask is what avoid repetions of tasks :
	// No check has to be made if the minute mask is not changed !
	if ((today->tm_min >> 1) == (signed)lastMinuteMask) return;
	lastMinuteMask = (today->tm_min >> 1);

	while (Cron!=NULL)
	{
		min = Cron->mskMinute & (1 << lastMinuteMask );
		hour = Cron->mskHour & (1 << today->tm_hour );
		mday = Cron->mskMDay & (1 << today->tm_mday );
		wday = Cron->mskWDay & (1 << today->tm_wday );
		month = Cron->mskMonth & (1 << (today->tm_mon+1) );

		if ((min != 0) && (hour != 0) && (mday != 0) && (wday != 0) && (month != 0))
		{
			if (ServerScp::g_nVerboseCrontab)
					InfoOut("nxw-crontab : (%d/%d %d:%02d) starting task - %s [%s]\n",
						today->tm_mon+1, today->tm_mday,
						today->tm_hour, today->tm_min, Cron->strCommand, Cron->strParams);

			dispatchCommand(Cron->strCommand,Cron->strParams);

			if (ServerScp::g_nVerboseCrontab)
					InfoOut("nxw-crontab : (%d/%d %d:%02d) %s ended\n",
						today->tm_mon+1, today->tm_mday,
						today->tm_hour, today->tm_min, Cron->strCommand);
		}

		Cron = Cron->next;
	}
}






/************************************************************************************************
 **	STATIC FUNCTIONS
 ************************************************************************************************/


//----------------------------------------------------------------------------------------------
//------------------------------------- getFirstValidChar --------------------------------------
//----------------------------------------------------------------------------------------------
// returns the first non-blank char of a string
static char getFirstValidChar (char *str)
{
	if (strlen(str)==0) return '\0';
	int nLen = strlen(str);

	for (int i = 0; i < nLen; i++)
		if (str[i]>' ') return str[i]; //discards any ascii less or equal than 32 (space code)

	return '\0';
}

//----------------------------------------------------------------------------------------------
//---------------------------------------- getToken --------------------------------------------
//----------------------------------------------------------------------------------------------
//extracts a token from a string (using only spaces as separators)
static bool getToken (char **pstrSource, char **pstrRes)
{
	char *strRes = *pstrSource;

	int i=0;
	int nLen = strlen(*pstrSource);

	if (nLen==0) return false; //nothing can be extracted from nothing ;)

	while ((*pstrSource)[i]==' ') {
		if(i >= nLen) return false; //seems to be impossible... anyway this is safer =)
		i++;
	}

	strRes = (*pstrSource) + i;

	while (((*pstrSource)[i]!=' ')&&((*pstrSource)[i]!='\0')) {
		if(i >= nLen) return false;
		i++;
	}
	if ((*pstrSource)[i] != '\0') {
		(*pstrSource)[i] = '\0';
		(*pstrSource) += i+1;
	}
	else (*pstrSource) += i; //This is the case where the token is the end of the string

	*pstrRes = strRes;
	return true;
}


//----------------------------------------------------------------------------------------------
//---------------------------------------- isCharInStr -----------------------------------------
//----------------------------------------------------------------------------------------------
static bool isCharInStr (char *str, char chr)
{
	int nLen = strlen(str);

	for (int i=0; i<nLen; i++)
		if (str[i]==chr) return true;

	return false;
}


//----------------------------------------------------------------------------------------------
//---------------------------------------- addTaskToCrontab ------------------------------------
//----------------------------------------------------------------------------------------------
// This does all the dirty work at load time (creates nodes in list (= )
static void addTaskToCrontab(char *str)
{
//This macro is for internal use of this function [dirty but handy ;)]
#define EXTRACTTOKEN { if (!getToken(&str, &tkn)) { ErrOut("nxw-crontab : syntax error : %s\n",line); safedelete(line); safedelete(Cron); return; } }

	char *tkn;
	char *line;
	CronTab *Cron = new CronTab;
	int nLen, i;

	// Keep a copy of the line to signal errors
	line = new char[strlen(str)+1];
	strcpy(line, str);


	// extracts the  tokens from string
	EXTRACTTOKEN;
	Cron->mskMinute = calcMask (tkn, true);
	EXTRACTTOKEN;
	Cron->mskHour = calcMask (tkn, false);
	EXTRACTTOKEN;
	Cron->mskMDay = calcMask (tkn, false);
	EXTRACTTOKEN;
	Cron->mskMonth = calcMask (tkn, false);
	EXTRACTTOKEN;
	Cron->mskWDay = calcMask (tkn, false);

	//copy the command
	EXTRACTTOKEN;
	Cron->strCommand = new char[strlen(tkn)+1];
	strcpy(Cron->strCommand, tkn);
	//now lowercase the command and truncate string at CR/LF if exists (legacy of fgets =))
	nLen = strlen(Cron->strCommand);

	for (i = 0; i < nLen; i++) {
		if ((Cron->strCommand[i]=='\n')||(Cron->strCommand[i]=='\r')) {
			Cron->strCommand[i] = '\0';
			break;
		}
		if ((Cron->strCommand[i]>='A')&&(Cron->strCommand[i]<='Z'))
			Cron->strCommand[i] += 'a' - 'A';
	}

	//copy the parameters
	Cron->strParams = new char[strlen(str)+1];
	strcpy(Cron->strParams, str);

	//here the case must be preserved, just erase any CR/LF
	nLen = strlen(Cron->strParams);

	for (i = 0; i < nLen; i++) {
		if ((Cron->strParams[i]=='\n')||(Cron->strParams[i]=='\r')) {
			Cron->strParams[i] = '\0';
			break;
		}
	}


	Cron->next = NULL;

	//Yeah now we must insert this crontab in the Queue...
	if (TaskH == NULL) {
		//First time we insert in queue
		TaskH = TaskT = Cron;
	} else {
		TaskT->next = Cron;
		TaskT = Cron;
	}


	// Increment valid lines counter
	s_nTasks++;
}




//----------------------------------------------------------------------------------------------
//------------------------------------------- calcMask -----------------------------------------
//----------------------------------------------------------------------------------------------
// calcMask : calculates a 32bit mask from a given string
static UI32 calcMask (char *str, bool isMinutes)
{
	bool bFlag[32];
	int i;
	int nLen = strlen(str);
	char *tkn;
	int nVal, nFrom, nTo, nStep, nShift;
	UI32 mskRes;

	if (isCharInStr(str,'*')) return 0xFFFFFFFF;

	for (i = 0; i < 32; i++) bFlag[i] = false;

	if (isMinutes) nShift = 1;
		else nShift = 0;

	//Now we will parse the string
	//At first we'll separate the tokens
	//it's simple just change the commas in spaces and use the getToken ;)

	for (i = 0; i < nLen; i++)
		if(str[i]==',') str[i] = ' ';

	while (getToken(&str,&tkn))
	{
		if (!isCharInStr(tkn,'-')) {
			//simple element, we'll add it to our flag list
			nVal = atoi(tkn);
			bFlag[nVal>>nShift] = true;
		} else {
			//complex element see if step is specified
			nStep = 1;
			if (!isCharInStr(tkn,'-')) sscanf(tkn, "%d-%d", &nFrom, &nTo);
			else sscanf(tkn, "%d-%d/%d", &nFrom, &nTo, &nStep);
			for (i = nFrom; i<=nTo; i+=nStep) bFlag[i>>nShift] = true;
		}
	}

	//Now the bFlag array is filled with the booleans we need - just make a dword out of it
	mskRes = 0;

	for (i = 0 ; i<32; i++)
		if (bFlag[i]) mskRes |= (1 << i);

	return mskRes;
}












//define EMULMODE to avoid real execution of commands and simply outputs to the screen
//the commandline (gy mode)

//#define EMULMODE

#ifdef EMULMODE
#define CHECKCMD(A) { if (strcmp(cmd, #A )==0) { debugBroadcast( #A, params ); return;} }
#define CHECKCMDAMX(A) { if (strcmp(cmd, #A )==0) { debugBroadcast( #A, params ); return;} }
#else
#define CHECKCMD(A) { if (strcmp(cmd, #A )==0) { exec_ ##A (params); return;} }
#define CHECKCMDAMX(A) { if (strcmp(cmd, #A )==0) { exec_amx ##A (params); return;} }
#endif

// Proto of exec_gy : this is called by some other important functions to alert GMs ;)
static void exec_gy(char *txt);


static void exec_save(char *dummy)
{
   sysbroadcast(TRANSLATE("World will be saved in 30 seconds.."));

   s_nSaveTime = uiCurrentTime+30*MY_CLOCKS_PER_SEC;
   if (s_nSaveTime==0) s_nSaveTime++; //just in case...
}

static void exec_isave(char *dummy)
{
	if ( !cwmWorldState->Saving() )
	{
		cwmWorldState->saveNewWorld();
	} 
}


static void exec_shutdown(char *dummy)
{
	int n = atoi(dummy);
	endtime=uiCurrentTime+(CLOCKS_PER_SEC*n);
}

static void exec_whologow(char *dummy)
{
	int j=0;
	time_t ltime;
	struct tm *today;
	FILE *File = fopen(dummy, "wt");

	if (File == NULL) { ErrOut("crontab : whologow failed\n"); return; }

	time(&ltime);
	today = localtime( &ltime );


	fprintf(File,"----------------------------------------------------------\n");
	fprintf(File,"Log time : %d/%d %d:%02d\n",
				today->tm_mon+1, today->tm_mday, today->tm_hour, today->tm_min);

	fprintf(File,"Users in the World:\n");

	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if( ps==NULL )
			continue;
		
		P_CHAR pj=ps->currChar();
		if( ISVALIDPC(pj) ) //Keeps NPC's from appearing on the list
		{
			j++;
			fprintf(File, "%i) %s [%x]\n", (j-1), pj->getCurrentNameC(), pj->getSerial32());
		}
	}
	fprintf(File,"Total Users Online: %d\n", j);
	fprintf(File,"End of userlist\n");
	fclose(File);
}

static void exec_wholog(char *dummy)
{
	int j=0;
	time_t ltime;
	struct tm *today;
	FILE *File = fopen(dummy, "a+t");

	if (File == NULL) { ErrOut("crontab : wholog failed\n"); return; }

	time(&ltime);
	today = localtime( &ltime );


	fprintf(File,"----------------------------------------------------------\n");
	fprintf(File,"Log time : %d/%d %d:%02d\n",
				today->tm_mon+1, today->tm_mday, today->tm_hour, today->tm_min);

	fprintf(File,"Users in the World:\n");

	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if( ps==NULL )
			continue;
		P_CHAR pj=ps->currChar();
		if( ISVALIDPC(pj) ) //Keeps NPC's from appearing on the list
		{
			j++;
			fprintf(File, "%i) %s [%x ]\n", (j-1), pj->getCurrentNameC(), pj->getSerial32());
		}
	}
	fprintf(File,"Total Users Online: %d\n", j);
	fprintf(File,"End of userlist\n");
	fclose(File);
}



static void exec_loaddefaults (char *dummy)
{
	loadserverdefaults();
	exec_gy("Server was resetted (defaults loaded)");
}


static void exec_guardson (char *dummy)
{
	exec_gy("Guards are ON");
	server_data.guardsactive=1;
}

static void exec_guardsoff (char *dummy)
{
	exec_gy("Guards are OFF");
	server_data.guardsactive=0;
}




static void exec_weather (char *dummy)
{
	check_region_weatherchange();
}



static void exec_gcollect (char *dummy)
{
	gcollect();
}


static void exec_restock (char *dummy)
{
	Restocks->doRestock();
}

static void exec_restockall (char *dummy)
{
	Restocks->doRestockAll();
}

static void exec_zerokills (char *dummy)
{
return;
/*	for(int a=0;a<charcount;a++)
	{
		P_CHAR pc = MAKE_CHARREF_LR(a);
		pc->kills=0;
		setcharflag(pc);//AntiChrist
	}		

	exec_gy("Murder count has been set to 0"); */
}

static void exec_respawn (char *dummy)
{
		//Respawn->Start();
}

static void exec_broadcast(char *txt)
{
	UI08 sysname[30]={ 0x00, };
	strcpy((char *)sysname, "System");
/*
        ID: 0x01010101 // ???? why 1's ???
        Model: 0x0101
        Type: 1
        Color: 0x0040
        FontType: 0x0003
*/
	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWSOCKET s=sw.getSocket();
		if( s!=INVALID ) {
			SendSpeechMessagePkt(s, 0x01010101, 0x0101, 1, 0x0040, 0x0003, sysname, txt);
		}
	}
	Network->ClearBuffers(); // uhm ..... 
}

static void exec_gy(char *txt)
{
	UI08 name[30]={ 0x00, };
	strcpy((char *)name, "[CronTab Service - GM Only]");

	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if( ps==NULL )
			continue;
		P_CHAR pj=ps->currChar();
		if (ISVALIDPC(pj) && pj->IsGM())
		{
			SendSpeechMessagePkt(ps->toInt(), 0x01010101, 0x0101, 1, 0x0040, 0x0003, name, txt);
		}
	}

	Network->ClearBuffers();
}




static void exec_alert(char *params)
{
	sysbroadcast(params);
}

static void exec_amxcall(char *params)
{
	AmxFunction::g_prgOverride->CallFn(params);
}

//  *CFG <section>.<property>=<value> : do a CFG command 
static void exec_cfg(char *params)
{
}

static void exec_amxrun(char *params)
{
	AmxProgram *prg = new AmxProgram(params);
	prg->CallFn(-1);
	safedelete(prg);
}

/*!
\author Luxor
*/
static void exec_resetdailylimit(char *params)
{
        ConOut("Resetting daily limits (if any)...");

	cAllObjectsIter objs;
	for( objs.rewind(); !objs.IsEmpty(); objs++ ) {
		if ( !isCharSerial(objs.getSerial()) )
			continue;

		P_CHAR pc = static_cast<P_CHAR>(objs.getObject());
		if(ISVALIDPC(pc))
			pc->statGainedToday = 0;
	}
	ConOut("[ OK ]\n");
}

static void exec_backup (char *dummy)
{
	cChar::archive();
	cItem::archive();
	Guildz.archive();
	prison::archive();
	Books::archive();
	exec_isave( NULL );
}



// Used for emulation mode ;)
#ifdef EMULMODE
static void debugBroadcast(char *str1, char *str2)
{
	char s[800];
	sprintf(s,"crontab simulated command : %s(%s)", str1, str2);
	exec_gy(s);
}
#endif


void dispatchCommand(char *cmd, char *params)
{

	//To add a command (e.g. the command goofy) you should :
	//   - add a CHECKCMD(goofy) line
	//   - add a static void exec_goofy(char *params) function above
	// Remember to use ALWAYS lowercase command names ;)

	CHECKCMD(gcollect);
	CHECKCMD(restock);
	CHECKCMD(restockall);
	CHECKCMD(guardson);
	CHECKCMD(guardsoff);
	CHECKCMD(loaddefaults);
	CHECKCMD(respawn);
	CHECKCMD(save);
	CHECKCMD(shutdown);
	CHECKCMD(zerokills);
	CHECKCMD(gy);
	CHECKCMD(weather);
	CHECKCMD(broadcast);
	CHECKCMD(alert);
	CHECKCMD(wholog);
	CHECKCMD(whologow);
	CHECKCMD(backup);
	CHECKCMD(cfg);
	CHECKCMD(resetdailylimit);
	CHECKCMDAMX(run);
	CHECKCMDAMX(call);



	//Last : syntax error in command : log the fault!
	ErrOut("crontab syntax error, command : %s\n", cmd);

}











