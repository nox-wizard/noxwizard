#include "nxwcommn.h"
#include "debug.h"
#include <assert.h>

UI32 MapTileWidth  = 768;
UI32 MapTileHeight = 512;

//#define DEBUG_MAP_STUFF	1

#ifdef DEBUG_MAP_STUFF
void bitprint(FILE *fp, unsigned char x)
{
	for (int i = 7; i >= 0; --i)
	{
		if ((x & 0x80) == 0)
			fprintf(fp, "0");
		else
			fprintf(fp, "1");
		if (4 == i)
			fprintf(fp, " ");
		x = x << 1;
	}
}
#endif


cMapStuff::cMapStuff() : versionCache(0), versionRecordCount(0), versionMemory(0), StaMem(0), TileMem(0),
Cache(0), Map0CacheHit(0), Map0CacheMiss(0), StaticBlocks(0),
mapfile(0), sidxfile(0), statfile(0), verfile(0), tilefile(0), multifile(0), midxfile(0)
{
	// after a mess of bugs with the structures not matching the physical record sizes
	// i've gotten paranoid...
	assert(sizeof(versionrecord) >= VersionRecordSize);
	assert(sizeof(st_multi) >= MultiRecordSize);
	assert(sizeof(land_st) >= LandRecordSize);
	assert(sizeof(tile_st) >= TileRecordSize);
	assert(sizeof(map_st) >= MapRecordSize);
	assert(sizeof(st_multiidx) >= MultiIndexRecordSize);
	// staticrecord is not listed here because we explicitly don't read in some
	// unknown bytes to save memory

	mapname[0] = sidxname[0] = statname[0] = vername[0] =
		tilename[0] = multiname[0] = midxname[0] = '\0';

	memset(tilecache, 0x00, sizeof(tilecache));
	memset(Map0Cache, 0x00, sizeof(Map0Cache));
	memset(StaticCache, 0x00, sizeof(StaticCache));

}

cMapStuff::~cMapStuff()
{
	if (versionCache) safedeletearray(versionCache);

	if( mapfile )   safedelete(mapfile);
	if( sidxfile )  safedelete(sidxfile);
	if( statfile )  safedelete(statfile);
	if( verfile )   safedelete(verfile);
	if( tilefile )  safedelete(tilefile);
	if( multifile ) safedelete(multifile);
	if( midxfile )  safedelete(midxfile);
}

void cMapStuff::Load()
{
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	ConOut("Preparing to open *.mul files...\n(If they don't open, fix your paths in server.cfg)\n");
	mapfile= new MULFile(mapname,"rb");
	ConOut("	%s\n", mapname);
	if (mapfile==0 || !mapfile->ready())
    {
		sprintf(temp,"ERROR: Map %s not found...\n",mapname);
		LogError(temp);
		keeprun=false;
		return;
    }
	ConOut("	%s\n", sidxname);
	sidxfile= new MULFile(sidxname,"rb");
	if (sidxfile==0 || !sidxfile->ready())
    {
		sprintf(temp, "ERROR: Statics Index %s not found...\n",sidxname);
		LogError(temp);
		keeprun=false;
		return;
    }
	ConOut("	%s\n", statname);
	statfile= new MULFile(statname,"rb");
	if (statfile==0 || !statfile->ready())
    {
		sprintf(temp, "ERROR: Statics File %s not found...\n",statname);
		LogError(temp);
		keeprun=false;
		return;
	}
	ConOut("	%s\n", vername);
	verfile= new MULFile(vername,"rb");
	if (verfile==0 || !verfile->ready())
    {
		sprintf(temp,"ERROR: Version File %s not found...assuming empty file\n",vername);
		LogWarning(temp);
		safedelete(verfile);
		verfile = 0;
    }
	ConOut("	%s\n", tilename);
	tilefile= new MULFile(tilename,"rb");
	if (tilefile==0 || !tilefile->ready())
    {
		sprintf(temp, "ERROR: Tiledata File %s not found...\n",tilename);
		LogError(temp);
		keeprun=false;
		return;
    }
	ConOut("	%s\n", multiname);
	multifile= new MULFile(multiname,"rb");
	if (multifile==0 || !multifile->ready())
    {
		sprintf(temp,"ERROR: Multi data file %s not found...\n",multiname);
		LogError(temp);
		keeprun=false;
		return;
    }
	ConOut("	%s\n", midxname);
	midxfile=new MULFile(midxname,"rb");
	if (midxfile==0 || !midxfile->ready())
    {
		sprintf(temp, "ERROR: Multi index file %s not found...\n",midxname);
		LogError(temp);
		keeprun=false;
		return;
    }
	ConOut("Mul files successfully opened (but not yet loaded.)\n\n");

	CacheVersion();
	if( Cache )
    {
		if( Cache != 1 )
			Cache = 1; // Make sure this is only 1 or 0
		CacheTiles(); // has to be exactly here, or loadnewlorld cant access correct tiles ... LB
		CacheStatics();
    }

}

// this stuff is rather buggy thats why I separted it from uox3.cpp
// feel free to correct it, but be carefull
// bugfixing this stuff often has a domino-effect with walking etc.
// LB 24/7/99

// oh yah, well that's encouraging.. NOT! at least LB was kind enough to
// move this out into a separate file. he gets kudos for that!
//int cMapStuff::TileHeight(int tilenum)
signed char cMapStuff::TileHeight(int tilenum)
{

	tile_st tile;

	SeekTile(tilenum, &tile);

	// For Stairs+Ladders
	if (tile.flag2 & 4)
		return (signed char)(tile.height/2);	// hey, lets just RETURN half!
	return (tile.height);

}

