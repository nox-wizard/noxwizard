  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file
\brief Header defining cItem class
*/
#ifndef __ITEMS_H
#define __ITEMS_H

#include "amx/amxcback.h"
#include "object.h"
#include "magic.h"
#include "item_array.h"
#include "item.h"

/*!
\author Luxor
\brief AMX events for items
*/
enum AmxItemEvents {
	EVENT_IONSTART = 0,
        EVENT_IONDAMAGE,
        EVENT_IONEQUIP,
        EVENT_IONUNEQUIP,
        EVENT_IONCLICK,
        EVENT_IONDBLCLICK,
        EVENT_IPUTINBACKPACK,
        EVENT_IDROPINLAND,
        EVENT_IONCHECKCANUSE,
        EVENT_IONTRANSFER,
        EVENT_IONSTOLEN,
        EVENT_IONPOISONED,
        EVENT_IONDECAY,
        EVENT_IONREMOVETRAP,
        EVENT_IONLOCKPICK,
        EVENT_IONWALKOVER,
        EVENT_IONPUTITEM,
        EVENT_ITAKEFROMCONTAINER,
        ALLITEMEVENTS
};

//! deletion type
enum DelType
{
	DELTYPE_UNKNOWN,
	DELTYPE_DECAY,
	DELTYPE_GMREMOVE
};

/*!
\brief Item class
*/
class cItem : public cObject
{
public:
        /*!
        \brief Redefinition of = operator for cItem class
        \author Luxor
        \since 0.82rc3
        */
        cItem& operator=(cItem& b);

        static void	archive();
	static void	safeoldsave();

public:
	cItem(SERIAL serial);

//@{
/*!
\name Who is
*/
	AmxEvent	*amxevents[ALLITEMEVENTS];

	SI32		hp;	//!< Number of hit points an item has.
	SI32		maxhp;	//!< Max number of hit points an item can have.

	void		Refresh();
	const char*	getRealItemName();

	SI32		getName(char* itemname);
	
//@}

//@{
/*!
\name Look
*/
	inline SI16 id() const
	{ return (SI16)((id1<<8)|id2); }

	inline SI16 animid() const
	{ return ( animid1 && animid2 ) ? (SI16)((animid1<<8)|animid2) : id(); }

	UI08	id1;			//!< Item visuals as stored in the client
	UI08	id2;
	void	setId(SI16 id);
	SI08	isFieldSpellItem() const;

	LOGICAL IsCorpse() const;
	LOGICAL IsSpellScroll() const;	//!< predefined spells
	LOGICAL IsSpellScroll72() const;//!< includes the scrolls with a variable name
	LOGICAL IsTree() const;		//!< this is used in AxeTarget()
	LOGICAL IsTree2() const;	//!< this is used in SwordTarget() to give kindling.
	LOGICAL IsInstrument() const;

	LOGICAL IsAxe() const;
	LOGICAL IsSword() const;
	LOGICAL IsSwordType() const;
	LOGICAL IsMace1H() const;
	LOGICAL IsMace2H() const;
	LOGICAL IsMaceType() const;
	LOGICAL IsFencing1H() const;
	LOGICAL IsFencing2H() const;
	LOGICAL IsFencingType() const;
	LOGICAL IsBow() const;
	LOGICAL IsCrossbow() const;
	LOGICAL IsHeavyCrossbow() const;
	LOGICAL IsBowType() const;
	LOGICAL IsArrow() const;
	LOGICAL IsBolt() const;
	LOGICAL IsStave() const;
	LOGICAL IsSpecialMace() const;
	LOGICAL IsChaosOrOrderShield() const;
	LOGICAL IsShield() const;

	LOGICAL IsLog() const;
	LOGICAL IsShaft() const;
	LOGICAL IsBoard() const;
	LOGICAL IsFeather() const;
	LOGICAL IsCutLeather() const;
	LOGICAL IsHide() const;
	LOGICAL IsBoltOfCloth() const;
	LOGICAL IsCutCloth() const;
	LOGICAL IsCloth() const;
	LOGICAL IsChest() const;

