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
#include "cmdtable.h"
#include "sregions.h"
#include "bounty.h"
#include "sndpkg.h"
#include "debug.h"
#include "amx/amxcback.h"
#include "calendar.h"
#include "race.h"
#include "scp_parser.h"
#include "commands.h"
#include "addmenu.h"
#include "telport.h"
#include "accounts.h"
#include "worldmain.h"
#include "data.h"
#include "spawn.h"
#include "trade.h"
#include "chars.h"
#include "items.h"
#include "basics.h"
#include "inlines.h"
#include "nox-wizard.h"
#include "archive.h"
#include "map.h"
#include "jail.h"

// Includes command definitions
#include "commands/tweaking.h"
#include "commands/privlevels.h"
#include "commands/addremove.h"

static char s_szCmdTableTemp[TEMP_STR_SIZE];

//@{
/*!
\name Targets
*/

target_st target_s_tele = { target_tele, "Select teleport target..." };
target_st target_s_remove = { target_remove, "Select item to remove..." };
target_st target_s_release = { target_release, "Select player to release from jail." };
target_st target_s_xbank = { target_xbank, "Select target to open bank of." };
target_st target_s_xsbank = { target_xsbank, "Select target to open specialbank of." };//AntiChrist
target_st target_s_makegm = { target_makegm, "Select character to make a GM." };
target_st target_s_makecns = { target_makecns, "Select character to make a Counselor." };
target_st target_s_killhair = { target_killhair, "Select character for cutting hair." };
target_st target_s_killbeard = { target_killbeard, "Select character for shaving." };
target_st target_s_kill = { target_kill, "Select character to kill." };
target_st target_s_resurrect = { target_resurrect, "Select character to resurrect." };
target_st target_s_bolt = { target_bolt, "Select character to bolt." };
target_st target_s_kick = { target_kick, "Select character to kick." };
target_st target_s_xgo = { target_xgo, "Select char to teleport." };
target_st target_s_npcaction = { target_npcaction, "Select npc to make act." };
target_st target_s_setamount = { target_setamount, "Select item to edit amount." };
target_st target_s_freeze = { target_freeze, "Select player to freeze in place." };
target_st target_s_unfreeze = { target_unfreeze, "Select player to unfreeze." };
target_st target_s_tiledata = { target_tiledata, "Select item to inspect." };
target_st target_s_heal = { target_heal, "Select person to heal." };
target_st target_s_mana = { target_mana, "Select person to restore mana to." };
target_st target_s_stamina = { target_stamina, "Select person to refresh." };
target_st target_s_fullstats = { target_fullstats, "Select creature to restore full stats." };
target_st target_s_hide = { target_hide, "Select creature to hide." };
target_st target_s_unhide = { target_unhide, "Select creature to reveal." };
target_st target_s_possess = { target_possess, "Select creature to possess." };
target_st target_s_telestuff = { target_telestuff, "Select player/object to teleport." };
target_st target_s_emptypack = { target_emptypack, "Select character to empty pack." };
target_st target_s_ban = { target_ban, "Select character to BAN." };
target_st target_s_spy = { target_spy, "Select player to spy..." };

//@}


// Class cCommand implemetation
cCommand::cCommand(char* name, int privm, int privb, int type, void command(), unsigned char targMask) {
    cmd_name= name;
    cmd_priv_m= privm;
    cmd_priv_b= privb;
    cmd_type= type;
    cmd_extra= command;
    exec= NULL;
    targetingMask= targMask;
}

cCommand::cCommand(char* name, int privm, int privb, cCommandStep* command, unsigned char targMask) {
    cmd_name= name;
    cmd_priv_m= privm;
    cmd_priv_b= privb;
    cmd_type= CMD_MANAGEDCMD;
    cmd_extra= NULL;
    exec= command;
    targetingMask= targMask;
}

bool cCommand::notValid(P_CHAR character) {


	return  ((character->IsTrueGM() && !character->IsGM()) ||		// a restricted GM outside his region(s)
		     (character->account!=0)&&(cmd_priv_m!=255)&&
		     (!(character->priv3[cmd_priv_m]&
		     (0-0xFFFFFFFF<<cmd_priv_b))));

}

bool cCommand::isValid(P_CHAR character) {
	return !notValid(character);
}

P_COMMANDSTEP cCommand::buildSteps(MANAGEDSTEP(command), ...) {
    P_COMMANDSTEP first= new cCommandStep(command);
    P_COMMANDSTEP current= first;
    va_list cmds;
    va_start(cmds, command);
	void* cmd= va_arg(cmds, void*);
    while(cmd!=NULL) {
		// I must cast here or can't compile on vs.net :(
		P_COMMANDSTEP next= new cCommandStep((P_MANAGEDSTEP)cmd);
        current->nextStep= next;
        current= next;
        cmd= va_arg(cmds, void*);
    }
    va_end(cmds);
    return first;
}

