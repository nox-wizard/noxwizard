  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


/*!
\file cmdtable.h
\brief Declaration all ingame commands
\author Crackerjack
\remark This code is an attempt to clean up the messy "if/then/else" routines
	currently in use for GM commands, as well as adding more functionality
	and more potential for functionality.

	Current features:
	\li Actual table of commands to execute, what perms are required, dialog
		messages for target commands, etc handled by a central system

	\li /SETPRIV3 user-friendliness - /SETPRIV3 < command > works now instead of
		all that cumbersome bitmask math (Eg; /SETPRIV3 SAVE and click on a
		char and they get SAVE command perms, /SETPRIV3 !SAVE and click on a
		char and their SAVE privs are removed)
 */

#ifndef __CMDTABLE_H
#define __CMDTABLE_H

#include "nxwcommn.h"

/*!
\brief Type of command
*/
enum CmdType
{
	CMD_ITEMMENU =1,	//!< Open item menu specified in cmd_extra
	CMD_TARGET,		//!< Call target struct specified in cmd_extra
	CMD_FUNC,		//!< Call function specified in cmd_extra, must be of type GMFUNC
	CMD_TARGETX,		//!< target with addx[] argument
	CMD_TARGETXY,		//!< target with addx & y [] arguments
	CMD_TARGETXYZ,		//!< target with addx & y & z [] arguments
	CMD_TARGETHX,		//!< target with hex addx[] argument
	CMD_TARGETHXY,		//!< target with hex addx & y [] arguments
	CMD_TARGETHXYZ,		//!< target with hex addx & y & z [] arguments
	CMD_TARGETID1,		//!< target with 1 addid #
	CMD_TARGETID2,		//!< target with 2 addid #s
	CMD_TARGETID3,		//!< target with 3 addid #s
	CMD_TARGETID4,		//!< target with 4 addid #s
	CMD_TARGETHID1,		//!< target with 1 hex addid #
	CMD_TARGETHID2,		//!< target with 2 hex addid #s
	CMD_TARGETHID3,		//!< target with 3 hex addid #s
	CMD_TARGETHID4,		//!< target with 4 hex addid #s
	CMD_TARGETTMP,		//!< target with tempint
	CMD_TARGETHTMP,		//!< target with hex tempint
	CMD_MANAGEDCMD =32	//!< command is full self managed
};

// Definitions of casting and declarataion of command step functions
#define P_MANAGEDSTEP		void (*)(NXWCLIENT)
#define MANAGEDSTEP(name)	void (*name)(NXWCLIENT)

/*!
\brief Last targeting and self targeting access test there bits
*/
enum
{
	TARG_NONE,		//!< No special modifiers for this command
	TARG_LAST,		//!< Can accept last target
	TARG_SELF		//!< Can accept self as a target
};

/** Defines a target function: 
 * name: is the name of the function you want to create
 * min: is the minimum number of parameters you must supply
 * errmsg: message sent to the client if not all parameters supplied
 * msg: message for target selection (eg what to target)
 */
#define TARGETFUNC(name,min,errmsg,msg) \
	void name(NXWCLIENT client) { \
		if(client->cmdParams.size()<min) \
			client->sysmsg(errmsg); \
		else \
			client->doTargeting(msg); \
	}

class cCommandStep; class cCommand;

// Pointers to a command and to a command-step
typedef cCommand* P_COMMAND;
typedef cCommandStep* P_COMMANDSTEP;

class cCommandStep {
public:
    cCommandStep(void cmd(NXWCLIENT));
    void execute(NXWCLIENT client);
	void (*command)(NXWCLIENT);
	P_COMMANDSTEP nextStep;
};

class cCommand {
public:
    cCommand(char* name, int privm, int privb, int type, void command(), unsigned char targMask);
    cCommand(char* name, int privm, int privb, cCommandStep* command, unsigned char targMask);
    bool isValid(P_CHAR character);
    bool notValid(P_CHAR character);

    static P_COMMANDSTEP buildSteps(MANAGEDSTEP(command), ...);

