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

LOGICAL cItem::IsSword() const
{
	return isWeaponLike( id(), SWORD1H );
}

LOGICAL cItem::IsAxe() const
{
	return isWeaponLike( id(), AXE1H);
}

LOGICAL cItem::IsSwordType() const
{
	return isWeaponLike( id(), SWORD1H, AXE1H );
}

LOGICAL cItem::IsMace1H() const
{

	return isWeaponLike( id(), MACE1H);

}

LOGICAL cItem::IsMace2H() const
{
	return isWeaponLike( id(), MACE2H);

}

LOGICAL cItem::IsMaceType() const
{
	return isWeaponLike( id(), MACE1H, MACE2H );
}

LOGICAL cItem::IsFencing1H() const
{
	return isWeaponLike( id(), FENCING1H);
}

LOGICAL cItem::IsFencing2H() const
{
	return isWeaponLike( id(), FENCING2H);
}

LOGICAL cItem::IsFencingType() const
{
	return isWeaponLike( id(), FENCING1H, FENCING2H );
}

LOGICAL cItem::IsBow() const
{
	return isWeaponLike( id(), BOW );
}
LOGICAL cItem::IsCrossbow() const	
{
	return isWeaponLike( id(), CROSSBOW );

}

LOGICAL cItem::IsHeavyCrossbow() const
{
	return isWeaponLike( id(), HEAVYCROSSBOW );

}

LOGICAL cItem::IsBowType() const
{
	return isWeaponLike( id(), BOW, CROSSBOW, HEAVYCROSSBOW );
}

LOGICAL cItem::IsStave() const // -Fraz- added
{
	return isWeaponLike( id(), STAVE1H, STAVE2H );

}

LOGICAL cItem::IsSpecialMace() const // -Fraz- The OSI macing weapons that do stamina and armor damage 2handed only
{
	return isWeaponLike( id(), STAVE1H, STAVE2H, MACE2H );
}

SI08 cItem::isFieldSpellItem() const //LB
{
	int a= 0;

	if( (id()==0x3996) || (id()==0x398C) ) a= 1; // fire field
	if( (id()==0x3915) || (id()==0x3920) ) a= 2; // poison field
	if( (id()==0x3979) || (id()==0x3967) ) a= 3; // paralyse field
	if( (id()==0x3956) || (id()==0x3946) ) a= 4; // energy field;

	return a;
}


LOGICAL cItem::IsDagger() const
{
    return ( id()==0x0F51 || id()==0x0F52 );
}

bool itemById::IsCorpse( UI16 id )
{
	return (id==0x2006);
}

LOGICAL cItem::IsCorpse() const
{
	return itemById::IsCorpse( id() );
}

bool itemById::IsTree( UI16 id )	// this is used in AxeTarget()
{
	return (id==0x0CD0 || id==0x0CD3 || id==0x0CD6 ||
		id==0x0CD8 || id==0x0CDA || id==0x0CDD ||
		id==0x0CE0 || id==0x0CE3 || id==0x0CE6 ||
		(id>=0x0CCA && id<=0x0CCD) ||
		(id>=0x12B8 && id<=0x12BB) ||
		id==0x0D42 || id==0x0D43 || id==0x0D58 ||
		id==0x0D59 || id==0x0D70 || id==0x0D85 ||
		id==0x0D94 || id==0x0D95 || id==0x0D98 ||
		id==0x0Da4 || id==0x0Da8 || id==0x0D58);
}

LOGICAL cItem::IsTree() const
{
	return itemById::IsTree( id() );
}

bool itemById::IsTree2( UI16 id )	// this is used in SwordTarget() to give kindling. Donno why it's different
{
	return (id==0x0CD0 || id==0x0CD3 || id==0x0CD6 ||
		id==0x0CD8 || id==0x0CDA || id==0x0CDD ||
		id==0x0CE0 || id==0x0CE3 || id==0x0CE6 ||
		(id>=0x0CCA && id<=0x0CCD) ||
		(id>=0x12B8 && id<=0x12BB) );
}

