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
#include "layer.h"

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
					if ((!(strcmp("STR",script1)))||(!(strcmp("STRENGTH",script1))))
					{
						pc->setStrength( getRangedValue(script2) );
						pc->st2 = pc->getStrength();
					}
					if ((!(strcmp("DEX",script1)))||(!(strcmp("DEXTERITY",script1))))
					{
						pc->dx= getRangedValue(script2);
						pc->dx2 = pc->dx;
					}
					if ((!(strcmp("INT",script1)))||(!(strcmp("INTELLIGENCE",script1))))
					{
						pc->in= getRangedValue(script2);
						pc->in2 = pc->in;
					}
					if (script1[0]=='@') pc->loadEventFromScript(script1, script2);
					if ((!(strcmp("ALCHEMY",script1)))||(!(strcmp("SKILL0",script1)))) pc->baseskill[ALCHEMY] = getRangedValue(script2);
					if ((!(strcmp("ANATOMY",script1)))||(!(strcmp("SKILL1",script1)))) pc->baseskill[ANATOMY] = getRangedValue(script2);
					if ((!(strcmp("ANIMALLORE",script1)))||(!(strcmp("SKILL2",script1)))) pc->baseskill[ANIMALLORE] = getRangedValue(script2);
					if ((!(strcmp("ITEMID",script1)))||(!(strcmp("SKILL3",script1)))) pc->baseskill[ITEMID] = getRangedValue(script2);
					if ((!(strcmp("ARMSLORE",script1)))||(!(strcmp("SKILL4",script1)))) pc->baseskill[ARMSLORE] = getRangedValue(script2);
					if ((!(strcmp("PARRYING",script1)))||(!(strcmp("SKILL5",script1)))) pc->baseskill[PARRYING] = getRangedValue(script2);
					if ((!(strcmp("BEGGING",script1)))||(!(strcmp("SKILL6",script1)))) pc->baseskill[BEGGING] = getRangedValue(script2);
					if ((!(strcmp("BLACKSMITHING",script1)))||(!(strcmp("SKILL7",script1)))) pc->baseskill[BLACKSMITHING] = getRangedValue(script2);
					if ((!(strcmp("BOWCRAFT",script1)))||(!(strcmp("SKILL8",script1)))) pc->baseskill[BOWCRAFT] = getRangedValue(script2);
					if ((!(strcmp("PEACEMAKING",script1)))||(!(strcmp("SKILL9",script1)))) pc->baseskill[PEACEMAKING] = getRangedValue(script2);
					if ((!(strcmp("CAMPING",script1)))||(!(strcmp("SKILL10",script1)))) pc->baseskill[CAMPING] = getRangedValue(script2);
					if ((!(strcmp("CARPENTRY",script1)))||(!(strcmp("SKILL11",script1)))) pc->baseskill[CARPENTRY] = getRangedValue(script2);
					if ((!(strcmp("CARTOGRAPHY",script1)))||(!(strcmp("SKILL12",script1)))) pc->baseskill[CARTOGRAPHY] = getRangedValue(script2);
					if ((!(strcmp("COOKING",script1)))||(!(strcmp("SKILL13",script1)))) pc->baseskill[COOKING] = getRangedValue(script2);
					if ((!(strcmp("DETECTINGHIDDEN",script1)))||(!(strcmp("SKILL14",script1)))) pc->baseskill[DETECTINGHIDDEN] = getRangedValue(script2);
					if ((!(strcmp("ENTICEMENT",script1)))||(!(strcmp("SKILL15",script1)))) pc->baseskill[ENTICEMENT] = getRangedValue(script2);
					if ((!(strcmp("EVALUATINGINTEL",script1)))||(!(strcmp("SKILL16",script1)))) pc->baseskill[EVALUATINGINTEL] = getRangedValue(script2);
					if ((!(strcmp("HEALING",script1)))||(!(strcmp("SKILL17",script1)))) pc->baseskill[HEALING] = getRangedValue(script2);
					if ((!(strcmp("FISHING",script1)))||(!(strcmp("SKILL18",script1)))) pc->baseskill[FISHING] = getRangedValue(script2);
					if ((!(strcmp("FORENSICS",script1)))||(!(strcmp("SKILL19",script1)))) pc->baseskill[FORENSICS] = getRangedValue(script2);
					if ((!(strcmp("HERDING",script1)))||(!(strcmp("SKILL20",script1)))) pc->baseskill[HERDING] = getRangedValue(script2);
					if ((!(strcmp("HIDING",script1)))||(!(strcmp("SKILL21",script1)))) pc->baseskill[HIDING] = getRangedValue(script2);
					if ((!(strcmp("PROVOCATION",script1)))||(!(strcmp("SKILL22",script1)))) pc->baseskill[PROVOCATION] = getRangedValue(script2);
					if ((!(strcmp("INSCRIPTION",script1)))||(!(strcmp("SKILL23",script1)))) pc->baseskill[INSCRIPTION] = getRangedValue(script2);
					if ((!(strcmp("LOCKPICKING",script1)))||(!(strcmp("SKILL24",script1)))) pc->baseskill[LOCKPICKING] = getRangedValue(script2);
					if ((!(strcmp("MAGERY",script1)))||(!(strcmp("SKILL25",script1)))) pc->baseskill[MAGERY] = getRangedValue(script2);
					if ((!(strcmp("MAGICRESISTANCE",script1)))||(!(strcmp("RESIST",script1)))||(!(strcmp("SKILL26",script1)))) pc->baseskill[MAGICRESISTANCE] = getRangedValue(script2);
					if ((!(strcmp("TACTICS",script1)))||(!(strcmp("SKILL27",script1)))) pc->baseskill[TACTICS] = getRangedValue(script2);
					if ((!(strcmp("SNOOPING",script1)))||(!(strcmp("SKILL28",script1)))) pc->baseskill[SNOOPING] = getRangedValue(script2);
					if ((!(strcmp("MUSICIANSHIP",script1)))||(!(strcmp("SKILL29",script1)))) pc->baseskill[MUSICIANSHIP] = getRangedValue(script2);
					if ((!(strcmp("POISONING",script1)))||(!(strcmp("SKILL30",script1)))) pc->baseskill[POISONING] = getRangedValue(script2);
					if ((!(strcmp("ARCHERY",script1)))||(!(strcmp("SKILL31",script1)))) pc->baseskill[ARCHERY] = getRangedValue(script2);
					if ((!(strcmp("SPIRITSPEAK",script1)))||(!(strcmp("SKILL32",script1)))) pc->baseskill[SPIRITSPEAK] = getRangedValue(script2);
					if ((!(strcmp("STEALING",script1)))||(!(strcmp("SKILL33",script1)))) pc->baseskill[STEALING] = getRangedValue(script2);
					if ((!(strcmp("TAILORING",script1)))||(!(strcmp("SKILL34",script1)))) pc->baseskill[TAILORING] = getRangedValue(script2);
					if ((!(strcmp("TAMING",script1)))||(!(strcmp("SKILL35",script1)))) pc->baseskill[TAMING] = getRangedValue(script2);
					if ((!(strcmp("TASTEID",script1)))||(!(strcmp("SKILL36",script1)))) pc->baseskill[TASTEID] = getRangedValue(script2);
					if ((!(strcmp("TINKERING",script1)))||(!(strcmp("SKILL37",script1)))) pc->baseskill[TINKERING] = getRangedValue(script2);
					if ((!(strcmp("TRACKING",script1)))||(!(strcmp("SKILL38",script1)))) pc->baseskill[TRACKING] = getRangedValue(script2);
					if ((!(strcmp("VETERINARY",script1)))||(!(strcmp("SKILL39",script1)))) pc->baseskill[VETERINARY] = getRangedValue(script2);
					if ((!(strcmp("SWORDSMANSHIP",script1)))||(!(strcmp("SKILL40",script1)))) pc->baseskill[SWORDSMANSHIP] = getRangedValue(script2);
					if ((!(strcmp("MACEFIGHTING",script1)))||(!(strcmp("SKILL41",script1))))pc->baseskill[MACEFIGHTING] = getRangedValue(script2);
					if ((!(strcmp("FENCING",script1)))||(!(strcmp("SKILL42",script1)))) pc->baseskill[FENCING] = getRangedValue(script2);
					if ((!(strcmp("WRESTLING",script1)))||(!(strcmp("SKILL43",script1)))) pc->baseskill[WRESTLING] = getRangedValue(script2);
					if ((!(strcmp("LUMBERJACKING",script1)))||(!(strcmp("SKILL44",script1)))) pc->baseskill[LUMBERJACKING] = getRangedValue(script2);
					if ((!(strcmp("MINING",script1)))||(!(strcmp("SKILL45",script1)))) pc->baseskill[MINING] = getRangedValue(script2);

					// lb, new skills
					if ((!(strcmp("MEDITATION",script1)))||(!(strcmp("SKILL46",script1)))) pc->baseskill[MEDITATION] = getRangedValue(script2);
					if ((!(strcmp("STEALTH",script1)))||(!(strcmp("SKILL47",script1)))) pc->baseskill[STEALTH] = getRangedValue(script2);
					if ((!(strcmp("REMOVETRAPS",script1)))||(!(strcmp("SKILL48",script1)))) pc->baseskill[REMOVETRAPS] = getRangedValue(script2);

					if ((!(strcmp("DYEHAIR",script1))))
					{
						int serial;
						UI32 ci;
						serial=pc->getSerial32();
						if (serial > INVALID)
							for (ci=0;ci<pointers::pContMap[serial].size();ci++)
							{
								P_ITEM pii_i=pointers::pContMap[serial][ci];
								if (ISVALIDPI(pii_i))
									if ((pii_i->layer==LAYER_HAIR) && (pii_i->getContSerial()==serial))
									{
										hairobject=pii_i;
										break;
									}
							}
						P_ITEM phair= hairobject;
						if(ISVALIDPI(phair)) {

							phair->setColor( hex2num(script2) );
							phair->Refresh();
							pc->teleport();
						}
					}

					if ((!(strcmp("DYEBEARD",script1))))
					{
						int serial;
						UI32 ci;
						serial=pc->getSerial32();
						if (serial > INVALID)
						for (ci=0;ci<pointers::pContMap[serial].size();ci++)
						{
							P_ITEM pii_i=pointers::pContMap[serial][ci];
							if (ISVALIDPI(pii_i))
								if ((pii_i->layer==LAYER_BEARD) && (pii_i->getContSerial()==serial))
								{
									beardobject=pii_i;
									break;
								}
						}
						if (true)
						{
							P_ITEM pbeard= beardobject;
							if(ISVALIDPI(pbeard)) {
								pbeard->setColor( hex2num(script2) );
								pbeard->Refresh();
								pc->teleport();
							}
						}
					}

					if (!(strcmp("KILLHAIR",script1)))
					{
						int serial;
						UI32 ci;
						serial=pc->getSerial32();
						if (serial > INVALID)
						for (ci=0;ci<pointers::pContMap[serial].size();ci++)
						{
							P_ITEM pii_i=pointers::pContMap[serial][ci];
							if (ISVALIDPI(pii_i))
								if ((pii_i->layer==LAYER_HAIR) && (pii_i->getContSerial()==serial))
								{
									pii_i->deleteItem();
									break;
								}
						}
					}

					if (!(strcmp("KILLBEARD",script1)))
					{
						int serial;
						UI32 ci;
						serial=pc->getSerial32();
						if (serial > INVALID)
						for (ci=0;ci<pointers::pContMap[serial].size();ci++)
						{
							P_ITEM pii_i=pointers::pContMap[serial][ci];
							if (ISVALIDPI(pii_i))
								if ((pii_i->layer==LAYER_BEARD) && (pii_i->getContSerial()==serial))
								{
									pii_i->deleteItem();
									break;
								}
						}
					}

					if (!(strcmp("KILLPACK",script1)))
					{
						int serial;
						UI32 ci;
						serial=pc->getSerial32();
						if (serial > INVALID)
						for (ci=0;ci<pointers::pContMap[serial].size();ci++)
						{
							P_ITEM pii_i=pointers::pContMap[serial][ci];
							if (ISVALIDPI(pii_i))
								if ((pii_i->layer==LAYER_BACKPACK) && (pii_i->getContSerial()==serial))
								{
									pii_i->deleteItem();
									break;
								}
						}
					}
					if (!(strcmp("ITEM",script1)))
					{
						x=str2num(script2);
						P_ITEM pi= item::CreateScriptItem(INVALID, x, 0);
						packnum= pc->getBackpack();

						if (ISVALIDPI(pi))
						{
							pi->setPosition( 50+(rand()%80), 50+(rand()%80), 9);
							if(pi->layer==LAYER_HAIR || pi->layer==LAYER_BEARD)
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

					if (!(strcmp(script1,"SKIN")))
					{
						i=hex2num(script2);
						pc->setSkinColor(i);
						pc->setOldSkinColor(i);
						pc->teleport();
					}

					if (!(strcmp("POLY",script1)))
					{
						x=hex2num(script2);
						pc->SetBodyType(x);
						pc->SetOldBodyType(x);
						pc->teleport();
					}

					if (!(strcmp("ADVOBJ",script1)))
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

