  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "sndpkg.h"
#include "custmenu.h"
#include "collector.h"
#include "addmenu.h"


/*!
\brief Class MakeItemPtr, extended pointer to MakeItem
*/
class MakeItemPtr {
private:
    MakeItem* m_ptr;
public:

    MakeItemPtr() { m_ptr = NULL; }
    MakeItemPtr(MakeItem* ptr) { m_ptr = ptr; }

    MakeItem* get() { return m_ptr; }
    void set(MakeItem* ptr) { m_ptr = ptr; }
};

/*!
\brief Class cAddMenu for make menu
*/
class cAddMenu : public cCustomMenu {
private:
	int m_nSection;
	vector<string> m_vCommands1;
	vector<string> m_vCommands2;
public:
	bool checkShouldAdd (cScpEntry* entry, P_CHAR pc);
	char* cleanString(const char *s);
	void loadFromScript (int section, P_CHAR pc = NULL);
	void buttonSelected(NXWSOCKET  s, UI16 btn, int seed );

};

map<int, MakeItemPtr> g_mapMakeItems;	//!< All the MakeItems


/*!
\brief Parses a "raw" item stored in a single line
\author Xanathar
\return void
\param sz byte number to get
*/
void RawItem::parse(char *sz)
{
   	int params[3];
   	fillIntArray(sz, params, 3);
    id = params[0];
    color = params[1];
    number = params[2];
    if (number < 1) number = 1;
}


/*!
\brief Constructor of class MakeItem
*/
MakeItem::MakeItem() {
    mana = stam = hit = 0;
    skillToCheck = INVALID;
    minskill = 0; maxskill = 1000;
    reqspell = INVALID;
    for (int i=0; i<MakeItem::MAXREQITEM; i++) reqitem[i].id = 0;
}

/*!
\brief Check if the player is skilled enough and have requested items
\return bool can or can't 
\param pc the player
\param inMenu if write a sysmessage on error
\todo Add message if haven't enough item..
*/
bool MakeItem::checkReq(P_CHAR pc, bool inMenu)
{
    VALIDATEPCR(pc,false);
    if (pc->IsGM()) return true;
    if ((skillToCheck!=INVALID)&& (pc->skill[skillToCheck]<minskill)) {
        if (!inMenu) pc->sysmsg(TRANSLATE("You're not enough skilled"));
        return false;
    }

    for (int i=0; i<MakeItem::MAXREQITEM; i++) {
        if (reqitem[i].id!=0) {
           if (pc->CountItems(reqitem[i].id, reqitem[i].color)<(UI32)reqitem[i].number) return false;
        }
    }
    return true;
}

/*!
\brief Get the Specified MakeItem
\return MakeItem* the pointer to MakeItem number n 
\param n MakeItem number
*/
MakeItem* getMakeItem(int n)
{
    MakeItemPtr* mip = &g_mapMakeItems[n];
    if (mip->get()!=NULL) return mip->get();

    // we're here so no makeitem number n has been loaded yet
    // so search it :]
    char script1[1024], script2[1024];
    sprintf(script1, "SECTION MAKE %d", n);
    cScpIterator* iter = Scripts::Create->getNewIterator(script1);

    if (iter==NULL) return NULL;

    MakeItem* mi = new MakeItem;
    int reqres = 0;

    do {
		iter->parseLine(script1, script2);
		if (!(strcmp((char*)script1,"DO"))) {
		    if (strlen(script2) < 4) {
		        WarnOut("Malformed DO command\n");
		        return NULL;
		    }
		    char *p = strstr(script2, " ");
		    if (p==NULL) {
		        mi->cmd1 = script2;
		        mi->cmd2 = "";
		    } else {
		        *p = '\0';
		        mi->cmd1 = script2;
		        mi->cmd2 = p+1;
		    }
		} else if (!(strcmp((char*)script1,"SKILL"))) {
		    mi->skillToCheck = str2num(script2);
		} else if (!(strcmp((char*)script1,"MINSKILL"))) {
		    mi->minskill = str2num(script2);
		} else if (!(strcmp((char*)script1,"MAXSKILL"))) {
		    mi->maxskill = str2num(script2);
		} else if (!(strcmp((char*)script1,"MANA"))) {
		    mi->mana = str2num(script2);
		} else if (!(strcmp((char*)script1,"STAM"))) {
		    mi->stam = str2num(script2);
		} else if (!(strcmp((char*)script1,"REQSPELL"))) {
		    mi->reqspell = str2num(script2);
		} else if (!(strcmp((char*)script1,"HP"))) {
		    mi->hit = str2num(script2);
		}  else if (!(strcmp((char*)script1,"REQ"))) {
			if (reqres <MakeItem::MAXREQITEM) {
    		    mi->reqitem[reqres++].parse(script2);
    		} else {
				WarnOut("makeitem %d has more than %d required resources\n", n, MakeItem::MAXREQITEM);
    		}
		}
    } while  (strcmp((char*)script1,"}"));

    mip = new MakeItemPtr(mi);
    g_mapMakeItems[n] = *mip;
    safedelete(mip);

    return getMakeItem(n);
}