// Class cCommandTable implemetation
cCommandTable::cCommandTable() {
    cTweaking::init(this);
	cPrivLevels::init(this);
	cAddRemove::init(this);
    // Commands for all players
    addPlayerCommand("BOUNTY",     0,  CMD_FUNC,        (CMD_DEFINE)&command_bounty);
    // GM Commands
    addGmCommand("RELOADRACE",      0, 0,  CMD_FUNC,        (CMD_DEFINE)&command_reloadracescript);
    addGmCommand("SETPRIV3",        0, 0,  CMD_FUNC,        (CMD_DEFINE)&command_setpriv3);
    addGmCommand("SPREADPRIV3",     0, 0,  CMD_FUNC,        (CMD_DEFINE)&command_spreadpriv3);
//    addGmCommand("USE",             0, 1,  CMD_TARGET,      (CMD_DEFINE)&target_s_use);
    addGmCommand("RESEND",		  0, 2,	 CMD_FUNC,        (CMD_DEFINE)&command_resend);
    addGmCommand("POINT",		      0, 3,	 CMD_FUNC,        (CMD_DEFINE)&command_teleport);
    addGmCommand("WHERE",           0, 4,  CMD_FUNC,        (CMD_DEFINE)&command_where);
//    addGmCommand("ADDU",            0, 5,  CMD_ITEMMENU,    (CMD_DEFINE)1); // Opens the GM add menu.
    addGmCommand("Q",			      0, 6,  CMD_FUNC,        (CMD_DEFINE)&command_q);
    addGmCommand("NEXT",            0, 7,  CMD_FUNC,        (CMD_DEFINE)&command_next);
    addGmCommand("CLEAR",           0, 8,  CMD_FUNC,        (CMD_DEFINE)&command_clear);
    addGmCommand("GOTOCUR",         0, 9,  CMD_FUNC,        (CMD_DEFINE)&command_gotocur);
    addGmCommand("GMTRANSFER",      0, 10, CMD_FUNC,        (CMD_DEFINE)&command_gmtransfer);
    addGmCommand("JAIL",            0, 11, CMD_FUNC,        (CMD_DEFINE)&command_jail);
    addGmCommand("RELEASE",         0, 12, CMD_TARGET,      (CMD_DEFINE)&target_s_release);
    addGmCommand("STATS",          0, 13, CMD_FUNC,      (CMD_DEFINE)&command_stats);
    addGmCommand("OPTIONS",		  0, 14, CMD_FUNC,      (CMD_DEFINE)&command_options);
    addGmCommand("GOPLACE",		  0, 15, CMD_FUNC,        (CMD_DEFINE)&command_goplace);
    addGmCommand("GOCHAR",		  0, 16, CMD_FUNC,        (CMD_DEFINE)&command_gochar);
    addGmCommand("FIX",             0, 17, CMD_FUNC,        (CMD_DEFINE)&command_fix);
    addGmCommand("XGOPLACE",        0, 18, CMD_FUNC,        (CMD_DEFINE)&command_xgoplace);
    addGmCommand("SHOWIDS",         0, 19, CMD_FUNC,        (CMD_DEFINE)&command_showids);
    addGmCommand("POLY",            0, 20, CMD_FUNC,        (CMD_DEFINE)&command_poly);
    addGmCommand("SETGMMOVEEFF",    0, 21, CMD_FUNC,        (CMD_DEFINE)&command_setGmMoveEff); //mmm dupe here :(
    addGmCommand("SKIN",            0, 21, CMD_FUNC,		  (CMD_DEFINE)&command_skin);
    addGmCommand("ACTION",          0, 22, CMD_FUNC,        (CMD_DEFINE)&command_action);
    addGmCommand("TELE",            0, 23, CMD_TARGETLOC,   (CMD_DEFINE)&target_s_tele);
    addGmCommand("XTELE",           0, 24, CMD_FUNC,        (CMD_DEFINE)&command_xtele);
    addGmCommand("GO",              0, 25, CMD_FUNC,        (CMD_DEFINE)&command_go);
    addGmCommand("XGO",             0, 26, CMD_TARGETXYZ,   (CMD_DEFINE)&target_s_xgo);
//  addGmCommand("SETMOREX",        0, 27, CMD_TARGETX,     (CMD_DEFINE)&target_s_setmorex);
//	addGmCommand("SETMOREY",        0, 28, CMD_TARGETX,     (CMD_DEFINE)&target_s_setmorey);
//    addGmCommand("SETMOREZ",        0, 29, CMD_TARGETX,     (CMD_DEFINE)&target_s_setmorez);
    addGmCommand("ZEROKILLS",	      0, 30, CMD_FUNC,        (CMD_DEFINE)&command_zerokills);
//    addGmCommand("SETMOREXYZ",      0, 31, CMD_TARGETXYZ,   (CMD_DEFINE)&target_s_setmorexyz);
//BYTE-1
//    addGmCommand("SETHEXMOREXYZ",   1, 0,  CMD_TARGETHXYZ,  (CMD_DEFINE)&target_s_sethexmorexyz);
    //addGmCommand("SETNPCAI",        1, 1,  CMD_TARGETHX,    (CMD_DEFINE)&target_s_setnpcai);
    addGmCommand("XBANK",           1, 2,  CMD_TARGET,      (CMD_DEFINE)&target_s_xbank);
    addGmCommand("XSBANK",          1, 2,  CMD_TARGET,      (CMD_DEFINE)&target_s_xsbank);//AntiChrist
    addGmCommand("POST",            1, 2,  CMD_FUNC,        (CMD_DEFINE)&command_post);
    addGmCommand("GPOST",           1, 2,  CMD_FUNC,        (CMD_DEFINE)&command_gpost);
    addGmCommand("RPOST",           1, 2,  CMD_FUNC,        (CMD_DEFINE)&command_rpost);
    addGmCommand("LPOST",           1, 2,  CMD_FUNC,        (CMD_DEFINE)&command_lpost);
//    addGmCommand("TILE",            1, 3,  CMD_FUNC,        (CMD_DEFINE)&command_tile);
//    addGmCommand("IWIPE",           1, 5,  CMD_FUNC,        (CMD_DEFINE)&command_iwipe);
    addGmCommand("ADD",             1, 6,  CMD_FUNC,        (CMD_DEFINE)&command_add);
    addGmCommand("ADDX",            1, 7,  CMD_FUNC,        (CMD_DEFINE)&command_addx);
    addGmCommand("RENAME",          1, 8,  CMD_FUNC,        (CMD_DEFINE)&command_rename);
    addGmCommand("TITLE",          1, 9,  CMD_FUNC,        (CMD_DEFINE)&command_title);
    addGmCommand("SAVE",            1, 10, CMD_FUNC,        (CMD_DEFINE)&command_save);
    addGmCommand("REMOVE",          1, 11, CMD_TARGET,      (CMD_DEFINE)&target_s_remove);
//    addGmCommand("TRAINER",         1, 12, CMD_TARGET,      (CMD_DEFINE)&target_s_trainer);
    addGmCommand("DYE",             1, 13, CMD_FUNC,        (CMD_DEFINE)&command_dye);
    addGmCommand("NEWZ",            1, 14, CMD_FUNC,	    (CMD_DEFINE)&command_newz);
//    addGmCommand("SETTYPE",         1, 15, CMD_TARGETID1,   (CMD_DEFINE)&target_s_settype);
//    addGmCommand("ITRIG",           1, 16, CMD_TARGETX,     (CMD_DEFINE)&target_s_itrig);
//    addGmCommand("CTRIG",           1, 17, CMD_TARGETX,     (CMD_DEFINE)&target_s_ctrig);
//    addGmCommand("TTRIG",		      1, 18, CMD_TARGETX,     (CMD_DEFINE)&target_s_ttrig);
//    addGmCommand("WTRIG",           1, 19, CMD_FUNC,        (CMD_DEFINE)&command_wtrig);
//    addGmCommand("SETID",           1, 20, CMD_TARGETHXY,   (CMD_DEFINE)&target_s_setid);
    addGmCommand("SETPRIV",         1, 21, CMD_FUNC,        (CMD_DEFINE)&command_setpriv);
    addGmCommand("DECAY",           1, 22, CMD_FUNC,        (CMD_DEFINE)&command_decay);
    addGmCommand("SHOWTIME",        1, 23, CMD_FUNC,        (CMD_DEFINE)&command_showtime);
//    addGmCommand("SETMORE",         1, 24, CMD_TARGETHID4,  (CMD_DEFINE)&target_s_setmore);
    addGmCommand("SHUTDOWN",        1, 25, CMD_FUNC,        (CMD_DEFINE)&command_shutdown);
    addGmCommand("MAKEGM",          1, 26, CMD_TARGET,      (CMD_DEFINE)&target_s_makegm);
    addGmCommand("MAKECNS",         1, 27, CMD_TARGET,      (CMD_DEFINE)&target_s_makecns);
    addGmCommand("KILLHAIR",        1, 28, CMD_TARGET,      (CMD_DEFINE)&target_s_killhair);
    addGmCommand("KILLBEARD",       1, 29, CMD_TARGET,      (CMD_DEFINE)&target_s_killbeard);
    addGmCommand("EMPTYPACK",        1, 30, CMD_TARGET,      (CMD_DEFINE)&target_s_emptypack);
//    addGmCommand("SETFONT",         1, 31, CMD_TARGETHID1,  (CMD_DEFINE)&target_s_setfont);
    addGmCommand("APPETITE",        1, 6,  CMD_FUNC,        (CMD_DEFINE)&command_appetite);
//BYTE-2
//    addGmCommand("WHOLIST",				2, 0,  CMD_FUNC,        (CMD_DEFINE)&command_wholist);
    addGmCommand("SKILLS",          2, 0,  CMD_FUNC,        (CMD_DEFINE)&command_skills);
    addGmCommand("PLAYERLIST",      2, 0,  CMD_FUNC,        (CMD_DEFINE)&command_playerlist); // other dupes
    addGmCommand("PL",              2, 0,	 CMD_FUNC,        (CMD_DEFINE)&command_playerlist);
    addGmCommand("KILL",            2, 1,	 CMD_TARGET,	  (CMD_DEFINE)&target_s_kill);
    addGmCommand("RESURRECT",	      2, 2,  CMD_TARGET,      (CMD_DEFINE)&target_s_resurrect);
    addGmCommand("RES",             2, 2,  CMD_TARGET,	  (CMD_DEFINE)&target_s_resurrect);
    addGmCommand("BOLT",            2, 3,	 CMD_TARGET,      (CMD_DEFINE)&target_s_bolt);
    addGmCommand("SFX",             2, 4,  CMD_FUNC,        (CMD_DEFINE)&command_sfx);
    addGmCommand("NPCACTION",       2, 5,  CMD_TARGETHID1,  (CMD_DEFINE)&target_s_npcaction);
    addGmCommand("LIGHT",           2, 6,  CMD_FUNC,        (CMD_DEFINE)&command_light);
    addGmCommand("SETAMOUNT",       2, 7,  CMD_TARGETX,     (CMD_DEFINE)&target_s_setamount);
//    addGmCommand("SETAMOUNT2",	  2, 8,	 CMD_TARGETX,	  (CMD_DEFINE)&target_s_setamount2);
    addGmCommand("DISCONNECT",      2, 9,	 CMD_FUNC,        (CMD_DEFINE)&command_disconnect);
    addGmCommand("KICK",            2, 10, CMD_TARGET,      (CMD_DEFINE)&target_s_kick);
    addGmCommand("TELL",            2, 11, CMD_FUNC,        (CMD_DEFINE)&command_tell);
    addGmCommand("DRY",             2, 12, CMD_FUNC,        (CMD_DEFINE)&command_dry);
    addGmCommand("RAIN",            2, 12, CMD_FUNC,        (CMD_DEFINE)&command_rain);
    addGmCommand("SNOW",            2, 12, CMD_FUNC,        (CMD_DEFINE)&command_snow);
    addGmCommand("SETSEASON",       2, 12, CMD_FUNC,        (CMD_DEFINE)&command_setseason);
//    addGmCommand("ECLIPSE",         2, 13, CMD_FUNC,        (CMD_DEFINE)&command_eclipse);
    addGmCommand("SEND",            2, 14, CMD_FUNC,        (CMD_DEFINE)&command_send);
    addGmCommand("BLT2",            2, 14, CMD_FUNC,        (CMD_DEFINE)&command_blt2);
    addGmCommand("WEB",             2, 14, CMD_FUNC,        (CMD_DEFINE)&command_web);
    addGmCommand("TEMP",            2, 14, CMD_FUNC,        (CMD_DEFINE)&command_temp);
    addGmCommand("GMMENU",          2, 15, CMD_FUNC,        (CMD_DEFINE)&command_gmmenu);
    addGmCommand("ITEMMENU",        2, 16, CMD_FUNC,        (CMD_DEFINE)&command_itemmenu);
    addGmCommand("ADDITEM",         2, 17, CMD_FUNC,        (CMD_DEFINE)&command_additem);
    addGmCommand("DUPE",            2, 18, CMD_FUNC,        (CMD_DEFINE)&command_dupe);
//    addGmCommand("MOVETOBAG",       2, 19, CMD_TARGET,      (CMD_DEFINE)&target_s_movetobag);
    addGmCommand("COMMAND",         2, 20, CMD_FUNC,        (CMD_DEFINE)&command_command);
    addGmCommand("GCOLLECT",        2, 21, CMD_FUNC,        (CMD_DEFINE)&command_gcollect);
    addGmCommand("ALLMOVEON",       2, 22, CMD_FUNC,        (CMD_DEFINE)&command_allmoveon);
    addGmCommand("ALLMOVEOFF",      2, 23, CMD_FUNC,        (CMD_DEFINE)&command_allmoveoff);
    addGmCommand("SHOWHS",          2, 24, CMD_FUNC,        (CMD_DEFINE)&command_showhs);
    addGmCommand("HIDEHS",          2, 25, CMD_FUNC,        (CMD_DEFINE)&command_hidehs);
//    addGmCommand("SETMOVABLE",      2, 26, CMD_TARGETX,     (CMD_DEFINE)&target_s_setmovable);
    addGmCommand("SET",             2, 27, CMD_FUNC,        (CMD_DEFINE)&command_set);
//    addGmCommand("SETVISIBLE",      2, 28, CMD_TARGETX,     (CMD_DEFINE)&target_s_setvisible);
//    addGmCommand("SETDIR",          2, 29, CMD_TARGETX,     (CMD_DEFINE)&target_s_setdir);
//    addGmCommand("SETSPEECH",       2, 30, CMD_TARGETX,     (CMD_DEFINE)&target_s_setspeech);
//    addGmCommand("SETOWNER",        2, 31, CMD_TARGETHID4,  (CMD_DEFINE)&target_s_setowner);
//BYTE-3
    addGmCommand("ADDNPC",          3, 0,  CMD_FUNC,        (CMD_DEFINE)&command_addnpc);
    addGmCommand("FREEZE",          3, 1,  CMD_TARGET,      (CMD_DEFINE)&target_s_freeze);
    addGmCommand("UNFREEZE",        3, 2,  CMD_TARGET,      (CMD_DEFINE)&target_s_unfreeze);
// 3.3 free!!
//    addGmCommand("GUMPMENU",        3, 4,  CMD_FUNC,        (CMD_DEFINE)&command_gumpmenu);
    addGmCommand("TILEDATA",        3, 5,  CMD_TARGET,      (CMD_DEFINE)&target_s_tiledata);
//    addGmCommand("RECALL",          3, 6,  CMD_TARGET,      (CMD_DEFINE)&target_s_recall);
//    addGmCommand("MARK",            3, 7,  CMD_TARGET,      (CMD_DEFINE)&target_s_mark);
//    addGmCommand("GATE",            3, 8,  CMD_TARGET,      (CMD_DEFINE)&target_s_gate);
    addGmCommand("HEAL",            3, 9,  CMD_TARGET,      (CMD_DEFINE)&target_s_heal);
//    addGmCommand("NPCTARGET",       3, 10, CMD_TARGET,      (CMD_DEFINE)&target_s_npctarget);
//    addGmCommand("CACHESTATS",      3, 11, CMD_FUNC,        (CMD_DEFINE)&command_cachestats);
    addGmCommand("NPCRECT",         3, 12, CMD_FUNC,        (CMD_DEFINE)&command_npcrect);
    addGmCommand("NPCCIRCLE",       3, 13, CMD_FUNC,        (CMD_DEFINE)&command_npccircle);
    addGmCommand("NPCWANDER",       3, 14, CMD_FUNC,        (CMD_DEFINE)&command_npcwander);
    addGmCommand("NPCRECTCODED",    3, 12, CMD_FUNC,        (CMD_DEFINE)&command_npcrectcoded);
    addGmCommand("TWEAK",			3, 15, CMD_FUNC,		(CMD_DEFINE)&command_tweak);
//    addGmCommand("SBOPEN",          3, 16, CMD_TARGET,      (CMD_DEFINE)&target_s_sbopen);
	addGmCommand("SECONDSPERUOMINUTE", 3, 17, CMD_FUNC,        (CMD_DEFINE)&command_secondsperuominute);
    addGmCommand("BRIGHTLIGHT",     3, 18, CMD_FUNC,        (CMD_DEFINE)&command_brightlight);
    addGmCommand("DARKLIGHT",       3, 19, CMD_FUNC,        (CMD_DEFINE)&command_darklight);
    addGmCommand("DUNGEONLIGHT",    3, 20, CMD_FUNC,        (CMD_DEFINE)&command_dungeonlight);
    addGmCommand("TIME",            3, 21, CMD_FUNC,        (CMD_DEFINE)&command_time);
    addGmCommand("MANA",            3, 22, CMD_TARGET,      (CMD_DEFINE)&target_s_mana);
    addGmCommand("STAMINA",         3, 23, CMD_TARGET,      (CMD_DEFINE)&target_s_stamina);
    addGmCommand("GMOPEN",          3, 24, CMD_FUNC,        (CMD_DEFINE)&command_gmopen);
//    addGmCommand("MAKESHOP",        3, 25, CMD_TARGET,      (CMD_DEFINE)&target_s_makeshop);
//    addGmCommand("BUY",             3, 26, CMD_TARGET,      (CMD_DEFINE)&target_s_buy);
//    addGmCommand("SETVALUE",        3, 27, CMD_TARGETX,     (CMD_DEFINE)&target_s_setvalue);
//    addGmCommand("SETRESTOCK",      3, 28, CMD_TARGETX,     (CMD_DEFINE)&target_s_setrestock);
    addGmCommand("RESTOCK",         3, 29, CMD_FUNC,        (CMD_DEFINE)&command_restock);
    addGmCommand("RESTOCKALL",      3, 30, CMD_FUNC,        (CMD_DEFINE)&command_restockall);
 addGmCommand("SETSHOPRESTOCKRATE", 3, 31, CMD_FUNC,        (CMD_DEFINE)&command_setshoprestockrate);
//BYTE-4
    addGmCommand("WHO",             4, 0,  CMD_FUNC,        (CMD_DEFINE)&command_who);
    addGmCommand("GMS",             4, 1,  CMD_FUNC,        (CMD_DEFINE)&command_gms);
//    addGmCommand("SELL",            4, 2,  CMD_TARGET,      (CMD_DEFINE)&target_s_sell);
    addGmCommand("MIDI",            4, 3,  CMD_FUNC,        (CMD_DEFINE)&command_midi);
//    addGmCommand("GUMPOPEN",        4, 4,  CMD_FUNC,        (CMD_DEFINE)&command_gumpopen);
    addGmCommand("RESPAWN",         4, 5,  CMD_FUNC,        (CMD_DEFINE)&command_respawn);
    addGmCommand("REGSPAWNALL",     4, 5,  CMD_FUNC,        (CMD_DEFINE)&command_regspawnall);
    addGmCommand("REGSPAWNMAX",     4, 5,  CMD_FUNC,        (CMD_DEFINE)&command_regspawnmax);
    addGmCommand("REGSPAWN",        4, 5,  CMD_FUNC,        (CMD_DEFINE)&command_regspawn);
    addGmCommand("REGEDIT",         4, 5,  CMD_FUNC,        (CMD_DEFINE)&command_regedit);
//    addGmCommand("SETSPATTACK",     4, 6,  CMD_TARGETHTMP,  (CMD_DEFINE)&target_s_setspattack);
//    addGmCommand("SETSPADELAY",     4, 7,  CMD_TARGETHTMP,  (CMD_DEFINE)&target_s_setspadelay);
//    addGmCommand("SETPOISON",       4, 8,  CMD_TARGETHTMP,  (CMD_DEFINE)&target_s_setpoison);
//    addGmCommand("SETPOISONED",     4, 9,  CMD_TARGETHTMP,  (CMD_DEFINE)&target_s_setpoisoned);
//    addGmCommand("SETADVOBJ",       4, 10, CMD_TARGETHTMP,  (CMD_DEFINE)&target_s_setadvobj);
//    addGmCommand("SETWIPE",         4, 11, CMD_TARGETID1,	  (CMD_DEFINE)&target_s_setwipe);
    addGmCommand("FULLSTATS",       4, 12, CMD_TARGET,      (CMD_DEFINE)&target_s_fullstats);
    addGmCommand("HIDE",            4, 13, CMD_TARGET,      (CMD_DEFINE)&target_s_hide);
    addGmCommand("UNHIDE",          4, 14, CMD_TARGET,      (CMD_DEFINE)&target_s_unhide);
    addGmCommand("RELOADSERVER",    4, 15, CMD_FUNC,        (CMD_DEFINE)&command_reloadserver);
//    addGmCommand("READINI",         4, 15, CMD_FUNC,        (CMD_DEFINE)&command_readini);
    addGmCommand("LOADDEFAULTS",    4, 16, CMD_FUNC,        (CMD_DEFINE)&command_loaddefaults);
    addGmCommand("CQ",              4, 17, CMD_FUNC,        (CMD_DEFINE)&command_cq);
//    addGmCommand("WIPENPCS",        4, 18, CMD_FUNC,        (CMD_DEFINE)&command_wipenpcs);
    addGmCommand("CNEXT",           4, 19, CMD_FUNC,        (CMD_DEFINE)&command_cnext);
    addGmCommand("CCLEAR",          4, 20, CMD_FUNC,        (CMD_DEFINE)&command_cclear);
    addGmCommand("MINECHECK",       4, 21, CMD_FUNC,        (CMD_DEFINE)&command_minecheck);
    addGmCommand("INVUL",           4, 22, CMD_FUNC,        (CMD_DEFINE)&command_invul);
    addGmCommand("NOINVUL",         4, 23, CMD_FUNC,        (CMD_DEFINE)&command_noinvul);
    addGmCommand("GUARDSON",        4, 24, CMD_FUNC,        (CMD_DEFINE)&command_guardson);
    addGmCommand("GUARDSOFF",       4, 25, CMD_FUNC,        (CMD_DEFINE)&command_guardsoff);
//    addGmCommand("HOUSE",           4, 26, CMD_TARGETHTMP,  (CMD_DEFINE)&target_s_house);
//    addGmCommand("ANNOUNCEON",      4, 27, CMD_FUNC,        (CMD_DEFINE)&command_announceon);
    //addGmCommand("ANNOUNCEOFF",     4, 28, CMD_FUNC,        (CMD_DEFINE)&command_announceoff);
//    addGmCommand("WF",              4, 29, CMD_FUNC,        (CMD_DEFINE)&command_wf);  ndEndy unused so remove
    addGmCommand("NODECAY",         4, 30, CMD_FUNC,        (CMD_DEFINE)&command_nodecay);
//    addGmCommand("SPLIT",           4, 31, CMD_TARGETHTMP,  (CMD_DEFINE)&target_s_split);
//    addGmCommand("SPLITCHANCE",     4, 31, CMD_TARGETHTMP,  (CMD_DEFINE)&target_s_splitchance);
//BYTE-5
//    addGmCommand("WANIM",           5, 0,  CMD_FUNC,        (CMD_DEFINE)&command_wanim);
    addGmCommand("POSSESS",         5, 1,  CMD_TARGET,      (CMD_DEFINE)&target_s_possess);
    addGmCommand("SETTIME",         5, 2,  CMD_FUNC,        (CMD_DEFINE)&command_settime);
//    addGmCommand("KILLALL",         5, 3,  CMD_FUNC,        (CMD_DEFINE)&command_killall);
    addGmCommand("PDUMP",           5, 4,  CMD_FUNC,        (CMD_DEFINE)&command_pdump);
//    addGmCommand("READSPAWNREGIONS",5, 6,  CMD_FUNC,        (CMD_DEFINE)&command_readspawnregions);
//    addGmCommand("CLEANUP",         5, 7,  CMD_FUNC,        (CMD_DEFINE)&command_cleanup);
    addGmCommand("GY",              5, 8,  CMD_FUNC,        (CMD_DEFINE)&command_gy);
//    addGmCommand("TILEW",           5, 9,  CMD_FUNC,        (CMD_DEFINE)&command_tilew);
    addGmCommand("SQUELCH",         5, 10, CMD_FUNC,        (CMD_DEFINE)&command_squelch);
    addGmCommand("MUTE",            5, 10, CMD_FUNC,        (CMD_DEFINE)&command_squelch);
    addGmCommand("TELESTUFF",       5, 11, CMD_TARGET,	    (CMD_DEFINE)&target_s_telestuff);
    addGmCommand("SPAWNKILL",       5, 12, CMD_FUNC,        (CMD_DEFINE)&command_spawnkill);
    //addGmCommand("SHOWPRIV3",       5, 13, CMD_TARGET,      (CMD_DEFINE)&target_s_showpriv3);
//    addGmCommand("NEWX",            5, 14, CMD_TARGETX,     (CMD_DEFINE)&target_s_newx);
//    addGmCommand("NEWY",            5, 14, CMD_TARGETX,     (CMD_DEFINE)&target_s_newy);
//    addGmCommand("INCX",            5, 14, CMD_TARGETX,     (CMD_DEFINE)&target_s_incx);
//    addGmCommand("INCY",            5, 14, CMD_TARGETX,     (CMD_DEFINE)&target_s_incy);

//    addGmCommand("INCZ",            5, 14, CMD_TARGETX,     (CMD_DEFINE)&target_s_incz);
//CMD_MANAGEDCMD

    addGmCommand("SHOWP",           5, 15, CMD_FUNC,        (CMD_DEFINE)&command_showp);
    addGmCommand("YELL",            5, 15, CMD_FUNC,        (CMD_DEFINE)&command_yell);
//5.16&5.17 free
//to find a slot
    addGmCommand("CFG",             5, 18, CMD_FUNC,        (CMD_DEFINE)&command_cfg);
    addGmCommand("PASSWORD",        5, 19, CMD_FUNC,        (CMD_DEFINE)&command_password);
    addGmCommand("READACCOUNTS",    5, 20, CMD_FUNC,        (CMD_DEFINE)&command_readaccounts);
//    addGmCommand("LETUSIN",         5, 21, CMD_FUNC,        (CMD_DEFINE)&command_letusin);
    addGmCommand("SERVERSLEEP",     5, 22, CMD_FUNC,        (CMD_DEFINE)&command_serversleep);
//addGmCommand("RELOADCACHEDSCRIPTS", 5, 23, CMD_FUNC,        (CMD_DEFINE)&command_reloadcachedscripts);
//    addGmCommand("SETHOME",         5, 24, CMD_TARGETXYZ,	  (CMD_DEFINE)&target_s_sethome);
//    addGmCommand("SETWORK",         5, 24, CMD_TARGETXYZ,   (CMD_DEFINE)&target_s_setwork);
//    addGmCommand("SETFOOD",         5, 24, CMD_TARGETXYZ,   (CMD_DEFINE)&target_s_setfood);
//{"GLOW",        5,  25, CMD_TARGET,		(CMD_DEFINE)&target_s_glow},
//{"UNGLOW",      5,  25, CMD_TARGET,		(CMD_DEFINE)&target_s_unglow},
    addGmCommand("SETMURDER",       5, 26, CMD_FUNC,        (CMD_DEFINE)&command_setmurder);//taken from 6904t2(5/10/99) - AntiChrist
    addGmCommand("SETMENUPRIV",     5, 27, CMD_FUNC,        (CMD_DEFINE)&command_setmenupriv);
    addGmCommand("SPY",				5, 28, CMD_TARGET,      (CMD_DEFINE)&target_s_spy);
//    addGmCommand("DELID",           5, 29, CMD_FUNC,        (CMD_DEFINE)&command_delid);
//    addGmCommand("DELTYPE",         5, 30, CMD_FUNC,        (CMD_DEFINE)&command_deltype);
    addGmCommand("SYSM",            5, 31, CMD_FUNC,        (CMD_DEFINE)&command_sysm);
}

