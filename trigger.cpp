  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "nxwcommn.h"
#include <errno.h>
#include "basics.h"
#include "sndpkg.h"
#include "debug.h"
#include "tmpeff.h"
#include "version.h"
#include "console.h"
#include "trigger.h"
#include "nxwgui.h"
#include "set.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "skills.h"
#include "range.h"
#include "utils.h"
#include "scripts.h"
#include "network.h"



//debugger things :] --begin--
bool g_bStepInTriggers = false;
extern "C" int g_nTraceMode; //by xan -> trace mode
bool g_bDebuggingTriggers = false;
//debugger things :] -- end --

#define TRIGMODE_ITEM 1
#define TRIGMODE_ENVOKED 2
#define TRIGMODE_ANYITEM 3
#define TRIGMODE_NPC 4
#define TRIGMODE_TARGET 8
#define TRIGMODE_STATIC 16
#define TRIGMODE_NPCENVOKE 32

#define FORCEMODE(_T) { if ((m_nTriggerType&_T)==0) { ConOut("Parsed %s in wrong trigger type\n", cmd); STOPTRIGGER; } }
#define UNSUPPORTED { printUnsupportedMessage(); m_bStop = true; return; }
#define STOPTRIGGER { m_bStop = true; return; }

#define NPCONLY  FORCEMODE(TRIGMODE_NPC)
#define ITEMONLY FORCEMODE(TRIGMODE_ITEM)
#define DYNAONLY FORCEMODE(TRIGMODE_ANYITEM|TRIGMODE_TARGET)
#define ISNPC(_A) ((_A == TRIGMODE_NPC) || (_A == TRIGMODE_NPCENVOKE))
#define ISNOTNPC(_A) ((_A != TRIGMODE_NPC) && (_A != TRIGMODE_NPCENVOKE))