LOGICAL cItem::IsTree2() const
{
	return itemById::IsTree2( id() );
}

LOGICAL cItem::IsSpellScroll() const
{
	return (id()>0x1F2C && id()<0x1F6D);
}

LOGICAL cItem::IsSpellScroll72() const
{
	return (id() >= 0x1F2D && id() <= 0x1F72);
}

LOGICAL cItem::IsInstrument() const
{
	return (id()==0x0E9C || id()==0x0E9D || id()==0x0E9E || id()==0x0EB1 || id()==0x0EB2 || id()==0x0EB3 || id()==0x0EB4);
}

LOGICAL cItem::IsArrow() const
{
	return ((id()==0x0F3F)||(id()==0x0F42));
}

LOGICAL cItem::IsBolt() const
{
	return ((id()==0x1BFB)||(id()==0x1BFE));
}


LOGICAL cItem::IsChaosOrOrderShield() const
{
	return (id()>=0x1BC3 && id()<=0x1BC5);
}

LOGICAL cItem::IsShield() const
{
	return ((id()>=0x1B72 && id()<=0x1B7B) || IsChaosOrOrderShield());
}

bool itemById::IsLog( UI16 id )
{
	return ( id>=0x1BDD && id<=0x1BE2 );
}

LOGICAL cItem::IsLog() const
{
	return itemById::IsLog( id() );
}

bool itemById::IsShaft( UI16 id )
{
	return ( id>=0x1BD4 && id<=0x1BD6 );
}

LOGICAL cItem::IsShaft() const
{
	return itemById::IsShaft( id() );
}


LOGICAL cItem::IsBoard() const
{
	return ( id()>=0x1BD7 && id()<=0x1BDC );
}

bool itemById::IsFeather( UI16 id )
{
	return ( id>=0x1BD1 && id<=0x1BD3 );
}

LOGICAL cItem::IsFeather() const
{
	return itemById::IsFeather( id() );
}

LOGICAL cItem::IsCutLeather() const
{
	return ( id()==0x1067 || id()==0x1068 || id()==0x1081 || id()==0x1082 );
}

LOGICAL cItem::IsHide() const
{
	return ( id()==0x1078 || id()==0x1079 );
}

LOGICAL cItem::IsForge() const
{
	return ( id()==0x0FB1 || ( id()>=0x197A && id()<=0x19A9 ) );
}

LOGICAL cItem::IsAnvil() const
{
	return ( id()==0x0FAF || id()==0x0FB0 );
}

LOGICAL cItem::IsCookingPlace() const
{
	return ((id()>=0x0DE3 && id()<=0x0DE9)||
			(id()==0x0FAC || id()==0x0FB1)||
			(id()>=0x197A && id()<=0x19B6)||
			(id()>=0x0461 && id()<=0x0480)||
			(id()>=0x0E31 && id()<=0x0E33)||
			(id()==0x19BB || id()==0x1F2B)||
			(id()>=0x092B && id()<=0x0934)||
			(id()>=0x0937 && id()<=0x0942)||
			(id()>=0x0945 && id()<=0x0950)||
			(id()>=0x0953 && id()<=0x095E)||
			(id()>=0x0961 && id()<=0x096C) );
}

LOGICAL cItem::IsCutCloth() const
{
	return ( id()>=0x1766 && id()<=0x1768 );
}

LOGICAL cItem::IsCloth() const
{
	return ((id()>=0x175D && id()<=0x1765)|| IsCutCloth() );
}

LOGICAL cItem::IsBoltOfCloth() const
{
	return ( id()>=0x0F95 && id()<=0x0F9C );
}

LOGICAL cItem::IsChest() const
{
	return (( id()>=0x09A8 && id()<=0x09AB) ||
		    ( id()>=0x0E40 && id()<=0x0E43) ||
		    ( id()==0x0E7C) || (id()==0x0E7D) ||
		    ( id()==0x0E80));
}

