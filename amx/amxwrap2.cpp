/*
    This file is part of NoX-Wizard
    -----------------------------------------------------------------------------
    Portions are copyright 2001, Marco Mastropaolo (Xanathar).

    NoX-Wizard is a modified version of Uox (and before that of wolfpack)
    so all previous copyright notices applies and above all

	Copyright 1997, 98 by Marcus Rating (Cironian)

    -----------------------------------------------------------------------------

    NoX-Wizard is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    NoX-Wizard is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with NoX-Wizard; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    -----------------------------------------------------------------------------

    NoX-Wizard also contains portions of code from the Small toolkit.

    The software toolkit "Small", the compiler, the abstract machine and the
    documentation, are copyright (c) 1997-2001 by ITB CompuPhase.
	See amx_vm.c and/or small_license.txt for more information about this.

    ==============================================================================
*/
/*****************************************************************
 Wrapper Native Functions for AMX abstract machine
 by Xanathar
 
 Comments : this is one of the borest files I've ever written :D

 12-09-2002 <Luxor>: Changed to work with serials insted of old indexes :)
 *****************************************************************/
#include "nxwcommn.h"
#include "basics.h"
#include "cmdtable.h"
#include "speech.h"
#include "sndpkg.h"
#include "sregions.h"
#include "amxscript.h"
#include "amxwraps.h"
#include "amxcback.h"
#include "version.h"
#include "calendar.h"

#define T_CHAR 0
#define T_STRING 1
#define T_INT 2
#define T_BOOL 3
#define T_SHORT 4
#define T_UNICODE 5

static void *getCalPropertyPtr(int i, int property, int prop2); //Sparhawk

static char emptyString[1] = { '\0' };
static wstring emptyUnicodeString;

static bool  	getCharBoolProperty(P_CHAR pc, int property, int prop2);
static int   	getCharIntProperty(P_CHAR pc, int property, int prop2, int prop3=INVALID );
static short 	getCharShortProperty(P_CHAR pc, int property, int prop2);
static char	getCharCharProperty(P_CHAR pc, int property, int prop2);
static char*	getCharStrProperty(P_CHAR pc, int property, int prop2);
static wstring* getCharUniProperty( P_CHAR pc, int property, int prop2 );


static bool  	getItemBoolProperty(P_ITEM pi, int property, int prop2);
static int   	getItemIntProperty(P_ITEM pi, int property, int prop2);
static short 	getItemShortProperty(P_ITEM pi, int property, int prop2);
static char	getItemCharProperty(P_ITEM pi, int property, int prop2);
static const char*	getItemStrProperty(P_ITEM pi, int property, int prop2);

extern int g_nStringMode;

//static void *getGuildPropertyPtr(int i, int property, int prop2);


static int getPropertyType(int property)
{
	if (property < 100) return T_BOOL;
	if (property < 200) return T_CHAR;
	if (property < 400) return T_INT;
	if (property < 450) return T_SHORT;
	if (property < 500) return T_STRING;
	return T_UNICODE;
}

/*
 Function	:	_getCalProperty
 Purpose	:	return calendar property
 Inputparms	:	params[1]	-	never
			params[2]	-	property
			params[3]	-	optional month or weekday number
 Outputparms:	params[4]	-	string/array reference
 Returnvalue:	value of numeric property or length of params[4]
 Creator	:	Sparhawk
 Datecreated:	2001-09-15
 Nxw version:	054b
 Version	:	1.0
 Dateupdated:	2001-09-15
 Notes	:
 History	:
*/
NATIVE2(_getCalProperty)
{
	int tp = getPropertyType(params[2]);

	if (tp==T_INT) {
		int *p = reinterpret_cast<int*>(getCalPropertyPtr(-1, params[1], params[2]));
		cell i = *p;
		return i;
	}
	if (tp==T_BOOL) {
		bool *p = reinterpret_cast<bool*>(getCalPropertyPtr(-1, params[1], params[2]));
		cell i = *p;
		return i;
	}
	if (tp==T_SHORT) {
		short *p = reinterpret_cast<short*>(getCalPropertyPtr(-1, params[1], params[2]));
		cell i = *p;
		return i;
	}
	if (tp==T_CHAR) {
		char *p = reinterpret_cast<char*>(getCalPropertyPtr(-1, params[1], params[2]));
		cell i = *p;
		return i;
	}

	//we're here so we should pass a string, params[4] is a str ptr

	char str[100];
	cell *cptr;
	strcpy(str, reinterpret_cast<char*>(getCalPropertyPtr(params[1], params[2], params[3])));

	amx_GetAddr(amx,params[4],&cptr);
	amx_SetString(cptr,str, g_nStringMode);

	return strlen(str);
}



NATIVE2(_getCharProperty)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);

	if ( ISVALIDPC( pc ) )
	{
		int tp = getPropertyType(params[2]);
		if (tp==T_INT)
		{
			/*cell *cptr2;
			amx_GetAddr(amx,params[4],&cptr2);
			int prop3 = *cptr2;*/
			int p = getCharIntProperty( pc, params[2], params[3], params[4]);
			cell i = p;
			return i;
		}
		if (tp==T_BOOL)
		{
			bool p = getCharBoolProperty( pc, params[2], params[3]);
			cell i = p;
			return i;
		}
		if (tp==T_SHORT)
		{
			short p = getCharShortProperty( pc, params[2], params[3]);
			cell i = p;
			return i;
		}
		if (tp==T_CHAR)
		{
			char p = getCharCharProperty( pc, params[2], params[3]);
			cell i = p;
			return i;
		}
		if (tp==T_STRING )
		{
		//we're here so we should pass a string, params[4] is a str ptr
	  		char str[100];	
  			cell *cptr;
	  		strcpy(str, getCharStrProperty( pc, params[2], params[3]));

  			amx_GetAddr(amx,params[4],&cptr);
	  		amx_SetString(cptr,str, g_nStringMode);

  			return strlen(str);
		}
		if (tp==T_UNICODE )
		{
			wstring* w=getCharUniProperty( pc, params[2], params[3] );
			if( w==NULL ) w=&emptyUnicodeString;
			cell *cptr;
	  		amx_GetAddr(amx,params[4],&cptr);
			amx_SetStringUnicode(cptr, w );
			return w->length();
		}

  	}
  	return INVALID;
}

