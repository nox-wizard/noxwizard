  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


/***********************************************************************************
  Xanathar : New server.cfg interface

  Works like the classic server.cfg except that :

		o It contains even the options once contained in noxwizard.ini & noxwizard.cfg
		o Setup happens in two phases, since .ini data should be read after Map initialization
		o It supports dynamic configuration through CFG <section>.<option>=value

  for example n remote administration or as a gm command one can type


	  CFG REPSYS.MAXKILLS=6  [in remote admin]

	  'CFG REPSYS.MAXKILLS=6  [in gm commands]

	or something equivalent to
	  execCfgCommand("REPSYS.MAXKILLS=%d", 6); [in amx scripts]

    A stock function will be contained in amx scripts :

		setCfgOption("REPSYS", "MAXKILLS", 6);

	Similar to

    stock setCfgOption (const section[], const property[], const value)
	{
		execCfgCommand("%s.%s=%d", section, property, value);
	}


***********************************************************************************/

#include "nxwcommn.h"
#include "party.h"
#include "debug.h"
#include "version.h"
#include "calendar.h"
#include "amx/amxscript.h"
#include "data.h"
#include "extractionskills.h"
#include "inlines.h"
#include "scripts.h"

static int loadnxwoptions (char *script1, char *script2);

#ifdef __BEOS__
#define INADDR_NONE 0xffffffff
#endif

static char temp_map[120];
static char temp_statics[120];
static char temp_staidx[120];
static char temp_verdata[120];
static char temp_tiledata[120];
static char temp_multimul[120];
static char temp_multiidx[120];

// Xanathar : parses a server.cfg section
static int parseSection(int (*parseLine)(char *s1, char *s2 ) ) // by Xanathar qsort
{
	int loopexit=0;
	int errors = 0;
	do
	{
		readw2();
		if ((parseLine(script1, script2)<0)&&(strcmp(script1, "}")))
		{
			ConOut("\nERROR : Syntax error, can't parse %s\n", script1);
			errors++;
		}
	}
	while ( (strcmp(script1, "}")) && (++loopexit < MAXLOOPS) );
	return -errors;
}

// Xanathar : parses a CFG command line. As it is now it's useless, but provides
// room for other improvements in future (err maybe.. booh ;))
static char *gprop, *gval;

static int parseCfgLine(int (*parseLine)(char *s1, char *s2 ) ) // by Xanathar qsort
{
	return parseLine(gprop, gval);
}

static int chooseSection(char *section,  int (*parseSec)(int (*parseLine)(char *s1, char *s2 )));

//NoX-Wizard defaults, ripped from NXWCFG
//[Reputation]
namespace ServerScp {

int g_nInstantGuard=0;
int g_nAllowMagicInTown=0;
int g_nSnoopWillCriminal=1;
int g_nStealWillCriminal=2;
int g_nChopWillCriminal=2;
int g_nPolymorphWillCriminal=1;
int g_nLootingWillCriminal=1;
int g_nLowKarmaWillCriminal=1;
int g_nHelpingGreyWillCriminal=1;
int g_nHelpingCriminalWillCriminal=2;
int g_nGreyCanBeLooted=1;
int g_nSnoopKarmaLoss=-2000;
int g_nStealKarmaLoss=-9000;
int g_nSnoopFameLoss=0;
int g_nStealFameLoss=0;
int g_nChopKarmaLoss=-100;
int g_nChopFameLoss=0;
int g_nBountyKarmaGain=+100;
int g_nBountyFameGain=50;
int g_nEnableKarmaLock=1;
int g_nVerboseCrontab = 0;
int g_nPopUpHelp = 1;
int g_nStatDailyLimit = 999;

int g_nWalkIgnoreWetBit = 0;

char* g_strKarmaUnlockPrayer="I PRAY MY GODS TO FORGIVE MY ACTIONS";
int g_nEnableBooks = 0;
int g_nUnequipOnReequip = 0;
int g_nEquipOnDclick = 0;

//[Special]
int g_nShowPCNames=1;
int g_nShowFireResistanceInTitle=1;
int g_nShowPoisonResistanceInTitle=1;
int g_nShowParalisysResistanceInTitle=1;
int g_nDisableFieldResistanceRaise=1;
int g_nAdventureMode=0;

//[Remote_Administration]
int g_nRacTCPPort=2594;
int g_nUseCharByCharMode=1;

//[AMX Scripting]
int g_nEnableAMXScripts=1;
int g_nDeamonMode = 0;
int g_css_override_case_sensitive = 1;

//[System]
int g_nUseAccountEncryption=0;
//[System]

//[Windows]
int g_nAutoDetectMuls=1;
int g_nLineBuffer=50;

int g_nAutoDetectIP=1;
int g_nBehindNat=0;
bool g_bEnableInternalCompiler = true;
int g_nStealthOnHorse = 0;
int g_nStealthArLimit = 1;
int g_nLoadDebugger = 0;
int g_nCheckBySmall = 0;
int g_nStopOnAnyCall = 0; //<-- XAN : UNUSED! :D

int g_nRedirectOutput = 0;
char g_szOutput[2048];

int g_nShoveStmDamage = 2;
float g_fStaminaUsage[6]={0.10f,0.25f,0.50f,0.75f,1.0f,10.0f};

int g_nUseNewNpcMagic = 1;

int g_nPlayersCanDeleteRoles = 1;
unsigned int g_nLimitRoleNumbers = 5;
int g_nLimitPlayerSparring = 0;
int g_nStatsAdvanceSystem = 0;
int g_nBankLimit = 0;

// important: loaddefault has to be called before.
// currently it is. makes no sense to change that too.
TIMERVAL g_nRestockTimeRate = 15;

}


extern bool g_bNTService;


void loadclientsallowed()
{
   int num_c=0, s=0, loopexit=0;
   std::vector<std::string> cls;

   do
   {
		readFullLine();

		if(!(strcmp(script1,"SERVER_DEFAULT"))) { s=1; break; }
		else if(!(strcmp(script1,"ALL"))) { s=2; break; }
		else if (strlen(script1)>3)
		{
			num_c++;
			cls.push_back(script1);
		}
   }
   while ( (strcmp(script1, "}")) && (++loopexit < MAXLOOPS) );

   if (num_c==0 && s==0) s=1; // no entry == pick server_default

   switch(s)
   {
	   case 0:
               clientsAllowed.pop_back();  // delete the entry from loaddefaults
			   copy(cls.begin(), cls.end(), back_inserter(clientsAllowed) ); // copy temp vector to perm vector
		       break;

	   case 1:
		       break; // already in vector from loaddefaults

	   case 2:
		       std::string sd("ALL");
		       clientsAllowed.pop_back();    // delete server_defualt entry from loaddefaults
			   clientsAllowed.push_back(sd); // now add the correct one
		       break;
   }


}

void loadserverdefaults()
{
	std::string sd("ALL");

	strcpy(ServerScp::g_szOutput, "nxwout");

	clientsAllowed.push_back(sd);
	strcpy(serv[0][0], "NoX-Wizard Shard");
	strcpy(serv[0][1], "127.0.0.1");
	strcpy(serv[0][2], "2593");

	strcpy(temp_map, "./map0.mul");
	strcpy(temp_statics, "./statics0.mul");
	strcpy(temp_staidx, "./staidx0.mul");
	strcpy(temp_verdata, "./verdata.mul");
	strcpy(temp_tiledata, "./tiledata.mul");
	strcpy(temp_multimul, "./multi.mul");
	strcpy(temp_multiidx, "./multi.idx");

	strcpy(server_data.Unicodelanguage,"ENU"); //UNICODE font name for multiple language support - N6
	strcpy(server_data.specialbanktrigger,"WARE"); //AntiChrist - Special Bank word trigger
	server_data.usespecialbank=0;	//AntiChrist - 1= Special Bank enabled
	server_data.goldweight=0.005;	//what a goldpiece weighs this is in hundreths of a stone! AntiChrist
	server_data.poisontimer=180; // lb
	server_data.armoraffectmana=1; //xan
	server_data.decaytimer=DECAYTIMER;
	// Should we check character age for delete?
	server_data.checkcharage = 0 ;

	server_data.skillcap=700;	// default=no cap
	server_data.statcap=200;		// default=no cap

	//taken from 6904t2(5/10/99) - AntiChrist
	server_data.playercorpsedecaymultiplier=3;	// Player's corpse decays 3x the normal decay rate
	server_data.lootdecayswithcorpse=1;			// JM - 0 Loot disappears with corpse, 1 loot gets left when corpse decays

	server_data.auto_a_create = 1;                     // for auto accounts
	server_data.auto_a_reload = 0;

	server_data.invisibiliytimer=INVISTIMER;
	server_data.hungerrate=HUNGERRATE;
	server_data.hunger_system=1;
	server_data.hungerdamagerate=10;			// every 10 seconds
	server_data.snoopdelay=15;
	server_data.hungerdamage=2;
	server_data.skilldelay=SKILLDELAY;
	server_data.objectdelay=1;					// in seconds
	server_data.bandagedelay=6;					// in seconds
	server_data.bandageincombat=1;				// 0=no, 1=yes; can use bandages while healer and/or patient are in combat ?
	server_data.inactivitytimeout=300;			// seconds of inactivity until player will be disconnected
	server_data.hitpointrate=REGENRATE1;
	server_data.staminarate=REGENRATE2;
	server_data.manarate=REGENRATE3;
	server_data.skilladvancemodifier=1000;		// Default Modulo val for skill advance
    server_data.statsadvancemodifier=500;		// Default Modulo val for stats advance
	server_data.gatetimer=GATETIMER;
	server_data.minecheck=1;
	server_data.showdeathanim=1;				// 1 = yes/true
	server_data.combathitmessage=1;
	server_data.attackstamina=0;				// AntiChrist - attacker looses stamina when hits
	server_data.monsters_vs_animals=0;			// By default monsters won't attack animals;
	server_data.animals_attack_chance=15;		// a 15% chance
	server_data.animals_guarded=0;				// By default players can attack animals without summoning guards
	server_data.npc_base_fleeat=NPC_BASE_FLEEAT;
	server_data.npc_base_reattackat=NPC_BASE_REATTACKAT;
	server_data.maxabsorbtion=20;				// 20 Arm (single armour piece) -- Magius(CHE)
	server_data.maxnohabsorbtion=100;			// 100 Arm (total armour) -- Magius(CHE)
	server_data.sellbyname=0;		// Values= 0(Disabled) or 1(Enabled) - The NPC Vendors buy from you if your item has the same name of his item! --- Magius(CHE)
	server_data.skilllevel=5;		// Range from 1 to 10 - This value if the difficulty to create an item using a make command: 1 is easy, 10 is difficult! - Magius(CHE)
	server_data.sellmaxitem=50;		// Set maximum amount of items that one player can sell at one time (5 is standard OSI) --- Magius(CHE)
	server_data.npcdamage=2;
	server_data.rank_system=1;		// Rank system to make various type of a single intem based on the creator's skill! - Magius(CHE)
	server_data.guardsactive=1;
	server_data.errors_to_console=1;
	server_data.bg_sounds=2;
	server_data.announceworldsaves=1;
	server_data.joinmsg=1;
	server_data.partmsg=1;
	server_data.stablingfee=0.25;
	server_data.savespawns=1;

	server_data.gm_log=0;
	server_data.pvp_log=0;
	server_data.speech_log=0;
	server_data.server_log=0;

	server_data.rogue=1;
	server_data.quittime=300;//Instalog

	strcpy(server_data.msgboardpath, "save/msgboard/");	   // Dupois - Added Dec 20, 1999 for message boards (current dir)
	server_data.backup_save_ratio=1; //LB

	server_data.msgpostaccess=0;           // Dupois - Added Dec 20, 1999 for message boards (GM only)
	server_data.msgpostremove=0;           // Dupois - Added Dec 20, 1999 for message boards (GM only)
	server_data.msgretention=30;           // Dupois - Added Dec 20, 1999 for message boards (30 Days)
	server_data.escortactive=1;            // Dupois - Added Dec 20, 1999 for message boards (Active)
	server_data.escortinitexpire=86400;    // Dupois - Added Dec 20, 1999 for message boards (24 hours)
	server_data.escortactiveexpire=1800;   // Dupois - Added Dec 20, 1999 for message boards (1/2 Hour)
	server_data.escortdoneexpire=1800;     // Dupois - Added Dec 20, 1999 for message boards (1/2 Hour)

	server_data.bountysactive=1;           // Dupois - Added July 18, 2000 for bountys (1-Active)
	server_data.bountysexpire=0;           // Dupois - Added July 18, 2000 for bountys (0-Never - in days)


	server_data.maxstealthsteps=10;// AntiChrist - stealth=100.0 --> 10 steps allowed
	server_data.runningstaminasteps=15;// AntiChrist - how often ( in steps ) stamina decreases while running

	server_data.boatspeed=0.75;//Boats

	server_data.cutscrollreq=1;//AntiChrist

	server_data.persecute=1;//AntiChrist

	server_data.feature=0; //Transparent mode: don't send packet 0xB9 
	server_data.html=-1;//HTML

	server_data.tamed_disappear=1; // Ripper
	server_data.houseintown=0; // Ripper
	server_data.shoprestock=1; //Ripper
	server_data.commandPrefix = '\''; //Ripper

	server_data.CheckBank = 2000; // Ripper..2000 default
	server_data.defaultSelledItem = 10;

	server_data.housedecay_secs=604800*2; // LB, one week //xan, 2 weeks default

    server_data.eclipsemode=0; // Light to dark..

	//cwmWorldState->SetLoopSaveAmt(-1);

	speed.srtime=30;
	speed.itemtime=(float)1.5;
	speed.npctime=(float)1.0; // lb, why -1 zippy ??????
    speed.npcaitime=(float)1.2;
	speed.tamednpctime=(float)0.9;//AntiChrist
	speed.npcfollowtime=(float)0.5; //Ripper
	speed.nice=2;

	resource.logs=3;
	resource.logtime=600;
	resource.logarea=10;
	resource.lograte=3;
	resource.logtrigger=0;
	resource.logstamina=-5;
	//REPSYS
	repsys.crimtime=120;
	repsys.maxkills=4;
	repsys.murderdecay=420;
	//RepSys ---^
	begging_data.range=3;
	begging_data.timer=300;
	strcpy(begging_data.text[0],"Could thou spare a few coins?");
	strcpy(begging_data.text[1],"Hey buddy can you spare some gold?");
	strcpy(begging_data.text[2],"I have a family to feed, think of the children.");
	fishing_data.basetime=FISHINGTIMEBASE;
	fishing_data.randomtime=FISHINGTIMER;
	spiritspeak_data.spiritspeaktimer=SPIRITSPEAKTIMER;

	server_data.blockaccbadpass=0;		//elcabesa tempblock
    server_data.n_badpass=0;			//elcabesa tempblock
	server_data.time_badpass=0;			//elcabesa tempblock
	server_data.always_add_hex=0;	//endymion add command stuff
	server_data.staminaonhorse=0.50; 
	server_data.disable_z_checking=0;
 	server_data.archivePath="save/";
 	server_data.savePath="backup/";
 	server_data.worldfileExtension=".wsc";
 	server_data.characterWorldfile="nxwchars";
 	server_data.itemWorldfile="nxwitems";
 	server_data.guildWorldfile="nxwguild";
 	server_data.jailWorldfile="nxwjail";
	server_data.bookWorldfile="nxwbook";
}

