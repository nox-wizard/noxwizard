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
\return char*
\param s itemmenu number to be loaded
*/
std::string cMakeMenu::cleanString( std::string s )
{
	char *p = strstr(s.c_str(), " ");
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

    int minskl=0; // To calculate minimum skill required

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
					}

					type=MAKE_NEED_MENUORITEM;
				}
				break;
			case MAKE_NEED_MENUORITEM: //MAKEMENU 3  or ADDITEM $item_golden_ringmail_tunic
				if( ( lha=="MAKEMENU" ) || ( lha=="ADDITEM" ) ) {
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

    
	//we're here -> we can do the stuff ;]

    // sequence is :
    //  - item removals
    //  - skill check
    //  - stat removals
    //  - do what should be done

	for( int j=0; j<2; ++j ) {
		cRawItem& raw = mi->reqitems[j];
		if( raw.id!=0 )
	        pc->delItems( raw.id, raw.number, raw.color );
	}

    if( !pc->checkSkill((Skill)mi->skillToCheck, mi->minskill, mi->maxskill) ) {
        pc->sysmsg(TRANSLATE("You failed"));
        return;
    }

	mi->command->execute( ps->toInt() );

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

	if( pc->custmenu!=INVALID )
		Menus.removeMenu( pc->custmenu, pc );

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





#ifdef UAZ

/*!
\author Magius(CHE) & Duke
\param s socket to calc the rank to
\param skill skill index to use
\return the item's rank
\brief Calculate item rank based on player's skill
*/
int Skills::CalcRank(NXWSOCKET s,int skill)
{
/*	if (s < 0 || s >= now)
		return 1;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pc,itemmake[s].minrank);

	int rk_range,rank;
	double sk_range,randnum,randnum1;

	rk_range=itemmake[s].maxrank-itemmake[s].minrank;
	sk_range=(double) 50.00+pc->skill[skill]-itemmake[s].minskill;
    
	if (sk_range<=0) rank=itemmake[s].minrank;
	else if (sk_range>=1000) rank=itemmake[s].maxrank;

	randnum=static_cast<double>(rand()%1000);

	if (randnum<=sk_range) rank=itemmake[s].maxrank;
	else
	{ 
		if (SrvParms->skilllevel==11)
			randnum1=(double) (rand()%1000);
		else
			randnum1=(double) (rand()%1000)-((randnum-sk_range)/(11-SrvParms->skilllevel));
        
		rank=(int) (randnum1*rk_range)/1000;
		rank+=itemmake[s].minrank-1;

		if (rank>itemmake[s].maxrank) rank=itemmake[s].maxrank;
		if (rank<itemmake[s].minrank) rank=itemmake[s].minrank;
	}

	return rank;*/
}

/*!
\author Magius(CHE) & Duke
\brief Modify variables base on item's rank
\param s socket of the crafter
\param i serial of the item to modify
\param rank rank of the item
*/
void Skills::ApplyRank(NXWSOCKET s, int i,int rank)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	const P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPI(pi);

//	char tmpmsg[512];
//	*tmpmsg='\0';
	
	if(SrvParms->rank_system==0) return;
	
	if(SrvParms->rank_system==1)
	{
		pi->rank=rank;
// Variables to change: LODAMAGE,HIDAMAGE,ATT,DEF,HP,MAXHP
		if (pi->lodamage>0) pi->lodamage=(rank*pi->lodamage)/10;
		if (pi->hidamage>0) pi->hidamage=(rank*pi->hidamage)/10;
		if (pi->att>0) pi->att=(rank*pi->att)/10;
		if (pi->def>0) pi->def=(rank*pi->def)/10;
		if (pi->hp>0) pi->hp=(rank*pi->hp)/10;
		if (pi->maxhp>0) pi->maxhp=(rank*pi->maxhp)/10;

		switch(rank)
		{
			case 1: sysmessage(s,TRANSLATE("You made an item with no quality!")); break;
			case 2: sysmessage(s,TRANSLATE("You made an item very below standard quality!"));break;
			case 3: sysmessage(s,TRANSLATE("You made an item below standard quality!"));break;
			case 4: sysmessage(s,TRANSLATE("You made a weak quality item!"));break;
			case 5: sysmessage(s,TRANSLATE("You made a standard quality item!"));break;
			case 6: sysmessage(s,TRANSLATE("You made a nice quality item!"));break;
			case 7: sysmessage(s,TRANSLATE("You made a good quality item!"));break;
			case 8: sysmessage(s,TRANSLATE("You made a great quality item!"));break;
			case 9: sysmessage(s,TRANSLATE("You made a beautiful quality item!"));break;
			case 10: sysmessage(s,TRANSLATE("You made a perfect quality item!"));break;
		}

///		sysmessage(s,tmpmsg); // O_o why !?
	} 
	else pi->rank=rank;
}


