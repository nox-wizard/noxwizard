  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "containers.h"
#include "scp_parser.h"
#include "items.h"
#include "basics.h"
#include "inlines.h"
#include "scripts.h"

CONTINFOGUMPMAP contInfoGump;
CONTINFOMAP contInfo;

void loadcontainers()
{

	cScpIterator*	iter = NULL;
	std::string	script1,
			script2;
	SI32 		gump = INVALID;
	BasicPosition	uprleft = {INVALID,INVALID};
	BasicPosition	dwnrght = {INVALID,INVALID};
	UI32VECTOR	*vet = new UI32VECTOR;

	int cont=0;

	int loopexit=0;
	do
	{
		safedelete(iter);
		iter = Scripts::Containers->getNewIterator("SECTION CONTAINER %i", cont++);
		if( iter==NULL ) continue;

		gump = INVALID;
		uprleft.x = INVALID;
		uprleft.y = INVALID;
		dwnrght.x = INVALID;
		dwnrght.y = INVALID;
		vet->clear();

		do
		{

			iter->parseLine(script1, script2);
			if ( script1[0]!='}' && script1[0]!='{' )
			{
				if	( "ID" == script1 )
					vet->push_back( str2num( script2 ) );
				else if ( "GUMP" == script1 )
					gump = str2num( script2 );
				else if ( "X1" == script1 )
					uprleft.x= str2num( script2 );
				else if ( "Y1" == script1 )
					uprleft.y= str2num( script2 );
				else if ( "X2" == script1 )
					dwnrght.x= str2num( script2 );
				else if ( "Y2" == script1 )
					dwnrght.y= str2num( script2 );
				else
					WarnOut("[ERROR] wrong line ( %s ) parsed on containers.xss", script1.c_str() );
			}
		}
		while ( script1[0] !='}' && ++loopexit < MAXLOOPS );

		if( (gump!=INVALID) && (uprleft.x!=INVALID) && (dwnrght.x!=INVALID) && (uprleft.y!=INVALID) && (dwnrght.y!=INVALID) )
		{
			cont_gump_st dummy;

			dummy.downright = dwnrght;
			dummy.upperleft = uprleft;
			dummy.gump	= gump;

			contInfoGump[gump] = dummy;

			CONTINFOGUMPMAP::iterator iter( contInfoGump.find(gump) );
			if( iter != contInfoGump.end() )
			{
				UI32VECTOR::iterator ids( vet->begin() ), end( vet->end() );
				for(; ids != end; ++ids )
					contInfo[(*ids)] = iter;
			}
			else
				ConOut("[ERROR] on parse of containers.xss" );
		}
		else
			ConOut("[ERROR] on parse of containers.xss" );
	}
	while ( script1 != "EOF" && ++loopexit < MAXLOOPS );

	safedelete(iter);

//	ConOut("\n");
//	for(CONTINFOMAP::iterator debug=contInfo.begin(); debug!=contInfo.end(); debug++ )
//		ConOut( "id %i ha gump %i \n ", debug->first, (debug->second)->second.gump);



}



