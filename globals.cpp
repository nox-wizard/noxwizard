  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "nxwcommn.h"
#include "globals.h"

server_st server_data;

#if defined(__unix__)
termios termstate ;
#endif
unsigned int uiCurrentTime;


unsigned short int doorbase[DOORTYPES]={
0x0675, 0x0685, 0x0695, 0x06A5, 0x06B5, 0x06C5, 0x06D5, 0x06E5, 0x0839, 0x084C,
0x0866, 0x00E8, 0x0314, 0x0324, 0x0334, 0x0344, 0x0354};

char skillname[SKILLS+1][20]={
"ALCHEMY", "ANATOMY", "ANIMALLORE", "ITEMID", "ARMSLORE", "PARRYING", "BEGGING", "BLACKSMITHING", "BOWCRAFT",
"PEACEMAKING", "CAMPING", "CARPENTRY", "CARTOGRAPHY", "COOKING", "DETECTINGHIDDEN", "ENTICEMENT", "EVALUATINGINTEL",
"HEALING", "FISHING", "FORENSICS", "HERDING", "HIDING", "PROVOCATION", "INSCRIPTION", "LOCKPICKING", "MAGERY",
"MAGICRESISTANCE", "TACTICS", "SNOOPING", "MUSICIANSHIP", "POISONING", "ARCHERY", "SPIRITSPEAK", "STEALING",
"TAILORING", "TAMING", "TASTEID", "TINKERING", "TRACKING", "VETERINARY", "SWORDSMANSHIP", "MACEFIGHTING", "FENCING",
"WRESTLING", "LUMBERJACKING", "MINING", "MEDITATION", "STEALTH", "REMOVETRAPS", "ALLSKILLS", "STR", "DEX", "INT", "FAME", "KARMA",
"NXWFLAG0", "NXWFLAG1", "NXWFLAG2", "NXWFLAG3",
"AMXFLAG0", "AMXFLAG1", "AMXFLAG2", "AMXFLAG3", "AMXFLAG4", "AMXFLAG5",
"AMXFLAG6", "AMXFLAG7", "AMXFLAG8", "AMXFLAG9", "AMXFLAGA", "AMXFLAGB",
"AMXFLAGC", "AMXFLAGD", "AMXFLAGE", "AMXFLAGF", "ACCOUNT"
};

char spellname[71][25]={
        "Clumsy","Create Food","Feeblemind","Heal","Magic Arrow","Night Sight","Reactive Armor","Weaken",
        "Agility","Cunning","Cure","Harm","Magic Trap","Magic Untrap","Protection","Strength",
        "Bless","Fireball","Magic Lock","Poison","Telekinesis","Teleport","Unlock","Wall Of Stone",
        "Arch Cure","Arch Protection","Curse","Fire Field","Greater Heal","Lightning","Mana Drain","Recall",
        "Blade Spirits","Dispel Field","Incognito","Magic Reflection","Mind Blast","Paralyze","Poison Field","Summon Creature",
        "Dispel","Energy Bolt","Explosion","Invisibility","Mark","Mass Curse","Paralyze Field","Reveal",
        "Chain Lightning","Energy Field","Flame Strike","Gate Travel","Mana Vampire","Mass Dispel","Meteor Swarm","Polymorph",
        "Earthquake","Energy Vortex","Resurrection","Summon Air Elemental",
        "Summon Daemon","Summon Earth Elemental","Summon Fire Elemental","Summon Water Elemental",
        "Random", "Necro1", "Necro2", "Necro3", "Necro4", "Necro5"
};
//unsigned char login04a[6]="\xA9\x09\x24\x02";
//unsigned char login04b[61]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
//unsigned char login04c[18]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
//unsigned char login04d[64]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
//unsigned char login03[12]="\x8C\x00\x00\x00\x00\x13\x88\x7B\x7B\x7B\x01";
//unsigned char goxyz[20]="\x20\x00\x05\xA8\x90\x01\x90\x00\x83\xFF\x00\x06\x08\x06\x49\x00\x00\x02\x00";
//unsigned char sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
//unsigned char removeitem[6]="\x1D\x00\x00\x00\x00";
//unsigned char gmprefix[10]="\x7C\x00\x00\x01\x02\x03\x04\x00\x64";
//unsigned char gmmiddle[5]="\x00\x00\x00\x00";
//unsigned char sfx[13]="\x54\x01\x12\x34\x00\x00\x06\x40\x05\x9A\x00\x00";
//unsigned char doact[15]="\x6E\x01\x02\x03\x04\x01\x02\x00\x05\x00\x01\x00\x00\x01";
//unsigned char bpitem[20]="\x40\x0D\x98\xF7\x0F\x4F\x00\x00\x09\x00\x30\x00\x52\x40\x0B\x00\x1A\x00\x00";
//unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
//unsigned char gump2[4]="\x00\x00\x00";
//unsigned char gump3[3]="\x00\x00";
//unsigned char dyevat[10]="\x95\x40\x01\x02\x03\x00\x00\x0F\xAB";
//unsigned char updscroll[11]="\xA6\x01\x02\x02\x00\x00\x00\x00\x01\x02";
//unsigned char spc[2]="\x20";
//unsigned char bounce[3]="\x27\x00";
//unsigned char extmove[18]="\x77\x01\x02\x03\x04\x01\x90\x01\x02\x01\x02\x0A\x00\xED\x00\x00\x00";
//unsigned char particleSystem[49];

