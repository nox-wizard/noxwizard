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
\brief Declaration of miscellaneous class
\author Zippy

Declaration of class cCommands, cGuilds, cGump, MapStaticIterator,
cMapStuff, cFishing, cSkills, cNetworkStuff and a lot of structures
 */
#ifndef __Classes_h
#define __Classes_h

#include "client.h"
#include "targeting.h"

/*!
\brief New structure for basic guild related infos
\author DasRaetsel
*/
struct guild_st
{
	UI08	free;				//!< Guild slot used?
	TEXT	name[41];			//!< Name of the guild
	TEXT	abbreviation[4];		//!< Abbreviation of the guild
	SI32	type;				//!< Type of guild (0=standard/1=chaos/2=order)
	TEXT	charter[51];			//!< Charter of guild
	TEXT	webpage[51];			//!< Web url of guild
	SI32	stone;				//!< The serial of the guildstone
	SI32	master;				//!< The serial of the guildmaster
	SI32	recruits;			//!< Amount of recruits
	SI32	recruit[MAXGUILDRECRUITS+1];	//!< Serials of candidates
	SI32	members;			//!< Amount of members
	SI32	member[MAXGUILDMEMBERS+1];	//!< Serials of all the members
	SI32	wars;				//!< Amount of wars
	SI32	war[MAXGUILDWARS+1];		//!< Numbers of Guilds we have declared war to
	SI32	priv;				//!< Some dummy to remember some values
};

/*!
\brief Old Guild Class
\todo Remove it
*/
class cGuilds
{
private:
	void EraseMember(int c);
	void EraseGuild(int guildnumber);
	void ToggleAbbreviation(int s);
	int SearchSlot(int guildnumber, int type);
	void ChangeName(NXWSOCKET s, char *text);
	void ChangeAbbreviation(int s, char *text);
	void ChangeTitle(int s, char *text);
	void ChangeCharter(int s, char *text);
	void ChangeWebpage(int s, char *text);
	int CheckValidPlace(int s);
	void Broadcast(int guildnumber, char *text);
	void CalcMaster(int guildnumber);
	void SetType(int guildnumber, int type);
public:
	guild_st guilds[MAXGUILDS]; //lb, moved from nxwcommn.h cauz global variabels cant be changed in constuctors ...
	cGuilds();
	virtual ~cGuilds();
	int  GetType(int guildnumber);
	void StonePlacement(int s);
	void Menu(int s, int page);
	void Resign(P_CHAR pc, NXWSOCKET socket);
	void Recruit(int s);
	void TargetWar(int s);
	void StoneMove(int s);
	int Compare(P_CHAR player1,P_CHAR player2);
	void GumpInput(int s, int type, int index, char *text);
	void GumpChoice(NXWSOCKET s, int main, int sub);
	int SearchByStone(int s);
	void Title(int s, int player2);
	void Read(int guildnumber);
	void Write(FILE *wscfile);
	void CheckConsistancy();
};
////////////////////////END REMOVE////////////////////////////


// use this value whereever you need to return an illegal z value
const signed char illegal_z = -128;	// reduced from -1280 to -128, to fit in with a valid signed char


#endif