P_ITEM Skills::MakeMenuTarget(NXWSOCKET s, int x, int skill, int amount)
{
	if ( s < 0 || s >= now ) //Luxor
		return NULL;
	
	NXWCLIENT ps = getClientFromSocket( s );
	if(ps==NULL) return NULL;
    P_CHAR pc = ps->currChar();
	VALIDATEPCR(pc, NULL);
	
	Location charpos= pc->getPosition();
    int rank=10; // For Rank-System --- Magius§(çhe)
    
	//cMMT *targ = cMMT::factory(skill);
	//if(!targ) return;

	P_ITEM pack = pc->getBackpack();
    int amt=pc->getAmount(itemmake[s].Mat1id, itemmake[s].Mat1color, true );
    if ( !pc->IsGM() && ( amt < itemmake[s].needs || !ISVALIDPI(pack) ) )
    {
        pc->sysmsg(TRANSLATE("You do not have enough resources anymore!!"));
		//delete targ;
        return NULL;
    }

    if(itemmake[s].Mat2id>0) {
		int amt=pc->getAmount(itemmake[s].Mat2id, itemmake[s].Mat2color, true );
		if ( !pc->IsGM() && ( amt < itemmake[s].needs || !ISVALIDPI(pack) ) )
		{
	        pc->sysmsg(TRANSLATE("You do not have enough resources anymore!!"));
			//delete targ;
			return NULL;
		}
	}

/*
    By Polygon:
    When doing cartography, check if empty map is still there
*/
    if (skill == CARTOGRAPHY)   // Is it carto?
    {
        if (!Skills::HasEmptyMap(DEREF_P_CHAR(pc)))   // Did the map disappear?
        {
            pc->sysmsg(TRANSLATE("You don't have your blank map anymore!!"));
			//delete targ;
            return NULL;
        }
    }
//  END OF: By Polygon


	int mat1needed = 1;
	int mat2needed = 1;

    if(pc->making==999)
		{
		}
    else
		if(!pc->checkSkill( static_cast<Skill>(skill), itemmake[s].minskill, itemmake[s].maxskill) && !pc->IsGM()) //GM cannot fail! - AntiChrist
    {
        // Magius(CHE) §
        // With these 2 lines if you have a resouce item with
        // Amount=1 and fail to work on it, this resouce will be
        // Removed anyway. So noone can increase his skill using
        // a single resource item.
		
		//targ->playbad(s);
		//targ->failmsg(s);

		mat1needed = (itemmake[s].needs/2 > 1) ? itemmake[s].needs/2 : 1;
		mat2needed = (itemmake[s].needs/2 > 1) ? itemmake[s].needs/2 : 1;

		switch(skill)
		{

			case BOWCRAFT:	//Luxor bug fix
				pack->DeleteAmount( mat1needed, itemmake[s].Mat1id, itemmake[s].Mat1color );
				if (itemmake[s].Mat2id > 0)
						pack->DeleteAmount( mat2needed, itemmake[s].Mat2id, itemmake[s].Mat2color );
				break;
			
			case CARTOGRAPHY:	//Polygon: Do sounds, message and deletion if carto fails
				Skills::DelEmptyMap(DEREF_P_CHAR(pc));
				pc->playSFX(0x0249);
				pc->sysmsg( TRANSLATE("You scratch on the map but the result is unusable"));
				break;

			/*
			case CARPENTRY:     
			case INSCRIPTION:
			case TAILORING:     
			case TINKERING:
			case BLACKSMITHING:*/
			default:
				pack->DeleteAmount( mat1needed, itemmake[s].Mat1id, itemmake[s].Mat1color );
				///Endymion TEST
				if (itemmake[s].Mat2id > 0)
						pack->DeleteAmount( mat2needed, itemmake[s].Mat2id, itemmake[s].Mat2color );
				///Endymion END TEST
				break;	//Luxor bug fix
		}
		itemmake[s].Mat2id=0;
		Zero_Itemmake(s);
	}
	else 
	{
		mat1needed = (itemmake[s].needs > 1) ? itemmake[s].needs : 1;
		mat2needed = (itemmake[s].needs > 1) ? itemmake[s].needs : 1;

		switch(skill)
		{	
			case INSCRIPTION: //don't use default, cauz we delete 1 scroll
				pack->DeleteAmount( 1, itemmake[s].Mat1id ); break;
			case BOWCRAFT:	//Luxor bug fix
				pack->DeleteAmount( mat1needed, itemmake[s].Mat1id, itemmake[s].Mat1color );
				if (itemmake[s].Mat2id > 0)
						pack->DeleteAmount( mat2needed, itemmake[s].Mat2id, itemmake[s].Mat2color );
				break;
				// Polygon: Delete empty map for carto
			case CARTOGRAPHY:	
				if (!DelEmptyMap(DEREF_P_CHAR(pc))) {
					//delete targ;
					return NULL; 
				}
			
			/*
			case BLACKSMITHING:
			case CARPENTRY:	
			case TAILORING:
			case TINKERING:*/
			default:
				pack->DeleteAmount( mat1needed, itemmake[s].Mat1id, itemmake[s].Mat1color );
				///Endymion TEST
				if (itemmake[s].Mat2id > 0)
						pack->DeleteAmount( mat2needed, itemmake[s].Mat2id, itemmake[s].Mat2color );
				///Endymion END TEST
				break;
		}
		itemmake[s].Mat1id=0;
		itemmake[s].Mat2id=0;
		P_ITEM pi = item::CreateFromScript( x, pack, amount );
        if (!ISVALIDPI(pi))
        {
            LogWarning("bad script item # %d(Item Not found).", x);
			//delete targ;
            return NULL; //invalid script item
        }
		//int c=DEREF_P_ITEM(pi);
        // Starting Rank System Addon, Identify Item and Store the Creator Name- by Magius(CHE)

        //if( pi->getSecondaryName()=="#" )
            //pi->setCurrentName(pi->getSecondaryName()); // Item identified! - }
        
		rank=10;
        ApplyRank(s,DEREF_P_ITEM(pi),rank);

        if(!pc->IsGM())	//AntiChrist - do this only if not a GM! bugfix - to avoid "a door mixed by GM..."
        {
            pi->creator = pc->getCurrentName(); // Memorize Name of the creator
            if (pc->skill[skill]>950)
                pi->madewith=skill+1; // Memorize Skill used
            else
                pi->madewith=0-skill-1; // Memorize Skill used ( negative not shown )
        }
        else
        {
            pi->creator = "";
            pi->madewith=0;
        }

        pi->magic=1; // JM's bugfix

        if (skill == BLACKSMITHING          // let's see if we can make exceptional quality
            && SrvParms->rank_system==0)    // this would interfer with the rank system
        {
            pi->rank=30; // Ripper..used for item smelting
			switch( pi->color() )
			{
				case 0x0000 :	// Silver
					pi->smelt = 2;
					break;
				case 0x0466 :	// Gold
					pi->smelt = 3;
					break;
				case 0x0150 : // Agapite
					pi->smelt = 4;
					break;
				case 0x0386 : // Shadow
					pi->smelt = 5;
					break;
				case 0x0191 : // Mythril
					pi->smelt = 6;
					break;
				case 0x02E7 : // Bronze
					pi->smelt = 7;
					break;
				case 0x022F : // Verite
					pi->smelt = 8;
					break;
				case 0x02C3 : // Merkite
					pi->smelt = 9;
					break;
				case 0x046E : // Copper
					pi->smelt = 10;
					break;
				default:	  // Iron
					pi->smelt = 1;
			}

            int modifier = 1;
            int skmin = itemmake[s].minskill;
            int skcha = pc->skill[skill];
            int skdif = skcha-skmin;
            if ( ( skdif>0 ) && ( skmin!=1000 )     // just to be sure ...
                && skmin >= 500                     // only for higher level items
                && skdif > (rand()%(1000-skmin)))   // 1st test depends on how far the players
            {                                       // skill is above the requirements

                if (!(rand()%10)) modifier=10;      // 10% chance for 10% more
                if (!(rand()%100)) modifier=5;      // 1% chance for 20% more
                if (!(rand()%1000)) modifier=2;     // only one in a thousand for 50% more
                if (modifier > 1)
                {
					std::string p1, p2, p3, p4;
                    UI32 spaceleft=0;                // for space left in name field
                    if ( pi->getCurrentName()=="#" )
                    {
                        char tmp[100];
                        pi->getName(tmp);               // get the name from tile data
                        pi->setCurrentName(tmp);
                    }
                    spaceleft = pi->getCurrentName().length() -pi->getCurrentName().length();
                    switch (modifier)
                    {
						case 10: 
							p1 = TRANSLATE(" of high quality");
							p2 = TRANSLATE(" of h.q.");
							p3 = TRANSLATE("(hQ)");
							break;
						case  5: 
							p1 = TRANSLATE(" of very high quality");
							p2 = TRANSLATE(" of v.h.q.");
							p3 = TRANSLATE("(vhQ)");
							break;
						case  2: 
							p1 = TRANSLATE(" of exceptional quality");
							p2 = TRANSLATE(" of e.q.");
							p3 = TRANSLATE("(eQ)");
							break;
                    }
                    if (p1.length() < spaceleft)
						p4 = p1;
                    else if (p2.length() < spaceleft)
						p4 = p2;
                    else if (p3.length() < spaceleft)
						p4 = p3;
					
					pi->setCurrentName( pi->getCurrentName() + p4);

                    pi->hp += pi->hp / modifier;
                    pi->maxhp = pi->hp;
                    pi->def += pi->def / modifier;
                    pi->att += pi->att / modifier;
                    pi->lodamage += pi->lodamage / modifier;
                    pi->hidamage += pi->hidamage / modifier;
                }
            }
        }

        if(pc->making==999)
            pc->making=DEREF_P_ITEM(pi); // store item #
        else
            pc->making=0;
		
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

        if(!pc->making) 
			pc->sysmsg(TRANSLATE("You create the item and place it in your backpack."));

		statwindow(pc,pc);
        Zero_Itemmake(s);

        /*
			Code added by Polygon
			Creates a key for every chest-container
			and puts it in the chest
		*/
        if( pi->IsChest())
        {
            pi->more1 = pi->getSerial().ser1;       // Store the serial number in the more-value
            pi->more2 = pi->getSerial().ser2;       // Needed as unique lock-ID
            pi->more3 = pi->getSerial().ser3;
            pi->more4 = pi->getSerial().ser4;
            P_ITEM key=item::CreateFromScript( 339, pi);
			if(!ISVALIDPI(key))
			{
				//delete targ;
				return NULL;
			}
			//int c=DEREF_P_ITEM(key);
            key->type = ITYPE_KEY;              // Item is a key
            key->more1 = pi->more1;     // Copy the lock-number to the keys more-variable
            key->more2 = pi->more2;     // to make it fit the lock
            key->more3 = pi->more3;
            key->more4 = pi->more4;
            key->creator = pc->getCurrentName();    // Store the creator
            key->Refresh();                         // Refresh it
            pc->sysmsg( TRANSLATE("You create a corresponding key and put it in the chest"));
        }
//      End of: By Polygon
/*
        By Polygon:
        Give the map the correct attributes so that it
        shows the correct part of the map and has correct size
*/
        if ((x>70000) && (x < 70013)) // Is it a map?
        {
            int tlx, tly, lrx, lry; // Tempoarly storing of topleft and lowright x,y
            if (x<70004)    // Is it a detail map? do +-250 from pos
            {
                tlx = charpos.x - 250;
                tly = charpos.y - 250;
                lrx = charpos.x + 250;
                lry = charpos.y + 250;
            }
            else if (x<70007)   // Is it a regional map? do +-500 from pos
            {
                tlx = charpos.x - 500;
                tly = charpos.y - 500;
                lrx = charpos.x + 500;
                lry = charpos.y + 500;
            }
            else if (x<70010)   // Is it a land map? do +-1000 from pos
            {
                tlx = charpos.x - 1000;
                tly = charpos.y - 1000;
                lrx = charpos.x + 1000;
                lry = charpos.y + 1000;
            }
            else                // So it's a worldmap eh?
            {
                tlx = 0;
                tly = 0;
                lrx = 0x13FF;
                lry = 0x0FFF;
            }
            // Check if we are over the borders and correct errors
            if (tlx < 0)    // Too far left?
            {
                lrx -= tlx; // Add the stuff too far left to the right border (tlx is neg. so - and - gets + ;)
                tlx = 0;    // Set tlx to correct value
            }
            else if (lrx > 0x13FF) // Too far right?
            {
                tlx -= lrx - 0x13FF;    // Subtract what is to much from the left border
                lrx = 0x13FF;   // Set lrx to correct value
            }
            if (tly < 0)    // Too far top?
            {
                lry -= tly; // Add the stuff too far top to the bottom border (tly is neg. so - and - gets + ;)
                tly = 0;    // Set tly to correct value
            }
            else if (lry > 0x0FFF) // Too far bottom?
            {
                tly -= lry - 0x0FFF;    // Subtract what is to much from the top border
                lry = 0x0FFF;   // Set lry to correct value
            }
            // Set the map data
            pi->more1=  (tlx)>>8;     // Set topleft x
            pi->more2=  (tlx)%256;    //  --- " ---
            pi->more3=  (tly)>>8;     // Set topleft y
            pi->more4=  (tly)%256;    //  --- " ---
            pi->moreb1= (lrx)>>8;    // Set lowright x
            pi->moreb2= (lrx)%256;   //  --- " ---
            pi->moreb3= (lry)>>8;    // Set lowright y
            pi->moreb4= (lry)%256;   //  --- " ---
        }
//      END OF: By Polygon
		return pi;
	}
	return NULL;
}