P_COMMAND cCommandTable::addGmCommand(char* name, int privm, int privb, int type, void command()) {
    P_COMMAND cmd= new cCommand(name, privm, privb, type, command, false);
	P_COMMAND old= command_map[name];
    command_map[name]= cmd;
	return old;
}

P_COMMAND cCommandTable::addGmCommand(P_COMMAND cmd) {
	P_COMMAND old= command_map[cmd->cmd_name];
    command_map[cmd->cmd_name]= cmd;
	return old;
}

P_COMMAND cCommandTable::addPlayerCommand(char* name, int privb, int type, void command()) {
	P_COMMAND old= command_map[name];
    P_COMMAND cmd= new cCommand(name, 255, privb, type, command, false);
    command_map[name]= cmd;
	return old;
}

P_COMMAND cCommandTable::findCommand(char* name) {
    return command_map[name];
}

td_cmditer cCommandTable::getIteratorBegin() {
    return command_map.begin();
}

td_cmditer cCommandTable::getIteratorEnd() {
    return command_map.end();
}

// Class cCommandStep implemetation
cCommandStep::cCommandStep(void cmd(NXWCLIENT)) {
    command= cmd;
    nextStep= NULL;
}

void cCommandStep::execute(NXWCLIENT client) {
    command(client);
}

cCommandTable* commands = new cCommandTable();

static char *ch="abcdefg";

/* Actual commands go below. Define all the dynamic command_ functions as
 * the name of the function in lowercase, a void function, accepting one
 * integer, which is the player # that sent the command.
 * Like this:
 * void command_rain(int s)
 *
 * If you stick comments (double-slash comments) between the function definition
 * and the opening block, they will automatically be used as the
 * HTML documentation for that command. The first line will be used as the
 * short documentation, any subsequent lines will be used as a seperate
 * "more info" page for your command. HTML is allowed, but remember that
 * everything is fit into a template.
 */

// When player has been murdered and they are
// a ghost, they can use the bounty command to
// place a bounty on the murderers head

void command_bounty(NXWSOCKET  s)
{
	// Check if boountys are active

	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	if( !SrvParms->bountysactive )
	{
		pc_cs->sysmsg(TRANSLATE("The bounty system is not active."));
		return;
	}

	if( !pc_cs->dead )
	{
		pc_cs->sysmsg(TRANSLATE("You can only place a bounty while you are a ghost."));
		pc_cs->murdererSer = 0;
		return;
	}

	if( pc_cs->murdererSer == 0 )
	{
		pc_cs->sysmsg(TRANSLATE("You can only place a bounty once after someone has murdered you."));
		return;
	}

	if( tnum == 2 )
	{
		int nAmount = strtonum(1);
		if( BountyWithdrawGold( pc_cs, nAmount ) )
		{
			P_CHAR pchar_murderer = pointers::findCharBySerial(pc_cs->murdererSer);
			if( BountyCreate( pchar_murderer, nAmount ) )
			{
				if(ISVALIDPC(pchar_murderer)) 
				pc_cs->sysmsg(TRANSLATE("You have placed a bounty of %d gold coins on %s."), 
						nAmount, pchar_murderer->getCurrentNameC() );
			}
			else
			{
				pc_cs->sysmsg(TRANSLATE("You were not able to place a bounty (System Error)") );
			}

			// Set murdererSer to 0 after a bounty has been
			// placed so it can only be done once
			pc_cs->murdererSer = 0;
		}
		else
		{
			pc_cs->sysmsg(TRANSLATE("You do not have enough gold to cover the bounty."));
		}
	}
	else
	{
		pc_cs->sysmsg(TRANSLATE("To place a bounty on a murderer, use BOUNTY <amount>"));
	}

	return;
}


void command_serversleep(NXWSOCKET  s)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	int seconds;

#ifdef __BEOS__
	pc->sysmsg("Command not supported under BeOS");
#else

	if (tnum==2)
	{
		seconds = strtonum(1);
		sysbroadcast(TRANSLATE("server is going to sleep for %i seconds!"),seconds); // broadcast server sleep

		NxwSocketWrapper sw;
		sw.fillOnline();

		for( ; !sw.getClient(); sw++ ) {
			NXWCLIENT ps=sw.getClient();
			if( ps!=NULL )
				Network->FlushBuffer(ps->toInt());
		}

		seconds=seconds*1000;
		Sleep(seconds);
		sysbroadcast(TRANSLATE("server is back from a %i second break"),seconds/1000);

		sw.clear();
		sw.fillOnline();

		for( ; !sw.getClient(); sw++ ) { //not absolutely necessary
			NXWCLIENT ps=sw.getClient();
			if( ps!=NULL )
				Network->FlushBuffer(ps->toInt());
		}
	}
	else 
	{
		pc->sysmsg("Invalid number of arguments");
	}
#endif
}

//
// Sparhawk enable online reloading of race scripts, race scripts are NOT cached all data is converted to binary
//
void command_reloadracescript(NXWSOCKET  s)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	Race::reload( "scripts/race.xss" );
	Race::parse();
	pc->sysmsg("Racescripts reloaded.");
}

// Returns the current bulletin board posting mode for the player
void command_post(NXWSOCKET  s)
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	strcpy( s_szCmdTableTemp, "You are currently posting " );

	switch( pc_cs->postType )
	{
		case MsgBoards::LOCALPOST:
			strcat( s_szCmdTableTemp, "a message to a single board [LOCAL]." );
			break;

		case MsgBoards::REGIONALPOST:
			strcat( s_szCmdTableTemp,"a message to all boards in this area [REGIONAL].");
			break;

		case MsgBoards::GLOBALPOST:
			strcat( s_szCmdTableTemp,"a message to all boards in the world [GLOBAL]." );
			break;

	}

	pc_cs->sysmsg( s_szCmdTableTemp );
	sysmessage( s, s_szCmdTableTemp );
	return;
}

// Sets the current bulletin board posting mode for the player to GLOBAL
// ALL bulletin boards will see the next message posted to any bulletin board
void command_gpost(NXWSOCKET  s)
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	pc_cs->postType = MsgBoards::GLOBALPOST;
	pc_cs->sysmsg("Now posting GLOBAL messages." );
}

