  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "nxwcommn.h"
#include "itemid.h"
#include "sregions.h"
#include "sndpkg.h"
#include "magic.h"
#include "debug.h"
#include "amx/amxcback.h"
#include "set.h"
#include "tmpeff.h"
#include "race.h"
#include "npcai.h"
#include "layer.h"
#include "network.h"
#include "set.h"
#include "accounts.h"
#include "jail.h"
#include "nxw_utils.h"
#include "weight.h"
#include "boats.h"
#include "archive.h"
#include "walking.h"
#include "rcvpkg.h"
#include "map.h"
#include "chars.h"
#include "items.h"
#include "inlines.h"
#include "basics.h"
#include "magic.h"
#include "skills.h"
#include "range.h"
#include "classes.h"
#include "utils.h"
#include "nox-wizard.h"
#include "targeting.h"
#include "cmds.h"


void cChar::setClient(NXWCLIENT client)
{
	if (m_client != NULL) safedelete(m_client);
	m_client = client;
}

void cChar::archive()
{
	std::string saveFileName( SrvParms->savePath + SrvParms->characterWorldfile + SrvParms->worldfileExtension );
	std::string timeNow( getNoXDate() );
	for( SI32 i = timeNow.length() - 1; i >= 0; --i )
		switch( timeNow[i] )
		{
			case '/' :
			case ' ' :
			case ':' :
				timeNow[i]= '-';
		}
	std::string archiveFileName( SrvParms->archivePath + SrvParms->characterWorldfile + timeNow + SrvParms->worldfileExtension );


	if( rename( saveFileName.c_str(), archiveFileName.c_str() ) != 0 )
	{
		LogWarning("Could not rename/move file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
	}
	else
	{
		InfoOut("Renamed/moved file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
	}
}

void cChar::safeoldsave()
{
	std::string oldFileName( SrvParms->savePath + SrvParms->characterWorldfile + SrvParms->worldfileExtension );
	std::string newFileName( SrvParms->savePath + SrvParms->characterWorldfile + SrvParms->worldfileExtension + "$" );
	remove( newFileName.c_str() );
	rename( oldFileName.c_str(), newFileName.c_str() );
}

cChar::cChar( SERIAL ser ) : cObject()
{

	m_client = NULL;

	setSerial32(ser);

	setMultiSerial32Only(INVALID);//Multi serial
	setOwnerSerial32Only(INVALID);


	setCurrentName("<this is a bug>");
	setRealName("<this is a bug>");
	title[0]=0x00;


	///TIMERS
	antiguardstimer=uiCurrentTime;
	antispamtimer=uiCurrentTime;
	begging_timer=uiCurrentTime;
	fishingtimer=uiCurrentTime;
	hungertime=uiCurrentTime;
	invistimeout=uiCurrentTime;
	nextact=uiCurrentTime;
	nextAiCheck=uiCurrentTime;
	npcmovetime=uiCurrentTime;
	skilldelay=uiCurrentTime;
	//

	gmMoveEff=0;
	homeloc.x=0;
	homeloc.y=0;
	homeloc.z=0;
	workloc.x=0;
	workloc.y=0;
	workloc.z=0;
	foodloc.x=0;
	foodloc.y=0;
	foodloc.z=0;


	party=INVALID;
	privLevel = PRIVLEVEL_GUEST;

	setId( BODY_MALE );
	race=INVALID;
	custmenu=INVALID;
	unicode = false; // This is set to 1 if the player uses unicode speech, 0 if not
	account=INVALID;
	setPosition( 100, 100, 0 );
	setOldPosition( 0, 0, 0, 0 );
	dir=0; //&0F=Direction
	setOldId( 0x0190 ); // Character body type
	setOldColor( 0x0000 ); // Skin color
	keyserial=INVALID;  // for renaming keys
	SetPriv(0); // 1:GM clearance, 2:Broadcast, 4:Invulnerable, 8: single click serial numbers
	// 10: Don't show skill titles, 20: GM Pagable, 40: Can snoop others packs, 80: Counselor clearance
	priv2=0;	// 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
	// 10: no need mana, 20: dispellable, 40: permanent magic reflect, 80: no need reagents
	fonttype=3; // Speech font to use
	saycolor=0x1700; // Color for say messages
	emotecolor=0x0023; // Color for emote messages
	setStrength(50, ( ser == INVALID ? false : true ) ); // Strength
	st2=0; // Reserved for calculation
	dx=50; // Dexterity
	dx2=0; // Reserved for calculation
	in=50; // Intelligence
	in2=0; // Reserved for calculation
	hp=50; // Hitpoints
	stm=50; // Stamina
	mn=50; // Mana
	mn2=0; // Reserved for calculation
	hidamage=0; //NPC Damage
	lodamage=0; //NPC Damage
	SetCreationDay( getclockday() );
	resetSkill();
	resetBaseSkill();
	npc=0;
	shopkeeper = false; //true=npc shopkeeper
	setOwnerSerial32Only(INVALID);
	tamed = false; // True if NPC is tamed
	robe = INVALID; // Serial number of generated death robe (If char is a ghost)
	SetKarma(0);
	fame=0;
	//pathnum = PATHNUM;
	kills=0; //PvP Kills
	deaths=0;
	dead = false; // Is character dead
	packitemserial=INVALID; // Only used during character creation
	fixedlight=255; // Fixed lighting level (For chars in dungeons, where they dont see the night)
	speech=0; // For NPCs: Number of the assigned speech block
	weight=0; //Total weight
	att=0; // Intrinsic attack (For monsters that cant carry weapons)
	def=0; // Intrinsic defense
	war=0; // War Mode
	questType = MsgBoards::QTINVALID; //xan - was not initialized!
	targserial=INVALID; // Current combat target
	timeout=0; // Combat timeout (For hitting)
	timeout2=0;

	setRegenRate( STAT_HP, SrvParms->hitpointrate, VAR_REAL );
	setRegenRate( STAT_HP, SrvParms->hitpointrate, VAR_EFF );
	updateRegenTimer( STAT_HP );
	setRegenRate( STAT_STAMINA, SrvParms->staminarate, VAR_REAL );
	setRegenRate( STAT_STAMINA, SrvParms->staminarate, VAR_EFF );
	updateRegenTimer( STAT_STAMINA );
	setRegenRate( STAT_MANA, SrvParms->manarate, VAR_REAL );
	setRegenRate( STAT_MANA, SrvParms->manarate, VAR_EFF );
	updateRegenTimer( STAT_MANA );

	runeserial=INVALID; // Used for naming runes
	attackerserial=INVALID; // Character who attacked this character
	nextAiCheck=uiCurrentTime;

	npcmovetime=uiCurrentTime; // Next time npc will walk
	npcWander=WANDER_NOMOVE; // NPC Wander Mode
	fleeTimer=INVALID;
	oldnpcWander=WANDER_NOMOVE; // Used for fleeing npcs
	ftargserial=INVALID; // NPC Follow Target
	fx1=-1; //NPC Wander Point 1 x
	fx2=-1; //NPC Wander Point 2 x
	fy1=-1; //NPC Wander Point 1 y
	fy2=-1; //NPC Wander Point 2 y
	fz1=0; //NPC Wander Point 1 z

	spawnserial=INVALID; // Spawned by
	hidden=UNHIDDEN;
	invistimeout=0;
	ResetAttackFirst(); // 0 = defending, 1 = attacked first
	onhorse=false; // On a horse?
	hunger=6;  // Level of hungerness, 6 = full, 0 = "empty"
	hungertime=0; // Timer used for hunger, one point is dropped every 20 min
	npcaitype=NPCAI_GOOD; // NPC ai
	callnum=-1; //GM Paging
	playercallnum=-1; //GM Paging
	pagegm=0; //GM Paging
	region=255;
	skilldelay=0;
	objectdelay=0;
	combathitmessage=0;
	making=-1; // skill number of skill using to make item, 0 if not making anything.
	blocked=0;
	dir2=0;
	spiritspeaktimer=0; // Timer used for duration of spirit speak
	spattack=0;
	spadelay=0;
	spatimer=0;
	taming=0; //Skill level required for taming
	summontimer=0; //Timer for summoned creatures.
	fishingtimer=0; // Timer used to delay the catching of fish
	advobj=0; //Has used advance gate?
	poison=0; // used for poison skill
	poisoned=POISON_NONE; // type of poison
	poisontime=0; // poison damage timer
	poisontxt=0; // poision text timer
	poisonwearofftime=0; // LB, makes poision wear off ...
	fleeat=0;
	reattackat=0;
	trigger=0; //Trigger number that character activates
	trigword[0]='\x00'; //Word that character triggers on.
	envokeid=0;
	envokeitem=-1;
	split=0;
	splitchnc=0;
	targtrig=0; //Stores the number of the trigger the character for targeting
	ra=0;  // Reactive Armor spell
	trainer=INVALID; // Serial of the NPC training the char, -1 if none.
	trainingplayerin=0; // Index in skillname of the skill the NPC is training the player in
	cantrain=true;
	ResetGuildTitleToggle();		// Toggle for Guildtitle								(DasRaetsel)
	SetGuildTitle( "" );	// Title Guildmaster granted player 					(DasRaetsel)
	SetGuildFealty( INVALID ); 	// Serial of player you are loyal to (default=yourself) (DasRaetsel)
	SetGuildNumber( 0 );		// Number of guild player is in (0=no guild)			(DasRaetsel)

	flag=0x02; //1=red 2=grey 4=Blue 8=green 10=Orange // grey as default - AntiChrist
	tempflagtime=0;

	murderrate=0; //#of ticks until one murder decays //REPSYS
	murdersave=0;

	crimflag=0; //Time when No longer criminal
	casting=0; // 0/1 is the cast casting a spell?
	spelltime=0; //Time when they are done casting....
	spell=magic::SPELL_INVALID; //current spell they are casting....
	spellaction=0; //Action of the current spell....
	nextact=0; //time to next spell action....
	squelched=0; // zippy  - squelching
	mutetime=0; //Time till they are UN-Squelched.
	med=0; // 0=not meditating, 1=meditating //Morrolan - Meditation
	stealth=-1; //AntiChrist - stealth ( steps already done, -1=not using )
	running=0; //AntiChrist - Stamina Loose while running
	logout=0;//Time till logout for this char -1 means in the world or already logged out //Instalog
	swingtargserial=INVALID; //Tagret they are going to hit after they swing
	holdg=0; // Gold a player vendor is holding for Owner
	fly_steps=0; //LB -> used for flyging creatures
	guarded=false; // True if CHAR is guarded by some NPC
	smoketimer=0;
	smokedisplaytimer=0;
	carve=-1; // AntiChrist - for new carving system
	antiguardstimer=0; // AntiChrist - for "GUARDS" call-spawn
	polymorph=false;//polymorph - AntiChrist
	incognito=false;//incognito - AntiChrist
	namedeedserial=INVALID;
	postType = MsgBoards::LOCALPOST;
	questDestRegion = 0;
	questOrigRegion= 0;
	questBountyReward= 0;
	questBountyPostSerial = 0;
	murdererSer = 0;
	spawnregion = INVALID;
	npc_type = 0;
	stablemaster_serial = INVALID;
	timeused_last = getclock();
	time_unused = 0;
	npcMoveSpeed = (float)NPCSPEED;
	npcFollowSpeed = (float)NPCFOLLOWSPEED;
	setNpcMoveTime();
	resetNxwFlags();
	resetAmxEvents();
	prevX = prevY = prevZ = 0;
	ResetGuildTraitor();
	SetGuildType( INVALID );
	magicsphere = 0;
	resetResists();
	lightdamaged = false;
	holydamaged = false;
	damagetype = DAMAGE_BLUDGEON;
	fstm=0.0f;

	setGuild( NULL, NULL );

	jailed=false;
	morphed=0;
	resetLockSkills();

	this->beardserial=INVALID;
	this->oldbeardcolor=0;
	this->oldbeardstyle=0;

	this->hairserial=INVALID;
	this->oldhaircolor=0;
	this->oldhairstyle=0;

	possessorSerial = INVALID; //Luxor
	possessedSerial = INVALID; //Luxor

	mounted=false;
	lootVector.clear();

	SetInnocent(); //Luxor
	targetcallback = INVALID;

	statGainedToday = 0;	//Luxor

	speechCurrent = NULL; //Luxor
	lastRunning = 0; //Luxor
	path = NULL; //Luxor
	spellTL = NULL; //Luxor

	staticProfile=NULL;

	vendorItemsSell = NULL;
	vendorItemsBuy = NULL;

	oldmenu=INVALID;

	commandLevel=0;
	// initializing crypt
#ifdef ENCRYPTION
	crypter = NULL;
#endif
	// initializing amx
	resetAmxEvents();
}

/*
\brief Destructor
*/
cChar::~cChar()
{
	if( staticProfile!=NULL )
		safedelete( staticProfile );
	if( speechCurrent!=NULL )
		safedelete( speechCurrent );
	if ( path != NULL )
		safedelete( path );
#ifdef ENCRYPTION
	if ( crypter != NULL )
		safedelete(crypter);
#endif
}


/*!
\note Don't add onstart events in npc scripts, because then they'll also be executed when character is created
add onstart event to character programmatically
*/
void cChar::loadEventFromScript(TEXT *script1, TEXT *script2)
{

#define CASECHAREVENT( NAME, ID ) 	else if (!strcmp( NAME,script1))	amxevents[ID] = newAmxEvent(script2);


	if (!strcmp("@ONSTART",script1))	{
		amxevents[EVENT_CHR_ONSTART] = newAmxEvent(script2);
		newAmxEvent(script2)->Call(getSerial32(), INVALID);
	}
	CASECHAREVENT("@ONDEATH",EVENT_CHR_ONBEFOREDEATH)
	CASECHAREVENT("@ONBEFOREDEATH",EVENT_CHR_ONBEFOREDEATH)		// SYNONYM OF ONDEATH EVENT
	CASECHAREVENT("@ONDIED",EVENT_CHR_ONAFTERDEATH)
	CASECHAREVENT("@ONAFTERDEATH",EVENT_CHR_ONAFTERDEATH)			// SYNONYM OF ONDIED EVENT
	CASECHAREVENT("@ONKILL",EVENT_CHR_ONKILL)
	CASECHAREVENT("@ONWOUNDED",EVENT_CHR_ONWOUNDED)
	CASECHAREVENT("@ONHIT",EVENT_CHR_ONHIT)
	CASECHAREVENT("@ONHITMISS",EVENT_CHR_ONHITMISS)
	CASECHAREVENT("@ONGETHIT",EVENT_CHR_ONGETHIT)
	CASECHAREVENT("@ONWALK",EVENT_CHR_ONWALK)
	CASECHAREVENT("@ONBLOCK",EVENT_CHR_ONBLOCK)
	CASECHAREVENT("@ONHEARTBEAT",EVENT_CHR_ONHEARTBEAT)
	CASECHAREVENT("@ONDISPEL",EVENT_CHR_ONDISPEL)
	CASECHAREVENT("@ONRESURRECT",EVENT_CHR_ONRESURRECT)
	CASECHAREVENT("@ONFLAGCHANGE",EVENT_CHR_ONFLAGCHG)
	CASECHAREVENT("@ONADVANCESKILL",EVENT_CHR_ONADVANCESKILL)
	CASECHAREVENT("@ONGETSKILLCAP",EVENT_CHR_ONGETSKILLCAP)
	CASECHAREVENT("@ONGETSTATCAP",EVENT_CHR_ONGETSTATCAP)
	CASECHAREVENT("@ONADVANCESTAT",EVENT_CHR_ONADVANCESTAT)
	CASECHAREVENT("@ONBEGINATTACK",EVENT_CHR_ONBEGINATTACK)
	CASECHAREVENT("@ONBEGINDEFENSE",EVENT_CHR_ONBEGINDEFENSE)
	CASECHAREVENT("@ONTRANSFER",EVENT_CHR_ONTRANSFER)
	CASECHAREVENT("@ONMULTIENTER",EVENT_CHR_ONMULTIENTER)
	CASECHAREVENT("@ONMULTILEAVE",EVENT_CHR_ONMULTILEAVE)
	CASECHAREVENT("@ONSNOOPED",EVENT_CHR_ONSNOOPED)
	CASECHAREVENT("@ONSTOLEN",EVENT_CHR_ONSTOLEN)
	CASECHAREVENT("@ONPOISONED",EVENT_CHR_ONPOISONED)
	CASECHAREVENT("@ONREGIONCHANGE",EVENT_CHR_ONREGIONCHANGE)
	CASECHAREVENT("@ONCASTSPELL",EVENT_CHR_ONCASTSPELL)
	CASECHAREVENT("@ONREPUTATIONCHANGE",EVENT_CHR_ONREPUTATIONCHG)
	CASECHAREVENT("@ONBREAKMEDITATION",EVENT_CHR_ONBREAKMEDITATION)
	CASECHAREVENT("@ONCLICK",EVENT_CHR_ONCLICK)
	CASECHAREVENT("@ONMOUNT",EVENT_CHR_ONMOUNT)
	CASECHAREVENT("@ONDISMOUNT",EVENT_CHR_ONDISMOUNT)
	CASECHAREVENT("@ONHEARPLAYER",EVENT_CHR_ONHEARPLAYER)
	CASECHAREVENT("@ONDOCOMBAT",EVENT_CHR_ONDOCOMBAT)
	CASECHAREVENT("@ONCOMBATHIT",EVENT_CHR_ONCOMBATHIT)
	CASECHAREVENT("@ONSPEECH",EVENT_CHR_ONSPEECH)
	CASECHAREVENT("@ONCHECKNPCAI",EVENT_CHR_ONCHECKNPCAI)
	else if (!strcmp("@ONCREATION",script1)) 	newAmxEvent(script2)->Call(getSerial32(), INVALID);
	/*
	if	(!strcmp("@ONDEATH",script1)) 		setAmxEvent( EVENT_CHR_ONDEATH, script2 );
	else if (!strcmp("@ONKILL",script1))		setAmxEvent( EVENT_CHR_ONKILL, script2 );
	else if (!strcmp("@ONWOUNDED",script1)) 	setAmxEvent( EVENT_CHR_ONWOUNDED, script2 );
	else if (!strcmp("@ONHIT",script1))		setAmxEvent( EVENT_CHR_ONHIT, script2 );
	else if (!strcmp("@ONHITMISS",script1)) 	setAmxEvent( EVENT_CHR_ONHITMISS, script2 );
	else if (!strcmp("@ONGETHIT",script1))		setAmxEvent( EVENT_CHR_ONGETHIT, script2 );
	else if (!strcmp("@ONWALK",script1))		setAmxEvent( EVENT_CHR_ONWALK, script2 );
	else if (!strcmp("@ONBLOCK",script1))		setAmxEvent( EVENT_CHR_ONBLOCK, script2 );
	else if (!strcmp("@ONSTART",script1))
	{
							AmxEvent *event = setAmxEvent( EVENT_CHR_ONSTART, script2 );
							if( event )
								event->Call(getSerial32(), INVALID);
	}
	else if (!strcmp("@ONHEARTBEAT",script1)) 	setAmxEvent( EVENT_CHR_ONHEARTBEAT, script2 );
	else if (!strcmp("@ONDISPEL",script1)) 		setAmxEvent( EVENT_CHR_ONDISPEL, script2 );
	else if (!strcmp("@ONRESURRECT",script1)) 	setAmxEvent( EVENT_CHR_ONRESURRECT, script2 );
	else if (!strcmp("@ONFLAGCHANGE",script1)) 	setAmxEvent( EVENT_CHR_ONFLAGCHG, script2 );
	else if (!strcmp("@ONADVANCESKILL",script1)) 	setAmxEvent( EVENT_CHR_ONADVANCESKILL, script2 );
	else if (!strcmp("@ONGETSKILLCAP",script1)) 	setAmxEvent( EVENT_CHR_ONGETSKILLCAP, script2 );
	else if (!strcmp("@ONGETSTATCAP",script1)) 	setAmxEvent( EVENT_CHR_ONGETSTATCAP, script2 );
	else if (!strcmp("@ONADVANCESTAT",script1)) 	setAmxEvent( EVENT_CHR_ONADVANCESTAT, script2 );
	else if (!strcmp("@ONBEGINATTACK",script1)) 	setAmxEvent( EVENT_CHR_ONBEGINATTACK, script2 );
	else if (!strcmp("@ONBEGINDEFENSE",script1)) 	setAmxEvent( EVENT_CHR_ONBEGINDEFENSE, script2 );
	else if (!strcmp("@ONTRANSFER",script1)) 	setAmxEvent( EVENT_CHR_ONTRANSFER, script2 );
	else if (!strcmp("@ONMULTIENTER",script1)) 	setAmxEvent( EVENT_CHR_ONMULTIENTER, script2 );
	else if (!strcmp("@ONMULTILEAVE",script1)) 	setAmxEvent( EVENT_CHR_ONMULTILEAVE, script2 );
	else if (!strcmp("@ONSNOOPED",script1)) 	setAmxEvent( EVENT_CHR_ONSNOOPED, script2 );
	else if (!strcmp("@ONSTOLEN",script1)) 		setAmxEvent( EVENT_CHR_ONSTOLEN, script2 );
	else if (!strcmp("@ONPOISONED",script1)) 	setAmxEvent( EVENT_CHR_ONPOISONED, script2 );
	else if (!strcmp("@ONREGIONCHANGE",script1)) 	setAmxEvent( EVENT_CHR_ONREGIONCHANGE, script2 );
	else if (!strcmp("@ONCASTSPELL",script1)) 	setAmxEvent( EVENT_CHR_ONCASTSPELL, script2 );
	else if (!strcmp("@ONREPUTATIONCHANGE",script1))setAmxEvent( EVENT_CHR_ONREPUTATIONCHG, script2 );
	else if (!strcmp("@ONBREAKMEDITATION",script1)) setAmxEvent( EVENT_CHR_ONBREAKMEDITATION, script2 );
	else if (!strcmp("@ONCLICK",script1)) 		setAmxEvent( EVENT_CHR_ONCLICK, script2 );
	else if (!strcmp("@ONMOUNT",script1)) 		setAmxEvent( EVENT_CHR_ONMOUNT, script2 );
	else if (!strcmp("@ONDISMOUNT",script1)) 	setAmxEvent( EVENT_CHR_ONDISMOUNT, script2 );
	else if (!strcmp("@ONHEARPLAYER",script1)) 	setAmxEvent( EVENT_CHR_ONHEARPLAYER, script2 );
	else if (!strcmp("@ONDOCOMBAT",script1)) 	setAmxEvent( EVENT_CHR_ONDOCOMBAT, script2 );
	else if (!strcmp("@ONCOMBATHIT",script1)) 	setAmxEvent( EVENT_CHR_ONCOMBATHIT, script2 );
	else if (!strcmp("@ONSPEECH",script1))		setAmxEvent( EVENT_CHR_ONSPEECH, script2 );
	else if (!strcmp("@ONCHECKNPCAI",script1))	setAmxEvent( EVENT_CHR_ONCHECKNPCAI, script2 );

	else if (!strcmp("@ONCREATION",script1)) 	newAmxEvent(script2)->Call(getSerial32(), INVALID);
	*/
}

/*!
\author Luxor
\brief Checks if the stats are regular
*/
void cChar::checkSafeStats()
{
	SI32 nHP, nMN, nSTM;

	nHP = qmin( getStrength(), hp );
	nSTM = qmin( dx, stm );
	nMN = qmin( in, mn );

	//
	// Deactivate temp effects and items stats bonuses
	//
	tempfx::tempeffectsoff();
	NxwItemWrapper si;
	P_ITEM pi;
	si.fillItemWeared( this, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {
		pi = si.getItem();
		if ( !ISVALIDPI(pi) )
			continue;

		if ( pi->st2 != 0 )
			modifyStrength(-pi->st2);
		if ( pi->dx2 != 0 )
			dx -= pi->dx2;
		if ( pi->in2 != 0 )
			in -= pi->in2;
	}

	//
	// Check if stats are correct
	//
	if ( getStrength() != st3 )
		setStrength(st3);

	if ( dx != dx3 )
		dx = dx3;

	if ( in != in3 )
		in = in3;

	//
	// Reactivate temp effects and items stats bonuses
	//
	for( si.rewind(); !si.isEmpty(); si++ ) {
		pi = si.getItem();
		if ( !ISVALIDPI(pi) )
			continue;

		if ( pi->st2 != 0 )
			modifyStrength(pi->st2);
		if ( pi->dx2 != 0 )
			dx += pi->dx2;
		if ( pi->in2 != 0 )
			in += pi->in2;
	}

	tempfx::tempeffectson();

	hp = qmin( getStrength(), nHP );
	stm = qmin( dx, nSTM );
	mn = qmin( in, nMN );
}

/*!
\brief return the guild type
\author Sparhawk
\date 31/08/2002
\since 0.82
\return short type of guild. INVALID = no guild 0 = standard guild 1 = order guild 2 = chaos guild
*/
short cChar::GetGuildType()
{
	return guildType;
}

/*!
\brief set the guild type
\author Sparhawk
\date 31/08/2002
\since 0.82
\param newGuildType value must be between INVALID and MAX_GUILDTYPE
*/
void cChar::SetGuildType(short newGuildType)
{
	if ( newGuildType >= INVALID && newGuildType <= MAX_GUILDTYPE )
		guildType = newGuildType;
}

/*!
\brief return guild traitor status
\author Sparhawk
\date 31/08/2002
\since 0.82
\return bool: true is traitor
*/
LOGICAL cChar::IsGuildTraitor()
{
	return guildTraitor;
}

/*!
\brief set guild traitor status
\author Sparhawk
\date 31/08/2002
\since 0.82
*/
void cChar::SetGuildTraitor()
{
	guildTraitor = true;
}

/*!
\brief reset guild traitor status
\author Sparhawk
\date 31/08/2002
\since 0.82
*/
void cChar::ResetGuildTraitor()
{
	guildTraitor = false;
}

/*!
\brief return guild title toggle status
\author Sparhawk
\date 31/08/2002
\since 0.82
\return true is show title
*/
LOGICAL	cChar::HasGuildTitleToggle()
{
	return guildToggle;
}

/*!
\brief set guild title toggle
\author Sparhawk
\date 31/08/2002
\since 0.82
*/
void cChar::SetGuildTitleToggle()
{
	guildToggle = true;
}

/*!
\brief reset guild title toggle
\author Sparhawk
\date 31/08/2002
\since 0.82
*/
void cChar::ResetGuildTitleToggle()
{
	guildToggle = false;
}

/*!
\brief return the guild fealty
\author Sparhawk
\date 31/08/2002
\since 0.82
\return serial of guild fealty
*/
SERIAL cChar::GetGuildFealty()
{
	return guildFealty;
}

/*!
\brief set the guild fealty
\author Sparhawk
\date 31/08/2002
\since 0.82
\param newGuildFealty serial of char to set the guild fealty
*/
void cChar::SetGuildFealty(SERIAL newGuildFealty)
{
	guildFealty = newGuildFealty;
}

/*!
\brief return the guild number
\author Sparhawk
\date 31/08/2002
\since 0.82
\return guild number
*/
SI32 cChar::GetGuildNumber()
{
	return guildNumber;
}

/*!
\brief set the guild number
\author Sparhawk
\date 31/08/2002
\since 0.82
\param newGuildNumber guild number to set to the char
*/
void cChar::SetGuildNumber(SI32 newGuildNumber)
{
	guildNumber = newGuildNumber;
}

/*!
\brief return the guild title
\author Sparhawk
\date 31/08/2002
\since 0.82
\return guild title
*/
TEXT* cChar::GetGuildTitle()
{
	return guildTitle;
}

/*!
\brief set the guild title
\author Sparhawk
\date 31/08/2002
\since 0.82
\param newGuildTitle guild title
*/
void cChar::SetGuildTitle(TEXT* newGuildTitle)
{
	if ( strlen( newGuildTitle ) < sizeof( guildTitle ) )
		strcpy( guildTitle, newGuildTitle );
	else
	{
		strncpy( guildTitle, newGuildTitle, ( sizeof( guildTitle ) - 1 ) );
		guildTitle[ sizeof( guildTitle ) - 1 ] = '\0';
	}
}

void cChar::setStrength(UI32 val, bool check/*= true*/)
{
	str.value= val;
	if( check )
		checkEquipement();
}

/*!
\author Duke
\date 26/03/2001
\brief Returns the item on the given layer, if any
*/
P_ITEM cChar::GetItemOnLayer(UI08 layer)
{

	NxwItemWrapper si;
	si.fillItemWeared( this, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pi=si.getItem();
		if( ISVALIDPI(pi) && pi->layer==layer)
			return pi;
	}

	return NULL;
}

/*!
\brief get the bank box
\author Endymion
\param banktype BANK_GOLD or BANK_ITEM .. see note
\note BANK_GOLD are gold&items ( if specialbank onlygold not for only this region )
	  BANK_ITEM (only if specialbank) are the bank region of player
*/
P_ITEM cChar::GetBankBox( short banktype )
{

	NxwItemWrapper si;
	si.fillItemWeared( this, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if( ISVALIDPI(pi) && pi->type == ITYPE_CONTAINER && pi->morex == MOREX_BANK) {
			if(!SrvParms->usespecialbank) { // only a bank
				if((pi->morey != MOREY_GOLDONLYBANK) && (pi->morez == 0)) // All Region
					return pi;
			}
			else { //special bank are used ( so there are ware ( for region ) and bank )
				if( banktype == BANK_GOLD && pi->morey == MOREY_GOLDONLYBANK )
					return pi;
				else // ware ( for item )
					if ( banktype == BANK_ITEM && pi->morez == this->region )
						return pi; //correct region
			}
		}
	}
	// If we reach this point, bankbox wasn't found == wasn't created yet.


	char temp[TEMP_STR_SIZE];

	if( banktype == BANK_ITEM)
		sprintf(temp, TRANSLATE("%s's item bank box."), getCurrentNameC());
	else
		sprintf(temp, TRANSLATE("%s's bank box."), getCurrentNameC());

	P_ITEM pi = item::addByID(ITEMID_BANKBOX, 1, temp, 0, getPosition());

	pi->setContSerial(getSerial32());
	pi->setOwnerSerial32Only(getSerial32());

	pi->type = ITYPE_CONTAINER;
	pi->layer = LAYER_BANKBOX;
	pi->morex = MOREX_BANK;

	if(SrvParms->usespecialbank) {
		if(banktype == BANK_ITEM) {
			pi->morey = 0;
			pi->morez = this->region;
		} else {
			pi->morey = MOREY_GOLDONLYBANK;
			pi->morez = 0; // All Region
		}
	}

	pi->Refresh();

	return pi;
}


/*!
\author Duke
\date 17/03/2001
\brief stops meditation if necessary. Displays message if a socket is passed
*/
void cChar::disturbMed()
{
	if (!med) return; // no reason to stay here :]

	if (amxevents[EVENT_CHR_ONBREAKMEDITATION]) {
		g_bByPass = false;
		//<Luxor>
		NXWCLIENT cli = getClient();
		if (cli != NULL)
  			amxevents[EVENT_CHR_ONBREAKMEDITATION]->Call( getSerial32() );
		//</Luxor>
		if (g_bByPass==true) return;
  	}

	/*
	//<Luxor>
	NXWCLIENT cli = getClient();
	if (cli != NULL)
	{
		g_bByPass = false;
		runAmxEvent( EVENT_CHR_ONBREAKMEDITATION, getSerial32(), cli->toInt(), INVALID );
		if (g_bByPass==true)
			return;
	}
	//</Luxor>
	*/
   	med=0;
   	sysmsg(TRANSLATE("You break your concentration."));
}

/*!
\brief Reveals the char if he was hidden
\author Duke
\date 17/03/2001
\date 20/03/2003 ported unhidesendchar into the function code - Akron
*/
void cChar::unHide()
{
	//if hidden but not permanently or forced unhide requested
	if ( IsHiddenBySkill() && isPermaHidden() )
	{
		stealth=-1;
		hidden=UNHIDDEN;

		// unhidesendchar port by Akron
		setcharflag2(this);//AntiChrist - bugfix for highlight color not being updated

		SERIAL my_serial = getSerial32();
		Location my_pos = getPosition();

		NxwSocketWrapper sw;
		sw.fillOnline( this, false );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWSOCKET i = sw.getSocket();
			NXWCLIENT ps_i = sw.getClient();
			if( ps_i==NULL ) continue;

			P_CHAR pj=ps_i->currChar();
			if (ISVALIDPC(pj))
			{
				if (pj->getSerial32() != my_serial) { //to other players : recreate player object
					SendDeleteObjectPkt(i, my_serial);
					impowncreate(i, this, 0);
				} else {
					SendDrawGamePlayerPkt(i, my_serial, getId(), 0x00, getColor(), (poisoned ? 0x04 : 0x00), my_pos, 0x0000, dir|0x80);
				}
			}
		}

		if (IsGM())
			tempfx::add(this, this, tempfx::GM_UNHIDING, 3, 0, 0);
	}
}

/*!
\brief makes a character fight the other
\author Duke
\date 20/03/2001
*/
void cChar::fight(P_CHAR other)
{
	VALIDATEPC(other);
	//if (!war) toggleCombat(); //Luxor
	targserial=other->getSerial32();
	unHide();
	disturbMed(); // Meditation
	attackerserial=other->getSerial32();
	if (npc)
	{
		/*if (!war)
			toggleCombat();
		setNpcMoveTime();*/

		//Luxor: let's use npcattack
		npcattacktarget(this, other);
	}
}

/*!
\brief count items of given id and color
\author Duke
\date 26/03/2001
\param ID id of items to count
\param col color of the items to count

Searches the character recursively, counting the items of the given ID and (if given) color
*/
UI32 cChar::CountItems(short ID, short col)
{
	P_ITEM pi= getBackpack();
	return (ISVALIDPI(pi))? pi->CountItems(ID, col) : 0 ;
}

SI32 cChar::countBankGold()
{
	P_ITEM pi = GetBankBox(BANK_GOLD);
	return (ISVALIDPI(pi)) ? pi->CountItems(ITEMID_GOLD) : 0;
}

/*!
\brief finds the equipment weapon of a character
\return the pointer to the weapon
*/
P_ITEM cChar::getWeapon()
{

	NxwItemWrapper si;
	si.fillItemWeared( this, false, false, true );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if( ISVALIDPI(pi) )
			if (//(pi->lodamage != 0 || pi->hidamage != 0) && //If it's a weapon, it must have a lodamage and an hidamage (Luxor)
					(pi->layer==LAYER_1HANDWEAPON && pi->type!=ITYPE_SPELLBOOK) // not a spellbook (hozonko)
					|| (pi->layer==LAYER_2HANDWEAPON && !getShield()) ) //Morrolan don't check for shields
			{
				return pi;
			}
	}

	return NULL;
}

/*!
\author Duke
\date 20/05/2001
\brief finds the equipped shield of a character
\return the pointer to the shield
*/
P_ITEM cChar::getShield()
{
	P_ITEM pi=GetItemOnLayer(LAYER_2HANDWEAPON);
	if (pi && pi->IsShield() )
		return pi;
	else
		return NULL;
}

/*!
\brief Show a container to player
\author GHisha
\param pCont the container
*/
void cChar::showContainer(P_ITEM pCont)
{
	VALIDATEPI(pCont);
	NXWCLIENT ps=getClient();
	if(ps==NULL) return;
	NXWSOCKET s=ps->toInt();

	NxwItemWrapper si;
	si.fillItemsInContainer( pCont, false, false );
	SI32 count=si.size();

	UI08 bpopen[7]= { 0x24, 0x40, 0x0B, 0x00, 0x1A, 0x00, 0x3C };
	UI16 gump= pCont->getContGump();

	LongToCharPtr(pCont->getSerial32(), bpopen+1);
	ShortToCharPtr(gump, bpopen+5);

	Xsend(s, bpopen, 7);
//AoS/	Network->FlushBuffer(s);

	UI08 bpopen2[5]= { 0x3C, 0x00, 0x05, 0x00, 0x00 };

	ShortToCharPtr(count, bpopen2+3);
	count=(count*19)+5;
	ShortToCharPtr(count, bpopen2+1);
	Xsend(s, bpopen2, 5);

	UI08 bpitem[19]= { 0x40,0x0D,0x98,0xF7,0x0F,0x4F,0x00,0x00,0x09,0x00,0x30,0x00,0x52,0x40,0x0B,0x00,0x1A,0x00,0x00 };


	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if(!ISVALIDPI(pi))
			continue;

		//fix location of items if they mess up. (needs tweaked for container types)
		if (pi->getPosition("x") > 150) pi->setPosition("x", 150);
		if (pi->getPosition("y") > 140) pi->setPosition("y", 140);
		//end fix
		LongToCharPtr(pi->getSerial32(), bpitem);
		ShortToCharPtr(pi->animid(), bpitem +4);
		ShortToCharPtr(pi->amount, bpitem +7);
		ShortToCharPtr(pi->getPosition().x, bpitem +9);
		ShortToCharPtr(pi->getPosition().y, bpitem +11);
		LongToCharPtr(pCont->getSerial32(), bpitem +13);
		ShortToCharPtr(pi->getColor(), bpitem +17);
		//bpitem[19]=pi->decaytime=0;//HoneyJar // reseting the decaytimer in the backpack
		bpitem[19]= 0;
		Xsend(s, bpitem, 19);
	}
//AoS/ Network->FlushBuffer(s);
}

