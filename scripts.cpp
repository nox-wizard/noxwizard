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
\brief Functions that handle the timer controlled stuff (? Akron)
\author Duke
\date 26/10/2000
*/

#include "nxwcommn.h"
#include "debug.h"
#include "basics.h"
#include "globals.h"
#include "scripts.h"
#include "inlines.h"
#include "race.h"

char g_strScriptTemp[TEMP_STR_SIZE]; //xan -> this is terrible :(





void readwscline () // Read line from *.WSC
{


	int i, valid=0;
	char c;
	g_strScriptTemp[0]=0;
	while (!valid)
	{
		i=0;
		if (feof(wscfile)) return;
		c=(char)fgetc(wscfile);
		while (c!=10)
		{
			if (c!=13)
			{
				g_strScriptTemp[i]=c;
				i++;
			}
			if (feof(wscfile) || i>=512 ) return;
			c=(char)fgetc(wscfile);
		}
		g_strScriptTemp[i]=0;
		valid=1;
		if (g_strScriptTemp[0]=='/' && g_strScriptTemp[1]=='/') valid=0;
		if (g_strScriptTemp[0]=='{') valid=0;
		if (g_strScriptTemp[0]==0) valid=0;
		if (g_strScriptTemp[0]==10) valid=0;
		if (g_strScriptTemp[0]==13) valid=0;
	}

}

// reads the ENTIRE line, spaces + ='s DONT't tokenize the line, but sections, line feeds etc are ignored
// LB
void readFullLine ()
{


	int i=0;
	readwscline();
	script1[0]=0;
	while(g_strScriptTemp[i]!=0  && i<1024 ) i++;
	strncpy(script1, g_strScriptTemp, i);
	script1[i]=0;
	return;

}

void readw2 ()
{


	int i=0;

	readwscline();
	script1[0]=0;
	script2[0]=0;
	script3[0]=0;
	while(g_strScriptTemp[i]!=0 && g_strScriptTemp[i]!=' ' && g_strScriptTemp[i]!='=' && i<1024 ) i++;
	strncpy(script1, g_strScriptTemp, i);
	script1[i]=0;
	if (script1[0]!='}' && g_strScriptTemp[i]!=0) strcpy(script2, (g_strScriptTemp+i+1));
	return;

}


void readw3 ()
{


	int i=0,j;

	readwscline();
	script1[0]=0;
	script2[0]=0;
	script3[0]=0;
	while(g_strScriptTemp[i]!=0 && g_strScriptTemp[i]!=' ' && g_strScriptTemp[i]!='=' && i<1024 ) i++;
	strncpy(script1, g_strScriptTemp, i);
	script1[i]=0;
	if (script1[0]=='}' || g_strScriptTemp[i]==0) return;
	i++;
	j=i;
	while(g_strScriptTemp[i]!=0 && g_strScriptTemp[i]!=' ' && g_strScriptTemp[i]!='=' && i <1024 ) i++;
	strncpy(script2, (g_strScriptTemp+j), i-j);
	script2[i-j]=0;
	strcpy(script3, (g_strScriptTemp+i+1));


}

/*!
\brief Gets the token number num in the passed string
\author Sparhawk, ported to std::string by Akron
\param s the string to get the token from
\param num 0-based index of the token
*/
void gettokennum(std::string s, int num)
{
	int j = 0;

	memset(gettokenstr, 0, 255);

	std::string::iterator it( s.begin() ), end( s.end() );
	int tokenfound = -1;

	while( it != end && tokenfound != num )
	{
		if( isspace( *it ) )
			++it;
		else
		{
			if( ++tokenfound == num )
			{
				do
				{
					gettokenstr[ j++ ] = *it;
				} while( (++it != end ) && !isspace( *it ) );
			}
			else
			{
				++it;
				while( it != end )
					if( !isspace( *it ) )
						++it;
					else
						break;
			}
		}
	}
/*
	int j = 0, loopexit = 0;
	std::string::iterator i = s.begin();
	memset(gettokenstr, 0, 255);
	
	while( num != 0 && (++loopexit < MAXLOOPS) )
	{
		if ( *i == 0 )
			return;
		else
		{
			if( isspace(*i) && i != s.begin() && !isspace(*(i-1)) )
				num--;
			i++;
		}
	}

	loopexit=0;
	while(num!=-1 && (++loopexit < MAXLOOPS) )
	{
		if ( *i == 0 )
			return;
		else
		{
			if ( isspace(*i) && i != s.begin() && !isspace(*(i+1)) )
				num--;
			else
				gettokenstr[j++] = *i;
			i++;
		}
	}
*/
}

