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
#include "los.h"

/*!
\brief Constructor
\author Duke
\param x1 First position ( x )
\param y1 First position ( y )
\param x2 Second position ( x )
\param y2 Second position ( y )
\note not need order, the function check for find the upper and down corner
*/
cRectangle::cRectangle(SI16 x1, SI16 y1, SI16 x2, SI16 y2)
{
	lowerLeftX=min(x1,x2);
	lowerLeftY=min(y1,y2);
	upperRightX=max(x1,x2);
	upperRightY=max(y1,y2);
}

/*!
\brief check if position is into the rectangle
\author Duke
\param x coord x
\param y coord y
\return true if into rectangle
*/
bool cRectangle::includes(SI16 x, SI16 y)
{
	return (!(x < lowerLeftX || x > upperRightX || y < lowerLeftY || y > upperRightY) );
}

/*!
\brief check if an rectangle is into this rectangle
\author Duke
\param other the other rectangle
\return true if into rectangle
\note into all or only partial make return true
*/
bool cRectangle::overlaps(cRectangle& other)
{
	return ((includes(other.lowerLeftX, other.lowerLeftY)  )||
			(includes(other.lowerLeftX, other.upperRightY) )||
			(includes(other.upperRightX,other.lowerLeftY)	 )||
			(includes(other.upperRightX,other.upperRightY) ));
}


/*!
\brief Constructor
\author Duke
\param posA source position
\param posB dest position
\return none
*/
cLoS::cLoS(Coord_cl& posA,Coord_cl& posB) : lineRect(posA.x,posA.y,posB.x,posB.y)
{
	pos1=posA;
	pos2=posB;

	noXdelta=noYdelta=noZdelta=false;
	if (pos1.x == pos2.x)
		noXdelta=true;
	if (pos1.y == pos2.y)
		noYdelta=true;

	Xdir=1, Ydir=1;
	if (pos1.x > pos2.x)
		Xdir = -1;
	if (pos1.y > pos2.y)
		Ydir = -1;
	if (pos1.z > pos2.z)
		Zdir = -1;

	incX=incY=incZ=1;	// initialize to nonzero values
	if (!noXdelta)		// if there is a x-dimension, both incY and incZ are based on x
	{
		incY= float (pos2.y-pos1.y)/(pos2.x-pos1.x);
		incZ= static_cast<float>(fabs(float (pos2.z-pos1.z)/(pos2.x-pos1.x)));
	}
	else 
	{
		if (!noYdelta)	// if all x are the same, both incX and incZ are based on y
		{
			incX= float (pos2.x-pos1.x)/(pos2.y-pos1.y);
			incZ= static_cast<float>(fabs(float (pos2.z-pos1.z)/(pos2.y-pos1.y)));
		}
	}
	incYhalf = static_cast<float>(fabs(incY)*0.5);	// just to speed up repeated calculations

	if (noXdelta && noYdelta)
	{
		minZ = min(pos1.z,pos2.z);
		maxZ = max(pos1.z,pos2.z);
	}
}


/*!
\brief Get z value at given position
\author Duke
\param x position
\param y position
\return the z value
\warning if both Xdelta and Ydelta are zero, the return value is undefined ie. it's 0, but meaningless !
*/
SI16 cLoS::getZatXY(SI16 x, SI16 y)
{
	if (!noXdelta)
		return static_cast<short>(pos1.z + (x-pos1.x)*incZ*Zdir);
	else
		return static_cast<short>(pos1.z + (y-pos1.y)*incZ*Zdir);
}

/*!
\brief ??Check if position is on le line of sight??
\author Duke
\param x position
\param y position
\return true ?? false ??
*/
bool cLoS::crossesTile(SI16 x, SI16 y)
{
	if (!lineRect.includes(x,y))
		return false;	// the tile is far from where the line is

	if (noXdelta || noYdelta)	// AND inside the box (see above)
		return true;			// => a vertical line

	// let's think of the tile as a square. The borders have a length of 1.
	// x/y is in the middle of the tile, so the borders are at +/- 0.5

	// determine the y-values of our *line* for the x-values of the square's borders
	float yLine = float(pos1.y + (x-pos1.x)*incY*Ydir);
	if ( (float(yLine + incYhalf)) < (float(y - 0.5)) )
		return false;	// the line goes through below the square
	if ( (float(yLine - incYhalf)) < (float(y + 0.5)) )
		return false;	// the line goes through above

	return true;
}

