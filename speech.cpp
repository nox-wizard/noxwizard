  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "sndpkg.h"
#include "debug.h"
#include "amx/amxcback.h"
#include "calendar.h"
#include "set.h"
#include "trigger.h"
#include "house.h"
#include "npcai.h"
#include "network.h"
#include "commands.h"
#include "packets.h"
#include "boats.h"
#include "scp_parser.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "classes.h"
#include "utils.h"
#include "nox-wizard.h"
#include "scripts.h"


#define MAXBUFFER_REAL MAXBUFFER

static NxwCharWrapper*	nearbyNpcs = NULL;
//static pCharVector	*nearbyPcs  = 0;

extern void checkAmxSpeech(int s, char *speech);

void wchar2char (const char* str);
void char2wchar (const char* str);

static SI32 findKeyword( const std::string &str, const std::string &keyword );
static std::string trimString( const std::string &str );

int response(NXWSOCKET  s)
{
	P_CHAR pc= MAKE_CHAR_REF( currchar[s] );
	VALIDATEPCR( pc, 0 );

	if( !pc->IsOnline() ) return 0;

	//Araknesh I morti non vengono cagati :)
	//SPARHAWK --- Hmmm but what about ghost speak????? MUST CHECK THIS OUT
	if ( pc->dead ) return 0;

	Location charpos= pc->getPosition();

	SI32 i, j, skill=-1;

	char search1[50];


	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	int x=-1;
	int y=0;

	char *comm;
	static UI08 nonuni[MAXBUFFER_REAL];
	static char buffer1[MAXBUFFER];
	if(pc->unicode)
	{
		j = (buffer[s][1]<<8)+buffer[s][2];
		for (i=13; i<j; i+=2 )
			nonuni[(i-13)/2]=buffer[s][i];
		for (i=0; i < MAXBUFFER_REAL-8; ++i)
			buffer1[i+8]=toupper(nonuni[i]); 	 // Sparhawk 	Let's forget about toupper and use strcasecmp for case insensitive comparisons
								 //		Needed for none english characters? Probably yes
	}
        else
        {
		ConOut("Non-Unicode speech\n");
		for (i=7; i < MAXBUFFER_REAL; i++)
			buffer1[i]=toupper(buffer[s][i]);	// Sparhawk 	Let's forget about toupper and use strcasecmp for case insensitive comparisons
								//		Needed for none english characters? Probably yes
	}
	comm=&buffer1[8];


	x= charpos.x;
	y= charpos.y;

	//
	// name
	//
	bool requestName = ( strstr( comm, "NAME") != NULL ) || ( strstr( comm, "WHO ARE YOU") != NULL );
	//
	// packup
	//	this should become <playervendorname> packup
	//	possibly add all packup
	//
	bool requestPackup = (strstr( comm, "PACKUP") != NULL);
	//
	// time
	//
	bool requestTime = (strstr( comm, "TIME") != NULL);
	//
	// location
	//
	bool requestLocation = (strstr( comm, "LOCATION") != NULL) || (strstr( comm, "WHERE AM I") != NULL);
	//
	// i will take thee
	//
	strcpy(search1,"I WILL TAKE THEE");
	bool requestAcceptEscort = ( strstr( comm, search1) != NULL );
	//
	// destination
	//
	strcpy(search1,"DESTINATION");
	bool requestDestination = ( strstr( comm, search1) != NULL );
	//
	// bank
	//
	bool requestBank = ( strstr( comm, "BANK") != NULL );
	//
	// balance
	//
    	bool requestBalance = ( strstr( comm, "BALANCE") != NULL );
    	//
    	// special bank
    	//
    	bool requestSpecialBank = false;
	if(SrvParms->usespecialbank)
		requestSpecialBank = (strstr( comm, SrvParms->specialbanktrigger) != NULL );

	//
	// train
	// train <skillname>
	//
	bool requestTrain = ( strstr( comm, "TRAIN") != NULL ) || ( strstr( comm, "TEACH") != NULL ) || ( strstr( comm, "LEARN") != NULL );
	//
	// <petname> follow
	//
	bool requestFollow = (strstr( comm, " FOLLOW") != NULL );
	//
	// <petname> follow me
	//
	bool requestFollowMe = (strstr( comm, " FOLLOW ME") != NULL );
	//
	// all attack
	//
	bool requestAllAttack = ( strstr( comm, "ALL ATTACK") != NULL ) || ( strstr( comm, "ALL KILL" ) != NULL );
	//
	// <petname> kill
	//
	bool requestAttack = ( strstr( comm, " KILL") != NULL ) || ( strstr( comm, " ATTACK") != NULL) ;
	//
	// <petname> fetch
	//
	bool requestFetch = (strstr( comm, " FETCH") !=NULL) || (strstr( comm, " GET") !=NULL);
	//
	// <petname> come
	//
	bool requestCome = ( strstr( comm, " COME") != NULL );
	//
	// <petname> guard me
	//
	bool requestGuardMe = ( strstr( comm, " GUARD ME" ) != NULL);
	//
	// <petname> guard
	//
	bool requestGuard = false;
	if (!requestGuardMe)
		requestGuard = ( strstr(comm, " GUARD") != NULL);
	//
	// <petname> stay
	//
	bool requestStay = ( strstr( comm, " STOP") != NULL ) || ( strstr( comm, " STAY") != NULL );
	//
	// <petname> transfer
	//
	bool requestTransfer = ( strstr( comm, " TRANSFER" ) != NULL );
	//
	// <petname> release
	//
	bool requestRelease = ( strstr( comm, " RELEASE" ) != NULL );
	//
	// release all
	//
	bool requestReleaseAll = ( strstr( comm, "RELEASE ALL" ) != NULL );


	bool handledRequest = false;

	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( x, y, VISRANGE, true, false );

	for( sc.rewind(); !sc.isEmpty(); sc++ )
	{
		P_CHAR pc_map = sc.getChar();
		if(ISVALIDPC(pc_map)) {
		
			//
			// Sparhawk	char_dist() should be a configurable audiorange
			//		default configuration in server.cfg
			//		could be overriden through race definition
			//		could be overriden by npc definition
			//		also range should differ for whispering talking and shouting
			//
			if (pc_map->npc && ( pc->distFrom(pc_map) <= 4 ) )
			{
				//k=mapchar;
				//
				//	Disallow responses from dead or disabled npcs
				//
				if ( !TIMEOUT( pc_map->disabled ) )
				{
					switch ( pc_map->getId() )
					{
						case BODY_MALE		:
						case BODY_FEMALE	:
							pc_map->talkAll( TRANSLATE( "Leave me alone!" ), 0 );
							break;
						default		:
							if ( chance( 40 ) )
								pc_map->playMonsterSound(SND_IDLE);
							break;
					}
					handledRequest = true;
					continue;
				}

				//
				// 	Handle name request
				//		Sparhawk	certain classes like druids should be able to get a name from a non humanoid npc
				//				also for certain skill level in animal lore????
				//
				if ( requestName )
				{
					if( pc_map->npcaitype != NPCAI_EVIL )
					{
						switch ( pc_map->getId() )
						{
							case BODY_MALE		:
							case BODY_FEMALE	:
								sprintf( temp, TRANSLATE("I am %s."), pc_map->getCurrentNameC());
								pc_map->talkAll( temp, 0);
								break;
							default		:
								if ( chance( 40 ) )
									pc_map->playMonsterSound(SND_IDLE);
								break;
						}
					}
					else
					{
						switch ( pc_map->getId() )
						{
							case BODY_MALE		:
							case BODY_FEMALE	:
								sprintf( temp, TRANSLATE("I %s will kill you."), pc_map->getCurrentNameC());
								pc_map->talkAll( temp, 0);
								break;
							default		:
								if ( chance( 40 ) )
									pc_map->playMonsterSound(SND_STARTATTACK);
								break;
						}
					}
					handledRequest = true;
					continue;
				}
				//
				// 	Handle packup player vendor request
				//		Sparhawk	packup should preferably handle items still held for sale by the vendor
				//				also all remaining accounting should be done
				//				<playervendorname> packup should be allowed
				//				packup should packup all player vendors within range
				//
				if ( requestPackup && pc_map->npcaitype == NPCAI_PLAYERVENDOR && (pc->isOwnerOf( pc_map ) || pc->IsGM()))
				{
					int pvDeed;
					// lets make the deed and place in your pack and delete vendor.
					strcpy( temp, "Employment deed" );
					P_ITEM pDeed = item::CreateFromScript( "$item_employment_deed", pc->getBackpack() );
					VALIDATEPIR(pDeed, true);
					pvDeed= DEREF_P_ITEM(pDeed);

					pDeed->Refresh();
					sprintf(temp, TRANSLATE("Packed up vendor %s."), pc_map->getCurrentNameC());
					pc_map->playMonsterSound(SND_DIE);
					pc_map->Delete();
					sysmessage(s, temp);
					// return 1;
					handledRequest = true;
				}
				//
				// 	Handle time request
				//
				if ( requestTime )
				{
					if(pc_map->npcaitype!= NPCAI_EVIL && pc_map->npcaitype!= NPCAI_PLAYERVENDOR && (pc_map->getId()==0x0190 || pc_map->getId()==0x0191))
					{
						int hour = Calendar::g_nHour % 12;
						if (hour==0) hour = 12;
						int ampm = (Calendar::g_nHour>=12) ? 1 : 0;
						int minute = Calendar::g_nMinute;
						if (ampm || (!ampm && hour==12))
							sprintf(temp, "%s %2.2d %s %2.2d %s", TRANSLATE("it is now"), hour, ":", minute, TRANSLATE("in the evening."));
						else
							sprintf(temp, "%s %2.2d %s %2.2d %s", TRANSLATE("it is now"), hour, ":",minute, TRANSLATE("in the morning."));
						pc_map->talkAll( temp, 0);
						return 1;
					}
					handledRequest = true;
					continue;
				}
				//
				// 	Handle location request
				//
				if ( requestLocation )
				{
					if( pc_map->npcaitype!= NPCAI_EVIL && pc_map->npcaitype!= NPCAI_PLAYERVENDOR  && (pc_map->getId()==0x0190 || pc_map->getId()==0x0191))
					{
						if (strlen(region[pc->region].name)>0)
							sprintf(temp, TRANSLATE("You are in %s"),region[pc->region].name);
						else
							strcpy(temp,TRANSLATE("You are in the wilderness"));
						pc_map->talkAll( temp, 0 );
						sprintf( temp, "%i %i (%i)", charpos.x, charpos.y, charpos.z );
						pc_map->talkAll( temp, 0 );
						return 1;	// Sparhawk	No use having multiple npcs return same info
					}
					handledRequest = true;
					continue;
				}
				//
				// 	Handle Accept Escort
				//		Sparhawk	Need to take into account whether pc is allready engaged in escort quest
				//
				if ( requestAcceptEscort && pc_map->questType == MsgBoards::ESCORTQUEST )
				{
					if ( pc_map->ftargserial == INVALID )
					{
						// Set the NPC to follow the PC
						pc_map->ftargserial = pc->getSerial32();

						// Set the NPC to wander freely
						pc_map->npcWander = WANDER_FOLLOW;

						// Set the expire time if nobody excepts the quest
						pc_map->summontimer = ( uiCurrentTime + ( MY_CLOCKS_PER_SEC * SrvParms->escortactiveexpire ) );

						// Send out the rant about accepting the escort
						sprintf(temp, TRANSLATE("Lead on! Payment shall be made when we arrive at %s."), region[pc_map->questDestRegion].name);
						pc_map->talkAll(temp, 0);

						// Remove post from message board (Mark for deletion only - will be cleaned during cleanup)
						MsgBoards::MsgBoardQuestEscortRemovePost( DEREF_P_CHAR(pc_map) );

						// Return 1 so that we indicate that we handled the message
						return 1;
					}
					else
						requestDestination = true;
				}
				//
				// 	Handle find out where escort wants to go
				//
				if ( requestDestination )
				{
					if ( pc_map->questType==MsgBoards::ESCORTQUEST )
					{
						if ( pc_map->ftargserial == pc->getSerial32() )
						{
							// Send out the rant about accepting the escort
							sprintf(temp, TRANSLATE("Lead on to %s. I shall pay thee when we arrive."), region[pc_map->questDestRegion].name);
							pc_map->talkAll(temp, 0);
						}
						else if ( pc_map->ftargserial == INVALID )	// If nobody has been accepted for the quest yet
						{
							// Send out the rant about accepting the escort
							sprintf(temp, TRANSLATE("I am seeking an escort to %s. Wilt thou take me there?"), region[pc_map->questDestRegion].name);
							pc_map->talkAll(temp, 0);
						}
						else // They must be enroute
						{
							// Send out a message saying we are already being escorted
							P_CHAR pc_ftarg=pointers::findCharBySerial(pc_map->ftargserial);
							if(ISVALIDPC(pc_ftarg)) {
								sprintf(temp, TRANSLATE("I am already being escorted to %s by %s."), region[pc_map->questDestRegion].name, pc_ftarg->getCurrentNameC() );
								pc_map->talkAll(temp, 0);
							}
						}
						// Return success ( we handled the message )
						return 1;
					}
					else
					{
						continue;
					}
				}
				//
				// 	Handle bank
				//		Sparhawk	Soundeffect needed let's try to create one depending on the value of banked stuff
				//
				if ( requestBank )
				{
					if ( pc_map->npcaitype == NPCAI_BANKER )
					{
						pc->openBankBox(pc);
						return 1;
					}
					else
					{
						continue;
					}
				}
				//
				// 	Handle balance
				//
				if ( requestBalance )
				{
					if ( pc_map->npcaitype == NPCAI_BANKER )
					{
						sprintf(temp, TRANSLATE("%s's balance as of now is %i."), pc->getCurrentNameC(), pc->countBankGold());
						pc_map->talkAll( temp, 1);
						return 1;
					}
					else
					{
						continue;
					}
				}
				//
				// 	Handle special bank
				//		Sparhawk	Soundeffect needed let's try to create one depending on the value of banked stuff
				//
				if( requestSpecialBank )
				{
					if ( pc_map->npcaitype == NPCAI_BANKER )
					{
						pc->openSpecialBank(pc);
						return 1;
					}
					else
						continue;
				}
				//
				// 	Handle train
				//
				//		Sparhawk	All npcs not only humanoids can teach skills
				//				<trainername> train should be allowed
				//
				if ( requestTrain && pc_map->cantrain && pc_map->HasHumanBody() )
				{
					pc->trainer = INVALID; //this is to prevent errors when a player says "train <skill>" then doesn't pay the npc
					for(i=0;i<ALLSKILLS;i++)
					{
						if( strstr( comm, skillname[i]) )
						{
							skill=i;  //Leviathan fix
							break;
						}
					}

					if(skill == INVALID) // Didn't ask to be trained in a specific skill - Leviathan fix
					{
						if( pc->trainer == INVALID) //not being trained, asking what skills they can train in
						{
							pc_map->trainingplayerin='\xFF'; // Like above, this is to prevent  errors when a player says "train <skill>" then doesn't pay the npc
							strcpy(temp,TRANSLATE("I can teach thee the following skills: "));
							for(j=0;j<ALLSKILLS;j++)
							{
								if(pc_map->baseskill[j]>10)
								{
									sprintf(temp2,"%s, ", strlwr(skillname[j]));
									strupr(skillname[j]); // I found out strlwr changes the actual  string permanently, so this undoes that
									if(!y)
										temp2[0]=toupper(temp2[0]); // If it's the first skill,  capitalize it.
									strcat(temp,temp2);
									y++;
								}
							}
							if( y ) // skills and a trainer ?
							{
								temp[strlen(temp)-2]='.'; // Make last character a . not a ,	just to look nicer
								pc_map->talk(s, temp,0);
							}
							else
							{
							pc_map->talk(s, TRANSLATE("I am sorry, but I have nothing to teach thee"),0);
							}
							return 1;
						}
						handledRequest = true;
					}
					else // They do want to learn a specific skill
					{
						if( pc_map->baseskill[skill] > 10 )
						{
							x=skill;
							sprintf(temp,TRANSLATE("Thou wishest to learn of  %s?"),strlwr(skillname[x]));
							strupr(skillname[x]); // I found out strlwr changes the actual string permanently, so this undoes that
							if(pc->baseskill[x] >= 250 )
							{
								strcat(temp, TRANSLATE(" I can teach thee no more than thou already knowest!"));
							}
							else
							{
								UI32 sum = pc->getSkillSum();
								if (sum >= SrvParms->skillcap * 10)
									strcat(temp, TRANSLATE(" I can teach thee no more. Thou already knowest too much!"));
								else
								{
									int delta = pc_map->getTeachingDelta(pc, skill, sum);
									int perc = (pc->baseskill[x] + delta)/10;

									sprintf(temp2, TRANSLATE(" Very well I, can train thee up to the level of %i percent for %i gold. Pay for less and I shall teach thee less."),perc,delta);
									strcat(temp, temp2);
									pc->trainer=pc_map->getSerial32();
									pc_map->trainingplayerin=x;
								}
								pc_map->talk(s, temp,0);
								return 1;
							}
						}
						pc_map->talk(s, TRANSLATE("I am sorry but I cannot train thee in that skill."),0);
						// return 1;
						handledRequest = true;
					}
				}
				//
				// Handle follow
				//
				if ( requestFollow )
				{
					if (pc->isOwnerOf(pc_map) || (pc->IsGM())) //owner of the char || a GM
					{
						strcpy( search1, pc_map->getCurrentNameC() );
						strupr( search1 );
						bool requestPetname = ( strstr( comm, search1) != NULL );
						if ( requestPetname )
						{
							pc->guarded = false; // Hmmm still not very nice needs to be changed <sparhawk>
							if ( pc_map->npcaitype == NPCAI_PLAYERVENDOR )
							{
								pc_map->talk(s, TRANSLATE("I am sorry but I have to mind the shop."),0);
								return 0;
							}
							if ( requestFollowMe )
							{
								pc_map->ftargserial = pc->getSerial32();
								pc_map->npcWander = WANDER_FOLLOW;
								pc_map->playMonsterSound(SND_STARTATTACK);
								return 1;
							}
							//
							// Select target to follow
							//

							P_TARGET targ = clientInfo[s]->newTarget( new cCharTarget() );
							targ->buffer[0]=pc_map->getSerial32();
							targ->code_callback=target_follow;
							targ->send( getClientFromSocket(s) );
							sysmessage( s, TRANSLATE("Click on the target to follow.") );
							return 1;
						}
					}
					else
						continue;
				}
				//
				// 	Handle Luxor's All Attack
				//
				if ( requestAllAttack )
				{
					NxwCharWrapper sc;
					sc.fillOwnedNpcs( pc, false, true );
					//I don't understand why i must check for animals stabled too... probably a bug
					if ( sc.size()==0 )
					{
						sysmessage(s,TRANSLATE("You dont have pets following you"));
						return 1;
					}
					if ( region[pc->region].priv&0x01 ) // Ripper..No pet attacking in town.
					{
						sysmessage(s,TRANSLATE("You cant have pets attack in town!"));
						return 1;
					}
					pc->guarded = false;
					P_TARGET targ=clientInfo[s]->newTarget( new cCharTarget() );
					targ->code_callback=target_allAttack;
					targ->send( getClientFromSocket( s ) );
					sysmessage( s, TRANSLATE("Select the target to attack."));
					return 1;
				}
				//
				// 	Handle attack
				//
				if ( requestAttack )
				{
					if ( pc->isOwnerOf( pc_map ) || ( pc->IsGM() ) )
					{
						pc->guarded = false;
						strcpy(search1, pc_map->getCurrentNameC() );
						strupr( search1 );
						bool requestPetname = ( strstr( comm, search1) != NULL );
						if ( requestPetname )
						{
							if (region[pc->region].priv&0x01) // Ripper..No pet attacking in town.
							{
								sysmessage( s, TRANSLATE("You can't have pets attack in town!"));
								return 0;
							}
							if (pc_map->npcaitype== NPCAI_PLAYERVENDOR )
								return 0; //ripper

							P_TARGET targ = clientInfo[s]->newTarget( new cCharTarget() );
							targ->code_callback=target_playerVendorBuy;
							targ->buffer[0] = pc_map->getSerial32();
							targ->send( getClientFromSocket(s) );
							//pet kill code here
							sysmessage( s, TRANSLATE("Select the target to attack.") );
							return 1;
						}
					}
					continue;
				}
				//
				// Handle fetch
				//
				if ( requestFetch &&  pc_map->npcaitype != NPCAI_PLAYERVENDOR )
				{
					if (pc->isOwnerOf(pc_map) || (pc->IsGM()))
					{
						strcpy(search1, pc_map->getCurrentNameC());
						strupr(search1);
						bool requestPetname = ( strstr( comm, search1) != NULL );
						if ( requestPetname )
						{
							pc->guarded = false;
							P_TARGET targ = clientInfo[s]->newTarget( new cObjectTarget() );
							targ->code_callback=target_fetch;
							targ->buffer[0]=pc_map->getSerial32();
							targ->send( getClientFromSocket(s) );							
							sysmessage( s, TRANSLATE("Click on the object to fetch."));
							return 1;
						}
					}
					continue;
				}
				//
				// Handle come
				//
				if ( requestCome )
				{
					if ( pc_map->npcaitype != NPCAI_PLAYERVENDOR && (pc->isOwnerOf(pc_map) || pc->IsGM()))
					{
						strcpy(search1, pc_map->getCurrentNameC());
						strupr(search1);
						bool requestPetname = (strstr( comm, search1) != NULL);
						if ( requestPetname )
						{
							pc->guarded = false;
							pc_map->ftargserial=pc->getSerial32();
							pc_map->npcWander=WANDER_FOLLOW;
							sysmessage(s, TRANSLATE("Your pet begins following you."));
							return 1;
						}
					}
					continue;
				}
				//
				// Handle guard
				//
				if ( (requestGuardMe || requestGuard ) )
				{
					if ( pc_map->npcaitype != NPCAI_PLAYERVENDOR && (pc->isOwnerOf(pc_map) || pc->IsGM()))
					{
						strcpy(search1, pc_map->getCurrentNameC());
						strupr(search1);
						bool requestPetname = ( strstr( comm, search1) != NULL);
						if (requestPetname)
						{
							P_TARGET targ=clientInfo[s]->newTarget( new cCharTarget() );
							targ->code_callback=target_guard;
							targ->buffer[0] = pc_map->getSerial32();	// the pet's serial
							targ->buffer[1] = 0;
							if ( requestGuardMe )
								targ->buffer[1]=1;	// indicates we already know whom to guard (for future use)
										// for now they still must click on themselves (Duke)
							targ->send( getClientFromSocket( s ) );
							sysmessage( s, TRANSLATE("Click on the char to guard.") );

							return 1;
						}
					}
					continue;
				}
				//
				// Handle stay
				//	Sparhawk	allow for stay/stop without petname selection is done through target cursor
				//
				if ( requestStay )
				{
					if (pc_map->npcaitype !=NPCAI_PLAYERVENDOR && (pc->isOwnerOf(pc_map) || pc->IsGM()))
					{
						strcpy( search1, pc_map->getCurrentNameC());
						strupr( search1 );
						if ( strstr( comm, search1) != NULL ) //if petname is in
						{
							pc->guarded = false; // Sparhawk	How about when more than 1 pets is guarding me??
							//pet stop code here
							pc_map->ftargserial= INVALID;
							pc_map->targserial= INVALID;
							if ( pc_map->war )
								pc_map->toggleCombat();	// Sparhawk Allmost all pet commands need to do this and non pet commands need
												// to check for war status
							pc_map->npcWander=WANDER_NOMOVE;
							return 1;
						}
					}
					continue;
				}
				//
				// Handle transfer ownership of pet
				//
				if ( requestTransfer )
				{
					if ( pc_map->npcaitype != NPCAI_PLAYERVENDOR && (pc->isOwnerOf(pc_map) || pc->IsGM()) )
					{
						strcpy( search1, pc_map->getCurrentNameC() );
						strupr( search1 );
						if ( strstr( comm, search1 ) != NULL )
						{
							pc->guarded = false; // Sparhawk	How about when more than 1 pets is guarding me??
							
							if (pc_map->amxevents[EVENT_CHR_ONTRANSFER])
							{
								g_bByPass = false;
								pc_map->amxevents[EVENT_CHR_ONTRANSFER]->Call(pc_map->getSerial32(), pc->getSerial32());
								if (g_bByPass==true)
									return 0;
							}
							/*
							pc_map->runAmxEvent( EVENT_CHR_ONTRANSFER, pc_map->getSerial32(), pc->getSerial32());
							if (g_bByPass==true)
								return 0;
							*/
							//pet transfer code here
							P_TARGET targ = clientInfo[s]->newTarget( new cCharTarget() );
							targ->code_callback=target_transfer;
							targ->buffer[0]=pc_map->getSerial32();
							targ->send( getClientFromSocket(s) );
							sysmessage( s, TRANSLATE("Select character to transfer your pet to."));
							return 1;
						}
					}
					continue;
				}
				//
				// Handle release pet
				//
				if ( requestRelease )
				{
					if ( pc_map->npcaitype != NPCAI_PLAYERVENDOR && (pc->isOwnerOf(pc_map) || pc->IsGM() ))
					{
						strcpy(search1, pc_map->getCurrentNameC());
						strupr(search1);
						if ( strstr( comm, search1) != NULL )
						{
							pc->guarded = false; // Sparhawk	How about when more than 1 pets is guarding me??
							if (pc_map->summontimer)
							{
								pc_map->summontimer=uiCurrentTime;
							}
							//pet release code here
							pc_map->ftargserial=INVALID;
							pc_map->npcWander=WANDER_FREELY_CIRCLE;
							pc_map->setOwnerSerial32(INVALID);
							pc_map->taming=2000;//he cannot be retamed	Sparhawk	This is bullshit!!!
							//taken from 6904t2(5/10/99) - AntiChrist
							pc_map->tamed = false;
							sprintf(temp, TRANSLATE("*%s appears to have decided that it is better off without a master *"), pc_map->getCurrentNameC());
							pc_map->talkAll(temp,0);
							pc_map->playSFX( 0x01FE );
							if(SrvParms->tamed_disappear==1)
								pc_map->Delete();
							return 1;
						}
					}
					continue;
				}
				//
				// Handle release all pets
				//
				if ( requestReleaseAll )
				{
					if( pc_map->npcaitype != NPCAI_PLAYERVENDOR && (pc->isOwnerOf(pc_map) || pc->IsGM() ) )
					{
						if (pc_map->summontimer)
						{
							pc_map->summontimer=uiCurrentTime;
						}
						//pet release code here
						pc_map->ftargserial=INVALID;
						pc_map->npcWander=WANDER_FREELY_CIRCLE;
						pc_map->setOwnerSerial32(INVALID);
						pc_map->taming=2000;//he cannot be retamed
						pc_map->tamed = false;
						sprintf(temp, TRANSLATE("*%s appears to have decided that it is better off without a master *"), pc_map->getCurrentNameC());
						pc_map->talkAll(temp,0);
						pc_map->playSFX( 0x01FE);
						if(SrvParms->tamed_disappear==1)
							pc_map->Delete();
						handledRequest = true;	// Sparhawk	all pets in vicinity should be released
					}
					else
						continue;
				}
				//
				// 	Handle speech trigger
				//
				if ( pc_map->trigger && ( pc_map->trigword.length()>0 ) )
				{
					if (pc_map->trigword.length()>0)
					{
						strcpy(search1, pc_map->trigword.c_str());
						strupr(search1);
						bool requestTrigger = ( strstr( comm, search1) != NULL );
						if ( requestTrigger )
						{
							if ( !TIMEOUT( pc_map->disabled ) )//AntiChrist
								pc_map->talkAll(TRANSLATE("I'm a little busy now! Leave me be!"),0);
							else
							{
								triggerNpc(s,pc_map,TRIGTYPE_NPCWORD);
							}
							//return 1;	// Sparhawk multiple npcs should be triggered if having the same trigword
							handledRequest = true;
						}
					}
					continue;
				}
			}
		}

	}

	if ( handledRequest )
		return 1;
	else
		return 0;


}

