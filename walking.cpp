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
*/

#include "nxwcommn.h"
#include "network.h"
#include "sregions.h"
#include "sndpkg.h"
#include "debug.h"
#include "amx/amxcback.h"
#include "trigger.h"
#include "house.h"
#include "tmpeff.h"
#include "packets.h"

/*!
\brief Calculates the adjacent direction (counterclockwise)
\author Duke
\param dir initial direction
\return the adjacent direction
*/
int getLeftDir(int dir)
{
	dir &= 7;	// make sure it's valid
	return dir==0 ? 7 : dir-1;
}
/*!
\brief Calculates the adjacent direction (clockwise)
\author Duke
\param dir initial direction
\return the adjacent direction
*/
int getRightDir(int dir)
{
	dir &= 7;
	return dir==7 ? 0 : dir+1;
}

/*!
\brief Calculats and changes the given coords one step into the given direction
\author Duke
\param dir the direction
\param x pointer to the x coord
\param y pointer to the y coord
\todo use reference instead of pointer?
*/
void getXYfromDir(int dir, int *x, int *y)
{
	switch(dir&0x07)
	{
	case 0: (*y)--;		break;
	case 1: (*x)++; (*y)--;	break;
	case 2: (*x)++;		break;
	case 3: (*x)++; (*y)++;	break;
	case 4: (*y)++;		break;
	case 5: (*x)--; (*y)++;	break;
	case 6: (*x)--;		break;
	case 7: (*x)--; (*y)--;	break;
	}
}

/*!
\brief Return direction to target coordinate
\param pc pointer to the char
\param targetX the target X-coordinate
\param targetY the target Y-coordinate
\return the direction to the coordinats
*/
int getDirFromXY( P_CHAR pc, UI32 targetX, UI32 targetY )
{
	int direction = pc->dir;
	Location pcpos= pc->getPosition();

	if ( targetX < pcpos.x )
		if ( targetY < pcpos.y )
			direction = NORTHWEST;
		else if ( targetY > pcpos.y )
			direction = SOUTHWEST;
		else
			direction = WEST;
	else
		if ( targetX > pcpos.x )
			if ( targetY < pcpos.y )
				direction = NORTHEAST;
			else if ( targetY > pcpos.y )
				direction = SOUTHEAST;
			else
				direction = EAST;
		else if ( targetY < pcpos.y )
			direction = NORTH;
		else
			direction = SOUTH;
	return direction;
}


/*!
\brief Checks if the Char is allowed to move at all (not frozen, overloaded...)
\author Duke
\param pc ptr to the car
\param sequence walksequence
\return true if the walk is allowed
*/
bool WalkHandleAllowance(P_CHAR pc, int sequence)
{
	VALIDATEPCR(pc,false);
	if (pc->isStabled() || pc->mounted) return false; // shouldnt be called for stabled pets, just to be on the safe side

	NXWSOCKET  s = pc->getSocket();
	if(s!=INVALID)
	{
		if ((walksequence[s]+1!=sequence)&&(sequence!=256))
		{
			deny(s, pc, sequence);
			return false;
		}
	}

	if (!pc->dead && !pc->npc && !(pc->IsGMorCounselor()))
	{
		if (!weights::CheckWeight(pc) || (pc->stm<1)) // this has to executes, even if s==-1, thus that many !=-1's
		{
			if (s!=INVALID)
			{
			  pc->sysmsg(TRANSLATE("You are too fatigued to move."));
			  walksequence[s]=INVALID;
			  pc->teleport( TELEFLAG_NONE );
			  deny(s, pc, sequence); // !!!
			  return false;
			}
		}
	}

	if(pc->IsFrozen()) // lord binary !!!
	{
		pc->teleport();
		if (s>INVALID)
		{
			if (pc->casting)
				pc->sysmsg(TRANSLATE("You cannot move while casting."));
			else
				pc->sysmsg(TRANSLATE("You are frozen and cannot move."));
			deny(s, pc, sequence); // !!!
		}
		return false;
	}
	return true;
}


///////////////
// Name:	WalkingHandleHiding
// history:	cut from WalkHandleRunning() by Xanathar, 14.06.2001
// Purpose:	handles stealth/hiding
//
bool WalkingHandleHiding (P_CHAR pc, int dir)
{
	VALIDATEPCR(pc, false);

	if (dir&0x80)
		//AntiChrist - we have to unhide always if running
                if (!pc->IsHiddenBySpell())
			pc->unHide();
	else {
		if(pc->stealth>INVALID)
		{ //AntiChrist - Stealth
			pc->stealth++;
			if( (UI32)pc->stealth >= (UI32)(((SrvParms->maxstealthsteps*pc->skill[STEALTH])/1000.0)) )
				pc->unHide();
		}
		else
		{
			if (pc->IsHidden() && !pc->IsHiddenBySpell()) //Luxor's invisibility spell
				pc->unHide();
		}
	}


	return true;

}
///////////////
// Name:	WalkHandleRunning
// history:	cut from walking() by Duke, 27.10.2000
// Purpose:	handles running, stamina
//
bool WalkHandleRunning(P_CHAR pc, int dir)
{
	VALIDATEPCR(pc, false);

	if (dir&0x80)
	{ //AntiChrist -- if running
                pc->setRunning();
		//AntiChrist - we have to unhide always if running
		if( pc->IsHidden() && !pc->IsHiddenBySpell() )
			pc->unHide();
		//Running stamina - AntiChrist
		pc->updateRegenTimer( STAT_STAMINA );  //Don't regenerate stamina while running
		pc->running++;

		// Horse Stamina loss bug Fixed by blackwind.
		// Without loss of stamina players will be able to runaway for ever..
		if (!pc->dead)
		{
			if(
				( (!pc->isMounting() && pc->running>(SrvParms->runningstaminasteps)*2) ) ||
				( pc->isMounting() && pc->running>(SrvParms->runningstaminasteps*2)*2 )
				)
			{ //The first *2 it's because i noticed that a step(animation) correspond to 2 walking calls
				pc->running=0;
				--pc->stm;
				pc->updateStats(2);
			}
		}
	}
	else
	{
		pc->running=0;
		if( pc->stealth > INVALID )
		{ //AntiChrist - Stealth
			if( /*pc->stealth >= 0 &&*/		// give them at least one step, regardless of skill (Duke)
				(UI32)pc->stealth > ((SrvParms->maxstealthsteps*pc->skill[STEALTH])/1000))
			{
				pc->unHide();
			}
			++pc->stealth;
		}
		else
		{
			if( pc->IsHidden() && !pc->IsHiddenBySpell() )
				pc->unHide();
		}
	}
	return true;

}