// Sets the current bulletin board posting mode for the player to REGIONAL
// All bulletin boards in the same region as the board posted to will show the
// next message posted
void command_rpost(NXWSOCKET  s)
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	pc_cs->postType = MsgBoards::REGIONALPOST;
	pc_cs->sysmsg("Now posting REGIONAL messages." );
}

// Sets the current bulletin board posting mode for the player to LOCAL
// Only this bulletin board will have this post
void command_lpost(NXWSOCKET  s)
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	pc_cs->postType = MsgBoards::REGIONALPOST;
	pc_cs->sysmsg("Now posting LOCAL messages." );
}



void target_setMurderCount( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc);

	pc->kills = t->buffer[0];
    setcharflag(pc);

}

void command_setmurder( NXWCLIENT ps )
{
	if( tnum == 2 )
	{
		P_TARGET targ = clientInfo[ ps->toInt() ]->newTarget( new cCharTarget() );
		targ->buffer[0]=strtonum(1);
		targ->send( ps );
		ps->sysmsg( "Select the person to set the murder count of: ");
	}
}


void command_readaccounts(NXWSOCKET  s)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	Accounts->LoadAccounts();
	pc->sysmsg("Accounts reloaded...attention, if you changed exisiting(!) account numbers you should use the letusin command afterwards ");
}

void command_showp(NXWSOCKET  s)
// Displays hex values of your PRIV3 settings.
{

	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	for( int i=0;i<7;i++)
	{
		sprintf(s_szCmdTableTemp, "priv3%c : %X ", ch[i],pc_cs->priv3[i]);
		pc_cs->sysmsg(s_szCmdTableTemp);
	}
}

/*!
\brief Alter GM Command Privs

\param s socket to set the priv3 to
\note <P>The Setpriv3 command has 3 different formats:</P>
<LI><B>One argument</B> - set PRIV3 to a template found in the game scripts
<LI><B>Two arguments</B> - SETPRIV3 [+ or -] COMMAND, example: To allow a user to
have access to the SAVE command, use <B>SETPRIV3 + SAVE</B>, to take awake a
user's access to MAKECNS, use <B>SETPRIV3 - MAKECNS</B></LI>
<LI><B>Six or Seven Arguments</B> - Manually set the entire setpriv3
structure as a sequence of six or seven eight-digit hexadecimal
numbers.</LI>

\note For more information on Meta-GM permissions, check out
<A HREF="http://stud4.tuwien.ac.at/%7Ee9425109/uox3_1.htm">
Lord Binary's UOX Site</A>.
*/

void target_setPriv3( NXWCLIENT ps, P_TARGET t )
{
    P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc);

	NXWSOCKET s = ps->toInt();

    pc->priv3[0]=priv3a[s];
    pc->priv3[1]=priv3b[s];
    pc->priv3[2]=priv3c[s];
    pc->priv3[3]=priv3d[s];
    pc->priv3[4]=priv3e[s];
    pc->priv3[5]=priv3f[s];
    pc->priv3[6]=priv3g[s];
}

void target_setPriv3X( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc = MAKE_CHAR_REF( t->getClicked() );
	VALIDATEPC(pc);

	NXWSOCKET socket = ps->toInt();

	int grantcmd=1;
	int revokecmd;

	P_COMMAND cmd = commands->findCommand( (char*)t->buffer_str[0].c_str() );

	if( t->buffer[0] )
	{
		pc->priv3[ cmd->cmd_priv_m ] |= ( grantcmd << cmd->cmd_priv_b );
		sysmessage( socket, "%s has been granted access to the %s command.", pc->getCurrentNameC(), cmd->cmd_name );
	}
	else
	{
		revokecmd = 0xFFFFFFFF - ( grantcmd << cmd->cmd_priv_b );
		pc->priv3[ cmd->cmd_priv_m ] &= revokecmd;
		sysmessage( socket, "%s has been revoked access to the %s command.", pc->getCurrentNameC(), cmd->cmd_name );
	}
}


void command_setpriv3( NXWCLIENT ps )
{

	P_CHAR pc_cs=ps->currChar();
	VALIDATEPC(pc_cs);

	NXWSOCKET s = ps->toInt();

	UI32 i;
	int y;

	switch(tnum) {
		case 7:
		case 8: {
			if (SrvParms->gm_log)
				WriteGMLog(pc_cs, "setpriv3 executed!\n");

			priv3a[s]=strtonum(1);
			priv3b[s]=strtonum(2);
			priv3c[s]=strtonum(3);
			priv3d[s]=strtonum(4);
			priv3e[s]=strtonum(5);
			priv3f[s]=strtonum(6);
			priv3g[s] = (tnum==8)? strtonum(7) : 0;

			P_TARGET targ = clientInfo[s]->newTarget( new cCharTarget() );
			targ->code_callback = target_setPriv3;
			targ->send( ps );
			ps->sysmsg( "Select player to set command clearance");
			}
			break;
		case 2: {
			y=strtonum(1);
			if (SrvParms->gm_log)
				WriteGMLog(pc_cs, "setpriv3 executed!\n");

			//AntiChrist-this was metagm[y%255]-
			if ((y>255)||(y<0)) //was (y<255), Ummon
			{
				LogError("setpriv3-command: avoiding crash. argument was out of range [0-255]!\n");
				pc_cs->sysmsg("Setpriv3-command argument has to be between 0 and 255.");
				return;
			}
			priv3a[s]=metagm[y][0];
			priv3b[s]=metagm[y][1];
			priv3c[s]=metagm[y][2];
			priv3d[s]=metagm[y][3];
			priv3e[s]=metagm[y][4];
			priv3f[s]=metagm[y][5];
			priv3g[s]=metagm[y][6];

			P_TARGET targ = clientInfo[s]->newTarget( new cCharTarget() );
			targ->code_callback = target_setPriv3;
			targ->send( ps );
			ps->sysmsg( "Select player to set command clearance");
			}
			break;
		case 3:	/* new code to allow + COMMAND or - COMMAND 07/24/99 */
			if((!strcmp((char*)comm[1], "+"))||(!strcmp((char*)comm[1], "-"))) {
				y=comm[1][0]=='+' ? 1 : 0;
				for (i=0;i<strlen((char*)comm[2]);i++) {
					comm[2][i]=toupper(comm[2][i]);
				}

				P_COMMAND cmd = commands->findCommand((char*)comm[2]);

				if(cmd==NULL) {
					pc_cs->sysmsg("That command doesn't exist.");
				} else if(cmd->cmd_priv_m==255) {
					pc_cs->sysmsg("No special permissions are neccessary to use that command.");
				} else {
					
					P_TARGET targ = clientInfo[s]->newTarget( new cCharTarget() );
					targ->code_callback = target_setPriv3X;
					targ->buffer[0]=y;
					targ->buffer_str[0] = cmd->cmd_name;
					targ->send( ps );
					ps->sysmsg( "Select player to alter %s command access.", cmd->cmd_name);
				}
			} else {
				pc_cs->sysmsg("2-Argument Usage: /SETPRIV3 +/- COMMAND");
			}
			break;
		default:
			pc_cs->sysmsg("This command takes 1, 2, 6, or 7 arguments.");
			break;
	}
}

/*!
\brief To spread priv3 among a class of characters
\param s the socket
\todo Script-defined ALL-types. Maybe only with NxW :)
\todo the function is empty, need to be write maybe?

See definitions of ALL-types below to have a list of possible class. Feel free to definite more yourself.
<b>Usage:</b> 'spreadpriv3 [ALL|ALLPG|ALLGM|ALLCNS|ALLGMCNS\ALLADMIN] [+/- COMMAND|SECTION-NUMBER]
*/
void command_spreadpriv3(NXWSOCKET  s)
{
        return;
        /*int ichar, icmd, imeta;
	int y, err, Condtype;
	bool Cond=false;
	bool allcommand=false;
	int loopexit=0, forexit=0;
	int grantcmd=1; //it was 0-0xFFFFFFFF
	int revokecmd;

    PC_CHAR pcc_cs=MAKE_CHARREF_LOGGED(currchar[s],err);
    if (err) return;


	if((tnum==3)||(tnum==4))
	{
		for (icmd=0; icmd<strlen((char*)comm[1]); icmd++)
		{
			comm[1][icmd]=toupper(comm[1][icmd]);
		}

		Condtype=0;
		if (!strcmp((char*)comm[1], "ALL"))
		{
			Condtype=1;
		}
		else if (!strcmp((char*)comm[1], "ALLPG"))
		{
			Condtype=2;
		}
		else if (!strcmp((char*)comm[1], "ALLGM"))
		{
			Condtype=3;
		}
		else if (!strcmp((char*)comm[1], "ALLCNS"))
		{
			Condtype=4;
		}
		else if (!strcmp((char*)comm[1], "ALLGMCNS"))
		{
			Condtype=5;
		}
		else if (!strcmp((char*)comm[1], "ALLADMIN"))
		{
			Condtype=6;
		}

		if (Condtype!=0)
		{
			//ALL-TYPES:
			//ALL every not-npc character
			//ALLPG every !(isTrueGM() || isCounselor()) character
			//ALLGM  every (isTrueGM()) character (but without all commands)
			//ALLCNS every (isCounselor()) character
			//ALLGMCNS every (isTrueGM() || isCounselor()) character (but without all commands)
			//ALLADMIN for now defined as GMs (isTrueGM()) who have all commands


			forexit=0;
			for(ichar=0; ((ichar<charcount)&&(forexit==0)); ichar++)
			{
				P_CHAR pc_i=MAKE_CHAR_REF(ichar);
				if(!ISVALIDPC(pc_i))
					continue;
				
				allcommand = (pc_i->priv3[0]==0xFFFFFFFF)
							&&(pc_i->priv3[1]==0xFFFFFFFF)
							&&(pc_i->priv3[2]==0xFFFFFFFF)
							&&(pc_i->priv3[3]==0xFFFFFFFF)
							&&(pc_i->priv3[4]==0xFFFFFFFF)
							&&(pc_i->priv3[5]==0xFFFFFFFF)
							&&(pc_i->priv3[6]==0xFFFFFFFF);

				Cond=false;

				switch (Condtype)
				{
				case 1: //ALL
					Cond=(pc_i->account>0);
					break;
				case 2: //ALLPG
					Cond=((pc_i->account>0) && !(pc_i->IsTrueGM() || pc_i->IsCounselor()));
					break;
				case 3: //ALLGM
					Cond=((pc_i->account>0) && (pc_i->IsTrueGM()) && !allcommand);
					break;
				case 4: //ALLCNS
					Cond=((pc_i->account>0) && (pc_i->IsCounselor()) && !(pc_i->IsTrueGM()));
					break;
				case 5: //ALLGMCNS
					Cond=((pc_i->account>0) && (pc_i->IsTrueGM() || pc_i->IsCounselor()) && !allcommand);
					break;
				case 6: //ALLADMIN
					Cond=((pc_i->account>0) && (pc_i->IsTrueGM()) && allcommand);
					break;
				default:
					Cond=false;
					break;
				}


				if (Cond)
				{
					if (tnum==4)
					{
						if((!strcmp((char*)comm[2], "+"))||(!strcmp((char*)comm[2], "-")))
						{
							for (icmd=0; icmd<strlen((char*)comm[3]); icmd++)
							{
								comm[3][icmd]=toupper(comm[3][icmd]);
							}     */

/*							icmd=0; z=-1; loopexit=0;
							while((command_table[icmd].cmd_name)&&(z==-1)&&(++loopexit < MAXLOOPS))
							{
								if(!(strcmp((char*)command_table[icmd].cmd_name, (char*)comm[3]))) z=icmd;
								icmd++;
							} */
							
							/*P_COMMAND cmd = commands->findCommand((char*)comm[3]);

							if(cmd==NULL)
							{
								sysmessage(s, "That command doesn't exist.");
								forexit=1;
							}
							else
							{
								if(cmd->cmd_priv_m==255)
								{
									sysmessage(s, "No special permissions are neccessary to use that command.");
									forexit=1;
								}
								else
								{
									y=comm[2][0]=='+' ? 1 : 0;
									if (y)
									{
										pc_i->priv3[cmd->cmd_priv_m] |= (grantcmd<<cmd->cmd_priv_b);
									}
									else
									{
										revokecmd=0xFFFFFFFF-(grantcmd<<cmd->cmd_priv_b); //Ummon
										pc_i->priv3[cmd->cmd_priv_m] &= revokecmd;
									}
//									sysmessage(s, temp);
								}
							}
						}
					}
					else if (tnum==3)
					{
						y=strtonum(2);
						if((y>255)||(y<0))
						{
							LogError("spreadpriv3-command: avoiding crash. argument was out of range [0-255]!\n");
							sysmessage(s,"Spreadpriv3-command argument has to be between 0 and 255.");
							return;
						}

						for (imeta=0; imeta<7; imeta++) {
							pc_i->priv3[imeta]=metagm[y][imeta]; 
						}
					}
					else
					{
						sysmessage(s, "2 or 3 Arguments Usage: /SPREADPRIV3 [ALL,ALLPG,ALLGM,ALLCNS,ALLGMCNS,ALLADMIN] [+/- COMMAND,SECTION-NUMBER]");
					}
				}
			}
		}
		else
		{
			sysmessage(s, "2 or 3 Arguments Usage: /SPREADPRIV3 [ALL,ALLPG,ALLGM,ALLCNS,ALLGMCNS,ALLADMIN] [+/- COMMAND,SECTION-NUMBER]");
		}
	}
	else
	{
		sysmessage(s, "2 or 3 Arguments Usage: /SPREADPRIV3 [ALL,ALLPG,ALLGM,ALLCNS,ALLGMCNS,ALLADMIN] [+/- COMMAND,SECTION-NUMBER]");
	} */
}

