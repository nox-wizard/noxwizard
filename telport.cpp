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
#include "scp_parser.h"
#include "set.h"
#include "chars.h"
#include "items.h"
#include "inlines.h"
#include "basics.h"
#include "scripts.h"


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
	std::string script1, script2;
	cScpIterator* iter = NULL;
	int i;

	P_ITEM packnum=NULL;
	P_ITEM hairobject=NULL, beardobject=NULL;

	if ((pc->advobj==0)||(allways==1))
	{
		staticeffect(DEREF_P_CHAR(pc), 0x37, 0x3A, 0, 15);
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
					if	(script1[0]=='@') pc->loadEventFromScript( const_cast< char* >(script1.c_str()), const_cast< char* >(script2.c_str()));
					else if ("STR" == script1 || "STRENGTH" == script1 )
					{
						pc->setStrength( getRangedValue(script2) );
						pc->st2 = pc->getStrength();
					}
					else if ("DEX" == script1 ||"DEXTERITY" == script1 )
					{
						pc->dx= getRangedValue(script2);
						pc->dx2 = pc->dx;
					}
					else if ("INT" == script1 ||"INTELLIGENCE" == script1 )
					{
						pc->in= getRangedValue(script2);
						pc->in2 = pc->in;
					}
					else if ("ALCHEMY" == script1 ||"SKILL0" == script1 ) pc->baseskill[ALCHEMY] = getRangedValue(script2);
					else if ("ANATOMY" == script1 ||"SKILL1" == script1 ) pc->baseskill[ANATOMY] = getRangedValue(script2);
					else if ("ANIMALLORE" == script1 ||"SKILL2" == script1 ) pc->baseskill[ANIMALLORE] = getRangedValue(script2);
					else if ("ITEMID" == script1 ||"SKILL3" == script1 ) pc->baseskill[ITEMID] = getRangedValue(script2);
					else if ("ARMSLORE" == script1 ||"SKILL4" == script1 ) pc->baseskill[ARMSLORE] = getRangedValue(script2);
					else if ("PARRYING" == script1 ||"SKILL5" == script1 ) pc->baseskill[PARRYING] = getRangedValue(script2);
					else if ("BEGGING" == script1 ||"SKILL6" == script1 ) pc->baseskill[BEGGING] = getRangedValue(script2);
					else if ("BLACKSMITHING" == script1 ||"SKILL7" == script1 ) pc->baseskill[BLACKSMITHING] = getRangedValue(script2);
					else if ("BOWCRAFT" == script1 ||"SKILL8" == script1 ) pc->baseskill[BOWCRAFT] = getRangedValue(script2);
					else if ("PEACEMAKING" == script1 ||"SKILL9" == script1 ) pc->baseskill[PEACEMAKING] = getRangedValue(script2);
					else if ("CAMPING" == script1 ||"SKILL10" == script1 ) pc->baseskill[CAMPING] = getRangedValue(script2);
					else if ("CARPENTRY" == script1 ||"SKILL11" == script1 ) pc->baseskill[CARPENTRY] = getRangedValue(script2);
					else if ("CARTOGRAPHY" == script1 ||"SKILL12" == script1 ) pc->baseskill[CARTOGRAPHY] = getRangedValue(script2);
					else if ("COOKING" == script1 ||"SKILL13" == script1 ) pc->baseskill[COOKING] = getRangedValue(script2);
					else if ("DETECTINGHIDDEN" == script1 ||"SKILL14" == script1 ) pc->baseskill[DETECTINGHIDDEN] = getRangedValue(script2);
					else if ("ENTICEMENT" == script1 ||"SKILL15" == script1 ) pc->baseskill[ENTICEMENT] = getRangedValue(script2);
					else if ("EVALUATINGINTEL" == script1 ||"SKILL16" == script1 ) pc->baseskill[EVALUATINGINTEL] = getRangedValue(script2);
					else if ("HEALING" == script1 ||"SKILL17" == script1 ) pc->baseskill[HEALING] = getRangedValue(script2);
					else if ("FISHING" == script1 ||"SKILL18" == script1 ) pc->baseskill[FISHING] = getRangedValue(script2);
					else if ("FORENSICS" == script1 ||"SKILL19" == script1 ) pc->baseskill[FORENSICS] = getRangedValue(script2);
					else if ("HERDING" == script1 ||"SKILL20" == script1 ) pc->baseskill[HERDING] = getRangedValue(script2);
					else if ("HIDING" == script1 ||"SKILL21" == script1 ) pc->baseskill[HIDING] = getRangedValue(script2);
					else if ("PROVOCATION" == script1 ||"SKILL22" == script1 ) pc->baseskill[PROVOCATION] = getRangedValue(script2);
					else if ("INSCRIPTION" == script1 ||"SKILL23" == script1 ) pc->baseskill[INSCRIPTION] = getRangedValue(script2);
					else if ("LOCKPICKING" == script1 ||"SKILL24" == script1 ) pc->baseskill[LOCKPICKING] = getRangedValue(script2);
					else if ("MAGERY" == script1 ||"SKILL25" == script1 ) pc->baseskill[MAGERY] = getRangedValue(script2);
					else if ("MAGICRESISTANCE" == script1 ||"RESIST" == script1 ||"SKILL26" == script1 ) pc->baseskill[MAGICRESISTANCE] = getRangedValue(script2);
					else if ("TACTICS" == script1 ||"SKILL27" == script1 ) pc->baseskill[TACTICS] = getRangedValue(script2);
					else if ("SNOOPING" == script1 ||"SKILL28" == script1 ) pc->baseskill[SNOOPING] = getRangedValue(script2);
					else if ("MUSICIANSHIP" == script1 ||"SKILL29" == script1 ) pc->baseskill[MUSICIANSHIP] = getRangedValue(script2);
					else if ("POISONING" == script1 ||"SKILL30" == script1 ) pc->baseskill[POISONING] = getRangedValue(script2);
					else if ("ARCHERY" == script1 ||"SKILL31" == script1 ) pc->baseskill[ARCHERY] = getRangedValue(script2);
					else if ("SPIRITSPEAK" == script1 ||"SKILL32" == script1 ) pc->baseskill[SPIRITSPEAK] = getRangedValue(script2);
					else if ("STEALING" == script1 ||"SKILL33" == script1 ) pc->baseskill[STEALING] = getRangedValue(script2);
					else if ("TAILORING" == script1 ||"SKILL34" == script1 ) pc->baseskill[TAILORING] = getRangedValue(script2);
					else if ("TAMING" == script1 ||"SKILL35" == script1 ) pc->baseskill[TAMING] = getRangedValue(script2);
					else if ("TASTEID" == script1 ||"SKILL36" == script1 ) pc->baseskill[TASTEID] = getRangedValue(script2);
					else if ("TINKERING" == script1 ||"SKILL37" == script1 ) pc->baseskill[TINKERING] = getRangedValue(script2);
					else if ("TRACKING" == script1 ||"SKILL38" == script1 ) pc->baseskill[TRACKING] = getRangedValue(script2);
					else if ("VETERINARY" == script1 ||"SKILL39" == script1 ) pc->baseskill[VETERINARY] = getRangedValue(script2);
					else if ("SWORDSMANSHIP" == script1 ||"SKILL40" == script1 ) pc->baseskill[SWORDSMANSHIP] = getRangedValue(script2);
					else if ("MACEFIGHTING" == script1 ||"SKILL41" == script1 )pc->baseskill[MACEFIGHTING] = getRangedValue(script2);
					else if ("FENCING" == script1 ||"SKILL42" == script1 ) pc->baseskill[FENCING] = getRangedValue(script2);
					else if ("WRESTLING" == script1 ||"SKILL43" == script1 ) pc->baseskill[WRESTLING] = getRangedValue(script2);
					else if ("LUMBERJACKING" == script1 ||"SKILL44" == script1 ) pc->baseskill[LUMBERJACKING] = getRangedValue(script2);
					else if ("MINING" == script1 ||"SKILL45" == script1 ) pc->baseskill[MINING] = getRangedValue(script2);
					else if ("MEDITATION" == script1 ||"SKILL46" == script1 ) pc->baseskill[MEDITATION] = getRangedValue(script2);
					else if ("STEALTH" == script1 ||"SKILL47" == script1 ) pc->baseskill[STEALTH] = getRangedValue(script2);
					else if ("REMOVETRAPS" == script1 ||"SKILL48" == script1 ) pc->baseskill[REMOVETRAPS] = getRangedValue(script2);
					else if ("DYEHAIR" == script1 )
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
					else if ("DYEBEARD" == script1 )
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
					else if( "KILLHAIR" == script1 )
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
									pii_i->Delete();
									break;
								}
						}
					}
					else if ( "KILLBEARD" == script1 )
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
									pii_i->Delete();
									break;
								}
						}
					}
					else if ( "KILLPACK" == script1 )
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
									pii_i->Delete();
									break;
								}
						}
					}
					else if ( "ITEM" == script1 )
					{
						char itmnum[1024], amount[1024];
						splitLine( const_cast< char* >(script2.c_str()), itmnum, amount );
						int amt = str2num( amount );
						if( amt == 0 )
							amt=INVALID;
						P_ITEM pi= item::CreateFromScript( itmnum, NULL, amt );
						packnum= pc->getBackpack();

						if (ISVALIDPI(pi))
						{
							pi->setPosition( 50+(rand()%80), 50+(rand()%80), 9);
							if(pi->layer==LAYER_HAIR || pi->layer==LAYER_BEARD)
							{
								pi->setContSerial(pc->getSerial32());
							}
							else
							{
								if(ISVALIDPI(packnum)) 
									packnum->AddItem( pi );
							}
							pi->Refresh();//AntiChrist
							pc->teleport();
						}
					}
					else if ( script1 == "SKIN" )
					{
						i=hex2num(script2);
						pc->setColor(i);
						pc->setOldColor(i);
						pc->teleport();
					}
					else if ( "POLY" == script1 )
					{
						x=hex2num(script2);
						pc->setId(x);
						pc->setOldId(x);
						pc->teleport();
					}
					else if ( "ADVOBJ" == script1 )
					{
						pc->advobj = str2num(script2);
					}
				}
			}
			while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
			safedelete(iter);
	}
	else pc->sysmsg(TRANSLATE("You have already used an advancement object with this character."));
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
	si.fillItemsAtXY( charpos );
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

