  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"

server_st server_data;

#if defined(__unix__)
termios termstate ;
#endif
UI32 uiCurrentTime;

UI32 VersionRecordSize = 20L;
UI32 MultiRecordSize = 12L;
UI32 LandRecordSize = 26L;
UI32 TileRecordSize = 37L;
UI32 MapRecordSize = 3L;
UI32 MultiIndexRecordSize = 12L;
UI32 StaticRecordSize = 7L;
//  Should characters have expiration before delete

UI16 doorbase[DOORTYPES]={
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

#ifndef __unix__
	WSADATA wsaData;
	WORD wVersionRequested;
	SI32 oldtime, newtime;
#else
	UI32 oldtime, newtime;
#endif

UI32 updatepctime=0;

UI32 polyduration=90;

int gatecount=0;
//Time variables
int day=0, hour = 12, minute = 1, ampm = 0; //Initial time is noon.
int secondsperuominute=5; //Number of seconds for a UO minute.
UI32 uotickcount=1;
int moon1update=0;
int moon2update=0;

int openings=0;
UI32 initialserversec ;
UI32 initialservermill ;
UI08 moon1=0;
UI08 moon2=0;
UI08 dungeonlightlevel=0x15;
UI08 worldfixedlevel=255;
UI08 worldcurlevel=0;
UI08 worldbrightlevel=0x00; //used to be 0x05
UI08 worlddarklevel=0x017; //used to be 0x1a - too dark
int goldamount;
int defaultpriv1;
int defaultpriv2;
UI32 teffectcount=0; // No temp effects to start with
UI32 nextfieldeffecttime=0;
UI32 nextnpcaitime=0;
UI32 nextdecaytime=0;


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
short g_nMainTCPPort=2593; //deafult!
timeval nettimeout;
int now;

FILE *infile, *scpfile, *lstfile, *wscfile, *mapfile, *sidxfile, *statfile, *verfile, *tilefile, *multifile, *midxfile;

UI32 servcount;
UI32 startcount;

UI08 xcounter;
UI08 ycounter;
int secure; // Secure mode


repsys_st repsys;
resource_st resource;

//int xycount;

UI08 w_anim[3]={0x1d,0x95,0}; // flying blood instead of lightnings, LB in a real bad mood

int save_counter;//LB, world backup rate
int gDecayItem=0; //Last item that was checked for decay Tauriel 3/7/99
int gRespawnItem=0; //Last item that was checked for respawn Tauriel 3/7/99


teffect_st *teffects;

int locationcount;
UI32 logoutcount;//Instalog

char *cline;
int tnum;

UI32 starttime, endtime, lclock;
bool overflow;
UI08 wtype;
int executebatch;
bool g_nShowLayers;

UI32 fly_p=18; // flying probability = 1/fly_p each step (if it doesnt fly)
UI08 fly_steps_max=27;

UI32 respawntime=0;
UI32 gatedesttime=0;

int stablockcachei;
int stablockcachehit;
int stablockcachemiss;

UI08 season=0;
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

UI32 serverstarttime;

tracking_st tracking_data = {TRACKINGRANGE,MAXTRACKINGTARGETS,TRACKINGTIMER,TRACKINGDISPLAYTIME};
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


UI32 hungerdamagetimer=0; // For hunger damage


//@{
/*!
\name Classes definitions
*/
CWorldMain	*cwmWorldState;
cGuilds		*Guilds;
cMapStuff	*Map;
cTargets	*Targ;
//@}


//////////////////// ARRAYS that didnt get initialized above ////////////////////

/////////////////////////////////////////////
///             MAXCLIENT arrays          ///
/////////////////////////////////////////////
// maximum too cause maxclient = maximum +1

char noweather[MAXCLIENT+1]; //LB
UI08 LSD[MAXCLIENT];
UI08 DRAGGED[MAXCLIENT];
UI08 EVILDRAGG[MAXCLIENT]; //!< we need this for UO3D clients to save dragging history , LB

char firstpacket[MAXCLIENT+1];

int newclient[MAXCLIENT];
char unsigned buffer[MAXCLIENT][MAXBUFFER];
char  outbuffer[MAXCLIENT][MAXBUFFER];
int whomenudata [(MAXCLIENT)*10]; //!< LB, for improved whomenu, ( is important !!!
int client[MAXCLIENT];
short int walksequence[MAXCLIENT];
signed char addid5[MAXCLIENT];
int acctno[MAXCLIENT];
UI08 clientip[MAXCLIENT][4];
make_st itemmake[MAXCLIENT];
int tempint[MAXCLIENT];
UI08 addid1[MAXCLIENT];
UI08 addid2[MAXCLIENT];
UI08 addid3[MAXCLIENT];
UI08 addid4[MAXCLIENT];
UI08 dyeall[MAXCLIENT];
int addx[MAXCLIENT];
int addy[MAXCLIENT];
int addx2[MAXCLIENT];
int addy2[MAXCLIENT];
signed char addz[MAXCLIENT];
int addmitem[MAXCLIENT];
char xtext[MAXCLIENT][31];
UI08 perm[MAXCLIENT];
UI08 cryptclient[MAXCLIENT];
UI08 usedfree[MAXCLIENT];
int binlength[MAXIMUM+1];
int boutlength[MAXIMUM+1];

int clickx[MAXCLIENT];
int clicky[MAXCLIENT];
int currentSpellType[MAXCLIENT]; //!< 0=spellcast, 1=scrollcast, 2=wand cast
UI08 targetok[MAXCLIENT];
UI08 clientDimension[MAXCLIENT];
//int spattackValue[MAXCLIENT];

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
 unitile_st xyblock[XYMAX];
 gmpage_st gmpages[MAXPAGES];
 gmpage_st counspages[MAXPAGES];
 std::multimap <int, tele_locations_st> tele_locations; //!< can't use a map here, so using a multimap with x as the key :(
 char serv[MAXSERV][3][30]; //!< Servers list

 char saveip[30]; //!< Servers list
 char start[MAXSTART][5][30]; //!< Startpoints list
 title_st title[ALLSKILLS+1];
 UI08 *comm[CMAX];
 int layers[MAXLAYERS];

 location_st location[4000];
 logout_st logout[1024];//!< Instalog
 regions::region_st region[256];
 advance_st wpadvance[1000];
 UI32 metagm[256][7]; //!< for meta gm script
 signed short int menupriv[64][256]; //!< for menu priv script
 int validEscortRegion[256];
 creat_st creatures[2048];

int priv3a[MAXCLIENT];
int priv3b[MAXCLIENT];
int priv3c[MAXCLIENT];
int priv3d[MAXCLIENT];
int priv3e[MAXCLIENT];
int priv3f[MAXCLIENT];
int priv3g[MAXCLIENT];
 
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
char goldamountstr[10];
char defaultpriv1str[2];
char defaultpriv2str[2];

namespace Unicode {
char temp[TEMP_STR_SIZE];
char temp2[TEMP_STR_SIZE];
}
//char temp3[TEMP_STR_SIZE];
//char temp4[TEMP_STR_SIZE];

char mapname[512], sidxname[512], statname[512], vername[512], tilename[512], multiname[512], midxname[512];
char saveintervalstr[4];
char hname[40];
char scpfilename[32];//AntiChrist

int dummy__; //!< dont touch ! nomen est omen !

amxVariableServer amxVS;
