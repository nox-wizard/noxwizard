  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
 \file chars.h
 \brief Declaration of cChar class
 */
#ifndef __CHARS_H
#define __CHARS_H

#include "ai.h"
#include "object.h"
#include "magic.h"

#include "char_array.h"
#include "npcs.h"
#include "msgboard.h"
#include "target.h"
#include "constants.h"
#include "weight.h"
#include "skills.h"
#include "jail.h"

//@{
/*!
\name nxwflags
\brief native extension to game play :)
*/
#define NCF0_GREY			1	 // char is grey
#define NCF0_PERMAGREY			2	 // char is permagrey
#define NCF0_RFIRE			4	 // char resists fire (future use)
#define NCF0_RPARALISYS 		8	 // char resists paralisys (future use)
#define NCF0_RPOISON			16	 // char resists poison (future use)
#define NCF0_WATERWALK			32	 // char walks on water (npc only)
#define NCF0_BIT6			64
#define NCF0_BIT7			128

#define NCF1_BIT0			1
#define NCF1_BIT1			2
#define NCF1_BIT2			4
#define NCF1_BIT3			8
#define NCF1_BIT4			16
#define NCF1_BIT5			32
#define NCF1_BIT6			64
#define NCF1_BIT7			128

#define NCF2_BIT0			1
#define NCF2_BIT1			2
#define NCF2_BIT2			4
#define NCF2_BIT3			8
#define NCF2_BIT4			16
#define NCF2_BIT5			32
#define NCF2_BIT6			64
#define NCF2_BIT7			128

#define NCF3_BIT0			1
#define NCF3_BIT1			2
#define NCF3_BIT2			4
#define NCF3_BIT3			8
#define NCF3_BIT4			16
#define NCF3_BIT5			32
#define NCF3_BIT6			64
#define NCF3_BIT7			128

#define RES_FIRE			NCF0_RFIRE
#define RES_FREEZE			NCF0_RPARALISYS
#define RES_POISON			NCF0_RPOISON

//@}

enum Races {
	UNKNOWN = 0,
	UNDEAD,
	ORC,
	DEAMON,
	ELEMENTAL,
	DRAGON,
	ANIMAL,
	WYVERN_WYRM,
	ARACHNID,
	TERATHAN,
	OOZE_SLIME,
	ABERRATION,
	GARGOYLES,
	LIZARDMAN,
	RATMAN,
	OPHIDIAN,
	GIANTPEOPLE,
	CONSTRUCTS,
	HUMANOID_ABERRATION,
	VEGETABLE,
	ANIMAL_ABERRATION
};


/*!
\brief AMX Events for Characters
\author Akron
*/
enum AmxCharEvents
{
	EVENT_CHR_ONDEATH 		=  0,
	EVENT_CHR_ONBEFOREDEATH		=  0,
	EVENT_CHR_ONWOUNDED		=  1,
	EVENT_CHR_ONHIT			=  2,
	EVENT_CHR_ONHITMISS		=  3,
	EVENT_CHR_ONGETHIT		=  4,
	EVENT_CHR_ONREPUTATIONCHG	=  5,
	EVENT_CHR_ONDISPEL		=  6,
	EVENT_CHR_ONRESURRECT		=  7,
	EVENT_CHR_ONFLAGCHG		=  8,
	EVENT_CHR_ONWALK		=  9,
	EVENT_CHR_ONADVANCESKILL	= 10,
	EVENT_CHR_ONADVANCESTAT		= 11,
	EVENT_CHR_ONBEGINATTACK		= 12,
	EVENT_CHR_ONBEGINDEFENSE	= 13,
	EVENT_CHR_ONTRANSFER		= 14,
	EVENT_CHR_ONMULTIENTER		= 15,
	EVENT_CHR_ONMULTILEAVE		= 16,
	EVENT_CHR_ONSNOOPED		= 17,
	EVENT_CHR_ONSTOLEN		= 18,
	EVENT_CHR_ONPOISONED		= 19,
	EVENT_CHR_ONREGIONCHANGE	= 20,
	EVENT_CHR_ONCASTSPELL		= 21,
	EVENT_CHR_ONGETSKILLCAP		= 22,
	EVENT_CHR_ONGETSTATCAP		= 23,
	EVENT_CHR_ONBLOCK		= 24,
	EVENT_CHR_ONSTART		= 25,
	EVENT_CHR_ONHEARTBEAT		= 26,
	EVENT_CHR_ONBREAKMEDITATION	= 27,
	EVENT_CHR_ONCLICK		= 28,
	EVENT_CHR_ONMOUNT		= 29,
	EVENT_CHR_ONDISMOUNT		= 30,
	EVENT_CHR_ONKILL		= 31,
	EVENT_CHR_ONHEARPLAYER		= 32,
	EVENT_CHR_ONDOCOMBAT		= 33,
	EVENT_CHR_ONCOMBATHIT		= 34,
	EVENT_CHR_ONSPEECH		= 35,
	EVENT_CHR_ONCHECKNPCAI		= 36,
	EVENT_CHR_ONDIED		= 37,
	EVENT_CHR_ONAFTERDEATH		= 37,
	ALLCHAREVENTS			= 38
};