	LOGICAL IsForge() const;
	LOGICAL IsAnvil() const;
	LOGICAL IsCookingPlace() const;
	LOGICAL IsDagger() const;

	LOGICAL IsFish() const;
	LOGICAL IsFishWater() const;

	LOGICAL IsSign() const;
	LOGICAL IsBrassSign() const;
	LOGICAL IsWoodenSign() const;
	LOGICAL IsGuildSign() const;
	LOGICAL IsTradeSign() const;
	LOGICAL IsBankSign() const;
	LOGICAL IsTheatreSign() const;
	LOGICAL IsHouse() const;
	LOGICAL isSpawner() const;

	UI08		animid1;	//!< elcabesa animation
	UI08		animid2;	//!< elcabesa animation
	void		animSetId(SI16 id);	//!< elcabesa animation
	LOGICAL		useAnimId;

	UI08		color1;		//!< Hue
	UI08		color2;		//!< Hue

	inline SI16 color()
	{ return (SI16)((color1<<8)|color2); }

	void	setColor(UI16 color);

	SI08		layer;		//!< Layer if equipped on paperdoll
	SI08		oldlayer;	//!< Old layer - used for bouncing bugfix - AntiChrist
	SI08		scriptlayer;	//!< Luxor, for scripted setted Layer

	SI08		magic;		//!< 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable.
	SI08		visible;	//!< 0=Normally Visible, 1=Owner & GM Visible, 2=GM Visible
	SI16		dir;
//@}

//@{
/*!
\name Container
*/
private:
	Serial 		contserial;
	Serial		oldcontserial;

public:
	SERIAL		getContSerial(LOGICAL old= false) const;
	UI08		getContSerialByte(UI08 nByte, LOGICAL old= false) const;
	PC_OBJECT	getContainer() const;

	void		setCont(P_OBJECT obj);
	void		setContSerial(SI32 serial, LOGICAL old= false, LOGICAL update= true);
	void		setContSerialByte(UI32 nByte, BYTE value, LOGICAL old= false);
	LOGICAL		isContainer() const { return type==1 || type==12 || type==63 || type==8 || type==13 || type==64; }
		//!< check if item is a container
	LOGICAL		isSecureContainer() const { return type==8 || type==13 || type==64; }
		//!< check if item is a secure container
	void		SetRandPosInCont(P_ITEM pCont);
	bool		ContainerPileItem( P_ITEM pItem ); //!< try to find an item in the container to stack with
	SI32		secureIt; //!< secured chests
	LOGICAL		AddItem(P_ITEM pItem, short xx=-1, short yy=-1);	//!< Add Item to container
	SI32		DeleteAmountByID(int amount, unsigned int scriptID);
	SI16		getContGump();
	void		putInto( P_ITEM pi );
//@}

//@{
/*!
\name Extra Info (mores)
*/
	UI08		more1;
	UI08		more2;
	UI08		more3;
	UI08		more4;
	SI08		moreb1;
	SI08		moreb2;
	SI08		moreb3;
	SI08		moreb4;
	UI32		morex;
	UI32		morey;
	UI32		morez;
//@}

//@{
/*!
\name Amount
*/
	UI16		amount;		//!< Amount of items in pile
	UI16		amount2;	//!< Used to track things like number of yards left in a roll of cloth
	SI32		ReduceAmount(const short amount);
	SI32		IncreaseAmount(const short amount);
	void		setAmount(const short amt);
	SI32		DeleteAmount(int amount, short id, short color=-1);
	
	SI32		CountItems(SI16 ID=-1, SI16 col= -1,LOGICAL bAddAmounts = true)
	{ return pointers::containerCountItems(getSerial32(), ID, col, bAddAmounts); }
	
