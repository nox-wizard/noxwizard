  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __SRVPARAMS_H_
#define __SRVPARAMS_H_

#include "typedefs.h"

void loadserverdefaults();
void loadserverscript();
void loadclientsallowed();
void saveserverscript();

void preloadSections(char *fn);
int cfg_command (char *commandstr);
extern void commitserverscript(); // second phase setup

void saveserverscript();
#define SrvParms (&server_data)	// in preparation of making sever_data a class.Use SrvParms->xxx instead of server_dat.xxx (Duke)

#define NPCSPEED .2	// 5 steps every second
#define NPCFOLLOWSPEED ((NPCSPEED)/4)
#define TRACKINGRANGE 10        // tracking range is at least TRACKINGRANGE, additional distance is calculated by Skill,INT
#define TRACKINGTIMER 30        // tracking last at least TRACKINGTIMER seconds, additional time is calculated by Skill,INT, & DEX
#define TRACKINGDISPLAYTIME 30  // tracking will display above the character every TRACKINGDISPLAYTIME seconds

#define FISHINGTIMEBASE 10              // it takes at least FISHINGTIMEBASE sec to fish
#define FISHINGTIMER 5

#define DECAYTIMER 300  // 5 minute decay rate
#define INVISTIMER 60 // invisibility spell lasts 1 minute
#define HUNGERRATE 6000 // 15 minutes
#define SKILLDELAY 5   // Skill usage delay     (5 seconds)
#define REGENRATE1 8 // Seconds to heal ONE hp
#define REGENRATE2 3 // Seconds to heal ONE stm
#define REGENRATE3 5 // Seconds to heal ONE mn
#define GATETIMER 30


#define NPC_BASE_FLEEAT 20
#define NPC_BASE_REATTACKAT 40

struct server_st
{
 unsigned int eclipsemode;
 unsigned int checkcharage ;
 unsigned int poisontimer;
 //unsigned int repairworldtimer; // Ripper
 unsigned int joinmsg;
 unsigned int partmsg;
 unsigned int decaytimer;

 //taken from 6904t2(5/10/99) - AntiChrist
 unsigned int playercorpsedecaymultiplier;
 unsigned int lootdecayswithcorpse;

 unsigned int invisibiliytimer;
 unsigned int hunger_system;
 unsigned int hungerrate;
 unsigned int skilldelay;
 unsigned int snoopdelay;
 unsigned int objectdelay;
 unsigned int bandagedelay;					// in seconds, on top of objectdelay
 unsigned int bandageincombat;				// 0=no, 1=yes; can use bandages while healer and/or patient are in combat ?
 unsigned int inactivitytimeout;			// seconds of inactivity until player will be disconnected (duke)
			// LB: if no packet received from client in that time period, disconnection ! (except idle packet)
			// thats necessary, cauz in case of client/OS crashes or IP changes on the fly players didnt get disconnected

 unsigned int hitpointrate;
 unsigned int staminarate;
 unsigned int manarate;
 unsigned int skilladvancemodifier;
 unsigned int statsadvancemodifier;
 unsigned int gatetimer;
 unsigned int minecheck;
 unsigned int showdeathanim;
 unsigned int combathitmessage;
 unsigned int monsters_vs_animals;
 unsigned int animals_attack_chance;
 unsigned int animals_guarded;
 unsigned int npc_base_fleeat;
 unsigned int npc_base_reattackat;
 unsigned int tamed_disappear; // Ripper
 unsigned int houseintown; // Ripper
 unsigned int shoprestock; // Ripper
 unsigned char guardsactive;
 unsigned char bg_sounds;
 unsigned char commandPrefix; //Ripper

 std::string archivePath;
 std::string savePath;
 std::string worldfileExtension;
 std::string characterWorldfile;
 std::string itemWorldfile;
 std::string guildWorldfile;
 std::string jailWorldfile;
 std::string bookWorldfile;