int getRangedValue( std::string str )
{
	int	lovalue ,
		hivalue ,
		retcode = 0;

	gettokennum( str, 0);
	lovalue=str2num(gettokenstr);
	gettokennum( str, 1);
	hivalue=str2num(gettokenstr);

	if (hivalue) {
		retcode = RandomNum(lovalue, hivalue);
	} else {
		retcode = lovalue;
	}
	return retcode;
}
// Added by Krozy on 7-Sep-98
// New getstatskillvalue function.
// Takes a string, gets the tokens.
// If its one value - It returns that value.
// If its two values - It gets a random number between the values
int getRangedValue(char *stringguy) {

	char values[512];
	int lovalue,hivalue,retcode;

	strcpy(values, stringguy);
	gettokennum(values, 0);
	lovalue=str2num(gettokenstr);
	gettokennum(values, 1);
	hivalue=str2num(gettokenstr);

	if (hivalue) {
		retcode = RandomNum(lovalue, hivalue);
	} else {
		retcode = lovalue;
	}
	return retcode;

}

void splitLine( char* source, char *head, char *tail )
{
  char* sourceIndex = source;
  char* headIndex = head;
  char* tailIndex = tail;

  while (*sourceIndex != '\0' && *sourceIndex == ' ' )
    ++sourceIndex;

  while (*sourceIndex != '\0' && *sourceIndex != ' ' )
  {
    *headIndex = *sourceIndex;
    ++sourceIndex;
    ++headIndex;
  }
  *headIndex = '\0';

  while (*sourceIndex != '\0' && *sourceIndex == ' ' )
    ++sourceIndex;

  while (*sourceIndex != '\0')
  {
    *tailIndex = *sourceIndex;
    ++sourceIndex;
    ++tailIndex;
  }
  *tailIndex = '\0';
}

void splitLine( const std::string& source, std::string& head, std::string& tail )
{
  SI32 index		= 0;
  SI32 sourceEnd	= source.size();

  while ( (index < sourceEnd) && isspace(source[index]) )
    ++index;

  while ((index < sourceEnd) && !isspace(source[index]) )
  {
    head += source[index];
    ++index;
  }

  while ( (index < sourceEnd) && isspace(source[index]) )
    ++index;

  while ((index < sourceEnd))
  {
    tail += source[ index ];;
    ++index;
  }
}



