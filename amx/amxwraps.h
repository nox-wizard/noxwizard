/*!
 ***********************************************************************************
 *  file    : amxwraps.h
 *
 *  Project : Nox-Wizard
 *
 *  Author  : 
 *
 *  Purpose : header
 *
 ***********************************************************************************
 *//*
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW)		   [http://www.noxwizard.com]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums or mail staff@noxwizard.com  ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    CHANGELOG:
    -----------------------------------------------------------------------------
    DATE					DEVELOPER				DESCRIPTION
    -----------------------------------------------------------------------------

 ***********************************************************************************
 *//*! \file amxwraps.h
	\brief header
 */


#ifndef __AMXWRAPS_H__
#define __AMXWRAPS_H__

#include "amx_api.h"

#define NATIVE(_A) static cell AMX_NATIVE_CALL _A (AMX *amx,cell *params)
#define NATIVE2(_A) cell AMX_NATIVE_CALL _A (AMX *amx,cell *params)

extern int g_AmxErrNo;

enum { AMXERRNO_OK = 0, AMXERRNO_INVSOCKET, AMXERRNO_INVITEM, AMXERRNO_INVCHAR, AMXERRNO_INVREGION,
    AMXERRNO_UNSUPPORTED, AMXERRNO_UNKNOWN, AMXERRNO_INVSET };


typedef enum {
	T_CHAR = 0,
	T_STRING,
	T_INT,
	T_BOOL,
	T_SHORT,
	T_UNICODE,
} VAR_TYPE;

VAR_TYPE getPropertyType(int property);

enum PROP_TYPE{
	PROP_CHARACTER = 0,
	PROP_ITEM, 
	PROP_CALENDAR,
	PROP_GUILD
};


//second cp/ci for positions
#define NXW_CI2_X 0
#define NXW_CI2_Y 1
#define NXW_CI2_Z 2

#define NXW_CP2_X 0
#define NXW_CP2_Y 1
#define NXW_CP2_Z 2
#define NXW_CP2_DZ 3

//for stats : effective, decimals and real value. actual is hp/stm/mn
#define NXW_CP2_EFF   0
#define NXW_CP2_DEC   1
#define NXW_CP2_REAL  2
#define NXW_CP2_ACT   3

#define NXW_CP2_STARTATTACK 0
#define NXW_CP2_IDLE 1
#define NXW_CP2_ATTACK 2
#define NXW_CP2_DEFEND 3
#define NXW_CP2_DIE 4

const int NXW_CP_B_CANTRAIN = 0;
const int NXW_CP_B_DEAD = 1;
const int NXW_CP_B_FREE = 2;
const int NXW_CP_B_GUARDED = 3;
const int NXW_CP_B_GUILDTRAITOR = 4;
const int NXW_CP_B_JAILED = 5;
const int NXW_CP_B_INCOGNITO = 6;
const int NXW_CP_B_ONHORSE = 7;
const int NXW_CP_B_POLYMORPH = 8;
const int NXW_CP_B_TAMED = 9;
const int NXW_CP_B_UNICODE = 10;
const int NXW_CP_B_SHOPKEEPER = 11;
const int NXW_CP_B_ATTACKFIRST = 12;
const int NXW_CP_B_ISBEINGTRAINED = 13;
const int NXW_CP_B_GUILDTOGGLE = 14;
const int NXW_CP_B_OVERWEIGHTED = 15;
const int NXW_CP_B_MOUNTED = 16;
const int NXW_CP_B_FROZEN = 17;