void PlVGetgold(NXWSOCKET s, CHARACTER v)//PlayerVendors
{
	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc_currchar = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC( pc_currchar );
	P_CHAR pc_vendor = MAKE_CHAR_REF(v);
	VALIDATEPC( pc_vendor );

	unsigned int pay=0, give=pc_vendor->holdg, t=0;
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	if (pc_currchar->isOwnerOf( pc_vendor ))
	{
		if ( pc_vendor->holdg<1)
		{
			pc_vendor->talk(s,TRANSLATE("I have no gold waiting for you."),0);
			pc_vendor->holdg=0;
			return;
		}

		if( pc_vendor->holdg <= 65535 )
		{
			if ( pc_vendor->holdg>9)
			{
				pay=(int)(pc_vendor->holdg*.1);
				give-=pay;
			}
			else
			{
				pay = pc_vendor->holdg;
				give=0;
			}
			pc_vendor->holdg=0;
		}
		else
		{
			t = pc_vendor->holdg-65535;
			pc_vendor->holdg=65535;
			pay=6554;
			give=58981;
		}
		if (give) //Luxor
			item::CreateFromScript( "$item_gold_coin", pc_currchar->getBackpack(), give );

		sprintf(temp, TRANSLATE("Today's purchases total %i gold. I am keeping %i gold for my self. Here is the remaining %i gold. Have a nice day."),pc_vendor->holdg,pay,give);
		pc_vendor->talk(s,temp,0);
		pc_vendor->holdg=t;
	}
	else
		pc_vendor->talk(s,TRANSLATE("I don't work for you!"),0);
}

