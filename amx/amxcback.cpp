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
		return (0!=pi->amxevents[EVENT_IONCHECKCANUSE]->Call(pi->getSerial32(), pc->getSerial32(), g_nType));
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

		