P_ITEM cChar::getBackpack()
{
	P_ITEM pi=pointers::findItemBySerial(packitemserial);
	if (ISVALIDPI(pi))
	{
		if ( pi->getContSerial() == getSerial32() && pi->layer == LAYER_BACKPACK )
			return pi;
	}


// - For some reason it's not defined, so go look for it and
// record it for next time


	pi=GetItemOnLayer(LAYER_BACKPACK);
	if( ISVALIDPI(pi) )
	{
		packitemserial= pi->getSerial32();
		return pi;
	}
	else
		return NULL;
}

LOGICAL cChar::isInBackpack( P_ITEM pi )
{
	VALIDATEPIR(pi, false);
	P_ITEM pack = getBackpack();
	VALIDATEPIR( pack, false);
	P_ITEM pCont;

	pCont = pi->getOutMostCont();
	VALIDATEPIR(pCont, false);

	return (pCont->getSerial32() == pack->getSerial32());

}

void cChar::setMultiSerial(long mulser)
{
	if (getMultiSerial32() != INVALID) // if it was set, remove the old one
		pointers::delFromMultiMap(this);

	setMultiSerial32Only(mulser);

	if (getMultiSerial32()!=INVALID) 	// if there is multi, add it
		pointers::addToMultiMap(this);
}