const int NXW_CP_C_BLOCKED = 101;
const int NXW_CP_C_CELL = 102;
const int NXW_CP_C_COMMANDLEVEL = 103;
const int NXW_CP_C_DIR = 104;
const int NXW_CP_C_DIR2 = 105;
const int NXW_CP_C_FIXEDLIGHT = 106;
const int NXW_CP_C_FLAG = 107;
const int NXW_CP_C_FLY_STEPS = 108;
const int NXW_CP_C_GMRESTRICT = 109;
const int NXW_CP_C_HIDDEN = 110;
//const int NXW_CP_C_ID = 111;  moved to short
const int NXW_CP_C_LOCKSKILL = 112;
//const int NXW_CP_C_MULTISERIAL2 = 113;  removed
const int NXW_CP_C_NPC = 114;
const int NXW_CP_C_NPCTYPE = 115;
const int NXW_CP_C_NPCWANDER = 116;
const int NXW_CP_C_OLDNPCWANDER = 117;
const int NXW_CP_C_ORGSKIN = 118;
//const int NXW_CP_C_OWNSERIAL2 = 119; removed
const int NXW_CP_C_PRIV2 = 121;
const int NXW_CP_C_REACTIVEARMORED = 122;
const int NXW_CP_C_REGION = 123;
//const int NXW_CP_C_SERIAL2 = 124; removed
const int NXW_CP_C_SHOP = 125; // Sparhawk: DEPRECIATED, only for backwards compatability, has been changed to const int NXW_CP_B_SHOPKEEPER = 11;
//const int NXW_CP_C_SKIN = 126; moved to short
const int NXW_CP_C_SPEECH = 127;
const int NXW_CP_C_WAR = 128;
//const int NXW_CP_C_XID = 129; moved to short
//const int NXW_CP_C_XSKIN = 130; moved to short
const int NXW_CP_C_NXWFLAGS = 131;
const int NXW_CP_I_RESISTS = 132;
const int NXW_CP_C_TRAININGPLAYERIN = 133;
const int NXW_CP_C_PRIV = 134;
const int NXW_CP_C_DAMAGETYPE = 135;

