  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "boats.h"
#include "debug.h"
#include "network.h"
#include "sndpkg.h"

#define X 0
#define Y 1

static std::map<int,boat_db> s_boat;

//============================================================================================
//UooS Item translations - You guys are the men! :o)

//[4]=direction of ship
//[4]=Which Item (PT Plank, SB Plank, Hatch, TMan)
//[2]=Coord (x,y) offsets
signed short int iSmallShipOffsets[4][4][2]=
// X  Y  X  Y  X  Y  X  Y
{ { {-2, 0}, {2, 0}, {0, -4}, {1, 4} },//Dir
  { {0, -2}, {0, 2}, {4, 0}, {-4, 0} },
  { {2, 0}, {-2, 0}, {0, 4}, {0, -4}  },
  { {0, 2},  {0,-2}, {-4, 0}, {4, 0}  }
};
//  P1    P2   Hold  Tiller
signed short int iMediumShipOffsets[4][4][2]=
// X  Y  X  Y  X  Y  X  Y
{ { {-2, 0}, {2, 0}, {0,-4}, {1, 5} },
  { {0,-2}, {0, 2}, {4, 0},{-5, 0} },
  { {2, 0}, {-2, 0}, {0, 4}, {0,-5} },
  { {0, 2}, {0,-2}, {-4, 0}, {5, 0} }
};
signed short int iLargeShipOffsets[4][4][2]=
// X  Y  X  Y  X  Y  X  Y
{ { {-2,-1}, {2,-1}, {0,-5}, {1, 5} },
  { {1,-2}, {1, 2}, {5, 0},{-5, 0} },
  { {2, 1}, {-2, 1}, {0, 5}, {0,-5} },
  { {-1, 2}, {-1,-2}, {-5, 0}, {5, 0} }
};
//Ship Items
//[4] = direction
//[6] = Which Item (PT Plank Up,PT Plank Down, SB Plank Up, SB Plank Down, Hatch, TMan)
UI08 cShipItems[4][6]=
{
 {0xB1,0xD5,0xB2,0xD4,0xAE,0x4E},
 {0x8A,0x89,0x85,0x84,0x65,0x53},
 {0xB2,0xD4,0xB1,0xD5,0xB9,0x4B},
 {0x85,0x84,0x8A,0x89,0x93,0x50}
};
//============================================================================================

/*!
\brief A sort of getboart() only more general
\todo delete or write it
*/
P_ITEM findmulti(Location where)
{
/*	int lastdist=30;
	P_ITEM pmulti=NULL;
	
	NxwItemWrapper si;
	si.fillItemsNearXYZ( where );
	for( ; !si.isEmpty(); si++ ) {
		P_ITEM pi=si.getItem();
		if(!ISVALIDPI(pi))
			continue;

		if (pi->id1>=0x40)
		{
			Location itmpos= pi->getPosition();

			int dx=abs((int)where.x - (int)itmpos.x);
			int dy=abs((int)where.y - (int)itmpos.y);
			int ret=(int)(hypot(dx, dy));

			if (ret<=lastdist)
			{
				lastdist=ret;
				if (inmulti(where,pi))
					pmulti=pi;
			}
		}
	}

	return pmulti;*/
	return NULL;
}

bool inmulti(Location where, P_ITEM pi)//see if they are in the multi at these chords (Z is NOT checked right now)
// PARAM WARNING: z is unreferenced
{
	VALIDATEPIR(pi,false);
	
	char temp[TEMP_STR_SIZE];
	int j;
	SI32 length;			// signed long int on Intel
	st_multi multi;
	MULFile *mfile;
	Map->SeekMulti(pi->id()-0x4000, &mfile, &length);
	length=length/sizeof(st_multi);
	if (length == -1 || length>=17000000)//Too big...
	{
		sprintf(temp,"inmulti() - Bad length in multi file. Avoiding stall. (Item Name: %s)\n", pi->getCurrentNameC() );
		LogError( temp ); // changed by Magius(CHE) (1)
		length = 0;
	}

	for (j=0;j<length;j++)
	{
		Location itmpos= pi->getPosition();
		mfile->get_st_multi(&multi);
		if (/*(multi.visible)&&*/((itmpos.x+multi.x) == where.x) && ((itmpos.y+multi.y) == where.y))
		{
			return true;
		}
	}
	return false;
}

void boats::PlankStuff(P_CHAR pc , P_ITEM pi)//If the plank is opened, double click Will send them here
{
	VALIDATEPC(pc);

	P_ITEM boat =GetBoat(pc->getPosition());
	if (boat!=NULL) //we are on boat
	{
		boat->type2 = 0; //STOP the BOAT
		LeaveBoat(pc,pi);//get of form boat
	}
	else // we are not on boat
	{

		P_ITEM boat2;

		boat2=search_boat_by_plank(pi);
		if (boat2 == NULL)
		{
			WarnOut("Can't find boats!\n");
			return;
		}

		boat2->type2 = 0; //STOP the BOAT

		NxwCharWrapper sc;
		sc.fillOwnedNpcs( pc, false, true );
		for( sc.rewind(); !sc.isEmpty(); sc++ ) 
		{
			P_CHAR pc_b=sc.getChar();

			if( ISVALIDPC( pc_b ))
			{
				Location boatpos= boat2->getPosition();
				pc_b->MoveTo( boatpos.x+1, boatpos.y+1, boatpos.z+2 );
				pc_b->setMultiSerial( boat2->getSerial32() );
				pc_b->teleport();
			   
			}
		}


        OpenPlank(pi); //lb

		Location boatpos= boat2->getPosition();
		pc->MoveTo( boatpos.x+1, boatpos.y+1, boatpos.z+3 );
		pc->teleport();
		pc->sysmsg(TRANSLATE("you entered a boat"));
       // pc->setMultiSerial( boat2->getSerial32() ); it's has just been called by pc->teleport, so wee need it not
	}

}

