  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include <stdlib.h>

#include "nxwcommn.h"
#include "network.h"
#include "sndpkg.h"

#include "race.h"
#include "racescriptentry.h"
#include "raceskill.h"
#include "racetokenizer.h"

using namespace std;

//
//	STATIC DATA
//

bool	Race::activeRaceSystem	= false;
string*	Race::globalWebRoot	= new string( "" );
string* Race::globalWebLink	= new string( "" );
short	Race::startLocation[3]	= { 0, 0, 0 };
bool	Race::teleportOnEnlist	= false;
bool	Race::withWebInterface	= false;
std::map<UI32, class Race*> Race::raceMap;
cScpScript* Race::script ;

//
//	STATIC FUNCTIONS
//

Race* Race::getRace( UI32 raceId )
{
	std::map<UI32, class Race*>::iterator raceIter = raceMap.find( raceId );
	if ( raceIter != raceMap.end() )
		return raceIter->second;
	else
		return NULL;
}


RequiredType Race::getBeardPerm( UI32 raceId )
{
	return raceMap[raceId]->beardPerm;
}

RequiredType Race::getHairPerm( UI32 raceId )
{
	return raceMap[raceId]->hairPerm;
}

bool Race::isRaceSystemActive( void )
{
	return activeRaceSystem;
}

bool Race::isPlayerRace( const UI32 raceId )
{
	return raceMap[raceId]->isPlayerRace();
}

void Race::load( const char* scriptFileName )
{
	script = new cScpScript( const_cast<char *>(scriptFileName) );
}

void Race::parse( void )
{
	if ( !script )
	{
		// oops...we need to switch the race system off and create default race ourselves
		activeRaceSystem = false;
		return;
	}

	parseGlobalSection();
	parseRaceSection();

//DEBUG
//	map<UI32, class Race*>::iterator it(raceMap.begin()), end(raceMap.end());
//	for(;it!=end;++it)
//		it->second->show();
//	SDbgOut("Finished race parsing\n");
	//scriptConOut();
	safedelete(script);
}

void Race::parseGlobalSection( void )
{
	std::string	sectionName = "SECTION GLOBAL";
	UI32 globalSectionCount = script->countSectionInStr( sectionName );

	if ( globalSectionCount != 1 )
	{
		if ( globalSectionCount == 0 )
			ErrOut("Race::parseSectionGlobal() no global section found!\n");
		else
			ErrOut("Race::parseSectionGlobal() multiple global sections found!\n");
		activeRaceSystem = false;
		return;
	}

	UI32		/*i,*/
			loopexit;
	cScpIterator*	iter = NULL;
	string		lha, rha;

	iter = script->getNewIterator( sectionName );
	if ( iter == NULL )
	{
		// OOPS we've got a major problem here! ABORT ??? -> TODO
		ErrOut("Race::parseSectionGlobal() no global section found!\n");
		return ;
	}
	loopexit=0;
	do
	{
		iter->parseLine( lha, rha );
		switch( lha[0] )
		{
			case '{':
			case '}':
				break;
			case '@':
				break;
			case 'M':
				if ( lha == "MODE" )
				{
					if ( rha == "ON" )
						activeRaceSystem = true;
					else if ( rha == "OFF" )
						activeRaceSystem = false;
					else
					{
						WarnOut("%s invalid %s %s\n", sectionName.c_str(), lha.c_str(), rha.c_str() );
						activeRaceSystem = false;
					}
				}
				break;
			case 'S':
				if ( lha == "STARTLOCATION" )
				{
					// quick and dirty for now
					// this will be fixed using a special class location
					UI32 x, y, z;
					sscanf( rha.c_str(), "%u %u %u", &x, &y, &z);
					startLocation[0] = x;
					startLocation[1] = y;
					startLocation[2] = z;
				}
				break;
			case 'T':
				if ( lha == "TELEPORTONENLIST"  )
				{
					if ( rha == "ON" )
						teleportOnEnlist = true;
					else if ( rha == "OFF" )
						teleportOnEnlist = false;
					else
					{
						WarnOut("%s invalid %s %s\n", sectionName.c_str(), lha.c_str(), rha.c_str() );
						teleportOnEnlist = false;
					}
				}
				break;
			case 'W':
				if ( lha == "WEBINTERFACE" )
				{
					if ( rha == "ON" )
						withWebInterface = true;
					else if ( rha == "OFF" )
						withWebInterface = false;
					else
					{
						WarnOut("%s invalid %s %s\n", sectionName.c_str(), lha.c_str(), rha.c_str() );
						withWebInterface = false;
					}
				}
				else if ( rha == "WEBLINK" )
				{
					safedelete(globalWebLink);
					globalWebLink = new string( rha );
				}
				else if ( lha == "WEBROOT" )
				{
					safedelete(globalWebRoot);
					globalWebRoot = new string( rha );
				}
				break;
			default:
				WarnOut("%s invalid %s %s\n", sectionName.c_str(), lha.c_str(), rha.c_str() );
				break;
		}
	} while ( ( lha[0] != '}' ) && ( ++loopexit < MAXLOOPS ) );
}

void Race::parseRaceSection( void )
{
	std::string	sectionName = "SECTION RACE ";
	UI32 raceCount = script->countSectionInStr( sectionName );

	if ( raceCount <= 0 )
	{
		// Problem. Race.xss containes no RACE sections
		// We need to handle this. Let's create a dummy race 0 and route all
		// requests to it or just switch the race system off. TODO
		ErrOut("Race::loadRaces() no races\n");
		activeRaceSystem = false;
		return;
	}

	UI32		/*i,*/
//			raceId,
			loopexit,
			descriptionIndex;
	cScpIterator*	iter = NULL;
	std::string*	sectionParms;
	std::string	section, lha, rha;

	sectionParms = script->getFirstSectionParmsInStr( sectionName );

	do
	{
		section = sectionName + *sectionParms;
    iter = script->getNewIterator( section );
    if ( iter == NULL )
		{
			ErrOut( "Race::loadRaces() no %s\n", section.c_str() );
			return ;
		}
		Race* race = new Race();
		race->id = str2num( *sectionParms );
		loopexit=0;
		descriptionIndex = 0;
		do
		{
			iter->parseLine( lha, rha );
			switch( lha[0] )
			{
				case '{':
				case '}':
					break;
				case '@':
					break;
				case 'A':
					if 			( lha == "ALCHEMY" ||
					     			lha == "ANATOMY" ||
					     			lha == "ANIMALLORE" ||
					     			lha == "ARMSLORE" ||
					     			lha == "ARCHERY")						race->parseSkill( rha );
					break;
				case 'B':
					if 			( lha == "BEARD" )						race->beardPerm = rha;
					else if ( lha == "BEARDCOLOR" )				race->parseBeardColor( rha );
					else if ( lha == "BEGGING" ||
						  			lha == "BLACKSMITHING" ||
						  			lha == "BOWCRAFTING")				race->parseSkill( rha );
					break;
				case 'C':
					if 			( lha == "CAMPING" ||
					     			lha == "CARPENTRY" ||
					     			lha == "CARTOGRAPHY" ||
					     			lha == "COOKING")						race->parseSkill( rha );
				case 'D':
					if 			( lha == "DESCRIPTION" )			race->parseRaceDescription( rha );
					else if ( lha == "DETECTHIDDEN" )			race->parseSkill( rha );
					else if ( lha == "DEXCAP" ) 					race->dexCap = str2num( rha );
					else if	( lha == "DEXMOD" )						race->parseAbilityModifiers( DEXTERITY, rha );
					else if	( lha == "DEXSTART" )					race->dexStart = str2num( rha );
					break;
				case 'E':
					if 			( lha == "ENEMY" )						race->m_mapRacialRelations[ (UI32) str2num( rha ) ] = ENEMY;
					else if ( lha == "ENTICEMENT" ||
						  			lha == "EVALUATEINTELLECT")	race->parseSkill( rha );
					break;
				case 'F':
					if 			( lha == "FENCING" ||
					     			lha == "FISHING" ||
					     			lha == "FORENSICS")					race->parseSkill( rha );
					else if ( lha == "FRIEND" ) 					race->m_mapRacialRelations[ (UI32) str2num( rha ) ] = FRIEND;
					break;
				case 'G':
					if 			( lha == "GENDER" ) 					race->gender = rha;
					break;
				case 'H':
					if 			( lha == "HAIR" )							race->hairPerm = rha;
					else if ( lha == "HAIRCOLOR" )				race->parseHairColor( rha );
					else if ( lha == "HEALING" ||
						  			lha == "HERDING" ||
						  			lha == "HIDING")						race->parseSkill( rha );
					break;
				case 'I':
					if 			( lha == "INSCRIPTION" )			race->parseSkill( rha );
					else if ( lha == "INTCAP" ) 					race->intCap = str2num( rha );
					else if	( lha == "INTMOD" )						race->parseAbilityModifiers( INTELLIGENCE, rha );
					else if	( lha == "INTSTART" )					race->intStart = str2num( rha );
					else if ( lha == "ITEMID" )						race->parseSkill( rha );
					break;
				case 'L':
					if 			( lha == "LOCKPICKING" ||
					     			lha == "LUMBERJACKING")			race->parseSkill( rha );
				case 'M':
					if 			( lha == "MACEFIGHTING" ||
					     			lha == "MAGERY" ||
					     			lha == "MAGICRESISTANCE")		race->parseSkill( rha );
					else if ( lha == "MAGICRESISTANT" )		race->magicResistant = true; // may need to change this to %
					else if ( lha == "MEDITATION" ||
						  			lha == "MINING")						race->parseSkill( rha );
					else if ( lha == "MODE" )
						if ( rha == "ON" )									race->activeRace = true;
						else if( rha == "OFF" )							race->activeRace = false;
						else
						{
							WarnOut("%s invalid %s %s\n", sectionName.c_str(), lha.c_str(), rha.c_str() );
							race->activeRace = false;
						}
					else if ( lha == "MUSICIANSHIP" )			race->parseSkill( rha );
					break;
				case 'N':
					if 			( lha == "NAME" ) 						race->name = rha;
					else if ( lha == "NEUTRAL" )					race->m_mapRacialRelations[ (UI32) str2num( rha ) ] = NEUTRAL;
					break;
				case 'P':
					if 			( lha == "PLURALNAME" ) 			race->pluralName = rha;
					else if ( lha == "POISONING" )				race->parseSkill( rha );
					else if ( lha == "POISONRESISTANCE" ) race->parsePoisonResistance( rha );
					else if ( lha == "PROVOCATION" )			race->parseSkill( rha );
					break;
				case 'R':
					if 			( lha == "RACETYPE" ) 				race->raceType = rha;
					else if ( lha == "REMOVETRAP" )				race->parseSkill( rha );
					break;
				case 'S':
					if 			( lha == "SKILLCAP" )					race->skillCap = str2num( rha );
					else if ( lha == "SKINCOLOR" ) 				race->parseSkinColor( rha );
					else if ( lha == "SNOOPING" ||
						  			lha == "SPIRITSPEAK")				race->parseSkill( rha );
					else if ( lha == "STARTITEM" ) 				race->parseStartItem( rha );
					else if ( lha == "STATCAP" ) 					race->statCap = str2num( rha );
					else if ( lha == "STEALING" ||
						  			lha == "STEALTH")						race->parseSkill( rha );
					else if ( lha == "STRCAP" ) 					race->strCap = str2num( rha );
					else if	( lha == "STRMOD" )						race->parseAbilityModifiers( STRENGTH, rha );
					else if	( lha == "STRSTART" )					race->strStart = str2num( rha );
					else if ( lha == "SWORDMANSHIP" )			race->parseSkill( rha );
					break;
				case 'T':
					if 			( lha == "TAILORING" ||
					     			lha == "TAMING" ||
					     			lha == "TASTEID" ||
					     			lha == "TACTICS")						race->parseSkill( rha );
					else if ( lha == "TILEID" ) 					race->tileId = str2num( rha );
					else if ( lha == "TINKERING" ||
						  			lha == "TRACKING")					race->parseSkill( rha );
					break;
				case 'V':
					if 			( lha == "VETERINARY" )				race->parseSkill( rha );
					break;
				case 'W':
					if 			( lha == "WEBLINK" ) 					race->webLink = rha;
					else if ( lha == "WRESTLING" )				race->parseSkill( rha );
					break;
				default	:
					WarnOut("Race::parseRaceSection unknown tag %s\n", lha.c_str());
					break;
			}
		} while ( lha[0] != '}' && ++loopexit < MAXLOOPS );
		raceMap[race->id] = race;
		iter = NULL;
		sectionParms = script->getNextSectionParmsInStr( sectionName );
	} while ( --raceCount );
}

