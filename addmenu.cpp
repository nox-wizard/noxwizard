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
#include "addmenu.h"
#include "menu.h"
#include "scp_parser.h"
#include "inlines.h"
#include "scripts.h"


cAddMenu::cAddMenu( SERIAL section, P_CHAR pc ) : cBasicMenu( MENUTYPE_CUSTOM )
{
	oldmenu=new cOldMenu();
	this->section=section;
	loadFromScript( pc );
}

cAddMenu::~cAddMenu(  )
{

}

void cAddMenu::loadFromScript( P_CHAR pc )
{
	VALIDATEPC(pc);
	
	int nOpt = 0;
	bool bIcons = false;
	bool bNotDecided = true;

	cScpIterator* iter = Scripts::Menus->getNewIterator("SECTION MENU %d", section);
	
	if(iter == NULL) { // build a fake menu :]
		
		oldmenu->setParameters(1, 1);
		oldmenu->title = L"Menu Error, section not found";
		oldmenu->style = MENUTYPE_PAPER;
		std::wstring ws;
		string2wstring( std::string("Close this one here"), ws );
		oldmenu->addMenuItem( 0, 0, ws );

		commands.push_back( cScriptCommand( std::string("NOP"), std::string(" ") ) );
		return;
	}

	cScpEntry* entry = iter->getEntry();
	if (entry->getFullLine().c_str()[0]=='{') entry = iter->getEntry();
		while (entry->getFullLine().c_str()[0]!='}')
		{
			entry = iter->getEntry();
			if (entry->getFullLine().c_str()[0]!='}') {
				if( (bNotDecided)&&(entry->getFullLine().c_str()[0]!='<') ) 
					bIcons = true;
				bNotDecided = false;
				entry = iter->getEntry();
				nOpt++;
			}
		}

	iter->rewind();

	entry = iter->getEntry();
	if (entry->getFullLine().c_str()[0]=='{') entry = iter->getEntry();

	if (!bIcons) {
		if (nOpt<=10) 
			oldmenu->setParameters(nOpt, 1);
		else 
			oldmenu->setParameters(10,(nOpt/10)+1);
		oldmenu->style = MENUTYPE_STONE;
	} else {
		oldmenu->setParameters(nOpt, 1);
		oldmenu->style = MENUTYPE_ICONLIST;
	}

	string2wstring( entry->getFullLine(), oldmenu->title );

	nOpt = 0;

	while (entry->getFullLine().c_str()[0]!='}')
	{
		entry = iter->getEntry();
		if (entry->getFullLine().c_str()[0]!='}') {

			cScpEntry* entry2 = iter->getEntry();
			if( checkShouldAdd(entry2, pc) ) {
   				
				std::wstring w;
				
				if (!bIcons) 
					string2wstring( cleanString( entry->getFullLine() ), w );
   				else 
					string2wstring( entry->getFullLine(), w);

				
				oldmenu->addMenuItem(0, nOpt, w );

   				commands.push_back( cScriptCommand( entry2->getParam1(), entry2->getParam2() ) );
			}
			nOpt++;
		}
	}

	safedelete(iter);
}


/*!
\brief Decides where a menu item should be shown
\author Endymion
\param entry the script item
\param pc the player
*/
bool cAddMenu::checkShouldAdd (cScpEntry* entry, P_CHAR pc)
{
    if( entry==NULL ) return false;

    if( entry->getParam1().c_str()==NULL ) return false;

    // always all options for standard menu options :]
    if( strstr(entry->getParam1().c_str(), "MAKE")==NULL ) return true;
    
    if( pc->IsGM() ) return true; // gee, GMs can do *anything*...

    if( entry->getParam2().c_str()==NULL ) return false;

	cMakeItem* mi = getcMakeItem( atoi(entry->getParam2().c_str()) );
	if (mi==NULL) return false;

	return mi->checkReq(pc, true);
}

/*!
\brief Clean a String
\author Xanathar
\return char*
\param s itemmenu number to be loaded
*/
std::string cAddMenu::cleanString( std::string s )
{
	char *p = strstr(s.c_str(), " ");
	if (p!=NULL) 
		return std::string( p+1 );
	return s;
}



void cAddMenu::handleButton( NXWCLIENT ps, cClientPacket* pkg  )
{
	
	SERIAL button;
	if( isIconList( pkg->cmd ) )
		button = ((cPacketResponseToDialog*)pkg)->index.get()-1;
	else {
		button = ((cPacketMenuSelection*)pkg)->buttonId.get();
		if( button!=MENU_CLOSE )
			button = ((cMenu*)oldmenu->type)->rc_button[button];
	}

	if( button<=INVALID )
		return;

	commands[button].execute( ps->toInt() );
	
}

cServerPacket* cAddMenu::build()
{

	oldmenu->serial=this->serial;
	oldmenu->id=this->id;
	
	return oldmenu->build();
}