	char*	cmd_name;
	unsigned int	cmd_priv_m;	/* PRIV3 byte# - 0-6, 255=no privs needed */
	unsigned int	cmd_priv_b;	/* PRIV3 bit within byte - 0-31 */
	unsigned int	cmd_type;	/* Type of command - see above */
	void 		(*cmd_extra) ();	/* extra data - see above */
	cCommandStep* exec;
	unsigned char targetingMask;
};

typedef std::map< string, P_COMMAND> td_cmdmap;
typedef td_cmdmap::iterator td_cmditer;

class cCommandTable {
public:
    cCommandTable();
    P_COMMAND addPlayerCommand(char* name, int privb, int type, void command());
    P_COMMAND addGmCommand(char* name, int privm, int privb, int type, void command());
    P_COMMAND addGmCommand(P_COMMAND cmd);
    P_COMMAND findCommand(char* name);
    td_cmditer getIteratorBegin();
    td_cmditer getIteratorEnd();
       
private:
    td_cmdmap command_map;
};

extern cCommandTable* commands;

#define CMD_EXEC	void (*) (int)
#define CMD_DEFINE	void (*)()

typedef struct target_s TARGET_S;
struct target_s {	/* arguments to the target() function */
	int	a1, a2, a3, a4;
	char	txt[128];
};

/* All command_ functions take an int value of the player that triggered the command. */
#define CMD_HANDLER(name) extern void name (int)
#define TAR_HANDLER(name) extern TARGET_S name

/* Defined commands that are just being mapped to internal functions */
#define command_time telltime

/* All defined commands */
CMD_HANDLER(command_setGmMoveEff);
CMD_HANDLER(command_bounty);
CMD_HANDLER(command_post);
CMD_HANDLER(command_gpost);
CMD_HANDLER(command_rpost);
CMD_HANDLER(command_lpost);
CMD_HANDLER(command_cleanup);
CMD_HANDLER(command_regspawnall);
CMD_HANDLER(command_wipenpcs);
CMD_HANDLER(command_gms);
CMD_HANDLER(command_who);
CMD_HANDLER(command_gmtransfer);
CMD_HANDLER(command_stats);
CMD_HANDLER(command_options);
CMD_HANDLER(command_gotocur);
CMD_HANDLER(command_showp);
CMD_HANDLER(command_setpriv3);
CMD_HANDLER(command_spreadpriv3);
CMD_HANDLER(command_resend);
CMD_HANDLER(command_teleport);
CMD_HANDLER(command_where);
CMD_HANDLER(command_q);
CMD_HANDLER(command_next);
CMD_HANDLER(command_clear);
CMD_HANDLER(command_reloadcachedscripts);
/* CMD_HANDLER(command_gmtransfer); */
CMD_HANDLER(command_password);
CMD_HANDLER(command_goplace);
CMD_HANDLER(command_gochar);
CMD_HANDLER(command_fix);
CMD_HANDLER(command_xgoplace);
CMD_HANDLER(command_showids);
CMD_HANDLER(command_poly);
CMD_HANDLER(command_skin);
CMD_HANDLER(command_action);
CMD_HANDLER(command_xtele);
CMD_HANDLER(command_go);
CMD_HANDLER(command_zerokills);
CMD_HANDLER(command_tile);
//CMD_HANDLER(command_wipe);
//CMD_HANDLER(command_iwipe);
CMD_HANDLER(command_add);
CMD_HANDLER(command_appetite);
CMD_HANDLER(command_addx);
CMD_HANDLER(command_cfg);
//CMD_HANDLER(command_rename);
//CMD_HANDLER(command_title);
CMD_HANDLER(command_save);
CMD_HANDLER(command_dye);
CMD_HANDLER(command_wtrig);
CMD_HANDLER(command_setpriv);
CMD_HANDLER(command_nodecay);
CMD_HANDLER(command_send);
CMD_HANDLER(command_showtime);
CMD_HANDLER(command_settime);
CMD_HANDLER(command_shutdown);
//CMD_HANDLER(command_wholist);
CMD_HANDLER(command_playerlist);
CMD_HANDLER(command_blt2);
CMD_HANDLER(command_sfx);
CMD_HANDLER(command_light);
CMD_HANDLER(command_web);
CMD_HANDLER(command_disconnect);
CMD_HANDLER(command_tell);
CMD_HANDLER(command_dry);
/* lord binary's debugging code
CMD_HANDLER(command_npcs);
CMD_HANDLER(command_npcsd);
CMD_HANDLER(command_pcs);
CMD_HANDLER(command_charc);
CMD_HANDLER(command_itemc);
 * end of lord binary's debugging code */