LOGICAL cItem::IsFishWater() const // -Fraz- needed water tiles for fishing
{
	return ((id()==0x1798) || (id()==0x179B) ||
			(id()==0x179C) || (id()==0x1799) ||
			(id()==0x1797) || (id()==0x179A) ||
			(id()==0x346E) || (id()==0x346F) ||
			(id()>=0x3470) && (id()<=0x3485) ||
			(id()>=0x3494) && (id()<=0x349F) ||
			(id()>=0x34A0) && (id()<=0x34AB) ||
			(id()>=0x34B8) && (id()<=0x34BB) ||
			(id()>=0x34BD) && (id()<=0x34BF) ||
			(id()>=0x34C0) || (id()<=0x34C2) ||
			(id()>=0x34C3) && (id()<=0x34C5) ||
			(id()>=0x34C7) && (id()<=0x34CA) ||
			(id()>=0x34D1) && (id()<=0x34D5));
}

LOGICAL cItem::IsFish() const
{
	return ( id()>=0x09CC && id()<=0x09CF );
}


LOGICAL cItem::IsSign() const 
{ 
	return ((id()==0x0B95 || id()==0x0B96)|| (id()>=0x0BA3 && id()<=0x0C0E)|| (id()==0x0C43 || id()==0x0C44)); 
} 

LOGICAL cItem::IsWoodenSign() const 
{ 
	return ((id()==0x0BCF || id()==0x0BD0)); 
} 

LOGICAL cItem::IsBrassSign() const 
{ 
	return ((id()==0x0BD1 || id()==0x0BD2)); 
}

LOGICAL cItem::IsGuildSign() const 
{ 
	return ((id() >= 0x0BD3 && id() <= 0x0C0A)); 
} 

LOGICAL cItem::IsTradeSign() const 
{ 
	return ((id()==0x0B95 || id()==0x0B96) || 
			(id() >= 0x0BA3 && id() <= 0x0BCE)); 
} 

LOGICAL cItem::IsBankSign() const 
{ 
	return ((id() >= 0x0C0B && id() <= 0x0C0C)); 
} 

LOGICAL cItem::IsTheatreSign() const 
{ 
	return ((id() >= 0x0C0D && id() <= 0x0C0E)); 
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

LOGICAL cItem::IsHouse() const
{
    return itemById::IsHouse( id() );
}

/*!
\author Luxor
\brief Tells if an item is a dynamic spawner
*/
LOGICAL cItem::isSpawner() const
{
	return type == 61 || type == 62 || type == 63 || type == 64 || type == 65 || type == 69 || type == 125;
}


void itemGetPopUpHelp(char *str, P_ITEM pi)
{
	if (pi->IsInstrument()) 
		sprintf(str, TRANSLATE("This item is a musical instrument. You can use it for bardic skills like enticement or provocation"));	
	else if (pi->type == ITYPE_DOOR) 
		sprintf(str, TRANSLATE("This a door. To open or close it, double click on it."));
	else if (pi->type == ITYPE_LOCKED_DOOR) 
		sprintf(str, TRANSLATE("This a locked door. To open or close it, click on the proper key and target it."));
	else if (pi->type == ITYPE_FOOD)
		sprintf(str, TRANSLATE("This food you can eat when you're hungry. To eat, double click the food, but beware poisoned food!"));
	else if (pi->type == ITYPE_RUNE)
		sprintf(str, TRANSLATE("This is a rune for use with recall, mark and gate travel spells"));
	else if (pi->type == ITYPE_RESURRECT)
		sprintf(str, TRANSLATE("If you dye (or are dead) you can double click this item to resurrect!"));
	else if (pi->type == ITYPE_KEY)
		sprintf(str, TRANSLATE("This is a key you can use (double click) to open doors"));
	else if (pi->type == ITYPE_SPELLBOOK)
		sprintf(str, TRANSLATE("This is the spellbook, where you can write down your own spells for later use"));
	else if (pi->type == ITYPE_POTION)
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