void Race::parseRaceDescription( const string& sectionName )
{
	UI32		loopexit,
			descriptionIndex;
	cScpIterator*	iter = NULL;
	string		section("SECTION DESCRIPTION ");
	section += sectionName;

	iter = script->getNewIterator( section );
    	if ( iter == NULL )
	{
		ErrOut( "Race::parseRaceDescription() no %s\n", section.c_str() );
	}
	else
	{
		loopexit=0;
		descriptionIndex = 0;
		string Description;
		do
		{
			Description = iter->getEntry()->getFullLine();
			if( Description[0] != '{' && Description[0] != '}' )
				description[descriptionIndex++] = new string( Description );
		} while ( ( Description[0] != '}' ) && ( ++loopexit < MAXLOOPS ) );
	}
}

void Race::parsePoisonResistance( const string& sectionName )
{
	UI32		loopexit;
	cScpIterator*	iter = NULL;
	string		section("SECTION POISONRESISTANCE ");
	section += sectionName;

	iter = script->getNewIterator( section );
    	if ( iter == NULL )
	{
		ErrOut( "Race::parsePoisonResistance() no %s\n", section.c_str() );
	}
	else
	{
		string	lha, rha;

		loopexit=0;
		do
		{
			iter->parseLine( lha, rha );
			if ( lha[0]!='}' && lha[0]!='{' )
			{
				switch ( lha[0] )
				{
					case 'D':
						if ( lha == "DEADLY" )
							sscanf( rha.c_str(), "%f", &poisonResistance[DEADLYPOISON] );
						break;
					case 'G':
						if ( lha == "GREATER" )
							sscanf( rha.c_str(), "%f", &poisonResistance[GREATERPOISON] );
						break;
					case 'L':
						if ( lha == "LETHAL" )
							sscanf( rha.c_str(), "%f", &poisonResistance[LETHALPOISON] );
						break;
					case 'N':
						if ( lha == "NORMAL" )
							sscanf( rha.c_str(), "%f", &poisonResistance[REGULARPOISON] );
						break;
					case 'W':
						if ( lha == "WEAK" )
							sscanf( rha.c_str(), "%f", &poisonResistance[LESSERPOISON]  );
						break;
					default:
						WarnOut("Race::parseRaceSection unknown tag %s\n", lha.c_str());
						break;
				}
			}
		} while ( ( lha[0] != '}' ) && ( ++loopexit < MAXLOOPS ) );
	}
}

void Race::parseSkill( const string& sectionName )
{
	string section("SECTION RACESKILL ");
	section += sectionName;
	RaceScriptEntry	rse( script, section );
	skills += rse;
}

void Race::parseAbilityModifiers( const RACIALABILITY ability, const string& sectionName )
{
	string section("SECTION RACEABILITYMODIFIER ");
	section += sectionName;
	RaceScriptEntry	rse( script, section );
	switch( ability )
	{
		case STRENGTH 		:	strModifiers = rse; break;
		case DEXTERITY		: dexModifiers = rse; break;
		case INTELLIGENCE : intModifiers = rse; break;
	}
}

void Race::parseBeardColor( const string& sectionName )
{
	cScpIterator*	iter = NULL;
	string		section("SECTION BEARDCOLOR ");
  section += sectionName;

	iter = script->getNewIterator( section );
    	if ( iter == NULL )
	{
		ErrOut( "Race::parseBeardColor() no %s\n", section.c_str() );
	}
	else
	{
		UI32 loopexit = 0;
		string color;
		beardColor.clear();
		do
		{
			color = iter->getEntry()->getFullLine();
			if( color[0] != '{' && color[0] != '}' )
				beardColor.push_back( hex2num(color) );
		} while ( ( color[0] != '}' ) && ( ++loopexit < MAXLOOPS ) );
	}
}

void Race::parseHairColor( const string& sectionName )
{
	cScpIterator*	iter = NULL;
	string		section("SECTION HAIRCOLOR ");
	section += sectionName;

	iter = script->getNewIterator( section );
    	if ( iter == NULL )
	{
		ErrOut( "Race::parseHairColor() no %s\n", section.c_str() );
	}
	else
	{
		UI32 loopexit = 0;
		string color;
		hairColor.clear();
		do
		{
			color = iter->getEntry()->getFullLine();
			switch( color[0] )
			{
				case '{' :
				case '}' :
					break;
				default:
					hairColor.push_back( hex2num(color) );
			}
		} while ( ( color[0] != '}' ) && ( ++loopexit < MAXLOOPS ) );
	}
}

void Race::parseSkinColor( const string& sectionName )
{
	cScpIterator*	iter = NULL;
	string		section("SECTION SKINCOLOR ");
	section += sectionName;

	iter = script->getNewIterator( section );
    	if ( iter == NULL )
	{
		ErrOut( "Race::parseSkinColor() no %s\n", section.c_str() );
	}
	else
	{
		UI32 loopexit = 0;
		string color;
		skinColor.clear();
		do
		{
			color = iter->getEntry()->getFullLine();
			if( color[0] != '{' && color[0] != '}' )
				skinColor.push_back( hex2num(color) );
		} while ( ( color[0] != '}' ) && ( ++loopexit < MAXLOOPS ) );
	}
}

void Race::parseStartItem( const string& itemReference )
{
	//
	// Clear startItems vector as multiple startitem tags may be available (e.g. by using #copy)
	//
	startItems.clear();

	string 		section("SECTION STARTITEM ");
	section += itemReference;
	cScpIterator* 	iter    = NULL;
	iter = script->getNewIterator( section );
	if ( iter )
	{
		int 	loopexit = 0;
		string	itemId;
		do
		{
			itemId = iter->getEntry()->getFullLine();
			if( itemId[0] != '{' && itemId[0] != '}' )
				parseStartItemDetails( itemId );
		}
		while (( itemId[0] != '}') && (++loopexit < MAXLOOPS) );
	}
	else
		WarnOut( "Race::parseStartItem() unknown %s\n", section.c_str() );
}