#define DISPELTYPE_UNKNOWN		0
#define DISPELTYPE_DISPEL		1
#define DISPELTYPE_TIMEOUT		2
#define DISPELTYPE_GMREMOVE		3

#define STATCAP_CAP			0
#define STATCAP_STR			1
#define STATCAP_DEX			2
#define STATCAP_INT			3

#define REPUTATION_KARMA		1
#define REPUTATION_FAME			2

class cNxwClientObj;
class cPath;

/*!
\brief Character class
*/
class cChar : public cObject
{
	public:
		cChar( SERIAL ser );
		~cChar();
		void setClient(NXWCLIENT client);
	private:
		cNxwClientObj* m_client;
	public:
	//
	// Static properties and methods
	//
		static void		archive();
		static void		safeoldsave();
	public:
		void			MoveTo(Location newloc);
		void			MoveTo(UI16 x, SI32 y, SI08 z) { MoveTo( Loc(x, y, z) ); }
		void 			loadEventFromScript(TEXT *script1, TEXT *script2);
//@{
/*!
 \name Guilds
 \brief Guild related functions and attributes
*/
	private:
		SI08 			guildType;		//!< (0) Standard guild, (1) Chaos Guild, (2) Order guild
		LOGICAL			guildTraitor;		//!< (true) This character converted, (false) Neve converted, or not an order/chaos guild member
		LOGICAL			guildToggle;		//!< Toggle for Guildtitle
		SERIAL			guildFealty;		//!< Serial of player you are loyal to (default=yourself)
		SI32			guildNumber;		//!< Number of guild player is in (0=no guild)
		TEXT			guildTitle[21];		//!< Title Guildmaster granted player

	public:
		void			SetGuildType( SI08 newGuildType );
		const SI08		GetGuildType() const { return guildType; } //!< return the guild type

		const LOGICAL		IsGuildTraitor() const { return guildTraitor; } //!< return guild traitor status
		void			SetGuildTraitor();
		void			ResetGuildTraitor();

		const LOGICAL		HasGuildTitleToggle() const { return guildToggle; } //!< return guild title toggle status
		void			SetGuildTitleToggle();
		void			ResetGuildTitleToggle();

		const SERIAL		GetGuildFealty() const { return guildFealty; } //!< return the guild fealty
		void			SetGuildFealty( SERIAL newGuildFealty );

		const SI32		GetGuildNumber() const { return guildNumber; } //!< return the guild number
		void			SetGuildNumber( SI32 newGuildNumber );

		void			SetGuildTitle( TEXT *newGuildTitle );
		TEXT*			GetGuildTitle() { return guildTitle; } //!< return the guild title
//@}

//@{
/*!
\name Races
\brief Race related functions and attributes
*/
	public:
		SI32 			race;				//!< Race index
//@}

//@{
/*!
\name Date/Time
\brief Date/Time related functions and attributes
*/
	private:
		TIMERVAL		creationday;			//!< Day since EPOCH this character was created on
	public:
		void			SetCreationDay(TIMERVAL day);	//!< Set the creation day of a character
		const TIMERVAL		GetCreationDay() const { return creationday; }
			//!< Get the creation day of a character
//@}

//@{
/*!
\name Stats
\brief Stats related functions and attributes
*/
	private:
		UI32_s			str;				//!< Strength

	public:
		SI32			st2;				//!< Reserved for calculation
		SI32			st3;				//!< Luxor: safe strength value
		SI32			dx;				//!< Dexterity
		SI32			dx2;				//!< Reserved for calculation
		SI32			dx3;				//!< Luxor: safe dexterity value
		SI32			in;				//!< Intelligence
		SI32			in2;				//!< Reserved for calculation
		SI32			in3;				//!< Luxor: safe intelligence value
		SI32			statGainedToday;		//!< xan :-> for stat-gain cap

		const SI32		getStrength() const { return str.value; } //!< Get the strength-value

		void			setStrength(UI32 val, bool check= true);
		void			modifyStrength(SI32 mod, bool check= true);
		void			checkSafeStats();
//@}

	public:
		void			heartbeat();

