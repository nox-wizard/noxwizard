  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"

TIMERSMAP timers::alltimers;
TIMERSMAP timers::pausedtimers;

/*!
\brief Contructor
\author Endymion
*/
cTimer::cTimer(SERIAL newSerial, UI32 newSecs, UI08 newFlags, SI32 n, UI32 newMore1, UI32 newMore2)
{
	serial=newSerial;
	timer=uiCurrentTime+MY_CLOCKS_PER_SEC*secs;
	continued=n;
	more1=newMore1;
	more2=newMore2;
	flags=newFlags;
	amxevent=NULL;
	event=NULL;
	secs=newSecs;
};

/*!
\brief Destructor
\author Endymion
*/
cTimer::~cTimer()
{
};


/*!
\brief on timer
\author Endymion
*/
void cTimer::onTimer()
{
	if( continued!=INVALID ) {
		continued--;		
	}

	setTimer(secs);

	if (amxevent!=NULL) {
		g_bByPass = false;
		amxevent->Call(serial, more1, more2);
		if (g_bByPass==true)
			return;
	}

	if(event!=NULL) {
		bool breakTimer=false;
		P_CHAR pc=pointers::findCharBySerial( serial );
		if(ISVALIDPC(pc)) 
			breakTimer=event(pc, more1, more2);
		else {
			P_ITEM pi=pointers::findItemBySerial( serial );
			if(ISVALIDPI(pi))
				breakTimer=event(pi, more1, more2);
			else //not serial of char.. not of item.. is invalid so destroy
				breakTimer=true;
		}
		if(breakTimer)
			continued=0; //will be destroyed after end of check
	}
};

/*!
\brief Check Timer
\author Endymion
\return true if this timer is to delete
*/
bool cTimer::checkTimer()
{
	if( TIMEOUT( timer ) ) {
		onTimer();
		if(continued==0)
			return true;
	}
	return false;
};

/*!
\brief Check Timer
\author Endymion
\return true if this timer is to delete
\param secs the secs
*/
void cTimer::setTimer( UI32 secs )
{
	timer=uiCurrentTime+MY_CLOCKS_PER_SEC *secs;
}


/*!
\brief Check if timer can be paused
\author Endymion
\return true if can ben paused
*/
inline bool cTimer::canPause()
{
	return ( flags && TIMER_PAUSE );
}

/*!
\brief Check if timer must be saved
\author Endymion
\return true if must be saved
*/
inline bool cTimer::canSave()
{
	return ( flags && TIMER_SAVE );
}


/*!
\brief add a Timer to list
\author Endymion
*/
void timers::addTimer( SERIAL serial, cTimer& timer )
{
	alltimers[serial].push_back(timer);	
}

/*!
\brief check all timers
\author Endymion
*/
void timers::checkAllTimers( )
{
	TIMERSMAP::iterator prox;
	for( TIMERSMAP::iterator se = timers::alltimers.begin(); se!=timers::alltimers.end(); se=prox ) {
		prox=se; prox++;
		TIMERVECTOR::iterator next;
		for( TIMERVECTOR::iterator ti=se->second.begin(); ti!=se->second.end(); ti=next ) {
			next=ti; next++; //save of next
			if( (*ti).checkTimer() ) { //need to delete
				se->second.erase(ti);
			}
		}
		if( se->second.empty() ) { //no timer, so why have this? 
			timers::alltimers.erase( se );
		}
	}
}

/*!
\brief pause timer of given char 
\author Endymion
\note only timer with pause/resume are stored, other erased
\note a paused timer is moved on pausedtimers container
*/
void timers::pauseTimer( SERIAL serial )
{
	TIMERSMAP::iterator se = timers::alltimers.find(serial);
	if( se==alltimers.end() )
		return;
	TIMERVECTOR::iterator next;
	for( TIMERVECTOR::iterator ti=se->second.begin(); ti!=se->second.end(); ti=next ) {
		next=ti; next++; //save of next
		if( (*ti).canPause() )  //so i save it
			timers::pausedtimers[serial].push_back(*ti);
		se->second.erase(ti);
	}
}

/*!
\brief resume all paused timers of given char
\author Endymion
*/
void timers::resumeTimer( SERIAL serial )
{
	TIMERSMAP::iterator se = timers::pausedtimers.find(serial);
	if( se==timers::pausedtimers.end() )
		return;
	for( TIMERVECTOR::iterator ti=se->second.begin(); ti!=se->second.end(); ti++ ) 
		timers::alltimers[serial].push_back(*ti);
	
	timers::pausedtimers.erase(se);
}

