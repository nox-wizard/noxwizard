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

#include "npcs.h"
#include "msgboard.h"
#include "target.h"
#include "constants.h"
#include "menu.h"
#include "logsystem.h"
#include "globals.h"

#include "basics.h"
#include "items.h"
#include "cmds.h"

#ifndef TIMEOUT
#define TIMEOUT(X) (((X) <= uiCurrentTime) || overflow)
#endif

enum WanderMode {
	WANDER_NOMOVE = 0,
	WANDER_FOLLOW,
	WANDER_FREELY_CIRCLE,
	WANDER_FREELY_BOX,
	WANDER_FREELY,
	WANDER_FLEE,
	WANDER_AMX
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

enum DispelType
{
	DISPELTYPE_UNKNOWN		= 0,
	DISPELTYPE_DISPEL,
	DISPELTYPE_TIMEOUT,
	DISPELTYPE_GMREMOVE
};

enum StatCap
{
	STATCAP_CAP			= 0,
	STATCAP_STR,
	STATCAP_DEX,
	STATCAP_INT
};

#define REPUTATION_KARMA		1
#define REPUTATION_FAME			2

class cNxwClientObj;
class cPath;
class ClientCrypt;

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
		static void		archive();
		static void		safeoldsave();
	public:
		void			MoveTo(Location newloc);
		void 			loadEventFromScript(TEXT *script1, TEXT *script2);

		inline void		MoveTo(SI32 x, SI32 y, SI08 z)
		{ MoveTo( Loc(x, y, z) ); }
	public:
//@{
/*!
\name nxwflags_chars
\brief Flags used for the char
*/
		static const UI08 flagGrey		= 0x01; //!< Char is grey
		static const UI08 flagPermaGrey		= 0x02; //!< Char is permanent grey
		static const UI08 flagResistFire	= 0x04; //!< Char resists to fire (unused)
		static const UI08 flagResistParalisys	= 0x08; //!< Char resists to paralisys (unused)
		static const UI08 flagResistPoison	= 0x10; //!< Char resists to poison (unused)
		static const UI08 flagWaterWalk		= 0x20;	//!< Char walks on water (npc only)
		static const UI08 flagSpellTelekinesys	= 0x40; //!< Char under telekinesys spell (Luxor)
		static const UI08 flagSpellProtection	= 0x80; //!< Char under protection spell (Luxor)

		static const UI08 flagKarmaInnocent	= 0x04; //!< Char is innocent
		static const UI08 flagKarmaMurderer	= 0x01; //!< Char is murderer
		static const UI08 flagKarmaCriminal	= 0x02; //!< Char is criminal
//@}
		char			nxwflags[4]; // for special nxw features

		inline const LOGICAL resistsFire() const
		{ return nxwflags[0] & flagResistFire; }

		inline const LOGICAL resistsFreeze() const
		{ return nxwflags[0] & flagResistParalisys; }

//@{
/*!
\name cchar_privs
\brief Priv variables used by priv and priv2
*/

		static const UI08 flagPrivGM		= 0x01; //!< Char is GM
		static const UI08 flagPrivBroadcast	= 0x02; //!< Char can broadcast
		static const UI08 flagPrivInvulnerable	= 0x04; //!< Char is invulnerable
		static const UI08 flagPrivCanViewSerials= 0x08; //!< Char can view serial codes
		static const UI08 flagPrivNoSkillTitle	= 0x10; //!< Char hasn't skill title
		static const UI08 flagPrivPageAble	= 0x20;
		static const UI08 flagPrivCanSnoopAll	= 0x40;
		static const UI08 flagPrivCounselor	= 0x80;

		static const UI08 flagPriv2AllMove	= 0x01;
		static const UI08 flagPriv2Frozen	= 0x02;
		static const UI08 flagPriv2ViewHouseIcon= 0x04;
		static const UI08 flagPriv2PermaHidden	= 0x08;
		static const UI08 flagPriv2NoUseMana	= 0x10;
		static const UI08 flagPriv2Dispellable	= 0x20;
		static const UI08 flagPriv2Reflection	= 0x40;
		static const UI08 flagPriv2NoUseReagents= 0x80;
//@}

//@{
/*!
\name Priviledges
*/
	protected:
		// We really need so many vars?
		UI08 priv;	//!< see cchar_privs
		UI08 priv2;	//!< see cchar_privs
		UI08 privLevel; //!< privilege level