void responsevendor(NXWSOCKET  s, CHARACTER vendor)
{

//	CHARACTER cc=currchar[s];

	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_currchar);
	P_CHAR pc_vendor = MAKE_CHAR_REF(vendor);
	VALIDATEPC(pc_vendor);

	static char buffer1[MAXBUFFER_REAL]; // static becasue maxbuffer_ral close to stack limit of win-machines
	int i;
	char *comm;

	char *response1;
	char *response2;
	char *response3;
	char *response4;

	char search1[50];
	char search2[50];
	char search3[50];
	char search4[50];

	static char nonuni[MAXBUFFER_REAL];

	if(pc_currchar->unicode)
	{
		wchar2char( (char*) &buffer[s][13]);
		strcpy(nonuni, Unicode::temp);
	}

	int MB = MAXBUFFER_REAL;
	//if (server_data.packetsendstyle==PSS_UOX3) MB = MAXBUFFER_ASYNCH;
	// xan -> not enabled yet :|
	////////// ouch, this hurts !!!

	if(!(pc_currchar->unicode))
	{
	for (i=7; i < MB; i++)
		{
			tbuffer[i]=buffer1[i];
			buffer1[i]=toupper(buffer[s][i]);
		}
	}
	else
	{
	for (i=0; i < MB-8; i++)
		{
			tbuffer[i+8]=buffer1[i+8];
			buffer1[i+8]=toupper(nonuni[i]);
		}
	}
	/// end of that hurts

		comm=&buffer1[8];

		// Player said vendor BUY or vendor sell
		// In that case we alerady have the vendor-number from hte calling function
		// and dont need to search again
		strcpy(search1, "VENDOR");
		strcpy(search2, "SHOPKEEPER");
		strcpy(search4, " BUY");
		response1=(strstr(comm, search1));
		response2=(strstr(comm, search2));
		response4=(strstr(comm, search4));

		if (response4)//AntiChrist
		{
			if (response2 || response1)
			{
				if(pc_vendor->npcaitype==NPCAI_PLAYERVENDOR)
				{
					pc_vendor->talk(s,TRANSLATE("What would you like to buy?"),0);
					P_TARGET targ = clientInfo[s]->newTarget( new cItemTarget() );
					targ->code_callback=target_playerVendorBuy;
					targ->buffer[0]=pc_vendor->getSerial32();
					targ->send( getClientFromSocket(s) );
					return; // lb bugfix
				}
				else if(BuyShop(s, DEREF_P_CHAR(pc_vendor)))
					return; // lb bugfix
			}
		}

		//PlayerVendors
		response4=0;
		strcpy(search4, " GOLD");
		response4=(strstr( comm, search4));
		if (response4)//AntiChrist
		{
			if (response2 || response1)
			{
				if (pc_vendor->npcaitype==NPCAI_PLAYERVENDOR)
				{
					PlVGetgold(s, DEREF_P_CHAR(pc_vendor));
					return;
				}
			}
		}

		//end PlayerVendors --^

		response4=0;
		strcpy(search4, " SELL");
		response4=(strstr( comm, search4));
		if (response4)//AntiChrist
		{
			if (response2 || response1)
			{
				sellstuff(s, DEREF_P_CHAR(pc_vendor));
				return;
			}
		}


		// IF vendors are referred by NAME the vendor number passed by
		// doesnt have to be correct, thus we have to search again.
		// LB

		response4=strstr(comm," BUY");

		if (response4)//AntiChrist
		{
			
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc_currchar->getPosition(), 2, true, false );
			for( sc.rewind(); !sc.isEmpty(); sc++ ) {
				P_CHAR pc = sc.getChar();
				if(!ISVALIDPC(pc))
					continue;
				strcpy(search3, pc->getCurrentNameC());
				strupr(search3);
				response3=(strstr( comm, search3));
				if (response3)
				{
					if(pc->npcaitype==NPCAI_PLAYERVENDOR)
					{
						pc->talk(s,TRANSLATE("What would you like to buy?"),0);
						P_TARGET targ= clientInfo[s]->newTarget( new cItemTarget() );
						targ->code_callback = target_playerVendorBuy;
						targ->buffer[0]=pc->getSerial32();
						targ->send( getClientFromSocket(s) );
						return;
					}
					else
						if(BuyShop(s, DEREF_P_CHAR(pc)))
							return;
				}
			}
		} // if response4


		//PlayerVendors
		response4=0;
		strcpy(search4, " GOLD");
		response4=(strstr( comm, search4));

		if (response4)
		{
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc_currchar->getPosition(), VISRANGE, true, false );

			for( sc.rewind(); !sc.isEmpty(); sc++ )
			{
				P_CHAR pc = sc.getChar();
				if(!ISVALIDPC(pc))
					continue;

				strcpy(search3, pc->getCurrentNameC());
				strupr(search3);
				response3=(strstr( comm, search3));
				if (response3)
				{
					if (pc->npcaitype==NPCAI_PLAYERVENDOR)
					{
						PlVGetgold(s, DEREF_P_CHAR(pc));
						return;
					}
				}
			}
		}


		response4=0;
		strcpy(search4, " SELL");
		response4=(strstr( comm, search4));

		if (response4)
		{
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc_currchar->getPosition(), VISRANGE, true, false );

			for( sc.rewind(); !sc.isEmpty(); sc++ )
			{
				P_CHAR pc = sc.getChar();
				if(!ISVALIDPC(pc))
					continue;
				strcpy(search3, pc->getCurrentNameC());
				strupr(search3);
				response3=(strstr( comm, search3));
				if (response3)
				{
					sellstuff(s, DEREF_P_CHAR(pc));
					return;
				}
			}
		} // if response4
	return;
}

