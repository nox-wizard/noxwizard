/*!
 ***********************************************************************************
 *  file    : amxcback.cpp
 *
 *  Project : Nox-Wizard
 *
 *  Author  : 
 *
 *  Purpose : Implementation of Functions for AMX Callbacks and AMX Events
 *
 ***********************************************************************************
 *//*
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW)		   [http://www.noxwizard.com]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums or mail staff@noxwizard.com  ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    CHANGELOG:
    -----------------------------------------------------------------------------
    DATE					DEVELOPER				DESCRIPTION
    -----------------------------------------------------------------------------

 ***********************************************************************************
 */
 
#include "nxwcommn.h"
#include "amxcback.h"
#include "sndpkg.h"
#include "itemid.h"
#include "items.h"
#include "chars.h"
#include "client.h"
#include "network.h"

extern int g_nCurrentSocket;
extern int g_nTriggeredItem;
extern int g_nTriggerType;

#define MAXSTRLEN 50

static int g_nType;


/*!
\brief wrap for check usability
\author Xanathar
\return bool
\param pc player trying using
\param pi pointer to item to be used
\param type type of usability
\remarks Luxor - Added REQSKILL command support, three bug fix applied 
*/
bool checkItemUsability(P_CHAR pc, P_ITEM pi, int type)
{
	g_nType = type;
	VALIDATEPIR(pi, false);
	VALIDATEPCR(pc, false);

	NXWSOCKET s = pc->getSocket();

	if( !pi->isNewbie() )
	{
		if ( pi->st > pc->getStrength() ) 
		{
			pc->sysmsg(TRANSLATE("You are not strong enough to use that."));
			return false;
		}
		if ( pi->dx > pc->dx )
		{
			pc->sysmsg(TRANSLATE("You are not quick enough to use that."));
			return false;
		}
		if ( pi->in > pc->in )
		{
			pc->sysmsg(TRANSLATE("You are not intelligent enough to use that."));
			return false;
		}
		//Luxor: REQSKILL command support
		if (pi->reqskill[0] > 0 && pi->reqskill[1] > 0 )
		{
			if (pi->reqskill[1] > pc->skill[pi->reqskill[0]]) {
				pc->sysmsg(TRANSLATE("You are not skilled enough to use that."));
				return false;
			}
		}
	}

	if (s >-1 && s < now) //Luxor
	{
		
		if (pi->amxevents[EVENT_IONCHECKCANUSE]==NULL) return true;
		return (0!=pi->amxevents[EVENT_IONCHECKCANUSE]->Call(pi->getSerial32(), s, g_nType));
		/*
		AmxEvent* event = pi->getAmxEvent( EVENT_IONCHECKCANUSE );
		if ( !event ) return true;
		return ( 0 != event->Call(pi->getSerial32(), s, g_nType ) );
		*/
	}
	return true;
}


/*!
\brief Calls an amx event handler
\author Xanathar
\return cell
\param param1 parameter passed to amx callback fn
\param param2 parameter passed to amx callback fm
\param param3 parameter passed to amx callback fn
\param param4 parameter passed to amx callback fn
*/
cell AmxEvent::Call (int param1, int param2, int param3, int param4)
{ 
	g_nCurrentSocket = g_nTriggeredItem = g_nTriggerType = -1;
	if (valid) return AmxFunction::g_prgOverride->CallFn(function, param1, param2, param3, param4); 
	else return -1;
}

/*!
\brief Gets the fn name of an handler
\author Xanathar
\return char*
*/
char* AmxEvent::getFuncName (void) 
{ return funcname; }

/*!
\brief constructor for event handlers
\author Xanathar
\param fnname function name
\param dyn is dynamic ? (dynamic means : saved on worldsave)
*/
AmxEvent::AmxEvent(char *fnname, bool dyn)
{
	dynamic = dyn;
	funcname = new char[strlen(fnname)+3];
	strcpy(funcname, fnname);
	function = AmxFunction::g_prgOverride->getFnOrdinal(funcname);
	if (function <= -3) valid = false; else valid = true;
}
	

/*!
\brief quick constructor for dynamics
\author Xanathar
\param funidx index to amx function
*/
AmxEvent::AmxEvent(int funidx)
{
	dynamic = true;
	funcname = new char[strlen("%Dynamic-Callback%")+3];
	strcpy(funcname, "%Dynamic-Callback%");
	function = funidx;
	if (function <= -3) valid = false; else valid = true;
}


static AmxEvent *HashQueue[256];
static AmxEvent *Queue = NULL;


/*!
\brief initializes to nulls the hash queues for amx events
\author Xanathar
*/
void initAmxEvents(void)
{ 
	ConOut("Initializing event callback hash queue...");
	for (int i=0; i<256; i++) HashQueue[i] = NULL; 
	ConOut("[DONE]");
}