void cChar::MoveTo(Location newloc)
{
	// Avoid crash if go to 0,0
	if ((newloc.x < 1) || (newloc.y < 1))
		return;

	// <Luxor>
	if ( newloc != getPosition() && casting && !npc ) {
		sysmsg( "You stop casting the spell." );
		casting = 0;
		spell = magic::SPELL_INVALID;
		spelltime = 0;
	}
	// </Luxor>
#ifdef SPAR_C_LOCATION_MAP
	setPosition( newloc );
	pointers::updateLocationMap( this );
#else
	mapRegions->remove(this);
	setPosition( newloc );
	mapRegions->add( this );
#endif
}

UI32 cChar::getSkillSum()
{
	UI32 sum=0,a;
	for (a=0;a<ALLSKILLS;a++)
		sum+=baseskill[a];
	return sum; 	// this *includes* the decimal digit ie. xxx.y
}

/*!
\brief calculates how much the given player can learn from this teacher
\author Duke
\param pPlayer pointer to the player
\param skill index of skill to teach
\return how much the given player can learn
*/
SI32 cChar::getTeachingDelta(P_CHAR pPlayer, SI32 skill, SI32 sum)
{
	UI32 delta = qmin(250,baseskill[skill]/2); 	// half the trainers skill, but not more than 250
	delta -= pPlayer->baseskill[skill]; 				// calc difference
	if (delta <= 0)
		return 0;

	if (sum+delta >= SrvParms->skillcap * 10)			// would new skill value be above cap ?
		delta = (SrvParms->skillcap * 10) - sum;		// yes, so reduce it
	return delta;
}

/*!
\brief easy wrapper to sysmsg
\author Luxor
\param txt as default
\param ... as default
*/
void cChar::sysmsg(const TEXT *txt, ...)
{
	va_list argptr;
	char msg[512];
	va_start( argptr, txt );

	vsnprintf( msg, sizeof(msg)-1,(const char *)txt, argptr );
	va_end( argptr );
	if (getClient() != NULL)
		getClient()->sysmsg(msg);
}

/*!
\author Luxor
\brief gets the character current socket
\return the socket
*/
NXWSOCKET cChar::getSocket() const
{
        if ( npc )
		return INVALID;

	NXWCLIENT cli = getClient();
	if (cli != NULL) {
		if ( cli->toInt() >= 0 && cli->toInt() < now )
			return cli->toInt();
	}

	return INVALID;
}

/*!
\brief wrapper for AttackStuff()
\param pc as default
\author Xanathar
*/
void cChar::attackStuff(P_CHAR pc)
{
	VALIDATEPC(pc);
	NXWSOCKET s = getSocket();
	if (s==INVALID) return;
	AttackStuff (s, pc);
}

/*!
\author Xanathar
\param pc_i as default
*/
void cChar::helpStuff(P_CHAR pc_i)
{
	VALIDATEPC(pc_i);

	if (this==pc_i) return;
	if (pc_i->IsGrey()) setCrimGrey (this, ServerScp::g_nHelpingGreyWillCriminal);

	if (pc_i->IsInnocent())
	{
		if ((pc_i->GetKarma()>0)&&((pc_i->GetKarma()-GetKarma())>100)) {
			IncreaseKarma(+5);
			sysmsg(TRANSLATE("You've gained a little karma."));
		}
		return;
	}

	if (pc_i->IsCriminal()) setCrimGrey (this, ServerScp::g_nHelpingCriminalWillCriminal);

	if (pc_i->IsMurderer()) criminal(this);
}

/*!
\brief applies a poison to a char
\author Xanathar, modified by Endymion
\param poisontype the poison
\param secs the duration of poison ( if INVALID ( default ) default duration is used )
*/
void cChar::applyPoison(PoisonType poisontype, SI32 secs )
{
        unfreeze();
	if ( !IsInvul() && (::region[region].priv&0x40)) // LB magic-region change
	{
		if (poisontype>POISON_DEADLY) poisontype = POISON_DEADLY;
		else if (poisontype<POISON_WEAK) poisontype = POISON_WEAK;
		if ( poisontype>=poisoned ) {
			poisoned=poisontype;
			if( secs == INVALID )
				poisonwearofftime=uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParms->poisontimer); // lb
			else
				poisonwearofftime=uiCurrentTime+(MY_CLOCKS_PER_SEC*secs);

			NXWSOCKET s = getSocket();
			if (s != -1) impowncreate(s, this, 1); //Lb, sends the green bar !
			sysmsg( TRANSLATE("You have been poisoned!"));
			playSFX( 0x0246 ); //poison sound - SpaceDog
		}
	}
}

/*!
\author Xanathar
\brief unfreezes the char
*/
void cChar::unfreeze( LOGICAL calledByTempfx )
{
    if( !calledByTempfx )
		delTempfx( tempfx::SPELL_PARALYZE, false ); //Luxor

	if ( isFrozen() )
	{
		priv2 &= ~flagPriv2Frozen;
		if (!casting) //Luxor
			sysmsg(TRANSLATE("You are no longer frozen."));
	}
}

/*!
\brief damages a character
\author Xanathar & Luxor
\param amount how much damage
\param typeofdamage type of damage to use
\param stattobedamaged stat to be damaged
*/
void cChar::damage(SI32 amount, DamageType typeofdamage, StatType stattobedamaged)
{
	if (!npc && !IsOnline())
		return;
	P_CHAR myself=pointers::findCharBySerial(getSerial32());
	if ( ! ISVALIDPC(myself) )
		return;
	P_CHAR pc_att=pointers::findCharBySerial(attackerserial);
	SERIAL serial_att= ISVALIDPC(pc_att)? pc_att->getSerial32() : INVALID;

	if (amxevents[EVENT_CHR_ONWOUNDED]) {
		g_bByPass = false;
		amount = amxevents[EVENT_CHR_ONWOUNDED]->Call(getSerial32(), amount, serial_att);
		if (g_bByPass==true) return;
	}
	/*
	if ( getAmxEvent(EVENT_CHR_ONWOUNDED) != NULL ) {
		amount = runAmxEvent( EVENT_CHR_ONWOUNDED, getSerial32(), amount, serial_att );
		if (g_bByPass==true)
			return;
	}
	*/
	unfreeze();

	if (amount <= 0) return;
	// typeofdamage is ignored till now
    if (typeofdamage!=DAMAGE_PURE) {
    	amount -= SI32((amount/100.0)*float(calcResist(typeofdamage)));
    }
	if (amount <= 0) return;

	switch (stattobedamaged)
	{
		case STAT_MANA:
			mn = qmax(0, mn - amount);
			updateStats(STAT_MANA);
			break;
		case STAT_STAMINA:
			stm = qmax(0, stm - amount);
			updateStats(STAT_STAMINA);
			break;
		case STAT_HP:
		default:
			hp = qmax(0, hp - amount);
			updateStats(STAT_HP);
			if (hp<=0) Kill();
	}
}

/*!
\brief calculates resist power of a char
\author Luxor
\param typeofdamage type of damage to use
\return the resistance
*/
SI32 cChar::calcResist(DamageType typeofdamage)
{
	if (typeofdamage == DAMAGE_PURE || typeofdamage > MAX_RESISTANCE_INDEX)
		return 0;

	SI32 total = 0;
	total += resists[typeofdamage];

	NxwItemWrapper si;
	si.fillItemWeared( this, true, true, true );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if (ISVALIDPI(pi)) {
			total += pi->resists[typeofdamage];
		}
	}
	return total;
}

/*!
\brief Makes the char doing an action
\author Luxor
*/
void cChar::playAction(SI32 action)
{
	switch (action)
	{
		case 0x1A:// Mining-Gravedigging
		case 0x0B:
			if (onhorse)
				action = 0x1A;
			else
				action = 0x0b;
			break;
		case 0x1C:// LumberJacking-Bowcraft
		case 0x0D:
			if (onhorse)
				action = 0x1C;
			else
				action = 0x0D;
			break;
		case 0x1D:// Swordtarget
			// case 0x0D:
			if (onhorse)
				action = 0x1D;
			else
				action = 0x0D;
			break;
		case 0x0A:// Fist Fighting
			if (onhorse)
				action = 0x1A;
			else
				action = 0x0A;
			break;
		case 0x0E:// Smelting irons
			if (onhorse)
				action = 0x1C;
			else
				action = 0x0E;
			break;
		case 0x09:// Working ingots
			if (onhorse)
				action = 0x1A;
			else
				action = 0x09;
			break;
		case 0x14:// These can be done only if not onhorse
		case 0x22:
			if (onhorse)
				action = 0x00;
			break;
		default:
			break;
	}

	UI08 doact[14]={ 0x6E, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x00, 0x05, 0x00, 0x01, 0x0, 0x00, 0x01 };
	LongToCharPtr(getSerial32(), doact +1);
	ShortToCharPtr(action, doact +5);

	NxwSocketWrapper sw;
	sw.fillOnline( this, false );

	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if(ps!=NULL)
		{
			Xsend(ps->toInt(), doact, 14);
//AoS/			Network->FlushBuffer(ps->toInt());
		}
	}
}

/*!
\brief get online status
\return true if the player is owning the character online
\author Luxor
*/
LOGICAL const cChar::IsOnline() const
{
	if (getClient() != NULL) return true;

	if (npc) return false;

	if(Accounts->GetInWorld(account) == getSerial32())
		return true;

	return false;
}

/*!
\author Luxor
\brief shows speech text to all pcs near the char
\param txt the speech
\param antispam use or not antispam
*/
void cChar::talkAll(TEXT *txt, LOGICAL antispam)
{
	NxwSocketWrapper sw;
	sw.fillOnline( this, false );

	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if( ps!=NULL )
			talk(ps->toInt(), txt,antispam);
	}

}

/*!
\author Luxor
\brief Shows speech text of a char to the given char
\param s socket to send the speech to
\param txt the speech
\param antispam use or not antispam
*/
void cChar::talk(NXWSOCKET s, TEXT *txt, LOGICAL antispam)
{
	if( s < 0 || s >= now )
		return;

	LOGICAL machwas= true;

	if( antispam )
	{
		if( TIMEOUT(antispamtimer) )
			antispamtimer=uiCurrentTime+MY_CLOCKS_PER_SEC*10;
		else
			machwas = false;
	}

	if( machwas )
	{
		UI08 name[30]={ 0x00, };
		strcpy((char *)name, getCurrentNameC());

		saycolor=0x0481;

		if( npcaitype==NPCAI_EVIL )
		{
			saycolor=0x0026;
		}
		else if( npc && !tamed && !guarded && !war )
		{
			saycolor=0x005B;
		}

		SendSpeechMessagePkt(s, getSerial32(), getId(), 0, saycolor, fonttype, name, txt);
	}
}

/*!
\brief Shows emote text of a char
\author Luxor
\param socket socket to send the emote to
\param txt the emote
\param antispam use or not antispam
\todo document extra parameters
*/
void cChar::emote( NXWSOCKET socket, TEXT *txt, LOGICAL antispam, ... )
{
	LOGICAL sendEmote = true;
	emotecolor = 0x0026;

	if ( antispam )
	{
		if ( TIMEOUT( antispamtimer ) )
			antispamtimer=uiCurrentTime+MY_CLOCKS_PER_SEC*10;
		else
			sendEmote = false;
	}

	if ( sendEmote )
	{
		va_list argptr;
		char msg[512];
		va_start( argptr, antispam );
		vsnprintf( msg, sizeof( msg ) - 1, txt, argptr );
		va_end( argptr );

		UI08 name[30]={ 0x00, };
		strcpy((char *)name, getCurrentNameC());

		SendSpeechMessagePkt(socket, getSerial32(), getId(), 2, emotecolor, fonttype, name, msg);
	}
}

/*!
\brief Shows emote text of a char to all near pcs
\param txt the emote
\param antispam use or not antispam
\todo document extra parameters
*/
void cChar::emoteall( char *txt, LOGICAL antispam, ... )
{
	LOGICAL sendEmote = true;

	if ( antispam )
	{
		if ( TIMEOUT( antispamtimer ) )
			antispamtimer=uiCurrentTime+MY_CLOCKS_PER_SEC*10;
		else
			sendEmote = false;
	}

	if( sendEmote )
	{
		va_list argptr;
		char msg[512];
		va_start( argptr, antispam );
		vsnprintf( msg, sizeof( msg ) - 1, txt, argptr );
		va_end( argptr );

		NxwSocketWrapper sw;
		sw.fillOnline( this, false );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWCLIENT ps=sw.getClient();
			if( ps!=NULL )
				emote( ps->toInt(), msg, 0 );
		}
	}

}

/*!
\author Luxor
\brief Shows runic speech text of a char to the given socket
\param s socket to send the speech to
\param txt speech
\param antispam use or not antispam
*/
void cChar::talkRunic(NXWSOCKET s, TEXT *txt, LOGICAL antispam)
{
	LOGICAL machwas;

	if (s<0) return;

	if (antispam)
	{
		if (TIMEOUT(antispamtimer))
		{
			antispamtimer=uiCurrentTime+MY_CLOCKS_PER_SEC*10;
			machwas = true;
		}
		else
			machwas = false;
	}
	else
		machwas = true;

	if (machwas)
	{
		UI08 name[30]={ 0x00, };
		strcpy((char *)name, getCurrentNameC());

		SendSpeechMessagePkt(s, getSerial32(), getId(), 0, 0x0001, 0x0008, name, txt);
	}
}

/*!
\author Luxor
\brief Shows runic speech text to all pcs near the char
\param txt the speech
\param antispam use or not antispam
*/
void cChar::talkAllRunic(TEXT *txt, LOGICAL antispam)
{

	NxwSocketWrapper sw;
	sw.fillOnline( this, false );
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if( ps!=NULL )
			talkRunic(ps->toInt(), txt, antispam);
	}
}


/*!
\brief Get char's distance from the given character
\author Luxor
\return distance ( if invalid is returned VERY_VERY_FAR )
\param pc the char
*/
UI32 cChar::distFrom(P_CHAR pc)
{
	VALIDATEPCR(pc, VERY_VERY_FAR); //Endymion, fix: if not valid very far :P
	return (UI32)dist(getPosition(),pc->getPosition());
}

/*!
\brief Get char's distance from the given item
\author Endymion
\return distance ( if invalid is returned VERY_VERY_FAR )
\param pi the item
\note it check also if is subcontainer, or weared. so np call freely
*/
UI32 cChar::distFrom(P_ITEM pi)
{
	VALIDATEPIR(pi, VERY_VERY_FAR);
	P_ITEM cont=pi->getOutMostCont(); //return at least itself
	VALIDATEPIR(cont, VERY_VERY_FAR);

	if(cont->isInWorld())
	{
		return (UI32)dist(getPosition(),cont->getPosition());
	}
	else
		if(isCharSerial(cont->getContSerial())) //can be weared
			return distFrom( pointers::findCharBySerial(cont->getContSerial()) );
		else
			return VERY_VERY_FAR; //not world, not weared.. and another cont can't be

}

/*!
\author Luxor
\brief Tells if the char can see the given object
*/
LOGICAL cChar::canSee( cObject &obj )
{
	//
	// Check if the object is in visRange
	//
	R64 distance = dist( obj.getPosition(), getPosition(), false );
	if ( distance > VISRANGE ) // We cannot see it!
		return false;

	SERIAL ser = obj.getSerial32();
	if ( isCharSerial( ser ) ) {
		P_CHAR pc = P_CHAR( &obj );
		if ( !IsGM() ) { // Players only
			if ( pc->IsHidden() ) // Hidden chars cannot be seen by Players
				return false;
			if ( pc->dead && !pc->war && !dead ) // Non-persecuting ghosts can be seen only by other ghosts
				return false;
		}
		else
		{
			if ( !pc->IsOnline() && !pc->npc)
				return false;
		}
	}

	/*if ( isItemSerial( ser ) ) { //Future use
	}*/
	return true;
}


