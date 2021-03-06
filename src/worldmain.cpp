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
\brief Implementation of the CWorldMain class
*/
#include <fstream>

#include "nxwcommn.h"
#include "worldmain.h"
#include "sndpkg.h"
#include "sregions.h"
#include "debug.h"
#include "amx/amxcback.h"
#include "version.h"
#include "nxw_utils.h"
#include "tmpeff.h"
#include "amx/amxvarserver.h"
#include "layer.h"
#include "jail.h"
#include "accounts.h"
#include "data.h"
#include "boats.h"
#include "books.h"
#include "spawn.h"
#include "set.h"
#include "archive.h"
#include "trade.h"
#include "basics.h"
#include "inlines.h"
#include "items.h"
#include "chars.h"
#include "classes.h"
#include "scripts.h"
#include "skills.h"
#include "utils.h"
#include "nox-wizard.h"
#include "house.h"
#include "race.h"
#include "resourcemap.h"
#include "tmpeff.h"

#ifdef _WIN32
	using namespace std;
#endif

void split( std::string& source, std::string& first, std::string& second )
{
	first = "";
	second = "";

	int i=0;
	int size=source.size();

	for( i=0; i<size; ++i ) {
		if( (  source[i]!=0x09 ) && (  source[i]!=' ' ) )
			break;;
	}

	//now is to first character valid

	for( ; i<size; ++i ) {
		if( source[i]!=' ' )
			first+=source[i];
		else
			break;
	}
	if ( i < size )
		second=source.substr(i+1, source.length()-i);
}


cStringFile::cStringFile( std::string& path, const char* mode )
{
	inStream = new ifstream(path.c_str());
}

cStringFile::~cStringFile()
{
	if ( inStream != NULL )
		delete inStream;
}


void cStringFile::read( std::string& line )
{
	char linebuffer[255];
	line="";
	int byteCount;
	do
	{
		inStream->getline(linebuffer, 255); 
        byteCount = inStream->gcount();
        if (inStream->eof())
		{
			inStream->close();
			break;
		}
		else if (inStream->fail()) 
		{
           inStream->clear(inStream->rdstate() & ~ios::failbit);
		}
		line+=linebuffer;
	}
	while (byteCount >= 254 );  // reepat until line complete

	if( ( line.size()>1 ) && ( line[0]=='/' && line[1]=='/' ) ) //commented
		line="";
}

void cStringFile::read( std::string& first, std::string& second )
{

	first=""; second="";

	std::string line;
	read( line );

	split( line, first, second );

}

void cStringFile::read( std::string& first, std::string& second, std::string& third )
{
	std::string b;
	read( first, b );
	split( b, second, third );
}


bool cStringFile::eof()
{
	return (inStream!=NULL )? inStream->eof() || ! inStream->is_open() : true;
}



















//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWorldMain* cwmWorldState=NULL;


CWorldMain::CWorldMain()
{
	isSaving = false;
	this->cWsc=NULL;
	this->gWsc=NULL;
	this->iWsc=NULL;
	this->jWsc=NULL;
	this->chr_curr=1;
	this->itm_curr=1;
}

CWorldMain::~CWorldMain()
{
	if (iWsc)
		fclose(iWsc);
	if (cWsc)
		fclose(cWsc);
	if (gWsc)
		fclose(gWsc);
	if (jWsc)
		fclose(jWsc);
}


/*
\brief read from a file an unicode string
\author Endymion
\note intput is like 00AE001232120000, hex ( Big Endian Format )
\param f the file
\param name the variable name
\return the unicode string
*/
wstring HexVector2UnicodeString( char* s )
{
	std::wstring w;

	int i=0;
	char temp[7] = { '0','x', '0','0','0','0','\0'};
	char *dummy;
	int stringLength=strlen(s);
	wchar_t baffer=0;
	do {
		memcpy( &temp[2], &s[i], 4 );
		baffer = static_cast<wchar_t>( strtol( temp, &dummy, 0 ) );
		if( baffer!=0 )
			w += baffer;
		i+=4;
	} while ( baffer!=0 && i < stringLength);
	return w;
}