	private:
		void			generic_heartbeat();
		void			pc_heartbeat();
		void 			do_lsd();
		void			npc_heartbeat();

//@{
/*!
\name Appearence
*/
	public:
		std::string	title;
		UI16			oldhairstyle;
		UI16			oldbeardstyle;
		UI16			oldhaircolor;
		UI16			oldbeardcolor;
	//
	//	Body Type
	//
	private:
		UI16			id;				//!< Character body type
		UI16			xid;				//!< Backup of body type for ghosts
	public:
		const BODYTYPE		GetBodyType() const { return (BODYTYPE)id; }
		void			SetBodyType(BODYTYPE newBody);
		const BODYTYPE		GetOldBodyType() const { return (BODYTYPE) xid; }
		void			SetOldBodyType(BODYTYPE newBody);
		const LOGICAL		HasHumanBody() const { return (id==BODY_MALE) || (id==BODY_FEMALE); }
	//
	//	Skin Color
	//
	private:
		UI16			skin;				//!< Skin color
		UI16			xskin;				//!< Backup of skin color
	public:
		const UI16		getSkinColor() const { return skin; }
		void			setSkinColor( UI16 newColor );
		const UI16		getOldSkinColor() const { return xskin; }
		void			setOldSkinColor( UI16 newColor );

		void 			showLongName( P_CHAR showToWho, LOGICAL showSerials );
//@}


//@{
/*!
\name Combat
*/
	private:
		LOGICAL			attackfirst;		//!< 0 = defending, 1 = attacked first

	public:
		const LOGICAL		HasAttackedFirst() const { return attackfirst; }
		void 			SetAttackFirst();
		void 			ResetAttackFirst();

	private:
		LOGICAL			combatTimerOk();
		void			checkPoisoning(P_CHAR pc_def);
		void			doMissedSoundEffect();
		SI32			combatHitMessage(SI32 damage);
		void			doCombatSoundEffect(SI32 fightskill, P_ITEM pWeapon);
		void			undoCombat();

	public:
		void			checkPoisoning();
		void 			fight(P_CHAR pOpponent);

		/*!
		\author Luxor
		\brief Makes the char casting a spell (truly use magic::castSpell)
		\param spellnumber Spell identifier
		\param dest target location of the spell
		\todo Document parameters
		*/
		void			castSpell(magic::SpellId spellnumber, TargetLocation& dest, SI32 flags = 0, SI32 param = 0)
		{ magic::castSpell(spellnumber, dest, this, flags, param); }
		
		void			combatHit( P_CHAR pc_def, SI32 nTimeOut = 0 );
		void			doCombat();
		void			combatOnHorse();
		void			combatOnFoot();
		void			playCombatAction();
		SI32			calcAtt();
		SI32			calcDef(SI32 x = 0);
		void			setWresMove(SI32 move = 0);
		SI32			calcResist(DamageType typeofdamage);
		void			toggleCombat();
		SI32			getCombatSkill();
//@}

//@{
/*!
\name Priviledges
*/
	protected:
		UI08			priv;		/*!<	- 0x01: GM clearance
								- 0x02: Broadcast
								- 0x04: Invulnerable
							        - 0x08: single click serial numbers
							        - 0x10: Don't show skill titles
								- 0x20: GM Pagable
								- 0x40: Can snoop others packs
								- 0x80: Counselor clearance
							*/
	private:
		void		resetPriv3();

	public: // this will become private and i don't think 3 variables are needed to store privs...
		SI08			priv2;	            /*!< 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
										10: no need mana, 20: dispellable, 40: permanent magic reflect, 80: no need reagents
						*/
		SI32 			priv3[7];           //!< needed for Lord binarys meta-gm stuff

	public:
		const LOGICAL		IsTrueGM() const { return priv&CHRPRIV_GM; }
		const LOGICAL		IsGM() const { return (priv&CHRPRIV_GM && (!gmrestrict || (region==gmrestrict))) || (account == 0); }
		const LOGICAL		IsCounselor() const { return priv&CHRPRIV_COUNSELOR; }
		const LOGICAL		IsGMorCounselor() const { return priv&(CHRPRIV_COUNSELOR|CHRPRIV_GM); }
		const LOGICAL		IsInvul() const { return priv&CHRPRIV_INVUL; }
		const LOGICAL		CanSnoop() const { return priv&CHRPRIV_CANSNOOPALL; }
		const LOGICAL		CanBroadcast() const { return priv&CHRPRIV_BROADCAST; }
		const LOGICAL		CanSeeSerials() const { return priv&CHRPRIV_CANVIEWSERIALS; }

		UI08 			GetPriv() const { return priv; }
		void 			SetPriv(UI08 p);

		void 			MakeInvulnerable();
		void 			MakeVulnerable();

		SI08			GetPriv2() const { return priv2; }	//!< Temporary workaround until priv vars are settled
		void 			SetPriv2(SI08 p);
//@}

//@{
/*!
\name Char Status
*/
	private:
		SI32			karma;		//!< karma of the char
		SI32    		fame;		//!< fame of the char
	public: // will become private
		LOGICAL			incognito;          //!< AntiChrist - true if under incognito effect
		LOGICAL			polymorph;          //!< AntiChrist - true if under polymorph effect