/*!
\brief shows the new kind of add menus
\author Xanathar
\return void
\param pc the char to send the menu to
\param menu the itemmenu number
*/
void showAddMenu(P_CHAR pc, int menu)
{
	if (pc->customMenu != NULL) {
		safedelete(pc->customMenu);
	}

	pc->customMenu = new cAddMenu();
	cAddMenu *m = (cAddMenu*)pc->customMenu;
	m->loadFromScript(menu, pc);
	m->showMenu( calcSocketFromChar( DEREF_P_CHAR( pc ) ) );
}


/*!
\brief executes a "MAKE" command
\author Xanathar
\return void
\param pc player who do make
\param n item number
*/
void execMake(P_CHAR pc, int n)
{
    int i;
    MakeItem* mi = getMakeItem(n);
    NXWSOCKET sock;
    NXWCLIENT cli;

    if (pc->dead) {
        pc->sysmsg(TRANSLATE("Ever thought an ethereal soul can't really do some actions ?"));
        return;
    }

    cli = pc->getClient();
    if (cli==NULL) return;
    sock = cli->toLegacySocket();
    if (sock<=INVALID) return;

    //early abort on exceptional situations :]
    if (mi==NULL) return;
    if (pc==NULL) return;

    if (mi->reqspell!=INVALID) {
        if (!pc->knowsSpell((magic::SpellId)mi->reqspell)) {
            pc->sysmsg(TRANSLATE("You don't know that spell."));
            return;
        }
    }

    if (pc->hp < mi->hit) {
        pc->sysmsg(TRANSLATE("You could die for it.. "));
        return;
    }
    if (pc->stm < mi->stam) {
        pc->sysmsg(TRANSLATE("You're too tired "));
        return;
    }
    if (pc->mn < mi->mana) {
        pc->sysmsg(TRANSLATE("Your mind is too tired "));
        return;
    }
    if (mi->skillToCheck>INVALID) {
        if (pc->skill[mi->skillToCheck] < mi->minskill) {
            pc->sysmsg(TRANSLATE("You need to experience more to do that"));
            return;
        }
    }
    struct RawItem *requiredItem;
    for (i=0; i<MakeItem::MAXREQITEM; i++) {
    		requiredItem = &mi->reqitem[i];
        if (requiredItem->id > 0) {
           if (pc->CountItems(requiredItem->id, requiredItem->color)< (UI32)requiredItem->number) {
               pc->sysmsg(TRANSLATE("You've not enough resources"));
               return;
           }
        }
    }
    //we're here -> we can do the stuff ;]

    // sequence is :
    //  - item removals
    //  - skill check
    //  - stat removals
    //  - do what should be done

    for (i=0; i<MakeItem::MAXREQITEM; i++) {
        pc->delItems(mi->reqitem[i].id, mi->reqitem[i].number, mi->reqitem[i].color);
    }

    if (!pc->checkSkill((Skill)mi->skillToCheck, mi->minskill, mi->maxskill)) {
        pc->sysmsg(TRANSLATE("You failed"));
        return;
    }

    pc->damage(mi->hit,  DAMAGE_PURE, STAT_HP);
    pc->damage(mi->mana, DAMAGE_PURE, STAT_MANA);
    pc->damage(mi->stam, DAMAGE_PURE, STAT_STAMINA);

    char tmp1[1024];
    char tmp2[1024];
    strcpy(tmp1, mi->cmd1.c_str());
    strcpy(tmp2, mi->cmd2.c_str());

    scriptcommand(sock, tmp1, tmp2);

}