		inline void setPriv2(UI08 priv, LOGICAL set)
		{
			if ( set )
				priv2 |= priv;
			else
				priv2 &= ~priv;
		}

	public:
		const LOGICAL		IsTrueGM() const;

		inline const LOGICAL	IsGM() const
		{ return ( privLevel > PRIVLEVEL_GM ); }

		inline const LOGICAL	IsCounselor() const
		{ return ( privLevel > PRIVLEVEL_CNS ); }

		const LOGICAL		IsGMorCounselor() const;

		inline const LOGICAL	IsInvul() const
		{ return priv & flagPrivInvulnerable; }

		inline const LOGICAL	CanSnoop() const
		{ return priv & flagPrivCanSnoopAll; }

		inline const LOGICAL	CanBroadcast() const
		{ return priv & flagPrivBroadcast; }

		inline const LOGICAL	CanSeeSerials() const
		{ return priv & flagPrivCanViewSerials; }

		inline const LOGICAL	isFrozen() const
		{ return (priv2 & flagPriv2Frozen); }

		inline const LOGICAL	isPermaHidden() const
		{ return priv2 & flagPriv2PermaHidden; }

		inline const LOGICAL	isDispellable() const
		{ return priv2 & flagPriv2Dispellable; }

		inline const LOGICAL	canAllMove() const
		{ return priv2 & flagPriv2AllMove; }

		inline const LOGICAL	dontUseMana() const
		{ return priv2 & flagPriv2NoUseMana; }

		inline const LOGICAL	dontUseReagents() const
		{ return priv2 & flagPriv2NoUseReagents; }

		inline const LOGICAL	hasReflection() const
		{ return priv2 & flagPriv2Reflection; }

		inline void setAllMove(LOGICAL set = true)
		{ setPriv2(flagPriv2AllMove, set); }

		inline void setViewHouseIcon(LOGICAL set = true)
		{ setPriv2(flagPriv2ViewHouseIcon, set); }

		inline void setPermaHidden(LOGICAL set = true)
		{ setPriv2(flagPriv2PermaHidden, set); }

		inline void setReflection(LOGICAL set = true)
		{ setPriv2(flagPriv2Reflection, set); }


		inline const UI08 	GetPriv() const
		{ return priv; }

		inline void 		SetPriv(UI08 p)
		{ priv = p; }

		inline void 		MakeInvulnerable()
		{ priv |= flagPrivInvulnerable; }

		inline void 		MakeVulnerable()
		{ priv &= ~flagPrivInvulnerable; }

		inline const UI08	GetPriv2() const
		{ return priv2; }

		inline void		SetPriv2(SI08 p)
		{ priv2 = p; }
//@}

//@{
/*!
\name Char Status
*/
	private:
		SI32		karma;		//!< karma of the char
		SI32    	fame;		//!< fame of the char
	public: // will become private
		LOGICAL		incognito;	//!< AntiChrist - true if under incognito effect
		LOGICAL		polymorph;	//!< AntiChrist - true if under polymorph effect

		UI32        	kills;		//!< PvP Kills
		UI32        	deaths;		//!< Number of deaths
		LOGICAL		dead;		//!< Is the character dead ?
		R32		fstm;		//!< Unavowed - stamina to remove the next step

	public:
		inline const LOGICAL IsInnocent() const
		{ return (flag & flagKarmaInnocent); }

		inline const LOGICAL IsMurderer() const
		{ return (flag & flagKarmaMurderer); }

		inline const LOGICAL IsCriminal() const
		{ return (flag & flagKarmaCriminal); }

		LOGICAL const		IsGrey() const;

		inline const LOGICAL	IsHidden() const
		{ return (hidden != UNHIDDEN); }

		inline const LOGICAL	IsHiddenBySpell() const
		{ return (hidden & HIDDEN_BYSPELL); }

		inline const LOGICAL	IsHiddenBySkill() const
		{ return (hidden & HIDDEN_BYSKILL); }

		LOGICAL const		IsOverWeight();
		LOGICAL const		IsOnline() const;