/* given a z value, see if we can move
bool CanUseOldZ(int oldz, int newz)
{
	if (oldz == illegal_z)
		return true;

	return (newz <= oldz + MaxZstep) || newz >= oldz - 15;
}
*/

//<Anthalir>
signed char cMapStuff::StaticTop(Location where)
{
	return StaticTop(where.x, where.y, where.z);
}
//</Anthalir>

//o-------------------------------------------------------------o
//|   Function    :  char StaticTop(int x,int y,int oldz);
//|   Date        :  Unknown     Touched: Dec 21, 1998
//|   Programmer  :  Unknown
//o-------------------------------------------------------------o
//|   Purpose     :  Top of statics at/above given coordinates
//o-------------------------------------------------------------o
//int cMapStuff::StaticTop(int x, int y, int oldz)
signed char cMapStuff::StaticTop( UI32 x, UI32 y, signed char oldz)
{
//	int top = illegal_z;

	signed char top = illegal_z;
	int loopexit=0;

	MapStaticIterator msi(x, y);
	staticrecord *stat;
	while ( ((stat = msi.Next())!=0) && (++loopexit < MAXLOOPS) )
	{
//		int tempTop = stat->zoff + TileHeight(stat->itemid);
		signed char tempTop = stat->zoff + TileHeight(stat->itemid);
		if ((tempTop <= oldz + MaxZstep) && (tempTop > top))
		{
			top = tempTop;
		}
	}
	return top;

}


// author  : Lord Binary 17/8/99
// purpose : check if something is under a (static) roof.
//           I wrote this to check if a player is inside a static building (with roof)
//           to disable rain/snow when they enter buildings
//           this only works for static buildings.
//           Its perfect for weather stecks, if you want make a in_building()
//           you need to call it with x,y x+1,y x,y+1,x-1,y,x,y-1
//           if they all return 1 AND the player isnt running under a underpass or bridge
//           he/she is really in a building. though idunno yet how to check the later one (underpassing checks)
//           probably with the floor-bit

//bool cMapStuff::IsUnderRoof(int x, int y, int z)
bool cMapStuff::IsUnderRoof(UI32 x, UI32 y, signed char z)
{
	MapStaticIterator msi(x, y);
	staticrecord *stat;
	int loopexit=0;
	while ( ((stat = msi.Next())!=0) && (++loopexit < MAXLOOPS) )
	{
		tile_st tile;
		msi.GetTile(&tile);

		// this seems suspicious, if we are under a floor we return right away
		// i guess you can assume if they are under a floor they are outside
		// but can you promise that?? its too early for me to tell
		if (tile.flag1&1==1 && (tile.height+stat->zoff)>z )
			return false; // check the floor bit
		// if set -> this must be a underpassing/bridge

		if ((tile.height+stat->zoff)>z) // a roof  must be higher than player's z !
		{
			if (strstr("roof", (char *) tile.name) || strstr("shingle", (char *) tile.name))
				return true;
				/*if ( !strcmp(tile.name,"slate roof") || !strcmp(tile.name,"wooden shingles") ||
				!strcmp(tile.name,"thatch roof") || !strcmp(tile.name,"stone pavers") ||
				!strcmp(tile.name,"stone roof") || !strcmp(tile.name,"palm fronds") ||
				!strcmp(tile.name,"palm frond roof") || !strcmp(tile.name,"sandstone floor") ||
				!strcmp(tile.name,"marble roof") || !strcmp(tile.name,"tile roof") ||
				!strcmp(tile.name,"tent roof") || !strcmp(tile.name,"log roof") ||
				!strcmp( tile.name, "stone pavern") || !strcmp( tile.name, "wooden shingle") ||
			!strcmp( tile.name, "wooden board") || !strcmp( tile.name, "wooden boards")) return 1;*/

			// now why would not want to check the z value of wooden boards first??
			// this was after the if (.. >z), i'm moving this up inside of it
			if (!strcmp((char *) tile.name,"wooden boards"))
				return true;
			// ok, well going by the commented out section above we've likely got some bugs
			// because we've left out checking for "stone pavern" and "wooden board"
			// i'll stick these back in. even if these were bogus tile names it can't hurt
			if (!strcmp((char *) tile.name,"wooden board") ||
				!strcmp( (char *) tile.name, "stone pavern") ||
				!strcmp( (char *) tile.name, "stone pavers"))
				return true;
		}
	}
	return false;

}

//
// i guess this function returns where the tile is a 'blocker' meaning you can't pass through it
bool cMapStuff::DoesTileBlock(int tilenum)
{

	tile_st tile;

	SeekTile(tilenum, &tile);

	return (tile.flag1&0x40) == 0x40;

}

// crackerjack 8/9/1999 - finds the "corners" of a multitile object. I use
// this for when houses are converted into deeds.
void cMapStuff::MultiArea(P_ITEM pi, int *x1, int *y1, int *x2, int *y2)
{

	st_multi multi;
	MULFile *mfile = 0;
	SI32 length = 0;

	*x1 = *y1 = *x2 = *y2 = 0;
	SeekMulti(pi->id()-0x4000, &mfile, &length);
	length=length/MultiRecordSize;
	if (length == -1 || length>=17000000)
		length = 0;//Too big...  bug fix hopefully (Abaddon 13 Sept 1999)

	for (int j=0;j<length;j++)
	{
        mfile->get_st_multi(&multi);
		if(multi.x<*x1) *x1=multi.x;
		if(multi.x>*x2) *x2=multi.x;
		if(multi.y<*y1) *y1=multi.y;
		if(multi.y>*y2) *y2=multi.y;
	}
	*x1+= pi->getPosition().x;
	*x2+= pi->getPosition().x;
	*y1+= pi->getPosition().y;
	*y2+= pi->getPosition().y;

}

