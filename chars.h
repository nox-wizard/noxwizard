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

#include "object.h"
#include "magic.h"

#include "char_array.h"
#include "npcs.h"
#include "msgboard.h"
#include "target.h"
#include "constants.h"
#include "unicode.h"

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
	EVENT_CHR_ONDEATH = 0,
	EVENT_CHR_ONWOUNDED,
	EVENT_CHR_ONHIT,
	EVENT_CHR_ONHITMISS,
	EVENT_CHR_ONGETHIT,
	EVENT_CHR_ONREPUTATIONCHG,
	EVENT_CHR_ONDISPEL,
	EVENT_CHR_ONRESURRECT,
	EVENT_CHR_ONFLAGCHG,
	EVENT_CHR_ONWALK,
	EVENT_CHR_ONADVANCESKILL,
	EVENT_CHR_ONADVANCESTAT,
	EVENT_CHR_ONBEGINATTACK,
	EVENT_CHR_ONBEGINDEFENSE,
	EVENT_CHR_ONTRANSFER,
	EVENT_CHR_ONMULTIENTER,
	EVENT_CHR_ONMULTILEAVE,
	EVENT_CHR_ONSNOOPED,
	EVENT_CHR_ONSTOLEN,
	EVENT_CHR_ONPOISONED,
	EVENT_CHR_ONREGIONCHANGE,
	EVENT_CHR_ONCASTSPELL,
	EVENT_CHR_ONGETSKILLCAP,
	EVENT_CHR_ONGETSTATCAP,
	EVENT_CHR_ONBLOCK,
	EVENT_CHR_ONSTART,
	EVENT_CHR_ONHEARTBEAT,
	EVENT_CHR_ONBREAKMEDITATION,
	EVENT_CHR_ONCLICK,
	EVENT_CHR_ONMOUNT,
	EVENT_CHR_ONDISMOUNT,
	EVENT_CHR_ONKILL,
	EVENT_CHR_ONHEARPLAYER,
	EVENT_CHR_ONDOCOMBAT,
	EVENT_CHR_ONCOMBATHIT,
	EVENT_CHR_ONSPEECH,
	ALLCHAREVENTS
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

/*!
\brief Character class
*/
class cChar : public cObject
{
	public:
		cChar( SERIAL ser );
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
		void			MoveTo(SI32 x, SI32 y, SI08 z);
		void 			loadEventFromScript(TEXT *script1, TEXT *script2);
//@{
/*!
 \name Guilds
 \brief Guild related functions and attributes
*/
	private:
		short 			guildType;		//!< (0) Standard guild, (1) Chaos Guild, (2) Order guild
		LOGICAL			guildTraitor;		//!< (true) This character converted, (false) Neve converted, or not an order/chaos guild member
		LOGICAL			guildToggle;		//!< Toggle for Guildtitle
		SERIAL			guildFealty;		//!< Serial of player you are loyal to (default=yourself)
		SI32			guildNumber;		//!< Number of guild player is in (0=no guild)
		TEXT			guildTitle[21];		//!< Title Guildmaster granted player

	public:
		void			SetGuildType( short newGuildType );
		short			GetGuildType();

		LOGICAL			IsGuildTraitor();
		void			SetGuildTraitor();
		void			ResetGuildTraitor();

		LOGICAL			HasGuildTitleToggle();
		void			SetGuildTitleToggle();
		void			ResetGuildTitleToggle();

		SERIAL			GetGuildFealty();
		void			SetGuildFealty( SERIAL newGuildFealty );

		SI32			GetGuildNumber();
		void			SetGuildNumber( SI32 newGuildNumber );

		void			SetGuildTitle( TEXT *newGuildTitle );
		TEXT*			GetGuildTitle();
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
		TIMERVAL		GetCreationDay();		//!< Get the creation day of a character
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

		SI32			getStrength();			//!< Get the strength-value

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
		char			title[50];
		UI16			oldhairstyle;
		UI16			oldbeardstyle;
		UI16			oldhaircolor;
		UI16			oldbeardcolor;
	//
	//	Body Type
	//
	public:
		UI08			id1;				//!< Character body type
		UI08			id2;				//!< Character body type
		UI08			xid1;				//!< Backup of body type for ghosts
		UI08			xid2;				//!< Backup of body type for ghosts
	public:
		BODYTYPE		GetBodyType() const;
		void			SetBodyType(BODYTYPE newBody);
		BODYTYPE		GetOldBodyType() const;
		void			SetOldBodyType(BODYTYPE newBody);
		const LOGICAL		HasHumanBody() const;
	//
	//	Skin Color
	//
	public:
		UI08			skin1;				//!< Skin color
		UI08			skin2;				//!< Skin color
		UI08			xskin1;				//!< Backup of skin color
		UI08			xskin2;				//!< Backup of skin color
	public:
		UI32			getSkinColor();
		void			setSkinColor( UI32 newColor );
		UI32			getOldSkinColor();
		void			setOldSkinColor( UI32 newColor );