/*!
\brief Check if given id block the los
\author Duke
\param id the id
\return true if block
\todo add flag check
*/
bool cLoS::isBlocking(SI16 id)
{
	// Walls, Chimneys, ovens, not fences
	if (((id>=   6 && id<= 748))||((id>=  761)&&(id<=  881))||
		((id>= 895 && id<=1006))||((id>= 1057)&&(id<= 1061))||
		( id==1072)||(id==1073) ||((id>= 1080)&&(id<= 1166))||
		((id>=2347)&&(id<=2412))||((id>=16114)&&(id<=16134))||
		((id>=8538)&&(id<=8553))||((id>= 9535)&&(id<= 9555))||
		(id==12583))
		return true;
	
	// Doors, not gates
	if (( id>=1653 && id<=1782 )||( id>=8173 && id<=8188 ))
		return true;
	
	// Floors & Flat Roofing (Attacking through floors Roofs)
	if (((id>=1169 && id<=1413))||((id>=1508)&&(id<=1514))||
		((id>=1579 && id<=1586))||((id>=1591)&&(id<=1598)))
		return true;
	
	return false;
}

/*!
\brief Check if given id block the los, also with is height
\author Duke
\param id the id
\param baseZ the z of ??
\param zLine the z of ??
\return true if block 
*/
bool cLoS::isBlockedByHeight(SI16 id, SI16 baseZ, SI16 zLine)
{
	tile_st tile;
	Map->SeekTile(id, &tile);
	if ( (noXdelta && noYdelta) &&		// a vertikal line, zLine is undefined, using minZ & maxZ
		!(maxZ < baseZ) && !(minZ > (baseZ + tile.height)) )
	{
		if (isBlocking(id))
			return true;
	}
	else
	{
		float zLineTop    = float (zLine + 0.5*incZ);
		float zLineBottom = float (zLine - 0.5*incZ);
		if ( !(zLineTop < baseZ || zLineBottom > (baseZ + tile.height)) ) // not above or below
		{
			if (tile.height || incZ)
			{
				if (isBlocking(id))
					return true;
			}
		}
	}
	return false;
}

/*!
\brief Check if given item block the los
\author Duke
\param pi the item
\return true if block 
*/
bool cLoS::isBlockedByItem(P_ITEM pi)
{
	VALIDATEPIR(pi,true);
	if (pi->id1 < 0x40)	// unbelievable, but this significantly helps performance (Duke)
	{
		if (!crossesTile(pi->getPosition("x"), pi->getPosition("y")))
			return false;
		if (isBlockedByTile(pi))
			return true;
	}
	else
	{
		cRectangle maxRect( pi->getPosition("x") - BUILDRANGE,pi->getPosition("y") - BUILDRANGE,
							pi->getPosition("x") + BUILDRANGE,pi->getPosition("y") + BUILDRANGE);
		if (!maxRect.overlaps(lineRect))
			return false;
		if (isBlockedByMulti(pi))
			return true;
	}
	return false;
}

/*!
\brief Check if given multi item block the los
\author Duke
\param pi the multi item
\return true if block
*/
bool cLoS::isBlockedByMulti(P_ITEM pi)
{
	VALIDATEPIR(pi,true);
	
	MULFile *mfile;
	int length;
	Map->SeekMulti(pi->id()-0x4000, &mfile, &length);
	long count=length/sizeof(st_multi);
	if (count == -1 || count>=17000000)//Too big... bug fix hopefully (Abaddon 13 Sept 1999)
	{
		LogError("LoS - Bad length in multi file. Avoiding stall.");
		count = 0;
	}
	int j;
	for (j=0;j<count;j++)
	{
		st_multi multi;
		mfile->get_st_multi(&multi);
		if ((multi.visible) && crossesTile(pi->getPosition("x")+multi.x, pi->getPosition("y")+multi.y))
		{
			short zLine=getZatXY(pi->getPosition("x")+multi.x, pi->getPosition("y")+multi.y);
			if (isBlockedByHeight(multi.tile, pi->getPosition("z")+multi.z, zLine))
				return true;
		}
	}
	return false;
}

/*!
\brief Check if given tile block the los
\author Duke
\param pi the tile
\return true if block 
*/
bool cLoS::isBlockedByTile(P_ITEM pi)
{
	VALIDATEPIR(pi,true);	
	if (pi->visible==0)	// normally visible (!)
	{
		short zLine=getZatXY(pi->getPosition("x"), pi->getPosition("y"));
		
		if (isBlockedByHeight(pi->id(), pi->getPosition("z"), zLine))
			return true;
	}
	return false;
}