/*!
\author Luxor
\brief Teleports character to its current set coordinates.
*/
void cChar::teleport( UI08 flags, NXWCLIENT cli )
{


	P_ITEM p_boat = Boats->GetBoat(getPosition());
	if( ISVALIDPI(p_boat) ) {
		setMultiSerial(p_boat->getSerial32());
		Location boatpos = getPosition();
		boatpos.z = p_boat->getPosition().z +3;
		boatpos.dispz = p_boat->getPosition().dispz +3;
		setPosition( boatpos );
	} else
		setMultiSerial(INVALID);

	setcharflag2( this ); //AntiChrist - Update highlight color

	NXWSOCKET socket = getSocket();

    //
    // Send the draw player packet
    //
	if ( socket != INVALID ) {
		UI08 flag = 0x00;
		Location pos = getPosition();

		if( poisoned )
			flag |= 0x04;

		if ( IsHidden() )
			flag |= 0x80;

		SendDrawGamePlayerPkt(socket, getSerial32(), getId(), 0x00, getColor(), flag, pos, 0x0000, dir | 0x80, true);

		weights::NewCalc(this);
		statwindow( this, this );
		walksequence[socket] = INVALID;
	}


    //
    // Send the object remove packet
    //
	if ( cli == NULL ) { //ndEndy, this send also to current char?
		NxwSocketWrapper sw;
		sw.fillOnline( getOldPosition() );
		for ( sw.rewind(); !sw.isEmpty(); sw++ ) {
			NXWCLIENT ps_w = sw.getClient();
			if ( ps_w == NULL )
				continue;
			P_CHAR pc = ps_w->currChar();
            if ( ISVALIDPC( pc ) )
	            if ( distFrom( pc ) > VISRANGE || !canSee( *pc ) )
					ps_w->sendRemoveObject(static_cast<P_OBJECT>(this));
		}
	} else
		cli->sendRemoveObject(static_cast<P_OBJECT>(this));

        //
        // Send worn items and the char itself to the char (if online) and other players
        //
	if ( cli == NULL ) {
		NxwSocketWrapper sw;
		sw.fillOnline( this, false );
		for ( sw.rewind(); !sw.isEmpty(); sw++ ) {
			NXWCLIENT ps_i = sw.getClient();
			if( ps_i != NULL ) {
				impowncreate( ps_i->toInt(), this, 1 );
				//ndEndy not too sure of this
				if ( flags&TELEFLAG_SENDWORNITEMS )
					wornitems( ps_i->toInt(), this );
			}
		}
		sw.clear();
	} else {
		impowncreate( cli->toInt(), this, 1 );
		if ( flags&TELEFLAG_SENDWORNITEMS )
			wornitems( cli->toInt(), this );
	}


    //
    // Send other players and items to char (if online)
    //
    if ( cli == NULL || cli == getClient() )
		if ( socket != INVALID )
		{
			if ( flags&TELEFLAG_SENDNEARCHARS )
			{
				NxwCharWrapper sc;
				sc.fillCharsNearXYZ( getPosition(), VISRANGE, IsGM() ? false : true );
				for( sc.rewind(); !sc.isEmpty(); sc++ )
				{
					P_CHAR pc=sc.getChar();
					if( ISVALIDPC( pc ) )
					{
						if( getSerial32() != pc->getSerial32() )
						{
							if ( !pc->IsOnline() && !pc->npc )
							{
								if ( seeForLastTime( *pc ))
									getClient()->sendRemoveObject( P_OBJECT(pc) );
							}
							else
							{
								seeForLastTime( *pc );
								seeForFirstTime( *pc );
								impowncreate( socket, pc, 1 );
							}
						}
					}
				}

				if ( flags&TELEFLAG_SENDNEARITEMS )
				{
					NxwItemWrapper si;
					si.fillItemsNearXYZ( getPosition(), VISRANGE, false );
					for( si.rewind(); !si.isEmpty(); si++ )
					{
						P_ITEM pi = si.getItem();
						if( ISVALIDPI( pi ) )
							senditem( socket, pi );
					}
				}
			}
		}

	//
	// Send the light level
	//
	if ( socket != INVALID && (flags&TELEFLAG_SENDLIGHT) )
		dolight( socket, worldcurlevel );

    //
    // Check if the region changed
    //
	checkregion( this );

	//
	// Send the weather
	//
	if( socket != INVALID && (flags&TELEFLAG_SENDWEATHER) )
		pweather(socket);
}

/*!
\author Juliunus
\brief Changes the orientation of a player
\todo document parameters
*/
void cChar::facexy(SI32 facex, SI32 facey)
{
	SI32 x= getPosition().x;
	SI32 y= getPosition().y;

	if ((x == facex) && (y == facey))
		return;
	SI32 olddir = dir;
	if (x == facex)
		if (y > facey)
			dir = 0; // north
		else
			dir = 4; // south
	else
	{
		float coef =((float)facey - y) / ((float)facex - x);
		if (x < facex) // facing east
			if (coef > 2)
				dir = 4; // north
			else if (coef > 0.5)
				dir = 3; // north-east;
			else if (coef > -0.5)
				dir = 2; // east;
			else if (coef > -2)
				dir = 1; // south-east
			else dir = 0; // south
		else // facing west
			if (coef > 2)
				dir = 0; // south
			else if (coef > 0.5)
				dir = 7; // south-west
			else if (coef > -0.5)
				dir = 6; // west
			else if (coef > -2)
				dir = 5; // north-west
			else dir = 4; // north
	}
	if (dir != olddir)
		teleport( TELEFLAG_NONE );
}

/*!
\author Luxor
\brief Toggle war mode
*/
void cChar::toggleCombat()
{
	war=( !(war) );
	walking2( this );
}

/*!
\author Luxor
\brief returns char's combat skill
\return the index of the char's combat skill
*/
SI32 cChar::getCombatSkill()
{

	NxwItemWrapper si;
	si.fillItemWeared( this, false, false, true );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pi = si.getItem();
		if( ISVALIDPI( pi ) )
		{
			if( pi->layer == LAYER_1HANDWEAPON || pi->layer == LAYER_2HANDWEAPON )
			{
				if (pi->fightskill != 0)
				{
					return pi->fightskill;
				}
				else if (pi->IsSwordType() )
				{
					return SWORDSMANSHIP;
				}
				else if (pi->IsMaceType() )
				{
					return MACEFIGHTING;
				}
				else if (pi->IsFencingType() )
				{
					return FENCING;
				}
				else if (pi->IsBowType() )
				{
					return ARCHERY;
				}
			}
		}
	}

	return WRESTLING;

}

/*!
\author Luxor
\brief Returns line of sight from the char to the give char
\param pc pointer to the char to check line of sight from
\return true if is in line of sight
*/
LOGICAL cChar::losFrom(P_CHAR pc)
{
	VALIDATEPCR(pc, false );
	return lineOfSight( getPosition(), pc->getPosition() );
}

/*!
\author Xanathar
\brief plays a sound effect on a char
\param sound as default
\param onlyToMe Send sfx only to same char not to all near players (default: false)
*/
void cChar::playSFX(SI16 sound, LOGICAL onlyToMe)
{
	Location charpos = getPosition();

	charpos.z = 0;

	if(onlyToMe) {
		SendPlaySoundEffectPkt(getSocket(), 0x01, sound, 0x0000, charpos);
		return;
	}

	NxwSocketWrapper sw;
	sw.fillOnline( this, false );

	for( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps=sw.getClient();
		if(ps!=NULL)
		{
			SendPlaySoundEffectPkt(ps->toInt(), 0x01, sound, 0x0000, charpos);
		}
	}
}

/*!
\author Akron (port)
\brief Plays a monster sound effect
\param sfx sound effect
\note ported from sndpkg.cpp, i'm not the original author - Akron
*/
void cChar::playMonsterSound(MonsterSound sfx)
{

	P_CREATURE_INFO creature = creatures.getCreature( getId() );
	if( creature==NULL )
		return;

	SOUND s = creature->getSound( sfx );
	if( s!=INVALID )
		playSFX( s );

}


LOGICAL const cChar::CanDoGestures() const
{
	if (!IsGM())
	{
		if (hidden == HIDDEN_BYSPELL) return false;	//Luxor: cannot do magic gestures if under invisible spell

		NxwItemWrapper si;
		si.fillItemWeared( (P_CHAR)this, false, false, true );
		for( si.rewind(); !si.isEmpty(); si++ ) {

			P_ITEM pj=si.getItem();

			if( ISVALIDPI( pj ) )
			{
				if ( pj->layer == LAYER_2HANDWEAPON || ( pj->layer == LAYER_1HANDWEAPON && pj->type!=ITYPE_SPELLBOOK ) )
				{
					if (!(pj->getId()==0x13F9 || pj->getId()==0x0E8A || pj->getId()==0x0DF0 || pj->getId()==0x0DF2
						|| pj->IsChaosOrOrderShield() ))
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}

/*!
\author Xanathar
\brief Checks a skill for success
\param sk skill
\param low low bound
\param high high bound
\param bRaise should be raised?
*/
LOGICAL cChar::checkSkill(Skill sk, SI32 low, SI32 high, LOGICAL bRaise)
{
	NXWCLIENT ps = getClient();;
	NXWSOCKET s=INVALID;

	if ( sk < 0 || sk > TRUESKILLS ) //Luxor
		return false;

	if( ps != NULL )
		s=ps->toInt();

	if( dead )
		return false;

	if (IsGM())
		return true;

	bool skillused=false;


	if(high>1200)
		high=1200;

	SI32 charrange=skill[sk]-low;    // how far is the player's skill above the required minimum ?

	if(charrange<0)
		charrange=0;

	if (low == high)
	{
		LogCritical("minskill equals maxskill");
		return false;
	}

	float chance=static_cast<float>((charrange*890)/(high-low));

	if (sk!=FISHING)
		chance+=100;  // +100 means: *allways* a minimum of 10% for success
	else
		chance-=100;

	if (skill[sk]==1000)
		chance = 990; //gm always have 10% chance of fail

	if (chance>990)
		chance=990; // *allways* a 10% chance of failure

	if( chance >= rand()%1000 )
		skillused = true;
	else
		if (skillinfo[sk].unhide_onfail == 1)
			unHide();

	if (skillinfo[sk].unhide_onuse == 1)
		unHide();

	if(baseskill[sk]<high)
	{
		if (bRaise)
		{
			if(Skills::AdvanceSkill(DEREF_P_CHAR(this), sk, skillused))
			{
				Skills::updateSkillLevel(this, sk);
				if(!npc && IsOnline())
					updateskill(s, sk);
			}
		}
	}
	return skillused;
}

/*!
\author Xanathar
\brief Deletes items from backpack, by id
\param id id of the item to delete
\param amount amount of item to delete
\param color color of item to delete
\return number of items deleted
*/
SI32 cChar::delItems(short id, SI32 amount, short color)
{
	P_ITEM pi= getBackpack();
	if (pi==NULL) { return amount; }

	return pi->DeleteAmount(amount,id, color);
}

/*!
\brief Get the amount of the given id, color
\author Luxor, modified by Endymion for color and pack check
\return amount
\param id the id ( INVALID if no used )
\param col the color ( INVALID if no used )
\param onlyPrimaryBackpack false if search also in th subpack
\note changed to UI32 by Akron on 2003-03-18
*/
UI32 cChar::getAmount(short id, short col, bool onlyPrimaryBackpack)
{
	P_ITEM pi= getBackpack();
	VALIDATEPIR( pi, 0 );
	return pointers::containerCountItems(pi->getSerial32(), id, col, true, !onlyPrimaryBackpack);
}

/*!
\brief sends a remove packet to everyone nearby and deletes itself
\author Luxor
*/
void cChar::Delete()
{
        NxwSocketWrapper sc;
        sc.fillOnline( this );
	for ( sc.rewind(); !sc.isEmpty(); sc++ ) {
		NXWCLIENT ps = sc.getClient();
		if ( ps != NULL )
			ps->sendRemoveObject( static_cast<P_OBJECT>(this) );
	}
	archive::character::Delete( this );
}

/*!
\author Luxor
\brief Tells if a char sees an object for the first time
*/
LOGICAL cChar::seeForFirstTime( cObject &obj )
{
	SERIAL objser = obj.getSerial32();

	//
        // The char cannot see itself for the first time ;)
        //
	if ( objser == getSerial32() )
		return false;

	//
	// Check if we can see it
	//
	if ( !canSee( obj ) )
		return false;

	//
	// Check if the object was sent before
	//
	SERIAL_SLIST::iterator it( find( sentObjects.begin(), sentObjects.end(), objser ) );

	if ( it != sentObjects.end() ) // Already sent before
		return false;

	//
	// Seen for the first time, push it in the list and return true
	//
	sentObjects.push_front( objser );

	return true;
}


/*!
\author Luxor
\brief Tells if a char sees an object for the last time
*/
LOGICAL cChar::seeForLastTime( cObject &obj )
{
	SERIAL objser = obj.getSerial32();

        //
        // The char cannot see itself for the last time ;)
        //
	if ( objser == getSerial32() )
		return false;

	//
	// Check if we can see it
	//
	if ( canSee( obj ) )
		return false;

	//
	// Check if the object was sent before
	//
	SERIAL_SLIST::iterator it( find( sentObjects.begin(), sentObjects.end(), objser ) );

	if ( it == sentObjects.end() ) // Never sent before, so why remove it from the display?
		return false;

	//
	// Seen for the last time, remove it from the list and return true
	//
	sentObjects.erase( it );

	return true;
}


/*!
\author Xanathar
\brief Plays a moving effect from this to target char
\param destination the target char
\param id id of the effect
\param speed speed of the effect
\param loop loops
\param explode true if should do a final explosion
\param part particle effects structure
*/
void cChar::movingFX(P_CHAR destination, short id, SI32 speed, SI32 loop, LOGICAL explode, ParticleFx* part)
{
	movingeffect(DEREF_P_CHAR(this), DEREF_P_CHAR(destination), id >> 8, id & 0xFF,
		speed & 0xFF, loop & 0xFF, explode ? '\1' : '\0', part!=NULL, part);
}

/*!
\brief Plays a static effect on a char
\author Xanathar
\param id id of 2d effect; if -1, 2d effect is get from particles obj
\param speed speed of effect, -1 and it will be get from particles data
\param loop loop factor - -1 and it will be get from particles data
\param part optional particles data
\note if part == NULL then id, speed and loop MUST be >= 0
*/
void cChar::staticFX(short id, SI32 speed, SI32 loop, ParticleFx* part)
{
	if (part!=NULL) {
		if (id<=-1) id = (part->effect[0] << 8) + part->effect[1];
		if (speed<=-1) speed = part->effect[2];
		if (loop<=-1) loop = part->effect[3];
	}
	staticeffect(DEREF_P_CHAR(this), id >> 8, id & 0xFF, speed, loop, part!=NULL, part);
}

/*!
\brief Bolts a char
\author Xanathar
\param bNoParticles true if NOT to use particles
\todo backport
*/
void cChar::boltFX(LOGICAL bNoParticles)
{
	UI08 effect[28]={ 0x70, 0x00, };

 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	Location pos2;
	pos2.x = 0; pos2.y = 0; pos2.z = 0;
	MakeGraphicalEffectPkt_(effect, 0x01, getSerial32(), 0, 0, getPosition(), pos2, 0, 0, 1, 0);

	if (bNoParticles) // no UO3D effect ? lets send old effect to all clients
	{
		 NxwSocketWrapper sw;
		 sw.fillOnline( this, false );
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			 NXWSOCKET j=sw.getSocket();
			 if( j!=INVALID )
			 {
				Xsend(j, effect, 28);
//AoS/				Network->FlushBuffer(j);
			 }
		 }
	   return;
	}
	else
	{
		 NxwSocketWrapper sw;
		 sw.fillOnline( this, false );
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			 NXWSOCKET j=sw.getSocket();
			 if( j!=INVALID )
			 {
			 if (clientDimension[j]==2) // 2D client, send old style'd
			 {
				 Xsend(j, effect, 28);
//AoS/				Network->FlushBuffer(j);
			 } else if (clientDimension[j]==3) // 3d client, send 3d-Particles
			 {
				//TODO!!!! fix it!
				bolteffectUO3D(DEREF_P_CHAR(this));
				unsigned char particleSystem[49];
				Xsend(j, particleSystem, 49);
//AoS/				Network->FlushBuffer(j);
			 }
			 else if (clientDimension[j] != 2 && clientDimension[j] !=3 ) { sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); }
		 }
	   }
	}
}

/*!
\brief Plays <i>circle of blood</i> or similar effect
\author Xanathar
\param id effect id
\todo backport
*/
void cChar::circleFX(short id)
{
	bolteffect2(DEREF_P_CHAR(this),id >> 8,id & 0xFF);
}

/*!
\author Xanathar
\brief Hides a player by skill
*/
void cChar::hideBySkill()
{
	P_CHAR pc_att=pointers::findCharBySerial(attackerserial);

	if ( ISVALIDPC(pc_att) && char_inVisRange( this, pc_att ) )
    	{
    		if ( !npc )
        		sysmsg( TRANSLATE("You cannot hide while fighting.") );
        	return;
    	}

    	if ( IsHidden() )
    	{
    		if ( !npc )
        		sysmsg( TRANSLATE("You are already hidden") );
        	return;
    	}

    	if ( !checkSkill( HIDING, 0, 1000) )
    	{
    		if ( !npc )
        		sysmsg( TRANSLATE("You are unable to hide here.") );
        	return;
    	}

    	if ( IsGM() )
    	{
        	staticeffect( DEREF_P_CHAR(this), 0x37, 0x09, 0x09, 0x19);
        	playSFX( 0x0208 );
        	tempfx::add(this, this, tempfx::GM_HIDING, 1, 0, 0);
        	// immediate hiding overwrites the effect.
        	// so lets hide after 4 secs.
        	// 1 sec works fine now so changed to this.
        	return;
	}

	if ( !npc )
		sysmsg( TRANSLATE("You have hidden yourself well.") );

	hidden = HIDDEN_BYSKILL;
	teleport( TELEFLAG_NONE );
}

/*!
\author Xanathar & Luxor
\brief Hides a player by spell
*/
void cChar::hideBySpell(SI32 timer)
{
	if (timer == INVALID) timer = SrvParms->invisibiliytimer;
	tempfx::add(this, this, tempfx::SPELL_INVISIBILITY, 0,0,0, timer);
}

/*!
\brief Cures poison
\author Luxor
*/
void cChar::curePoison()
{
	poisoned = POISON_NONE;
	poisonwearofftime = uiCurrentTime;
	if (getClient() != NULL) impowncreate(getClient()->toInt(), this, 1);
}

/*!
\author Xanather
\brief Resurrects a char
\param healer Player that resurrected the char
*/
void cChar::resurrect( NXWCLIENT healer )
{
	NXWCLIENT ps=getClient();
	if( ps==NULL )
		return;

	if (dead)
	{
		dead=false;
		if(!npc || morphed)
			morph();
		hp= getStrength();
		stm= dx;
		mn=in;

		if (amxevents[EVENT_CHR_ONRESURRECT]) {
			g_bByPass = false;
			amxevents[EVENT_CHR_ONRESURRECT]->Call(getSerial32(), (healer!=NULL)? healer->currCharIdx() : INVALID );
			if (g_bByPass==true) return;
		}
		/*
		g_bByPass = false;
		runAmxEvent( EVENT_CHR_ONRESURRECT, getSerial32(), (healer!=NULL)? healer->toInt() : INVALID );
		if (g_bByPass==true)
			return;
		*/
		modifyFame(0);
		playSFX( 0x0214);
		setId( getOldId() );
		setColor( getOldColor() );
		attackerserial=INVALID;
		ResetAttackFirst();
		war=0;
		hunger=6;

		NxwItemWrapper si;
		si.fillItemWeared( this, false, false, true );
		for( si.rewind(); !si.isEmpty(); si++ )
		{
			P_ITEM pj=si.getItem();
			if(!ISVALIDPI(pj))
				continue;
			if( pj->layer == LAYER_NECK )
			{
				pj->layer = LAYER_BACKPACK;
				packitemserial=pj->getSerial32(); //Tauriel packitem speedup
			}
		}

		P_ITEM pj= pointers::findItemBySerial(robe);
		if( ISVALIDPI(pj))
			pj->Delete();

		P_ITEM pi = item::CreateFromScript( "$item_robe_1", this );
		if(ISVALIDPI(pi)) {
			pi->setCurrentName( "a resurrect robe" );
			pi->layer = LAYER_OUTER_TORSO;
			pi->setContSerial(getSerial32());
			pi->dye=1;
		}
		teleport( TELEFLAG_SENDWORNITEMS | TELEFLAG_SENDLIGHT );
	}
		else
			if( healer!=NULL )
				healer->sysmsg( TRANSLATE("That person isn't dead") );
}

/*!
\author Xanathar
\brief Checks char weight
\note this function modify the class variable, very bad...
\return true if the char is over weight
*/
LOGICAL const cChar::IsOverWeight()
{
	if (IsGM()) return false;
	return weights::CheckWeight2(this)!=0;
}

/*!
\author Xanathar
\brief Sets owner fields
\param owner new owner
*/
void cChar::setOwner(P_CHAR owner)
{
	setOwnerSerial32(owner->getSerial32());
	npcWander=WANDER_NOMOVE;
	tamed = true;
	npcaitype=NPCAI_GOOD;
}

/*!
\author Xanathar
\brief Gets beard item
\return the beard
\note Based on Antichrist incognito code
*/
P_ITEM cChar::getBeardItem()
{


	NxwItemWrapper si;
	si.fillItemWeared( this, true, true, true );
	for( si.rewind(); !si.isEmpty(); si++ ) {
		P_ITEM pj=si.getItem();
		if (ISVALIDPI(pj) && pj->layer==LAYER_BEARD) {
			beardserial= pj->getSerial32();
			return pj;
		}
	}

	beardserial=INVALID;
	return NULL;

}

/*!
\author Xanathar
\note Based on Antichrist incognito code
\brief Gets hairs item
\return the hairs
*/
P_ITEM cChar::getHairItem()
{

	NxwItemWrapper si;
	si.fillItemWeared( this, true, true, true );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pj=si.getItem();
		if (ISVALIDPI(pj) && pj->layer==LAYER_HAIR)
		{
			hairserial= pj->getSerial32();
			return pj;
		}
	}

    hairserial = INVALID;
    return NULL;
}