void Race::parseStartItemDetails( const string& itemReference )
{
	string section("SECTION ITEM ");
	section += itemReference;
	cScpIterator* iter = NULL;
	iter = Scripts::Items->getNewIterator(section);
	if ( iter )
	{
		int 		loopexit = 0;
		string		lha, rha;
		RaceStartItem	raceItem;

		raceItem.itemReference = str2num( const_cast<char*>(itemReference.c_str()) );
		do
		{
			iter->parseLine( lha, rha );
			if ( lha[0]!='}' && lha[0]!='{' )
			{
				switch ( lha[0] )
				{
					case 'G':
						if ( lha == "GENDER" )
							raceItem.gender = rha.c_str() ;
						break;
					case 'L':
						if ( lha == "LAYER"  )
							raceItem.layer = str2num( rha );
						break;
					case 'M':
						if ( lha == "MOVABLE" && str2num( rha ) == 2 )
							raceItem.protectedItem = true;
					case 'S':
						if ( lha == "SKINCOLOR" )
							raceItem.skinColor = true;
						break;
				}
			}
		}
		while ( lha[0] != '}' && ++loopexit < MAXLOOPS );

		if( !raceItem.layer && raceItem.protectedItem )
			raceItem.protectedItem = false; // no protection on layer 0

		//startItems.push_back( raceItem );
	}
	else
		ErrOut( "Race::parseStartItemDetails() unknown %s\n", itemReference.c_str() );
	safedelete( iter );
}

void Race::scriptConOut( void )
{
	UI32 raceCount = script->countSectionInStr("SECTION");

	if ( raceCount <= 0 )
	{
		// Problem. Race.xss containes no RACE sections
		// We need to handle this. Let's create a dummy race 0 and route all
		// requests to it or just switch the race system off. TODO
		ErrOut("Race::scriptConOut() no sections\n");
		return;
	}

	UI32		/*i,*/
			loopexit;
	cScpIterator*	iter = NULL;
	char 		sect[512], script1[512], script2[512];
	std::string*	sectionParms;
	std::string	sectionName("SECTION");

	sectionParms = script->getFirstSectionParmsInStr(sectionName);

	do
	{
		sprintf( sect, "%s%s", sectionName.c_str(), const_cast<char *>(sectionParms->c_str()) );
    iter = script->getNewIterator( sect );
    if ( iter == NULL )
		{
			// OOPS we've got a major problem here! ABORT ??? -> TODO
			ErrOut( "Race::scriptConOut() no %s %s\n", sectionName.c_str(), sectionParms->c_str());
			return ;
		}
		loopexit=0;
		do
		{
			iter->parseLine(script1, script2);
		} while ( ( script1[0] != '}' ) && ( ++loopexit < MAXLOOPS ) );
		safedelete(iter);
		sectionParms = script->getNextSectionParmsInStr(sectionName);
	} while ( --raceCount );
}

void Race::reload( const char* fileName )
{
	// delete all RaceInstances
	// as there are no pointer references in cChar but only an numerical index no update of pc's or npc's is necessary
	//
	map<UI32, class Race*>::iterator raceIter( raceMap.begin() ), raceEnd( raceMap.end() );

	for(; raceIter != raceEnd; ++raceIter )	safedelete(raceIter->second);
	load( fileName );
}

void Race::handleButton( const NXWSOCKET socket, const UI32 gump, const UI32 button )
{
	switch( gump )
	{
		case 100:
			switch( button )
			{
				case 0: // Web launch
					if ( withWebInterface )
					{
						string*	url = new string( *globalWebRoot + *globalWebLink );
						weblaunch( socket, (char*) url->c_str() );
					}
					enlistDialog1( socket );
					break;
				case 1:	// race information
					dialogRaceInfo( socket );
					break;
				case 2: // race selection
					dialogChoosePlayerRace( socket );
					break;
			}
			break;
		case 110: // race information menu
			switch( button )
			{
				case 0: // Web launch
					if ( withWebInterface )
					{
						string*	url = new string( *globalWebRoot + *globalWebLink );
						weblaunch( socket, (char*) url->c_str() );
					}
					dialogRaceInfo( socket );
					break;
				case 999:	// Okay
					enlistDialog1( socket );
					break;
				case 1:	// General information
					dialogNotImplemented( socket );
					break; // not yet implemented
				case 2: // Player races
					showRaceNameList( socket, PCRACE, 112, false, true, true );
					break;
				case 3: // Non-player races
					showRaceNameList( socket, NPCRACE, 113, false, true, true );
					break;
				case 4: // All races
					showRaceNameList( socket, PCNPCRACE, 114, false, true, true );
					break;
				default:
					ErrOut( "Race::handleButton unknown button %d pressed in gump %d\n", button, gump );
					dialogRaceInfo( socket );
					break;
			}
			break;
		case 111:	// General race information
			switch( button )
			{
				case 0: // Web launch
					if ( withWebInterface )
					{
						string*	url = new string( *globalWebRoot + *globalWebLink );
						weblaunch( socket, (char*) url->c_str() );
					}
					dialogRaceInfo( socket );
					break;
				case 999:	// Okay
					dialogRaceInfo( socket );
					break;
				default:
					break;
			}
			break;
		case 112:	// Player race information
			switch( button )
			{
				case 999:	// Okay
					dialogRaceInfo( socket );
					break;
				default:
					// if button is not a valid race then the web interface button has been pressed
					if ( raceMap.find( button ) == raceMap.end() )
					{
						string*	url = new string( *globalWebRoot + *globalWebLink );
						weblaunch( socket, (char*) url->c_str() );
						dialogPlayerRaceInfo( socket );
					}
					else
						showRaceDescription( socket, button, 115, false, true, true );
					break;
			}
			break;
		case 113:	// Non player information
			switch( button )
			{
				case 999:	// Okay
					dialogRaceInfo( socket );
					break;
				default:
					// if button is not a valid race then the web interface button has been pressed
					if ( raceMap.find( button ) == raceMap.end() )
					{
						string*	url = new string( *globalWebRoot + *globalWebLink );
						weblaunch( socket, (char*) url->c_str() );
						dialogNonPlayerRaceInfo( socket );
					}
					else
						showRaceDescription( socket, button, 115, false, true, true );
					break;
			}
			break;
		case 114:	// Any race information
			switch( button )
			{
				case 999:	// Okay
					dialogRaceInfo( socket );
					break;
				default:
					// if button is not a valid race then the web interface button has been pressed
					if ( raceMap.find( button ) == raceMap.end() )
					{
						string*	url = new string( *globalWebRoot + *globalWebLink );
						weblaunch( socket, (char*) url->c_str() );
						dialogAnyRaceInfo( socket );
					}
					else
						showRaceDescription( socket, button, 115, false, true, true );
					break;
			}
			break;
		case 115:
			switch( button )
			{
				case 0:
					dialogRaceInfo( socket );
					break;
				default:
					{
						string*	url = new string( *globalWebRoot + raceMap[button]->webLink );
						weblaunch( socket, (char*) url->c_str() );
					}
					showRaceDescription( socket, button, 115, false, true, true );
					break;
			}
			break;
		case 120:	// user selected a race proceed with enlisting user in race
			switch( button )
			{
				case 999:	// Okay
					enlistDialog1( socket );
					break;
				default: // user has choosen a race to become a member of
					{
					P_CHAR pc = MAKE_CHARREF_LOGGED( currchar[socket], err );
					pc->race = button;
					// start racial parameter dialog. For now all settings like coloring are done automatically i.e. no choices.
					//enlistDialog121( socket );
										{
					extern void KillTarget(P_CHAR pc, int ly);

					Race* race = raceMap[ pc->race ];
					if(!race) return;
					//
					// Alter appearance if necessary
					//
					// Only alter skincolor if set in race.xss
					//
					if( !race->skinColor.empty() )
					{
						UI32 skinColor = RandomNum( 0, race->skinColor.size() -1 );
						pc->setSkinColor(race->skinColor[skinColor]);
						pc->setOldSkinColor(race->skinColor[skinColor]);
					}
					if ( RT_PROHIBITED == getBeardPerm( pc->race ) )
						KillTarget( pc, 0x10 );
					if ( RT_PROHIBITED == getHairPerm( pc->race ) )
						KillTarget( pc, 0xb );
					//
					// unequip/unwear most things and put them in the backpack
					//
					unsigned char layer;
					P_ITEM pi;
					P_ITEM backpack = pc->getBackpack(); // Hmmm check needed?
					if (!(ISVALIDPI(backpack))) return;// Unavowed: workaround to prevent crashes.
					for( layer = 1; layer < 30; layer++)
					{
						switch( layer )
						{
							case 11:
							case 16:
							case 21:
							case 25:
							case 26:
							case 27:
							case 28:
							case 29:
								break;
							default:
								pi = pc->GetItemOnLayer( layer );
								if ( ISVALIDPI( pi ) )
									backpack->AddItem( pi );
								break;
						}

					}
					//
					// equip / wear items or put them in backpack
					//
					if ( !race->startItems.empty() )
					{
						vector< class RaceStartItem >::iterator iter( race->startItems.begin() ), end( race->startItems.end() );
						for( ; iter != end; ++iter )
						{
						   	pi = item::CreateScriptItem( socket, static_cast< int >(iter->itemReference), 0);
    						if ( ISVALIDPI( pi ) )
							{
								if( ( (pc->GetBodyType() == BODY_MALE) && (iter->gender == MALE) ) ||
								    ( (pc->GetBodyType() == BODY_FEMALE) && (iter->gender == FEMALE) ) ||
									( iter->gender == FEMALE_OR_MALE)
									)
								{
									if( iter->protectedItem )
										pi->magic = 2;
									if( pi->layer && !pc->GetItemOnLayer( pi->layer ) )
									{
										if( iter->skinColor )
											pi->setColor( pc->getSkinColor() );

										pi->setContSerial( pc->getSerial32() );
									}
									else
										backpack->AddItem( pi );
								}
								else	
									pi->deleteItem();
							}
						}
					}
					//
					// if teleported to safe location return player to original position
					// TODO allow pc to be teleported to racial home region
					//
					if ( teleportOnEnlist )
					{
						/*
						pc->x = pc->oldx;
						pc->y = pc->oldy;
						pc->z = pc->oldz;
						*/
						pc->setPosition( pc->getOldPosition() );
						pc->MoveTo( pc->getPosition() );
					}
					pc->teleport();
					sysmessage( socket, "You have become a %s", getName( pc->race  )->c_str() );
					}
					break;
				}
			}
			break;
		case 121: // handle racial paramter choices made by user
			switch (button )
			{
				case 0:
					dialogChoosePlayerRace( socket );
					break;
				default:
					break;
			}
			break;
		default:
			break;

	}
}