const int NXW_CP_I_ACCOUNT = 200;
const int NXW_CP_I_ADVOBJ = 201;
const int NXW_CP_I_ATT = 202;
const int NXW_CP_I_ATTACKER = 203;
const int NXW_CP_I_BEARDCOLOR = 204;
const int NXW_CP_I_BEARDSERIAL = 205;
const int NXW_CP_I_BEARDSTYLE = 206;
const int NXW_CP_I_CALLNUM = 207;
const int NXW_CP_I_CARVE = 208;
const int NXW_CP_I_CASTING = 209;
const int NXW_CP_I_CLIENTIDLETIME = 210;
const int NXW_CP_I_COMBATHITMESSAGE = 211;
const int NXW_CP_I_CREATIONDAY = 212;
const int NXW_CP_I_CRIMINALFLAG = 213;
const int NXW_CP_I_DEATHS = 214;
const int NXW_CP_I_DEF = 215;
const int NXW_CP_I_DEXTERITY = 216;
const int NXW_CP_I_DISABLED = 217;
const int NXW_CP_I_FAME = 218;
const int NXW_CP_I_FLEEAT = 219;
const int NXW_CP_I_FOODPOSITION = 220;
const int NXW_CP_I_FPOS1_NPCWANDER = 221;
const int NXW_CP_I_FPOS2_NPCWANDER = 222;
const int NXW_CP_I_FTARG = 223;
const int NXW_CP_I_GMMOVEEFF = 224;
const int NXW_CP_I_GUILDFEALTY = 225;
const int NXW_CP_I_GUILDNUMBER = 226;
const int NXW_CP_I_HAIRCOLOR = 228;
const int NXW_CP_I_HAIRSERIAL = 229;
const int NXW_CP_I_HAIRSTYLE = 230;
const int NXW_CP_I_HIDAMAGE = 231;
const int NXW_CP_I_HOLDGOLD = 232;
const int NXW_CP_I_HOMELOCPOS = 233;
const int NXW_CP_I_HUNGER = 234;
const int NXW_CP_I_HUNGERTIME = 235;
const int NXW_CP_I_INTELLIGENCE = 236;
const int NXW_CP_I_KARMA = 237;
const int NXW_CP_I_KEYNUMBER = 238;
const int NXW_CP_I_KILLS = 239;
const int NXW_CP_I_LODAMAGE = 240;
const int NXW_CP_I_LOGOUT = 241;
const int NXW_CP_I_MAKING = 242;
const int NXW_CP_I_MEDITATING = 243;
const int NXW_CP_I_MULTISERIAL = 245;
const int NXW_CP_I_MURDERERSER = 246;
const int NXW_CP_I_MURDERRATE = 247;
const int NXW_CP_I_MUTETIME = 248;
const int NXW_CP_I_NAMEDEED = 249;
const int NXW_CP_I_NEXTACT = 250;
const int NXW_CP_I_NPCAI = 251;
const int NXW_CP_I_NPCMOVETIME = 252;
const int NXW_CP_I_OBJECTDELAY = 253;
const int NXW_CP_I_OLDPOS = 254;
const int NXW_CP_I_OWNSERIAL = 255;
const int NXW_CP_I_PACKITEM = 256;
const int NXW_CP_I_POISON = 257;
const int NXW_CP_I_POISONED = 258;
const int NXW_CP_I_POISONTIME = 259;
const int NXW_CP_I_POISONTXT = 260;
const int NXW_CP_I_POISONWEAROFFTIME = 261;
const int NXW_CP_I_POSITION = 262;
const int NXW_CP_I_POSTTYPE = 263;
const int NXW_CP_I_PREVPOS = 264;
//const int NXW_CP_I_PRIV3 = 265;
const int NXW_CP_I_QUESTBOUNTYPOSTSERIAL = 266;
const int NXW_CP_I_QUESTBOUNTYREWARD = 267;
const int NXW_CP_I_QUESTDESTREGION = 268;
const int NXW_CP_I_QUESTORIGREGION = 269;
const int NXW_CP_I_REATTACKAT = 270;
const int NXW_CP_I_REGENRATE = 271;
const int NXW_CP_I_SCRIPTID = 272;
const int NXW_CP_I_GUILD = 273;
const int NXW_CP_I_ROBE = 274;
const int NXW_CP_I_RUNNING = 275;
const int NXW_CP_I_SERIAL = 276;
const int NXW_CP_I_SKILLDELAY = 277;
const int NXW_CP_I_SMOKEDISPLAYTIME = 279;
const int NXW_CP_I_SMOKETIMER = 280;
const int NXW_CP_I_SPADELAY = 281;
const int NXW_CP_I_SPATIMER = 282;
const int NXW_CP_I_SPATTACK = 283;
const int NXW_CP_I_SPAWNREGION = 284;
const int NXW_CP_I_SPAWNSERIAL = 285;
const int NXW_CP_I_SPELL = 286;
const int NXW_CP_I_SPELLACTION = 287;
const int NXW_CP_I_SPELLTIME = 288;
const int NXW_CP_I_SPLIT = 290;
const int NXW_CP_I_SPLITCHNC = 291;
const int NXW_CP_I_SQUELCHED = 292;
const int NXW_CP_I_STABLEMASTER_SERIAL = 293;
const int NXW_CP_I_STEALTH = 294;
const int NXW_CP_I_STRENGHT = 295;
const int NXW_CP_I_SUMMONTIMER = 296;
const int NXW_CP_I_SWINGTARG = 297;
const int NXW_CP_I_TAMING = 299;
const int NXW_CP_I_TARG = 300;
const int NXW_CP_I_TARGTRIG = 301;
const int NXW_CP_I_TEMPFLAGTIME = 302;
const int NXW_CP_I_TIME_UNUSED = 303;
const int NXW_CP_I_TIMEOUT = 304;
const int NXW_CP_I_TIMEUSED_LAST = 305;
const int NXW_CP_I_TRAINER = 309;
const int NXW_CP_I_FLEETIMER = 310;
const int NXW_CP_I_TRIGGER = 311;
const int NXW_CP_I_WEIGHT = 312;
const int NXW_CP_I_WORKLOCPOS = 313;
const int NXW_CP_I_AMXFLAGS = 314;
const int NXW_CP_I_RACE = 315;
const int NXW_CP_I_CX = 316;
const int NXW_CP_I_CY = 317;
const int NXW_CP_I_CZ = 318;
const int NXW_CP_I_LASTMOVETIME = 319;
const int NXW_CP_I_PARTY = 320;