/*!
\author Xanathar
\note based on Antichrist incognito code
\brief Characters morphing (incognito, polymorph, etc)
\note Calling it with no params will undo any morphing
\note Any morphing with backup on will undo all previous morphings and install itself
\note Can recurse :]
\todo document parameters
*/
void cChar::morph ( short bodyid, short skincolor, short hairstyle, short haircolor,
    short beardstyle, short beardcolor, const char* newname, LOGICAL bBackup)
{
	if ((bodyid==INVALID)&&(skincolor==INVALID)&&(hairstyle==INVALID)&&
	    (haircolor==INVALID)&&(beardstyle==INVALID)&&(beardcolor==INVALID)&& (newname==NULL))
	{ // if empty morph called, no matter which bBackup status
		if(!morphed)
		{
			WarnOut("cChar::morph(<void>) called on non-morphed char\n");
			return;
		}
		morphed = false; //otherwise it will inf-loop
		morph( getOldId(), getOldColor(), oldhairstyle, oldhaircolor, oldbeardstyle, oldbeardcolor,getRealNameC(), false);
		return;
	}

	// if already morphed and should backup, restore old backup first
	if ((morphed)&&(bBackup))
		morph();

	P_ITEM phair = getHairItem();
	P_ITEM pbeard = getBeardItem();

	if (bBackup)
	{
		setOldId( getId() );
		setOldColor( getColor() );

		setRealName( getCurrentNameC() );
		if(ISVALIDPI(pbeard))
		{
			oldbeardstyle = pbeard->getId();
			oldbeardcolor = pbeard->getColor();
		}
		if(ISVALIDPI(phair))
		{
			oldhairstyle = phair->getId();
			oldhaircolor = phair->getColor();
		}
	}

	if(bodyid!=INVALID)
		setId( bodyid );

	if(skincolor!=INVALID)
		setColor( skincolor );

	if (newname!=NULL)
		setCurrentName(newname);

	if(ISVALIDPI(pbeard))
	{
		if (beardstyle!=INVALID)
			pbeard->setId( beardstyle );
		if (beardcolor!=INVALID)
			pbeard->setColor( beardcolor );
	}

	if(ISVALIDPI(phair))
	{
		if (hairstyle!=INVALID)
			phair->setId( hairstyle );
		if (haircolor!=INVALID)
			phair->setColor( haircolor );
	}

	morphed = bBackup;

	teleport( TELEFLAG_SENDWORNITEMS );

}

/*!
\brief shifts a player in the body of a npc, usually used only by GMs
\author Luxor
\param pc the character to possess
*/
void cChar::possess(P_CHAR pc)
{
	if ( !IsOnline() )
		return;

	VALIDATEPC(pc);
	bool bSwitchBack = false;

	NXWSOCKET socket = getSocket();
	if ( socket == INVALID )
		return;

	if ( possessorSerial != INVALID ) { //We're in a possessed Char! Switch back to possessor
		P_CHAR pcPossessor = pointers::findCharBySerial( possessorSerial );
		if ( ISVALIDPC( pcPossessor ) ) {
			bSwitchBack = true;
			pc = pcPossessor;
			possessorSerial = INVALID;
		} else
			return;
	} else { //Normal checks to prevent possessing a not permitted char
		if ( pc->shopkeeper ) {
			sysmsg( "You cannot use shopkeepers." );
			return;
		}
		if ( !pc->npc ) {
			sysmsg( "You can only possess NPCs." );
			return;
		}
		if ( pc->possessorSerial != INVALID ) {
				sysmsg( "This NPC is already possessed by someone!" );
				return;
		}
	}

	UI08 usTemp;
	SI08 sTemp;

	//PRIV
	usTemp = GetPriv();
	SetPriv( pc->GetPriv() );
	pc->SetPriv(usTemp);

	//PRIV2
	sTemp = GetPriv2();
	SetPriv2( pc->GetPriv2() );
	pc->SetPriv2(sTemp);

	//commandLevel
	usTemp = commandLevel;
	commandLevel = pc->commandLevel;
	pc->commandLevel = usTemp;

	//Serials
	if ( bSwitchBack ) {
		possessorSerial = INVALID;
		pc->possessedSerial = INVALID;
	} else {
		pc->possessorSerial = getSerial32();
		possessedSerial = pc->getSerial32();
	}

	//Network related stuff
	( bSwitchBack ) ? npc = 1 : pc->npc = 0;
	currchar[ socket ] = pc->getSerial32();
	pc->setClient( new cNxwClientObj( socket ) );
	setClient( NULL );

	//Set offline the old body, and online the new one
	if ( bSwitchBack ) {
		Accounts->SetOffline( pc->account );
		Accounts->SetOnline( pc->account, pc );

	} else {
		Accounts->SetOffline( account );
		Accounts->SetOnline( account, pc );
	}

	//Let's go! :)
	Network->enterchar( socket );
}

/*!
\brief Jails a char
\author Xanathar
\param seconds second to jail the character for
\todo backport
*/
void cChar::jail (SI32 seconds)
{
	prison::jail( NULL, this, seconds );
}

/*!
\author Zippy, Xanathar
\brief Kicks a char
*/
void cChar::kick ()
{
    NXWSOCKET j = getSocket();
    if(j > INVALID) {
        sysmsg(TRANSLATE("You have been kicked!"));
        Network->Disconnect(j);
    }
}

/*!
\author Xanathar
\brief Teleports char to a goplace location
\param loc goplace location
*/
void cChar::goPlace(SI32 loc)
{
    SI32 xx,yy,zz;
    location2xyz(loc, xx,yy,zz);
    MoveTo( xx,yy,zz );
}

/*!
\author Xanathar
\brief Checks if a char knows a given spell
\param spellnumber spell identifier to check
\return true if the char know the spell
*/
LOGICAL cChar::knowsSpell(magic::SpellId spellnumber)
{

    NxwItemWrapper sw;
    sw.fillItemsInContainer( getBackpack(), false, false );
	sw.fillItemWeared( this );

    for( sw.rewind(); !sw.isEmpty(); sw++ ){
        P_ITEM pi = sw.getItem();
        if( ISVALIDPI(pi)) {
            if (pi->type==ITYPE_SPELLBOOK) {
                if (pi->containsSpell(spellnumber)) return true;
                //if it doesn't contain it, we loop again, since pc may have more
                //than one spellbook in the backpack :]
            }
        }
    }

    return false;
}

/*!
\brief Manages/causes character death
\author Xanathar
\note Original by LB, ANtichrist, Ripper, Tauriel, JustMicheal annd others
\note expanded call to old PlayDeathSound
*/
void cChar::Kill()
{
	if (dead || npcaitype == NPCAI_PLAYERVENDOR || IsInvul() )
		return;

	NXWCLIENT ps=getClient();

	char murderername[128];
	murderername[0] = '\0';

	if (amxevents[EVENT_CHR_ONBEFOREDEATH]) {
		g_bByPass = false;
		amxevents[EVENT_CHR_ONBEFOREDEATH]->Call(getSerial32(), INVALID);
		if (g_bByPass==true) return;
	}

	/*
	g_bByPass = false;
	runAmxEvent( EVENT_CHR_ONDEATH, getSerial32(), s );
	if (g_bByPass==true)
		return;
	*/
	if ( ps != NULL )
		unmountHorse();	//Luxor bug fix
	if (morphed)
		morph();

	dead = true;
	poisoned = POISON_NONE;
	poison = hp = 0;

	if( getOldId() == BODY_FEMALE)
	{
		switch(RandomNum(0, 3)) // AntiChrist - uses all the sound effects
		{
			case 0:	playSFX( 0x0150 ); break;// Female Death
			case 1:	playSFX( 0x0151 ); break;// Female Death
			case 2:	playSFX( 0x0152 ); break;// Female Death
			case 3:	playSFX( 0x0153 ); break;// Female Death
		}
	}
	else if ( getOldId()  == BODY_MALE)
	{
		switch( RandomNum(0, 3) ) // AntiChrist - uses all the sound effects
		{
			case 0:	playSFX( 0x015A ); break;// Male Death
			case 1:	playSFX( 0x015B ); break;// Male Death
			case 2:	playSFX( 0x015C ); break;// Male Death
			case 3:	playSFX( 0x015D ); break;// Male Death
		}
	}
	else
		playMonsterSound(SND_DIE);

	if( polymorph )
	{ // legacy code : should be cut when polymorph will be translated to morph
		setId( getOldId() );
		polymorph=false;
		teleport( TELEFLAG_SENDWORNITEMS );
	}

	murdererSer = INVALID;

	//--------------------- reputation stuff
#ifdef SPAR_C_LOCATION_MAP
	PCHAR_VECTOR *pCV = pointers::getNearbyChars( this, VISRANGE*2, pointers::COMBATTARGET );
	PCHAR_VECTOR it( pCV->begin() ), end( pCV->end() );
	P_CHAR pKiller = 0;
	while( it != end )
	{
		pKiller = (*it);
		if( pKiller->npcaitype==NPCAI_TELEPORTGUARD )
		{
			pKiller->summontimer=(uiCurrentTime+(MY_CLOCKS_PER_SEC*20));
			pKiller->npcWander=WANDER_FREELY_CIRCLE;
			pKiller->setNpcMoveTime();
			pKiller->talkAll(TRANSLATE("Thou have suffered thy punishment, scoundrel."),0);
		}

		pKiller->targserial=INVALID;
		pKiller->timeout=0;

		P_CHAR pk_att = pointers::findCharBySerial( pKiller->attackerserial );
		if ( pk_att )
		{
			pk_att->ResetAttackFirst();
			pk_att->attackerserial = INVALID;
		}

		pKiller->attackerserial = INVALID;
		pKiller->ResetAttackFirst();

		if( pKiller->attackerserial == getSerial32() )
		{
			pKiller->attackerserial = INVALID;
			pKiller->ResetAttackFirst();
		}

		if( !pKiller->npc )
		{
			strncpy(murderername, pKiller->getCurrentNameC(), 48);

			if( pKiller->party != INVALID )
			{
				PCHAR_VECTOR *pcvParty = pointers::getNearbyChars( pKiller, VISRANGE, pointers::PARTYMEMBER );
				PCHAR_VECTOR partyIt( pcvParty->begin() ), partyEnd( pcvParty->end() );
				P_CHAR pMember = 0;
				while( partyIt != partyEnd )
				{
					pMember = (*partyIt);
					pMember->IncreaseKarma( (0-(karma)), this  );
					pMember->modifyFame( fame );
					++partyIt;
				}
			}

			pKiller->IncreaseKarma( (0-(karma)), this  );
			pKiller->modifyFame( fame );

			//murder count \/
			if (!npc)
			{ // PvP
				if ( !IsGrey() && IsInnocent() && Guilds->Compare(pKiller,this) == 0 )
				{
					murdererSer = pKiller->getSerial32();
					++pKiller->kills;
					pKiller->sysmsg(TRANSLATE("You have killed %i innocent people."), pKiller->kills);

					if (pKiller->kills==(unsigned)repsys.maxkills)
						pKiller->sysmsg(TRANSLATE("You are now a murderer!"));
					setcharflag(pKiller);

				if (SrvParms->pvp_log)
				{
						LogFile pvplog("PvP.log");
						pvplog.Write("%s was killed by %s!\n",getCurrentNameC(), pKiller->getCurrentNameC());
				}
			}   // was innocent

			if (pKiller->amxevents[EVENT_CHR_ONKILL])
				pKiller->amxevents[EVENT_CHR_ONKILL]->Call( pKiller->getSerial32(), getSerial32() );

				//pk->runAmxEvent( EVENT_CHR_ONKILL, pk->getSerial32(), pk->getClient()->toInt(), getSerial32(), s);
			} //PvP
		}//if !npc
		else
		{
			if (pKiller->amxevents[EVENT_CHR_ONKILL])
				pKiller->amxevents[EVENT_CHR_ONKILL]->Call( pKiller->getSerial32(), getSerial32() );
			if (pKiller->war)
				pKiller->toggleCombat(); // ripper
		}
		++it;
	}
#else
	P_CHAR pk = MAKE_CHAR_REF(0);

	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( getPosition(), VISRANGE*2, false );
	for( sc.rewind(); !sc.isEmpty(); sc++ )
	{
		pk = sc.getChar();
		if(!ISVALIDPC(pk) || pk->targserial!=getSerial32() )
			continue;

		if (pk->npcaitype==NPCAI_TELEPORTGUARD)
		{
			pk->summontimer=(uiCurrentTime+(MY_CLOCKS_PER_SEC*20));
			pk->npcWander=WANDER_FREELY_CIRCLE;
			pk->setNpcMoveTime();
			pk->talkAll(TRANSLATE("Thou have suffered thy punishment, scoundrel."),0);
		}

		pk->targserial=INVALID;
		pk->timeout=0;

		P_CHAR pk_att=pointers::findCharBySerial(pk->attackerserial);
		if (ISVALIDPC(pk_att))
		{
			pk_att->ResetAttackFirst();
			pk_att->attackerserial=INVALID; // lb crashfix
		}

		pk->attackerserial=INVALID;
		pk->ResetAttackFirst();

		if (!pk->npc)
		{
			strncpy(murderername, pk->getCurrentNameC(), 48);

			NxwCharWrapper party;
			party.fillPartyFriend( pk, VISRANGE, true );
			for( party.rewind(); !party.isEmpty(); party++ )
			{
				P_CHAR fr=party.getChar();
				if( ISVALIDPC(fr) )
				{
					fr->IncreaseKarma( (0-(karma)), this  );
					fr->modifyFame( fame );
				}
			}

			pk->IncreaseKarma( (0-(karma)), this  );
			pk->modifyFame( fame );

			//murder count \/
			if (!npc)
			{ // PvP
				if ( (!IsGrey()) && IsInnocent() && Guilds->Compare(pk,this)==0)
				{
					murdererSer = pk->getSerial32();
					pk->kills++;
					pk->sysmsg(TRANSLATE("You have killed %i innocent people."), pk->kills);

					if (pk->kills==(unsigned)repsys.maxkills)
						pk->sysmsg(TRANSLATE("You are now a murderer!"));
					setcharflag(pk);

				if (SrvParms->pvp_log)
				{
						LogFile pvplog("PvP.log");
						pvplog.Write("%s was killed by %s!\n",getCurrentNameC(), pk->getCurrentNameC());
				}
			}   // was innocent

			if (pk->amxevents[EVENT_CHR_ONKILL])
				pk->amxevents[EVENT_CHR_ONKILL]->Call( pk->getSerial32(), getSerial32() );

				//pk->runAmxEvent( EVENT_CHR_ONKILL, pk->getSerial32(), pk->getClient()->toInt(), getSerial32(), s);
			} //PvP
		}//if !npc
		else
		{
			if (pk->amxevents[EVENT_CHR_ONKILL])
				pk->amxevents[EVENT_CHR_ONKILL]->Call( pk->getSerial32(), getSerial32() );
			if (pk->war)
				pk->toggleCombat(); // ripper

		}


	}
#endif

	// Remove summoned monsters

	//--------------------- trade stuff

	if ( summontimer > 0 )
	{
		Delete();
		return;
	}
	NxwItemWrapper weared;
	weared.fillItemWeared( this, true, true, false );
	for( weared.rewind(); !weared.isEmpty(); weared++ ) {

		P_ITEM pi_j=weared.getItem();

		if(!ISVALIDPI(pi_j))
			continue;

		if ((pi_j->type==ITYPE_CONTAINER) && (pi_j->getPosition().x==26) && (pi_j->getPosition().y==0) &&
			(pi_j->getPosition().z==0) && (pi_j->getId()==0x1E5E) )
		{
			endtrade(pi_j->getSerial32());
		}
	}

	//--------------------- corpse & ghost stuff

	bool hadHumanBody=HasHumanBody();
	SI32 corpseid = (getId() == BODY_FEMALE)? BODY_DEADFEMALE : BODY_DEADMALE;

	if( ps!=NULL )
		morph( corpseid, 0, 0, 0, 0, 0, NULL, true);

	if (!npc)
	{
		P_ITEM pDeathRobe = item::addByID( ITEMID_DEATHSHROUD, 1, "a death shroud", 0, getPosition());
        	if (ISVALIDPI(pDeathRobe))
		{
			robe = pDeathRobe->getSerial32();
			pDeathRobe->setContSerial(getSerial32());
			pDeathRobe->layer = LAYER_OUTER_TORSO;
			pDeathRobe->def = 1;
			pDeathRobe->Refresh();
		}
		else
		{
			ErrOut("cChar::Kill() : Failed to create death robe\n");
		}
	}

	char szCorpseName[128];
	sprintf(szCorpseName, "corpse of %s", getCurrentNameC());

	P_ITEM pCorpse = item::addByID( ITEMID_CORPSEBASE, 1, szCorpseName, getOldColor(), getPosition());
	if (!ISVALIDPI(pCorpse))
	{
	    // panic
	    PanicOut("cChar::Kill() failed to create corpse.\n");
	    return;
	}

	if (!npc)
	{
		// looting mode of corpse
		if     ( IsInnocent() )
			pCorpse->more2 = 1;
		else if(IsGrey())
			pCorpse->more2 = ServerScp::g_nGreyCanBeLooted+1;
		else if (IsCriminal())
			pCorpse->more2 = 2;
		else if (IsMurderer())
			pCorpse->more2 = 3;
		pCorpse->setOwnerSerial32(getSerial32());
		pCorpse->more4 = char( SrvParms->playercorpsedecaymultiplier&0xff ); // how many times longer for the player's corpse to decay
	}

	pCorpse->amount = getOldId(); // Amount == corpse type
	pCorpse->morey = hadHumanBody;

	pCorpse->carve=carve;               //store carve section - AntiChrist

	pCorpse->setSecondaryName(getCurrentNameC());

	pCorpse->type = ITYPE_CONTAINER;

	pCorpse->more1 = 0;
	pCorpse->dir = dir;
	pCorpse->corpse = 1;
	pCorpse->setDecayTime();

	pCorpse->murderer = string(murderername);
	pCorpse->murdertime = uiCurrentTime;
	pCorpse->Refresh();


	//--------------------- dropping items to corpse

	P_ITEM pBackPack = getBackpack();
	if (!ISVALIDPI(pBackPack))
		pBackPack = pCorpse;
	//
	//	Sparhawk:	new just in time loot handling
	//
	if( !lootVector.empty() )
	{
		std::vector< UI32 >::iterator it( lootVector.begin() ), end( lootVector.end() );
		while( it != end )
		{
			char lootItem[32];
			numtostr( *it, lootItem );
			npcs::AddRandomLoot( pBackPack, lootItem );
			++it;
		}
		lootVector.clear();
	}
	//
	//
	//


	NxwItemWrapper si;
	si.fillItemWeared( this, false, false, true ); //Endymion adding weared item
	si.fillItemsInContainer( pBackPack, false, false ); //Endymion adding backpack item

	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pi_j=si.getItem();
		if( !ISVALIDPI(pi_j) )
			continue;

		//the backpack
		if (pi_j->getSerial32() == pBackPack->getSerial32() ) continue;
		//not the death robe
		if (pi_j->getSerial32() == robe) continue;
		//weared so remove stat bonus ( MOVING IT TO CORPSE so not continue :] )
		if (pi_j->getContSerial()==getSerial32())
		{
			if(pi_j->st2) modifyStrength(-pi_j->st2);
			if(pi_j->dx2) dx-=pi_j->dx2;
			if(pi_j->in2) in-=pi_j->in2;
		}
		//spell book or newbie are moved to backpack
		if ( pi_j->type == ITYPE_SPELLBOOK || pi_j->isNewbie() ) {
			pi_j->setContSerial( pBackPack->getSerial32() );
			pi_j->Refresh();
			continue;
		}

		pi_j->setContSerial( pCorpse->getSerial32() );
		//General Lee
		Location lj = pi_j->getPosition();
		lj.y = RandomNum(85,160);
		lj.x = RandomNum(20,70);
		pi_j->setPosition( lj );
		pi_j->Refresh();
		//General Lee

	}

        if ( !npc )
		teleport( TELEFLAG_SENDWORNITEMS );

	//<Luxor>
	pCorpse->Refresh();
	if (getClient() != NULL) {
		deathmenu(getClient()->toInt());
	}
	//</Luxor>

	if (amxevents[EVENT_CHR_ONAFTERDEATH])
	{
		g_bByPass = false;
		amxevents[EVENT_CHR_ONAFTERDEATH]->Call(getSerial32(), pCorpse->getSerial32() );
	}

	if ( npc )
	{
		Delete();
	}
	else
		++deaths;
}