/*!
\brief Resends server data to client
\param s the socket
*/
void command_resend(NXWSOCKET s)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	pc->teleport();
}

void command_teleport(NXWSOCKET s)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	pc->teleport();
}

/*!
\brief Prints your current coordinates + region
\param s the socket
*/
void command_where(NXWSOCKET  s)
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	Location charpos= pc_cs->getPosition();

	if (strlen(region[pc_cs->region].name)>0)
		pc_cs->sysmsg("You are at: %s",region[pc_cs->region].name);
	else 
		pc_cs->sysmsg("You are at: unknown area");

	pc_cs->sysmsg("%i %i (%i)", charpos.x, charpos.y, charpos.z);
}

// Shows the GM or Counsellor queue.
void command_q(NXWSOCKET  s)
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	if (!pc_cs->IsGM()) //They are not a GM
		Commands::ShowGMQue(s, 0);
	else
		Commands::ShowGMQue(s, 1); // They are a GM
}

// For Counselors or GM's, goes to next call in queue.
void command_next(NXWSOCKET  s)
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	if (!pc_cs->IsGM()) //They are not a GM
	   Commands::NextCall(s, 0);
	else
	   Commands::NextCall(s, 1); // They are a GM
}

void command_clear(NXWSOCKET  s)
// For Counselor's and GM's, removes current call from queue.
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	if (!pc_cs->IsGM()) //They are not a GM
	   donewithcall(s, 0);
	else
	   donewithcall(s, 1); // They are a GM
}

void command_goplace(NXWSOCKET  s)
// (d) Teleports you to a location from the LOCATIONS.SCP file.
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	if (tnum==2)
	{
		pc_cs->doGmEffect();

		pc_cs->goPlace( strtonum(1) );
		pc_cs->teleport();

		pc_cs->doGmEffect();
	}
}

// (h h h h) Teleports you to another character.
void command_gochar(NXWSOCKET  s)
{

	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	PC_CHAR pc_i=NULL;

	switch( tnum ) {
		case 5:	{
		
			Serial serial;
			serial.ser1=strtonum(1);
			serial.ser2=strtonum(2);
			serial.ser3=strtonum(3);
			serial.ser4=strtonum(4);

			pc_i = pointers::findCharBySerial( serial.serial32 );
			break;
		}
		case 2: {
		
			NXWCLIENT ps=getClientFromSocket( strtonum(1) );
			if( ps==NULL )
				return;

			pc_i = ps->currChar();
			break;
		}
		default:
			return;
	}

	if( ISVALIDPC(pc_i) ) {

		pc_cs->doGmEffect();
		
		pc_cs->MoveTo( pc_i->getPosition() );
		pc_cs->teleport();

		pc_cs->doGmEffect();

	}
}

void command_fix(NXWSOCKET  s)
// Try to compensate for messed up Z coordinates. Use this if you find yourself half-embedded in the ground.
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	Location charpos= pc_cs->getPosition();

	if (err)
		return;

	if (tnum == 2)
	{
		if (validtelepos(pc_cs)==-1)
		{
			charpos.dispz= charpos.z= strtonum(1);

		}
		else
		{
			charpos.dispz= charpos.z= validtelepos(pc_cs);
		}
		pc_cs->teleport();
	}

	pc_cs->setPosition( charpos );
	return;
}

void target_xgo( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc = pointers::findCharBySerial( t->getClicked() );
	if(ISVALIDPC(pc)) {
	    pc->MoveTo( t->buffer[0], t->buffer[1], t->buffer[2] );
		pc->teleport();
    }
}

void command_xgoplace( NXWCLIENT ps )
// (d) Send another character to a location in your LOCATIONS.SCP file.
{
	if (tnum==2)
	{
		int x, y, z;
		location2xyz( strtonum(1), x, y, z );
		if( x>0 )
		{
			NXWSOCKET s = ps->toInt();
			P_TARGET targ = clientInfo[s]->newTarget( new cCharTarget() );
			targ->code_callback = target_xgo;
			targ->buffer[0]=x;
			targ->buffer[1]=y;
			targ->buffer[2]=z;
			targ->send( ps );
			ps->sysmsg( "Select char to teleport.");
		}
	}

}

void command_showids(NXWSOCKET  s)
// Display the serial number of every item on your screen.
{
	
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);

	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( pc_cs->getPosition() );
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pc=sc.getChar();
		if(ISVALIDPC(pc)) {
			pc->showLongName( pc_cs, true );
		}
	}

}

void command_poly(NXWSOCKET  s)
// (h h) Polymorph yourself into any other creature.
{
	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_currchar);

	if (tnum==3)
	{
		int k,c1;

                k = (strtonum(1)<<8) | strtonum(2);
                if (k>=0x000 && k<=0x3E1) // lord binary, body-values >0x3e crash the client
		{
			pc_currchar->SetBodyType(k);
			pc_currchar->SetOldBodyType(k);

			c1= pc_currchar->getSkinColor();					// transparency for mosnters allowed, not for players,
											// if polymorphing from monster to player we have to switch from transparent to semi-transparent
											// or we have that sit-down-client crash

			if ((c1 & 0x4000) && (k >= BODY_MALE && k<= 0x03E1))
			{
				if (c1!=0x8000)
				{
					pc_currchar->setSkinColor(0xF000);
					pc_currchar->setOldSkinColor(0xF000);
				}
			}
		}
	}

	pc_currchar->teleport();
}

void command_skin(NXWSOCKET  s)
// (h h) Change the hue of your skin.
{
	if (tnum == 3)
	{
		int k, body;

		P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);

		body = pc_currchar->GetBodyType();
		k = (strtonum(1,16) << 8) | strtonum(2,16);
		if ((k & 0x4000) && (body >= BODY_MALE && body <= 0x03E1))
			k = 0xF000;

		if (k != 0x8000)
		{
			pc_currchar->setSkinColor(k);
			pc_currchar->setOldSkinColor(k);
			pc_currchar->teleport();
		}
	}
}

void command_action(NXWSOCKET  s)
// (h) Preform an animated action sequence.
{
	if (tnum==2)
	{
		P_CHAR pc= MAKE_CHAR_REF(currchar[s]);
		pc->playAction(strtonum(1));
	}
	return;
}

void command_setseason(NXWSOCKET  s)
// Sets the season globally
// Season change packet structure:
// BYTE 0xBC	(Season change command)
// BYTE season	(0 spring, 1 summer, 2 fall, 3 winter, 4 dead, 5 unknown (rotating?))
// BYTE unknown	If 0, cannot change from undead, so 1 is the default
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	UI08 setseason[3]={ 0xBC, 0x00, 0x01 };

	pc->sysmsg("Plz, notice that setseason may or may not work correctly depending on current calendar status");

	if(tnum==2)
	{
		setseason[1]=strtonum(1);
		season=(int)setseason[1];
		NxwSocketWrapper sw;
		sw.fillOnline();
		for( sw.rewind(); !sw.isEmpty(); sw++ ) {
			Xsend( sw.getSocket(), setseason, 3);
//AoS/			Network->FlushBuffer(sw.getSocket());
		}
	}
	else
		pc->sysmsg("Setseason takes one argument.");
}

void target_xtele( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc=pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc);

	P_CHAR curr=ps->currChar();

	pc->MoveTo( curr->getPosition() );
	pc->teleport();
}

void command_xtele( NXWCLIENT ps )
// (nothing) Teleport a player to your position.
{
	P_TARGET targ = clientInfo[ps->toInt()]->newTarget( new cCharTarget() );
	targ->code_callback = target_xtele;
	targ->send( ps );
	ps->sysmsg( "Select char to teleport to your position." );
}

void command_go(NXWSOCKET  s)
// (d d d) Go to the specified X/Y/Z coordinates
{
	if (tnum==4)
	{
		int x=strtonum(1);int y=strtonum(2);int z=strtonum(3); // LB
		if (x<6144 && y<4096 && z>-127 && z<128)
		{

			P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
			pc_currchar->doGmEffect(); 	// Adding the gmmove effects ..Aldur
			

			pc_currchar->MoveTo( x,y,z );
			pc_currchar->teleport();

			pc_currchar->doGmEffect();
		}
	}
	return;
}

void command_zerokills(NXWSOCKET  s)
// Sets all PK counters to 0.
{
                        return;
                        /*sysmessage(s,"Zeroing all player kills...");
			for(int a=0;a<charcount;a++)
			{
				P_CHAR pc = MAKE_CHARREF_LR(a);
				pc->kills=0;
				setcharflag(pc);//AntiChrist
			}
			sysmessage(s,"All player kills are now 0.");*/
}


void command_appetite(NXWSOCKET socket )
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[socket] );
	VALIDATEPC( pc );

	switch( (pc->IsGMorCounselor()	? 6 : pc->hunger) )
	{
		case 6: 
		case 5: pc->sysmsg(TRANSLATE("You are still stuffed from your last meal"));
						break;
		case 4: pc->sysmsg(TRANSLATE("You are not very hungry but could eat more"));
						break;
		case 3: pc->sysmsg(TRANSLATE("You are feeling fairly hungry"));
						break;
		case 2: pc->sysmsg(TRANSLATE("You are extremely hungry"));
						break;
		case 1: pc->sysmsg(TRANSLATE("You are very weak from starvation"));
						break;
		case 0:	pc->sysmsg(TRANSLATE("You must eat very soon or you will die!"));
						break;
	}
}

/*!
\brief Adds an item when using 'add # #
*/
void target_addTarget( NXWCLIENT ps, P_TARGET t )
{
    
    P_ITEM pi = item::CreateFromScript( "$item_hardcoded" );
    VALIDATEPI(pi);

	UI16 id = DBYTE2WORD( t->buffer[0], t->buffer[1] );
	
	tile_st tile;
    data::seekTile( id, tile );

    pi->setId( id );
    pi->pileable = tile.flags&TILEFLAG_STACKABLE;
    pi->setDecay( false );
	Location location = t->getLocation();
	location.z+=tileHeight( t->getModel() );
    pi->MoveTo( location );
    pi->Refresh();

}

void command_add(NXWSOCKET  s)
// (h h) Adds a new item, or opens the GM menu if no hex codes are specified.
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC( pc );

	NXWCLIENT ps = pc->getClient();
	if( ps==NULL )	return;

	if (tnum!=3)//AntiChrist
	{
		ps->sysmsg( "Syntax error. Usage: /add <id1> <id2>" );
	} 
	else
	{
		UI32 a;
		UI32 b;
		if ( server_data.always_add_hex ) { 
			a=hexnumber(1);
			a=hexnumber(2);
		}
		else {
			b=strtonum(1);
			b=strtonum(2);
		}
		clientInfo[s]->resetTarget();
		if( a<0x40 )
		{
			P_TARGET trg = clientInfo[s]->newTarget( new cLocationTarget() );
			ps->sysmsg( "Select location for item..." );
			trg->buffer[0]=a;
			trg->buffer[1]=b;
			trg->code_callback = target_addTarget;
			trg->send( ps );
		}
	}
}




// (h h) Adds a new item to your current location.
void command_addx(NXWSOCKET  s)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	if (tnum==3)
	{
		addid1[s] = (unsigned char) strtonum(1);
		addid2[s] = (unsigned char) strtonum(2);
		Commands::AddHere(s, pc->getPosition("z"));
	}
	if (tnum==4)
	{
		addid1[s] = (unsigned char) strtonum(1);
		addid2[s] = (unsigned char) strtonum(2);
		Commands::AddHere(s, strtonum(3));
	}

}


void target_rename( NXWCLIENT ps, P_TARGET t )
{
	P_OBJECT po = objects.findObject( t->getClicked() );
	VALIDATEPO( po );

	po->setCurrentName( t->buffer_str[0] );
}


void command_rename( NXWCLIENT ps )
// (text) Renames any dynamic object in the game.
{
	if (tnum>1)
	{
		P_TARGET targ = clientInfo[ps->toInt()]->newTarget( new cObjectTarget() );
		targ->buffer_str[0] = tbuffer[Commands::cmd_offset+7];
		targ->code_callback = target_rename;
		targ->send( ps );
		ps->sysmsg( "Select object to rename..." );

	}
}


void command_cfg(NXWSOCKET  s)
// (text) Renames any dynamic item in the game.
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	if (tnum>1)
	{
		int n=cfg_command(&tbuffer[Commands::cmd_offset+4]);
		if (n!=0) sysmessage(s, 0x799, "Error number : %d", n);
	}
}


void command_save(NXWSOCKET  s)
// Saves the current world data into NXWITEMS.WSC and NXWCHARS.WSC.
{
	cwmWorldState->saveNewWorld();
}

void target_setpriv( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc=pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC(pc);
	P_CHAR curr=ps->currChar();
	VALIDATEPC(curr);

	if (SrvParms->gm_log)   //Logging
		WriteGMLog(curr, "%s as given %s Priv [%x][%x]\n", curr->getCurrentNameC(), pc->getCurrentNameC(), t->buffer[0], t->buffer[1] );

    pc->SetPriv( t->buffer[0] );
    pc->priv2=t->buffer[1];
}

