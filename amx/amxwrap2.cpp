  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*!
 \brief Wrapper Native Functions for AMX abstract machine
 \author Xanathar

 This is one of the borest files I've ever written :D - Xan
 This file contains property stuff

 \date 12-09-2002 Luxor: changed to work with serials instead of old indexes
 */

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
#include "menu.h"
#include "guild.h"
#include "set.h"
#include "inlines.h"
#include "items.h"
#include "chars.h"
#include "race.h"
#include "layer.h"
#include "party.h"

static void *getCalPropertyPtr(int i, int property, int prop2); //Sparhawk

static char emptyString[1] = { '\0' };
static wstring emptyUnicodeString;

extern int g_nStringMode;

VAR_TYPE getPropertyType(int property)
{
	if (property < 100) return T_BOOL;
	if (property < 200) return T_CHAR;
	if (property < 400) return T_INT;
	if (property < 450) return T_SHORT;
	if (property < 500) return T_STRING;
	return T_UNICODE;
}

#define CHECK(A,B) case A: return B;
#define GETPWSTRING( P ) ( (P!=NULL)? *P : emptyUnicodeString )




////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/////////////////////////   ITEM PROPERTY   ////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


NATIVE2(_getItemProperty)
{
	P_ITEM pi = pointers::findItemBySerial(params[1]);

	if( ISVALIDPI( pi ) )
	{
		VAR_TYPE tp = getPropertyType(params[2]);

		switch( tp ) {
			case T_INT: {
				int p = getItemIntProperty( pi, params[2], params[3]);
				cell i = p;
				return i;
			}
			case T_BOOL: {
				bool p = getItemBoolProperty( pi, params[2], params[3]);
				cell i = p;
				return i;
			}
			case T_SHORT: {
				short p = getItemShortProperty( pi, params[2], params[3]);
				cell i = p;
				return i;
			}
			case T_CHAR: {
				char p = getItemCharProperty( pi, params[2], params[3]);
				cell i = p;
				return i;
			}
			case T_STRING: {

			  	char str[100];
	  			strcpy(str, getItemStrProperty( pi, params[2], params[3]));

  				cell *cptr;
  				amx_GetAddr(amx,params[4],&cptr);
	  			amx_SetString(cptr,str, g_nStringMode);

  				return strlen(str);

			}
			case T_UNICODE: {
				wstring& w=getItemUniProperty( pi, params[2], params[3] );

				cell *cptr;
	  			amx_GetAddr(amx,params[4],&cptr);
				amx_SetStringUnicode(cptr, w );

				return w.length();
			}
		}

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

	cell* cptr;
	amx_GetAddr(amx,params[4],&cptr);

	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {

		case T_INT: {
			int p = *cptr;
			setItemIntProperty( pi, params[2], params[3], p );
			return p;
		}
		case T_BOOL: {
			bool p = *cptr ? true : false;
			setItemBoolProperty( pi, params[2], params[3], p );
			return p;
		}
		case T_SHORT: {
			short p = static_cast<short>(*cptr & 0xFFFF);
			setItemShortProperty( pi, params[2], params[3], p );
			return p;
		}
		case T_CHAR: {
			char p = static_cast<char>(*cptr & 0xFF);
			setItemCharProperty( pi, params[2], params[3], p );
			return p;
		}
		case T_STRING: {
			//we're here so we should get a ConOut format string, params[4] is the str format

			printstring(amx,cptr,params+5,(int)(params[0]/sizeof(cell))-1);
			g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';


			setItemStrProperty( pi, params[2], params[3], g_cAmxPrintBuffer );

			g_nAmxPrintPtr=0;
			return 0;
		}
		case T_UNICODE: {

			std::wstring buffer;
			amx_GetStringUnicode( buffer, cptr );

			setItemUniProperty( pi, params[2], params[3], buffer );

			g_nAmxPrintPtr=0;
			return 0;
		}
		default:
			return 0;
	}
}

void setItemUniProperty( P_ITEM pi, int property, int prop2, std::wstring& value )
{
	switch( property )
	{
		case INVALID:
		default :
			ErrOut("itm_setProperty called with invalid property %d!\n", property );
			break;
	}
}




void setItemBoolProperty(P_ITEM pi, int property, int prop2, bool value )
{
	switch( property )
	{
		case NXW_IP_B_INCOGNITO :		   //dec value :  0;
			pi->incognito = value;
			break;
		default :
			ErrOut("itm_setProperty called with invalid property %d!\n", property );
			break;
	}
}

void setItemIntProperty(P_ITEM pi, int property, int prop2, int value )
{
	switch( property )
	{
		case NXW_IP_I_ATT :					   //dec value :  200;
			pi->att = value;
			break;
		case NXW_IP_I_CARVE :					   //dec value :  201;
			pi->carve = value;
			break;
		case NXW_IP_I_CONTAINERSERIAL :				   //dec value :  202;
			//pi->contserial = value;
			pi->setContSerial(value, false, false);
			break;
		case NXW_IP_I_DECAYTIME :				   //dec value :  203;
			pi->setDecayTime( (TIMERVAL) value );
			break;
		case NXW_IP_I_DEF :					   //dec value :  204;
			pi->def = value;
			break;
		case NXW_IP_I_DEXBONUS :				   //dec value :  205;
			pi->dx2 = value;
			break;
		case NXW_IP_I_DEXREQUIRED :				   //dec value :  206;
			pi->dx = value;
			break;
		case NXW_IP_I_DISABLED :				   //dec value :  207;
			pi->disabled = value;
			break;
		case NXW_IP_I_GATENUMBER :				   //dec value :  208;
			pi->gatenumber = value;
			break;
		case NXW_IP_I_GATETIME :				   //dec value :  209;
			pi->gatetime = value;
			break;
//		case NXW_IP_I_GLOW :					   //dec value :  210;
//			pi->glow = value;
//			break;
		case NXW_IP_I_GOOD :					 	//dec value :  211;
			pi->good = value;
			break;
		case NXW_IP_I_HIDAMAGE :				   //dec value :  212;
			pi->hidamage = value;
			break;
		case NXW_IP_I_HP :					   //dec value :  213;
			pi->hp = value;
			break;
		case NXW_IP_I_INTBONUS :				   //dec value :  214;
			pi->in2 = value;
			break;
		case NXW_IP_I_INTREQUIRED :				   //dec value :  215;
			pi->in = value;
			break;
		case NXW_IP_I_ITEMHAND :				   //dec value :  216;
			pi->itmhand = value;
			break;
		case NXW_IP_I_LODAMAGE :				   //dec value :  217;
			pi->lodamage = value;
			break;
		case NXW_IP_I_MADEWITH :				   //dec value :  218;
			pi->madewith = value;
			break;
		case NXW_IP_I_MAXHP :					   //dec value :  219;
			pi->maxhp = value;
			break;
		case NXW_IP_I_MOREPOSITION:				//dec value : 220;
			switch(prop2) {
				case NXW_CI2_X:
					pi->morex = value;
					break;
				case NXW_CI2_Y:
					pi->morey = value;
					break;
				default :
					pi->morez = value;
					break;
			} break;
		case NXW_IP_I_MULTISERIAL :				   //dec value :  221;
			pi->setMultiSerial32Only(value);
			break;
		case NXW_IP_I_MURDERTIME :				   //dec value :  222;
			pi->murdertime = value;
			break;
		case NXW_IP_I_OLDCONTAINERSERIAL :			   //dec value :  223;
			pi->setContSerial(value, true, false);
			break;
		case NXW_IP_I_OLDPOSITION:								//dec value : 224;
			switch(prop2) {
				case NXW_CI2_X:
					pi->setOldPosition("x", value);
					break;
				case NXW_CI2_Y:
					pi->setOldPosition("y", value);
					break;
				default :
					pi->setOldPosition("z", value);
					break;
			} break;
		case NXW_IP_I_OWNERSERIAL :				   //dec value :  225;
			pi->setOwnerSerial32Only(value);
			break;
		case NXW_IP_I_POISONED :				   //dec value :  226;
			pi->poisoned = (PoisonType)value;
			break;
		case NXW_IP_I_POSITION:							//dec value : 227;
			switch(prop2) {
				case NXW_CI2_X:
					pi->setPosition("x", value);
					break;
				case NXW_CI2_Y:
					pi->setPosition("y", value);
					break;
				default :
					pi->setPosition("z", value);
					break;
			} break;
		case NXW_IP_I_RANK :					   //dec value :  228;
			pi->rank = value;
			break;
		case NXW_IP_I_REQSKILL :				   //dec value :  229;
			pi->wpsk = value;
			break;
		case NXW_IP_I_RESTOCK :					   //dec value :  230;
			pi->restock = value;
			break;
		case NXW_IP_I_RNDVALUERATE :				   //dec value :  231;
			pi->rndvaluerate = value;
			break;
		case NXW_IP_I_SECUREIT :				   //dec value :  232;
			pi->secureIt = value;
			break;
		case NXW_IP_I_SERIAL :					   //dec value :  233;
			pi->setSerial32(value);
			break;
		case NXW_IP_I_SMELT :					   //dec value :  234;
			pi->smelt = value;
			break;
		case NXW_IP_I_SPAWNREGION :				   //dec value :  235;
			pi->spawnregion = value;
			break;
		case NXW_IP_I_SPAWNSERIAL :				   //dec value :  236;
			pi->spawnserial = value;
			break;
		case NXW_IP_I_SPEED :					   //dec value :  237;
			pi->spd = value;
			break;
		case NXW_IP_I_STRBONUS :				   //dec value :  238;
			pi->st2 = value;
			break;
		case NXW_IP_I_STRREQUIRED :				   //dec value :  239;
			pi->st = value;
			break;
		case NXW_IP_I_TIME_UNUSED :				   //dec value :  240;
			pi->time_unused = value;
			break;
		case NXW_IP_I_TIME_UNUSEDLAST :				   //dec value :  241;
			pi->timeused_last = value;
			break;
		case NXW_IP_I_TRIGGER :					   //dec value :  242;
			pi->trigger = value;
			break;
		case NXW_IP_I_TRIGGERUSES :				   //dec value :  243;
			pi->tuses = value;
			break;
		case NXW_IP_I_TRIGTYPE :				   //dec value :  244;
			pi->trigtype = value;
			break;
		case NXW_IP_I_TYPE :					   //dec value :  245;
			pi->type = value;
			break;
		case NXW_IP_I_TYPE2 :					   //dec value :  246;
			pi->type2 = value;
			break;
		case NXW_IP_I_VALUE :					   //dec value :  247;
			pi->value = value;
			break;
		case NXW_IP_I_WEIGHT :					   //dec value :  248;
			pi->weight = value;
			break;
		case NXW_IP_I_WIPE :					   //dec value :  249;
			pi->wipe = value;
			break;
		case NXW_IP_I_AMXFLAGS : 				  //dec value :  250;
			//
			// AMXFLAGS ARE NOW HANDLED AS NEW STYLE AMX VARS
			//
			//pi->amxflags[params[3]] = value;
			if ( prop2 >= 0 && prop2 <= 15 )
				amxVS.updateVariable( pi->getSerial32(), prop2, value );
			break;
		case NXW_IP_I_SCRIPTID :				//dec value 251;
			pi->setScriptID( value );
			break;
		case NXW_IP_I_ANIMID :					//dec value 252;
			pi->animSetId( value );
			break;
		case NXW_IP_I_RESISTS :					//dec value: 253;
			pi->resists[prop2] = value;
			break;
		case NXW_IP_I_AUXDAMAGE :				//dec value: 254;
			pi->auxdamage = value;
			break;
		case NXW_IP_I_AMMO :					//dec value: 255;
			pi->ammo = value;
			break;
		case NXW_IP_I_AMMOFX :					//dec value: 256;
			pi->ammoFx = value;
			break;
		default :
			ErrOut("itm_setProperty called with invalid property %d!\n", property );
			break;
	}
}

void setItemShortProperty(P_ITEM pi, int property, int prop2, short value )
{
	switch( property )
	{
		case NXW_IP_S_AMOUNT :			   //dec value :  400;
			pi->amount = value;
			break;
		case NXW_IP_S_AMOUNT2 :			   //dec value :  401;
			pi->amount2 = value;
			break;
		case NXW_IP_S_DIR :			   //dec value :  402;
			pi->dir = value;
			break;
		case NXW_IP_S_COLOR :
			pi->setColor( value );
			break;
		case NXW_IP_S_ID :
			pi->setId( value );
			break;
		default :
			ErrOut("itm_setProperty called with invalid property %d!\n", property );
			break;
	}
}

void setItemCharProperty(P_ITEM pi, int property, int prop2, char value )
{
	switch( property )
	{
		case NXW_IP_C_DOORDIR :			   				//dec value :  103;
			pi->doordir = value;
			break;
		case NXW_IP_C_DOOROPEN :		   				//dec value :  104;
			pi->dooropen = value;
			break;
		case NXW_IP_C_DYE :		   					//dec value :  105;
			pi->dye = value;
			break;
		case NXW_IP_C_FREE :							//dec value :  106;
			break;
//		case NXW_IP_C_GLOWFX : 							//dec value :  107;
//			pi->glow_effect = value;
//			break;
//		case NXW_IP_C_GLOWOLDCOLOR :					   	//dec value :  108;
//			if (prop2 > 1)
//				pi->glow_c1 = value;
//			else
//				pi->glow_c2 = value;
//			break;
		case NXW_IP_C_LAYER :		   					//dec value :  110;
			pi->layer = value;
			break;
		case NXW_IP_C_MAGIC :		   					//dec value :  111;
			pi->magic = value;
			break;
		case NXW_IP_C_OFFSPELL :		   				//dec value :  114;
			pi->offspell = value;
			break;
		case NXW_IP_C_OLDLAYER :		   				//dec value :  115;
			pi->oldlayer = value;
			break;
		case NXW_IP_C_PILEABLE :		   				//dec value :  117;
			pi->pileable = value;
			break;
		case NXW_IP_C_PRIV :		   					//dec value :  118;
			pi->priv = value;
			break;
		case NXW_IP_C_VISIBLE : 						//dec value :  120;
			pi->visible = value;
			break;
		case NXW_IP_C_MORE:
			switch(prop2) {
				case 1:
					pi->more1 = value;
					break;
				case 2:
					pi->more2 = value;
					break;
				case 3:
					pi->more3 = value;
					break;
				case 4:
					pi->more4 = value;
					break;
			}
			break;
		case NXW_IP_C_MOREB:
			switch(prop2) {
				case 1:
					pi->moreb1 = value;
					break;
				case 2:
					pi->moreb2 = value;
					break;
				case 3:
					pi->moreb3 = value;
					break;
				case 4:
					pi->moreb4 = value;
					break;
			}
			break;

		case NXW_IP_C_DAMAGETYPE :						//dec value: 121;
			pi->damagetype = static_cast<DamageType>(value);
			break;
		case NXW_IP_C_AUXDAMAGETYPE :						//dec value: 122;
			pi->auxdamagetype = static_cast<DamageType>(value);
			break;
		default :
			ErrOut("itm_setProperty called with invalid property %d!\n", property );
			break;
	}
}

void setItemStrProperty(P_ITEM pi, int property, int prop2, char* value )
{
	switch( property )
	{
		case NXW_IP_STR_CREATOR :				   //dec value :  450;
			pi->creator = value;
			break;
		case NXW_IP_STR_DESCRIPTION :				   //dec value :  451;
			//strcpy(pi->desc, g_cAmxPrintBuffer );
			pi->vendorDescription = value;
			break;
		case NXW_IP_STR_DISABLEDMSG :				   //dec value :  452;
			if( pi->disabledmsg==NULL )
				pi->disabledmsg = new std::string( value );
			else
				(*pi->disabledmsg) = value;
			break;
		case NXW_IP_STR_MURDERER :				   //dec value :  453;
			pi->murderer = string(value);
			break;
		case NXW_IP_STR_NAME :					   //dec value :  454;
			pi->setCurrentName(value);
			break;
		case NXW_IP_STR_NAME2 :					   //dec value :  455;
			pi->setSecondaryName(value);
			break;
		default :
			ErrOut("itm_setProperty called with invalid property %d!\n", property );
			break;
	}
}


wstring& getItemUniProperty( P_ITEM pi, int property, int prop2 )
{
	switch( property )
	{
		case INVALID:
		default:
			ErrOut("itm_getProperty called with invalid property %d!\n", property );
			return emptyUnicodeString;
	}
}


bool getItemBoolProperty( P_ITEM pi, int property, int prop2)
{
	switch( property )
	{
		CHECK(NXW_IP_B_INCOGNITO, pi->incognito )   //dec value :  0;
		default:
			ErrOut("itm_getProperty called with invalid property %d!\n", property );
			return false;
	}
}

int getItemIntProperty( P_ITEM pi, int property, int prop2)
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
		default:
			ErrOut("itm_getProperty called with invalid property %d!\n", property );
			return INVALID;
	}
}