/*!
\brief decides where a menu item should be shown
\author Xanathar
\return void
\param entry the script item
\param pc the player
*/
bool cAddMenu::checkShouldAdd (cScpEntry* entry, P_CHAR pc)
{
    if (entry==NULL) return false;
    if (NULL==entry->getParam1().c_str()) return false;
    // always all options for standard menu options :]
    if (NULL==strstr(entry->getParam1().c_str(), "MAKE")) return true;
    // gee, GMs can do *anything*...
    if (pc->IsGM()) return true;
       // bugged makeitemz check :[
    if (NULL==entry->getParam2().c_str()) return false;

	MakeItem* mi = getMakeItem(atoi(entry->getParam2().c_str()));
	if (mi==NULL) return false;

	return mi->checkReq(pc, true);
}


/*!
\brief Clean a String
\author Xanathar
\return char*
\param s itemmenu number to be loaded
*/
char* cAddMenu::cleanString(const char *s)
{
	// Akron, fix for compile with Borland C++ Builder
	// in bcb strstr doesn't accept const char * as parameter :/
	char *t = const_cast<char*>(s);
	char *p = strstr(t, " ");
	if (p!=NULL) return p+1;
	return t;
}

/*!
\brief loads an itemmenu from a script
\author Xanathar
\return void
\param section itemmenu number to be loaded
\param pc the player
*/
void cAddMenu::loadFromScript (int section, P_CHAR pc)
{
	int nOpt = 0;
	bool bIcons = false;
	bool bNotDecided = true;

	char sz[80];
	sprintf(sz, "SECTION MENU %d", section);

	cScpEntry* entry;
	cScpEntry* entry2;
	cScpIterator* iter = Scripts::Menus->getNewIterator(sz);
	if (iter == NULL) {
		// build a fake menu :]
		setParameters(1, 1);
		setTitle("Menu Error, section not found");
		setStyle(MENUSTYLE_PAPER, 0x0);
		addMenuItem(0, 0, "Close this one here");
		m_vCommands1.push_back("NOP"); //default command :]
		m_vCommands2.push_back(" "); //default command :]
		buildMenu();
		return;
	}

	entry = iter->getEntry();
	if (entry->getFullLine().c_str()[0]=='{') entry = iter->getEntry();
		while (entry->getFullLine().c_str()[0]!='}')
		{
			entry = iter->getEntry();
			if (entry->getFullLine().c_str()[0]!='}') {
				if ((bNotDecided)&&(entry->getFullLine().c_str()[0]!='<')) bIcons = true;
				bNotDecided = false;
				entry = iter->getEntry();
				nOpt++;
			}
		}

	iter->rewind();

	entry = iter->getEntry();
	if (entry->getFullLine().c_str()[0]=='{') entry = iter->getEntry();

	if (!bIcons) {
		if (nOpt<=10) setParameters(nOpt, 1);
		else setParameters(10,(nOpt/10)+1);
		setStyle(MENUSTYLE_STONE, 0x0);
	} else {
		setParameters(nOpt, 1);
		setStyle(MENUSTYLE_ICONLIST, 0x0);
	}

	setTitle(const_cast<char*>(entry->getFullLine().c_str()));

	nOpt = 0;

	while (entry->getFullLine().c_str()[0]!='}')
	{
		entry = iter->getEntry();
		if (entry->getFullLine().c_str()[0]!='}') {
			entry2 = iter->getEntry();
			if ((pc)&&(checkShouldAdd(entry2, pc))) {
   				if (!bIcons) addMenuItem(nOpt / 10, nOpt % 10, cleanString(entry->getFullLine().c_str()));
   				else addMenuItem(0, nOpt, const_cast<char*>(entry->getFullLine().c_str()));
   				m_vCommands1.push_back(entry2->getParam1());
   				m_vCommands2.push_back(entry2->getParam2());
			}
			nOpt++;
		}
	}

	buildMenu();
	safedelete(iter);
}


/*!
\brief see custmenu.h / custmenu.cpp for explanations :)
\author Xanathar
\return void
\param s socket which clicked the button
\param btn button number
\param seed menu seed
*/
void cAddMenu::buttonSelected(NXWSOCKET  s, UI16 btn, int seed )
{
	if (btn < 10) return;
	if (seed!=m_nSeed) {
		sysmessage(s, COLOR_LIGHTBLUE, TRANSLATE("Sorry, this menu was rather old :("));
		return;
	}
	//ConOut("MENUS : %s %s\n", m_vCommands1[btn-10].c_str(), m_vCommands2[btn-10].c_str());
	scriptcommand (s, const_cast<char*>(m_vCommands1[btn-10].c_str()),
	const_cast<char*>(m_vCommands2[btn-10].c_str())); // Execute command from script
}