static int loadspeed(char *script1, char *script2)//Lag Fix -- Zippy -- NEW FUNCTION
{
        if(!(strcmp(script1,"NICE"))) speed.nice=str2num(script2);
		else if(!(strcmp(script1,"CHECK_ITEMS"))) speed.itemtime=(float)atof(script2);
		else if(!(strcmp(script1,"CHECK_SPAWNREGIONS"))) speed.srtime=str2num(script2);
		else if(!(strcmp(script1,"CHECK_NPCS"))) speed.npctime=(float)atof(script2);
        else if(!(strcmp(script1,"CHECK_NPCAI"))) speed.npcaitime=(float)atof(script2);
        else if(!(strcmp(script1,"CHECK_TAMEDNPCS"))) speed.tamednpctime=(float)atof(script2);//AntiChrist
		else if(!(strcmp(script1,"CHECK_NPCFOLLOW"))) speed.npcfollowtime=(float)atof(script2);//Ripper
		else return -1;
		return 0;
}


static int loadcombat(char *script1, char *script2) // By Magius(CHE)
{
		if(!(strcmp(script1,"COMBAT_HIT_MESSAGE"))) server_data.combathitmessage=str2num(script2);
		else if(!(strcmp(script1,"MAX_ABSORBTION"))) server_data.maxabsorbtion=str2num(script2); //MAgius(CHE)
		else if(!(strcmp(script1,"MAX_NON_HUMAN_ABSORBTION"))) server_data.maxnohabsorbtion=str2num(script2); //MAgius(CHE) (2)
		else if(!(strcmp(script1,"NPC_DAMAGE_RATE"))) server_data.npcdamage=str2num(script2); //MAgius(CHE) (3)
		else if(!(strcmp(script1,"MONSTERS_VS_ANIMALS"))) server_data.monsters_vs_animals=str2num(script2);
		else if(!(strcmp(script1,"ANIMALS_ATTACK_CHANCE"))) server_data.animals_attack_chance=str2num(script2);
		else if(!(strcmp(script1,"ANIMALS_GUARDED"))) server_data.animals_guarded=str2num(script2);
		else if(!(strcmp(script1,"NPC_BASE_FLEEAT"))) server_data.npc_base_fleeat=str2num(script2);
		else if(!(strcmp(script1,"NPC_BASE_REATTACKAT"))) server_data.npc_base_reattackat=str2num(script2);
		else if(!(strcmp(script1,"ATTACKSTAMINA"))) server_data.attackstamina=str2num(script2); // antichrist (6) - for ATTACKSTAMINA
		else return -1;
		return 0;
}

static int loadregenerate(char *script1, char *script2) // by Magius(CHE)
{
		if(!(strcmp(script1,"HITPOINTS_REGENRATE"))) server_data.hitpointrate=str2num(script2);
		else if(!(strcmp(script1,"STAMINA_REGENRATE"))) server_data.staminarate=str2num(script2);
		else if(!(strcmp(script1,"MANA_REGENRATE"))) server_data.manarate=str2num(script2);
		else if(!(strcmp(script1,"ARMOR_AFFECT_MANA_REGEN"))) server_data.armoraffectmana=str2num(script2);
		else return -1;
		return 0;
}


static int block_acc(char *script1, char *script2) // elcabesa tempblock
{													// elcabesa tempblock
		if(!(strcmp(script1,"BLOCKACCBADPASS"))) server_data.blockaccbadpass=str2num(script2);		// elcabesa tempblock
		else if(!(strcmp(script1,"N_BADPASS"))) server_data.n_badpass=str2num(script2);				// elcabesa tempblock
		else if(!(strcmp(script1,"TIME_BLOCKED"))) server_data.time_badpass=str2num(script2);		// elcabesa tempblock
		else return -1;																						// elcabesa tempblock
		return 0;																							// elcabesa tempblock
}																											// elcabesa tempblock




static int loadhunger(char *script1, char *script2) // by Magius(CHE)
{
		if(!(strcmp(script1,"HUNGERRATE"))) server_data.hungerrate=str2num(script2);
		else if(!(strcmp(script1,"HUNGER_DAMAGE"))) server_data.hungerdamage=str2num(script2);
		else if(!(strcmp(script1,"HUNGER_SYSTEM"))) server_data.hunger_system=str2num(script2);
		else if(!(strcmp(script1,"HUNGER_DAMAGE_RATE"))) server_data.hungerdamagerate=str2num(script2);
		else return -1;
		return 0;
}

static int loadvendor(char *script1, char *script2) // by Magius(CHE)
{
		if(!(strcmp(script1,"SELLBYNAME"))) {
			server_data.sellbyname=str2num(script2);
			if (server_data.sellbyname!=0) server_data.sellbyname=1;  //MAgius(CHE)
		}
		else if(!(strcmp(script1,"SELLMAXITEM"))) {
			server_data.sellmaxitem=str2num(script2);
		}
		else if(!(strcmp(script1,"RESTOCKRATE"))) {
			ServerScp::g_nRestockTimeRate=str2num(script2);
		}
		else if(!(strcmp(script1,"TRADESYSTEM"))) {
			server_data.trade_system=str2num(script2); //Magius(CHE)
			if (server_data.trade_system!=0) server_data.trade_system=1; // Magiu(CHE)
		}
		else if(!(strcmp(script1,"RANKSYSTEM"))) {
			server_data.rank_system=str2num(script2);
			if (server_data.rank_system!=0) server_data.rank_system=1; //Magius(CHE)
		}
		else if(!(strcmp( script1,"CHECKBANK"))) {
            server_data.CheckBank=str2num(script2);
		}
		else if(!(strcmp( script1,"SELLFORITEM"))) {
            server_data.defaultSelledItem=str2num(script2);
		}
		else return -1;
		return 0;
}

static int loadresources(char *script1, char *script2)
{
		if(!(strcmp(script1,"MINECHECK"))) server_data.minecheck=str2num(script2); // Moved by Magius(CHE)
		else if(!(strcmp(script1,"LOGS_PER_AREA"))) resource.logs=str2num(script2);
		else if(!(strcmp(script1,"LOG_RESPAWN_TIME"))) resource.logtime=str2num(script2);
		else if(!(strcmp(script1,"LOG_RESPAWN_RATE"))) resource.lograte=str2num(script2);
		else if(!(strcmp(script1,"LOG_RESPAWN_AREA"))) resource.logarea=str2num(script2);
		else if(!(strcmp(script1,"LOG_TRIGGER"))) resource.logtrigger=str2num(script2);
		else if(!(strcmp(script1,"LOG_STAMINA"))) resource.logstamina=str2num(script2);
		else if(!(strcmp(script1,"ORE_PER_AREA"))) ores.n=str2num(script2);
		else if(!(strcmp(script1,"ORE_AREA_WIDTH"))) ores.area_width=str2num(script2);
		else if(!(strcmp(script1,"ORE_AREA_HEIGHT"))) ores.area_height=str2num(script2);
		else if(!(strcmp(script1,"ORE_RESPAWN_TIME"))) ores.time=str2num(script2);
		else if(!(strcmp(script1,"ORE_RESPAWN_RATE"))) ores.rate=str2num(script2);
		else if(!(strcmp(script1,"ORE_STAMINA"))) ores.stamina=str2num(script2);
		else return -1;
		return 0;
}

static int loadrepsys(char *script1, char *script2) //Repsys
{
		if(!(strcmp(script1,"MURDER_DECAY"))) repsys.murderdecay=str2num(script2);
		else if(!(strcmp(script1,"MAXKILLS"))) repsys.maxkills=str2num(script2);
		else if(!(strcmp(script1,"CRIMINAL_TIME"))) repsys.crimtime=str2num(script2);
		else if(!(strcmp(script1,"ALLOWMAGICINTOWN"))) ServerScp::g_nAllowMagicInTown=str2num(script2);
		else if(!(strcmp(script1,"SNOOPWILLCRIMINAL"))) ServerScp::g_nSnoopWillCriminal=str2num(script2);
		else if(!(strcmp(script1,"STEALWILLCRIMINAL"))) ServerScp::g_nStealWillCriminal=str2num(script2);
		else if(!(strcmp(script1,"CHOPWILLCRIMINAL"))) ServerScp::g_nChopWillCriminal=str2num(script2);
		else if(!(strcmp(script1,"POLYMORPHWILLCRIMINAL"))) ServerScp::g_nPolymorphWillCriminal=str2num(script2);
		//Arakensh insta guard
		else if(!(strcmp(script1,"INSTANTGUARD"))) ServerScp::g_nInstantGuard=str2num (script2);
		////
		else if(!(strcmp(script1,"LOOTINGWILLCRIMINAL"))) ServerScp::g_nLootingWillCriminal=str2num(script2);
		else if(!(strcmp(script1,"HELPINGGREYWILLCRIMINAL"))) ServerScp::g_nHelpingGreyWillCriminal=str2num(script2);
		else if(!(strcmp(script1,"HELPINGCRIMINALWILLCRIMINAL"))) ServerScp::g_nHelpingCriminalWillCriminal=str2num(script2);
		else if(!(strcmp(script1,"GREYCANBELOOTED"))) ServerScp::g_nGreyCanBeLooted=str2num(script2);
		else if(!(strcmp(script1,"SNOOPKARMALOSS"))) ServerScp::g_nSnoopKarmaLoss=str2num(script2);
		else if(!(strcmp(script1,"STEALKARMALOSS"))) ServerScp::g_nStealKarmaLoss=str2num(script2);
		else if(!(strcmp(script1,"SNOOPFAMELOSS"))) ServerScp::g_nSnoopFameLoss=str2num(script2);
		else if(!(strcmp(script1,"STEALFAMELOSS"))) ServerScp::g_nStealFameLoss=str2num(script2);
		else if(!(strcmp(script1,"CHOPKARMALOSS"))) ServerScp::g_nChopKarmaLoss=str2num(script2);
		else if(!(strcmp(script1,"CHOPFAMELOSS"))) ServerScp::g_nChopFameLoss=str2num(script2);
		else if(!(strcmp(script1,"BOUNTYKARMAGAIN"))) ServerScp::g_nBountyKarmaGain=str2num(script2);
		else if(!(strcmp(script1,"BOUNTYFAMEGAIN"))) ServerScp::g_nBountyFameGain=str2num(script2);
		else if(!(strcmp(script1,"ENABLEKARMALOCK"))) ServerScp::g_nEnableKarmaLock=str2num(script2);
		else if(!(strcmp(script1,"KARMAUNLOCKPRAYER")))
		{
			ServerScp::g_strKarmaUnlockPrayer=new char[strlen(script2)+1];
			strcpy(ServerScp::g_strKarmaUnlockPrayer,script2);
		}
		else return -1;

		if (!repsys.murderdecay) { repsys.murderdecay=420; return -2; }
		if (!repsys.maxkills) { repsys.maxkills=4; return -2; }
		if (!repsys.crimtime) { repsys.crimtime=120; return -2; }
		return 0;
}