short getItemShortProperty( P_ITEM pi, int property, int prop2)
{
	switch( property )
	{
		CHECK(NXW_IP_S_AMOUNT, pi->amount )   //dec value :  400;
		CHECK(NXW_IP_S_AMOUNT2, pi->amount2 )   //dec value :  401;
		CHECK(NXW_IP_S_DIR, pi->dir )   //dec value :  402;
		CHECK(NXW_IP_S_COLOR, pi->getColor() )
		CHECK(NXW_IP_S_ID, pi->getId() )
		default:
			ErrOut("itm_getProperty called with invalid property %d!\n", property );
			return INVALID;
	}
}

char getItemCharProperty( P_ITEM pi, int property, int prop2)
{
	switch( property )
	{
		CHECK(NXW_IP_C_CORPSE, pi->corpse )   		//dec value :  102;
		CHECK(NXW_IP_C_DOORDIR, pi->doordir )   				//dec value :  103;
		CHECK(NXW_IP_C_DOOROPEN, pi->dooropen )   				//dec value :  104;
		CHECK(NXW_IP_C_DYE, pi->dye )   					//dec value :  105;
		CHECK(NXW_IP_C_LAYER, pi->layer )   					//dec value :  110;
		CHECK(NXW_IP_C_MAGIC, pi->magic )   					//dec value :  111;
		CHECK(NXW_IP_C_OFFSPELL, pi->offspell )   				//dec value :  114;
		CHECK(NXW_IP_C_OLDLAYER, pi->oldlayer )   				//dec value :  115;
		CHECK(NXW_IP_C_PILEABLE, pi->pileable )   				//dec value :  117;
		CHECK(NXW_IP_C_PRIV, pi->priv )   					//dec value :  118;
		CHECK(NXW_IP_C_VISIBLE, pi->visible )					//dec value :  120;
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
		CHECK(NXW_IP_C_DAMAGETYPE, pi->damagetype)				//dec value :  121;
		CHECK(NXW_IP_C_AUXDAMAGETYPE, pi->auxdamagetype)			//dec value :  122;
		default:
			ErrOut("itm_getProperty called with invalid property %d!\n", property );
			return '\0';
	}
}

const char* getItemStrProperty( P_ITEM pi, int property, int prop2)
{
	switch( property )
	{
		CHECK(NXW_IP_STR_CREATOR, pi->creator.c_str() )   //dec value :  450;
		CHECK(NXW_IP_STR_DESCRIPTION, pi->vendorDescription.c_str() )   //dec value :  451;
		CHECK(NXW_IP_STR_DISABLEDMSG, (pi->disabledmsg!=NULL)? (char*)pi->disabledmsg->c_str() : "" )   //dec value :  452;
		CHECK(NXW_IP_STR_MURDERER, pi->murderer.c_str() )   //dec value :  453;
		CHECK(NXW_IP_STR_NAME, pi->getCurrentNameC() )   //dec value :  454;
		CHECK(NXW_IP_STR_NAME2, pi->getSecondaryNameC() )   //dec value :  455;
		default:
			ErrOut("itm_getProperty called with invalid property %d!\n", property );
			return const_cast<char*>(emptyString);
	}
}





////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////// CHARACTER PROPERTY ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