void Race::enlist( const NXWSOCKET socket )
{
	if ( teleportOnEnlist )
	{
		P_CHAR pc = MAKE_CHARREF_LOGGED( currchar[socket], err );
		/*
		Location charpos= pc->getPosition();
		pc->oldx = charpos.x;
		pc->oldy = charpos.y;
		pc->oldz = charpos.z;
		*/
		pc->setOldPosition( pc->getPosition() );
		pc->MoveTo( startLocation[0], startLocation[1], (signed char)startLocation[2] );
		pc->teleport();
	}
	enlistDialog1( socket );
}

/*
void Race::resurrect( void )
{
}
*/

//	Function		: enlistDialog1
//	Purpose			: Start race selection process for player character
//				  shows introductory gump
//	Input			: valid socket number
//	Output			: none
//	Creator			: Sparhawk
//	Version			: 070.01
//	Date created		: 2002-02-03
//	Date last updated	: 2002-02-03
//	History			: none as yet
//
void Race::enlistDialog1( const NXWSOCKET socket )
{
	UI32	i,
		linecount	=	0,
		linecount1	=	0,
		length = 0,
		length2 = 0,
		line,
		textlines,
		size;
	string	*menuLines0	= new string[50];
	string	*menuLines1	= new string[50];


	menuLines0[linecount++] = "{ noclose }";
	menuLines0[linecount++] = "{ page 0 }";

	menuLines0[linecount++] = "{ resizepic 0 0 5120 320 340 }";    	// Background
	menuLines0[linecount++] = "{ gumppic 20 80 1418 }";

	if ( withWebInterface )
	{
		menuLines0[linecount++] = "{ tilepic 286  8 3811 }"; 		// Race info on web
		menuLines0[linecount++] = "{ button 294 30 1209 1210 1 0 0 }";
	}

	menuLines0[linecount++] = "{ text  73   8  152  0 }";		// Header
	menuLines0[linecount++] = "{ text  85   8   95  1 }";
	menuLines0[linecount++] = "{ text 151   8  152  2 }";
	menuLines0[linecount++] = "{ text 163   8   95  3 }";
	menuLines0[linecount++] = "{ text 189   8  152  4 }";
	menuLines0[linecount++] = "{ text 201   8   95  5 }";

	menuLines0[linecount++] = "{ text  13  42 1153  6 }";		// Info paragraph 1
	menuLines0[linecount++] = "{ text  25  42   95  7 }";
	menuLines0[linecount++] = "{ text  13  58   95  8 }";

	menuLines0[linecount++] = "{ text  13  88 1153  9 }";		// Info paragraph 2
	menuLines0[linecount++] = "{ text  25  88   95 10 }";
	menuLines0[linecount++] = "{ text  13 106   95 11 }";

	if ( teleportOnEnlist )
	{
		menuLines0[linecount++] = "{ text  13 136 1153 12 }";	// Teleport to safe location info
		menuLines0[linecount++] = "{ text  25 136   95 13 }";
		menuLines0[linecount++] = "{ text  13 154   95 14 }";
		menuLines0[linecount++] = "{ text  13 172   95 15 }";

		menuLines0[linecount++] = "{ button 13 199 1209 1210 1 0 1 }"; 	// Information about races
		menuLines0[linecount++] = "{ text  33 196 1153 16 }";
		menuLines0[linecount++] = "{ text  42 196   95 17 }";
		menuLines0[linecount++] = "{ button 13 229 1209 1209 1 0 2 }"; 	// Choose a race
		menuLines0[linecount++] = "{ text  33 226 1153 18 }";
		menuLines0[linecount++] = "{ text  45 226   95 19 }";
	}
	else	// don't leave an ugly gap when teleport is off
	{
		menuLines0[linecount++] = "{ button 13 139 1209 1210 1 0 1 }";	// Race info
		menuLines0[linecount++] = "{ text  33 136 1153 16 }";
		menuLines0[linecount++] = "{ text  42 136   95 17 }";
		menuLines0[linecount++] = "{ button 13 169 1209 1209 1 0 2 }";	// Race selection
		menuLines0[linecount++] = "{ text  33 166 1153 18 }";
		menuLines0[linecount++] = "{ text  45 166   95 19 }";
	}

	for( line=0; line < linecount; line++)
		if ( !menuLines0[line].empty() )
			length  += menuLines0[line].size();

	//                          123456789012345678901234567890123456789012345
	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "oxwizard";
	menuLines1[linecount1++] = "R";
	menuLines1[linecount1++] = "ace";
	menuLines1[linecount1++] = "S";
	menuLines1[linecount1++] = "ystem";

	menuLines1[linecount1++] = "D";
	menuLines1[linecount1++] = "ear player the Noxwizard Race System";
	menuLines1[linecount1++] = "has been activated on this shard.";

	menuLines1[linecount1++] = "A";
	menuLines1[linecount1++] = "s you do not belong to a race yet.....";
	menuLines1[linecount1++] = "you must choose one now!";

	menuLines1[linecount1++] = "Y";
	menuLines1[linecount1++] = "ou have been transported to a safe location";
	menuLines1[linecount1++] = "and will be returned after you've made your";
	menuLines1[linecount1++] = "choice.";

	menuLines1[linecount1++] = "I";
	menuLines1[linecount1++] = "nformation about races";

	menuLines1[linecount1++] = "S";
	menuLines1[linecount1++] = "elect your race";

	textlines = 0;
	length2 += length + 1;
	for( line = 0; line < linecount1; line++)
	{
		if ( menuLines1[line].empty() )
			menuLines1[line] = " ";
		length += menuLines1[line].size() * 2 + 2;
		++textlines;
	}
	length += 24;

	unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	gump1[1]  = length >> 8;
	gump1[2]  = length % 256;
	gump1[7]  = 0;
	gump1[8]  = 0;
	gump1[9]  = 0;
	gump1[10] = 100; // Gump Number (type)
	gump1[19] = length2>>8;
	gump1[20] = length2%256;
	Xsend(socket, gump1, 21);

	for( line = 0; line < linecount; line++ )
		Xsend(socket, const_cast<char *>(menuLines0[line].c_str() ), menuLines0[line].size() );

	unsigned char gump2[4]="\x00\x00\x00";
	gump2[1]  = textlines >> 8;
	gump2[2]  = textlines % 256;

	Xsend(socket, gump2, 3);
	unsigned char gump3[3]="\x00\x00";
	for( line = 0;line < linecount1; line++ )
	{
		size = menuLines1[line].size();
		gump3[0] = size >> 8;
		gump3[1] = size % 256;
		Xsend(socket, gump3, 2);
		gump3[0]=0;
		for (i = 0; i < size; i++ )
		{
			gump3[1] = menuLines1[line][i];
			Xsend(socket, gump3, 2); // this is very sloppy. better is to extend gump3
						 // and send the entire packet at once
		}
	}

}


