  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file timing.cpp
\brief functions that handle the timer controlled stuff
\author Duke
\note not necessarily ALL those functions
*/

#include "nxwcommn.h"
#include "basics.h"
#include "sregions.h"
#include "sndpkg.h"
#include "srvparms.h"
#include "amx/amxcback.h"
#include "calendar.h"
#include "magic.h"
#include "race.h"
#include "tmpeff.h"
#include "debug.h"
#include "house.h"
#include "jail.h"
#include "timers.h"
#include "accounts.h"
#include "boats.h"
#include "spawn.h"
#include "trade.h"
#include "html.h"
#include "chars.h"
#include "items.h"
#include "inlines.h"

extern bool g_bMustExecAICode;

static SI32 linInterpolation (SI32 ix1, SI32 iy1, SI32 ix2, SI32 iy2, SI32 ix);

void checkFieldEffects( UI32 currenttime, P_CHAR pc, char timecheck )
{

	VALIDATEPC(pc);

	if ( (timecheck && !(nextfieldeffecttime<=currenttime)) ) //changed by Luxor
		return;
#ifdef SPAR_NEW_WR_SYSTEM
	pItemVectorIt itemIt( pc->nearbyItems->begin() ), itemEnd( pc->nearbyItems->end() );

	for( ; itemIt != itemEnd; ++itemIt ) {

		P_ITEM pi= (*itemIt);
#else
	NxwItemWrapper si;
	si.fillItemsNearXYZ( pc->getPosition(), 2, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
#endif
		if(ISVALIDPI(pi) ) {

			if ( pi->getPosition().x == pc->getPosition().x && pi->getPosition().y == pc->getPosition().y )

				//Luxor: added new field damage handling
				switch( pi->getId() )
				{
					case 0x3996:
					case 0x398C: //Fire Field
						if (!pc->resistsFire())
							tempfx::add(pc, pc, tempfx::FIELD_DAMAGE, SI32(pi->morex/100.0), DAMAGE_FIRE, 0, 1);
						return;
					case 0x3915:
					case 0x3920: //Poison Field
						if ((pi->morex<997)) {
							tempfx::add(pc, pc, tempfx::FIELD_DAMAGE, 2, DAMAGE_POISON, 0, 2);
							pc->applyPoison(POISON_WEAK);
						} else {
							tempfx::add(pc, pc, tempfx::FIELD_DAMAGE, 3, DAMAGE_POISON, 0, 2); // gm mages can cast greater poison field, LB
							pc->applyPoison(POISON_NORMAL);
						}
						return;
					case 0x3979:
					case 0x3967: //Para Field
						if (chance(50)) {
							tempfx::add(pc, pc, tempfx::SPELL_PARALYZE, 0, 0, 0, 3);
							pc->playSFX( 0x0204 );
						}
						return;
				}
		}
	}
}

void checktimers() // Check shutdown timers
{

	overflow = (lclock > uiCurrentTime);
	if (endtime)
	{
		if ( endtime <= uiCurrentTime ) keeprun=false;
	}
	lclock = uiCurrentTime;

}

void checkauto() // Check automatic/timer controlled stuff (Like fighting and regeneration)
{
//	static TIMERVAL checkspawnregions=0;
       	static TIMERVAL checktempfx=0;
	static TIMERVAL checknpcs=0;
	static TIMERVAL checktamednpcs=0;
	static TIMERVAL checknpcfollow=0;
	static TIMERVAL checkitemstime=0;
	static TIMERVAL lighttime=0;
	static TIMERVAL htmltime=0;
	static TIMERVAL housedecaytimer=0;

	LOGICAL lightChanged = false;

	//
	// Accounts
	//
	if (SrvParms->auto_a_reload > 0 && TIMEOUT( Accounts->lasttimecheck + (SrvParms->auto_a_reload*60*MY_CLOCKS_PER_SEC) ) )
		Accounts->CheckAccountFile();
	//
	// Weather (change is handled by crontab)
	//
	// Calendar
	//
	if ( TIMEOUT( uotickcount ) )
	{
		if (Calendar::advanceMinute())
			day++;
		uotickcount=uiCurrentTime+secondsperuominute*MY_CLOCKS_PER_SEC;
		if (Calendar::g_nMinute%8==0)
			moon1=(moon1+1)%8;
		if (Calendar::g_nMinute%3==0)
			moon2=(moon2+1)%8;
	}
	//
	// Light
	//
	if( TIMEOUT( lighttime ) )
	{
		SI32 lightLevel = worldcurlevel;

		SI32 timenow = (Calendar::g_nHour * 60) + Calendar::g_nMinute;
		SI32 dawntime = (Calendar::g_nCurDawnHour * 60) + Calendar::g_nCurDawnMin;
		SI32 sunsettime = (Calendar::g_nCurSunsetHour * 60) + Calendar::g_nCurSunsetMin;
		SI32 nighttime = qmin((sunsettime+120), (1439));
		SI32 morntime = qmax((dawntime-120), (0));
		SI32 const middaytime = 750;
//		SI32 const midnighttime = 0; // unused variable
		SI32 dawnlight = (((worlddarklevel - worldbrightlevel))/3) + worldbrightlevel;
		//
		// default lights at dawn and sunset
		//
		if ( timenow == dawntime || timenow==sunsettime )
			lightLevel = dawnlight;
		//
		// highest light at midday
		//
		else if( timenow == middaytime )
			lightLevel = qmax(worldbrightlevel-1, 0);
		//
		// darkest light during night
		//
		else if( timenow >= nighttime )
			lightLevel = worlddarklevel;
		//
		else if( timenow <= morntime )
			lightLevel = worlddarklevel;
		//
		// fading light slight before dawn
		//
		else if( timenow > morntime && timenow < dawntime )
			lightLevel = linInterpolation(morntime, worlddarklevel, dawntime, dawnlight, timenow);
		//
		// fading light slight from dawn to midday
		else if( timenow > dawntime &&  timenow < middaytime )
			lightLevel = linInterpolation(dawntime, dawnlight, middaytime, worldbrightlevel, timenow);
		//
		// fading light slight from midday to sunset
		//
		else if( timenow > middaytime && timenow < sunsettime )
			lightLevel = linInterpolation(middaytime, worldbrightlevel, sunsettime, dawnlight, timenow);
		//
		// fading light slight from sunset to night
		//
		else if( timenow > sunsettime && timenow < nighttime )
			lightLevel = linInterpolation(sunsettime, dawnlight, nighttime, worlddarklevel, timenow);

		if (wtype)
			lightLevel += 2;
		if (moon1+moon2<4)
			++lightLevel;
		if (moon1+moon2<10)
			++lightLevel;

		if (lightLevel != worldcurlevel)
		{
			worldcurlevel = lightLevel;
			lightChanged  = true;
		}
		lighttime=uiCurrentTime+secondsperuominute*5*MY_CLOCKS_PER_SEC;
	}

	//
	//	Housedecay and stabling
	//
	if ( TIMEOUT( housedecaytimer ) )
	{
		//////////////////////
		///// check_houses
		/////////////////////
		if( SrvParms->housedecay_secs != UINVALID )
			check_house_decay();
		housedecaytimer = uiCurrentTime+MY_CLOCKS_PER_SEC*60*60; // check only each hour
	}
	//
	// Spawns
	//
	if( TIMEOUT( Spawns->check ) )
	{
		Spawns->doSpawn();
	}

	//
	// Shoprestock
	//
	Restocks->doRestock();

	//
	// Prison release
	//
	prison::checkForFree();

	//
	// Temporary effects
	//
        if( TIMEOUT( checktempfx ) )
		tempfx::checktempeffects();

	//
	// Characters & items
	//
	NxwSocketWrapper sw;
	sw.fillOnline();

	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWCLIENT ps = sw.getClient();
		if( ps == NULL )
			continue;

		P_CHAR pc=ps->currChar();
		if( !ISVALIDPC( pc ) )
			continue;

		if( lightChanged )
			dolight(ps->toInt(),worldcurlevel);

		pc->heartbeat();

		if( TIMEOUT( checknpcs ) || TIMEOUT( checktamednpcs ) || TIMEOUT( checknpcfollow ) )
		{
#ifdef SPAR_C_LOCATION_MAP
			PCHAR_VECTOR *pCV = pointers::getNearbyChars( pc, VISRANGE, pointers::NPC );
			PCHAR_VECTOR it( pCV->begin() ), end( pCV->end() );
			P_CHAR pNpc = 0;
			while( it != end )
			{
				pNpc = (*it);
				if( pNpc->lastNpcCheck != uiCurrentTime &&
				    (TIMEOUT( checknpcs ) ||
				    (TIMEOUT( checktamednpcs ) && pNpc->tamed) ||
				    (TIMEOUT( checknpcfollow ) && pNpc->npcWander == WANDER_FOLLOW ) ) )
				{
					pNpc->heartbeat();
					pNpc->lastNpcCheck = uiCurrentTime;
				}
				++it;
			}
#else
			NxwCharWrapper sc;
			sc.fillCharsNearXYZ( pc->getPosition(), VISRANGE, true, false );
			for( sc.rewind(); !sc.isEmpty(); sc++ )
			{
				P_CHAR npc=sc.getChar();

				if(!ISVALIDPC(npc) || !npc->npc )
					continue;

				if( npc->lastNpcCheck != uiCurrentTime &&
				    (TIMEOUT( checknpcs ) ||
				    (TIMEOUT( checktamednpcs ) && npc->tamed) ||
				    (TIMEOUT( checknpcfollow ) && npc->npcWander == WANDER_FOLLOW ) ) )
				{
					npc->heartbeat();
					npc->lastNpcCheck = uiCurrentTime;
				}
			}
#endif
		}

		if( TIMEOUT( checkitemstime ) )
		{
			NxwItemWrapper si;
			si.fillItemsNearXYZ( pc->getPosition(), 2*VISRANGE, false );
			for( si.rewind(); !si.isEmpty(); si++ )
			{
				P_ITEM pi=si.getItem();

				if( !ISVALIDPI( pi ) )
					continue;

				pi->doDecay();

				switch( pi->type )
				{
					case  51	:
					case  52	:
						//if( TIMEOUT( pi->gatetime ) )
							//for (int k=0;k<2;++k)	Sparhawk what's this???? Let's comment it out for now
							//	pi->deleteItem(); // bugfix for items disappearing
							//pi->deleteItem();
						break;
					case  61    :
					case  62	:
					case  63	:
					case  64	:
					case  65	:
					case  69	:
					case 125	:
						break; //SPAWNERS may not decay!!! --> Sparhawk then don't use the decay tag in the script
					case  88	:
						if( pi->morey >= 0 && pi->morey < 25 )
							if (pc->distFrom(pi)<=pi->morey)
								if( (UI32)RandomNum(1,100) <= pi->morez )
									soundeffect4(ps->toInt(), pi, pi->morex);
						break;
					case 117	:	// Boats
						if( pi->type2 == 1 || pi->type2 == 2 )
							if( TIMEOUT( pi->gatetime ) )
							{
								if (pi->type2==1)
									Boats->Move(ps->toInt(),pi->dir,pi);
								else
								{
									int dir=pi->dir+4;
									dir%=8;
									Boats->Move(ps->toInt(),dir,pi);
								}
								pi->gatetime=(TIMERVAL)(uiCurrentTime + (R64)(SrvParms->boatspeed*MY_CLOCKS_PER_SEC));
							}
						break;
				}
			}
		}
	}//for i<now


	if( TIMEOUT( checkitemstime ) )
		checkitemstime = (TIMERVAL)((R64) uiCurrentTime+(speed.itemtime*MY_CLOCKS_PER_SEC));
	if( TIMEOUT( checknpcs ) )
		checknpcs = (TIMERVAL)((R64) uiCurrentTime+(speed.npctime*MY_CLOCKS_PER_SEC));
	if( TIMEOUT( checktamednpcs ) )
		checktamednpcs=(TIMERVAL)((R64) uiCurrentTime+(speed.tamednpctime*MY_CLOCKS_PER_SEC));
	if( TIMEOUT( checknpcfollow ) )
		checknpcfollow=(TIMERVAL)((R64) uiCurrentTime+(speed.npcfollowtime*MY_CLOCKS_PER_SEC));
	//
	// Html
	//
	if(SrvParms->html>0 && (htmltime<=uiCurrentTime ))
	{
		updatehtml();
		htmltime=uiCurrentTime+(SrvParms->html*MY_CLOCKS_PER_SEC);
	}
	//
	// Finish
	//
	if ( TIMEOUT( nextfieldeffecttime ) )
		nextfieldeffecttime = (TIMERVAL)((R64) uiCurrentTime + (0.5*MY_CLOCKS_PER_SEC));
	if ( TIMEOUT( nextdecaytime ) )
		nextdecaytime = uiCurrentTime + (15*MY_CLOCKS_PER_SEC);
        if( TIMEOUT( checktempfx ) )
		checktempfx = (TIMERVAL)((R64) uiCurrentTime+(0.5*MY_CLOCKS_PER_SEC));
}

static SI32 linInterpolation (SI32 ix1, SI32 iy1, SI32 ix2, SI32 iy2, SI32 ix)
{
	#define NSIN(X) ((static_cast<R32>(1.0+sin((2.0*X-1.0)*PI)))/2.0f)
	#define NLIN(X) (X)

	R32 x1 = static_cast<R32>(ix1);
	R32 x2 = static_cast<R32>(ix2);//reinterpret_cast
	R32 y1 = static_cast<R32>(iy1);
	R32 y2 = static_cast<R32>(iy2);
	R32  x = static_cast<R32>(ix);
	R32 X2 = x2 - x1;
	R32 Y2 = y2 - y1;
	R32  X =  x - x1;
	R32  Y = (NLIN((X/X2))*Y2)+y1;

	SI32 y = static_cast<SI32>(Y);

	return y;
}