/*
static LOGICAL respond( P_CHAR pc, NXWSOCKET socket, string &speech )
{
	LOGICAL success = false;
	pCharVector	nearbyPlayerVendors;
	UI32		i = 0;
	UI32		j = nearbyNpcs->size();
	while( i < j )
	{
		pc_a_npc = (*nearbyNpcs)[i];
		if( pc_a_npc->npc_ai == NPCAI_PLAYERVENDOR )
			nearbyPlayerVendors.push_back(pc_a_npc);
		++i;
	}
	if( !nearbyPlayerVendors.empty() )
	{
	}
	return success;
}
*/

static LOGICAL pricePlayerVendorItem( P_CHAR pc, NXWSOCKET socket, string &price )
{
	LOGICAL success = false;
	if ( pc->fx2 == 17 )
	{
		int i = str2num( const_cast<char*>(price.c_str()) );
		P_ITEM pi = MAKE_ITEM_REF( pc->fx1 );
		if( ISVALIDPI( pi ) )
		{
			if (i>0)
			{
				pi->value = i;
				pc->fx2 = 18;
				sysmessage( socket, TRANSLATE("The price of item %s has been set to %i."), pi->getCurrentNameC(), i);
				sysmessage( socket, TRANSLATE("Enter a description for this item."));
			}
			else
			{
				pc->fx2 = 18;
				sysmessage( socket, TRANSLATE("No price entered, this item's price has been set to %i."), pi->value);
				sysmessage( socket, TRANSLATE("Enter a description for this item."));
			}
		}
		else
		{
			sysmessage( socket, TRANSLATE("This item is invalid and cannot be priced") );
			pc->fx1 = INVALID;
			pc->fx2 = INVALID;
		}
		success = true;
	}
	return success;
}

static LOGICAL describePlayerVendorItem( P_CHAR pc, NXWSOCKET socket, string &description )
{
	LOGICAL success = false;
	if( pc->fx2 == 18 )
	{
		P_ITEM pi = MAKE_ITEM_REF( pc->fx1 );
		if( ISVALIDPI( pi ) )
		{
			//strcpy( pi->desc, description.c_str() );
			pi->vendorDescription = description;
			sysmessage( socket, TRANSLATE("This item is now described as %s, "), description.c_str() );
		}
		else
			sysmessage( socket, TRANSLATE("This item is invalid and cannot be described") );
		pc->fx1 = INVALID;
		pc->fx2 = INVALID;
		success = true;
	}
	return success;
}