// return the height of a multi item at the given x,y. this seems to actually return a height
//int cMapStuff::MultiHeight(int i, int x, int y, int oldz)
signed char cMapStuff::MultiHeight(P_ITEM pi, UI32 x, UI32 y, signed char oldz)
{                                                                                                                                  	st_multi multi;

	MULFile *mfile = 0;
	SI32 length = 0;
	SeekMulti(pi->id()-0x4000, &mfile, &length);
	length = length / MultiRecordSize;
	if (length == -1 || length>=17000000)//Too big... bug fix hopefully (Abaddon 13 Sept 1999)
	{
		length = 0;
	}
	//ConOut("Doing multitile for #%x\n", items[i].serial);
	//Check for height at and above

	for (int j=0;j<length;j++)
	{
		mfile->get_st_multi(&multi);
		if (/*(multi.visible) &&*/ (pi->getPosition().x + multi.x == x) && (pi->getPosition().y + multi.y == y))
		{
			int tmpTop = pi->getPosition("z") + multi.z;
			if ((tmpTop<=oldz+MaxZstep)&& (tmpTop>=oldz-1))
			{
				//ConOut("At or above=%i\n",multi.z);
				return multi.z;
			}
			else if ((tmpTop>=oldz-MaxZstep)&& (tmpTop<oldz-1))
			{
				//ConOut("Below=%i\n",multi.z);
				return multi.z;
			}
		}
	}
	return 0;

}

//<Anthalir>
signed char cMapStuff::DynamicElevation(Location where)
{
	return DynamicElevation(where.x, where.y, where.z);
}
//</Anthalir>

// This was fixed to actually return the *elevation* of dynamic items at/above given coordinates
//int cMapStuff::DynamicElevation(int x, int y, int oldz)
signed char cMapStuff::DynamicElevation( UI32 x, UI32 y, signed char oldz)
{

	//int z = illegal_z;
	signed char z = illegal_z;
	signed char tempVal = illegal_z;


	NxwItemWrapper si;
	si.fillItemsAtXY( x, y );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pi=si.getItem();
		if(ISVALIDPI(pi)) {

			if (pi->id()>0x4000)
			{
				z = MultiHeight(pi, x ,y, oldz);
				// this used to do a z++, but that doesn't take into account the fact that
				// the itemp[] the multi was based on has its own elevation
				tempVal = z + pi->getPosition("z") + 1;
	//			z = z + pi->z + 1;
				z = tempVal;
				//z += pi->z + 1;
			}
			else if ((pi->getPosition().x == x) && (pi->getPosition().y == y))
			{
				signed char ztemp = pi->getPosition("z") + TileHeight( pi->id() );
				if ((ztemp <= oldz + MaxZstep) && (ztemp > z))
				{
					z=ztemp;
				}
			}
		}
	}
	return z;

}


int cMapStuff::MultiTile(P_ITEM pi, UI32 x, UI32 y, signed char oldz)
{

	SI32 length = 0;
	st_multi multi;
	MULFile *mfile = 0;
	SeekMulti(pi->id()-0x4000, &mfile, &length);
	length=length/MultiRecordSize;
	if (length == -1 || length>=17000000)//Too big... bug fix hopefully (Abaddon 13 Sept 1999)
	{
		ConOut("cMapStuff::MultiTile->Bad length in multi file. Avoiding stall.\n");
		length = 0;
	}

	for (int j=0;j<length;j++)
	{
		mfile->get_st_multi(&multi);
		if ((multi.visible && (pi->getPosition().x + multi.x == x) && (pi->getPosition().y + multi.y == y)
			&& (abs(pi->getPosition("z")+multi.z-oldz)<=1)))
		{
			int mt=multi.tile;
			/*if (DoesTileBlock(mt)) return mt;
			else return 0;*/
			return mt;
		}

	}
	return 0;

}

// returns which dynamic tile is present at (x,y) or INVALID if no tile exists
// originally by LB & just michael
//int cMapStuff::DynTile(int x, int y, int oldz)
int cMapStuff::DynTile(UI32 x, UI32 y, signed char oldz)
{

	NxwItemWrapper si;
	si.fillItemsAtXY( x, y );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pi=si.getItem();
		if(ISVALIDPI(pi)) {

			if ( pi->id()>0x4000 )
			{
				return MultiTile(pi, x ,y, oldz);
			}
			else if ((pi->getPosition().x == x) && (pi->getPosition().y == y))
			{
				return pi->id();
			}
		}

	}

	return INVALID;

}


/* returns object type, 0=map, 1=static, 2=dynamic
// an enum type would have been better, but at least they documented the values!
// This was unused, so I removed it for now! - fur 12/8/1999
char cMapStuff::o_Type(int x, int y, int oldz)
{
	int z = 0;

	if (StaticTop(x, y, oldz) == illegal_z)
		z=0;
	else
		z=1;

	if (DynamicElevation(x, y, oldz) != illegal_z)
		z=2;
	return z;
}
*/