static int loadserver(char *script1, char *script2)
{
		if(	!strcmp(script1,"ARCHIVEPATH"))			server_data.archivePath = script2;
		else if(!strcmp(script1,"SAVEPATH"))			server_data.savePath = script2;
		else if(!strcmp(script1,"CHARACTERWORLDFILE"))		server_data.characterWorldfile = script2;
		else if(!strcmp(script1,"ITEMWORLDFILE"))		server_data.itemWorldfile = script2;
		else if(!strcmp(script1,"GUILDWORLDFILE"))		server_data.guildWorldfile = script2;
		else if(!strcmp(script1,"JAILWORLDFILE"))		server_data.jailWorldfile = script2;
		else if(!strcmp(script1,"BOOKWORLDFILE"))		server_data.bookWorldfile = script2;
		else if(!strcmp(script1,"WORLDFILEEXTENSION"))		server_data.worldfileExtension = script2;
		else if(!strcmp(script1,"SKILLCAP"))			server_data.skillcap=str2num(script2);
		else if(!strcmp(script1,"ENABLEBOOKS"))			ServerScp::g_nEnableBooks = str2num(script2);
		else if(!strcmp(script1,"UNEQUIPONREEQUIP"))		ServerScp::g_nUnequipOnReequip = str2num(script2);
		else if(!(strcmp(script1,"SHOWPCNAMES")))		ServerScp::g_nShowPCNames=str2num(script2);
		else if(!strcmp(script1,"EQUIPONDCLICK"))		ServerScp::g_nEquipOnDclick = str2num(script2);
		else if(!strcmp(script1,"DAILYSTATCAP"))		ServerScp::g_nStatDailyLimit =str2num(script2);
		else if(!strcmp(script1,"BANKITEMLIMIT"))		ServerScp::g_nBankLimit=str2num(script2);
		else if(!strcmp(script1,"ADDCOMMANDHEX"))		server_data.always_add_hex=str2num(script2); //Endymion
		else if(!strcmp(script1,"CSSOVERRIDES"))		ServerScp::g_css_override_case_sensitive=str2num(script2); //Endymion
		else if(!strcmp(script1,"SPARRINGLIMIT"))		ServerScp::g_nLimitPlayerSparring=str2num(script2);
		else if(!strcmp(script1,"LIMITEROLENUMBERS"))		ServerScp::g_nLimitRoleNumbers=str2num(script2);
		else if(!strcmp(script1,"PLAYERCANDELETEROLES"))	ServerScp::g_nPlayersCanDeleteRoles=str2num(script2);
		else if(!strcmp(script1,"SHOVESTAMINALOSS"))		ServerScp::g_nShoveStmDamage=str2num(script2);
		else if(!strcmp(script1,"STATCAP"))			server_data.statcap=str2num(script2);
		else if(!strcmp(script1,"ENABLEFEATURE"))		server_data.feature=str2num(script2);
		else if(!strcmp(script1,"POPUPHELP"))			ServerScp::g_nPopUpHelp=str2num(script2);
		else if(!strcmp(script1,"SKILLADVANCEMODIFIER"))
		{
			int mod=str2num(script2);
            		server_data.skilladvancemodifier=(mod>=1 ? mod : 1);    // must be nonzero && positive
		}
        	else if(!strcmp(script1,"STATSADVANCEMODIFIER"))
		{
            		int mod=str2num(script2);
            		server_data.statsadvancemodifier=(mod>=1 ? mod : 1);    // must be nonzero && positive
		}
		else if(!(strcmp(script1,"STATSADVANCESYSTEM"))) ServerScp::g_nStatsAdvanceSystem=str2num(script2);
		else if(!strcmp(script1,"USESPECIALBANK")) 		server_data.usespecialbank=str2num(script2); //AntiChrist - Special Bank
		else if(!strcmp(script1,"SPECIALBANKTRIGGER"))
		{
			strcpy(server_data.specialbanktrigger,script2);
			for(unsigned int i=0; i<strlen(server_data.specialbanktrigger);i++)
				server_data.specialbanktrigger[i]=toupper(server_data.specialbanktrigger[i]);
		}
		else if(!strcmp(script1,"LANGUAGE"))			//N6 - Unicode language font
		{
			strcpy(server_data.Unicodelanguage,script2);
			for(int i=0; i<3;i++)
				server_data.Unicodelanguage[i]=toupper(server_data.Unicodelanguage[i]);//Uppercase please
				server_data.Unicodelanguage[3]=0;//cut to 3 letters
		}
		else if(!strcmp(script1,"STAMINAUSAGE"))
		{
			sscanf(script2,"%f %f %f %f %f %f",&ServerScp::g_fStaminaUsage[0],&ServerScp::g_fStaminaUsage[1],&ServerScp::g_fStaminaUsage[2],&ServerScp::g_fStaminaUsage[3],&ServerScp::g_fStaminaUsage[4],&ServerScp::g_fStaminaUsage[5]);
			int x;
			for(x=0;x<6;x++) if(ServerScp::g_fStaminaUsage[x]<=0)
			{
				ServerScp::g_fStaminaUsage[x]=0.000001f;
				WarnOut("g_fStaminaUsage[%i] is invalid! Setting to 0.000001.\nPlease check STAMINAUSAGE in server.cfg\n",x);
			}
			
		}
		else if(!strcmp(script1,"STAMINALOSSONHORSE"))	server_data.staminaonhorse=atof(script2);
		else if(!strcmp(script1,"GOLDWEIGHT"))			server_data.goldweight=atof(script2);
		else if(!strcmp(script1,"DECAYTIMER"))			server_data.decaytimer=str2num(script2);
		else if(!strcmp(script1,"HEARTBEAT"))			heartbeat=str2num(script2)!=0;
		else if(!strcmp(script1,"PLAYERCORPSEDECAYMULTIPLIER"))	server_data.playercorpsedecaymultiplier=str2num(script2);//JM
		else if(!strcmp(script1,"LOOTDECAYSWITHCORPSE")) 	server_data.lootdecayswithcorpse=str2num(script2);
		else if(!strcmp(script1,"DISABLEZCHECKING")) 	server_data.disable_z_checking=str2num(script2);
		else if(!strcmp(script1,"INVISTIMER"))			server_data.invisibiliytimer=str2num(script2);
		else if(!strcmp(script1,"SKILLDELAY"))			server_data.skilldelay=str2num(script2);
		else if(!strcmp(script1,"SKILLLEVEL"))
		{
			server_data.skilllevel=str2num(script2); //MAgius(CHE)
			if (server_data.skilllevel<1 || server_data.skilllevel>10) server_data.skilllevel=5;
		}
		else if(!strcmp(script1,"OBJECTDELAY"))			server_data.objectdelay=str2num(script2);
		else if(!strcmp(script1,"SNOOPDELAY"))			server_data.snoopdelay=str2num(script2);
		else if(!strcmp(script1,"BANDAGEDELAY"))		server_data.bandagedelay=str2num(script2);
		else if(!strcmp(script1,"BANDAGEINCOMBAT"))		server_data.bandageincombat=str2num(script2);
		else if(!strcmp(script1,"INACTIVITYTIMEOUT"))		server_data.inactivitytimeout=str2num(script2);
		else if(!strcmp(script1,"GATETIMER"))			server_data.gatetimer=str2num(script2);
		else if(!strcmp(script1,"SHOWDEATHANIM"))		server_data.showdeathanim=str2num(script2);
		else if(!strcmp(script1,"GUARDSACTIVE"))		server_data.guardsactive=str2num(script2);
		else if(!strcmp(script1,"ANNOUNCE_WORLDSAVES"))		server_data.announceworldsaves=str2num(script2);
		else if(!strcmp(script1,"BG_SOUNDS"))			server_data.bg_sounds=str2num(script2);
		else if(!strcmp(script1,"POISONTIMER"))			server_data.poisontimer=str2num(script2); // lb
		else if(!strcmp(script1,"JOINMSG"))			server_data.joinmsg=str2num(script2);
		else if(!strcmp(script1,"PARTMSG"))			server_data.partmsg=str2num(script2);
		else if(!strcmp(script1,"SERVER_LOG"))			server_data.server_log=str2num(script2);
		else if(!strcmp(script1,"SPEECH_LOG"))			server_data.speech_log=str2num(script2);
		else if(!strcmp(script1,"PVP_LOG"))			server_data.pvp_log=str2num(script2);
		else if(!strcmp(script1,"GM_LOG"))			server_data.gm_log=str2num(script2);
		else if(!strcmp(script1,"LOG_PATH"))			strcpy(server_data.log_path, script2);
		else if(!strcmp(script1,"SAVESPAWNREGIONS"))		server_data.savespawns=str2num(script2);
		else if(!strcmp(script1,"ROGUE"))			server_data.rogue=str2num(script2);
		else if(!strcmp(script1,"CHAR_TIME_OUT"))		server_data.quittime=str2num(script2);//Instalog
		else if(!strcmp(script1,"MAXSTEALTHSTEPS"))		server_data.maxstealthsteps=str2num(script2);//AntiChrist
		else if(!strcmp(script1,"STEALTHONHORSE"))		ServerScp::g_nStealthOnHorse=str2num(script2);//AntiChrist
		else if(!strcmp(script1,"STEALTHARLIMIT"))		ServerScp::g_nStealthArLimit=str2num(script2);//AntiChrist
		else if(!strcmp(script1,"RUNNINGSTAMINASTEPS"))		server_data.runningstaminasteps=str2num(script2);//AntiChrist
		else if(!strcmp(script1,"BOAT_SPEED"))			server_data.boatspeed=(float)atof(script2);//Boats
		else if(!strcmp(script1,"STABLING_FEE"))		server_data.stablingfee=(float)atof(script2);//Boats
		else if(!strcmp(script1,"HTML"))			server_data.html=str2num(script2);//HTML
        else if(!strcmp(script1,"CUT_SCROLL_REQUIREMENTS"))	server_data.cutscrollreq=str2num(script2);//AntiChrist
        else if(!strcmp(script1,"PERSECUTION"))			server_data.persecute=str2num(script2);//AntiChrist
		else if(!strcmp(script1,"AUTO_CREATE_ACCTS"))		server_data.auto_a_create = str2num( script2 );
		else if(!strcmp(script1,"CHECKCHARAGE"))		server_data.checkcharage = str2num( script2 );
		else if(!strcmp(script1,"AUTO_RELOAD_ACCTS"))		server_data.auto_a_reload = str2num( script2 );
		else if(!strcmp(script1,"MSGBOARDPATH"))		strcpy(server_data.msgboardpath,script2);
		else if(!strcmp(script1,"MSGPOSTACCESS"))		server_data.msgpostaccess=str2num(script2);
		else if(!strcmp(script1,"MSGPOSTREMOVE"))		server_data.msgpostremove=str2num(script2);
		else if(!strcmp(script1,"MSGRETENTION"))		server_data.msgretention=str2num(script2);
		else if(!strcmp(script1,"ESCORTACTIVE"))		server_data.escortactive=str2num(script2);
		else if(!strcmp(script1,"ESCORTINITEXPIRE"))		server_data.escortinitexpire=str2num(script2);
		else if(!strcmp(script1,"ESCORTACTIVEEXPIRE"))		server_data.escortactiveexpire=str2num(script2);
		else if(!strcmp(script1,"ESCORTDONEEXPIRE"))		server_data.escortdoneexpire=str2num(script2);
		else if(!strcmp(script1,"BOUNTYSACTIVE"))		server_data.bountysactive=str2num(script2);
		else if(!strcmp(script1,"BOUNTYSEXPIRE"))		server_data.bountysexpire=str2num(script2);
		else if(!strcmp(script1,"TAMED_DISAPPEAR"))		server_data.tamed_disappear=str2num(script2);
		else if(!strcmp(script1,"HOUSEINTOWN"))			server_data.houseintown=str2num(script2);
		else if(!strcmp(script1,"SHOPRESTOCK"))			server_data.shoprestock=str2num(script2);
		else if(!strcmp(script1, "COMMANDPREFIX"))		server_data.commandPrefix=script2[0];
		else if(!strcmp(script1,"ERRORS_TO_CONSOLE"))		server_data.errors_to_console=str2num( script2 );
		else if(!strcmp(script1,"HOUSEDECAY_SECS"))		server_data.housedecay_secs=str2num( script2 );
		else if(!strcmp(script1,"BEGGING_TIME"))		begging_data.timer=str2num(script2);
		else if(!strcmp(script1,"BEGGING_RANGE"))		begging_data.range=str2num(script2);
		else if(!strcmp(script1,"BEGGING_TEXT0"))		strcpy(begging_data.text[0],script2);
		else if(!strcmp(script1,"BEGGING_TEXT1"))		strcpy(begging_data.text[1],script2);
		else if(!strcmp(script1,"BEGGING_TEXT2"))		strcpy(begging_data.text[2],script2);
		else if(!strcmp(script1,"BASE_FISHING_TIME"))		fishing_data.basetime=str2num(script2);
		else if(!strcmp(script1,"RANDOM_FISHING_TIME"))		fishing_data.randomtime=str2num(script2);
		else if(!strcmp(script1,"SPIRITSPEAKTIMER"))		spiritspeak_data.spiritspeaktimer=str2num(script2);
		else return -1;
		return 0;
}



static int loadtime_light(char *script1, char *script2)
{
		if(!(strcmp(script1,"ABSDAY"))) day=str2num(script2);
		else if(!(strcmp(script1,"MOON1UPDATE"))) moon1update=str2num(script2);
		else if(!(strcmp(script1,"MOON2UPDATE"))) moon2update=str2num(script2);
		else if(!(strcmp(script1,"MOON1"))) moon1=str2num(script2);
		else if(!(strcmp(script1,"MOON2"))) moon2=str2num(script2);
		else if(!(strcmp(script1,"DUNGEONLIGHTLEVEL"))) dungeonlightlevel=str2num(script2);
		else if(!(strcmp(script1,"WORLDFIXEDLEVEL"))) worldfixedlevel=str2num(script2);
		else if(!(strcmp(script1,"WORLDCURLEVEL"))) worldcurlevel=str2num(script2);
		else if(!(strcmp(script1,"WORLDBRIGHTLEVEL"))) worldbrightlevel=str2num(script2);
		else if(!(strcmp(script1,"WORLDDARKLEVEL"))) worlddarklevel=str2num(script2);
		else if(!(strcmp(script1,"SECONDSPERUOMINUTE"))) secondsperuominute=str2num(script2);
		else if(!(strcmp(script1,"SEASON"))) season=str2num(script2);
		else if(!(strcmp(script1,"DAY"))) Calendar::g_nDay=str2num(script2);
		else if(!(strcmp(script1,"HOUR"))) Calendar::g_nHour=str2num(script2);
		else if(!(strcmp(script1,"MINUTE"))) Calendar::g_nMinute=str2num(script2);
		else if(!(strcmp(script1,"MONTH"))) Calendar::g_nMonth=str2num(script2);
		else if(!(strcmp(script1,"WEEKDAY"))) Calendar::g_nWeekday=str2num(script2);
		else if(!(strcmp(script1,"YEAR"))) Calendar::g_nYear=str2num(script2);
		else if(!(strcmp(script1,"OVERRIDEWINTERSEASON"))) Calendar::g_nWinterOverride=str2num(script2);
		else return -1;
		return 0;
}

