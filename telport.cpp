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
\brief functions that handle the teleporting
\author Duke
*/
#include "nxwcommn.h"
#include "sndpkg.h"
#include "debug.h"

void teleporters(P_CHAR pc)
{
	VALIDATEPC(pc);

	Location charpos= pc->getPosition();

	std::multimap<int, tele_locations_st>::iterator iter_tele_locations( tele_locations.find(charpos.x) ),
							iter_tele_locations_end( tele_locations.end() );

	while  ((charpos.x==iter_tele_locations->second.origem.x) && (iter_tele_locations!=iter_tele_locations_end) )
	{
		if(charpos.y == iter_tele_locations->second.origem.y)
		{
			if((iter_tele_locations->second.origem.z == /*999*/127)||(charpos.z == iter_tele_locations->second.origem.z))
			{
				if ( pc->npc==0 )
					{
						// Look for an NPC Araknesh fix animals in teleporters with pg
						
						NxwCharWrapper sc;
						sc.fillCharsNearXYZ( pc->getPosition(), 4 );
						for( sc.rewind(); !sc.isEmpty(); sc++ ) {

							P_CHAR pc_i=sc.getChar();
							// That is following this player character
							if ( (pc_i->npc) && (pc_i->ftargserial==pc->getSerial32()) )
							{
								// Teleport the NPC along with the player
								pc_i->MoveTo(	iter_tele_locations->second.destination.x,
														iter_tele_locations->second.destination.y,
														static_cast<char>(iter_tele_locations->second.destination.z) );
								pc_i->teleport();
							}
						}
					}

				pc->MoveTo(	iter_tele_locations->second.destination.x,
									iter_tele_locations->second.destination.y,
									static_cast<char>(iter_tele_locations->second.destination.z) );
				pc->teleport();
				return;
			}
		}
		iter_tele_locations++;
	}

}

void read_in_teleport()
{
	char text[1024];
	char seps[]	= " ,\t\n";
	char *token;

    cScpIterator* iter = NULL;

	iter = Scripts::Regions->getNewIterator("SECTION TELEPORT");

	if(iter==NULL)
	{
		ErrOut("Teleport Data not found\n");
		error=1;
		keeprun=false;
		return;
	}

	do
	{
		strcpy(text, iter->getEntry()->getFullLine().c_str());

		if ((text[0]!=';')&&(text[0]!='/')&&(text[0]!='}')&&(text[0]!='{'))
		{
			tele_locations_st dummy;

			token = strtok( text, seps );

			dummy.origem.x = atoi(token);
			token = strtok( NULL, seps );
			dummy.origem.y = atoi(token);
			token = strtok( NULL, seps );
			if (token[0] == 'A')
				dummy.origem.z = 127/*999*/;
			else
				dummy.origem.z = atoi(token);

			token = strtok( NULL, seps );
			dummy.destination.x = atoi(token);
			token = strtok(NULL, seps );
			dummy.destination.y = atoi(token);
			token = strtok(NULL, seps);
			dummy.destination.z = atoi(token);

			tele_locations.insert(pair<int, tele_locations_st>(dummy.origem.x, dummy));

		}
	} while ((text[0]!='}'));

	safedelete(iter);

}

int validtelepos(P_CHAR pc)
{
	VALIDATEPCR(pc,INVALID);
	int z=INVALID;
	Location charpos= pc->getPosition();

	if ((charpos.x>=1397)&&(charpos.x<=1400)&&
		(charpos.y>=1622)&&(charpos.y<=1630))
		z=28;
	if ((charpos.x>=1510)&&(charpos.x<=1537)&&
		(charpos.y>=1455)&&(charpos.y<=1456))
		z=15;
	return z;

}