	SI32		CountItemsByID(UI32 scriptID, LOGICAL bAddAmounts)
	{ return pointers::containerCountItemsByID(getSerial32(), scriptID, bAddAmounts); }
//@}

//@{
/*!
\name Weight
*/
	UI32		weight;
	R32		getWeight();
	R32		getWeightActual();
//@}

//@{
/*!
\name Position
*/
	LOGICAL		isInWorld() const { return contserial.serial32 == -1; } //!< is the item in world or in a container?
	void		MoveTo(Location newloc);
	void		MoveTo(UI16 x, UI16 y, SI08 z) { MoveTo( Loc(x, y, z) ); }	//!< move the item
//@}

//@{
/*!
\name weapon and armour related
\brief weapon, armour, wear related stuff
\author Xan & Luxor (mostly)
*/
	Skill		getCombatSkill();
	UI32		att;		//!< Item attack
	UI32		def;		//!< Item defense
	Skill		fightskill;	//!< skill used by item
	SI32		reqskill[2];	//!< required skill by item
	DamageType	damagetype;	//!< for different damage types system
	DamageType	auxdamagetype;	//!< Additional damage :]
	SI32		auxdamage;	//!< Additional damage :]
	SI32		lodamage;	//!< Minimum Damage weapon inflicts
	SI32		hidamage;	//!< Maximum damage weapon inflicts
	SI32		wpsk;		//!< The skill needed to use the item
	SI32		spd;		//!< The speed of the weapon
	SI32		itmhand;	//!< ITEMHAND system - AntiChrist
	SI32		resists[MAX_RESISTANCE_INDEX];	//!< for resists system
	SI32		st;		//!< The strength needed to equip the item
	SI32		st2;		//!< The strength the item gives
	SI32		dx;		//!< The dexterity needed to equip the item
	SI32		dx2;		//!< The dexterity the item gives
	SI32		in;		//!< The intelligence needed to equip the item
	SI32		in2;		//!< The intelligence the item gives
	PoisonType	poisoned;	//!< type of poison that poisoned item
	UI32		ammo;		//!< Ammo used (firing weapon)
	UI32		ammoFx;		//!< Flying ammo animation (firing weapon)
//@}

//@{
/*!
\name Magic Related
*/
	SI32		countSpellsInSpellBook();
	LOGICAL		containsSpell(magic::SpellId spellnumber);
	UI32		gatetime;
	SI32		gatenumber;
	SI08		offspell;
//@}

//@{
/*!
\name Corpse related
*/
	LOGICAL		corpse;		//!< Is item a corpse
	string		murderer;	//!< char's name who kille the char (forensic ev.)
	SI32		murdertime;	//!< when the people has been killed
//@}

//@{
/*!
\name Creation related
\author Magius (CHE)
*/
	SI32		smelt;		//!< for item smelting
	/*!
	\brief for rank system, this value is the LEVEL of the item from 1 to 10.

	Simply multiply the rank*10 and calculate the MALUS this item has from the original.<br>
	for example: RANK 5 ---> 5*10=50% of malus<br>
	this item has same values decreased by 50%..<br>
	RANK 1 ---> 1*10=10% this item has 90% of malus!<br>
	RANK 10 --> 10*10=100% this item has no malus! RANK 10 is automatically setted if you select RANKSYSTEM 0.<br>
	Vars: LODAMAGE,HIDAMAGE,ATT,DEF,HP,MAXHP<br>
	*/
	SI32		rank;

	std::string	creator;	//!< Store the name of the player made this item
	SI32		good;		//!< Store type of GOODs to trade system! (Plz not set as UNSIGNED)
	SI32		rndvaluerate;	//!< Store the value calculated base on RANDOMVALUE in region.scp.