/*!
\brief Update npcmove timer
\author Sparhawk
*/
void cChar::setNpcMoveTime()
{
//	npcmovetime = uiCurrentTime;
	if ( npcWander == WANDER_FOLLOW )
		npcmovetime = UI32( uiCurrentTime + ( float( npcFollowSpeed * MY_CLOCKS_PER_SEC ) ) );
	else
		npcmovetime = UI32( uiCurrentTime + ( float( npcMoveSpeed * MY_CLOCKS_PER_SEC ) ) );
}

/*!
\brief unequip items that you can nop longer wear/use
\author LB
*/
void cChar::checkEquipement()
{
	char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	P_ITEM pi;

	if (npc) return;

	Location charpos = getPosition();

	NxwItemWrapper si;
	si.fillItemWeared( this, false, false, true );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		pi=si.getItem();
		if(!ISVALIDPI(pi))
			continue;
		if (((pi->st > getStrength()) || !checkItemUsability(this, pi, ITEM_USE_CHECKEQUIP)) && !pi->isNewbie())//if strength required > character's strength, and the item is not newbie
		{
			if( strncmp(pi->getCurrentNameC(), "#", 1) )
				pi->getName(temp2);
			else
				strcpy(temp2,pi->getCurrentNameC());

			if( pi->st > getStrength()) sysmsg(TRANSLATE("You are not strong enough to keep %s equipped!"), temp2);
			playSFX( itemsfx(pi->getId()) );

			//Subtract stats bonus and poison
			modifyStrength(-pi->st2,false);
			dx-=pi->dx2;
			in-=pi->in2;
			if(poison && pi->poisoned) poison-=pi->poisoned;
			if(poison<0) poison=0;

			pi->setContSerial(INVALID);
			pi->MoveTo(charpos.x, charpos.y, charpos.z);

			NxwSocketWrapper sw;
			sw.fillOnline( this, false );

			for( sw.rewind(); !sw.isEmpty(); sw++ ) {
				NXWCLIENT ps=sw.getClient();
				if(ps!=NULL ) {
					wornitems(ps->toInt(), this );
					senditem(ps->toInt(), pi);
				}
			}
		}
	}
}

/*!
\author ANthalir
\brief Equip an item
\return 0 if item equipped, 1 if not equipped (layer already used),
2 if small function cancelled the equip
\param pi item to equip
\param drag true if called in wear_item
*/
SI32 cChar::Equip(P_ITEM pi, LOGICAL drag)
{

	tile_st item;

	g_bByPass= false;

	// call the Small function
	// function(item, chr)

	if (pi->amxevents[EVENT_IONEQUIP] != NULL)
		pi->amxevents[EVENT_IONEQUIP]->Call(pi->getSerial32(), this->getSerial32() );
	//runAmxEvent( EVENT_IONEQUIP, pi->getSerial32(), s );

	// if bypass() function called return
	if (g_bByPass == true)
		return 2;

	// AntiChrist -- remove BONUS STATS given by equipped special items
	modifyStrength( pi->st2, false );
	dx += pi->dx2;
	in += pi->in2;

	// AntiChrist -- for poisoned items
	if(pi->poisoned)
	{
		if(poison < pi->poisoned)
			poison=pi->poisoned;
	}

	statwindow(this, this);

	if (drag)
		return 0;

	data::seekTile( pi->getId(), item );

	NxwItemWrapper si;
	si.fillItemWeared( this, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pj= si.getItem();
		if( !ISVALIDPI(pj))
			continue;

		if ((pj->layer == item.quality) && ((pj->itmhand != 3) && (pi->itmhand != 3)))	// layer already used
			return 1;																	// don't equip the item

		// for weapons
		if (      ((pj->layer == LAYER_1HANDWEAPON)||(pj->layer == LAYER_2HANDWEAPON))
			 &&  ((item.quality == LAYER_1HANDWEAPON)||(item.quality == LAYER_2HANDWEAPON))
			 && (pj->itmhand != 3) && (pi->itmhand != 3)
		   )
			return 1;
	}

	pi->layer= item.quality;
	pi->setContSerial(getSerial32());

	checkSafeStats();
	teleport( TELEFLAG_SENDWORNITEMS );

	return 0;
}

/*!
\author Anthalir
\brief Unequip an item
\return 0 if item unequipped, 1 if bypass called, item not unequipped
\param pi item to unequip
\param drag true when function called in get_item
*/
SI32 cChar::UnEquip(P_ITEM pi, LOGICAL drag)
{
	checkSafeStats();

	NXWSOCKET s = getSocket();
	P_ITEM pack = getBackpack();

	g_bByPass= false;

	if (pi->amxevents[EVENT_IONUNEQUIP] != NULL)
		pi->amxevents[EVENT_IONUNEQUIP]->Call(pi->getSerial32(), this->getSerial32());
	/*
	pi->runAmxEvent( EVENT_IONUNEQUIP, pi->getSerial32(), s );
	if (g_bByPass)
		return 1;
	*/

	// AntiChrist -- remove BONUS STATS given by equipped special items
	modifyStrength(-pi->st2, false);
	dx -= pi->dx2;
	in -= pi->in2;
	// AntiChrist -- for poisoned items
	if (pi->poisoned)
	{
		poison -= pi->poisoned;
		if (poison < 0)
			poison = 0;
	}
	statwindow(this, this);

    if (drag)
		return 0;


	pi->layer= 0;
	pi->setContSerial( pack->getSerial32() );
	sendbpitem(s, pi);

	return 0;
}

const LOGICAL cChar::IsGrey() const
{
	if ( npc || IsMurderer() || IsCriminal() )
		return false;
	else
		if ( (karma <= -10000) || (nxwflags[0] & flagPermaGrey) || (nxwflags[0] & flagGrey) )
			return true;
		else
			return false;
}

void cChar::SetMurderer()
{

	if (amxevents[EVENT_CHR_ONFLAGCHG])
		amxevents[EVENT_CHR_ONFLAGCHG]->Call(getSerial32() );
	//runAmxEvent( EVENT_CHR_ONFLAGCHG, getSerial32(), getSocket() );

	flag=flagKarmaMurderer;
}

void cChar::SetInnocent()
{

	if (amxevents[EVENT_CHR_ONFLAGCHG])
		amxevents[EVENT_CHR_ONFLAGCHG]->Call(getSerial32() );
	//runAmxEvent( EVENT_CHR_ONFLAGCHG, getSerial32(), getSocket() );
	flag=flagKarmaInnocent;
}

void cChar::SetCriminal()
{

	if (amxevents[EVENT_CHR_ONFLAGCHG])
		amxevents[EVENT_CHR_ONFLAGCHG]->Call(getSerial32() );
	//runAmxEvent( EVENT_CHR_ONFLAGCHG, getSerial32(), getSocket() );
	flag=flagKarmaCriminal;
}

void cChar::doSingleClickOnCharacter( SERIAL serial )
{
	P_CHAR pc = pointers::findCharBySerial(serial);

	if ( ISVALIDPC(pc) )
	{
		pc->onSingleClick( this );
	}
	else
	{
		LogMessage("<%d> cChar::doSingleClickOnCharacter couldn't find char serial: %d\n", __LINE__, serial);
	}
}

void cChar::doSingleClickOnItem( SERIAL serial )
{
	char temp[TEMP_STR_SIZE];
	SI32 amt = 0, wgt;
	char itemname[100];
	char temp2[100];
//	extern skill_st skill[SKILLS + 1]; // unused variable

	P_ITEM pi = pointers::findItemBySerial(serial);
	P_CHAR pj;

	if ( pi == NULL )
	{
		WarnOut("cChar::doSingleclick couldn't find item serial: %d\n", serial);
		return;
	}

	if (pi->amxevents[EVENT_IONCLICK]!=NULL)
	{
		g_bByPass = false;
		pi->amxevents[EVENT_IONCLICK]->Call(pi->getSerial32(), this->getSerial32() );
		if ( g_bByPass==true )
			return;
	}
	/*
	g_bByPass = false;
	pi->runAmxEvent( EVENT_IONCLICK, pi->getSerial32(), getSocket() );
	if ( g_bByPass==true )
		return;
	*/

	pi->getName( itemname );

	if ( pi->type == ITYPE_SPELLBOOK )
	{
		sprintf( temp, TRANSLATE("[%i spells]"), pi->countSpellsInSpellBook() );
		itemmessage(getSocket(), temp, serial, 0x0481);
	}

	if ( CanSeeSerials() )
	{
		if (pi->amount > 1)
			sprintf( temp, "%s [%x]: %i", itemname, pi->getSerial32(), pi->amount);
		else
			sprintf( temp, "%s [%x]", itemname, pi->getSerial32());
		itemmessage(getSocket(), temp, serial);
		return;
	}

	// Click in a Player Vendor item, show description, price and return
	if (!pi->isInWorld() && isItemSerial(pi->getContSerial()))
	{
		P_ITEM cont = (P_ITEM)pi->getContainer();
		if( ISVALIDPI(cont) ) {
			pj = cont->getPackOwner();
			if( ISVALIDPC(pj) )
			{
				if( pj->npcaitype==NPCAI_PLAYERVENDOR )
				{
					if ( !pi->creator.empty() && pi->madewith>0)
						sprintf( temp2, TRANSLATE("%s %s by %s"), pi->vendorDescription.c_str(), ::skillinfo[pi->madewith - 1].madeword, pi->creator.c_str());
					else
						strcpy( temp2, pi->vendorDescription.c_str() );

					sprintf( temp, TRANSLATE("%s at %igp"), temp2, pi->value );
					itemmessage(getSocket(), temp, serial);
					return;
				}
			}
		}
	}

	// From now on, we will build the message into temp, and let itemname with just the name info
	// Add amount info.
	if (!pi->pileable || pi->amount == 1)
		strncpy( temp, itemname, 100);
	else
		if (itemname[strlen(itemname) - 1] != 's') // avoid iron ingotss : x
			sprintf( temp, "%ss : %i", itemname, pi->amount);
		else
			sprintf( temp, "%s : %i", itemname, pi->amount);

	// Add creator's mark (if any)
	if ( !pi->creator.empty() && pi->madewith > 0)
		sprintf( temp, TRANSLATE("%s %s by %s"), temp, ::skillinfo[pi->madewith - 1].madeword, pi->creator.c_str());

	if (pi->type == ITYPE_WAND) // Fraz
	{
		if (!(strcmp(pi->getSecondaryNameC(), pi->getCurrentNameC())))
		{
			sprintf( temp, TRANSLATE("%s %i charge"), temp, pi->morez);
			if (pi->morez != 1)
				strcat(temp, "s");
		}
	}
	else if (pi->type == ITYPE_ITEMID_WAND || pi->type == ITYPE_FIREWORKS_WAND)
	{
			if (!(strcmp(pi->getSecondaryNameC(), pi->getCurrentNameC())))
			{
				sprintf( temp, TRANSLATE("%s %i charge"), temp, pi->morex);
				if (pi->morex != 1)
					strcat(temp, "s");
			}
	}
	// Corpse highlighting...Ripper
	if (pi->corpse==1)
	{
		if(pi->more2==1)
		    itemmessage( getSocket(),TRANSLATE("[Innocent]"),serial, 0x005A);
		else if(pi->more2==2)
			itemmessage( getSocket(),TRANSLATE("[Criminal]"),serial, 0x03B2);
		else if(pi->more2==3)
			itemmessage( getSocket(),TRANSLATE("[Murderer]"),serial, 0x0026);
	}  // end highlighting
	// Let's handle secure/locked down stuff.
	if (pi->magic == 4 && pi->type != ITYPE_DOOR && pi->type != ITYPE_GUMPMENU)
	{
		if (pi->secureIt !=1)
			itemmessage(getSocket(), TRANSLATE("[locked down]"), serial, 0x0481);
		if (pi->secureIt == 1 && pi->magic == 4)
			itemmessage(getSocket(), TRANSLATE("[locked down & secure]"), serial, 0x0481);
	}

	itemmessage(getSocket(), temp, serial);

	// Send the item/weight as the last line in case of containers
	if (pi->type == ITYPE_CONTAINER || pi->type == ITYPE_UNLOCKED_CONTAINER || pi->type == ITYPE_NODECAY_ITEM_SPAWNER || pi->type == ITYPE_TRASH)
	{
		wgt = (SI32) weights::LockeddownWeight(pi, &amt); // get stones and item #, LB
		if (amt>0)
		{
			sprintf( temp2, TRANSLATE("[%i items, %i stones]"), amt, wgt);
			itemmessage(getSocket(), temp2, serial);
		}
		else
			itemmessage(getSocket(), TRANSLATE("[0 items, 0 stones]"), serial);
	}

}

void cChar::doSingleClick( SERIAL serial )
{
	if ( isCharSerial( serial ) )
		doSingleClickOnCharacter( serial );
	else
		doSingleClickOnItem( serial );

}

void cChar::onSingleClick( P_CHAR clickedBy )
{

	if ( amxevents[EVENT_CHR_ONCLICK] != NULL )
	{
		g_bByPass = false;
		amxevents[EVENT_CHR_ONCLICK]->Call( getSerial32(), clickedBy->getSerial32() );
		if ( g_bByPass==true )
			return;
	}
	/*
	g_bByPass = false;
	runAmxEvent( EVENT_CHR_ONCLICK, getSerial32(), clickedBy->getSerial32() );
	if ( g_bByPass==true )
		return;
	*/
	//<Luxor>
	if (ServerScp::g_nShowPCNames || npc || getSerial32() == clickedBy->getSerial32()) {
		showLongName( clickedBy, false );
	}
	//</Luxor>
}

/*!
\brief Function for the different gm movement effects
\author Aldur
\remarks
	\remark if we can find new effects they can be added here and will be active
	for 'go 'goiter 'goplace 'whilst and 'tell for gm's and counselors
	\remark
		\li 0 = none
		\li 1 = flamestrike
		\li 2 - 6 = different sparkles
*/
void cChar::doGmEffect()
{
	if( !isPermaHidden() )
	{
		Location charpos= getPosition();

		switch( gmMoveEff )
		{
		case 1:	// flamestrike
			staticeffect3(charpos.x+1, charpos.y+1, charpos.z+10, 0x37, 0x09, 0x09, 0x19, 0);
			//soundeffect(s, 0x02, 0x08);
			playSFX( 0x0802);
			break;

		case 2: // sparklie (fireworks wand style)
			staticeffect3(charpos.x+1, charpos.y+1, charpos.z+10, 0x37, 0x3A, 0x09, 0x19, 0); break;

		case 3: // sparklie (fireworks wand style)
			staticeffect3(charpos.x+1, charpos.y+1, charpos.z+10, 0x37, 0x4A, 0x09, 0x19, 0); break;

		case 4: // sparklie (fireworks wand style)
			staticeffect3(charpos.x+1, charpos.y+1, charpos.z+10, 0x37, 0x5A, 0x09, 0x19, 0); break;

		case 5: // sparklie (fireworks wand style)
			staticeffect3(charpos.x+1, charpos.y+1, charpos.z+10, 0x37, 0x6A, 0x09, 0x19, 0); break;

		case 6: // sparklie (fireworks wand style)
			staticeffect3(charpos.x+1, charpos.y+1, charpos.z+10, 0x37, 0x7A, 0x09, 0x19, 0); break;
		}
	}
	return;
}