#ifndef __unix__
	WSADATA wsaData;
	WORD wVersionRequested;
	long int oldtime, newtime;
#else
	unsigned long int oldtime, newtime;
#endif

unsigned long int updatepctime=0;

unsigned int polyduration=90;

int gatecount=0;
//Time variables
int day=0, hour = 12, minute = 1, ampm = 0; //Initial time is noon.
int secondsperuominute=5; //Number of seconds for a UO minute.
UI32 uotickcount=1;
int moon1update=0;
int moon2update=0;

int openings=0;
unsigned long initialserversec ;
unsigned long initialservermill ;
unsigned char moon1=0;
unsigned char moon2=0;
unsigned char dungeonlightlevel=0x15;
unsigned char worldfixedlevel=255;
unsigned char worldcurlevel=0;
unsigned char worldbrightlevel=0x00; //used to be 0x05
unsigned char worlddarklevel=0x017; //used to be 0x1a - too dark
int goldamount;
int defaultpriv1;
int defaultpriv2;
unsigned int teffectcount=0; // No temp effects to start with
unsigned int nextfieldeffecttime=0;
unsigned int nextnpcaitime=0;
unsigned int nextdecaytime=0;


bool heartbeat;

int len_connection_addr;
struct sockaddr_in connection;
struct sockaddr_in client_addr;
struct hostent *he;
int err, error;
bool keeprun;
int a_socket;

fd_set conn ;
fd_set all ;
fd_set errsock;

int nfds;
short g_nMainTCPPort=2593; //default!
timeval nettimeout;
int now;

FILE *wscfile;

unsigned int servcount;
unsigned int startcount;

int secure; // Secure mode


repsys_st repsys;
resource_st resource;

//int xycount;

unsigned char w_anim[3]={0x1d,0x95,0}; // flying blood instead of lightnings, LB in a real bad mood

int save_counter;//LB, world backup rate
int gDecayItem=0; //Last item that was checked for decay Tauriel 3/7/99
int gRespawnItem=0; //Last item that was checked for respawn Tauriel 3/7/99


teffect_st *teffects;

int locationcount;
unsigned int logoutcount;//Instalog

char *cline;
int tnum;

unsigned int starttime, endtime, lclock;
bool overflow;
unsigned char wtype;
int executebatch;
bool g_nShowLayers;

unsigned int respawntime=0;
unsigned int gatedesttime=0;

int stablockcachei;
int stablockcachehit;
int stablockcachemiss;

unsigned char season=0;
int networkTime = 0;
int timerTime = 0;
int autoTime = 0;
int loopTime = 0;
int networkTimeCount = 1000;
int timerTimeCount = 1000;
int autoTimeCount = 1000;
int loopTimeCount = 1000;

int global_lis=1;