 int backup_save_ratio; // LB, each X'th save-intervall a backup is done if a path is given ...

 unsigned int maxabsorbtion; // Magius(CHE) --- Maximum Armour of the single pieces of armour (for player)!
 unsigned int maxnohabsorbtion; // Magius(CHE) --- Maximum Armour of the total armour (for NPC)!
 unsigned int npcdamage; // Magius(CHE) --- DAmage divided by this number if the attacked is an NPC!
 unsigned int sellbyname; // Magius(CHE)
 unsigned int sellmaxitem; // Magius(CHE)
 unsigned int skilllevel; // Magius(CHE)
 unsigned int trade_system; // Magius(CHE)
 unsigned int rank_system; // Magius(CHE)
 unsigned char hungerdamage;
 unsigned int hungerdamagerate;
 char armoraffectmana; // Should armor slow mana regen ?

 char announceworldsaves;

 unsigned char auto_a_create; //Create accounts when people log in with unknown name/password
 unsigned int  auto_a_reload; //Auto reload accounts if file was touched, this stores the time interval in minutes.

 unsigned int rogue;
 unsigned int maxstealthsteps;//AntiChrist - max number of steps allowed with stealth skill at 100.0
 unsigned int runningstaminasteps;//AntiChrist - max number of steps allowed with stealth skill at 100.0
 unsigned short int quittime;//Instalog
 int html;//HTML
 float boatspeed;

 unsigned int savespawns;


 float stablingfee;

 int attackstamina;	// AntiChrist (6) - for ATTACKSTAMINA

 unsigned int skillcap;//LB skill cap
 unsigned int statcap;//AntiChrist stat cap

 char specialbanktrigger[50]; //special bank trigger - AntiChrist
 int usespecialbank; //special bank - AntiChrist
 char Unicodelanguage[4];//UNICODE font name for multiple language support -  N6
 
 int CheckBank; // Vendors check bank for gold- Ripper
 UI32 defaultSelledItem; //!< default number of item who a vendor can sell fot every item

 int cutscrollreq;//AntiChrist - cut skill requirements for scrolls

 int persecute;//AntiChrist - persecute thing

 char errors_to_console; // LB
 char gm_log;
 char speech_log;
 char pvp_log;
 char server_log;
 char log_path[256];

 unsigned long int housedecay_secs;

 double goldweight;//AntiChrist - configurable gold weight
 char msgboardpath[256];   // Dupois - Added April 4, 1999 for msgboard.cpp - path to store BB* files (ie C:\POSTS\)
 int  msgpostaccess;       // Dupois - Added April 4, 1999 for msgboard.cpp - who gets to post messages (0=GM only, 1 = everyone)
 int  msgpostremove;       // Dupois - Added April 4, 1999 for msgboard.cpp - who gets to remove post messages (0=GM only, 1 = everyone)
 int  msgretention;        // Dupois - Added April 4, 1999 for msgboard.cpp - how long to keep posts before deleting them automatically (in days - default 30)
 int  escortactive;        // Dupois - Added April 4, 1999 for escort quests - are escort quests created
 int  escortinitexpire;    // Dupois - Added April 4, 1999 for escort quests - time to expire after spawning NPC in secs(0=never)
 int  escortactiveexpire;  // Dupois - Added April 4, 1999 for escort quests - time to expire after quest has been accepted by player in secs(0=never)
 int  escortdoneexpire;    // Dupois - Added April 4, 1999 for escort quests - time to expire after quest has been completed by player in secs(0=never)
 int  bountysactive;       // Dupois - Added July 18, 2000 for bounty quests - whether bounties are active (1-default) or inactive (0)
 int  bountysexpire;       // Dupois - Added July 18, 2000 for bounty quests - number of days when an unclaimed bounty expires and is deleted (0=never-default)

 int  blockaccbadpass;		//elcabesa tempblock
 unsigned int n_badpass;	//elcabesa tempblock
 unsigned int time_badpass;	//elcabesa tempblock