NATIVE2(_getCharProperty)
{
	P_CHAR pc = pointers::findCharBySerial(params[1]);

	if ( ISVALIDPC( pc ) )
	{
		VAR_TYPE tp = getPropertyType( params[2] );
		switch( tp ) {
			case T_INT: {
				int p = getCharIntProperty( pc, params[2], params[3], params[4]);
				cell i = p;
				return i;
			}
			case T_BOOL: {
				bool p = getCharBoolProperty( pc, params[2], params[3]);
				cell i = p;
				return i;
			}
			case T_SHORT: {
				short p = getCharShortProperty( pc, params[2], params[3]);
				cell i = p;
				return i;
			}
			case T_CHAR: {
				char p = getCharCharProperty( pc, params[2], params[3]);
				cell i = p;
				return i;
			}
			case T_STRING: {
				//we're here so we should pass a string, params[4] is a str ptr
	  			char str[100];
	  			strcpy(str, getCharStrProperty( pc, params[2], params[3] ) );

	  			cell *cptr;
  				amx_GetAddr(amx,params[4],&cptr);
	  			amx_SetString(cptr,str, g_nStringMode);

  				return strlen(str);
			}
			case T_UNICODE: {
				wstring& w=getCharUniProperty( pc, params[2], params[3] );

				cell *cptr;
	  			amx_GetAddr(amx,params[4],&cptr);
				amx_SetStringUnicode(cptr, w );

				return w.length();
			}
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

	cell *cptr;
	amx_GetAddr(amx,params[4],&cptr);

	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {

		case T_INT : {
			int p = *cptr;
			setCharIntProperty( pc, params[2], params[3], params[5], p );
			return p;
		}
		case T_BOOL : {
			bool p = *cptr ? true : false;
			setCharBoolProperty( pc, params[2], params[3], params[5], p );
			return p;
		}
		case T_SHORT : {
			short p = static_cast<short>(*cptr & 0xFFFF);
			setCharShortProperty( pc, params[2], params[3], params[5], p );
			return p;
		}
		case T_CHAR : {
			char p = static_cast<char>(*cptr & 0xFF);
			setCharCharProperty( pc, params[2], params[3], params[5], p );
			return p;
		}
		case T_STRING : {
			//we're here so we should get a ConOut format string, params[4] is the str format

			printstring(amx,cptr,params+5,(int)(params[0]/sizeof(cell))-1);
			g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';

			setCharStrProperty( pc, params[2], params[3], params[5], g_cAmxPrintBuffer );

			g_nAmxPrintPtr=0;
			return 0;
		}
		case T_UNICODE : {

			std::wstring buffer;
			amx_GetStringUnicode( buffer, cptr );

			setCharUniProperty( pc, params[2], params[3], params[5], buffer );

			g_nAmxPrintPtr=0;
	  		return 0;

		}
	}
  	return 0;
}

void setCharStrProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, char* value )
{
	switch( property )
	{
		case NXW_CP_STR_DISABLEDMSG :			  				//dec value: 450;
			if( pc->disabledmsg==NULL )
				pc->disabledmsg = new std::string( value );
			else
				pc->disabledmsg->copy( value, strlen( value ) );
			break;
		case NXW_CP_STR_GUILDTITLE :							//dec value: 451;
			pc->GetGuildTitle();
			break;
		case NXW_CP_STR_LASTON :		  					//dec value: 452;
			break;
		case NXW_CP_STR_NAME :								//dec value: 453;
			pc->setCurrentName( value );
			break;
		case NXW_CP_STR_ORGNAME :		  					//dec value: 454;
			break;
		case NXW_CP_STR_TITLE :			  					//dec value: 455;
			pc->title = value;
			break;
		case NXW_CP_STR_TRIGWORD :		  					//dec value: 456;
			pc->trigword = value;
			break;
		case NXW_CP_STR_SPEECHWORD :		 				//dec value: 457;
			strcpy( script1, value );
			break;
		case NXW_CP_STR_SPEECH :			 				//dec value: 458;
			strcpy( script2, value );
			break;
		default :
			ErrOut("chr_setProperty called with invalid property %d!\n", property );
			break;
	}
}

void setCharShortProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, short value )
{
	switch( property )
	{
		case  NXW_CP_S_BASESKILL :				  		//dec value: 400;
			pc->baseskill[subproperty] = value;
			break;
		case NXW_CP_S_SKILL :				  			//dec value: 401;
			pc->skill[subproperty] = value;
			break;
		case NXW_CP_S_GUILDTYPE :				  		//dec value: 402;
			pc->SetGuildType( value );
			break;
		case NXW_CP_S_ID :
			pc->setId( value );
			break;
		case NXW_CP_S_SKIN :
			pc->setColor( value );
			break;
		case NXW_CP_S_XID :
			pc->setOldId( value );
			break;
		case NXW_CP_S_XSKIN :
			pc->setOldColor( value );
			break;
		case NXW_CP_S_RACE :
			pc->race = value;
		default :
			ErrOut("chr_setProperty called with invalid property %d!\n", property );
			break;
	}
}


void setCharCharProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, char value )
{
	switch( property )
	{
		case NXW_CP_C_BLOCKED :							//dec value: 101;
			pc->blocked = value;
			break;
		case NXW_CP_C_COMMANDLEVEL :				  		//dec value: 103;
			pc->commandLevel = value;
			break;
		case NXW_CP_C_DIR :	  						//dec value: 104;
			pc->dir = value;
			break;
		case NXW_CP_C_DIR2 :			  				//dec value: 105;
			pc->dir2 = value;
			break;
		case NXW_CP_C_FIXEDLIGHT :				  		//dec value: 106;
			pc->fixedlight = value;
			break;
		case NXW_CP_C_FLAG :			  				//dec value: 107;
			pc->flag = value;
			break;
		case NXW_CP_C_FLY_STEPS :			  			//dec value: 108;
			pc->fly_steps = value;
			break;
		case NXW_CP_C_GMRESTRICT :				  		//dec value: 109;
			pc->gmrestrict = value;
			break;
		case NXW_CP_C_HIDDEN :				  			//dec value: 110;
			pc->hidden = value;
			break;
		case NXW_CP_C_LOCKSKILL : 						//dec value: 112;
			pc->lockSkill[subproperty] = value;
			break;
		case NXW_CP_C_NPC :							//dec value: 114;
			pc->npc = value;
			break;
		case NXW_CP_C_NPCTYPE : 		  				//dec value: 115;
			pc->npc_type = value;
			break;
		case NXW_CP_C_NPCWANDER :						//dec value: 116;
			pc->npcWander = value;
			break;
		case NXW_CP_C_OLDNPCWANDER : 			  			//dec value: 117;
			pc->oldnpcWander = value;
			break;
		case NXW_CP_C_PRIV2 :				  			//dec value: 121;
			pc->SetPriv2(value);
			break;
		case NXW_CP_C_REACTIVEARMORED : 		  			//dec value: 122;
			pc->ra = value;
			break;
		case NXW_CP_C_REGION : 				  			//dec value: 123;
			pc->region = value;							//candidate for removal
			break;
		case NXW_CP_C_SHOP :				 			//dec value: 125; Sparhawk: DEPRECIATED, use case NXW_CP_B_SHOPKEEPER : pc->shopkeeper )
			pc->shopkeeper = value;
			break;
		case NXW_CP_C_SPEECH : 				  			//dec value: 127;
			pc->speech = value;
			break;
		case NXW_CP_C_WAR :			  				//dec value: 128;
			pc->war = value;
			break;
		case NXW_CP_C_NXWFLAGS :						//dec value: 131;
			pc->nxwflags[subproperty] = value;
			break;
		case NXW_CP_I_RESISTS : 						//dec value: 132;
			pc->resists[subproperty] = value;
			break;
		case NXW_CP_C_TRAININGPLAYERIN : 					//dec value: 133;
			pc->trainingplayerin = value;
			break;
		case NXW_CP_C_PRIV : 				  			//dec value: 134;
			pc->SetPriv(value);
			break;
		case NXW_CP_C_DAMAGETYPE :						//dec value: 319;
			pc->damagetype = static_cast<DamageType>(value);
			break;
		default :
			ErrOut("chr_setProperty called with invalid property %d!\n", property );
			break;
	}
}

void setCharBoolProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, bool value )
{
	switch( property )
	{
		case NXW_CP_B_CANTRAIN :						//dec value: 0;
			if ( value )
				pc->setCanTrain();
			else
				pc->resetCanTrain();
			break;
		case NXW_CP_B_DEAD :							//dec value: 1;
			break;
		case NXW_CP_B_FREE :							//dec value: 2;
			break;
		case NXW_CP_B_GUARDED :							//dec value: 3;
			pc->guarded = value;
			break;
		case NXW_CP_B_GUILDTRAITOR :						//dec value: 4;
			if ( value )
				pc->SetGuildTraitor();
			else
				pc->ResetGuildTraitor();
			break;
		case NXW_CP_B_INCOGNITO :						//dec value: 6;
			pc->guarded = value;
			break;
		case NXW_CP_B_POLYMORPH :						//dec value: 8;
			pc->polymorph = value;						//Candidate for removal
			break;
		case NXW_CP_B_TAMED :							//dec value: 9;
			pc->tamed = value;
			break;
		case NXW_CP_B_UNICODE :							//dec value: 10;
			pc->unicode = value;
			break;
		case NXW_CP_B_SHOPKEEPER :						// dec value 11;
			pc->shopkeeper = value;
			break;
		case NXW_CP_B_ATTACKFIRST :						//dec value: 12;
			if (value)
				pc->SetAttackFirst();
			else
				pc->ResetAttackFirst();
			break;
		case NXW_CP_B_ISBEINGTRAINED :						//dec value: 13;
			break;
		case NXW_CP_B_GUILDTOGGLE :						//dec value: 14;
			if ( value )
				pc->SetGuildTitleToggle();
			else
				pc->ResetGuildTitleToggle();
			break;
		case NXW_CP_B_FROZEN :
			if( value )
				pc->freeze();
			else
				pc->unfreeze();
			break;
		default :
			ErrOut("chr_setProperty called with invalid property %d!\n", property );
			break;
	}
}

void setCharIntProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, int value )
{
	switch( property )
	{
		case NXW_CP_I_ACCOUNT :		  				//dec value: 200;
			pc->account = value;
			break;
		case NXW_CP_I_ADVOBJ :			  			//dec value: 201;
			pc->advobj = value;
			break;
		case NXW_CP_I_ATT :		  				//dec value: 202;
			pc->att = value;
			break;
		case NXW_CP_I_ATTACKER :		  			//dec value: 203;
			pc->attackerserial = value;
			break;
		case NXW_CP_I_BEARDCOLOR :					//dec value: 204;
			// obsolete
			break;
		case NXW_CP_I_BEARDSERIAL :			  		//dec value: 205;
			pc->beardserial = value;
			break;
		case NXW_CP_I_BEARDSTYLE :					//dec value: 206;
			//obsolete
			break;
		case NXW_CP_I_CALLNUM :			  			//dec value: 207;
			pc->callnum = value;
			break;
		case NXW_CP_I_CARVE :		  				//dec value: 208;
			pc->carve = value;
			break;
		case NXW_CP_I_CASTING :			  			//dec value: 209;
			pc->casting = value;
			break;
		case NXW_CP_I_CLIENTIDLETIME :				  	//dec value: 210;
			pc->clientidletime = value;
			break;
		case NXW_CP_I_COMBATHITMESSAGE :			  	//dec value: 211;
			pc->combathitmessage = value;
			break;
		case NXW_CP_I_CREATIONDAY :			  		//dec value: 212;
			pc->SetCreationDay(value);
			break;
		case NXW_CP_I_CRIMINALFLAG :			  		//dec value: 213;
			pc->crimflag = value;
			break;
		case NXW_CP_I_DEATHS :			  			//dec value: 214;
			pc->deaths = value;
			break;
		case NXW_CP_I_DEF :		  				//dec value: 215;
			pc->def = value;
			break;
		case NXW_CP_I_DEXTERITY:					//dec value: 216
			switch( subproperty )
			{
				case NXW_CP2_DEC :
					pc->dx2 = value;
					break;
				case NXW_CP2_REAL :
					pc->dx3 = value;
					break;
				case NXW_CP2_ACT :
					pc->stm = value;
					break;
				case NXW_CP2_EFF:
					pc->dx = value;
					break;
			}
			pc->updateStats(2);
			break;
		case NXW_CP_I_DISABLED :		  			//dec value: 217;
			pc->disabled = value;
			break;
		case NXW_CP_I_FAME :		  				//dec value: 218;
			pc->SetFame( value );
			break;
		case NXW_CP_I_FLEEAT :			  			//dec value: 219;
			pc->fleeat = value;
			break;
		case NXW_CP_I_FOODPOSITION:					//dec value: 220;
			switch( subproperty )
			{
				case NXW_CP2_X :
					pc->foodloc.x = value;
					break;
				case NXW_CP2_Y :
					pc->foodloc.y = value;
					break;
				case NXW_CP2_Z :
					pc->foodloc.z = value;
					break;
			}
			break;
		case NXW_CP_I_FPOS1_NPCWANDER:					//dec value: 221;
			switch( subproperty )
			{
				case NXW_CP2_X :
					pc->fx1 = value;
					break;
				case NXW_CP2_Y :
					pc->fy1 = value;
					break;
				case NXW_CP2_Z :
					pc->fz1 = value;
					break;
			}
			break;
		case NXW_CP_I_FPOS2_NPCWANDER:					//dec value 222;
			switch( subproperty )
			{
				case NXW_CP2_X :
					pc->fx2 = value;
					break;
				case NXW_CP2_Y :
					pc->fy2 = value;
				case NXW_CP2_Z :
					break;
			}
			break;
		case NXW_CP_I_FTARG :			  				//dec value: 223;
			pc->ftargserial = value;
			break;
		case NXW_CP_I_GMMOVEEFF :			  			//dec value: 224;
			pc->gmMoveEff = value;
			break;
		case NXW_CP_I_GUILDFEALTY :				  		//dec value: 225;
			pc->SetGuildFealty( value );
			break;
		case NXW_CP_I_GUILDNUMBER :				  		//dec value: 226;
			pc->SetGuildNumber( value );
			break;
		case NXW_CP_I_HAIRCOLOR :						//dec value: 228;
			//obsolete
			break;
		case NXW_CP_I_HAIRSERIAL :				  		//dec value: 229;
			pc->hairserial = value;
			break;
		case NXW_CP_I_HAIRSTYLE :						//dec value: 230;
			//obsolete
			break;
		case NXW_CP_I_HIDAMAGE :			  			//dec value: 231;
			pc->hidamage = value;
			break;
		case NXW_CP_I_HOLDGOLD :			  			//dec value: 232;
			pc->holdg = value;
			break;
		case NXW_CP_I_HOMELOCPOS :						//dec value: 233
			switch( subproperty )
			{
				case NXW_CP2_X :
					pc->homeloc.x = value;
					break;
				case NXW_CP2_Y :
					pc->homeloc.y = value;
					break;
				case NXW_CP2_Z :
					pc->homeloc.z = value;
					break;
			}
			break;
		case NXW_CP_I_HUNGER :				  			//dec value: 234;
			pc->hunger = value;
			break;
		case NXW_CP_I_HUNGERTIME :				  		//dec value: 235;
			pc->hungertime = value;
			break;
		case NXW_CP_I_INTELLIGENCE:						//dec value: 236;
			switch( subproperty )
			{
				case NXW_CP2_DEC  :
					pc->in2 = value;
					break;
				case NXW_CP2_REAL :
					pc->in3 = value;
					break;
				case NXW_CP2_ACT  :
					pc->mn = value;
					break;
				case NXW_CP2_EFF  :
					pc->in = value;
					break;
			}
			pc->updateStats(1);
			break;
		case NXW_CP_I_KARMA : 				 			//dec value: 237;
			pc->SetKarma( value );
			break;
		case NXW_CP_I_KEYNUMBER :			  			//dec value: 238;
			pc->keyserial = value;
			break;
		case NXW_CP_I_KILLS :			  				//dec value: 239;
			pc->kills = value;
			break;
		case NXW_CP_I_LODAMAGE :			  			//dec value: 240;
			pc->lodamage = value;
			break;
		case NXW_CP_I_LOGOUT :				  			//dec value: 241;
			pc->logout = value;
			break;
		case NXW_CP_I_MAKING :				  			//dec value: 242;
			pc->making = value;
			break;
		case NXW_CP_I_MEDITATING :			  			//dec value: 243;
			pc->med = value;
			break;
		case NXW_CP_I_MULTISERIAL :						//dec value: 245;
			pc->setMultiSerial( value );
			break;
		case NXW_CP_I_MURDERERSER :				  		//dec value: 246;
			pc->murdererSer = value;
			break;
		case NXW_CP_I_MURDERRATE :				  		//dec value: 247;
			pc->murderrate = value;
			break;
		case NXW_CP_I_MUTETIME :			  			//dec value: 248;
			pc->mutetime = value;
			break;
		case NXW_CP_I_NAMEDEED :			  			//dec value: 249;
			pc->namedeedserial = value;
			break;
		case NXW_CP_I_NEXTACT :				  			//dec value: 250;
			pc->nextact = value;
			break;
		case NXW_CP_I_NPCAI :				  			//dec value: 251;
			pc->npcaitype = value;
			break;
		case NXW_CP_I_NPCMOVETIME :				  		//dec value: 252;
			pc->npcmovetime = value;
			break;
		case NXW_CP_I_OBJECTDELAY :				  		//dec value: 253;
			pc->objectdelay = value;
			break;
		case NXW_CP_I_OLDPOS:						//dec value: 254;
			switch( subproperty )
			{
				case NXW_CP2_X :
					pc->setOldPosition("x",value);
					break;
				case NXW_CP2_Y :
					pc->setOldPosition("y",value);
					break;
				case NXW_CP2_Z :
					pc->setOldPosition("z",value);
					break;
			}
			break;
		case NXW_CP_I_OWNSERIAL :				  		//dec value: 255;
			pc->setOwnerSerial32( value );
			break;
		case NXW_CP_I_PACKITEM :			  			//dec value: 256;
			pc->packitemserial = value;
			break;
		case NXW_CP_I_POISON :				  			//dec value: 257;
			pc->poison = value;
			break;
		case NXW_CP_I_POISONED :			  			//dec value: 258;
			pc->poisoned = (PoisonType)value;
			break;
		case NXW_CP_I_POISONTIME :				  		//dec value: 259;
			pc->poisontime = value;
			break;
		case NXW_CP_I_POISONTXT :			  			//dec value: 260;
			pc->poisontxt = value;
			break;
		case NXW_CP_I_POISONWEAROFFTIME :				  	//dec value: 261;
			pc->poisonwearofftime = value;
			break;
		case NXW_CP_I_POSITION:						//dec value: 262;
			switch( subproperty )
			{
				case NXW_CP2_X :
					pc->setPosition("x",value);
					break;
				case NXW_CP2_Y :
					pc->setPosition("y",value);
					break;
				case NXW_CP2_Z :
					pc->setPosition("z",value);
			} break;
		case NXW_CP_I_POSTTYPE :			  			//dec value: 263;
			pc->postType = (MsgBoards::PostType)value;
			break;
		case NXW_CP_I_PREVPOS:						//dec value: 264
			switch( subproperty )
			{
				case NXW_CP2_X :
					pc->prevX = value;
					break;
				case NXW_CP2_Y :
					pc->prevY = value;
					break;
				case NXW_CP2_Z :
					pc->prevZ = value;
					break;
			}
			break;
		case NXW_CP_I_QUESTBOUNTYPOSTSERIAL :					  //dec value: 266;
			pc->questBountyPostSerial = value;
			break;
		case NXW_CP_I_QUESTBOUNTYREWARD :				  	//dec value: 267;
			pc->questBountyReward = value;
			break;
		case NXW_CP_I_QUESTDESTREGION :					  	//dec value: 268;
			pc->questDestRegion = value;
			break;
		case NXW_CP_I_QUESTORIGREGION :					  	//dec value: 269;
			pc->questOrigRegion = value;
			break;
		case NXW_CP_I_REATTACKAT :				  		//dec value: 270;
			pc->reattackat = value;
			break;
		case NXW_CP_I_REGENRATE :				  		//dec value: 271;
			pc->setRegenRate( static_cast<StatType>(subproperty), value, static_cast<VarType>(subsubproperty) );
			break;
		case NXW_CP_I_SCRIPTID :			  				//dec value: 272;
			pc->setScriptID( value );
			break;
		case NXW_CP_I_GUILD:							//dec value: 273
			//todo
			break;
		case NXW_CP_I_ROBE :			  				//dec value: 274;
			pc->robe = value;
			break;
		case NXW_CP_I_RUNNING :				  			//dec value: 275;
			pc->running = value;
			break;
		case NXW_CP_I_SERIAL :				  			//dec value: 276;
			pc->setSerial32(value);
			break;
		case NXW_CP_I_SKILLDELAY :				  		//dec value: 277;
			pc->skilldelay = value;
			break;
		case NXW_CP_I_SMOKEDISPLAYTIME :			  		//dec value: 279;
			pc->smokedisplaytimer = value;
			break;
		case NXW_CP_I_SMOKETIMER :				  		//dec value: 280;
			pc->smoketimer = value;
			break;
		case NXW_CP_I_SPADELAY :			  			//dec value: 281;
			pc->spadelay = value;
			break;
		case NXW_CP_I_SPATIMER :			  			//dec value: 282;
			pc->spatimer = value;
			break;
		case NXW_CP_I_SPATTACK :			  			//dec value: 283;
			pc->spattack = value;
			break;
		case NXW_CP_I_SPAWNREGION :				  		//dec value: 284;
			pc->spawnregion = value;
			break;
		case NXW_CP_I_SPAWNSERIAL :				  		//dec value: 285;
			pc->spawnserial = value;
			break;
		case NXW_CP_I_SPELL :			  				//dec value: 286;
			pc->spell = static_cast<enum magic::SpellId>(value);
			break;
		case NXW_CP_I_SPELLACTION :				  		//dec value: 287;
			pc->spellaction = value;
			break;
		case NXW_CP_I_SPELLTIME :			  			//dec value: 288;
			pc->spelltime = value;
			break;
		case NXW_CP_I_SPLIT :			  				//dec value: 290;
			pc->split = value;
			break;
		case NXW_CP_I_SPLITCHNC :			  			//dec value: 291;
			pc->splitchnc = value;
			break;
		case NXW_CP_I_SQUELCHED :			  			//dec value: 292;
			pc->squelched = value;
			break;
		case NXW_CP_I_STEALTH :				  			//dec value: 294;
			pc->stealth = value;
			break;
		case NXW_CP_I_STRENGHT:							//dec value: 295;
			switch( subproperty )
			{
				case NXW_CP2_DEC :
					pc->st2 = value;
					break;
				case NXW_CP2_REAL:
					pc->st3 = value;
					break;
				case NXW_CP2_ACT :
					pc->hp = value;
					break;
				case NXW_CP2_EFF :
					pc->setStrength(value);
					break;
			}
			pc->updateStats(0);
			break;
		case NXW_CP_I_SUMMONTIMER :				  		//dec value: 296;
			pc->summontimer = value;
			break;
		case NXW_CP_I_SWINGTARG :			  			//dec value: 297;
			pc->swingtargserial = value;
			break;
		case NXW_CP_I_TAMING :				  			//dec value: 299;
			pc->taming = value;
			break;
		case NXW_CP_I_TARG :			  				//dec value: 300;
			pc->targserial = value;
			break;
		case NXW_CP_I_TARGTRIG :			  			//dec value: 301;
			pc->targtrig = value;
			break;
		case NXW_CP_I_TEMPFLAGTIME :				  		//dec value: 302;
			pc->tempflagtime = value;
			break;
		case NXW_CP_I_TIME_UNUSED :				  		//dec value: 303;
			pc->time_unused = value;
			break;
		case NXW_CP_I_TIMEOUT :				 			//dec value: 304;
			pc->timeout = value;
			break;
		case NXW_CP_I_TIMEUSED_LAST :				  		//dec value: 305;
			pc->timeused_last = value;
			break;
		case NXW_CP_I_TRAINER :				  			//dec value: 309;
			break;
		case NXW_CP_I_FLEETIMER:
			pc->fleeTimer = value;
			break;
		case NXW_CP_I_TRIGGER :				  			//dec value: 311;
			pc->trigger = value;
			break;
		case NXW_CP_I_WEIGHT :				  			//dec value: 312;
			pc->weight = value;
			break;
		case NXW_CP_I_WORKLOCPOS:					//dec value: 313;
			switch( subproperty )
			{
				case NXW_CP2_X :
					pc->workloc.x = value;
					break;
				case NXW_CP2_Y :
					pc->workloc.y = value;
					break;
				case NXW_CP2_Z :
					pc->workloc.z = value;
					break;
			}
			break;
		case NXW_CP_I_AMXFLAGS :					//dec value: 314;
			//
			// AMXFLAGS ARE NOW HANDLED AS NEW STYLE AMX VARS
			//
			//pc->amxflags[params[3]] = p;
			if ( subproperty >= 0 && subproperty <= 15 )
				amxVS.updateVariable( pc->getSerial32(), subproperty, value );
			break;
		case NXW_CP_I_RACE :		 				//dec value: 315
			pc->race = value;
			break;
		case NXW_CP_I_CX :			 			//dec value: 316
			pc->setPosition("x",value);
			break;
		case NXW_CP_I_CY :			 			//dec value: 317
			pc->setPosition("y",value);
			break;
		case NXW_CP_I_CZ :			 			//dec value: 318
			pc->setPosition("z",value);
			break;
		case NXW_CP_I_LASTMOVETIME :					//dec value: 319
			pc->LastMoveTime = value;
			break;
		default :
			ErrOut("chr_setProperty called with invalid property %d!\n", property );
			break;
	}

}