// return the elevation of MAP0.MUL at given coordinates, we'll assume since its land
// the height is inherently 0
//int cMapStuff::MapElevation(int x, int y)
signed char cMapStuff::MapElevation(UI32 x, UI32 y)
{
	map_st map = SeekMap0( x, y );
	// make sure nothing can move into black areas
	if (430 == map.id || 475 == map.id || 580 == map.id || 610 == map.id ||
		611 == map.id || 612 == map.id || 613 == map.id)
		return illegal_z;
	// more partial black areas
	//if ((map.id >= 586 && map.id <= 601) || (map.id >= 610 && map.id <= 613))
	//	return illegal_z;
	return map.z;

}

//<Anthalir>
signed char cMapStuff::AverageMapElevation(Location where, int &id)
{
	return AverageMapElevation(where.x, where.y, id);
}
//</Anthalir>

// compute the 'average' map height by looking at three adjacent cells
//int cMapStuff::AverageMapElevation(int x, int y, int &id)
signed char cMapStuff::AverageMapElevation(UI32 x, UI32 y, int &id)
{
	// first thing is to get the map where we are standing

	map_st map1 = Map->SeekMap0( x, y );
	id = map1.id;
	// if this appears to be a valid land id, <= 2 is invalid
	if (map1.id > 2 && illegal_z != MapElevation(x, y))
	{
		// get three other nearby maps to decide on an average z?
		signed char map2z = Map->MapElevation( x + 1, y );
		signed char map3z = Map->MapElevation( x, y + 1);
		signed char map4z = Map->MapElevation( x + 1, y + 1);

		signed char testz = 0;
		if (abs(map1.z - map4z) <= abs(map2z - map3z))
		{
			if (illegal_z == map4z)
				testz = map1.z;
			else
			{
				testz = (signed char)((map1.z + map4z) >> 1);
				if (testz%2<0) --testz;
				// ^^^ Fix to make it round DOWN, not just in the direction of zero
			}
		} else {
			if (illegal_z == map2z || illegal_z == map3z)
				testz = map1.z;
			else
			{
				testz = (signed char)((map2z + map3z) >> 1);
				if (testz%2<0) --testz;
				// ^^^ Fix to make it round DOWN, not just in the direction of zero
			}
		}
		return testz;
	}
//	ConOut("Uhh.. Someone's walking on something funny..\n");
	return illegal_z;

}

/* return whether a tile is ????
** this really doesn't look like it does anything, because id1 will never be 68
char cMapStuff::MapType(int x, int y) // type of MAP0.MUL at given coordinates
{
	map_st map = SeekMap0( x, y );
	const char id1 = map.id>>8;
	const char id2 = map.id % 256;
	if ( id1 != 68 )
		return 1;
	else if( id2 != 2 )
		return 1;
	return 0;
}*/


// since the version data will potentiall affect every map related operation
// we are always going to cache it.   we are going to allocate maxRecordCount as
// given by the file, but actually we aren't going to use all of them, since we
// only care about the patches made to the 6 files the server needs.  so the
// versionRecordCount hold how many we actually saved
void cMapStuff::CacheVersion()
{
	if (verfile==0) return;
	verfile->seek(0, SEEK_SET);
	UI32 maxRecordCount = 0;
	verfile->getULong(&maxRecordCount);

	if (0 == maxRecordCount)
		return;
	if (0 == (versionCache = new versionrecord[maxRecordCount]))
		return;

	ConOut("Caching version data..."); fflush(stdout);
	versionMemory = maxRecordCount * sizeof(versionrecord);
	for (UI32 i = 0; i < maxRecordCount; ++i)
    {
		if (verfile->eof())
		{
			ErrOut("Avoiding bad read crash with verdata.mul.\n");
			return;
		}
		versionrecord *ver = versionCache + versionRecordCount;
		assert(ver);
		verfile->get_versionrecord(ver);

		// see if its a record we care about
		switch(ver->file)
		{
		case VERFILE_MULTIIDX:
		case VERFILE_MULTI:
		case VERFILE_TILEDATA:
			++versionRecordCount;
			break;
		case VERFILE_MAP:
		case VERFILE_STAIDX:
		case VERFILE_STATICS:
			// at some point we may need to handle these cases, but OSI hasn't patched them as of
			// yet, so no need slowing things down processing them
			ErrOut("VERDATA contains statics/map data. Ignoring version record.\n");
			break;
		default:
			// otherwise its for a file we don't care about
			break;
		}
		/*
		if (ver->file == VERFILE_MAP)
			ConOut("map0.mul patch!\n");
		else if (ver->file == VERFILE_STAIDX)
			ConOut("staidx0.mul index patch!\n");
		else if (ver->file == VERFILE_STATICS)
			ConOut("statics0.mul table patch!\n");
		else if (ver->file == VERFILE_MULTIIDX)
			ConOut("multi.idx table patch!\n");
		else if (ver->file == VERFILE_MULTI)
			ConOut("multi.mul table patch!\n");
		else if (ver->file == VERFILE_TILEDATA)
			ConOut("tiledata.mul table patch!\n");
			*/
    }
	ConOut("[DONE]\n(Cached %ld patches out of %ld possible).\n", versionRecordCount, maxRecordCount);
}


SI32 cMapStuff::VerSeek(SI32 file, SI32 block)
{
	if (!verfile)
		return 0;

	for (UI32 i = 0; i < versionRecordCount; ++i)
	{
		if (versionCache[i].file == file && versionCache[i].block == block)
		{
			verfile->seek(versionCache[i].filepos, SEEK_SET);
			return versionCache[i].length;
		}
	}
	return 0;

}


