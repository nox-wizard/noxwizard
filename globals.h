  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


/*!
\file
\brief Globals declarations
\note here are listed <b>all</b> globals, not just those contained in globals.cpp
*/
#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "amx/amxvarserver.h"
#include "regions.h"
#include "scp_parser.h"
#include "client.h"


// GOOD GLOBALZ (Those which are just fine)

extern server_st server_data;


#ifdef __unix__
extern unsigned long int oldtime, newtime;
#define SLPMULTI 1000
#else
extern WSADATA wsaData;
extern WORD wVersionRequested;
extern long int oldtime, newtime;
#endif

extern bool g_nShowLayers;

extern int dummy__; //used for default parameters of type int& and int*

extern repsys_st repsys;
extern resource_st resource;

extern class cGuilds		*Guilds;
extern struct teffect_st	*teffects;

extern std::vector<std::string> clientsAllowed;
extern class cRegion *mapRegions; //setup map regions Tauriel

extern  std::multimap <int, tele_locations_st> tele_locations; // can't use a map here :(
// Profiling
extern int networkTime;
extern int timerTime;
extern int autoTime;
extern int loopTime;
extern int networkTimeCount;
extern int timerTimeCount;
extern int autoTimeCount;
extern int loopTimeCount;

//server.cfg/scp things :)
extern begging_st begging_data;
extern fishing_st fishing_data;
extern spiritspeak_st spiritspeak_data;
extern speed_st speed;//Lag Fix -- Zippy


extern char n_scripts[NUM_SCRIPTS][512] ; // array of script filenames

extern short g_nMainTCPPort;


/*****************************************************************************
 EVIL GLOBALZ (Those to be cut from NXW whenever possible)
 *****************************************************************************/
extern char script1[512];
extern char script2[512];
extern char script3[512];
extern char script4[512]; // added to use for newbie items



/*****************************************************************************
 NEUTRAL GLOBALZ (Those We've not yet evaluated if risky or fine)
 *****************************************************************************/


extern int escortRegions;
/////////////////////////////////////////////
///             MAXCLIENT arrays          ///
/////////////////////////////////////////////
// maximum too cause maxclient = maximum +1



extern P_CLIENT clientInfo[MAXCLIENT];

extern bool cryptedClient[MAXCLIENT];

extern unsigned char  buffer[MAXCLIENT][MAXBUFFER];
extern char  outbuffer[MAXCLIENT][MAXBUFFER];
extern int client[MAXCLIENT];
extern short int walksequence[MAXCLIENT];
extern int acctno[MAXCLIENT];
extern unsigned char clientip[MAXCLIENT][4];
extern int binlength[MAXIMUM+1];
extern int boutlength[MAXIMUM+1];
extern unsigned char clientDimension[MAXCLIENT]; // stores if the client is the new 3d or old 2d one




//////////////////////////////////////////////
//              MAxBuffer                  //
/////////////////////////////////////////////

extern char tbuffer[MAXBUFFER];
extern char xoutbuffer[MAXBUFFER*5];





extern gmpage_st gmpages[MAXPAGES];
extern gmpage_st counspages[MAXPAGES];
extern skill_st skillinfo[SKILLS+1];
extern unsigned short int doorbase[DOORTYPES];
extern char skillname[SKILLS+1][20];
extern char serv[MAXSERV][3][30]; // Servers list

extern char saveip[30];
extern char start[MAXSTART][5][30]; // Startpoints list
extern title_st title[ALLSKILLS+1];
extern unsigned char *comm[CMAX];
extern int layers[MAXLAYERS];

extern location_st location[4000];
extern logout_st logout[1024];//Instalog
extern region_st region[256];
extern advance_st wpadvance[1000];
extern char spellname[71][25];
extern unsigned int metagm[256][7]; // for meta gm script
extern int validEscortRegion[256];

extern int npcshape[5]; //Stores the coords of the bouding shape for the NPC

//extern jail_st jails[11];

/////////////////////////////////////////
///////////// global string vars /////////
/////////////////////////////////////////

extern char idname[256];
extern char pass1[256];
extern char pass2[256];
extern char fametitle[128];
extern char skilltitle[50];
extern char prowesstitle[50];
extern char *completetitle;
extern char gettokenstr[256];

#define TEMP_STR_SIZE 1024

namespace Unicode {
extern char temp[TEMP_STR_SIZE];
extern char temp2[TEMP_STR_SIZE];
}




////////////////////////////////
//  small packet-arrays
///////////////////////////////

extern unsigned char w_anim[3];

#ifdef __unix__
extern termios termstate ;
#endif
extern int g_nCurrentSocket;


// Global Variables
extern unsigned int uiCurrentTime;
extern unsigned int polyduration;


extern int gDecayItem; //Last item that was checked for decay Tauriel 3/7/99
extern int gRespawnItem; //Last item that was checked for respawn Tauriel 3/7/99

extern int save_counter;//LB, world backup rate

extern int gatecount;
//Time variables
//extern int day, hour, minute, ampm; //Initial time is noon.
extern int day;
extern int secondsperuominute; //Number of seconds for a UO minute.
extern UI32 uotickcount;
extern int moon1update;
extern int moon2update;

extern unsigned char moon1;
extern unsigned char moon2;
extern unsigned char dungeonlightlevel;
extern unsigned char worldfixedlevel;
extern unsigned char worldcurlevel;
extern unsigned char worldbrightlevel;
extern unsigned char worlddarklevel;

extern int goldamount;

extern int defaultpriv1;
extern int defaultpriv2;

extern unsigned int teffectcount; // No temp effects to start with
extern unsigned int nextfieldeffecttime;
extern unsigned int nextnpcaitime;
extern unsigned int nextdecaytime;

//extern int autosaved, saveinterval,dosavewarning;
extern bool heartbeat;
extern int len_connection_addr;
extern struct sockaddr_in connection;
extern struct sockaddr_in client_addr;
extern struct hostent *he;
extern int err, error;
extern bool keeprun;
extern int a_socket;
extern fd_set conn ;
extern fd_set all ;
extern fd_set errsock ;
extern int nfds;
extern timeval nettimeout;
extern int now;


extern FILE *wscfile;
extern unsigned int servcount;
extern unsigned int startcount;



extern int secure; // Secure mode

extern unsigned char season;

//extern int xycount;

extern int locationcount;
extern unsigned int logoutcount;//Instalog

extern unsigned long int updatepctime;



extern int global_lis;

extern char *cline;

extern int tnum;
extern unsigned int starttime, endtime, lclock;
extern bool overflow;

//extern unsigned char globallight;
extern unsigned char wtype;

extern int executebatch;
extern int openings;

extern unsigned int respawntime;
extern unsigned int gatedesttime;

extern int donpcupdate;
extern unsigned int hungerdamagetimer; // Used for hunger damage

extern unsigned long int serverstarttime;




extern unsigned long initialserversec ;
extern unsigned long initialservermill ;





#endif //__GLOBALS_H__
