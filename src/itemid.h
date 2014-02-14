  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef _ITEMID_H
#define _ITEMID_H

/*!
\brief Items By Id
*/
namespace itemById {
	bool IsTree( UI16 id );
	bool IsTree2( UI16 id );
	bool IsCorpse( UI16 id );
	bool IsShaft( UI16 id );
	bool IsFeather( UI16 id );
	bool IsLog( UI16 id );
	bool IsHouse( UI16 id );
};

typedef enum {
	WEAPON_INVALID=INVALID,
	SWORD1H=0,
	SWORD2H,
	AXE1H,
	AXE2H,
	MACE1H,
	MACE2H,
	FENCING1H,
	FENCING2H,
	STAVE1H,
	STAVE2H,
	BOW,
	CROSSBOW,
	HEAVYCROSSBOW,
} WEAPONTYPE;

typedef std::map<SI32,WEAPONTYPE> WEAPONINFOMAP;

void loadweaponsinfo();

bool isWeaponLike( SI32 id, WEAPONTYPE type1, WEAPONTYPE type2=WEAPON_INVALID, WEAPONTYPE type3=WEAPON_INVALID );

#endif