char cMapStuff::VerTile(int tilenum, tile_st *tile)
{
	if (!verfile || tilenum==-1) return 0;

	const SI32 block=(tilenum/32);
	if (VerSeek(VERFILE_TILEDATA, block+0x200)==0)
	{
		//  ConOut("No Ver\n");
		return 0;
	}
	else
	{
		//  ConOut("Ver\n");
		const SI32 pos=4+(TileRecordSize*(tilenum%32)); // correct
		verfile->seek(pos, SEEK_CUR);
		verfile->get_tile_st(tile);
		return 1;
	}

}

void cMapStuff::SeekTile(int tilenum, tile_st *tile)
{
	assert(tilenum >= 0);
	assert(tile);
	if (tilenum>=0x4000)
	{
		strcpy((char *) tile->name, "multi");
		tile->flag1=0;
		tile->flag2=0;
		tile->flag3=0;
		tile->flag4=0;
		tile->weight=255;
		tile->height=0;
		return;
	}

	if ( Cache )
	{
		// fill it up straight from the cache
		memcpy(tile, tilecache + tilenum, sizeof(tile_st));
#ifdef DEBUG_MAP_STUFF
		ConOut("SeekTile - cache hit!\n");
#endif
	}
	else
	{
		if (VerTile(tilenum, tile))
		{
#ifdef DEBUG_MAP_STUFF
			ConOut("Loaded tile %d from verdata.mul\n", tilenum);
#endif
		}
		else
		{
			// TILEDATA_TILE is the amount to skip past all of the land_st's
			// plus skip 4 bytes per block for the long separating them
			const SI32 block=(tilenum/32);
			const SI32 pos=TILEDATA_TILES+((block + 1) * 4) + (TileRecordSize*tilenum); // correct
			tilefile->seek(pos, SEEK_SET);
			tilefile->get_tile_st(tile);
		}

#ifdef DEBUG_MAP_STUFF
		ConOut("Tile #%d is '%s' ", tilenum, tile->name);
		ConOut("flag1: "); bitprint(stdout, tile->flag1);
		ConOut("flag2: "); bitprint(stdout, tile->flag2);
		ConOut("flag3: "); bitprint(stdout, tile->flag3);
		ConOut("flag4: "); bitprint(stdout, tile->flag4);
		ConOut("\n");
#endif
	}

}

void cMapStuff::CacheTiles()
{

	// temp disable caching so we can fill the cache
	Cache = 0;
	ConOut("Caching tiledata"); fflush(stdout);
	TileMem = 0x4000 * sizeof( tile_st );
	memset(tilecache, 0, TileMem);

	const int tenPercent = 0x4000 / 9;
	for (UI32 i = 0; i < 0x4000; ++i)
    {
		SeekTile(i, tilecache + i);

		if (i % tenPercent == 0)
		{
			ConOut("...%d0%%", 1 + (i / tenPercent)); fflush(stdout);
		}
    }
	ConOut(" Done.\n");
	Cache = 1;

#ifdef DEBUG_TILE_BITS
	for (int bit = 0x01; bit <= 0x0080; bit = bit << 1)
	{
		char buf[30];
		sprintf(buf, "static1-%d.txt", bit);
		FILE *fp = fopen(buf, "w");
		for (int i = 0; i < 0x4000; ++i)
		{
			tile_st *tile = tilecache + i;
			if ((tile->flag1 & bit) == bit)
			{
				fprintf(fp, "%04x tile '%-20.20s'\t", i, tile->name);
				fprintf(fp, "flag1: "); bitprint(fp, tile->flag1);
				fprintf(fp, " flag2: "); bitprint(fp, tile->flag2);
				fprintf(fp, " flag3: "); bitprint(fp, tile->flag3);
				fprintf(fp, " flag4: "); bitprint(fp, tile->flag4);
				fprintf(fp, "\n");
			}
		}
		fclose(fp);
		sprintf(buf, "static2-%d.txt", bit);
		fp = fopen(buf, "w");
		int i;
		for (i = 0; i < 0x4000; ++i)
		{
			tile_st *tile = tilecache + i;
			if ((tile->flag2 & bit) == bit)
			{
				fprintf(fp, "%04x tile '%-20.20s'\t", i, tile->name);
				fprintf(fp, " flag1: "); bitprint(fp, tile->flag1);
				fprintf(fp, " flag2: "); bitprint(fp, tile->flag2);
				fprintf(fp, " flag3: "); bitprint(fp, tile->flag3);
				fprintf(fp, " flag4: "); bitprint(fp, tile->flag4);
				fprintf(fp, "\n");
			}
		}
		fclose(fp);
		sprintf(buf, "static3-%d.txt", bit);
		fp = fopen(buf, "w");
		for (i = 0; i < 0x4000; ++i)
		{
			tile_st *tile = tilecache + i;
			if ((tile->flag3 & bit) == bit)
			{
				fprintf(fp, "%04x tile '%-20.20s'\t", i, tile->name);
				fprintf(fp, " flag1: "); bitprint(fp, tile->flag1);
				fprintf(fp, " flag2: "); bitprint(fp, tile->flag2);
				fprintf(fp, " flag3: "); bitprint(fp, tile->flag3);
				fprintf(fp, " flag4: "); bitprint(fp, tile->flag4);
				fprintf(fp, "\n");
			}
		}
		fclose(fp);
		sprintf(buf, "static4-%d.txt", bit);
		fp = fopen(buf, "w");
		for (i = 0; i < 0x4000; ++i)
		{
			tile_st *tile = tilecache + i;
			if ((tile->flag4 & bit) == bit)
			{
				fprintf(fp, "%04x tile '%-20.20s'\t", i, tile->name);
				fprintf(fp, " flag1: "); bitprint(fp, tile->flag1);
				fprintf(fp, " flag2: "); bitprint(fp, tile->flag2);
				fprintf(fp, " flag3: "); bitprint(fp, tile->flag3);
				fprintf(fp, " flag4: "); bitprint(fp, tile->flag4);
				fprintf(fp, "\n");
			}
		}
		fclose(fp);
	}
#endif

}

