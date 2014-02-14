  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#ifndef __CMDTABLE_H
#define __CMDTABLE_H

#include "nxwcommn.h"
#include "target.h"

/*!
\brief Type of command
*/
enum CmdType
{
	CMD_TARGET=1,		//!< Call target struct specified in cmd_extra
	CMD_FUNC,		//!< Call function specified in cmd_extra, must be of type GMFUNC
	CMD_TARGETN,		//!< target with 1 param
	CMD_TARGETNNN,		//!< target with 3 param
	CMD_TARGETS,		//!< target with 1 string param
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

/* All command_ functions take an int value of the player that triggered the command. */
#define CMD_HANDLER(name) extern void name ( NXWCLIENT ps )


/* Defined commands that are just being mapped to internal functions */
#define command_time telltime

/* All defined commands */
CMD_HANDLER(command_setGmMoveEff);
CMD_HANDLER(command_bounty);
CMD_HANDLER(command_post);
CMD_HANDLER(command_gpost);
CMD_HANDLER(command_rpost);
CMD_HANDLER(command_lpost);
CMD_HANDLER(command_regspawnall);
CMD_HANDLER(command_wipenpcs);
CMD_HANDLER(command_gms);
CMD_HANDLER(command_who);
CMD_HANDLER(command_gmtransfer);
CMD_HANDLER(command_stats);
CMD_HANDLER(command_options);
CMD_HANDLER(command_gotocur);
CMD_HANDLER(command_resend);
CMD_HANDLER(command_teleport);
CMD_HANDLER(command_where);
CMD_HANDLER(command_q);
CMD_HANDLER(command_next);
CMD_HANDLER(command_clear);
CMD_HANDLER(command_newz);
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
//CMD_HANDLER(command_wipe);
//CMD_HANDLER(command_iwipe);
CMD_HANDLER(command_add);
CMD_HANDLER(command_appetite);
CMD_HANDLER(command_addx);
CMD_HANDLER(command_cfg);
CMD_HANDLER(command_rename);
CMD_HANDLER(command_title);
CMD_HANDLER(command_save);
CMD_HANDLER(command_dye);
CMD_HANDLER(command_setpriv);
CMD_HANDLER(command_nodecay);
CMD_HANDLER(command_send);
CMD_HANDLER(command_showtime);
CMD_HANDLER(command_settime);
CMD_HANDLER(command_shutdown);
CMD_HANDLER(command_skills);
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
CMD_HANDLER(command_secondsperuominute);
CMD_HANDLER(command_brightlight);
CMD_HANDLER(command_darklight);
CMD_HANDLER(command_dungeonlight);
CMD_HANDLER(command_time);
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
CMD_HANDLER(command_decay);
CMD_HANDLER(command_pdump);
//CMD_HANDLER(command_rename);
CMD_HANDLER(command_gy);
CMD_HANDLER(command_yell);
CMD_HANDLER(command_squelch);
CMD_HANDLER(command_squelch);
CMD_HANDLER(command_spawnkill);

/*
CMD_HANDLER(command_setacct);
CMD_HANDLER(command_addacct);
CMD_HANDLER(command_banacct);
CMD_HANDLER(command_unbanacct);*/


CMD_HANDLER(command_letusin);
CMD_HANDLER(command_readaccounts);

CMD_HANDLER(command_serversleep);

//taken from 6904t2(5/10/99) - AntiChrist
CMD_HANDLER(command_setmurder);
CMD_HANDLER(command_jail);
CMD_HANDLER(command_sysm);

// SPARHAWK reload race script
CMD_HANDLER(command_reloadracescript);
// SPARHAWK npcrect for small controlled npc's, stores bound info compressed into FX1 & FX2
CMD_HANDLER(command_npcrectcoded);
CMD_HANDLER(command_tweak);





void target_tele( NXWCLIENT ps, P_TARGET t );
void target_remove( NXWCLIENT ps, P_TARGET t );
void target_jail( NXWCLIENT ps, P_TARGET t );
void target_release( NXWCLIENT ps, P_TARGET t );
void target_xbank( NXWCLIENT ps, P_TARGET t );
void target_xsbank( NXWCLIENT ps, P_TARGET t );
void target_makegm( NXWCLIENT ps, P_TARGET t );
void target_makecns( NXWCLIENT ps, P_TARGET t );
void target_killhair( NXWCLIENT ps, P_TARGET t );
void target_killbeard( NXWCLIENT ps, P_TARGET t );
void target_kill( NXWCLIENT ps, P_TARGET t );
void target_resurrect( NXWCLIENT ps, P_TARGET t );
void target_bolt( NXWCLIENT ps, P_TARGET t );
void target_kick( NXWCLIENT ps, P_TARGET t );
void target_xgo( NXWCLIENT ps, P_TARGET t );
void target_movetobag( NXWCLIENT ps, P_TARGET t );
void target_npcaction( NXWCLIENT ps, P_TARGET t );
void target_setamount( NXWCLIENT ps, P_TARGET t );
void target_freeze( NXWCLIENT ps, P_TARGET t );
void target_unfreeze( NXWCLIENT ps, P_TARGET t );
void target_tiledata( NXWCLIENT ps, P_TARGET t );
void target_heal( NXWCLIENT ps, P_TARGET t );
void target_mana( NXWCLIENT ps, P_TARGET t );
void target_stamina( NXWCLIENT ps, P_TARGET t );
void target_fullstats( NXWCLIENT ps, P_TARGET t );
void target_hide( NXWCLIENT ps, P_TARGET t );
void target_unhide( NXWCLIENT ps, P_TARGET t );
void target_possess( NXWCLIENT ps, P_TARGET t );
void target_telestuff( NXWCLIENT ps, P_TARGET t );
void target_emptypack( NXWCLIENT ps, P_TARGET t );
void target_ban( NXWCLIENT ps, P_TARGET t );
void target_spy( NXWCLIENT ps, P_TARGET t );
void target_title( NXWCLIENT ps, P_TARGET t );




#endif /* __CMDTABLE_H */