void setCharUniProperty( P_CHAR pc, int property, int subproperty, int subsubproperty, wstring& value )
{
	switch( property )
	{
		case NXW_CP_UNI_SPEECH_CURRENT : {
			pc->deleteSpeechCurrent();
			pc->setSpeechCurrent( new std::wstring( value ) );
			break;
		}
		case NXW_CP_UNI_PROFILE : {
			pc->profile = value;
			break;
		}
		default :
			ErrOut("chr_setProperty called with invalid property %d!\n", property );
			break;
	}
}

bool getCharBoolProperty( P_CHAR pc, int property, int prop2 )
{
	switch( property )
	{
		CHECK(  NXW_CP_B_CANTRAIN , pc->canTrain() )			//dec value: 0;
		CHECK(  NXW_CP_B_DEAD , pc->dead )  				//dec value: 1;
		CHECK(  NXW_CP_B_GUARDED , pc->guarded )  			//dec value: 3;
		CHECK(  NXW_CP_B_GUILDTRAITOR , pc->IsGuildTraitor() )  	//dec value: 4;
		CHECK(  NXW_CP_B_INCOGNITO , pc->incognito )  			//dec value: 6;
		CHECK(  NXW_CP_B_JAILED , pc->jailed )  			//dec value: 5;
		CHECK(  NXW_CP_B_ONHORSE , pc->isMounting() )  			//dec value: 7;
		CHECK(  NXW_CP_B_POLYMORPH , pc->polymorph)  			//dec value: 8;
		CHECK(  NXW_CP_B_TAMED , pc->tamed )  				//dec value: 9;
		CHECK(  NXW_CP_B_UNICODE , pc->unicode )  			//dec value: 10;
		CHECK(  NXW_CP_B_SHOPKEEPER , pc->shopkeeper ) 			//dec value: 11;
		CHECK(  NXW_CP_B_ATTACKFIRST , pc->HasAttackedFirst() )  	//dec value: 12;
		CHECK(  NXW_CP_B_ISBEINGTRAINED, pc->isBeingTrained() )  	//dec value: 13;
		CHECK(  NXW_CP_B_GUILDTOGGLE , pc->HasGuildTitleToggle() )  	//dec value: 14;
		CHECK(  NXW_CP_B_OVERWEIGHTED, pc->IsOverWeight() )     	//dec value: 15;
		CHECK(  NXW_CP_B_MOUNTED, pc->mounted )     	//dec value: 15;
		CHECK(  NXW_CP_B_FROZEN, pc->isFrozen() )     	//dec value: 15;
		default:
			ErrOut("chr_getProperty called with invalid property %d!\n", property );
			return false;
	}
}

int getCharIntProperty( P_CHAR pc, int property, int prop2, int prop3 )
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
		CHECK(  NXW_CP_I_QUESTBOUNTYPOSTSERIAL , pc->questBountyPostSerial )  //dec value: 266;
		CHECK(  NXW_CP_I_QUESTBOUNTYREWARD , pc->questBountyReward )  	//dec value: 267;
		CHECK(  NXW_CP_I_QUESTDESTREGION , pc->questDestRegion )  	//dec value: 268;
		CHECK(  NXW_CP_I_QUESTORIGREGION , pc->questOrigRegion )  	//dec value: 269;
		CHECK(  NXW_CP_I_REATTACKAT , pc->reattackat )  		//dec value: 270;
		CHECK(  NXW_CP_I_REGENRATE, pc->getRegenRate( static_cast<StatType>(prop2), static_cast<VarType>(prop3) ); )
		CHECK(  NXW_CP_I_SCRIPTID , pc->getScriptID() )  				//dec value: 272;
		CHECK(	NXW_CP_I_GUILD, (pc->getGuild()!=NULL)? pc->getGuild()->serial : INVALID )			//dec value: 273
		CHECK(  NXW_CP_I_ROBE , pc->robe )  				//dec value: 274;
		CHECK(  NXW_CP_I_RUNNING , pc->running )  			//dec value: 275;
		CHECK(  NXW_CP_I_SERIAL , pc->getSerial32() )  			//dec value: 276;
		CHECK(  NXW_CP_I_SKILLDELAY , pc->skilldelay )  		//dec value: 277;
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
		CHECK(  NXW_CP_I_TAMING , pc->taming )  			//dec value: 299;
		CHECK(  NXW_CP_I_TARG , pc->targserial )  				//dec value: 300;
		CHECK(  NXW_CP_I_TARGTRIG , pc->targtrig )  			//dec value: 301;
		CHECK(  NXW_CP_I_TEMPFLAGTIME , pc->tempflagtime )  		//dec value: 302;
		CHECK(  NXW_CP_I_TIME_UNUSED , pc->time_unused )  		//dec value: 303;
		CHECK(  NXW_CP_I_TIMEOUT , pc->timeout )  			//dec value: 304;
		CHECK(  NXW_CP_I_TIMEUSED_LAST , pc->timeused_last )  		//dec value: 305;
		CHECK(  NXW_CP_I_TRAINER , pc->getTrainer() )  			//dec value: 309;
		CHECK(  NXW_CP_I_FLEETIMER , pc->fleeTimer )  			//dec value: 310;
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
		CHECK(  NXW_CP_I_PARTY, pc->party )		//dec value: 320
		default:
			ErrOut("chr_getProperty called with invalid property %d!\n", property );
			return INVALID;
	}
}