void boats::LeaveBoat(P_CHAR pc, P_ITEM pi)//Get off a boat (dbl clicked an open plank while on the boat.
{
	VALIDATEPC(pc);
	VALIDATEPI(pi);

	Location lipos = pi->getPosition();

	//long int pos, pos2, length;

	UI32 x,x2= lipos.x;
	UI32 y,y2= lipos.y;
	SI08 z= lipos.z;
	SI08 mz,sz,typ;
	P_ITEM pBoat=GetBoat(pc->getPosition());


	if (pBoat==NULL) return;
#define XX 6
#define YY 6
	for(x=x2-XX;x<=x2+XX;x++)
	{
		for(y=y2-YY;y<=y2+YY;y++)
		{
			sz=(signed char) Map->StaticTop(x,y,z); // MapElevation() doesnt work cauz we are in a multi !!

			mz=(signed char) Map->MapElevation(x,y);
			if (sz==illegal_z) typ=0;
			else typ=1;
			//o=Map->o_Type(x,y,z);

			if((typ==0 && mz!=-5) || (typ==1 && sz!=-5))// everthing the blocks a boat is ok to leave the boat ... LB
			{
				
				NxwCharWrapper sc;
				sc.fillOwnedNpcs( pc, false, true );
				for( sc.rewind(); !sc.isEmpty(); sc++ )
				{

					P_CHAR pc_b=sc.getChar();
					if( ISVALIDPC(pc_b))
					{
							
						pc_b->MoveTo( x,y, typ ? sz : mz );

						pc_b->setMultiSerial(INVALID);

						pc_b->teleport();

					}
				}

           		mapRegions->remove(pc);

				pc->setMultiSerial(INVALID);

				Location where;
				where.x = x;
				where.y = y;


				if (typ)
				{
					where.z = where.dispz= sz;
				}
				else
				{
					where.z = where.dispz = mz;
				}

				pc->setPosition( where );

				mapRegions->add(pc);

				pc->sysmsg(TRANSLATE("You left the boat."));
				pc->teleport();//Show them they moved.
				return;
			}
		}//for y
	}//for x
	pc->sysmsg(TRANSLATE("You cannot get off here!"));

}


void boats::TurnStuff_i(P_ITEM p_b, P_ITEM pi, int dir, int type)//Turn an item that was on the boat when the boat was turned.
{

	VALIDATEPI(pi);

	Location bpos= p_b->getPosition();
	Location itmpos = pi->getPosition();

	int dx= itmpos.x - bpos.x;//get their distance x
	int dy= itmpos.y - bpos.y;//and distance Y

	mapRegions->remove(pi);
	
	itmpos.x = bpos.x;
	itmpos.x = bpos.y;

	if(dir)//turning right
	{
		itmpos.x += dy*-1;
		itmpos.y += dx;
	} else {//turning left
		itmpos.x += dy;
		itmpos.y += dx*-1;
	}

	pi->setPosition( itmpos );
	mapRegions->add(pi);
	pi->Refresh();
}


void boats::TurnStuff_c(P_ITEM p_b, P_CHAR pc, int dir, int type)//Turn an item that was on the boat when the boat was turned.

{

	VALIDATEPC(pc);

	Location bpos= p_b->getPosition();
	Location charpos= pc->getPosition();

	int dx= charpos.x - bpos.x;
	int dy= charpos.y - bpos.y;

	mapRegions->remove(pc);

	charpos.x= bpos.x;
	charpos.y= bpos.y;
	
	if(dir)
	{
		charpos.x+= dy*-1;
		charpos.y+= dx;

	} else {
		charpos.x+= dy;
		charpos.y+= dx*-1;

	}
	pc->setPosition( charpos );
	//Set then in their new cell
	mapRegions->add(pc);
	
	pc->teleport();

}

void boats::Turn(P_ITEM pi, int turn)//Turn the boat item, and send all the people/items on the boat to turnboatstuff()
{

	VALIDATEPI(pi);
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	int id2=pi->id2 ;
	NXWSOCKET Send[MAXCLIENT];
	int serial;
	SI32 itiller, i1, i2, ihold;
	P_ITEM tiller, p1, p2, hold;
	int dir, d=0;


	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ ) {

		NXWCLIENT ps_i=sw.getClient();
		if( ps_i==NULL )
			continue;
		if(pi->distFrom(ps_i->currChar())<=BUILDRANGE)
		{
			Send[d]=ps_i->toInt();
			
			//////////////FOR ELCABESA VERY WARNING BY ENDYMION
			//////THIS PACKET PAUSE THE CLIENT
			SendPauseResumePkt(ps_i->toInt(), 0x01);
			d++;
		}
	}

	//Of course we need the boat items!
	serial=calcserial(pi->moreb1,pi->moreb2,pi->moreb3,pi->moreb4);
	if(serial<0) return;
	itiller = calcItemFromSer( serial | 0x40000000 );
	tiller = MAKE_ITEM_REF( itiller );
	VALIDATEPI(tiller);
	i1 = calcItemFromSer( pi->morex | 0x40000000);
	p1 = MAKE_ITEM_REF( i1 );
	VALIDATEPI(p1);
	i2 = calcItemFromSer( pi->morey | 0x40000000);
	p2 = MAKE_ITEM_REF( i2 );
	VALIDATEPI(p2);
	ihold = calcItemFromSer( pi->morez | 0x40000000);
	hold = MAKE_ITEM_REF( ihold );
	VALIDATEPI(hold);

	if(turn)//Right
	{
		pi->dir+=2;
		id2++;
	} else {//Left
		pi->dir-=2;
		id2--;
	}
	if(pi->dir>7) pi->dir-=8;//Make sure we dont have any DIR errors
	if(pi->dir<0) pi->dir+=8;
	if(id2<pi->more1) id2+=4;//make sure we don't have any id errors either
	if(id2>pi->more2) id2-=4;//Now you know what the min/max id is for :-)

	pi->id2=id2;//set the id

	if(pi->id2==pi->more1) pi->dir=0;//extra DIR error checking
	if(pi->id2==pi->more2) pi->dir=6;