const int NXW_CP_S_BASESKILL = 400;
const int NXW_CP_S_SKILL = 401;
const int NXW_CP_S_GUILDTYPE = 402;
const int NXW_CP_S_ID = 403;
const int NXW_CP_S_SKIN = 404;
const int NXW_CP_S_XID = 405;
const int NXW_CP_S_XSKIN = 406;
const int NXW_CP_S_ICON = 407;
const int NXW_CP_S_SOUND = 408;
const int NXW_CP_S_RACE = 409;

const int NXW_CP_STR_DISABLEDMSG = 450;
const int NXW_CP_STR_GUILDTITLE = 451;
const int NXW_CP_STR_LASTON = 452;
const int NXW_CP_STR_NAME = 453;
const int NXW_CP_STR_ORGNAME = 454;
const int NXW_CP_STR_TITLE = 455;
const int NXW_CP_STR_TRIGWORD = 456;
const int NXW_CP_STR_SPEECHWORD = 457;
const int NXW_CP_STR_SPEECH = 458;

const int NXW_CP_UNI_SPEECH_CURRENT = 500;
const int NXW_CP_UNI_PROFILE = 501;


bool  		getCharBoolProperty(P_CHAR pc, int property, int prop2);
int   		getCharIntProperty(P_CHAR pc, int property, int prop2, int prop3=INVALID );
short 		getCharShortProperty(P_CHAR pc, int property, int prop2);
char			getCharCharProperty(P_CHAR pc, int property, int prop2);
const char*	getCharStrProperty(P_CHAR pc, int property, int prop2);
wstring&		getCharUniProperty( P_CHAR pc, int property, int prop2 );

void	setCharIntProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, int value );
void	setCharBoolProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, LOGICAL value );
void	setCharShortProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, short value );
void	setCharCharProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, char value );
void	setCharStrProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, char* value );
void	setCharUniProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, wstring& value );


///// NOW ITEM PROPS :)
const int NXW_IP_B_INCOGNITO = 0;
//const int NXW_IP_C_COLOR = 100; moved to short
//const int NXW_IP_C_CONTAINERSERIAL2 = 101; removed
const int NXW_IP_C_CORPSE = 102;
const int NXW_IP_C_DOORDIR = 103;
const int NXW_IP_C_DOOROPEN = 104;
const int NXW_IP_C_DYE = 105;
const int NXW_IP_C_FREE = 106;
//const int NXW_IP_C_GLOWFX = 107;
//const int NXW_IP_C_GLOWOLDCOLOR = 108;
//const int NXW_IP_C_ID = 109; moved to short
const int NXW_IP_C_LAYER = 110;
const int NXW_IP_C_MAGIC = 111;
const int NXW_IP_C_MORE = 112;
const int NXW_IP_C_MOREB = 113;
const int NXW_IP_C_OFFSPELL = 114;
const int NXW_IP_C_OLDLAYER = 115;
//const int NXW_IP_C_OWNERSERIAL2 = 116; removed
const int NXW_IP_C_PILEABLE = 117;
const int NXW_IP_C_PRIV = 118;
//const int NXW_IP_C_SERIAL2 = 119; removed
const int NXW_IP_C_VISIBLE = 120;
const int NXW_IP_C_DAMAGETYPE = 121;
const int NXW_IP_C_AUXDAMAGETYPE = 122;