/*!
\brief Check if the player is skilled enough and have requested items
\return bool can or can't 
\param pc the player
\param inMenu if write a sysmessage on error
\todo Add message if haven't enough item..
*/
bool cMakeItem::checkReq( P_CHAR pc, bool inMenu )
{
    VALIDATEPCR(pc,false);

    if (pc->IsGM()) return true;

    if( (skillToCheck!=INVALID) && (pc->skill[skillToCheck]<minskill) ) {
        if (!inMenu) 
			pc->sysmsg(TRANSLATE("You're not enough skilled"));
        return false;
    }

	std::vector< cRawItem >::iterator iter( reqitems.begin() ), end( reqitems.end() );
	for( ; iter!=end; iter++ ) {
        if( iter->id!=0 ) {
           if( pc->CountItems( iter->id, iter->color)<iter->number ) 
			   return false;
        }
    }
    return true;
}



cRawItem::cRawItem( std::string& s ) 
{
   	int params[3];
   	fillIntArray( (char*)s.c_str(), params, 3, 0 );
    id = params[0];
    color = params[1];
    number = params[2];
    if(number < 1)
		number = 1;
}

cRawItem::~cRawItem() 
{
}

cMakeItem::cMakeItem()
{
    mana = stam = hit = 0;
    skillToCheck = INVALID;
    minskill = 0; maxskill = 1000;
    reqspell = INVALID;
}

cMakeItem::~cMakeItem()
{
}

cMakeItem* getcMakeItem( SERIAL n )
{

	static std::map< SERIAL, class cMakeItem > make_items; //!< make items cached

	std::map< SERIAL, cMakeItem >::iterator mi_iter( make_items.find( n ) );
	if( mi_iter!=make_items.end() ) {
		return &mi_iter->second;
	}

    // we're here so no makeitem number n has been loaded yet
    // so search it :]
    cScpIterator* iter = Scripts::Create->getNewIterator( "SECTION MAKE %d", n );

    if (iter==NULL) 
		return NULL;

    cMakeItem mi;
    int reqres = 0;

	std::string script1, script2;
    do {
		iter->parseLine( script1, script2 );
		if( script1=="DO" ) {
		    if( script2.size() < 4) {
		        WarnOut("Malformed DO command\n");
		        return NULL;
		    }
		    char *p = strstr(script2.c_str(), " ");
		    if (p==NULL) {
		        mi.command.command = script2;
		        mi.command.param = "";
		    } else {
		        *p = '\0';
		        mi.command.command = script2;
		        mi.command.param = p+1;
		    }
		} else if ( script1 =="SKILL" ) {
		    mi.skillToCheck = str2num(script2);
		} else if ( script1=="MINSKILL" ) {
		    mi.minskill = str2num(script2);
		} else if ( script1=="MAXSKILL" ) {
		    mi.maxskill = str2num(script2);
		} else if ( script1=="MANA" ) {
		    mi.mana = str2num(script2);
		} else if ( script1=="STAM" ) {
		    mi.stam = str2num(script2);
		} else if ( script1=="REQSPELL" ) {
		    mi.reqspell = str2num(script2);
		} else if ( script1=="HP" ) {
		    mi.hit = str2num(script2);
		}  else if ( script1=="REQ" ) {
   		    mi.reqitems.push_back( cRawItem(script2) );
			reqres++;
		}
    } while( script1!="}" );

    make_items.insert( make_pair( n, mi ) );

    return getcMakeItem(n);

}


/*!
\brief executes a "MAKE" command
\author Xanathar
\param pc player who do make
\param n item number
*/
void execMake( P_CHAR pc, int n )
{
    
	VALIDATEPC( pc );

    cMakeItem* mi = getcMakeItem(n);
	if( mi==NULL )
		return;

    if (pc->dead) {
        pc->sysmsg(TRANSLATE("Ever thought an ethereal soul can't really do some actions ?"));
        return;
    }

    NXWCLIENT cli = pc->getClient();
    if(cli==NULL) return;

    NXWSOCKET sock = cli->toInt();
    if(sock<=INVALID) return;

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

	std::vector< cRawItem >::iterator iter( mi->reqitems.begin() ), end( mi->reqitems.end() );
	for( ; iter!=end; ++iter ) {

        if( iter->id > 0) {
           if( pc->CountItems( iter->id, iter->color)< iter->number ) {
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

	for( iter=mi->reqitems.begin(); iter!=end; ++iter ) {
        pc->delItems( iter->id, iter->number, iter->color );
    }

    if( !pc->checkSkill((Skill)mi->skillToCheck, mi->minskill, mi->maxskill) ) {
        pc->sysmsg(TRANSLATE("You failed"));
        return;
    }

    pc->damage( mi->hit,  DAMAGE_PURE, STAT_HP );
    pc->damage( mi->mana, DAMAGE_PURE, STAT_MANA );
    pc->damage( mi->stam, DAMAGE_PURE, STAT_STAMINA );

	mi->command.execute( sock );

}














/*!
\brief Shows the add menus
\author Endymion
\param pc the char to send the menu to
\param menu the itemmenu number
*/
void showAddMenu( P_CHAR pc, int menu )
{
	VALIDATEPC( pc );
	
	if( pc->custmenu!=INVALID )
		Menus.removeMenu( pc->custmenu, pc );

	P_MENU pm = Menus.insertMenu( new cAddMenu( menu, pc ) );
	pc->custmenu = pm->serial;
	pm->show( pc );

}