//wait until set have appropriate function
   /* serial= pi->getSerial32(); // lb !!!

	int a;
    for(a=0;a<imultisp[serial%HASHMAX].max;a++)
	{
		c=imultisp[serial%HASHMAX].pointer[a];
		if (c!=-1)
			TurnStuff_i(pi,MAKE_ITEM_REF(c),turn,1);
	}

	for (a=0;a<cmultisp[serial%HASHMAX].max;a++)
	{
		c=cmultisp[serial%HASHMAX].pointer[a];
		if (c!=-1)
			TurnStuff_c(pi,MAKE_CHAR_REF(c),turn,0);
	}
*/
	//Set the DIR for use in the Offsets/IDs array
	dir=(pi->dir&0x0F)/2;

	//set it's Z to 0,0 inside the boat
	Location bpos= pi->getPosition();

	p1->MoveTo( bpos.x, bpos.y, p1->getPosition().z);
	p1->id2= cShipItems[dir][PORT_P_C];//change the ID

	p2->MoveTo( bpos.x, bpos.y, p2->getPosition().z);
	p2->id2=cShipItems[dir][STAR_P_C];

	tiller->MoveTo( bpos.x, bpos.y, tiller->getPosition().z);
	tiller->id2=cShipItems[dir][TILLERID];

	hold->MoveTo(bpos.x, bpos.y, hold->getPosition().z);
	hold->id2=cShipItems[dir][HOLDID];

	Location itmpos;

	switch(pi->more1)//Now set what size boat it is and move the specail items
	{
	case 0x00:
	case 0x04:
        	mapRegions->remove( p1 );
		itmpos= p1->getPosition();
		itmpos.x+= iSmallShipOffsets[dir][PORT_PLANK][X];
		itmpos.y+= iSmallShipOffsets[dir][PORT_PLANK][Y];
		p1->setPosition( itmpos );
		mapRegions->add( p1 );

		mapRegions->remove( p2 );
		itmpos= p2->getPosition();
		itmpos.x+= iSmallShipOffsets[dir][STARB_PLANK][X];
		itmpos.y+= iSmallShipOffsets[dir][STARB_PLANK][Y];
		p2->setPosition( itmpos );
		mapRegions->add( p2 );

		mapRegions->remove( tiller );
		itmpos= tiller->getPosition();
		itmpos.x+= iSmallShipOffsets[dir][TILLER][X];
		itmpos.y+= iSmallShipOffsets[dir][TILLER][Y];
		tiller->setPosition( itmpos );
		mapRegions->add( tiller );

		mapRegions->remove( hold );
		itmpos= hold->getPosition();
		itmpos.x+= iSmallShipOffsets[dir][HOLD][X];
		itmpos.y+= iSmallShipOffsets[dir][HOLD][Y];
		hold->setPosition( itmpos );
		mapRegions->add( hold );
		break;

	case 0x08:
	case 0x0C:
		mapRegions->remove( p1 );
		itmpos= p1->getPosition();
		itmpos.x+= iMediumShipOffsets[dir][PORT_PLANK][X];
		itmpos.y+= iMediumShipOffsets[dir][PORT_PLANK][Y];
		p1->setPosition( itmpos );
		mapRegions->add( p1 );

		mapRegions->remove( p2 );
		itmpos= p2->getPosition();
		itmpos.x+= iMediumShipOffsets[dir][STARB_PLANK][X];
		itmpos.y+= iMediumShipOffsets[dir][STARB_PLANK][Y];
		p2->setPosition( itmpos );
		mapRegions->add( p2 );

		mapRegions->remove( tiller );
		itmpos= tiller->getPosition();
		itmpos.x+= iMediumShipOffsets[dir][TILLER][X];
		itmpos.y+= iMediumShipOffsets[dir][TILLER][Y];
		tiller->setPosition( itmpos );
		mapRegions->add( tiller );

		mapRegions->remove( hold );
		itmpos= hold->getPosition();
		itmpos.x+= iMediumShipOffsets[dir][HOLD][X];
		itmpos.y+= iMediumShipOffsets[dir][HOLD][Y];
		hold->setPosition( itmpos );
		mapRegions->add( hold );

		break;
	case 0x10:
	case 0x14:

		mapRegions->remove( p1 );
		itmpos= p1->getPosition();
		itmpos.x+= iLargeShipOffsets[dir][PORT_PLANK][X];
		itmpos.y+= iLargeShipOffsets[dir][PORT_PLANK][Y];
		p1->setPosition( itmpos );
		mapRegions->add( p1 );

		mapRegions->remove( p2 );
		itmpos= p2->getPosition();
		itmpos.x+= iLargeShipOffsets[dir][STARB_PLANK][X];
		itmpos.y+= iLargeShipOffsets[dir][STARB_PLANK][Y];
		p2->setPosition( itmpos );
		mapRegions->add( p2 );

		mapRegions->remove( tiller );
		itmpos= tiller->getPosition();
		itmpos.x+= iLargeShipOffsets[dir][TILLER][X];
		itmpos.y+= iLargeShipOffsets[dir][TILLER][Y];
		tiller->setPosition( itmpos );
		mapRegions->add( tiller );

		mapRegions->remove( hold );
		itmpos= hold->getPosition();
		itmpos.x+= iLargeShipOffsets[dir][HOLD][X];
		itmpos.y+= iLargeShipOffsets[dir][HOLD][Y];
		hold->setPosition( itmpos );
		mapRegions->add( hold );

		break;

	default: { sprintf(temp,"Turnboatstuff() more1 error! more1 = %c not found!\n",pi->more1);
		       LogWarning(temp);
			 }
	}

	p1->Refresh();
	p2->Refresh();
	hold->Refresh();
	tiller->Refresh();

	for (int a=0;a<d;a++) {
		/////////FOR ELCABESA VERY IMPORTAT BY ENDY 
		///////THIS PACKET RESUME CLIENT
		SendPauseResumePkt(Send[a], 0x00);
	}
}