void Race::dialogRaceInfo( const NXWSOCKET socket )
{
	UI32	i,
		linecount		=	0,
		linecount1	=	0,
		length			= 0,
		length2			= 0,
		line,
		textlines,
		size;
	char	buffer[512];
	string	*menuLines0	= new string[50];
	string	*menuLines1	= new string[50];


	menuLines0[linecount++] = "noclose";
	menuLines0[linecount++] = "page 0";
	menuLines0[linecount++] = "resizepic 0 0 5120 320 340";    	// The background
	menuLines0[linecount++] = "gumppic 20 80 1418";
	menuLines0[linecount++] = "button 10 300 2130 2129 1 0 999"; 	// OKAY

	menuLines0[linecount++] = "text 73 8 152 0";	// Header text 0..5
	menuLines0[linecount++] = "text 85 8 95 1";
	menuLines0[linecount++] = "text 151 8 152 2";
	menuLines0[linecount++] = "text 163 8 95 3";
	menuLines0[linecount++] = "text 189 8 152 4";
	menuLines0[linecount++] = "text 201 8 95 5";

	menuLines0[linecount++] = "text 13 42 1153 6";
	menuLines0[linecount++] = "text 25 42 95 7";

	menuLines0[linecount++] = "button 13 82 1209 1210 1 0 1"; // General Information
	menuLines0[linecount++] = "text 33 82 1153 8";
	menuLines0[linecount++] = "text 45 82 95 9";

	menuLines0[linecount++] = "button 13 102 1209 1210 1 0 2"; // Player races
	menuLines0[linecount++] = "text 33 102 1153 10";
	menuLines0[linecount++] = "text 45 102 95 11";

	menuLines0[linecount++] = "button 13 122 1209 1210 1 0 3"; // Non player races
	menuLines0[linecount++] = "text 33 122 1153 12";
	menuLines0[linecount++] = "text 45 122 95 13";

	menuLines0[linecount++] = "button 13 142 1209 1210 1 0 4"; // All races
	menuLines0[linecount++] = "text 33 142 1153 14";
	menuLines0[linecount++] = "text 45 142 95 15";

	if ( withWebInterface )
	{
		menuLines0[linecount++] = "tilepic 286  8 3811"; 		// Race info on web
		menuLines0[linecount++] = "button 294 30 1209 1210 1 0 0";
	}

	for( line=0; line < linecount; line++)
		if ( !menuLines0[line].empty() )
			length  += menuLines0[line].size() + 4;

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "oxwizard";
	menuLines1[linecount1++] = "R";
	menuLines1[linecount1++] = "ace";
	menuLines1[linecount1++] = "S";
	menuLines1[linecount1++] = "ystem";

	menuLines1[linecount1++] = "C";
	menuLines1[linecount1++] = "hoose an option";

	menuLines1[linecount1++] = "G";
	menuLines1[linecount1++] = "eneral information";

	menuLines1[linecount1++] = "P";
	menuLines1[linecount1++] = "layer races";

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "on player races";

	menuLines1[linecount1++] = "A";
	menuLines1[linecount1++] = "ll races";


	textlines = 0;
	length2 = length + 1;

	for( line = 0; line < linecount1; line++)
	{
		if ( menuLines1[line].empty() )
			menuLines1[line] = " ";
		length += menuLines1[line].size() * 2 + 2;
		++textlines;
	}

	length += 24;

	unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	gump1[1]  = length >> 8;
	gump1[2]  = length % 256;
	gump1[7]  = 0;
	gump1[8]  = 0;
	gump1[9]  = 0;
	gump1[10] = 110; // Gump Number (type)
	gump1[19] = length2>>8;
	gump1[20] = length2%256;
	Xsend(socket, gump1, 21);

	for( line = 0; line < linecount; line++ )
	{
		sprintf(buffer, "{ %s }", menuLines0[line].c_str() );
		Xsend(socket, buffer, strlen(buffer) );
	}

	unsigned char gump2[4]="\x00\x00\x00";
	gump2[1]  = textlines >> 8;
	gump2[2]  = textlines % 256;

	Xsend(socket, gump2, 3);

	unsigned char gump3[3]="\x00\x00";
	for( line = 0;line < linecount1; line++ )
	{
		size = menuLines1[line].size();
		gump3[0] = size >> 8;
		gump3[1] = size % 256;
		Xsend(socket, gump3, 2);
		gump3[0]=0;
		for (i = 0; i < size; i++ )
		{
			gump3[1] = menuLines1[line][i];
			Xsend(socket, gump3, 2);
		}
	}

}


void Race::dialogNotImplemented( const NXWSOCKET socket )
{
	UI32	i,
		linecount		= 0,
		linecount1	= 0,
		length			= 0,
		length2			= 0,
		line,
		textlines,
		size;
	char	buffer[512];
	string	*menuLines0	= new string[50];
	string	*menuLines1	= new string[50];


	menuLines0[linecount++] = "noclose";
	menuLines0[linecount++] = "page 0";
	menuLines0[linecount++] = "resizepic 0 0 5120 320 340";    	// The background
	menuLines0[linecount++] = "gumppic 20 80 1418";
	menuLines0[linecount++] = "button 10 300 2130 2129 1 0 999"; 	// OKAY

	menuLines0[linecount++] = "text  73   8  152  0";	// Header text 0..5
	menuLines0[linecount++] = "text  85   8   95  1";
	menuLines0[linecount++] = "text 151   8  152  2";
	menuLines0[linecount++] = "text 163   8   95  3";
	menuLines0[linecount++] = "text 189   8  152  4";
	menuLines0[linecount++] = "text 201   8   95  5";

	menuLines0[linecount++] = "text  13  42 1153  6";
	menuLines0[linecount++] = "text  25  42   95  7";

	if ( withWebInterface )
	{
		menuLines0[linecount++] = "tilepic 286  8 3811"; 		// Race info on web
		menuLines0[linecount++] = "button 294 30 1209 1210 1 0 0";
	}

	for( line=0; line < linecount; line++)
		if ( !menuLines0[line].empty() )
			length += menuLines0[line].size() + 4;

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "oxwizard";
	menuLines1[linecount1++] = "R";
	menuLines1[linecount1++] = "ace";
	menuLines1[linecount1++] = "S";
	menuLines1[linecount1++] = "ystem";

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "ot implemented yet";

	textlines = 0;
	length2 = length + 1;

	for( line = 0; line < linecount1; line++)
	{
		if ( menuLines1[line].empty() )
			menuLines1[line] = " ";
		length += menuLines1[line].size() * 2 + 2;
		++textlines;
	}

	length += 24;

	unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	gump1[1]  = length >> 8;
	gump1[2]  = length % 256;
	gump1[7]  = 0;
	gump1[8]  = 0;
	gump1[9]  = 0;
	gump1[10] = 111; // Gump Number (type)
	gump1[19] = length2>>8;
	gump1[20] = length2%256;
	Xsend(socket, gump1, 21);

	for( line = 0; line < linecount; line++ )
	{
		sprintf(buffer, "{ %s }", menuLines0[line].c_str() );
		Xsend(socket, buffer, strlen(buffer) );
	}

	unsigned char gump2[4]="\x00\x00\x00";
	gump2[1]  = textlines >> 8;
	gump2[2]  = textlines % 256;

	Xsend(socket, gump2, 3);
	unsigned char gump3[3]="\x00\x00";
	for( line = 0;line < linecount1; line++ )
	{
		size = menuLines1[line].size();
		gump3[0] = size >> 8;
		gump3[1] = size % 256;
		Xsend(socket, gump3, 2);
		gump3[0]=0;
		for (i = 0; i < size; i++ )
		{
			gump3[1] = menuLines1[line][i];
			Xsend(socket, gump3, 2);
		}
	}

}

void Race::dialogPlayerRaceInfo( const NXWSOCKET socket )
{
	UI32	i,
		linecount		= 0,
		linecount1	= 0,
		length			= 0,
		length2			= 0,
		line,
		textlines,
		size;
	char	buffer[512];
	string	*menuLines0	= new string[50];
	string	*menuLines1	= new string[50];


	menuLines0[linecount++] = "noclose";
	menuLines0[linecount++] = "page 0";
	menuLines0[linecount++] = "resizepic 0 0 5120 320 340";    	// The background
	menuLines0[linecount++] = "gumppic 20 80 1418";
	menuLines0[linecount++] = "button 10 300 2130 2129 1 0 999"; 	// OKAY

	menuLines0[linecount++] = "text  73   8  152  0";	// Header text 0..5
	menuLines0[linecount++] = "text  85   8   95  1";
	menuLines0[linecount++] = "text 151   8  152  2";
	menuLines0[linecount++] = "text 163   8   95  3";
	menuLines0[linecount++] = "text 189   8  152  4";
	menuLines0[linecount++] = "text 201   8   95  5";

	menuLines0[linecount++] = "text  13  42 1153  6";
	menuLines0[linecount++] = "text  25  42   95  7";

	if ( withWebInterface )
	{
		menuLines0[linecount++] = "tilepic 286  8 3811"; 		// Race info on web
		menuLines0[linecount++] = "button 294 30 1209 1210 1 0 0";
	}

	for( line=0; line < linecount; line++)
		if ( menuLines0[line].empty() )
			length += menuLines0[line].size() + 4;

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "oxwizard";
	menuLines1[linecount1++] = "R";
	menuLines1[linecount1++] = "ace";
	menuLines1[linecount1++] = "S";
	menuLines1[linecount1++] = "ystem";

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "ot implemented yet";

	textlines = 0;
	length2 = length + 1;

	for( line = 0; line < linecount1; line++)
	{
		if ( menuLines1[line].empty() )
			menuLines1[line] = " ";
		length += menuLines1[line].size() * 2 + 2;
		++textlines;
	}

	length += 24;

	unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	gump1[1]  = length >> 8;
	gump1[2]  = length % 256;
	gump1[7]  = 0;
	gump1[8]  = 0;
	gump1[9]  = 0;
	gump1[10] = 112; // Gump Number (type)
	gump1[19] = length2>>8;
	gump1[20] = length2%256;
	Xsend(socket, gump1, 21);

	for( line = 0; line < linecount; line++ )
	{
		sprintf(buffer, "{ %s }", menuLines0[line].c_str() );
		Xsend(socket, buffer, strlen(buffer) );
	}

	unsigned char gump2[4]="\x00\x00\x00";
	gump2[1]  = textlines >> 8;
	gump2[2]  = textlines % 256;

	Xsend(socket, gump2, 3);

	unsigned char gump3[3]="\x00\x00";
	for( line = 0;line < linecount1; line++ )
	{
		size = menuLines1[line].size();
		gump3[0] = size >> 8;
		gump3[1] = size % 256;
		Xsend(socket, gump3, 2);
		gump3[0]=0;
		for (i = 0; i < size; i++ )
		{
			gump3[1] = menuLines1[line][i];
			Xsend(socket, gump3, 2);
		}
	}

}

