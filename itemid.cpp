  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file itemid.cpp
\brief functions that wrap the checking for a list of item IDs
\author Duke
\note cut from various cpps and 'restyled' by Duke, 24/02/01
\note this file should contain ONLY THAT type of functions, feel free to collect more
*/
#include "nxwcommn.h"
#include "itemid.h"
#include "debug.h"
#include "npcai.h"
#include "scp_parser.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "scripts.h"

WEAPONINFOMAP weaponinfo;


void loadweaponsinfo()
{

    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];
	SI32 id=INVALID;
	int type=SWORD1H;
	
	int loopexit=0;
	do
	{
		safedelete(iter);
		iter = Scripts::WeaponInfo->getNewIterator("SECTION WEAPONTYPE %i", type );
		if( iter==NULL ) continue;

		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if (!strcmp("ID", script1)) {		  
					id = str2num(script2);
					weaponinfo[id]=static_cast<WEAPONTYPE>(type);
				}
			}

		}
        while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

		type++;
    }
	while ( (strcmp("EOF", script1)) && (++loopexit < MAXLOOPS) );

    safedelete(iter);

}

bool isWeaponLike( SI32 id, WEAPONTYPE type1, WEAPONTYPE type2, WEAPONTYPE type3 )
{
	WEAPONINFOMAP::iterator iter( weaponinfo.find( id ) );
	if( iter==weaponinfo.end() )
		return false;
	else {
		return ( iter->second == type1 ) || ( iter->second==type2 ) || ( iter->second==type3 );
	}
}

bool cItem::IsSword()
{
	return isWeaponLike( this->getId(), SWORD1H );
}

bool cItem::IsAxe()
{
	return isWeaponLike( this->getId(), AXE1H);
}

bool cItem::IsSwordType()
{
	return isWeaponLike( this->getId(), SWORD1H, AXE1H );
}

bool cItem::IsMace1H()
{

	return isWeaponLike( this->getId(), MACE1H);

}

bool cItem::IsMace2H()
{
	return isWeaponLike( this->getId(), MACE2H);

}

bool cItem::IsMaceType()
{
	return isWeaponLike( this->getId(), MACE1H, MACE2H );
}

bool cItem::IsFencing1H()
{
	return isWeaponLike( this->getId(), FENCING1H);

}

bool cItem::IsFencing2H()
{
	return isWeaponLike( this->getId(), FENCING2H);
}

bool cItem::IsFencingType()
{
	return isWeaponLike( this->getId(), FENCING1H, FENCING2H );
}

bool cItem::IsBow()
{
	return isWeaponLike( this->getId(), BOW );
}
bool cItem::IsCrossbow()	
{
	return isWeaponLike( this->getId(), CROSSBOW );

}

bool cItem::IsHeavyCrossbow()
{
	return isWeaponLike( this->getId(), HEAVYCROSSBOW );

}

bool cItem::IsBowType()
{
	return isWeaponLike( this->getId(), BOW, CROSSBOW, HEAVYCROSSBOW );
}

bool cItem::IsStave() // -Fraz- added
{
	return isWeaponLike( this->getId(), STAVE1H, STAVE2H );

}

bool cItem::IsSpecialMace() // -Fraz- The OSI macing weapons that do stamina and armor damage 2handed only
{
	return isWeaponLike( this->getId(), STAVE1H, STAVE2H, MACE2H );
}

SI08 cItem::isFieldSpellItem() //LB
{
	int a= 0;


	if( (getId()==0x3996) || (getId()==0x398C) ) a= 1; // fire field
	if( (getId()==0x3915) || (getId()==0x3920) ) a= 2; // poison field
	if( (getId()==0x3979) || (getId()==0x3967) ) a= 3; // paralyse field
	if( (getId()==0x3956) || (getId()==0x3946) ) a= 4; // energy field;

	return a;
}


bool cItem::IsDagger()
{
    return ( getId()==0x0F51 || getId()==0x0F52 );
}

bool itemById::IsCorpse( UI16 id )
{
	return (id==0x2006);
}

bool cItem::IsCorpse() { return itemById::IsCorpse( this->getId() ); }

bool itemById::IsTree( UI16 id )	// this is used in AxeTarget()
{
	if (id==0x0CD0 || id==0x0CD3 || id==0x0CD6 ||
		id==0x0CD8 || id==0x0CDA || id==0x0CDD ||
		id==0x0CE0 || id==0x0CE3 || id==0x0CE6 ||
		(id>=0x0CCA && id<=0x0CCD) ||
		(id>=0x12B8 && id<=0x12BB) ||
		id==0x0D42 || id==0x0D43 || id==0x0D58 ||
		id==0x0D59 || id==0x0D70 || id==0x0D85 ||
		id==0x0D94 || id==0x0D95 || id==0x0D98 ||
		id==0x0Da4 || id==0x0Da8 || id==0x0D58)
		return true;
	return false;
}