/*!
\brief Menus for playermade objects
\param s socket of the crafter
\param m menu to use
\param skill skill index being used
*//*
void Skills::MakeMenu(NXWSOCKET s, int m, int skill)
{
	if ( s < 0 || s >= now || skill < 0 || skill >= TRUESKILLS )	//Luxor
		return;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
    char script1[1024]; //this proc goes heavy on stack space >:[
    char script2[1024];
    int total, i;
    char lentext;
    char sect[512];
    char gmtext[30][257];
    cScpIterator* iter = NULL;
    int gmid[30];
    int gmnumber=0;
    int gmindex;
    int minres=0; // To calculate minimum resources required! By Magius(CHE) for Rank System
    int minskl=0; // To calculate minimum skill required! By Magius(CHE) for Rank System
    int tmpgmnumber=0; // By Magius(CHE) for Rank System
    make_st *imk=&itemmake[s];
//    int pScp = 0;
    pc->making=skill;

    sprintf(sect, "SECTION MAKEMENU %i", m);

    iter = Scripts::Create->getNewIterator(sect);

    if (iter==NULL) return;

    gmindex=m;
    strcpy(script1, iter->getEntry()->getFullLine().c_str());
    strcpy(gmtext[0],script1);

    iter->parseLine(script1, script2); //thiz one to discard the { !!! >:]

    iter->parseLine(script1, script2);

    int loopexit=0;
    do
    {
        if (script1[0]!='}')
        {
            char tmp[99];
            gmnumber++;
            tmpgmnumber++; // Magius(CHE)
            gmid[gmnumber]=hex2num(script1);
            strcpy(gmtext[gmnumber], script2);

            iter->parseLine(script1, script2);
            if (strcmp(script1,"RESOURCE"))
            {
                sprintf(tmp,"create.scp, MAKEMENU %i: Expected 'RESOURCE <num>' after '%s'!", m,gmtext[gmnumber]);
                LogWarning(tmp);
                safedelete(iter);
                return;
            }
            imk->needs=str2num(script2);

            iter->parseLine(script1, script2);
            imk->minskill=str2num(script2);
            imk->maxskill=imk->minskill*SrvParms->skilllevel; // by Magius(CHE)
            if (strcmp(script1,"SKILL"))
            {
                sprintf(tmp,"create.scp, MAKEMENU %i: Expected 'SKILL <num>' after 'RESOURCE %i'!", m,imk->needs);
                LogWarning(tmp);
                safedelete(iter);
                return;
            }
            {
                char append[20];
                sprintf(append, " - [%d/%d.%d]", imk->needs,imk->minskill/10,imk->minskill%10);
                strcat(gmtext[gmnumber],append);
            }
            if (imk->maxskill<200) imk->maxskill=200;
            if ((imk->has<imk->needs) || ((imk->has2) &&
                (imk->has2<imk->needs)) || (pc->skill[skill]<imk->minskill))
                gmnumber--;

            iter->parseLine(script1, script2);
            imk->number=str2num(script2);
            if (minres>imk->needs || !minres) minres=imk->needs;
            if (minskl>imk->minskill || !minskl) minskl=imk->minskill;
            if (strcmp(script1,"ADDITEM") && strcmp(script1,"MAKEMENU"))
            {
                sprintf(tmp,"create.scp, MAKEMENU %i: Expected 'ADDITEM/MAKEMENU <num>' after 'SKILL %i'!", m,imk->minskill);
                LogWarning(tmp);
                safedelete(iter);
                return;
            }

            iter->parseLine(script1, script2);
            if (!strcmp(script1,"RANK")) // Item Rank System - by Magius(CHE)
            {
                gettokennum(script2, 0);
                imk->minrank=str2num(gettokenstr);
                gettokennum(script2, 1);
                imk->maxrank=str2num(gettokenstr);
                iter->parseLine(script1, script2);
            } else
            { // Set maximum rank if the item is not ranked!
                imk->minrank=10;
                imk->maxrank=10;
            }
            if (SrvParms->rank_system==0)
            {
                imk->minrank=10;
                imk->maxrank=10;
            }
        }
    }
    while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );


    safedelete(iter);
    if (!gmnumber)
    {
        sysmessage(s,TRANSLATE("You aren't skilled enough to make anything with what you have."));
        return;
    }
    sprintf(temp, "%i: %s", m, gmtext[0]);
    lentext=sprintf(gmtext[0], "%s", temp);
    total=9+1+lentext+1;
    for (i=1;i<=gmnumber;i++)
    {
        total+=4+1+strlen(gmtext[i]);
    }
	unsigned char gmprefix[10]="\x7C\x00\x00\x01\x02\x03\x04\x00\x64";
    gmprefix[1]=total>>8;
    gmprefix[2]=total%256;
    gmprefix[3]= pc->getSerial().ser1;
    gmprefix[4]= pc->getSerial().ser2;
    gmprefix[5]= pc->getSerial().ser3;
    gmprefix[6]= pc->getSerial().ser4;
    gmprefix[7]=(gmindex+MAKEMENUOFFSET)>>8;
    gmprefix[8]=(gmindex+MAKEMENUOFFSET)%256;
    Xsend(s, gmprefix, 9);
    Xsend(s, &lentext, 1);
    Xsend(s, gmtext[0], lentext);
    lentext=gmnumber;
    Xsend(s, &lentext, 1);
	unsigned char gmmiddle[5]="\x00\x00\x00\x00";
    for (i=1;i<=gmnumber;i++)
    {
        gmmiddle[0]=gmid[i]>>8;
        gmmiddle[1]=gmid[i]%256;
        Xsend(s, gmmiddle, 4);
        lentext=strlen(gmtext[i]);
        Xsend(s, &lentext, 1);
        Xsend(s, gmtext[i], lentext);
    }

}*/

#endif