		inline const LOGICAL	InGuardedArea() const
		{ return ::region[region].priv & RGNPRIV_GUARDED; }

		LOGICAL const		CanDoGestures() const;

		void 			SetMurderer();
		void 			SetInnocent();
		void 			SetCriminal();

		inline void 		SetPermaGrey()
		{ nxwflags[0] |= flagGrey|flagPermaGrey; }

		//! Makes a character temporary grey
		void 			SetGrey()
		{ if (!npc) tempfx::add(this, this, tempfx::GREY, 0, 0, 0, 0x7FFF); }

		void 			unHide();

		//! Return the karma of the char
		inline const SI32	GetKarma() const
		{ return karma; }

		//! Set the karma of the char
		inline void		SetKarma(SI32 newkarma)
		{ karma = newkarma; }

		//! Return the fame of the char
		inline const SI32	GetFame() const
		{ return fame; }

		inline void		SetFame(SI32 newfame)
		{ fame=newfame; }

		void			IncreaseKarma(SI32 value, P_CHAR pKilled = 0 );
		void			modifyFame( SI32 value );
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
		//! Set the creation day of a character
		inline void		SetCreationDay(TIMERVAL day)
		{ creationday = day; }

		//! Get the creation day of a character
		inline const TIMERVAL	GetCreationDay() const
		{ return creationday; }
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

		void			setStrength(UI32 val, bool check= true);
		void			checkSafeStats();

		//! Get the strength-value
		inline const SI32	getStrength() const
		{ return str.value; }

		/*!
		\brief modify the strength
		\author Anthalir
		\since 0.82
		\param mod signed value representing the value to add to curent strength:
				\li negative: lower the str
				\li positive: rise the str
		\todo document check parameter
		*/
		inline void		modifyStrength(SI32 mod, bool check= true)
		{ setStrength( str.value + mod, check ); }
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

		inline const LOGICAL	HasHumanBody() const
		{ return (getId()==BODY_MALE) || (getId()==BODY_FEMALE); }

		void 			showLongName( P_CHAR showToWho, LOGICAL showSerials );
//@}


//@{
/*!
\name Combat
*/
	private:
		LOGICAL			attackfirst;		//!< 0 = defending, 1 = attacked first

	public:
		LOGICAL			HasAttackedFirst()
		{ return attackfirst; }

		inline void 		SetAttackFirst(LOGICAL af = true)
		{ attackfirst = af; }

		inline void 		ResetAttackFirst()
		{ SetAttackFirst(false); }

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

		/*!
		\author Luxor
		\brief Makes the char casting a spell
		\param spellnumber Spell identifier
		\param dest target location of the spell
		\todo Document parameters
		*/
		inline void		castSpell(magic::SpellId spellnumber, TargetLocation& dest, SI32 flags = 0, SI32 param = 0)
		{ magic::castSpell(spellnumber, dest, this, flags, param); }
//@}

//@{
/*!
\name Char Equipment
*/
	public:
		inline const LOGICAL	IsWearing(P_ITEM pi) const
		{ return getSerial32() == pi->getContSerial(); }

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
		cPath*			path;			//!< current path
		void			walkNextStep();		//!< walk next path step
		SERIAL_SLIST		sentObjects;
	public:
		LOGICAL			canSee( cObject &obj );	//!< can it see the object?
		LOGICAL			seeForFirstTime( cObject &obj );	//!< does it see the object for the first time?
		LOGICAL			seeForLastTime( cObject &obj ); //!< does it see the object for the first time?
		void			walk();			//!< execute walk code <Luxor>
		inline LOGICAL		hasPath() { return (path!=NULL); } //!< has a path set?
		void			follow( P_CHAR pc ); //!< follow pc
		void flee( P_CHAR pc, SI32 seconds=INVALID ); //!< flee from pc
		void			pathFind( Location pos, LOGICAL bOverrideCurrentPath = true );	//!< Walk to position
		SI08			dir;			//!< &0F=Direction
		UI32			LastMoveTime;		//!< server time of last move
//@}


//@{
/*!
\name Party
*/
	public:
		SI32			party;
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
		FUNCIDX		targetcallback;
		UI08			gmrestrict;			//!< for restricting GMs to certain regions