void advancementobjects(CHARACTER s, int x, int allways)
{
	P_CHAR pc = MAKE_CHAR_REF( s );
	VALIDATEPC( pc );
	char sect[512];
	int loopexit=0;
	char script1[1024], script2[1024];
	cScpIterator* iter = NULL;
	int i;

	P_ITEM packnum=NULL;
	P_ITEM hairobject=NULL, beardobject=NULL;

	if ((pc->advobj==0)||(allways==1))
	{
		staticeffect(s, 0x37, 0x3A, 0, 15);
		pc->playSFX( 0x01E9);
		pc->advobj=x;
		sprintf(sect, "SECTION ADVANCEMENT %i", x);
		iter = Scripts::Advance->getNewIterator(sect);
		if (iter==NULL)
		{
			ErrOut("advancement object: Script section not found. Aborting.\n");
			pc->advobj=0;
			return;
		}
		else
			do
			{
				iter->parseLine(script1, script2);
				if ((script1[0]!='}')&&(script1[0]!='{'))
				{
					if ((!(strcmp("STR",(char*)script1)))||(!(strcmp("STRENGTH",(char*)script1))))
					{
						pc->setStrength( getRangedValue((char*)script2) );
						pc->st2 = pc->getStrength();
					}
					if ((!(strcmp("DEX",(char*)script1)))||(!(strcmp("DEXTERITY",(char*)script1))))
					{
						pc->dx= getRangedValue((char*)script2);
						pc->dx2 = pc->dx;
					}
					if ((!(strcmp("INT",(char*)script1)))||(!(strcmp("INTELLIGENCE",(char*)script1))))
					{
						pc->in= getRangedValue((char*)script2);
						pc->in2 = pc->in;
					}
					if (script1[0]=='@') pc->loadEventFromScript(script1, script2);
					if ((!(strcmp("ALCHEMY",(char*)script1)))||(!(strcmp("SKILL0",(char*)script1)))) pc->baseskill[ALCHEMY] = getRangedValue((char*)script2);
					if ((!(strcmp("ANATOMY",(char*)script1)))||(!(strcmp("SKILL1",(char*)script1)))) pc->baseskill[ANATOMY] = getRangedValue((char*)script2);
					if ((!(strcmp("ANIMALLORE",(char*)script1)))||(!(strcmp("SKILL2",(char*)script1)))) pc->baseskill[ANIMALLORE] = getRangedValue((char*)script2);
					if ((!(strcmp("ITEMID",(char*)script1)))||(!(strcmp("SKILL3",(char*)script1)))) pc->baseskill[ITEMID] = getRangedValue((char*)script2);
					if ((!(strcmp("ARMSLORE",(char*)script1)))||(!(strcmp("SKILL4",(char*)script1)))) pc->baseskill[ARMSLORE] = getRangedValue((char*)script2);
					if ((!(strcmp("PARRYING",(char*)script1)))||(!(strcmp("SKILL5",(char*)script1)))) pc->baseskill[PARRYING] = getRangedValue((char*)script2);
					if ((!(strcmp("BEGGING",(char*)script1)))||(!(strcmp("SKILL6",(char*)script1)))) pc->baseskill[BEGGING] = getRangedValue((char*)script2);
					if ((!(strcmp("BLACKSMITHING",(char*)script1)))||(!(strcmp("SKILL7",(char*)script1)))) pc->baseskill[BLACKSMITHING] = getRangedValue((char*)script2);
					if ((!(strcmp("BOWCRAFT",(char*)script1)))||(!(strcmp("SKILL8",(char*)script1)))) pc->baseskill[BOWCRAFT] = getRangedValue((char*)script2);
					if ((!(strcmp("PEACEMAKING",(char*)script1)))||(!(strcmp("SKILL9",(char*)script1)))) pc->baseskill[PEACEMAKING] = getRangedValue((char*)script2);
					if ((!(strcmp("CAMPING",(char*)script1)))||(!(strcmp("SKILL10",(char*)script1)))) pc->baseskill[CAMPING] = getRangedValue((char*)script2);
					if ((!(strcmp("CARPENTRY",(char*)script1)))||(!(strcmp("SKILL11",(char*)script1)))) pc->baseskill[CARPENTRY] = getRangedValue((char*)script2);
					if ((!(strcmp("CARTOGRAPHY",(char*)script1)))||(!(strcmp("SKILL12",(char*)script1)))) pc->baseskill[CARTOGRAPHY] = getRangedValue((char*)script2);
					if ((!(strcmp("COOKING",(char*)script1)))||(!(strcmp("SKILL13",(char*)script1)))) pc->baseskill[COOKING] = getRangedValue((char*)script2);
					if ((!(strcmp("DETECTINGHIDDEN",(char*)script1)))||(!(strcmp("SKILL14",(char*)script1)))) pc->baseskill[DETECTINGHIDDEN] = getRangedValue((char*)script2);
					if ((!(strcmp("ENTICEMENT",(char*)script1)))||(!(strcmp("SKILL15",(char*)script1)))) pc->baseskill[ENTICEMENT] = getRangedValue((char*)script2);
					if ((!(strcmp("EVALUATINGINTEL",(char*)script1)))||(!(strcmp("SKILL16",(char*)script1)))) pc->baseskill[EVALUATINGINTEL] = getRangedValue((char*)script2);
					if ((!(strcmp("HEALING",(char*)script1)))||(!(strcmp("SKILL17",(char*)script1)))) pc->baseskill[HEALING] = getRangedValue((char*)script2);
					if ((!(strcmp("FISHING",(char*)script1)))||(!(strcmp("SKILL18",(char*)script1)))) pc->baseskill[FISHING] = getRangedValue((char*)script2);
					if ((!(strcmp("FORENSICS",(char*)script1)))||(!(strcmp("SKILL19",(char*)script1)))) pc->baseskill[FORENSICS] = getRangedValue((char*)script2);
					if ((!(strcmp("HERDING",(char*)script1)))||(!(strcmp("SKILL20",(char*)script1)))) pc->baseskill[HERDING] = getRangedValue((char*)script2);
					if ((!(strcmp("HIDING",(char*)script1)))||(!(strcmp("SKILL21",(char*)script1)))) pc->baseskill[HIDING] = getRangedValue((char*)script2);
					if ((!(strcmp("PROVOCATION",(char*)script1)))||(!(strcmp("SKILL22",(char*)script1)))) pc->baseskill[PROVOCATION] = getRangedValue((char*)script2);
					if ((!(strcmp("INSCRIPTION",(char*)script1)))||(!(strcmp("SKILL23",(char*)script1)))) pc->baseskill[INSCRIPTION] = getRangedValue((char*)script2);
					if ((!(strcmp("LOCKPICKING",(char*)script1)))||(!(strcmp("SKILL24",(char*)script1)))) pc->baseskill[LOCKPICKING] = getRangedValue((char*)script2);
					if ((!(strcmp("MAGERY",(char*)script1)))||(!(strcmp("SKILL25",(char*)script1)))) pc->baseskill[MAGERY] = getRangedValue((char*)script2);
					if ((!(strcmp("MAGICRESISTANCE",(char*)script1)))||(!(strcmp("RESIST",(char*)script1)))||(!(strcmp("SKILL26",(char*)script1)))) pc->baseskill[MAGICRESISTANCE] = getRangedValue((char*)script2);
					if ((!(strcmp("TACTICS",(char*)script1)))||(!(strcmp("SKILL27",(char*)script1)))) pc->baseskill[TACTICS] = getRangedValue((char*)script2);
					if ((!(strcmp("SNOOPING",(char*)script1)))||(!(strcmp("SKILL28",(char*)script1)))) pc->baseskill[SNOOPING] = getRangedValue((char*)script2);
					if ((!(strcmp("MUSICIANSHIP",(char*)script1)))||(!(strcmp("SKILL29",(char*)script1)))) pc->baseskill[MUSICIANSHIP] = getRangedValue((char*)script2);
					if ((!(strcmp("POISONING",(char*)script1)))||(!(strcmp("SKILL30",(char*)script1)))) pc->baseskill[POISONING] = getRangedValue((char*)script2);
					if ((!(strcmp("ARCHERY",(char*)script1)))||(!(strcmp("SKILL31",(char*)script1)))) pc->baseskill[ARCHERY] = getRangedValue((char*)script2);
					if ((!(strcmp("SPIRITSPEAK",(char*)script1)))||(!(strcmp("SKILL32",(char*)script1)))) pc->baseskill[SPIRITSPEAK] = getRangedValue((char*)script2);
					if ((!(strcmp("STEALING",(char*)script1)))||(!(strcmp("SKILL33",(char*)script1)))) pc->baseskill[STEALING] = getRangedValue((char*)script2);
					if ((!(strcmp("TAILORING",(char*)script1)))||(!(strcmp("SKILL34",(char*)script1)))) pc->baseskill[TAILORING] = getRangedValue((char*)script2);
					if ((!(strcmp("TAMING",(char*)script1)))||(!(strcmp("SKILL35",(char*)script1)))) pc->baseskill[TAMING] = getRangedValue((char*)script2);
					if ((!(strcmp("TASTEID",(char*)script1)))||(!(strcmp("SKILL36",(char*)script1)))) pc->baseskill[TASTEID] = getRangedValue((char*)script2);
					if ((!(strcmp("TINKERING",(char*)script1)))||(!(strcmp("SKILL37",(char*)script1)))) pc->baseskill[TINKERING] = getRangedValue((char*)script2);
					if ((!(strcmp("TRACKING",(char*)script1)))||(!(strcmp("SKILL38",(char*)script1)))) pc->baseskill[TRACKING] = getRangedValue((char*)script2);
					if ((!(strcmp("VETERINARY",(char*)script1)))||(!(strcmp("SKILL39",(char*)script1)))) pc->baseskill[VETERINARY] = getRangedValue((char*)script2);
					if ((!(strcmp("SWORDSMANSHIP",(char*)script1)))||(!(strcmp("SKILL40",(char*)script1)))) pc->baseskill[SWORDSMANSHIP] = getRangedValue((char*)script2);
					if ((!(strcmp("MACEFIGHTING",(char*)script1)))||(!(strcmp("SKILL41",(char*)script1))))pc->baseskill[MACEFIGHTING] = getRangedValue((char*)script2);
					if ((!(strcmp("FENCING",(char*)script1)))||(!(strcmp("SKILL42",(char*)script1)))) pc->baseskill[FENCING] = getRangedValue((char*)script2);
					if ((!(strcmp("WRESTLING",(char*)script1)))||(!(strcmp("SKILL43",(char*)script1)))) pc->baseskill[WRESTLING] = getRangedValue((char*)script2);
					if ((!(strcmp("LUMBERJACKING",(char*)script1)))||(!(strcmp("SKILL44",(char*)script1)))) pc->baseskill[LUMBERJACKING] = getRangedValue((char*)script2);
					if ((!(strcmp("MINING",(char*)script1)))||(!(strcmp("SKILL45",(char*)script1)))) pc->baseskill[MINING] = getRangedValue((char*)script2);

					// lb, new skills
					if ((!(strcmp("MEDITATION",(char*)script1)))||(!(strcmp("SKILL46",(char*)script1)))) pc->baseskill[MEDITATION] = getRangedValue((char*)script2);
					if ((!(strcmp("STEALTH",(char*)script1)))||(!(strcmp("SKILL47",(char*)script1)))) pc->baseskill[STEALTH] = getRangedValue((char*)script2);
					if ((!(strcmp("REMOVETRAPS",(char*)script1)))||(!(strcmp("SKILL48",(char*)script1)))) pc->baseskill[REMOVETRAPS] = getRangedValue((char*)script2);

					if ((!(strcmp("DYEHAIR",(char*)script1))))
					{
						int serial;
						UI32 ci;
						serial=pc->getSerial32();
						if (serial > INVALID)
							for (ci=0;ci<pointers::pContMap[serial].size();ci++)
							{
								P_ITEM pii_i=pointers::pContMap[serial][ci];
								if (ISVALIDPI(pii_i))
									if ((pii_i->layer==0x0B) && (pii_i->getContSerial()==serial))
									{
										hairobject=pii_i;
										break;
									}
							}
						P_ITEM phair= hairobject;
						if(ISVALIDPI(phair)) {

							x=hex2num(script2);
							WORD2DBYTE(x, phair->color1, phair->color2 );
							phair->Refresh();
							pc->teleport();
						}
					}

					if ((!(strcmp("DYEBEARD",(char*)script1))))
					{
						int serial;
						UI32 ci;
						serial=pc->getSerial32();
						if (serial > INVALID)
						for (ci=0;ci<pointers::pContMap[serial].size();ci++)
						{
							P_ITEM pii_i=pointers::pContMap[serial][ci];
							if (ISVALIDPI(pii_i))
								if ((pii_i->layer==0x10) && (pii_i->getContSerial()==serial))
								{
									beardobject=pii_i;
									break;
								}
						}
						if (true)
						{
							P_ITEM pbeard= beardobject;
							if(ISVALIDPI(pbeard)) {
								x=hex2num(script2);
								WORD2DBYTE(x, pbeard->color1, pbeard->color2);
								pbeard->Refresh();
								pc->teleport();
							}
						}
					}

					if (!(strcmp("KILLHAIR",(char*)script1)))
					{
						int serial;
						UI32 ci;
						serial=pc->getSerial32();
						if (serial > INVALID)
						for (ci=0;ci<pointers::pContMap[serial].size();ci++)
						{
							P_ITEM pii_i=pointers::pContMap[serial][ci];
							if (ISVALIDPI(pii_i))
								if ((pii_i->layer==0x0B) && (pii_i->getContSerial()==serial))
								{
									pii_i->deleteItem();
									break;
								}
						}
					}

					if (!(strcmp("KILLBEARD",(char*)script1)))
					{
						int serial;
						UI32 ci;
						serial=pc->getSerial32();
						if (serial > INVALID)
						for (ci=0;ci<pointers::pContMap[serial].size();ci++)
						{
							P_ITEM pii_i=pointers::pContMap[serial][ci];
							if (ISVALIDPI(pii_i))
								if ((pii_i->layer==0x10) && (pii_i->getContSerial()==serial))
								{
									pii_i->deleteItem();
									break;
								}
						}
					}

					if (!(strcmp("KILLPACK",(char*)script1)))
					{
						int serial;
						UI32 ci;
						serial=pc->getSerial32();
						if (serial > INVALID)
						for (ci=0;ci<pointers::pContMap[serial].size();ci++)
						{
							P_ITEM pii_i=pointers::pContMap[serial][ci];
							if (ISVALIDPI(pii_i))
								if ((pii_i->layer==0x15) && (pii_i->getContSerial()==serial))
								{
									pii_i->deleteItem();
									break;
								}
						}
					}
					if (!(strcmp("ITEM",(char*)script1)))
					{
						x=str2num(script2);
						P_ITEM pi= item::CreateScriptItem(INVALID, x, 0);
						packnum= pc->getBackpack();

						if (ISVALIDPI(pi))
						{
							pi->setPosition( 50+(rand()%80), 50+(rand()%80), 9);
							if(pi->layer==0x0b || pi->layer==0x10)
							{
								//setserial(DEREF_P_ITEM(pi),s,4);
								pi->setContSerial(pc->getSerial32());
							}
							else
							{
								if(ISVALIDPI(packnum)) 
									//setserial(DEREF_P_ITEM(pi),DEREF_P_ITEM(packnum),1);
									pi->setContSerial(packnum->getSerial32());
							}
							pi->Refresh();//AntiChrist
							pc->teleport();
						}
					}

					if (!(strcmp((char*)script1,"SKIN")))
					{
						i=hex2num(script2);
						pc->skin1=i>>8;
						pc->skin2=i%256;
						pc->xskin1=i>>8;
						pc->xskin2=i%256;
						pc->teleport();
					}

					if (!(strcmp("POLY",(char*)script1)))
					{
						x=hex2num(script2);
						pc->id1=x>>8;
						pc->xid1=x>>8;
						pc->id2=x%256;
						pc->xid2=x%256;
						pc->teleport();
					}

					if (!(strcmp("ADVOBJ",(char*)script1)))
					{
						x=str2num(script2);
						pc->advobj=x;
					}
				}
			}
			while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
			safedelete(iter);
	}
	else sysmessage(calcSocketFromChar(s),TRANSLATE("You have already used an advancement object with this character."));

}


