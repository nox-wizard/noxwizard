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

extern class CWorldMain		*cwmWorldState;
extern class cAccounts		*Accounts;
extern class cGuilds		*Guilds;
extern class cMapStuff		*Map;
extern class cTargets		*Targ;
extern class cPartys		*Partys;
extern struct teffect_st	*teffects;
extern class cSpawns		*Spawns;
extern class cAreas		*Areas;
extern class cRestockMng	*Restocks;

namespace Scripts {
	extern class cScpScript* Advance;
	extern class cScpScript* Calendar;
	extern class cScpScript* Carve;
	extern class cScpScript* Colors;
	extern class cScpScript* Create;
	extern class cScpScript* CronTab;
	extern class cScpScript* Envoke;
	extern class cScpScript* Fishing;
	extern class cScpScript* Gumps;
	extern class cScpScript* HardItems;
	extern class cScpScript* House;

	extern class cScpScript* HostDeny;
	extern class cScpScript* HtmlStrm;
	extern class cScpScript* Items;
	extern class cScpScript* Location;
	extern class cScpScript* MenuPriv;
	extern class cScpScript* Menus;
	extern class cScpScript* MetaGM;
	extern class cScpScript* Misc;
	extern class cScpScript* MList;
	extern class cScpScript* MsgBoard;
	extern class cScpScript* Necro;
	extern class cScpScript* Newbie;
	extern class cScpScript* Npc;
	extern class cScpScript* NpcMagic;
	extern class cScpScript* NTrigrs;
	extern class cScpScript* Override;
	extern class cScpScript* Polymorph;
	extern class cScpScript* Regions;
	extern class cScpScript* Skills;
	extern class cScpScript* Spawn;
	extern class cScpScript* Speech;
	extern class cScpScript* Spells;
	extern class cScpScript* Teleport;
	extern class cScpScript* Titles;
	extern class cScpScript* Tracking;
	extern class cScpScript* Triggers;
	extern class cScpScript* WTrigrs;
	extern class cScpScript* Mountable;
	extern class cScpScript* WeaponInfo;
	extern class cScpScript* Containers;
	extern class cScpScript* Areas;
};
extern std::vector<std::string> clientsAllowed;
extern class cRegion *mapRegions; //setup map regions Tauriel

// - the below structure is for looking up items based on serial #
// - item's serial, owner's serial, char's serial, and container's serial
//extern lookuptr_st ownsp[HASHMAX]; //spawnsp[HASHMAX];
//extern lookuptr_st cownsp[HASHMAX]; //cspawnsp[HASHMAX];
//extern lookuptr_st imultisp[HASHMAX], cmultisp[HASHMAX]; /*glowsp[HASHMAX], stablesp[HASHMAX];*/

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
extern tracking_st tracking_data;
extern begging_st begging_data;
extern fishing_st fishing_data;
extern spiritspeak_st spiritspeak_data;
extern speed_st speed;//Lag Fix -- Zippy

extern UI32 VersionRecordSize;
extern UI32 MultiRecordSize;
extern UI32 LandRecordSize;
extern UI32 TileRecordSize;
extern UI32 MapRecordSize;
extern UI32 MultiIndexRecordSize;
extern UI32 StaticRecordSize;


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

extern char firstpacket[MAXCLIENT+1];
extern char noweather[MAXCLIENT+1]; //LB
extern unsigned char LSD[MAXCLIENT];
extern unsigned char DRAGGED[MAXCLIENT];
extern unsigned char EVILDRAGG[MAXCLIENT];

extern int newclient[MAXCLIENT];
extern unsigned char  buffer[MAXCLIENT][MAXBUFFER];
extern char  outbuffer[MAXCLIENT][MAXBUFFER];
extern int whomenudata [(MAXCLIENT)*10]; // LB, for improved whomenu, ( is important !!!
extern int client[MAXCLIENT];
extern short int walksequence[MAXCLIENT];
extern signed char addid5[MAXCLIENT];
extern int acctno[MAXCLIENT];
extern unsigned char clientip[MAXCLIENT][4];
extern make_st itemmake[MAXCLIENT];
extern int tempint[MAXCLIENT];
extern unsigned char addid1[MAXCLIENT];
extern unsigned char addid2[MAXCLIENT];
extern unsigned char addid3[MAXCLIENT];
extern unsigned char addid4[MAXCLIENT];
extern unsigned char dyeall[MAXCLIENT];
extern int addx[MAXCLIENT];
extern int addy[MAXCLIENT];
extern int addx2[MAXCLIENT];
extern int addy2[MAXCLIENT];
extern signed char addz[MAXCLIENT];
extern int addmitem[MAXCLIENT];
extern char xtext[MAXCLIENT][31];
extern unsigned char perm[MAXCLIENT];
extern unsigned char cryptclient[MAXCLIENT];
extern unsigned char usedfree[MAXCLIENT];
extern int binlength[MAXIMUM+1];
extern int boutlength[MAXIMUM+1];
extern unsigned char clientDimension[MAXCLIENT]; // stores if the client is the new 3d or old 2d one

//extern int spattackValue[MAXCLIENT];
extern int clickx[MAXCLIENT];
extern int clicky[MAXCLIENT];
extern int currentSpellType[MAXCLIENT]; // 0=spellcast, 1=scrollcast, 2=wand cast
extern unsigned char targetok[MAXCLIENT];

//////////////////////////////////////////////
//              MAxBuffer                  //
/////////////////////////////////////////////

extern char tbuffer[MAXBUFFER];
extern char xoutbuffer[MAXBUFFER*5];





extern unitile_st xyblock[XYMAX];
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

extern creat_st creatures[2048]; //LB, stores the base-sound+sound flags of monsters, animals
extern location_st location[4000];
extern logout_st logout[1024];//Instalog
extern region_st region[256];
extern advance_st wpadvance[1000];
extern char spellname[71][25];
extern unsigned int metagm[256][7]; // for meta gm script
extern signed short int menupriv[64][256]; // for menu priv script
extern int validEscortRegion[256];

extern int priv3a[MAXCLIENT];
extern int priv3b[MAXCLIENT];
extern int priv3c[MAXCLIENT];
extern int priv3d[MAXCLIENT];
extern int priv3e[MAXCLIENT];
extern int priv3f[MAXCLIENT];
extern int priv3g[MAXCLIENT];

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
extern char goldamountstr[10];
extern char defaultpriv1str[2];
extern char defaultpriv2str[2];

#define TEMP_STR_SIZE 1024

namespace Unicode {
extern char temp[TEMP_STR_SIZE];
extern char temp2[TEMP_STR_SIZE];
}

extern char temp3[1024];
extern char temp4[1024];

extern char mapname[512], sidxname[512], statname[512], vername[512], tilename[512], multiname[512], midxname[512];
extern char saveintervalstr[4];
extern char hname[40];
extern char scpfilename[32];//AntiChrist


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

extern unsigned int fly_p; // flying probability = 1/fly_p each step (if it doesnt fly)
extern unsigned char fly_steps_max;

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


extern FILE *infile, *scpfile, *lstfile, *wscfile, *mapfile, *sidxfile, *statfile, *verfile, *tilefile, *multifile, *midxfile;
extern unsigned int servcount;
extern unsigned int startcount;



extern unsigned char xcounter;
extern unsigned char ycounter;        //x&y counter used for placing deleted items and chars
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