short getCharShortProperty( P_CHAR pc, int property, int prop2 )
{
	switch( property )
	{
		CHECK(  NXW_CP_S_BASESKILL , pc->baseskill[prop2] )  		//dec value: 400;
		CHECK(  NXW_CP_S_SKILL , pc->skill[prop2] )  			//dec value: 401;
		CHECK(  NXW_CP_S_GUILDTYPE , pc->GetGuildType() )  		//dec value: 402;
		CHECK(  NXW_CP_S_ID, pc->getId() )
		CHECK(  NXW_CP_S_SKIN, pc->getColor() )
		CHECK(  NXW_CP_S_XID, pc->getOldId() )
		CHECK(  NXW_CP_S_XSKIN, pc->getOldColor() )
		case NXW_CP_S_ICON : {
			P_CREATURE_INFO creature = creatures.getCreature( pc->getId() );
			return ( creature!=NULL )? creature->icon : INVALID;

		}
		case NXW_CP_S_SOUND : {
			P_CREATURE_INFO creature = creatures.getCreature( pc->getId() );
			return ( creature!=NULL )? creature->getSound( static_cast<MonsterSound>(prop2) ) : INVALID;
		}
		CHECK(  NXW_CP_S_RACE, pc->race )
		default:
			ErrOut("chr_getProperty called with invalid property %d!\n", property );
			return INVALID;
	}
}

char getCharCharProperty( P_CHAR pc, int property, int prop2 )
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
		CHECK(  NXW_CP_C_LOCKSKILL , pc->lockSkill[prop2] )  		//dec value: 112;
		CHECK(  NXW_CP_C_NPC , pc->npc )  				//dec value: 114;
		CHECK(  NXW_CP_C_NPCTYPE , pc->npc_type )  			//dec value: 115;
		CHECK(  NXW_CP_C_NPCWANDER , pc->npcWander )  			//dec value: 116;
		CHECK(  NXW_CP_C_OLDNPCWANDER , pc->oldnpcWander )  		//dec value: 117;
		CHECK(  NXW_CP_C_PRIV2 , pc->GetPriv2() )  			//dec value: 121;
		CHECK(  NXW_CP_C_REACTIVEARMORED , pc->ra )  			//dec value: 122;
		CHECK(  NXW_CP_C_REGION , pc->region )  			//dec value: 123;
		CHECK(  NXW_CP_C_SHOP , pc->shopkeeper )  			//dec value: 125; Sparhawk: DEPRECIATED, use CHECK(  NXW_CP_B_SHOPKEEPER , pc->shopkeeper )
		CHECK(  NXW_CP_C_SPEECH , pc->speech )  			//dec value: 127;
		CHECK(  NXW_CP_C_WAR , pc->war )  				//dec value: 128;
		CHECK(  NXW_CP_C_NXWFLAGS, pc->nxwflags[prop2])			//dec value: 131;
		CHECK(  NXW_CP_I_RESISTS, pc->resists[prop2])			//dec value: 132;
		CHECK(  NXW_CP_C_TRAININGPLAYERIN , pc->getSkillTaught() )  	//dec value: 133;
		CHECK(  NXW_CP_C_PRIV , pc->GetPriv() )  			//dec value: 134;
		CHECK(  NXW_CP_C_DAMAGETYPE, pc->damagetype)			//dec value: 135;
		default:
			ErrOut("chr_getProperty called with invalid property %d!\n", property );
			return '\0';
	}
}

const char* getCharStrProperty( P_CHAR pc, int property, int prop2 )
{
	switch( property )
	{
		CHECK(  NXW_CP_STR_DISABLEDMSG , (pc->disabledmsg!=NULL)? pc->disabledmsg->c_str() : emptyString )  		//dec value: 450;
		CHECK(  NXW_CP_STR_GUILDTITLE , pc->GetGuildTitle() )  		//dec value: 451;
		CHECK(  NXW_CP_STR_LASTON , "<obsolete>" )  			//dec value: 452;
		CHECK(  NXW_CP_STR_NAME, pc->getCurrentNameC() )  //dec value: 453;
		CHECK(  NXW_CP_STR_ORGNAME , "<obsolete>" )  			//dec value: 454;
		CHECK(  NXW_CP_STR_TITLE , pc->title.c_str() )  			//dec value: 455;
		CHECK(  NXW_CP_STR_TRIGWORD , pc->trigword.c_str() )  			//dec value: 456;
		CHECK(	NXW_CP_STR_SPEECHWORD, script1 ) 			//dec value: 457;
		CHECK(	NXW_CP_STR_SPEECH, script2 ) 				//dec value: 458;
		default:
			ErrOut("chr_getProperty called with invalid property %d!\n", property );
			return emptyString;
	}
}