		UI32        	kills;				//!< PvP Kills
		UI32        	deaths;				//!< Number of deaths
		LOGICAL			dead;				//!< Is the character dead ?
		R32				fstm;				//!< Unavowed - stamina to remove the next step

	public:
		LOGICAL const		IsInnocent() const { return flag&CHRFLAG_INNOCENT; }
		LOGICAL const		IsMurderer() const { return flag&CHRFLAG_MURDERER; }
		LOGICAL const		IsCriminal() const { return flag&CHRFLAG_CRIMINAL; }
		LOGICAL const		IsGrey() const;
		LOGICAL const 		IsHidden() const { return hidden != UNHIDDEN; }
		LOGICAL const 		IsHiddenBySpell() const { return (hidden & HIDDEN_BYSPELL); }
		LOGICAL const 		IsHiddenBySkill() const { return (hidden & HIDDEN_BYSKILL); }
		LOGICAL const		IsOverWeight() { return !IsGM() && weights::CheckWeight2(this); } //!< Checks char weight
		LOGICAL const		InGuardedArea() const { return ::region[region].priv & RGNPRIV_GUARDED; }
		LOGICAL const		IsOnline() const;
		LOGICAL const		IsFrozen() const { return ((priv2&CHRPRIV2_FROZEN)!=0); }
		LOGICAL const		CanDoGestures() const;

		void 			SetMurderer();
		void 			SetInnocent();
		void 			SetCriminal();
		void 			SetPermaGrey();
		void 			SetGrey();
		void 			unHide();

		const SI32		GetKarma() const { return karma; } //!< return the karma of the char
		void			SetKarma(SI32 newkarma);
		void			IncreaseKarma(SI32 value, P_CHAR killed=NULL);

		const SI32		GetFame() const { return fame; } //!< return the fame of the char
		void			SetFame(SI32 newfame);
		void			modifyFame( SI32 value );
//@}

//@{
/*!
\name Char Equipment
*/
	public:
		const LOGICAL		IsWearing(P_ITEM pi) const { return getSerial32() == pi->getContSerial(); }
		SI32			Equip(P_ITEM pi, LOGICAL drag = false);
		SI32			UnEquip(P_ITEM pi, LOGICAL drag = false);
		void			checkEquipement();
	#ifdef ENDY_NEW_WEAR
	public:
		SERIAL wear[LAYER_COUNT];	//!< wear serial list indexed by layer
	#endif
//@}

//@{
/*!
\name Movement
*/
	private:
		cPath*			path;			//!< current path <Luxor>
		void			walkNextStep();		//!< walk next path step <Luxor>
		SERIAL_SLIST		sentObjects;
	public:
		LOGICAL			seeForFirstTime( P_OBJECT po );	//!< does it see the object for the first time? <Luxor>
		LOGICAL			seeForLastTime( P_OBJECT po ); //!< does it see the object for the first time? <Luxor>
		void			walk();			//!< execute walk code <Luxor>
		inline LOGICAL		hasPath() { return (path!=NULL); } //!< has a path set? <Luxor>
		void			follow( P_CHAR pc ); //!< follow pc <Luxor>
		void			pathFind( Location pos, LOGICAL bOverrideCurrentPath = true );	//!< Walk to position <Luxor>
		SI08			dir;			//!< &0F=Direction
		UI32			LastMoveTime;		//!< server time of last move
//@}


//@{
/*!
\name Party
*/
	public:
		SI32			party;
		SI32			requestedParty;
		SI32			reqPartySerial;
		LOGICAL			partyCanLoot;
//@}

//@{
/*!
\name NPCs
*/
	public:
		UI32			lastNpcCheck; // timestamp used to reduce # of checkNPC calls in checkAuto() to 1 (Sparhawk)
		char			npc;				//!< 1=Character is an NPC
		LOGICAL			doorUse;			//!< True if npc can open doors
		LOGICAL			shopkeeper;			//!< true=npc shopkeeper

	public:
		// Skyfire's NPC advancments.
		Location workloc;	                //!< work location (not used for now)
		Location homeloc;	                //!< home location (not used for now)
		Location foodloc;	                //!< food location (not used for now)
		//Skyfire - End NPC's home/work/food vars'

		/*UI08			pathnum;
		path_st			path[PATHNUM];*/

		TIMERVAL		nextAiCheck;

		SI32			hidamage;           //!< NPC Hi Damage
		SI32			lodamage;           //!< NPC Lo Damage
//@}

//@{
/*!
\name Others
*/
	public:
		SI32			targetcallback;     //!< xan:this is used for targetting callbacks
		class cCustomMenu *customMenu;      //!< xan : this is for custom menus

		UI08			gmrestrict;			//!< for restricting GMs to certain regions

		void			doGmEffect();
//@}

