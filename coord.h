// This code is used with friendly permission from Varan Dar from 
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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

#if !defined(__COORD_H__)
#define __COORD_H__


//Forward class declaration
class cCoord;

// typedefs


//Class definitions

class cCoord
{
public:
	cCoord(void);
	cCoord(const cCoord& clCoord);
	cCoord(UI16, UI16, SI08 = 0, UI08 uiMap = 0, UI08 uiPlane = 0);
	// Operators
	cCoord& operator= (const cCoord& clCoord);
	bool operator==(const cCoord&) const;
	bool operator!=(const cCoord&) const;
	cCoord operator+(const cCoord& src) const;
	cCoord operator-(const cCoord& src) const;
	unsigned int distance( const cCoord &src) const;
	unsigned char direction( const cCoord &dest ) const;
	bool lineOfSight( const cCoord &target, bool touch = false );
	bool lineOfSight( const cCoord &target, UI16 targethight, bool touch = false );
	static UI32 distance ( const cCoord &a, const cCoord &b );

	/*!
		Displays an effect staying at this position.
	*/
	void effect( UI16 id, UI08 speed = 10, UI08 duration = 5, UI16 hue = 0, UI16 renderMode = 0 );
	
	static cCoord null;

private:
	double specialFloor( const double value ) const;

public:
	UI16 	x;
	UI16 	y;
	SI08 	z;
	UI08    map;
};

#endif // __COORD_H__