namespace Scripts {
	cScpScript* Advance = NULL; //cScpScript("advance.scp");
	cScpScript* Calendar = NULL; //cScpScript("calendar.scp");
	cScpScript* Carve = NULL; //cScpScript("carve.scp");
	cScpScript* Colors = NULL; //cScpScript("colors.scp");
	cScpScript* Create = NULL; //cScpScript("create.scp");
	cScpScript* CronTab = NULL; //cScpScript("crontab.scp");
	cScpScript* Envoke = NULL; //cScpScript("envoke.scp");
	cScpScript* Fishing = NULL; //cScpScript("fishing.scp");
	cScpScript* HardItems = NULL; //cScpScript("harditems.scp");
	cScpScript* House = NULL; //cScpScript("house.scp");
	cScpScript* Creatures = NULL;
	cScpScript* HostDeny = NULL; //cScpScript("host_deny.xss");
	cScpScript* HtmlStrm = NULL; //cScpScript("htmlstrm.scp");
	cScpScript* Items = NULL; //cScpScript("items.scp");
	cScpScript* Location = NULL; //cScpScript("location.scp");
	cScpScript* Menus = NULL; //cScpScript("menus.scp");
	cScpScript* MetaGM = NULL; //cScpScript("metagm.scp");
	cScpScript* Misc = NULL; //cScpScript("misc.scp");
	cScpScript* MList = NULL; //cScpScript("mlist.scp");
	cScpScript* MsgBoard = NULL; //cScpScript("msgboard.scp");
	cScpScript* Necro = NULL; //cScpScript("necro.scp");
	cScpScript* Newbie = NULL; //cScpScript("newbie.scp");
	cScpScript* Npc = NULL; //cScpScript("npc.scp");
	cScpScript* NpcMagic = NULL; //cScpScript("npcmagic.scp");
	cScpScript* Override = NULL; //cScpScript("override.scp");
	cScpScript* Polymorph = NULL; //cScpScript("polymorph.scp");
	cScpScript* Regions = NULL; //cScpScript("regions.scp");
	cScpScript* Skills = NULL; //cScpScript("skills.scp");
	cScpScript* Spawn = NULL; //cScpScript("spawn.scp");
	cScpScript* Speech = NULL; //cScpScript("speech.scp");
	cScpScript* Spells = NULL; //cScpScript("spells.scp");
	cScpScript* Teleport = NULL; //cScpScript("teleport.scp");
	cScpScript* Titles = NULL; //cScpScript("titles.scp");
	cScpScript* Triggers = NULL; //cScpScript("triggers.scp");
	cScpScript* WTrigrs = NULL; //cScpScript("wtrigrs.scp");
	cScpScript* Mountable = NULL; //cScpScript("mount.xss");
	cScpScript* WeaponInfo = NULL; //cScpScript("weaponinfo.xss");
	cScpScript* Containers = NULL; //cScpScript("containers.xss");
	cScpScript* Areas = NULL; //cScpScript("areas.xss");
};

void newScriptsInit()
{
	cScpScript* Dummy = new cScpScript("scripts/symbols.xss");
	safedelete(Dummy);
	Scripts::Advance = new cScpScript("scripts/advance.xss");
//	Scripts::Calendar = new cScpScript("calendar.scp");
	Scripts::Carve = new cScpScript("scripts/carve.xss");
	Scripts::Colors = new cScpScript("scripts/colors.xss");
	Scripts::Create = new cScpScript("scripts/create.xss");
//	Scripts::CronTab = new cScpScript("crontab.scp");
	Scripts::Envoke = new cScpScript("scripts/envoke.xss");
	Scripts::Fishing = new cScpScript("scripts/fishing.xss");
	Scripts::HardItems = new cScpScript("scripts/harditems.xss");
	Scripts::House = new cScpScript("scripts/house.xss");
	Scripts::Creatures = new cScpScript("scripts/creatures.xss");
	Scripts::HostDeny = new cScpScript("config/hostdeny.xss");
	Scripts::HtmlStrm = new cScpScript("scripts/html.xss");
	Scripts::Items = new cScpScript("scripts/items.xss");
	Scripts::Location = new cScpScript("scripts/location.xss");
	Scripts::Menus = new cScpScript("scripts/menus.xss");
	Scripts::MetaGM = new cScpScript("scripts/metagm.xss");
	Scripts::Misc = new cScpScript("scripts/misc.xss");
	Scripts::MsgBoard = new cScpScript("scripts/msgboard.xss");
	Scripts::Necro = new cScpScript("scripts/necro.xss");
	Scripts::Newbie = new cScpScript("scripts/newbie.xss");
	Scripts::Npc = new cScpScript("scripts/npc.xss");
	Scripts::NpcMagic = new cScpScript("scripts/npcmagic.xss");
//	Scripts::NTrigrs = new cScpScript("ntrigrs.scp");
//	Scripts::Override = new cScpScript("override.scp");
	Scripts::Polymorph = new cScpScript("scripts/polymorph.xss");
	Race::load("scripts/race.xss");
	Scripts::Regions = new cScpScript("scripts/regions.xss");
	Scripts::Skills = new cScpScript("scripts/skills.xss");
	Scripts::Spawn = new cScpScript("scripts/spawn.xss");
	Scripts::Speech = new cScpScript("scripts/speech.xss");
	Scripts::Spells = new cScpScript("scripts/spells.xss");
//	Scripts::Teleport = new cScpScript("teleport.scp");
	Scripts::Titles = new cScpScript("scripts/titles.xss");
	Scripts::Triggers = new cScpScript("scripts/triggers.xss");
//	Scripts::WTrigrs = new cScpScript("wtrigrs.scp");
	Scripts::Mountable = new cScpScript("scripts/mounts.xss");
	Scripts::WeaponInfo = new cScpScript("scripts/weaponinfo.xss");
	Scripts::Containers = new cScpScript("scripts/containers.xss");
	Scripts::Areas = new cScpScript("scripts/areas.xss");
}