	public:
		bool jailed;


//@{
/*!
\name Trainer
*/
	public:
		SERIAL			trainer;
			//!< Serial of the NPC training the char, -1 if none.
		UI08			trainingplayerin;
			//!< Index in skillname of the skill the NPC is training the player in
		LOGICAL			cantrain;

	public:
		const LOGICAL		isBeingTrained() const { return trainer != INVALID; }
		const SERIAL		getTrainer() const { return trainer; }

		const UI08		getSkillTaught() const { return trainingplayerin; }

		const LOGICAL		canTrain() const { return cantrain; }
		void			setCanTrain();
		void			resetCanTrain();
//@}

//@{
/*!
\name Mount
*/
	public:
		bool			mounted; //!< if now is mounted by a char

	private:
		LOGICAL			onhorse; //!< On a horse
	public:
		SI32				unmountHorse();
		void			mounthorse( P_CHAR mount );
		SERIAL			getHorse();
		void			setOnHorse();
		bool			isMounting( );
		bool			isMounting( P_CHAR horse );
//@}

//@{
/*!
\name Guilds
\author Endymion
*/
	private:
		SERIAL			guild; //!< Serial of guild

	public:
		const LOGICAL		isGuilded() const { return ISVALIDGUILD( guild ); } //!< check if guilded
		void			setGuild(SERIAL newGuild);
		const SERIAL		getGuild() const { return guild; } //!< get guild
//@}

//@{
/*!
\name Target
*/
	private:
		P_TARGET		current_target; //!< target

	public:
		const LOGICAL		isTargeting() const { return current_target; }
		void			setTarget( P_TARGET newtarget );
		void			doTarget();
//@}

//@{
/*!
\name Stats regen rate info
*/
private:
	regen_st 	regens[ALL_STATS]; //!< stats regen info
public:
	void setRegenRate( StatType stat, UI32 rate, VarType type );
	UI32 getRegenRate( StatType stat, VarType type );
	bool regenTimerOk( StatType stat );
	void updateRegenTimer( StatType stat );
//@}

//@{
/*!
\name Npc Vendor
\note fist draft.. need to make map with id used in recive for make faster
*/

public:
	std::vector<vendor_item>*	vendorItemsSell; //!< all items sell by vendor
	std::vector<vendor_item>*	vendorItemsBuy; //!< all items sell by vendor

//@}

	/********************************/
	/*     TO REMOVE/REPLACE        */
	/********************************/
	public:
		void 			setMultiSerial(long mulser);
		const LOGICAL		isOwnerOf(const cObject *obj) const { return getSerial32() == obj->getOwnerSerial32(); }

	/********************************/

	/// INDEX -> SERIAL
		//SI32 			keynumb;  // for renaming keys
		SERIAL			keyserial;  //!< for renaming keys
		//SI32			packitem; // Only used during character creation
		SERIAL			packitemserial; //!< Only used during character creation
		//SI32			targ; // Current combat target
		SERIAL			targserial; //!< Current combat target
		//SI32			runenumb; // Used for naming runes
		SERIAL			runeserial; //!< Used for naming runes
		//SI32			attacker; // Character who attacked this character
		SERIAL			attackerserial; //!< Character who attacked this character
		//SI32			ftarg; // NPC Follow Target
		SERIAL			ftargserial; //!< NPC Follow Target
		//SI32			smeltitem;
		SERIAL			smeltserial;
		//SI32			tailitem;
		SERIAL			tailserial;
		//SI32			swingtarg; //Tagret they are going to hit after they swing
		SERIAL			swingtargserial; //!< Target they are going to hit after they swing
		//SI32			namedeed;
		SI32			namedeedserial;

		//UI32			trackingtarget; // Tracking target ID
		SERIAL			trackingtarget_serial; //!< Tracking target ID
		//UI32			trackingtargets[MAXTRACKINGTARGETS];
		SERIAL			trackingtargets_serial[MAXTRACKINGTARGETS];


	// END

		SI32 			account; // changed to signed, lb



	private:
		void		resetBaseSkill();
		void		resetSkill();
		void		resetTrackingTargets();
		void		resetNxwFlags();
		void		resetAmxEvents();
		void		resetResists();
		void		resetLockSkills();

	private:
		wstring* profile; //!< player profile
	public:
		wstring* getProfile() const { return profile; }
		void setProfile( wstring* profile );
		void resetProfile();

	private:
		wstring* speechCurrent;
	public:
		wstring* getSpeechCurrent() const { return speechCurrent; } //!< Return current speech
		void setSpeechCurrent( wstring* speech );
		void resetSpeechCurrent();
		void deleteSpeechCurrent();

	private:
		SERIAL	stablemaster_serial; //!< the stablemaster serial
	public:
		const LOGICAL isStabled() const { return stablemaster_serial!=INVALID; } //!< Check if char is stabled
		const SERIAL getStablemaster() const { return stablemaster_serial; } //!< Get the character's stablemaster
		void stable( P_CHAR stablemaster );
		void unStable();