static LOGICAL renameRune( P_CHAR pc, NXWSOCKET socket, string &name )
{
	LOGICAL success = false;
	P_ITEM pi = pointers::findItemBySerial( pc->runeserial );
	if( ISVALIDPI( pi ) )
	{
		pi->setCurrentName( TRANSLATE("Rune to %s"), name.c_str() );
		sysmessage( socket, TRANSLATE("Rune renamed to: Rune to %s"), name.c_str() );
		pc->runeserial = INVALID;
		success = true;
	}
	return success;
}

static LOGICAL renameSelf( P_CHAR pc, NXWSOCKET socket, string &name )
{
	LOGICAL success = false;
	if( pc->namedeedserial != INVALID )
	{
		P_ITEM pi = pointers::findItemBySerial( pc->namedeedserial );
		if( ISVALIDPI( pi ) )
		{
			pi->setCurrentName( name.c_str());
			pc->setCurrentName( name.c_str());
			sysmessage(socket, TRANSLATE("Your new name is: %s"), name.c_str());
		}
		else
			sysmessage(socket, TRANSLATE("Invalid namedeed") );
		pc->namedeedserial = INVALID;
		success = true;
	}
	return success;
}

static LOGICAL renameKey( P_CHAR pc, NXWSOCKET socket, string &name )
{
	LOGICAL success = false;
	P_ITEM pi = pointers::findItemBySerial( pc->keyserial );
	if( ISVALIDPI( pi ) )
	{
		pi->setCurrentName( name.c_str() );
		sysmessage( socket, TRANSLATE("Key renamed to: %s"), name.c_str() );
		success = true;
	}
	pc->keyserial = INVALID;
	return success;
}

static LOGICAL pageGameMaster( P_CHAR pc, NXWSOCKET socket, string &reason )
{
	LOGICAL success = false;
	if (pc->pagegm == 1)
	{
		char temp[TEMP_STR_SIZE];
		strcpy( gmpages[ pc->playercallnum ].reason, reason.c_str());
		sprintf(temp, "GM Page from %s [%08x]: %s", pc->getCurrentNameC(), pc->getSerial32(), gmpages[pc->playercallnum].reason);
		bool foundGm = false;
		
		NxwSocketWrapper sw;
		sw.fillOnline( );
		
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWCLIENT ps=sw.getClient();
			if( ps==NULL )
				continue;
			
			P_CHAR gamemaster = ps->currChar();
			if( ISVALIDPC( gamemaster ) )
				if ( gamemaster->IsGM() )
				{
					foundGm = true;
					sysmessage(ps->toInt(), temp);
				}
		}
		if (foundGm)
			sysmessage(socket, TRANSLATE("Available Game Masters have been notified of your request."));
		else
			sysmessage(socket, TRANSLATE("There was no Game Master available to take your call."));
		pc->pagegm = 0;
		success = true;
	}
	return success;
}

static LOGICAL pageCouncillor( P_CHAR pc, NXWSOCKET socket, string &reason )
{
	LOGICAL success = false;
	if (pc->pagegm == 2) // Counselor page
	{
		char temp[TEMP_STR_SIZE];
		strcpy( counspages[pc->playercallnum].reason, reason.c_str() );
		sprintf(temp, TRANSLATE("Counselor Page from %s [%08x]: %s"),pc->getCurrentNameC(), pc->getSerial32(), counspages[pc->playercallnum].reason);
		bool foundCons = false;
		P_CHAR councillor;
		
		NxwSocketWrapper sw;
		sw.fillOnline( );
		
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWCLIENT ps=sw.getClient();
			if( ps==NULL )
				continue;
			
			councillor = ps->currChar();
			if( ISVALIDPC( councillor ) )
				if ( councillor->IsCounselor() )
				{
					foundCons = true;
					sysmessage(ps->toInt(), temp);
				}
		}
		if (foundCons)
			sysmessage(socket, TRANSLATE("Available Counselors have been notified of your request."));
		else
			sysmessage(socket, TRANSLATE("There was no Counselor available to take your call."));
		pc->pagegm = 0;
		success = true;
	}
	return success;
}

static LOGICAL resignFromGuild( P_CHAR pc, NXWSOCKET socket, string &resign )
{
	LOGICAL success = false;
	if (!resign.compare("I RESIGN FROM MY GUILD"))
	{
		Guilds->Resign( pc, socket );
		success = true; // just to make sure this speech is not processed further
	}
	return success;
}

static LOGICAL callGuards( P_CHAR pc, NXWSOCKET socket, string &helpcall )
{
	LOGICAL success = false;
	if( helpcall.find("GUARDS") != std::string::npos )
	{
		//
		// TODO Check if not part of another word or part of not help call related speech
		//
		callguards(currchar[socket]);
		success = true;
	}
	return success;
}