wstring& getCharUniProperty( P_CHAR pc, int property, int prop2 )
{
	switch( property )
	{
		CHECK(  NXW_CP_UNI_SPEECH_CURRENT , GETPWSTRING( pc->getSpeechCurrent() ) )
		CHECK(  NXW_CP_UNI_PROFILE, pc->profile )
		default:
			ErrOut("chr_getProperty called with invalid property %d!\n", property );
			return emptyUnicodeString;
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////// CALENDAR PROPERTY ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
\author Sparhawk
\brief Return calendar property
\param 1 never
\param 2 property
\param 3 option month or weekend number
\param 4 string/array reference ( output )
\return value of numeric property or length of params[4]
*/
NATIVE2(_getCalProperty)
{
	int tp = getPropertyType(params[2]);

	if (tp==T_INT) {
		int *p = reinterpret_cast<int*>(getCalPropertyPtr(INVALID, params[1], params[2]));
		cell i = *p;
		return i;
	}
	if (tp==T_BOOL) {
		bool *p = reinterpret_cast<bool*>(getCalPropertyPtr(INVALID, params[1], params[2]));
		cell i = *p;
		return i;
	}
	if (tp==T_SHORT) {
		short *p = reinterpret_cast<short*>(getCalPropertyPtr(INVALID, params[1], params[2]));
		cell i = *p;
		return i;
	}
	if (tp==T_CHAR) {
		char *p = reinterpret_cast<char*>(getCalPropertyPtr(INVALID, params[1], params[2]));
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

/*!
\brief Return addres to calendar property value
\param 1 never used
\param 2 property
\param 3 prop2, optional month or weekday number
\return pointer to value of calendar property
\author	Sparhawk
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


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////////// GUILD PROPERTY ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

NATIVE2(_guild_setProperty)
{
	// params[1] = guild
	// params[2] = property
	// params[3] = subproperty
	// params[4] = value to set property to

	SERIAL guild = params[1];
	P_GUILD pGuild = Guildz.getGuild( guild );
	if( pGuild==NULL )
	{
		LogError( "guild_setProperty called with invalid guild %d", guild );
		return INVALID;
	}

	int tp = getPropertyType(params[2]);

	cell *cptr;
	amx_GetAddr(amx,params[4],&cptr);

	switch( tp ) {

	case T_INT: {
		int p = *cptr;

		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
				break;
		}

		return p;
	}
	break;

	case T_BOOL: {

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
	break;

	case T_SHORT: {

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
	break;

	case T_CHAR: {

		char p = static_cast<char>(*cptr & 0xFF);

		switch( params[2] )
		{
			case  INVALID :				  		//dec value: 100;
			default :
				ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_STRING: {

		//we're here so we should get a ConOut format string, params[4] is the str format

		printstring(amx,cptr,params+5,(int)(params[0]/sizeof(cell))-1);
		g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';
		std::string str( g_cAmxPrintBuffer );
		switch( params[2] )
		{
			case NXW_GP_STR_NAME :			  				//dec value: 450;
				pGuild->setName( str );
				break;
			case NXW_GP_STR_WEBPAGE :		  				//dec value: 451;
				pGuild->webpage = g_cAmxPrintBuffer;
				break;
			case NXW_GP_STR_ABBREVIATION :	  				//dec value: 452;
				pGuild->setAbbreviation( str );
				break;
			default :
				ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		g_nAmxPrintPtr=0;
		return 0;
	}
	break;

	case T_UNICODE: {

		wstring w;
		amx_GetStringUnicode( w, cptr );

		switch( params[2] )
		{
			case NXW_GP_UNI_CHARTER :
				pGuild->charter = w;
				break;
			default :
				ErrOut("chr_setProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		g_nAmxPrintPtr=0;
	  	return 0;
	}
	break;

	default:
		return INVALID;
	}
}

NATIVE2(_guild_getProperty)
{

	SERIAL guild = params[1];
	P_GUILD pGuild = Guildz.getGuild( guild );
	if( pGuild==NULL )
	{
		LogError( "guild_setProperty called with invalid guild %d", guild );
		return INVALID;
	}

	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {

	case T_INT: {

		int p;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		cell i = p;
		return i;
	}
	break;

	case T_BOOL: {

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
	break;

	case T_SHORT: {

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
	break;

	case T_CHAR: {

		char p;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		cell i = p;
		return i;
	}
	break;

	case T_STRING: {

		//we're here so we should pass a string, params[4] is a str ptr

	  	char str[100];
		cell *cptr;
		switch(params[2]) {
			case NXW_GP_STR_NAME:
			  	strcpy(str, pGuild->getName().c_str());
				break;
			case NXW_GP_STR_WEBPAGE:
			  	strcpy(str, pGuild->webpage.c_str());
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
	break;

	case T_UNICODE: {

		wstring* w=NULL;
		switch( params[2] )
		{
			case NXW_GP_UNI_CHARTER :
				w = &pGuild->charter;
				break;
			default :
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		if( w==NULL ) w=&emptyUnicodeString;
		cell *cptr;
	  	amx_GetAddr(amx,params[4],&cptr);
		amx_SetStringUnicode(cptr, *w );
		return w->length();

	}
	break;

	default:
		return INVALID;
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//////////////////////// GUILD MEMBER PROPERTY /////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

NATIVE2(_guildMember_setProperty)
{
	// params[1] = member
	// params[2] = property
	// params[3] = subproperty
	// params[4] = value to set property to

	SERIAL member_serial = params[1];

	P_CHAR pc = pointers::findCharBySerial( member_serial );
	VALIDATEPCR( pc, INVALID );

	P_GUILD_MEMBER member = pc->getGuildMember();
	if ( member==NULL )
	{
		LogError( "guildMember_setProperty called with invalid member %d", member_serial );
		return INVALID;
	}

	int tp = getPropertyType( params[2] );

	cell *cptr;
	amx_GetAddr(amx,params[4],&cptr);

	switch( tp ) {

	case T_INT: {
		int p = *cptr;

		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("guildMember_setProperty called with invalid property %d!\n", params[2] );
				break;
		}

		return p;
	}
	break;

	case T_BOOL: {

		bool p = *cptr ? true : false;

		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("guildMember_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_SHORT: {

		short p = static_cast<short>(*cptr & 0xFFFF);
		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("guildMember_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_CHAR: {

		char p = static_cast<char>(*cptr & 0xFF);

		switch( params[2] )
		{
			case NXW_GMP_C_RANK :				  		//dec value: 100;
				member->rank = p;
				break;
			case NXW_GMP_C_TITLETOGGLE:
				member->toggle = static_cast<GUILD_TITLE_TOGGLE>(p);
				break;
			default :
				ErrOut("guildMember_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_STRING: {

		//we're here so we should get a ConOut format string, params[4] is the str format

		printstring(amx,cptr,params+5,(int)(params[0]/sizeof(cell))-1);
		g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';
		switch( params[2] )
		{
			case NXW_GMP_STR_TITLE :			  				//dec value: 450;
				member->title = g_cAmxPrintBuffer;
				break;
			default :
				ErrOut("guildMember_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		g_nAmxPrintPtr=0;
		return 0;
	}
	break;

	case T_UNICODE: {

		wstring w;
		amx_GetStringUnicode( w, cptr );

		switch( params[2] )
		{
			case INVALID :
			default :
				ErrOut("guildMember_setProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		g_nAmxPrintPtr=0;
	  	return 0;
	}
	break;

	default:
		return INVALID;
	}
}

NATIVE2(_guildMember_getProperty)
{

	SERIAL member_serial = params[1];

	P_CHAR pc = pointers::findCharBySerial( member_serial );
	VALIDATEPCR( pc, INVALID );

	P_GUILD_MEMBER member = pc->getGuildMember();
	if ( member==NULL )
	{
		LogError( "guildMember_getProperty called with invalid member %d", member_serial );
		return INVALID;
	}


	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {

	case T_INT: {

		int p;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		cell i = p;
		return i;
	}
	break;

	case T_BOOL: {

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
	break;

	case T_SHORT: {

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
	break;

	case T_CHAR: {

		char p;
		switch(params[2]) {
			case NXW_GMP_C_RANK :				  		//dec value: 100;
				p = member->rank;
				break;
			case NXW_GMP_C_TITLETOGGLE:
				p = member->toggle;
				break;
			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		cell i = p;
		return i;
	}
	break;

	case T_STRING: {

		//we're here so we should pass a string, params[4] is a str ptr

	  	char str[100];
		cell *cptr;
		switch(params[2]) {
			case NXW_GMP_STR_TITLE:
			  	strcpy(str, member->title.c_str());
				break;
			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		amx_GetAddr(amx,params[4],&cptr);
  		amx_SetString(cptr,str, g_nStringMode);

		return strlen(str);
	}
	break;

	case T_UNICODE: {

		wstring* w=NULL;
		switch( params[2] )
		{
			case INVALID :
			default :
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		if( w==NULL ) w=&emptyUnicodeString;
		cell *cptr;
	  	amx_GetAddr(amx,params[4],&cptr);
		amx_SetStringUnicode(cptr, *w );
		return w->length();

	}
	break;

	default:
		return INVALID;
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////// GUILD RECRUIT PROPERTY ///////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

NATIVE2(_guildRecruit_setProperty)
{
	// params[1] = recruit
	// params[2] = property
	// params[3] = subproperty
	// params[4] = value to set property to

	SERIAL recruit_serial = params[1];

	P_CHAR recruiter = pointers::findCharBySerial( recruit_serial );
	VALIDATEPCR( recruiter, INVALID );

	P_GUILD guild = recruiter->getGuild();
	if ( guild == NULL )	return INVALID;

	P_GUILD_RECRUIT recruit = guild->getRecruit( recruit_serial );
	if( recruit==NULL )
	{
		LogError( "guildRecruit_setProperty called with invalid guild %d", recruit_serial );
		return INVALID;
	}


	int tp = getPropertyType(params[2]);

	cell *cptr;
	amx_GetAddr(amx,params[4],&cptr);

	switch( tp ) {

	case T_INT: {
		int p = *cptr;

		switch( params[2] )
		{
			case NXW_GRP_I_RECRUITER:
				//todo
				break;
			default :
				ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
				break;
		}

		return p;
	}
	break;

	case T_BOOL: {

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
	break;

	case T_SHORT: {

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
	break;

	case T_CHAR: {

		char p = static_cast<char>(*cptr & 0xFF);

		switch( params[2] )
		{
			case  INVALID:
			default :
				ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_STRING: {

		//we're here so we should get a ConOut format string, params[4] is the str format

		printstring(amx,cptr,params+5,(int)(params[0]/sizeof(cell))-1);
		g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';
		switch( params[2] )
		{
			case INVALID :			  				//dec value: 450;
			default :
				ErrOut("guild_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		g_nAmxPrintPtr=0;
		return 0;
	}
	break;

	case T_UNICODE: {

		wstring w;
		amx_GetStringUnicode( w, cptr );

		switch( params[2] )
		{
			case INVALID :
			default :
				ErrOut("chr_setProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		g_nAmxPrintPtr=0;
	  	return 0;
	}
	break;

	default:
		return INVALID;
	}
}

NATIVE2(_guildRecruit_getProperty)
{

	SERIAL guild = params[1];
	P_GUILD pGuild = Guildz.getGuild( guild );
	if( pGuild==NULL )
	{
		LogError( "guild_setProperty called with invalid guild %d", guild );
		return INVALID;
	}

	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {

	case T_INT: {

		int p;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		cell i = p;
		return i;
	}
	break;

	case T_BOOL: {

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
	break;

	case T_SHORT: {

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
	break;

	case T_CHAR: {

		char p;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		cell i = p;
		return i;
	}
	break;

	case T_STRING: {

		//we're here so we should pass a string, params[4] is a str ptr

	  	char str[100];
		cell *cptr;
		switch(params[2]) {
			case NXW_GP_STR_NAME:
			  	strcpy(str, pGuild->getName().c_str());
				break;
			case NXW_GP_STR_WEBPAGE:
			  	strcpy(str, pGuild->webpage.c_str());
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
	break;

	case T_UNICODE: {

		wstring* w=NULL;
		switch( params[2] )
		{
			case NXW_GP_UNI_CHARTER :
				w = &pGuild->charter;
				break;
			default :
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		if( w==NULL ) w=&emptyUnicodeString;
		cell *cptr;
	  	amx_GetAddr(amx,params[4],&cptr);
		amx_SetStringUnicode(cptr, *w );
		return w->length();

	}
	break;

	default:
		return INVALID;
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//////////////////////////// MENU PROPERTY /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


NATIVE2(_setMenuProperty)
{
	// params[1] = menu
	// params[2] = property
	// params[3] = subproperty
	// params[4] = value to set property to

	cMenu* menu = (cMenu*)Menus.getMenu( params[1] );
	if( !ISVALIDPM( menu ) )
	{
		LogError( "menu_setProperty called with invalid guild %d", params[1] );
		return INVALID;
	}

	cell* cptr;
	amx_GetAddr(amx,params[4],&cptr);

	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {

	case T_INT: {
		int p = *cptr;

		switch( params[2] )
		{
			case NXW_MP_I_X :
				menu->x = p;
				break;

			case NXW_MP_I_Y :
				menu->y = p;
				break;

			case NXW_MP_I_ID :
				menu->id = static_cast<MENU_TYPE>(p);
				break;

			case NXW_MP_I_BUFFER:
				if( ISVALIDMENUBUFFER( params[3] ) )
					menu->buffer[ params[3] ]=p;
				break;

			default :
				ErrOut("menu_setProperty called with invalid property %d!\n", params[2] );
				break;
		}

		return p;
	}
	break;

	case T_BOOL: {

		bool p = *cptr? true : false;

		switch( params[2] )
		{
			case NXW_MP_B_CLOSEABLE :
				menu->setCloseable( p );
				break;

			case NXW_MP_B_MOVEABLE :
				menu->setMoveable( p );
				break;

			case NXW_MP_B_DISPOSEABLE :
				menu->setDisposeable( p );
				break;

			default :
				ErrOut("menu_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_SHORT: {

		short p = static_cast<short>(*cptr & 0xFFFF);
		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("menu_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_CHAR: {

		char p = static_cast<char>(*cptr & 0xFF);

		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("menu_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_STRING: {

		//we're here so we should get a ConOut format string, params[4] is the str format

		printstring(amx,cptr,params+5,(int)(params[0]/sizeof(cell))-1);
		g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';
		switch( params[2] )
		{
			case NXW_MP_STR_CALLBACK :
				menu->setCallBack( std::string( g_cAmxPrintBuffer ) );
				break;
			case NXW_MP_STR_BUFFER :
				if( ISVALIDMENUBUFFER( params[3] ) )
					menu->buffer_str[ params[3] ] = g_cAmxPrintBuffer;
			default :
				ErrOut("menu_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		g_nAmxPrintPtr=0;
		return 0;
	}
	break;

	case T_UNICODE: {

		wstring w;
		amx_GetStringUnicode( w, cptr );

		switch( params[2] )
		{
			case INVALID :
			default :
				ErrOut("menu_setProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		g_nAmxPrintPtr=0;
	  	return 0;
	}
	break;

	default:
		return INVALID;
	}
}



NATIVE2(_getMenuProperty)
{

	cMenu* menu = (cMenu*)Menus.getMenu( params[1] );
	if( !ISVALIDPM( menu ) )
	{
		LogError( "menu_getProperty called with invalid menu %d", params[1] );
		return INVALID;
	}

	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {

	case T_INT: {

		int p;
		switch(params[2]) {
			case NXW_MP_I_X:
				p = menu->x;
				break;
			case NXW_MP_I_Y:
				p = menu->y;
				break;
			case NXW_MP_I_ID:
				p = menu->id;
				break;
			case NXW_MP_I_BUFFER:
				p = ISVALIDMENUBUFFER( params[3] )? menu->buffer[ params[3] ] : INVALID;
				break;
			default:
				ErrOut("menu_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_BOOL: {

		bool p;
		switch(params[2]) {

			case NXW_MP_B_CLOSEABLE:
				p = menu->getCloseable();
			break;

			case NXW_MP_B_MOVEABLE:
				p = menu->getMoveable();
			break;

			case NXW_MP_B_DISPOSEABLE:
				p = menu->getDisposeable();
			break;

			case NXW_MP_B_RADIO:
				p = menu->getRadio( params[3] );
			break;

			case NXW_MP_B_CHECK:
				p = menu->getCheckBox( params[3] );
			break;

			default:
				ErrOut("guild_getProperty called with invalid property %d!\n", params[2] );
				return false;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_SHORT: {

		short p;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("menu_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_CHAR: {

		char p;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("menu_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_STRING: {

		//we're here so we should pass a string, params[4] is a str ptr

	  	char str[100];
		cell *cptr;
		switch(params[2]) {
			case NXW_MP_I_BUFFER:
				if( !ISVALIDMENUBUFFER( params[3] ) )
					return INVALID;
				else
					strcpy( str, menu->buffer_str[ params[3] ].c_str() );
				break;
			default:
				ErrOut("menu_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}

		amx_GetAddr(amx,params[4],&cptr);
  		amx_SetString(cptr,str, g_nStringMode);

		return strlen(str);
	}
	break;

	case T_UNICODE: {

		wstring* w=NULL;
		switch( params[2] )
		{
			case NXW_MP_UNI_TEXT :
				w = menu->getText( params[3] );
				break;
			default :
				ErrOut("menu_getProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		if( w==NULL ) w=&emptyUnicodeString;
		cell *cptr;
	  	amx_GetAddr(amx,params[4],&cptr);
		amx_SetStringUnicode(cptr, *w );
		return w->length();

	}
	break;

	default:
		return INVALID;
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//////////////////////////// RACE PROPERTY /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


NATIVE2(_getRaceProperty)
{

	Race* race = Race::getRace( params[1] );
	if( race==NULL )
	{
		LogError( "race_getProperty called with invalid race %d", params[1] );
		return INVALID;
	}

	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {

	case T_INT: {

		int p;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("race_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_BOOL: {

		bool p;
		switch(params[2]) {
		case INVALID:
			default:
				ErrOut("race_getProperty called with invalid property %d!\n", params[2] );
				return false;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_SHORT: {

		short p;
		switch(params[2]) {
			case RP_S_SKIN:
				p = ( !race->skinColor.empty() )? race->skinColor[ rand()%race->skinColor.size() ] : 0;
				break;
			default:
				ErrOut("race_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_CHAR: {

		char p;
		switch(params[2]) {
			case RP_C_TYPE:
				p = race->raceType.getValue();
				break;
			case RP_C_LAYER_PERMITTED:
				if( params[2]==LAYER_BEARD )
					p = race->beardPerm;
				else if( params[2]==LAYER_HAIR )
					p = race->hairPerm;
				else
			default:
				ErrOut("race_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_STRING: {

		//we're here so we should pass a string, params[4] is a str ptr

	  	char str[100];
		cell *cptr;
		switch(params[2]) {
			case RP_STR_NAME:
				strcpy( str, race->name.c_str() );
				break;
			case RP_STR_WEBLINK :
				strcpy( str, race->webLink.c_str() );
				break;
			case RP_STR_DESCRIPTION :
				if( params[2] == RP2_DESCRIPTION_COUNT )
					race->description.size();
				else
					race->description[ params[3] ];
			default:
				ErrOut("race_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}

		amx_GetAddr(amx,params[4],&cptr);
  		amx_SetString(cptr,str, g_nStringMode);

		return strlen(str);
	}
	break;

	case T_UNICODE: {

		wstring* w=NULL;
		switch( params[2] )
		{
			case INVALID :
			default :
				ErrOut("race_getProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		if( w==NULL ) w=&emptyUnicodeString;
		cell *cptr;
	  	amx_GetAddr(amx,params[4],&cptr);
		amx_SetStringUnicode(cptr, *w );
		return w->length();

	}
	break;

	default:
		return INVALID;
	}
}

NATIVE2(_getRaceGlobalProp)
{

	VAR_TYPE tp = getPropertyType(params[1]);

	switch( tp ) {

	case T_INT: {

		int p;
		switch(params[1]) {
			case INVALID:
			default:
				ErrOut("race_getGlobalProp called with invalid property %d!\n", params[1] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_BOOL: {

		bool p;
		switch(params[1]) {

			case RP_B_TELEPORT_ON_ENLIST:
				p = Race::teleportOnEnlist;
			break;
			case RP_B_WITH_WEB_INTERFACE:
				p = Race::withWebInterface;
			break;
			default:
				ErrOut("race_getGlobalProp called with invalid property %d!\n", params[1] );
				return false;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_SHORT: {

		short p;
		switch(params[1]) {
			case INVALID:
			default:
				ErrOut("race_getGlobalProp called with invalid property %d!\n", params[1] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_CHAR: {

		char p;
		switch(params[1]) {
			case INVALID:
			default:
				ErrOut("race_getGlobalProp called with invalid property %d!\n", params[1] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_STRING: {

		//we're here so we should pass a string, params[4] is a str ptr

	  	char str[100];
		cell *cptr;
		switch(params[1]) {
			case RP_STR_WEBROOT:
				strcpy( str, Race::globalWebRoot.c_str() );
				break;
			default:
				ErrOut("race_getGlobalProp called with invalid property %d!\n", params[1] );
				return INVALID;
		}

		amx_GetAddr(amx,params[4],&cptr);
  		amx_SetString(cptr,str, g_nStringMode);

		return strlen(str);
	}
	break;

	case T_UNICODE: {

		wstring* w=NULL;
		switch( params[1] )
		{
			case INVALID :
			default :
				ErrOut("race_getGlobalProp called with invalid property %d!\n", params[1] );
				break;
  		}

		if( w==NULL ) w=&emptyUnicodeString;
		cell *cptr;
	  	amx_GetAddr(amx,params[4],&cptr);
		amx_SetStringUnicode(cptr, *w );
		return w->length();

	}
	break;

	default:
		return INVALID;
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//////////////////////////// PARTY PROPERTY /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


NATIVE2( _party_setProperty )
{
	// params[1] = party
	// params[2] = property
	// params[3] = subproperty
	// params[4] = value to set property to

	P_PARTY party = Partys.getParty( params[1] );
	if( party==NULL )
	{
		LogError( "party_setProperty called with invalid party %d", params[1] );
		return INVALID;
	}

	cell* cptr;
	amx_GetAddr(amx,params[4],&cptr);

	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {

	case T_INT: {
		int p = *cptr;

		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("party_setProperty called with invalid property %d!\n", params[2] );
				break;
		}

		return p;
	}
	break;

	case T_BOOL: {

		bool p = *cptr? true : false;

		switch( params[2] )
		{
			case PP_B_CANLOOT : {
				P_PARTY_MEMBER member = party->getMember( params[3] );
				if( member==NULL )
					return p;
				else
					member->canLoot = p;
				break;
			}
			default :
				ErrOut("party_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_SHORT: {

		short p = static_cast<short>(*cptr & 0xFFFF);
		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("party_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_CHAR: {

		char p = static_cast<char>(*cptr & 0xFF);

		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("party_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		return p;
	}
	break;

	case T_STRING: {

		//we're here so we should get a ConOut format string, params[4] is the str format

		printstring(amx,cptr,params+5,(int)(params[0]/sizeof(cell))-1);
		g_cAmxPrintBuffer[qmin(g_nAmxPrintPtr,48)] = '\0';
		switch( params[2] )
		{
			case INVALID:
			default :
				ErrOut("party_setProperty called with invalid property %d!\n", params[2] );
				break;
		}
		g_nAmxPrintPtr=0;
		return 0;
	}
	break;

	case T_UNICODE: {

		wstring w;
		amx_GetStringUnicode( w, cptr );

		switch( params[2] )
		{
			case INVALID :
			default :
				ErrOut("party_setProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		g_nAmxPrintPtr=0;
	  	return 0;
	}
	break;

	default:
		return INVALID;
	}
}



NATIVE2( _party_getProperty )
{

	// params[1] = party
	// params[2] = property
	// params[3] = subproperty

	P_PARTY party = Partys.getParty( params[1] );
	if( party==NULL )
	{
		LogError( "party_getProperty called with invalid party %d", params[1] );
		return INVALID;
	}

	VAR_TYPE tp = getPropertyType(params[2]);

	switch( tp ) {

	case T_INT: {

		int p;
		switch(params[2]) {
			case PP_I_LEADER: {
				p = party->getLeader();
				break;
			}
			default:
				ErrOut("party_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_BOOL: {

		bool p;
		switch(params[2]) {

			case PP_B_CANLOOT: {
				P_PARTY_MEMBER member = party->getMember( params[3] );
				p = (member!=NULL)? member->canLoot : false;
			}
			break;

			default:
				ErrOut("party_getProperty called with invalid property %d!\n", params[2] );
				return false;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_SHORT: {

		short p;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("party_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_CHAR: {

		char p;
		switch(params[2]) {
			case PP_C_MEMBERS:
				p=party->members.size();
				break;
			case PP_C_CANDIDATES:
				p=party->candidates.size();
				break;
			default:
				ErrOut("party_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}
		return static_cast<cell>(p);
	}
	break;

	case T_STRING: {

		//we're here so we should pass a string, params[4] is a str ptr

	  	char str[100];
		cell *cptr;
		switch(params[2]) {
			case INVALID:
			default:
				ErrOut("party_getProperty called with invalid property %d!\n", params[2] );
				return INVALID;
		}

		amx_GetAddr(amx,params[4],&cptr);
  		amx_SetString(cptr,str, g_nStringMode);

		return strlen(str);
	}
	break;

	case T_UNICODE: {

		wstring* w=NULL;
		switch( params[2] )
		{
			case INVALID :
			default :
				ErrOut("party_getProperty called with invalid property %d!\n", params[2] );
				break;
  		}

		if( w==NULL ) w=&emptyUnicodeString;
		cell *cptr;
	  	amx_GetAddr(amx,params[4],&cptr);
		amx_SetStringUnicode(cptr, *w );
		return w->length();

	}
	break;

	default:
		return INVALID;
	}
}
