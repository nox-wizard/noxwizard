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
\brief Declaration of class cTimer and stuff related with it
*/


#ifndef __TIMERS_H
#define __TIMERS_H

#include "constants.h"
#include "typedefs.h"
#include <string>

using namespace std;

#define TIMER_PAUSE 0x01 //!< pause after logout of player ( only if playes timer )
#define TIMER_SAVE  0x02 //!< save on worldsave

typedef bool (*TIMER_EVENT)(cObject*, UI32&, UI32&);


/*!
\brief is a timer, not more not less
\author Endymion
\since 0.82
*/
class cTimer
{
public:
	cTimer(SERIAL serial, UI32 secs, UI08 flags=TIMER_PAUSE, SI32 n=INVALID, UI32 more1=0, UI32 more2=0 );
	virtual ~cTimer();

	UI32 timer;		//!< timer count
	SERIAL serial;	//!< serial of char or item
	UI32	secs;	//!< seconds

	TIMER_EVENT event;			//!< on timer is called this function ( if set )
	class AmxEvent* amxevent;	//!< on timer is called this function ( if set ) - SMALL VERSION

private:
	UI08 flags;		//!< bitfield ( TIMER_PAUSE, TIMER_SAVE )
public:
	inline bool canPause();
	inline bool canSave();
	SI32 continued;	//!< how much time do this ( INVALID is forever )
	UI32 more1;		//!< help variable ( small especially )
	UI32 more2;		//!< help variable ( small especially )

public:
	virtual void onTimer();
	bool checkTimer();
	void setTimer( UI32 secs );

};

typedef std::vector<cTimer> TIMERVECTOR;
typedef std::map<SERIAL, TIMERVECTOR > TIMERSMAP;

namespace timers {
	extern TIMERSMAP alltimers;		//!< all the timers currretly used
	extern TIMERSMAP pausedtimers;	//!< paused timer are stored here

	void addTimer( SERIAL serial, cTimer& timer );
	void checkAllTimers( );
	void pauseTimer( SERIAL serial );
	void resumeTimer( SERIAL serial );
}


#endif