LOGICAL boats::Speech(P_CHAR pc, NXWSOCKET socket, std::string &talk)//See if they said a command.
{
	/*
		pc & socket validation done in talking()
	*/
	P_ITEM pBoat=GetBoat(pc->getPosition());
	//
	// As we don't want a message logged when not on a boat we cannot use VALIDATEPIR
	if( !ISVALIDPI( pBoat ) )
		return false;
	//
	// if the pc is not the boat owner..we don't care what he says
	//
	if(pBoat->getOwnerSerial32()!= pc->getSerial32())
	{
		return false;
	}
	boat_db* boat=search_boat(pBoat->getSerial32());
	if(boat==NULL)
		return  false;

	P_ITEM tiller=boat->p_tiller;
	VALIDATEPIR(tiller,false); // get the tiller man
	//
	// Sparhawk: talk has allready been capitalized in talking
	//
	//char msg[512];
	//strncpy(msg,talk,512); // make a local copy of the string,and make sure to not have a overflow
	//strncpy(msg,strupr(msg),512);// then convert the string to uppercase

	int dir=pBoat->dir&0x0F;
	if( talk == "FORWARD" || talk == "UNFURL SAIL" )
	{
		pBoat->type2=1;//Moving
		Move(socket,dir,pBoat);
		itemtalk(tiller, TRANSLATE("Aye, sir."));
		return true;
	}
	if( talk == "BACKWARD" )
	{
		pBoat->type2=2;//Moving backward
		if(dir>=4)
			dir-=4;
		else
			dir+=4;
		Move(socket,dir,pBoat);
		itemtalk(tiller, TRANSLATE("Aye, sir."));
		return true;
	}
	if( talk == "ONE LEFT" || talk == "DRIFT LEFT" )
	{
		dir-=2;
		if(dir<0)
			dir+=8;
		Move(socket,dir,pBoat);
		itemtalk(tiller,TRANSLATE("Aye, sir."));
		return true;
	}
	if( talk == "ONE RIGHT" || talk == "DRIFT RIGHT" )
	{
		dir+=2;
		if(dir>=8) dir-=8;
		Move(socket,dir,pBoat);
		itemtalk(tiller, TRANSLATE("Aye, sir."));
		return true;
	}
	if( talk == "STOP" || talk == "FURL SAIL" )
	{
		pBoat->type2=0;
		itemtalk(tiller,TRANSLATE("Aye, sir."));
		return true;
	}

	if( talk == "TURN LEFT" || talk == "TURN PORT" )
	{
		if (good_position(pBoat, pBoat->getPosition(), -1) && collision(pBoat,pBoat->getPosition(),-1)==false)
		{
		  Turn(pBoat,0);
		  itemtalk(tiller, TRANSLATE("Aye, sir."));
		  return true;
		}
		else
		{
			pBoat->type2=0;
			itemtalk(tiller, TRANSLATE("Arr,somethings in the way"));
			return true;
		}
	}

	if( talk == "TURN RIGHT" || talk == "TURN STARBOARD" )
	{
		if (good_position(pBoat, pBoat->getPosition(), 1) && collision(pBoat,pBoat->getPosition(),1)==false)
		{
		  Turn(pBoat,1);
		  itemtalk(tiller, TRANSLATE("Aye, sir."));
		  return true;
		} else
		{
			pBoat->type2=0;
			itemtalk(tiller, TRANSLATE("Arr,somethings in the way"));
			return true;
		}
	}
	if( talk == "COME ABOUT" || talk == "TURN ABOUT" )
	{

		if (good_position(pBoat, pBoat->getPosition(), 2) && collision(pBoat,pBoat->getPosition(),2)==false)
		{
			Turn(pBoat,1);
			Turn(pBoat,1);
			itemtalk(tiller, TRANSLATE("Aye, sir."));
			return true;
		}
		else
		{
			pBoat->type2=0;
			itemtalk(tiller, TRANSLATE("Arr,somethings in the way"));
			return true;
		}
	}
	if( talk.substr(0, 9) == "SET NAME " )
	{
		tiller->setCurrentName( talk.substr( 9 ).c_str() );
		//char tmp[200];
		//sprintf(tmp,"%s%s", "a ship named ", &msg[9]);
		//tiller->setCurrentName(tmp);
		return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////
////						NEW BOAT-SYSTEM					////
////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////
// Function name     : tile_check ( helper function for good_position )
// Description       : check if all the boats tile are in water
// Return type       : bool
// Author            : Elcabesa
// Changes           : none yet
// Called form		 : boats::good_position()


LOGICAL boats::tile_check(st_multi multi,P_ITEM pBoat,map_st map,int x, int y,int dir)
{
	land_st land;
	int dx,dy;
	switch(dir)
		{
		case -1:
			dx=x-multi.y;
			dy=y+multi.x;
			break;
		case 0:
			dx=x+multi.x;
			dy=y+multi.y;
			break;
		case 1:
			dx=x+multi.y;
			dy=y-multi.x;
			break;

		case 2:

			dx=x-multi.y;

			dy=y-multi.x;

			break;
		}
	MapStaticIterator msi(dx,dy);
	staticrecord *stat;
	int loopexit=0;
	tile_st tile;
	while ( ((stat = msi.Next())!=NULL) && (++loopexit <MAXLOOPS) )
	{
		msi.GetTile(&tile);
		if(!(strstr((char *) tile.name, "water") || strstr((char *) tile.name, "lava")))
		{
			Map->SeekLand(map.id, &land);
			if (!(land.flag1&0x80))//not a "wet" tile
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return true;
		}

	}
	return false;
}



///////////////////////////////////////////////////////////////////
// Function name     : good_position
// Description       : check if this is a good position for building or moving a boat
// Return type       : void
// Author            : Elcabesa
// Changes           : none yet

LOGICAL boats::good_position(P_ITEM pBoat, Location where, int dir)
{
	UI32 x= where.x, y= where.y;
	LOGICAL good_pos=false;
	char temp[TEMP_STR_SIZE];
	map_st map;
	int j;
	SI32 length;			// signed long int on Intel
	st_multi multi;
	MULFile *mfile;
	Map->SeekMulti(pBoat->id()-0x4000, &mfile, &length);
	length=length/sizeof(st_multi);
	if (length == -1 || length>=17000000)//Too big...
	{
		sprintf(temp,"good_position() - Bad length in multi file. Avoiding stall. (Item Name: %s)\n", pBoat->getCurrentNameC() );
		LogError( temp ); // changed by Magius(CHE) (1)
		length = 0;
	}
	for (j=0;j<length;j++)
	{
	    mfile->get_st_multi(&multi);
		if (multi.visible)
		{
			switch(dir)
			{
			case -1:
				map = Map->SeekMap0(x-multi.y,y+multi.x);
				break;
			case 0:
				map = Map->SeekMap0(x+multi.x,y+multi.y);
				break;
			case 1:
				map = Map->SeekMap0(x+multi.y,y-multi.x);
				break;

			case 2:

				map = Map->SeekMap0(x-multi.x,y-multi.y);

				break;
			}
			switch(map.id)
			{
	//water tiles:
				case 0x00A8://168
				case 0x00A9://169
				case 0x00AA://170
				case 0x00Ab://171
				case 0x0136://310
				case 0x0137://311
				case 0x3FF0://16368
				case 0x3FF1://16369
				case 0x3FF2://16370
				case 0x3FF3://16371
	//Lava tiles:
				case 0x01F4://500
				case 0x01F5://501
				case 0x01F6://502
				case 0x01F7://503
					good_pos=true;
					break;
				default:// we are in default if we are nearer coast
					{
						good_pos=tile_check(multi,pBoat,map,x,y,dir);
						if (good_pos==false)
							return false;
					}
			}
		}
	}
	return good_pos;
}



///////////////////////////////////////////////////////////////////
// Function name     : Build
// Description       : build a boat
// Return type       : void
// Author            : Elcabesa
// Changes           : none yet
// Called form	     : buildhouse()
LOGICAL boats::Build(NXWSOCKET  s, P_ITEM pBoat, char id2)
{
	if ( s < 0 || s >= now )
		return false;
	P_CHAR pc_cs=MAKE_CHARREF_LRV(currchar[s],false);

	int nid2=id2;

	if( !ISVALIDPI(pBoat) )
	{
		pc_cs->sysmsg(TRANSLATE("There was an error creating that boat."));
		return false;
	}

	if(id2!=0x00 && id2!=0x04 && id2!=0x08 && id2!=0x0C && id2!=0x10 && id2!=0x14)//Valid boat ids (must start pointing north!)
	{
		pc_cs->sysmsg(TRANSLATE("The deed is broken, please contact a Game Master."));
		return false;
	}
	//Start checking for a valid position:
	if (good_position(pBoat, pBoat->getPosition(), 0)==false)
	{
		return false;
	}
	if(collision(pBoat, pBoat->getPosition(),0)==true)
	{
		return false;
	}
	// Okay we found a good  place....

	pBoat->setOwnerSerial32(pc_cs->getSerial32());
	pBoat->more1=id2;//Set min ID
	pBoat->more2=nid2+3;//set MAX id
	pBoat->type=ITYPE_BOATS;//Boat type
	
	Location lb = pBoat->getPosition();
	lb.z = -5;
	pBoat->setPosition(lb);//Z in water

	//	strcpy(pBoat->name,"a mast");//Name is something other than "%s's house"
	pBoat->setCurrentName("a mast");

	P_ITEM pTiller=item::CreateFromScript( "$item_tillerman" );
	VALIDATEPIR( pTiller, false );
	Location lt = pTiller->getPosition();
	lt.z=-5;
	pTiller->priv=0;

	P_ITEM pPlankR=item::CreateFromScript( "$item_plank2" );//Plank2 is on the RIGHT side of the boat
	VALIDATEPIR( pPlankR, false );
	pPlankR->type=ITYPE_BOATS;
	pPlankR->type2=3;
	pPlankR->more1= pBoat->getSerial().ser1;//Lock this item!
	pPlankR->more2= pBoat->getSerial().ser2;
	pPlankR->more3= pBoat->getSerial().ser3;
	pPlankR->more4= pBoat->getSerial().ser4;
	Location lpr = pPlankR->getPosition();
	lpr.z=-5;
	pPlankR->priv=0;//Nodecay

	P_ITEM pPlankL=item::CreateFromScript( "$item_plank1" );//Plank1 is on the LEFT side of the boat
	VALIDATEPIR( pPlankL, false );
	pPlankL->type=ITYPE_BOATS;//Boat type
	pPlankL->type2=3;//Plank sub type
	pPlankL->more1= pBoat->getSerial().ser1;
	pPlankL->more2= pBoat->getSerial().ser2;//Lock this
	pPlankL->more3= pBoat->getSerial().ser3;
	pPlankL->more4= pBoat->getSerial().ser4;
	Location lpl= pPlankL->getPosition();
	lpl.z =-5;
	pPlankL->priv=0;

	P_ITEM pHold=item::CreateFromScript( "$item_hold1" );
	VALIDATEPIR( pHold, false );
	pHold->more1= pBoat->getSerial().ser1;//Lock this too :-)
	pHold->more2= pBoat->getSerial().ser2;
	pHold->more3= pBoat->getSerial().ser3;
	pHold->more4= pBoat->getSerial().ser4;

	pHold->type=ITYPE_CONTAINER;//Container
	Location lh = pHold->getPosition();
	lh.z=-5;
	pHold->priv=0;




	pBoat->moreb1= pTiller->getSerial().ser1;//Tiller ser stored in boat's Moreb
	pBoat->moreb2= pTiller->getSerial().ser2;
	pBoat->moreb3= pTiller->getSerial().ser3;
	pBoat->moreb4= pTiller->getSerial().ser4;
	pBoat->morex= pPlankL->getSerial32();//Store the other stuff anywhere it will fit :-)
	pBoat->morey= pPlankR->getSerial32();
	pBoat->morez= pHold->getSerial32();

	Location boatpos= pBoat->getPosition();



	switch(id2)//Give everything the right Z for it size boat
	{
	case 0x00:
	case 0x04:
		lt.x = boatpos.x + 1;
		lt.y = boatpos.y + 4;
		lpr.x = boatpos.x + 2;
		lpr.y = boatpos.y;
		lpl.x = boatpos.x - 2;
		lpl.y = boatpos.y;
		lh.x = boatpos.x;
		lh.y = boatpos.y - 4;
		break;
	case 0x08:
	case 0x0C:
		lt.x = boatpos.x + 1;
		lt.y = boatpos.y + 5;
		lpr.x = boatpos.x + 2;
		lpr.y = boatpos.y;
		lpl.x = boatpos.x - 2;
		lpl.y = boatpos.y;
		lh.x = boatpos.x;
		lh.y = boatpos.y - 4;
		break;
	case 0x10:
	case 0x14:
		lt.x = boatpos.x + 1;
		lt.y = boatpos.y + 5;
		lpr.x = boatpos.x + 2;
		lpr.y = boatpos.y - 1;
		lpl.x = boatpos.x - 2;
		lpl.y = boatpos.y - 1;
		lh.x = boatpos.x;
		lh.y = boatpos.y - 5;
		break;
	}

	pTiller->setPosition( lt );
	pPlankL->setPosition( lpl );
	pPlankR->setPosition( lpr );
	pHold->setPosition( lh );

	mapRegions->add(pTiller);//Make sure everything is in da regions!
	mapRegions->add(pPlankL);
	mapRegions->add(pPlankR);
	mapRegions->add(pHold);
	mapRegions->add(pBoat);

	//their x pos is set by BuildHouse(), so just fix their Z...
	boatpos.z+=3;
	boatpos.dispz=boatpos.z;

	pc_cs->MoveTo(boatpos);
	//setserial(DEREF_P_CHAR(pc_cs),DEREF_P_ITEM(pBoat),8);
	pc_cs->setMultiSerial( pBoat->getSerial32() );
	insert_boat(pBoat); // insert the boat in the boat_database
	return true;
}

///////////////////////////////////////////////////////////////////
// Function name     : collision
// Description       : handle if at these coord there is another boat
// Return type       : bool TRUE boat collision,FALSE not obat collision
// Author            : Elcabesa
LOGICAL boats::collision(P_ITEM pi,Location where,int dir)
{
	int x= where.x, y= where.y;
	std::map<int,boat_db>::iterator iter_boat;
	for(iter_boat=s_boat.begin();iter_boat!=s_boat.end();iter_boat++)
	{
		boat_db coll=iter_boat->second;
		if(coll.serial != pi->getSerial32())
		{
			int xx=abs(x - (int)coll.p_serial->getPosition().x);
			int yy=abs(y - (int)coll.p_serial->getPosition().y);
			double dist=hypot(xx, yy);
			if(dist<10)
			{
				if(boat_collision(pi,x,y,dir,coll.p_serial)==true)

				return true;

			}
		}
	}
	return false;
}

/*!
 \brief Check if 2 boats are put upon
 \author Elcabesa
 \return true if collision, else false
 \param pBoat1 first boat
 \param pBoat2 second boat
 */
LOGICAL boats::boat_collision(P_ITEM pBoat1,UI32 x1, UI32 y1,int dir,P_ITEM pBoat2)
{
	char temp[TEMP_STR_SIZE];

	int i,j;

	UI32 x,y;
	SI32 length1,length2;			// signed long int on Intel
	st_multi multi1,multi2;
	MULFile *mfile1,*mfile2;
	Map->SeekMulti(pBoat1->id()-0x4000, &mfile1, &length1);
	length1=length1/sizeof(st_multi);
	if (length1 == -1 || length1>=17000000)//Too big...
	{
		sprintf(temp,"boat_collision() - Bad length in multi file. Avoiding stall. (Item Name: %s)\n", pBoat1->getCurrentNameC() );
		LogError( temp );
		length1 = 0;
	}
	Map->SeekMulti(pBoat2->id()-0x4000, &mfile2, &length2);
	length2=length2/sizeof(st_multi);
	if (length2 == -1 || length2>=17000000)//Too big...
	{
		sprintf(temp,"boat_collision() - Bad length in multi file. Avoiding stall. (Item Name: %s)\n", pBoat2->getCurrentNameC() );
		LogError( temp );
		length2 = 0;
	}
	for(i=0;i<length1;i++)
	{
		for (j=0;j<length2;j++)
		{
			mfile1->get_st_multi(&multi1);
			mfile2->get_st_multi(&multi2);


			switch(dir)
			{
			case -1:
				x=x1-multi1.y;
				y=y1+multi1.x;
				break;

			case 0:
				x=x1+multi1.x;
				y=y1+multi1.y;
				break;

			case 1:
				x=x1+multi1.y;
				y=y1-multi1.x;
				break;

			case 2:
				x=x1-multi1.x;
				y=y1-multi1.y;
				break;

			default:
				LogError("boat_collision() - bad boat turning direction\n");
			}

			if (multi1.visible&&multi2.visible)
			{
				if ( (x==multi2.x+pBoat2->getPosition().x) && (y==multi2.y+pBoat2->getPosition().y) )
				{
					return true;
				}

			}		
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////
// Function name     : OpenPlank
// Description       : Open, or close the plank (called from keytarget() )
// Return type       : void
// Author            : unknow
// Changes           : none yet

void boats::OpenPlank(P_ITEM pi)
{
	switch(pi->id2)
	{
		//Open plank->
		case (unsigned char)0xE9: pi->id2=(unsigned char)0x84; break;
		case (unsigned char)0xB1: pi->id2=(unsigned char)0xD5; break;
		case (unsigned char)0xB2: pi->id2=(unsigned char)0xD4; break;
		case (unsigned char)0x8A: pi->id2=(unsigned char)0x89; break;
		case (unsigned char)0x85: pi->id2=(unsigned char)0x84; break;
		//Close Plank->
		case (unsigned char)0x84: pi->id2=(unsigned char)0xE9; break;
		case (unsigned char)0xD5: pi->id2=(unsigned char)0xB1; break;
		case (unsigned char)0xD4: pi->id2=(unsigned char)0xB2; break;
		case (unsigned char)0x89: pi->id2=(unsigned char)0x8A; break;
		default: LogWarning("WARNING: Invalid plank ID called! Plank %i '%s' [ %04x ]\n",DEREF_P_ITEM(pi),pi->getCurrentNameC(),pi->id()); break;
	}
}

/*!
\brief check if there is a boat at this position and return the boat
\return the pointer to the boat or NULL
\author Elcabesa
*/
P_ITEM boats::GetBoat(Location pos)
{

	std::map<int,boat_db>::iterator iter_boat;
	if( s_boat.size()==0 )
		return NULL;
	for(iter_boat=s_boat.begin();iter_boat!=s_boat.end();iter_boat++)
	{
		boat_db boat=iter_boat->second;
		P_ITEM pBoat=boat.p_serial;
		if(!ISVALIDPI(pBoat))
			continue;
		UI32 xx= ( pos.x < boat.p_serial->getPosition().x ? pos.x - boat.p_serial->getPosition().x : boat.p_serial->getPosition().x - pos.x );
		UI32 yy= ( pos.y < boat.p_serial->getPosition().y ? pos.y - boat.p_serial->getPosition().y : boat.p_serial->getPosition().y - pos.y );
		double dist=hypot(xx, yy);
		if(dist<10)
		{
			int i;
			SI32 length;			// signed long int on Intel
			st_multi multi;
			MULFile *mfile;

			Map->SeekMulti(pBoat->id()-0x4000, &mfile, &length);
			length=length/sizeof(st_multi);
			if (length == -1 || length>=17000000)//Too big...
			{
				LogError("GetBoat() - Bad length in multi file. Avoiding stall. (Item Name: %s)\n", pBoat->getCurrentNameC() );
				length = 0;
			}
			for(i=0;i<length;i++)
			{
				mfile->get_st_multi(&multi);
				if (   ((UI32)(multi.x + pBoat->getPosition().x) == pos.x) && ((UI32)(multi.y + pBoat->getPosition().y) == pos.y) )
				{
					return  pBoat;
				}
			}
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////
// Function name     : Move
// Description       : move a boat, not turn it
// Return type       : int
// Author            : unknow
// Changes           : none yet

void boats::Move(NXWSOCKET  s, int dir, P_ITEM pBoat)
{
	iMove(s,dir,pBoat,false);
}


///////////////////////////////////////////////////////////////////
// Function name     : iMove
// Description       : really move a boat, not turn it, and move all the items on a boat
// Return type       : int
// Parameter		 : bool forced TRUE the boat is forced to go, it doesn't check block
// Author            : Elcabesa
// Changes           : none yet

void boats::iMove(NXWSOCKET  s, int dir, P_ITEM pBoat, LOGICAL forced)
{
	int tx=0,ty=0;
	int serial;

	if (pBoat==NULL) return;
	boat_db* boat=search_boat(pBoat->getSerial32());

	if(boat==NULL)
		return;
	P_ITEM tiller=boat->p_tiller;
	P_ITEM p1=boat->p_l_plank;
	P_ITEM p2=boat->p_r_plank;
	P_ITEM hold=boat->p_container;

	//////////////FOR ELCABESA VERY WARNING BY ENDYMION
	//////THIS PACKET PAUSE THE CLIENT
	SendPauseResumePkt(s, 0x01);

	switch(dir&0x0F)//Which DIR is it going in?
	{
	case 0:
		ty--;
		break;
	case 1:
		tx++;
		ty--;
		break;
	case 2:
		tx++;
		break;
	case 3:
		tx++;
		ty++;
		break;
	case 4:
		ty++;
		break;
	case 5:
		tx--;
		ty++;
		break;
	case 6:
		tx--;
		break;
	case 7:
		tx--;
		ty--;
		break;
	default:
		{
		  LogWarning("Warning: Boat direction error: %i int boat %i\n", pBoat->dir&0x0F, pBoat->getSerial32());
		  break;
		}
	}


#define XBORDER 200
#define YBORDER 200

	Location boatpos= pBoat->getPosition();

	if( (boatpos.x+tx<=XBORDER || boatpos.x+tx>=((MapTileWidth*8)-XBORDER))
		|| (boatpos.y+ty<=YBORDER || boatpos.y+ty>=((MapTileHeight*8)-YBORDER))) //bugfix LB
	{
		pBoat->type2=0;
		itemtalk(tiller,TRANSLATE("Arr, Sir, we've hit rough waters!"));
		SendPauseResumePkt(s, 0x00);
		return;
	}


	boatpos.x+= tx;
	boatpos.y+= ty;

	if(!good_position(pBoat, boatpos, 0) && (!forced))
	{
		pBoat->type2=0;
		itemtalk(tiller, TRANSLATE("Arr, somethings in the way!"));
		SendPauseResumePkt(s, 0x00);
		return;
	}
	if(collision(pBoat, boatpos,0)==true)
	{
		pBoat->type2=0;
		itemtalk(tiller, TRANSLATE("Arr, another ship in the way"));
		SendPauseResumePkt(s, 0x00);
		return;
	}

	Location tillerpos= tiller->getPosition();
	tillerpos.x+= tx;
	tillerpos.y+= ty;

	Location p1pos= p1->getPosition();
	p1pos.x+= tx;
	p1pos.y+= ty;

	Location p2pos= p2->getPosition();
	p2pos.x+= tx;
	p2pos.y+= ty;

	Location holdpos= hold->getPosition();
	holdpos.x+= tx;
	holdpos.y+= ty;

	//Move all the special items
	pBoat->MoveTo( boatpos );
	tiller->MoveTo( tillerpos );
	p1->MoveTo( p1pos );
	p2->MoveTo( p2pos );
	hold->MoveTo( holdpos );

	serial= pBoat->getSerial32();

/*wait until set hav appropriate function
	for (a=0;a<imultisp[serial%HASHMAX].max;a++)  // move all item upside the boat
	{
		c=imultisp[serial%HASHMAX].pointer[a];
		if(c!=-1)
		{
			P_ITEM pi= MAKE_ITEMREF_LOGGED(c,err);
			if(!err)
			{
				mapRegions->remove(pi);
				Location itmpos= pi->getPosition();
				itmpos.x+= tx;
				itmpos.y+= ty;
				pi->setPosition( itmpos );
				pi->Refresh();
				mapRegions->add(pi);
			}
		}
	}

	for (a=0;a<cmultisp[serial%HASHMAX].max;a++) // move all char upside the boat
	{
		c=cmultisp[serial%HASHMAX].pointer[a];
		if (c!=-1)
		{
		   pc_c=MAKE_CHARREF_LOGGED(c,err);
		   if (!err)
		   {
			   Location charpos= pc_c->getPosition();
			   mapRegions->remove(pc_c);
			   charpos.x+= tx;
			   charpos.y+= ty;
			   pc_c->MoveTo(charpos);
			   pc_c->teleport();
			   mapRegions->add(pc_c);
		   }
		}
	}
*/

	SendPauseResumePkt(s, 0x00);
	pBoat->Refresh();
	tiller->Refresh();
	p1->Refresh();
	p2->Refresh();
	hold->Refresh();
}

/*!
\brief insert a boat inside boat_db struct and add it to the s_boat map
\author elcabesa
\param pi pointer to the boat to be inserted
\since 0.8
*/
void insert_boat(P_ITEM pi)
{
	boat_db boat;
	boat.serial= pi->getSerial32();
	boat.tiller_serial= calcserial(pi->moreb1,pi->moreb2,pi->moreb3,pi->moreb4);
	boat.l_plank_serial= pi->morex;
	boat.r_plank_serial= pi->morey;
	boat.container= pi->morez;
	boat.p_serial= pi;
	boat.p_l_plank= pointers::findItemBySerial(boat.l_plank_serial);
	boat.p_r_plank= pointers::findItemBySerial(boat.r_plank_serial);
	boat.p_tiller= pointers::findItemBySerial(boat.tiller_serial);
	boat.p_container= pointers::findItemBySerial(boat.container);
	s_boat.insert(std::make_pair(pi->getSerial32(), boat)); // insert a boat in the boat search tree
}



boat_db* search_boat(SI32 ser)
{
	std::map<int,boat_db>::iterator iter_boat;
	iter_boat= s_boat.find(ser);
	if (iter_boat == s_boat.end()) return 0;
	else
		return &iter_boat->second;
}


P_ITEM search_boat_by_plank(P_ITEM pl)
{
	VALIDATEPIR(pl,NULL);
	Serial ser;
	ser.ser1=pl->more1;
	ser.ser2=pl->more2;
	ser.ser3=pl->more3;
	ser.ser4=pl->more4;
	boat_db*  boat=search_boat(ser.serial32);
	return boat->p_serial;
}