int donpcupdate;

unsigned long int serverstarttime;

begging_st begging_data; // was = {BEGGINGRANGE,"Could thou spare a few coins?","Hey buddy can you spare some gold?","I have a family to feed, think of the children."};
fishing_st fishing_data = {FISHINGTIMEBASE,FISHINGTIMER};
spiritspeak_st spiritspeak_data = {SPIRITSPEAKTIMER};
speed_st speed;
int triggerx;
int triggery;
signed char triggerz;
// Script files that need to be cached
// Crackerjack Jul 31/99
char n_scripts[NUM_SCRIPTS][512] =
{"--don't load this--","--don't load this--","--don't load this--","--don't load this--",
	"--don't load this--","--don't load this--","--don't load this--","--don't load this--",
	"--don't load this--", "--don't load this--","--don't load this--", "--don't load this--",
	"--don't load this--", "--don't load this--", "--don't load this--", "--don't load this--",
	"--don't load this--", "--don't load this--", "--don't load this--", "--don't load this--",
	"scripts/spawn.scp", "--don't load this--", "--don't load this--", "--don't load this--",
	"--don't load this--", "--don't load this--", "--don't load this--", "--don't load this--", "--don't load this--", "--don't load this--",
	"--don't load this--", "--don't load this--"};//the two ""s are for custom scripts.



int escortRegions = 0;


unsigned int hungerdamagetimer=0; // For hunger damage


//@{
/*!
\name Classes definitions
*/
cRegion		*mapRegions;
cGuilds		*Guilds;
//@}


//////////////////// ARRAYS that didnt get initialized above ////////////////////

/////////////////////////////////////////////
///             MAXCLIENT arrays          ///
/////////////////////////////////////////////
// maximum too cause maxclient = maximum +1

bool cryptedClient[MAXCLIENT];

char unsigned buffer[MAXCLIENT][MAXBUFFER];
char  outbuffer[MAXCLIENT][MAXBUFFER];
int client[MAXCLIENT];
short int walksequence[MAXCLIENT];
int acctno[MAXCLIENT];
unsigned char clientip[MAXCLIENT][4];
int binlength[MAXIMUM+1];
int boutlength[MAXIMUM+1];

unsigned char clientDimension[MAXCLIENT];

//////////////////////////////////////////////
//              MAxBuffer                  //
/////////////////////////////////////////////

 char tbuffer[MAXBUFFER];
 char xoutbuffer[MAXBUFFER*5];

/******************************************************/

///////////////////////////////////////////
///   and the rest                        /
///////////////////////////////////////////

 skill_st skillinfo[SKILLS+1];
 gmpage_st gmpages[MAXPAGES];
 gmpage_st counspages[MAXPAGES];
 std::multimap <int, tele_locations_st> tele_locations; //!< can't use a map here, so using a multimap with x as the key :(
 char serv[MAXSERV][3][30]; //!< Servers list

 char saveip[30]; //!< Servers list
 char start[MAXSTART][5][30]; //!< Startpoints list
 title_st title[ALLSKILLS+1];
 unsigned char *comm[CMAX];
 int layers[MAXLAYERS];

 location_st location[4000];
 logout_st logout[1024];//!< Instalog
 region_st region[256];
 advance_st wpadvance[1000];
 unsigned int metagm[256][7]; //!< for meta gm script
 int validEscortRegion[256];

int npcshape[5]; //!< Stores the coords of the bouding shape for the NPC

//jail_st jails[11];

std::vector<std::string> clientsAllowed; //!< client version controll system

/////////////////////////////////////////
///////////// global string vars /////////
/////////////////////////////////////////

char idname[256];
char script1[512];
char script2[512];
char script3[512];
char script4[512]; //!< added to use for newbie items
char pass1[256];
char pass2[256];
char fametitle[128];
char skilltitle[50];
char prowesstitle[50];
char *completetitle;
char gettokenstr[256];

namespace Unicode {
char temp[TEMP_STR_SIZE];
char temp2[TEMP_STR_SIZE];
}


int dummy__; //!< dont touch ! nomen est omen !

amxVariableServer amxVS;
