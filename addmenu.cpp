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
#include "skills.h"

cMakeMenu::cMakeMenu( SERIAL section ) : cBasicMenu( MENUTYPE_CUSTOM )
{
	oldmenu = new cOldMenu();
	this->section=section;
}

cMakeMenu::cMakeMenu( SERIAL section, P_CHAR pc, int skill, UI16 firstId, COLOR firstColor, UI16 secondId, COLOR secondColor ) : cBasicMenu( MENUTYPE_CUSTOM )
{
	oldmenu = new cOldMenu();
	this->section=section;
	this->skill=skill;

	mat[0].id = firstId;
	mat[0].color= firstColor;


	mat[1].id = secondId;
	mat[1].color = secondColor;

	loadFromScript( pc );
}

cMakeMenu::~cMakeMenu()
{
	if( oldmenu!=NULL )
		delete oldmenu;
	std::vector<cMakeItem*>::iterator iter( makeItems.begin() ), end( makeItems.end() );
	for( ; iter!=end; iter++ )
		if( (*iter)!=NULL )
			delete (*iter);

}

/*!
\brief Clean a String
\author Xanathar
\param s itemmenu number to be loaded
*/
std::string cMakeMenu::cleanString( std::string s )
{
	const char *p = strstr(s.c_str(), " ");
	if (p!=NULL) 
		return std::string( p+1 );
	return s;
}

void cMakeMenu::loadFromScript( P_CHAR pc )
{
	VALIDATEPC(pc);

	NXWCLIENT ps = pc->getClient();
	if( ps==NULL ) return;

    cScpIterator* iter = Scripts::Create->getNewIterator("SECTION MAKEMENU %i", section);
    if (iter==NULL) return;

	std::vector<std::string> names; //name
	std::vector<std::string> models; //models
    
	this->mat[0].number = ( mat[0].id!=0 )? pc->CountItems( mat[0].id, mat[0].color ) : 0;
	this->mat[1].number = ( mat[1].id!=0 )? pc->CountItems( mat[1].id, mat[1].color ) : 0;

	//da passare a checkReq PDPD
	

	cMakeItem* imk = NULL;
    pc->making=skill;

	oldmenu->style=MENUTYPE_ICONLIST;

/* makemenu menu
SECTION MAKEMENU 1
{
    Blacksmithing
    1416 Armor
    RESOURCE 10
    SKILL 500
    MAKEMENU 3
    13BA Weapons
    RESOURCE 3
    SKILL 100
    MAKEMENU 4
    1BC3 Shields
    RESOURCE 8
    SKILL 250
    MAKEMENU 2
    09ed Misc
    RESOURCE 6
    SKILL 150
    MAKEMENU 105
}
*/
/* makemenu item
SECTION MAKEMENU 2002
{
    Golden Ring Mail
    13F2 Gloves
    RESOURCE 10
    SKILL 338
    ADDITEM $item_golden_ringmail_gloves
    13EF Sleeves
    RESOURCE 14
    SKILL 385
    ADDITEM $item_golden_ringmail_sleeves
    13F1 Leggings
    RESOURCE 16
    SKILL 408
    ADDITEM $item_golden_ringmail_leggings
    13ED Tunic
    RESOURCE 18
    SKILL 432
    ADDITEM $item_golden_ringmail_tunic
}
*/

enum MakeParamType {
MAKE_NEED_NAME = 0, //Blacksmithing
MAKE_NEED_INFO, //1416 Armor
MAKE_NEED_RESOURCE, //RESOURCE 10
MAKE_NEED_SKILL, //SKILL 500
MAKE_NEED_MENUORITEM, //MAKEMENU 3  or ADDITEM $item_golden_ringmail_tunic
};

	MakeParamType type = MAKE_NEED_NAME;
	bool error=false;
	bool canSee = true;
	int item = INVALID;

    int loopexit=0;
    do
    {
		std::string lha;
		std::string rha;
        
	    iter->parseLine(lha, rha);

		if( lha[0]=='{' )
			continue;

		if( lha[0]=='}' )
			break;

		switch( type ) {

			case MAKE_NEED_NAME: //Blacksmithing
				string2wstring( lha, oldmenu->title );
				type = MAKE_NEED_INFO;
				break;

			case MAKE_NEED_INFO: { //1416 Armor
				item++;
				names.push_back( rha );
				models.push_back( lha );
				type = MAKE_NEED_RESOURCE;
				break;
			}
			case MAKE_NEED_RESOURCE: //RESOURCE 10
		        if( lha!="RESOURCE" ) {
					if ( item > INVALID ) //Luxor
						LogWarning("create.xss, MAKEMENU %i: Expected 'RESOURCE <num>' after '%s'!", section, names[item].c_str() );
					error=true;
				}
				else {
					int needs = str2num(rha);

					imk = new cMakeItem();
					
					imk->skillToCheck=this->skill;
					imk->reqitems[0].id=mat[0].id;
					imk->reqitems[0].color=mat[0].color;
					imk->reqitems[0].number=needs;
					imk->reqitems[1].id=mat[1].id;
					imk->reqitems[1].color=mat[1].color;
					imk->reqitems[1].number=needs;

					type=MAKE_NEED_SKILL;
				}
				break;
			case MAKE_NEED_SKILL:  //SKILL 500
				if( lha!="SKILL" )
				{
					LogWarning("create.xss, MAKEMENU %i: Expected 'SKILL <num>' after RESOURCE !", section );
					error=true;
		        }
				else {

					imk->minskill=str2num(rha);
				    imk->maxskill=imk->minskill*SrvParms->skilllevel; // by Magius(CHE)
					if( imk->maxskill<200 )
						imk->maxskill=200;

					if( !imk->checkReq( pc, true, &this->mat[0] ) )
					{
						safedelete( imk );
						item--;
						names.pop_back();
						models.pop_back();
						canSee = false;
					}
					else {
						makeItems.push_back( imk );
						
						std::wstring w;
						char b[TEMP_STR_SIZE];
						if( mat[0].id!=0 ) {
							sprintf( b, "%s %s - [%d/%d.%d]", models[item].c_str(), names[item].c_str(), imk->reqitems[0].number, imk->minskill/10, imk->minskill%10 );
						}
						else {
							sprintf( b, "%s %s", models[item].c_str(), names[item].c_str() );
						}
						string2wstring( std::string( b ), w );
						oldmenu->addMenuItem( 0, item, w );
						type=MAKE_NEED_MENUORITEM;
						canSee = true;
					}
					type=MAKE_NEED_MENUORITEM;
				}
				break;
			case MAKE_NEED_MENUORITEM: //MAKEMENU 3  or ADDITEM $item_golden_ringmail_tunic
				if( ( lha=="MAKEMENU" ) || ( lha=="ADDITEM" ) ) {
					if( canSee )
						imk->command = new cScriptCommand( lha, rha );
					type = MAKE_NEED_INFO;
				}
				else {
					LogWarning("create.xss, MAKEMENU %i: Expected'MAKEMENU or ADDITEM after 'SKILL %i'!", section, imk->minskill );
					error=true;
				}
				break;
		}
	}
	while( !error  && (++loopexit < MAXLOOPS) );


    if( item<=0 ) {
        ps->sysmsg( TRANSLATE("You aren't skilled enough to make anything with what you have.") );
    }

	safedelete(iter);
	if( error ) {
		ConOut( "[ERROR] on cration of makemenu %d\n", section );
        return;
	}

}