/*!
\brief Prints an error banner
\author Xanathar
*/
static void printUnsupportedMessage()
{
	ConOut("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	ConOut("|| These commands are no more supported in NoX-Wizard %s  ||\n", VERNUMB);
	ConOut("|| while they were supported in previous versions and in     ||\n");
	ConOut("|| other emulators like UOX, Wolfpack, Lonewolf, UIO.You can ||\n");
	ConOut("|| use AMX scripts to get the same results, better.          ||\n");
	ConOut("||  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  ||\n");
	ConOut("|| Unsupported commands list :                               ||\n");
	ConOut("||   CMSG, COLOR, NEEDCOLOR, NEEDDUR, NEEDMAXDUR, OUTRANGE,  ||\n");
	ConOut("||   REQCOLOR, and IFREQ on npc scripts.                     ||\n");
	ConOut("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
}

/*!
\brief Parses IF commands, like IFKARMA or IFHUNGER
\author Xanathar
\param cmd as default
\param par as default
\param comparevalue the value to be compared
*/
void cTriggerContext::parseIfCommand(char* cmd, char* par, int comparevalue)
{
    char *mode;
	mode = strstr(par, ">");
	if (mode == 0) mode = strstr(par, "<");
	if (mode == 0) {
		WarnOut ("Trigger syntax error : malformed %s\n", cmd);
	}
	char opt = mode[0];
	int j = str2num(mode+1);
    if (opt == '>')
    {
        if (!(comparevalue >= j))
        {
            if (strlen(m_szFailMsg)) sysmessage(m_socket, m_szFailMsg);
	    else sysmessage(m_socket, TRANSLATE("You need more %s to do that!"), cmd+2);
            STOPTRIGGER;
        }
    }
    else if (opt == '<')
    {
        if (!(comparevalue <= j))
        {
            if (strlen(m_szFailMsg)) sysmessage(m_socket, m_szFailMsg);
	    else sysmessage(m_socket, TRANSLATE("You need less %s to do that!"), cmd+2);
            STOPTRIGGER;
        }
    }
    else
		WarnOut ("Trigger syntax error : malformed %s\n", cmd);
}

/*!
\brief Executes an IADD command
\author Xanathar
\param par parameters part of the command
*/
void cTriggerContext::parseIAddCommand(char* par)
{
	int array[3];
	fillIntArray(par, array, 3, 0);
	int triggerx, triggery, triggerz;

	UI32 itmNumber = array[0];
	UI32 InBackpack = array[1];
	UI32 itmamount = (array[2]<=0)? INVALID : array[2];
    
	Location charpos= m_pcCurrChar->getPosition();

    switch (m_pcCurrChar->dir)
    {
    case 0:
        triggerx = charpos.x;
        triggery = charpos.y - 1;
        break;
    case 1:
        triggerx = charpos.x + 1;
        triggery = charpos.y - 1;
        break;
    case 2:
        triggerx = charpos.x + 1;
        triggery = charpos.y;
        break;
    case 3:
        triggerx = charpos.x + 1;
        triggery = charpos.y + 1;
        break;
    case 4:
        triggerx = charpos.x;
        triggery = charpos.y + 1;
        break;
    case 5:
        triggerx = charpos.x - 1;
        triggery = charpos.y + 1;
        break;
    case 6:
        triggerx = charpos.x - 1;
        triggery = charpos.y;
        break;
    case 7:
        triggerx = charpos.x - 1;
        triggery = charpos.y - 1;
        break;
    default:
        triggerx = charpos.x + 1;
        triggery = charpos.y + 1;
        break;
    }
    triggerz = charpos.z;

	P_ITEM pi = NULL;
	if (!InBackpack) 
	{
		pi=item::CreateFromScript( itmNumber, NULL, itmamount );
		if( ISVALIDPI(pi) ) {
			pi->MoveTo(triggerx, triggery, triggerz);
			pi->Refresh();
		}
	} 
	else 
	{
		pi = item::CreateFromScript( itmNumber, m_pcCurrChar->getBackpack(), itmamount );
	}

    // Added colormem token here! by Magius(CHE) §
    if( ISVALIDPI(pi) ) {
		if( m_nColor1!=0xFF ) {
			pi->setColor( DBYTE2WORD( m_nColor1, m_nColor2 ) );
			pi->Refresh();
		}
    }
    // end addons
	m_piAdded = pi;
}

/*!
\brief Check if an item is an envoker
\param eid1 id of the item
\param eid2 id of the item
\return true if the item is an envoker
*/
bool checkenvoke( UI16 eid )
{
    char temp[100];
    char temp2[100];
    char script1[1024];

    cScpIterator* iter = 0;
    sprintf(temp, "x%xx", eid );      // antichrist
    sprintf(temp2, "x%x ", eid );      // antichrist


    iter = Scripts::Envoke->getNewIterator("SECTION ENVOKE");
	if (iter==0) {
		ErrOut("Can't find SECTION ENVOKE! \n");
		return false;
	}

	do {
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		if ((strstr(script1, temp) > 0) || (strstr(script1, temp2) > 0)) {
			safedelete(iter);
			return true;
		}
	} while ((script1[0]!='}'));

	safedelete(iter);
    return false;
}

/*!
\brief Common Initialization routine
\author Xanathar
\param number the number of the trigger, if known
\param s the socket
\param trigtype the trigger type
\param id the id for envoked/statics triggers
*/
void cTriggerContext::init(int number, NXWSOCKET  s, int trigtype, UI16 id)
{
	m_nNumber = number;
	m_iter = 0;
	strcpy(m_szFailMsg,"");
	strcpy(m_szDisableMsg,"");
	m_nColor1 = m_nColor2 = 0xFF; //FFFF = coloring disabled
	m_nTriggerType = trigtype;
	m_socket = s;
	m_pi = 0;
	m_piNeededItem = 0;
	m_pcAdded = 0;
	m_piAdded = 0; // the added item
	m_pcNpc = 0;
	m_piEnvoked = 0;
	m_pcCurrChar = MAKE_CHAR_REF(currchar[s]);
	m_bStop = false;
	char sect[500];

	switch(m_nTriggerType) {
	case TRIGMODE_ITEM:
		sprintf(sect, "SECTION ITEMTRG %d", number);
		m_iter = Scripts::Triggers->getNewIterator(sect);
		break;
	case TRIGMODE_ENVOKED:
	case TRIGMODE_NPCENVOKE:
        	sprintf(sect, "x%x", id);
		m_iter = Scripts::Envoke->getNewIteratorInStr(sect);
		break;
	case TRIGMODE_NPC:
		sprintf(sect, "SECTION NPCTRG %d", number);
		m_iter = Scripts::Triggers->getNewIterator(sect);
		break;
	case TRIGMODE_TARGET:
		sprintf(sect, "SECTION ITEMTRG %d", number);
		m_iter = Scripts::Triggers->getNewIterator(sect);
		break;
	default:
		m_iter = 0;
		return;
	}

}

/*!
\brief Constructor for item & statics triggers
\author Xanathar
\param number the number, -1 for envoke and statics
\param s the socket
\param itm pointer the item, if not static
\param trigtype the trigger type
*/
cTriggerContext::cTriggerContext(int number, NXWSOCKET  s, P_ITEM itm, int trigtype)
{
	if (trigtype==TRIGMODE_ENVOKED) {
		init(number, s, trigtype, itm->getId());
		m_pi = itm;
	} else if (trigtype==TRIGMODE_STATIC) {
		init(number, s, trigtype, ShortFromCharPtr(buffer[s] +17) );
		m_pi = 0;
	} else {
		init(number, s, trigtype);
		m_pi = itm;
	}
	if ((trigtype==TRIGMODE_ENVOKED)||(trigtype==TRIGMODE_STATIC)) {
		P_ITEM pi = pointers::findItemBySerial(m_pcCurrChar->envokeitem);
		if ( pi ) {
			m_piEnvoked = pi;
		} else  { //panic
			m_pcCurrChar->sysmsg(TRANSLATE("That didn't seem to work."));
			safedelete(m_iter); //<- this way trigger is disabled!
		}
	}
}

/*!
\brief Constructor for NPC triggers
\author Xanathar
\param number the trigger number
\param s the socket
\param pc the npc
\param trigtype trigger type
*/
cTriggerContext::cTriggerContext(int number, NXWSOCKET  s, P_CHAR pc, int trigtype)
{
	init(number, s, trigtype, pc->getId());
	m_pcNpc = pc;
	if (trigtype==TRIGMODE_NPCENVOKE) {
		P_ITEM pi = pointers::findItemBySerial(m_pcCurrChar->envokeitem);
		if (ISVALIDPI(pi)) {
			m_piEnvoked = pi;
		} else { //panic
			sysmessage(s, TRANSLATE("That didn't seem to work."));
			safedelete(m_iter); //<- this way trigger is disabled!
		}
	}
}

/*!
\brief Destructor
\author Xanathar
*/
cTriggerContext::~cTriggerContext()
{
	safedelete(m_iter);
}

/*!
\brief Checks if some of internal items/cahrs got deleted
\note require that garbage collection didn't happen
\author Xanathar
*/
void cTriggerContext::checkPtrsValidity()
{
	if (!ISVALIDPI(m_pi)) m_pi = 0;
	if (!ISVALIDPI(m_piEnvoked)) m_piEnvoked = 0;
	if (!ISVALIDPI(m_piAdded)) m_piAdded = 0;
	if (!ISVALIDPI(m_piNeededItem) || m_piNeededItem == m_pi ) m_piNeededItem = 0;
	if (!ISVALIDPC(m_pcNpc)) m_pcNpc = 0;
	if (!ISVALIDPC(m_pcAdded)) m_pcAdded = 0;
	if ((m_pcCurrChar!=0)) if(!ISVALIDPC(m_pcCurrChar)){ m_bStop = true; } //panic!
	if (m_pcCurrChar==0) { m_bStop = true; } //panic!
}

/*!
\brief Executes the trigger
\author Xanathar
*/
void cTriggerContext::exec()
{
    char script1[1024];
    char script2[1024];
	char command[128];

	command[0] = '\0';

	if (m_iter==0) return;
	m_iter->rewind();

	do {
		m_iter->parseLine(script1, script2);
		if ((script1[0]!='}')&&(script1[0]!='{')) {
			//debugger support
			if ((ServerScp::g_nLoadDebugger)&&(g_bStepInTriggers)) {
				SDbgOut("Executing %s(%s) [G=GO, otherwise STEP]\n", script1, script2);
				g_bDebuggingTriggers = true;
				SDbgIn(command, 120);
				if (command[0]=='G') g_bStepInTriggers = false;
				g_bDebuggingTriggers = false;
			}

			// go execute it!
			parseLine(script1, script2);
			checkPtrsValidity();
			//debugger native tracing :
			if (ServerScp::g_nLoadDebugger!=0) {
				if (g_nTraceMode!=0) {
					STraceOut("\tExecuted %s(%s)\n", script1, script2);
				}
			}
		}
	} while ((!m_bStop)&&(script1[0]!='}'));

	if ((ServerScp::g_nLoadDebugger)) {
			if (m_bStop)
				SDbgOut("Trigger exited reaching a failure condition [this is not an error]\n");
			else
				SDbgOut("Trigger exited reaching its end\n");
	}

}

/*!
\brief Entry Point for dynamic item triggers
\author Xanathar
\param ts the socket
\param pi the item triggered
\param eventType the type of trigger
*/
void triggerItem(NXWSOCKET  ts, P_ITEM pi, int eventType)
{
	VALIDATEPI(pi);

	P_CHAR pc=MAKE_CHAR_REF(currchar[ts]);
	VALIDATEPC(pc);

	if ( (pi->disabled>0)&&(!TIMEOUT(pi->disabled))) {
		if(pi->disabledmsg!=NULL)
			sysmessage(ts, "%s", pi->disabledmsg->c_str());
		return;
	}

	int n = -1, t = 0;

	switch (eventType)
	{
		case TRIGTYPE_DBLCLICK:
		case TRIGTYPE_WALKOVER:
			n = pi->trigger;
			t = TRIGMODE_ITEM;
			break;
		case TRIGTYPE_ENVOKED:
			n = -1;
			t = TRIGMODE_ENVOKED;
			break;
		case TRIGTYPE_TARGET:
			n = pc->targtrig;
			t = TRIGMODE_TARGET;
			break;
		default:
			WarnOut("triggerItem(ts=%d, pi=%s, ttype=%d) -> unknown type\n", ts,pi->getCurrentNameC(), eventType);
			return;
	}

	cTriggerContext ltc(n, ts, pi, t);
	ltc.exec();
}

/*!
\brief Entry point for all npc triggers
\author Xanathar
\param ts the socket
\param pc the npc
\param eventType the type of trigger
*/
void triggerNpc(NXWSOCKET  ts, P_CHAR pc, int eventType)
{
	if (ts < 0) return;
	if (pc == 0) return;

	if ( !TIMEOUT( pc->disabled ) ) {
		sysmessage(ts, "%s", pc->disabledmsg);
		return;
	}

	if (eventType==TRIGTYPE_NPCWORD) {
		cTriggerContext ltc(pc->trigger, ts, pc, TRIGMODE_NPC);
		ltc.exec();
	} else if (eventType==TRIGTYPE_NPCENVOKED) {
		cTriggerContext ltc(pc->trigger, ts, pc, TRIGMODE_NPCENVOKE);
		ltc.exec();
	}
}

/*!
\brief Entry point for envoked statics
\author Xanathar
\param ts the socket
*/
void triggerTile(NXWSOCKET  ts)
{
	if (ts < 0) return;

	cTriggerContext ltc(-1, ts, static_cast<P_ITEM>(0), TRIGMODE_STATIC);
	ltc.exec();
}

/*!
\author Xanathar
\brief Parses a line of trigger language
\param cmd the command
\param par the parameters
\note reindented by Akron
\todo IDFX command needs to be changed
*/
void cTriggerContext::parseLine(char* cmd, char* par)
{
	P_CHAR pc;
	switch(cmd[0])
	{
		case '@':
			if (!(strcmp("@CALL", cmd))) {
				g_nCurrentSocket = m_socket;
				int rt = AmxFunction::g_prgOverride->CallFn( AmxFunction::g_prgOverride->getFnOrdinal(par), m_socket );
				if (rt) STOPTRIGGER;
			}
			break;
		case 'A':
			if (!(strcmp("ACT", cmd))) { //player performs an action
				m_pcCurrChar->playAction(str2num(par, BASE_HEX)); //uses improveds action fns :]
			}
			break;
		case 'B':
			if (!(strcmp("BOLT", cmd))) { // bolts the player
				m_pcCurrChar->boltFX(false);
			}
			break;

		case 'C':
			if (!(strcmp("CSKL", cmd))) {  // Make a check on the players skill
				int skill = str2num(par);
				if (!m_pcCurrChar->checkSkill( static_cast<Skill>(skill), 0, 1000)) STOPTRIGGER;
			} else if (!(strcmp("CSKLNR", cmd))) {  // Make a check on the players skill
				int skill = str2num(par);
				if (!m_pcCurrChar->checkSkill( static_cast<Skill>(skill), 0, 1000, false)) STOPTRIGGER;
			} else if (!(strcmp("CSKLNRNOT", cmd))) {  // Make a check on the players skill
				int skill = str2num(par);
				if (m_pcCurrChar->checkSkill( static_cast<Skill>(skill), 0, 1000, false)) STOPTRIGGER;
			} else if (!(strcmp("CSKLNOT", cmd))) {  // Make a check on the players skill
				int skill = str2num(par);
				if (m_pcCurrChar->checkSkill( static_cast<Skill>(skill), 0, 1000)) STOPTRIGGER;
			}
			else if (!(strcmp("CONOUT", cmd))) { // Display a message on console, if in debug mode
				ConOut("[TRIG] %s\n", par);
			}
			else if (!(strcmp("CALL", cmd))) { // Display a message on console, if in debug mode
				int j = 0;
				j = str2num(par);
				if (j == 0) {
					WarnOut("CALL command syntax changed from previous NoX-Wizards\n");
					STOPTRIGGER;
				}
				if ((ISNPC(m_nTriggerType))&&(m_pcNpc!=0)) {
					triggerNpc(m_socket, m_pcNpc, TRIGTYPE_NPCWORD);
				}
				if ((ISNOTNPC(m_nTriggerType))&&(m_pi!=0)) {
					triggerItem(m_socket, m_pi, TRIGTYPE_DBLCLICK);
				}
			}
			else if (!(strcmp("CMSG", cmd))) {
				UNSUPPORTED;
			}
			else if (!(strcmp("COLOR", cmd))) {
				UNSUPPORTED;
			}
			else if (!(strcmp("CALCUSES", cmd))) { // Calculate an items uses
				if (m_pi) {
					if (m_pi->tuses){
						m_pi->tuses += str2num(par);
					}
				}
			}
			break;

		case 'D':
			if (!(strcmp("DO", cmd))) { // Executes a standard command
				static char tmppar[1024];
				strcpy(tmppar, par);
				char *p = strstr(tmppar, " ");
				if (p!=0) p = "  ";
				else {
					*p = '\0';
					p++;
				}
				scriptcommand(m_socket, tmppar, p);
			}
			else if (!(strcmp("DBGOUT", cmd))) { // Display a message on console, if in debug mode
					if (ServerScp::g_nLoadDebugger) SDbgOut("[TRIGGER DBG] > %s\n", par);
				}
			else if (!(strcmp("DEX", cmd))) { // Do math on players dexterity
				int params[2];
				fillIntArray(par, params, 2);
				if (params[1] == -1) {
					if (params[0] < 0) sysmessage(m_socket, TRANSLATE("You are getting clumsier!"));
						else sysmessage(m_socket, TRANSLATE("You are getting more agile!"));
					m_pcCurrChar->dx += params[0];
			if (m_pcCurrChar->dx < 1) m_pcCurrChar->dx = 1;
				} else { //extended syntax
					if (params[0]<0)
						tempfx::add(m_pcCurrChar, m_pcCurrChar, tempfx::SPELL_CLUMSY, params[0], 0, 0, params[1]);
					else
						tempfx::add(m_pcCurrChar, m_pcCurrChar, tempfx::SPELL_AGILITY, params[0], 0, 0, params[1]);
				}
			}

			else if (!(strcmp("DISABLE", cmd))) {  // Disable NPC for n seconds
				if ((ISNPC(m_nTriggerType))&&(m_pcNpc!=0)) {
					m_pcNpc->disabled = (uiCurrentTime +(MY_CLOCKS_PER_SEC*str2num(par)));
					if (m_pcNpc) {
						if( m_pcNpc->disabledmsg!=NULL )
							(*m_pcNpc->disabledmsg) = m_szDisableMsg;
						else
							m_pcNpc->disabledmsg= new std::string( m_szDisableMsg );
					}
				}
				if ((ISNOTNPC(m_nTriggerType))&&(m_pi!=0)) {
					m_pi->disabled = (uiCurrentTime +(MY_CLOCKS_PER_SEC*str2num(par)));
					if (m_pi) {
						if( m_pi->disabledmsg!=NULL )
							(*m_pi->disabledmsg)=m_szDisableMsg;
						else
							m_pi->disabledmsg = new std::string( m_szDisableMsg ); 
					}
				}
			} else if (!(strcmp("DISABLEMSG", cmd))) { // Disable NPC Message
				strncpy(m_szDisableMsg, par, 48);
			}
			else if (!(strcmp("DUR", cmd))) { // Disable NPC Message
                                if ( ISVALIDPI(m_pi) )
					parseDurCommand(m_pi, par);
				return;
			}
			break;

		case 'E':
			if (!(strcmp("EMT", cmd))) {  // Player says something when trigger is activated
				P_CHAR pc_emt=MAKE_CHAR_REF(currchar[m_socket]);
				if(ISVALIDPC(pc_emt))
					pc_emt->emote(m_socket,par,1);
			} else if (!(strcmp("EVDUR", cmd))) {
                                if ( ISVALIDPI(m_piEnvoked) )
					parseDurCommand(m_piEnvoked, par);
				return;
			} else if (!(strcmp("EVMAXDUR", cmd))) {
				parseMaxDurCommand(m_piEnvoked, par);
				return;
			}
			break;

		case 'F':
			if (!(strcmp("FAME", cmd))) {
				P_CHAR pc_curr=MAKE_CHAR_REF(currchar[m_socket]);
				VALIDATEPC(pc_curr);
				pc_curr->modifyFame(str2num(par));
			} else if (!(strcmp("FMSG", cmd))) { // Set fail message
				strncpy(m_szFailMsg, par, 48);
			}
			break;

		case 'H':
			if ((!(strcmp("HEA", cmd)))||(!(strcmp("HEAL", cmd)))) {
				int j = str2num(par);

				if ((j < 0) && (m_pcCurrChar->hp < abs(j)) && (!(m_pcCurrChar->IsInvul())))
				{
					m_pcCurrChar->Kill();
					STOPTRIGGER;
				} else {
					m_pcCurrChar->hp += j;
					if (m_pcCurrChar->hp > m_pcCurrChar->getStrength())
					m_pcCurrChar->hp = m_pcCurrChar->getStrength();
					m_pcCurrChar->updateStats(0);
				}
			} else if (!(strcmp("HUNGER", cmd))) { // Do math on players hunger from 0 to 6 - Magius(CHE)
				int j = str2num(par);
				m_pcCurrChar->hunger += j;
				if (m_pcCurrChar->hunger > 6) m_pcCurrChar->hunger = 6;
				if (m_pcCurrChar->hunger < 1) m_pcCurrChar->hunger = 1;

				switch (m_pcCurrChar->hunger) {
					case 0:
						sysmessage(m_socket, TRANSLATE("You are still extremely hungry."));
						break;
					case 1:
						sysmessage(m_socket, TRANSLATE("You are still extremely hungry."));
						break;
					case 2:
						sysmessage(m_socket, TRANSLATE("You feel much less hungry."));
						break;
					case 3:
						sysmessage(m_socket, TRANSLATE("You begin to feel more satiated."));
						break;
					case 4:
						sysmessage(m_socket, TRANSLATE("You feel quite full."));
						break;
					case 5:
						sysmessage(m_socket, TRANSLATE("You are nearly stuffed."));
						break;
					case 6:
						sysmessage(m_socket, TRANSLATE("You have no hunger!"));
						break;
				} //switch
			} // if cmd == hunger
			break;
		case 'I':
			if (!(strcmp("IFKARMA", cmd))) { // If karma meets a certain criteria
				parseIfCommand(cmd, par, m_pcCurrChar->GetKarma());
				return;
			} else if (!(strcmp("IFFAME", cmd))) { // If karma meets a certain criteria
				parseIfCommand(cmd, par, m_pcCurrChar->GetFame());
				return;
			} else if (!(strcmp("IFHUNGER", cmd))) { // If karma meets a certain criteria
				parseIfCommand(cmd, par, m_pcCurrChar->hunger);
				return;
			} else if (!(strcmp("IFOWNER", cmd))) {
				int ser = -1;

				if ((ISNPC(m_nTriggerType))&&(m_pcNpc!=0))
					ser = m_pcNpc->getOwnerSerial32();
				if ((ISNOTNPC(m_nTriggerType))&&(m_pi!=0))
					ser = m_pi->getOwnerSerial32();

				if (ser != m_pcCurrChar->getSerial32())
					sysmessage(m_socket, TRANSLATE("You do not own that."));

			} else if (!(strcmp("IFREQ", cmd))) {
				if (m_pi!=0) {
					DYNAONLY;
					char sect[128], *p;
					sprintf(sect, "x%x", m_pcCurrChar->envokeid );
					p = strstr(par, sect);
					if (p != 0)
					{
						p += strlen(sect);
						int j = str2num(p);
						m_pcCurrChar->targtrig = j;
						triggerItem(m_socket, m_pi, TRIGTYPE_TARGET);
						STOPTRIGGER;
					}
				}
			} else if (!(strcmp("IFNPCTRIGGER", cmd))) {
				if(ISNOTNPC(m_nTriggerType)) STOPTRIGGER;
			} else if (!(strcmp("IFITEMTRIGGER", cmd))) {
				if(ISNPC(m_nTriggerType)) STOPTRIGGER;
			} else if (!(strcmp("IDADD", cmd))) { // Add item in front of player by ID //ndEndy DEPRECATED
				int r = 0;
				int array[4];
				//syntax : IDADD <id1> <id2> <minimum> <maximum>
				//creates a random number of item with id 0x<id1><id2>
				//all numbers are hex!!
				fillIntArray(par, array, 4, 1, BASE_HEX);
				r = RandomNum(array[2], array[3]);
				if ((r < 0)||(r > 65535)) r = 1;
				if (r == 0) return;

				P_ITEM pc = item::CreateFromScript( "$item_hardcoded" );
				if (!ISVALIDPI(pc)) STOPTRIGGER;

				pc->setId( DBYTE2WORD( array[0], array[1] ) );
				if( m_nColor1 != 0xFF)
					pc->setColor( DBYTE2WORD( m_nColor1, m_nColor2 ) );

				if( ISVALIDPC( m_pcCurrChar ) ) {
					P_ITEM pack=m_pcCurrChar->getBackpack();
					if( ISVALIDPI(pack) )
						pack->AddItem( pc );
					else {
						pc->MoveTo( m_pcCurrChar->getPosition() );
						pc->Refresh();
					}
				}

				m_piAdded = pc;
				if (m_pi==0) m_pi = m_piAdded;
				if (m_piAdded==0) STOPTRIGGER;
			} else if (!(strcmp("IDFX", cmd))) { // Makes an effect at players by ID
				m_pcCurrChar->boltFX(false);
				// TODO IDFX command needs to be changed
			} else if (!(strcmp("INT", cmd))) { // Do math on players intelligence
				int params[2];
				fillIntArray(par, params, 2);
				if (params[1] == -1) {
					if (params[0] < 0)
						sysmessage(m_socket, TRANSLATE("You are getting stupid!"));
					else
						sysmessage(m_socket, TRANSLATE("You are getting more cunning!"));
					m_pcCurrChar->in += params[0];
					if (m_pcCurrChar->in < 1)
						m_pcCurrChar->in = 1;
				} else { //extended syntax
					if (params[0]<0)
						tempfx::add(m_pcCurrChar, m_pcCurrChar, tempfx::SPELL_FEEBLEMIND, params[0], 0, 0, params[1]);
					else
						tempfx::add(m_pcCurrChar, m_pcCurrChar, tempfx::SPELL_CUNNING, params[0], 0, 0, params[1]);
				}
			} else if (!(strcmp("IADD", cmd))) { // Add item in front of player //if 2nd param. is 1, add item into player's backpack - AntiChrist (with autostack)
				parseIAddCommand(par);
				if (m_pi==0) m_pi = m_piAdded;
			}

		case 'J' :
			if (!(strcmp("JUMP", cmd))) { // Display a message on console, if in debug mode
				int j = str2num(par);
				if (j == 0) {
					STOPTRIGGER;
				}
				if ((ISNPC(m_nTriggerType))&&(m_pcNpc!=0)) {
					triggerNpc(m_socket, m_pcNpc, TRIGTYPE_NPCWORD);
					STOPTRIGGER;
				}
				if ((ISNOTNPC(m_nTriggerType))&&(m_pi!=0)) {
					triggerItem(m_socket, m_pi, TRIGTYPE_DBLCLICK);
					STOPTRIGGER;
				}
			}


		case 'K' :
			if (!(strcmp("KARMA", cmd))) {
				P_CHAR pc_curr=MAKE_CHAR_REF(currchar[m_socket]);
				VALIDATEPC(pc_curr);
				pc_curr->IncreaseKarma(str2num(par));
			}
			break;

		case 'M':
			if (!(strcmp("MAKE", cmd))) {
				int array[4];
				array[0] = array[1] = 0;
				array[2] = array[3] = 16;
				fillIntArray(par, array, 4, -1, BASE_INARRAY);

				P_CHAR pc_make=MAKE_CHAR_REF(currchar[m_socket]);
				VALIDATEPC(pc_make);

				//ndEndy PDFARE
				//itemmake[m_socket].Mat1id = (array[2] << 8) + array[3];
				//itemmake[m_socket].has = pc_make->getAmount( itemmake[m_socket].Mat1id);
				//itemmake[m_socket].has2 = pc_make->getAmount( itemmake[m_socket].Mat2id);
				//itemmake[m_socket].coloring = -1; //coloring; // Magius(CHE) §
				//Skills::MakeMenu(m_socket, array[0], array[1]);
			} else if (!(strcmp("MAXDUR", cmd))) {
				parseMaxDurCommand(m_pi, par);
				return;
			} else if (!(strcmp("MISC", cmd))) {
				strlwr(par);
				if (!strcmp("bank", par)) {
						P_CHAR pc_curr=MAKE_CHAR_REF(currchar[m_socket]);
						VALIDATEPC(pc_curr);
						pc_curr->openBankBox(pc_curr);
				} else if (!strcmp("ware", par)) {
						P_CHAR pc_curr=MAKE_CHAR_REF(currchar[m_socket]);
						VALIDATEPC(pc_curr);
						pc_curr->openSpecialBank(pc_curr);
				} else if (!strcmp("balance", par)) {
					static char tmp[500];
					sprintf(tmp, TRANSLATE("You have %i gp in your bank account!"), m_pcCurrChar->countBankGold());
					if (m_pcNpc!=0) m_pcNpc->talk(m_socket, tmp, 0);
					else sysmessage(m_socket, tmp);
				}
			} else if (!(strcmp("MSG", cmd))) { // Display a message when trigger is activated
				sysmessage(m_socket, par);
			} else if (!(strcmp("MAN", cmd))) {  // Do math on players mana
				int j = str2num(par);
				if ((j < 0) &&(m_pcCurrChar->mn < abs(j))) {
					sysmessage(m_socket, TRANSLATE("Your mind is too tired to do that."));
					STOPTRIGGER;
				}
				m_pcCurrChar->mn += j;
				if (m_pcCurrChar->mn>m_pcCurrChar->in)
					m_pcCurrChar->mn = m_pcCurrChar->in;
				m_pcCurrChar->updateStats(1);
			} else if (!(strcmp("MEMCOLOR", cmd))) { // Store the item color in memory by Magius(CHE) §
				UNSUPPORTED;
			}
			break;

		case 'N':
			//////////////////////////////////////////////////////////////////////////
			// NPCHEA & NPCHEAL
			//////////////////////////////////////////////////////////////////////////
			if (! (strcmp("NPCHEA", cmd) && strcmp("NPCHEAL", cmd)) ) {
				int j = str2num(par);
				if (m_pcNpc==0) return;
				if ((j < 0) && (m_pcNpc->hp < abs(j)) && (!(m_pcNpc->IsInvul())))
				{
					m_pcNpc->Kill();
					STOPTRIGGER;
				} else {
					m_pcNpc->hp += j;
					if (m_pcNpc->hp > m_pcNpc->getStrength())
						m_pcNpc->hp = m_pcNpc->getStrength();
						m_pcNpc->updateStats(0);
				}
			} else if (! (strcmp("NEWNPCHEA", cmd) && strcmp("NEWNPCHEAL", cmd)) ) {
				int j = str2num(par);
				if ( m_pcAdded==0) return;
				if ((j < 0) && (m_pcAdded->hp < abs(j)) && (!(m_pcAdded->IsInvul())))
				{
					m_pcAdded->Kill();
					STOPTRIGGER;
				} else {
					m_pcAdded->hp += j;
					if (m_pcAdded->hp > m_pcAdded->getStrength())
						m_pcAdded->hp = m_pcAdded->getStrength();
					m_pcAdded->updateStats(0);
				}
			} else if (!(strcmp("NPCBOLT", cmd))) { // bolts the player
				if ( m_pcNpc==0) return;
				m_pcNpc->boltFX(false);
			} else if (!(strcmp("NEWNPCBOLT", cmd))) { // bolts the player
				if ( m_pcAdded==0) return;
				m_pcAdded->boltFX(false);
			} else if (!(strcmp("NPCACT", cmd))) { // Make npc perform an action
				if (m_pcNpc!=0) m_pcNpc->playAction(hex2num(par));
			} else if (!(strcmp("NADD", cmd))) {  // Add a NPC at given location - AntiChrist -- Fixed here by Magius(CHE) §
				// Usage: NADD <npc_number> <life_in_seconds>
				int array[2];
				fillIntArray(par, array, 2, 0);
				int uiNpcNum = array[0];
				int uiTimer = array[1];
				if (uiNpcNum==0) return;
				Location charpos= m_pcCurrChar->getPosition();

				int triggerx = charpos.x + 1;
				int triggery = charpos.y + 1;
				int triggerz = charpos.z;

				m_pcAdded = npcs::AddNPC(m_socket,NULL,uiNpcNum,triggerx,triggery,triggerz);

				if (uiTimer>0) {// if we have a timer
					m_pcAdded->summontimer = uiCurrentTime +(uiTimer*MY_CLOCKS_PER_SEC);
				}
				if (m_pcNpc == 0)
					m_pcNpc = m_pcAdded;
			}

			//////////////////////////////////////////////////////////////////////////
			// NEWOWNER
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("NEWOWNER", cmd)))  // Set ownership of NPC
		{
				if (m_pcAdded!=0) {
					m_pcAdded->setOwnerSerial32(m_pcCurrChar->getSerial32());
					m_pcAdded->tamed = true;
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// NEWTYPE
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("NEWTYPE", cmd)))  // Set type of item
		{
				if (m_pi!=0) {
					m_pi->type = str2num(par);
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// NEEDCOLOR
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("NEEDCOLOR", cmd)))  // Set ownership of NPC
		{ UNSUPPORTED; }

			//////////////////////////////////////////////////////////////////////////
			// NEEDDUR
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("NEEDDUR", cmd)))  // Set ownership of NPC
		{ UNSUPPORTED; }

			//////////////////////////////////////////////////////////////////////////
			// NEEDMAXDUR
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("NEEDMAXDUR", cmd)))  // Set ownership of NPC
		{ UNSUPPORTED; }

		//////////////////////////////////////////////////////////////////////////
			// NEED
			//////////////////////////////////////////////////////////////////////////
			else if (!(strcmp("NEED", cmd)))  // The item here is required and will be removed
		{
		// OH MY GOD
				char sect[100];
				NxwItemWrapper si;
				si.fillItemWeared( m_pcCurrChar, true, true, true );
				si.fillItemsInContainer( m_pcCurrChar->getBackpack(), true, false );
				for( si.rewind(); !si.isEmpty(); si++ ) {
					m_piNeededItem=si.getItem();
					if (ISVALIDPI(m_piNeededItem)) {
						sprintf(sect, "x%x", m_piNeededItem->getId() );
						if (strstr(par, sect))
						{
							break;
						}
					}
				}


		if (!ISVALIDPI(m_piNeededItem)) {
			if (strlen(m_szFailMsg))
			sysmessage(m_socket, m_szFailMsg); // Added by Magius(CHE)
			else {
						sysmessage(m_socket, TRANSLATE("You don't have the neccessary supplies to do that.")); // Changed by Magius(CHE)
					}
					STOPTRIGGER;
		}

		}
		//////////////////////////////////////////////////////////////////////////
			// NEWTYPE
			//////////////////////////////////////////////////////////////////////////
		else if ((!(strcmp("NEWTYPE", cmd))) ||(!(strcmp("SETTYPE", cmd))))  // Set active item type
		{
		if (m_piAdded!=0)
			m_piAdded->type = str2num(par);
		}
		//////////////////////////////////////////////////////////////////////////
			// NEWNAME
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("NEWNAME", cmd)))  // Give the new item/npc a name
		{
		if (m_piAdded!=0)
		{
			m_piAdded->setCurrentName(par);
		}
		if (m_pcAdded!=0)
		{
			//strcpy(m_pcAdded->getCurrentNameC(), par);
					m_pcAdded->setCurrentName(par);
		}
		}
		//////////////////////////////////////////////////////////////////////////
			// NEWTRIG
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("NEWTRIG", cmd)))  // Give the new item/npc a dynamic trigger number
		{
		if (m_piAdded!=0)
		{
			m_piAdded->trigger = str2num(par);
		}
		if (m_pcAdded!=0)
		{
			m_pcAdded->trigger = str2num(par);
		}
		}
		//////////////////////////////////////////////////////////////////////////
			// NEWWORD
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("NEWWORD", cmd)))  // Give the new npc a triggerword
		{
		if (m_pcAdded!=0)
		{
			m_pcAdded->trigword= par;
		}
		}
		//////////////////////////////////////////////////////////////////////////
			// NEWSPEECH
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("NEWSPEECH", cmd)))  // Give the new npc a new spech -- MAgius(CHE) §
		{
		if (m_pcAdded!=0)
		{
			m_pcAdded->speech = str2num(par);
		}
		}
		break;

		case 'O':
		//////////////////////////////////////////////////////////////////////////
		// OPENGUMP
		//////////////////////////////////////////////////////////////////////////
		if (!strcmp("OUTRANGE", cmd)) {
					UNSUPPORTED;
		}

		break;

		case 'P':
		//////////////////////////////////////////////////////////////////////////
			// PUT
			//////////////////////////////////////////////////////////////////////////
		if (!(strcmp("PUT", cmd)))  // Send player to X Y Z when triggered
		{
			int array[3];
					fillIntArray(par, array, 3, 0);

					/*
			m_pcCurrChar->x = array[0];
			m_pcCurrChar->y = array[1];
			m_pcCurrChar->z = array[2];
					*/
					m_pcCurrChar->MoveTo( array[0], array[1], array[2] );
			int c=currchar[m_socket];
					pc=MAKE_CHAR_REF(c);
					pc->teleport();
		}
		break;
		case 'R':
		//////////////////////////////////////////////////////////////////////////
			// REQCOLOR
			//////////////////////////////////////////////////////////////////////////
		if (!(strcmp("REQCOLOR", cmd)))  // Set the color check on REQUIRED item by Magius(CHE) §
		{
					UNSUPPORTED;
		}
		//////////////////////////////////////////////////////////////////////////
			// RANGE
			//////////////////////////////////////////////////////////////////////////
		if (!(strcmp("RANGE", cmd))) { // Set the color check on REQUIRED item by Magius(CHE) §
			int x1,y1,x2,y2,z2,p,param = str2num(par);

					P_ITEM pack;
					Location charpos= m_pcCurrChar->getPosition();
					x1 = charpos.x;
			y1 = charpos.y;
					if (m_pi==0) return;

			x2 = m_pi->getPosition("x");
			y2 = m_pi->getPosition("y");
			z2 = m_pi->getPosition("z");
			p = currchar[m_socket];
			pack = (MAKE_CHAR_REF(currchar[m_socket]))->getBackpack();

			if (pack!=0) {// lb
			if (m_pi->getContSerial() != pack->getSerial32())
			{
				int dx = abs(x1 - x2);
				int dy = abs(y1 - y2);
				if ((dx>param) ||(dy>param))
				{
				if (strlen(m_szFailMsg)) {
					sysmessage(m_socket, m_szFailMsg);
								}
				else {
					sysmessage(m_socket, TRANSLATE("You are not close enough to use that."));
								}
								STOPTRIGGER;
				} //((dx>str2num(script2)) ||(dy>str2num(script2)))
			} //(pi->contserial != items[r].serial)
					}	//r!=-1
		}
		//////////////////////////////////////////////////////////////////////////
			// RAND
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("RAND", cmd)))  // Does a random check
		{
			int array[2], p;
					fillIntArray(par, array, 2, 0);

					if (array[0] != 0) p = (rand()%(array[0])) + 1;
					else p = 1;

			if (p>array[1])
			{
			if (strlen(m_szFailMsg))
				sysmessage(m_socket, m_szFailMsg);
			STOPTRIGGER;
			}
		}
		//////////////////////////////////////////////////////////////////////////
			// REQ
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("REQ", cmd)))  // Check if envoked by certain item. Added By Magius(CHE) to fix Targ trigger
		{
			char sect[128];
					sprintf(sect, "x%x", m_pcCurrChar->envokeid );
			if (!strstr(par, sect))
			{
			sysmessage(m_socket, TRANSLATE("That didn't seem to work."));
						STOPTRIGGER;
			}
		}
		if (!(strcmp("REMOVE", cmd)))  // The item here is required and will be removed
		{
					if ( !ISVALIDPI(m_pi) ) {
						STOPTRIGGER;
					}

					if (m_pi->amount>1)
						m_pi->amount--;
					else {
						m_pi->Delete();
						m_pi = 0;
					}
		}
		if (!(strcmp("RNDUSES", cmd)))  // The item here is required and will be removed
		{
			if (m_pi!=0)
			{
			if (!m_pi->tuses)
			{
				int array[2];
							fillIntArray(par, array, 2, 0);
				if (array[1] != 0)
				m_pi->tuses = (rand()%(array[1])) + array[0];
				else
				m_pi->tuses = array[0];
			}
			}
				}
		break;

		case 'S':
		//////////////////////////////////////////////////////////////////////////
			// SND
			//////////////////////////////////////////////////////////////////////////
		if (!(strcmp("SND", cmd)))  // Play a sound when trigger is activated
		{
			int array[2];
					fillIntArray(par, array, 2, 0, 16);
					m_pcCurrChar->playSFX( DBYTE2WORD( array[0], array[1] ));
		}
		//////////////////////////////////////////////////////////////////////////
			// SETTRG
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("SETTRG", cmd)))  // Set npcs trigger to new trigger
		{
			if ((ISNPC(m_nTriggerType))&&(m_pcNpc!=0))
						m_pcNpc->trigger = str2num(par);

			if ((ISNOTNPC(m_nTriggerType))&&(m_pi!=0))
						m_pi->trigger = str2num(par);
		}
		//////////////////////////////////////////////////////////////////////////
			// SPEECH
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("SPEECH", cmd)))  // Set the triggered npc a new spech -- Magius(CHE) §
		{
			if (m_pcNpc!=0)
			{
			m_pcNpc->speech = str2num(par);
			}
		}
		// End NPC Triggers
		//////////////////////////////////////////////////////////////////////////
			// SETID
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("SETID", cmd)))  // Set chars id to new id
		{
			int array[2];
			fillIntArray(par, array, 2, 0, 16);

			if ((ISNPC(m_nTriggerType))&&(m_pcNpc!=0)) {
			m_pcNpc->setId( DBYTE2WORD( array[0], array[1] ) );
			m_pcNpc->setOldId( DBYTE2WORD( array[0], array[1] ) );
			for (int j = 0; j < now; j++)
				if (clientInfo[j]->ingame && char_inVisRange(m_pcNpc,MAKE_CHAR_REF(currchar[j])))
				m_pcNpc->teleport();
			}
			if ((ISNOTNPC(m_nTriggerType))&&(m_pi!=0)) {
				m_pi->setId( DBYTE2WORD( array[0], array[1] ) );
				m_pi->Refresh();
			}
		}
		else if (!(strcmp("SETEVID", cmd)))  // Set envoked items id to new id
		{
			int array[2];
			fillIntArray(par, array, 2, 0, 16);
			if (m_piEnvoked != 0)
			{
				m_piEnvoked->setId( DBYTE2WORD( array[0], array[1] ) );
				m_piEnvoked->Refresh();
			}
		}
		//////////////////////////////////////////////////////////////////////////
			// SETUSES
			//////////////////////////////////////////////////////////////////////////
			else if (!(strcmp("SETUSES", cmd))) { // Set an items uses
		if (m_pi!=0) {
			m_pi->tuses = str2num(par);
		}
		}
		//////////////////////////////////////////////////////////////////////////
			// STAM
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("STAM", cmd)))  // Do math on players stamina
		{
					int j = str2num(par);
					if ((j < 0) &&(m_pcCurrChar->stm < abs(j))) {
						sysmessage(m_socket, TRANSLATE("Your body is too tired to do that."));
						STOPTRIGGER;
					}
					m_pcCurrChar->stm += j;
					if (m_pcCurrChar->stm>m_pcCurrChar->dx)
						m_pcCurrChar->stm = m_pcCurrChar->dx;
					m_pcCurrChar->updateStats(1);
		}
		//////////////////////////////////////////////////////////////////////////
			// STR
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("STR", cmd)))  // Do math on players strength
		{
					int params[2];
					fillIntArray(par, params, 2);
					if (params[1] == -1) {
						if (params[0] < 0)
							sysmessage(m_socket, TRANSLATE("You are getting weaker!"));
						else
							sysmessage(m_socket, TRANSLATE("You are getting stronger!"));

						m_pcCurrChar->modifyStrength(params[0]);

						if (m_pcCurrChar->getStrength() < 1)
							m_pcCurrChar->setStrength(1);
					} else { //extended syntax
						if (params[0]<0)
							tempfx::add(m_pcCurrChar, m_pcCurrChar, tempfx::SPELL_WEAKEN, params[0], 0, 0, params[1]);
						else
							tempfx::add(m_pcCurrChar, m_pcCurrChar, tempfx::SPELL_STRENGHT, params[0], 0, 0, params[1]);
					}
		}
		//////////////////////////////////////////////////////////////////////////
			// SETWORD
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("SETWORD", cmd)))  // Sets the trigger word of an NPC
		{
			if (m_pcNpc!=0) m_pcNpc->trigword= par;
		}
		//////////////////////////////////////////////////////////////////////////
			// SKL
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("SKL", cmd)))  // Do math on the players skill
		{
					int params[2];
					fillIntArray(par, params, 2, 0);
					int p = params[0];
					int j = params[1];

			if (j!=0) {
			if (m_pcCurrChar->baseskill[p] < 1000) {
				m_pcCurrChar->baseskill[p] += j;
				if (m_pcCurrChar->baseskill[p]>1000)
				m_pcCurrChar->baseskill[p] = 1000;
			}
			} else {
						Skills::AdvanceSkill(currchar[m_socket], p, 1);
			}
		}
		//////////////////////////////////////////////////////////////////////////
			// SETOWNER
			//////////////////////////////////////////////////////////////////////////
		else if (!(strcmp("SETOWNER", cmd)))  // Set ownership of NPC
		{
					if (m_pcNpc!=0) {
						m_pcNpc->setOwnerSerial32(m_pcCurrChar->getSerial32());
						m_pcNpc->tamed = true;
					}
		}
		break;
		case 'T':
		//////////////////////////////////////////////////////////////////////////
			// TALK
			//////////////////////////////////////////////////////////////////////////
		if (!(strcmp("TALK", cmd)))  // the triggered NPC now talking!
		{
			if (m_pcNpc!=0) {
			m_pcNpc->talk(m_socket, par, 0);
					}
		}
			//////////////////////////////////////////////////////////////////////////
			// TARG
			//////////////////////////////////////////////////////////////////////////
		if (!(strcmp("TARG", cmd)))  // Give a targeter with trigger number
		{
			P_TARGET targ=clientInfo[m_socket]->newTarget( new cItemTarget() );
			targ->code_callback=target_trigger;
			targ->send( getClientFromSocket(m_socket) );
			sysmessage( m_socket, TRANSLATE("Select a target") );
			m_pcCurrChar->targtrig = str2num(par);
		}
		break;
		case 'U':
		//////////////////////////////////////////////////////////////////////////
			// USEUP
			//////////////////////////////////////////////////////////////////////////
		if (!(strcmp("USEUP", cmd)))  // The item here is required and will be removed
		{
					if ( !ISVALIDPI(m_piNeededItem) ) {
						parseLine("NEED", par); //recurse on NEED command :]
						if (m_bStop) return;
					}

					if ( !ISVALIDPI(m_piNeededItem) || m_piNeededItem == m_pi ) {
						STOPTRIGGER;
					}

					if (m_piNeededItem->amount>1)
						m_piNeededItem->amount--;
					else {
						m_piNeededItem->Delete();
						m_piNeededItem = 0;
					}
		}
	} //switch
}