const int NXW_IP_I_ATT = 200;
const int NXW_IP_I_CARVE = 201;
const int NXW_IP_I_CONTAINERSERIAL = 202;
const int NXW_IP_I_DECAYTIME = 203;
const int NXW_IP_I_DEF = 204;
const int NXW_IP_I_DEXBONUS = 205;
const int NXW_IP_I_DEXREQUIRED = 206;
const int NXW_IP_I_DISABLED = 207;
const int NXW_IP_I_GATENUMBER = 208;
const int NXW_IP_I_GATETIME = 209;
//const int NXW_IP_I_GLOW = 210;
const int NXW_IP_I_GOOD = 211;
const int NXW_IP_I_HIDAMAGE = 212;
const int NXW_IP_I_HP = 213;
const int NXW_IP_I_INTBONUS = 214;
const int NXW_IP_I_INTREQUIRED = 215;
const int NXW_IP_I_ITEMHAND = 216;
const int NXW_IP_I_LODAMAGE = 217;
const int NXW_IP_I_MADEWITH = 218;
const int NXW_IP_I_MAXHP = 219;
const int NXW_IP_I_MOREPOSITION = 220;
const int NXW_IP_I_MULTISERIAL = 221;
const int NXW_IP_I_MURDERTIME = 222;
const int NXW_IP_I_OLDCONTAINERSERIAL = 223;
const int NXW_IP_I_OLDPOSITION = 224;
const int NXW_IP_I_OWNERSERIAL = 225;
const int NXW_IP_I_POISONED = 226;
const int NXW_IP_I_POSITION = 227;
const int NXW_IP_I_RANK = 228;
const int NXW_IP_I_REQSKILL = 229;
const int NXW_IP_I_RESTOCK = 230;
const int NXW_IP_I_RNDVALUERATE = 231;
const int NXW_IP_I_SECUREIT = 232;
const int NXW_IP_I_SERIAL = 233;
const int NXW_IP_I_SMELT = 234;
const int NXW_IP_I_SPAWNREGION = 235;
const int NXW_IP_I_SPAWNSERIAL = 236;
const int NXW_IP_I_SPEED = 237;
const int NXW_IP_I_STRBONUS = 238;
const int NXW_IP_I_STRREQUIRED = 239;
const int NXW_IP_I_TIME_UNUSED = 240;
const int NXW_IP_I_TIME_UNUSEDLAST = 241;
const int NXW_IP_I_TRIGGER = 242;
const int NXW_IP_I_TRIGGERUSES = 243;
const int NXW_IP_I_TRIGTYPE = 244;
const int NXW_IP_I_TYPE = 245;
const int NXW_IP_I_TYPE2 = 246;
const int NXW_IP_I_VALUE = 247;
const int NXW_IP_I_WEIGHT = 248;
const int NXW_IP_I_WIPE = 249;
const int NXW_IP_I_AMXFLAGS = 250;
const int NXW_IP_I_SCRIPTID = 251;
const int NXW_IP_I_ANIMID = 252;
const int NXW_IP_I_RESISTS = 253;
const int NXW_IP_I_AUXDAMAGE = 254;
const int NXW_IP_I_AMMO = 255;
const int NXW_IP_I_AMMOFX = 256;

const int NXW_IP_S_AMOUNT = 400;
const int NXW_IP_S_AMOUNT2 = 401;
const int NXW_IP_S_DIR = 402;
const int NXW_IP_S_COLOR = 403;
const int NXW_IP_S_ID = 404;

const int NXW_IP_STR_CREATOR = 450;
const int NXW_IP_STR_DESCRIPTION = 451;
const int NXW_IP_STR_DISABLEDMSG = 452;
const int NXW_IP_STR_MURDERER = 453;
const int NXW_IP_STR_NAME = 454;
const int NXW_IP_STR_NAME2 = 455;


bool  		getItemBoolProperty(P_ITEM pi, int property, int prop2);
int   		getItemIntProperty(P_ITEM pi, int property, int prop2);
short 		getItemShortProperty(P_ITEM pi, int property, int prop2);
char			getItemCharProperty(P_ITEM pi, int property, int prop2);
const char*	getItemStrProperty(P_ITEM pi, int property, int prop2);
wstring&		getItemUniProperty( P_ITEM pi, int property, int prop2 );

void setItemBoolProperty(P_ITEM pi, int property, int prop2, bool value );
void setItemIntProperty(P_ITEM pi, int property, int prop2, int value );
void setItemShortProperty(P_ITEM pi, int property, int prop2, short value );
void setItemCharProperty(P_ITEM pi, int property, int prop2, char value );
void setItemStrProperty(P_ITEM pi, int property, int prop2, char* value );
void setItemUniProperty( P_ITEM pi, int property, int prop2, std::wstring& value );