cServerPacket* cMakeMenu::build()
{
	oldmenu->serial=this->serial;
	oldmenu->id=this->id;
	
	return oldmenu->build();
}

void cMakeMenu::handleButton( NXWCLIENT ps, cClientPacket* pkg  )
{
	SERIAL button;
	if( isIconList( pkg->cmd ) )
		button = ((cPacketResponseToDialog*)pkg)->index.get();
	else {
		button = ((cPacketMenuSelection*)pkg)->buttonId.get();
		if( button!=MENU_CLOSE )
			button = ((cMenu*)oldmenu->type)->getButton( button );
	}

	if( button<=MENU_CLOSE )
		return;

	execMake( ps, button-1 );
}


/*!
\brief executes a "MAKE" command
\author Xanathar
\param pc player who do make
\param n item number
*/
void cMakeMenu::execMake( NXWCLIENT ps, UI32 item )
{
    
	P_CHAR pc = ps->currChar();

    if( pc->dead ) {
        pc->sysmsg(TRANSLATE("Ever thought an ethereal soul can't really do some actions ?"));
        return;
    }

	cMakeItem* mi = makeItems[item];

	if( ( mi==NULL ) || ( mi->command==NULL ) )
		return;

	if( mi->command->command=="MAKEMENU" ) {
		Skills::MakeMenu( pc, str2num( mi->command->param ), skill, mat[0].id, mat[0].color, mat[1].id, mat[1].color );
		return;
	}

	if( pc->IsGM() ) {
		mi->command->execute( ps->toInt() );
		return;
	}

	if( !mi->checkReq( pc ) )
		return;

    
	bool failed = false;
    if( !pc->checkSkill((Skill)mi->skillToCheck, mi->minskill, mi->maxskill) ) {
		failed = true;
    }

	for( int j=0; j<2; ++j ) {
		cRawItem& raw = mi->reqitems[j];
		if( raw.id!=0 ) {

			UI16 matToDel = raw.number;
			if( failed )
				matToDel = ( matToDel/2>0 )? matToDel/2 : 1;

	        pc->delItems( raw.id, matToDel, raw.color );
		}
	}


	switch( skill )
	{
       	case MINING	:
			pc->playSFX(0x0054);
			break;
       	case BLACKSMITHING	:
			pc->playSFX(0x002A);
			break;
       	case CARPENTRY :
			pc->playSFX(0x023D);
			break;
       	case INSCRIPTION :
			pc->playSFX(0x0249);
			break;
       	case TAILORING :
			pc->playSFX(0x0248);
			break;
       	case TINKERING :
			pc->playSFX(0x002A);
			break;
       	case CARTOGRAPHY :
			pc->playSFX(0x0249);
		break;
	}

	pc->setObjectDelay();

	std::string script;
	std::string amount;
	splitLine( mi->command->param, script, amount );

	if( !failed ) {
		P_ITEM pi = item::CreateFromScript( str2num( script ), pc->getBackpack(), (amount!="")? str2num( amount ) : INVALID );
		VALIDATEPI(pi);

		ps->sysmsg(TRANSLATE("You create the item and place it in your backpack."));

		pi->magic = 1;

        pi->creator = pc->getCurrentName(); // Memorize Name of the creator
        if (pc->skill[skill]>950)
            pi->madewith=skill+1; // Memorize Skill used
        else
            pi->madewith=0-skill-1; // Memorize Skill used ( negative not shown )

	}
	else {
        ps->sysmsg(TRANSLATE("You failed"));
	}


}