		void			doGmEffect();
//@}

	public:
		bool jailed;

//@{
/*!
\name Menu
*/
	public:
		SERIAL custmenu;	//!< legacy code, used for old mnu_ and hard coded menu

//@}


//@{
/*!
\name Trainer
*/
	public:
		SERIAL	trainer;		//!< Serial of the NPC training the char, -1 if none.
		UI08	trainingplayerin;	//!< Index in skillname of the skill the NPC is training the player in
		LOGICAL	cantrain;

	public:
		inline const LOGICAL	isBeingTrained() const
		{ return (trainer != INVALID); }

		inline const SERIAL	getTrainer() const
		{ return trainer; }

		inline const UI08	getSkillTaught() const
		{ return trainingplayerin; }

		inline const LOGICAL	canTrain() const
		{ return cantrain; }

		inline void		setCanTrain(LOGICAL c = true)
		{ cantrain = c; }

		inline void		resetCanTrain()
		{ setCanTrain(false); }
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
		P_GUILD guild; //!< the guild
		P_GUILD_MEMBER member; //!< the guild member info

	public:
		bool	isGuilded();
		void	setGuild( P_GUILD guild, P_GUILD_MEMBER member );
		P_GUILD	getGuild();
		P_GUILD_MEMBER getGuildMember();
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

public:
	TIMERVAL skilldelay;
	TIMERVAL objectdelay;
public:
	inline void setSkillDelay( UI32 seconds = server_data.skilldelay )
	{ skilldelay =  uiCurrentTime + seconds * MY_CLOCKS_PER_SEC; }

	inline const LOGICAL canDoSkillAction() const
	{ return TIMEOUT( skilldelay ); }

	inline void setObjectDelay( UI32 seconds = server_data.objectdelay )
	{ objectdelay = uiCurrentTime + seconds * MY_CLOCKS_PER_SEC; }

	inline const LOGICAL canDoObjectAction() const
	{ return TIMEOUT( objectdelay ); }


	/********************************/
	/*     TO REMOVE/REPLACE        */
	/********************************/
	public:
		void 			setMultiSerial(long mulser);

		inline const LOGICAL	isOwnerOf(const cObject *obj) const
		{ return getSerial32() == obj->getOwnerSerial32(); }

	/********************************/

		SERIAL			keyserial;  //!< for renaming keys
		SERIAL			packitemserial; //!< Only used during character creation
		SERIAL			targserial; //!< Current combat target
		SERIAL			runeserial; //!< Used for naming runes
		SERIAL			attackerserial; //!< Character who attacked this character
		SERIAL			ftargserial; //!< NPC Follow Target
		SERIAL			swingtargserial; //!< Target they are going to hit after they swing
		SI32			namedeedserial;

		SI32 			account; // changed to signed, lb



	public:
		wstring profile; //!< player profile

	private:
		wstring* speechCurrent;
	public:
		//! Return current speech
		inline const wstring* getSpeechCurrent() const
		{ return speechCurrent; }

		//! Set current speech
		inline void setSpeechCurrent( wstring* speech )
		{ speechCurrent=speech; }

		//! Reset current speech
		inline void resetSpeechCurrent()
		{ setSpeechCurrent(NULL); }

		void deleteSpeechCurrent();

	private:
		SERIAL	stablemaster_serial; //!< the stablemaster serial
	public:
		//! Check if char is stabled
		inline const LOGICAL isStabled() const
		{ return getStablemaster()!=INVALID; }

		//! Get the character's stablemaster
		inline const SERIAL getStablemaster() const
		{ return stablemaster_serial; }

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
		UI16			baseskill[ALLSKILLS+1]; // Base skills without stat modifiers
		UI16			skill[ALLSKILLS+1]; // List of skills (with stat modifiers)


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
		SI32			fleeTimer;
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
		TIMERVAL		fishingtimer; // Timer used to delay the catching of fish

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
		std::string		trigword; //Word that character triggers on.
		UI16			envokeid;
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
		SI32			casting; // 0/1 is the cast casting a spell?
		TIMERVAL		spelltime; //Time when they are done casting....
		magic::SpellId		spell; //current spell they are casting....
		SI32			spellaction; //Action of the current spell....
		magic::CastingType	spelltype;
		TIMERVAL		nextact; //time to next spell action....
		TargetLocation*		spellTL; //Luxor: npc spell targetlocation

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