NATIVE2(_setCharProperty)
{
	// params[1] = chr
	// params[2] = property
	// params[3] = subproperty
	// params[4] = value to set property to
	// params[5] = another sub property

	P_CHAR pc = pointers::findCharBySerial(params[1]);
	if (!ISVALIDPC( pc ))
		return INVALID;

	int tp = getPropertyType(params[2]);

	cell *cptr;
	amx_GetAddr(amx,params[4],&cptr);

	if (tp==T_INT) {
		int p = *cptr;

		switch( params[2] )
		{
			case NXW_CP_I_ACCOUNT :		  				//dec value: 200;
				pc->account = p;
				break;
			case NXW_CP_I_ADVOBJ :			  			//dec value: 201;
				pc->advobj = p;
				break;
			case NXW_CP_I_ATT :		  				//dec value: 202;
				pc->att = p;
				break;
			case NXW_CP_I_ATTACKER :		  			//dec value: 203;
				pc->attackerserial = p;
				break;
			case NXW_CP_I_BEARDCOLOR :					//dec value: 204;
				// obsolete
				break;
			case NXW_CP_I_BEARDSERIAL :			  		//dec value: 205;
				pc->beardserial = p;
				break;
			case NXW_CP_I_BEARDSTYLE :					//dec value: 206;
				//obsolete
				break;
			case NXW_CP_I_CALLNUM :			  			//dec value: 207;
				pc->callnum = p;
				break;
			case NXW_CP_I_CARVE :		  				//dec value: 208;
				pc->carve = p;
				break;
			case NXW_CP_I_CASTING :			  			//dec value: 209;
				pc->casting = p;
				break;
			case NXW_CP_I_CLIENTIDLETIME :				  	//dec value: 210;
				pc->clientidletime = p;
				break;
			case NXW_CP_I_COMBATHITMESSAGE :			  	//dec value: 211;
				pc->combathitmessage = p;
				break;
			case NXW_CP_I_CREATIONDAY :			  		//dec value: 212;
				pc->SetCreationDay(p);
				break;
			case NXW_CP_I_CRIMINALFLAG :			  		//dec value: 213;
				pc->crimflag = p;
				break;
			case NXW_CP_I_DEATHS :			  			//dec value: 214;
				pc->deaths = p;
				break;
			case NXW_CP_I_DEF :		  				//dec value: 215;
				pc->def = p;
				break;
			case NXW_CP_I_DEXTERITY:					//dec value: 216
				switch( params[3] )
				{
					case NXW_CP2_DEC :
						pc->dx2 = p;
					case NXW_CP2_REAL :
						pc->dx3 = p;
					case NXW_CP2_ACT :
						pc->stm = p;
					case NXW_CP2_EFF:
						pc->dx = p;
				}
				break;
			case NXW_CP_I_DISABLED :		  			//dec value: 217;
				pc->disabled = p;
				break;
			case NXW_CP_I_FAME :		  				//dec value: 218;
				pc->SetFame( p );
				break;
			case NXW_CP_I_FLEEAT :			  			//dec value: 219;
				pc->fleeat = p;
				break;
			case NXW_CP_I_FOODPOSITION:					//dec value: 220;
				switch( params[3] )
				{
					case NXW_CP2_X :
						pc->foodloc.x = p;
						break;
					case NXW_CP2_Y :
						pc->foodloc.y = p;
						break;
					case NXW_CP2_Z :
						pc->foodloc.z = p;
				}
				break;
			case NXW_CP_I_FPOS1_NPCWANDER:					//dec value: 221;
				switch( params[3] )
				{
					case NXW_CP2_X :
						pc->fx1 = p;
						break;
					case NXW_CP2_Y :
						pc->fy1 = p;
						break;
					case NXW_CP2_Z :
						pc->fz1 = p;
						break;
				}
				break;
			case NXW_CP_I_FPOS2_NPCWANDER:					//dec value 222;
				switch( params[3] )
				{
					case NXW_CP2_X :
						pc->fx2 = p;
						break;
					case NXW_CP2_Y :
						pc->fy2 = p;
					case NXW_CP2_Z :
						break;
				}
				break;
			case NXW_CP_I_FTARG :			  				//dec value: 223;
				pc->ftargserial = p;
				break;
			case NXW_CP_I_GMMOVEEFF :			  			//dec value: 224;
				pc->gmMoveEff = p;
				break;
			case NXW_CP_I_GUILDFEALTY :				  		//dec value: 225;
				pc->SetGuildFealty( p );
				break;
			case NXW_CP_I_GUILDNUMBER :				  		//dec value: 226;
				pc->SetGuildNumber( p );
				break;
			case NXW_CP_I_HAIRCOLOR :						//dec value: 228;
				//obsolete
				break;
			case NXW_CP_I_HAIRSERIAL :				  		//dec value: 229;
				pc->hairserial = p;
				break;
			case NXW_CP_I_HAIRSTYLE :						//dec value: 230;
				//obsolete
				break;
			case NXW_CP_I_HIDAMAGE :			  			//dec value: 231;
				pc->hidamage = p;
				break;
			case NXW_CP_I_HOLDGOLD :			  			//dec value: 232;
				pc->holdg = p;
				break;
			case NXW_CP_I_HOMELOCPOS :						//dec value: 233
				switch( params[3] )
				{
					case NXW_CP2_X :
						pc->homeloc.x = p;
						break;
					case NXW_CP2_Y :
						pc->homeloc.y = p;
						break;
					case NXW_CP2_Z :
						pc->homeloc.z = p;
						break;
				}
				break;
			case NXW_CP_I_HUNGER :				  			//dec value: 234;
				pc->hunger = p;
				break;
			case NXW_CP_I_HUNGERTIME :				  		//dec value: 235;
				pc->hungertime = p;
				break;
			case NXW_CP_I_INTELLIGENCE:						//dec value: 236;
				switch( params[3] )
				{
					case NXW_CP2_DEC  :
						pc->in2 = p;
						break;
					case NXW_CP2_REAL :
						pc->in3 = p;
						break;
					case NXW_CP2_ACT  :
						pc->mn = p;
						break;
					case NXW_CP2_EFF  :
						pc->in = p;
						break;
				}
				break;
			case NXW_CP_I_KARMA : 				 			//dec value: 237;
				pc->SetKarma( p );
				break;
			case NXW_CP_I_KEYNUMBER :			  			//dec value: 238;
				pc->keyserial = p;
				break;
			case NXW_CP_I_KILLS :			  				//dec value: 239;
				pc->kills = p;
				break;
			case NXW_CP_I_LODAMAGE :			  			//dec value: 240;
				pc->lodamage = p;
				break;
			case NXW_CP_I_LOGOUT :				  			//dec value: 241;
				pc->logout = p;
				break;
			case NXW_CP_I_MAKING :				  			//dec value: 242;
				pc->making = p;
				break;
			case NXW_CP_I_MEDITATING :			  			//dec value: 243;
				pc->med = p;
				break;
			case NXW_CP_I_MENUPRIV :			  			//dec value: 244;
				pc->menupriv = p;
				break;
			case NXW_CP_I_MULTISERIAL :						//dec value: 245;
				pc->setMultiSerial( p );
				break;
			case NXW_CP_I_MURDERERSER :				  		//dec value: 246;
				pc->murdererSer = p;
				break;
			case NXW_CP_I_MURDERRATE :				  		//dec value: 247;
				pc->murderrate = p;
				break;
			case NXW_CP_I_MUTETIME :			  			//dec value: 248;
				pc->mutetime = p;
				break;
			case NXW_CP_I_NAMEDEED :			  			//dec value: 249;
				pc->namedeedserial = p;
				break;
			case NXW_CP_I_NEXTACT :				  			//dec value: 250;
				pc->nextact = p;
				break;
			case NXW_CP_I_NPCAI :				  			//dec value: 251;
				pc->npcaitype = p;
				break;
			case NXW_CP_I_NPCMOVETIME :				  		//dec value: 252;
				pc->npcmovetime = p;
				break;
			case NXW_CP_I_OBJECTDELAY :				  		//dec value: 253;
				pc->objectdelay = p;
				break;
			case NXW_CP_I_OLDPOS:						//dec value: 254;
				switch( params[3] )
				{
					case NXW_CP2_X :
						pc->setOldPosition("x",p);
						break;
					case NXW_CP2_Y :
						pc->setOldPosition("y",p);
						break;
					case NXW_CP2_Z :
						pc->setOldPosition("z",p);
						break;
				}
				break;
			case NXW_CP_I_OWNSERIAL :				  		//dec value: 255;
				pc->setOwnerSerial32( p );
				break;
			case NXW_CP_I_PACKITEM :			  			//dec value: 256;
				pc->packitemserial = p;
				break;
			case NXW_CP_I_POISON :				  			//dec value: 257;
				pc->poison = p;
				break;
			case NXW_CP_I_POISONED :			  			//dec value: 258;
				pc->poisoned = (PoisonType)p;
				break;
			case NXW_CP_I_POISONTIME :				  		//dec value: 259;
				pc->poisontime = p;
				break;
			case NXW_CP_I_POISONTXT :			  			//dec value: 260;
				pc->poisontxt = p;
				break;
			case NXW_CP_I_POISONWEAROFFTIME :				  	//dec value: 261;
				pc->poisonwearofftime = p;
				break;
			case NXW_CP_I_POSITION:						//dec value: 262;
				switch( params[3] )
				{
					case NXW_CP2_X :
						pc->setPosition("x",p);
						break;
					case NXW_CP2_Y :
						pc->setPosition("y",p);
						break;
					case NXW_CP2_Z :
						pc->setPosition("z",p);
				} break;
			case NXW_CP_I_POSTTYPE :			  			//dec value: 263;
				pc->postType = (MsgBoards::PostType)p;
				break;
			case NXW_CP_I_PREVPOS:						//dec value: 264
				switch( params[3] )
				{
					case NXW_CP2_X :
						pc->prevX = p;
					case NXW_CP2_Y :
						pc->prevY = p;
					case NXW_CP2_Z :
						pc->prevZ = p;
				} break;
			case NXW_CP_I_PRIV3 :				  			//dec value: 265;
				pc->priv3[params[3]] = p;
				break;
			case NXW_CP_I_QUESTBOUNTYPOSTSERIAL :					  //dec value: 266;
				pc->questBountyPostSerial = p;
				break;
			case NXW_CP_I_QUESTBOUNTYREWARD :				  	//dec value: 267;
				pc->questBountyReward = p;
				break;
			case NXW_CP_I_QUESTDESTREGION :					  	//dec value: 268;
				pc->questDestRegion = p;
				break;
			case NXW_CP_I_QUESTORIGREGION :					  	//dec value: 269;
				pc->questOrigRegion = p;
				break;
			case NXW_CP_I_REATTACKAT :				  		//dec value: 270;
				pc->reattackat = p;
				break;
			case NXW_CP_I_REGENRATE :				  		//dec value: 271;
				pc->setRegenRate( static_cast<StatType>(params[3]), p, static_cast<VarType>(params[5]) );
				break;
			case NXW_CP_I_SCRIPTID :			  				//dec value: 272;
				pc->setScriptID( p );
				break;
			case NXW_CP_I_ROBE :			  				//dec value: 274;
				pc->robe = p;
				break;
			case NXW_CP_I_RUNNING :				  			//dec value: 275;
				pc->running = p;
				break;
			case NXW_CP_I_SERIAL :				  			//dec value: 276;
				pc->setSerial32(p);
				break;
			case NXW_CP_I_SKILLDELAY :				  		//dec value: 277;
				pc->skilldelay = p;
				break;
			case NXW_CP_I_SMELTITEM :			  			//dec value: 278;
				pc->smeltserial = p;
				break;
			case NXW_CP_I_SMOKEDISPLAYTIME :			  		//dec value: 279;
				pc->smokedisplaytimer = p;
				break;
			case NXW_CP_I_SMOKETIMER :				  		//dec value: 280;
				pc->smoketimer = p;
				break;
			case NXW_CP_I_SPADELAY :			  			//dec value: 281;
				pc->spadelay = p;
				break;
			case NXW_CP_I_SPATIMER :			  			//dec value: 282;
				pc->spatimer = p;
				break;
			case NXW_CP_I_SPATTACK :			  			//dec value: 283;
				pc->spattack = p;
				break;
			case NXW_CP_I_SPAWNREGION :				  		//dec value: 284;
				pc->spawnregion = p;
				break;
			case NXW_CP_I_SPAWNSERIAL :				  		//dec value: 285;
				pc->spawnserial = p;
				break;
			case NXW_CP_I_SPELL :			  				//dec value: 286;
				pc->spell = static_cast<enum magic::SpellId>(p);
				break;
			case NXW_CP_I_SPELLACTION :				  		//dec value: 287;
				pc->spellaction = p;
				break;
			case NXW_CP_I_SPELLTIME :			  			//dec value: 288;
				pc->spelltime = p;
				break;
			case NXW_CP_I_SPLIT :			  				//dec value: 290;
				pc->split = p;
				break;
			case NXW_CP_I_SPLITCHNC :			  			//dec value: 291;
				pc->splitchnc = p;
				break;
			case NXW_CP_I_SQUELCHED :			  			//dec value: 292;
				pc->squelched = p;
				break;
			case NXW_CP_I_STEALTH :				  			//dec value: 294;
				pc->stealth = p;
				break;
			case NXW_CP_I_STRENGHT:							//dec value: 295;
				switch( params[3] )
				{
					case NXW_CP2_DEC :
						pc->st2 = p;
						break;
					case NXW_CP2_REAL:
						pc->st3 = p;
						break;
					case NXW_CP2_ACT :
						pc->hp = p;
						break;
					case NXW_CP2_EFF :
						pc->setStrength(p);
				}
				break;
			case NXW_CP_I_SUMMONTIMER :				  		//dec value: 296;
				pc->summontimer = p;
				break;
			case NXW_CP_I_SWINGTARG :			  			//dec value: 297;
				pc->swingtargserial = p;
				break;
			case NXW_CP_I_TAILITEM :			  			//dec value: 298;
				pc->tailserial = p;
				break;
			case NXW_CP_I_TAMING :				  			//dec value: 299;
				pc->taming = p;
				break;
			case NXW_CP_I_TARG :			  				//dec value: 300;
				pc->targserial = p;
				break;
			case NXW_CP_I_TARGTRIG :			  			//dec value: 301;
				pc->targtrig = p;
				break;
			case NXW_CP_I_TEMPFLAGTIME :				  		//dec value: 302;
				pc->tempflagtime = p;
				break;
			case NXW_CP_I_TIME_UNUSED :				  		//dec value: 303;
				pc->time_unused = p;
				break;
			case NXW_CP_I_TIMEOUT :				 			//dec value: 304;
				pc->timeout = p;
				break;
			case NXW_CP_I_TIMEUSED_LAST :				  		//dec value: 305;
				pc->timeused_last = p;
				break;
			case NXW_CP_I_TRACKINGDISPLAYTIMER :					  //dec value: 306;
				pc->trackingdisplaytimer = p;
				break;
			case NXW_CP_I_TRACKINGTARGET :					  	//dec value: 307;
				pc->trackingtarget_serial = p;
				break;
			case NXW_CP_I_TRACKINGTIMER :				  		//dec value: 308;
				pc->trackingtimer = p;
				break;
			case NXW_CP_I_TRAINER :				  			//dec value: 309;
				break;
			case NXW_CP_I_TRIGGER :				  			//dec value: 311;
				pc->trigger = p;
				break;
			case NXW_CP_I_WEIGHT :				  			//dec value: 312;
				pc->weight = p;
				break;
			case NXW_CP_I_WORKLOCPOS:					//dec value: 313;
				switch( params[3] )
				{
					case NXW_CP2_X :
						pc->workloc.x = p;
						break;
					case NXW_CP2_Y :
						pc->workloc.y = p;
						break;
					case NXW_CP2_Z :
						pc->workloc.z = p;
						break;
				}
				break;
			case NXW_CP_I_AMXFLAGS :					//dec value: 314;
				//
				// AMXFLAGS ARE NOW HANDLED AS NEW STYLE AMX VARS
				//
				//pc->amxflags[params[3]] = p;
				if ( params[3] >= 0 && params[3] <= 15 )
					amxVS.updateVariable( pc->getSerial32(), params[3], p );
				break;
			case NXW_CP_I_RACE :		 				//dec value: 315
				pc->race = p;
				break;
			case NXW_CP_I_CX :			 			//dec value: 316
				pc->setPosition("x",p);
				break;
			case NXW_CP_I_CY :			 			//dec value: 317
				pc->setPosition("y",p);
				break;
			case NXW_CP_I_CZ :			 			//dec value: 318
				pc->setPosition("z",p);
				break;
			case NXW_CP_I_LASTMOVETIME :					//dec value: 319
				pc->LastMoveTime = p;
				break;
			default :
				ErrOut("chr_setProperty called with invalid property %d!\n", params[2] );
				break;
		}

		if (params[2]==NXW_CP_I_DEXTERITY) pc->updateStats(2);
		else if (params[2]==NXW_CP_I_INTELLIGENCE) pc->updateStats(1);
		else if (params[2]==NXW_CP_I_STRENGHT) pc->updateStats(0);

		return p;
	}
	if (tp==T_BOOL)
	{
		bool p = *cptr ? true : false;

		switch( params[2] )
		{
			case NXW_CP_B_CANTRAIN :						//dec value: 0;
				if ( p )
					pc->setCanTrain();
				else
					pc->resetCanTrain();
				break;
			case NXW_CP_B_DEAD :							//dec value: 1;
				break;
			case NXW_CP_B_FREE :							//dec value: 2;
				break;
			case NXW_CP_B_GUARDED :							//dec value: 3;
				pc->guarded = p;
				break;
			case NXW_CP_B_GUILDTRAITOR :						//dec value: 4;
				if (p)
					pc->SetGuildTraitor();
				else
					pc->ResetGuildTraitor();
				break;
			case NXW_CP_B_INCOGNITO :						//dec value: 6;
				pc->guarded = p;
				break;
			case NXW_CP_B_POLYMORPH :						//dec value: 8;
				pc->polymorph = p;						//Candidate for removal
				break;
			case NXW_CP_B_TAMED :							//dec value: 9;
				pc->tamed = p;
				break;
			case NXW_CP_B_UNICODE :							//dec value: 10;
				pc->unicode = p;
				break;
			case NXW_CP_C_SHOPKEEPER :						// dec value 11;
				pc->shopkeeper = p;
				break;
			case NXW_CP_B_ATTACKFIRST :						//dec value: 12;
				if (p)
					pc->SetAttackFirst();
				else
					pc->ResetAttackFirst();
				break;
			case NXW_CP_B_ISBEINGTRAINED :						//dec value: 13;
				break;
			case NXW_CP_B_GUILDTOGGLE :						//dec value: 14;
				if ( p )
					pc->SetGuildTitleToggle();
				else
					pc->ResetGuildTitleToggle();
				break;
			default :
				ErrOut("chr_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	if (tp==T_SHORT) {
		short p = static_cast<short>(*cptr & 0xFFFF);
		switch( params[2] )
		{
			case  NXW_CP_S_BASESKILL :				  		//dec value: 400;
				pc->baseskill[params[3]] = p;
				break;
			case NXW_CP_S_SKILL :				  			//dec value: 401;
				pc->skill[params[3]] = p;
				break;
			case NXW_CP_S_GUILDTYPE :				  		//dec value: 402;
				pc->SetGuildType( p );
				break;
			default :
				ErrOut("chr_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	if (tp==T_CHAR) {
		char p = static_cast<char>(*cptr & 0xFF);

		switch( params[2] )
		{
			case NXW_CP_C_BLOCKED :							//dec value: 101;
				pc->blocked = p;
				break;
			case NXW_CP_C_COMMANDLEVEL :				  		//dec value: 103;
				pc->commandLevel = p;
				break;
			case NXW_CP_C_DIR :	  						//dec value: 104;
				pc->dir = p;
				break;
			case NXW_CP_C_DIR2 :			  				//dec value: 105;
				pc->dir2 = p;
				break;
			case NXW_CP_C_FIXEDLIGHT :				  		//dec value: 106;
				pc->fixedlight = p;
				break;
			case NXW_CP_C_FLAG :			  				//dec value: 107;
				pc->flag = p;
				break;
			case NXW_CP_C_FLY_STEPS :			  			//dec value: 108;
				pc->fly_steps = p;
				break;
			case NXW_CP_C_GMRESTRICT :				  		//dec value: 109;
				pc->gmrestrict = p;
				break;
			case NXW_CP_C_HIDDEN :				  			//dec value: 110;
				pc->hidden = p;
				break;
			case NXW_CP_C_ID :							//dec value: 111;
				{
				  UI16 id = pc->GetBodyType();
				if ( params[3] > 1 )
					pc->SetBodyType((id & 0x00FF) | ( p << 8));
				else
					pc->SetBodyType((id & 0xFF00) | ( p %256));
				}
				break;
			case NXW_CP_C_LOCKSKILL : 						//dec value: 112;
				pc->lockSkill[params[3]] = p;
				break;
			case NXW_CP_C_MULTISERIAL2 :						//dec value: 113;
				if ( params[3] >=1 && params[3] <= 4 )
					pc->setMultiSerialByte( static_cast<UI32>(params[3]), p );
				break;
			case NXW_CP_C_NPC :							//dec value: 114;
				pc->npc = p;
				break;
			case NXW_CP_C_NPCTYPE : 		  				//dec value: 115;
				pc->npc_type = p;
				break;
			case NXW_CP_C_NPCWANDER :						//dec value: 116;
				pc->npcWander = p;
				break;
			case NXW_CP_C_OLDNPCWANDER : 			  			//dec value: 117;
				pc->oldnpcWander = p;
				break;
			//case NXW_CP_C_ORGSKIN : (prop2>1) ? &chars[i].orgskin1 : &chars[i].orgskin2 )  //dec value: 118;
			case NXW_CP_C_OWNSERIAL2 :						//dec value: 119;
				if ( params[3] >=1 && params[3] <= 4 )
					pc->setOwnerSerialByte( static_cast<UI32>(params[3]), p );
				break;
			case NXW_CP_C_PRIV2 :				  			//dec value: 121;
				pc->SetPriv2(p);
				break;
			case NXW_CP_C_REACTIVEARMORED : 		  			//dec value: 122;
				pc->ra = p;
				break;
			case NXW_CP_C_REGION : 				  			//dec value: 123;
				pc->region = p;							//candidate for removal
				break;
			case NXW_CP_C_SERIAL2 :
				if ( params[3] >=1 && params[3] <= 4 )
					pc->setSerialByte( static_cast<UI32>(params[3]), p );
				break;
			case NXW_CP_C_SHOP :				 			//dec value: 125; Sparhawk: DEPRECIATED, use case NXW_CP_B_SHOPKEEPER : pc->shopkeeper )
				pc->shopkeeper = p;
				break;
			case NXW_CP_C_SKIN : 							//dec value: 126;
				{
				  UI16 color = pc->getSkinColor();
				if ( params[3] > 1 )
					pc->setSkinColor((color & 0x00FF) | (p << 8));
				else
					pc->setSkinColor((color & 0xFF00) | (p %256));
				}
				break;
			case NXW_CP_C_SPEECH : 				  			//dec value: 127;
				pc->speech = p;
				break;
			case NXW_CP_C_WAR :			  				//dec value: 128;
				pc->war = p;
				break;
			case NXW_CP_C_XID :							//dec value: 129;
				{
				  UI16 oldbody = pc->GetOldBodyType();
				if ( params[3] > 1 )
					pc->SetOldBodyType((oldbody & 0x00FF) | ( p << 8));
				else
					pc->SetOldBodyType((oldbody & 0xFF00) | ( p %256));
				}
				break;
			case NXW_CP_C_XSKIN : 							//dec value: 130;
				{
				  UI16 oldcolor = pc->getOldSkinColor();
				if ( params[3] > 1 )
					pc->setOldSkinColor((oldcolor & 0x00FF) | (p << 8));
				else
					pc->setOldSkinColor((oldcolor & 0xFF00) | (p %256));
				}
				break;
			case NXW_CP_C_NXWFLAGS :						//dec value: 131;
				pc->nxwflags[params[3]] = p;
				break;
			case NXW_CP_I_RESISTS : 						//dec value: 132;
				pc->resists[params[3]] = p;
				break;
			case NXW_CP_C_TRAININGPLAYERIN : 					//dec value: 133;
				pc->trainingplayerin = p;
				break;
			case NXW_CP_C_PRIV : 				  			//dec value: 134;
				pc->SetPriv(p);
				break;
			case NXW_CP_C_DAMAGETYPE :						//dec value: 319;
				pc->damagetype = static_cast<DamageType>(p);
				break;
			default :
				ErrOut("chr_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	if (tp==T_STRING) {	
		//we're here so we should get a ConOut format string, params[4] is the str format

		cell *cstr;
		amx_GetAddr(amx,params[4],&cstr);
		printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
		g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';
		switch( params[2] )
		{
			case NXW_CP_STR_DISABLEDMSG :			  				//dec value: 450;
				strcpy( pc->disabledmsg, g_cAmxPrintBuffer );
				break;
			case NXW_CP_STR_GUILDTITLE :							//dec value: 451;
				pc->GetGuildTitle();
				break;
			case NXW_CP_STR_LASTON :		  					//dec value: 452;
				break;
			case NXW_CP_STR_NAME :								//dec value: 453;
				pc->setCurrentName( g_cAmxPrintBuffer );
				break;
			case NXW_CP_STR_ORGNAME :		  					//dec value: 454;
				break;
			case NXW_CP_STR_TITLE :			  					//dec value: 455;
				strcpy( pc->title, g_cAmxPrintBuffer );
				break;
			case NXW_CP_STR_TRIGWORD :		  					//dec value: 456;
				strcpy( pc->trigword, g_cAmxPrintBuffer );
				break;
			case NXW_CP_STR_SPEECHWORD :		 				//dec value: 457;
				strcpy( script1, g_cAmxPrintBuffer );
				break;
			case NXW_CP_STR_SPEECH :			 				//dec value: 458;
				strcpy( script2, g_cAmxPrintBuffer );
				break;
			default :
				ErrOut("chr_setProperty called with invalid property %d!\n", params[2] );
				break;
  		}
	  	g_nAmxPrintPtr=0;
		return 0;
	}
	if (tp==T_UNICODE) {	
		cell *cstr;
		amx_GetAddr(amx,params[4],&cstr);

		switch( params[2] )
		{
			case NXW_CP_UNI_SPEECH_CURRENT :		
				pc->setSpeechCurrent( new wstring() );
				amx_GetStringUnicode( pc->getSpeechCurrent(), cstr );
				break;
			case NXW_CP_UNI_PROFILE :				
				pc->setProfile( new wstring() );
				amx_GetStringUnicode( pc->getProfile(), cstr );
				break;
			default :
				ErrOut("chr_setProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		g_nAmxPrintPtr=0;
	  	return 0;

	}
  	g_nAmxPrintPtr=0;
  	return 0;
}


NATIVE2(_getItemProperty)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);

	if ( ISVALIDPI( pi ) )
	{
		int tp = getPropertyType(params[2]);

		if (tp==T_INT)
		{
			int p = getItemIntProperty( pi, params[2], params[3]);
			cell i = p;
			return i;
		}
		if (tp==T_BOOL)
		{
			bool p = getItemBoolProperty( pi, params[2], params[3]);
			cell i = p;
			return i;
		}
		if (tp==T_SHORT)
		{
			short p = getItemShortProperty( pi, params[2], params[3]);
			cell i = p;
			return i;
		}
		if (tp==T_CHAR) {
			char p = getItemCharProperty( pi, params[2], params[3]);
			cell i = p;
			return i;
		}

		//we're here so we should pass a string, params[4] is a str ptr

	  	char str[100];
  		cell *cptr;
	  	strcpy(str, getItemStrProperty( pi, params[2], params[3]));

  		amx_GetAddr(amx,params[4],&cptr);
	  	amx_SetString(cptr,str, g_nStringMode);

  		return strlen(str);
  	}
  	return INVALID;
}

NATIVE2(_setItemProperty)
{
	// params[1] = chr
	// params[2] = property
	// params[3] = subproperty
	// params[4] = value to set property to

	P_ITEM pi = pointers::findItemBySerial(params[1]);
	if (!ISVALIDPI( pi ) )
		return 0;

	int tp = getPropertyType(params[2]);

	cell *cptr;
	amx_GetAddr(amx,params[4],&cptr);


	if (tp==T_INT) {
		int p = *cptr;

		switch( params[2] )
		{
			case NXW_IP_I_ATT :					   //dec value :  200;
				pi->att = p;
				break;
			case NXW_IP_I_CARVE :					   //dec value :  201;
				pi->carve = p;
				break;
			case NXW_IP_I_CONTAINERSERIAL :				   //dec value :  202;
				//pi->contserial = p;
				pi->setContSerial(p, false, false);
				break;
			case NXW_IP_I_DECAYTIME :				   //dec value :  203;
				pi->setDecayTime( (TIMERVAL) p );
				break;
			case NXW_IP_I_DEF :					   //dec value :  204;
				pi->def = p;
				break;
			case NXW_IP_I_DEXBONUS :				   //dec value :  205;
				pi->dx2 = p;
				break;
			case NXW_IP_I_DEXREQUIRED :				   //dec value :  206;
				pi->dx = p;
				break;
			case NXW_IP_I_DISABLED :				   //dec value :  207;
				pi->disabled = p;
				break;
			case NXW_IP_I_GATENUMBER :				   //dec value :  208;
				pi->gatenumber = p;
				break;
			case NXW_IP_I_GATETIME :				   //dec value :  209;
				pi->gatetime = p;
				break;
//			case NXW_IP_I_GLOW :					   //dec value :  210;
//				pi->glow = p;
//				break;
			case NXW_IP_I_GOOD :					 	//dec value :  211;
				pi->good = p;
				break;
			case NXW_IP_I_HIDAMAGE :				   //dec value :  212;
				pi->hidamage = p;
				break;
			case NXW_IP_I_HP :					   //dec value :  213;
				pi->hp = p;
				break;
			case NXW_IP_I_INTBONUS :				   //dec value :  214;
				pi->in2 = p;
				break;
			case NXW_IP_I_INTREQUIRED :				   //dec value :  215;
				pi->in = p;
				break;
			case NXW_IP_I_ITEMHAND :				   //dec value :  216;
				pi->itmhand = p;
				break;
			case NXW_IP_I_LODAMAGE :				   //dec value :  217;
				pi->lodamage = p;
				break;
			case NXW_IP_I_MADEWITH :				   //dec value :  218;
				pi->madewith = p;
				break;
			case NXW_IP_I_MAXHP :					   //dec value :  219;
				pi->maxhp = p;
				break;
			case NXW_IP_I_MOREPOSITION:				//dec value : 220;
				switch(params[3]) {
					case NXW_CI2_X:
						pi->morex = p;
						break;
					case NXW_CI2_Y:
						pi->morey = p;
						break;
					default :
						pi->morez = p;
						break;
				} break;
			case NXW_IP_I_MULTISERIAL :				   //dec value :  221;
				pi->setMultiSerial32Only(p);
				break;
			case NXW_IP_I_MURDERTIME :				   //dec value :  222;
				pi->murdertime = p;
				break;
			case NXW_IP_I_OLDCONTAINERSERIAL :			   //dec value :  223;
				pi->setContSerial(p, true, false);
				break;
			case NXW_IP_I_OLDPOSITION:								//dec value : 224;
				switch(params[3]) {
					case NXW_CI2_X:
						pi->setOldPosition("x", p);
						break;
					case NXW_CI2_Y:
						pi->setOldPosition("y", p);
						break;
					default :
						pi->setOldPosition("z", p);
						break;
				} break;
			case NXW_IP_I_OWNERSERIAL :				   //dec value :  225;
				pi->setOwnerSerial32Only(p);
				break;
			case NXW_IP_I_POISONED :				   //dec value :  226;
				pi->poisoned = (PoisonType)p;
				break;
			case NXW_IP_I_POSITION:							//dec value : 227;
				switch(params[3]) {
					case NXW_CI2_X:
						pi->setPosition("x", p);
						break;
					case NXW_CI2_Y:
						pi->setPosition("y", p);
						break;
					default :
						pi->setPosition("z", p);
						break;
				} break;
			case NXW_IP_I_RANK :					   //dec value :  228;
				pi->rank = p;
				break;
			case NXW_IP_I_REQSKILL :				   //dec value :  229;
				pi->wpsk = p;
				break;
			case NXW_IP_I_RESTOCK :					   //dec value :  230;
				pi->restock = p;
				break;
			case NXW_IP_I_RNDVALUERATE :				   //dec value :  231;
				pi->rndvaluerate = p;
				break;
			case NXW_IP_I_SECUREIT :				   //dec value :  232;
				pi->secureIt = p;
				break;
			case NXW_IP_I_SERIAL :					   //dec value :  233;
				pi->setSerial32(p);
				break;
			case NXW_IP_I_SMELT :					   //dec value :  234;
				pi->smelt = p;
				break;
			case NXW_IP_I_SPAWNREGION :				   //dec value :  235;
				pi->spawnregion = p;
				break;
			case NXW_IP_I_SPAWNSERIAL :				   //dec value :  236;
				pi->spawnserial = p;
				break;
			case NXW_IP_I_SPEED :					   //dec value :  237;
				pi->spd = p;
				break;
			case NXW_IP_I_STRBONUS :				   //dec value :  238;
				pi->st2 = p;
				break;
			case NXW_IP_I_STRREQUIRED :				   //dec value :  239;
				pi->st = p;
				break;
			case NXW_IP_I_TIME_UNUSED :				   //dec value :  240;
				pi->time_unused = p;
				break;
			case NXW_IP_I_TIME_UNUSEDLAST :				   //dec value :  241;
				pi->timeused_last = p;
				break;
			case NXW_IP_I_TRIGGER :					   //dec value :  242;
				pi->trigger = p;
				break;
			case NXW_IP_I_TRIGGERUSES :				   //dec value :  243;
				pi->tuses = p;
				break;
			case NXW_IP_I_TRIGTYPE :				   //dec value :  244;
				pi->trigtype = p;
				break;
			case NXW_IP_I_TYPE :					   //dec value :  245;
				pi->type = p;
				break;
			case NXW_IP_I_TYPE2 :					   //dec value :  246;
				pi->type2 = p;
				break;
			case NXW_IP_I_VALUE :					   //dec value :  247;
				pi->value = p;
				break;
			case NXW_IP_I_WEIGHT :					   //dec value :  248;
				pi->weight = p;
				break;
			case NXW_IP_I_WIPE :					   //dec value :  249;
				pi->wipe = p;
				break;
			case NXW_IP_I_AMXFLAGS : 				  //dec value :  250;
				//
				// AMXFLAGS ARE NOW HANDLED AS NEW STYLE AMX VARS
				//
				//pi->amxflags[params[3]] = p;
				if ( params[3] >= 0 && params[3] <= 15 )
					amxVS.updateVariable( pi->getSerial32(), params[3], p );
				break;
			case NXW_IP_I_SCRIPTID :				//dec value 251;
				pi->setScriptID( p );
				break;
			case NXW_IP_I_ANIMID :					//dec value 252;
				pi->animSetId( p );
				break;
			case NXW_IP_I_RESISTS :					//dec value: 253;
				pi->resists[params[3]] = p;
				break;
			case NXW_IP_I_AUXDAMAGE :				//dec value: 254;
				pi->auxdamage = p;
				break;
			case NXW_IP_I_AMMO :					//dec value: 255;
				pi->ammo = p;
				break;
			case NXW_IP_I_AMMOFX :					//dec value: 256;
				pi->ammoFx = p;
				break;
			default :
				ErrOut("itm_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	if (tp==T_BOOL)
	{
		bool p = *cptr ? true : false;

		switch( params[2] )
		{
			case NXW_IP_B_INCOGNITO :		   //dec value :  0;
				pi->incognito = p;
				break;
			default :
				ErrOut("itm_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	if (tp==T_SHORT) {
		short p = static_cast<short>(*cptr & 0xFFFF);
		switch( params[2] )
		{
			case NXW_IP_S_AMOUNT :			   //dec value :  400;
				pi->amount = p;
				break;
			case NXW_IP_S_AMOUNT2 :			   //dec value :  401;
				pi->amount2 = p;
				break;
			case NXW_IP_S_DIR :			   //dec value :  402;
				pi->dir = p;
				break;
			default :
				ErrOut("itm_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	if (tp==T_CHAR) {
		char p = static_cast<char>(*cptr & 0xFF);

		switch( params[2] )
		{
			case NXW_IP_C_COLOR :					   		//dec value :  100;
				if (params[3] > 1)
					pi->color1 = p;
				else
					pi->color2 = p;
				break;
			case NXW_IP_C_DOORDIR :			   				//dec value :  103;
				pi->doordir = p;
				break;
			case NXW_IP_C_DOOROPEN :		   				//dec value :  104;
				pi->dooropen = p;
				break;
			case NXW_IP_C_DYE :		   					//dec value :  105;
				pi->dye = p;
				break;
			case NXW_IP_C_FREE :							//dec value :  106;
				break;
//			case NXW_IP_C_GLOWFX : 							//dec value :  107;
//				pi->glow_effect = p;
//				break;
//			case NXW_IP_C_GLOWOLDCOLOR :					   	//dec value :  108;
//				if (params[3] > 1)
//					pi->glow_c1 = p;
//				else
//					pi->glow_c2 = p;
//				break;
			case NXW_IP_C_ID :				   			//dec value :  109;
				if (params[3] > 1)
					pi->id1 = p;
				else
					pi->id2 = p;
				break;
			case NXW_IP_C_LAYER :		   					//dec value :  110;
				pi->layer = p;
				break;
			case NXW_IP_C_MAGIC :		   					//dec value :  111;
				pi->magic = p;
				break;
			case NXW_IP_C_OFFSPELL :		   				//dec value :  114;
				pi->offspell = p;
				break;
			case NXW_IP_C_OLDLAYER :		   				//dec value :  115;
				pi->oldlayer = p;
				break;
			case NXW_IP_C_PILEABLE :		   				//dec value :  117;
				pi->pileable = p;
				break;
			case NXW_IP_C_PRIV :		   					//dec value :  118;
				pi->priv = p;
				break;
			case NXW_IP_C_VISIBLE : 						//dec value :  120;
				pi->visible = p;
				break;
			case NXW_IP_C_CONTAINERSERIAL2:
				switch(params[3])
				{
					case 1: pi->setContSerialByte(1, p); break;
					case 2: pi->setContSerialByte(2, p); break;
					case 3: pi->setContSerialByte(3, p); break;
					case 4: pi->setContSerialByte(4, p); break;
				} 
				break;

			case NXW_IP_C_MORE:
				switch(params[3]) {
					case 1:
						pi->more1 = p;
						break;
					case 2:
						pi->more2 = p;
						break;
					case 3:
						pi->more3 = p;
						break;
					case 4:
						pi->more4 = p;
						break;
				} break;

			case NXW_IP_C_MOREB:
				switch(params[3]) {
					case 1:
						pi->moreb1 = p;
						break;
					case 2:
						pi->moreb2 = p;
						break;
					case 3:
						pi->moreb3 = p;
						break;
					case 4:
						pi->moreb4 = p;
						break;
				} break;

			case NXW_IP_C_OWNERSERIAL2:
				switch(params[3]) {
					case 1:
						pi->setOwnerSerialByte(1, p);
						break;
					case 2:
						pi->setOwnerSerialByte(2, p);
						break;
					case 3:
						pi->setOwnerSerialByte(3, p);
						break;
					case 4:
						pi->setOwnerSerialByte(4, p);
						break;
				} break;
			case NXW_IP_C_SERIAL2:
				switch(params[3]) {
					case 1:
						pi->setSerialByte(1, p);
						break;
					case 2:
						pi->setSerialByte(2, p);
						break;
					case 3:
						pi->setSerialByte(3, p);
						break;
					case 4:
						pi->setSerialByte(4, p);

						break;
				} break;
			case NXW_IP_C_DAMAGETYPE :						//dec value: 121;
				pi->damagetype = static_cast<DamageType>(p);
				break;
			case NXW_IP_C_AUXDAMAGETYPE :						//dec value: 122;
				pi->auxdamagetype = static_cast<DamageType>(p);
				break;
			default :
				ErrOut("itm_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	//we're here so we should get a ConOut format string, params[4] is the str format

	cell *cstr;
	amx_GetAddr(amx,params[4],&cstr);
	printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';
	switch( params[2] )
	{
		case NXW_IP_STR_CREATOR :				   //dec value :  450;
			pi->creator = g_cAmxPrintBuffer;
			break;
		case NXW_IP_STR_DESCRIPTION :				   //dec value :  451;
			//strcpy(pi->desc, g_cAmxPrintBuffer );
			pi->vendorDescription = g_cAmxPrintBuffer;
			break;
		case NXW_IP_STR_DISABLEDMSG :				   //dec value :  452;
			pi->disabledmsg = g_cAmxPrintBuffer;
			break;
		case NXW_IP_STR_MURDERER :				   //dec value :  453;
			pi->murderer = string(g_cAmxPrintBuffer);
			break;
		case NXW_IP_STR_NAME :					   //dec value :  454;
			pi->setCurrentName(g_cAmxPrintBuffer);
			break;
		case NXW_IP_STR_NAME2 :					   //dec value :  455;
			pi->setSecondaryName(g_cAmxPrintBuffer);
			break;
		default :
			ErrOut("itm_setProperty called with invalid property %d!\n", params[2] );
			break;
	}
	g_nAmxPrintPtr=0;
	return 0;
}

/*************************************************************************************
 HERE AFTER : PROPERTIES GETPTRS
 *************************************************************************************/
#define CHECK(A,B) case A: return B;

static bool getItemBoolProperty( P_ITEM pi, int property, int prop2)
{
	switch( property )
	{
		CHECK(NXW_IP_B_INCOGNITO, pi->incognito )   //dec value :  0;

	}
	ErrOut("itm_getProperty called with invalid property %d!\n", property );
	return false;
}

static int getItemIntProperty( P_ITEM pi, int property, int prop2)
{
	switch( property )
	{
		CHECK(NXW_IP_I_ATT, pi->att )							//dec value :  200;
		CHECK(NXW_IP_I_CARVE, pi->carve )						//dec value :  201;
		CHECK(NXW_IP_I_CONTAINERSERIAL, pi->getContSerial() )	//dec value :  202;
		CHECK(NXW_IP_I_DECAYTIME, pi->getDecayTime() )				//dec value :  203;
		CHECK(NXW_IP_I_DEF, pi->def )							//dec value :  204;
		CHECK(NXW_IP_I_DEXBONUS, pi->dx2 )						//dec value :  205;
		CHECK(NXW_IP_I_DEXREQUIRED, pi->dx )					//dec value :  206;
		CHECK(NXW_IP_I_DISABLED, pi->disabled )					//dec value :  207;
		CHECK(NXW_IP_I_GATENUMBER, pi->gatenumber )				//dec value :  208;
		CHECK(NXW_IP_I_GATETIME, pi->gatetime )					//dec value :  209;
//		CHECK(NXW_IP_I_GLOW, pi->glow )							//dec value :  210;
		CHECK(NXW_IP_I_GOOD, pi->good )							//dec value :  211;
		CHECK(NXW_IP_I_HIDAMAGE, pi->hidamage )					//dec value :  212;
		CHECK(NXW_IP_I_HP, pi->hp )								//dec value :  213;
		CHECK(NXW_IP_I_INTBONUS, pi->in2 )						//dec value :  214;
		CHECK(NXW_IP_I_INTREQUIRED, pi->in )					//dec value :  215;
		CHECK(NXW_IP_I_ITEMHAND, pi->itmhand )					//dec value :  216;
		CHECK(NXW_IP_I_LODAMAGE, pi->lodamage )					//dec value :  217;
		CHECK(NXW_IP_I_MADEWITH, pi->madewith )					//dec value :  218;
		CHECK(NXW_IP_I_MAXHP, pi->maxhp )						//dec value :  219;
		case NXW_IP_I_MOREPOSITION:									//dec value :  220;
			switch(prop2) {
				case NXW_CI2_X: return pi->morex;
				case NXW_CI2_Y: return pi->morey;
				default :	return pi->morez;
			}
		CHECK(NXW_IP_I_MULTISERIAL, pi->getMultiSerial32() )	//dec value :  221;
		CHECK(NXW_IP_I_MURDERTIME, pi->murdertime )				//dec value :  222;
		CHECK(NXW_IP_I_OLDCONTAINERSERIAL, pi->getContSerial(true) )  //dec value :  223;
		case NXW_IP_I_OLDPOSITION:																		//dec value :  224;
			switch(prop2) {
				case NXW_CI2_X: return pi->getOldPosition().x;
				case NXW_CI2_Y: return pi->getOldPosition().y;
				default :	return pi->getOldPosition().z;
			}
		CHECK(NXW_IP_I_OWNERSERIAL, pi->getOwnerSerial32() )				//dec value :  225;
		CHECK(NXW_IP_I_POISONED, pi->poisoned )					//dec value :  226;
		case NXW_IP_I_POSITION:													//dec value : 227;
			switch(prop2) {
				case NXW_CI2_X: return pi->getPosition().x;
				case NXW_CI2_Y: return pi->getPosition().y;
				default :	return pi->getPosition().z;
			}
		CHECK(NXW_IP_I_RANK, pi->rank )							//dec value :  228;
		CHECK(NXW_IP_I_REQSKILL, pi->wpsk )						//dec value :  229;
		CHECK(NXW_IP_I_RESTOCK, pi->restock )					//dec value :  230;
		CHECK(NXW_IP_I_RNDVALUERATE, pi->rndvaluerate )			//dec value :  231;
		CHECK(NXW_IP_I_SECUREIT, pi->secureIt )					//dec value :  232;
		CHECK(NXW_IP_I_SERIAL, pi->getSerial32() )				//dec value :  233;
		CHECK(NXW_IP_I_SMELT, pi->smelt )						//dec value :  234;
		CHECK(NXW_IP_I_SPAWNREGION, pi->spawnregion )			//dec value :  235;
		CHECK(NXW_IP_I_SPAWNSERIAL, pi->spawnserial )			//dec value :  236;
		CHECK(NXW_IP_I_SPEED, pi->spd )							//dec value :  237;
		CHECK(NXW_IP_I_STRBONUS, pi->st2 )						//dec value :  238;
		CHECK(NXW_IP_I_STRREQUIRED, pi->st )					//dec value :  239;
		CHECK(NXW_IP_I_TIME_UNUSED, pi->time_unused )			//dec value :  240;
		CHECK(NXW_IP_I_TIME_UNUSEDLAST, pi->timeused_last )		//dec value :  241;
		CHECK(NXW_IP_I_TRIGGER, pi->trigger )					//dec value :  242;
		CHECK(NXW_IP_I_TRIGGERUSES, pi->tuses )					//dec value :  243;
		CHECK(NXW_IP_I_TRIGTYPE, pi->trigtype )					//dec value :  244;
		CHECK(NXW_IP_I_TYPE, pi->type )							//dec value :  245;
		CHECK(NXW_IP_I_TYPE2, pi->type2 )						//dec value :  246;
		CHECK(NXW_IP_I_VALUE, pi->value )						//dec value :  247;
		CHECK(NXW_IP_I_WEIGHT, (int)pi->getWeight() )			//dec value :  248;
		CHECK(NXW_IP_I_WIPE, pi->wipe )							//dec value :  249;
		case NXW_IP_I_AMXFLAGS :	//dec value :  250;
				//
				// AMXFLAGS ARE NOW HANDLED AS NEW STYLE AMX VARS
				//
				if ( prop2 >= 0 && prop2 <= 15 )
				{
					int value;
					amxVS.selectVariable( pi->getSerial32(), prop2, value );
					return value;
				}
				return 0;
		CHECK(NXW_IP_I_SCRIPTID, (int)pi->getScriptID() )		//dec value :  251;
		CHECK(NXW_IP_I_ANIMID, (int)pi->animid() )			//dec value :  252;
		CHECK(NXW_IP_I_RESISTS, pi->resists[prop2])			//dec value :  253;
		CHECK(NXW_IP_I_AUXDAMAGE, pi->auxdamage )			//dec value :  254;
		CHECK(NXW_IP_I_AMMO, pi->ammo )					//dec value: 255;
		CHECK(NXW_IP_I_AMMOFX, pi->ammoFx )				//dec value: 256;
	}
	ErrOut("itm_getProperty called with invalid property %d!\n", property );
	return INVALID;
}

static short getItemShortProperty( P_ITEM pi, int property, int prop2)
{
	switch( property )
	{
		CHECK(NXW_IP_S_AMOUNT, pi->amount )   //dec value :  400;
		CHECK(NXW_IP_S_AMOUNT2, pi->amount2 )   //dec value :  401;
		CHECK(NXW_IP_S_DIR, pi->dir )   //dec value :  402;
	}
	ErrOut("itm_getProperty called with invalid property %d!\n", property );
	return INVALID;
}

static char getItemCharProperty( P_ITEM pi, int property, int prop2)
{
	switch( property )
	{
		CHECK(NXW_IP_C_COLOR, (prop2>1) ? pi->color1 : pi->color2 )   		//dec value :  100;
		CHECK(NXW_IP_C_CORPSE, pi->corpse )   		//dec value :  102;
		CHECK(NXW_IP_C_DOORDIR, pi->doordir )   				//dec value :  103;
		CHECK(NXW_IP_C_DOOROPEN, pi->dooropen )   				//dec value :  104;
		CHECK(NXW_IP_C_DYE, pi->dye )   					//dec value :  105;
		CHECK(NXW_IP_C_ID, (prop2>1) ? pi->id1 : pi->id2 )   			//dec value :  109;
		CHECK(NXW_IP_C_LAYER, pi->layer )   					//dec value :  110;
		CHECK(NXW_IP_C_MAGIC, pi->magic )   					//dec value :  111;
		CHECK(NXW_IP_C_OFFSPELL, pi->offspell )   				//dec value :  114;
		CHECK(NXW_IP_C_OLDLAYER, pi->oldlayer )   				//dec value :  115;
		CHECK(NXW_IP_C_PILEABLE, pi->pileable )   				//dec value :  117;
		CHECK(NXW_IP_C_PRIV, pi->priv )   					//dec value :  118;
		CHECK(NXW_IP_C_VISIBLE, pi->visible )					//dec value :  120;
		case NXW_IP_C_CONTAINERSERIAL2:
			switch(prop2)
			{
				case 1: return pi->getContSerialByte(1);
				case 2: return pi->getContSerialByte(2);
				case 3: return pi->getContSerialByte(3);
				case 4: return pi->getContSerialByte(4);
			}
		case NXW_IP_C_MORE:
			switch(prop2) {
				case 1: return pi->more1;
				case 2: return pi->more2;
				case 3: return pi->more3;
				case 4: return pi->more4;
			}
		case NXW_IP_C_MOREB:
			switch(prop2) {
				case 1: return pi->moreb1;
				case 2: return pi->moreb2;
				case 3: return pi->moreb3;
				case 4: return pi->moreb4;
			}
		case NXW_IP_C_OWNERSERIAL2:
			switch(prop2) {
				case 1: return pi->getOwnerSerial().ser1;
				case 2: return pi->getOwnerSerial().ser2;
				case 3: return pi->getOwnerSerial().ser3;
				case 4: return pi->getOwnerSerial().ser4;
			}
		case NXW_IP_C_SERIAL2:
			switch(prop2) {
				case 1: return pi->getSerial().ser1;
				case 2: return pi->getSerial().ser2;
				case 3: return pi->getSerial().ser3;
				case 4: return pi->getSerial().ser4;
			}
		CHECK(NXW_IP_C_DAMAGETYPE, pi->damagetype)				//dec value :  121;
		CHECK(NXW_IP_C_AUXDAMAGETYPE, pi->auxdamagetype)			//dec value :  122;
	}
	ErrOut("itm_getProperty called with invalid property %d!\n", property );
	return '\0';
}

static const char* getItemStrProperty( P_ITEM pi, int property, int prop2)
{
	switch( property )
	{
		CHECK(NXW_IP_STR_CREATOR, pi->creator.c_str() )   //dec value :  450;
		CHECK(NXW_IP_STR_DESCRIPTION, pi->vendorDescription.c_str() )   //dec value :  451;
		CHECK(NXW_IP_STR_DISABLEDMSG, pi->disabledmsg.c_str() )   //dec value :  452;
		CHECK(NXW_IP_STR_MURDERER, pi->murderer.c_str() )   //dec value :  453;
		CHECK(NXW_IP_STR_NAME, pi->getCurrentNameC() )   //dec value :  454;
		CHECK(NXW_IP_STR_NAME2, pi->getSecondaryNameC() )   //dec value :  455;
	}
	ErrOut("itm_getProperty called with invalid property %d!\n", property );
	return const_cast<char*>(emptyString);
}

static bool getCharBoolProperty( P_CHAR pc, int property, int prop2 )
{
	switch( property )
	{
		CHECK(  NXW_CP_B_CANTRAIN , pc->canTrain() )			//dec value: 0;
		CHECK(  NXW_CP_B_DEAD , pc->dead )  				//dec value: 1;
		CHECK(  NXW_CP_B_GUARDED , pc->guarded )  			//dec value: 3;
		CHECK(  NXW_CP_B_GUILDTRAITOR , pc->IsGuildTraitor() )  	//dec value: 4;
		CHECK(  NXW_CP_B_INCOGNITO , pc->incognito )  			//dec value: 6;
		CHECK(  NXW_CP_B_ONHORSE , pc->isMounting() )  			//dec value: 7;
		CHECK(  NXW_CP_B_POLYMORPH , pc->polymorph)  			//dec value: 8;
		CHECK(  NXW_CP_B_TAMED , pc->tamed )  				//dec value: 9;
		CHECK(  NXW_CP_B_UNICODE , pc->unicode )  			//dec value: 10;
		CHECK(  NXW_CP_C_SHOPKEEPER , pc->shopkeeper ) 			//dec value: 11;
		CHECK(  NXW_CP_B_ATTACKFIRST , pc->HasAttackedFirst() )  	//dec value: 12;
		CHECK(  NXW_CP_B_ISBEINGTRAINED, pc->isBeingTrained() )  	//dec value: 13;
		CHECK(  NXW_CP_B_GUILDTOGGLE , pc->HasGuildTitleToggle() )  	//dec value: 14;
		CHECK(  NXW_CP_B_OVERWEIGHTED, pc->IsOverWeight() )     	//dec value: 15;
		CHECK(  NXW_CP_B_MOUNTED, pc->mounted )     	//dec value: 15;
	}
	ErrOut("chr_getProperty called with invalid property %d!\n", property );
	return false;
}

static int getCharIntProperty( P_CHAR pc, int property, int prop2, int prop3 )
{
	switch( property )
	{
		CHECK(  NXW_CP_I_ACCOUNT , pc->account )  			//dec value: 200;
		CHECK(  NXW_CP_I_ADVOBJ , pc->advobj )  			//dec value: 201;
		CHECK(  NXW_CP_I_ATT , pc->att )  				//dec value: 202;
		CHECK(  NXW_CP_I_ATTACKER , pc->attackerserial )  			//dec value: 203;
		//obsolete CHECK(  NXW_CP_I_BEARDCOLOR , (prop2>1) ? &chars[i].beardcolor1 : &chars[i].beardcolor2 )  //dec value: 204;
		CHECK(  NXW_CP_I_BEARDSERIAL , pc->beardserial )  		//dec value: 205;
		//obsolete CHECK(  NXW_CP_I_BEARDSTYLE , (prop2>1) ? &chars[i].beardstyle1 : &chars[i].beardstyle2 )  //dec value: 206;
		CHECK(  NXW_CP_I_CALLNUM , pc->callnum )  			//dec value: 207;
		CHECK(  NXW_CP_I_CARVE , pc->carve )  				//dec value: 208;
		CHECK(  NXW_CP_I_CASTING , pc->casting )  			//dec value: 209;
		CHECK(  NXW_CP_I_CLIENTIDLETIME , pc->clientidletime )  	//dec value: 210;
		CHECK(  NXW_CP_I_COMBATHITMESSAGE , pc->combathitmessage )  	//dec value: 211;
		CHECK(  NXW_CP_I_CREATIONDAY , pc->GetCreationDay() )  		//dec value: 212;
		CHECK(  NXW_CP_I_CRIMINALFLAG , pc->crimflag )  		//dec value: 213;
		CHECK(  NXW_CP_I_DEATHS , pc->deaths )  			//dec value: 214;
		CHECK(  NXW_CP_I_DEF , pc->def )  				//dec value: 215;
		case NXW_CP_I_DEXTERITY:					//dec value: 216
			switch(prop2) {
				case NXW_CP2_DEC: return pc->dx2;
				case NXW_CP2_REAL: return pc->dx3;
				case NXW_CP2_ACT: return pc->stm;
				case NXW_CP2_EFF:
				default : return pc->dx;
			}
		CHECK(  NXW_CP_I_DISABLED , pc->disabled )  			//dec value: 217;
		CHECK(  NXW_CP_I_FAME , pc->GetFame() )  				//dec value: 218;
		CHECK(  NXW_CP_I_FLEEAT , pc->fleeat )  			//dec value: 219;
		case NXW_CP_I_FOODPOSITION:					//dec value: 220;
			switch(prop2) {
				case NXW_CP2_X: return pc->foodloc.x;
				case NXW_CP2_Y: return pc->foodloc.y;
				case NXW_CP2_Z: return pc->foodloc.z;
				default : return pc->foodloc.z;
			}
		case NXW_CP_I_FPOS1_NPCWANDER:					//dec value: 221;
			switch(prop2) {
				case NXW_CP2_X: return pc->fx1;
				case NXW_CP2_Y: return pc->fy1;
				case NXW_CP2_Z: return pc->fz1;
				default : return pc->fz1;
			}
		case NXW_CP_I_FPOS2_NPCWANDER:					//dec value 222;
			switch(prop2) {
				case NXW_CP2_X: return pc->fx2;
				case NXW_CP2_Y: return pc->fy2;
				case NXW_CP2_Z: return pc->fz1;
				default : return pc->fz1;
			}
		CHECK(  NXW_CP_I_FTARG , pc->ftargserial )  				//dec value: 223;
		CHECK(  NXW_CP_I_GMMOVEEFF , pc->gmMoveEff )  			//dec value: 224;
		CHECK(  NXW_CP_I_GUILDFEALTY , pc->GetGuildFealty() )  		//dec value: 225;
		CHECK(  NXW_CP_I_GUILDNUMBER , pc->GetGuildNumber() )  		//dec value: 226;
		//obsolete CHECK(  NXW_CP_I_HAIRCOLOR , (prop2>1) ? &chars[i].haircolor1 : &chars[i].haircolor2 )  //dec value: 228;
		CHECK(  NXW_CP_I_HAIRSERIAL , pc->hairserial )  		//dec value: 229;
		//obsolete CHECK(  NXW_CP_I_HAIRSTYLE , (prop2>1) ? &chars[i].hairstyle1 : &chars[i].hairstyle2 )  //dec value: 230;
		CHECK(  NXW_CP_I_HIDAMAGE , pc->hidamage )  			//dec value: 231;
		CHECK(  NXW_CP_I_HOLDGOLD , pc->holdg )  			//dec value: 232;
		case NXW_CP_I_HOMELOCPOS:					//dec value: 233
			switch(prop2) {
				case NXW_CP2_X: return pc->homeloc.x;
				case NXW_CP2_Y: return pc->homeloc.y;
				case NXW_CP2_Z: return pc->homeloc.z;
				default : return pc->homeloc.z;
			}
		CHECK(  NXW_CP_I_HUNGER , pc->hunger )  			//dec value: 234;
		CHECK(  NXW_CP_I_HUNGERTIME , pc->hungertime )  		//dec value: 235;
		case NXW_CP_I_INTELLIGENCE:					//dec value: 236;
			switch(prop2) {
				case NXW_CP2_DEC: return pc->in2;
				case NXW_CP2_REAL: return pc->in3;
				case NXW_CP2_ACT: return pc->mn;
				case NXW_CP2_EFF:
				default : return pc->in;
			}
		CHECK(  NXW_CP_I_KARMA , pc->GetKarma() )  			//dec value: 237;
		CHECK(  NXW_CP_I_KEYNUMBER , pc->keyserial )  			//dec value: 238;
		CHECK(  NXW_CP_I_KILLS , pc->kills )  				//dec value: 239;
		CHECK(  NXW_CP_I_LODAMAGE , pc->lodamage )  			//dec value: 240;
		CHECK(  NXW_CP_I_LOGOUT , pc->logout )  			//dec value: 241;
		CHECK(  NXW_CP_I_MAKING , pc->making )  			//dec value: 242;
		CHECK(  NXW_CP_I_MEDITATING , pc->med )  			//dec value: 243;
		CHECK(  NXW_CP_I_MENUPRIV , pc->menupriv )  			//dec value: 244;
		CHECK(  NXW_CP_I_MULTISERIAL , pc->getMultiSerial32() )  	//dec value: 245;
		CHECK(  NXW_CP_I_MURDERERSER , pc->murdererSer )  		//dec value: 246;
		CHECK(  NXW_CP_I_MURDERRATE , pc->murderrate )  		//dec value: 247;
		CHECK(  NXW_CP_I_MUTETIME , pc->mutetime )  			//dec value: 248;
		CHECK(  NXW_CP_I_NAMEDEED , pc->namedeedserial )  			//dec value: 249;
		CHECK(  NXW_CP_I_NEXTACT , pc->nextact )  			//dec value: 250;
		CHECK(  NXW_CP_I_NPCAI , pc->npcaitype )  			//dec value: 251;
		CHECK(  NXW_CP_I_NPCMOVETIME , pc->npcmovetime )  		//dec value: 252;
		CHECK(  NXW_CP_I_OBJECTDELAY , pc->objectdelay )  		//dec value: 253;
		case NXW_CP_I_OLDPOS:						//dec value: 254;
			switch(prop2) {
				case NXW_CP2_X: return pc->getOldPosition().x;
				case NXW_CP2_Y: return pc->getOldPosition().y;
				case NXW_CP2_Z: return pc->getOldPosition().z;
				default : return pc->getOldPosition().z;
			}
		CHECK(  NXW_CP_I_OWNSERIAL , pc->getOwnerSerial32() )  		//dec value: 255;
		CHECK(  NXW_CP_I_PACKITEM , pc->packitemserial )  			//dec value: 256;
		CHECK(  NXW_CP_I_POISON , pc->poison )  			//dec value: 257;
		CHECK(  NXW_CP_I_POISONED , pc->poisoned )  			//dec value: 258;
		CHECK(  NXW_CP_I_POISONTIME , pc->poisontime )  		//dec value: 259;
		CHECK(  NXW_CP_I_POISONTXT , pc->poisontxt )  			//dec value: 260;
		CHECK(  NXW_CP_I_POISONWEAROFFTIME , pc->poisonwearofftime )  	//dec value: 261;
		case NXW_CP_I_POSITION:						//dec value: 262;
			switch(prop2) {
				case NXW_CP2_X: return pc->getPosition().x;
				case NXW_CP2_Y: return pc->getPosition().y;
				case NXW_CP2_Z: return pc->getPosition().z;
				default : return pc->getPosition().dispz;
			}
		CHECK(  NXW_CP_I_POSTTYPE , pc->postType )  			//dec value: 263;
		case NXW_CP_I_PREVPOS:						//dec value: 264
			switch(prop2) {
				case NXW_CP2_X: return pc->prevX;
				case NXW_CP2_Y: return pc->prevY;
				case NXW_CP2_Z: return pc->prevZ;
				default : return pc->prevZ;
			}
		CHECK(  NXW_CP_I_PRIV3 , pc->priv3[prop2] )  			//dec value: 265;
		CHECK(  NXW_CP_I_QUESTBOUNTYPOSTSERIAL , pc->questBountyPostSerial )  //dec value: 266;
		CHECK(  NXW_CP_I_QUESTBOUNTYREWARD , pc->questBountyReward )  	//dec value: 267;
		CHECK(  NXW_CP_I_QUESTDESTREGION , pc->questDestRegion )  	//dec value: 268;
		CHECK(  NXW_CP_I_QUESTORIGREGION , pc->questOrigRegion )  	//dec value: 269;
		CHECK(  NXW_CP_I_REATTACKAT , pc->reattackat )  		//dec value: 270;
		CHECK(  NXW_CP_I_REGENRATE, pc->getRegenRate( static_cast<StatType>(prop2), static_cast<VarType>(prop3) ); )
		CHECK(  NXW_CP_I_SCRIPTID , pc->getScriptID() )  				//dec value: 272;
		CHECK(  NXW_CP_I_ROBE , pc->robe )  				//dec value: 274;
		CHECK(  NXW_CP_I_RUNNING , pc->running )  			//dec value: 275;
		CHECK(  NXW_CP_I_SERIAL , pc->getSerial32() )  			//dec value: 276;
		CHECK(  NXW_CP_I_SKILLDELAY , pc->skilldelay )  		//dec value: 277;
		CHECK(  NXW_CP_I_SMELTITEM , pc->smeltserial )  			//dec value: 278;
		CHECK(  NXW_CP_I_SMOKEDISPLAYTIME , pc->smokedisplaytimer )  	//dec value: 279;
		CHECK(  NXW_CP_I_SMOKETIMER , pc->smoketimer )  		//dec value: 280;
		CHECK(  NXW_CP_I_SPADELAY , pc->spadelay )  			//dec value: 281;
		CHECK(  NXW_CP_I_SPATIMER , pc->spatimer )  			//dec value: 282;
		CHECK(  NXW_CP_I_SPATTACK , pc->spattack )  			//dec value: 283;
		CHECK(  NXW_CP_I_SPAWNREGION , pc->spawnregion )  		//dec value: 284;
		CHECK(  NXW_CP_I_SPAWNSERIAL , pc->spawnserial )  		//dec value: 285;
		CHECK(  NXW_CP_I_SPELL , pc->spell )  				//dec value: 286;
		CHECK(  NXW_CP_I_SPELLACTION , pc->spellaction )  		//dec value: 287;
		CHECK(  NXW_CP_I_SPELLTIME , pc->spelltime )  			//dec value: 288;
		CHECK(  NXW_CP_I_SPLIT , pc->split )  				//dec value: 290;
		CHECK(  NXW_CP_I_SPLITCHNC , pc->splitchnc )  			//dec value: 291;
		CHECK(  NXW_CP_I_SQUELCHED , pc->squelched )  			//dec value: 292;
		CHECK(  NXW_CP_I_STABLEMASTER_SERIAL , pc->getStablemaster() )  //dec value: 293;
		CHECK(  NXW_CP_I_STEALTH , pc->stealth )  			//dec value: 294;
		case NXW_CP_I_STRENGHT:						//dec value: 295;
			switch(prop2) {
				case NXW_CP2_DEC: return pc->st2;
				case NXW_CP2_REAL: return pc->st3;
				case NXW_CP2_ACT: return pc->hp;
				case NXW_CP2_EFF:
				default : return pc->getStrength();
			}
		CHECK(  NXW_CP_I_SUMMONTIMER , pc->summontimer )  		//dec value: 296;
		CHECK(  NXW_CP_I_SWINGTARG , pc->swingtargserial )  			//dec value: 297;
		CHECK(  NXW_CP_I_TAILITEM , pc->tailserial )  			//dec value: 298;
		CHECK(  NXW_CP_I_TAMING , pc->taming )  			//dec value: 299;
		CHECK(  NXW_CP_I_TARG , pc->targserial )  				//dec value: 300;
		CHECK(  NXW_CP_I_TARGTRIG , pc->targtrig )  			//dec value: 301;
		CHECK(  NXW_CP_I_TEMPFLAGTIME , pc->tempflagtime )  		//dec value: 302;
		CHECK(  NXW_CP_I_TIME_UNUSED , pc->time_unused )  		//dec value: 303;
		CHECK(  NXW_CP_I_TIMEOUT , pc->timeout )  			//dec value: 304;
		CHECK(  NXW_CP_I_TIMEUSED_LAST , pc->timeused_last )  		//dec value: 305;
		CHECK(  NXW_CP_I_TRACKINGDISPLAYTIMER , pc->trackingdisplaytimer )  //dec value: 306;
		CHECK(  NXW_CP_I_TRACKINGTARGET , pc->trackingtarget_serial )  	//dec value: 307;
		CHECK(  NXW_CP_I_TRACKINGTIMER , pc->trackingtimer )  		//dec value: 308;
		CHECK(  NXW_CP_I_TRAINER , pc->getTrainer() )  			//dec value: 309;
		CHECK(  NXW_CP_I_TRIGGER , pc->trigger )  			//dec value: 311;
		CHECK(  NXW_CP_I_WEIGHT , pc->weight )  			//dec value: 312;
		case NXW_CP_I_WORKLOCPOS:					//dec value: 313;
			switch(prop2) {
				case NXW_CP2_X: return pc->workloc.x;
				case NXW_CP2_Y: return pc->workloc.y;
				case NXW_CP2_Z: return pc->workloc.z;
				default : return pc->workloc.z;
			}
		// CHECK(  NXW_CP_I_AMXFLAGS, pc->amxflags[prop2])			//dec value: 314;
		case NXW_CP_I_AMXFLAGS :
				//
				// AMXFLAGS ARE NOW HANDLED AS NEW STYLE AMX VARS
				//
				if ( prop2 >= 0 && prop2 <= 15 )
				{
					int value;
					amxVS.selectVariable( pc->getSerial32(), prop2, value );
					return value;
				}
				return 0;
		CHECK(  NXW_CP_I_RACE, pc->race ) 				//dec value: 315
		CHECK(  NXW_CP_I_CX, pc->getPosition().x ) 			//dec value: 316
		CHECK(  NXW_CP_I_CY, pc->getPosition().y ) 			//dec value: 317
		CHECK(  NXW_CP_I_CZ, pc->getPosition().z ) 			//dec value: 318
		CHECK(  NXW_CP_I_LASTMOVETIME, pc->LastMoveTime)		//dec value: 319

	}
	ErrOut("chr_getProperty called with invalid property %d!\n", property );
	return -1;
}


static short getCharShortProperty( P_CHAR pc, int property, int prop2 )
{
	switch( property )
	{
		CHECK(  NXW_CP_S_BASESKILL , pc->baseskill[prop2] )  		//dec value: 400;
		CHECK(  NXW_CP_S_SKILL , pc->skill[prop2] )  			//dec value: 401;
		CHECK(  NXW_CP_S_GUILDTYPE , pc->GetGuildType() )  		//dec value: 402;
	}
	ErrOut("chr_getProperty called with invalid property %d!\n", property );
	return -1;
}

static char getCharCharProperty( P_CHAR pc, int property, int prop2 )
{
	switch( property )
	{
		CHECK(  NXW_CP_C_BLOCKED , pc->blocked )  			//dec value: 101;
		CHECK(  NXW_CP_C_COMMANDLEVEL , pc->commandLevel )  		//dec value: 103;
		CHECK(  NXW_CP_C_DIR , pc->dir )  				//dec value: 104;
		CHECK(  NXW_CP_C_DIR2 , pc->dir2 )  				//dec value: 105;
		CHECK(  NXW_CP_C_FIXEDLIGHT , pc->fixedlight )  		//dec value: 106;
		CHECK(  NXW_CP_C_FLAG , pc->flag )  				//dec value: 107;
		CHECK(  NXW_CP_C_FLY_STEPS , pc->fly_steps )  			//dec value: 108;
		CHECK(  NXW_CP_C_GMRESTRICT , pc->gmrestrict )  		//dec value: 109;
		CHECK(  NXW_CP_C_HIDDEN , pc->hidden )  			//dec value: 110;
		CHECK(  NXW_CP_C_ID , (prop2>1) ? (pc->GetBodyType()>>8) : (pc->GetBodyType()%256) )  		//dec value: 111;
		CHECK(  NXW_CP_C_LOCKSKILL , pc->lockSkill[prop2] )  		//dec value: 112;
		case NXW_CP_C_MULTISERIAL2 :					//dec value: 113;
			switch(prop2) {
				case 1:		return pc->getMultiSerial().ser1;
				case 2:		return pc->getMultiSerial().ser2;
				case 3:		return pc->getMultiSerial().ser3;
				default :	return pc->getMultiSerial().ser4;
			}
		CHECK(  NXW_CP_C_NPC , pc->npc )  				//dec value: 114;
		CHECK(  NXW_CP_C_NPCTYPE , pc->npc_type )  			//dec value: 115;
		CHECK(  NXW_CP_C_NPCWANDER , pc->npcWander )  			//dec value: 116;
		CHECK(  NXW_CP_C_OLDNPCWANDER , pc->oldnpcWander )  		//dec value: 117;
		//CHECK(  NXW_CP_C_ORGSKIN , (prop2>1) ? &chars[i].orgskin1 : &chars[i].orgskin2 )  //dec value: 118;
		case NXW_CP_C_OWNSERIAL2 :					//dec value: 119;
			switch(prop2) {
				case 1: 	return pc->getOwnerSerial().ser1;
				case 2: 	return pc->getOwnerSerial().ser2;
				case 3: 	return pc->getOwnerSerial().ser3;
				default : 	return pc->getOwnerSerial().ser4;
			}
		CHECK(  NXW_CP_C_PRIV2 , pc->GetPriv2() )  			//dec value: 121;
		CHECK(  NXW_CP_C_REACTIVEARMORED , pc->ra )  			//dec value: 122;
		CHECK(  NXW_CP_C_REGION , pc->region )  			//dec value: 123;
		case NXW_CP_C_SERIAL2 :
			switch(prop2) {
				case 1:		return pc->getSerial().ser1;
				case 2:		return pc->getSerial().ser2;
				case 3:		return pc->getSerial().ser3;
				default :	return pc->getSerial().ser4;
			}
		CHECK(  NXW_CP_C_SHOP , pc->shopkeeper )  			//dec value: 125; Sparhawk: DEPRECIATED, use CHECK(  NXW_CP_B_SHOPKEEPER , pc->shopkeeper )
		CHECK(  NXW_CP_C_SKIN , (prop2>1) ? (pc->getSkinColor()>>8) : (pc->getSkinColor()%256) )  	//dec value: 126;
		CHECK(  NXW_CP_C_SPEECH , pc->speech )  			//dec value: 127;
		CHECK(  NXW_CP_C_WAR , pc->war )  				//dec value: 128;
		CHECK(  NXW_CP_C_XID , (prop2>1) ? (pc->GetOldBodyType()>>8) : (pc->GetOldBodyType()%256) )  	//dec value: 129;
		CHECK(  NXW_CP_C_XSKIN , (prop2>1) ? (pc->getOldSkinColor()>>8) : (pc->getOldSkinColor()%256) )  //dec value: 130;
		CHECK(  NXW_CP_C_NXWFLAGS, pc->nxwflags[prop2])			//dec value: 131;
		CHECK(  NXW_CP_I_RESISTS, pc->resists[prop2])			//dec value: 132;
		CHECK(  NXW_CP_C_TRAININGPLAYERIN , pc->getSkillTaught() )  	//dec value: 133;
		CHECK(  NXW_CP_C_PRIV , pc->GetPriv() )  			//dec value: 134;
		CHECK(  NXW_CP_C_DAMAGETYPE, pc->damagetype)			//dec value: 135;
	}
	ErrOut("chr_getProperty called with invalid property %d!\n", property );
	return '\0';
}

static char* getCharStrProperty( P_CHAR pc, int property, int prop2 )
{
	switch( property )
	{
		CHECK(  NXW_CP_STR_DISABLEDMSG , pc->disabledmsg )  		//dec value: 450;
		CHECK(  NXW_CP_STR_GUILDTITLE , pc->GetGuildTitle() )  		//dec value: 451;
		CHECK(  NXW_CP_STR_LASTON , "<obsolete>" )  			//dec value: 452;
		CHECK(  NXW_CP_STR_NAME, const_cast<char *>(pc->getCurrentNameC()) )  //dec value: 453;
		CHECK(  NXW_CP_STR_ORGNAME , "<obsolete>" )  			//dec value: 454;
		CHECK(  NXW_CP_STR_TITLE , pc->title )  			//dec value: 455;
		CHECK(  NXW_CP_STR_TRIGWORD , pc->trigword )  			//dec value: 456;
		CHECK(	NXW_CP_STR_SPEECHWORD, script1 ) 			//dec value: 457;
		CHECK(	NXW_CP_STR_SPEECH, script2 ) 				//dec value: 458;
	}
	ErrOut("chr_getProperty called with invalid property %d!\n", property );
	return const_cast<char*>(emptyString);
}

static wstring* getCharUniProperty( P_CHAR pc, int property, int prop2 )
{
	switch( property )
	{
		CHECK(  NXW_CP_UNI_SPEECH_CURRENT , pc->getSpeechCurrent() )
		CHECK(  NXW_CP_UNI_PROFILE , pc->getProfile() )
	}
	ErrOut("chr_getProperty called with invalid property %d!\n", property );
	return NULL;
}


/*
 Function	:	_getCalPropertyPtr
 Purpose	:	return addres to calendar property value
 Inputparms	:	i		-	never used for compliance to _getXXXPropertyPtr parameter model
			property
			prop2		-	optional month or weekday number
 Outputparms:	none
 Returnvalue:	pointer to value of calendar property
 Creator	:	Sparhawk
 Datecreated:	2001-09-15
 Nxw version:	054b
 Version	:	1.0
 Dateupdated:	2001-09-15
 Notes	:
 History	:
*/
static void *getCalPropertyPtr(int i, int property, int prop2)
{
	switch(property)
	{
		CHECK( NXW_CALP_I_YEAR	   , 	&Calendar::g_nYear )
		CHECK( NXW_CALP_I_MONTH	   , 	&Calendar::g_nMonth )
		CHECK( NXW_CALP_I_MAXMONTH , 	&Calendar::g_nMaxMonth )
		CHECK( NXW_CALP_I_DAY	   , 	&Calendar::g_nDay	)
		CHECK( NXW_CALP_I_WEEKDAY  , 	&Calendar::g_nWeekday )
		CHECK( NXW_CALP_I_MAXWEEKDAY,	&Calendar::g_nMaxWeekday )
		CHECK( NXW_CALP_I_HOUR	   , 	&Calendar::g_nHour	)
		CHECK( NXW_CALP_I_MINUTE   , 	&Calendar::g_nMinute )
		CHECK( NXW_CALP_I_DAWNHOUR , 	(prop2 > Calendar::g_nMaxMonth) ? &Calendar::g_Months[Calendar::g_nMaxMonth].m_nDawnHour :
					     	(prop2 < 0) ? &Calendar::g_Months[1].m_nDawnHour :
					     	(prop2 == 0) ? &Calendar::g_Months[Calendar::g_nMonth].m_nDawnHour :
					     	&Calendar::g_Months[prop2].m_nDawnHour )
		CHECK( NXW_CALP_I_DAWNMINUTE ,	(prop2 > Calendar::g_nMaxMonth) ? &Calendar::g_Months[Calendar::g_nMaxMonth].m_nDawnMin :
						(prop2 < 0) ? &Calendar::g_Months[1].m_nDawnMin :
						(prop2 == 0) ? &Calendar::g_Months[Calendar::g_nMonth].m_nDawnMin :
						&Calendar::g_Months[prop2].m_nDawnMin )
		CHECK( NXW_CALP_I_SUNSETHOUR , 	(prop2 > Calendar::g_nMaxMonth) ? &Calendar::g_Months[Calendar::g_nMaxMonth].m_nSunsetHour :
					     	(prop2 < 0) ? &Calendar::g_Months[1].m_nSunsetHour :
					     	(prop2 == 0) ? &Calendar::g_Months[Calendar::g_nMonth].m_nSunsetHour :
						&Calendar::g_Months[prop2].m_nSunsetHour )
		CHECK( NXW_CALP_I_SUNSETMINUTE,	(prop2 > Calendar::g_nMaxMonth) ? &Calendar::g_Months[Calendar::g_nMaxMonth].m_nSunsetMin :
						(prop2 < 0) ? &Calendar::g_Months[1].m_nSunsetMin :
						(prop2 == 0) ? &Calendar::g_Months[Calendar::g_nMonth].m_nSunsetMin :
						&Calendar::g_Months[prop2].m_nSunsetMin )
		CHECK( NXW_CALP_I_SEASON,	(prop2 > Calendar::g_nMaxMonth) ? &Calendar::g_Months[Calendar::g_nMaxMonth].m_nSeason :
						(prop2 < 0) ? &Calendar::g_Months[1].m_nSeason :
						(prop2 == 0) ? &Calendar::g_Months[Calendar::g_nMonth].m_nSeason :
						&Calendar::g_Months[prop2].m_nSeason )
		CHECK( NXW_CALP_STR_MONTHNAME,	(prop2 > Calendar::g_nMaxMonth) ? Calendar::g_Months[Calendar::g_nMaxMonth].m_strName :
						(prop2 < 0) ? Calendar::g_Months[1].m_strName :
						(prop2 == 0) ? Calendar::g_Months[Calendar::g_nMonth].m_strName :
						Calendar::g_Months[prop2].m_strName )
		CHECK( NXW_CALP_STR_WEEKDAYNAME,(prop2 > Calendar::g_nMaxWeekday) ? Calendar::g_Weekday[Calendar::g_nMaxWeekday].m_strName :
						(prop2 < 0) ? Calendar::g_Weekday[1].m_strName :
						(prop2 == 0) ? Calendar::g_Weekday[Calendar::g_nMaxWeekday].m_strName :
						Calendar::g_Weekday[prop2].m_strName )
		default:
			ErrOut("Script ERROR in Get cal property!\n");
			return emptyString;
	}
}


NATIVE2(_setGuildProperty)
{
	// params[1] = chr
	// params[2] = property
	// params[3] = subproperty
	// params[4] = value to set property to

	SERIAL guild = params[1];
	if( !Guildz.find( guild ) )
	{
		LogError( "guild_setProperty called with invalid guild %d", guild );
		return INVALID;
	}
	//
	// Fetch a copy of the guild record from the database
	//
	cGuild *pGuild = Guildz.fetch();
	P_CHAR pc;

	int tp = getPropertyType(params[2]);

	cell *cptr;
	amx_GetAddr(amx,params[4],&cptr);


	if (tp==T_INT) {
		int p = *cptr;

		switch( params[2] )
		{
			case NXW_GP_I_GUILDMASTER :	  				//dec value: 200;
				pc = pointers::findCharBySerial(p);
				if( ISVALIDPC(pc) )
				{
					//
					// Add new guildmaster as guild member
					//
					if( !pGuild->isMember( pc ) )
					{
						if( !pGuild->addMember( pc ) )
							p = INVALID;
					}
					if( p != INVALID )
					{
						//
						// Replace the old gamemaster
						//
						P_CHAR pOldGuildMaster = pointers::findCharBySerial( pGuild->getGuildMaster() );
						pGuild->setGuildMaster( pc );
						//
						// Commit the changes
						//
						if( Guildz.update( pGuild ) )
						{
							//
							// Notify the old guildmaster and log the replacement
							//
							if( ISVALIDPC( pOldGuildMaster ) )
							{
								LogWarning("Amxscript replaced guildmaster %d by character %d\n", pOldGuildMaster->getSerial32(), pc->getSerial32() );
								if( pOldGuildMaster->IsOnline() )
								{
									NXWSOCKET socket = calcSocketFromChar( DEREF_P_CHAR( pOldGuildMaster ) );
									if( socket != INVALID )
										sysmessage( socket, "Amxscript has replaced you as master for guild %s by %s", pGuild->getName().c_str(), pc->getCurrentNameC() );
								}
							}
						}
						else
							p = INVALID;
					}
				}
				else
				{
					LogError( "guild_setProperty called with invalid char %d", p );
					p = INVALID;
				}
				break;
			default :
				ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
				break;
		}

		return p;
	}
	if (tp==T_BOOL)
	{
		bool p = *cptr ? true : false;

		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	if (tp==T_SHORT) {
		short p = static_cast<short>(*cptr & 0xFFFF);
		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	if (tp==T_CHAR) {
		char p = static_cast<char>(*cptr & 0xFF);

		switch( params[2] )
		{
			case  NXW_GP_C_TYPE :				  		//dec value: 100;
				if( pGuild->setType( p ) )
					Guildz.update( pGuild );
				else
					LogError("guild_setProperty called with invalid type %d!\n", p );
				break;
			default :
				ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	//we're here so we should get a ConOut format string, params[4] is the str format

	cell *cstr;
	amx_GetAddr(amx,params[4],&cstr);
	printstring(amx,cstr,params+5,(int)(params[0]/sizeof(cell))-1);
	g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';
	switch( params[2] )
	{
		case NXW_GP_STR_NAME :			  				//dec value: 450;
			pGuild->setName( g_cAmxPrintBuffer );
			//
			// Guild names have to be unique, so we have to account for a commit error
			//
			if( !Guildz.update( pGuild ) )
				LogError("guild_setProperty called with invalid guildname %s!\n", g_cAmxPrintBuffer );
			break;
		case NXW_GP_STR_WEBPAGE :		  				//dec value: 451;
			pGuild->setWebPage( g_cAmxPrintBuffer );
			Guildz.update( pGuild );
			break;
		case NXW_GP_STR_CHARTER :		  				//dec value: 452;
			pGuild->setCharter( g_cAmxPrintBuffer );
			Guildz.update( pGuild );
			break;
		case NXW_GP_STR_ABBREVIATION :	  				//dec value: 453;
			pGuild->setAbbreviation( g_cAmxPrintBuffer );
			Guildz.update( pGuild );
			break;
		default :
			ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
			break;
	}
	g_nAmxPrintPtr=0;
	return 0;
}

NATIVE2(_getGuildProperty)
{

	SERIAL guild = params[1];
	if( !Guildz.find( guild ) )
	{
		LogError( "guild_setProperty called with invalid guild %d", guild );
		return INVALID;
	}
	//
	// Fetch a copy of the guild record from the database
	//
	cGuild *pGuild = Guildz.fetch();

	int tp = getPropertyType(params[2]);

	if (tp==T_INT)
	{
		int p;
		switch(params[2]) {
			case NXW_GP_I_GUILDMASTER:
				p = pGuild->getGuildMaster();
				break;
			case NXW_GP_I_MEMBERCOUNT:
				p = pGuild->members.size();
				break;
			case NXW_GP_I_RECRUITCOUNT:
				p = pGuild->recruits.size();
				break;
			default:

				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		cell i = p;
		return i;
	}
	if (tp==T_BOOL)
	{
		bool p; 
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return false;
		}
		cell i = p;
		return i;
	}
	if (tp==T_SHORT)
	{
		short p;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		cell i = p;
		return i;
	}
	if (tp==T_CHAR) {
		char p; 
		switch(params[2]) {
			case NXW_GP_C_TYPE:
				p = pGuild->getType();
				break;
			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		cell i = p;
		return i;
	}

	//we're here so we should pass a string, params[4] is a str ptr

  	char str[100];
	cell *cptr;
	switch(params[2]) {
		case NXW_GP_STR_NAME:
		  	strcpy(str, pGuild->getName().c_str());
			break;
		case NXW_GP_STR_WEBPAGE:
		  	strcpy(str, pGuild->getWebPage().c_str());
			break;
		case NXW_GP_STR_CHARTER:
		  	strcpy(str, pGuild->getCharter().c_str());
			break;
		case NXW_GP_STR_ABBREVIATION:
		  	strcpy(str, pGuild->getAbbreviation().c_str());
			break;
		default:
			ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
			return INVALID;
	}
	amx_GetAddr(amx,params[4],&cptr);
  	amx_SetString(cptr,str, g_nStringMode);

	return strlen(str);
}