// calendar properties added by Sparhawk
const int NXW_CALP_I_YEAR = 200;
const int NXW_CALP_I_MONTH = 201;
const int NXW_CALP_I_MAXMONTH = 202;
const int NXW_CALP_I_DAY = 203;
const int NXW_CALP_I_WEEKDAY = 204;
const int NXW_CALP_I_MAXWEEKDAY = 205;
const int NXW_CALP_I_HOUR = 206 ;
const int NXW_CALP_I_MINUTE = 207;
const int NXW_CALP_I_DAWNHOUR = 208;
const int NXW_CALP_I_DAWNMINUTE = 209;
const int NXW_CALP_I_SUNSETHOUR = 210;
const int NXW_CALP_I_SUNSETMINUTE = 211;
const int NXW_CALP_I_SEASON = 212 ;
const int NXW_CALP_STR_MONTHNAME = 450 ;
const int NXW_CALP_STR_WEEKDAYNAME = 451;
/* not implemented yet
const int NXW_CALP_STR_SEASONNAME = 452;
*/


// Speech constants added by Sparhawk
const int NXW_SPEECH_TALK = 1;
const int NXW_SPEECH_TALK_ALL = 2;
const int NXW_SPEECH_TALK_RUNIC = 3;
const int NXW_SPEECH_TALK_ALL_RUNIC = 4;
const int NXW_SPEECH_EMOTE = 5;
const int NXW_SPEECH_EMOTE_ALL = 6;

//Endymion Guild Constant

//const int NXW_GP_B = 0;
//const int NXW_GP_C = 100;
const int NXW_GP_C_TYPE = 100;
//const int NXW_GP_I = 200;
//const int NXW_GP_S = 400;
//const int NXW_GP_STR = 450;
const int NXW_GP_STR_NAME = 450;
const int NXW_GP_STR_WEBPAGE = 451;
const int NXW_GP_STR_ABBREVIATION = 452;
//const int NXW_GP_UNI = 500;
const int NXW_GP_UNI_CHARTER = 500;

//Endymion Guild Member Constant

//const int NXW_GP_B = 0;
//const int NXW_GP_C = 100;
const int NXW_GMP_C_RANK = 100;
const int NXW_GMP_C_TITLETOGGLE = 101;
//const int NXW_GP_I = 200;
//const int NXW_GP_S = 400;
//const int NXW_GP_STR = 450;
const int NXW_GMP_STR_TITLE = 450;
//const int NXW_GP_UNI = 500;

//Endymion Guild Recruit Constant

//const int NXW_GP_B = 0;
//const int NXW_GP_C = 100;
//const int NXW_GP_I = 200;
const int NXW_GRP_I_RECRUITER = 200;
//const int NXW_GP_S = 400;
//const int NXW_GP_STR = 450;
//const int NXW_GP_UNI = 500;



const int NXW_MP_B_CLOSEABLE = 0;
const int NXW_MP_B_MOVEABLE = 1;
const int NXW_MP_B_DISPOSEABLE = 2;
const int NXW_MP_B_RADIO = 3;
const int NXW_MP_B_CHECK = 4;
const int NXW_MP_I_X = 200;
const int NXW_MP_I_Y = 201;
const int NXW_MP_I_ID = 202;
const int NXW_MP_I_BUFFER = 203;
const int NXW_MP_STR_CALLBACK = 450;
const int NXW_MP_STR_BUFFER = 451;
const int NXW_MP_UNI_TEXT = 500;


///
/// Race constants
///

#define RP2_DESCRIPTION_COUNT 1

const int RP_B_TELEPORT_ON_ENLIST = 0;
const int RP_B_WITH_WEB_INTERFACE = 1;

const int RP_C_TYPE = 100;
const int RP_C_LAYER_PERMITTED = 101;

const int RP_S_SKIN = 400;

const int RP_STR_NAME = 450;
const int RP_STR_DESCRIPTION = 451;
const int RP_STR_WEBLINK = 452;
const int RP_STR_WEBROOT = 453;

///
/// Party constants
///

const int PP_B_CANLOOT = 0;

const int PP_C_MEMBERS = 100;
const int PP_C_CANDIDATES = 101;

const int PP_I_LEADER = 200;



#endif //__AMXWRAPS_H__