/*!
\brief creates a new amx event or load a previous equivalent one
\author Xanathar
\return AmxEvent* 
\param funcname name of function
\param dynamic dynamic/static status
*/
AmxEvent* newAmxEvent(char *funcname, bool dynamic)
{
	int i;
	int hash = 0;
	int ln = strlen(funcname);

	for (i=0; i<ln; i++) hash += funcname[i];

	hash &= 0xFF;

	AmxEvent *p = HashQueue[hash];

	while(p!=NULL) 
	{
		if (!strcmp(p->getFuncName(), funcname)){
			if (p->shouldBeSaved()!=dynamic) return p;
		}
		p = p->hashNext;
	}

	p = new AmxEvent(funcname, dynamic);
	
	p->listNext = Queue;
	Queue = p;
	
	p->hashNext = HashQueue[hash];
	HashQueue[hash] = p;
	
	return p;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//// TIMER CALLBACKS 
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#define MAXTIMERS 64

struct _timevent_st {
	int callback;
	int timer;
	int interval;
	int more1;
	int more2;
};

static _timevent_st _timevent[MAXTIMERS];


/*!
\brief Initializes amx timers
\author Xanathar
*/
void initTimers (void)
{
	ConOut("Initializing timer callback array [max %d timers]...", MAXTIMERS);
	for (int i=0; i<MAXTIMERS; i++) _timevent[i].callback = -1;
	ConOut("[DONE]\n");
}


/*!
\brief adds an amx timer
\author Xanathar
\return int 
\param cback amx callback function (as returned by funcidx())
\param time interval in milliseconds
\param more1 a custom value for this timer
\param more2 a custom value for this timer
*/
int addTimer (int cback, int time, int more1, int more2)
{
	for (int i=0; i<MAXTIMERS; i++) {
		if (_timevent[i].callback==-1) {
			_timevent[i].callback = cback;
			_timevent[i].timer = uiCurrentTime+time;
			_timevent[i].interval = time;
			_timevent[i].more1 = more1;
			_timevent[i].more2 = more2;
			return 0;
		}
	}
	ConOut("Maximum Timer number exceeded!\n");
	return -1;
}



/*!
\brief checks amx timers for events
\author Xanathar
*/
void checkTimer (void)
{
	int i;
	
	for (i=0; i<MAXTIMERS; i++) {
		if ((_timevent[i].callback>=0)&&(((UI32)(_timevent[i].timer)<=uiCurrentTime)||(overflow))) {
						_timevent[i].interval = 
							AmxFunction::g_prgOverride->CallFn(_timevent[i].callback, _timevent[i].interval, uiCurrentTime, 
							_timevent[i].more1, _timevent[i].more2);
						_timevent[i].timer = uiCurrentTime+_timevent[i].interval;
						if (_timevent[i].interval == 0) _timevent[i].callback = -1;
		}
	}
}
		


/*!
\brief fires an amx target
\author Xanathar
\return int 
\param s socket to send the target to
\param callback callback fn to be called (as returned by funcidx())
\param phrase phrase to tell to player
*/
int amxTarget (int s, int callback, char *phrase)
{
	if (s<0) return INVALID;
	P_CHAR p = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPCR( p, INVALID );
	
	p->targetcallback = callback;
	
	target(s, 0, 1, 0, 191, phrase ); 

	return 0;
}


/*!
\brief handles the callback of amx targets
\author Xanathar, rewritten by Luxor
\param s socket to send the target to
\param TL Target Location
\remarks Luxor - Totally rewritten to use TargetLocation class
*/
void targetCallback (int s, TargetLocation &TL)
{
    NXWCLIENT client = getClientFromSocket(s);
	if( client==NULL )
		return;

    P_CHAR p = client->currChar();
    VALIDATEPC(p);
    int cback = p->targetcallback;
    if (cback == INVALID) return;
    
    P_CHAR pc = TL.getChar();
    if (ISVALIDPC(pc)) {
        if (cback>=0) AmxFunction::g_prgOverride->CallFn(cback, s, pc->getSerial32(), INVALID, INVALID, INVALID, INVALID);
        return;
    }

    P_ITEM pi = TL.getItem();
    if (ISVALIDPI(pi)) {
        if (cback>=0) AmxFunction::g_prgOverride->CallFn(cback, s, INVALID, pi->getSerial32(), INVALID, INVALID, INVALID);
        return;
    }

    int x, y, z;
    TL.getXYZ(x,y,z);

    if (cback>=0) AmxFunction::g_prgOverride->CallFn(cback, s, INVALID, INVALID, x, y, z);
}