bool cItem::IsTree() { return itemById::IsTree( this->getId() ); }

bool itemById::IsTree2( UI16 id )	// this is used in SwordTarget() to give kindling. Donno why it's different
{
	if (id==0x0CD0 || id==0x0CD3 || id==0x0CD6 ||
		id==0x0CD8 || id==0x0CDA || id==0x0CDD ||
		id==0x0CE0 || id==0x0CE3 || id==0x0CE6 ||
		(id>=0x0CCA && id<=0x0CCD) ||
		(id>=0x12B8 && id<=0x12BB) )
		return true;
	return false;
}

bool cItem::IsTree2() { return itemById::IsTree2( this->getId() ); }

bool cItem::IsSpellScroll()
{
	return (getId()>0x1F2C && getId()<0x1F6D);
}

bool cItem::IsSpellScroll72()
{
	return (getId() >= 0x1F2D && getId() <= 0x1F72);
}

bool cItem::IsInstrument()
{

	return (getId()==0x0E9C || getId()==0x0E9D || getId()==0x0E9E || getId()==0x0EB1 || getId()==0x0EB2 || getId()==0x0EB3 || getId()==0x0EB4);
}

bool cItem::IsArrow()
{
	return ((getId()==0x0F3F)||(getId()==0x0F42));
}

bool cItem::IsBolt()
{
	return ((getId()==0x1BFB)||(getId()==0x1BFE));
}


bool cItem::IsChaosOrOrderShield()
{
	return (getId()>=0x1BC3 && getId()<=0x1BC5);
}

bool cItem::IsShield()
{
	return ((getId()>=0x1B72 && getId()<=0x1B7B) || IsChaosOrOrderShield());
}

bool itemById::IsLog( UI16 id )
{
	return ( id>=0x1BDD && id<=0x1BE2 );
}

bool cItem::IsLog() { return itemById::IsLog( this->getId()); }


bool itemById::IsShaft( UI16 id )
{
	return ( id>=0x1BD4 && id<=0x1BD6 );
}

bool cItem::IsShaft() { return itemById::IsShaft( this->getId() ); }


bool cItem::IsBoard()
{
	return ( getId()>=0x1BD7 && getId()<=0x1BDC );
}

bool itemById::IsFeather( UI16 id )
{
	return ( id>=0x1BD1 && id<=0x1BD3 );
}

bool cItem::IsFeather() { return itemById::IsFeather( this->getId() ); }

bool cItem::IsCutLeather()
{
	return ( getId()==0x1067 || getId()==0x1068 || getId()==0x1081 || getId()==0x1082 );
}

bool cItem::IsHide()
{
	return ( getId()==0x1078 || getId()==0x1079 );
}

bool cItem::IsForge()
{
	return ( getId()==0x0FB1 || ( getId()>=0x197A && getId()<=0x19A9 ) );
}

bool cItem::IsAnvil()
{
	return ( getId()==0x0FAF || getId()==0x0FB0 );
}

bool cItem::IsCookingPlace()
{
	UI16 id= this->getId();
	return ((id>=0x0DE3 && id<=0x0DE9)||
			(id==0x0FAC || id==0x0FB1)||
			(id>=0x197A && id<=0x19B6)||
			(id>=0x0461 && id<=0x0480)||
			(id>=0x0E31 && id<=0x0E33)||
			(id==0x19BB || id==0x1F2B)||
			(id>=0x092B && id<=0x0934)||
			(id>=0x0937 && id<=0x0942)||
			(id>=0x0945 && id<=0x0950)||
			(id>=0x0953 && id<=0x095E)||
			(id>=0x0961 && id<=0x096C) );
}

bool cItem::IsCutCloth()
{
	return ( getId()>=0x1766 && getId()<=0x1768 );
}

bool cItem::IsCloth()
{
	return ((getId()>=0x175D && getId()<=0x1765)|| IsCutCloth() );
}

bool cItem::IsBoltOfCloth()
{
	return ( getId()>=0x0F95 && getId()<=0x0F9C );
}

bool cItem::IsChest()
{
	UI16 id= this->getId();
	return (( id>=0x09A8 && id<=0x09AB) ||
		    ( id>=0x0E40 && id<=0x0E43) ||
		    ( id==0x0E7C) || (id==0x0E7D) ||
		    ( id==0x0E80));
}

bool cItem::IsFishWater() // -Fraz- needed water tiles for fishing
{
	UI16 id= this->getId();
	return ((id==0x1798) || (id==0x179B) ||
			(id==0x179C) || (id==0x1799) ||
			(id==0x1797) || (id==0x179A) ||
			(id==0x346E) || (id==0x346F) ||
			(id>=0x3470) && (id<=0x3485) ||
			(id>=0x3494) && (id<=0x349F) ||
			(id>=0x34A0) && (id<=0x34AB) ||
			(id>=0x34B8) && (id<=0x34BB) ||
			(id>=0x34BD) && (id<=0x34BF) ||
			(id>=0x34C0) || (id<=0x34C2) ||
			(id>=0x34C3) && (id<=0x34C5) ||
			(id>=0x34C7) && (id<=0x34CA) ||
			(id>=0x34D1) && (id<=0x34D5));
}