void Race::dialogNonPlayerRaceInfo( const NXWSOCKET socket )
{
	UI32	i,
		linecount		= 0,
		linecount1	= 0,
		length			= 0,
		length2			= 0,
		line,
		textlines,
		size;
	char	buffer[512];
	string	*menuLines0	= new string[50];
	string	*menuLines1	= new string[50];


	menuLines0[linecount++] = "noclose";
	menuLines0[linecount++] = "page 0";
	menuLines0[linecount++] = "resizepic 0 0 5120 320 340";    	// The background
	menuLines0[linecount++] = "gumppic 20 80 1418";
	menuLines0[linecount++] = "button 10 300 2130 2129 1 0 999"; 	// OKAY

	menuLines0[linecount++] = "text  73   8  152  0";	// Header text 0..5
	menuLines0[linecount++] = "text  85   8   95  1";
	menuLines0[linecount++] = "text 151   8  152  2";
	menuLines0[linecount++] = "text 163   8   95  3";
	menuLines0[linecount++] = "text 189   8  152  4";
	menuLines0[linecount++] = "text 201   8   95  5";

	menuLines0[linecount++] = "text  13  42 1153  6";
	menuLines0[linecount++] = "text  25  42   95  7";

	if ( withWebInterface )
	{
		menuLines0[linecount++] = "tilepic 286  8 3811"; 		// Race info on web
		menuLines0[linecount++] = "button 294 30 1209 1210 1 0 0";
	}

	for( line=0; line < linecount; line++)
		if ( !menuLines0[line].empty() )
			length += menuLines0[line].size() + 4;

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "oxwizard";
	menuLines1[linecount1++] = "R";
	menuLines1[linecount1++] = "ace";
	menuLines1[linecount1++] = "S";
	menuLines1[linecount1++] = "ystem";

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "ot implemented yet";

	textlines = 0;
	length2 = length + 1;

	for( line = 0; line < linecount1; line++)
	{
		if ( menuLines1[line].empty() )
			menuLines1[line] = " ";
		length += menuLines1[line].size() * 2 + 2;
		++textlines;
	}

	length += 24;

	unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	gump1[1]  = length >> 8;
	gump1[2]  = length % 256;
	gump1[7]  = 0;
	gump1[8]  = 0;
	gump1[9]  = 0;
	gump1[10] = 113; // Gump Number (type)
	gump1[19] = length2>>8;
	gump1[20] = length2%256;
	Xsend(socket, gump1, 21);

	for( line = 0; line < linecount; line++ )
	{
		sprintf(buffer, "{ %s }", menuLines0[line].c_str() );
		Xsend(socket, buffer, strlen(buffer) );
	}

	unsigned char gump2[4]="\x00\x00\x00";
	gump2[1]  = textlines >> 8;
	gump2[2]  = textlines % 256;

	Xsend(socket, gump2, 3);

	unsigned char gump3[3]="\x00\x00";
	for( line = 0;line < linecount1; line++ )
	{
		size = menuLines1[line].size();
		gump3[0] = size >> 8;
		gump3[1] = size % 256;
		Xsend(socket, gump3, 2);
		gump3[0]=0;
		for (i = 0; i < size; i++ )
		{
			gump3[1] = menuLines1[line][i];
			Xsend(socket, gump3, 2);
		}
	}

}


void Race::dialogAnyRaceInfo( const NXWSOCKET socket )
{
	UI32	i,
		linecount		=  0,
		linecount1	=  0,
		length			=  0,
		length2			=  0,
		line,
		textlines,
		size;
	char	buffer[512];
	string	*menuLines0	= new string[50];
	string	*menuLines1	= new string[50];


	menuLines0[linecount++] = "noclose";
	menuLines0[linecount++] = "page 0";
	menuLines0[linecount++] = "resizepic 0 0 5120 320 340";    	// The background
	menuLines0[linecount++] = "gumppic 20 80 1418";
	menuLines0[linecount++] = "button 10 300 2130 2129 1 0 999"; 	// OKAY

	menuLines0[linecount++] = "text  73   8  152  0";	// Header text 0..5
	menuLines0[linecount++] = "text  85   8   95  1";
	menuLines0[linecount++] = "text 151   8  152  2";
	menuLines0[linecount++] = "text 163   8   95  3";
	menuLines0[linecount++] = "text 189   8  152  4";
	menuLines0[linecount++] = "text 201   8   95  5";

	menuLines0[linecount++] = "text  13  42 1153  6";
	menuLines0[linecount++] = "text  25  42   95  7";

	if ( withWebInterface )
	{
		menuLines0[linecount++] = "tilepic 286  8 3811"; 		// Race info on web
		menuLines0[linecount++] = "button 294 30 1209 1210 1 0 0";
	}

	for( line=0; line < linecount; line++)
		if ( !menuLines0[line].empty() )
			length += menuLines0[line].size() + 4;

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "oxwizard";
	menuLines1[linecount1++] = "R";
	menuLines1[linecount1++] = "ace";
	menuLines1[linecount1++] = "S";
	menuLines1[linecount1++] = "ystem";

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "ot implemented yet";

	textlines = 0;
	length2 = length + 1;

	for( line = 0; line < linecount1; line++)
	{
		if ( menuLines1[line].empty() )
			menuLines1[line] = " ";
		length += menuLines1[line].size() * 2 + 2;
		++textlines;
	}

	length += 24;

	unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	gump1[1]  = length >> 8;
	gump1[2]  = length % 256;
	gump1[7]  = 0;
	gump1[8]  = 0;
	gump1[9]  = 0;
	gump1[10] = 114; // Gump Number (type)
	gump1[19] = length2>>8;
	gump1[20] = length2%256;
	Xsend(socket, gump1, 21);

	for( line = 0; line < linecount; line++ )
	{
		sprintf(buffer, "{ %s }", menuLines0[line].c_str() );
		Xsend(socket, buffer, strlen(buffer) );
	}

	unsigned char gump2[4]="\x00\x00\x00";
	gump2[1]  = textlines >> 8;
	gump2[2]  = textlines % 256;

	Xsend(socket, gump2, 3);

	unsigned char gump3[3]="\x00\x00";
	for( line = 0;line < linecount1; line++ )
	{
		size = menuLines1[line].size();
		gump3[0] = size >> 8;
		gump3[1] = size % 256;
		Xsend(socket, gump3, 2);
		gump3[0]=0;
		for (i = 0; i < size; i++ )
		{
			gump3[1] = menuLines1[line][i];
			Xsend(socket, gump3, 2);
		}
	}

}


void Race::dialogChoosePlayerRace( const NXWSOCKET socket )
{
	showRaceNameList( socket, PCRACE, 120, false, true, true );
}

void Race::enlistDialog121( const NXWSOCKET socket )
{
	UI32	i,
		linecount		= 0,
		linecount1	= 0,
		length			= 0,
		length2			= 0,
		line,
		textlines,
		size;
	char	buffer[512];
	string	*menuLines0	= new string[50];
	string	*menuLines1	= new string[50];


	menuLines0[linecount++] = "noclose";
	menuLines0[linecount++] = "page 0";
	menuLines0[linecount++] = "resizepic 0 0 5120 320 340";    	// The background
	menuLines0[linecount++] = "gumppic 20 80 1418";
	menuLines0[linecount++] = "button 10 300 2130 2129 1 0 999"; 	// OKAY

	menuLines0[linecount++] = "text  73   8  152  0";	// Header text 0..5
	menuLines0[linecount++] = "text  85   8   95  1";
	menuLines0[linecount++] = "text 151   8  152  2";
	menuLines0[linecount++] = "text 163   8   95  3";
	menuLines0[linecount++] = "text 189   8  152  4";
	menuLines0[linecount++] = "text 201   8   95  5";

	menuLines0[linecount++] = "text  13  42 1153  6";
	menuLines0[linecount++] = "text  25  42   95  7";

	if ( withWebInterface )
	{
		menuLines0[linecount++] = "tilepic 286  8 3811"; 		// Race info on web
		menuLines0[linecount++] = "button 294 30 1209 1210 1 0 0";
	}

	for( line=0; line < linecount; line++)
		if ( menuLines0[line].empty() )
			length += menuLines0[line].size() + 4;

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "oxwizard";
	menuLines1[linecount1++] = "R";
	menuLines1[linecount1++] = "ace";
	menuLines1[linecount1++] = "S";
	menuLines1[linecount1++] = "ystem";

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "ot implemented yet";

	textlines = 0;
	length2 = length + 1;

	for( line = 0; line < linecount1; line++)
	{
		if ( menuLines1[line].empty() )
			menuLines1[line] = " ";
		length += menuLines1[line].size() * 2 + 2;
		++textlines;
	}

	length += 24;

	unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	gump1[1]  = length >> 8;
	gump1[2]  = length % 256;
	gump1[7]  = 0;
	gump1[8]  = 0;
	gump1[9]  = 0;
	gump1[10] = 121; // Gump Number (type)
	gump1[19] = length2>>8;
	gump1[20] = length2%256;
	Xsend(socket, gump1, 21);

	for( line = 0; line < linecount; line++ )
	{
		sprintf(buffer, "{ %s }", menuLines0[line].c_str() );
		Xsend(socket, buffer, strlen(buffer) );
	}

	unsigned char gump2[4]="\x00\x00\x00";
	gump2[1]  = textlines >> 8;
	gump2[2]  = textlines % 256;

	Xsend(socket, gump2, 3);

	unsigned char gump3[3]="\x00\x00";
	for( line = 0;line < linecount1; line++ )
	{
		size = menuLines1[line].size();
		gump3[0] = size >> 8;
		gump3[1] = size % 256;
		Xsend(socket, gump3, 2);
		gump3[0]=0;
		for (i = 0; i < size; i++ )
		{
			gump3[1] = menuLines1[line][i];
			Xsend(socket, gump3, 2);
		}
	}

}