void deleteNewScripts()
{
	cScpScript* Dummy = new cScpScript("scripts/symbols.xss");
	safedelete(Dummy);//
	safedelete(Scripts::Advance );//= new cScpScript("scripts/advance.xss");
//	Scripts::Calendar = new cScpScript("calendar.scp");
	safedelete(Scripts::Carve );//= new cScpScript("carve.scp");
	safedelete(Scripts::Colors );//= new cScpScript("colors.scp");
	safedelete(Scripts::Create );//= new cScpScript("create.scp");
//	Scripts::CronTab = new cScpScript("crontab.scp");
	safedelete(Scripts::Envoke );//= new cScpScript("envoke.scp");
	safedelete(Scripts::Fishing );//= new cScpScript("fishing.scp");
	safedelete(Scripts::HardItems );//= new cScpScript("harditems.scp");
	safedelete(Scripts::House );//= new cScpScript("house.scp");
	safedelete(Scripts::Creatures );
	safedelete(Scripts::HostDeny);
	safedelete(Scripts::HtmlStrm );//= new cScpScript("htmlstrm.scp");
	safedelete(Scripts::Items );//= new cScpScript("items.scp");
	safedelete(Scripts::Location );//= new cScpScript("location.scp");
	safedelete(Scripts::Menus );//= new cScpScript("scripts/menus.xss");
	safedelete(Scripts::MetaGM );//= new cScpScript("metagm.scp");
	safedelete(Scripts::Misc );//= new cScpScript("misc.scp");
	safedelete(Scripts::MList );//= new cScpScript("mlist.scp");
	safedelete(Scripts::MsgBoard);// = new cScpScript("msgboard.scp");
	safedelete(Scripts::Necro );//= new cScpScript("scripts/necro.xss");
	safedelete(Scripts::Newbie );//= new cScpScript("newbie.scp");
	safedelete(Scripts::Npc );// new cScpScript("scripts/npc.xss");
	safedelete(Scripts::NpcMagic);// = new cScpScript("npcmagic.scp");
//	safedelete(Scripts::NTrigrs);// = new cScpScript("ntrigrs.scp");
//	Scripts::Override = new cScpScript("override.scp");
	safedelete(Scripts::Polymorph);// = new cScpScript("polymorph.scp");
	safedelete(Scripts::Regions);// = new cScpScript("regions.scp");
	safedelete(Scripts::Skills);// = new cScpScript("skills.scp");
	safedelete(Scripts::Spawn);// = new cScpScript("spawn.scp");
	safedelete(Scripts::Speech);// = new cScpScript("speech.scp");
	safedelete(Scripts::Spells);// = new cScpScript("spells.scp");
	safedelete(Scripts::Teleport);// = new cScpScript("teleport.scp");
	safedelete(Scripts::Titles);// = new cScpScript("titles.scp");
	safedelete(Scripts::Triggers);// = new cScpScript("triggers.scp");
//	safedelete(Scripts::WTrigrs);// = new cScpScript("wtrigrs.scp");
	safedelete(Scripts::Mountable);
	safedelete(Scripts::WeaponInfo);
	safedelete(Scripts::Containers);
	safedelete(Scripts::Areas);
}