void cChar::showLongName( P_CHAR showToWho, LOGICAL showSerials )
{
	VALIDATEPC( showToWho );
	NXWSOCKET socket = showToWho->getSocket();
	if (socket < 0 || socket > now) return;

	char temp[TEMP_STR_SIZE];
 	char temp1[TEMP_STR_SIZE];
//        extern title_st title[ALLSKILLS +1]; // unused variable

	*(temp1)='\0';

	if( fame >= 10000 )
	{ // adding Lord/Lady to title overhead
		switch ( getId() )
		{
			case BODY_FEMALE :
				if ( strcmp(::title[9].other,"") )
				{
					sprintf(temp,"%s ",::title[9].other);
					strcat(temp1,temp);
				}
				break;
			case BODY_MALE	:
				if (strcmp(::title[10].other,""))
				{
					sprintf(temp,"%s ",::title[10].other);
					strcat(temp1,temp);
				}
				break;
			default	:
				break;
		}
	}

	if ( !npc && !IsOnline() && strcmp(::title[8].other,"") )
	{
		sprintf( temp, "%s (%s)",::title[8].other, getCurrentNameC());
		strcat( temp1, temp );
	}
	else
		strcat( temp1, getCurrentNameC() );

	if ( showToWho->CanSeeSerials() || showSerials )
	{
		sprintf( temp, " [%x]", getSerial32() );
		strcat( temp1, temp );
	}

	if ( squelched )
	{
		if ( !strcmp(::title[7].other,"") )
		{
			sprintf(temp," [%s]",::title[7].other);
			strcat(temp1,temp);
		}
	}

	if ( IsInvul() && account != 0 )
	{
		if (strcmp(::title[11].other,""))
		{
			sprintf(temp, " [%s]",::title[11].other);
			strcat(temp1,temp);
		}
	}
	if ( account==0 && IsGM() )
	{
		if (strcmp(::title[12].other,""))
		{
			sprintf(temp, " [%s]",::title[12].other);
			strcat(temp1,temp);
		}
	}
	if ( isFrozen() )
	{
		if (strcmp(::title[13].other,""))
		{
			sprintf(temp, " [%s]",::title[13].other);
			strcat(temp1,temp);
		}
	}
	if (guarded)
	{
		if (strcmp(::title[14].other,""))
		{
			sprintf(temp, " [%s]",::title[14].other);
			strcat(temp1,temp);
		}
	}
	if (tamed && npcaitype==NPCAI_PETGUARD && getOwnerSerial32()==showToWho->getSerial32() && showToWho->guarded)
	{
		if  (strcmp(::title[15].other,""))
		{
			sprintf(temp, " [%s]",::title[15].other);
			strcat(temp1,temp);
		}
	}
	if (tamed && npcaitype!=NPCAI_PLAYERVENDOR )
	{
		if  (strcmp(::title[16].other,""))
		{
			sprintf(temp, " [%s]",::title[16].other);
			strcat(temp1,temp);
		}
	}
	if (war)
	{
		if  (strcmp(::title[17].other,""))
		{
			sprintf(temp, " [%s]",::title[17].other);
			strcat(temp1,temp);
		}
	}
	if (!npc && IsCriminal() && kills < repsys.maxkills )
	{
		if  (strcmp(::title[18].other,""))
		{
			sprintf(temp, " [%s]",::title[18].other);
			strcat(temp1,temp);
		}
	}
	if (!npc && kills>=repsys.maxkills)
	{
		if  (strcmp(::title[19].other,""))
		{
			sprintf(temp, " [%s]",::title[19].other);
			strcat(temp1,temp);
		}
	}

	Guilds->Title( socket, DEREF_P_CHAR(this) );

	UI16 color;
	SI32 guild = Guilds->Compare(showToWho,this);

	UI08 sysname[30]={ 0x00, };
	strcpy((char *)sysname, "System");

	if (guild==1) //Same guild (Green)
	{
		color = 0x0043;
	}
	else if (guild==2) //enemy (Orange)
	{
		color = 0x0030;
	}
	else if( IsGM() && account==0 ) //Admin & GM get yellow names
	{
		color = 0x0481;
	}
	else if (IsGrey())
	{
		color = 0x03B2;
	}
	else
	{
		switch(flag)
		{
		case 0x01:	color = 0x0026; break;//red
		case 0x04:	color = 0x005A; break;//blue
		case 0x08:	color = 0x0049; break;//green
		case 0x10:	color = 0x0030; break;//orange
		default:	color = 0x03B2;	//grey
		}
	}

	SendSpeechMessagePkt(socket, getSerial32(), 0x0101, 6, color, 0x0003, sysname, temp1);
}

/*!
\brief makes the character drinking something
\author Luxor
\since 0.82a
\param pi the object to drink from
*/
void cChar::drink(P_ITEM pi)
{
        VALIDATEPI(pi);

        if (pi->type == ITYPE_POTION) {
                tempfx::add(this, pi, tempfx::DRINK_EMOTE, 0, 0, 0, 0);
                tempfx::add(this, pi, tempfx::DRINK_EMOTE, 0, 0, 0, 1);
                tempfx::add(this, pi, tempfx::DRINK_EMOTE, 0, 0, 0, 2);
                tempfx::add(this, pi, tempfx::DRINK_FINISHED, 0, 0, 0, 3);
        }
}

/*!
\brief check if guilded
\author Endymion
\return true if the char is guilded
*/
bool cChar::isGuilded()
{
	return guild!=NULL;
}

/*!
\brief set Guild
\author Endymion
\param guild the guild
*/
void cChar::setGuild( P_GUILD guild, P_GUILD_MEMBER member )
{
	this->guild = guild;
	this->member = member;
}

/*!
\brief get Guild
\author Endymion
\return the guild
*/
P_GUILD cChar::getGuild()
{
	return guild;
}

/*!
\brief get Guild member
\author Endymion
\return the guild member
*/
P_GUILD_MEMBER cChar::getGuildMember()
{
	return member;
}


/*!
\brief open a bankbox
\author Endymion
\param pc the character to open the bank of
\note Added to cChar by Akron (todo from endymion)
*/
void cChar::openBankBox(P_CHAR pc)
{
	VALIDATEPC(pc);
	if ((getSerial32() != pc->getSerial32()) && !IsGMorCounselor())
		return;

	P_ITEM bank = pc->GetBankBox(BANK_GOLD);
	wearIt(getSocket(), bank);
	showContainer(bank);
}

/*!
\brief region specific bankbox
\author Endymion
\param pc character owner of bank

If activated, you can only put golds into normal banks
and there are special banks (for now we still use normal bankers,
but u have to say the SPECIALBANKTRIGGER word to open it)
where u can put all the items: one notice: the special bank
is caracteristic of regions....so in Britain you don't find
the items you leaved in Minoc!
All this for increasing pk-work and commerce! :)
(and surely the Mercenary work, so now have to pay strong
warriors to escort u during your travels!)
*/
void cChar::openSpecialBank(P_CHAR pc)
{
	VALIDATEPC( pc );
	if((getSerial32()!=pc->getSerial32()) && !IsGMorCounselor())
		return;

	P_ITEM bank = pc->GetBankBox(BANK_ITEM);
	wearIt(getSocket(), bank);
	showContainer(bank);
}

void cChar::heartbeat()
{
	if ( dead )
		return;
	if ( pointers::findCharBySerial(getSerial32()) == NULL )
		return;
	if( npc )
		npc_heartbeat();
	else
		pc_heartbeat();

}

void cChar::generic_heartbeat()
{
	LOGICAL update[3] = { false, false, false };

	if (hp > getStrength()) {
		hp = getStrength();
		update[ 0 ] = true;
	}
	if (stm > dx) {
		stm = dx;
		update[ 2 ] = true;
	}
	if (mn > in) {
		mn = in;
		update[ 1 ] = true;
	}

	//HP REGEN
	if( this->regenTimerOk( STAT_HP ) ) {
		if (hp < getStrength() && (hunger > 3 || SrvParms->hungerrate == 0)) {
			hp++;
			update[ 0 ] = true;
		}

		this->updateRegenTimer( STAT_HP );
	}

	//STAMINA REGEN
	if( this->regenTimerOk( STAT_STAMINA )) {
		if (stm < dx) {
			stm++;
			update[ 2 ] = true;
		}

		this->updateRegenTimer( STAT_STAMINA );
	}

	//MANA REGEN
	if( this->regenTimerOk( STAT_MANA ) )
	{
		if (mn < in)
		{
			mn += 3;
			update[ 1 ] = true;
		}
		else if (med)
		{
			if( !npc )
				sysmsg(TRANSLATE("You are at peace."));
			med = 0;
		}

		UI32 manarate = this->getRegenRate( STAT_MANA, VAR_REAL );
		if(SrvParms->armoraffectmana)
		{
			if (med)
				manarate += UI32( calcDef(0) / 10.0 ) - UI32( skill[MEDITATION]/222.2 );
			else
				manarate += UI32( calcDef(0) / 5.0 );
		}
		else
		{
			if(med)
				manarate -= UI32( skill[MEDITATION]/222.2 );
		}
                manarate = qmax( 1, manarate );
		this->setRegenRate( STAT_MANA, manarate, VAR_EFF );
		this->updateRegenTimer( STAT_MANA );

	}
	if ( hp <= 0 )
		Kill();
	else
		for( UI32 i = 0; i < 3; i++ )
			if( update[ i ] )
				updateStats( i );
}

void checkFieldEffects(UI32 currenttime, P_CHAR pc, char timecheck );

void target_castSpell( NXWCLIENT ps, P_TARGET t )
{
	TargetLocation TL( t );
	magic::castSpell( static_cast<magic::SpellId>(t->buffer[0]), TL, ps->currChar() );
}


void cChar::pc_heartbeat()
{
	if ( Accounts->GetInWorld( account ) == getSerial32() && logout > 0 && ( logout <= (SI32)uiCurrentTime  ) )
	{
		Accounts->SetOffline( account);
		logout = INVALID;
		teleport( TELEFLAG_NONE );
		return;
	}
	if( !IsOnline() )
		return;

	if ( amxevents[EVENT_CHR_ONHEARTBEAT] )
	{
		g_bByPass = false;
		amxevents[EVENT_CHR_ONHEARTBEAT]->Call( getSerial32(), uiCurrentTime );
		if( g_bByPass == true ) return;
		if( dead )	// Killed as result of script action
			return;
	}
	/*
	g_bByPass = false;
	runAmxEvent( EVENT_CHR_ONHEARTBEAT, getSerial32(), uiCurrentTime );
	if( g_bByPass == true )
		return;
	*/
	if( dead )	// Killed as result of script action
		return;
	generic_heartbeat();
	if( dead )	// Killed as result of generic heartbeat action
		return;

	if ( SrvParms->hunger_system && TIMEOUT( hungertime ) && SrvParms->hungerrate > 1 )
	{
		if ( !IsGMorCounselor() && hunger )
		{
			--hunger;

			switch( hunger )
			{
				case 6:
				case 5: sysmsg( TRANSLATE("You are still stuffed from your last meal") );
					break;
				case 4: sysmsg( TRANSLATE("You are not very hungry but could eat more") );
					break;
				case 3: sysmsg( TRANSLATE("You are feeling fairly hungry") );
					break;
				case 2: sysmsg( TRANSLATE("You are extremely hungry") );
					break;
				case 1: sysmsg( TRANSLATE("You are very weak from starvation") );
					break;
				case 0:	sysmsg( TRANSLATE("You must eat very soon or you will die!") );
					break;
			}
			hungertime = uiCurrentTime+(SrvParms->hungerrate*MY_CLOCKS_PER_SEC); // Bookmark
		}
	}
	if ( SrvParms->hunger_system && TIMEOUT( hungerdamagetimer ) && SrvParms->hungerdamage > 0 ) // Damage them if they are very hungry
	{
		hungerdamagetimer=uiCurrentTime+(SrvParms->hungerdamagerate*MY_CLOCKS_PER_SEC); /** set new hungertime **/
		if (hp > 0 && hunger<2 && !IsCounselor() && !dead)
		{
			sysmsg( TRANSLATE("You are starving !") );
			hp -= SrvParms->hungerdamage;
			updateStats(0);
			if(hp<=0)
			{
				Kill();
				sysmsg(TRANSLATE("You have died of starvation"));
			}
		}
	}
	if( dead )	// Starved to death
		return;

	checkFieldEffects( uiCurrentTime, this, 1 );
	if( dead )
		return;

	checkPoisoning();
	if( dead )	// Poison took it's toll
		return;

	int timer;

	NXWCLIENT ps = getClient();
	NXWSOCKET socket = getSocket();

	if     ( swingtargserial == INVALID )
		doCombat();
	else //if( TIMEOUT( timeout ) )
		combatHit( pointers::findCharBySerial( swingtargserial ) );


	if ( !TIMEOUT( smoketimer ) )
	{
		if ( TIMEOUT( smokedisplaytimer ) )
		{
			smokedisplaytimer = uiCurrentTime + 5 * MY_CLOCKS_PER_SEC;
			staticeffect( DEREF_P_CHAR( this ), 0x37, 0x35, 0, 30 );
			playSFX( 0x002B );
			switch( RandomNum( 0, 6 ) )
			{
			 case 0:	emote(socket,TRANSLATE("*Drags in deep*") ,1);		break;
			 case 1:	emote(socket,TRANSLATE("*Coughs*"),1);				break;
			 case 2:	emote(socket,TRANSLATE("*Retches*"),1);				break;
			 case 3:	emote(socket,TRANSLATE("*Hacking cough*"),1);		break;
			 case 4:	emote(socket,TRANSLATE("*Sighs in contentment*"),1 );break;
			 case 5:	emote(socket,TRANSLATE("*Puff puff*") ,1);			break;
			 case 6:	emote(socket,TRANSLATE("Wheeeee!!! Smoking!"),1);	break;
			 default:	break;
			}
		}
	}

	if ( clientInfo[socket]->lsd )
		do_lsd(); //LB's LSD potion-stuff

	if ( TIMEOUT( mutetime ) && squelched == 2 )
	{
		squelched = 0;
		mutetime  = 0;
		sysmsg( TRANSLATE("You are no longer squelched!") );
	}

/*	if ( IsCriminal() && ( crimflag <= uiCurrentTime  ) )
	{
		sysmsg( TRANSLATE("You are no longer a criminal.") );
		crimflag = 0;
		SetInnocent();
	}*/ //Luxor: now criminal flag is handled by CRIMINAL tempfx

	if ( TIMEOUT( murderrate ) )
	{
		if ( kills > 0)
			--kills;
		if ( kills == repsys.maxkills && repsys.maxkills > 0 )
		{
			sysmsg( TRANSLATE( "You are no longer a murderer." ) );
			SetInnocent();
		}
		murderrate = ( repsys.murderdecay * MY_CLOCKS_PER_SEC ) + uiCurrentTime;
	}

	setcharflag2( this );

	if ( casting )
	{
		if ( TIMEOUT( spelltime ) )//Spell is complete target it.
		{
			if ( magic::spellRequiresTarget( spell ) ) {
				P_TARGET targ = clientInfo[socket]->newTarget( new cTarget() );
				targ->code_callback = target_castSpell;
				targ->buffer[0]=spell;
				targ->send( ps );
				ps->sysmsg( TRANSLATE("Select your target") );
			}
			else
			{
		    		TargetLocation TL( this );
				magic::castSpell( spell, TL, this );
			}
			casting   = 0;
			spelltime = 0;
		}
		else if ( TIMEOUT( nextact ) ) //redo the spell action
		{ //<Luxor>
			nextact = uiCurrentTime + UI32(MY_CLOCKS_PER_SEC*1.5);
			if ( isMounting() )
				playAction( 0x1b );
			else
				playAction( spellaction );
		//</Luxor>
		}
	}

	if( SrvParms->bg_sounds >= 1 )
	{
		if( SrvParms->bg_sounds > 10 )
			SrvParms->bg_sounds = 10;
		timer = SrvParms->bg_sounds * 100;
		if ( !timer ) ++timer;
		if( rand() % timer == timer / 2 )
			bgsound( DEREF_P_CHAR(this) );
	}

	if( TIMEOUT( spiritspeaktimer ) && spiritspeaktimer > 0 )
		spiritspeaktimer = 0;


	if( onhorse )
	{
		P_ITEM pHorse = GetItemOnLayer(LAYER_MOUNT);
		if(!pHorse)
			onhorse = false;	// turn it off, we aren't on one because there's no item!
		else
			if( pHorse->canDecay() && pHorse->getDecayTime() != 0 && TIMEOUT( pHorse->getDecayTime() ) )
			{
				onhorse = false;
				pHorse->Delete();
			}
	}

	if( GetFame() < 0 )
		SetFame(0);
}

