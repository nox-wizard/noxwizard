// This code is used with friendly permission from Varan Dar from 
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================


// Include files

#include "network.h"
#include "items.h"
#include "globals.h"
#include "coord.h"
#include "data.h"
#include "inlines.h"
#include "set.h"

// System Includes
#include <math.h>
#include <set>

cCoord cCoord::null( (UI16)0xFFFF, (UI16)0xFFFF, (SI08)0xFF, (UI08)0x00FF );

cCoord::cCoord(void)
{
	x = y = z = map = 0;
}

cCoord::cCoord(const cCoord& clCoord)
{
	(*this) = clCoord;
}

cCoord::cCoord(UI16 uiX, UI16 uiY, SI08 siZ, UI08 uiMap, UI08 uiPlane)
{
	x = uiX;
	y = uiY;
	z = siZ;
	map = uiMap;
	//plane = uiPlane;
}


/*!
 * Calculates the distance between 2 coordinates. If they are in different planes and maps, the
 * return value is infinite (0xFFFFFFFF). The exception is if one of the planes is COORD_ALL_PLANES
 *
 * @param &src : Coordinate to compare
 *
 * @return UI32  : distance result
 */
unsigned int cCoord::distance( const cCoord &src ) const
{
	UI32 uiResult = ~0U; // Initialize with *infinite*
	if ( map == src.map )
	{
		R64 rfResult = sqrt(static_cast<R64>((x - src.x)*(x - src.x) + (y - src.y)*(y - src.y)/* + (z - src.z)*(z - src.z)*/));
		uiResult = static_cast<UI32>(floor(rfResult)); // truncate
	}
	return uiResult;
}

UI32 cCoord::distance ( const cCoord &a, const cCoord &b )
{
	return a.distance( b );
}

// Operators
cCoord& cCoord::operator=(const cCoord& clCoord)
{
	x = clCoord.x;
	y = clCoord.y;
	z = clCoord.z;
	map = clCoord.map;
	return (*this);
}

bool cCoord::operator== (const cCoord& src) const
{
	return (x == src.x && y == src.y && z == src.z && map == src.map);
}

bool cCoord::operator!= (const cCoord& src) const
{
	return !(x == src.x && y == src.y && z == src.z && map == src.map);
}

cCoord cCoord::operator+ (const cCoord& src) const
{
	return cCoord(this->x + src.x, this->y + src.y, this->z + src.z, this->map );
}

cCoord cCoord::operator- (const cCoord& src) const
{
	return cCoord(this->x - src.x, this->y - src.y, this->z - src.z, this->map );
}

/*
void cCoord::effect( UI16 id, UI08 speed, UI08 duration, UI16 hue, UI16 renderMode )
{
	cUOTxEffect effect;
	effect.setType( ET_STAYSOURCEPOS );
	effect.setSourcePos( (*this) );
	effect.setId( id );
	effect.setSpeed( speed );
	effect.setDuration( duration );
	effect.setHue( hue );
	effect.setRenderMode( renderMode );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && ( mSock->player()->pos().distance( (*this) ) <= mSock->player()->visualRange() ) )
			mSock->send( &effect );
	}
}
*/
// Calculates the direction from one location to another
unsigned char cCoord::direction( const cCoord &dest ) const
{
	unsigned char dir;
	short xdif, ydif;

	xdif = dest.x - x;
	ydif = dest.y - y;

	if ((xdif==0)&&(ydif<0)) dir=0;
	else if ((xdif>0)&&(ydif<0)) dir=1;
	else if ((xdif>0)&&(ydif==0)) dir=2;
	else if ((xdif>0)&&(ydif>0)) dir=3;
	else if ((xdif==0)&&(ydif>0)) dir=4;
	else if ((xdif<0)&&(ydif>0)) dir=5;
	else if ((xdif<0)&&(ydif==0)) dir=6;
	else if ((xdif<0)&&(ydif<0)) dir=7;
	else dir=255;

	return dir;
}


bool cCoord::lineOfSight( const cCoord &target, bool touch )
{
	return lineOfSight( target, 0, touch );
}

/*	idee hinter dem system
	wir wollen testen ob vom einem punkt aus eine sichtlinie auf irgendeinen punkt
	eines ausgedehnten objektes vorhanden ist
	problem dabei:
	versetzte l�cken m�ssen auch abgefangen werden
	l�sung:
	die sichtlinie zu jedem punkt (jedem z-level) des objektes einzeln zu berechnen
	dabei h�ren wir auf, sobald wir eine gefunden haben die durchgeht, oder wissen
	dass keine durchgeht
	wir fangen von oben an, da mauern die nur bis zu den beinen gehn extrem h�ufiger
	sind als mauern nur von oben den kopf verdecken und der algorithmus deshalb
	im allgemeinen wenn eine sichtlinie vorhanden ist diese sofort findet, falls nicht
	l�uft er wohl bis zum ende durch
	vorgehensweise bei der berechnung:
	wir erstellen erst eine liste aller x- und y-koordinaten durch die wir durchm�ssen
	und legen dann die geraden mit unterschiedlicher steigung in z-richtung durch

	zusatzbemerkungen:
	ein gegenstand verhindert sichkontakt, falls er das noshoot-flag hat
	berge (map) verhindern die sicht immer

	die koordinate selber ist die, ab er derjenige guckt, also bei chars die augenh�he
	sourceheight wird nur f�r touch gebraucht
	die koordinate vom target dagegen ist diejenige zu f�ssen des items/chars

	idea:
	we try if there is a line of sight between one point and any point of a large object
	problem:
	"shifted gabs" have to be handled correctly
	solution:
	the line to each point of the object ( each z-level ) has to be calculated itself
	there we stop after we know, that one line is open, or all closed
	we start from above, because walls usually are in the way in the erea of our feets,
	not our heads
	so we create a list of all x- and y- coordinates we have to pass and then draw 
	straight lines in z-direction with different gradients

	comments:
	an object denies line of sight, if it has no-shoot flag
	mountains always deny line of sight
	an object cannot be touched, if there is anything impassable or floor/roof in the way

	as coordinate of the source we use the point, where chars have there eyes,
	sourceheight is only used for touch = true
	the coordinate of the target is the one of the bottom of the object

*/

double cCoord::specialFloor( const double value ) const
{
	double dblvalue = value * 2;
	if( fabs( dblvalue - floor(dblvalue) ) < 0.0001 )
	{
		return floor( dblvalue ) / 2;
	}
	else if( fabs( dblvalue - floor(dblvalue) ) > 0.9999 )
	{
		return ceil( dblvalue ) / 2;
	}
	else
	{
		return dblvalue / 2;
	}
}