char cMapStuff::VerLand(int landnum, land_st *land)
{

	if (!verfile) return 0;
	const SI32 block=(landnum/32);
	if (VerSeek(VERFILE_TILEDATA, block)==0)
	{
		//  ConOut("No Ver\n");
		return 0;
	}
	//  ConOut("Ver\n");
	const SI32 pos=4+(LandRecordSize*(landnum%32)); // correct
	//fseek(verfile, pos, SEEK_CUR);
	//fread(land, sizeof(land_st), 1, verfile);
	verfile->seek(pos, SEEK_CUR);
	verfile->get_land_st(land);
	return 1;


}

void cMapStuff::SeekLand(int landnum, land_st *land)
{
	const SI32 block=(landnum/32);
	if (!VerLand(landnum, land))
	{
		const SI32 pos = ((block + 1) *4) + (LandRecordSize * landnum); // correct
		tilefile->seek(pos, SEEK_SET);
		tilefile->get_land_st(land);
	}

}

void cMapStuff::SeekMulti(int multinum, MULFile **mfile, SI32 *length)
{
	const int len=VerSeek(VERFILE_MULTI, multinum);
	if (len==0)
	{
		st_multiidx multiidx;
        midxfile->seek(multinum*MultiIndexRecordSize, SEEK_SET);
		midxfile->get_st_multiidx(&multiidx);
		multifile->seek(multiidx.start, SEEK_SET);
		*mfile=multifile;
		*length=multiidx.length;
	}
	else
	{
		*mfile=verfile;
		*length=len;
	}

}

//<Anthalir>
MapStaticIterator::MapStaticIterator(Location where, bool exact /*= true*/)
{
	MapStaticIterator(where.x, where.y, exact);
}
//</Anthalir>

/*
** Use this iterator class anywhere you would have used SeekInit() and SeekStatic()
** Unlike those functions however, it will only return the location of tiles that match your
** (x,y) EXACTLY.  They also should be significantly faster since the iterator saves
** a lot of info that was recomputed over and over and it returns a pointer instead
** of an entire structure on the stack.  You can call First() if you need to reset it.
** This iterator hides all of the map implementation from other parts of the program.
** If you supply the exact variable, it tells it whether to iterate over those items
** with your exact (x,y) otherwise it will loop over all items in the same cell as you.
** (Thats normally only used for filling the cache)
**
** Usage:
**		MapStaticIterator msi(x, y);
**
**      staticrecord *stat;
**      tile_st tile;
**      while (stat = msi.Next())
**      {
**          msi.GetTile(&tile);
**  		    ... your code here...
**	  	}
*/
MapStaticIterator::MapStaticIterator( UI32 x, UI32 y, bool exact) :
baseX((x) / 8), baseY((y) / 8), remainX((x) % 8), remainY((y) % 8), length(0), index(0),
pos(0), exactCoords(exact), tileid(0)
{
	assert(baseX < MapTileWidth);
	assert(baseY < MapTileHeight);

	if (baseX >= MapTileWidth)  return;
	if (baseY >= MapTileHeight) return;

	if ( Map->Cache )
	{
		length = Map->StaticCache[baseX][baseY].CacheLen;
	}
	else
	{
		const SI32 indexPos = (( baseX * MapTileHeight * 12L ) + ( baseY * 12L ));
		Map->sidxfile->seek(indexPos, SEEK_SET);
		if (!Map->sidxfile->eof() )
		{
			Map->sidxfile->getLong(&pos);
			if ( pos != -1 )
			{
				Map->sidxfile->getULong(&length);
				length /= StaticRecordSize;
				//ConOut("MSI indexpos: %ld, pos at %lx, length: %lu\n", indexPos, pos, length);
			}
		}
	}

}

staticrecord *MapStaticIterator::First()
{

	index = 0;
	return Next();

}