/*!
\brief Parsers DUR / EVDUR commands
\author Xanathar
\param pi the item to be DURed
\param par parameters
*/
void cTriggerContext::parseDurCommand(P_ITEM pi, char *par)
{
	if (pi == 0)
		STOPTRIGGER;

	SI32 array[2];
	fillIntArray( par, array, 2 );
	SI32 j = array[0];
	SI32 p = array[1];
	SI32 c = pi->hp;
	if (p <= 0)
		p = 100;
	if (pi->maxhp>0)
	{
		if (p >= ((rand()%100) + 1) )
		{
			if ( c >= pi->maxhp && j > 0 )
			sysmessage(m_socket, TRANSLATE("Your %s is already totally repaired!"), pi->getRealItemName());
			pi->hp += j;
			if (pi->hp >= pi->maxhp)
				pi->hp = pi->maxhp;

			if (pi->hp - c > 0)
				//sysmessage(m_socket, TRANSLATE("Your %s is now repaired! [%.1f%%]"), pi->getRealItemName(),  static_cast<float>(pi->hp) / static_cast<float>(pi->maxhp) * 100.0f);
				sysmessage(m_socket, TRANSLATE("Your %s is now somewhat repaired!"), pi->getRealItemName());
			else
				if ( pi->hp - c < 0 )
				sysmessage(m_socket, TRANSLATE("Your %s appears to be more ruined than before!"), pi->getRealNameC());

			if (pi->hp <= 0)
			{
				sysmessage(m_socket, TRANSLATE("Your %s was too old and it has been destroyed!"), pi->getRealItemName());
				if (pi->amount>1)
					--pi->amount;
				else
				{
					pi->Delete();
					pi = 0;
				}
			}
		}
	}
	else
		sysmessage(m_socket, TRANSLATE("You can't repair %s !"), pi->getRealItemName());
}