///////////////
// Name:	WalkCollectBlockers
// history:	cut from walking() by Duke, 20.11.2000
// Purpose:	Collects Landscape plus static and dynamic items in an array
//
UI32 WalkCollectBlockers(P_CHAR pc)
{
	VALIDATEPCR(pc, 0);
	UI32 blockers_count= 0;


	int mapid = 0;
	SI08 mapz = Map->AverageMapElevation(pc->getPosition(), mapid);
	if (mapz != illegal_z)
	{
		land_st land;
		Map->SeekLand(mapid, &land);

		xyblock[blockers_count].type=0;
		xyblock[blockers_count].basez = mapz;
		xyblock[blockers_count].id = mapid;
		xyblock[blockers_count].flag1=land.flag1;
		xyblock[blockers_count].flag2=land.flag2;
		xyblock[blockers_count].flag3=land.flag3;
		xyblock[blockers_count].flag4=land.flag4;
		xyblock[blockers_count].height=0;
		xyblock[blockers_count].weight=255;
		++blockers_count;
	}

	NxwItemWrapper si;
	si.fillItemsNearXYZ( pc->getPosition() );
	for( si.rewind(); !si.isEmpty(); si++ ) {
		P_ITEM pi=si.getItem();
		if(!ISVALIDPI(pi))
			continue;
		if (pi->id1<0x40) // Not a Multi
		{
			if ((pi->getPosition("x")== pc->getPosition("x")) && (pi->getPosition("y")==pc->getPosition("y")))
			{
				if (pi->trigger!=0)
				{
					if ((pi->trigtype==1)&&(!pc->dead))
					{
						if ( TIMEOUT( pi->disabled ) )//AntiChrist
						{
							triggerItem(pc->getSocket(), pi, TRIGTYPE_WALKOVER);  //When player steps on a trigger
						}
					}
				}
				else
				{
					
					if (pi->amxevents[EVENT_IONWALKOVER] != NULL )
					{
						pi->amxevents[EVENT_IONWALKOVER]->Call( pi->getSerial32(), pc->getSocket() );
						g_bByPass = false; //ndEndy ?? what is this?
					}
					/*
					pi->runAmxEvent( EVENT_IONWALKOVER, pi->getSerial32(), pc->getSocket() );
					g_bByPass = false;
					*/
				}
				tile_st tile;
				Map->SeekTile(pi->id(), &tile);
				xyblock[blockers_count].type=1;
				xyblock[blockers_count].basez= pi->getPosition("z");
				xyblock[blockers_count].id=pi->id();
				xyblock[blockers_count].flag1=tile.flag1;
				xyblock[blockers_count].flag2=tile.flag2;
				xyblock[blockers_count].flag3=tile.flag3;
				xyblock[blockers_count].flag4=tile.flag4;
				xyblock[blockers_count].height=tile.height;
				xyblock[blockers_count].weight=tile.weight;
				++blockers_count;
			}
		}
		else	// Multi Tile
		{
			if ( (abs(pi->getPosition("x") - (int)pc->getPosition("x"))<=BUILDRANGE) &&
				 (abs(pi->getPosition("y") - (int)pc->getPosition("y"))<=BUILDRANGE) )
			{
				MULFile *mfile = NULL;
				SI32 length = 0;

				Map->SeekMulti(pi->id()-0x4000, &mfile, &length);
				length=length/MultiRecordSize;
				if ((length == INVALID) || (length>=17000000))//Too big... bug fix hopefully (Abaddon 13 Sept 1999)
				{
					//ConOut("walking() - Bad length in multi file. Avoiding stall.\n");
					length = 0;
				}
				int j;
				for (j = 0; j < length; ++j)
				{
					st_multi multi;
					mfile->get_st_multi(&multi);
					if (multi.visible && (pi->getPosition("x")+multi.x == pc->getPosition("x")) && (pi->getPosition("y")+multi.y == pc->getPosition("y")))
					{
						tile_st tile;
						Map->SeekTile(multi.tile, &tile);
						xyblock[blockers_count].type=2;
						xyblock[blockers_count].basez= multi.z+pi->getPosition("z");
						xyblock[blockers_count].id= multi.tile;
						xyblock[blockers_count].flag1= tile.flag1;
						xyblock[blockers_count].flag2= tile.flag2;
						xyblock[blockers_count].flag3= tile.flag3;
						xyblock[blockers_count].flag4= tile.flag4;
						xyblock[blockers_count].height= tile.height;
						xyblock[blockers_count].weight= 255;
						++blockers_count;
					}
				}
			}
		}
	}

    MapStaticIterator msi( pc->getPosition("x"), pc->getPosition("y") );
	staticrecord *stat;
	int loopexit=0;
	while ( ((stat = msi.Next())!=NULL)  && (++loopexit < MAXLOOPS))
	{
		//ConOut("staticr[X] type=%d, id=%d\n", 2, stat->itemid);
		tile_st tile;
		msi.GetTile(&tile);
		xyblock[blockers_count].type= 2;
		xyblock[blockers_count].basez= stat->zoff;
		xyblock[blockers_count].id= stat->itemid;
		xyblock[blockers_count].flag1= tile.flag1;
		xyblock[blockers_count].flag2= tile.flag2;
		xyblock[blockers_count].flag3= tile.flag3;
		xyblock[blockers_count].flag4= tile.flag4;
		xyblock[blockers_count].height= tile.height;
		xyblock[blockers_count].weight= 255;
		++blockers_count;
	}
	return blockers_count;


}