void teleporters(P_CHAR pc)
{
	VALIDATEPC(pc);

	Location charpos= pc->getPosition();

	std::multimap<int, tele_locations_st>::iterator iter_tele_locations( tele_locations.find(charpos.x) ),
							iter_tele_locations_end( tele_locations.end() );

	while  ( charpos.x == iter_tele_locations->second.origem.x && iter_tele_locations!=iter_tele_locations_end )
	{
		if( charpos.y == iter_tele_locations->second.origem.y )
		{
			if( iter_tele_locations->second.origem.z == 127 || abs( charpos.z - iter_tele_locations->second.origem.z ) <= 30 )
			{
				if ( !pc->npc )
				{
					// Look for an NPC Araknesh fix animals in teleporters with pg

					NxwCharWrapper sc;
					sc.fillCharsNearXYZ( pc->getPosition(), 4 );
					for( sc.rewind(); !sc.isEmpty(); sc++ )
					{
						P_CHAR pc_i=sc.getChar();
						// That is following this player character
						if ( (pc_i->npc) && (pc_i->ftargserial==pc->getSerial32()) )
						{
							// Teleport the NPC along with the player
							pc_i->MoveTo(	iter_tele_locations->second.destination.x,
									iter_tele_locations->second.destination.y,
									iter_tele_locations->second.destination.z );
							pc_i->teleport();
						}
					}
				}

				pc->MoveTo(	iter_tele_locations->second.destination.x,
						iter_tele_locations->second.destination.y,
						iter_tele_locations->second.destination.z );
				pc->teleport();
				return;
			}
		}
		++iter_tele_locations;
	}

}