	/*!
	\brief Store the skills used to make this item

	Note by Magius: Value range to -ALLSKILLS-1 to ALLSKILLS+1<br>
	To calculate skill used to made this item:<br>
	if is a positive value, substract 1 it.<br>
		Ex) madewith=34 , 34-1=33 , 33=STEALING<br>
	if is a negative value, add 1 from it and invert value.<br>
		Ex) madewith=-34 , -34+1=-33 , Abs(-33)=33=STEALING.<br>
	0 = NULL<br>

	So... a positive value is used when the item is made by a
	player with 95.0+ at that skill. Infact in this way when
	you click on the item appear its name and the name of the
	creator. A Negative value if the player is not enought
	skilled!
	*/
	SI32		madewith;
	std::string	vendorDescription;
//@}

//@{
/*!
\name Spawn
*/
	SERIAL		spawnserial;
	SERIAL		spawnregion;
	void		SetMultiSerial(SI32 mulser);
//@}

//@{
/*!
\name Buy & Sell
*/
	SI32		value;		//!< Price shopkeeper sells item at.
	SI32		restock;	//!< Number up to which shopkeeper should restock this item
//@}

//@{
/*!
\name Trigger
*/
	SI32		trigger;	//!< Trigger number that item activates
	SI32		trigtype;	//!< Type of trigger
	SI32		tuses;		//!< Number of uses for trigger
//@}

//@{
/*!
\name Special Use
*/
	UI32		type;		//!< For things that do special things on doubleclicking
	UI32		type2;
	SI32		carve;		//!< for new carve system
	LOGICAL		incognito;	//!< for items under incognito effect
	SI32		wipe;		//!< Should this item be wiped with the /wipe command
	UI32		time_unused;	//!< used for house decay and possibly for more in future, gets saved
	UI32		timeused_last;	//!< helper attribute for time_unused, doesnt get saved
//@}

//@{
/*!
\name Effect related
*/
//	SI32		glow; // LB identifies glowing objects
//	SI08		glow_c1; // for backup of old color
//	SI08		glow_c2;
//	SI08		glow_effect; 
	SI08		doordir; // Reserved for doors
	LOGICAL		dooropen;
	void		explode(NXWSOCKET  s);
//@}

	LOGICAL		dye;		//!< Reserved: Can item be dyed by dye kit

	SI08		priv;		//!< Bit 0, decay off/on.  Bit 1, newbie item off/on.  Bit 2 Dispellable

private:
	TIMERVAL	decaytime;

public:
	LOGICAL		doDecay();
	const LOGICAL	canDecay() const { return priv &0x01; }
	void		setDecay( const LOGICAL on = true );

	void		setDecayTime( const TIMERVAL delay = uiCurrentTime+(SrvParms->decaytimer*MY_CLOCKS_PER_SEC) );
	const TIMERVAL	getDecayTime() const { return decaytime; }

	const LOGICAL	isNewbie() const { return priv&0x02; }
	void		setNewbie( const LOGICAL on = true );

	const LOGICAL	isDispellable() const { return priv&0x04; }
	void		setDispellable( const LOGICAL on = true );

	LOGICAL		pileable; //!< Can item be piled?
	bool		PileItem( P_ITEM pItem );

	P_ITEM		getOutMostCont( short rec=50 );
	P_CHAR		getPackOwner();

	UI32		distFrom( P_CHAR pc );
	UI32		distFrom( P_ITEM pi );

public:
	virtual void	Delete();
/*
public:
	LOGICAL 	isValidAmxEvent( UI32 eventId );
*/
} PACK_NEEDED;

#if 0
// will be fully implemented in 0.83
// Anthalir
/*

  properties of containers:
	- one or more key(s)
	- a list of items stored in it
	- a gump
*/
class cContainerItem : public cItem
{
private:
	vector<SI32>		ItemList;

	SI16				getGumpType();
	LOGICAL				pileItem(P_ITEM pItem);
	void				setRandPos(P_ITEM pItem);

public:
						cContainerItem(LOGICAL ser= true);
	LOGICAL				addItem(P_ITEM pItem, SI16 x= -1, SI16 y= -1);
	UI32				removeItems(UI32 scriptID, UI32 amount/*= 1*/);
	void				dropItem(P_ITEM pi);
	UI32				countItems(UI32 scriptID, LOGICAL bAddAmounts= false);

} PACK_NEEDED;

class cWeapon : public cItem
{
public:
        cWeapon(SERIAL serial);
} PACK_NEEDED;
#endif

extern LOGICAL LoadItemEventsFromScript (P_ITEM pi, char *script1, char *script2);

#endif