static int loadnxwoptions (char *script1, char *script2)
{
	if(!(strcmp(script1,"ENABLEAMX"))) ServerScp::g_nEnableAMXScripts=str2num(script2);
/*#ifndef _WINDOWS
	else if(!(strcmp(script1,"DEBUGAMX"))) ServerScp::g_nLoadDebugger=str2num(script2);
#endif*/
	else if(!(strcmp(script1,"CHECKBYSMALL"))) ServerScp::g_nCheckBySmall|=(str2num(script2)!=0); // |= becuase can be also activated by arg -check
	else if(!(strcmp(script1,"USEINTERNALCOMPILER"))) ServerScp::g_bEnableInternalCompiler=(str2num(script2)!=0);
	else if(!(strcmp(script1,"MAPHEIGHT"))) map_height=str2num(script2);
	else if(!(strcmp(script1,"MAPWIDTH"))) map_width=str2num(script2);
	else if(!(strcmp(script1,"AUTODETECTMULS"))) ServerScp::g_nAutoDetectMuls=str2num(script2);
	else if(!(strcmp(script1,"LINEBUFFER"))) ServerScp::g_nLineBuffer=str2num(script2);
	else if(!(strcmp(script1,"USEACCOUNTENCRYPTION"))) ServerScp::g_nUseAccountEncryption=str2num(script2);
	else if(!(strcmp(script1,"VERBOSECRONTAB"))) ServerScp::g_nVerboseCrontab=str2num(script2);
	else if(!(strcmp(script1,"CHARMODE"))) ServerScp::g_nUseCharByCharMode=str2num(script2);
	else if(!(strcmp(script1,"PORT"))) ServerScp::g_nRacTCPPort=str2num(script2);
	else if(!(strcmp(script1,"DISABLEFIELDRESISTANCERAISE"))) ServerScp::g_nDisableFieldResistanceRaise=str2num(script2);
	else if(!(strcmp(script1,"ADVENTUREMODE"))) ServerScp::g_nAdventureMode=str2num(script2);
	else if(!(strcmp(script1,"SHOWPARALISYSRESISTANCEINTITLE"))) ServerScp::g_nShowParalisysResistanceInTitle=str2num(script2);
	else if(!(strcmp(script1,"SHOWPOISONRESISTANCEINTITLE"))) ServerScp::g_nShowPoisonResistanceInTitle=str2num(script2);
	else if(!(strcmp(script1,"SHOWFIRERESISTANCEINTITLE"))) ServerScp::g_nShowFireResistanceInTitle=str2num(script2);
	else if(!(strcmp(script1,"IGNOREWETBIT"))) ServerScp::g_nWalkIgnoreWetBit=str2num(script2);
	else if(!(strcmp(script1,"PARTYSHAREKARMA"))) Partys.shareKarma=str2num(script2);
	else if(!(strcmp(script1,"PARTYSHAREFAME"))) Partys.shareFame=str2num(script2);
	else if(!(strcmp(script1,"ENABLEPRIVATEMSG"))) Partys.canPMsg=str2num(script2);
	else if(!(strcmp(script1,"ENABLEPUBLICMSG"))) Partys.canBroadcast=str2num(script2);
	else if(!(strcmp(script1,"CONSOLEONFILE"))) ServerScp::g_nRedirectOutput=str2num(script2);
	else if(!(strcmp(script1,"STDOUTFILE"))) strcpy(ServerScp::g_szOutput,script2);
	else if(!(strcmp(script1,"NEWNPCMAGIC"))) ServerScp::g_nUseNewNpcMagic=str2num(script2);
	else if(!(strcmp(script1,"DEAMONMODE"))) ServerScp::g_nDeamonMode=str2num(script2);
	else if(!(strcmp(script1,"INITIALGOLD"))) goldamount=str2num(script2);
	else if(!(strcmp(script1,"INITIALPRIV1"))) defaultpriv1=str2num(script2);
	else if(!(strcmp(script1,"INITIALPRIV2"))) defaultpriv2=str2num(script2);
	else return -1;

#ifdef _WINDOWS
		if (ServerScp::g_nDeamonMode) {
			MessageBox(NULL, "Background mode is not supported in GUI mode.", "Warning",MB_ICONINFORMATION);
			ServerScp::g_nDeamonMode = 0;
		}
#endif
#ifdef _CONSOLE
		ServerScp::g_nDeamonMode = (g_bNTService) ? 1 : 0;
#endif
	return 0;
}


static int loadinioptions (char *script1, char *script2)
{
	if(!(strcmp(script1,"NAME"))) strcpy(serv[0][0], script2);
	else if(!(strcmp(script1,"IP"))) {strcpy(serv[0][1], script2);strcpy(saveip, script2);}
	else if(!(strcmp(script1,"PORT"))) strcpy(serv[0][2], script2);
	else if(!(strcmp(script1,"AUTODETECTIP"))) ServerScp::g_nAutoDetectIP=str2num(script2);
	else if(!(strcmp(script1,"BEHINDNAT"))) ServerScp::g_nBehindNat=str2num(script2);
	else if(!(strcmp(script1,"MAP"))) strcpy(temp_map, script2);
	else if(!(strcmp(script1,"MAP_CACHE"))) map_cache = str2num( script2 );
	else if(!(strcmp(script1,"STATICS"))) strcpy(temp_statics, script2);
	else if(!(strcmp(script1,"STAIDX"))) strcpy(temp_staidx, script2);
	else if(!(strcmp(script1,"STATICS_CACHE"))) statics_cache = str2num( script2 );
	else if(!(strcmp(script1,"VERDATA"))) strcpy(temp_verdata, script2);
	else if(!(strcmp(script1,"TILEDATA"))) strcpy(temp_tiledata, script2);
	else if(!(strcmp(script1,"MULTIMUL"))) strcpy(temp_multimul, script2);
	else if(!(strcmp(script1,"MULTIIDX"))) strcpy(temp_multiidx, script2);
	else return INVALID;
	return 0;
}

extern bool g_bInMainCycle;

static int loadstartlocs (char *script1, char *script2)
{
	static bool bCompleted = false;

	if (g_bInMainCycle) return 0;

	if (strlen(script2)<3) return 0;

	if (bCompleted) {
		ConOut("Attempt to load more than 9 starting locations. Further locations will be ignored\n");
		return -13;
	}

	if(!(strcmp(script1,"CITY"))) strcpy(start[startcount][0], script2);
	else if(!(strcmp(script1,"PLACE"))) strcpy(start[startcount][1], script2);
	else if(!(strcmp(script1,"POSITION")))
	{
		int i, j=0, k = 2, ln = strlen(script2);
		//set defaults to 0,0,0 to avoid parsing errors :)
		start[startcount][2][0]=start[startcount][3][0]=start[startcount][4][0] = '0';
		start[startcount][2][0]=start[startcount][3][0]=start[startcount][4][0] = '\0';

		for (i=0; i<=ln; i++) {
			if (script2[i]=='\0') break;
			if (script2[i]==',')
			{
				start[startcount][k][j] = '\0';
				k++;
				j = 0;
			}
			else start[startcount][k][j++] = script2[i];
		}

		if ((++startcount)==9) bCompleted = true;
	} else return -1;
	return 0;
}

static int loadPreloads(char *script1, char *script2) // by Xan
{
		if(!(strcmp(script1,"CONSOLEONFILE"))) ServerScp::g_nRedirectOutput=str2num(script2);
		else if(!(strcmp(script1,"STDOUTFILE"))) strcpy(ServerScp::g_szOutput,script2);
		else return 0; //never error on preloads!
		return 0;

}


void preloadSections(char *fn)
{
	int loopexit=0;
	wscfile=fopen(fn, "r");

	if(wscfile==NULL) return;

	do
	{
		readw2();
		if (!(strcmp(script1, "SECTION"))) {
			if(!(strcmp(script2, "PRELOAD"))) {
				chooseSection(script2, parseSection);
			}
		}
	}
	while ( (strcmp(script1, "EOF")) && (++loopexit < MAXLOOPS) );
	fclose(wscfile);
}

void loadshardlist()
{
	FILE *F = fopen("custom/shards.cfg", "rt");
	
	ConOut("Loading Shards list...");
	if(!F) {
		ConOut(" [ KO ]\n");
		return;
	}
	do
	{
		if(servcount >= MAXSERV) {
			ConOut(" [FULL]\n");
			fclose(F);
			return;
		}
		
		do
		{
			readSplitted(F, script1, script2);
			if (!strcmp(script1, "NAME")) {
				strncpy(serv[servcount][0], script2, 30);
			} else if (!strcmp(script1, "IP")){
				strncpy(serv[servcount][1], script2, 30);
			} else if (!strcmp(script1, "PORT")) {
				strncpy(serv[servcount][2], script2, 30);
			}
		}
		while ( (!feof(F)) &&
			(strcmp(script1, "}")) &&
			(strcmp(script1, "EOF")) );
		if (!strcmp(script1, "}"))
			servcount++;
	}while (!feof(F));
	fclose(F);
	ConOut(" [ OK ]\n");
}

void loadserverscript(char *fn) // Load a server script
{
	int loopexit=0;
	wscfile=fopen(fn, "r");
	ConOut("Loading server settings [%s]...", fn);
	if(wscfile==NULL)
	{
		ConOut(" [FAIL]\n",fn);
		return;
	}
	do
	{
		readw2();
		if (!(strcmp(script1, "SECTION")))
		{
			if(!(strcmp(script2, "CLIENTS_ALLOWED"))) loadclientsallowed();
			else chooseSection(script2, parseSection);
			// end addons by Magius(CHE)
		}
	}
	while ( (strcmp(script1, "EOF")) && (++loopexit < MAXLOOPS) );
	fclose(wscfile);
	ConOut(" [ OK ]\n",fn);
}

void loadserverscript() // Load server script
{
	loadserverscript("config/server.cfg");
	loadserverscript("custom/server.cfg");
	servcount=1;		//In NoX-Wizard you can have ONLY one server running :)
				// ^-- Are you sure !? ;) default shard is the first =)

	loadshardlist();	// and now .. the others ... =)
}

void commitserverscript() // second phase setup
{
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	if (startcount<9)
	{
		if (startcount==0)
		{   //default starting locz
			ConOut("\n  Warning no starting locations... defaulting to classic ones!\n");
			strcpy(start[startcount][0], "Yew");
			strcpy(start[startcount][1], "The Sturdy Bow");
			strcpy(start[startcount][2], "567");
			strcpy(start[startcount][3], "978");
			strcpy(start[startcount][4], "0");
			startcount++;
			strcpy(start[startcount][0], "Minoc");
			strcpy(start[startcount][1], "The Barnacle Tavern");
			strcpy(start[startcount][2], "2477");
			strcpy(start[startcount][3], "407");
			strcpy(start[startcount][4], "15");
			startcount++;
			strcpy(start[startcount][0], "Britain");
			strcpy(start[startcount][1], "Sweet Dreams Inn");
			strcpy(start[startcount][2], "1496");
			strcpy(start[startcount][3], "1629");
			strcpy(start[startcount][4], "10");
			startcount++;
			strcpy(start[startcount][0], "Moonglow");
			strcpy(start[startcount][1], "The Scholars Inn");
			strcpy(start[startcount][2], "4404");
			strcpy(start[startcount][3], "1169");
			strcpy(start[startcount][4], "0");
			startcount++;
			strcpy(start[startcount][0], "Trinsic");
			strcpy(start[startcount][1], "The Traveller's Inn");
			strcpy(start[startcount][2], "1844");
			strcpy(start[startcount][3], "2745");
			strcpy(start[startcount][4], "0");
			startcount++;
			strcpy(start[startcount][0], "Magincia");
			strcpy(start[startcount][1], "The Great Horns Tavern");
			strcpy(start[startcount][2], "3738");
			strcpy(start[startcount][3], "2223");
			strcpy(start[startcount][4], "20");
			startcount++;
			strcpy(start[startcount][0], "Jhelom");
			strcpy(start[startcount][1], "The Morning Star Inn");
			strcpy(start[startcount][2], "1378");
			strcpy(start[startcount][3], "3817");
			strcpy(start[startcount][4], "0");
			startcount++;
			strcpy(start[startcount][0], "Skara Brae");
			strcpy(start[startcount][1], "The Falconers Inn");
			strcpy(start[startcount][2], "594");
			strcpy(start[startcount][3], "2227");
			strcpy(start[startcount][4], "0");
			startcount++;
			strcpy(start[startcount][0], "Vesper");
			strcpy(start[startcount][1], "The Ironwood Inn");
			strcpy(start[startcount][2], "2771");
			strcpy(start[startcount][3], "977");
			strcpy(start[startcount][4], "0");
			startcount++;
		}
		ConOut("\n  Warning, insufficient starting locations... padding...\n");
		for (; startcount<9; startcount++)
		{
			strcpy(start[startcount][0], start[0][0]);
			strcpy(start[startcount][1], start[0][1]);
			strcpy(start[startcount][2], start[0][2]);
			strcpy(start[startcount][3], start[0][3]);
			strcpy(start[startcount][4], start[0][4]);
		}
	}

	data::setPath( Map_File, std::string( temp_map ) );
	data::setPath( StaIdx_File, std::string( temp_staidx ) );
	data::setPath( Statics_File, std::string( temp_statics ) );
	data::setPath( VerData_File, std::string( temp_verdata ) );
	data::setPath( TileData_File, std::string( temp_tiledata ) );
	data::setPath( Multi_File, std::string( temp_multimul ) );
	data::setPath( MultiIdx_File, std::string( temp_multiidx ) );

		// name resovling of server-address, LB 7-JULY 2000

	unsigned int i;
	unsigned long ip;
	sockaddr_in m_sin;
	hostent *hpe;
	char *name;

    m_sin.sin_family = AF_INET;

	for (i = 0; i < servcount; i++)
	{
		ip = inet_addr(serv[i][1]);

		if (ip == INADDR_NONE) // adresse-name instead of ip adress given ! trnslate to ip string
		{
			name = serv[i][1];
			ConOut("host: %s\n", name);
			hpe = gethostbyname(name);

			if (hpe == NULL)
			{
#if	defined(__unix__)
				// We should be able to use the xti error functions, cant find them so...
				// sprintf(temp,"warning: %d resolving name: %s\n", t_srerror(t_errno),name) ;
#ifndef __BEOS__
				sprintf(temp, "warning: Error desolving name: %s : %s\n", name, hstrerror(h_errno));
#endif
#else
				sprintf(temp, "warning: %d resolving name: %s\n", WSAGetLastError(), name);
#endif
				LogWarning(temp);
				LogWarning("switching to localhost\n");

				strcpy(serv[i][1], "127.0.0.1");
			}
			else
			{
				memcpy(&(m_sin.sin_addr), hpe->h_addr, hpe->h_length);

				strncpy(serv[i][1], inet_ntoa(m_sin.sin_addr), 28);
				serv[i][1][29] = 0;
			} // end else resolvable
		}
	} // end server loop

}