#define NPCMAGIC_FLAGS (SPELLFLAG_DONTCRIMINAL+SPELLFLAG_DONTREQREAGENTS+SPELLFLAG_DONTCHECKSPELLBOOK+SPELLFLAG_IGNORETOWNLIMITS+SPELLFLAG_DONTCHECKSKILL)
void cChar::npc_heartbeat()
{
	if( stablemaster_serial != INVALID )
		return;

	if( mounted )
		return;

	if ( amxevents[EVENT_CHR_ONHEARTBEAT] )
	{
		g_bByPass = false;
		amxevents[EVENT_CHR_ONHEARTBEAT]->Call( getSerial32(), uiCurrentTime );
		if ( g_bByPass == true )
			return;
		if( dead )	// Killed as result of action in script
			return;
	}
	/*
	g_bByPass = false;
	runAmxEvent( EVENT_CHR_ONHEARTBEAT, getSerial32(), uiCurrentTime );
	if ( g_bByPass == true )
		return;
	*/
	if( dead )	// Killed as result of action in script
		return;
	//
	//	Enable if possible
	//
	if ( TIMEOUT( disabled ) )
		disabled = 0;
	else
		return;
	//
	generic_heartbeat();
	if( dead )	// Killed as result of action in generic heartbeat
		return;
	//
	//	Handle poisoning
	//
	checkPoisoning();
	if( dead )
		return;
	checkFieldEffects( uiCurrentTime, this, 0 );
	if( dead )
		return;
	//
	//	Handle summoned npc's
	//
	if ( TIMEOUT( summontimer ) && summontimer > 0 )
	{

		if ( amxevents[EVENT_CHR_ONDISPEL] )
		{
			g_bByPass = false;
			amxevents[EVENT_CHR_ONDISPEL]->Call( getSerial32(), INVALID, DISPELTYPE_TIMEOUT );
			if ( g_bByPass == true ) return;
		}
		/*
		g_bByPass = false;
		runAmxEvent( EVENT_CHR_ONDISPEL, getSerial32(), INVALID, DISPELTYPE_TIMEOUT );
		if ( g_bByPass == true ) return;
		*/
		// Dupois - Added Dec 20, 1999
		// QUEST expire check - after an Escort quest is created a timer is set
		// so that the NPC will be deleted and removed from the game if it hangs around
		// too long without every having its quest accepted by a player so we have to remove
		// its posting from the message board before icing the NPC
		// Only need to remove the post if the NPC does not have a follow target set
		if ( questType == MsgBoards::ESCORTQUEST && ftargserial == INVALID )
		{
			MsgBoards::MsgBoardQuestEscortRemovePost( DEREF_P_CHAR(this) );
			MsgBoards::MsgBoardQuestEscortDelete( DEREF_P_CHAR(this) );
		}
		else
		{
			playSFX( 0x01FE );
			dead = true;
			Delete();
		}
		return;
	}
	//
	//	Handle hunger
	//
	if ( SrvParms->hungerrate > 1 && TIMEOUT( hungertime ) )
	{
		if ( hunger )
			--hunger;

		if( tamed && npcaitype != NPCAI_PLAYERVENDOR )
		{
			emotecolor = 0x0026;
			switch( hunger )
			{
				case 6: break;
				case 5: emoteall( TRANSLATE( "* %s looks a little hungry *" ), 1, getCurrentNameC() );
					break;
				case 4: emoteall( TRANSLATE( "* %s looks fairly hungry *" ), 1, getCurrentNameC() );
					break;
				case 3: emoteall( TRANSLATE( "* %s looks extremely hungry *" ), 1, getCurrentNameC() );
					break;
				case 2: emoteall( TRANSLATE( "* %s looks weak from starvation *"), 1, getCurrentNameC() );
					break;
				case 1: emoteall( TRANSLATE("* %s must eat very soon or he will die! *"), 1, getCurrentNameC() );
					break;
				case 0:	ftargserial = INVALID;
					npcWander = WANDER_FREELY_CIRCLE;
					setOwnerSerial32( INVALID );
					emoteall( TRANSLATE("* %s appears to have decided that it is better off without a master *"), 0, getCurrentNameC());
					playSFX( 0x01FE);
					if( SrvParms->tamed_disappear )
					{
						Delete();
						return;
					}
					break;
			}
		}
		hungertime = uiCurrentTime + ( SrvParms->hungerrate * MY_CLOCKS_PER_SEC );
	}

	if( npcWander!=WANDER_FLEE ) {
		if( hp < getStrength() * fleeat / 100 ) {
			flee( pointers::findCharBySerial( ftargserial ) );
			setNpcMoveTime();
		}
	}
	else
		if( ( ( fleeTimer==INVALID ) && ( hp > getStrength() * reattackat / 100 ) ) ||
			( ( fleeTimer!=INVALID ) && TIMEOUT( fleeTimer ) ) )
		{
			npcWander = oldnpcWander;
			setNpcMoveTime();
			oldnpcWander = WANDER_NOMOVE; // so it won't save this at the wsc file
			fleeTimer=INVALID;
		}

	//
	//	Handle ai
	//
	if ( TIMEOUT( nextAiCheck ) )
		npcs::checkAI( this );

	//
	//	Handle walking
	//
	if ( TIMEOUT( npcmovetime ) )
		walk();

        //
        //	Handle combat
        //
	if ( swingtargserial == INVALID )
		doCombat();
	else
		if( TIMEOUT( timeout )  )
			combatHit( pointers::findCharBySerial( swingtargserial ) );

	setcharflag2( this );


	//
	//	Handle spell casting (Luxor)
	//
	if ( casting ) {
		if ( TIMEOUT( spelltime ) ) {
			if ( spellTL != NULL ) {
	    			magic::castSpell( spell, *spellTL, this, NPCMAGIC_FLAGS );
				casting   = 0;
				spelltime = 0;
				safedelete( spellTL );
			}
		} else if ( TIMEOUT( nextact ) ) {
			nextact = uiCurrentTime + UI32(MY_CLOCKS_PER_SEC*1.5);
			if ( isMounting() )
				playAction( 0x1b );
			else
				playAction( spellaction );
		}
	}
}
#undef NPCMAGIC_FLAGS

/*!
\author Sparhawk
\remark Luxor: damage calculation adjusted.
*/
void cChar::checkPoisoning()
{
	if ( poisoned > 0 && !IsInvul() )
	{
		if ( TIMEOUT( poisontime ) )
		{
			if ( !TIMEOUT( poisonwearofftime ) )
			{
				switch ( poisoned )
				{
				case POISON_WEAK:
					poisontime= uiCurrentTime + ( 15 * MY_CLOCKS_PER_SEC );
					// between 0% and 5% of player's hp reduced by racial combat poison resistance
					hp -= SI32(
							qmax( ( ( hp ) * RandomNum( 0, 5 ) ) / 100, 3 ) *
							( (100 - Race::getPoisonResistance( race, POISON_WEAK ) ) / 100 )
						     );
					break;
				case POISON_NORMAL:
					poisontime = uiCurrentTime + ( 10 * MY_CLOCKS_PER_SEC );
					// between 5% and 10% of player's hp reduced by racial combat poison resistance
					hp -= SI32(
							qmax( ( ( hp ) * RandomNum( 5, 10 ) ) / 100, 5 ) *
							( (100 - Race::getPoisonResistance( race, POISON_NORMAL ) ) / 100 )
						      );
					break;
				case POISON_GREATER:
					poisontime = uiCurrentTime+( 10 * MY_CLOCKS_PER_SEC );
					// between 10% and 15% of player's hp reduced by racial combat poison resistance
					hp -= SI32(
							qmax( ( ( hp ) * RandomNum( 10,15 ) ) / 100, 7 ) *
							( (100 - Race::getPoisonResistance( race, POISON_GREATER ) ) / 100 )
						     );
					break;
				case POISON_DEADLY:
					poisontime = uiCurrentTime + ( 5 * MY_CLOCKS_PER_SEC );
					// between 15% and 20% of player's hp reduced by racial combat poison resistance
					if ( hp <= (getStrength()/4) ) {
						stm = qmax( stm - 6, 0 );
						updateStats( STAT_STAMINA );
					}
					hp -= SI32(
							qmax( ( ( hp ) * RandomNum( 15, 20 ) ) / 100, 6) *
							( (100 - Race::getPoisonResistance( race, POISON_DEADLY ) ) / 100 )
						     );
					break;
				default:
					ErrOut("checkPoisoning switch fallout for char with serial [%u]\n", getSerial32() );
					poisoned = POISON_NONE;
					return;
				}
				if ( hp < 1 )
				{
					Kill();
					if ( !npc )
						sysmsg( TRANSLATE("The poison has killed you.") );
				}
				else
				{
					updateStats( STAT_HP );
					if ( poisontxt <= uiCurrentTime  )
					{
						emotecolor = 0x0026;
						switch ( poisoned )
						{
						case POISON_WEAK:
							emoteall( TRANSLATE("* %s looks a bit nauseous *"), 1, getCurrentNameC() );
							break;
						case POISON_NORMAL:
							emoteall( TRANSLATE("* %s looks disoriented and nauseous! *"), 1, getCurrentNameC());
							break;
						case POISON_GREATER:
							emoteall( TRANSLATE("* %s is in severe pain! *"), 1, getCurrentNameC());
							break;
						case POISON_DEADLY:
							emoteall( TRANSLATE("* %s looks extremely weak and is wrecked in pain! *"), 1, getCurrentNameC());
							break;
						}
						poisontxt = uiCurrentTime + ( 10 * MY_CLOCKS_PER_SEC );
					}
				}
			}
			else
			{
				poisoned = POISON_NONE;
				impowncreate( getSocket(), this, 1 ); // updating to blue stats-bar ...
				if ( !npc )
					sysmsg( TRANSLATE( "The poison has worn off." ) );
			}
		}
	}
}

/*NOT USED NOW IS NOT GOOD FOR BIG SHARD
void cChar::restock()
{
	if ( npc && shopkeeper && TIMEOUT( shoprestocktime ) && SrvParms->shoprestock == 1 )
	{

		NxwItemWrapper si;
		si.fillItemWeared( this, false, false, false );
		for( si.rewind(); !si.isEmpty(); si++ ) {

			P_ITEM pici=si.getItem();
			if (ISVALIDPI(pici) && pici->layer==LAYER_TRADE_RESTOCK )
			{
				NxwItemWrapper sc;
				sc.fillItemsInContainer( pici, true, true );
				for( sc.rewind(); !sc.isEmpty(); sc++ ) {

					P_ITEM pic=sc.getItem();
					if (ISVALIDPI(pic))
					{
						if (pic->restock)
						{
							SI32 tmp=qmin(pic->restock, (pic->restock/2)+1);
							pic->amount += tmp;
							pic->restock -= tmp;
						}
						//
						//	All items in shopkeeper need a new randomvaluerate.
						//
						if (SrvParms->trade_system==1)
							StoreItemRandomValue(pic,calcRegionFromXY( getPosition() ));// Magius(CHE) (2)

					}
				}
			}
		}
	}
}
*/
void cChar::do_lsd()
{
	if (rand()%15==0)
	{
		NXWSOCKET socket = getSocket();

		int c1 = 0,c2 = 0,ctr = 0,xx,yy,icnt=0;
		SI08 zz;

		Location charpos = getPosition();

		NxwItemWrapper si;
		si.fillItemsNearXYZ( charpos, VISRANGE, false );
		for( si.rewind(); !si.isEmpty(); si++ ) {

			P_ITEM pi=si.getItem();
			if(!ISVALIDPI(pi))
				continue;

			UI16 color=pi->getColor(); // fetch item's color and covert to 16 bit
			if (rand()%44==0)
				color+= pi->getPosition().x  - pi->getPosition().y;
			else
				color+= charpos.x + charpos.y;
			color+= rand()%3; // add random "noise"
			ctr++;

			// lots of color consistancy checks
			color=color%0x03E9;
			WORD2DBYTE( color, c1, c2 );
			if (color<0x0002 || color>0x03E9 )
				color=0x03E9;

			if( ((color&0x4000)>>14)+((color&0x8000)>>15) )
				color =0x1000 +rand()%255;

			if (rand()%10==0)
				zz= pi->getPosition().z + rand()%33;
			else
				zz= pi->getPosition().z;
			if (rand()%10==0)
				xx= pi->getPosition().x + rand()%3;
			else
				xx= pi->getPosition().x;
			if (rand()%10==0)
				yy= pi->getPosition().y + rand()%3;
			else
				yy= pi->getPosition().y;
			WORD2DBYTE(color, c1, c2);
			if (distFrom(pi)<13) if (rand()%7==0)
			{
				icnt++;
				if (icnt%10==0 || icnt<10)
					senditem_lsd(socket, DEREF_P_ITEM(pi),c1,c2,xx,yy,zz); // attempt to cut packet-bombing by this thing
			}
		}// end of if item

		if (rand()%33==0)
		{
			if (rand()%10>3) soundeffect5(socket, 0x00F8); // lsd sound :)
			else
			{
				int snd=rand()%19;
				if (snd>9)
					soundeffect5(socket,(0x01<<8)|((snd-10)%256));
				else
					soundeffect5(socket,246+snd);
			}
		}
	}

}

/*
\brief Delete current speech
\author Endymion
\warning DELETE older speech string
*/
void cChar::deleteSpeechCurrent()
{
	if( speechCurrent!=NULL )
		safedelete(speechCurrent);
}

void cChar::setRegenRate( StatType stat, UI32 rate, VarType type )
{
	if( stat>=ALL_STATS ) return;
	switch( type ) {
		case VAR_EFF:
			regens[stat].rate_eff=rate; break;
		case VAR_REAL:
			regens[stat].rate_real=rate; break;
		default:
			break; //error here?
	}
}

UI32 cChar::getRegenRate( StatType stat, VarType type )
{
	if( stat>=ALL_STATS ) return 0;
	switch( type ) {
		case VAR_EFF:
			return regens[stat].rate_eff;
		case VAR_REAL:
			return regens[stat].rate_real;
		default:
			return 0; //error here?
	}
}

bool cChar::regenTimerOk( StatType stat )
{
	if( stat>=ALL_STATS ) return false;
	return TIMEOUT( regens[stat].timer );
}

void cChar::updateRegenTimer( StatType stat )
{
	if( stat>=ALL_STATS ) return;
	regens[stat].timer= uiCurrentTime+ regens[stat].rate_eff*MY_CLOCKS_PER_SEC;
}
/*
LOGICAL cChar::isValidAmxEvent( UI32 eventId )
{
	if( eventId < ALLCHAREVENTS )
		return true;
	else
		return false;
}
*/

/*
\brief Stable the character
\author Endymion
\param stablemaster the stablemaster
*/
void cChar::stable( P_CHAR stablemaster )
{
	VALIDATEPC(stablemaster);
	if( !this->npc ) return;
	this->stablemaster_serial=stablemaster->getSerial32();
	pointers::addToStableMap( this );
}

/*
\brief Unstable the character
\author Endymion
*/
void cChar::unStable()
{
	if( !isStabled() ) return;
	pointers::delFromStableMap( this );
	this->stablemaster_serial=INVALID;
}

/*!
\brief increase or decrease the karma of the char
\author Endymion
\since 0.82a
\param value positive or negative value to add to karma
\param killed ptr to killed char
\note every increase of karma has a related event
\note Sparhawk: karma increase now can also be applied to npc's
*/
void cChar::IncreaseKarma( SI32 value, P_CHAR pKilled )
{
	SI32 nCurKarma		= GetKarma();

	if( nCurKarma > 10000 )
		SetKarma( 10000 );
	else
		if( nCurKarma < -10000 )
			SetKarma( -10000 );

	if( value != 0 )
	{
		SI32 	nKarma			= value,
			nChange			= 0;

		LOGICAL	positiveKarmaEffect	= false;

		if	( nCurKarma < nKarma && nKarma > 0 )
		{
			nChange=((nKarma-nCurKarma)/75);
			SetKarma( GetKarma() + nChange );
			positiveKarmaEffect = true;
		}
		else if ( nCurKarma > nKarma )
		{
			if ( !ISVALIDPC( pKilled) )
			{
				nChange=((nCurKarma-nKarma)/50);
				SetKarma( GetKarma() + nChange );
			}
			else if( pKilled->GetKarma()>0 )
			{
				nChange=((nCurKarma-nKarma)/50);
				SetKarma( GetKarma() + nChange );
			}
		}

		if( nChange != 0 )
		{
			if ( amxevents[EVENT_CHR_ONREPUTATIONCHG] )
			{
				g_bByPass = false;
				SI32 n = nChange;
				if (!positiveKarmaEffect)
					n = -nChange;
				amxevents[EVENT_CHR_ONREPUTATIONCHG]->Call( getSerial32(), n, REPUTATION_KARMA );
			}
			/*
			pc->runAmxEvent( EVENT_CHR_ONREPUTATIONCHG, pc->getSerial32(), (!nEffect ? -nChange : nChange), REPUTATION_KARMA);
			if (g_bByPass==true)
				return;
			*/
			if( !g_bByPass && !npc )
			{
				if(nChange<=25)
				{
					if(positiveKarmaEffect)
						sysmsg( TRANSLATE("You have gained a little karma."));
					else
						sysmsg( TRANSLATE("You have lost a little karma."));
				}
				else if(nChange<=75)
				{
					if(positiveKarmaEffect)
						sysmsg( TRANSLATE("You have gained some karma."));
					else
						sysmsg( TRANSLATE("You have lost some karma."));
				}
				else if(nChange<=100)
				{
					if(positiveKarmaEffect)
						sysmsg( TRANSLATE("You have gained alot of karma."));
					else
						sysmsg( TRANSLATE("You have lost alot of karma."));
				}
				else if(nChange>100)
				{
					if(positiveKarmaEffect)
						sysmsg( TRANSLATE("You have gained a huge amount of karma."));
					else
						sysmsg( TRANSLATE("You have lost a huge amount of karma."));
				}
			}
		}
	}
}

/*!
\brief increase or decrease the fame of the char
\author Endymion
\since 0.82a
\param value positive or negative value to add to fame
\note every increase of karma have an event and stuff related
*/
void cChar::modifyFame( SI32 value )
{
	if( GetFame() > 10000 )
		SetFame( 10000 );

	if ( value != 0 )
	{
		SI32	nFame	= value;
		int	nChange	= 0,
			nEffect	= 0;
		int	nCurFame= fame;

		if( nCurFame > nFame ) // if player fame greater abort function
		{
			return;
		}

		if( nCurFame < nFame )
		{
			nChange=(nFame-nCurFame)/75;
			fame=(nCurFame+nChange);
			nEffect=1;
		}

		if( dead )
		{
			if(nCurFame<=0)
			{
				fame=0;
			}
			else
			{
				nChange=(nCurFame-0)/25;
				fame=(nCurFame-nChange);
			}
			nEffect=0;
		}
		if( nChange != 0 )
		{

			if (amxevents[EVENT_CHR_ONREPUTATIONCHG])
			{
				g_bByPass = false;
				int n = nChange;
				if (!nEffect) n = -nChange;
				amxevents[EVENT_CHR_ONREPUTATIONCHG]->Call(getSerial32(), n, REPUTATION_FAME);
			}
			/*
			pc->runAmxEvent( EVENT_CHR_ONREPUTATIONCHG, pc->getSerial32(), (!nEffect ? -nChange : nChange), REPUTATION_FAME);
			*/
			if ( !g_bByPass && !npc )
			{
				if(nChange<=25)
				{
					if(nEffect)
						sysmsg( TRANSLATE("You have gained a little fame."));
					else
						sysmsg( TRANSLATE("You have lost a little fame."));
				}
				else if(nChange<=75)
				{
					if(nEffect)
						sysmsg( TRANSLATE("You have gained some fame."));
					else
						sysmsg( TRANSLATE("You have lost some fame."));
				}
				else if(nChange<=100)
				{
					if(nEffect)
						sysmsg( TRANSLATE("You have gained alot of fame."));
					else
						sysmsg( TRANSLATE("You have lost alot of fame."));
				}
				else if(nChange>100)
				{
					if(nEffect)
						sysmsg( TRANSLATE("You have gained a huge amount of fame."));
					else
						sysmsg( TRANSLATE("You have lost a huge amount of fame."));
				}
			}
		}
	}
}

/*!
\author Luxor
\brief checks a skill for success (with sparring check)
\return true if success
\param sk skill
\param low low bound
\param high high bound
\todo document pcd parameter
\todo backport from Skills::
*/
const LOGICAL cChar::checkSkillSparrCheck(Skill sk, SI32 low, SI32 high, P_CHAR pcd)
{
	return Skills::CheckSkillSparrCheck(DEREF_P_CHAR(this),sk, low, high, pcd);
}