///////////////
// Name:	WalkEvaluateBlockers
// history:	cut from walking() by Duke, 20.11.2000
// Purpose:	Decides if something in the array blocks the walker
//
void WalkEvaluateBlockers(P_CHAR pc, SI08 *pz, SI08 *pdispz, UI32 blockers)
{
	VALIDATEPC(pc);
	
	if( blockers == 0 ) return;	// nothing to do since there is nothing on the way

	SI08 z, oldz, seekz,dispz = -128;
	int num;
	UI32 i;
	char gmbody;
	UI32 blockers_count= blockers;

	z=-128;

	Location pcpos= pc->getPosition();

	oldz= pcpos.z;

	if( pc->IsGM() || pc->dead ) // gms and ghosts can walk through doors
		gmbody=1;
	else
		gmbody=0;

	bool bIgnoreWetBit = (ServerScp::g_nWalkIgnoreWetBit!=0)&&(pc->npc == 0);

	int loopexit=0;
	do
	{
		seekz=127;
		num=INVALID;
		for ( i = 0; i < blockers_count; ++i )
		{
			if ((xyblock[i].basez+xyblock[i].height)<seekz)
			{
				num=i;
				seekz=xyblock[i].basez+xyblock[i].height;
			}
		}

		if (num==INVALID)
		{
#ifdef DEBUG
			ConOut("(walking) Error?\n");
#endif
			blockers_count= 0;

		}
		else
		{
			if (xyblock[num].type!=0)
			{
				if (xyblock[num].height==0) xyblock[num].height++;
				if ((!(xyblock[num].id==1))&&(xyblock[num].basez<=oldz+MaxZstep))
				{
					//TEMP REMOVE.. DONT DONT ERASE.. MONDAY I READD THIS			// ehm ... of which year ? o_O' 
					//if( xyblock[num].flag1&0x40 &&(xyblock[num].basez+xyblock[num].height<=oldz+MaxZstep) )
					//	z=-128;
					//else
					if ((!bIgnoreWetBit)&&(xyblock[num].flag1&0x80)&&(!((pc->IsGM())||(pc->dead)||((pc->nxwflags[0]&NCF0_WATERWALK)!=0))))
					{
						z=-128;
					}
					else
					{
						if (gmbody)
						{
							if ( ((xyblock[num].weight==255)&&(!(pc->priv2&1))) || (xyblock[num].type==2) )
								if ( (xyblock[num].weight==255) || (xyblock[num].type==2) )
								{
									if (xyblock[num].flag2&0x04) // is tile climbable
									{
										if (xyblock[num].basez<oldz+MaxZstep)
										{
											z=xyblock[num].basez+xyblock[num].height;
											dispz=xyblock[num].basez+(xyblock[num].height/2);

										}
										else
										{
											if ((pc->IsGM())||(pc->dead)||(xyblock[num].flag4&0x20)) // tile is door
											{
												dispz=z=xyblock[num].basez;
											}
											else
											{
												z=-128;
											}
										}
									}
									else
									{
										if (xyblock[num].basez+xyblock[num].height<oldz+MaxZstep)
										{
											dispz=z=xyblock[num].basez+xyblock[num].height;
										}
										else
										{
											if ((pc->IsGM())||(pc->dead)||(xyblock[num].flag4&0x20)) // tile is door
											{
												dispz=z=xyblock[num].basez;
											}
											else
											{
												z=-128;
											}
										}
									}
								}
						}
						else
						{
							if ((xyblock[num].flag1&0x40) && (!(pc->IsGM()))) // lb, castle walk bugfix without sideffects with very.
							{
								if ((pc->npc) && (xyblock[num].basez+xyblock[num].height>pcpos.z))
									z=-128; //xan -> doesn't work with small shop! :)
							}
							else
							{
								if ((xyblock[num].flag2&4)) // is tile climbable
								{
									if (xyblock[num].basez<oldz+MaxZstep)
									{
										z=xyblock[num].basez+xyblock[num].height;
										dispz=xyblock[num].basez+(xyblock[num].height/2);
									}
									else
									{
										if (pc->IsGM())
										{
											dispz=z=xyblock[num].basez;
										}
										else
										{
											z=-128;
										}
									}
								}
								else
								{
									if (xyblock[num].basez+xyblock[num].height<oldz+MaxZstep)
									{
										dispz=z=xyblock[num].basez+xyblock[num].height;
									}
									else
									{
										if (pc->IsGM())
										{
											dispz=z=xyblock[num].basez;
										}
										else
										{
											if ((xyblock[num].flag2&0x20)&&(pc->npc))
											{
												z=-128; //xan : can't understand this :)
											}
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				if (((xyblock[num].flag1&0x80)&&(xyblock[num].flag1&0x40))&&(!(pc->IsGM())))
				{
					if(pc->getSocket() == INVALID) {
						if((pc->nxwflags[0]&NCF0_WATERWALK)==0)
						{
							z=-128;
						}
						else {
							dispz=z=Map->Height( pc->getPosition() );
						}
					}

				}
				else
				{
					if ((z==-128)||(xyblock[num].basez+xyblock[num].height<oldz+MaxZstep))
					{
						dispz=z=xyblock[num].basez;
					}
				}
			}

			//if (xycount==0) break;
			if( blockers_count==0 ) break;
			memcpy(&xyblock[num],&xyblock[blockers_count-1],sizeof(unitile_st));

			--blockers_count;
		}
	}
	while ((blockers_count > 0)  && (++loopexit < MAXLOOPS) );

	*pz=z; *pdispz=dispz;

}

///////////////
// Name:	WalkHandleBlocking
// history:	cut from walking() by Duke, 27.10.2000
// Purpose:	Handles a 'real move' if the Char is not only changing direction
//

bool WalkHandleBlocking(P_CHAR pc, int sequence, int dir, int oldx, int oldy)
{
	VALIDATEPCR(pc, false);
	if (pc->npc)
		pc->setNpcMoveTime(); //reset move timer

	Location pcpos= pc->getPosition();

	switch(dir&0x0F)
	{
		case 0: pc->setPosition("y", pcpos.y-1);
			break;
		case 1: { pc->setPosition("x", pcpos.x+1); pc->setPosition("y", pcpos.y-1); }
			break;
		case 2: pc->setPosition("x", pcpos.x+1);
			break;
		case 3: { pc->setPosition("x", pcpos.x+1); pc->setPosition("y", pcpos.y+1);}
			break;
		case 4: pc->setPosition("y", pcpos.y+1);
			break;
		case 5: { pc->setPosition("x", pcpos.x-1); pc->setPosition("y", pcpos.y+1);}
			break;
		case 6: pc->setPosition("x", pcpos.x-1);
			break;
		case 7: { pc->setPosition("x", pcpos.x-1); pc->setPosition("y", pcpos.y-1);}
			break;
		default:
			ErrOut("Switch fallout. walking.cpp, walking()\n"); //Morrolan
			ErrOut("\tcaused by chr %s. dir: %i dir&0x0f: %i dir-passed : %i dp&0x0f : %i\n", pc->getCurrentNameC(), pc->dir, pc->dir&0x0f, dir, dir&0x0f);
			if (pc->getSocket() != INVALID) deny(pc->getSocket(), pc, sequence); // lb, crashfix
			return false;
	}

	UI32 blockers = WalkCollectBlockers(pc);

	SI08 z, dispz=0;

	WalkEvaluateBlockers(pc, &z, &dispz, blockers);

	// check if player is banned from a house - crackerjack 8/12/99
	int j;

	if (pc->npc==0) // this is also called for npcs .. LB ?????? Sparhawk Not if you're excluding npc's
	{
		P_ITEM pi_multi=findmulti( pc->getPosition() );
		
		if((!ISVALIDPI(pi_multi))&&(pc->getMultiSerial32() != INVALID))
		{
			pc->setMultiSerial(INVALID); // Elcabesa bug-fix  we MUST use setmultiserial  NOT pc->multis = -1;
			//xan : probably the plr has exited the boat walking!
			//pc->multi1 = pc->multi2 = pc->multi3 = pc->multi4 = 0xFF;
			pc->setMultiSerial32Only(-1);
		}

		if(ISVALIDPI(pi_multi))
		{
			if (pc->getMultiSerial32() < 0)
			{
				//xan : probably the plr has entered the boat walking!
				pc->setMultiSerial32Only(INVALID);
				P_ITEM boat = boats::GetBoat(pc->getPosition());
				if (boat!=NULL) {
					pc->setMultiSerial( boat->getSerial32() );
					
					NxwCharWrapper pets;
					pets.fillOwnedNpcs( pc, false, true );
					for( pets.rewind(); !pets.isEmpty(); pets++ ) {
					   
						P_CHAR pc_b=pets.getChar();
						if(ISVALIDPC(pc_b)) {
							pc->MoveTo( boat->getPosition("x")+1, boat->getPosition("y")+1, boat->getPosition("z")+2 );
							pc->setMultiSerial( boat->getSerial32() );
							pc_b->teleport();
						}
					}
				}
			}

			if ( ISVALIDPI(pi_multi) && (pi_multi->IsHouse()) )
			{
				int sx, sy, ex, ey;
				j=on_hlist(pi_multi, pc->getSerial().ser1, pc->getSerial().ser2, pc->getSerial().ser3, pc->getSerial().ser4, NULL);

				if(j==H_BAN)
				{
					Map->MultiArea(pi_multi,&sx,&sy,&ex,&ey);
					pc->sysmsg(TRANSLATE("You are banned from that location."));
					Location pcpos= pc->getPosition();
					pcpos.x= ex;
					pcpos.y= ey+1;
					pc->setPosition( pcpos );
					pc->teleport();
					return false;
				}

				// house refreshment code moved to dooruse()

			} // end of is_house
		} // end of is_multi
	} // end of is player

	if ( z == -128 )
	{
		Location pcpos= pc->getPosition();
		pcpos.x= oldx;
		pcpos.y= oldy;
		pc->setPosition( pcpos );
		NXWSOCKET socket = pc->getSocket();
		if ( socket != INVALID )
			deny( socket, pc, sequence );
		else
			pc->blocked = 1;
		return false;
	}

	int nowx2,nowy2;

	//Char mapRegions
	pcpos= pc->getPosition();

	nowx2= pcpos.x;
	nowy2= pcpos.y;
	/*
	pc->x= oldx;
	pc->y= oldy; // we have to remove it with OLD x,y ... LB, very important
	pc->MoveTo(nowx2,nowy2,z);
	*/
	pc->setPosition("x", oldx);
	pc->setPosition("y", oldy);
	pc->MoveTo( nowx2, nowy2, z );
	return true;
}


void WalkingHandleRainSnow(P_CHAR pc)
{
	VALIDATEPC(pc);
	NXWSOCKET s = pc->getSocket();

	int i;
	int wtype = region[pc->region].wtype;

  /********* LB's no rain & snow in buildings stuff ***********/
	if (!pc->npc && pc->IsOnline() && wtype!=0 ) // check for being in buildings (for weather) only for PC's, check only neccasairy if it rains or snows ...
	{
		int j=indungeon(pc); // dung-check
		i=Map->StaticTop( pc->getPosition() ); // static check

	// dynamics-check
		int x=Map->DynamicElevation( pc->getPosition() );
		if (x!=-127) if (boats::GetBoat(pc->getPosition())!=NULL) x=-127; // check for dynamic buildings except boats
		if (x==1 || x==0) x=-127; // 1 seems to be the multi-borders
	// bugfix LB

		int kk=noweather[s];
		if (j || i || x!=-127 )
			noweather[s]=1;
		else
			noweather[s]=0; // no rain & snow in static buildings+dungeons;
		if (kk-noweather[s]!=0)
			weather(s, 0); // iff outside-inside changes resend weather ...
	// needs to be de-rem'd if weather is available again
  }
}


/*void WalkingHandleGlowingItems(P_CHAR pc)
{
	VALIDATEPC(pc);
	
	int i;
	if (pc->IsOnline())
	{
		int serial,serhash,ci;
		serial=pc->getSerial32();
		serhash=serial%HASHMAX;
		for (ci=0;ci<glowsp[serhash].max;ci++)
		{
			i=glowsp[serhash].pointer[ci];
			if (i!=INVALID)
			{
				if (items[i].free==0)
				{
					pc->glowHalo(&items[i]);
				}
			}
		}
	}

}*/


void walking(P_CHAR pc, int dir, int sequence)
{
	int newx, newy;
	VALIDATEPC( pc );

	NXWSOCKET  s = pc->getSocket();

	if (!WalkHandleAllowance(pc,sequence))		// check sequence, frozen, weight etc.
		return;

	WalkHandleRunning(pc,dir);

	int oldx= pc->getPosition().x;
	int oldy= pc->getPosition().y;

	if ((dir&0x0F)==pc->dir )
		if( !WalkHandleBlocking(pc,sequence,dir, oldx, oldy) )
			return;

	WalkingHandleHiding(pc,dir);

	if (s!=INVALID)
	{
		cPacketWalkAck walkok;
		walkok.sequence=buffer[s][2];
		walkok.notoriety=0x41;
		//if (pc->hidden==1) walkok.notoriety=0x00;
		//if (pc->isHidden()) walkok.notoriety=0x00;
		walkok.send( pc->getClient() );
		
		walksequence[s]=sequence;
		walksequence[s]%=255;
	}


	newx= pc->getPosition().x;
	newy= pc->getPosition().y;

	sendToPlayers( pc, dir );

	if (dir>INVALID && (dir&0x0F)<8)
		pc->dir=(dir&0x0F);
	else
		ConOut("dir-screwed : %i\n",dir);


	if( oldx!=newx || oldy!=newy ) 
	{
		//Luxor: moved WalkHandleItemsAtNewPos before socket check.
		handleItemsAtNewPos( pc, oldx, oldy, newx, newy );
		if (s!=INVALID)
		{
			handleCharsAtNewPos( pc );
			pc->LastMoveTime = uiCurrentTime;
		}

		if( !pc->npc || pc->questType || pc->tamed )
			objTeleporters(pc);
		teleporters(pc);

		if( s!=INVALID )
			WalkingHandleRainSnow(pc); // while rain and snow are disabled its a waste of CPU cycles

		magic::checkGateCollision( pc ); // Luxor: gates :)
		checkregion(pc);
	}


	if(pc->getCombatSkill() ==ARCHERY)  // -Frazurbluu- add in changes for archery skill, and dexterity
    {                                        //  possibly weapon speed?? maybe not, cause crossbows notta running shooting
		if ( pc->targserial!= INVALID)
        {
            if( pc->timeout>= uiCurrentTime)
               pc->timeout= uiCurrentTime + (3*CLOCKS_PER_SEC);

		}
    }

} 



void walking2(P_CHAR pc_s) // Only for switching to combat mode
{
	VALIDATEPC(pc_s);
	int sendit;

	Location charpos= pc_s->getPosition();

	NxwSocketWrapper sw;
	sw.fillOnline( pc_s, false );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWCLIENT ps_i=sw.getClient();
		if( ps_i==NULL ) continue;

		P_CHAR pc_i=ps_i->currChar();
		if (ISVALIDPC(pc_i) )
		{
			{
				if ((pc_s->IsHidden() || (pc_s->dead && !pc_s->war)) && pc_s->getSerial32() != pc_i->getSerial32() && !pc_i->IsGM())
				{
					if (!pc_i->dead)
					{
						SendDeleteObjectPkt(ps_i->toInt(), pc_s->getSerial32());
					 	sendit = 0;
					}
					else
						sendit = 1;
				}
				else
					sendit = 1; // LB 9-12-99 client 1.26.2 fix

				if (sendit)
				{
					NXWSOCKET s = ps_i->toInt();
					UI08 dir = pc_s->dir & 0x7F, flag, hi_color;

					// running stuff

					if (pc_s->npc && pc_s->war) // Skyfire
					{
						dir |= 0x80;
					}
					if (pc_s->npc && pc_s->ftargserial!=INVALID)
					{
						dir |= 0x80;
					}


					if (pc_s->war)
						flag = 0x40;
					else
						flag = 0x00;
					if (pc_s->IsHidden())
						flag |= 0x80;
					if (pc_s->poisoned)
						flag |= 0x04; // AntiChrist -- thnx to SpaceDog

					if (pc_s->kills >= 4)
						hi_color = 6; // ripper

					int guild;
					guild = Guilds->Compare(pc_s, pc_i);
					if (guild == 1)// Same guild (Green)
						hi_color = 2;
					else if (guild == 2) // Enemy guild.. set to orange
						hi_color = 5;
					else if (pc_s->IsMurderer())      // show red
						hi_color = 6;
					else if (pc_s->IsInnocent()) // show blue
						hi_color = 1;
					else if (pc_s->flag == 0x08) // show green
						hi_color = 2;
					else if (pc_s->flag == 0x10) // show orange
						hi_color = 5;
					else
						hi_color = 3;            // show grey

					// end of if sendit

					if (!pc_s->war) // we have to execute this no matter if invisble or not LB
					{
						pc_s->attackerserial=INVALID;
						pc_s->targserial=INVALID;
					}

					SendUpdatePlayerPkt(s, pc_s->getSerial32(), pc_s->GetBodyType(), charpos, dir, pc_s->getSkinColor(), flag, hi_color);
				}
			}
		}
	}
}
//</XAN>
int npcSelectDir(P_CHAR pc_i, int j)
{
	VALIDATEPCR(pc_i, -1);
	if (pc_i->blocked)
	{
		short x=0;
		if (j/2.0!=j/2)
			x=1;
		if (pc_i->blocked<=2)
			j =pc_i->dir2;	/* =(j-2-x)%8; //works better  ????*/
		else
		{
			if (rand()%2) j=pc_i->dir2=(j-2-x)%8;
			else j=pc_i->dir2=(j+2+x)%8;
		}
	}
	if (j<0)
		j=rand()%8;
	return j;
}

int npcSelectDirWarOld(P_CHAR pc_i, int j)
{
	VALIDATEPCR(pc_i, -1);
	if (pc_i->blocked)
	{
		short x=0;
		if (j/2.0!=j/2)
			x=1;
		if (pc_i->blocked<=2)
			j =pc_i->dir2 =(j-2-x)%8; //works better  ????
		else
		{
			if (rand()%2) j=pc_i->dir2=(j-2-x)%8;
			else j=pc_i->dir2=(j+2+x)%8;
		}
	}
	if (j<0)
		j=rand()%8;
	return j;
}

/*!
\author Luxor
\brief Calls the pathfinding algorithm and creates a new path
*/
void cChar::pathFind( Location pos, LOGICAL bOverrideCurrentPath )
{
	if ( hasPath() && !bOverrideCurrentPath )
		return;

	if ( hasPath() )
		safedelete( path );

        LOGICAL bOk = true;
	Location loc = pos;
	if ( isWalkable( pos ) == illegal_z ) { // If it isn't walkable, we can only reach the nearest tile
		bOk = false;
		for ( UI32 i = 1; i < 4; i++ ) {
                        // East
			loc = Loc( pos.x + i, pos.y, pos.z );
			if ( isWalkable( loc ) != illegal_z ) {
				bOk = true;
				break;
			}
			
			// West
			loc = Loc( pos.x - i, pos.y, pos.z );
			if ( isWalkable( loc ) != illegal_z ) {
				bOk = true;
				break;
			}
                        
			// South
			loc = Loc( pos.x, pos.y + i, pos.z );
			if ( isWalkable( loc ) != illegal_z ) {
				bOk = true;
				break;
			}

			// North
			loc = Loc( pos.x, pos.y - i, pos.z );
			if ( isWalkable( loc ) != illegal_z ) {
				bOk = true;
				break;
			}

			// North-East
			loc = Loc( pos.x + i, pos.y - i, pos.z );
			if ( isWalkable( loc ) != illegal_z ) {
				bOk = true;
				break;
			}

			// North-West
			loc = Loc( pos.x - i, pos.y - i, pos.z );
			if ( isWalkable( loc ) != illegal_z ) {
				bOk = true;
				break;
			}

			// South-East
			loc = Loc( pos.x + i, pos.y + i, pos.z );
			if ( isWalkable( loc ) != illegal_z ) {
				bOk = true;
				break;
			}

			// South-West
			loc = Loc( pos.x - i, pos.y + i, pos.z );
			if ( isWalkable( loc ) != illegal_z ) {
				bOk = true;
				break;
			}
		}
	}

        if ( bOk )
		path = new cPath( getPosition(), loc );
}

/*!
\author Luxor
*/
void cChar::walkNextStep()
{
	if ( IsFrozen() )
		return;
	if ( !hasPath() )
		return;
	Location pos = path->getNextPos();

	if ( pos == getPosition() )
		return;

	if ( /*path->targetReached() ||*/ isWalkable( pos ) == illegal_z ) {
                safedelete( path );
		return;
	}


        // <LB> Flying creatures stuff
	SI32 b = GetBodyType();
	if ( b < 0 || b > 2047 )
		b = 0;
	if ( ( creatures[b].who_am_i ) & 0x1 ) { // Can it fly?
		if ( fly_steps > 0 ) {
			if ( chance( 33 ) )
				playAction( 0x13 ); // Flying animation
		}
	}
	// </LB>

	Location oldpos = getPosition();
        SI08 dirXY = getDirFromXY( this, pos.x, pos.y );
        dir = dirXY & 0x0F;
	MoveTo( pos );
        sendToPlayers( this, dirXY );
	setNpcMoveTime();
}

/*!
\author Luxor
*/
void cChar::follow( P_CHAR pc )
{
	if ( IsFrozen() ) {
		if ( hasPath() )
			safedelete( path );
		return;
	}
	if ( UI32(dist( getPosition(), pc->getPosition() )) <= 1 ) { // Target reached
		if ( hasPath() )
			safedelete( path );
		facexy( pc->getPosition().x, pc->getPosition().y );
		return;
	}
	if ( !hasPath() || path->targetReached() ) { // We haven't got a right path, call the pathfinding.
		pathFind( pc->getPosition(), true );
		walkNextStep();
		return;
	}

	R64 distance = dist( path->getFinalPos(), pc->getPosition() );
	if ( distance <= 3.0 ) { // Path finalPos is pretty near... let's not overhead the processor
		walkNextStep();
	} else { // Path finalPos is too far, call the pathfinding.
		pathFind( pc->getPosition(), true );
		walkNextStep();
	}
}

/*!
\author Luxor
*/
void cChar::walk()
{
	P_CHAR pc_att = pointers::findCharBySerial( attackerserial );
	if ( !ISVALIDPC( pc_att ) )
		pc_att = pointers::findCharBySerial( targserial );
	if ( !ISVALIDPC( pc_att ) )
		war = 0;

	if ( war && npcWander != 5 && ( pc_att->IsOnline() || pc_att->npc ) ) { //We are following a combat target
                follow( pc_att );
                return;
        }        

	switch( npcWander )
	{
		case 0: //No movement
			break;
		case 1: //Follow the follow target
		{
			P_CHAR pc = pointers::findCharBySerial( ftargserial );
			if ( !ISVALIDPC( pc ) )
				break;
			if ( pc->dead )
				break;
			if ( pc->questDestRegion == region )
				MsgBoards::MsgBoardQuestEscortArrive( this, pc );
			follow( pc );
		}
			break;
		case 2: // Wander freely, in a defined circle
			npcwalk( this, (chance( 20 ) ? rand()%8 : dir), 2 );
			break;
		case 3: // Wander freely, within a defined box
			npcwalk( this, (chance( 20 ) ? rand()%8 : dir), 1 );
			break;
		case 4: // Wander freely, avoiding obstacles
			npcwalk( this, (chance( 20 ) ? rand()%8 : dir), 0 );
			break;
		case 5: //FLEE!!!!!!
		{
			P_CHAR target = pointers::findCharBySerial( targserial );
			if (ISVALIDPC(target)) {
				if ( distFrom( target ) < VISRANGE )
					getDirFromXY( this, target->getPosition().x, target->getPosition().y );
				npcwalk( this, npcSelectDir( this, (  getDirFromXY( this, target->getPosition().x, target->getPosition().y ) +4 )%8 )%8,0);
			}
		}
			break;
		case 6: // Sparhawk: script controlled movement
		{
			UI32 l = dir;
			if (amxevents[EVENT_CHR_ONWALK])
			{
				g_bByPass = false;
				amxevents[EVENT_CHR_ONWALK]->Call(getSerial32(), dir, dir);
				if (g_bByPass==true)
					return;
			}
			/*
			pc_i->runAmxEvent( EVENT_CHR_ONWALK, pc_i->getSerial32(), pc_i->dir, pc_i->dir);
			if (g_bByPass==true)
				return;
			*/
			int k = dir;
			dir = l;
			l = npcmovetime;
			npcwalk( this, k, 0);
			if ( l != npcmovetime ) // it's been changed through small
				return;
		}
			break;
		default:
			ErrOut("cChar::walk() unknown npcwander [%i] serial %u\n", npcWander, getSerial32() );
			break;
	}
	setNpcMoveTime();
}

int checkBounds(P_CHAR pc, int newX, int newY, int type)
{
	VALIDATEPCR(pc, 0);
	int move=0;
	switch (type)
	{
	case 0: move=1;break;
	case 1: move=checkBoundingBox(newX, newY, pc->fx1, pc->fy1, pc->fz1, pc->fx2, pc->fy2);break;
	case 2: move=checkBoundingCircle(newX, newY, pc->fx1, pc->fy1, pc->fz1, pc->fx2);break;
	default: move=0;	// invalid type given
	}
	return move;
}

void npcwalk( P_CHAR pc_i, int newDirection, int type)   //type is npcwalk mode (0 for normal, 1 for box, 2 for circle) // Sparhawk should be changed to npcwander
{
	VALIDATEPC(pc_i);

	if (pc_i->priv2 & CHRPRIV2_FROZEN) return;

	Location charpos= pc_i->getPosition();

	/////////// LB's flying creatures stuff, flying animation if they stand still ///////

	int b = pc_i->GetBodyType();
	if ( b < 0 || b > 2047 )
		b = 0;
	if ( ( creatures[b].who_am_i ) & 0x1 ) // can it fly ?
	{
		if ( pc_i->fly_steps > 0 )
		{
			if ( chance( 33 ) )
				pc_i->playAction(0x13); // flying animation
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////

	bool valid, move;
	if ( pc_i->dir == newDirection )  // If we're moving, not changing direction
	{
		int newX = charpos.x;
		int newY = charpos.y;
		getXYfromDir( pc_i->dir, &newX, &newY );	// get coords of the location we want to walk
                //<Luxor>
		Location newpos = Loc( newX, newY, charpos.z );
		valid = ( isWalkable( newpos ) != illegal_z );
		//</Luxor>
		if ( valid )
		{
			move = checkBounds( pc_i, newX, newY, type );
			if ( move )
			{
				walking(  pc_i , newDirection, 256 );
			}
			else 	// We're out of the boundary, so we need to get back
			{
				int direction = getDirFromXY( pc_i, pc_i->fx1, pc_i->fy1 );
				getXYfromDir( direction, &newX, &newY );
				//<Luxor>
				newpos = Loc( newX, newY, charpos.z );
				valid = ( isWalkable( newpos ) != illegal_z );
				//</Luxor>
				if ( !valid ) // try to bounce around obstacle
				{
					direction = pc_i->dir;
					getXYfromDir( pc_i->dir, &newX, &newY );
					//<Luxor>
					newpos = Loc( newX, newY, charpos.z );
					valid = ( isWalkable( newpos ) != illegal_z );
					//</Luxor>
					bool clockwise = chance( 50 );
					while( direction != pc_i->dir && !valid )
					{
						if ( clockwise )
							direction = getRightDir( direction );
						else
							direction = getLeftDir( direction );
						getXYfromDir( pc_i->dir, &newX, &newY );
						//<Luxor>
						newpos = Loc( newX, newY, charpos.z );
						valid = ( isWalkable( newpos ) != illegal_z );
						//</Luxor>
					}
				}
				if ( valid )
				{
					move = true;
					walking(  pc_i , direction, 256 );
				}
			}
		}
		else	// avoid obstacle
		{
			int direction;
			bool clockwise = chance( 50 );
			if ( clockwise )
				direction = getRightDir( pc_i->dir );
			else
				direction = getLeftDir( pc_i->dir );
			while( !valid && direction != pc_i->dir )
			{
				getXYfromDir( direction, &newX, &newY );
				//<Luxor>
				newpos = Loc( newX, newY, charpos.z );
				valid = ( isWalkable( newpos ) != illegal_z );
				//</Luxor>
				if ( clockwise )
					direction = getRightDir( direction );
				else
					direction = getLeftDir( direction );
			}
			if ( valid )
			{
				move = true;
				walking(  pc_i , direction, 256 );
			}
		}

		
		if ( (!valid || !move) && pc_i->amxevents[EVENT_CHR_ONBLOCK] )
		{
			g_bByPass = false;
			pc_i->amxevents[EVENT_CHR_ONBLOCK]->Call( pc_i->getSerial32(), newX, newY, charpos.z);
			if (g_bByPass==true)
				return;
		}
		/*
		if ( (!valid || !move) && pc_i->getAmxEvent( EVENT_CHR_ONBLOCK ) )
		{
			pc_i->runAmxEvent( EVENT_CHR_ONBLOCK, pc_i->getSerial32(), newX, newY, charpos.z);
			if (g_bByPass==true)
				return;
		}
		*/
	}
	else	// Change direction
	{
		walking(  pc_i , newDirection, 256);
	}
}


//namespace walking {

/*!
\author Luxor
*/
void handleCharsAtNewPos( P_CHAR pc )
{
	VALIDATEPC( pc );

	NxwCharWrapper sc;
	P_CHAR pc_curr;
	sc.fillCharsAtXY( pc->getPosition(), !pc->IsGM(), false );

	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		pc_curr = sc.getChar();
		if ( !ISVALIDPC( pc_curr ) )
			continue;
		if ( pc->IsGMorCounselor() || pc_curr->getSerial32() == pc->getSerial32() )
			continue;
		if ( pc_curr->dead || pc_curr->IsInvul() )
			continue;
		if ( pc_curr->IsHidden() )
			pc->sysmsg( TRANSLATE("You shoved something invisible aside.") );
		else
			pc->sysmsg( TRANSLATE("Being perfectly rested, you shove %s out of the way."), pc_curr->getCurrentNameC() );

		pc->stm = qmax( pc->stm-ServerScp::g_nShoveStmDamage, 0 );
		pc->updateStats( STAT_STAMINA );
		if ( pc->IsHidden() && !pc->IsHiddenBySpell() )
			pc->unHide(); //xan, shoving in stealth will unhide
	}
}

///////////////
// Name:	WalkHandleItemsAtNewPos
// history:	cut from walking() by Duke, 27.10.2000
// Purpose:	sends the newly visible items to the screen and checks for item effects
//
bool handleItemsAtNewPos(P_CHAR pc, int oldx, int oldy, int newx, int newy)
{
	VALIDATEPCR(pc, false);

	NXWCLIENT ps=pc->getClient();
	if ( ps == NULL ) //Luxor
		return false;

	Location pcpos=pc->getPosition();

	NxwItemWrapper si;
	si.fillItemsNearXYZ( pcpos, VISRANGE + 5, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pi=si.getItem();
		if(!ISVALIDPI(pi))
			continue;
			if( pi->id()>=0x407C && pi->id()<=0x407E )
			{
				int di= item_dist(pc, pi);

				if (di<=BUILDRANGE && di>=VISRANGE)
				{
					senditem(ps->toInt(), pi);
				}

			}
			else if ( pc->seeForFirstTime( P_OBJECT(pi) ) ) // Luxor
				senditem( ps->toInt(), pi );
	}
	return true;
}


/*!
\author Luxor
*/
void sendToPlayers( P_CHAR pc, SI08 dir )
{
	VALIDATEPC( pc );

	NXWCLIENT ps = NULL;
	NXWCLIENT cli = pc->getClient();
	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( pc->getPosition(), VISRANGE * 3, !pc->IsGM() );

	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pc_curr = sc.getChar();
		if( !ISVALIDPC(pc_curr) )
			continue;

		if ( pc->seeForLastTime( pc_curr ) ) {
			if ( cli != NULL )
				cli->sendRemoveObject( P_OBJECT(pc_curr) );
		}
		if ( pc->seeForFirstTime( pc_curr ) ) {
			if ( cli != NULL )
				impowncreate( cli->toInt(), pc_curr, 1);
		}

		ps = pc_curr->getClient();
		if ( ps == NULL )
			continue;

		// pc has just walked out pc_curr's vis circle
		if ( pc_curr->seeForLastTime( pc ) ) {
			ps->sendRemoveObject( P_OBJECT(pc) );
			continue;
		}

		// It's seen for the first time, send a draw packet
		if ( pc_curr->seeForFirstTime( P_OBJECT( pc ) ) ) {
			impowncreate( ps->toInt(), pc, 1 );
			continue;
		}

                if ( !pc_curr->IsGM() ) { // Players only
			if ( pc->IsHidden() ) // Hidden chars cannot be seen by Players
				return;
			if ( pc->dead && !pc->war && !pc_curr->dead ) // Non-persecuting ghosts can be seen only by other ghosts
				return;
		}

		NXWSOCKET socket = ps->toInt();
		UI08 flag, hi_color;

		// If it's an npc, and it's fighting or following something let's show it running
		if ( pc->npc && ( pc->war || pc->ftargserial != INVALID ) )
			dir |= 0x80;

		// <LB> Flying stuff for npcs
		if ( pc->npc && !(dir&0x80) ) { // If npc and it isn't already running
			SI32 d;
			UI16 skid = pc->GetBodyType();
			if ( skid > 2047 ) skid = 0;
			if ( (creatures[skid].who_am_i) & 0x1 ) { // If true, the npc can fly
				if ( pc->fly_steps > 0 ) {
					pc->fly_steps--;
					dir |= 0x80; // run mode = fly for that ones that can fly
				} else {
					if ( fly_p != 0 )
						d = rand() % fly_p;
					else
						d = 0;
					if ( d == 0 ) {
						if ( fly_steps_max != 0 )
							pc->fly_steps = ( rand() % fly_steps_max ) + 2;
						else
							pc->fly_steps += 2;
					}
				}
			}
		}
		// </LB>

		if ( pc->war )
			flag = 0x40;
		else
			flag = 0x00;
		if ( pc->IsHidden() )
			flag |= 0x80;
		if ( pc->dead && !pc->war )
			flag |= 0x80; // Ripper
		if ( pc->poisoned )
			flag |= 0x04; // AntiChrist -- thnx to SpaceDog

		SI32 guild = Guilds->Compare( pc, pc_curr );
		if ( guild == 1 )		// Same guild (Green)
			hi_color = 2;
		else if ( guild == 2 )		// Enemy guild.. set to orange
			hi_color = 5;
		else if ( pc->IsGrey() )
			hi_color = 3;           // grey
		else if ( pc->IsMurderer() )
			hi_color = 6;		// If a bad, show as red.
		else if ( pc->IsInnocent() )
			hi_color = 1;		// If a good, show as blue.
		else if ( pc->flag == 0x08 )
			hi_color = 2;		// green (guilds)
		else if ( pc->flag == 0x10 ) 
			hi_color = 5;		// orange (guilds)
		else
			hi_color = 3;		// grey

		SendUpdatePlayerPkt( ps->toInt(), pc->getSerial32(), pc->GetBodyType(), pc->getPosition(), dir, pc->getSkinColor(), flag, hi_color );
	}
}




//} //namespace walking