namespace Speech
{
namespace Stablemaster
{
static LOGICAL respond( P_CHAR pc, NXWSOCKET socket, std::string &speech );
static LOGICAL stablePet( P_CHAR pc, NXWSOCKET socket, std::string &speech, NxwCharWrapper &nearbyStablemasters );
static LOGICAL claimPet( P_CHAR pc, NXWSOCKET socket, std::string &speech, NxwCharWrapper &nearbyStablemasters );

static LOGICAL respond( P_CHAR pc, NXWSOCKET socket, std::string &speech )
{
	LOGICAL 	success = false;
	NxwCharWrapper	nearbyStablemasters;
	P_CHAR		pc_a_npc;

	for( nearbyNpcs->rewind(); !nearbyNpcs->isEmpty(); (*nearbyNpcs)++ )
	{
		pc_a_npc = nearbyNpcs->getChar();
		if( (pc_a_npc->npc_type == 1) && ( pc_a_npc->distFrom( pc ) <= 5 ) )
			nearbyStablemasters.insertChar( pc_a_npc );
	}

	nearbyStablemasters.rewind();

	if( !nearbyStablemasters.isEmpty() )
	{
		if( !success )	success = stablePet( pc, socket, speech, nearbyStablemasters );
		if( !success )	success = claimPet( pc, socket, speech, nearbyStablemasters );
	}
	return success;
}

static LOGICAL stablePet( P_CHAR pc, NXWSOCKET socket, std::string &speech, NxwCharWrapper &nearbyStablemasters )
{
/*
	command					action
	--------------------------------	--------------
	stable					stablemaster nearest to pc stables pet nearest to stablemaster

	stable <pet>				stablemaster nearest to pc stables pet with name <pet>
						if multiple pets with same name exist the one nearest to the stablemaster will be selected

	stable all				stablemaster nearest to pc stables all pets

	<stable master> stable			stablemaster with name <stable master> stables pet nearest to him/her

	<stable master> stable <pet>		stablemaster with name <stable master> stables pet with <name>
						if multiple pets with same name exist the one nearest to the stablemaster

	<stable master> stable all		stablemaster with name <stable master> stables all pets

	<stable master> stable all <pet>	stablemaster with name <stable master> stables all pets with name <pet>
*/
	LOGICAL success = false;
	SI32 tokenPosition = findKeyword( speech, "STABLE");
	if( tokenPosition != INVALID )
	{
		std::string 	stablemasterName( trimString( speech.substr( 0, tokenPosition ) ) );
		LOGICAL 	findStablemasterByName = (stablemasterName.length() != 0);
		std::string 	petName( trimString( speech.substr( tokenPosition + 6) ) );
		tokenPosition = findKeyword( petName, "ALL");
		LOGICAL stableAllPets = (tokenPosition == 0 );
		if( stableAllPets )
			petName = trimString( petName.substr( 3 ) );
		LOGICAL 	findPetByName = (petName.length() != 0);
		//
		// Find stable master, in case of multiple stable masters the one nearest will be selected
		//
		P_CHAR		pc_stablemaster = 0;
		P_CHAR		pc_pet = 0;
		P_CHAR		pc_a_npc;
		NxwCharWrapper	petsToStable;
		LOGICAL		petFound = false;

		for( nearbyStablemasters.rewind(); !nearbyStablemasters.isEmpty(); nearbyStablemasters++  )
		{
			pc_a_npc = nearbyStablemasters.getChar();
			if( (!findStablemasterByName ||
			    (findStablemasterByName && !strcasecmp(stablemasterName.c_str(), pc_a_npc->getCurrentNameC()))))
			{
				if( !pc_stablemaster )
					pc_stablemaster = pc_a_npc;
				else
					if( (!pc_a_npc->war) && ( pc_stablemaster->distFrom( pc ) >  pc_a_npc->distFrom( pc ) ) )
						pc_stablemaster = pc_a_npc;
			}
		}
		if( !pc_stablemaster )
			success = true;
		else
		{
			for( nearbyNpcs->rewind(); !nearbyNpcs->isEmpty(); (*nearbyNpcs)++ )
			{
				pc_a_npc = nearbyNpcs->getChar();
				if( pc->isOwnerOf( pc_a_npc ) && !pc_a_npc->isStabled() && !pc_a_npc->mounted )
				{
					if( (!findPetByName) || (findPetByName && !strcasecmp( petName.c_str(), pc_a_npc->getCurrentNameC())))
						if( pc->distFrom( pc_stablemaster ) <= 8 )
							if( stableAllPets )
							{
								petFound = true;
								petsToStable.insertChar( pc_a_npc );
							}
							else
								if( !petFound )
								{
									petFound = true;
									petsToStable.insertChar( pc_a_npc );
								}
								//else
									//
									// Multiple pets select nearest ( NOT IMPLEMENTED YET )
									//
									//if( char_dist( pc_stablemaster, pc_a_npc ) < char_dist( pc_stablemaster, petsToStable[0] ) )
									//	petsToStable[0] = pc_a_npc;
				}
			}
			if( !petFound )
			{
				pc_stablemaster->talk( socket, TRANSLATE("Which pet?"), 0);
				return true;
			}
			else
			{
				for( petsToStable.rewind(); !petsToStable.isEmpty(); petsToStable++ )
				{
					pc_pet = petsToStable.getChar();
					
					SERIAL pc_pet_serial = pc_pet->getSerial32();
		 
					NxwSocketWrapper sw;
					sw.fillOnline( pc_pet, false );
					for ( sw.rewind(); !sw.isEmpty(); sw++ ) {
						NXWCLIENT ps=sw.getClient();
						if( ps==NULL )
							continue;
						
						SendDeleteObjectPkt(ps->toInt(), pc_pet_serial);
					}

					pc_pet->war=0;
					//p_pet->ftarg=-1;
					//p_pet->targ=-1;
					pc_pet->attackerserial= INVALID;
					pc->war  	= 0;
					pc->targserial	= INVALID;
#ifdef SPAR_C_LOCATION_MAP
					pointers::delFromLocationMap( pc_pet );
#else
					mapRegions->remove( pc_pet );
#endif
					pc_pet->stable( pc_stablemaster );
					// set timer for fee calculation
					pc_pet->time_unused=0;
					pc_pet->timeused_last = getclock();
				}
				char temp[TEMP_STR_SIZE];
				if( petsToStable.size() == 1 )
					sprintf(temp,TRANSLATE("I have stabled %s"), pc_pet->getCurrentNameC());
				else
					sprintf(temp,TRANSLATE("I have stabled %d pets"), petsToStable.size() );
				pc_stablemaster->talk(socket,temp,0);
			}
			success = true;
		}
	}
	return success;
}

static LOGICAL claimPet( P_CHAR pc, NXWSOCKET socket, std::string &speech, NxwCharWrapper &nearbyStablemasters )
{
	/*
		command					Action
		---------------------------------	-------------
		claim					return a pet
		claim <pet>				return a pet named <pet>
		claim all				return all pets
		claim all <pet>				return all pets named <pet>
		<stablemaster> claim
		<stablemaster> claim <pet>
		<stablemaster> claim all <pet>
	*/
	LOGICAL success = false;
	SI32 tokenPosition = findKeyword( speech, "CLAIM");
	if( tokenPosition != INVALID )
	{
		std::string 	stablemasterName( trimString( speech.substr( 0, tokenPosition ) ) );
		LOGICAL 	findStablemasterByName = (stablemasterName.length() != 0);
		std::string 	petName( trimString( speech.substr( tokenPosition + 5) ) );
		tokenPosition = findKeyword( petName, "ALL");
		LOGICAL 	claimAllPets = (tokenPosition == 0 );
		if( claimAllPets )
			petName = trimString( petName.substr( 3 ) );
		LOGICAL 	findPetByName = (petName.length() != 0);
		P_CHAR		pc_a_npc = 0;
		P_CHAR		pc_stablemaster = 0;
		P_CHAR		pc_pet = 0;
		LOGICAL 	found = false;
		NxwCharWrapper	stabledPets;
		for( nearbyStablemasters.rewind(); !nearbyStablemasters.isEmpty() && !found; nearbyStablemasters++ )
		{
			pc_a_npc = nearbyStablemasters.getChar();
			if( (!findStablemasterByName ||
			    (findStablemasterByName && !strcasecmp( stablemasterName.c_str(), pc_a_npc->getCurrentNameC()))))
			{
				if( !pc_stablemaster )
					pc_stablemaster = pc_a_npc;
				else
					if( pc_stablemaster->distFrom( pc ) > pc_a_npc->distFrom( pc ) )
						pc_stablemaster = pc_a_npc;
				//
				// Stable master found, check stabled animals
				//
				if( !pc_stablemaster->war )
				{
					SI32 ii = 0;
					while ( ( pc_a_npc = pointers::stableSearch( pc_stablemaster->getSerial32(), &ii ) ) != 0 )
					{
						if( pc->isOwnerOf( pc_a_npc ) )
							if(findPetByName)
							{
								if(!strcasecmp(petName.c_str(),pc_a_npc->getCurrentNameC()))
								{
									found = true;
									stabledPets.insertChar( pc_a_npc );
									if( !claimAllPets )
										break;
								}
							}
							else
							{
								stabledPets.insertChar( pc_a_npc );
								if( !claimAllPets )
								{
									found = true;
									break;
								}
							}
					}
				}
			}
		}
		
stabledPets.rewind();	// GH!

		if( stabledPets.isEmpty() )
		{
				for( nearbyStablemasters.rewind(); !nearbyStablemasters.isEmpty(); nearbyStablemasters++ )
				{
					pc_stablemaster = nearbyStablemasters.getChar();
					if( pc_stablemaster->war )
						pc_stablemaster->talk( socket, TRANSLATE("I'm busy!"), 0 );
					else
						pc_stablemaster->talk( socket, TRANSLATE("I don't have it"), 0 );
				}
			success = true;
		}
		else
		{
			//
			// Calculate and display stabling fee = (fee per 10 minutes) * number of 10 minute blocks + 5 basefee
			//
			float 	f_fee	= 0.0f;
			UI32	fee	= 0;
			//
			// Calculate fee. Game master stable for free, everyone else has to pay
			//
			if( !pc->IsGM() )
			{
				for( stabledPets.rewind(); !stabledPets.isEmpty(); stabledPets++  )
				{
					pc_pet	 = stabledPets.getChar();
					f_fee	+= ( pc_pet->time_unused / 600.0f) * SrvParms->stablingfee;
					fee	+= ( (int) f_fee) + 5;
				}
			}
			char temp[TEMP_STR_SIZE];
			sprintf(temp,TRANSLATE("That's %i gold pieces"), fee );
			pc_stablemaster->talk( socket, temp, 0);
			//
			// Check pet owner financials
			//
			if( pc->CountGold() >= fee )
			{
				pc->getBackpack()->DeleteAmount( fee, 0x0EED, INVALID );
				//
				// Unstable pet
				//
				for( stabledPets.rewind(); !stabledPets.isEmpty(); stabledPets++  )
				{
					pc_pet = stabledPets.getChar();
					
					pc_pet->unStable();
					
					pc_pet->timeused_last = getclock();
					pc_pet->time_unused=0;
#ifdef SPAR_C_LOCATION_MAP
					pointers::addToLocationMap( pc_pet );
#else
					mapRegions->add( pc_pet );
#endif
					pc_pet->teleport();
				}
				if( stabledPets.size() == 1 )
					sprintf( temp, TRANSLATE("Thx! Here's your pet"));
				else
					sprintf( temp, TRANSLATE("Thx! Here're your %d pets"), stabledPets.size());
			}
			else
				sprintf(temp,TRANSLATE("You don't have %i gold pieces on you!"), fee );
			pc_stablemaster->talk( socket, temp, 0);
			success = true;
		}
	}
	return success;
}

} //namespace Stablemaster

namespace Guard
{
static LOGICAL respond( P_CHAR pc, NXWSOCKET socket, std::string &speech );
static LOGICAL requestChaosShield( P_CHAR pc, NXWSOCKET socket, std::string &speech, NxwCharWrapper &nearbyGuards );
static LOGICAL requestOrderShield( P_CHAR pc, NXWSOCKET socket, std::string &speech, NxwCharWrapper &nearbyGuards );
static LOGICAL requestHelp( P_CHAR pc, NXWSOCKET socket, std::string &speech, NxwCharWrapper &nearbyGuards );

static LOGICAL respond( P_CHAR pc, NXWSOCKET socket, std::string &speech )
{
	LOGICAL success = false;
	NxwCharWrapper	nearbyGuards;
	NxwCharWrapper	nearbyOrderGuards;
	NxwCharWrapper	nearbyChaosGuards;
	P_CHAR		pc_a_npc;

	for( nearbyNpcs->rewind(); !nearbyNpcs->isEmpty(); (*nearbyNpcs)++ )
	{
		pc_a_npc = nearbyNpcs->getChar();
		switch( pc_a_npc->npcaitype )
		{
			case NPCAI_GUARD	:
			case NPCAI_TELEPORTGUARD:
				nearbyGuards.insertChar( pc_a_npc );
				break;
			case NPCAI_ORDERGUARD	:
				if( pc->distFrom( pc_a_npc ) < 5 )
					nearbyOrderGuards.insertChar( pc_a_npc );
				break;
			case NPCAI_CHAOSGUARD	:
				if( pc->distFrom( pc_a_npc ) < 5 )
					nearbyChaosGuards.insertChar( pc_a_npc );
				break;
		}
	}
	
nearbyGuards.rewind();		//GH!
nearbyOrderGuards.rewind();
nearbyChaosGuards.rewind();

	if( !nearbyOrderGuards.isEmpty() )
	{
		if( !success )	success = requestOrderShield( pc, socket, speech, nearbyOrderGuards );
	}
	if( !nearbyChaosGuards.isEmpty() )
	{
		if( !success )	success = requestChaosShield( pc, socket, speech, nearbyChaosGuards );
	}
	if( !success )	success = requestHelp( pc, socket, speech, nearbyGuards );
	return success;
}

static LOGICAL requestChaosShield( P_CHAR pc, NXWSOCKET socket, std::string &speech, NxwCharWrapper &nearbyChaosGuards )
{
	LOGICAL success = false;
	SI32 tokenPosition = findKeyword( speech, "SHIELD");
	if( tokenPosition != INVALID )
	{
		std::string 	objectName( trimString( speech.substr( tokenPosition + 6) ) );
		if( objectName.empty() )
		{
			std::string 	guardName( trimString( speech.substr( 0, tokenPosition ) ) );
			LOGICAL 	findGuardByName = (guardName.length() != 0);
			P_CHAR		pc_a_npc;
			P_CHAR		chaosGuard = 0;
			for( nearbyChaosGuards.rewind(); !nearbyChaosGuards.isEmpty(); nearbyChaosGuards++  )
			{
				pc_a_npc = nearbyChaosGuards.getChar();
				if( (!findGuardByName) || findGuardByName && !strcasecmp( guardName.c_str(), pc_a_npc->getCurrentNameC() ) )
					if( !chaosGuard )
						chaosGuard = pc_a_npc;
					else
						if( pc->distFrom( chaosGuard ) > pc->distFrom( pc_a_npc ) )
							chaosGuard = pc_a_npc;
			}
			if( chaosGuard )
			{
				if( Guilds->GetType( pc->GetGuildNumber() ) == 2 || pc->IsGM() )
				{
					if( !pc->getAmount(0x1BC3) )
					{
						P_ITEM pi =  pc->GetItemOnLayer( 2 );
						if( ISVALIDPI( pi ) )
						{
							if( pi->getId() != 0x1BC3 )
								success = true;
						}
						else
							success = true;
					}
					if( success )
					{
						item::CreateFromScript( "$item_chaos_shield", pc->getBackpack() );
						chaosGuard->talk( socket, TRANSLATE("Here's is your new shield."), 0);
					}
					else
						chaosGuard->talk( socket, TRANSLATE("You already have a Chaos shield!"), 0);
				}
				else
					chaosGuard->talk( socket,TRANSLATE("You're not a Chaos guild member!"), 0);
				success = true;
			}
		}
	}
	return success;
}

static LOGICAL requestOrderShield( P_CHAR pc, NXWSOCKET socket, std::string &speech, NxwCharWrapper &nearbyOrderGuards )
{
	LOGICAL success = false;
	SI32 tokenPosition = findKeyword( speech, "SHIELD");
	if( tokenPosition != INVALID )
	{
		std::string 	objectName( trimString( speech.substr( tokenPosition + 6) ) );
		if( objectName.empty() )
		{
			std::string 	guardName( trimString( speech.substr( 0, tokenPosition ) ) );
			LOGICAL 	findGuardByName = (guardName.length() != 0);
			P_CHAR		pc_a_npc;
			P_CHAR		orderGuard = 0;
			for( nearbyOrderGuards.rewind(); !nearbyOrderGuards.isEmpty(); nearbyOrderGuards++  )
			{
				pc_a_npc = nearbyOrderGuards.getChar();
				if( (!findGuardByName) || findGuardByName && !strcasecmp( guardName.c_str(), pc_a_npc->getCurrentNameC() ) )
					if( !orderGuard )
						orderGuard = pc_a_npc;
					else
						if( pc->distFrom( orderGuard ) > pc->distFrom( pc_a_npc ) )
							orderGuard = pc_a_npc;
			}
			if( orderGuard )
			{
				if( Guilds->GetType( pc->GetGuildNumber() ) == 2 || pc->IsGM() )
				{
					if( !pc->getAmount(0x1BC4) )
					{
						P_ITEM pi =  pc->GetItemOnLayer( 2 );
						if( ISVALIDPI( pi ) )
						{
							if( pi->getId() != 0x1BC4 )
								success = true;
						}
						else
							success = true;
					}
					if( success )
					{
						item::CreateFromScript( "$item_order_shield", pc->getBackpack() );
						orderGuard->talk( socket, TRANSLATE("Here's is your new shield."), 0);
					}
					else
						orderGuard->talk( socket, TRANSLATE("You already have an Order shield!"), 0);
				}
				else
					orderGuard->talk( socket,TRANSLATE("You're not an Order guild member!"), 0);
				success = true;
			}
		}
	}
	return success;
}

static LOGICAL requestHelp( P_CHAR pc, NXWSOCKET socket, std::string &speech, NxwCharWrapper &nearbyGuards )
{
	LOGICAL success = false;
	//if( region[pc->region].priv&0x01 && SrvParms->guardsactive || !TIMEOUT( pc->antiguardstimer ) )
	return success;
}

} // namespace Guards

static LOGICAL buyFromVendor( P_CHAR pc, NXWSOCKET socket, string &speech, NxwCharWrapper &nearbyVendors )
{
	LOGICAL success = false;
//	SI32 tokenPosition = INVALID;
	/*
	if(!speech.compare(0,10,"VENDOR BUY") || !speech.compare(0,14,"SHOPKEEPER BUY") )
	{
		success = true;
	}
	else
		if( tokenPosition = speech.find("BUY") != INVALID )
		{
		;
		}
	*/
	//
	// select nearest vendor
	//

	nearbyVendors.rewind();
	P_CHAR	pc_vendor = nearbyVendors.getChar(); //first
	P_CHAR	pc_a_npc;
	for( nearbyVendors.rewind(); !nearbyVendors.isEmpty(); nearbyVendors++ )
	{
		pc_a_npc = nearbyVendors.getChar();
		if( pc->distFrom( pc_vendor ) >  pc->distFrom( pc_a_npc ) )
			pc_vendor = pc_a_npc;
	}
	if( pc_vendor->npcaitype == NPCAI_PLAYERVENDOR )
	{
		pc_vendor->talk( socket, TRANSLATE("What would you like to buy?"), 0 );
		P_TARGET targ = clientInfo[socket]->newTarget( new cItemTarget() );
		targ->buffer[0]= pc_vendor->getSerial32();
		targ->send( getClientFromSocket( socket ) );
		success = true;
	}
	else
		if( BuyShop( socket, DEREF_P_CHAR( pc_vendor ) ) )
			success = true;
	return success;

}

}// namespace Speech