		void 			showLongName( P_CHAR showToWho, LOGICAL showSerials );
//@}


//@{
/*!
\name Combat
*/
	private:
		LOGICAL			attackfirst;		//!< 0 = defending, 1 = attacked first

	public:
		LOGICAL			HasAttackedFirst();
		void 			SetAttackFirst();
		void 			ResetAttackFirst();

	private:
		LOGICAL			combatTimerOk();
		void			checkPoisoning(P_CHAR pc_def);
		void			doMissedSoundEffect();
		SI32			combatHitMessage(SI32 damage);
		void			doCombatSoundEffect(SI32 fightskill, P_ITEM pWeapon);

	public:
		void			checkPoisoning();
		void 			fight(P_CHAR pOpponent);
		void			castSpell(magic::SpellId spellnumber, TargetLocation& dest, SI32 flags = 0, SI32 param = 0);
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
		char			priv2;	            /*!< 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
										10: no need mana, 20: dispellable, 40: permanent magic reflect, 80: no need reagents
						*/
		SI32 			priv3[7];           //!< needed for Lord binarys meta-gm stuff

	public:
		const LOGICAL		IsTrueGM() const;
		const LOGICAL		IsGM() const;
		const LOGICAL		IsCounselor() const;
		const LOGICAL		IsGMorCounselor() const;
		const LOGICAL		IsInvul() const;
		const LOGICAL		CanSnoop() const;
		const LOGICAL		CanBroadcast() const;
		const LOGICAL		CanSeeSerials() const;

		UI08 			GetPriv() const;
		void 			SetPriv(UI08 p);

		void 			MakeInvulnerable();
		void 			MakeVulnerable();

		char			GetPriv2() const;	//!< Temporary workaround until priv vars are settled
		void 			SetPriv2(UI08 p);
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
		LOGICAL const		IsInnocent() const;
		LOGICAL const		IsMurderer() const;
		LOGICAL const		IsCriminal() const;
		LOGICAL const		IsGrey() const;
		LOGICAL const 		IsHidden() const;
		LOGICAL const 		IsHiddenBySpell() const;
		LOGICAL const 		IsHiddenBySkill() const;
		LOGICAL const		IsOverWeight();
		LOGICAL const		InGuardedArea() const;
		LOGICAL const		IsOnline() const;
		LOGICAL const		IsFrozen() const;
		LOGICAL const		CanDoGestures() const;

		void 			SetMurderer();
		void 			SetInnocent();
		void 			SetCriminal();
		void 			SetPermaGrey();
		void 			SetGrey();
		void 			unHide();

		const SI32		GetKarma() const;
		void			SetKarma(SI32 newkarma);
		void			IncreaseKarma(SI32 value, P_CHAR killed=NULL);

		const SI32		GetFame() const;
		void			SetFame(SI32 newfame);
		void			modifyFame( SI32 value );
//@}

//@{
/*!
\name Char Equipment
*/
	public:
		LOGICAL			IsWearing(P_ITEM pi);
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
	public:
		char			dir;			//!< &0F=Direction
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

		UI08			pathnum;
		path_st			path[PATHNUM];

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
		char			trainingplayerin;
			//!< Index in skillname of the skill the NPC is training the player in
		LOGICAL			cantrain;

	public:
		LOGICAL			isBeingTrained();
		SERIAL			getTrainer();

		char			getSkillTaught();

		LOGICAL			canTrain();
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
*/
	private:
		SERIAL			guild; //!< Serial of guild

	public:
		bool			isGuilded();
		void			setGuild(SERIAL newGuild);
		SERIAL			getGuild();
//@}

//@{
/*!
\name Target
*/
	private:
		P_TARGET		current_target; //!< target

	public:
		bool			isTargeting();
		void			setTarget( P_TARGET newtarget );
		void			doTarget();
//@}

		/********************************/
	/*     TO REMOVE/REPLACE        */
	/********************************/
	public:
		void 			setMultiSerial(long mulser);
		const LOGICAL		isOwnerOf(const cObject *obj) const;

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