void target_setprivItem( NXWCLIENT ps, P_TARGET t )
{
	P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
	VALIDATEPI(pi);

	switch( t->buffer[0] )
	{
        case 0	:
			//pi->priv=pi->priv&0xFE; // lb ...
			pi->setDecay( false );
			pi->setNewbie();
			pi->setDispellable();
			break;
        case 1	:
			pi->setDecay();
			break;
        case 3	:
			pi->priv = t->buffer[1];
			break;
	}
}


void command_setpriv( NXWCLIENT ps )
// (Chars/NPCs: h h, Objects: h) Sets priviliges on a Character or object.
{
	if (tnum==3)
	{
		P_TARGET targ = clientInfo[ ps->toInt() ]->newTarget( new cCharTarget() );
		targ->code_callback = target_setpriv;
		targ->buffer[0]=strtonum(1);
		targ->buffer[1]=strtonum(2);
		targ->send( ps );
		ps->sysmsg( "Select char to edit priv." );
	}
	else if (tnum==2)
	{
		P_TARGET targ = clientInfo[ ps->toInt() ]->newTarget( new cItemTarget() );
		targ->code_callback = target_setprivItem;
		targ->buffer[0]=3;
		targ->buffer[1]=strtonum(1);
		targ->send( ps );
		ps->sysmsg( "Select object to edit priv." );
	}
}

void command_nodecay( NXWCLIENT ps )
// Prevents an object from ever decaying.
{
	P_TARGET targ = clientInfo[ps->toInt()]->newTarget( new cItemTarget() );
	targ->code_callback=target_setprivItem;
	targ->buffer[0]=0;
	targ->send( ps );
	ps->sysmsg( "Select object to make permenant." );
}


void command_send(NXWSOCKET  s)
// Debugging command
{
	int i;
	for (i=1;i<tnum;i++) s_szCmdTableTemp[i-1]=strtonum(i);
	//ConOut("Sending to client %i.\n",s);
	Xsend(s, s_szCmdTableTemp, tnum-1);
//AoS/	Network->FlushBuffer(s);
}

void command_showtime(NXWSOCKET  s)
// Displays the current UO time.
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int hour = Calendar::g_nHour % 12;
	if (hour==0) hour = 12;
	int ampm = (Calendar::g_nHour>=12) ? 1 : 0;
	int minute = Calendar::g_nMinute;

	if (ampm || (!ampm && hour==12))
		sprintf(s_szCmdTableTemp, "%s %2.2d %s %2.2d %s", "NoX-Wizard: Time: ", hour, ":", minute, "PM");
	else
		sprintf(s_szCmdTableTemp, "%s %2.2d %s %2.2d %s", "NoX-Wizard: Time: ", hour, ":",minute, "AM");

	pc->sysmsg(s_szCmdTableTemp);
}

void command_settime(NXWSOCKET  s)
// (d d) Sets the current UO time in hours and minutes.
{
	int newhours, newminutes;
			if (tnum==3)
			{
				newhours = strtonum(1);
				newminutes = strtonum(2);
				if ((newhours < 25) && (newhours > 0) && (newminutes > -1) && (newminutes <60))
				{
					Calendar::g_nHour = newhours;
					Calendar::g_nMinute = newminutes;
				}
			}
			return;

}

void command_shutdown(NXWSOCKET  s)
// (d) Shuts down the server. Argument is how many minutes until shutdown.
{
			if (tnum==2)
			{
				endtime=uiCurrentTime+(MY_CLOCKS_PER_SEC*strtonum(1));
				if (strtonum(1)==0)
				{
					endtime=0;
					sysbroadcast(TRANSLATE("Shutdown has been interrupted."));
				}
				else endmessage(0);
			}
			return;
}

// List of all online player
void command_playerlist(NXWSOCKET  s)
{ 
	static AmxFunction* cmd = NULL;
	if( cmd==NULL )
		cmd= new AmxFunction( "command_playerlist" );
	
	cmd->Call( s );
}

void command_skills( NXWSOCKET s )
{
	static AmxFunction* cmd = NULL;
	if( cmd==NULL )
		cmd= new AmxFunction( "command_skills" );
	
	cmd->Call( s );
}

void command_blt2(NXWSOCKET  s)
// Debugging command.
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	pc->sysmsg("This command is intended for DEBUG and now it's disabled.");
}

void command_sfx(NXWSOCKET  s)
// (h h) Plays the specified sound effect.
{
	if (tnum==3)
	{
		soundeffect(s, (strtonum(1)<<8)|(strtonum(2)%256) );
	}
}

void command_light(NXWSOCKET  s)
// (h) Sets the light level. 0=brightest, 15=darkest, -1=enable day/night cycles.
{
	if (tnum==2)
	{
		worldfixedlevel=strtonum(1);
		if (worldfixedlevel!=255) setabovelight(worldfixedlevel);
		else setabovelight(worldcurlevel);
	}
}

void command_web(NXWSOCKET  s)
// Debugging command.
{
	if (tnum>1)
	{
		strcpy(xtext[s], &tbuffer[Commands::cmd_offset+4]);
		weblaunch(s, xtext[s]);
	}
	return;
}


void command_disconnect(NXWSOCKET  s)
// (d) Disconnects the user logged in under the specified slot.
{
	if (tnum==2) Network->Disconnect(strtonum(1));
	return;
}

void command_tell(NXWSOCKET  s)
// (d text) Sends an anonymous message to the user logged in under the specified slot.
{
	if (tnum>2)
	{
		int m=strtonum(1);
		if (m<0) sysbroadcast(&tbuffer[Commands::cmd_offset+6]); else
		tellmessage(s, strtonum(1), &tbuffer[Commands::cmd_offset+6]);
	}
}

void command_dry(NXWSOCKET  s)
// Set weather to dry (no rain or snow).
{
	wtype=0;
	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if( ps!=NULL )
			weather( ps->toInt(), 0 );
	}
}


void command_rain(NXWSOCKET  s)
// Sets the weather condition to rain.
{
	
	NxwSocketWrapper sw;

	if (wtype==2)
	{
		wtype=0;
		sw.fillOnline();
		for( sw.rewind(); !sw.isEmpty(); sw++ ) {
			NXWCLIENT ps=sw.getClient();
			if( ps!=NULL )
				weather( ps->toInt(), 0 );
		}
	}

	wtype=1;

	sw.clear();
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if( ps!=NULL )
			weather( ps->toInt(), 0 );
	}
}

void command_snow(NXWSOCKET  s)
// Sets the weather condition to snow.
{

	NxwSocketWrapper sw;
	if (wtype==1)
	{
		wtype=0;
		sw.fillOnline();
		for( sw.rewind(); !sw.isEmpty(); sw++ ) {
			NXWCLIENT ps=sw.getClient();
			if( ps!=NULL )
				weather( ps->toInt(), 0 );
		}
	}
	
	wtype=2;
	sw.clear();
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if( ps!=NULL )
			weather( ps->toInt(), 0 );
	}
}

void command_gmmenu(NXWSOCKET  s)
// (d) Opens the specified GM Menu.
{
	if (tnum==2) gmmenu(s, strtonum(1));
}

void command_itemmenu(NXWSOCKET  s)
// (d) Opens the specified Item Menu from ITEMS.SCP.
{
	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	if (tnum==2) 
		showAddMenu(pc, strtonum(1)); //itemmenu(s, strtonum(1));

}

// (d) Adds the specified item from ITEMS.XSS
void command_additem(NXWSOCKET  s)
{
	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	if (tnum==2)
	{
		if ( tbuffer[Commands::cmd_offset+8] == '$')
		{
			if( !evaluateOneDefine(&tbuffer[Commands::cmd_offset+8]) ) 
			{
				pc->sysmsg("Item symbol %s undefined !", &tbuffer[Commands::cmd_offset+8]);
				return;
			}
			addmitem[s] = str2num(&tbuffer[Commands::cmd_offset+8],10);
		}
		else
		{
			addmitem[s] = strtonum(1);
		}
		sprintf(s_szCmdTableTemp, "Select location for item. [Number: %i]", addmitem[s]);
		target(s, 0, 1, 0, 26, s_szCmdTableTemp);
	}
	return;
}

void target_dupe( NXWCLIENT ps, P_TARGET t )
{
    int n = t->buffer[0];
	if( n>=1)
    {
        P_ITEM pi=pointers::findItemBySerial( t->getClicked() );
        if (ISVALIDPI(pi))
        {
            for (int j=0; j<n; j++ )
            {
                Commands::DupeItem( ps->toInt(), pi->getSerial32(), pi->amount );
                ps->sysmsg( "DupeItem done.");
            }
        }
    }
}

void command_dupe( NXWCLIENT ps )
// (d / nothing) Duplicates an item. If a parameter is specified, it's how many copies to make.
{
	P_TARGET targ = clientInfo[ps->toInt()]->newTarget( new cItemTarget() );
	targ->buffer[0] = (tnum==2)? strtonum(1) : 1;
	targ->code_callback=target_dupe;
	targ->send( ps );
	ps->sysmsg( "Select an item to dupe." );
}


void command_command(NXWSOCKET  s)
// Executes a trigger scripting command.
{
			int i;
			int loopexit=0;
			char c=0;
			if (tnum>1)
			{
				i=0;
				script1[0]=0;
				script2[0]=0;
				while(tbuffer[Commands::cmd_offset+8+i]!=' ' && tbuffer[Commands::cmd_offset+8+i]!=0 && (++loopexit < MAXLOOPS) ) i++;
				strncpy(script1,&tbuffer[Commands::cmd_offset+8],i);
				script1[i]=0;
				if ((script1[0]!='}')&&(c!=0)) strcpy(script2, &tbuffer[Commands::cmd_offset+8+i+1]);
				scriptcommand(s, script1, script2);
			}
			return;
}

void command_gcollect(NXWSOCKET  s)
// Runs garbage collection routines.
{
	gcollect();
	sysmessage(s,"command succesfull");
	return;
}

void command_allmoveon(NXWSOCKET  s)
// Enables GM ability to pick up all objects.
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	pc->priv2 |= CHRPRIV2_ALLMOVE;
	pc->teleport();
	pc->sysmsg("ALLMOVE enabled.");
}

void command_allmoveoff(NXWSOCKET  s)
// Disables GM ability to pick up all objects.
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	pc->priv2 &= ~CHRPRIV2_ALLMOVE;
	pc->teleport();
	pc->sysmsg("ALLMOVE disabled.");
}

void command_showhs(NXWSOCKET  s)
// Makes houses appear as deeds. (The walls disappear and there's a deed on the ground in their place.)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	pc->priv2 |= CHRPRIV2_VIEWHOUSEASICON;
	pc->teleport();
	pc->sysmsg("House icons visible. (Houses invisible)");
}

void command_hidehs(NXWSOCKET  s)
// Makes houses appear as houses (opposite of /SHOWHS).
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	pc->priv2 &= ~CHRPRIV2_VIEWHOUSEASICON;
	pc->teleport();
	pc->sysmsg("House icons hidden. (Houses visible)");
}


void command_set(NXWSOCKET  s)
// (text, d) Set STR/DEX/INT/Skills on yourself arguments are skill & amount.
{
	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int i;
	int loopexit=0;
	if (tnum==3)
	{
		i=0;
		script1[0]=0;
		while (tbuffer[Commands::cmd_offset+4+i]!=' ' && tbuffer[Commands::cmd_offset+4+i]!=0 && (++loopexit < MAXLOOPS) ) i++;
		strncpy(script1,&tbuffer[Commands::cmd_offset+4],i);
		script1[i]=0;
		strupr(script1);
		addx[s]=-1;

		for (i=0;i<SKILLS;i++)
		{
			if (!(strcmp(skillname[i], script1))) { /*ConOut("%s\n",skillname[i]);*/
				if (i == I_ACCOUNT) {
					if (pc->account!=0) {
						pc->sysmsg("Only Admin can do this!!!");
						return;
					}
				}
				addx[s]=i;
			}
		}


		if (addx[s]!=-1)
		{
			addy[s]=strtonum(2);
			target(s, 0, 1, 0, 36, "Select character to modify.");
		}
	}
}


void command_temp(NXWSOCKET  s)
// Debugging command.
{
	sysmessage(s, TRANSLATE("This command is simply no more supported. Sorry :["));
}


void target_addNpc( NXWCLIENT ps, P_TARGET t )
{
	if( t->buffer[0]==0 ) {
	
		P_CHAR pc=archive::character::New();

		pc->setCurrentName("Dummy");
		pc->SetBodyType( DBYTE2WORD( t->buffer[0], t->buffer[1] ) );
		pc->SetOldBodyType( DBYTE2WORD( t->buffer[0], t->buffer[1] ) );
		pc->setSkinColor(0);
		pc->setOldSkinColor(0);
		pc->SetPriv(0x10);

		Location loc = t->getLocation();
		loc.z+=tileHeight( t->getModel() );
		pc->MoveTo( loc.x, loc.y, loc.z );
		pc->npc=1;
		pc->teleport();
	}
	else {
		//PDDAFARE
		//P_CHAR pc = npcs::
	}
}