void makeGhost( wstring* from, wstring* to )
{
	to->erase();

	if ( from == NULL || to == NULL )
		return;

	wstring::iterator iter( from->begin() ), end( from->end() );
	for( ; iter != end; iter++ ) {
		/*if( (*iter)!=32 )
			(*to)+= ((*iter) %2)? L'O' : L'o';*/

		(*to) += ( (RandomNum(1,2) == 2) ? L'O' : L'o' );
	}

}


void talking( NXWSOCKET socket, string speech) // PC speech
{
	if (socket < 0 || socket >= now) //Luxor
		return;

	P_CHAR pc = MAKE_CHAR_REF( currchar[socket] );
	VALIDATEPC( pc );

	UI32 i, j;
	int match;
	char sect[512];

	char name[30] = {0,};	// it **IS** important to 0 out the remaining gaps
	strcpy(name, pc->getCurrentNameC());

	// len+font+color+type = same postion for non unicode and unicode speech packets
	// but 8 ... x DIFFER a lot for unicode and non unicode packets !!!

	if ( buffer[socket][3] == '\x09' && pc->CanBroadcast() )
	{
		broadcast(socket);
		return;
	}

	//
	// Pass speech to small
	//
	g_bByPass = false;

	std::string back( speech.c_str() ); //Luxor
		
	checkAmxSpeech( pc->getSerial32(), const_cast<char *>(back.c_str()) );
	//
	// Allow for bypass set in checkAmxSpeech.This way certain speech like passwords will not be echoed to the screen
	//
	if (g_bByPass)
		return;

	//<Luxor>
	
	if ( pc->amxevents[EVENT_CHR_ONSPEECH] ) {
		g_bByPass = false;
		strcpy( script2, speech.c_str() );
		pc->amxevents[EVENT_CHR_ONSPEECH]->Call( pc->getSerial32() );
		if( g_bByPass == true )
			return;
	}
	/*
        if ( pc->getAmxEvent( EVENT_CHR_ONSPEECH ) != NULL ) {
                strcpy( script2, speech.c_str() );
                pc->runAmxEvent( EVENT_CHR_ONSPEECH, pc->getSerial32() );

                if( g_bByPass == true )
                        return;
        }
	*/

	//</Luxor>

	//
	// Process speech which should not be send/echoed to others
	//
	std::string backk( speech.c_str() ); //Luxor
	if( magic::checkMagicalSpeech( pc, const_cast<char *>(backk.c_str()) ) ) //Luxor
		return;
	if( pricePlayerVendorItem( pc, socket, speech ) )
		return;

	if( describePlayerVendorItem( pc, socket, speech ) )
		return;

	if( renameRune( pc, socket, speech ) )
		return;

	if( renameSelf( pc, socket, speech ) )
		return;

	if( renameKey( pc, socket, speech ) )
		return;

	if( pageGameMaster( pc, socket, speech ) )
		return;

	if( pageCouncillor( pc, socket, speech ) )
		return;

	if ( pc->squelched )
	{
		pc->sysmsg(TRANSLATE("You have been squelched."));
		return;
	}

	if ( speech[0] == SrvParms->commandPrefix )
	{
		Commands::Command(socket, const_cast<char*>(speech.c_str()) );
		return;
	}

	/*
	Sparhawk	Entry and exit point for hidden GM to GM speech
	if ( pc->IsGM() && pc->hidden )
	{
		talkingGM2GM( s );
		return;
	}
	*/
	//
	// Unhide when talking
	//
	if (!pc->IsHiddenBySpell())
		pc->unHide();
	//
	// Collect all pcs in visual range
	//
	

	//
	// Echo speech to self and pcs in visual range
	//
	cPacketUnicodeSpeech talk;
	talk.obj=pc->getSerial32();
	talk.model=pc->getId();
	talk.type= buffer[socket][3];
	talk.color= DBYTE2WORD( buffer[socket][4], buffer[socket][5] );
	talk.font= DBYTE2WORD( buffer[socket][6], buffer[socket][7] );
	talk.name+=pc->getCurrentName();

	wstring speechUni;
	string2wstring( speech, speechUni );

	wstring* speechGhostUni=NULL;

	int range;
	switch ( buffer[socket][3] ) {
		case 0x09 : //yell
			range=(int)(VISRANGE*1.5);
			break;
		case 0x08 : //wisper
			range=2;
			break;
		default:
			range=VISRANGE;
	}

	NxwSocketWrapper sw;
	sw.fillOnline( pc, false, range );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWCLIENT ps=sw.getClient();
		if( ps==NULL )
			continue;
		P_CHAR a_pc= ps->currChar();
		if(!ISVALIDPC(a_pc))
			continue;

		if( a_pc->unicode )			// language
		{
			talk.language = LongFromCharPtr( buffer[socket]+8 );
		}
		else
		{
			talk.language = calcserial( 'E', 'N', 'U',  0 );
		}

		bool ghost;
		if( pc->dead && !a_pc->dead && !a_pc->IsGMorCounselor() && a_pc->spiritspeaktimer == 0 ) {
			if( speechGhostUni==NULL ) {
				speechGhostUni=new wstring();
				makeGhost( pc->getSpeechCurrent(), speechGhostUni );
			}
			//ndEndy not set speechGhostUni because want send true speech to event
			ghost=true;
		}
		else
			ghost=false;

		pc->setSpeechCurrent( &speechUni );

		if( a_pc->amxevents[EVENT_CHR_ONHEARPLAYER]!=NULL )
			a_pc->amxevents[EVENT_CHR_ONHEARPLAYER]->Call( a_pc->getSerial32(), pc->getSerial32(), ghost );
		//a_pc->runAmxEvent( EVENT_CHR_ONHEARPLAYER, a_pc->getSerial32(), pc->getSerial32(), ghost );

		bool modifiedInEvent = false;
		if( pc->getSpeechCurrent()==&speechUni ) { //so not was modified in event
			modifiedInEvent=false;
			if( ghost )
				pc->setSpeechCurrent( speechGhostUni );
		}
		else
			modifiedInEvent=true;


		talk.msg = pc->getSpeechCurrent();
		talk.send( a_pc->getClient() );

		if( modifiedInEvent )
			pc->deleteSpeechCurrent();
	}

	pc->resetSpeechCurrent();
	if( speechGhostUni!=NULL )
		safedelete(speechGhostUni);

	if ( buffer[socket][3] == 0 || buffer[socket][3] == 2) //speech type
	{
		pc->saycolor = ShortFromCharPtr(buffer[socket] +4);
	}
	//
	// Advanced speech logging by elcabesa
	//
	if (SrvParms->speech_log)
	{
		SpeechLogFile logfile(pc);
		logfile.Write("%s [%08x] [%i] said: %s\n", pc->getCurrentNameC(), pc->getSerial32(), pc->account, speech.c_str());

		int n= 0;
		string namelist= "to: ";

		
		NxwSocketWrapper sw;
		sw.fillOnline( pc );

		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			
			NXWCLIENT ps=sw.getClient();
			if(ps==NULL)
				continue;
			P_CHAR pc_new_char = ps->currChar();
			if( ISVALIDPC( pc_new_char ) )
			{
				namelist+= "[" + string( pc_new_char->getCurrentNameC() ) + "] ";
				++n;
			}
		}
		if( n==0 )
			namelist+= "himself or NPC";

		logfile.Write(namelist + "\n");
	}
	//
	// Make npcs, houses, boats etc not respond to dead people
	//
	if (pc->dead)
		return;
	//
	//	Capitalize speech for easy handling
	//
	j = speech.size();
	for( i = 0; i < j; ++i)
		speech[i] = toupper( speech[i] );
	//
	// Process speech for guildstones, boat tillermen, houses and npcs
	//
	if( resignFromGuild( pc, socket, speech ) )
		return;

	if( callGuards( pc, socket, speech ) )
		return;

	if( Boats->Speech( pc, socket, speech ) )
		return;

	if( house_speech( pc, socket, speech ) )
		return;
	//
	// Collect all npcs in visual range
	//
	if( nearbyNpcs != NULL )
		safedelete(nearbyNpcs);
		
	nearbyNpcs = new NxwCharWrapper;
	nearbyNpcs->fillCharsNearXYZ(pc->getPosition(), range, true, false );
	nearbyNpcs->rewind(); //Luxor: this is important! Otherwise isEmpty will return true!
	if( nearbyNpcs->isEmpty() ) {
		safedelete(nearbyNpcs);
		nearbyNpcs = NULL; //Luxor
		return;
	}
	//
	// Handle stablemaster commands
	//
	if( Speech::Stablemaster::respond( pc, socket, speech ) )
		return;
	//
	// Handle guard commands
	//
	if( Speech::Guard::respond( pc, socket, speech ) )
		return;
	//
	//
	//
	if( response( socket ) )
		return;
	//
	// Process npc triggers
	//

	P_CHAR pc_found = NULL;

	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( pc->getPosition(), 2, true, false );
	//pCharVector	sc( pointers::getCharFromWorldMap( pc->getPosition().x, pc->getPosition().y, VISRANGE, pointers::NPC ));
	//pCharVectorIt	scIt( sc.begin() ), scEnd( sc.end() );

	//for( ; scIt != scEnd; ++scIt )
	//{
	//P_CHAR pj = (*scIt);
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pj=sc.getChar();
		if(ISVALIDPC(pj)) {
			if ((pc->getSerial32() != pj->getSerial32()) && (pj->npc) )
			{
				pc_found = pj;
				break;
			}
		}
	}

	if ( ISVALIDPC(pc_found) &&(pc_found->speech) )
	{

		if(abs(pc_found->getPosition("z")-pc->getPosition("z")) >3 ) return;

		responsevendor(socket, DEREF_P_CHAR(pc_found));

		cScpIterator* iter = NULL;
		char script1[1024];
		char script2[1024];

		iter = Scripts::Speech->getNewIterator("SECTION SPEECH %i", pc_found->speech);
		if (iter==NULL) return;
		match = 0;
		strcpy(sect, "NO DEFAULT TEXT DEFINED");
		int loopexit2 = 0;
		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if (!(strcmp("DEFAULT", script1)))
				{
					strcpy(sect, script2);
				}
				if (!(strcmp("ON", script1)))
				{
					if (match != 1)
					{
						for(i=0; i < strlen(script2); i++)
							script2[i] = toupper( script2[i] );

						match = (speech.find( script2 ) != std::string::npos);
					}
				}
				if (!(strcmp("SAY", script1)))
				{
					if (match == 1)
					{
						pc_found->talk(socket, script2, 0);
						match = 2;
					}
				}

				if (!(strcmp("TRG", script1))) // Added by Magius(CHE) §
				{
					if (match == 1)
					{
						pc_found->trigger = str2num(script2);
						triggerNpc(socket, pc_found, TRIGTYPE_NPCWORD);
						strcpy(script1, "DUMMY");
						match = 2;
					}
				}

				if ( !strcmp("@CALL", script1) )	// Courtesy of Flamegod - Revelation emu
					AmxFunction::g_prgOverride->CallFn( AmxFunction::g_prgOverride->getFnOrdinal( script2 ), static_cast<int>(pc_found->getSerial32()), static_cast<int>(socket) );

			}
		}
		while ((script1[0] != '}')  && (++loopexit2 < MAXLOOPS) && (match != 2));
		if (match == 0)
			pc_found->talk(socket, sect, 0);
		safedelete(iter);
	}
}

