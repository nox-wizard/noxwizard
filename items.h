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
#include "item.h"
#include "globals.h"

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
	~cItem();


//@{
/*!
\name Who is
*/
	AmxEvent	*amxevents[ALLITEMEVENTS];

	SI32		hp;	//!< Number of hit points an item has.
	SI32		maxhp;	//!< Max number of hit points an item can have.

	void		Refresh();
	SI32		getName(char* itemname);
	const char*	getRealItemName();
//@}

//@{
/*!
\name Look
*/
	inline SI16 animid()
	{ return ( animid1 && animid2 ) ? (SI16)((animid1<<8)|animid2) : getId(); }

	SI08	isFieldSpellItem();

	LOGICAL IsCorpse();
	LOGICAL IsSpellScroll();	//!< predefined spells
	LOGICAL IsSpellScroll72();	//!< includes the scrolls with a variable name
	LOGICAL IsTree();		//!< this is used in AxeTarget()
	LOGICAL IsTree2();		//!< this is used in SwordTarget() to give kindling.
	LOGICAL IsInstrument();

	LOGICAL IsAxe();
	LOGICAL IsSword();
	LOGICAL IsSwordType();
	LOGICAL IsMace1H();
	LOGICAL IsMace2H();
	LOGICAL IsMaceType();
	LOGICAL IsFencing1H();
	LOGICAL IsFencing2H();
	LOGICAL IsFencingType();
	LOGICAL IsBow();
	LOGICAL IsCrossbow();
	LOGICAL IsHeavyCrossbow();
	LOGICAL IsBowType();
	LOGICAL IsArrow();
	LOGICAL IsBolt();
	LOGICAL IsStave();
	LOGICAL IsSpecialMace();
	LOGICAL IsChaosOrOrderShield();
	LOGICAL IsShield();

	LOGICAL IsLog();
	LOGICAL IsShaft();
	LOGICAL IsBoard();
	LOGICAL IsFeather();
	LOGICAL IsCutLeather();
	LOGICAL IsHide();
	LOGICAL IsBoltOfCloth();
	LOGICAL IsCutCloth();
	LOGICAL IsCloth();
	LOGICAL IsChest();

	LOGICAL IsForge();
	LOGICAL IsAnvil();
	LOGICAL IsCookingPlace();
	LOGICAL IsDagger();

	LOGICAL IsFish();
	LOGICAL IsFishWater();

	LOGICAL IsSign();
	LOGICAL IsBrassSign();
	LOGICAL IsWoodenSign();
	LOGICAL IsGuildSign();
	LOGICAL IsTradeSign();
	LOGICAL IsBankSign();
	LOGICAL IsTheatreSign();
	LOGICAL IsHouse();
	LOGICAL isSpawner();

	UI08		animid1;	//!< elcabesa animation
	UI08		animid2;	//!< elcabesa animation
	void		animSetId(SI16 id);	//!< elcabesa animation
	LOGICAL		useAnimId;

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
	SI32		getContSerial(LOGICAL old= 0) const;
	BYTE		getContSerialByte(UI32 nByte, LOGICAL old= false) const;
	const cObject*	getContainer() const;

	inline void	setCont(P_OBJECT obj)
	{ setContSerial(obj->getSerial32()); }

	void		setContSerial(SI32 serial, LOGICAL old= false, LOGICAL update= true);
	void		setContSerialByte(UI32 nByte, BYTE value, LOGICAL old= false);

	//! check if item is a container
	inline const LOGICAL isContainer() const
	{ return type==1 || type==12 || type==63 || type==8 || type==13 || type==64; }

	inline const LOGICAL isSecureContainer() const
	{ return type==8 || type==13 || type==64; }

	//SI16		GetContGumpType();
	void		SetRandPosInCont(P_ITEM pCont);
	bool		ContainerPileItem( P_ITEM pItem );	// try to find an item in the container to stack with
	SI32		secureIt; // secured chests
	LOGICAL		AddItem(P_ITEM pItem, short xx=-1, short yy=-1);	// Add Item to container
	SI32		DeleteAmountByID(int amount, unsigned int scriptID);
	SI16		getContGump();
	void		putInto( P_ITEM pi );
//@}