/*!
\brief Check if given position is blocked by land
\author Duke
\param x position
\param y position
\return true if block 
*/
bool cLoS::isBlockedByLand(SI16 x, SI16 y)
{
	map_st map1 = Map->SeekMap0( x, y );
	short id = map1.id;
	if (id != 2)
	{
		if ( // Mountain walls
			((id>= 431)&&(id<= 432))||
			((id>= 467)&&(id<= 475))||
			((id>= 543)&&(id<= 560))||
			((id>=1754)&&(id<=1757))||
			((id>=1787)&&(id<=1789))||
			((id>=1821)&&(id<=1824))||
			((id>=1851)&&(id<=1854))||
			((id>=1881)&&(id<=1884))
			)
		{
			return true;
		}
	}
	return false;
}

/*!
\brief Check if given position is blocked by static
\author Duke
\param x position
\param y position
\return true if block 
*/
bool cLoS::isBlockedByStatic(SI16 x, SI16 y)
{
	short zLine=getZatXY(x,y);
	MapStaticIterator msi(x, y);
	staticrecord *stat;
	int loopexit=0;
	while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
	{
		if( stat )
			if (isBlockedByHeight(stat->itemid, stat->zoff, zLine))
				return true;
	}
	return false;
}

/*!
\brief firstTile will iterate through the tiles *along* the line
\author Duke
\param x ptr to position
\param y ptr to position
\todo use references?
*/
void cLoS::firstTile(SI16* x, SI16* y)
{
	*x=pos1.x;
	*y=pos1.y;
}

/*!
\brief nextTile will iterate through the tiles *along* the line
\author Duke
\param x position
\param y position
\return false if no more tile
*/
bool cLoS::nextTile(SI16* x, SI16* y)
{
	if (noXdelta && noYdelta)
		return false;

	if (noXdelta)
	{
		*y += Ydir;
	}
	else if (noYdelta)
	{
		*x += Xdir;
	}
	else
	{
		// this part determines if our line leaves the tile through a side border or the top border
		// the variable names refer to a line that goes lower left to top right
		// xdir and ydir assure that it also works for other directions
		float nextRightBorderY = static_cast<float>(pos1.y + (*x - pos1.x + Xdir*0.5)*incY);
		float nextTopBorderY = static_cast<float>(*y + (Ydir*0.5));
		if ((nextTopBorderY-nextRightBorderY)*Ydir >= 0)
		{
			*x += Xdir;
		}
		if ((nextTopBorderY-nextRightBorderY)*Ydir <= 0)
		{
			*y += Ydir;
		}
	}
	if (*x == pos2.x && *y == pos2.y)	// a rather risky termination :-/ I traded the safer way for speed :)
		return false;

	return true;
}

/*!
\brief Check the Line of Sight between source and target ( flags otional )
\author Duke
\param source the source position
\param target the target position
\param checkfor the option flags 
\return ??
*/
int line_of_sight2(Coord_cl &source, Coord_cl &target, int checkfor)
{
	cLoS line(source,target);

	// checking map and static items first
	short x,y;
	line.firstTile(&x,&y);
	do
	{
		if (line.isBlockedByLand(x,y))
			return false;
		if (line.isBlockedByStatic(x,y))
			return false;
	}
	while (line.nextTile(&x,&y));

	// then check the dynamic tiles because they usually don't block

	NxwItemWrapper si;
	si.fillItemsNearXYZ( source.x, source.y, VISRANGE, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if ( ISVALIDPI(pi) && line.isBlockedByItem(pi))
			return false;
	}
	// later we can add a check for characters here

	return true;

}

/*!
\brief Check the Line of Sight between source and target ( flags otional )
\author Anthalir
\param a the source position
\param b the target position
\param checkfor the option flags
\param s socket
\return ??
*/
int line_of_sight(int s, Location a, Location b, int checkfor)
{
	return line_of_sight(s, a.x, a.y, a.z, b.x, b.y, b.z, checkfor);
}

/*!
\brief Check the Line of Sight between source and target ( flags otional )
\author Anthalir
\param s
\param x1 the source position x
\param y1 the source position y
\param z1 the source position z
\param x2 the target position x
\param y2 the target position y
\param z2 the target position z
\param checkfor the option flags 
\return ??
*/
int line_of_sight(int s, int x1, int y1, int z1, int x2, int y2, int z2, int checkfor)
{
	Coord_cl src, dst;
	src.x = x1; src.y = y1; src.z = z1;
	dst.x = x2; dst.y = y2; dst.z = z2;
	return line_of_sight2(src, dst, checkfor);
}