/*!
\brief Check if the player is skilled enough and have requested items
\return bool can or can't 
\param pc the player
\param inMenu if write a sysmessage on error
\todo Add message if haven't enough item..
*/
bool cMakeItem::checkReq( P_CHAR pc, bool inMenu, cRawItem* def )
{

    if( pc->IsGM() ) 
		return true;

    if( (skillToCheck!=INVALID) && (pc->skill[skillToCheck]<minskill) ) {
        if( !inMenu ) 
			pc->sysmsg(TRANSLATE("You're not enough skilled"));
        return false;
    }

	for( int i=0; i<2; ++i ) {
        cRawItem& raw = reqitems[i];
		if( raw.id!=0 ) {
			bool have = ( def!=NULL )? (def[i].number>=raw.number) : ( pc->CountItems( raw.id, raw.color)>= raw.number );
			if( !have ) {
				if( !inMenu )
					pc->sysmsg(TRANSLATE("You've not enough resources"));
				return false;
			}
        }
    }


    return true;
}


void Skills::MakeMenu( P_CHAR pc, int m, int skill, P_ITEM first, P_ITEM second )
{

	Skills::MakeMenu( 
		pc, m, skill, ISVALIDPI(first)? first->getId() : 0, ISVALIDPI(first)? first->getColor() : 0,
		ISVALIDPI(second)? second->getId() : 0, ISVALIDPI(second)? second->getColor() : 0 
	);
	
}

void Skills::MakeMenu( P_CHAR pc, int m, int skill, UI16 firstId, COLOR firstColor, UI16 secondId, COLOR secondColor )
{

	if( ( skill < 0 ) || ( skill >= TRUESKILLS ) )	//Luxor
		return;
	
	VALIDATEPC(pc);

	if( pc->custmenu!=INVALID ) {
		Menus.removeMenu( pc->custmenu, pc );
		pc->custmenu=INVALID;
	}

	P_MENU pm = Menus.insertMenu( new cMakeMenu( m, pc, skill, firstId, firstColor, secondId, secondColor ) );
	pc->custmenu = pm->serial;
	pm->show( pc );


}






cAddMenu::cAddMenu( SERIAL section, P_CHAR pc ) : cMakeMenu( section )
{
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
		oldmenu->setParameters(10,(nOpt/10)+1);
		oldmenu->style = MENUTYPE_STONE;
	} else {
		oldmenu->style = MENUTYPE_ICONLIST;
	}

	string2wstring( entry->getFullLine(), oldmenu->title );

	nOpt = 0;

	while (entry->getFullLine().c_str()[0]!='}')
	{
		entry = iter->getEntry();
		if (entry->getFullLine().c_str()[0]!='}') {

			cScpEntry* entry2 = iter->getEntry();
   				
			std::wstring w;
				
			if (!bIcons) 
				string2wstring( cleanString( entry->getFullLine() ), w );
			else 
				string2wstring( entry->getFullLine(), w);

			oldmenu->addMenuItem( nOpt/10, nOpt%10, w );

			commands.push_back( cScriptCommand( entry2->getParam1(), entry2->getParam2() ) );
			nOpt++;

		}
	}

	safedelete(iter);
}





void cAddMenu::handleButton( NXWCLIENT ps, cClientPacket* pkg  )
{
	
	SERIAL button;
	if( isIconList( pkg->cmd ) )
		button = ((cPacketResponseToDialog*)pkg)->index.get();
	else {
		button = ((cPacketMenuSelection*)pkg)->buttonId.get();
		if( button!=MENU_CLOSE )
			button = ((cMenu*)oldmenu->type)->getButton( button );
	}

	if( button<=MENU_CLOSE )
		return;

	commands[button-1].execute( ps->toInt() );
	
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

cRawItem::cRawItem( SI32 id, COLOR color, UI32 number )
{
	this->id=id;
	this->color=color;
	this->number= (number>=1)? number : 1;
}

cRawItem::~cRawItem() 
{
}

cMakeItem::cMakeItem()
{
    skillToCheck = INVALID;
    minskill = 0; maxskill = 1000;
	command=NULL;
}

cMakeItem::~cMakeItem()
{
	if( command!=NULL )
		delete command;
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






