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
cMapStuff, cNetworkStuff and a lot of structures
 */
#ifndef __Classes_h
#define __Classes_h

struct tile_st
{
 SI32 unknown1;  //!< longs must go at top to avoid bus errors - fur
 SI32 animation;
 UI08 flag1;
 UI08 flag2;
 UI08 flag3;
 UI08 flag4;
 UI08 weight;
 SI08 layer;
 SI08 unknown2;
 SI08 unknown3;
 SI08 height;
 TEXT name[23];	//!< manually padded to long to avoid bus errors - fur
} PACK_NEEDED;

struct land_st
{
 UI08 flag1;
 UI08 flag2;
 UI08 flag3;
 UI08 flag4;
 UI08 unknown1;
 UI08 unknown2;
 TEXT name[20];
};

struct map_st
{
	short int id;
	signed char z;
};

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

namespace gumps
{
	void Button(int s, UI32 button, char tser1, char tser2,char tser3, char tser4, UI32 type, char radio);
	void Input(int s);
	void Menu(NXWSOCKET s, int m, P_ITEM pi_it);
	void Open(int s, int i, int num, int num2);
};

class MapStaticIterator
{
private:
	staticrecord staticArray;
	SI32 pos;
	unsigned char remainX, remainY;
	UI32 index, length, tileid, baseX, baseY;
	bool exactCoords;

public:
	MapStaticIterator(UI32 x, UI32 y, bool exact = true);
	MapStaticIterator( Location where, bool exact= true);	// Added by Anthalir
	~MapStaticIterator() { };

	staticrecord *First();
	staticrecord *Next();
	void GetTile(tile_st *tile) const;
	UI32 GetPos() const { return pos; }
	UI32 GetLength() const { return length; }
};

extern UI32 MapTileWidth;//  = 768;
extern UI32 MapTileHeight;// = 512;

#define MAXMAPTILEWIDTH 768
#define MAXMAPTILEHEIGHT 512

class cMapStuff
{
//Variables
private:
	friend class MapStaticIterator;

        // moved from global vars into here - fur 11/3/1999
        MULFile *mapfile, *sidxfile, *statfile, *verfile, *tilefile, *multifile, *midxfile;

	// tile caching items
	tile_st tilecache[0x4000];

	// static caching items
	unsigned long StaticBlocks;
	struct StaCache_st
	{
		staticrecord *Cache;
		unsigned short CacheLen;   // i've seen this goto to at least 273 - fur 10/29/1999
	};

	// map caching items
	struct MapCache
	{
		unsigned short xb;
		unsigned short yb;
		unsigned char  xo;
		unsigned char  yo;
		map_st Cache;
	};
	MapCache Map0Cache[MAP0CACHE];

	// version caching items
	versionrecord *versionCache;
	UI32 versionRecordCount;

	// caching functions
	void CacheTiles( void );
	void CacheStatics( void );

public:
	// these used to be [512], thats a little excessive for a filename.. - fur
	char mapname[80], sidxname[80], statname[80], vername[80],
	  tilename[80], multiname[80], midxname[80];
	unsigned long StaMem, TileMem, versionMemory;
	unsigned int Map0CacheHit, Map0CacheMiss;
	// ok this is rather silly, allocating all the memory for the cache, even if
	// they haven't chosen to cache?? - fur
	StaCache_st StaticCache[MAXMAPTILEWIDTH][MAXMAPTILEHEIGHT];
	unsigned char Cache;
	
// Functions
private:
	char VerLand(int landnum, land_st *land);
	signed char MultiHeight(P_ITEM pi, UI32 x, UI32 y, signed char oldz);
	int MultiTile(P_ITEM pi, UI32 x, UI32 y, signed char oldz);
	SI32 VerSeek(SI32 file, SI32 block);
	char VerTile(int tilenum, tile_st *tile);
	bool IsTileWet(int tilenum);
	bool TileWalk(int tilenum);
	void CacheVersion();

	int DynTile( UI32 x, UI32 y, signed char oldz );
	bool DoesTileBlock(int tilenum);
	bool DoesStaticBlock(UI32 x, UI32 y, signed char oldz);

public:
	cMapStuff();
	~cMapStuff();

	void Load();

	// height functions
	bool IsUnderRoof(UI32 x, UI32 y, signed char z);
	signed char StaticTop(Location where);	// added by Anthalir
	signed char StaticTop(UI32 x, UI32 y, signed char oldz);
	signed char DynamicElevation(Location where);	// added by Anthalir
	signed char DynamicElevation(UI32 x, UI32 y, signed char oldz);
	signed char MapElevation(UI32 x, UI32 y);
	signed char AverageMapElevation(Location where, int &id);	// added by Anthalir
	signed char AverageMapElevation(UI32 x, UI32 y, int &id);
	signed char TileHeight( int tilenum );
	signed char Height(Location where);	// added by Anthalir
	signed char Height(UI32 x, UI32 y, signed char oldz);

	// look at tile functions
	void MultiArea(P_ITEM pi, int *x1, int *y1, int *x2, int *y2);
	void SeekTile(int tilenum, tile_st *tile);
	void SeekMulti(int multinum, MULFile **mfile, SI32 *length);
	void SeekLand(int landnum, land_st *land);
	map_st SeekMap0( unsigned short x, unsigned short y );
	bool IsRoofOrFloorTile( tile_st *tile );
	bool IsRoofOrFloorTile( unitile_st *tile );

	// misc functions
	bool CanMonsterMoveHere( UI32 x, UI32 y, signed char z );

};

#include "client.h"
// use this value whereever you need to return an illegal z value
const signed char illegal_z = -128;	// reduced from -1280 to -128, to fit in with a valid signed char

/*!
\brief Weight related stuff
*/
namespace weights {

	int	CheckWeight(P_CHAR pc);
	float	RecursePacks(P_ITEM bp);
	int	CheckWeight2(P_CHAR pc);
	void	NewCalc(P_CHAR pc);
	float	LockeddownWeight(P_ITEM pItem, int *total);
}

#include "targeting.h"

/*
class cNetworkStuff
{
public:
	cNetworkStuff();
	void enterchar(int s);
	void startchar(int s);
	void LoginMain(int s);
	void xSend(int s, const void *point, int length, int test);
	void Disconnect(int s);
	void ClearBuffers();
	void CheckConn();
	void CheckMessage();
	void SockClose();
	void FlushBuffer(int s);
	void LoadHosts_deny( void );
	bool CheckForBlockedIP(sockaddr_in ip_address);

	int kr,faul; // needed because global varaibles cant be changes in constructores LB

private:

	std::vector<ip_block_st> hosts_deny;

	void DoStreamCode(NXWSOCKET s);
	int  Pack(void *pvIn, void *pvOut, int len);
	void Login2(int s);
	void Relay(int s);
	void GoodAuth(int s);   
	void charplay (int s);
	void CharList(int s);
	int  Receive(int s, int x, int a);
	void GetMsg(int s);
	char LogOut(int s);
	void pSplit(char *pass0);
	void sockInit();

protected:

};

*/



#endif