void CWorldMain::loadChar() // Load a character from WSC
{
	P_CHAR pc = archive::character::Instance();
	if ( pc == 0 )
		return;
	//
	// Sparhawk:	Fix for 0.73 worldfiles	where players can no longer use their characters
	//		because removed player characters in 0.73 have ACCOUNT -1
	//		DO NOT REMOVE
	//
	LOGICAL hasAccount 		= false;
	LOGICAL hasInvalidAccount 	= false;
	LOGICAL hasCreationDay		= false;
	//
	//
	//
	int	i;
	int	j,
		loops = 0;

	pc->hungertime=uiCurrentTime+(SrvParms->hungerrate*MY_CLOCKS_PER_SEC/2);    // avoid loss of one hungerpoint for each restart (Duke, 6.6.2001)

	amxVS.setServerMode();
	do
	{
		readw2();
		switch(script1[0])
		{
		case '@':
			pc->loadEventFromScript(script1, script2);
			break;
		case 'A':
		case 'a':
			//
			// CONVERT OLD STYLE AMXFLAGS TO NEW STYLE
			//
			if ( !strcmp( "AMXFLAG0", script1 ) )
				amxVS.insertVariable( INVALID, 0, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG1", script1 ) )
				amxVS.insertVariable( INVALID, 1, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG2", script1 ) )
				amxVS.insertVariable( INVALID, 2, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG3", script1 ) )
				amxVS.insertVariable( INVALID, 3, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG4", script1 ) )
				amxVS.insertVariable( INVALID, 4, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG5", script1 ) )
				amxVS.insertVariable( INVALID, 5, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG6", script1 ) )
				amxVS.insertVariable( INVALID, 6, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG7", script1 ) )
				amxVS.insertVariable( INVALID, 7, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG8", script1 ) )
				amxVS.insertVariable( INVALID, 8, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG9", script1 ) )
				amxVS.insertVariable( INVALID, 9, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGA", script1 ) )
				amxVS.insertVariable( INVALID, 10, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGB", script1 ) )
				amxVS.insertVariable( INVALID, 11, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGC", script1 ) )
				amxVS.insertVariable( INVALID, 12, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGD", script1 ) )
				amxVS.insertVariable( INVALID, 13, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGE", script1 ) )
				amxVS.insertVariable( INVALID, 14, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGF", script1 ) )
				amxVS.insertVariable( INVALID, 15, str2num( script2 ) );
			//
			// READ NEW STYLE AMX VARS
			//
			else if ( !strcmp( "AMXINT", script1 ) )
			{
				splitLine( script2, script1, script3 );
				amxVS.insertVariable( INVALID, str2num( script1 ), str2num( script3 ) );
			}
			else if ( !strcmp( "AMXINTVEC", script1 ) )
			{
				splitLine( script2, script1, script3 );
				SI32 vectorId = str2num( script1 );
				SI32 vectorSize = str2num( script3 );
				amxVS.insertVariable( INVALID, vectorId, vectorSize, 0 );
				SI32 vectorIndex = 0;
				do
				{
					readw2();
					switch( script1[0] )
					{
						case '{' :
						case '}' :
							break;
						default	 :
							if( vectorIndex < vectorSize )
							{
								amxVS.updateVariable( INVALID, vectorId, vectorIndex, str2num( script1 ) );
								++vectorIndex;
							}
							break;
					}
				}
				while( script1[0] != '}' );
				strcpy( script1, "DUMMY" );
			}
			else if (!strcmp("AMXSTR", script1) )
			{
				splitLine( script2, script1, script3 );
				amxVS.insertVariable( INVALID, str2num( script1 ), script3 );
			}
			//
			//
			//
			else if (!strcmp(script1, "ACCOUNT"))
			{
				hasAccount = true;
				if( str2num( script2 ) == -1 )
					//
					// Sparhawk:	Fix for 0.73 worldfiles	where players can no longer use their characters
					//		because removed player characters in 0.73 have ACCOUNT -1
					//		DO NOT REMOVE
					//
					hasInvalidAccount = true;
				else
					Accounts->AddCharToAccount( str2num(script2), pc );
				if ( str2num( script2 ) == 0 )
					pc->setPrivLevel(PRIVLEVEL_ADMIN);
			}
			else if (!strcmp(script1, "ALLMOVE"))
				pc->SetPriv2((signed char)str2num(script2));
			else if (!strcmp(script1, "ATT"))
				pc->att=str2num(script2);
			else if (!strcmp(script1, "ADVOBJ"))
				pc->advobj=str2num(script2);
				break;

		case 'B':
		case 'b':
			if (!(strcmp(script1, "BODY")))
			{
				UI16 body = (unsigned short)str2num(script2);
				pc->setId( body );
				pc->setOldId( body );
			}
		break;

		case 'C':
		case 'c':
			if (!strcmp(script1, "CREATIONDAY"))
			{
				//
				// Sparhawk:	Fix for 0.73 worldfiles	where players can no longer use their characters
				//		because removed player characters in 0.73 have ACCOUNT -1
				//		DO NOT REMOVE
				//
				hasCreationDay = true;
				//
				pc->SetCreationDay(str2num(script2));
			}
			else if (!strcmp(script1, "CARVE"))			{ pc->carve=str2num(script2); }
			else if (!strcmp(script1, "CANTRAIN"))		{ pc->cantrain = true; }
		break;

		case 'D':
		case 'd':
			if (!strcmp(script1, "DISPZ"))				{  pc->setPosition("dZ", str2num(script2));}
			else if (!strcmp(script1, "DAMAGETYPE"))		{ pc->damagetype= static_cast<DamageType>(str2num(script2));}	//Luxor
			else if (!strcmp(script1, "DIR"))			{ pc->dir=(signed char)str2num(script2);}
			else if (!strcmp(script1, "DEXTERITY"))		{ pc->dx=str2num(script2); pc->dx3=pc->dx;}
			else if (!strcmp(script1, "DEXTERITY2"))		{ pc->dx2=str2num(script2); }
			else if (!strcmp(script1, "DEATHS"))			{ pc->deaths=str2num(script2);  }
			else if (!strcmp(script1, "DEAD"))			{ pc->dead = true; } // dead only appears in file if it's true.
			else if (!strcmp(script1, "DEF"))			{ pc->def=str2num(script2);}
			else if (!strcmp(script1, "DISABLED"))		{ pc->disabled=uiCurrentTime+(str2num(script2)*MY_CLOCKS_PER_SEC);}//AntiChrist
			else if (!strcmp(script1, "DISABLEMSG"))	{ pc->disabledmsg = new std::string( script2 );}
			else if (!strcmp(script1, "DOORUSE"))			{ pc->doorUse=str2num(script2);}
		break;

		case 'E':
		case 'e':
			if (!strcmp(script1, "EMOTE"))
			{
				pc->emotecolor = (unsigned short)str2num(script2);
			}
		break;

		case 'F':
		case 'f':
			if (!strcmp(script1, "FAME"))				{ pc->SetFame(str2num(script2)); }
			else if (!strcmp(script1, "FIXEDLIGHT"))		{ pc->fixedlight=(unsigned char)str2num(script2); }
			else if (!strcmp(script1, "FONT"))			{ pc->fonttype=(unsigned char)str2num(script2);}
			else if (!strcmp(script1, "FLEEAT"))			{ pc->fleeat=str2num(script2);}
			else if (!strcmp(script1, "FX1"))			{ pc->fx1=str2num(script2);}
			else if (!strcmp(script1, "FY1"))			{ pc->fy1=str2num(script2);}
			else if (!strcmp(script1, "FZ1"))			{ pc->fz1=(signed char)str2num(script2);}
			else if (!strcmp(script1, "FX2"))			{ pc->fx2=str2num(script2);}
			else if (!strcmp(script1, "FY2"))			{ pc->fy2=str2num(script2);}
			else if (!strcmp(script1, "FOODX"))			{pc->foodloc.x=(UI16)str2num(script2);}
			else if (!strcmp(script1, "FOODY"))			{pc->foodloc.y=(UI16)str2num(script2);}
			else if (!strcmp(script1, "FOODZ"))			{pc->foodloc.z=(SI08)str2num(script2);}
			else if (!strcmp( script1, "FOLLOWSPEED" ) ){ pc->npcFollowSpeed = (float)atof( script2 ); }


		break;

		case 'G':
  case 'g':
   if (!strcmp(script1, "GUILDTYPE"))
   {
    i = str2num(script2);  // Get the guild type
    if(i<INVALID||i>MAX_GUILDTYPE)
     i=INVALID;     // Default to no guild type
    pc->SetGuildType( static_cast<short>(i) );
   }
   else if (!strcmp(script1, "GUILDTRAITOR"))
   {
    if(!strcmp(script2, "YES"))
     pc->SetGuildTraitor(); // yes this character has switch guilds
   }
   else if (!strcmp(script1, "GUILDTOGGLE")) { (str2num(script2) ? pc->SetGuildTitleToggle() : pc->ResetGuildTitleToggle() );}
   else if (!strcmp(script1, "GUILDNUMBER")) { pc->SetGuildNumber( str2num(script2) );}
   else if (!strcmp(script1, "GMRESTRICT"))  { pc->gmrestrict=(unsigned char)str2num(script2); }
   else if (!strcmp(script1, "GUILDTITLE"))  { pc->SetGuildTitle(script2);}
   else if (!strcmp(script1, "GUILDFEALTY")) { pc->SetGuildFealty( str2num(script2) );}
   //////////////////////////////////
   // Reads in the Gm movement effect int
   //
   //
   else if (!strcmp(script1, "GMMOVEEFF")) { pc->gmMoveEff=str2num(script2);}

   //
   //
   //Aldur
   //////////////////////////////////
  break;

		case 'H':
		case 'h':
			if (!strcmp(script1, "HITPOINTS"))			{ pc->hp=str2num(script2);}
			//Luxor: players vendors fix
			else if (!strcmp(script1, "HOLDG"))		{ pc->holdg=str2num(script2);}
			else if (!strcmp(script1, "HIDAMAGE"))		{ pc->hidamage=str2num(script2);}
			else if (!strcmp(script1, "HIDDEN"))			{ pc->hidden=(unsigned char)str2num(script2);}
			else if (!strcmp(script1, "HUNGER"))			{ pc->hunger=str2num(script2);}
			else if (!strcmp(script1, "HOLDGOLD"))		{ pc->holdg=str2num(script2);}
			else if (!strcmp(script1, "HOMEX"))			{ pc->homeloc.x=(UI16)str2num(script2);}
			else if (!strcmp(script1, "HOMEY"))			{ pc->homeloc.y=(UI16)str2num(script2);}
			else if (!strcmp(script1, "HOMEZ"))			{ pc->homeloc.z=(SI08)str2num(script2);}
			else if (!strcmp(script1, "HIRE"))			{ pc->setHireFee(str2num(script2));}
		break;

		case 'I':
		case 'i':
			if (!strcmp(script1, "INTELLIGENCE"))		{ pc->in=str2num(script2);pc->in3=pc->in;}
			else if (!strcmp(script1, "INTELLIGENCE2"))	{ pc->in2=str2num(script2);}
		break;

		case 'K':
		case 'k':
			if (!strcmp(script1, "KARMA"))				{ pc->SetKarma(str2num(script2));  }
			else if (!strcmp(script1, "KILLS"))			{ pc->kills=str2num(script2); }
		break;

		case 'L':
		case 'l':
//			if (!strcmp(script1, "LASTON"))				{ pc->laston=strdup(script2);/*strcpy(dummy.laston,pc->laston);*/} //load last time character was on
			if (!strcmp(script1, "LODAMAGE"))		{ pc->lodamage=str2num(script2);}
			else if ( !strcmp( "LOOT", script1 ) )
			{
				do
				{
					readw2();
					switch( script1[0] )
					{
						case '{' :
						case '}' :
							break;
						default	 :
							pc->lootVector.push_back( str2num( script1 ) );
							break;
					}
				}
				while( script1[0] != '}' );
				strcpy( script1, "DUMMY" );
			}
		break;

		case 'M':
		case 'm':
			if (!(strcmp("MAGICSPHERE",script1)))	{pc->magicsphere = str2num(script2);  }
			else if (!strcmp(script1, "MANA"))			{ pc->mn=str2num(script2);}
			else if ( !strcmp( script1, "MOVESPEED" ) ) { pc->npcMoveSpeed = (float)atof( script2 ); }
			else if ( !strcmp(script1, "MURDERRATE") )	{ pc->murderrate= str2num(script2);  }
			else if ( !strcmp(script1, "MOUNTED") )		{ pc->mounted= true;  }
			else if ( !strcmp(script1, "MURDERSAVE") )	{ pc->murdersave= str2num(script2);  }
		break;

		case 'N':
		case 'n':
			if (!(strcmp("NXWFLAG0",script1)))					{pc->nxwflags[0] = (char)str2num(script2);  }
			else if (!(strcmp("NXWFLAG1",script1)))				{pc->nxwflags[1] = (char)str2num(script2);  }
			else if (!(strcmp("NXWFLAG2",script1)))				{pc->nxwflags[2] = (char)str2num(script2);  }
			else if (!(strcmp("NXWFLAG3",script1)))				{pc->nxwflags[3] = (char)str2num(script2);  }
			else if (!strcmp(script1, "NPC"))			{ pc->npc=(char)str2num(script2);}
			else if (!strcmp(script1, "NPCWANDER"))		{ pc->npcWander=(char)str2num(script2);}
			else if (!strcmp(script1, "NOTRAIN"))		{ pc->cantrain=false;}
			else if (!strcmp(script1, "NPCTYPE"))		{ pc->npc_type=(char)str2num(script2);  }
//			else if (!strcmp(script1, "NAME"))			{ strcpy(pc->name,script2); }
			else if (!strcmp(script1, "NAME"))			{ pc->setCurrentName(script2);}
			else if (!strcmp(script1, "NPCAITYPE"))		{ pc->npcaitype=str2num(script2);}
		break;

		case 'O':
		case 'o':
			if (!strcmp(script1, "OLDNPCWANDER"))		{ pc->oldnpcWander=(char)str2num(script2);}
			else if (!strcmp(script1, "OLDX"))			{ pc->setOldPosition("x", str2num(script2)); }
			else if (!strcmp(script1, "OLDY"))			{ pc->setOldPosition("y", str2num(script2)); }
			else if (!strcmp(script1, "OLDZ"))			{ pc->setOldPosition("z", str2num(script2)); }
			else if (!strcmp(script1, "OWN"))			{ pc->setOwnerSerial32Only(str2num(script2)); }
		break;

		case 'P':
		case 'p':
			if (!strcmp(script1, "PRIV"))				{ pc->SetPriv((unsigned char)str2num(script2));}
			else if (!strcmp(script1, "PRIVLEVEL"))				{ pc->setPrivLevel((unsigned char)str2num(script2));}
			else if (!strcmp(script1, "POISON"))			{ pc->poison=str2num(script2);}
			else if (!strcmp(script1, "POISONED"))		{ pc->poisoned=(PoisonType)str2num(script2);}

			else if (!strcmp( script1, "PC_FTARG" ) )   { P_CHAR temp=MAKE_CHAR_REF(str2num(script2)); pc->ftargserial = ISVALIDPC(temp)? temp->getSerial32() : INVALID;} //legacy code
			else if (!strcmp( script1, "PC_FTARGSER" ) )   {pc->ftargserial=str2num(script2); }
			else if (!strcmp( script1, "POSSESSEDSERIAL" ) )   {pc->possessedSerial=str2num(script2); }
			else if (!(strcmp(script1, "PROFILE"))) {
				pc->profile = HexVector2UnicodeString( script2 );
			}
		break;

		case 'Q':
		case 'q':
			if (!strcmp(script1, "QUESTTYPE"))			  pc->questType=(MsgBoards::QuestType)str2num(script2);
			else if (!strcmp(script1, "QUESTDESTREGION"))  pc->questDestRegion=str2num(script2);
			else if (!strcmp(script1, "QUESTORIGREGION"))  pc->questOrigRegion=str2num(script2);
			else if (!strcmp(script1, "QUESTBOUNTYPOSTSERIAL")) pc->questBountyPostSerial=str2num(script2);
			else if (!strcmp(script1, "QUESTBOUNTYREWARD")) pc->questBountyReward=str2num(script2);
		break;

		case 'R':
		case 'r':
			if (!strcmp(script1, "RACE"))				pc->setRace((SI32)str2num(script2));
			else if (!strcmp(script1, "ROBE"))				pc->robe = str2num(script2);
			else if (!strcmp(script1, "REATTACKAT"))		{ pc->reattackat=str2num(script2);}
			else if (!strcmp(script1, "REGEN_HP"))		{ UI32 v=str2num(script2); pc->setRegenRate( STAT_HP, v, VAR_REAL ); pc->setRegenRate( STAT_HP, v, VAR_EFF ); }
			else if (!strcmp(script1, "REGEN_ST"))		{ UI32 v=str2num(script2); pc->setRegenRate( STAT_STAMINA, v, VAR_REAL ); pc->setRegenRate( STAT_STAMINA, v, VAR_EFF ); }
			else if (!strcmp(script1, "REGEN_MN"))		{ UI32 v=str2num(script2); pc->setRegenRate( STAT_MANA, v, VAR_REAL ); pc->setRegenRate( STAT_MANA, v, VAR_EFF ); }
			else if (!(strcmp("RESISTS", script1)))
			{// Added by Luxor
				int params[2];
				fillIntArray(script2, params, 2, 0, 10);
				if (params[0] < MAX_RESISTANCE_INDEX)
					pc->resists[params[0]] = params[1];
			}
		break;

		case 'S':
		case 's':
			if( !strcmp(script1, "SCRIPTID") )
				pc->setScriptID(str2num(script2));
			else if (!strcmp(script1, "SERIAL"))
			{
				i = str2num(script2);
				//if (charcount2<=i) charcount2=i+1;
				pc->setSerial32(i);
				objects.updateCharSerial( i );
//				setptr(&charsp[i%HASHMAX], x); //Load into charsp array
			}
			else if (!(strcmp(script1, "SPAWNSERIAL")))
				pc->setSpawnSerial(str2num(script2));
			else if (!(strcmp(script1, "SPAWNREGION")))
				pc->setSpawnRegion(str2num(script2));
			else if (!strcmp(script1, "SAY"))
			{
				pc->saycolor = (unsigned short)str2num(script2);
			}
			else if (!strcmp(script1, "STRENGTH"))		{ pc->setStrength(str2num(script2), false);pc->st3=pc->getStrength();}
			else if (!strcmp(script1, "STRENGTH2"))		{ pc->st2=str2num(script2);}
			else if (!strcmp(script1, "STAMINA"))		{ pc->stm=str2num(script2);}
			else if (!strcmp(script1, "SUMMONREMAININGSECONDS"))
			{//AntiChrist - let's restore remaining seconds!
				i=str2num(script2);

				pc->summontimer=uiCurrentTime+(i*MY_CLOCKS_PER_SEC);

			}
//			else if (!strcmp(script1, "SHOP"))			{ pc->shopkeeper;} // no effect
			else if (!strcmp(script1, "SPEECH"))			{ pc->speech=(char)str2num(script2); }
			else if ((script1[0]=='S')&&(script1[1]=='K')&&(script1[2]=='I')&&
				(script1[3]=='L')&&(script1[4]=='L'))
			{
				pc->baseskill[j=str2num(&script1[5])]=(unsigned short)str2num(script2);
				Skills::updateSkillLevel(pc, j);
			}
			else if (!strcmp(script1, "SKIN"))
			{
				pc->setColor((unsigned short)str2num(script2));

			}
			else if (!strcmp(script1, "SPATTACK"))		{ pc->spattack=str2num(script2);}
			else if (!strcmp(script1, "SPADELAY"))		{ pc->spadelay=str2num(script2);}
			else if (!strcmp(script1, "SPLIT"))			{ pc->split=str2num(script2);}
			else if (!strcmp(script1, "SPLITCHANCE"))	{ pc->splitchnc=str2num(script2);}
			else if (!strncmp(script1, "SKL", 3 ))		// for skill locking
			{
				pc->lockSkill[j=str2num(&script1[3])]=(unsigned char)str2num(script2);
			}
			else if (!strcmp(script1, "STABLEMASTER"))
			{
				pc->stable( pointers::findCharBySerial( str2num(script2) ) );
			}
		break;

		case 'T':
		case 't':
			if (!strcmp(script1, "TEMPFX"))				
			{ 
				tempfx::cTempfx *fx = new tempfx::cTempfx();
				fx->load();
				pc->addTempfx(fx);
				script1[0]='\0';
			}
			else if (!strcmp(script1, "TITLE"))				{ pc->title =script2; }
			else if (!strcmp(script1, "TAMING"))			{ pc->taming=str2num(script2);}
			else if (!strcmp(script1, "TRIGGER"))		{ pc->trigger=str2num(script2);;}
			else if (!strcmp(script1, "TRIGWORD"))		{ pc->trigword=script2;}
			else if (!strcmp(script1, "TIME_UNUSED"))	{ pc->time_unused=str2num(script2);}
		break;

		case 'X':
		case 'x':
			if (!strcmp(script1, "X"))					{ pc->setPosition("x", str2num(script2)); }
			else if (!strcmp(script1, "XBODY"))
			{
				pc->setOldId((unsigned short) str2num(script2) );
			}
			else if (!strcmp(script1, "XSKIN"))
			{
				pc->setOldColor( (unsigned short)str2num(script2) );
			}
		break;

		case 'W':
		case 'w':
			if (!strcmp(script1, "WAR"))					{ pc->war=(char)str2num(script2);}
			else if (!strcmp(script1, "WORKX"))			{ pc->workloc.x=(UI16)str2num(script2);}
			else if (!strcmp(script1, "WORKY"))			{ pc->workloc.y=(UI16)str2num(script2);}
			else if (!strcmp(script1, "WORKZ"))			{ pc->workloc.z=(SI08)str2num(script2);}
		break;

		case 'Y':
		case 'y':
			if (!strcmp(script1, "Y"))					{ pc->setPosition("y", str2num(script2)); }
		break;

		case 'Z':
		case 'z':
			if (!strcmp(script1, "Z"))
			{
				pc->setPosition("z", str2num(script2));
				pc->setPosition("dZ", str2num(script2));

			}
		break;
		}
		loops++;
 }  while (strcmp(script1, "}") && loops<=200);

	amxVS.setUserMode();

	//
	// Sparhawk:	Fix for 0.73 worldfiles	where players can no longer use their characters
	//		because removed player characters in 0.73 have ACCOUNT -1
	//		DO NOT REMOVE
	//
	if( !pc->npc && hasInvalidAccount && hasCreationDay )
	{
		safedelete(pc);
		return;
	}
	//
	// Sparhawk:	Fix for 0.82 worldfiles allready processed by 0.82 server before this conversion patch was added
	//		DO NOT REMOVE
	//
	if( !pc->npc && !hasAccount && hasCreationDay )
	{
		safedelete(pc);
		return;
	}
	//
	//
	pc->region=static_cast<unsigned char>(calcRegionFromXY( pc->getPosition() )); //LB bugfix
#ifdef SPAR_C_LOCATION_MAP
	//
	// Hmmm...is not needed here because lowerdown a move to is done!
	// will correct this later
	pointers::addToLocationMap(pc);
#else
	mapRegions->add(pc);
#endif

 // begin of meta gm stuff

	if (pc->npc)
	{
		pc->war=0;
		if(pc->mounted)
			pointers::pMounted[pc->getOwnerSerial32()]=pc;
// counter variables for html
		npcCount+=1;
	}
	else
	{
		// counter variables for html
		charCount +=1;
	}

	UI16 max_x = map_width  * 8;
	UI16 max_y = map_height * 8;
	Location pcpos= pc->getPosition();

	if( ((pcpos.x<100) && (pcpos.y<100)) || ((pcpos.x>max_x) || (pcpos.y>max_y) || (pcpos.x<0) || (pcpos.y<0)))
	{
		if( !pc->npc )
			pc->MoveTo( 900,300,30 ); //player in an invalid location
		else
			pc->Delete();
	}

	setcharflag2(pc);//AntiChrist
// Set gender variable according to race system or normal system 
	if ( Race::isRaceSystemActive() )
	{
		if ( pc->getRace() > 0 )
		{
		// Lookup gender here, but this is not possible with current code	
		}
	}
	else
	{
		if ( pc->getId() == BODY_MALE || pc->getId() == BODY_DEADMALE )
			pc->setGender(GENDER_MALE);
		else if ( pc->getId() == BODY_FEMALE || pc->getId() == BODY_DEADFEMALE )
			pc->setGender(GENDER_FEMALE);
		else
			pc->setGender(GENDER_NEUTRAL);
	}
	amxVS.moveVariable( INVALID, pc->getSerial32() );

}

//! Load an item from WSC
void loaditem()
{
	P_ITEM pi = archive::item::New( true );

	if( pi == 0 )
		return;

	unsigned long int i;
	int loops=0;
	char bad=0;

	amxVS.setServerMode();
	do
	{
		readw2();
		switch (script1[0])
		{
		case '@': LoadItemEventsFromScript (pi, script1, script2); break;

		case 'a':
		case 'A':
			//
			// CONVERT OLD STYLE AMXFLAGS TO NEW STYLE
			//
			if ( !strcmp( "AMXFLAG0", script1 ) )
				amxVS.insertVariable( INVALID, 0, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG1", script1 ) )
				amxVS.insertVariable( INVALID, 1, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG2", script1 ) )
				amxVS.insertVariable( INVALID, 2, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG3", script1 ) )
				amxVS.insertVariable( INVALID, 3, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG4", script1 ) )
				amxVS.insertVariable( INVALID, 4, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG5", script1 ) )
				amxVS.insertVariable( INVALID, 5, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG6", script1 ) )
				amxVS.insertVariable( INVALID, 6, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG7", script1 ) )
				amxVS.insertVariable( INVALID, 7, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG8", script1 ) )
				amxVS.insertVariable( INVALID, 8, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAG9", script1 ) )
				amxVS.insertVariable( INVALID, 9, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGA", script1 ) )
				amxVS.insertVariable( INVALID, 10, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGB", script1 ) )
				amxVS.insertVariable( INVALID, 11, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGC", script1 ) )
				amxVS.insertVariable( INVALID, 12, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGD", script1 ) )
				amxVS.insertVariable( INVALID, 13, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGE", script1 ) )
				amxVS.insertVariable( INVALID, 14, str2num( script2 ) );
			else  if ( !strcmp( "AMXFLAGF", script1 ) )
				amxVS.insertVariable( INVALID, 15, str2num( script2 ) );
			else if (!strcmp("AMXINT", script1) )
			{
				splitLine( script2, script1, script3 );
				amxVS.insertVariable( INVALID, str2num( script1 ), str2num( script3 ) );
			}
			else if ( !strcmp( "AMXINTVEC", script1 ) )
			{
				splitLine( script2, script1, script3 );
				SI32 vectorId = str2num( script1 );
				SI32 vectorSize = str2num( script3 );
				amxVS.insertVariable( INVALID, vectorId, vectorSize, 0 );
				SI32 vectorIndex = 0;
				do
				{
					readw2();
					switch( script1[0] )
					{
						case '{' :
							continue;
						case '}' :
							break;
						default	 :
							if( vectorIndex < vectorSize )
							{
								amxVS.updateVariable( INVALID, vectorId, vectorIndex, str2num( script1 ) );
								++vectorIndex;
							}
							break;
					}
				}
				while( script1[0] != '}' );
			}
			else if (!strcmp("AMXSTR", script1) )
			{
				splitLine( script2, script1, script3 );
				amxVS.insertVariable( INVALID, str2num( script1 ), script3 );
			}
			else if (!(strcmp(script1, "AMOUNT")))
				pi->amount=(unsigned short)str2num(script2);
			else if (!(strcmp(script1, "AMOUNT2")))
				pi->amount2=(unsigned short)str2num(script2);
			else if (!(strcmp(script1, "ATT")))
				pi->att=str2num(script2);
			else if (!strcmp("ANIMID", script1))
				//pi->animSetId(str2num(script2));
				break; // disable anim id
			else if (!strcmp("AUXDAMAGE", script1))
				pi->auxdamage = str2num(script2);
  			else if (!strcmp("AUXDAMAGETYPE", script1))
				pi->auxdamagetype = static_cast<DamageType>(str2num(script2));
			else if (!strcmp("AMMO", script1))
				pi->ammo = str2num(script2);
			else if (!strcmp("AMMOFX", script1))
				pi->ammoFx = str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'c':
		case 'C':
			if (!(strcmp(script1, "COLOR")))
			{
				i=str2num(script2);

				if ( !((i&0x4000) || (i&0x8000)) || (i == 32767))
				{
					pi->setColor( static_cast<COLOR>(i) );
				}
				else
				{
					pi->setColor( 0 ); // bugged color found, leave it undyed
					WarnOut("item# %i with problematic hue corrected\n", pi->getSerial32());
				}
			}
			else if (!(strcmp(script1, "CONT")))
			{
				pi->setContSerial(str2num(script2), false, false);
				if ( (isCharSerial(pi->getContSerial())) && (! ISVALIDPC(pointers::findCharBySerial(pi->getContSerial())) ) )
					bad=true;
			}
			else if (!(strcmp(script1, "CORPSE")))
				pi->corpse=str2num(script2);
			else if (!(strcmp(script1, "CARVE")))
				pi->carve=str2num(script2);
			else if (!(strcmp(script1, "CREATOR")))
				pi->creator = script2;
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'd':
		case 'D':
			if (!(strcmp(script1, "DISABLEMSG")))
				pi->disabledmsg = new std::string( script2 );
			else if (!(strcmp(script1, "DISABLED")))
				pi->disabled=uiCurrentTime+(str2num(script2)*MY_CLOCKS_PER_SEC);
			else if (!(strcmp(script1, "DIR")))
				pi->dir=(short)str2num(script2);
			else if (!(strcmp(script1, "DOORFLAG")))
				pi->doordir=(signed char)str2num(script2);
			else if (!(strcmp(script1, "DYEABLE")))
				pi->dye=str2num(script2);
			else if (!(strcmp(script1, "DEF")))
				pi->def=str2num(script2);
			else if (!(strcmp(script1, "DESC")))
				//strcpy(pi->desc,script2);
				pi->vendorDescription = script2;
			else if (!(strcmp(script1, "DX")))
				pi->dx=str2num(script2);
			else if (!(strcmp(script1, "DX2")))
				pi->dx2=str2num(script2);
  			else if (!strcmp("DAMAGETYPE", script1))
				pi->damagetype = static_cast<DamageType>(str2num(script2));
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'g':
		case 'G':
			if (!(strcmp(script1, "GOOD")))
				pi->good=str2num(script2);
			/*else if (!(strcmp(script1, "GLOW")))
				pi->glow=str2num(script2);
			else if (!(strcmp(script1, "GLOWBC")))
			{
				i=str2num(script2);
				pi->glow_c1=static_cast<char>((i>>8));
				pi->glow_c2=static_cast<char>(i%256);
			}
			else if (!(strcmp(script1, "GLOWTYPE")))
				pi->glow_effect=str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);*/
			break;

		case 'h':
		case 'H':
			if (!(strcmp(script1, "HIDAMAGE")))
				pi->hidamage=str2num(script2);
			else if (!(strcmp(script1, "HP")))
				pi->hp=str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'i':
		case 'I':
			if (!(strcmp(script1, "IN")))
				pi->in=str2num(script2);
			else if (!(strcmp(script1, "IN2")))
				pi->in2=qmax(0,str2num(script2));
			else if (!(strcmp(script1, "ITEMHAND")))
				pi->itmhand=str2num(script2);
			else if (!(strcmp(script1, "ID")))
			{
				i=str2num(script2);
				//elcabesa bugfix  dummyes swinging
				if( (i==0x1071) || (i==0x1075))
					i--;
				// elcabesa bugfigx end
				pi->setId( static_cast<UI16>(i) );

				if (i>=0x4000)
				{
					multiVector m_vec;
					data::seekMulti( (unsigned short) (i-0x4000), m_vec );
					if( m_vec.empty() )
					{
						LogWarning("bad item, serial: %i name: %s\n",pi->getSerial32(), pi->getCurrentNameC());
						bad=1;
					}
				}

			}
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'l':
		case 'L':
			if (!(strcmp(script1, "LAYER")))
				pi->layer=(signed char)str2num(script2);
			else if (!(strcmp(script1, "LODAMAGE")))
				pi->lodamage=str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'm':
		case 'M':
			if (!(strcmp(script1, "MORE")))
			{
				i=str2num(script2);
				pi->more1=(unsigned char)(i>>24);
				pi->more2=(unsigned char)(i>>16);
				pi->more3=(unsigned char)(i>>8);
				pi->more4=(unsigned char)(i%256);
			}
			else if (!(strcmp(script1, "MORE2")))
			{
				i=str2num(script2);
				pi->moreb1=(unsigned char)(i>>24);
				pi->moreb2=(unsigned char)(i>>16);
				pi->moreb3=(unsigned char)(i>>8);
				pi->moreb4=(unsigned char)(i%256);
			}
			else if (!(strcmp(script1, "MOVABLE")))
				pi->magic=(signed char)str2num(script2);
			else if (!(strcmp(script1, "MAXHP")))
				pi->maxhp=str2num(script2);
			else if (!(strcmp(script1, "MOREX")))
				pi->morex=str2num(script2);
			else if (!(strcmp(script1, "MOREY")))
				pi->morey=str2num(script2);
			else if (!(strcmp(script1, "MOREZ")))
				pi->morez=str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'n':
		case 'N':
			if (!(strcmp(script1, "NAME")))
				pi->setCurrentName(script2);
			else if (!(strcmp(script1, "NAME2")))
				pi->setSecondaryName(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'o':
		case 'O':
			if (!(strcmp(script1, "OWNER")))
				pi->setOwnerSerial32Only(str2num(script2));
			else if (!(strcmp(script1, "OFFSPELL")))
				pi->offspell=(signed char)str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'p':
		case 'P':
			if (!(strcmp(script1, "PILEABLE")))
				pi->pileable=str2num(script2);
			else if (!(strcmp(script1, "POISONED")))
				pi->poisoned=(PoisonType)str2num(script2);
			else if (!(strcmp(script1, "PRIV")))
				pi->priv=(signed char)str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'r':
		case 'R':
			if (!(strcmp(script1, "RESTOCK")))
				pi->restock=str2num(script2);
			else if (!(strcmp(script1, "RANK")))
			{
				pi->rank=str2num(script2); // By Magius(CHE)
				if (pi->rank<=0)
					pi->rank=10;
			}
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 's':
		case 'S':
			if (!strcmp(script1, "SCRIPTID"))
				pi->setScriptID( str2num(script2) );
			else if (!(strcmp(script1, "SK_MADE")))
				pi->madewith=str2num(script2);
			else if (!(strcmp(script1, "SERIAL")))
			{
				i=str2num(script2);
				//if (itemcount2<=i)
				//	itemcount2=i+1;
				pi->setSerial32(i);
				objects.updateItemSerial( i );
			}
			else if (!(strcmp(script1, "SMELT")))
				pi->smelt=str2num(script2);
			else if (!(strcmp(script1, "SECUREIT")))
				pi->secureIt=str2num(script2);
			else if (!(strcmp(script1, "SOUNDFX")))
				pi->itemSoundEffect=(SI32) str2num(script2);
			else if (!(strcmp(script1, "ST")))
				pi->st=str2num(script2);
			else if (!(strcmp(script1, "ST2")))
				pi->st2=str2num(script2);
			else if (!(strcmp(script1, "SPD")))
				pi->spd=str2num(script2);
			else if (!(strcmp(script1, "SPAWNSERIAL")))
				pi->setSpawnSerial(str2num(script2));
			else if (!(strcmp(script1, "SPAWNREGION")))
				pi->setSpawnRegion(str2num(script2));

			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 't':
		case 'T':
			if (!(strcmp(script1, "TEMPFX")))
			{
				tempfx::cTempfx *fx = new tempfx::cTempfx ();
				fx->load();
				pi->addTempfx(fx);
				script1[0]='\0';
			}
			else if (!(strcmp(script1, "TYPE")))
				pi->type=str2num(script2);
			else if (!(strcmp(script1, "TYPE2")))
				pi->type2=str2num(script2);
			else if (!(strcmp(script1, "TRIGGER")))
				pi->trigger=str2num(script2);
			else if (!(strcmp(script1, "TIME_UNUSED")))
				pi->time_unused=str2num(script2);
			else if (!(strcmp(script1, "TRIGTYPE")))
				pi->trigtype=str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'u':
		case 'U':
			if (!(strcmp(script1, "USES")))
				pi->tuses=str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'v':
		case 'V':
			if (!(strcmp(script1, "VISIBLE")))
				pi->visible=(signed char)str2num(script2);
			else if (!(strcmp(script1, "VALUE")))
				pi->value=str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'x':
		case 'X':
			if (!(strcmp(script1, "X")))
				pi->setPosition("x", str2num(script2));
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'y':
		case 'Y':
			if (!(strcmp(script1, "Y")))
				pi->setPosition("y", str2num(script2));
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'w':
		case 'W':
			if (!(strcmp(script1, "WIPE")))
				pi->wipe=str2num(script2);
			else if (!(strcmp(script1, "WEIGHT")))
				pi->weight=str2num(script2);
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;

		case 'z':
		case 'Z':
			if (!(strcmp(script1, "Z")))
				pi->setPosition("z", str2num(script2));
			else WarnOut("Unrecognised attribute : \"%s\", while loading items\n", script1);
			break;
		}

		loops++;
	} while (strcmp(script1, "}") && loops<=200);

	if (bad)
	{
		pi->Delete();
		return;
	}

	amxVS.setUserMode();
	//StoreItemRandomValue(pi,-1); // Magius(CHE) (2)

	pi->timeused_last=getclock();
	if(pi->animid()==pi->getId()) //elcabesa animation bugfix..if we have not defined a animid use the id of object
		pi->animSetId(0);
	pi->weight=(UI32)pi->getWeight();

	if (pi->maxhp==0) pi->maxhp=pi->hp;
	// Tauriel adding region pointers

	if (pi->isInWorld())
	{
		int max_x = map_width  * 8;
		int max_y = map_height * 8;

#ifdef SPAR_I_LOCATION_MAP
		pointers::addToLocationMap(pi);
#else
		mapRegions->add(pi);
#endif

		if( (pi->type==ITYPE_BOATS) && (pi->type2==0) ) //it's a boat!!
		{
			std::string oldname=pi->getCurrentName();
			SI16 oldid = pi->getId();
			cBoat *boat=new cBoat();
			int multinumber = pi->in;
			if ( multinumber == 0 )
			{
// old boat save, we need to calculate the multinumber
				if ( oldid >= 0x4000 && oldid <= 0x04003 )
					multinumber=20;
				if ( oldid >= 0x4004 && oldid <= 0x4007 )
					multinumber=21;
				if ( oldid >= 0x4008 && oldid <= 0x400b )
					multinumber=22;
				if ( oldid >= 0x400c && oldid <= 0x400f )
					multinumber=23;
				if ( oldid >= 0x4010 && oldid <= 0x4013 )
					multinumber=24;
				if ( oldid >= 0x4014 && oldid <= 0x4017 )
					multinumber=25;
			}
			boat->createMulti(multinumber, pi);
			boat->setHold(pi->morez);
			boat->setLeftPlank(pi->morex);
			boat->setRightPlank (pi->morey);
			boat->setTiller (((unsigned char)pi->moreb1<<24)+((unsigned char)pi->moreb2<<16)+((unsigned char)pi->moreb3<<8)+(unsigned char)pi->moreb4);
			boat->setSerial(pi->getSerial32());
			pi->setId(oldid);
			pi->setCurrentName (oldname.c_str ());
			cBoat::insert_boat(pi, boat);
		}

		if ((pi->getPosition("x")<0) || (pi->getPosition("y")<0) || (pi->getPosition("x")>max_x) || (pi->getPosition("y")>max_y))	// lord bianry
			pi->Delete();
	}
	if ( pi->isSpawner() )
		Spawns->loadFromItem(pi);

	amxVS.moveVariable( INVALID, pi->getSerial32() );

	if( pi->layer==LAYER_TRADE_RESTOCK )
		Restocks->addNewRestock( pi );
	itemCount +=1;

}

void CWorldMain::loadNewWorld() // Load world from NXW*.WSC
{
	std::string fileName;
//	unsigned char memerr=0; // unused variable
	char outper[4];
//	unsigned int i=0; // unused variable
//	unsigned int percent=0,a=0,pred=0,maxm=0; // unused variable
	*outper='\0';


	//charcount=0;
	//itemcount=0;
	//charcount2=1;
	//itemcount2=0x40000000;
	fileName = SrvParms->savePath + SrvParms->characterWorldfile + SrvParms->worldfileExtension;
	wscfile=fopen( fileName.c_str(),"r");
	if(!wscfile)
	{
		WarnOut("%s not found.\n", fileName.c_str());
		return;
	} else {
		ConOut("Loading characters ");

		int loopexit=0;
		UI32 ichar=1;

		do
		{
			readw3();
			if (!(strcmp(script1, "SECTION")))
			{
				if (!(strcmp(script2, "CHARACTER")))
				{
					loadChar();
					ichar++;
				}
			}
		} while (strcmp(script1,"EOF") && !feof(wscfile) && (++loopexit < MAXLOOPS*10) );
		fclose(wscfile);
		ConOut("[DONE]\n");
	}

	fileName = SrvParms->savePath + SrvParms->itemWorldfile + SrvParms->worldfileExtension;
	wscfile=fopen( fileName.c_str(), "r");
	if (wscfile==NULL)
	{
		WarnOut("%s not found.\n", fileName.c_str());
		return;
	} else {
		ConOut("Loading items ");

		int loopexit=0;
		UI32 iitem=1;

		do
		{
			readw3();
			if (!strcmp(script1, "SECTION"))
			{
				if (!strcmp(script2, "WORLDITEM"))
				{
					// ConOut ("Loading item No %s\n", script3);
					loaditem();
				}
				iitem++;
			}

		} while (strcmp(script1, "EOF") && !feof(wscfile) && (++loopexit < MAXLOOPS*10) );
		fclose(wscfile);
	}

	ConOut("[DONE]\n");

	Books::LoadBooks();

	Guildz.load();
	loadPrison();
	cHouses::load();

	//Luxor: reload dynamic spawners here.
	// Spawns->clearDynamic();
	cAllObjectsIter objs;
	P_CHAR pc = NULL;
	P_ITEM pi = NULL;
	for( objs.rewind(); !objs.IsEmpty(); objs++ ) {
		if ( isCharSerial( objs.getSerial() ) && ISVALIDPC( (pc=static_cast<P_CHAR>(objs.getObject())) ) ) 
		{
			if( pc->dead && pc->HasHumanBody() )
				pc->morph( ((pc->getId() == BODY_FEMALE) ? (short)BODY_DEADFEMALE : (short)BODY_DEADMALE ), 0, 0, 0, 0, 0, NULL, true);

			if ( pc->getSpawnSerial() != INVALID )
			{
				cSpawnDinamic *dynSpawner=Spawns->getDynamicSpawn(pc->getSpawnSerial());
				if ( dynSpawner != NULL )
					dynSpawner->addSpawned(pc);
				else
					pc->setSpawnSerial(INVALID);
			}
			else if ( pc->getSpawnRegion() != INVALID )
			{
				// add the item to the static spawn region
				cSpawnScripted *staticSpawner=Spawns->getScriptedSpawn(pc->getSpawnRegion());
				if ( staticSpawner != NULL )
					staticSpawner->addSpawned(pc);
				else
					pc->setSpawnRegion(INVALID);
			}
		}
		if ( isItemSerial( objs.getSerial() ) && ISVALIDPI( (pi=static_cast<P_ITEM>(objs.getObject())) ) ) 
		{
			if ( pi->isSpawner() )
				Spawns->loadFromItem(pi);
			if ( pi->getSpawnSerial() != INVALID )
			{
				cSpawnDinamic *dynSpawner=Spawns->getDynamicSpawn(pi->getSpawnSerial());
				if ( dynSpawner != NULL )
					dynSpawner->addSpawned(pi);
			}
			else if ( pi->getSpawnRegion() != INVALID )
			{
				// add the item to the static spawn region
				cSpawnScripted *staticSpawner=Spawns->getScriptedSpawn(pi->getSpawnRegion());
				staticSpawner->addSpawned(pi);
			}
		}

	}
	cResourceMap::load();

	return;
}

//o---------------------------------------------------------------------------o
//|	Class		:	CWorldMain::savenewworld(char x)
//|	Date		:	Unknown
//|	Programmer	:	Unknown
//o---------------------------------------------------------------------------o
//| Purpose		:	Save current world state. Stores all values in an easily
//|					readable script file "*.wsc". This stores all world items
//|					and NPC/PC character information for a given shard
//o---------------------------------------------------------------------------o
void CWorldMain::binarySaveWorld()
{
	std::string fileName;
	fstream *worldfile;
	if ( ( cwmWorldState->Saving() ) ) {
		return;
	}
	tempfx::tempeffectsoff();

	if (SrvParms->server_log)
		ServerLog.Write("Server data save\n");
   if (server_data.announceworldsaves==1)//ANNOUNCE_SAVES in server.cfg
		sysbroadcast(TRANSLATE("World data saving...%d"), server_data.announceworldsaves);
   else
   {
		NxwSocketWrapper sw;
		sw.fillOnline();
		for( sw.rewind(); !sw.isEmpty(); sw++ ) 
		{
			NXWCLIENT ps = sw.getClient();
			if( ps != 0 ) 
			{
				P_CHAR pc=ps->currChar();
				if(ISVALIDPC(pc) && pc->IsGMorCounselor()) 
					pc->sysmsg(TRANSLATE("World will be saved in 30 seconds..."));
			}
		}
   }

	InfoOut("World data saving..." );
////CHARS SAVE
	std::string oldFileName( SrvParms->savePath + SrvParms->characterWorldfile + SrvParms->worldfileBinaryExtension );
	std::string newFileName( SrvParms->savePath + SrvParms->characterWorldfile + SrvParms->worldfileBinaryExtension + "$" );
	remove( newFileName.c_str() );
	rename( oldFileName.c_str(), newFileName.c_str() );

	fileName = SrvParms->savePath + SrvParms->characterWorldfile + SrvParms->worldfileBinaryExtension;
	worldfile = new fstream(fileName.c_str(), ios::binary);
	if (!worldfile || !worldfile->is_open())
	{
		ErrOut("Error, couldn't open %s for writing. Check file permissions.\n", fileName.c_str() );
		rename( newFileName.c_str(), oldFileName.c_str() );
		tempfx::tempeffectson();
		return;
	}


////END
	
}

//o---------------------------------------------------------------------------o
//|	Class		:	CWorldMain::savenewworld(char x)
//|	Date		:	Unknown
//|	Programmer	:	Unknown
//o---------------------------------------------------------------------------o
//| Purpose		:	Save current world state. Stores all values in an easily
//|					readable script file "*.wsc". This stores all world items
//|					and NPC/PC character information for a given shard
//o---------------------------------------------------------------------------o
void CWorldMain::saveNewWorld()
{
	std::string fileName;

	if ( ( cwmWorldState->Saving() ) ) {
		return;
	}

	tempfx::tempeffectsoff();

	if (SrvParms->server_log)
		ServerLog.Write("Server data save\n");

	Accounts->SaveAccounts();

	sysbroadcast(TRANSLATE("World data saving..."));


	InfoOut("World data saving..." );


////CHARS SAVE
	cChar::safeoldsave();
	fileName = SrvParms->savePath + SrvParms->characterWorldfile + SrvParms->worldfileExtension;
	cWsc=fopen( fileName.c_str(), "w");
	if (!cWsc)
	{
		ErrOut("Error, couldn't open %s for writing. Check file permissions.\n", fileName.c_str() );
		tempfx::tempeffectson();
		return;
	}

	fprintf(cWsc, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(cWsc, "// || NoX-Wizard charachters save (nxwchars.wsc)                          ||\n");
	fprintf(cWsc, "// || Automatically generated on worldsaves                               ||\n");
	fprintf(cWsc, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(cWsc, "// || Generated by NoX-Wizard version %s %s               ||\n", VERNUMB, OSFIX);
	fprintf(cWsc, "// || Requires NoX-Wizard version %s to be read correctly              ||\n", WSCREQVER);
	fprintf(cWsc, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n\n");
	fprintf(cWsc, "//\n");
////END

////ITEMS SAVE
	cItem::safeoldsave();
	fileName = SrvParms->savePath + SrvParms->itemWorldfile + SrvParms->worldfileExtension;
	iWsc=fopen( fileName.c_str(), "w");
	if (!iWsc)
	{
		ErrOut("Error, couldn't open %s for writing. Check file permissions.\n", fileName.c_str() );
		tempfx::tempeffectson();
		return;
	}


	fprintf(iWsc, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(iWsc, "// || NoX-Wizard items save (nxwitems.wsc)                                ||\n");
	fprintf(iWsc, "// || Automatically generated on worldsaves                               ||\n");
	fprintf(iWsc, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(iWsc, "// || Generated by NoX-Wizard version %s %s               ||\n", VERNUMB, OSFIX);
	fprintf(iWsc, "// || Requires NoX-Wizard version %s to be read correctly              ||\n", WSCREQVER);
	fprintf(iWsc, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n\n");
	fprintf(iWsc, "//\n");
////END

////GUILDS SAVE
//	Guilds->Write( iWsc ); //Endymion to remove
//Endymion Save new Guild System
	Guildz.save();
//Endymion End Save Guild System
////END

////JAIL SAVE
//Endymion Save new Jail System
	prison::safeoldsave();
	fileName = SrvParms->savePath + SrvParms->jailWorldfile + SrvParms->worldfileExtension;
	jWsc=fopen(fileName.c_str(), "w");
	if (!jWsc)
	{
		ErrOut("Error, couldn't open %s for writing. Check file permissions.\n", fileName.c_str() );
		tempfx::tempeffectson();
		return;
	}
	savePrison();
	fclose(jWsc); jWsc=NULL;
//Endymion End Save new Jail System
////END

	realworldsave();

	tempfx::tempeffectson();

	Books::safeoldsave();
	Books::SaveBooks();
	cHouses::safeoldsave();
	cHouses::save();
	cResourceMap::save();
	sysbroadcast(TRANSLATE("Worldsave Done!\n"));

	ConOut(" [DONE]\n");

	uiCurrentTime = getclock();
}

bool CWorldMain::Saving()
{
	return isSaving;
}

/*
\brief save on file a wstring
\author Endymion
\note save output is like 00E000123215, hex output
\param f the file
\param name the variable name
\param c the wstring
*/
void fprintWstring( FILE* f, char* name, wstring c )
{
	fprintf( f, "%s ", name );
	wstring::iterator iter( c.begin() ), end( c.end() );
	for( ; iter!=end; iter++ ) {
		fprintf( f, "%04x", (*iter) );
	}
	fprintf( f, "0000\n" );
}

// xan : this is an internal option -> with it enabled, player names etc will not be saved
//       to ease masking of private data in worldsaves :)
//#define DESTROY_REFERENCES


void CWorldMain::SaveChar( P_CHAR pc )
{
	char valid=0;
	int j;
	VALIDATEPC( pc );

	static cChar dummy(false);

	Location pcpos= pc->getPosition();
	// During save revert morph
	//Luxor safe stats system
	if (pc->getStrength() != pc->st3) pc->setStrength(pc->st3);
	if (pc->dx != pc->dx3) pc->dx = pc->dx3;
	if (pc->in != pc->in3) pc->in = pc->in3;
	if (pc->hp > pc->getStrength()) pc->hp = pc->getStrength();
	if (pc->stm > pc->dx) pc->stm = pc->dx;
	if (pc->mn > pc->in) pc->mn = pc->in;
	//End safe stats system

	//Endy for remove pg not without accounts
/*	if( pc->account==INVALID && !pc->npc )
		pc->free;
*/
	valid=1;
	if (pc->getSerial32() < 0) valid = 0;
	if (pc->summontimer ) valid = 0; //xan : we don't save summoned stuff
	/*
	if (pc->spawnregion!=INVALID || pc->spawnserial!=INVALID ) valid=0;
	*/
	if (valid)
	{
			fprintf(cWsc, "SECTION CHARACTER %i\n", this->chr_curr++);
			fprintf(cWsc, "{\n");
			fprintf(cWsc, "SERIAL %i\n", pc->getSerial32());
			//Luxor: if the char is morphed, we have to save the original values.
			if(pc->morphed!=dummy.morphed)
			{//save original name
#ifndef DESTROY_REFERENCES
				if ( pc->getBackupStats() != NULL )
					fprintf(cWsc, "NAME %s\n", pc->getBackupStats()->getName().c_str());
				else
				{
					fprintf(cWsc, "NAME %s\n", pc->secondary_name.c_str());
				}
#else
				if (pc->npc)
					fprintf(cWsc, "NAME %s\n", pc->name);
				else
					fprintf(cWsc, "NAME C%x\n", pc->serial);
#endif
			} else
			{
#ifndef DESTROY_REFERENCES
				fprintf(cWsc, "NAME %s\n", pc->current_name.c_str());
#else
				if (pc->npc)
					fprintf(cWsc, "NAME %s\n", pc->name);
				else
					fprintf(cWsc, "NAME C%x\n", pc->serial);
#endif
			}

#ifndef DESTROY_REFERENCES
			fprintf(cWsc, "TITLE %s\n", pc->title.c_str());
#endif
			if(pc->account!=dummy.account)
				fprintf(cWsc, "ACCOUNT %i\n", pc->account);
			if(pc->getPrivLevel()!=dummy.getPrivLevel())
				fprintf(cWsc, "PRIVLEVEL %i\n", pc->getPrivLevel());
/*			if (pc->laston)
				fprintf(cWsc, "LASTON %s\n", pc->laston);
*/			if (pc->GetCreationDay()!=dummy.GetCreationDay())
				fprintf(cWsc, "CREATIONDAY %i\n", pc->GetCreationDay());
		    if (pc->gmMoveEff!=dummy.gmMoveEff)
                fprintf(cWsc, "GMMOVEEFF %i\n", pc->gmMoveEff);
			if(pc->GetGuildType()!=dummy.GetGuildType())
				fprintf(cWsc,"GUILDTYPE %i\n",pc->GetGuildType());
			if(pc->IsGuildTraitor())
				fprintf(cWsc,"GUILDTRAITOR YES\n");

			// commented by Anthalir, we need to always save them
//			if (pcpos.x)
				fprintf(cWsc, "X %i\n", pcpos.x);
//			if (pcpos.y)
				fprintf(cWsc, "Y %i\n", pcpos.y);
//			if (pcpos.z)
				fprintf(cWsc, "Z %i\n", pcpos.z);
//			if (pcpos.dispz)
				fprintf(cWsc, "DISPZ %i\n", pcpos.dispz);

			if (pc->getOldPosition("x")!=dummy.getOldPosition("x"))
				fprintf(cWsc, "OLDX %i\n", pc->getOldPosition("x"));
			if (pc->getOldPosition("y")!=dummy.getOldPosition("y"))
				fprintf(cWsc, "OLDY %i\n", pc->getOldPosition("y"));
			if (pc->getOldPosition("z")!=dummy.getOldPosition("z"))
				fprintf(cWsc, "OLDZ %i\n", pc->getOldPosition("z"));

			if (pc->dir!=dummy.dir)
				fprintf(cWsc, "DIR %i\n", pc->dir);

			if (pc->doorUse!=dummy.doorUse)
				fprintf(cWsc, "DOORUSE 1\n"/*, pc->doorUse*/ );

			//Luxor: if the char is morphed, we have to save the original values.
			if(pc->morphed) {
				if ( pc->getBackupStats() != NULL && pc->getBackupStats()->getId() != dummy.getOldId() )
					fprintf(cWsc, "BODY %i\n", pc->getBackupStats()->getId());
				else
					fprintf(cWsc, "BODY %i\n", pc->getOldId());
			}
			else {
				if ( pc->getId() != dummy.getId() )
						fprintf(cWsc, "BODY %i\n", pc->getId());
			}
			if ( pc->getOldId() != dummy.getOldId() )
				fprintf(cWsc, "XBODY %i\n", pc->getOldId());

			//Luxor: if the char is morphed, we have to save the original values.
			if(pc->morphed) {
				if ( pc->getBackupStats() != NULL && pc->getBackupStats()->getColor() != dummy.getOldColor() )
					fprintf(cWsc, "SKIN %i\n", pc->getBackupStats()->getColor());
			} else {
				if ( pc->getColor() != dummy.getColor() )
					fprintf(cWsc, "SKIN %i\n", pc->getColor());
			}

			if ( pc->getOldColor() != dummy.getOldColor() )
				fprintf(cWsc, "XSKIN %i\n", pc->getOldColor());
			if (pc->GetPriv()!=dummy.GetPriv())
				fprintf(cWsc, "PRIV %i\n", pc->GetPriv());


        	if (pc->damagetype!=DAMAGE_PURE) //Luxor
            	fprintf(cWsc, "DAMAGETYPE %i\n", pc->damagetype);
			if (pc->getStablemaster()!=dummy.getStablemaster())
			    fprintf(cWsc, "STABLEMASTER %i\n", pc->getStablemaster());
			if (pc->npc_type!=dummy.npc_type)
				fprintf(cWsc, "NPCTYPE %i\n", pc->npc_type);
			if (pc->time_unused!=dummy.time_unused)
				fprintf(cWsc, "TIME_UNUSED %i\n", pc->time_unused);

			if (pc->GetPriv2()!=dummy.GetPriv2())
				fprintf(cWsc, "ALLMOVE %i\n", pc->GetPriv2());
			if (pc->fonttype!=dummy.fonttype)
				fprintf(cWsc, "FONT %i\n", pc->fonttype);
			if ( pc->saycolor != dummy.saycolor )
				fprintf(cWsc, "SAY %i\n", pc->saycolor);
			if ( pc->emotecolor != dummy.emotecolor )
				fprintf(cWsc, "EMOTE %i\n", pc->emotecolor);

			fprintf(cWsc, "STRENGTH %i\n", pc->st3);
			fprintf(cWsc, "STRENGTH2 %i\n", qmax(0, pc->st2) );
			fprintf(cWsc, "DEXTERITY %i\n", pc->dx3);
			fprintf(cWsc, "DEXTERITY2 %i\n", qmax(0,pc->dx2));
			fprintf(cWsc, "INTELLIGENCE %i\n", pc->in3);
			fprintf(cWsc, "INTELLIGENCE2 %i\n", qmax(0, pc->in2) );

			if (pc->hp!=dummy.hp)
				fprintf(cWsc, "HITPOINTS %i\n", pc->hp);
			if (pc->stm!=dummy.stm)
				fprintf(cWsc, "STAMINA %i\n", pc->stm);
			if (pc->mn!=dummy.mn)
				fprintf(cWsc, "MANA %i\n", pc->mn);

            if (pc->possessorSerial == INVALID) //Luxor
                    fprintf(cWsc, "NPC %i\n", pc->npc);
            else
                    fprintf(cWsc, "NPC %i\n", 1);

			if (pc->possessedSerial != INVALID) //Luxor
                                fprintf(cWsc, "POSSESSEDSERIAL %i\n", pc->possessedSerial);

			if (pc->holdg!=dummy.holdg) // bugfix lb, holdgold value never saved !!!
                fprintf(cWsc, "HOLDGOLD %i\n", pc->holdg);

			if (pc->shopkeeper!=dummy.shopkeeper)
				fprintf(cWsc, "SHOP\n");
            /*
			if ((pc->own1<<24)+(pc->own2<<16)+(pc->own3<<8)+pc->own4)
				fprintf(cWsc, "OWN %i\n", (pc->own1<<24)+(pc->own2<<16)+(pc->own3<<8)+pc->own4);
                */
            if (pc->getOwnerSerial32()!=dummy.getOwnerSerial32())
                fprintf(cWsc, "OWN %i\n", pc->getOwnerSerial32());
			if (pc->robe != dummy.robe)
				fprintf(cWsc, "ROBE %i\n", pc->robe);
			if (pc->GetKarma()!=dummy.GetKarma())
				fprintf(cWsc, "KARMA %i\n", pc->GetKarma());
			if (pc->GetFame()!=dummy.GetFame())
				fprintf(cWsc, "FAME %i\n", pc->GetFame());
			if (pc->kills!=dummy.kills)
				fprintf(cWsc, "KILLS %i\n", pc->kills);
			if (pc->deaths!=dummy.deaths)
				fprintf(cWsc, "DEATHS %i\n", pc->deaths);
			if (pc->dead!=dummy.dead)
				fprintf(cWsc, "DEAD\n");
//			if (pc->packitem!=dummy.packitem)
//				fprintf(cWsc, "PACKITEM %i\n", pc->packitem);
			if (pc->fixedlight!=dummy.fixedlight)
				fprintf(cWsc, "FIXEDLIGHT %i\n", pc->fixedlight);
			if (pc->speech!=dummy.speech)
				fprintf(cWsc, "SPEECH %i\n", pc->speech);
			if (pc->trigger!=dummy.trigger)
				fprintf(cWsc, "TRIGGER %i\n", pc->trigger);
			if (pc->trigword.length()>0)
				fprintf(cWsc, "TRIGWORD %s\n", pc->trigword.c_str());
			//Changed by Magius(CHE) �
			//tmpdisable=(long) pc->disabled-currenttime;
			if (pc->disabledmsg!=NULL)
				fprintf(cWsc, "DISABLEMSG %s\n", pc->disabledmsg->c_str()); // Added by Magius(CHE) �

			for (j=0;j<TRUESKILLS;j++)
			{
				// Don't save the default value given by initchar
				if ((pc->baseskill[j] != 10)&&(pc->baseskill[j]>1))
				{
				  fprintf(cWsc, "SKILL%i %i\n", j, pc->baseskill[j]);

				}
				if( pc->lockSkill[j] != 0 ) fprintf(cWsc, "SKL%i %i\n", j, pc->lockSkill[j] );

			}
			//for (j=0;j<ALLSKILLS;j++)
			// Don't save the default value given by initchar
			if (!pc->cantrain)
				fprintf(cWsc, "NOTRAIN\n");
			else
				fprintf(cWsc, "CANTRAIN\n");
			if ( pc->getHireFee() >= 0 )
				fprintf(cWsc, "HIRE %d\n", pc->getHireFee());

			if (pc->att!=dummy.att)
				fprintf(cWsc, "ATT %i\n", pc->att);
			if (pc->def!=dummy.def)
				fprintf(cWsc, "DEF %i\n", pc->def);
			if (pc->lodamage!=dummy.lodamage)
				fprintf(cWsc, "LODAMAGE %i\n", pc->lodamage);
			if (pc->hidamage!=dummy.hidamage)
				fprintf(cWsc, "HIDAMAGE %i\n", pc->hidamage);
			if (pc->war!=dummy.war)
				fprintf(cWsc, "WAR %i\n", pc->war);
			if (pc->npcWander!=dummy.npcWander)
				fprintf(cWsc, "NPCWANDER %i\n", pc->npcWander);
			if (pc->oldnpcWander!=dummy.oldnpcWander)
				fprintf(cWsc, "OLDNPCWANDER %i\n", pc->oldnpcWander);

			if (pc->ftargserial!=dummy.ftargserial)
				fprintf(cWsc, "PC_FTARGSER %i\n", pc->ftargserial);
			if (pc->carve!=dummy.carve)
				fprintf(cWsc, "CARVE %i\n", pc->carve);
			if (pc->fx1!=dummy.fx1)
				fprintf(cWsc, "FX1 %i\n", pc->fx1);
			if (pc->fy1!=dummy.fy1)
				fprintf(cWsc, "FY1 %i\n", pc->fy1);
			if (pc->fz1!=dummy.fz1)
				fprintf(cWsc, "FZ1 %i\n", pc->fz1);
			if (pc->fx2!=dummy.fx2)
				fprintf(cWsc, "FX2 %i\n", pc->fx2);
			if (pc->fy2!=dummy.fy2)
				fprintf(cWsc, "FY2 %i\n", pc->fy2);
			if (pc->IsHidden())
				fprintf(cWsc, "HIDDEN %i\n", pc->hidden);
			if (pc->hunger!=dummy.hunger)
				fprintf(cWsc, "HUNGER %i\n", pc->hunger);
			if (pc->npcaitype!=dummy.npcaitype)
				fprintf(cWsc, "NPCAITYPE %i\n", pc->npcaitype);
			if (pc->spattack!=dummy.spattack)
				fprintf(cWsc, "SPATTACK %i\n", pc->spattack);
			if (pc->spadelay!=dummy.spadelay)
				fprintf(cWsc, "SPADELAY %i\n", pc->spadelay);
			if (pc->magicsphere!=dummy.magicsphere)
				fprintf(cWsc, "MAGICSPHERE %i\n", pc->magicsphere);
			if (pc->mounted!=dummy.mounted)
				fprintf(cWsc, "MOUNTED\n");
			if (pc->taming!=dummy.taming)
				fprintf(cWsc, "TAMING %i\n", pc->taming);
/*			if (pc->summontimer)//AntiChrist //XAN : useless, we don't save summonz
				fprintf(cWsc, "SUMMONREMAININGSECONDS %i\n", (pc->summontimer/MY_CLOCKS_PER_SEC));
*/			if (pc->advobj!=dummy.advobj)
				fprintf(cWsc, "ADVOBJ %i\n", pc->advobj);
			if (pc->poison!=dummy.poison)
				fprintf(cWsc, "POISON %i\n", pc->poison);
			if (pc->poisoned!=dummy.poisoned)
				fprintf(cWsc, "POISONED %i\n", pc->poisoned);
			if ( pc->IsMurderer() && ( pc->murderrate>uiCurrentTime ) )
				fprintf(cWsc, "MURDERSAVE %i\n", ( ( pc->murderrate-uiCurrentTime) / MY_CLOCKS_PER_SEC ) );
			if (pc->fleeat!=dummy.fleeat)
				fprintf(cWsc, "FLEEAT %i\n", pc->fleeat);
			if (pc->getRace()!=dummy.getRace())
				fprintf(cWsc, "RACE %i\n", pc->getRace());
			if (pc->reattackat!=dummy.reattackat)
				fprintf(cWsc, "REATTACKAT %i\n", pc->reattackat);
			if (pc->holdg!=dummy.holdg) //Luxor: players vendors fix
				fprintf(cWsc, "HOLDG %i\n", pc->holdg);
			if (pc->split!=dummy.split)
				fprintf(cWsc, "SPLIT %i\n", pc->split);
			if (pc->splitchnc!=dummy.splitchnc)
				fprintf(cWsc, "SPLITCHANCE %i\n", pc->splitchnc);
// Begin of Guild related things (DasRaetsel)
			if (pc->HasGuildTitleToggle()!=dummy.HasGuildTitleToggle())
				fprintf(cWsc, "GUILDTOGGLE %i\n", 1);
			if (pc->GetGuildNumber()!=dummy.GetGuildNumber())
				fprintf(cWsc, "GUILDNUMBER %i\n", pc->GetGuildNumber());
			if (strlen( pc->GetGuildTitle() ) )
#ifndef DESTROY_REFERENCES
				fprintf(cWsc, "GUILDTITLE %s\n", pc->GetGuildTitle());
#else
				fprintf(cWsc, "GUILDTITLE CG%x\n", pc->serial);
#endif
			if (pc->GetGuildFealty()!=dummy.GetGuildFealty())
			{
				fprintf(cWsc, "GUILDFEALTY %i\n", pc->GetGuildFealty());
				fprintf(cWsc, "MURDERRATE %i\n",pc->murderrate);
			}
			if (pc->getRegenRate( STAT_HP, VAR_REAL ) != dummy.getRegenRate( STAT_HP, VAR_REAL ) )
				fprintf(cWsc, "REGEN_HP %i\n", pc->getRegenRate( STAT_HP, VAR_REAL ));
			if (pc->getRegenRate( STAT_STAMINA, VAR_REAL ) != dummy.getRegenRate( STAT_STAMINA, VAR_REAL ) )
				fprintf(cWsc, "REGEN_ST %i\n", pc->getRegenRate( STAT_STAMINA, VAR_REAL ));
			if (pc->getRegenRate( STAT_MANA, VAR_REAL ) != dummy.getRegenRate( STAT_MANA, VAR_REAL ) )
				fprintf(cWsc, "REGEN_MN %i\n", pc->getRegenRate( STAT_MANA, VAR_REAL ));

			if (pc->homeloc.x!=dummy.homeloc.x)
				fprintf(cWsc, "HOMEX %i\n", pc->homeloc.x);
			if (pc->homeloc.y!=dummy.homeloc.y)
				fprintf(cWsc, "HOMEY %i\n", pc->homeloc.y);
			if (pc->homeloc.z!=dummy.homeloc.z)
				fprintf(cWsc, "HOMEZ %i\n", pc->homeloc.z);
			if (pc->workloc.x!=dummy.workloc.x)
				fprintf(cWsc, "WORKX %i\n", pc->workloc.x);
			if (pc->workloc.y!=dummy.workloc.y)
				fprintf(cWsc, "WORKY %i\n", pc->workloc.y);
			if (pc->workloc.z!=dummy.workloc.z)
				fprintf(cWsc, "WORKZ %i\n", pc->workloc.z);
			if (pc->foodloc.x!=dummy.foodloc.x)
				fprintf(cWsc, "FOODX %i\n", pc->foodloc.x);
			if (pc->foodloc.y!=dummy.foodloc.y)
				fprintf(cWsc, "FOODY %i\n", pc->foodloc.y);
			if (pc->foodloc.z!=dummy.foodloc.z)
				fprintf(cWsc, "FOODZ %i\n", pc->foodloc.z);
			// Spawns
			if (pc->getSpawnSerial() != dummy.getSpawnSerial())
				fprintf(cWsc, "SPAWNSERIAL %i\n", pc->getSpawnSerial());
			if (pc->getSpawnRegion() != dummy.getSpawnRegion())
				fprintf(cWsc, "SPAWNREGION %i\n", pc->getSpawnRegion());

			// Dupois - Escort quests
			if (pc->questType!=dummy.questType && pc->questType<1000)
				fprintf(cWsc, "QUESTTYPE %i\n", pc->questType);
			if (pc->questDestRegion!=dummy.questDestRegion && pc->questDestRegion<1000)
				fprintf(cWsc, "QUESTDESTREGION %i\n", pc->questDestRegion);
			if (pc->questOrigRegion!=dummy.questOrigRegion && pc->questOrigRegion<1000)
				fprintf(cWsc, "QUESTORIGREGION %i\n", pc->questOrigRegion);
			if (pc->questBountyPostSerial !=dummy.questBountyPostSerial)
				fprintf(cWsc, "QUESTBOUNTYPOSTSERIAL %i\n", pc->questBountyPostSerial);

			if (pc->questBountyReward !=dummy.questBountyReward)
				fprintf(cWsc, "QUESTBOUNTYREWARD %i\n", pc->questBountyReward);

			if (pc->gmrestrict!=dummy.gmrestrict)
				fprintf(cWsc, "GMRESTRICT %i\n", pc->gmrestrict);
			if( pc->getScriptID()!=dummy.getScriptID() )
				fprintf(cWsc, "SCRIPTID %u\n", pc->getScriptID());		

			if( pc->npc && pc->npcMoveSpeed != NPCSPEED )
				fprintf(cWsc, "MOVESPEED %f\n", pc->npcMoveSpeed );
			if( pc->npc && pc->npcFollowSpeed != NPCFOLLOWSPEED )
				fprintf(cWsc, "FOLLOWSPEED %f\n", pc->npcFollowSpeed );
			if( pc->profile!=dummy.profile )
				fprintWstring( cWsc, "PROFILE", pc->profile );
			if( !pc->lootVector.empty() )
			{
				int last = pc->lootVector.size();
				fprintf(cWsc, "LOOT\n{\n" );
				for( int index = 0; index < last; ++index )
					fprintf(cWsc, "%i\n", pc->lootVector[index] );
				fprintf(cWsc, "}\n");
			}

			if (pc->nxwflags[0]!=dummy.nxwflags[0]) fprintf(cWsc, "NXWFLAG0 %i\n", pc->nxwflags[0]);
			if (pc->nxwflags[1]!=dummy.nxwflags[1]) fprintf(cWsc, "NXWFLAG1 %i\n", pc->nxwflags[1]);
			if (pc->nxwflags[2]!=dummy.nxwflags[2]) fprintf(cWsc, "NXWFLAG2 %i\n", pc->nxwflags[2]);
			if (pc->nxwflags[3]!=dummy.nxwflags[3]) fprintf(cWsc, "NXWFLAG3 %i\n", pc->nxwflags[3]);
			if (pc->hasTempfx())
			{
				TempfxVector *charTempfxVec = pc->getTempfxVec( );
				TempfxVector::iterator iter=charTempfxVec->begin();
				for ( ; iter != charTempfxVec->end(); iter++)
				{
					tempfx::cTempfx fx = *iter;
					fx.save(cWsc);
				}
			}

			for (int JJ = 0; JJ< MAX_RESISTANCE_INDEX; JJ++)
				if ( pc->resists[JJ] != dummy.resists[JJ] )
					fprintf(cWsc, "RESISTS %d %d\n", JJ, pc->resists[JJ]);

#define SAVECHAREVENT(A,B) if ((pc->amxevents[B])&&(pc->amxevents[B]->shouldBeSaved())) fprintf(cWsc, "%s %s\n", A, pc->amxevents[B]->getFuncName());
//#define SAVECHAREVENT(A,B) { AmxEvent *event = pc->getAmxEvent( B ); if ( event ) if( event->shouldBeSaved() ) fprintf(cWsc, "%s %s\n", A, event->getFuncName() ); }
			SAVECHAREVENT("@ONADVANCESKILL", EVENT_CHR_ONADVANCESKILL);
			SAVECHAREVENT("@ONADVANCESTAT", EVENT_CHR_ONADVANCESTAT);
			SAVECHAREVENT("@ONBEGINATTACK", EVENT_CHR_ONBEGINATTACK);
			SAVECHAREVENT("@ONBEGINDEFENSE", EVENT_CHR_ONBEGINDEFENSE);
			SAVECHAREVENT("@ONCASTSPELL", EVENT_CHR_ONCASTSPELL);
			SAVECHAREVENT("@ONDEATH", EVENT_CHR_ONBEFOREDEATH);
			SAVECHAREVENT("@ONDIED", EVENT_CHR_ONAFTERDEATH);
			SAVECHAREVENT("@ONBEFOREDEATH", EVENT_CHR_ONBEFOREDEATH);
			SAVECHAREVENT("@ONAFTERDEATH", EVENT_CHR_ONAFTERDEATH);
			SAVECHAREVENT("@ONKILL", EVENT_CHR_ONKILL);
			SAVECHAREVENT("@ONDISMOUNT", EVENT_CHR_ONDISMOUNT); //Unavowed
			SAVECHAREVENT("@ONDISPEL", EVENT_CHR_ONDISPEL);
			SAVECHAREVENT("@ONFLAGCHANGE", EVENT_CHR_ONFLAGCHG);
			SAVECHAREVENT("@ONGETHIT", EVENT_CHR_ONGETHIT);
			SAVECHAREVENT("@ONGETSKILLCAP", EVENT_CHR_ONGETSKILLCAP);
			SAVECHAREVENT("@ONGETSTATCAP", EVENT_CHR_ONGETSTATCAP);
			SAVECHAREVENT("@ONHIT", EVENT_CHR_ONHIT);
			SAVECHAREVENT("@ONHITMISS", EVENT_CHR_ONHITMISS);
			SAVECHAREVENT("@ONMOUNT", EVENT_CHR_ONMOUNT);
			SAVECHAREVENT("@ONMULTIENTER", EVENT_CHR_ONMULTIENTER);
			SAVECHAREVENT("@ONMULTILEAVE", EVENT_CHR_ONMULTILEAVE);
			SAVECHAREVENT("@ONPOISONED", EVENT_CHR_ONPOISONED);
			SAVECHAREVENT("@ONREGIONCHANGE", EVENT_CHR_ONREGIONCHANGE);
			SAVECHAREVENT("@ONREPUTATIONCHANGE", EVENT_CHR_ONREPUTATIONCHG);
			SAVECHAREVENT("@ONRESURRECT", EVENT_CHR_ONRESURRECT);
			SAVECHAREVENT("@ONSNOOPED", EVENT_CHR_ONSNOOPED);
			SAVECHAREVENT("@ONSTOLEN", EVENT_CHR_ONSTOLEN);
			SAVECHAREVENT("@ONTRANSFER", EVENT_CHR_ONTRANSFER);
			SAVECHAREVENT("@ONWALK", EVENT_CHR_ONWALK);
			SAVECHAREVENT("@ONWOUNDED", EVENT_CHR_ONWOUNDED);
			SAVECHAREVENT("@ONBREAKMEDITATION", EVENT_CHR_ONBREAKMEDITATION); //Ummon, 10/01
			SAVECHAREVENT("@ONBLOCK", EVENT_CHR_ONBLOCK);
			SAVECHAREVENT("@ONSTART", EVENT_CHR_ONSTART);
			SAVECHAREVENT("@ONHEARTBEAT", EVENT_CHR_ONHEARTBEAT);
			SAVECHAREVENT("@ONCLICK", EVENT_CHR_ONCLICK);
			SAVECHAREVENT("@ONHEARPLAYER", EVENT_CHR_ONHEARPLAYER);
			SAVECHAREVENT("@ONDOCOMBAT", EVENT_CHR_ONDOCOMBAT);
			SAVECHAREVENT("@ONCOMBATHIT", EVENT_CHR_ONCOMBATHIT);
			SAVECHAREVENT("@ONSPEECH", EVENT_CHR_ONSPEECH);
			SAVECHAREVENT("@ONCHECKNPCAI", EVENT_CHR_ONCHECKNPCAI );
			SAVECHAREVENT("@ONOPENCHAT", EVENT_CHR_ONOPENCHAT );
			SAVECHAREVENT("@ONOPENSTATUS", EVENT_CHR_ONOPENSTATUS );
			SAVECHAREVENT("@ONOPENSKILLS", EVENT_CHR_ONOPENSKILLS );
			SAVECHAREVENT("@ONDBLCLICK", EVENT_CHR_ONDBLCLICK );


			//
			//  SAVE NEW STYLE AMX VARIABLES
			//
			amxVS.saveVariable( pc->getSerial32(), cWsc );

			if(pc->getPrivLevel()!=dummy.getPrivLevel())
				fprintf(cWsc, "PRIVLEVEL %i\n", pc->getPrivLevel());
			if ( !(pc->npc || pc->IsGMorCounselor()) )
			{

				fprintf(cWsc, "\n");
			}

			fprintf(cWsc, "}\n\n");
	};
}

void CWorldMain::SaveItem( P_ITEM pi )
{

	VALIDATEPI( pi );

	static cItem dummy( false );

	if (pi->getSerial32()<0) return;

/*
// Now spawned objects are saved like everything else
	if (pi->getSpawnRegion()!=INVALID || pi->getSpawnSerial()!=INVALID )
		return;
*/
	if( pi->isInWorld() ) {

		if ( ( pi->getPosition().x < 100) && ( pi->getPosition().y < 100 ) ) { //garbage positions
			//Luxor: we must check if position is 0,0 for dragged items
			if (pi->getPosition().x != 0 && pi->getPosition().y != 0) {
				pi->Delete();
				return;
			}
		}

		if( pi->doDecay() )
			return;

	}
	// converting old house styles to new housesystem
	if ( pi->IsHouse() )
	{
		// Look if pi serial connects to a house in cHouses
		P_HOUSE house=cHouses::findHouse(pi->getSerial32());
		if ( house == NULL )
		{
			// No house information has been made yet
			P_ITEM temp = item::CreateFromScript( pi->morex, NULL);
			if ( !ISVALIDPI(temp) )
			{
				objects.eraseObject (temp);
				return;
			}
			P_HOUSE newHouse = new cHouse();
			newHouse->setSerial(pi->getSerial32());
			newHouse->createMulti(temp->morex, pi);
			newHouse->setOwner(pi->getOwnerSerial32());
			cHouses::addHouse(newHouse );
			temp->Delete();

		}
	}
	if ( (pi->type == ITYPE_NPC_SPAWNER )|| (pi->type == ITYPE_ITEM_SPAWNER ))
	{
		cSpawnDinamic *spawn = Spawns->getDynamicSpawn(pi->getSerial32());
		if ( spawn == NULL )
		{
			// No spawner has been made yet
			Spawns->loadFromItem(pi);
			spawn = Spawns->getDynamicSpawn(pi->getSerial32());
			spawn->clear();
			pi->amount2=0;
			spawn->current=0;
			spawn->nextspawn=uiCurrentTime+ (60*RandomNum( pi->morey, pi->morez)*MY_CLOCKS_PER_SEC);
		}
	}
	if ( ( !pi->isInWorld() || ((pi->getPosition("x") > 1) && (pi->getPosition("x") < 6144) && (pi->getPosition("y") < 4096))))
	{
		fprintf(iWsc, "SECTION WORLDITEM %i\n", this->itm_curr++);
		fprintf(iWsc, "{\n");
		fprintf(iWsc, "SERIAL %i\n", pi->getSerial32());
		fprintf(iWsc, "NAME %s\n", pi->getCurrentNameC());
		//<Luxor>: if the item is beard or hair of a morphed char, we must save the original ID and COLOR value
		if ( (pi->layer == LAYER_BEARD || pi->layer == LAYER_HAIR) && isCharSerial( pi->getContSerial() ) ) {
			P_CHAR pc_morphed = (P_CHAR)(pi->getContainer());
			if (ISVALIDPC(pc_morphed)) 
			{
				if (pc_morphed->morphed) 
				{
					if (pi->layer == LAYER_BEARD) 
					{ //beard
						if ( pc_morphed->getBackupStats() != NULL )
						{
							fprintf(iWsc, "ID %i\n", pc_morphed->getBackupStats()->getBeardStyle());
							fprintf(iWsc, "COLOR %i\n", pc_morphed->getBackupStats()->getBeardColor());

						}
					} 
					else 
					{ //hair
						if ( pc_morphed->getBackupStats() != NULL )
						{
							fprintf(iWsc, "ID %i\n", pc_morphed->getBackupStats()->getHairStyle());
							fprintf(iWsc, "COLOR %i\n", pc_morphed->getBackupStats()->getHairColor());

						}
					}
				} 
				else 
				{
					fprintf(iWsc, "ID %i\n", pi->getId());
					if (pi->getColor()!=dummy.getColor())
						fprintf(iWsc, "COLOR %i\n", pi->getColor());
				}
			} else {
				fprintf(iWsc, "ID %i\n", pi->getId());
				if (pi->getColor()!=dummy.getColor())
					fprintf(iWsc, "COLOR %i\n", pi->getColor());
			}
		} else {
			fprintf(iWsc, "ID %i\n", pi->getId());
			if (pi->getColor()!=dummy.getColor())
				fprintf(iWsc, "COLOR %i\n", pi->getColor());
		}
		//</Luxor>
		if( pi->getScriptID()!=dummy.getScriptID() )
			fprintf(iWsc, "SCRIPTID %u\n", pi->getScriptID());
		if ((pi->animid()!=pi->getId() )&&(pi->animid()!=dummy.animid()))
			fprintf(iWsc, "ANIMID %i\n", pi->animid());
		fprintf(iWsc, "NAME2 %s\n", pi->getSecondaryNameC());
#ifndef DESTROY_REFERENCES
		if ( !pi->creator.empty())	fprintf(iWsc, "CREATOR %s\n", pi->creator.c_str() ); // by Magius(CHE)
#endif
		if (pi->madewith!=dummy.madewith)
			fprintf(iWsc, "SK_MADE %i\n", pi->madewith ); // by Magius(CHE)

		fprintf(iWsc, "X %i\n", pi->getPosition().x);
		fprintf(iWsc, "Y %i\n", pi->getPosition().y);
		fprintf(iWsc, "Z %i\n", pi->getPosition().z);

		if (pi->getContSerial()!=dummy.getContSerial())
			fprintf(iWsc, "CONT %i\n", pi->getContSerial());
		if (pi->layer!=dummy.layer)
			fprintf(iWsc, "LAYER %i\n", pi->layer);
		if (pi->itmhand!=dummy.itmhand)
			fprintf(iWsc, "ITEMHAND %i\n", pi->itmhand);
		if (pi->type!=dummy.type)
			fprintf(iWsc, "TYPE %i\n", pi->type);
		//xan : don't save type2 for boats, so they'll restart STOPPED
		// elcabesa se non lo salvi per le barche poi non le riapri =)
		if (pi->type2!=dummy.type2)
		{
			if ( !( (pi->type==117) && ((pi->type2==1) ||(pi->type2==2)) ) )
			{
				fprintf(iWsc, "TYPE2 %i\n", pi->type2);
			}
		}
		if (pi->offspell!=dummy.offspell)
			fprintf(iWsc, "OFFSPELL %i\n", pi->offspell);
		if (((unsigned char)pi->more1<<24)+((unsigned char)pi->more2<<16)+((unsigned char)pi->more3<<8)+(unsigned char)pi->more4) //;
			fprintf(iWsc, "MORE %i\n", ((unsigned char)pi->more1<<24)+((unsigned char)pi->more2<<16)+((unsigned char)pi->more3<<8)+(unsigned char)pi->more4);
		if (((unsigned char)pi->moreb1<<24)+((unsigned char)pi->moreb2<<16)+((unsigned char)pi->moreb3<<8)+(unsigned char)pi->moreb4)
			fprintf(iWsc, "MORE2 %i\n", ((unsigned char)pi->moreb1<<24)+((unsigned char)pi->moreb2<<16)+((unsigned char)pi->moreb3<<8)+(unsigned char)pi->moreb4);
		if (pi->morex!=dummy.morex)
			fprintf(iWsc, "MOREX %i\n", pi->morex);
		if (pi->morey!=dummy.morey)
			fprintf(iWsc, "MOREY %i\n", pi->morey);
		if (pi->morez!=dummy.morez)
			fprintf(iWsc, "MOREZ %i\n", pi->morez);
		if (pi->amount!=dummy.amount)
			fprintf(iWsc, "AMOUNT %i\n", pi->amount);
		if (pi->amount2!=dummy.amount)
			fprintf(iWsc, "AMOUNT2 %i\n", pi->amount2);
		if (pi->pileable!=dummy.pileable)
			fprintf(iWsc, "PILEABLE %i\n", pi->pileable);
		if (pi->doordir!=dummy.doordir)
			fprintf(iWsc, "DOORFLAG %i\n", pi->doordir);
		if (pi->dye!=dummy.dye)
			fprintf(iWsc, "DYEABLE %i\n", pi->dye);
		if (pi->corpse!=dummy.corpse)
			fprintf(iWsc, "CORPSE %i\n", pi->corpse);
		if (pi->att!=dummy.att)
			fprintf(iWsc, "ATT %i\n", pi->att);
		if (pi->def!=dummy.def)
			fprintf(iWsc, "DEF %i\n", pi->def);
		if (pi->hidamage!=dummy.hidamage)
			fprintf(iWsc, "HIDAMAGE %i\n", pi->hidamage);
		if (pi->lodamage!=dummy.lodamage)
			fprintf(iWsc, "LODAMAGE %i\n", pi->lodamage);
		if (pi->auxdamage!=dummy.auxdamage)
			fprintf(iWsc, "AUXDAMAGE %i\n", pi->auxdamage);
		if (pi->damagetype!=dummy.damagetype)
			fprintf(iWsc, "DAMAGETYPE %i\n", pi->damagetype);
		if (pi->auxdamagetype!=dummy.auxdamagetype)
			fprintf(iWsc, "AUXDAMAGETYPE %i\n", pi->auxdamagetype);
		if (pi->ammo !=dummy.ammo )
			fprintf(iWsc, "AMMO %i\n", pi->ammo);
		if (pi->ammoFx !=dummy.ammoFx )
			fprintf(iWsc, "AMMOFX %i\n", pi->ammoFx);
		if (pi->st!=dummy.st)
			fprintf(iWsc, "ST %i\n", pi->st);
		if (pi->time_unused!=dummy.time_unused)
			fprintf(iWsc, "TIME_UNUSED %i\n", pi->time_unused);
		if (pi->weight!=dummy.weight)
			fprintf(iWsc, "WEIGHT %i\n", pi->weight);
		if (pi->hp!=dummy.hp)
			fprintf(iWsc, "HP %i\n", pi->hp);
		if (pi->maxhp!=dummy.maxhp)
			fprintf(iWsc, "MAXHP %i\n", pi->maxhp ); // Magius(CHE)
		if (pi->rank!=dummy.rank)
			fprintf(iWsc, "RANK %i\n", pi->rank ); // Magius(CHE)
		if (pi->st2!=dummy.st2)
			fprintf(iWsc, "ST2 %i\n", pi->st2);
		if (pi->dx!=dummy.dx)
			fprintf(iWsc, "DX %i\n", pi->dx);
		if (pi->dx2!=dummy.dx2)
			fprintf(iWsc, "DX2 %i\n", pi->dx2);
		if (pi->in!=dummy.in)
			fprintf(iWsc, "IN %i\n", pi->in);
		if (pi->in2!=dummy.in2)
			fprintf(iWsc, "IN2 %i\n", pi->in2);
		if (pi->spd!=dummy.spd)
			fprintf(iWsc, "SPD %i\n", pi->spd);
		if (pi->poisoned!=dummy.poisoned)
			fprintf(iWsc, "POISONED %i\n", pi->poisoned);
		if (pi->wipe!=dummy.wipe)
			fprintf(iWsc, "WIPE %i\n", pi->wipe);
		if (pi->magic!=dummy.magic)
			fprintf(iWsc, "MOVABLE %i\n", pi->magic);
		if (pi->getOwnerSerial32()!=dummy.getOwnerSerial32())
			fprintf(iWsc, "OWNER %i\n", pi->getOwnerSerial32());
		if (pi->visible!=dummy.visible)
			fprintf(iWsc, "VISIBLE %i\n", pi->visible);
		if (pi->dir!=dummy.dir)
			fprintf(iWsc, "DIR %i\n", pi->dir);
		if (pi->priv!=dummy.priv)
			fprintf(iWsc, "PRIV %i\n", pi->priv);
		if (pi->value!=dummy.value)
			fprintf(iWsc, "VALUE %i\n", pi->value);
		if (pi->restock!=dummy.restock)
			fprintf(iWsc, "RESTOCK %i\n", pi->restock);
		if (pi->trigger!=dummy.trigger)
			fprintf(iWsc, "TRIGGER %i\n", pi->trigger);
		if (pi->trigtype!=dummy.trigtype)
			fprintf(iWsc, "TRIGTYPE %i\n", pi->trigtype);
		if (pi->disabled!=dummy.disabled)
			fprintf(iWsc, "DISABLED %i\n", pi->disabled);
		if (pi->disabledmsg!=NULL)
			fprintf(iWsc, "DISABLEMSG %s\n", pi->disabledmsg->c_str() );
		if (pi->tuses!=dummy.tuses)
			fprintf(iWsc, "USES %i\n", pi->tuses);
		if (pi->good!=dummy.good )
			fprintf(iWsc, "GOOD %i\n", pi->good); // Magius(CHE)
		if (pi->secureIt!=dummy.secureIt)
			fprintf(iWsc, "SECUREIT %i\n", pi->secureIt);
		if (pi->smelt!=dummy.smelt)
			fprintf(iWsc, "SMELT %i\n", pi->smelt);
		if (pi->itemSoundEffect!=dummy.itemSoundEffect)
			fprintf(iWsc, "SOUNDFX %i\n", pi->itemSoundEffect);
		// Spawns
		if (pi->getSpawnSerial() != dummy.getSpawnSerial())
			fprintf(iWsc, "SPAWNSERIAL %i\n", pi->getSpawnSerial());
		if (pi->getSpawnRegion() != dummy.getSpawnRegion())
			fprintf(iWsc, "SPAWNREGION %i\n", pi->getSpawnRegion());
		//if (strlen(pi->desc)>0)	fprintf(iWsc, "DESC %s\n", pi->desc);	// save out our vendor description
		if (!pi->vendorDescription.empty())
			fprintf(iWsc, "DESC %s\n", pi->vendorDescription.c_str() );
		if (pi->hasTempfx())
		{
			TempfxVector *itemTempfxVec = pi->getTempfxVec( );
			TempfxVector::iterator iter=itemTempfxVec->begin();
			for ( ; iter != itemTempfxVec->end(); iter++)
			{
				tempfx::cTempfx fx = *iter;
				fx.save(iWsc);
			}
		}


#define SAVEITEMEVENT(A,B) { if (pi->amxevents[B]) if (pi->amxevents[B]->shouldBeSaved()) fprintf(iWsc, "%s %s\n", A, pi->amxevents[B]->getFuncName()); }
//#define SAVEITEMEVENT(A,B) { AmxEvent *event = pi->getAmxEvent( B ); if ( event ) if ( event->shouldBeSaved() ) fprintf(iWsc, "%s %s\n", A, event->getFuncName()); }

		SAVEITEMEVENT("@ONSTART", EVENT_IONSTART);
		SAVEITEMEVENT("@ONCHECKCANUSE", EVENT_IONCHECKCANUSE);
		SAVEITEMEVENT("@ONCLICK", EVENT_IONCLICK);
		SAVEITEMEVENT("@ONDAMAGE", EVENT_IONDAMAGE);
		SAVEITEMEVENT("@ONDBLCLICK", EVENT_IONDBLCLICK);
		SAVEITEMEVENT("@ONDECAY", EVENT_IONDECAY);
		SAVEITEMEVENT("@ONDROPINLAND", EVENT_IDROPINLAND);
		SAVEITEMEVENT("@ONDROPONCHAR", EVENT_IDROPONCHAR);
		SAVEITEMEVENT("@ONEQUIP", EVENT_IONEQUIP);
		SAVEITEMEVENT("@ONLOCKPICK", EVENT_IONLOCKPICK);
		SAVEITEMEVENT("@ONPOISONED", EVENT_IONPOISONED);
		SAVEITEMEVENT("@ONPUTINBACKPACK", EVENT_IPUTINBACKPACK);
		SAVEITEMEVENT("@ONREMOVETRAP", EVENT_IONREMOVETRAP);
		SAVEITEMEVENT("@ONSTOLEN", EVENT_IONSTOLEN);
		SAVEITEMEVENT("@ONTRANSFER", EVENT_IONTRANSFER);
		SAVEITEMEVENT("@ONUNEQUIP", EVENT_IONUNEQUIP);
		SAVEITEMEVENT("@ONWALKOVER", EVENT_IONWALKOVER);
		SAVEITEMEVENT("@ONPUTITEM", EVENT_IONPUTITEM);
		SAVEITEMEVENT("@ONTAKEFROMCONTAINER", EVENT_ITAKEFROMCONTAINER);

		//
		// SAVE NEW AMX VARS
		//
		amxVS.saveVariable( pi->getSerial32(), iWsc );

		fprintf(iWsc, "}\n\n");
	}
}

/*!
\brief Save prison
\author Endymion
*/
void CWorldMain::savePrison()
{
	fprintf(jWsc, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(jWsc, "// || NoX-Wizard jail save (nxwjail.wsc)                                  ||\n");
	fprintf(jWsc, "// || Automatically generated on worldsaves                               ||\n");
	fprintf(jWsc, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(jWsc, "// || Generated by NoX-Wizard version %s %s               ||\n", VERNUMB, OSFIX);
	fprintf(jWsc, "// || Requires NoX-Wizard version 0.82 to be read correctly               ||\n");
	fprintf(jWsc, "// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n\n");

	JAILEDVECTOR::iterator iter( prison::jailed.begin() ), end( prison::jailed.end() );
	for(; iter != end; ++iter )
	{
		fprintf(jWsc, "SECTION JAILED\n");
		fprintf(jWsc, "{\n");
		fprintf(jWsc, "SERIAL %i\n",	iter->serial);
		fprintf(jWsc, "OLDX %i\n",	iter->oldpos.x);
		fprintf(jWsc, "OLDY %i\n",	iter->oldpos.y);
		fprintf(jWsc, "OLDZ %i\n",	iter->oldpos.z);
		fprintf(jWsc, "OLDDISPZ %i\n", 	iter->oldpos.dispz);
		fprintf(jWsc, "CELL %i\n",	iter->cell);
		fprintf(jWsc, "SECS %i\n", 	(iter->timer - uiCurrentTime) / MY_CLOCKS_PER_SEC );
		fprintf(jWsc, "WHY %s\n",	iter->why.c_str() );
		fprintf(jWsc, "}\n\n");
	}
	fprintf(jWsc, "EOF\n");
}

void CWorldMain::loadjailed()
{

	cJailed j;

	do
	{
		readw2();
		switch(script1[0])
		{
			case 'C':
			case 'c':
				if (!(strcmp(script1, "CELL")))
					j.cell=str2num(script2);
				break;
			case 'O':
			case 'o':
				if (!(strcmp(script1, "OLDX")))
					j.oldpos.x=(UI16)str2num(script2);
				else if(!(strcmp(script1, "OLDY")))
					j.oldpos.y=(UI16)str2num(script2);
				else if(!(strcmp(script1, "OLDZ")))
					j.oldpos.z=(SI08)str2num(script2);
				else if(!(strcmp(script1, "OLDDISPZ")))
					j.oldpos.dispz=(SI08)str2num(script2);
				break;
			case 'W':
			case 'w':
				if (!strcmp(script1, "WHY"))
					j.why=script2;
				break;
			case 'S':
			case 's':
				if(!(strcmp(script1, "SERIAL")))
					j.serial=str2num(script2);
				else if(!(strcmp(script1, "SECS")))
					j.sec=str2num(script2);
				break;
		}

	}  while (strcmp(script1, "}"));
	P_CHAR pc = pointers::findCharBySerial( j.serial );
	if(ISVALIDPC(pc) && (j.sec > 0)) {
		j.timer=uiCurrentTime+(MY_CLOCKS_PER_SEC * j.sec );
		prison::jailed.push_back( j );
		pc->jailed=true;
	}
	else ConOut("Warning: Not valid jailed ( serial %i ), erasing it", j.serial );


}


/*!
\brief Load jail
\author Endymion
*/
void CWorldMain::loadPrison()
{
	std::string fileName( SrvParms->savePath + SrvParms->jailWorldfile + SrvParms->worldfileExtension );
	wscfile = fopen(fileName.c_str(),"r");
	if(!wscfile)
	{
		WarnOut("%s not found.\n", fileName.c_str());
		return;
	}
	else
	{

		ConOut("Loading prison ");
		prison::jailed.clear();
		do
		{
			readw3();
			if (!(strcmp(script1, "SECTION")))
			{
				if (!(strcmp(script2, "JAILED")))
				{
					loadjailed();
				}
			}
		}	while (strcmp(script1,"EOF") && !feof(wscfile) );


		fclose(wscfile);
		ConOut("[DONE]\n"); // Magius(CHE)

	}

}



void CWorldMain::realworldsave ()
{
	//Luxor: reload dynamic spawners here.
	// Wintermute, but not in every worldsave :)
	// Spawns->clearDynamic();
	cAllObjectsIter objs;
	this->chr_curr=1;
	this->itm_curr=1;
	P_ITEM pi = NULL;
	for( objs.rewind(); !objs.IsEmpty(); objs++ )
	{
		if( isCharSerial( objs.getSerial() ) )
			SaveChar( (P_CHAR)(objs.getObject()) );
		else 
		{
			pi = static_cast<P_ITEM>(objs.getObject());
			if ( ISVALIDPI(pi))
				SaveItem( pi );
			else
				objects.eraseObject(pi);
		}
	}

	fprintf(cWsc,"\nEOF\n\n");
	fclose(cWsc); cWsc=NULL;

	fprintf(iWsc, "EOF\n\n");
	fclose(iWsc); iWsc=NULL;
	Spawns->resetSpawnTime();
}

/* 

  Binary save code
  	ConOut("Finished text save at %d\n", getclock());
	std::string outfile=SrvParms->savePath + SrvParms->characterWorldfile + SrvParms->worldfileExtension + ".bin";
	ofstream tmpBinary;
	tmpBinary.open(outfile.c_str(), ios::out);
	ConOut("Starting save at %d\n", getclock());
	cAllObjectsIter objs2;
	int count=0;
	for( objs2.rewind(); !objs2.IsEmpty(); objs2++ )
	{
		if( isCharSerial( objs2.getSerial() ) )
		{
			((P_CHAR)objs2.getObject())->serialize(&tmpBinary);
			count++;
		}
	}
	ConOut("Finished binary save at %d saved %d chars\n", getclock(), count);
	tmpBinary.close();
*/