//void Race::showRaceNameList( const NXWSOCKET socket, const RACETYPE raceType, short int gumpId, bool canClose, bool canMove, bool withOk )
void Race::showRaceNameList( const NXWSOCKET socket, const RACETYPE raceType, BYTE gumpId, bool canClose, bool canMove, bool withOk )
{
	UI32	i,
		k,
		linecount	= 0,
		linecount1	= 0,
		pagenum		= 1,
		position	= 62,
		linenum		= 9,	// index start to racename strings in text tags
		buttonnum	= 7,
		length		= 0,
		length2		= 0,
		line,
		textlines,
		maxRaceId	= 0,
		raceCount	= raceMap.size(),
		size;

	char	buffer[512];
	string	*menuLines0	= new string[raceCount * 2 + 50];
	string	*menuLines1	= new string[raceCount * 2 + 50];

	std::map<UI32, class Race*>::iterator iter, begin( raceMap.begin() ), end( raceMap.end() );	// For now only indexing by race # is possible
																																															// Must add static reference map < racename, &Race*> to class
																																															// then races may easily be listed alphabetically
	menuLines0[linecount++] = "page 0";
	if ( !canClose )
		menuLines0[linecount++] = "noclose";
	if ( !canMove )
		menuLines0[linecount++] = "nomove";
	menuLines0[linecount++] = "resizepic 0 0 5120 320 340";    	// The background
	menuLines0[linecount++] = "gumppic 20 80 1418";
	if ( withOk )
		menuLines0[linecount++] = "button 10 300 2130 2129 1 0 999"; 	// OKAY
	menuLines0[linecount++] = "text  73   8  152  0";	// Header text 0..5
	menuLines0[linecount++] = "text  84   8   95  1";
	menuLines0[linecount++] = "text 151   8  152  2";
	menuLines0[linecount++] = "text 162   8   95  3";
	menuLines0[linecount++] = "text 189   8  152  4";
	menuLines0[linecount++] = "text 200   8   95  5";
	menuLines0[linecount++] = "text  20  32 1153  6";	// Prompt user to select race
	menuLines0[linecount++] = "text  31  32   95  7";

	menuLines0[linecount++] = "text  78 300   95  8";	// Footer
	sprintf(buffer, "page %i", pagenum);
	menuLines0[linecount++] = buffer;

	k=0;
	raceCount = 0;

	for( iter = begin; iter != end; ++iter )
	{
		if ( iter->second->id > maxRaceId )
			maxRaceId = iter->second->id;

		if ( ( iter->second->activeRace ) && ( PCNPCRACE == iter->second->raceType  || raceType == iter->second->raceType ) )
		{
			if( k > 0 && ( !( k%10 ) ) )
			{
				position=62;
				pagenum++;
				sprintf(buffer, "page %i", pagenum);
				menuLines0[linecount++] = buffer;
			}

			k++;
			sprintf(buffer, "text 40 %i 1153 %i", position - 4, linenum++ );
			menuLines0[linecount++] = buffer; // racenames - first letter
			sprintf(buffer, "text 51 %i 95 %i", position - 4, linenum++ );
			menuLines0[linecount++] = buffer; // racenames - rest of name

			sprintf(buffer, "button 20 %i 1209 1210 1 0 %i", position, iter->first );
		  	menuLines0[linecount++] = buffer;

			position+=20;
			buttonnum++;
			raceCount++;
		}
	}

	if ( withWebInterface )
	{
		menuLines0[linecount++] = "tilepic 286  8 3811"; 		// Race info on web
		sprintf( buffer, "button 294 30 1209 1210 1 0 %d", maxRaceId + 1 );
		menuLines0[linecount++] = buffer;
	}

	pagenum=1; //lets make some buttons
	for (i = 0;i < k;i += 10)
	{
		sprintf(buffer, "page %i", pagenum);
		menuLines0[linecount++] = buffer;
		if (i>=10)
		{
			sprintf(buffer, "button 270 303 2223 2223  0 %i", pagenum - 1);
			menuLines0[linecount++] = buffer; //back button
		}
		if ((k>10) && ((i+10)<k))
		{
			sprintf(buffer, "button 290 303 2224 2224 0 %i", pagenum + 1);
			menuLines0[linecount++] = buffer; //forward button
		}
		pagenum++;
	}

	for( line=0; line < linecount; line++)
		if ( !menuLines0[line].empty() )
			length += menuLines0[line].size() + 4;

	textlines = 0;
	line      = 0;

	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "oxwizard";
	menuLines1[linecount1++] = "R";
	menuLines1[linecount1++] = "ace";
	menuLines1[linecount1++] = "S";
	menuLines1[linecount1++] = "ystem";
	menuLines1[linecount1++] = "C";
	menuLines1[linecount1++] = "hoose a race:";
	sprintf( buffer, "Total races defined: %i", raceCount);
	menuLines1[linecount1++] = buffer;

	for ( iter = begin; iter != end; ++iter )
	{
		if ( ( iter->second->activeRace ) && ( PCNPCRACE == iter->second->raceType || raceType == iter->second->raceType ) )
		{
			menuLines1[linecount1++] = iter->second->name.substr( 0, 1 );
			menuLines1[linecount1++] = iter->second->name.substr( 1 );
		}
	}

	length2 = length + 1;

	for( line = 0; line < linecount1; line++)
	{
		if ( menuLines1[line].empty() )
			menuLines1[line] = " ";
		length += menuLines1[line].size() * 2 + 2;
		++textlines;
	}

	length += 24;

	unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	gump1[1]  = length >> 8;
	gump1[2]  = length % 256;
	gump1[7]  = 0;
	gump1[8]  = 0;
	gump1[9]  = 0;
	gump1[10] = gumpId; // Gump Number (type)
	gump1[19] = length2>>8;
	gump1[20] = length2%256;
	Xsend(socket, gump1, 21);

	for( line = 0; line < linecount; line++ )
	{
		sprintf(buffer, "{ %s }", menuLines0[line].c_str() );
		Xsend(socket, buffer, strlen(buffer) );
	}

	unsigned char gump2[4]="\x00\x00\x00";
	gump2[1]  = textlines >> 8;
	gump2[2]  = textlines % 256;

	Xsend(socket, gump2, 3);

	unsigned char gump3[3]="\x00\x00";
	for( line = 0;line < linecount1; line++ )
	{
		size = menuLines1[line].size();
		gump3[0] = size >> 8;
		gump3[1] = size % 256;
		Xsend(socket, gump3, 2);
		gump3[0]=0;
		for (i = 0; i < size; i++ )
		{
			gump3[1] = menuLines1[line][i];
			Xsend(socket, gump3, 2);
		}
	}

}