static SI32 findKeyword( const std::string &str, const std::string &keyword )
{
	SI32 tokenPosition = str.find( keyword );
	if( tokenPosition != INVALID )
		if( (tokenPosition != 0) && str[tokenPosition-1] != ' ' )
			tokenPosition = INVALID;
		else
			if( (tokenPosition + keyword.length() < str.length() -1) && str[tokenPosition+keyword.length()] != ' ')
				tokenPosition = INVALID;
	return tokenPosition;
}

static std::string trimString( const std::string &str )
{
	UI32 i = 0;
	UI32 j = str.length();

	while( (i < j) && str[i] == ' ' )
		++i;
	--j;
	while( str[j] == ' ' )
		--j;
	return str.substr( i, (j-i)+1 );
}

/* wchar2char and char2wchar converts between ANSI char and Wide Chars
used by UO Client. Be aware, those functions returns their results in
temp[1024] global variable */
void wchar2char (const char* str)
{
	memset(&Unicode::temp[0], 0, 1024);
	bool end = false;
	for (int i = 0; !end && i<1022 ; i++)
	{
		if (str[i] == 0 && str[i+1] == 0) end = true; // bugfix LB ... was str[i-1] not so good for i=0
		Unicode::temp[i] = str[i*2];
	}
}

void char2wchar (const char* str)
{
	memset(&Unicode::temp[0], 0, 1024);
	UI32 size = strlen(str);
	// client wants to have a 0 as very fist byte.
	// after that 0 the unicode text
	// after it two(!) 0's as termintor
	UI32 j=1;
	for (UI32 i = 0; i < size; i++)
	{
		Unicode::temp[j] = str[i];
		j+=2;
	}
}