CMD_HANDLER(command_setseason);
CMD_HANDLER(command_rain);
CMD_HANDLER(command_snow);
CMD_HANDLER(command_gmmenu);
CMD_HANDLER(command_itemmenu);
CMD_HANDLER(command_additem);
CMD_HANDLER(command_dupe);
CMD_HANDLER(command_command);
CMD_HANDLER(command_gcollect);
CMD_HANDLER(command_allmoveon);
CMD_HANDLER(command_allmoveoff);
CMD_HANDLER(command_showhs);
CMD_HANDLER(command_hidehs);
CMD_HANDLER(command_set);
CMD_HANDLER(command_temp);
CMD_HANDLER(command_addnpc);
CMD_HANDLER(command_readini);
CMD_HANDLER(command_cachestats);
CMD_HANDLER(command_npcrect);
CMD_HANDLER(command_npccircle);
CMD_HANDLER(command_npcwander);
CMD_HANDLER(command_secondsperuominute);
CMD_HANDLER(command_brightlight);
CMD_HANDLER(command_darklight);
CMD_HANDLER(command_dungeonlight);
CMD_HANDLER(command_time);
CMD_HANDLER(command_gmopen);
CMD_HANDLER(command_restock);
CMD_HANDLER(command_restockall);
CMD_HANDLER(command_setshoprestockrate);
/* CMD_HANDLER(command_who);
CMD_HANDLER(command_gms); */
CMD_HANDLER(command_midi);
CMD_HANDLER(command_respawn);
CMD_HANDLER(command_regspawnall);
CMD_HANDLER(command_regspawnmax);
CMD_HANDLER(command_regspawn);
CMD_HANDLER(command_regedit);
CMD_HANDLER(command_reloadserver);
CMD_HANDLER(command_loaddefaults);
CMD_HANDLER(command_cq);
/* CMD_HANDLER(command_wipenpcs); */
CMD_HANDLER(command_cnext);
CMD_HANDLER(command_cclear);
CMD_HANDLER(command_minecheck);
CMD_HANDLER(command_invul);
//CMD_HANDLER(command_rename2);
CMD_HANDLER(command_noinvul);
CMD_HANDLER(command_guardson);
CMD_HANDLER(command_guardsoff);
CMD_HANDLER(command_announceon);
CMD_HANDLER(command_announceoff);
CMD_HANDLER(command_wf);
CMD_HANDLER(command_decay);
CMD_HANDLER(command_killall);
CMD_HANDLER(command_pdump);
//CMD_HANDLER(command_rename);
CMD_HANDLER(command_readspawnregions);
/* CMD_HANDLER(command_cleanup); */
CMD_HANDLER(command_gy);
CMD_HANDLER(command_yell);
CMD_HANDLER(command_tilew);
CMD_HANDLER(command_squelch);
CMD_HANDLER(command_squelch);
CMD_HANDLER(command_spawnkill);
CMD_HANDLER(command_wanim);

/*
CMD_HANDLER(command_setacct);
CMD_HANDLER(command_addacct);
CMD_HANDLER(command_banacct);
CMD_HANDLER(command_unbanacct);*/


CMD_HANDLER(command_letusin);
CMD_HANDLER(command_readaccounts);

CMD_HANDLER(command_serversleep);

CMD_HANDLER(command_setmenupriv);