//
//
// Aldur
//////////////////////////////////

void objTeleporters(P_CHAR pc)
{
	VALIDATEPC(pc);

	Location charpos= pc->getPosition();
	

	NxwItemWrapper si;
	si.fillItemsNearXYZ( charpos, VISRANGE, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {
		P_ITEM pmi=si.getItem();
		if(!ISVALIDPI(pmi))
			continue;

		if (((UI32)pmi->getPosition("x") == charpos.x) && ((UI32)pmi->getPosition("y") == charpos.y) &&
			((abs(pmi->getPosition("z")) + 10) >= abs(charpos.z)) &&((abs(pmi->getPosition("z")) - 10) <= abs(charpos.z)))
			{
				if ((pmi->type == 60) && (pmi->morex + pmi->morey + pmi->morez >0))
				{
					pc->MoveTo( pmi->morex,pmi->morey,pmi->morez );
					pc->teleport();
				}

				// advancement objects
				if ((pmi->type == 80) && !pc->npc)
				if (pmi->more1 != 0 || pmi->more2 != 0 || pmi->more3 != 0 || pmi->more4 != 0)
				{
					if (pc->getSerial().ser1 == pmi->more1 && pc->getSerial().ser2 == pmi->more2 && pc->getSerial().ser3 == pmi->more3 && pc->getSerial().ser4 == pmi->more4)
						advancementobjects(DEREF_P_CHAR(pc), pmi->morex, 0);
				}
				else
					advancementobjects(DEREF_P_CHAR(pc), pmi->morex, 0);

				if ((pmi->type == 81)&&!(pc->npc))
					if (pmi->more1 != 0 || pmi->more2 != 0 || pmi->more3 != 0 || pmi->more4 != 0)
					{
						if (pc->getSerial().ser1 == pmi->more1 && pc->getSerial().ser2 == pmi->more2 && pc->getSerial().ser3 == pmi->more3 && pc->getSerial().ser4 == pmi->more4)
						advancementobjects(DEREF_P_CHAR(pc), pmi->morex, 1);
					}
					else
						advancementobjects(DEREF_P_CHAR(pc), pmi->morex, 1);
				
				// The above code lets you restrict a gate's use by setting its MORE values to a char's
				// serial #

				// damage objects
				if (!(pc->IsInvul()) && (pmi->type == 85))
				{
					pc->hp = pc->hp - (pmi->morex + RandomNum(pmi->morey, pmi->morez));

					if (pc->hp < 1)
						pc->Kill();
				}
				
				
				//
				//
				// Aldur
				//////////////////////////////////

				// sound objects
				if (pmi->type == 86)
				{
					if ((UI32)RandomNum(1, 100) <= pmi->morez)
						soundeffect3(pmi, (pmi->morex << 8) + pmi->morey);
				}
			}
	}

}