/*!
\brief Parses MAXDUR / EVMAXDUR commands
\author Xanathar
\param pi the item to be MAXDURed
\param par parameteres
*/
void cTriggerContext::parseMaxDurCommand(P_ITEM pi, char *par)
{
	if (pi == 0)
		STOPTRIGGER;

	SI32 array[2];
	fillIntArray(par, array, 2);
	SI32 j = array[0];
	SI32 p = array[1];

	if (p <= 0)
		p = 100;

	if (pi->maxhp>0)
	{
		if ((rand()%100) + 1 <= p)
		{
			pi->maxhp += j;
			if ( pi->hp >= pi->maxhp )
				pi->hp = pi->maxhp;

			if (str2num(script2) >= 0)
				sysmessage(m_socket, TRANSLATE("You increased the maximum durability of your %s !"), pi->getRealItemName());
			else
				sysmessage(m_socket, TRANSLATE("Your %s appears to be more ruined than before!"), pi->getRealItemName());

			if (pi->maxhp <= 0)
			{
				sysmessage(m_socket, TRANSLATE("Your %s was too old and it has been destoryed!"), pi->getRealItemName());
				if (pi->amount>1)
					--pi->amount;
				else
				{
					pi->Delete();
					pi = 0;
				}
			}
		}
	}
	else
		sysmessage(m_socket, TRANSLATE("You can't increase durability of %s !"), pi->getRealItemName());
}