	public:
		SI32		npcaitype; //!< NPC ai

	public:


		SERIAL oldmenu; //!< old menu serial

		SI32			stat3crc; // xan : future use to keep safe stats
		class			AmxEvent *amxevents[ALLCHAREVENTS];
		SI32			hp;  // HitpoSI32s
		SI32			stm; // Stamina
		SI32			mn;  // Mana
		SI32			mn2; // Reserved for calculation
		unsigned short		baseskill[ALLSKILLS+1]; // Base skills without stat modifiers
		unsigned short		skill[ALLSKILLS+1]; // List of skills (with stat modifiers)


		SERIAL			robe; // Serial number of generated death robe (If char is a ghost)
		UI08			fixedlight; // Fixed lighting level (For chars in dungeons, where they dont see the night)
		char			speech; // For NPCs: Number of the assigned speech block
		SI32			weight; //Total weight
		UI32			att; // Intrinsic attack (For monsters that cant carry weapons)
		UI32			def; // Intrinsic defense
		char			war; // War Mode
		char			wresmove; // Luxor: for Wrestling's special moves

		TIMERVAL		timeout; // Combat timeout (For hitting)
		TIMERVAL		timeout2; // memory of last shot timeout

		TIMERVAL		npcmovetime; // Next time npc will walk
		char			npcWander; // NPC Wander Mode
		char			oldnpcWander; // Used for fleeing npcs
		R32			npcMoveSpeed; // Used to controll npc walking speed
		R32			npcFollowSpeed; // Used to controll npc walking speed when following a target (npcwander = 1)
		SI32			fx1; //NPC Wander PoSI32 1 x
		SI32			fx2; //NPC Wander Point 2 x
		SI32			fy1; //NPC Wander Point 1 y
		SI32			fy2; //NPC Wander Point 2 y
		signed char		fz1; //NPC Wander Point 1 z
		UI08			hidden; // 0 = not hidden, 1 = hidden, 2 = invisible spell
		TIMERVAL		invistimeout;
		SI32			hunger;  // Level of hungerness, 6 = full, 0 = "empty"
		TIMERVAL		hungertime; // Timer used for hunger, one point is dropped every 20 min
		SI32			callnum; //GM Paging
		SI32			playercallnum; //GM Paging
		SI32			pagegm; //GM Paging
		//char region;
		UI08			region;
		TIMERVAL		skilldelay;
		TIMERVAL		objectdelay;
		SI32			combathitmessage;
		SI32			making; // skill number of skill using to make item, 0 if not making anything.
		char			blocked;
		char			dir2;
		TIMERVAL		spiritspeaktimer; // Timer used for duration of spirit speak
		SI32			spattack;
		TIMERVAL		spadelay;
		TIMERVAL		spatimer;
		SI32			taming; //Skill level required for taming
		TIMERVAL		summontimer; //Timer for summoned creatures.
		TIMERVAL		trackingtimer; // Timer used for the duration of tracking
		TIMERVAL		fishingtimer; // Timer used to delay the catching of fish

		char			nxwflags[4]; // for special nxw features
	//SI32			amxflags[16]; // for special things implemented by scripts :)
		SI32			magicsphere; // for npc ai
		//<Luxor>
		SI32			resists[MAX_RESISTANCE_INDEX];
		LOGICAL			holydamaged;
		LOGICAL			lightdamaged;
		DamageType		damagetype;
		//</Luxor>
		SI32			advobj; //Has used advance gate?

		SI32			poison; // used for poison skill
		PoisonType		poisoned; // type of poison
		TIMERVAL		poisontime; // poison damage timer
		TIMERVAL		poisontxt; // poision text timer
		TIMERVAL		poisonwearofftime; // LB, makes poision wear off ...

		SI32			fleeat;
		SI32			reattackat;
		SI32			trigger; //Trigger number that character activates
		std::string	trigword; //Word that character triggers on.
		char			envokeid1; //ID1 of item user envoked
		char			envokeid2; //ID2 of item user envoked
		SI32			envokeitem;
		SI32			split;
		SI32			splitchnc;
		SI32			targtrig; //Stores the number of the trigger the character for targeting
		char			ra;  // Reactive Armor spell

		char			flag; //1=red 2=grey 4=Blue 8=green 10=Orange
		TIMERVAL		tempflagtime;
		// End of Guild Related Character information

		wstring* staticProfile; //!< player static profile


		TIMERVAL		murderrate; //# of ticks until one murder decays //REPSYS
		TIMERVAL		murdersave; //# of second for murder decay

		TIMERVAL		crimflag; //Time when No longer criminal -1=Not Criminal
		SI32				casting; // 0/1 is the cast casting a spell?
		TIMERVAL		spelltime; //Time when they are done casting....
		magic::SpellId		spell; //current spell they are casting....
		SI32			spellaction; //Action of the current spell....
		magic::CastingType	spelltype;
		TIMERVAL		nextact; //time to next spell action....
		SI32			poisonserial; //AntiChrist -- poisoning skill