bool cItem::IsFish()
{
	return ( getId()>=0x09CC && getId()<=0x09CF );
}


bool cItem::IsSign() 
{ 
	UI16 id= this->getId();
	return ((id==0x0B95 || id==0x0B96)|| (id>=0x0BA3 && id<=0x0C0E)|| (id==0x0C43 || id==0x0C44)); 
} 

bool cItem::IsWoodenSign() 
{ 
	return ((getId()==0x0BCF || getId()==0x0BD0)); 
} 

bool cItem::IsBrassSign() 
{ 
	return ((getId()==0x0BD1 || getId()==0x0BD2)); 
}

bool cItem::IsGuildSign() 
{ 
	return ((getId() >= 0x0BD3 && getId() <= 0x0C0A)); 
} 

bool cItem::IsTradeSign() 
{ 
	UI16 id= this->getId();
	return ((id==0x0B95 || id==0x0B96) || 
			(id >= 0x0BA3 && id <= 0x0BCE)); 
} 

bool cItem::IsBankSign() 
{ 
	return ((getId() >= 0x0C0B && getId() <= 0x0C0C)); 
} 

bool cItem::IsTheatreSign() 
{ 
	return ((getId() >= 0x0C0D && getId() <= 0x0C0E)); 
} 


bool itemById::IsHouse( UI16 id )
{

  if (id < 0x0040) return false;

  if ( (id >= 0x0064) && (id <= 0x007f) ) return true;

  switch(id%256)
  {
	case 0x87:
	case 0x8c:
	case 0x8d:
	case 0x96:
	case 0x98:
	case 0x9a:
	case 0x9c:
	case 0x9e:
	case 0xa0:
	case 0xa2:
		return true;
  }

  if( id==0x0bb8 || id==0x1388 )
	  return true;

  return false;
}

bool cItem::IsHouse()
{
    return itemById::IsHouse( this->getId() );
}

/*!
\author Luxor
\brief Tells if an item is a dynamic spawner
*/
LOGICAL cItem::isSpawner()
{
	switch(type)
	{
		case 61:
		case 62:
		case 63:
		case 64:
		case 65:
		case 69:
		case 125:
			return true;
		default:
			return false;
	}
	return false;
}


void itemGetPopUpHelp(char *str, P_ITEM pi)
{
//	int id = (pi->id1<<8)+pi->id2;
	int type = pi->type;
	if (pi->IsInstrument()) 
		sprintf(str, TRANSLATE("This item is a musical instrument. You can use it for bardic skills like enticement or provocation"));	
	else if (type == ITYPE_DOOR) 
		sprintf(str, TRANSLATE("This a door. To open or close it, double click on it."));
	else if (type == ITYPE_LOCKED_DOOR) 
		sprintf(str, TRANSLATE("This a locked door. To open or close it, click on the proper key and target it."));
	else if (type == ITYPE_FOOD)
		sprintf(str, TRANSLATE("This food you can eat when you're hungry. To eat, double click the food, but beware poisoned food!"));
	else if (type == ITYPE_RUNE)
		sprintf(str, TRANSLATE("This is a rune for use with recall, mark and gate travel spells"));
	else if (type == ITYPE_RESURRECT)
		sprintf(str, TRANSLATE("If you dye (or are dead) you can double click this item to resurrect!"));
	else if (type == ITYPE_KEY)
		sprintf(str, TRANSLATE("This is a key you can use (double click) to open doors"));
	else if (type == ITYPE_SPELLBOOK)
		sprintf(str, TRANSLATE("This is the spellbook, where you can write down your own spells for later use"));
	else if (type == ITYPE_POTION)
		sprintf(str, TRANSLATE("This is a potion! You can drink that when you need its effects... but beware of poison potions!"));
}

void charGetPopUpHelp(char *str, P_CHAR pc)
{
	if (!pc->npc) {
		sprintf(str, TRANSLATE("He/She is a player like you. You've met new people!!"));	

		if (pc->IsCounselor()) {
			sprintf(str, TRANSLATE("He/She is a Counselor. You can ask him if you need help on the game"));	
		}
		
		if (pc->IsGM()) {
			sprintf(str, TRANSLATE("He/She is a Game Master. You can ask his help if you're stuck or have bugs or other problems"));	
		}
	}
	else {
		if (pc->npcaitype==NPCAI_PLAYERVENDOR)
			sprintf(str, TRANSLATE("He/She is a vendor which sells items owned by a player. Good shopping!"));
		else if (pc->npcaitype==NPCAI_EVIL)
			sprintf(str, TRANSLATE("Run AWAY!!! It will kill you!!"));
	}
}