/********************************
	Extra info
********************************/
	UI08		more1;		// For various stuff
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

//@{
/*!
\name Amount
*/
	UI16		amount;		//!< Amount of items in pile
	UI16		amount2;	//!< Used to track things like number of yards left in a roll of cloth
	SI32		ReduceAmount(const SI16 amount);
	SI32		IncreaseAmount(const SI16 amount);

	//! set the amount of piled items
	inline void	setAmount(const UI16 amt)
	{ amount= amt; Refresh(); }

	SI32			DeleteAmount(int amount, short id, short color=-1);
	inline const SI32	CountItems(short ID=-1, short col= -1,LOGICAL bAddAmounts = true) const
	{ return pointers::containerCountItems(getSerial32(), ID, col, bAddAmounts); }

	inline const SI32	CountItemsByID(unsigned int scriptID, LOGICAL bAddAmounts) const
	{ return pointers::containerCountItemsByID(getSerial32(), scriptID, bAddAmounts); }
//@}

//@{
/*!
\name Weight
*/
	UI32			weight;
	R32			getWeight();

	inline const R32	getWeightActual()
	{ return (amount>1)? getWeight()*amount : getWeight(); }
//@}

//@{
/*!
\name Position
*/
	inline const LOGICAL	isInWorld() const
	{ return contserial.serial32==INVALID; }

	void			MoveTo(Location newloc);

	inline void		MoveTo(SI32 x, SI32 y, SI08 z)
	{ MoveTo( Loc(x, y, z) ); }
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

	//char		creator[50];	//!< Store the name of the player made this item
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
	//char		desc[100];	//!< vendor description
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

/********************************
	Effect related
********************************/
//	SI32		glow; // LB identifies glowing objects
//	SI08		glow_c1; // for backup of old color
//	SI08		glow_c2;
//	SI08		glow_effect;
	SI08		doordir; // Reserved for doors
	LOGICAL		dooropen;
	void		explode(NXWSOCKET  s);


	LOGICAL		dye;		//!< Reserved: Can item be dyed by dye kit

	SI08		priv;		//!< Bit 0, decay off/on.  Bit 1, newbie item off/on.  Bit 2 Dispellable

private:
	TIMERVAL	decaytime;

public:
	LOGICAL		doDecay();

	inline const LOGICAL canDecay() const
	{ return priv&0x01; }

	void		setDecay( const LOGICAL on = true );

	inline const void setDecayTime( const TIMERVAL delay = uiCurrentTime+(SrvParms->decaytimer*MY_CLOCKS_PER_SEC) )
	{ decaytime = delay; }

	inline const TIMERVAL getDecayTime() const
	{ return decaytime; }

	inline const LOGICAL isNewbie() const
	{ return priv&0x02; }

	void		setNewbie( const LOGICAL on = true );

	inline const LOGICAL isDispellable() const
	{ return priv&0x04; }

	void		setDispellable( const LOGICAL on = true );

	LOGICAL		pileable; // Can item be piled
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
	UI32				removeItems(UI32 scriptID, UI32 amount/*= 1*/);
	void				dropItem(P_ITEM pi);
	UI32				countItems(UI32 scriptID, LOGICAL bAddAmounts= false);

} PACK_NEEDED;

class cWeapon : public cItem
{
public:
        cWeapon(SERIAL serial);
} PACK_NEEDED;

extern LOGICAL LoadItemEventsFromScript (P_ITEM pi, char *script1, char *script2);


#define MAKE_ITEM_REF(i) pointers::findItemBySerial(i)

#define DEREF_P_ITEM(pi) ( ( ISVALIDPI(pi) ) ? pi->getSerial32() : INVALID )

#define ISVALIDPI(pi) ( ( pi!=NULL && sizeof(*pi) == sizeof(cItem) ) ? (pi->getSerial32() > 0) : false )

#define VALIDATEPI(pi) if (!ISVALIDPI(pi)) { LogWarning("Warning: a non-valid P_ITEM pointer was used in %s:%d", basename(__FILE__), __LINE__); return; }
#define VALIDATEPIR(pi, r) if (!ISVALIDPI(pi)) { LogWarning("Warning: a non-valid P_ITEM pointer was used in %s:%d", basename(__FILE__), __LINE__); return r; }



#endif