//void Race::showRaceDescription( const NXWSOCKET socket, const UI32 raceId, short int gumpId, bool canClose, bool canMove, bool withOk )
void Race::showRaceDescription( const NXWSOCKET socket, const UI32 raceId, BYTE gumpId, bool canClose, bool canMove, bool withOk )
{
	UI32	i,
		k,
		linecount	= 0,
		linecount1	= 0,
		pagenum		= 1,
		position	= 62,
		linenum		= 7,	// index start to racename strings in text tags
		buttonnum	= 7,
		length		= 0,
		length2		= 0,
		line,
		textlines,
		size;

	char	buffer[512];
	string	*menuLines0	= new string[200];	// array size needs to be precalculated to avoid segmentation faults during indexing
	string	*menuLines1	= new string[200];	// as we are using only race descriptions here this can easily be calculated.

	Race*	race = raceMap[raceId];
	std::map<UI32, string*>::iterator iter, begin( race->description.begin() ), end( race->description.end() );
																								// Must add static reference map < racename, &Race*> to class
																								// then races may easily be listed alphabetically
	menuLines0[linecount++] = "page 0";
	if ( !canClose )
		menuLines0[linecount++] = "noclose";
	if ( !canMove )
		menuLines0[linecount++] = "nomove";
	/* Possibilities

	   1) define gump & gumplocation & gumpsize in section global of scripts/race.xss
	   2) calculate y location possibilities from above
	   3) calculate page size
	   4) create a generalized race system ui interface
	   5) create a specialized user race system interface
	   	allows a player to see a race description
	   6) create a specialized superuser race system interface
	   	shows actual race parameters
	   7) directives
	   	^p inserts a new page
	   etc etc
	*/
	menuLines0[linecount++] = "resizepic 0 0 5120 640 340";    	// The background
	menuLines0[linecount++] = "gumppic 340 80 1418";		// Decoration
	if ( withOk )
		menuLines0[linecount++] = "button 10 300 2130 2129 1 0 0"; 	// OKAY
	menuLines0[linecount++] = "text 233   8  152  0";	// Header text 0..5
	menuLines0[linecount++] = "text 244   8   95  1";
	menuLines0[linecount++] = "text 311   8  152  2";
	menuLines0[linecount++] = "text 322   8   95  3";
	menuLines0[linecount++] = "text 349   8  152  4";
	menuLines0[linecount++] = "text 360   8   95  5";
	sprintf( buffer, "text  %i 300  152  6", ( ( 640 - ( raceMap[raceId]->name.size() * 8 ) ) / 2 ) );	// Footer, center race name in background gump
														// needs validation check!!
	if ( withWebInterface )
	{
		menuLines0[linecount++] = "tilepic 606  8 3811"; 		// Race info on web
		sprintf( buffer, "button 614 30 1209 1210 1 0 %d", raceId );
		menuLines0[linecount++] = buffer;
	}


	menuLines0[linecount++] = buffer;
	sprintf(buffer, "page %i", pagenum);
	menuLines0[linecount++] = buffer;

	k=0;
	for( iter = begin; iter != end; ++iter )
	{
		if( k > 0 && ( !( k % 11 ) ) )
		{
			position=62;
			pagenum++;
			sprintf(buffer, "page %i", pagenum);
			menuLines0[linecount++] = buffer;
		}

		k++;
		if ( *iter->second == "^p" )
			while( k > 0 && ( k % 11 ) ) k++;
		else
		{
			if ( iter->second[0] >= "A" && iter->second[0] <= "Z" )
		 	{
				sprintf(buffer, "text 20 %i 1153 %i", position - 4, linenum++ );
				menuLines0[linecount++] = buffer; // Accentuate first letter on beginning of sentence
				sprintf(buffer, "text 31 %i   95 %i", position - 4, linenum++ );
				menuLines0[linecount++] = buffer; // remainder of sentence
		  	}
			else
			{
				sprintf(buffer, "text 20 %i 95 %i", position - 4, linenum++ );
				menuLines0[linecount++] = buffer; // race description line
			}

			position+=20;
			buttonnum++; // should be removed
		}
	}

	pagenum=1; //lets make some damn buttons
	for (i = 0;i < k;i += 11)
	{
		sprintf(buffer, "page %i", pagenum);
		menuLines0[linecount++] = buffer;
		if (i>=11)
		{
			sprintf(buffer, "button 590 303 2223 2223  0 %i", pagenum - 1);
			menuLines0[linecount++] = buffer; //back button
		}
		if ((k>11) && ((i+11)<k))
		{
			sprintf(buffer, "button 610 303 2224 2224 0 %i", pagenum + 1);
			menuLines0[linecount++] = buffer; //forward button
		}
		pagenum++;
	}

	for( line=0; line < linecount; line++)
		if ( !menuLines0[line].empty() )
			length += menuLines0[line].size() + 4;

	textlines = 0;
	line      = 0;

	/*
	The entire Noxwizard Racesystem Header lines should be retrieved from a static Race function
	*/
	menuLines1[linecount1++] = "N";
	menuLines1[linecount1++] = "oxwizard";
	menuLines1[linecount1++] = "R";
	menuLines1[linecount1++] = "ace";
	menuLines1[linecount1++] = "S";
	menuLines1[linecount1++] = "ystem";
	menuLines1[linecount1++] = raceMap[raceId]->name;

	for ( iter = begin; iter != end; ++iter )
	{
		if ( *iter->second != "^p" )
			if ( iter->second[0] >= "A" && iter->second[0] <= "Z" )
			{
				menuLines1[linecount1++] = iter->second->substr( 0, 1 );
				menuLines1[linecount1++] = iter->second->substr( 1 );
			}
			else
				if ( iter->second[0] != "^" )
					menuLines1[linecount1++] = *iter->second;
	}

	length2 = length + 1;
	for( line = 0; line < linecount1; line++)
	{
		if ( menuLines1[line].empty() )
			menuLines1[line] = " ";
		length += menuLines1[line].size() * 2 + 2;
		++textlines;
	}

	length += 24;

	unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	gump1[1]  = length >> 8;
	gump1[2]  = length % 256;
	gump1[7]  = 0;
	gump1[8]  = 0;
	gump1[9]  = 0;
	gump1[10] = gumpId; // Gump Number (type)
	gump1[19] = length2>>8;
	gump1[20] = length2%256;
	Xsend(socket, gump1, 21);

	for( line = 0; line < linecount; line++ )
	{
		sprintf(buffer, "{ %s }", menuLines0[line].c_str() );
		Xsend(socket, buffer, strlen(buffer) );
	}

	unsigned char gump2[4]="\x00\x00\x00";
	gump2[1]  = textlines >> 8;
	gump2[2]  = textlines % 256;

	Xsend(socket, gump2, 3);

	unsigned char gump3[3]="\x00\x00";
	for( line = 0;line < linecount1; line++ )
	{
		size = menuLines1[line].size();
		gump3[0] = size >> 8;
		gump3[1] = size % 256;
		Xsend(socket, gump3, 2);
		gump3[0]=0;
		for (i = 0; i < size; i++ )
		{
			gump3[1] = menuLines1[line][i];
			Xsend(socket, gump3, 2);
		}
	}

}


const std::string* Race::getName( const UI32 raceId )
{
	return &raceMap[raceId]->name;
}

const std::string* Race::getPluralName( const UI32 raceId )
{
	return raceMap[raceId]->getPluralName();
}

const std::string* Race::getPluralName( void )
{
	return &pluralName;
}

void Race::setName( const UI32 raceId, const char *newName )
{
//		raceMap[raceId]->Name = newName;
}

R32 Race::getPoisonResistance( const UI32 raceId, PoisonType poisonStrength )
{
	R32 pr = 0.0;
	if ( isRaceSystemActive() )
	{
		std::map<UI32, class Race*>::iterator raceIter = raceMap.find( raceId );
		if ( raceIter != raceMap.end() )
		{
			if( raceIter->second->isRaceActive() )
				pr = raceMap[raceId]->poisonResistance[poisonStrength];
		}
		else
			WarnOut("Race.cpp getPoisonResistance() called with unknown race %d\n", raceId);
	}
	return pr;
}

Race::Race( void )
{
	activeRace = false;
	skinColor.clear();
	poisonResistance[1] = 0.0;
	poisonResistance[2] = 0.0;
	poisonResistance[3] = 0.0;
	poisonResistance[4] = 0.0;
	poisonResistance[5] = 0.0;
	tileId = 0;
	webLink = "";
	startItems.clear();
	skillCap = SrvParms->skillcap;
}

Race::~Race( void )
{
}

bool Race::isRaceActive( void )
{
	return activeRace;
}

bool Race::isPlayerRace( void )
{
	return ( raceType == PCRACE || raceType == PCNPCRACE );
}

void Race::setRacialRelation( UI32 raceId, RACIALRELATION relation )
{
	m_mapRacialRelations[ raceId ] = relation;
}

RACIALRELATION Race::getRacialRelation( UI32 raceId1, UI32 raceId2 )
{
	std::map<UI32, class Race*>::iterator raceIter = raceMap.find( raceId1 );
	if ( raceIter != raceMap.end() )
		return raceIter->second->getRacialRelation( raceId2 );
	else
		return NEUTRAL;
}

RACIALRELATION Race::getRacialRelation( UI32 raceId )
{
	std::map<UI32, RACIALRELATION>::iterator relationIter = m_mapRacialRelations.find( raceId );

	if ( relationIter != m_mapRacialRelations.end() )
		return m_mapRacialRelations[ raceId ];
	else
		return NEUTRAL;
}

bool Race::isProtectedLayer( UI32 raceId, unsigned char layer )
{
	if( activeRaceSystem && layer )
	{
		Race* race = raceMap.find( raceId )->second;
		if ( race )
			return race->isProtectedLayer( layer );
	}
	return false;
}

bool Race::isProtectedLayer( unsigned char layer )
{
	vector< RaceStartItem >::iterator iter( startItems.begin() ), end( startItems.end() );

	for( ; iter != end; ++iter )
		if( iter->layer == layer && iter->protectedItem )
			return true;

	return false;
}

UI32 Race::getSkillCap()
{
	return skillCap;
}

void Race::setSkillCap( UI32 newSkillCap )
{
	skillCap = newSkillCap;
}

UI32 Race::getSkillAdvanceSuccess( UI32 skillId, UI32 baseSkill )
{
	return skills.getSkill( skillId ).getAdvance( baseSkill ).getSuccess();
}

UI32 Race::getSkillAdvanceFailure( UI32 skillId, UI32 baseSkill )
{
	return skills.getSkill( skillId ).getAdvance( baseSkill ).getFailure();
}

UI32 Race::getSkillAdvanceStrength( UI32 skillId )
{
	return skills.getSkill( skillId).getAdvanceStrength();
}

UI32 Race::getSkillAdvanceDexterity( UI32 skillId )
{
	return skills.getSkill( skillId).getAdvanceDexterity();
}

UI32 Race::getSkillAdvanceIntelligence( UI32 skillId )
{
	return skills.getSkill( skillId).getAdvanceIntelligence();
}

UI32 Race::getStatCap()
{
	return statCap;
}

UI32 Race::getStrCap()
{
	return strCap;
}

int Race::getStrModifier( UI32 baseStr )
{
	return strModifiers.getAbilityModifier( baseStr ).getModifier();
}

UI32 Race::getStrStart()
{
	return strStart;
}

UI32 Race::getDexCap()
{
	return dexCap;
}

int Race::getDexModifier( UI32 baseDex )
{
	return dexModifiers.getAbilityModifier( baseDex ).getModifier();
}

UI32 Race::getDexStart()
{
	return dexStart;
}

UI32 Race::getIntCap()
{
	return intCap;
}

int Race::getIntModifier( UI32 baseInt )
{
	return intModifiers.getAbilityModifier( baseInt ).getModifier();
}

UI32 Race::getIntStart()
{
	return intCap;
}

bool Race::getCanUseSkill( UI32 skillId )
{
	return skills.getSkill( skillId ).getCanUseSkill();
	
}

void Race::show()
{
	SDbgOut("Race %d %s\n", id, name.c_str() );
	skills.show();
}