		SI32			squelched; // zippy  - squelching
		TIMERVAL		mutetime; //Time till they are UN-Squelched.
		SI32			med; // 0=not meditating, 1=meditating //Morrolan - Meditation
		//SI32 statuse[3]; //Morrolan - stat/skill cap STR/INT/DEX in that order
		//SI32 skilluse[TRUESKILLS][1]; //Morrolan - stat/skill cap
		UI08			lockSkill[ALLSKILLS+1]; // LB, client 1.26.2b skill managment
		SI32			stealth; //AntiChrist - stealth ( steps already done, -1=not using )
		UI32			running; //AntiChrist - Stamina Loose while running
		UI32			lastRunning; //Luxor
		SI32			logout; //Time till logout for this char -1 means in the world or already logged out //Instalog
		TIMERVAL		clientidletime; // LB
		//UI32 swing;

		UI32			holdg; // Gold a player vendor is holding for Owner
		//SI32			weather;	//!< Weather!
		char			fly_steps; // number of step the creatures flies if it can fly
		TIMERVAL		trackingdisplaytimer;
		SI32			menupriv; // needed fro LB's menu priv system
		//taken from 6904t2(5/10/99) - AntiChrist
		LOGICAL			tamed;
		//taken from 6904t2(5/10/99) - AntiChrist
		LOGICAL			guarded;							// (Abaddon) if guarded
		TIMERVAL		smoketimer; // LB
		TIMERVAL		smokedisplaytimer;

		TIMERVAL		antispamtimer;//LB - anti spam

		TIMERVAL		antiguardstimer;//AntiChrist - anti "GUARDS" spawn

		SI32			carve; //AntiChrist - for new carve system

		SERIAL			hairserial;//there are needed for incognito stuff
		SERIAL			beardserial;


		TIMERVAL		begging_timer;
		MsgBoards::PostType	postType;
		MsgBoards::QuestType	questType;
		SI32			questDestRegion;
		SI32			questOrigRegion;
		SI32			questBountyReward;		// The current reward amount for the return of this chars head
		SERIAL			questBountyPostSerial;	// The global posting serial number of the bounty message
		SERIAL			murdererSer;			// Serial number of last person that murdered this char

		// COORDINATE	previousLocation;

		SI32 			prevX; // fix for looping gate travel bug (bounce back problem)
		SI32 			prevY;
		signed char 		prevZ;
		LOGICAL			morphed;

		UI08			commandLevel; 			// 0 = player, 1 = counselor, 2 = GM

		SERIAL			spawnserial; //!< Spawned by dynamic
		SERIAL			spawnregion; //!< Spawned by scripted

		char			npc_type;		// currently only used for stabling, (type==1 -> stablemaster)
							// can be used for other npc types too of course

		TIMERVAL 		time_unused;
		TIMERVAL 		timeused_last;

		void 			doSingleClick( SERIAL serial );
		void			onSingleClick( P_CHAR clickedBy );

	private:

		void 			doSingleClickOnCharacter( SERIAL serial );	// will become private function
		void 			doSingleClickOnItem( SERIAL serial );		// will become private function

	public:

		const LOGICAL		isRunning() const { return (uiCurrentTime - lastRunning) <= 100; } //!< tells if a character is running
		void			setRunning();
		void 			updateStats(SI32 stat);

		void 			setNextMoveTime(short tamediv=1);
		void 			disturbMed();

		void                    drink(P_ITEM pi);       //Luxor: delayed drinking
		void 			hideBySkill();
		void 			hideBySpell(SI32 timer = INVALID);
		const UI32		CountItems(short ID, short col= INVALID);
		const UI32		CountGold() { return CountItems(ITEMID_GOLD); }
		P_ITEM 			GetItemOnLayer(UI08 layer);
		P_ITEM 			GetBankBox( short type = BANK_GOLD);
		void			openBankBox( P_CHAR pc );
		void 			openSpecialBank( P_CHAR pc );
		SI32  			countBankGold();
	//	void 			addHalo(P_ITEM pi);
	//	void 			removeHalo(P_ITEM pi);
	//	void 			glowHalo(P_ITEM pi);
		P_ITEM 			getWeapon();
		P_ITEM 			getShield();
		void			showContainer( P_ITEM pCont );
		P_ITEM 			getBackpack();
		void			showBackpack() { showContainer( getBackpack() ); } //!< Show Backpack to player
		LOGICAL			isInBackpack( P_ITEM pi );


		// The bit for setting what effect gm movement
		// commands shows
		// 0 = off
		// 1 = FlameStrike
		// 2-6 = Sparkles
		SI32			gmMoveEff;