void command_addnpc( NXWCLIENT ps )
{
	P_CHAR pc = ps->currChar();
	VALIDATEPC(pc);

	NXWSOCKET s = ps->toInt();

	P_TARGET targ = clientInfo[ps->toInt()]->newTarget( new cLocationTarget() );
	if (tnum==3)
	{
		targ->buffer[0]=0;
		targ->buffer[1]=strtonum(1);
		targ->buffer[2]=strtonum(2);
	}
	else if (tnum==2)
	{
		if ( tbuffer[Commands::cmd_offset+7] == '$')
		{
			if( !evaluateOneDefine(&tbuffer[Commands::cmd_offset+7]) ) 
			{
				pc->sysmsg("Char symbol %s undefined !", &tbuffer[Commands::cmd_offset+8]);
				clientInfo[s]->resetTarget();
				return;
			}
			targ->buffer[1] = str2num(&tbuffer[Commands::cmd_offset+7],10);
		}
		else
		{
			targ->buffer[1]=strtonum(1);
		}
		targ->buffer[0]=1;
	}
	targ->code_callback=target_addNpc;
	targ->send( ps );
	ps->sysmsg( "Select location for the NPC.");
}


void command_npcrect(NXWSOCKET  s)
// (d d d d) Set bounding box for a NPC with a NPCWANDER of 3.
{
	if (tnum==5)
	{
		addx[s]=strtonum(1); // bugfix, LB, old npcshape worked only if its only excuted by ONE player at the same time
		addy[s]=strtonum(2);
		addx2[s]=strtonum(3);
		addy2[s]=strtonum(4);
		target(s, 0, 1, 0, 67, "Select the NPC to set the bounding rectangle for."); // lb bugfix, was 58 ...
	}
}

void command_npccircle(NXWSOCKET  s)
// (d d d) Set bounding circle for a NPC with a NPCWANDER of 2.
{
	if (tnum==4)
	{
		addx[s]=strtonum(1);
		addy[s]=strtonum(2);
		addx2[s]=strtonum(3);
		target(s, 0, 1, 0, 59, "Select the NPC to set the bounding circle for.");
	}
}

void command_npcwander(NXWSOCKET  s)
// (d) Sets the type of wandering a NPC does.
// <UL><LI>0 = NPC Does not move.</LI>
// <LI>1 = NPC Follows specified target. (See
// <A HREF="npctarget.html">/NPCTARGET</A>)</LI>
// <LI>2 = NPC Wanders Freely.</LI>
// <LI>3 = NPC stays in box specified by <A HREF="npcrect.html">NPCRECT</A>.</LI>
// <LI>4 = NPC stays in circle specified by <A HREF="npccircle.html">NPCCIRCLE</A>.</LI></UL>
{
	if (tnum==2)
	{
		npcshape[0]=strtonum(1);
		target(s, 0, 1, 0, 60, "Select the NPC to set the wander method for.");
	}
}

//
// Set the wander area for a small controlled walking npc (npcwander = 6 )
//
void command_npcrectcoded(NXWSOCKET  s)
{
	clickx[s]=-1;
	clicky[s]=-1;
	target(s,0,1,0,73,"Select first corner of bounding box.");
}


void command_tweak( NXWSOCKET s )
{
	static AmxFunction* cmd = NULL;
	if( cmd==NULL ) 
		cmd = new AmxFunction( "command_tweak" );

	cmd->Call( s );
}


void command_secondsperuominute(NXWSOCKET  s)
// (d) Sets the number of real-world seconds that pass for each UO minute.
{
	if (tnum==2)
	{
		secondsperuominute=strtonum(1);
		sysmessage(s, "Seconds per UO minute set.");
	}
}

void command_brightlight(NXWSOCKET  s)
// (h) Sets default daylight level.
{
	if (tnum==2)
	{
		worldbrightlevel=strtonum(1);
		sysmessage(s, "World bright light level set.");
	}
}

void command_darklight(NXWSOCKET  s)
// (h) Sets default nighttime light level.
{
	if (tnum==2)
	{
		worlddarklevel=strtonum(1);
		sysmessage(s, "World dark light level set.");
	}
}

void command_dungeonlight(NXWSOCKET  s)
// (h) Sets default dungeon light level.
{
	if (tnum==2)
	{
		dungeonlightlevel=qmin(strtonum(1), 27);
		sysmessage(s, "Dungeon light level set.");
	}
}

void command_gmopen(NXWSOCKET  s)
// (h / nothing) Opens specified layer on player/NPC, or player's pack if no layer specified.
// <P>Useful hex codes for this command are:</P>
// <TABLE BORDER=1>
// <TR><TD><B>15</B></TD><TD>Backpack</TD></TR>
// <TR><TD><B>1A</B></TD><TD>NPC Buy Restock container</TD></TR>
// <TR><TD><B>1B</B></TD><TD>NPC Buy no restock container</TD></TR>
// <TR><TD><B>1C</B></TD><TD>NPC Sell container</TD></TR>
// <TR><TD><B>1D</B></TD><TD>Bank Box</TD></TR>
// </TABLE>
{
	if (tnum==2) addmitem[s]=strtonum(1);
	else addmitem[s]=0x15;
	target(s, 0, 1, 0, 115, "Select the character to open the container on.");
}

void command_restock(NXWSOCKET  s)
// Forces a manual vendor restock.
{
	sysmessage(s, "Manual shop restock has occurred.");
	Restocks->doRestock();
}

void command_restockall(NXWSOCKET  s)
// Forces a manual vendor restock to maximum values.
{
	sysmessage(s, "Restocking all shops to their maximums");
	Restocks->doRestockAll();
}

void command_setshoprestockrate(NXWSOCKET  s)
// (d) Sets the universe's shop restock rate.
{
	if (tnum==2)
	{
		sysmessage(s, "NPC shop restock rate IS DISABLED.");
	}
	else sysmessage(s, "Invalid number of parameters.");
		return;

}

void command_midi(NXWSOCKET  s)
// (d d) Plays the specified MIDI file.
{
			if (tnum==3) playmidi(s, strtonum(1), strtonum(2));
			return;

}

// Forces a respawn.
void command_respawn(NXWSOCKET  s)
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	
	sysbroadcast(TRANSLATE("World is now respawning, expect some lag!"));
	LogMessage(s_szCmdTableTemp,"Respawn command called by %s.\n", pc->getCurrentNameC());
	//Respawn->Start();
	return;

}

void command_regspawnmax(NXWSOCKET  s)
// (d) Spawns in all regions up to the specified maximum number of NPCs/Items.
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	
	if (tnum==2)
	{
		sprintf(s_szCmdTableTemp,"MAX Region Respawn command called by %s.\n", pc->getCurrentNameC());//AntiChrist
		LogMessage(s_szCmdTableTemp);
		Commands::RegSpawnMax(s, strtonum(1));
		return;
	}

}

void command_regspawn(NXWSOCKET  s)
// (d d) Preforms a region spawn. First argument is region, second argument is max # of items/NPCs to spawn in that region.
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	
	if (tnum==3)
	{
		sprintf(s_szCmdTableTemp,"Specific Region Respawn command called by %s.\n", pc->getCurrentNameC());//AntiChrist
		LogMessage(s_szCmdTableTemp);
		Commands::RegSpawnNum(s, strtonum(1), strtonum(2));
		return;
	}
}

void command_regedit(NXWSOCKET  s)
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC( pc );
	sprintf(s_szCmdTableTemp,"Region edit command called by %s.\n", pc->getCurrentNameC());
	LogMessage(s_szCmdTableTemp);
	//newAmxEvent("gui_rgnList")->Call( pc->getSerial32(), 0 );
}

void command_reloadserver(NXWSOCKET  s)
// Reloads the SERVER.cfg file.
{
	loadserverscript();
	sysmessage(s,"server.cfg reloaded.");
}

void command_loaddefaults(NXWSOCKET  s)
// Loads the server defaults.
{
	loadserverdefaults();
}

void command_cq(NXWSOCKET  s)
// Display the counselor queue.
{
	Commands::ShowGMQue(s, 0); // Show the Counselor queue, not GM queue
}

void command_cnext(NXWSOCKET  s)
// Attend to the next call in the counselor queue.
{
	Commands::NextCall(s, 0); // Show the Counselor queue, not GM queue

}

void command_cclear(NXWSOCKET  s)
// Remove the current call from the counselor queue.
{
	donewithcall(s, 0); // Show the Counselor queue, not GM queue
}

void command_minecheck(NXWSOCKET  s)
// (d) Set the server mine check interval in minutes.
{
	if (tnum==2)
		SrvParms->minecheck=strtonum(1);
}

void command_invul(NXWSOCKET  s)
// Makes the targeted character immortal.
{
			addx[s]=1;
			target(s,0,1,0,179,"Select creature to make invulnerable.");

}

void command_noinvul(NXWSOCKET  s)
// Makes the targeted character mortal.
{
			addx[s]=0;
			target(s,0,1,0,179,"Select creature to make mortal again.");

}

void command_guardson(NXWSOCKET  s)
// Activates town guards.
{
	SrvParms->guardsactive=1;
	sysbroadcast(TRANSLATE("Guards have been reactivated."));

}

void command_guardsoff(NXWSOCKET  s)
// Deactivates town guards.
{
	SrvParms->guardsactive=0;
	sysbroadcast(TRANSLATE("Warning: Guards have been deactivated globally."));

}

void command_decay( NXWCLIENT ps )
// Enables decay on an object.
{
	P_TARGET targ=clientInfo[ps->toInt()]->newTarget( new cItemTarget() );
	targ->code_callback=target_setprivItem;
	targ->buffer[0]=1;
	targ->send( ps );
	ps->sysmsg( "Select object to decay.");

}

void command_pdump(NXWSOCKET  s)
// Display some performance information.
{
	sysmessage(s, "Performace Dump:");

	sprintf(s_szCmdTableTemp, "Network code: %fmsec [%i]" , (float)((float)networkTime/(float)networkTimeCount) , networkTimeCount);
	sysmessage(s, s_szCmdTableTemp);

	sprintf(s_szCmdTableTemp, "Timer code: %fmsec [%i]" , (float)((float)timerTime/(float)timerTimeCount) , timerTimeCount);
	sysmessage(s, s_szCmdTableTemp);

	sprintf(s_szCmdTableTemp, "Auto code: %fmsec [%i]" , (float)((float)autoTime/(float)autoTimeCount) , autoTimeCount);
	sysmessage(s, s_szCmdTableTemp);

	sprintf(s_szCmdTableTemp, "Loop Time: %fmsec [%i]" , (float)((float)loopTime/(float)loopTimeCount) , loopTimeCount);
	sysmessage(s, s_szCmdTableTemp);

	sprintf(s_szCmdTableTemp, "Simulation Cycles/Sec: %f" , (1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount))));
	sysmessage(s, s_szCmdTableTemp);
}

void command_gy(NXWSOCKET  s)
// (text) GM Yell - Announce a message to all online GMs.
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	if(now==1) {
		pc->sysmsg("There are no other users connected.");
		return;
	}

	sprintf(xtext[s], "(GM ONLY): %s", &tbuffer[Commands::cmd_offset+3]);

	UI32 id;
	UI16 model, color, font;

	id = pc->getSerial32();
	model = pc->GetBodyType();
	color = ShortFromCharPtr(buffer[s] +4);
	font = (buffer[s][6]<<8)|(pc->fonttype%256);

	UI08 name[30]={ 0x00, };
	strcpy((char *)name, pc->getCurrentNameC());


	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps_i=sw.getClient();
		if( ps_i==NULL )
			continue;

		P_CHAR pc_i = ps_i->currChar();
		if (ISVALIDPC(pc_i) && pc_i->IsGM())
		{
			NXWSOCKET allz = ps_i->toInt();
			SendSpeechMessagePkt(allz, id, model, 1, color, font, name, (UI08 *)xtext[s]);
		}
	}
}

void command_yell(NXWSOCKET  s)
// (text) GM Yell - Announce a message to all online players.
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	if(now==1) {
		pc->sysmsg("There are no other users connected.");
		return;
	}

	sprintf(xtext[s], "(GM MSG): %s", &tbuffer[Commands::cmd_offset+3]);

	UI32 id;
	UI16 model, color, font;

	id = pc->getSerial32();
	model = pc->GetBodyType();
	color = ShortFromCharPtr(buffer[s] +4);
	font = (buffer[s][6]<<8)|(pc->fonttype%256);

	UI08 name[30]={ 0x00, };
	strcpy((char *)name, pc->getCurrentNameC());

	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps_i=sw.getClient();
		if( ps_i==NULL )
			continue;

		P_CHAR pc_i = ps_i->currChar();
		if (ISVALIDPC(pc_i) )
		{
			NXWSOCKET allz = ps_i->toInt();
			SendSpeechMessagePkt(allz, id, model, 1, color, font, name, (UI08 *)xtext[s]);
		}
	}
}

void target_squelch( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR curr = ps->currChar();
	VALIDATEPC(curr);

	P_CHAR pc =pointers::findCharBySerial( t->getClicked() );
	if(ISVALIDPC(pc))
    {
        if(pc->IsGM())
        {
            curr->sysmsg(TRANSLATE("You cannot squelch GMs."));
            return;
        }

        if (pc->squelched)
        {
            pc->squelched=0;
            curr->sysmsg(TRANSLATE("Un-squelching..."));
            pc->sysmsg(TRANSLATE("You have been unsquelched!"));
            pc->mutetime=0;
        }
        else
        {
            pc->mutetime=0;
            curr->sysmsg( TRANSLATE("Squelching...") );
            pc->sysmsg( TRANSLATE("You have been squelched!") );

            if( t->buffer[0]!=INVALID )
            {
                pc->mutetime = uiCurrentTime+ t->buffer[0]*MY_CLOCKS_PER_SEC;
                pc->squelched=2;
            }
			else
				pc->squelched=1;
        }
    }
}