SI16 cItem::getContGump()
{
/*	switch(this->id1)
	{
		case 0x0E:
			switch (this->id2)
			{
				case 0x75:			// Backpack
				case 0x79: return 0x3C;	// Box/Pouch
							break;

				case 0x76: return 0x3D;	// Leather Bag
							break;

				case 0x77:			// Barrel
				case 0x7A:			// Square Basket
				case 0x7F: return 0x3E;	// Keg
							break;

				case 0x7C: return 0x4A;	// Silver Chest
							break;

				case 0x7D: return 0x43;	// Wooden Box
							break;


				case 0x3D:			// Large Wooden Crate
				case 0x3C:			// Large Wooden Crate
				case 0x3F:			// Small Wooden Crate
				case 0x3E:			// Small Wooden Crate
				case 0x7E: return 0x44;	// Wooden Crate
						break;

				case 0x80: return 0x4B;	// Brass Box
						break;

				case 0x40:					// Metal & Gold Chest
				case 0x41: return 0x42;	// Metal & Gold Chest
						break;


				case 0x43:					// Wooden & Gold chest
				case 0x42: return 0x49; // Wooden & Gold Chest
						break;

			}
			break;

		case 0x09:
			switch (this->id2)
			{
				case 0x90: return 0x41; // Round Basket
						break;

				case 0xB2: return 0x3C; // Backpack 2
						break;

				case 0xAA: return 0x43; // Wooden Box
						break;

				case 0xA8: return 0x40; // Metal Box
						break;

				case 0xAB: return 0x4A; // Metal/Silver Chest
						break;

				case 0xA9: return 0x44; // Small Wooden Crate
						break;
			}
			break;

		case 0x20:
			if (this->id2=='\x06') return 0x09; // Coffin
			break;

		case 0x0A:
			switch (this->id2)
			{
				case 0x97:		// Bookcase
				case 0x98:		// Bookcase
				case 0x99:		// Bookcase
				case 0x9a:		// Bookcase
				case 0x9b:		// Bookcase
				case 0x9c:		// Bookcase
				case 0x9d:		// Bookcase
				case 0x9e: return 0x4d; // Bookcase
						break;

				case 0x4d:		// Fancy Armoire
				case 0x51:		// Fancy Armoire
				case 0x4c:		// Fancy Armoire
				case 0x50: return 0x4e; // Fancy Armoire
						break;

				case 0x4f:		// Wooden Armoire
				case 0x53:		// Wooden Armoire
				case 0x4e:		// Wooden Armoire
				case 0x52: return 0x4f; // Wooden Armoire
						break;

				case 0x30:		// chest of drawers (fancy)
				case 0x38: return 0x48; // chest of drawers (fancy)
						break;

				case 0x2c:		// chest of drawers (wood)
				case 0x34:		// chest of drawers (wood)
				case 0x3c:		// Dresser
				case 0x3d:		// Dresser
				case 0x44:		// Dresser
				case 0x35: return 0x51; // Dresser
						break;
				case 0xb2: if (this->morex==1) return 0x4a;
						break;
			}
			break;

		case 0x3e:
			 return 0x4C;
			 break;

		default:
			break;
			

	}
	return 0x47;
*/
	CONTINFOMAP::iterator iter( contInfo.find( getId() ) );
	if( iter==contInfo.end() || iter->second==contInfoGump.end() )
		return 0x47;
	else 
		return iter->second->second.gump;

}

/*
\brief Put an item into this container
\author Endymion
\param pi the containet where put into
\note do refresh
\remarks is inverse of additem
*/
void cItem::putInto( P_ITEM pi )
{
	VALIDATEPI(pi);
	pi->AddItem( this );
}


// author: LB purpose: returns the type of pack
// to handle its x,y coord system corretly.
// interpretation of the result:
// valid x,y ranges depending on type:
// type -1 : no pack
// type  1 : y-range 50  .. 100
// type  2 : y-range 30  .. 80
// type  3 : y-range 100 .. 150
// type  4 : y-range 40  .. 140
// x-range 18 .. 118 for 1,2,3
//         40 .. 140 for 4
//
/*short cItem::GetContGumpType()
{
*//*	switch (id())
	{
	case 0x09a8: return 1;
	case 0x09a9: return 2;
	case 0x09aa: return 1;
	case 0x09ab: return 3;
	case 0x09b0: return 1;
	case 0x09b2: return 4;

	case 0x0e3c: return 2;
	case 0x0e3d: return 2;
	case 0x0e3e: return 2;
	case 0x0e3f: return 2;
	case 0x0e40: return 3;
	case 0x0e41: return 3;
	case 0x0e42: return 3;
	case 0x0e43: return 3;
	case 0x0e75: return 4;

	case 0x0e76: return 1;
	case 0x0e77: return 4;
	case 0x0e78: return 2;
	case 0x0e79: return 1;
	case 0x0e7a: return 1;

	case 0x0e7c: return 3;
	case 0x0e7d: return 1;
	case 0x0e7e: return 2;
	case 0x0e7f: return 4;
	case 0x0e80: return 1;
	case 0x0e83: return 4;

	case 0x0EFA: return 4;	// spellbook. Position shouldn't matter, but as it can be opened like a backpack...(Duke)

	case 0x2006: return 5;	// a corpse/coffin
	default: return -1;
	}
}*/

void cItem::SetRandPosInCont(P_ITEM pCont)
{

	setPosition("z", 9);
	CONTINFOMAP::iterator iter( contInfo.find( pCont->getId() ) );
	if( iter==contInfo.end() || iter->second==contInfoGump.end()) {
		setPosition("x", RandomNum(18, 118));
		setPosition("y", RandomNum(50, 100));
		LogWarning("trying to put something INTO a non container, id=0x%X",pCont->getId() );
	}
	else {
		setPosition("x", RandomNum(iter->second->second.upperleft.x, iter->second->second.downright.x));
		setPosition("y", RandomNum(iter->second->second.upperleft.y, iter->second->second.downright.y));
		
	}
	
}