staticrecord *MapStaticIterator::Next()
{
	tileid = 0;
	int loopexit=0;
	if (index >= length)
		return 0;

	if ( Map->Cache )
	{
		staticrecord *ptr;
#ifdef MAP_CACHE_DEBUG
		// turn this on for debugging z-level stuff where you want to see where
		// the characters are in relation to their cell, and which places in the 8x8
		// cell have static tiles defined.
		if (index == 0)
		{
			ConOut("baseX: %lu, baseY: %lu, remX: %d, remY: %d\n", baseX, baseY, (int) remainX, (int) remainY);
			ConOut(" 01234567\n");
			char testmap[9][9];
			memset(testmap, ' ', 9*9);
			for (int tmp = 0; tmp < length; ++tmp)
			{
				ptr = Map->StaticCache[baseX][baseY].Cache + tmp;
				testmap[ptr->yoff][ptr->xoff] = 'X';
			}
			testmap[remainY][remainX] = 'O';
			for (int foo = 0; foo < 8; ++foo)
			{
				testmap[foo][8] = '\0';
				ConOut("%d%s\n", foo, testmap[foo]);
			}
		}
#endif

		ptr = Map->StaticCache[baseX][baseY].Cache;
		do {
			ptr += index++;
			if (!exactCoords || (ptr->xoff == remainX && ptr->yoff == remainY))
			{
				tileid = ptr->itemid;
				return ptr;
			}
		} while ((index < length) && (++loopexit < MAXLOOPS) );
		return 0;
	}

	// this was sizeof(OldStaRec) which SHOULD be 7, but on some systems which don't know how to pack,
	const SI32 pos2 = pos + (StaticRecordSize * index);	// skip over all the ones we've read so far
	Map->statfile->seek(pos2, SEEK_SET);
	loopexit=0;
	do {
		if ( Map->statfile->eof( ) )
			break;

		Map->statfile->get_staticrecord(&staticArray,1);
		++index;
		// Sparhawk: 	for unknown reasons get_staticrecord returned junk under Linux
		//		causing the server to abort on the assertions
		//		solved this by ignoring the prob
		//assert(staticArray.itemid >= 0);
		// if these are ever larger than 7 we've gotten out of sync
		//assert(staticArray.xoff < 0x08);
		//assert(staticArray.yoff < 0x08);
		if (staticArray.itemid >= 0)
		{

			if (!exactCoords || (staticArray.xoff == remainX && staticArray.yoff == remainY))
			{
#ifdef DEBUG_MAP_STUFF
				ConOut("Found static at index: %lu, Length: %lu, indepos: %ld\n", index, length, pos2);
				ConOut("item is %d, x: %d, y: %d\n", staticArray.itemid, (int) staticArray.xoff, (int) staticArray.yoff);
#endif
				tileid = staticArray.itemid;
				return &staticArray;
			}
		}
	} while ((index < length) && (++loopexit < MAXLOOPS));

	return 0;

}

// since 99% of the time we want the tile at the requested location, here's a
// helper function.  pass in the pointer to a struct you want filled.
void MapStaticIterator::GetTile(tile_st *tile) const
{
	assert(tile);
	Map->SeekTile(tileid, tile);

}

/*
** some clean up to the caching and it wasn't reporting all the memory actually
** used by the StaticCache[][] in cMapStuff
*/
void cMapStuff::CacheStatics( void )
{
	StaticBlocks = ( MapTileWidth * MapTileHeight );
	const UI32 tableMemory = StaticBlocks * sizeof(staticrecord);
	const UI32 indexMemory = StaticBlocks * sizeof(StaCache_st);
	StaMem = tableMemory + indexMemory;
	/*ConOut("Blocks: %ld, Index: %ld, Table: %ld, Static File Size: %ld\n",
	(long) StaticBlocks, (long)sizeof(StaCache_st), (long) sizeof(staticrecord),
	(long) StaticBlocks * StaticRecordSize);*/
	ConOut("Going to need %ld table bytes + %ld index bytes = %ld total bytes to cache statics...\n",
		tableMemory, indexMemory, StaMem);

	ConOut( "Caching Statics0"); fflush(stdout);

	// we must be in caching mode, only turn it off for now because we are
	// trying to fill the cache.
	assert(Cache);
	Cache = 0;

	const UI32 tenPercent = StaticBlocks / 9;
	UI32 currentBlock = 0;
	for( UI32 x = 0; x < MapTileWidth; x++ )
    {
		for( unsigned int y = 0; y < MapTileHeight; y++ )
		{
			StaticCache[x][y].Cache = 0;
			StaticCache[x][y].CacheLen = 0;

			MapStaticIterator msi(x * 8, y * 8, false);
			UI32 length = msi.GetLength();
			if (length)
			{
				StaticCache[x][y].CacheLen = length;
				StaticCache[x][y].Cache = new staticrecord[length];
				// read them all in at once!
				statfile->seek(msi.GetPos(), SEEK_SET);
				statfile->get_staticrecord(StaticCache[x][y].Cache, length);
			}
			if (currentBlock++ % tenPercent == 0)
			{
				ConOut("...%d0%%", 1 + (currentBlock / tenPercent)); fflush(stdout);
			}
		}
    }

	// reenable the caching now that its filled
	Cache = 1;
	ConOut("[DONE]\n");

}

map_st cMapStuff::SeekMap0( unsigned short x, unsigned short y )
{
	const UI16 x1 = x /8, y1 = y / 8, x2 = x % 8, y2 = y % 8;
	static SI16 CurCachePos = 0;

	if( !Cache ) {
		map_st map;
		const SI32 pos = ( x1 * MapTileHeight * 196 ) + ( y1 * 196 ) + ( y2 * 24 ) + ( x2 * 3 ) + 4;
		mapfile->seek(pos, SEEK_SET);
		mapfile->get_map_st(&map);
		return map;
    }

	SI16 i;
	// sorry zip, but these loops should have been checking the newest stuff in the
	// cache first, so its more likely to find a hit faster - fur
	for ( i = CurCachePos; i >= 0; --i )
	{
		if( ( Map0Cache[i].xb == x1 && Map0Cache[i].yb == y1 ) && ( Map0Cache[i].xo == x2 && Map0Cache[i].yo == y2 ) )
		{
			++Map0CacheHit;
			return Map0Cache[i].Cache;
		}
	}
	// if still not found, start at the end where the newer items are
	for ( i = MAP0CACHE - 1; i > CurCachePos; --i )
	{
		if( ( Map0Cache[i].xb == x1 && Map0Cache[i].yb == y1 ) && ( Map0Cache[i].xo == x2 && Map0Cache[i].yo == y2 ) )
		{
			++Map0CacheHit;
			return Map0Cache[i].Cache;
		}
	}

	Map0CacheMiss++;

	const SI32 pos = ( x1 * MapTileHeight * 196 ) + ( y1 * 196 ) + ( y2 * 24 ) + ( x2 * 3 ) + 4;
	mapfile->seek(pos, SEEK_SET);
	map_st *ptr = &(Map0Cache[CurCachePos].Cache);
	mapfile->get_map_st(ptr);

	Map0Cache[CurCachePos].xb=x1;
	Map0Cache[CurCachePos].yb=y1;
	Map0Cache[CurCachePos].xo=static_cast<unsigned char>(x2);
	Map0Cache[CurCachePos].yo=static_cast<unsigned char>(y2);

	// don't increment this until AFTER we have loaded everything, i had to fix what zippy did
	CurCachePos++;
	if (CurCachePos >= MAP0CACHE)
		CurCachePos = 0;

	return *ptr;
}

