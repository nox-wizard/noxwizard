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
cTimer::cTimer(SERIAL serial, UI32 secs, UI08 flags, SI32 n, UI32 more1, UI32 more2)
{
	this->serial=serial;
	this->timer=uiCurrentTime+MY_CLOCKS_PER_SEC*secs;
	this->continued=n;
	this->more1=more1;
	this->more2=more2;
	this->flags=flags;
	this->amxevent=NULL;
	this->event=NULL;
	this->secs=secs;
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
	if( this->continued!=INVALID ) {
		this->continued--;		
	}

	this->setTimer(this->secs);

	if (this->amxevent!=NULL) {
		g_bByPass = false;
		this->amxevent->Call(this->serial, this->more1, this->more2);
		if (g_bByPass==true)
			return;
	}

	if(event!=NULL) {
		bool breakTimer=false;
		P_CHAR pc=pointers::findCharBySerial( this->serial );
		if(ISVALIDPC(pc)) 
			breakTimer=this->event(pc, this->more1, this->more2);
		else {
			P_ITEM pi=pointers::findItemBySerial( this->serial );
			if(ISVALIDPI(pi))
				breakTimer=this->event(pi, this->more1, this->more2);
			else //not serial of char.. not of item.. is invalid so destroy
				breakTimer=true;
		}
		if(breakTimer)
			this->continued=0; //will be destroyed after end of check
	}
};

/*!
\brief Check Timer
\author Endymion
\return true if this timer is to delete
*/
bool cTimer::checkTimer()
{
	if( TIMEOUT( this->timer ) ) {
		this->onTimer();
		if(this->continued==0)
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
	this->timer=uiCurrentTime+MY_CLOCKS_PER_SEC *secs;
}


/*!
\brief Check if timer can be paused
\author Endymion
\return true if can ben paused
*/
inline bool cTimer::canPause()
{
	return ( this->flags && TIMER_PAUSE );
}

/*!
\brief Check if timer must be saved
\author Endymion
\return true if must be saved
*/
inline bool cTimer::canSave()
{
	return ( this->flags && TIMER_SAVE );
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