//taken from 6904t2(5/10/99) - AntiChrist
CMD_HANDLER(command_setmurder);
CMD_HANDLER(command_delid); // Ripper
CMD_HANDLER(command_deltype); // Ripper
CMD_HANDLER(command_jail);
CMD_HANDLER(command_eclipse);
CMD_HANDLER(command_sysm);

// SPARHAWK reload race script
CMD_HANDLER(command_reloadracescript);
// SPARHAWK npcrect for small controlled npc's, stores bound info compressed into FX1 & FX2
CMD_HANDLER(command_npcrectcoded);
CMD_HANDLER(command_tweak);

/* all defined target commands */
TAR_HANDLER(target_use);
TAR_HANDLER(target_jail);
TAR_HANDLER(target_release);
TAR_HANDLER(target_tele);
TAR_HANDLER(target_xgo);
TAR_HANDLER(target_setmorex);
TAR_HANDLER(target_setmorey);
TAR_HANDLER(target_setmorez);
TAR_HANDLER(target_setmorexyz);
TAR_HANDLER(target_sethexmorexyz);
TAR_HANDLER(target_setnpcai);
TAR_HANDLER(target_xbank);
TAR_HANDLER(target_xsbank);//AntiChrist
TAR_HANDLER(target_remove);
TAR_HANDLER(target_newz);
TAR_HANDLER(target_settype);
TAR_HANDLER(target_itrig);
TAR_HANDLER(target_ctrig);
TAR_HANDLER(target_ttrig);
TAR_HANDLER(target_setid);
TAR_HANDLER(target_trainer);
TAR_HANDLER(target_setmore);
TAR_HANDLER(target_makegm);
TAR_HANDLER(target_makecns);
TAR_HANDLER(target_killhair);
TAR_HANDLER(target_killbeard);
TAR_HANDLER(target_killpack);
TAR_HANDLER(target_setfont);
TAR_HANDLER(target_kill);
TAR_HANDLER(target_resurrect);
TAR_HANDLER(target_bolt);
TAR_HANDLER(target_npcaction);
TAR_HANDLER(target_setamount);
TAR_HANDLER(target_setamount2);
TAR_HANDLER(target_kick);
TAR_HANDLER(target_movetobag);
TAR_HANDLER(target_setmovable);
TAR_HANDLER(target_setvisible);
TAR_HANDLER(target_setdir);
TAR_HANDLER(target_setspeech);
TAR_HANDLER(target_setowner);
TAR_HANDLER(target_freeze);
TAR_HANDLER(target_unfreeze);
TAR_HANDLER(target_tiledata);
TAR_HANDLER(target_recall);
TAR_HANDLER(target_mark);
TAR_HANDLER(target_gate);
TAR_HANDLER(target_heal);
TAR_HANDLER(target_npctarget);
//TAR_HANDLER(target_tweak);
TAR_HANDLER(target_sbopen);
TAR_HANDLER(target_mana);
TAR_HANDLER(target_stamina);
TAR_HANDLER(target_makeshop);
TAR_HANDLER(target_buy);
TAR_HANDLER(target_setvalue);
TAR_HANDLER(target_setrestock);
TAR_HANDLER(target_sell);
TAR_HANDLER(target_setspattack);
TAR_HANDLER(target_setspadelay);
TAR_HANDLER(target_setpoison);
TAR_HANDLER(target_setpoisoned);
TAR_HANDLER(target_setadvobj);
TAR_HANDLER(target_setwipe);
TAR_HANDLER(target_fullstats);
TAR_HANDLER(target_hide);
TAR_HANDLER(target_unhide);
TAR_HANDLER(target_house);
TAR_HANDLER(target_split);
TAR_HANDLER(target_splitchance);
TAR_HANDLER(target_possess);
TAR_HANDLER(target_telestuff);
TAR_HANDLER(target_newx);
TAR_HANDLER(target_newy);
TAR_HANDLER(target_incx);
TAR_HANDLER(target_incy);
TAR_HANDLER(target_incz);
TAR_HANDLER(target_ban);
//TAR_HANDLER(target_glow);
//TAR_HANDLER(target_unglow);
TAR_HANDLER(target_showskills);

#endif /* __CMDTABLE_H */