 int always_add_hex; //!< Endymion, if 1 the add command paramters are hexadecimal, else normal 

 double staminaonhorse; //!< Endymion, is the % of stamina loss when riding
 int disable_z_checking; //!< Endymion, if 1 the z cheching is disabled ( cool for worldbuilder, not for gameplay )

 int feature;
};

namespace ServerScp {
extern int g_nVerboseCrontab;
extern bool g_bEnableInternalCompiler;

extern int g_nAllowMagicInTown; //default :0;
extern int g_nSnoopWillCriminal; //default :1;
extern int g_nStealWillCriminal; //default :2;
extern int g_nChopWillCriminal; //default :2;
extern int g_nPolymorphWillCriminal; //default :1;
extern int g_nLootingWillCriminal; //default :1;
extern int g_nLowKarmaWillCriminal; //default :1; CURRENTLY DISABLED :)
extern int g_nHelpingGreyWillCriminal; //default :1;
extern int g_nHelpingCriminalWillCriminal; //default :1;
extern int g_nInstantGuard; //default :1 if yes when attacking someone in town guarded a guard spawn

extern int g_nGreyCanBeLooted; //default : 1;
extern int g_nSnoopKarmaLoss; //default :-2000;
extern int g_nStealKarmaLoss; //default :-9000;
extern int g_nSnoopFameLoss; //default :0;
extern int g_nStealFameLoss; //default :0;
extern int g_nChopKarmaLoss; //default :-100;
extern int g_nChopFameLoss; //default :0;
extern int g_nBountyKarmaGain; //default :+100;
extern int g_nBountyFameGain; //default :50;
extern int g_nEnableKarmaLock; //default :1;
extern char* g_strKarmaUnlockPrayer; //default :"I PRAY MY GODS TO FORGIVE MY ACTIONS";

extern int g_nEnableBooks; // default: 0 Books are off

extern int g_nUnequipOnReequip; // default 0: don't unequip
extern int g_nEquipOnDclick; // default 0: don't equip on dclick

//[Special]
extern int g_nShowPCNames; //default: 1;
extern int g_nShowFireResistanceInTitle; //default :1;
extern int g_nShowPoisonResistanceInTitle; //default :1;
extern int g_nShowParalisysResistanceInTitle; //default :1;
extern int g_nDisableFieldResistanceRaise;


//[Remote_Administration]
extern int g_nRacTCPPort; //default :2594;
extern int g_nUseCharByCharMode; //default :1;
extern char* g_strPasswordToEnter; //default :"RACADMINPASS";

//[AMX Scripting]
extern int g_nEnableAMXScripts; //default :1;
extern int g_nUseAccountEncryption;
extern int g_css_override_case_sensitive; //!< Endymion, if 1 the xss speech override are case sensitive
//[Windows]
extern int g_nAutoDetectIP;
extern int g_nBehindNat;
extern int g_nAutoDetectMuls;
extern int g_nLineBuffer;

extern int g_nDeamonMode;

extern int g_nWeatherChangeRate;//=512;

extern int g_nPopUpHelp;

extern int g_nWalkIgnoreWetBit;

extern int g_nStealthOnHorse;
extern int g_nStealthArLimit;

extern int g_nLoadDebugger;
extern int g_nStopOnAnyCall;
extern int g_nCheckBySmall; //!< check all object with scripted small function

extern int g_nRedirectOutput;
extern char g_szOutput[2048];

extern int g_nShoveStmDamage;
extern float g_fStaminaUsage[6];
extern int g_nUseNewNpcMagic;

extern int g_nLimitPlayerSparring;
extern int g_nStatsAdvanceSystem;
extern unsigned int g_nLimitRoleNumbers;
extern int g_nPlayersCanDeleteRoles;
extern int g_nBankLimit;

extern int g_nStatDailyLimit;
extern int g_nAdventureMode;

//[Restock]
extern unsigned int g_nRestockTimeRate;
}

#endif