		UI32			getSkillSum();
		SI32			getTeachingDelta(P_CHAR pPlayer, SI32 skill, SI32 sum);
		void			removeItemBonus(cItem* pi);
		LOGICAL			isSameAs(P_CHAR pc) {if (pc && (pc->getSerial32() == getSerial32())) return true; else return false;}
		LOGICAL			resist(SI32 n) const { return ((nxwflags[0]&n)!=0); }    // <-- what is this ?, xan

		const NXWCLIENT		getClient() const { return m_client; }
		const NXWSOCKET		getSocket() const;
		void			sysmsg(TEXT *txt, ...);
		void			attackStuff(P_CHAR pc);
		void			helpStuff(P_CHAR pc_i);
		void			applyPoison(PoisonType poisontype, SI32 secs = INVALID);
		void			setOwner(P_CHAR owner);
		void			curePoison();
		void			resurrect(NXWCLIENT healer = NULL);
		void			unfreeze( LOGICAL calledByTempfx = false );
		void			damage(SI32 amount, DamageType typeofdamage = DAMAGE_PURE, StatType stattobedamaged = STAT_HP);
		void			playAction(SI32 action);

//@{
/*!
\name Talk and Emote stuff
*/
		UI16			emotecolor;		//!< Color for emote messages
		UI08			fonttype;		//!< Speech font to use
		UI16			saycolor;		//!< Color for say messages
		LOGICAL			unicode;		//!< This is set to 1 if the player uses unicode speech, 0 if not

		void			talkAll(TEXT *txt, LOGICAL antispam = 1);
		void			talk(NXWSOCKET s, TEXT *txt, LOGICAL antispam = 1);
		void			emote(NXWSOCKET s,TEXT *txt, LOGICAL antispam, ...);
		void			emoteall(char *txt, LOGICAL antispam, ...);
		void			talkRunic(NXWSOCKET s, TEXT *txt, LOGICAL antispam = 1);
		void			talkAllRunic(TEXT *txt, LOGICAL antispam = 0);
//@}

		UI32			distFrom(P_CHAR pc);
		UI32			distFrom(P_ITEM pi);
		void			teleport( UI08 flags = TELEFLAG_SENDALL, NXWCLIENT cli = NULL );
		void			facexy(SI32 facex, SI32 facey);
		LOGICAL			losFrom(P_CHAR pc);
		void			playSFX(SI16 sound, LOGICAL onlyToMe = false);
		void			playMonsterSound(MonsterSound sfx);

		void			freeze();
		LOGICAL			checkSkill(Skill sk, SI32 low, SI32 high, LOGICAL bRaise = true);
		SI32			delItems(short id, SI32 amount = 1, short color = INVALID);

		/*!
		\author Luxor
		\brief checks a skill for success (with sparring check) (call Skills::CheckSkillSparrCheck)
		\return true if success
		\param sk skill
		\param low low bound
		\param high high bound
		\todo document pcd parameter
		*/
		LOGICAL			checkSkillSparrCheck(Skill sk, SI32 low, SI32 high, P_CHAR pcd)
		{ return (Skills::CheckSkillSparrCheck(DEREF_P_CHAR(this),sk, low, high, pcd)); }


		UI32			getAmount(short id, short col=INVALID, bool onlyPrimaryBackpack=false );

		void			movingFX(P_CHAR dst, UI16 id, UI08 speed, UI08 loop, LOGICAL explode, class ParticleFx* part = NULL);
		void			staticFX(short id, SI32 speed, SI32 loop, class ParticleFx* part = NULL);
		void			boltFX(LOGICAL bNoParticles);
		void			circleFX(short id);

		P_ITEM			getBeardItem();
		P_ITEM			getHairItem();
		void			morph ( short bodyid = INVALID, short skincolor = INVALID,
								short hairstyle = INVALID, short haircolor = INVALID, short beardstyle = INVALID,
								short beardcolor = INVALID, const char* newname = NULL, LOGICAL bBackup = true);


//@{
/*!
\name possess stuff
\author Luxor
*/
		SERIAL			possessorSerial;
		SERIAL			possessedSerial;
		void			possess( P_CHAR pc );
//@}

		void			jail (SI32 seconds = INVALID)
		{ prison::jail( NULL, this, seconds ); } //!< Jails a char (see prison::jail)

		void			Kill();
		void			kick();
		void			goPlace(SI32);
		LOGICAL			knowsSpell(magic::SpellId spellnumber);

		void			setNpcMoveTime();

	public:
	#ifdef SPAR_NEW_WR_SYSTEM
		NxwCharWrapper*		nearbyChars;
		NxwItemWrapper*		nearbyItems;
	#endif

	public:
		std::vector< UI32 >	lootVector;

	public:
		virtual void		Delete();
	/*
	public:
		LOGICAL			isValidAmxEvent( UI32 eventId );
	*/

} PACK_NEEDED;


#endif