	public:

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
		TIMERVAL		regen, regen2, regen3;//Regeneration times for mana, stamin, and str

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
		SI32			npcaitype; // NPC ai
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
		TEXT			trigword[31]; //Word that character triggers on.
		TIMERVAL		disabled; //Time character is disabled, cant trigger.
		TEXT			disabledmsg[256]; //Character is disabled, so dysplay this message. -- added by Magius(CHE) �
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


		cUnicodeString* profile; //!< player profile
		cUnicodeString* staticProfile; //!< player static profile


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

		SERIAL			spawnserial; //!< Spawned by dinamic
		SERIAL			spawnregion; //!< Spawned by scripted

		SERIAL			stablemaster_serial;
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


		cUnicodeString* speechCurrent;
		void 			updateStats(SI32 stat);

		void 			setNextMoveTime(short tamediv=1);
		void 			disturbMed();

		void                    drink(P_ITEM pi);       //Luxor: delayed drinking
		void 			hideBySkill();
		void 			hideBySpell(SI32 timer = INVALID);
		UI32  			CountItems(short ID, short col= INVALID);
		UI32  			CountGold();
		P_ITEM 			GetItemOnLayer(UI08 layer);
		P_ITEM 			GetBankBox( short type = BANK_GOLD);
		void			openBankBox( CHARACTER cc );
		SI32  			countBankGold();
	//	void 			addHalo(P_ITEM pi);
	//	void 			removeHalo(P_ITEM pi);
	//	void 			glowHalo(P_ITEM pi);
		P_ITEM 			getWeapon();
		P_ITEM 			getShield();
		void			showContainer( P_ITEM pCont );
		P_ITEM 			getBackpack();
		void			showBackpack();
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
		LOGICAL			resist(SI32 n)		 { return ((nxwflags[0]&n)!=0); }    // <-- what is this ?, xan

		NXWCLIENT		getClient() const;
		NXWSOCKET		getSocket() const;
		void			sysmsg(TEXT *txt, ...);
		void			attackStuff(P_CHAR pc);
		void			helpStuff(P_CHAR pc_i);
		void			applyPoison(PoisonType poisontype, SI32 secs = INVALID);
		void			setOwner(P_CHAR owner);
		void			curePoison();
		void			resurrect(NXWCLIENT healer = NULL);
		void			unfreeze();
		void			damage(SI32 amount, DamageType typeofdamage = DAMAGE_PURE, StatType stattobedamaged = STAT_HP);
		void			playAction(SI32 action);

//@{
/*!
\name Talk and Emote stuff
*/
		UI08			emotecolor1;		//!< Color for emote messages
		UI08			emotecolor2;		//!< Color for emote messages
		UI08			fonttype;		//!< Speech font to use
		UI08			saycolor1;		//!< Color for say messages
		UI08			saycolor2;		//!< Color for say messages
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
		void			teleport();
		void			facexy(SI32 facex, SI32 facey);
		LOGICAL			losFrom(P_CHAR pc);
		void			playSFX(SI16 sound);
		void			playMonsterSound(MonsterSound sfx);

		void			freeze();
		LOGICAL			checkSkill(Skill sk, SI32 low, SI32 high, LOGICAL bRaise = true);
		LOGICAL			checkSkillSparrCheck(Skill sk, SI32 low, SI32 high, P_CHAR pcd);
		SI32			delItems(short id, SI32 amount = 1, short color = INVALID);



		UI32			getAmount(short id, short col=INVALID, bool onlyPrimaryBackpack=false );

		void			movingFX(P_CHAR destination, short id, SI32 speed, SI32 loop, LOGICAL explode, class ParticleFx* part = NULL);
		void			staticFX(short id, SI32 speed, SI32 loop, class ParticleFx* part = NULL);
		void			boltFX(LOGICAL bNoParticles);
		void			circleFX(short id);

		void			spawnItemInBackpack(SI32 itemidx, LOGICAL bDigging = false);
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

		void			jail (SI32 seconds = INVALID);
		void			Kill();
		void			kick();
		void			goPlace(SI32);
		LOGICAL			knowsSpell(magic::SpellId spellnumber);

		void			setNpcMoveTime();

		void			deleteChar();

	public:
	#ifdef SPAR_NEW_WR_SYSTEM
		NxwCharWrapper*		nearbyChars;
		NxwItemWrapper*		nearbyItems;
	#endif

	public:
		std::vector< UI32 >	lootVector;
} PACK_NEEDED;

void openspecialbank(NXWSOCKET s, int i);

#endif