bool cCoord::lineOfSight( const cCoord &target, UI16 targetheight, bool touch )
{
	//Console::instance()->send( QString( "LOScheck: Source:%1,Target:%2,Targetheight:%3\n" ).arg( z ).arg( target.z ).arg( targetheight ) );
	if( target.map != map )
		return false;

	if( (x == target.x) && (y == target.y) && (z == target.z) )
		return true;		// if source and target are on the same position

	SI32 n = ( target.x - x ), m = ( target.y - y ), i = 0;
	SI08 sgn_x = ( x <= target.x ) ? 1 : (-1); // signum for x
	SI08 sgn_y = ( y <= target.y ) ? 1 : (-1); // signum for y
	SI08 sgn_z = ( z <= target.z ) ? 1 : (-1); // signum for z
	if( x == target.x )
		sgn_x = 0;
	if( y == target.y )
		sgn_y = 0;
	if( z == target.z )
		sgn_z = 0;

	std::vector< cCoord > collisions;

	//first we get our x-y-coordinates
	if( sgn_x == 0 && sgn_y == 0 && !sgn_z == 0 ) // should fix shooting through floor issues
		{
			collisions.push_back( cCoord( x, y, 0, map ) );
		}
	else if( sgn_x == 0 ) // if we are on the same x-level, just push every x/y coordinate in y-direction from src to trg into the array
		for( i = 0; i <= (sgn_y * m); ++i )
		{
			collisions.push_back( cCoord( x, y + (sgn_y * i), 0, map ) );
		}
	else if ( sgn_y == 0 ) // if we are on the same y-level, just push every x/y coordinate in x-direction from src to trg into the array
		for( i = 0; i <= (sgn_x * n); ++i )
		{
			collisions.push_back( cCoord( x + (sgn_x * i), y, 0, map ) );
		}
	else
	{
		SI32 oldpos = y;
		bool exaktpos = false;
		for( i = 0; (sgn_x * n >= sgn_y * m) && (i <= (sgn_x * n)); i++ )
		{
			//Console::instance()->send( QString( "x:%1\n" ).arg( i ) );
			SI32 gridx = x + (sgn_x * i);
			if( ( ( n == 0 ) && ( gridx == 0 ) ) ||
				( ( n + ( gridx * m ) == 0 ) ) )
				continue;
			else
			{
				if( exaktpos )
				{
					collisions.push_back( cCoord( gridx, oldpos-sgn_y, 0, map ) );
					//Console::instance()->send( QString( "add exaktpos coordinate %1,%2\n" ).arg( gridx ).arg( oldpos-sgn_y ) );
					exaktpos = false;
				}
					
				// linear evaluation of extended 2x2 matrix, abbreviated
				double t = (double)sgn_x * ((double)i+0.5) * (double)m / (double)n + (double)y;
				//Console::instance()->send( QString( "t:%1\n" ).arg( t ) );

				if( ((sgn_y>0) && (specialFloor(t)==oldpos+0.5)) || ((sgn_y<0) && (specialFloor(t)==oldpos-0.5)) )
				{
					exaktpos = true;
				}
				
				if( ((sgn_y>0) && (t<oldpos+0.5)) || ((sgn_y<0) && (t>oldpos-0.5)) || (oldpos==target.y) )
				{
					collisions.push_back( cCoord( gridx, oldpos, 0, map ) );
					//Console::instance()->send( QString( "add coordinate %1,%2\n" ).arg( gridx ).arg( oldpos ) );
				}
				// but if not, we have to take BOTH coordinates, which the calculated collision is between!
				else
				{ 
					collisions.push_back( cCoord( gridx, oldpos, 0, map ) );
					//Console::instance()->send( QString( "add coordinate %1,%2\n" ).arg( gridx ).arg( oldpos ) );
					oldpos += sgn_y;
					collisions.push_back( cCoord( gridx, oldpos, 0, map ) );
					//Console::instance()->send( QString( "add coordinate %1,%2\n" ).arg( gridx ).arg( oldpos ) );
				}
			}
		}
		
		oldpos = x;
		exaktpos = false;
		for( i = 0; (sgn_y * m >= sgn_x * n) && (i <= (sgn_y * m)); ++i )
		{
			//Console::instance()->send( QString( "y:%1\n" ).arg( i ) );
			SI32 gridy = y + (sgn_y * i);
			if( ( ( m == 0 ) && ( gridy == 0 ) ) ||
				( ( m + ( gridy * n ) == 0 ) ) )
				continue;
			else
			{
				if( exaktpos )
				{
					collisions.push_back( cCoord( oldpos-sgn_x, gridy, 0, map ) );
					//Console::instance()->send( QString( "add exaktpos coordinate %1,%2\n" ).arg( oldpos-sgn_x ).arg( gridy ) );
					exaktpos = false;
				}

				double t = (double)x + (double)n / (double)m * (double)sgn_y * ((double)i+0.5);
				//Console::instance()->send( QString( "t:%1\n" ).arg( t ) );

				if( ((sgn_x>0) && (specialFloor(t)==oldpos+0.5)) || ((sgn_x<0) && (specialFloor(t)==oldpos-0.5)) )
				{
					exaktpos = true;
				}

				if( ((sgn_x>0) && (t<oldpos+0.5)) || ((sgn_x<0) && (t>oldpos-0.5)) || (oldpos==target.x) )
				{
					collisions.push_back( cCoord( oldpos, gridy, 0, map ) );
					//Console::instance()->send( QString( "add coordinate %1,%2\n" ).arg( oldpos ).arg( gridy ) );

				}
				// but if not, we have to take BOTH coordinates, which the calculated collision is between!
				else
				{ 
					collisions.push_back( cCoord( oldpos, gridy, 0, map ) );
					//Console::instance()->send( QString( "add coordinate %1,%2\n" ).arg( oldpos ).arg( gridy ) );
					oldpos += sgn_x;;
					collisions.push_back( cCoord( oldpos, gridy, 0, map ) );
					//Console::instance()->send( QString( "add coordinate %1,%2\n" ).arg( oldpos ).arg( gridy ) );
				}
			}
		}
	}

	// the next will search for multis in the region 
	std::vector < P_ITEM > multis;
	NxwItemWrapper si;
	si.fillItemsAtXY(this->x, this->y);
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi = si.getItem();
		if( pi && pi->getId() >= 0x4000 )
		{
			multis.push_back( pi );
		}
	}
	//touch wird von notouch getrennt, da der notouch algorithmus wesentlich aufw�ndiger
	//ist (touch ben�tigt die erste for-schleife nicht), macht den code zwar bisserl
	//un�bersichtlicher, aber ist wesentlich effizienter

	//touch is separated from notouch, because the notouch calculation is much more to do
	//( one additional for-loop )

	if( targetheight > 0 )
	{
		--targetheight;
	}

	if( !touch )
	{
		for( i = target.z+targetheight; i >= target.z; --i )
		{
			bool blocked = false;
			//Console::instance()->send( QString( "i:%1\n" ).arg( i ) );
			double dz;
			double gradient;
			double offset;
			if( (sgn_x == 0) && (sgn_y == 0) )
			{
				dz = (double)i - (double)z;
				gradient = 1; //only to avoid problems, isnt used
				offset = 1;
			}
			else
			{
				dz = ( (double)i -(double)z ) / sqrt( ((double)target.x - (double)x)*((double)target.x - (double)x) + ((double)target.y - (double)y)*((double)target.y - (double)y) );
				gradient = (double)m / (double)n;
				offset = 0.5*( (double)y + (double)target.y - gradient*( x + target.x ) );
			}
	
			map_st map1, map2;
			SI32 j;
	
			bool posHigherThanMap;
			data::seekMap( this->x, this->y, map1 );
			if( map1.z > z )
			{
				posHigherThanMap = false;
			}
			else
			{
				posHigherThanMap = true;
			}
			
			//Console::instance()->send( QString( "after first things\n" ) );
			std::vector< cCoord >::iterator pit = collisions.begin();
			while( pit != collisions.end() )
			{
				//Console::instance()->send( QString( "coordinate:%1,%2 dz:%3\n" ).arg( (*pit).x ).arg( (*pit).y ).arg( dz ) );
				//lets see what z-coordinates we have to test
				//we do our calculations exakt, because its the only way to solve all problems
				//of floor
				//"minimum" ist am anfang der platte, "maximum" am ende
				//our line is y = gradient * x + offset
				//or x = ( y - offset ) / gradient
				//we now have to test, where the line cuts one position
				//start and endposition has to be done alone
				
				double z1 = -300;
				double z2 = -300;
				SI08 zmin, zmax;

				if( (sgn_x == 0) && (sgn_y == 0) )
				{
					if( dz > 0 )
					{
						zmin = z+1;
						zmax = i;
					}
					else
					{
						zmin = i+1;
						zmax = z;
					}
				}
				else if( sgn_x == 0 )
				{
					//gradient only in y-direction
					z1 = (double)i - dz*( fabs( (double)target.y - (double)((*pit).y) ) -0.5 ); 
					z2 = (double)i - dz*( fabs( (double)target.y - (double)((*pit).y) ) +0.5 );
					//Console::instance()->send( QString( "i:%1,ty:%2,cy:%3\n" ).arg( i ).arg( target.y ).arg( (*pit).y ) );
					//Console::instance()->send( QString( "z1:%1,z2:%2\n" ).arg( z1 ).arg( z2 ) );

					if( z1 > z2 )
					{
						zmin = (SI08)floor( (float)z2 );
						zmax = (SI08)ceilf( (float)z1 );
					}
					else
					{
						zmin = (SI08)floor( (float)z1 );
						zmax = (SI08)ceilf( (float)z2 );
					}

					/*another try, but i think its needed for all positions, not only start and end...
					//target 
					if( (*pit).y == target.y )
					{
						if( dz > 0 )
						{
							zmax = qmin( zmax, i );
							//Console::instance()->send( QString( "TargetY, zmax:%1, i:%2\n" ).arg( zmax ).arg( i ) );
						}
						else
						{
							zmin = qmax( zmin, i+1 );
							//Console::instance()->send( QString( "TargetY, zmin:%1, i:%2\n" ).arg( zmin ).arg( i ) );
						}
					}

					//source
					if( (*pit).y == y )
					{
						if( dz > 0 )
						{
							zmin = qmax( zmin, z );
							//Console::instance()->send( QString( "SourceY, zmin:%1, i:%2\n" ).arg( zmax ).arg( i ) );
						}
						else
						{
							zmax = qmin( zmax, z );
							//Console::instance()->send( QString( "SourceY, zmax:%1, i:%2\n" ).arg( zmax ).arg( i ) );
						}
					}*/
					if( dz > 0 )
					{
						zmax = qmin( zmax, i );
						zmin = qmax( zmin, z );
					}
					else
					{
						zmin = qmax( zmin, i+1 );
						zmax = qmin( zmax, z );
					}

				}
				else if( sgn_y == 0 )
				{
					//gradient only in y-direction
					z1 = (double)i - dz*( fabs( (double)target.x - (double)((*pit).x) ) -0.5 ); 
					z2 = (double)i - dz*( fabs( (double)target.x - (double)((*pit).x) ) +0.5 );
					//Console::instance()->send( QString( "i:%1,tx:%2,cx:%3\n" ).arg( i ).arg( target.x ).arg( (*pit).x ) );
					//Console::instance()->send( QString( "z1:%1,z2:%2\n" ).arg( z1 ).arg( z2 ) );

					if( z1 > z2 )
					{
						zmin = (SI08)floor( (float)z2 );
						zmax = (SI08)ceilf( (float)z1 );
					}
					else
					{
						zmin = (SI08)floor( (float)z1 );
						zmax = (SI08)ceilf( (float)z2 );
					}
					
					if( dz > 0 )
					{
						zmax = qmin( zmax, i );
						zmin = qmax( zmin, z );
					}
					else
					{
						zmin = qmax( zmin, i+1 );
						zmax = qmin( zmax, z );
					}

				}
				else
				{
					//4 lines to test
					double gradx = ( (double)target.y*(double)z - (double)y*(double)i ) / ( (double)target.y*(double)x - (double)y*(double)target.x );
					double grady = ( (double)target.x*(double)z - (double)x*(double)i ) / ( (double)y*(double)target.x - (double)target.y*(double)x );

					//Console::instance()->send( QString( "gradx:%1,grady:%2\n" ).arg( gradx ).arg( grady ) );
					//Console::instance()->send( QString( "Gradient:%1,Offset:%2\n" ).arg( gradient ).arg( offset ) );
					double temp;
					temp = specialFloor( gradient*( (double)((*pit).x) - 0.5 ) + offset );
					//Console::instance()->send( QString( "temp1:%1\n" ).arg( temp ) );
					if( ( temp >= ((double)((*pit).y)-0.5) ) && ( temp <= ((double)((*pit).y)+0.5) ) )
					{
						if( z1 > -300 )
						{
							z2 = gradx * ( (double)((*pit).x)-0.5 ) + grady * temp;
						}
						else
						{	
							z1 = gradx * ( (double)((*pit).x)-0.5 ) + grady * temp;
						}
						//Console::instance()->send( QString( "1:i:%1,tx:%2,ty:%3,cy:%4,cy:%5\n" ).arg( i ).arg( target.x ).arg( target.y ).arg( (*pit).x ).arg( (*pit).y ) );
						//Console::instance()->send( QString( "z1:%1,z2:%2\n" ).arg( z1 ).arg( z2 ) );
					}
					temp = specialFloor( gradient*( (double)((*pit).x) + 0.5 ) + offset );
					//Console::instance()->send( QString( "temp2:%1\n" ).arg( temp ) );
					if( ( temp >= ((double)((*pit).y)-0.5) ) && ( temp <= ((double)((*pit).y)+0.5) ) )
					{
						if( z1 > -300 )
						{
							z2 = gradx * ( (double)((*pit).x)+0.5 ) + grady * temp;
						}
						else
						{	
							z1 = gradx * ( (double)((*pit).x)+0.5 ) + grady * temp;
						}
						//Console::instance()->send( QString( "2:i:%1,tx:%2,ty:%3,cy:%4,cy:%5\n" ).arg( i ).arg( target.x ).arg( target.y ).arg( (*pit).x ).arg( (*pit).y ) );
						//Console::instance()->send( QString( "z1:%1,z2:%2\n" ).arg( z1 ).arg( z2 ) );
					}
					temp = specialFloor( (double)((*pit).y) - 0.5 - offset ) / gradient;
					//Console::instance()->send( QString( "temp3:%1\n" ).arg( temp ) );
					if( ( temp > ((double)((*pit).x)-0.5) ) && ( temp < ((double)((*pit).x)+0.5) ) )
					{
						if( z1 > -300 )
						{
							z2 = gradx * temp + grady * ((double)((*pit).y)-0.5);
						}
						else
						{
							z1 = gradx * temp + grady * ((double)((*pit).y)-0.5);
						}
						//Console::instance()->send( QString( "3:i:%1,tx:%2,ty:%3,cy:%4,cy:%5\n" ).arg( i ).arg( target.x ).arg( target.y ).arg( (*pit).x ).arg( (*pit).y ) );
						//Console::instance()->send( QString( "z1:%1,z2:%2\n" ).arg( z1 ).arg( z2 ) );
					}
					temp = specialFloor( (double)((*pit).y) + 0.5 - offset ) / gradient;
					//Console::instance()->send( QString( "temp4:%1\n" ).arg( temp ) );
					if( ( temp > ((double)((*pit).x)-0.5) ) && ( temp < ((double)((*pit).x)+0.5) ) )
					{
						if( z1 > -300 )
						{
							z2 = gradx * temp + grady * ((double)((*pit).y)+0.5);
						}
						else
						{
							z1 = gradx * temp + grady * ((double)((*pit).y)+0.5);
						}
						//Console::instance()->send( QString( "4:i:%1,tx:%2,ty:%3,cy:%4,cy:%5\n" ).arg( i ).arg( target.x ).arg( target.y ).arg( (*pit).x ).arg( (*pit).y ) );
						//Console::instance()->send( QString( "z1:%1,z2:%2\n" ).arg( z1 ).arg( z2 ) );
					}
					
					//Console::instance()->send( QString( "z1:%1,z2:%2\n" ).arg( z1 ).arg( z2 ) );
					if( z1 > z2 )
					{
						zmin = (SI08)floor( (float)z2 );
						zmax = (SI08)ceilf( (float)z1 );
					}
					else
					{
						zmin = (SI08)floor( (float)z1 );
						zmax = (SI08)ceilf( (float)z2 );
					}

					if( z2 == -300 )
					{
						zmin = (SI08)floor( z1 );
					}

					if( dz > 0 )
					{
						zmax = qmin( zmax, i );
						zmin = qmax( zmin, z );
					}
					else
					{
						zmin = qmax( zmin, i+1 );
						zmax = qmin( zmax, z );
					}
					//Console::instance()->send( QString( "zmin:%1,zmax:%2\n" ).arg( zmin ).arg( zmax ) );
				}

				/*SI08 zmin = (SI08)floor( i - dz*sqrt( ((double)target.x - (double)(*pit).x)*((double)target.x - (double)(*pit).x) + ((double)target.y - (double)(*pit).y)*((double)target.y - (double)(*pit).y) ) );
				SI08 zmax;
				//Console::instance()->send( QString( "zmin:%1,dz:%2\n" ).arg( zmin ).arg( dz ) );
				if( dz > 0 )
				{
					zmin = qmax( (SI08)floor( zmin - dz/2 ), z+1 );
					zmax = qmin( zmin + dz + 1, target.z+targetheight+1 );	//to prevent floor-mistakes
				}
				else
				{
					zmin = qmin( (SI08)floor( zmin + dz/2 ), target.z+1 );
					zmax = qmax( zmin - dz + 1, z );	//to prevent floor-mistakes
				}*/
	
				// Texture mapping  
				data::seekMap( (*pit).x, (*pit).y, map1 );
				data::seekMap( (*pit).x + sgn_x, (*pit).y + sgn_y,  map2 );
			
				//Console::instance()->send( QString( "maphoehe:%1\n" ).arg( map1.z ) );
				staticVector msi;
				data::collectStatics((*pit).x, (*pit).y, msi);
				if( (map1.id != 2) && (map2.id != 2) ) 
				{
					if( ( map1.z >= zmin ) && ( map1.z <= zmax ) )
					{
						//its just in our way
						//Console::instance()->send( QString( "Map gescheitert\n" ) );
						blocked = true;
						break;
					}
	
					//now we have to test, if this tile is under our line and the next above or
					//vice verse
					//in this case, both dont cut our line, but the mapconnection between them
					//should do
					if( ( ( map1.z < map2.z ) && ( map1.z < zmin ) && ( map2.z > zmax+dz ) ) ||						// 1) lineofsight collides with a map "wall"
						( ( map1.z > map2.z ) && ( map1.z > zmin ) && ( map2.z < zmax-dz ) ) ||
						( ( ( map1.id >= 431  && map1.id <= 432  ) ||	// 3) lineofsight cuts a mountain
						( map1.id >= 467  && map1.id <= 475  ) ||
						( map1.id >= 543  && map1.id <= 560  ) ||
						( map1.id >= 1754 && map1.id <= 1757 ) ||
						( map1.id >= 1787 && map1.id <= 1789 ) ||
						( map1.id >= 1821 && map1.id <= 1824 ) ||
						( map1.id >= 1851 && map1.id <= 1854 ) ||
						( map1.id >= 1881 && map1.id <= 1884 ) ) &&
						( posHigherThanMap ) && ( msi.size()==0 ) ) ) // mountains cut only if we are not beneath them
					{
						//Console::instance()->send( QString( "map1:%1,map2:%2\n" ).arg( map1.z ).arg( map2.z ) );
						blocked = true;
						break;
					}
				}	 
				
				//Console::instance()->send( QString( "after map\n" ) );
	
				// Statics
				tile_st tile;
				UI32 staticIndex=0;
				while( staticIndex < msi.size() )
				{
					data::seekTile(msi[staticIndex].id, tile );
					//if it is in our way
					//Console::instance()->send( QString( "tilepos:%1,zmax:%2" ).arg( msi->zoff ).arg( zmax ) );
					//Console::instance()->send( QString( "zmin:%3\n" ).arg( zmin ) );
					if( ( zmax >= msi[staticIndex].z ) && ( zmin <= ( msi[staticIndex].z+ tile.height ) ) )
					{
						//Console::instance()->send( QString( "statictile, id: %1\n" ).arg( msi->itemid ) );
						if( tile.flags&TILEFLAG_NOSHOOT )
						{
							blocked = true;
							break;
						}
					}
		
					++staticIndex;
				}
				if( blocked )
				{
					break;
				}
				
				//Console::instance()->send( QString( "after statics\n" ) );
				// Items
				NxwItemWrapper si;
				si.fillItemsNearXYZ(this->x, this->y);
				
				for( si.rewind(); !si.isEmpty(); si++ )
				{
					P_ITEM pi = si.getItem();
					if( ISVALIDPI(pi) && pi->getId() < 0x4000 )
					{
						data::seekTile(pi->getId(), tile );
						if(	( zmax >= pi->getPosition().z ) && ( zmin <= ( pi->getPosition().z + tile.height ) ) && ( pi->visible == 0 ) )
						{
							//Console::instance()->send( QString( "item, id: %1" ).arg( pi->id() ) );
							if( tile.flags&TILEFLAG_NOSHOOT )
							{
								blocked = true;
								break;
							}
						}	
					}
				}
				if( blocked )
				{
					break;
				}
	
				//Console::instance()->send( QString( "after items\n" ) );
				// Multis
				long multiIndex=0;
				si.clear();
				si.fillItemsAtXY(this->x, this->y);
				P_ITEM pi_m = si.getItem();
				while( ( multiIndex < multis.size() && pi_m == multis[multiIndex++] ) )
				{
					multiVector multiVec;
					data::seekMulti( pi_m->getId() - 0x4000, multiVec );
					if ( multiVec.size() == 0 )
						continue;
					for( j = 0; j < multiVec.size(); ++j )
					{
						multi_st multi=multiVec[j];
						if( ( multi.flags != 0 ) && ( pi_m->getPosition().x + multi.x == (*pit).x ) &&
							( pi_m->getPosition().y + multi.y == (*pit).y ) )			
						{
							data::seekTile(multi.block, tile );
							if( ( zmax >= pi_m->getPosition().z + multi.height ) &&
								( zmin <= pi_m->getPosition().z + multi.height + tile.height ) )
							{
								if( tile.flags&TILEFLAG_NOSHOOT )
								{
									blocked = true;
									break;
								}
							}	
						}
					}
				}
				//Console::instance()->send( QString( "after multis\n" ) );
				++pit;
			}
	
			if( !blocked )
				return true;
		}
		//there was no line to see through
		return false;
	}
	//now touch=true
	else
	{
		//Console::instance()->send( "touch\n" );
		double dz_up, dz_down;
		double gradient;
		double offset;
		if( (sgn_x == 0) && (sgn_y == 0) )
		{
			dz_up = ( (double)targetheight + (double)target.z ) - (double)z;
			dz_down = (double)target.z - ( (double)z - (double)15 );
			gradient = 1; //only to prevent problems, isnt used
			offset = 1;
		}
		else
		{
			//Console::instance()->send( QString( "xdiff:%1,ydiff:%2\n" ).arg( (double)target.x - (double)x ).arg( (double)target.y - (double)y ) );
			dz_up = ( ( (double)targetheight + (double)target.z ) - (double)z ) / sqrt( ((double)target.x - (double)x)*((double)target.x - (double)x) + ((double)target.y - (double)y)*((double)target.y - (double)y) );
			dz_down = ( (double)target.z - ( (double)z - (double)15 ) ) / sqrt( ((double)target.x - (double)x)*((double)target.x - (double)x) + ((double)target.y - (double)y)*((double)target.y - (double)y) );
			gradient = (double)m / (double)n;
			offset = 0.5*( (double)y + (double)target.y - gradient*( x + target.x ) );
		}
	
		map_st map1, map2;
		SI32 j;
	
		bool posHigherThanMap;
		data::seekMap( this->x, this->y, map1 );
		if( map1.z > z )
		{
			posHigherThanMap = false;
		}
		else
		{
			posHigherThanMap = true;
		}
		
		//Console::instance()->send( QString( "after first things\n" ) );
		std::vector< cCoord >::iterator pit = collisions.begin();
		while( pit != collisions.end() )
		{
			//Console::instance()->send( QString( "coordinate:%1,%2\n" ).arg( (*pit).x ).arg( (*pit).y ) );				
			//lets see what z-coordinates we have to test
			//anmerkung: touch kommt nur f�r chars vor, gr�sse von chars ist 15
			//SI08 zmin = (SI08)floor(  (double)z - (double)sourceheight + dz_down*sqrt( ((double)target.x - (double)(*pit).x)*((double)target.x - (double)(*pit).x) + ((double)target.y - (double)(*pit).y)*((double)target.y - (double)(*pit).y) ) );
			//SI08 zmax = (SI08)floor(  (double)z + dz_up*sqrt( ((double)target.x - (double)(*pit).x)*((double)target.x - (double)(*pit).x) + ((double)target.y - (double)(*pit).y)*((double)target.y - (double)(*pit).y) ) );
			SI08 zmin, zmax;
			double z1_up = -300;
			double z2_up = -300;
			double z1_down = -300;
			double z2_down = -300;
			bool targetpos = false;
			//Console::instance()->send( QString( "dz_down:%3,dz_up:%4\n" ).arg( dz_down ).arg( dz_up ) );

			if( (sgn_x == 0) && (sgn_y == 0) )
			{
				if( dz_up > 0 )
				{
					zmin = z+1;
					zmax = target.z;
				}
				else
				{
					zmin = target.z+1;
					zmax = z;
				}
				targetpos = true;
				if( (dz_up >= 0) && (dz_down >= 0) )
				{
					if( zmin < target.z )
					{
						zmax = target.z -1;
					}
					else
					{
						//we ignore this coordinate
						++pit;
						continue;
					}
				}
				else if( (dz_up <= 0) && (dz_down <= 0) )
				{
					if( zmax > target.z + targetheight+1 )
					{
						zmin = target.z + targetheight + 2;
					}
					else
					{
						++pit;
						continue;
					}
				}
				else 
				{
					//we may have to split the test into two if we would do it exactly
					//but i think we can throw away the test from down in this case
					if( zmax > target.z + targetheight+1 )
					{
						zmin = target.z + targetheight + 2;
					}
					else if( zmin < target.z )
					{
						zmax = target.z -1;
					}
					else
					{
						++pit;
						continue;
					}
				}
			}
			else if( sgn_x == 0 )
			{
				z1_up = target.z + targetheight - dz_up*( fabs( (double)target.y - (double)((*pit).y) ) -0.5 ); 
				z2_up = target.z + targetheight - dz_up*( fabs( (double)target.y - (double)((*pit).y) ) +0.5 );
			
				z1_down = target.z - dz_down*( fabs( (double)target.y - (double)((*pit).y) ) -0.5 );
				z2_down = target.z - dz_down*( fabs( (double)target.y - (double)((*pit).y) ) +0.5 );

				//Console::instance()->send( QString( "ty:%2,cy:%3\n" ).arg( target.y ).arg( (*pit).y ) );
				//Console::instance()->send( QString( "z1_up:%1,z2_up:%2,z1_down:%3,z2_down:%4\n" ).arg( z1_up ).arg( z2_up ).arg( z1_down ).arg( z2_down ) );

				zmax = (SI08)qmax( ceil( z1_up ), ceil( z2_up ) );
				zmin = (SI08)qmin( floor( z1_down ), floor( z2_down ) );

				//Console::instance()->send( QString( "y:zmin:%1,zmax:%2\n" ).arg( zmin ).arg( zmax ) );

				if( dz_up > 0 )
				{
					zmax = qmin( zmax, target.z+targetheight );
				}
				else
				{
					zmax = qmin( zmax, z );
				}

				//Console::instance()->send( QString( "y2:zmin:%1,zmax:%2\n" ).arg( zmin ).arg( zmax ) );

				if( dz_down > 0 )
				{
					zmin = qmax( zmin, z-14 );
				}
				else
				{
					zmin = qmax( zmin, target.z+1 );
				}

				//Console::instance()->send( QString( "y3:zmin:%1,zmax:%2\n" ).arg( zmin ).arg( zmax ) );

				if( (*pit).y == target.y )
				{
					targetpos = true;
					if( (dz_up >= 0) && (dz_down >= 0) )
					{
						if( zmin < target.z )
						{
							zmax = target.z -1;
						}
						else
						{
							//we ignore this coordinate
							++pit;
							continue;
						}
					}
					else if( (dz_up <= 0) && (dz_down <= 0) )
					{
						if( zmax > target.z + targetheight+1 )
						{
							zmin = target.z + targetheight + 2;
						}
						else
						{
							++pit;
							continue;
						}
					}
					else 
					{
						//we may have to split the test into two if we would do it exactly
						//but i think we can throw away the test from down in this case
						if( zmax > target.z + targetheight+1 )
						{
							zmin = target.z + targetheight + 2;
						}
						else if( zmin < target.z )
						{
							zmax = target.z -1;
						}
						else
						{
							++pit;
							continue;
						}
					}
					//Console::instance()->send( QString( "y4:zmin:%1,zmax:%2\n" ).arg( zmin ).arg( zmax ) );
				}
			}
			else if( sgn_y == 0 )
			{
				z1_up = target.z + targetheight - dz_up*( fabs( (double)target.x - (double)((*pit).x) ) -0.5 ); 
				z2_up = target.z + targetheight - dz_up*( fabs( (double)target.x - (double)((*pit).x) ) +0.5 );
			
				z1_down = target.z - dz_down*( fabs( (double)target.x - (double)((*pit).x) ) -0.5 );
				z2_down = target.z - dz_down*( fabs( (double)target.x - (double)((*pit).x) ) +0.5 );

				//Console::instance()->send( QString( "tx:%2,cx:%3\n" ).arg( target.x ).arg( (*pit).x ) );
				//Console::instance()->send( QString( "z1_up:%1,z2_up:%2,z1_down:%3,z2_down:%4\n" ).arg( z1_up ).arg( z2_up ).arg( z1_down ).arg( z2_down ) );

				zmax = (SI08)qmax( ceil( z1_up ), ceil( z2_up ) );
				zmin = (SI08)qmin( floor( z1_down ), floor( z2_down ) );

				if( dz_up > 0 )
				{
					zmax = (SI08)qmin( zmax, target.z+targetheight );
				}
				else
				{
					zmax = (SI08)qmin( zmax, z );
				}

				if( dz_down > 0 )
				{
					zmin = (SI08)qmax( zmin, z-14 );
				}
				else
				{
					zmin = (SI08)qmax( zmin, target.z+1 );
				}

				if( (*pit).x == target.x )
				{
					targetpos = true;
					if( (dz_up >= 0) && (dz_down >= 0) )
					{
						if( zmin < target.z )
						{
							zmax = target.z -1;
						}
						else
						{
							//we ignore this coordinate
							++pit;
							continue;
						}
					}
					else if( (dz_up <= 0) && (dz_down <= 0) )
					{
						if( zmax > target.z + targetheight+1 )
						{
							zmin = target.z + targetheight + 2;
						}
						else
						{
							++pit;
							continue;
						}
					}
					else 
					{
						//we may have to split the test into two if we would do it exactly
						//but i think we can throw away the test from down in this case
						if( zmax > target.z + targetheight+1 )
						{
							zmin = target.z + targetheight + 2;
						}
						else if( zmin < target.z )
						{
							zmax = target.z -1;
						}
						else
						{
							++pit;
							continue;
						}
					}
				}
			}
			else
			{
				double gradx_up = ( (double)target.y*(double)z - (double)y*(double)(target.z + targetheight) ) / ( (double)target.y*(double)x - (double)y*(double)target.x );
				double grady_up = ( (double)target.x*(double)z - (double)x*(double)(target.z + targetheight) ) / ( (double)y*(double)target.x - (double)target.y*(double)x );
				double gradx_down = ( (double)target.y*(double)(z-15) - (double)y*(double)(target.z) ) / ( (double)target.y*(double)x - (double)y*(double)target.x );
				double grady_down = ( (double)target.x*(double)(z-15) - (double)x*(double)(target.z) ) / ( (double)y*(double)target.x - (double)target.y*(double)x );

				//Console::instance()->send( QString( "gradx_up:%1,grady_up:%2,gradx_down:%3,grady_down:%4\n" ).arg( gradx_up ).arg( grady_up ).arg( gradx_down ).arg( grady_down ) );
				//Console::instance()->send( QString( "Gradient:%1,Offset:%2\n" ).arg( gradient ).arg( offset ) );

				double temp = specialFloor( gradient*( (double)((*pit).x) - 0.5 ) + offset );
				//Console::instance()->send( QString( "temp1:%1\n" ).arg( temp ) );
				if( ( temp >= ((double)((*pit).y)-0.5) ) && ( temp <= ((double)((*pit).y)+0.5) ) )
				{
					if( z1_up > -300 )
					{
						z2_up = gradx_up * ( (double)((*pit).x)-0.5 ) + grady_up * temp;
						z2_down = gradx_down * ( (double)((*pit).x)-0.5 ) + grady_down * temp;
					}
					else
					{
						z1_up = gradx_up * ( (double)((*pit).x)-0.5 ) + grady_up * temp;
						z1_down = gradx_down * ( (double)((*pit).x)-0.5 ) + grady_down * temp;
					}
					//Console::instance()->send( QString( "tx:%1,ty:%2,cy:%3,cy:%4\n" ).arg( target.x ).arg( target.y ).arg( (*pit).x ).arg( (*pit).y ) );
					//Console::instance()->send( QString( "z1_up:%1,z1_down:%2,z2_up:%3,z2_down:%4\n" ).arg( z1_up ).arg( z1_down ).arg( z2_up ).arg( z2_down ) );
				}
				temp = specialFloor( gradient*( (double)((*pit).x) + 0.5 ) + offset );
				//Console::instance()->send( QString( "temp2:%1\n" ).arg( temp ) );
				if( ( temp >= ((double)((*pit).y)-0.5) ) && ( temp <= ((double)((*pit).y)+0.5) ) )
				{
					if( z1_up > -300 )
					{
						z2_up = gradx_up * ( (double)((*pit).x)+0.5 ) + grady_up * temp;
						z2_down = gradx_down * ( (double)((*pit).x)+0.5 ) + grady_down * temp;
					}
					else
					{	
						z1_up = gradx_up * ( (double)((*pit).x)+0.5 ) + grady_up * temp;
						z1_down = gradx_down * ( (double)((*pit).x)+0.5 ) + grady_down * temp;
					}
					//Console::instance()->send( QString( "tx:%1,ty:%2,cy:%3,cy:%4\n" ).arg( target.x ).arg( target.y ).arg( (*pit).x ).arg( (*pit).y ) );
					//Console::instance()->send( QString( "z1_up:%1,z1_down:%2,z2_up:%3,z2_down:%4\n" ).arg( z1_up ).arg( z1_down ).arg( z2_up ).arg( z2_down ) );
				}
				temp = specialFloor( (double)((*pit).y) - 0.5 - offset ) / gradient;
				//Console::instance()->send( QString( "temp3:%1\n" ).arg( temp ) );
				if( ( temp > ((double)((*pit).x)-0.5) ) && ( temp < ((double)((*pit).x)+0.5) ) )
				{
					if( z1_up > -300 )
					{
						z2_up = gradx_up * temp + grady_up * ((double)((*pit).y)-0.5);
						z2_down = gradx_down * temp + grady_down * ((double)((*pit).y)-0.5);
					}
					else
					{
						z1_up = gradx_up * temp + grady_up * ((double)((*pit).y)-0.5);
						z1_down = gradx_down * temp + grady_down * ((double)((*pit).y)-0.5);
					}
					//Console::instance()->send( QString( "tx:%1,ty:%2,cy:%3,cy:%4\n" ).arg( target.x ).arg( target.y ).arg( (*pit).x ).arg( (*pit).y ) );
					//Console::instance()->send( QString( "z1_up:%1,z1_down:%2,z2_up:%3,z2_down:%4\n" ).arg( z1_up ).arg( z1_down ).arg( z2_up ).arg( z2_down ) );
				}
				temp = specialFloor( (double)((*pit).y) + 0.5 - offset ) / gradient;
				//Console::instance()->send( QString( "temp4:%1\n" ).arg( temp ) );
				if( ( temp > ((double)((*pit).x)-0.5) ) && ( temp < ((double)((*pit).x)+0.5) ) )
				{
					if( z1_up > -300 )
					{
						z2_up = gradx_up * temp + grady_up * ((double)((*pit).y)+0.5);
						z2_down = gradx_down * temp + grady_down * ((double)((*pit).y)+0.5);
					}
					else
					{
						z1_up = gradx_up * temp + grady_up * ((double)((*pit).y)+0.5);
						z1_down = gradx_down * temp + grady_down * ((double)((*pit).y)+0.5);
					}
					//Console::instance()->send( QString( "tx:%1,ty:%2,cy:%3,cy:%4\n" ).arg( target.x ).arg( target.y ).arg( (*pit).x ).arg( (*pit).y ) );
					//Console::instance()->send( QString( "z1_up:%1,z1_down:%2,z2_up:%3,z2_down:%4\n" ).arg( z1_up ).arg( z1_down ).arg( z2_up ).arg( z2_down ) );
				}

				//Console::instance()->send( QString( "ergebnis: z1_up:%1,z1_down:%2,z2_up:%3,z2_down:%4\n" ).arg( z1_up ).arg( z1_down ).arg( z2_up ).arg( z2_down ) );

				if( z2_up == -300 )
				{
					zmin = (SI08)floor( z1_down );
					zmax = (SI08)ceil( z1_up );
				}
				else
				{
					zmin = (SI08)qmin( floor( z1_down ), floor( z2_down ) );
					zmax = (SI08)qmax( ceil( z1_up ), ceil(  z2_up ) );
				}

				//Console::instance()->send( QString( "zmin:%1,zmax:%2\n" ).arg( zmin ).arg( zmax ) );

				if( dz_up > 0 )
				{
					zmax =(SI08) qmin( zmax, target.z+targetheight );
				}
				else
				{
					zmax = (SI08)qmin( zmax, z );
				}

				if( dz_down > 0 )
				{
					zmin = qmax( zmin, z-14 );
				}
				else
				{
					zmin = qmax( zmin, target.z+1 );
				}

				//Console::instance()->send( QString( "zmin:%1,zmax:%2\n" ).arg( zmin ).arg( zmax ) );

				if( ((*pit).x == target.x) && ((*pit).y == target.y) )
				{
					targetpos = true;
					if( (dz_up >= 0) && (dz_down >= 0) )
					{
						if( zmin < target.z )
						{
							zmax = target.z -1;
						}
						else
						{
							//we ignore this coordinate
							++pit;
							continue;
						}
					}
					else if( (dz_up <= 0) && (dz_down <= 0) )
					{
						if( zmax > target.z + targetheight+1)
						{
							zmin = target.z + targetheight + 2;
						}
						else
						{
							++pit;
							continue;
						}
					}
					else 
					{
						//we may have to split the test into two if we would do it exactly
						//but i think we can throw away the test from down in this case
						if( zmax > target.z + targetheight+1 )
						{
							zmin = target.z + targetheight + 2;
						}
						else if( zmin < target.z )
						{
							zmax = target.z -1;
						}
						else
						{
							++pit;
							continue;
						}
					}
				}				
			}
			//Console::instance()->send( QString( "zmin:%1,zmax:%2\n" ).arg( zmin ).arg( zmax ) );

			// Texture mapping  
			data::seekMap( (*pit).x, (*pit).y, map1 );
			data::seekMap( (*pit).x + sgn_x, (*pit).y + sgn_y,  map2 );
			
			//Console::instance()->send( QString( "try2" ) );
			staticVector msi;
			data::collectStatics((*pit).x, (*pit).y, msi);
			NxwItemWrapper si;
			si.fillItemsAtXY( (*pit).x, (*pit).y );
			if( (map1.id != 2) && (map2.id != 2) ) 
			{
				if( ( map1.z > zmin ) && ( map1.z < zmax ) )
				{
					//its just in our way
					//Console::instance()->send( QString( "map cut 1\n" ) );
					return false;
				}
				
				//now we have to test, if this tile is under our line and the next above or
				//vice verse
				//in this case, both dont cut our line, but the mapconnection between them
				//should do
				land_st tile ;
				data::seekLand( map1.id , tile);
				if( ( ( map1.z < map2.z ) && ( map1.z < zmin ) && ( map2.z > zmax+dz_down ) && !targetpos ) ||						// 1) lineofsight collides with a map "wall"
					( ( map1.z > map2.z ) && ( map1.z > zmin ) && ( map2.z < zmin+dz_down ) && !targetpos ) ||
					( tile.flags&TILEFLAG_IMPASSABLE && posHigherThanMap && msi.size()==0 && si.isEmpty() ) )
				{
					//Console::instance()->send( QString( "map1:%1,map2:%2,map1id:%3\n" ).arg( map1.z ).arg( map2.z ).arg( map1.id ) );
					if( ( map1.z < map2.z ) && ( map1.z < zmin ) && ( map2.z > zmax+dz_down ) )
					{
						//Console::instance()->send( QString( "mapcut1\n" ) ); 
					}
					else if( ( map1.z > map2.z ) && ( map1.z > zmin ) && ( map2.z < zmin+dz_down ) )
					{
						//Console::instance()->send( QString( "mapcut2\n" ) );
					}
					else if( tile.flags&TILEFLAG_IMPASSABLE && posHigherThanMap && msi.size()==0 && si.isEmpty() )
					{
						//Console::instance()->send( QString( "mapcut3\n" ) );
					}
					else
					{
						//Console::instance()->send( QString( "mapcut: this isnt possible\n" ) );
					}
					return false;
				}
			}	 
			
			//Console::instance()->send( QString( "after map\n" ) );
		
			// Statics
			tile_st tile;
			long staticIndex=0;
			while( staticIndex < msi.size() )
			{
				data::seekTile(msi[staticIndex].id, tile );
				//Console::instance()->send( QString( "statictilepos:%1,zmax:%2,zmin:%3\n" ).arg( msi->zoff ).arg( zmax ).arg( zmin ) );
				//if it is in our way
				if(	( zmax >= msi[staticIndex].z ) && ( zmin <= ( msi[staticIndex].z + tile.height ) ) )
				{
					if( tile.flags&TILEFLAG_IMPASSABLE  )
					{
						if( !( (tile.flags & TILEFLAG_FOLIAGE) && (tile.height > 1) ) )
						{
							return false;
						}
					}
				}
			
				++staticIndex;
			}
			
			//Console::instance()->send( QString( "after statics\n" ) );
			// Items
			//Console::instance()->send( QString( "Items at: %1,%2,%3,%4\n" ).arg( (*pit).x ).arg( (*pit).y ).arg( (*pit).z ).arg( (*pit).map ) );
			for( si.rewind(); !si.isEmpty(); si++ ) 
			{
				P_ITEM pi=si.getItem();
				if( pi && pi->getId() < 0x4000 )
				{
					data::seekTile(pi->getId(), tile );
					//Console::instance()->send( QString( "itemtilepos:%1,zmax:%2,zmin:%3\n" ).arg( pi->pos().z ).arg( zmax ).arg( zmin ) );
					if(	( zmax >= pi->getPosition().z ) && ( zmin <= ( pi->getPosition().z + tile.height ) ) && ( pi->visible == 0 ) )
					{
						if( tile.flags&TILEFLAG_IMPASSABLE )
						{
							if( !( (tile.flags & TILEFLAG_FOLIAGE) && (tile.height > 1) ) )
							{
								//Console::instance()->send( QString( "Item:%1,Z:%2,Height:%3\n" ).arg( pi->id() ).arg( pi->pos().z ).arg( tile.height ) );
								return false;
							}
						}
					}	
				}
			}
			
			//Console::instance()->send( QString( "after items\n" ) );
			// Multis
			long multiIndex=0;
			si.clear();
			si.fillItemsAtXY(this->x, this->y);
			P_ITEM pi_m = si.getItem();
			while( ( pi_m == multis[multiIndex++] ) )
			{
				multiVector multiVec;
				data::seekMulti( pi_m->getId() - 0x4000, multiVec );
				if ( multiVec.size() == 0 )
				{
					continue;
				}
				for( j = 0; j < multiVec.size(); ++j )
				{
					if( ( multiVec[j].flags ) && ( pi_m->getPosition().x + multiVec[j].x == (*pit).x ) &&
						( pi_m->getPosition().y + multiVec[j].y == (*pit).y ) )			
					{
						data::seekTile(multiVec[j].block, tile );
						if( ( zmax >= pi_m->getPosition().z + multiVec[j].height ) &&
							( zmin <= pi_m->getPosition().z + multiVec[j].height + tile.height ) )
						{
							if( tile.flags&TILEFLAG_IMPASSABLE ) // || tile.isRoofOrFloorTile() )
							{
								if( !( (tile.flags & TILEFLAG_FOLIAGE) && (tile.height > 1) ) )
								{
									return false;
								}							
							}
						}	
					}
				}
			}
			//Console::instance()->send( QString( "after multis\n" ) );
			++pit;
		}
		return true;
	}
}