void saveserverscript()
{
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	FILE *file;
	file=fopen("config/server.cfg", "w");
	if(!file) return; //only write can be..
	const char * t;
  std::vector<std::string>::const_iterator vis( clientsAllowed.begin() ), vis_end( clientsAllowed.end() );

	fprintf(file, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(file, "// || NoX-Wizard settings (server.cfg)                                    ||\n");
	//fprintf(file, "// || Automatically generated on worldsaves                               ||\n");
	fprintf(file, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(file, "// || Generated by NoX-Wizard version %s %s               ||\n", VERNUMB, OSFIX);
	fprintf(file, "// || Requires NoX-Wizard version %s to be read correctly              ||\n", SCPREQVER);
	fprintf(file, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(file, "// || This script is NOT compatible with 0.70 script format               ||\n");
	fprintf(file, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n\n");

	fprintf(file, "SECTION SHARD\n");
	fprintf(file, "{\n");
	fprintf(file, "// The name of your shard \n");
	fprintf(file, "NAME %s\n", serv[0][0]); // lb
	fprintf(file, "// The IP address of your shard. Ignored if AUTODETECTIP is on. \n");
	fprintf(file, "IP %s\n", saveip); // lb
	fprintf(file, "// The TCP port the server will listen to. \n");
	fprintf(file, "PORT %s\n", serv[0][2]); // lb
	fprintf(file, "// Set this to enable plug&play configuration of the network address \n");
	fprintf(file, "// Do not enable this if you want to create a login server or if you have \n");
	fprintf(file, "// particular security reasons about not enabling it! \n");
	fprintf(file, "AUTODETECTIP %i\n", ServerScp::g_nAutoDetectIP); // lb
	fprintf(file, "BEHINDNAT %i\n", ServerScp::g_nBehindNat); // lb
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION MULFILES\n");
	fprintf(file, "{\n");
	fprintf(file, "// In this section you have to define paths to mul files.\n");
	fprintf(file, "// You may skip this section if you use a Windows computer on which UO T2A or\n");
	fprintf(file, "// Renaissance is installed and AUTODETECTMULS is defined under the Windows section (this is the default)\n");
	fprintf(file, "// \n");
	fprintf(file, "// The file containing the map, usually map0.mul for T2A client and Britannia on UO3D,\n");
	fprintf(file, "// map2.mul for Ilshenar map on UO Third Dawn.\n");
	fprintf(file, "MAP %s\n", data::getPath( Map_File ).c_str());
	fprintf(file, "//  Setting to 1 loads the map's info into RAM. Warning: requires A LOT of memory!\n");
	fprintf(file, "MAP_CACHE %i\n", map_cache );
	fprintf(file, "// The files containing the statics, usually statics0.mul and staidx0.mul for T2A client\n");
	fprintf(file, "// and Britannia on UO3D, statics2.mul and staidx2.mul for Ilshenar map on UO Third Dawn.\n");
	fprintf(file, "STATICS %s\n", data::getPath( Statics_File ).c_str());
	fprintf(file, "STAIDX %s\n", data::getPath( StaIdx_File ).c_str());
	fprintf(file, "//  Setting to 1 loads the statics's info into RAM, significant speed boost\n");
	fprintf(file, "STATICS_CACHE %i\n", statics_cache );
	fprintf(file, "// The version data you'll use (verdata.mul, usually)\n");
	fprintf(file, "VERDATA %s\n", data::getPath( VerData_File ).c_str());
	fprintf(file, "// The tile data you'll use (tiledata.mul, usually)\n");
	fprintf(file, "TILEDATA %s\n", data::getPath( TileData_File ).c_str());
	fprintf(file, "// The files containing multi data (multi.mul and multi.idx usually)\n");
	fprintf(file, "MULTIMUL %s\n", data::getPath( Multi_File ).c_str());
	fprintf(file, "MULTIIDX %s\n", data::getPath( MultiIdx_File ).c_str());
	fprintf(file, "}\n\n");


	fprintf(file, "SECTION SKILLS\n");
	fprintf(file, "{\n");
	fprintf(file, "// sets the skillcap for players \n");
	fprintf(file, "SKILLCAP %i\n",server_data.skillcap);
	fprintf(file, "// sets the statcap for players \n");
	fprintf(file, "STATCAP %i\n",server_data.statcap);
	fprintf(file, "// sets the maximum stat gain in a single day (or until a reset in crontab.cfg) \n");
	fprintf(file, "DAILYSTATCAP %i\n",ServerScp::g_nStatDailyLimit);
	fprintf(file, "// enable stat advance sistem 1 or not 0\n");
	fprintf(file, "STATSADVANCESYSTEM %i\n",ServerScp::g_nStatsAdvanceSystem);
	fprintf(file, "// the higher the value, the slower players will gain skill \n");
	fprintf(file, "SKILLADVANCEMODIFIER %i\n",server_data.skilladvancemodifier);
	fprintf(file, "// the higher the value, the slower players will gain stats (values between 1000\n");
	fprintf(file, "// -2000 resemble OSI standards) \n");
    fprintf(file, "STATSADVANCEMODIFIER %i\n",server_data.statsadvancemodifier);
	fprintf(file, "// This value multiplied by an item's minskill value indicates the maximum skill diff to create \n");
	fprintf(file, "SKILLLEVEL %i\n",server_data.skilllevel); // By Magius(CHE)
	fprintf(file, "// How close you have to be to beg \n");
	fprintf(file, "BEGGING_RANGE %i\n",begging_data.range);
	fprintf(file, "BEGGING_TIME %i\n",begging_data.timer);
	fprintf(file, "// Text players will say when using the begging skill\n");
	fprintf(file, "BEGGING_TEXT0 %s\n",begging_data.text[0]);
	fprintf(file, "BEGGING_TEXT1 %s\n",begging_data.text[1]);
	fprintf(file, "BEGGING_TEXT2 %s\n",begging_data.text[2]);
	fprintf(file, "// Minimum amount of time needed for fishing \n");
	fprintf(file, "BASE_FISHING_TIME %i\n",fishing_data.basetime);
	fprintf(file, "// Random amount of time on top of the base time \n");
	fprintf(file, "RANDOM_FISHING_TIME %i\n",fishing_data.randomtime);
	fprintf(file, "// How long Spirit Speak lasts for (in seconds) \n");
	fprintf(file, "SPIRITSPEAKTIMER %i\n",spiritspeak_data.spiritspeaktimer);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION BANK\n");
	fprintf(file, "{\n");
	fprintf(file, "// Special bank will give each character a second bank box, intention is to use\n");
	fprintf(file, "// the first one for gold, and the special one for items \n");
	fprintf(file, "USESPECIALBANK %i\n",server_data.usespecialbank);	//AntiChrist - specialbank
	fprintf(file, "// The Trigger can be any word. When a character types it in the game, the\n");
	fprintf(file, "// special bank box will be opened \n");
	fprintf(file, "SPECIALBANKTRIGGER %s\n",server_data.specialbanktrigger);
	fprintf(file, "// If not 0, limit the number of items in a bank box\n");
	fprintf(file, "BANKITEMLIMIT %d\n",ServerScp::g_nBankLimit);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION STAMINA\n");
	fprintf(file, "{\n");
	fprintf(file, "// Define how often stamina will be reduced by 1 when running \n");
	fprintf(file, "RUNNINGSTAMINASTEPS %i\n",server_data.runningstaminasteps);//Instalog
	fprintf(file, "// Stamina to loose when players shove something out of the way \n");
    fprintf(file, "SHOVESTAMINALOSS %i\n",ServerScp::g_nShoveStmDamage);
	fprintf(file, "// Defines how much stamina is used each step\n");
	fprintf(file, "// at <0-24> <25-49> <50-74> <75-99> <100> <over> per cent of maximum weight\n");
	fprintf(file, "STAMINAUSAGE %f %f %f %f %f %f\n",ServerScp::g_fStaminaUsage[0],ServerScp::g_fStaminaUsage[1],ServerScp::g_fStaminaUsage[2],ServerScp::g_fStaminaUsage[3],ServerScp::g_fStaminaUsage[4],ServerScp::g_fStaminaUsage[5]);
	fprintf(file, "// %% of stamina loss when riding ( es 0.5 is 50%% ) \n");
	fprintf(file, "STAMINALOSSONHORSE %f\n",server_data.staminaonhorse);//AntiChrist - gold weight
	fprintf(file, "// Weight of gold coins \n");
	fprintf(file, "GOLDWEIGHT %f\n",server_data.goldweight);//AntiChrist - gold weight
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION SKILLS\n");
	fprintf(file, "{\n");
	fprintf(file, "// 1= Enables Thievery, 2=Disables Thievery \n");
	fprintf(file, "ROGUE %i\n",server_data.rogue);
	fprintf(file, "// Sets the amount of steps a character can make with stealth until he becomes visible again \n");
	fprintf(file, "MAXSTEALTHSTEPS %i\n",server_data.maxstealthsteps);//Instalog
	fprintf(file, "// Allow stealth on horse \n");
	fprintf(file, "STEALTHONHORSE %i\n",ServerScp::g_nStealthOnHorse);//Instalog
	fprintf(file, "// Limit stealthwith AR \n");
	fprintf(file, "STEALTHARLIMIT %i\n",ServerScp::g_nStealthArLimit);//Instalog
	fprintf(file, "// 1= casting from scrolls requires less skill than casting from spellbook, 0 = no difference \n");
    fprintf(file, "CUT_SCROLL_REQUIREMENTS %i\n",server_data.cutscrollreq);//AntiChrist
	fprintf(file, "// 1= Allow using bandages while fighting, 0=Forbid usage of bandages while fighting \n");
	fprintf(file, "BANDAGEINCOMBAT %i\n",server_data.bandageincombat);
	fprintf(file, "// 0=Let skill raise freely with sparring otherwise defending player must have N skill points\n");
	fprintf(file, "// more than the attacker for the attacker to gain (in PvP only).\n");
	fprintf(file, "SPARRINGLIMIT %i\n",ServerScp::g_nLimitPlayerSparring);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION LOG\n");
	fprintf(file, "{\n");
	fprintf(file, "// 1= Activate server logging, 0=deactivate logging  \n");
	fprintf(file, "SERVER_LOG %i\n",server_data.server_log); //Lb, splitt log to those 4
	fprintf(file, "// 1= Activate speech logging, 0=deactivate logging \n");
	fprintf(file, "SPEECH_LOG %i\n",server_data.speech_log);
	fprintf(file, "// 1= Activate player combat logging, 0=deactivate logging  \n");
	fprintf(file, "PVP_LOG %i\n",server_data.pvp_log);
	fprintf(file, "// 1= Activate gm action logging, 0=deactivate logging  \n");
	fprintf(file, "GM_LOG %i\n",server_data.gm_log);
	fprintf(file, "// 1=Show errors in server console, 0=don't show them \n");
	fprintf(file, "ERRORS_TO_CONSOLE %i\n",server_data.errors_to_console);
	fprintf(file, "// path where the logs will be written, separator is '/', path must exists\n");
	fprintf(file, "// and subdirectories \"speech\" and \"GM\" must also exist\n");
	fprintf(file, "LOG_PATH logs/\n");
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION TIMERS\n");
	fprintf(file, "{\n");
	fprintf(file, "// Sets the decaying time (seconds) for items \n");
	fprintf(file, "DECAYTIMER %i\n",server_data.decaytimer);
	fprintf(file, "// Sets the duration (seconds) of poison \n");
	fprintf(file, "POISONTIMER %i\n",server_data.poisontimer); // lb
	fprintf(file, "// Number of seconds that a gate remains open  \n");
	fprintf(file, "GATETIMER %i\n",server_data.gatetimer);
	fprintf(file, "// Characters will be logged out when inactive for the specified time (seconds?) \n");
	fprintf(file, "INACTIVITYTIMEOUT %i\n",server_data.inactivitytimeout);
	fprintf(file, "// Defines how long (seconds) a character stays in the game when he logs out \n");
	fprintf(file, "CHAR_TIME_OUT %i\n",server_data.quittime);//Instalog
	fprintf(file, "// Define how often a boat moves \n");
    fprintf(file, "BOAT_SPEED %f\n",server_data.boatspeed);//Boats
	fprintf(file, "// Fee payed for stabled animals \n");
	fprintf(file, "STABLING_FEE %f\n",server_data.stablingfee);//Boats
	fprintf(file, "// Length of time (INGAME seconds) until a player house will decay - counter is cpu intensitive\n");
	fprintf(file, "// so it only gets updated every 11 minutes - opening a house door will reset counter and refresh the house\n");
	fprintf(file, "HOUSEDECAY_SECS %ld\n",server_data.housedecay_secs);
	fprintf(file, "// Sets the decaying time for character corpses. \n");
	fprintf(file, "// Formula: decaytimer * multiplier = decaying time for corpses \n");
	fprintf(file, "PLAYERCORPSEDECAYMULTIPLIER %i\n", server_data.playercorpsedecaymultiplier);
	fprintf(file, "// 1= loot decays with corpse, 0 = loot gets put on the ground when corpse decays \n");
	fprintf(file, "LOOTDECAYSWITHCORPSE %i\n", server_data.lootdecayswithcorpse);
	fprintf(file, "// Sets the duration (seconds) of the invisibility spell \n");
	fprintf(file, "INVISTIMER %i\n",server_data.invisibiliytimer);
	fprintf(file, "// Prevents permanent skill usage, Sets the time (sec) between allowed skill usages \n");
	fprintf(file, "SKILLDELAY %i\n",server_data.skilldelay);
	fprintf(file, "// Prevents permanent usage of objects, Sets the minimum time (seconds) between usage of objects \n");
	fprintf(file, "OBJECTDELAY %i\n",server_data.objectdelay);
	fprintf(file, "// Prevents heavy macroing of snooping, Sets the minimum time (seconds) between snoops \n");
	fprintf(file, "SNOOPDELAY %i\n",server_data.snoopdelay);
	fprintf(file, "// Prevents permanent healing, sets delay time (seconds) between bandage usage \n");
	fprintf(file, "BANDAGEDELAY %i\n",server_data.bandagedelay);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION SERVER\n");
	fprintf(file, "{\n");
	
	fprintf(file, "// Worldfile extension\n");
	fprintf(file, "WORLDFILEEXTENSION %s\n", server_data.worldfileExtension.c_str() );
	fprintf(file, "// Character worldfile name\n");
	fprintf(file, "CHARACTERWORLDFILE %s\n", server_data.characterWorldfile.c_str() );
	fprintf(file, "// Item worldfile name\n");
	fprintf(file, "ITEMWORLDFILE %s\n", server_data.itemWorldfile.c_str() );
	fprintf(file, "// Guild worldfile name\n");
	fprintf(file, "GUILDWORLDFILE %s\n", server_data.guildWorldfile.c_str() );
	fprintf(file, "// Jail worldfile name\n");
	fprintf(file, "JAILWORLDFILE %s\n", server_data.jailWorldfile.c_str() );
	fprintf(file, "// Book worldfile name\n");
	fprintf(file, "BOOKWORLDFILE %s\n", server_data.bookWorldfile.c_str() );
	fprintf(file, "// Set directory where worldfile saves will be stored\n");
	fprintf(file, "SAVEPATH %s\n", server_data.savePath.c_str() );
	fprintf(file, "// Set directory where worldfile backups will be stored\n");
	fprintf(file, "ARCHIVEPATH %s\n", server_data.archivePath.c_str() );
	fprintf(file, "// 0= disable html, else put in the number of seconds between HTML generation \n");
	fprintf(file, "HTML %i\n",server_data.html);
	fprintf(file, "// Enables / Disable features for UO:T2A/UO:R/UO:3D Clients (VERY! experimental)\n");
	fprintf(file, "// 0: Dont send packet, (default)\n");
	fprintf(file, "// 1: T2A (chat button, popup help, ...) 2: LBR (plus T2A) animations \n");
 	fprintf(file, "ENABLEFEATURE %d\n", server_data.feature);
	fprintf(file, "// Enables / Disable PopUp Help for UO:T2A/UO:R/UO:3D Clients \n");
 	fprintf(file, "POPUPHELP %i\n",ServerScp::g_nPopUpHelp);
	fprintf(file, "// 1= ghosts can attack living players and drain mana,\n");
	fprintf(file, "// 0 = ghosts cannot attack living characters  \n");
	fprintf(file, "PERSECUTION %i\n",server_data.persecute);//AntiChrist
	fprintf(file, "// 1= Allow auto creation of accounts, 0=disable auto account creation \n");
	fprintf(file, "// If you disable it, you can easily create accounts using the ADDACCT\n");
	fprintf(file, "// command in the Remote Administration console\n");
	fprintf(file, "AUTO_CREATE_ACCTS %i\n", server_data.auto_a_create);
	fprintf(file, "// Actually ignored \n");
	fprintf(file, "AUTO_RELOAD_ACCTS %i\n", server_data.auto_a_reload);
	fprintf(file, "// if 0 it won't do char age checking. Else it's the number of days before a\n");
	fprintf(file, "// user is able to delete a character \n");
	fprintf(file, "CHECKCHARAGE %i\n", server_data.checkcharage) ;
	fprintf(file, "// 1=restock is being done automatically, 0=restock needs to be done manually by staff \n");
	fprintf(file, "SHOPRESTOCK %i\n",server_data.shoprestock); // Ripper
	fprintf(file, "// 1= show animation (character falls backwards on the ground), 2= don't show animation  \n");
	fprintf(file, "SHOWDEATHANIM %i\n",server_data.showdeathanim);
	fprintf(file, "// Check this to 1 to display chars name whenever they are single clicked \n");
	fprintf(file, "SHOWPCNAMES %i\n", ServerScp::g_nShowPCNames); // Luxor
	fprintf(file, "// 1= guards are active as specified in regions.scp, 0= turns guards off globally,\n");
	fprintf(file, "// overriding region settings \n");
	fprintf(file, "GUARDSACTIVE %i\n",server_data.guardsactive);
	fprintf(file, "// Must be 1 - 10, Higher the number, less chance of background sounds  \n");
	fprintf(file, "BG_SOUNDS %i\n",server_data.bg_sounds);
	fprintf(file, "// 1=spawned npcs will be saved to the worldfile, 0=spawned npcs will not be saved \n");
 	fprintf(file, "SAVESPAWNREGIONS %i\n",server_data.savespawns);
	fprintf(file, "// 1=Broadcasts a message to all players that a worldsave is being done,\n");
	fprintf(file, "// 0=don't broadcast worldsaves \n");
	fprintf(file, "ANNOUNCE_WORLDSAVES %i\n",server_data.announceworldsaves);
	fprintf(file, "// 1= Broadcasts a notice to all players when someone logs in, 0= don't broadcast anything \n");
	fprintf(file, "JOINMSG %i\n",server_data.joinmsg);
	fprintf(file, "// 1= Broadcasts a notice to all players when someone logs out, 0= don't broadcast anything \n");
	fprintf(file, "PARTMSG %i\n",server_data.partmsg);
	fprintf(file, "// 0=Animal wanders off when released, 1=Animal disappears when released \n");
	fprintf(file, "TAMED_DISAPPEAR %i\n",server_data.tamed_disappear); // Ripper
	fprintf(file, "// 1 = Allow placement of houses in towns, 0= forbid placement of houses in towns  \n");
	fprintf(file, "HOUSEINTOWN %i\n",server_data.houseintown); // Ripper
	fprintf(file, "// Sets the command prefix for in-game commands \n");
	fprintf(file, "COMMANDPREFIX %c\n", server_data.commandPrefix ); // Ripper
	fprintf(file, "// Max number of charachters per account  \n");
	fprintf(file, "LIMITEROLENUMBERS %d\n",ServerScp::g_nLimitRoleNumbers); // Ripper
	fprintf(file, "// Set if players can delete their charachters \n");
	fprintf(file, "PLAYERCANDELETEROLES %d\n", ServerScp::g_nPlayersCanDeleteRoles ); // Ripper
	fprintf(file, "// Set to enable books\n");
	fprintf(file, "ENABLEBOOKS %d\n", ServerScp::g_nEnableBooks ); // Sparhawk;
	fprintf(file, "// Set to unable unequipping items when equipping equivalent ones\n");
	fprintf(file, "UNEQUIPONREEQUIP %d\n", ServerScp::g_nUnequipOnReequip ); // juliunus
	fprintf(file, "// Set to equip on double-clicking\n");
	fprintf(file, "EQUIPONDCLICK %d\n", ServerScp::g_nEquipOnDclick ); // Anthalir
	fprintf(file, "// Language of the server. For UNICODE fonts \n");	//N6
	fprintf(file, "LANGUAGE %s\n",server_data.Unicodelanguage);			//End-N6
	fprintf(file, "// 1 = Add command params are in hex ( no need 0x ), 0 = normal \n");	//Endymion
	fprintf(file, "ADDCOMMANDHEX %d\n",server_data.always_add_hex);		//Endymion
	fprintf(file, "// 1 = Speech override are case sensitive, 0 = case insensitive \n");	//Endymion
	fprintf(file, "CSSOVERRIDES %d\n",ServerScp::g_css_override_case_sensitive);		//Endymion
	fprintf(file, "// 1 = Disable Z checking ( cool for worldbuilder but not for gamplay ) \n");	//Endymion
	fprintf(file, "DISABLEZCHECKING %d\n",server_data.disable_z_checking);		//Endymion
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION BILLBOARDS\n");
	fprintf(file, "{\n");
	fprintf(file, "// Path to message board files, default is the same directory where NoXWizard is \n");
	fprintf(file, "MSGBOARDPATH %s\n",server_data.msgboardpath);              // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "// 0 = only GMs can post messages, 1= anyone can post \n");
	fprintf(file, "MSGPOSTACCESS %i\n",server_data.msgpostaccess);            // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "// 0 = only GMs can remove messages, 2= anyone can remove \n");
	fprintf(file, "MSGPOSTREMOVE %i\n",server_data.msgpostremove);            // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "// Length of time to retain messages (0 indefinite), not sure on measurements \n");
	fprintf(file, "MSGRETENTION %i\n",server_data.msgretention);              // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "// 1=enable escort quests (msgboard), 0=disable quests \n");
	fprintf(file, "ESCORTACTIVE %i\n",server_data.escortactive);              // Dupois - Added Dec 20, 1999 for escorts
	fprintf(file, "// Length of time that the escort is summoned for (seconds?)  \n");
	fprintf(file, "ESCORTINITEXPIRE %i\n",server_data.escortinitexpire);      // Dupois - Added Dec 20, 1999 for escorts
	fprintf(file, "// Length of time in seconds before the escort quest expires on board \n");
	fprintf(file, "ESCORTACTIVEEXPIRE %i\n",server_data.escortactiveexpire);  // Dupois - Added Dec 20, 1999 for escorts
	fprintf(file, "// Length of time in seconds for which escort is alive once taken  \n");
	fprintf(file, "ESCORTDONEEXPIRE %i\n",server_data.escortdoneexpire);      // Dupois - Added Dec 20, 1999 for escorts
	fprintf(file, "// 1= Enable bounties, 0 = Disable bounties  \n");
	fprintf(file, "BOUNTYSACTIVE %i\n",server_data.bountysactive);            // Dupois - Added July 18, 2000 for bountys
	fprintf(file, "// Length of time (seconds?) until bounty expires  \n");
	fprintf(file, "BOUNTYSEXPIRE %i\n",server_data.bountysexpire);            // Dupois - Added July 18, 2000 for bountys
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION SPEED\n"); //Lag Fix -- Zippy
	fprintf(file, "{\n");
	fprintf(file, "// accepts 0 to 3, 0 is most processor intensive, 2 is default (and nice) \n");
    fprintf(file, "NICE %i\n",speed.nice);
	fprintf(file, "// How often items are checked for action (in seconds) \n");
	fprintf(file, "CHECK_ITEMS %f\n",speed.itemtime);
	fprintf(file, "// How often NPCs are checked for action (in seconds)  \n");
	fprintf(file, "CHECK_NPCS %f\n",speed.npctime);
	fprintf(file, "// How often tamed NPCs are checked for action (in seconds)  \n");
	fprintf(file, "CHECK_TAMEDNPCS %f\n",speed.tamednpctime);//AntiChrist
	fprintf(file, "// This is for the speed of npcs that are following you, the higher the x the slower they are\n");
	fprintf(file, "CHECK_NPCFOLLOW %f\n",speed.npcfollowtime);//Ripper
	fprintf(file, "// How often the NPCs AI is checked (must be no more than CHECK_NPCS)  \n");
	fprintf(file, "CHECK_NPCAI %f\n",speed.npcaitime);
	fprintf(file, "// Number of seconds between spawn regions being checked for more spawns  \n");
	fprintf(file, "CHECK_SPAWNREGIONS %i\n",speed.srtime);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION COMBAT\n");
	fprintf(file, "{\n");
	fprintf(file, "// 1=Display hit messages (\"player hits your left leg\" etc), 0=no messages  \n");
	fprintf(file, "COMBAT_HIT_MESSAGE %i\n",server_data.combathitmessage);
	fprintf(file, "// maximum damage that can be absorbed by a character via armor  \n");
	fprintf(file, "MAX_ABSORBTION %i\n",server_data.maxabsorbtion);
	fprintf(file, "// maximum damage that can be absorbed by an npc via armor \n");
	fprintf(file, "MAX_NON_HUMAN_ABSORBTION %i\n",server_data.maxnohabsorbtion);
	fprintf(file, "// If 1, animals and monsters would attack each other  \n");
	fprintf(file, "MONSTERS_VS_ANIMALS %i\n",server_data.monsters_vs_animals);
	fprintf(file, "// chance (percent) that a monster will attack an animal  \n");
	fprintf(file, "ANIMALS_ATTACK_CHANCE %i\n",server_data.animals_attack_chance);
	fprintf(file, "// If 1, then animals are guarded in guard regions  \n");
	fprintf(file, "ANIMALS_GUARDED %i\n",server_data.animals_guarded);
	fprintf(file, "// Damage/Rate is the amount of damage that an NPC will sustain  \n");
	fprintf(file, "NPC_DAMAGE_RATE %i\n",server_data.npcdamage);
	fprintf(file, "// base hp which NPCs flee at  \n");
	fprintf(file, "NPC_BASE_FLEEAT %i\n",server_data.npc_base_fleeat);
	fprintf(file, "// base hp which NPCs will attack at  \n");
	fprintf(file, "NPC_BASE_REATTACKAT %i\n",server_data.npc_base_reattackat);
	fprintf(file, "// Amount of stamina gained (or lost, if negative) on attacking. \n");
	fprintf(file, "// Must have equal to at least the absolute value of this to attack as well  \n");
	fprintf(file, "ATTACKSTAMINA %i\n",server_data.attackstamina);	// antichrist (6)
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION VENDOR\n");
	fprintf(file, "{\n");
	fprintf(file, "// if 1, then item is sold by name rather than ID (doesn't work correctly)\n");
	fprintf(file, "SELLBYNAME %i\n",server_data.sellbyname);	// Magius(CHE)
	fprintf(file, "// maximum number of items a character can sell during one transaction \n");
	fprintf(file, "SELLMAXITEM %i\n",server_data.sellmaxitem);	// Magius(CHE)
	fprintf(file, "// advanced trade system, if 1, then goods prices do fluctuate (based on GOOD settings)\n");
	fprintf(file, "TRADESYSTEM %i\n",server_data.trade_system);	// Magius(CHE)
	fprintf(file, "// Restock every n seconds \n");
	fprintf(file, "RESTOCKRATE %i\n", ServerScp::g_nRestockTimeRate );
	fprintf(file, "// if 1, then difficulty is graded and varying quality pieces are made  \n");
	fprintf(file, "RANKSYSTEM %i\n",server_data.rank_system);// Moved by Magius(CHE)
	fprintf(file, "// x=minimum item price required to let the player pay for\n");
	fprintf(file, "// an item with gold from his bankbox \n");
	fprintf(file, "CHECKBANK %i\n",server_data.CheckBank); // Ripper
	fprintf(file, "// Default number of item selled by vendor for every item \n");
	fprintf(file, "SELLFORITEM %i\n",server_data.defaultSelledItem);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION PARTYSYSTEM\n");
	fprintf(file, "{\n");
	fprintf(file, "// Set to 1 to enable party members share karma gains and losses  \n");
	fprintf(file, "PARTYSHAREKARMA %i\n", Partys.shareKarma );
	fprintf(file, "// Set to 1 to enable party members share fame gains and losses  \n");
	fprintf(file, "PARTYSHAREFAME %i\n", Partys.shareFame );
	fprintf(file, "// Set to 1 to enable private party messages \n");
	fprintf(file, "ENABLEPRIVATEMSG %i\n", Partys.canPMsg );
	fprintf(file, "// Set to 1 to enable party members to send a message to all his friends.\n");
	fprintf(file, "ENABLEPUBLICMSG %i\n", Partys.canBroadcast );
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION REGENERATE\n");
	fprintf(file, "{\n");
	fprintf(file, "// Define how often (seconds) will hitpoints regenerate by 1  \n");
	fprintf(file, "HITPOINTS_REGENRATE %i\n",server_data.hitpointrate);
	fprintf(file, "// Define how often (seconds) stamina will regenerate by 1  \n");
	fprintf(file, "STAMINA_REGENRATE %i\n",server_data.staminarate);
	fprintf(file, "// Define how often (seconds) mana will regenerate by 1  \n");
	fprintf(file, "MANA_REGENRATE %i\n",server_data.manarate);
	fprintf(file, "// If 1, enables meditation. Higher damage, less the speed regen rate  \n");
	fprintf(file, "ARMOR_AFFECT_MANA_REGEN %i\n",server_data.armoraffectmana);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION HUNGER\n");
	fprintf(file, "{\n");
	fprintf(file, "// Activate internal hunger system. 1 is activated, 0 no\n");
	fprintf(file, "HUNGER_SYSTEM %i\n",server_data.hunger_system);
	fprintf(file, "// How often you get hungrier in seconds  \n");
	fprintf(file, "HUNGERRATE %i\n",server_data.hungerrate);
	fprintf(file, "// How much health is lost when you are starving  \n");
	fprintf(file, "HUNGER_DAMAGE %i\n",server_data.hungerdamage);
	fprintf(file, "// How often you lose health when you are starving  \n");
	fprintf(file, "HUNGER_DAMAGE_RATE %i\n",server_data.hungerdamagerate);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION RESOURCE\n");
	fprintf(file, "{\n");
	fprintf(file, "// Dictates areas that can be mined. 0 is anywhere, 1 is mountains\n");
	fprintf(file, "// and cave floors, 2 is those areas that are flagged as mining regions\n");
	fprintf(file, "MINECHECK %i\n",server_data.minecheck);
	fprintf(file, "// Amount of ore in an area  \n");
	fprintf(file, "ORE_PER_AREA %i\n", ores.n);
	fprintf(file, "// Ore area width  \n");
	fprintf(file, "ORE_AREA_WIDTH %i\n", ores.area_width);
	fprintf(file, "// Ore area height  \n");
	fprintf(file, "ORE_AREA_HEIGHT %i\n", ores.area_height);
	fprintf(file, "//  How long it takes to respawn ore, in seconds \n");
	fprintf(file, "ORE_RESPAWN_TIME %i\n",ores.time);
	fprintf(file, "ORE_RESPAWN_RATE %i\n",ores.rate);
	fprintf(file, "// Stamina spent when mining \n");
	fprintf(file, "ORE_STAMINA %i\n", ores.stamina);
	fprintf(file, "// How many logs are in the area \n");
	fprintf(file, "LOGS_PER_AREA %i\n",resource.logs);
	fprintf(file, "// How long it takes for the logs to respawn  \n");
	fprintf(file, "LOG_RESPAWN_TIME %i\n",resource.logtime);
	fprintf(file, "LOG_RESPAWN_RATE %i\n",resource.lograte);
	fprintf(file, "// Number of log areas there are in the world \n");
	fprintf(file, "LOG_RESPAWN_AREA %i\n",resource.logarea);
	fprintf(file, "// Trigger fire for custom lumberjacking \n");
	fprintf(file, "LOG_TRIGGER %i\n",resource.logtrigger);
	fprintf(file, "// Stamina spent when lumberjacking \n");
	fprintf(file, "LOG_STAMINA %i\n",resource.logstamina);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION REPSYS\n");
	fprintf(file, "{\n");
	fprintf(file, "// Length of time (seconds) until the murder counts decrease by 1 \n");
	fprintf(file, "MURDER_DECAY %i\n", repsys.murderdecay);
	fprintf(file, "// Maximum amount of kills a character is allowed to have before he turns into a murderer\n");
	fprintf(file, "MAXKILLS %i\n", repsys.maxkills);
	fprintf(file, "// Length of time (seconds) before the criminal flag timer wears off\n");
	fprintf(file, "CRIMINAL_TIME %i\n", repsys.crimtime);
	fprintf(file, "// Check this to 1 to allow some spells to be cast in town\n");
	fprintf(file, "ALLOWMAGICINTOWN %d\n", ServerScp::g_nAllowMagicInTown);
	fprintf(file, "// Sets how the player become after snooping : 0:remains blue, 1:goes grey, 2:goes criminal\n");
	fprintf(file, "SNOOPWILLCRIMINAL %d\n", ServerScp::g_nSnoopWillCriminal);
	fprintf(file, "// Sets how the player become after stealing : 0:remains blue, 1:goes grey, 2:goes criminal\n");
	fprintf(file, "STEALWILLCRIMINAL %d\n", ServerScp::g_nStealWillCriminal);
	fprintf(file, "// Sets how the player become after chopping : 0:remains blue, 1:goes grey, 2:goes criminal\n");
	fprintf(file, "CHOPWILLCRIMINAL %d\n", ServerScp::g_nChopWillCriminal);
	fprintf(file, "// Sets how the player become after polymorphing : 0:remains blue, 1:goes grey, 2:goes criminal\n");
	fprintf(file, "POLYMORPHWILLCRIMINAL %d\n", ServerScp::g_nPolymorphWillCriminal);
	fprintf(file, "// Sets how the player become after looting a blue : 0:remains blue, 1:goes grey, 2:goes criminal\n");
	fprintf(file, "LOOTINGWILLCRIMINAL %d\n", ServerScp::g_nLootingWillCriminal);
	//Arakensh guard
	fprintf (file, "//When attack in town 1:the guard respawn instantanely 0:someone must call guards or if guard is near go to criminal or murderer\n");
	fprintf (file, "INSTANTGUARD %i\n",ServerScp::g_nInstantGuard);
	//end arakensh guard
	fprintf(file, "// Sets how the player become helping a grey : 0:remains blue, 1:goes grey, 2:goes criminal\n");
	fprintf(file, "HELPINGGREYWILLCRIMINAL %d\n", ServerScp::g_nHelpingGreyWillCriminal);
	fprintf(file, "// Sets how the player become helping a criminal : 0:remains blue, 1:goes grey, 2:goes criminal\n");
	fprintf(file, "HELPINGCRIMINALWILLCRIMINAL %d\n", ServerScp::g_nHelpingCriminalWillCriminal);
	fprintf(file, "// 1 if greys can be looted freely\n");
	fprintf(file, "GREYCANBELOOTED %d\n", ServerScp::g_nGreyCanBeLooted);
	fprintf(file, "// Karma loss when snooping (relative value)\n");
	fprintf(file, "SNOOPKARMALOSS %d\n", ServerScp::g_nSnoopKarmaLoss);
	fprintf(file, "// Karma loss when stealing (relative value)\n");
	fprintf(file, "STEALKARMALOSS %d\n", ServerScp::g_nStealKarmaLoss);
	fprintf(file, "// Fame loss when snooping (relative value)\n");
	fprintf(file, "SNOOPFAMELOSS %d\n", ServerScp::g_nSnoopFameLoss);
	fprintf(file, "// Fame loss when stealing (relative value)\n");
	fprintf(file, "STEALFAMELOSS %d\n", ServerScp::g_nStealFameLoss);
	fprintf(file, "// Karma loss when chopping dead bodies (absolute value!)\n");
	fprintf(file, "CHOPKARMALOSS %d\n", ServerScp::g_nChopKarmaLoss);
	fprintf(file, "// Fame loss when chopping dead bodies (absolute value!)\n");
	fprintf(file, "CHOPFAMELOSS %d\n", ServerScp::g_nChopFameLoss);
	fprintf(file, "// Karma gain when collecting a bounty (absolute value!)\n");
	fprintf(file, "BOUNTYKARMAGAIN %d\n", ServerScp::g_nBountyKarmaGain);
	fprintf(file, "// Fame gain when collecting a bounty (absolute value!)\n");
	fprintf(file, "BOUNTYFAMEGAIN %d\n", ServerScp::g_nBountyFameGain);
	fprintf(file, "// Set to 1 to enable Karma Lock when karma level drops down\n");
	fprintf(file, "ENABLEKARMALOCK %d\n", ServerScp::g_nEnableKarmaLock);
	fprintf(file, "// Karma unlock prayer\n");
	fprintf(file, "KARMAUNLOCKPRAYER %s\n", ServerScp::g_strKarmaUnlockPrayer);
	fprintf(file, "}\n\n");



	fprintf(file, "SECTION TIME_LIGHT\n");
	fprintf(file, "{\n");
	fprintf(file, "// Here follows current time parameters\n");
	fprintf(file, "ABSDAY %i\n", day);
	fprintf(file, "DAY %i\n", Calendar::g_nDay);
	fprintf(file, "HOUR %i\n", Calendar::g_nHour);
	fprintf(file, "MINUTE %i\n", Calendar::g_nMinute);
	fprintf(file, "MONTH %i\n", Calendar::g_nMonth);
	fprintf(file, "WEEKDAY %i\n", Calendar::g_nWeekday);
	fprintf(file, "YEAR %i\n", Calendar::g_nYear);
	fprintf(file, "// Here follows moon(s) status\n");
	fprintf(file, "MOON1UPDATE %i\n", moon1update);
	fprintf(file, "MOON2UPDATE %i\n", moon2update);
	fprintf(file, "MOON1 %i\n", moon1);
	fprintf(file, "MOON2 %i\n", moon2);
	fprintf(file, "SEASON %i\n", season); // lb
	fprintf(file, "// The light in dungeons\n");
	fprintf(file, "DUNGEONLIGHTLEVEL %i\n", dungeonlightlevel);
	fprintf(file, "// A fixed light level for the world\n");
	fprintf(file, "WORLDFIXEDLEVEL %i\n", worldfixedlevel);
	fprintf(file, "// Current light level\n");
	fprintf(file, "WORLDCURLEVEL %i\n", worldcurlevel);
	fprintf(file, "// World brightest light level\n");
	fprintf(file, "WORLDBRIGHTLEVEL %i\n", worldbrightlevel);
	fprintf(file, "// World darkest light level\n");
	fprintf(file, "WORLDDARKLEVEL %i\n", worlddarklevel);
	fprintf(file, "// Seconds to make a ingame time minute\n");
	fprintf(file, "SECONDSPERUOMINUTE %i\n", secondsperuominute); // lb
	fprintf(file, "// Season to display instead of standard winter one (mainly because of graphics leaks\n");
	fprintf(file, "// in the standard winter season ). Default is display spring (0). Winter is 3.\n");
	fprintf(file, "OVERRIDEWINTERSEASON %i\n", Calendar::g_nWinterOverride); // lb
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION MAGIC_RESISTANCE\n");
	fprintf(file, "{\n");
	fprintf(file, "// Check this to 1 to display whenever a charachter is fire-resistant \n");
	fprintf(file, "SHOWFIRERESISTANCEINTITLE %i\n", ServerScp::g_nShowFireResistanceInTitle); // lb
	fprintf(file, "// Check this to 1 to display whenever a charachter is poison-resistant \n");
	fprintf(file, "SHOWPOISONRESISTANCEINTITLE %i\n", ServerScp::g_nShowPoisonResistanceInTitle); // lb
	fprintf(file, "// Check this to 1 to display whenever a charachter is freeze-resistant \n");
	fprintf(file, "SHOWPARALISYSRESISTANCEINTITLE %i\n", ServerScp::g_nShowParalisysResistanceInTitle); // lb
	fprintf(file, "// Check this to 1 to disable magic resistance raise when walk on fields \n");
	fprintf(file, "DISABLEFIELDRESISTANCERAISE %i\n", ServerScp::g_nDisableFieldResistanceRaise); // lb
 	fprintf(file, "}\n\n");

	fprintf(file, "SECTION REMOTE_ADMIN\n");
	fprintf(file, "{\n");
	fprintf(file, "// This is the TCP port where the telnet client connects to. \n");
	fprintf(file, "// Using 0 will disable it. \n");
	fprintf(file, "PORT %i\n", ServerScp::g_nRacTCPPort); // lb
	fprintf(file, "// Set to 0 to use line by line mode, to 1 to char by char mode \n");
	fprintf(file, "CHARMODE %i\n", ServerScp::g_nUseCharByCharMode); // lb
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION NOXWIZARD_SYSTEM\n");
	fprintf(file, "{\n");
	fprintf(file, "// Set to 0 to disable Small (AMX) scripts, 1 to enable them (default) \n");
	fprintf(file, "ENABLEAMX %i\n", ServerScp::g_nEnableAMXScripts); // lb
/*#ifndef _WINDOWS
	fprintf(file, "// Set to 1 to enable the internal debugger for amx programs. If you use \n");
	fprintf(file, "// an external compiler, remember to include debug symbols.\n");
	fprintf(file, "DEBUGAMX %i\n", ServerScp::g_nLoadDebugger);
#endif //_WINDOWS*/
	fprintf(file, "// Set to 0 to disable the integrated Small compiler \n");
	fprintf(file, "USEINTERNALCOMPILER %i\n", (ServerScp::g_bEnableInternalCompiler ? 1 : 0)); // lb
	fprintf(file, "// Set to 1 to enable check of all object with scripted small function ( checkItem, checkNpc, checkPl in checks.sma )\n");
	fprintf(file, "CHECKBYSMALL 0\n" ); //ndEndy executed ONLY A TIME
	fprintf(file, "// Set to 1 enables Salted DES 56bit account encryption \n");
	fprintf(file, "USEACCOUNTENCRYPTION %i\n", ServerScp::g_nUseAccountEncryption); // lb
	fprintf(file, "// Set to 1 enables verbose comments on crontab tasks execution \n");
	fprintf(file, "VERBOSECRONTAB %i\n", ServerScp::g_nVerboseCrontab);
	fprintf(file, "// Set to 1 enables enhanced npc magic \n");
	fprintf(file, "NEWNPCMAGIC %i\n", ServerScp::g_nUseNewNpcMagic);
/*    if (ADVENTUREMODE) {
    	fprintf(file, "// Sets adventure mode on/off [Dangerous!] \n");
	    fprintf(file, "ADVENTUREMODE %i\n", ServerScp::g_nAdventureMode);
    }*/
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION PRELOAD\n");
	fprintf(file, "{\n");
	fprintf(file, "// Set to 1 copy all console command to a file. For deamon mode or \n");
	fprintf(file, "// NT service mode, this is always true \n");
	fprintf(file, "CONSOLEONFILE %i\n", ServerScp::g_nRedirectOutput);
	fprintf(file, "// File that will get console output \n");
	fprintf(file, "STDOUTFILE %s\n", ServerScp::g_szOutput);
	fprintf(file, "}\n\n");


	fprintf(file, "SECTION LINUX\n");
	fprintf(file, "{\n");
	fprintf(file, "// Runs as a deamon under Linux.\n");
	fprintf(file, "// Output goes to the file specified in PRELOAD, STDOUTFILE \n");
	fprintf(file, "DEAMONMODE %i\n", ServerScp::g_nDeamonMode); // lb
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION WINDOWS\n");
	fprintf(file, "{\n");
	fprintf(file, "// Set this to change the number of rows in the console window [console mode only] \n");
	fprintf(file, "LINEBUFFER %i\n", ServerScp::g_nLineBuffer); // lb
	fprintf(file, "// Set this to enable plug&play configuration of mul paths \n");
	fprintf(file, "// (Requires you have correctly installed a T2A/Renaissance client) \n");
	fprintf(file, "AUTODETECTMULS %i\n", ServerScp::g_nAutoDetectMuls); // lb
	fprintf(file, "}\n\n");


	fprintf(file, "SECTION MAP\n");
	fprintf(file, "{\n");
	fprintf(file, "// These are width and height of the map. Set to 768x512 for Britannian Maps, or \n");
	fprintf(file, "// to 288x200 for Ilshenar maps. Remember to use different worldfiles! \n");
	fprintf(file, "MAPWIDTH %i\n", map_width); // lb
	fprintf(file, "MAPHEIGHT %i\n", map_height); // lb
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION WALKING\n");
	fprintf(file, "{\n");
	fprintf(file, "// Enable this if you want clients to be able to walk over water-on-earth tiles.  \n");
	fprintf(file, "// This may have bad side effects, do not enable if you're not sure on what you're doing!  \n");
	fprintf(file, "// It's primarly designed for custom MAP0.MULs.  \n");
	fprintf(file, "IGNOREWETBIT %i\n",ServerScp::g_nWalkIgnoreWetBit);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION NEWPLAYERS\n");
	fprintf(file, "{\n");
	fprintf(file, "// Initial gold for players  \n");
	fprintf(file, "INITIALGOLD %i\n",goldamount);
	fprintf(file, "// Initial priv1 value (don't touch if you don't know exactly what you're doing!)  \n");
	fprintf(file, "INITIALPRIV1 %i\n",defaultpriv1);
	fprintf(file, "// Initial priv2 value (don't touch if you don't know exactly what you're doing!)  \n");
	fprintf(file, "INITIALPRIV2 %i\n",defaultpriv2);
	fprintf(file, "}\n\n");

	fprintf(file, "SECTION START_LOCATIONS\n");
	fprintf(file, "{\n");
	fprintf(file, "// Raw list of starting locations  \n");
	fprintf(file, "// Note : you must always have exactly nine starting locations.  \n");
	fprintf(file, "// If you insert less starting locations, the first one is dupped for padding.  \n");
	fprintf(file, "// You can't use CFG commands on this section. \n");
	fprintf(file, "// \n");
	for (unsigned int i=0; i<startcount; i++)
	{
		fprintf(file, "CITY %s\n",start[i][0]);
		fprintf(file, "PLACE %s\n",start[i][1]);
		fprintf(file, "POSITION %s,%s,%s\n",start[i][2],start[i][3],start[i][4]);
	}
	fprintf(file, "}\n\n");


	fprintf(file, "//Use this section for Client Version Control. You can't use CFG commands on this section\n");
	fprintf(file, "SECTION CLIENTS_ALLOWED\n");
	fprintf(file, "{\n");

  for (; vis != vis_end;  ++vis)
  {
    t = (*vis).c_str();  // a bit pervert to store c++ strings and operate with c strings, admitably
	  strcpy(temp,t);
	  fprintf(file, "%s\n",temp);
  }

	fprintf(file, "}\n\n\n\n");


	fprintf(file, "SECTION BLOCK_ACC_PSS\n");		//elcabesa tempblock
	fprintf(file, "{\n");							//elcabesa tempblock
	fprintf(file, "// set to 1 if you want an account blocked due to many bad password\n");	//elcabesa tempblock
	fprintf(file, "BLOCKACCBADPASS %i\n",server_data.blockaccbadpass);						//elcabesa tempblock
	fprintf(file, "// number of times you can mistake password before account will be blocked.\n");	//elcabesa tempblock
	fprintf(file, "N_BADPASS %i\n",server_data.n_badpass);								//elcabesa tempblock
	fprintf(file, "// time in minutes the account stay blocked\n");							//elcabesa tempblock
	fprintf(file, "TIME_BLOCKED %i\n",server_data.time_badpass);							//elcabesa tempblock
	fprintf(file, "}\n\n");


	fprintf(file, "// Please *do not remove* this line and don't put any data after this line\n");
	fprintf(file, "EOF\n\n");
	fclose(file);
}

//xan : the road to madness : two level of fn ptrs calls... mmm one day I'll rewrite this!
static int chooseSection(char *section,  int (*parseSec)(int (*parseLine)(char *s1, char *s2 )))
{
	if(!(strcmp(section, "SERVER"))) return parseSec(loadserver);
	else if(!(strcmp(section, "BANK"))) return parseSec(loadserver);
	else if(!(strcmp(section, "LOG"))) return parseSec(loadserver);
	else if(!(strcmp(section, "BILLBOARDS"))) return parseSec(loadserver);
	else if(!(strcmp(section, "STAMINA"))) return parseSec(loadserver);
	else if(!(strcmp(section, "SKILLS"))) return parseSec(loadserver);
	else if(!(strcmp(section, "TIMERS"))) return parseSec(loadserver);
	else if(!(strcmp(section, "SPEED"))) return parseSec(loadspeed); // Zippy
	else if(!(strcmp(section, "RESOURCE"))) return parseSec(loadresources);
	else if(!(strcmp(section, "REPSYS"))) return parseSec(loadrepsys);
	else if(!(strcmp(section, "TIME_LIGHT"))) return parseSec(loadtime_light);
	else if(!(strcmp(section, "HUNGER"))) return parseSec(loadhunger);
	else if(!(strcmp(section, "COMBAT"))) return parseSec(loadcombat);
	else if(!(strcmp(section, "VENDOR"))) return parseSec(loadvendor);
	else if(!(strcmp(section, "REGENERATE"))) return parseSec(loadregenerate);
	else if(!(strcmp(section, "MAGIC_RESISTANCE"))) return parseSec(loadnxwoptions);
	else if(!(strcmp(section, "REMOTE_ADMIN"))) return parseSec(loadnxwoptions);
	else if(!(strcmp(section, "NOXWIZARD_SYSTEM"))) return parseSec(loadnxwoptions);
	else if(!(strcmp(section, "WALKING"))) return parseSec(loadnxwoptions);
	else if(!(strcmp(section, "PARTYSYSTEM"))) return parseSec(loadnxwoptions);
	else if(!(strcmp(section, "WINDOWS"))) return parseSec(loadnxwoptions);
	else if(!(strcmp(section, "LINUX"))) return parseSec(loadnxwoptions);
	else if(!(strcmp(section, "MAP"))) return parseSec(loadnxwoptions);
	else if(!(strcmp(section, "SHARD"))) return parseSec(loadinioptions);
	else if(!(strcmp(section, "MULFILES"))) return parseSec(loadinioptions);
	else if(!(strcmp(section, "START_LOCATIONS"))) return parseSec(loadstartlocs);
	else if(!(strcmp(section, "PRELOAD"))) return parseSec(loadPreloads);
	else if(!(strcmp(section, "NEWPLAYERS"))) return parseSec(loadnxwoptions);
	else if(!(strcmp(section, "BLOCK_ACC_PSS"))) return parseSec(block_acc);	//elcabesa tempblock
	return -90;
}





int cfg_command (char *commandstr)
{
	char b[80];
	int i;

	//copy, uppercase and truncate
	for (i=0; i<79; i++)
	{
		if ((commandstr[i]>='a')&&(commandstr[i]<='z'))
			b[i] = commandstr[i]+ 'A'-'a';
		else b[i] = commandstr[i];
		if (b[i]=='\0') break;
	}
	b[79] = '\0'; //xan -> truncate to avoid overflowz

	char *section = b;
	char *property = NULL;
	char *value = NULL;
	int ln = strlen(b);

	for (i=0; i<ln; i++)
	{
		if (b[i]=='.')
		{
			b[i]='\0';
			property = b+i+1;
			break;
		}
	}

	if (property==NULL) return -3;

	ln = strlen(property);

	for (i=0; i<ln; i++)
	{
		if ((property[i]==' ')||(property[i]=='=')||(property[i]==':')||(property[i]==','))
		{
			property[i]='\0';
			value = property+i+1;
			break;
		}
	}
	if (value==NULL) return -4;

	gprop = property;
	gval = value;

	//now we have section, property and value.. parse them all :)
	return chooseSection(section, parseCfgLine);
}