void command_squelch( NXWCLIENT ps )
// (d / nothing) Squelchs specified player. (Makes them unnable to speak.)
{

	P_TARGET targ = clientInfo[ ps->toInt() ]->newTarget( new cCharTarget() );

    if (tnum==2)
	{
		int value = strtonum(1);
		if( value>0 )
		{
			targ->buffer[0]=strtonum(1);
		}
		else
			targ->buffer[0]=INVALID;
	}

	targ->code_callback = target_squelch;
	targ->send( ps );
	ps->sysmsg( "Select character to squelch." );

}

void command_spawnkill(NXWSOCKET  s)
// (d) Kills spawns from the specified spawn region in SPAWN.SCP.
{
	if (tnum==2)
	{
		Commands::KillSpawn(s, strtonum(1));
	}
}

void command_stats(NXWSOCKET  s)
{
	AmxFunction* cmd=NULL;
	if( cmd==NULL )
		cmd = new AmxFunction("command_stats");

	cmd->Call( s );
}

void command_options(NXWSOCKET  s)
{
	AmxFunction* cmd=NULL;
	if( cmd==NULL )
		cmd = new AmxFunction("command_options");

	cmd->Call( s );
}


void command_gotocur(NXWSOCKET  s)
// Goes to the current call in the GM/Counsellor Queue
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	if(pc->callnum==0)
	{
		pc->sysmsg("You are not currently on a call.");
	}
	else
	{
		P_CHAR pc_i = pointers::findCharBySerial( gmpages[pc->callnum].serial.serial32 );
		if(ISVALIDPC(pc_i))
		{
			Location charpos= pc_i->getPosition();

			pc->MoveTo( charpos );
			pc->sysmsg("Transporting to your current call.");
			pc->teleport();
			return;
		}

		pc_i = pointers::findCharBySerial( counspages[pc->callnum].serial.serial32 );
		if(ISVALIDPC(pc_i))
		{
			Location charpos= pc_i->getPosition();

			pc->MoveTo( charpos );
			sysmessage(s,"Transporting to your current call.");
			pc->teleport();
		}
	}
}

void command_gmtransfer(NXWSOCKET  s)
// Escilate a Counsellor Page into the GM Queue
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int i;
	int x2=0;

	if(pc->callnum!=0)
	{
		if(!pc->IsGM()) //Char is a counselor
		{
			for(i=1;i<MAXPAGES;i++)
			{
				if(gmpages[i].handled==1)
				{
					gmpages[i].handled=0;
					strcpy(gmpages[i].name,counspages[pc->callnum].name);
					strcpy(gmpages[i].reason,counspages[pc->callnum].reason);

					gmpages[i].serial= counspages[pc->callnum].serial;
					time_t current_time = time(0);
					struct tm *local = localtime(&current_time);
					sprintf(gmpages[i].timeofcall, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
					x2++;
					break;
				}
			}
			if (x2==0)
			{
				pc->sysmsg("The GM Queue is currently full. Contact the shard operator");
				pc->sysmsg("and ask them to increase the size of the queue.");
			}
			else
			{
				pc->sysmsg("Call successfully transferred to the GM queue.");
				donewithcall(s,1);
			}
		}
		else
		{
			pc->sysmsg("Only Counselors may use this command.");
		}
	}
	else
	{
		pc->sysmsg("You are not currently on a call");
	}
}

void command_who(NXWSOCKET  s)
// Displays a list of users currently online.
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	if(now==1) {
		pc->sysmsg("There are no other users connected.");
		return;
	}

	int j=0;
	pc->sysmsg("Current Users in the World:");
	
	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps_i=sw.getClient();
		if(ps_i!=NULL)
		{
			P_CHAR pc_i=ps_i->currChar();
			if(ISVALIDPC(pc_i)) {
				j++;
				sprintf(s_szCmdTableTemp, "%i) %s [%x]", (j-1), pc_i->getCurrentNameC(), pc_i->getSerial32());
				pc->sysmsg(s_szCmdTableTemp);
			}
		}
	}
	sprintf(s_szCmdTableTemp,"Total Users Online: %d\n", j);
	pc->sysmsg(s_szCmdTableTemp);
	pc->sysmsg("End of userlist");
}

void command_gms(NXWSOCKET  s)
{
	int j=0;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	pc->sysmsg("Current GMs and Counselors in the world:");

	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps_i=sw.getClient();
		if(ps_i!=NULL)
		{
			P_CHAR pc_i=ps_i->currChar();
			if(ISVALIDPC(pc_i) && pc_i->IsGMorCounselor() ) {
				j++;
				pc->sysmsg("%s", pc_i->getCurrentNameC());
			}
		}
	}
	sprintf(s_szCmdTableTemp, "Total Staff Online: %d\n", j);
	pc->sysmsg(s_szCmdTableTemp);
	pc->sysmsg("End of stafflist");
}

void command_regspawnall(NXWSOCKET  s)
{
	sysbroadcast(TRANSLATE("ALL Regions Spawning to MAX, this will cause some lag."));

	Spawns->doSpawnAll();
	
	sysmessage(s, "[DONE] All NPCs/items spawned in regions");
}

void command_sysm(NXWSOCKET  s)
{
	if (now == 1)
	{
		sysmessage(s, "There are no other users connected.");
		return;
	}
	strcpy(xtext[s], &tbuffer[Commands::cmd_offset + 5]);
	sysbroadcast(xtext[s]);
}

void target_jail( NXWCLIENT ps, P_TARGET t )
{
	P_CHAR pc=pointers::findCharBySerial( t->getClicked() );
	VALIDATEPC( pc );
	
	prison::jail( ps->currChar(), pc, t->buffer[0] );
}

void command_jail( NXWCLIENT ps )
// (d) Jails the target with given secs.
{
	P_TARGET targ=clientInfo[ ps->toInt() ]->newTarget( new cCharTarget() );
	targ->code_callback=target_jail;

	if (tnum==2 )
	{
		targ->buffer[0]=strtonum(1);
		ps->sysmsg( "Select Character to jail. [Jailtime: %i secs]", targ->buffer[0] );
	}
	else
	{
		targ->buffer[0]=24*60*60;
		ps->sysmsg( "Select Character to jail. [Jailtime: 1 day]" );
	}

	targ->send( ps );
}

// handler for the movement effect
// Aldur
//
void command_setGmMoveEff(NXWSOCKET  s)
// (h) set your movement effect.
{
	P_CHAR pc_cs=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_cs);
	 if (tnum==2)
		pc_cs->gmMoveEff = strtonum(1);
	return;
}


void command_password(NXWSOCKET  s)
// (text) Changes the account password
{
		
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	if (tnum>1)
	{
		char pwd[200];
		int ret;
		sprintf(pwd, "%s", &tbuffer[Commands::cmd_offset+9]);
		if ((!isalpha(pwd[0]))&&(!isdigit(pwd[0]))) {
			sysmessage(s, "Passwords must start with a letter or a number\n");
			return;
		}

		ret = Accounts->ChangePassword(pc->account, pwd);

		if (ret==0) 
			sprintf(pwd, "Password changed to %s", &tbuffer[Commands::cmd_offset+9]);
		else 
			sprintf(pwd, "Some Error occured while changing password!");

		sysmessage(s, pwd);
	}
	else 
		sysmessage(s, "You must digit 'PASSWORD <newpassword>");
		return;
}

void target_tele( NXWCLIENT ps, P_TARGET t )
{
	
	P_CHAR pc= ps->currChar();
	VALIDATEPC(pc);

	Location location = t->getLocation();
	Location charpos= pc->getPosition();

	if( line_of_sight( ps->toInt(), charpos.x, charpos.y, charpos.z, location.x, location.y, location.z, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING ) ||
		pc->IsGM() )
	{
		pc->doGmEffect();
		location.z+=tileHeight( t->getModel() );
		pc->MoveTo( location );
		pc->teleport();
		pc->doGmEffect();
	}

}

void target_remove( NXWCLIENT ps, P_TARGET t )
{
	
	SERIAL serial = t->getClicked();
	
	if( isCharSerial( serial ) )	{
		P_CHAR pc=pointers::findCharBySerial( serial );
		VALIDATEPC(pc);

		if (pc->amxevents[EVENT_CHR_ONDISPEL]) {
			pc->amxevents[EVENT_CHR_ONDISPEL]->Call( pc->getSerial32(), INVALID, DISPELTYPE_GMREMOVE );
		}

        if (pc->account>INVALID && !pc->npc)
        {
            ps->sysmsg( TRANSLATE("You cant delete players") );
            return;
        }

        ps->sysmsg( TRANSLATE("Removing character.") );
		pc->Delete();
	}
	else {
		P_ITEM pi=pointers::findItemBySerial( serial );
		VALIDATEPI(pi);

		ps->sysmsg( TRANSLATE("Removing item.") );
        if( pi->amxevents[EVENT_IONDECAY] )
			pi->amxevents[EVENT_IONDECAY]->Call( pi->getSerial32(), DELTYPE_GMREMOVE );
        ps->sysmsg( TRANSLATE("Removing item.") );
        pi->Delete();
	}

}


void target_dye( NXWCLIENT ps, P_TARGET t )
{
	
	SERIAL serial = t->getClicked();
	
	UI16 color = t->buffer[0];

	P_CHAR curr = ps->currChar();
	VALIDATEPC(curr);

    if( isItemSerial(serial) ) {
		P_ITEM pi=pointers::findItemBySerial(serial);
		if (ISVALIDPI(pi))
		{
			if( color==INVALID ) //open dye vat
			{
				SndDyevat( ps->toInt(), pi->getSerial32(), pi->id() );
			}	
			else {
				if (! ((color & 0x4000) || (color & 0x8000)) )
				{
					pi->setColor(color);
				}

				if (color == 0x4631)
				{
					pi->setColor(color);
				}

				pi->Refresh();
			}
		}
	}
	else {
		P_CHAR pc=pointers::findCharBySerial(serial);
		if(ISVALIDPC(pc))
		{
			if( color==INVALID ) //open dye vat
			{
				SndDyevat( ps->toInt(), pc->getSerial32(), 0x2106 );
			}
			else {
				
				UI16 body = pc->GetBodyType();

				if(  color < 0x8000  && body >= BODY_MALE && body <= BODY_DEADFEMALE ) color |= 0x8000; // why 0x8000 ?! ^^;

				if ((color & 0x4000) && (body >= BODY_MALE && body<= 0x03E1)) color = 0xF000; // but assigning the only "transparent" value that works, namly semi-trasnparency.

				if (color != 0x8000)
				{
					pc->setSkinColor(color);
					pc->setOldSkinColor(color);
					pc->teleport( TELEFLAG_NONE );

				}
			}
		}
    }

}




void command_dye( NXWCLIENT ps )
// (h h/nothing) Dyes an item a specific color, or brings up a dyeing menu if no color is specified.
{
	
	P_TARGET targ = clientInfo[ ps->toInt() ]->newTarget( new cObjectTarget() );
	

	if (tnum==3)
	{
		if ( server_data.always_add_hex ) { 
			targ->buffer[0]=DBYTE2WORD( hexnumber(1), hexnumber(2) );
		}
		else {
			targ->buffer[0]=DBYTE2WORD( strtonum(1), strtonum(2) );
		}
	}
	else
	{
		targ->buffer[0]=INVALID;
	}

	targ->code_callback = target_dye;
	targ->send( ps );
	ps->sysmsg( "Select item to dye..." );

}

void target_newz( NXWCLIENT ps, P_TARGET t )
{

    SERIAL serial = t->getClicked();

	if( isCharSerial( serial ) ) {
		P_CHAR pc=pointers::findCharBySerial( serial );
		VALIDATEPC(pc);

		Location location = pc->getPosition();
		location.z = location.dispz = t->buffer[0];
		pc->setPosition( location );
		pc->teleport();
	}
	else {
		P_ITEM pi=pointers::findItemBySerial( serial );
		VALIDATEPI(pi);

		Location location = pi->getPosition();
		location.z = t->buffer[0];
		pi->setPosition( location );
		pi->Refresh();
	}

};

void command_newz( NXWCLIENT ps )
{
	if (tnum==2) {
		P_TARGET targ = clientInfo[ ps->toInt() ]->newTarget( new cObjectTarget() );
		targ->buffer[0] = strtonum(1);
		targ->code_callback = target_newz;
		targ->send( ps );
		ps->sysmsg( "Select item to reposition..." );
	}

}


void target_setid( NXWCLIENT ps, P_TARGET t )
{

    SERIAL serial = t->getClicked();
	UI16 value = DBYTE2WORD( t->buffer[0], t->buffer[1] );

	if( isCharSerial( serial ) ) {
		P_CHAR pc=pointers::findCharBySerial( serial );
		VALIDATEPC(pc);

		pc->SetBodyType( value );
		pc->SetOldBodyType( value );
		pc->teleport();
	}
	else {
		P_ITEM pi=pointers::findItemBySerial( serial );
		VALIDATEPI(pi);

		pi->setId( value );
        pi->Refresh();
	}
}