		char			npc_type;		// currently only used for stabling, (type==1 -> stablemaster)
							// can be used for other npc types too of course

		TIMERVAL 		time_unused;
		TIMERVAL 		timeused_last;

		void 			doSingleClick( SERIAL serial );
		void			onSingleClick( P_CHAR clickedBy );

	private:
		inline void	resetBaseSkill()
		{ memset(baseskill, 0, sizeof(baseskill)); }

		inline void	resetSkill()
		{ memset(skill, 0, sizeof(skill)); }

		inline void	resetNxwFlags()
		{ memset(nxwflags, 0, sizeof(nxwflags)); }

		inline void	resetAmxEvents()
		{ memset(amxevents, 0, sizeof(amxevents)); }

		inline void	resetResists()
		{ memset(resists, 0, sizeof(resists)); }

		inline void	resetLockSkills()
		{ memset(lockSkill, 0, sizeof(lockSkill)); }

	private:

		void 			doSingleClickOnCharacter( SERIAL serial );	// will become private function
		void 			doSingleClickOnItem( SERIAL serial );		// will become private function

	public:
		//! tells if a character is running
		inline const LOGICAL	isRunning() const
		{ return ( (uiCurrentTime - lastRunning) <= 100 ); }

		inline void		setRunning()
		{ lastRunning = uiCurrentTime; }

		void 			updateStats(SI32 stat);

		void 			setNextMoveTime(short tamediv=1);
		void 			disturbMed();

		void                    drink(P_ITEM pi);       //Luxor: delayed drinking
		void 			hideBySkill();
		void 			hideBySpell(SI32 timer = INVALID);
		UI32  			CountItems(short ID, short col= INVALID);

		inline const UI32 	CountGold()
		{ return CountItems(ITEMID_GOLD); }

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
		LOGICAL			isInBackpack( P_ITEM pi );

		//! Show Backpack to player
		inline void		showBackpack()
		{ showContainer( getBackpack() ); }

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

		//! get the client
		inline NXWCLIENT	getClient() const
		{ return m_client; }

		NXWSOCKET		getSocket() const;
		void			sysmsg(const TEXT *txt, ...);
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

		//! Freeze the char
		inline void		freeze()
		{ priv2 |= flagPriv2Frozen; }

		LOGICAL			checkSkill(Skill sk, SI32 low, SI32 high, LOGICAL bRaise = true);
		SI32			delItems(short id, SI32 amount = 1, short color = INVALID);

		const LOGICAL	checkSkillSparrCheck(Skill sk, SI32 low, SI32 high, P_CHAR pcd);

		UI32			getAmount(short id, short col=INVALID, bool onlyPrimaryBackpack=false );

		void			movingFX(P_CHAR destination, short id, SI32 speed, SI32 loop, LOGICAL explode, class ParticleFx* part = NULL);
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

		void			jail (SI32 seconds = 60*60*24);
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
#ifdef ENCRYPTION
//@{
/*!
\name crypt
\brief  Encryption per client
*/
	public:
		inline void setCrypter(ClientCrypt * crypt)
		{ crypter=crypt; }

		inline ClientCrypt * getCrypter() const
		{ return crypter; }

	private:
		ClientCrypt * crypter;
//@}
#endif
} PACK_NEEDED;


#define MAKE_CHAR_REF(i) pointers::findCharBySerial(i)

#define DEREF_P_CHAR(pc) ( ( ISVALIDPC(pc) ) ? pc->getSerial32() : INVALID )

#define ISVALIDPC(pc) ( ( pc!=NULL && sizeof(*pc) == sizeof(cChar) ) ? (pc->getSerial32() >= 0) : false )

#define VALIDATEPC(pc) if (!ISVALIDPC(pc)) { LogWarning("a non-valid P_CHAR pointer was used in %s:%d", basename(__FILE__), __LINE__); return; }
#define VALIDATEPCR(pc, r) if (!ISVALIDPC(pc)) { LogWarning("a non-valid P_CHAR pointer was used in %s:%d", basename(__FILE__), __LINE__); return r; }



#endif