// these two functions don't look like they are actually used by anything
// anymore, at least we know which bit means wet
bool cMapStuff::IsTileWet(int tilenum)   // lord binary
{
	tile_st tile;
	SeekTile(tilenum, &tile);
	return (tile.flag1&0x80) == 0x80;

}

// i don't know what this bit means exactly, its a walkway? or you are allowed
// to walk?
bool cMapStuff::TileWalk(int tilenum)
{
	tile_st tile;

	SeekTile(tilenum, &tile);

	return (tile.flag4&0x04) == 0x04;
}

// Blocking statics at/above given coordinates?
//bool cMapStuff::DoesStaticBlock( int x, int y, int oldz )
bool cMapStuff::DoesStaticBlock( UI32 x, UI32 y, signed char oldz )
{

	MapStaticIterator msi(x, y);
	int loopexit=0;

	staticrecord *stat;
	while ( ((stat = msi.Next())!=0) && (++loopexit < MAXLOOPS) )
	{
		const int elev = stat->zoff + TileHeight(stat->itemid);
		//ConOut("tileh=%i ", TileHeight( stat.itemid ) );
		//ConOut("elev=%i stat.zoff=%i oldz=%i\n",elev,stat.zoff,oldz);
		//ConOut( "itemid=%i\n", stat.itemid );
		if( (elev > oldz) && (stat->zoff <= oldz ) )
		{
			//ConOut("OK I found it\n");
			bool btemp=DoesTileBlock(stat->itemid);
			//if (tilewet(stat.itemid)) return 0; // lb
			//ConOut("btemp=%i\n",btemp);
			if (btemp) return true;
		}
	}
	return false;

}

//<Anthalir>
signed char cMapStuff::Height(Location where)
{
	return Height(where.x, where.y, where.z);
}
//</Anthalir>

// Return new height of player who walked to X/Y but from OLDZ
//int cMapStuff::Height(int x, int y, int oldz)
signed char cMapStuff::Height( UI32 x, UI32 y, signed char oldz )
{
	// let's check in this order.. dynamic, static, then the map
	signed char dynz = DynamicElevation(x, y, oldz);
	if (illegal_z != dynz)
		return dynz;

	signed char staticz = StaticTop(x, y, oldz);
	if (illegal_z != staticz)
		return staticz;

	return MapElevation(x, y);

}

// can the monster move here from an adjacent cell at elevation 'oldz'
// use illegal_z if they are teleporting from an unknown z
//bool cMapStuff::CanMonsterMoveHere(int x, int y, int oldz)
bool cMapStuff::CanMonsterMoveHere(UI32 x, UI32 y, signed char oldz)
{

	if( x >= ( MapTileWidth * 8 ) || y >= ( MapTileHeight * 8 ) )
		return false;

	const signed char elev = Height(x, y, oldz);
	if (illegal_z == elev)
		return false;

	// is it too great of a difference z-value wise?
	if (oldz != illegal_z)
	{
		//ConOut("(%i %i %i) E=%i E-Z=%i Z-E=%i\n", x, y, oldz, elev, elev-oldz, oldz-elev );
		// you can climb MaxZstep, but fall up to 15
		if (elev - oldz > MaxZstep)
			return false;
		else if (oldz - elev > MaxZstep)
			return false;
	}

	// get the tile id of any dynamic tiles at this spot
	const int dt= DynTile(x,y,elev);

	// if there is a dynamic tile at this spot, check to see if its a blocker
	// if it does block, might as well short-circuit and return right away
	if (dt >= 0 && DoesTileBlock(dt))
			return false;

	// if there's a static block here in our way, return false
	if (DoesStaticBlock(x,y,elev))
			return false;

	return true;

}

bool cMapStuff::IsRoofOrFloorTile( tile_st *tile )
// checks to see if the tile is either a roof or floor tile
{

	if (tile->flag1&1 == 1 )
		return true; // check the floor bit

	if (strstr("roof", (char *) tile->name) || strstr("shingle", (char *) tile->name))
		return true;

	if( strstr( "floor", (char *)tile->name ) )
		return true;
	// now why would not want to check the z value of wooden boards first??
	// this was after the if (.. >z), i'm moving this up inside of it
	if (!strcmp((char *) tile->name,"wooden boards"))
		return true;
	// i'll stick these back in. even if these were bogus tile names it can't hurt
	if (!strcmp((char *) tile->name,"wooden board") ||
		!strcmp( (char *) tile->name, "stone pavern") ||
		!strcmp( (char *) tile->name, "stone pavers"))
		return true;

	return false;

}

bool cMapStuff::IsRoofOrFloorTile( unitile_st *tile )
// checks to see if the tile is either a roof or floor tile
{

	tile_st newTile;
	SeekTile( tile->id, &newTile );
	return IsRoofOrFloorTile( &newTile );

}
