  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "basics.h"
#include "itemid.h"
#include "sndpkg.h"
#include "srvparms.h"
#include "amx/amxcback.h"
#include "race.h"
#include "magic.h"
#include "network.h"
#include "tmpeff.h"

//<Luxor>: for skills implementation by small
/*
#undef AMXTASTEID
#define AMXTASTEID "__nxw_sk_tasteid"
#undef AMXANATOMY
#define AMXANATOMY "__nxw_sk_anatomy"
#undef AMXEVALINT
#define AMXEVALINT "__nxw_sk_evint"
*/
#undef AMX_SKILLS_MAIN
#define AMX_SKILLS_MAIN "__nxw_sk_main"
//</Luxor>

#include "debug.h"

//int goldsmithing;
//1=iron, 2=golden, 3=agapite, 4=shadow, 5=mythril, 6=bronze, 7=verite, 8=merkite, 9=copper, 10=silver
int ingottype=0;//will hold number of ingot type to be deleted

inline void SetSkillDelay(CHARACTER cc) 
{ 
	P_CHAR pc_cc=MAKE_CHAR_REF(cc);
	VALIDATEPC(pc_cc);
	SetTimerSec(&pc_cc->skilldelay,SrvParms->skilldelay);
}

/*!
\author Magius(CHE) & Duke
\param s socket to calc the rank to
\param skill skill index to use
\return the item's rank
\brief Calculate item rank based on player's skill
*/
int Skills::CalcRank(NXWSOCKET s,int skill)
{
	if (s < 0 || s >= now)
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

	return rank;
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

	const P_ITEM pi=MAKE_ITEMREF_LR(i); // on error return

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

/*!
\brief Resets all values into itemmake[s]
\author Magius(CHE)
\param s socket of the crafter
*/
void Skills::Zero_Itemmake(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    itemmake[s].has=0;
    itemmake[s].has2=0;
    itemmake[s].needs=0;
    itemmake[s].minskill=0;
    itemmake[s].maxskill=0;
    itemmake[s].Mat1id=0;       // id of material used to make item
    itemmake[s].Mat1color=INVALID;   // color of material doesn't matter by default (Duke)
    itemmake[s].Mat2id=0;       // id of second material used to make item      (vagrant)
    itemmake[s].Mat2color=INVALID;   // color of material doesn't matter by default
    itemmake[s].minrank=10; // value of minum rank level of the item to create! - Magius(CHE)
    itemmake[s].maxrank=10; // value of maximum rank level of the item to create! - Magius(CHE)
    itemmake[s].number=0; // scp number of the item - used to regognize the item selected and apply Magius(CHE) rank!

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
        
		if( SrvParms->rank_system==1 ) 
			rank=CalcRank(s,skill);
        else 
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
        // End Rank System Addon

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
*/
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
    targetok[s]=1;

}

/*!
\author Luxor
\brief Implements the hiding skill
\param s socket requesting hiding
*/
void Skills::Hide(NXWSOCKET s)
{
	if ( s < 0 || s >= now )
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	NxwCharWrapper sc;
	P_CHAR pj = NULL;
	sc.fillCharsNearXYZ( pc->getPosition(), 4 );
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		pj = sc.getChar();
		if ( ISVALIDPC(pj) && pj->getSerial32() != pc->getSerial32() && !pj->IsHidden() && pc->losFrom(pj) ) {
			pc->sysmsg( TRANSLATE("There is someone nearby who prevents you to hide.") );
			return;
		}
	}

	pc->hideBySkill();
}

/*!
\author AntiChrist
\brief Stealths a char
\param s socket requesting stealthing
*/
void Skills::Stealth(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    if ( (pc->isMounting()) && (ServerScp::g_nStealthOnHorse==0) ) {
        sysmessage(s,TRANSLATE("You can't stealth on horse!"));
        return;
    }

    if (pc->hidden==0) {
        sysmessage(s,TRANSLATE("You must hide first."));
        return;
    }
    
    if (pc->skill[HIDING]<800)
    {
        sysmessage(s,TRANSLATE("You are not hidden well enough. Become better at hiding."));
        pc->stealth = INVALID;
	pc->unHide();
        return;
    }

    //XAN : more osi-like

    int def = pc->calcDef(0);
    int min = 0, max = 1000;
    if (def <= 5) { min = 0; max = 650; }
    if (def == 6) { min = 450; max = 700; }
    if (def == 7) { min = 500; max = 750; }
    if (def == 8) { min = 550; max = 800; }
    if (def == 9) { min = 600; max = 850; }
    if (def == 10) { min = 650; max = 900; }
    if (def == 11) { min = 650; max = 900; }
    if (def == 12) { min = 700; max = 950; }
    if (def == 13) { min = 700; max = 950; }
    if (def == 14) { min = 750; max = 1000; }
    if (def >= 15) { min = 800; max = 1100; }

    if (ServerScp::g_nStealthArLimit==0) {
        def = 0;
        min = 0;
        max = 1000;
    }
    // do not invert the two parts of the || operator otherwise
    // it stops raising skills when training with plate-armor ! :)
    if ((!pc->checkSkill(STEALTH, min, max))||(def>20))
    {
        pc->hidden = UNHIDDEN;
        pc->stealth = INVALID;
	pc->teleport( TELEFLAG_SENDWORNITEMS );
        return;
    }
    
    sysmessage(s,TRANSLATE("You can move %i steps unseen."), ((SrvParms->maxstealthsteps*pc->skill[STEALTH])/1000) );
    pc->stealth = 0; //AntiChrist -- init. steps already done
    pc->hideBySkill();
}

void Skills::PeaceMaking(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    int inst = Skills::GetInstrument( s );
    if( inst == INVALID )
    {
        pc->sysmsg( TRANSLATE("You do not have an instrument to play on!"));
        return;
    }

    if ( pc->checkSkill( PEACEMAKING, 0, 1000) && pc->checkSkill( MUSICIANSHIP, 0, 1000) )
    {
		Skills::PlayInstrumentWell(s, inst);
        pc->sysmsg( TRANSLATE("You play your hypnotic music, stopping the battle."));

		NxwCharWrapper sc;
		sc.fillCharsNearXYZ( pc->getPosition(), VISRANGE, true, false );
		for( sc.rewind(); !sc.isEmpty(); sc++ ) {
			P_CHAR pcm = sc.getChar();
			if( ISVALIDPC( pcm ) ) {
				if (pcm->war && pc->getSerial32()!=pcm->getSerial32())
                {
                    pcm->sysmsg(TRANSLATE("You hear some lovely music, and forget about fighting."));
					if (pcm->war) 
						pcm->toggleCombat();
                    pcm->targserial = INVALID;
					pcm->attackerserial = INVALID;
					pcm->ResetAttackFirst();
                }
            }
        }
    } 
	else
    {
		Skills::PlayInstrumentPoor(s, inst);
        pc->sysmsg( TRANSLATE("You attempt to calm everyone, but fail."));
    }

}

void Skills::PlayInstrumentWell(NXWSOCKET s, int i)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPI(pi);

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    switch(pi->id())
    {
    case 0x0E9C:    pc->playSFX( 0x0038);  break;
    case 0x0E9D:
    case 0x0E9E:    pc->playSFX( 0x0052);  break;
    case 0x0EB1:
    case 0x0EB2:    pc->playSFX( 0x0045);  break;
    case 0x0EB3:
    case 0x0EB4:    pc->playSFX( 0x004C);  break;
    default:
        LogError("switch reached default");
    }

}

void Skills::PlayInstrumentPoor(NXWSOCKET s, int i)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPI(pi);

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    switch(pi->id())
    {
    case 0x0E9C:    pc->playSFX( 0x0039);  break;
    case 0x0E9D:
    case 0x0E9E:    pc->playSFX( 0x0053);  break;
    case 0x0EB1:
    case 0x0EB2:    pc->playSFX( 0x0046);  break;
    case 0x0EB3:
    case 0x0EB4:    pc->playSFX( 0x004D);  break;
    default:
        LogError("switch reached default");
    }

}

int Skills::GetInstrument(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return INVALID;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pc,INVALID);
    
	P_ITEM pack= pc->getBackpack();
    VALIDATEPIR(pack,INVALID);

    NxwItemWrapper si;
	si.fillItemsInContainer( pack, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {
        P_ITEM pi=si.getItem();
		if ( ISVALIDPI(pi) && pi->IsInstrument() )
            return DEREF_P_ITEM(pi);
	}

    return INVALID;

}

/*!
\author Duke
\date 20/04/2000
\brief Helper function for DoPotion()
\param s socket of the crafter
\param regid reagent identifier
\param regamount amount of reagents
\param regname name of the reagent

checks if player has enough regs for selected potion and delets them
*/
static bool DoOnePotion(NXWSOCKET s, UI16 regid, UI32 regamount, char* regname)
{
	if ( s < 0 || s >= now ) //Luxor
		return false;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pc,false);
    
    bool success=false;
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    if (pc->getAmount(regid) >= regamount)
    {
        success=true;
        sprintf(temp, TRANSLATE("*%s starts grinding some %s in the mortar.*"), pc->getCurrentNameC(), regname);
        pc->emoteall( temp,1); // LB, the 1 stops stupid alchemy spam
        delequan(DEREF_P_CHAR(pc),regid,regamount);
    }
    else
        sysmessage(s, TRANSLATE("You do not have enough reagents for that potion."));

    return success;

}

/*!
\author Duke
\brief Determines regs and quantity, creates working sound
indirectly calls CreatePotion() on success
*/
void Skills::DoPotion(NXWSOCKET s, SI32 type, SI32 sub, P_ITEM pi_mortar)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	VALIDATEPI(pi_mortar);

	bool success=false;

	switch((type*10)+sub)
	{
		case 11: success=DoOnePotion(s,0x0F7B, 1,"blood moss");     break;//agility
		case 12: success=DoOnePotion(s,0x0F7B, 3,"blood moss");     break;//greater agility
		case 21: success=DoOnePotion(s,0x0F84, 1,"garlic");         break;//lesser cure
		case 22: success=DoOnePotion(s,0x0F84, 3,"garlic");         break;//cure
		case 23: success=DoOnePotion(s,0x0F84, 6,"garlic");         break;//greater cure
		case 31: success=DoOnePotion(s,0x0F8C, 3,"sulfurous ash");  break;//lesser explosion
		case 32: success=DoOnePotion(s,0x0F8C, 5,"sulfurous ash");  break;//explosion
		case 33: success=DoOnePotion(s,0x0F8C,10,"sulfurous ash");  break;//greater explosion
		case 41: success=DoOnePotion(s,0x0F85, 1,"ginseng");        break;//lesser heal
		case 42: success=DoOnePotion(s,0x0F85, 3,"ginseng");        break;//heal
		case 43: success=DoOnePotion(s,0x0F85, 7,"ginseng");        break;//greater heal
		case 51: success=DoOnePotion(s,0x0F8D, 1,"spider's silk");  break;//night sight
		case 61: success=DoOnePotion(s,0x0F88, 1,"nightshade");     break;//lesser poison
		case 62: success=DoOnePotion(s,0x0F88, 2,"nightshade");     break;//poison
		case 63: success=DoOnePotion(s,0x0F88, 4,"nightshade");     break;//greater poison
		case 64: success=DoOnePotion(s,0x0F88, 8,"nightshade");     break;//deadly poison
		case 71: success=DoOnePotion(s,0x0F7A, 1,"black pearl");    break;//refresh
		case 72: success=DoOnePotion(s,0x0F7A, 5,"black pearl");    break;//total refreshment
		case 81: success=DoOnePotion(s,0x0F86, 2,"mandrake");       break;//strength
		case 82: success=DoOnePotion(s,0x0F86, 5,"mandrake");       break;//greater strength
		default:
			 LogError("switch reached default",(type*10)+sub);
			 return;
	}
    
	if (success)
	{
		tempfx::add(pc, pc, tempfx::ALCHEMY_GRIND, 0, 0, 0); // make grinding sound for a while
		tempfx::add(pc, pc, tempfx::ALCHEMY_GRIND, 0, 3, 0);
		tempfx::add(pc, pc, tempfx::ALCHEMY_GRIND, 0, 6, 0);
		tempfx::add(pc, pc, tempfx::ALCHEMY_GRIND, 0, 9, 0);
		tempfx::add(pc, pi_mortar, tempfx::ALCHEMY_END, type, sub, 0);  // this will indirectly call CreatePotion()
	}
}

/*!
\author Duke
\brief Does the appropriate skillcheck for the potion, creats it
in the mortar on success and tries to put it into a bottle
\param s pointer to the character crafter
\param type type of potion
\param sub subtype of potion
\param mortar serial of the mortar
*/
void Skills::CreatePotion(CHARACTER s, char type, char sub, int mortar)
{
	P_CHAR pc=MAKE_CHAR_REF(s);
	VALIDATEPC(pc);

	P_ITEM pi_mortar=MAKE_ITEM_REF(mortar);
	VALIDATEPI(pi_mortar);

	int success=0;

	switch((10*type)+sub)
	{
		case 11:success=pc->checkSkill( ALCHEMY,151, 651);break;//agility
		case 12:success=pc->checkSkill( ALCHEMY,351, 851);break;//greater agility
		case 21:success=pc->checkSkill( ALCHEMY,  0, 500);break;//lesser cure
		case 22:success=pc->checkSkill( ALCHEMY,251, 751);break;//cure
		case 23:success=pc->checkSkill( ALCHEMY,651,1151);break;//greater cure
		case 31:success=pc->checkSkill( ALCHEMY, 51, 551);break;//lesser explosion
		case 32:success=pc->checkSkill( ALCHEMY,351, 851);break;//explosion
		case 33:success=pc->checkSkill( ALCHEMY,651,1151);break;//greater explosion
		case 41:success=pc->checkSkill( ALCHEMY,  0, 500);break;//lesser heal
		case 42:success=pc->checkSkill( ALCHEMY,151, 651);break;//heal
		case 43:success=pc->checkSkill( ALCHEMY,551,1051);break;//greater heal
		case 51:success=pc->checkSkill( ALCHEMY,  0, 500);break;//night sight
		case 61:success=pc->checkSkill( ALCHEMY,  0, 500);break;//lesser poison
		case 62:success=pc->checkSkill( ALCHEMY,151, 651);break;//poison
		case 63:success=pc->checkSkill( ALCHEMY,551,1051);break;//greater poison
		case 64:success=pc->checkSkill( ALCHEMY,901,1401);break;//deadly poison
		case 71:success=pc->checkSkill( ALCHEMY,  0, 500);break;//refresh
		case 72:success=pc->checkSkill( ALCHEMY,251, 751);break;//total refreshment
		case 81:success=pc->checkSkill( ALCHEMY,251, 751);break;//strength
		case 82:success=pc->checkSkill( ALCHEMY,451, 951);break;//greater strength

		default:
			LogError("switch reached default");
			return;
	}

	if (success==0 && !pc->IsGM()) // AC bugfix
	{
		pc->emoteall(TRANSLATE("*%s tosses the failed mixture from the mortar, unable to create a potion from it.*"),0, pc->getCurrentNameC());
		return;
	}

	pi_mortar->type=17;
	pi_mortar->more1=type;
	pi_mortar->more2=sub;
	pi_mortar->morex=pc->skill[ALCHEMY];

	if (pc->getAmount(0x0F0E)<1)
	{
		target(pc->getSocket(), 0, 1, 0, 109, TRANSLATE("Where is an empty bottle for your potion?"));
	}
	else
	{
		pc->playSFX(0x0240); // Liquid sfx
		pc->emoteall(TRANSLATE("*%s pours the completed potion into a bottle.*"), 0, pc->getCurrentNameC());
		delequan(DEREF_P_CHAR(pc), 0x0F0E, 1);
		Skills::PotionToBottle(pc, pi_mortar);
	}
}

/*!
\author Duke
\brief Uses the targeted potion bottle <b>outside</b> the backpack to
pour in the potion from the mortar
\param s socket of the crafter
*/
void Skills::BottleTarget(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	P_ITEM pi=pointers::findItemBySerPtr(buffer[s]+7);
	VALIDATEPI(pi);
    
	if (!ISVALIDPI(pi) || pi->magic==4) 
		return;    // Ripper

	if (pi->id()==0x0F0E)   // an empty potion bottle ?
	{
		pi->ReduceAmount(1);
		
		P_ITEM pi_mortar=pointers::findItemBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s]));
		VALIDATEPI(pi_mortar);

		if (pi_mortar->type==17)
		{
			pc->emoteall(TRANSLATE("*%s pours the completed potion into a bottle.*"), 0, pc->getCurrentNameC());
			Skills::PotionToBottle(pc, pi_mortar);
		}
	}
	else
		sysmessage(s,TRANSLATE("This is not an appropriate container for a potion."));
}

#define CREATEINBACKPACK( ITEM ) pi = item::CreateFromScript( ITEM, pc->getBackpack() );

/*!
\author Endymion
\brief This really creates the potion
\param pc pointer to the crafter's character
\param pi_mortar pointer to the mortar's item
*/
void Skills::PotionToBottle( P_CHAR pc, P_ITEM pi_mortar )
{
	VALIDATEPC(pc);

	NXWCLIENT ps=pc->getClient();
	if( ps==NULL ) return;

	VALIDATEPI(pi_mortar);

	P_ITEM pi=NULL;

	int potionType= (10*pi_mortar->more1)+pi_mortar->more2;

	switch( potionType )    {
		case 11: CREATEINBACKPACK( "$normal_agility_potion" )		break;
		case 12: CREATEINBACKPACK( "$greater_agility_potion" )		break;

		case 21: CREATEINBACKPACK( "$lesser_cure_potion" )		break;
		case 22: CREATEINBACKPACK( "$normal_cure_potion" )		break;
		case 23: CREATEINBACKPACK( "$greater_cure_potion" )		break;

		case 31: CREATEINBACKPACK( "$lesser_explosion_potion" )		break;
		case 32: CREATEINBACKPACK( "$normal_explosion_potion" )		break;
		case 33: CREATEINBACKPACK( "$greater_explosion_potion" )	break;

		case 41: CREATEINBACKPACK( "$lesser_healing_potion" )		break;
		case 42: CREATEINBACKPACK( "$normal_healing_potion" )		break;
		case 43: CREATEINBACKPACK( "$greater_healing_potion" )		break;

		case 51: CREATEINBACKPACK( "$normal_nightsight_potion" )	break;

		case 61: CREATEINBACKPACK( "$lesser_poison_potion" )		break;
		case 62: CREATEINBACKPACK( "$normal_poison_potion" )		break;
		case 63: CREATEINBACKPACK( "$greater_poison_potion" )		break;
		case 64: CREATEINBACKPACK( "$deadly_poison_potion" )		break;

		case 71: CREATEINBACKPACK( "$normal_energy_potion" )		break;
		case 72: CREATEINBACKPACK( "$greater_energy_potion" )		break;

		case 81: CREATEINBACKPACK( "$normal_strength_potion" )		break;
		case 82: CREATEINBACKPACK( "$greater_strength_potion" )		break;
		default: 
			LogError("switch reached default into PotionToBottle");
			return;
	}

	VALIDATEPI(pi);

    
	if(!pc->IsGM())
    
	{
		pi->creator = pc->getCurrentName();

		if (pc->skill[ALCHEMY]>950) 
			pi->madewith=ALCHEMY+1;
		else 
			pi->madewith=0-ALCHEMY-1;
    
	} else  {
		pi->creator = "";
		pi->madewith=0;
	}

    pi->Refresh();
    pi_mortar->type=0;
}

char Skills::CheckSkillSparrCheck(int c, unsigned short int sk, int low, int high, P_CHAR pcd)
{
    P_CHAR pc=MAKE_CHARREF_LRV(c,0);
    bool bRaise = false;
    if (pcd->npc) bRaise = true;
    if (ServerScp::g_nLimitPlayerSparring==0) bRaise = true;
    if (pcd->skill[sk] > (pc->skill[sk]+ServerScp::g_nLimitPlayerSparring)) bRaise = true;

    return pc->checkSkill( static_cast<Skill>(sk), low, high, bRaise);
}

char Skills::AdvanceSkill(CHARACTER s, int sk, char skillused)
{
	if ( sk < 0 || sk >= TRUESKILLS ) //Luxor
		return 0;

    P_CHAR pc = MAKE_CHARREF_LRV(s, 0)

    int a,ges=0,d=0;
    unsigned char lockstate;
    int skillcap = SrvParms->skillcap;
    UI32 incval;
    int atrophy_candidates[ALLSKILLS+1];


    
    if (pc->amxevents[EVENT_CHR_ONGETSKILLCAP])
        skillcap = pc->amxevents[EVENT_CHR_ONGETSKILLCAP]->Call(pc->getSerial32(), pc->getSocket() );
	/*
	if ( pc->getAmxEvent(EVENT_CHR_ONGETSKILLCAP) != NULL )
		skillcap = pc->runAmxEvent( EVENT_CHR_ONGETSKILLCAP, pc->getSerial32(), pc->getSocket() );
	*/

    lockstate=pc->lockSkill[sk];
    if (pc->IsGM()) lockstate=0;
    // for gms no skill cap exists, also ALL skill will be interperted as up, no matter how they are set

    if (lockstate==2 || lockstate==1) return 0;// skill is locked -> forget it

    // also NOthing happens if you train a skill marked for atrophy !!!
    // skills only fall if others raise, ( osi quote ), so my interpretion
    // is that those marked as falling cant fall if you use them directly
    // exception: if you are gm its ignored!

    int c=0;

    for (int b=0;b<(ALLSKILLS+1);b++)
    {
        if (pc->lockSkill[b]==1 && pc->baseskill[b]!=0) // only count atrophy candidtes if they are above 0 !!!
        {
            atrophy_candidates[c]=b;
            c++;
        }
    }

    if (!pc->IsGM())
    {
        for (a=0;a<ALLSKILLS;a++)
        {
            ges+=pc->baseskill[a];
        }
        ges=ges/10;

        if (ges>skillcap && c==0) // skill capped and no skill is marked as fall down.
        {
            pc->sysmsg(TRANSLATE("You have reached the skill-cap of %i and no skill can fall!"), skillcap);
            return 0;
        }

    } else ges=0;

    //
    // Skill advance parameter
    //
    // 1. If the race system is active get them from there
    // 2. Else use the standard server tables
    //
    if( Race::isRaceSystemActive() )
    {
		if ( skillused )
		{
			Race* r = Race::getRace( (UI32) pc->race );
			if( r!=NULL ) 
				incval = r->getSkillAdvanceSuccess( (UI32) sk, (UI32) pc->baseskill[sk] );
			else 
				incval=0;
			//SDbgOut("Race advance success for skill %d with base %d is %d\n", sk, pc->baseskill[sk], incval * 10 );
		}
		else
		{
			Race* r = Race::getRace( (UI32) pc->race );
			if( r!=NULL )
				incval = r->getSkillAdvanceFailure( (UI32) sk, (UI32) pc->baseskill[sk] );
			else 
				incval =0;

			//SDbgOut("Race advance failure for skill %d with base %d is %d\n", sk, pc->baseskill[sk], incval * 10 );
		}
		incval *= 10;
    }
    else {
    
	int i = 0;
	int loopexit=0;
    	while ( (wpadvance[1+i+skillinfo[sk].advance_index].skill == sk &&
      	         wpadvance[1+i+skillinfo[sk].advance_index].base  <= pc->baseskill[sk] ) &&
    	     	  (++loopexit < MAXLOOPS) ) ++i;
	if(skillused)
   		incval=(wpadvance[i+skillinfo[sk].advance_index].success);
	else
		incval=(wpadvance[i+skillinfo[sk].advance_index].failure);
	incval *= 10;
    }

    int retval=0;
    if (incval>rand()%SrvParms->skilladvancemodifier)
    {
        retval=1;
    }

    
    if(pc->amxevents[EVENT_CHR_ONADVANCESKILL]!=NULL) {
        g_bByPass = false;
        retval = pc->amxevents[EVENT_CHR_ONADVANCESKILL]->Call(pc->getSerial32(), sk, skillused, retval);
        if (g_bByPass==true) return retval;
    }
	/*
	if ( pc->getAmxEvent(EVENT_CHR_ONADVANCESKILL) != NULL ) {
		retval = pc->runAmxEvent( EVENT_CHR_ONADVANCESKILL, pc->getSerial32(), sk, skillused, retval);
		if (g_bByPass==true)
			return retval;
	}
	*/
    if (retval)
    {
		pc->baseskill[sk]++;
        // no atrophy for gm's !!
        if (ges>skillcap) // atrophy only if cap is reached !!!
        // if we are above the skill cap -> we have to let the atrophy candidates fall
        // important: we have to let 2 skills fall, or we'll never go down to cap
        // (especially if we are far above the cap from previous verisons)
        {
            if (c==1)
            {
                d = (pc->baseskill[atrophy_candidates[0]]>=2)? 2 : 1; // avoid value below 0 (=65535 cause unsigned)
                {
                    if (d==1 && pc->baseskill[atrophy_candidates[0]]==0) 
						d=0; // should never happen ...
                    pc->baseskill[atrophy_candidates[0]]-=d;
					Skills::updateSkillLevel(pc, atrophy_candidates[0]);         // we HAVE to correct the skill-value
                    updateskill(pc->getSocket(), atrophy_candidates[0]); // and send changed skill values packet so that client can re-draw correctly
                }
            // this is very important cauz this is ONLY done for the calling skill value automatically .
            }
            else
            {
                for( int vol=0; vol<2; vol++ ) {

					d = ( c != 0)? rand()%c : 0;

					if (pc->baseskill[atrophy_candidates[d]]>=1)
					{
	                    pc->baseskill[atrophy_candidates[d]]--;
						Skills::updateSkillLevel(pc, atrophy_candidates[d]);
						updateskill(pc->getSocket(), atrophy_candidates[d]);
					}

                }
            }
        }
        if (ServerScp::g_nStatsAdvanceSystem == 0)
			Skills::AdvanceStats(s, sk);
    }
    if (ServerScp::g_nStatsAdvanceSystem == 1)
		Skills::AdvanceStats(s, sk);
    //AMXEXECS(s,AMXT_SPECIALS, 5, retval, AMX_AFTER);
    return retval;


}

/*!
\author Duke
\date 21/03/2000
\brief Little helper function for cSkills::AdvanceStats()

finds the appropriate line for the used skill in advance table
and uses the value of that skill (!) to increase the stat
and cuts it down to 100 if necessary

\param sk skill identifier
\param stat stat identifier
\param pc pointer to character to advance the stats to
\todo document missing paramteres
*/
static int AdvanceOneStat(UI32 sk, int i, char stat, bool *update, int type, P_CHAR pc)
{
	if ( sk < 0 || sk >= TRUESKILLS ) //Luxor
		return 0;

	int loopexit=0, limit=1000;
	*update = false;
	SI32 tmp;

	int stat2update1, stat2update2;
	int stat2update;

	switch( stat )
	{
		case 'S': tmp= pc->st3;	break;
		case 'D': tmp= pc->dx3;	break;
		case 'I': tmp= pc->in3;	break;
	}

	if( Race::isRaceSystemActive() )
		stat2update1 = Race::getRace( (UI32) pc->race )->getSkillAdvanceSuccess( sk, tmp*10 );

	while ((wpadvance[i+1].skill==sk) &&     // if NEXT line is for same skill and is not higher than our stat then proceed to it !
     		(wpadvance[i+1].base<=(tmp*10)) && (++loopexit < MAXLOOPS) )
  	      	i++;

	stat2update2 = wpadvance[i].success;         // gather small increases

	//SDbgOut("AdvanceOneStat() skill %d base %d succes %d %d\n", sk, (*stat)*10, stat2update1, stat2update2);

	if( Race::isRaceSystemActive() )
		stat2update = stat2update1;
	else
		stat2update = stat2update2;

	switch( stat )
	{
		case 'S':	pc->st2+= stat2update;
					tmp= pc->st2;
					break;

		case 'D':	pc->dx2+= stat2update;
					tmp= pc->dx2;
					break;

		case 'I':	pc->in2+= stat2update;
					tmp= pc->in2;
					break;
	}

	if ( tmp >= 1000)           // until they reach 1000
	{
		switch( stat )
		{
			case 'S':	pc->st2-= 1000;	if (pc->st2 < 0) pc->st2 = 0; tmp= pc->st3; break;
			case 'D':	pc->dx2-= 1000; if (pc->dx2 < 0) pc->dx2 = 0; tmp= pc->dx3; break;
			case 'I':	pc->in2-= 1000;	if (pc->in2 < 0) pc->in2 = 0; tmp= pc->in3; break;
		}
 //       *stat2 -= 1000;                     // then change it

 	
    if (pc->amxevents[EVENT_CHR_ONADVANCESTAT]) {
        g_bByPass = false;
        pc->amxevents[EVENT_CHR_ONADVANCESTAT]->Call(pc->getSerial32(), type, sk, tmp);
        if (g_bByPass==true) return false;
	}
	
	//pc->runAmxEvent( EVENT_CHR_ONADVANCESTAT, pc->getSerial32(), type, sk, tmp);
 	if (g_bByPass==true)
		return false;

		if( Race::isRaceSystemActive() )
		{
			switch( type )
			{
				case STATCAP_CAP:
					limit = Race::getRace( (UI32) pc->race )->getStatCap();
					SDbgOut("AdvanceOneStat() race %d %s statcap %d\n", pc->race, Race::getName( pc->race )->c_str(), limit );
					break;
				case STATCAP_STR:
					limit = Race::getRace( (UI32) pc->race )->getStrCap();
					SDbgOut("AdvanceOneStat() race %d %s strcap %d\n", pc->race, Race::getName( pc->race )->c_str(), limit );
					break;
				case STATCAP_DEX:
					limit = Race::getRace( (UI32) pc->race )->getDexCap();
					SDbgOut("AdvanceOneStat() race %d %s dexcap %d\n", pc->race, Race::getName( pc->race )->c_str(), limit );
					break;
				case STATCAP_INT:
					limit = Race::getRace( (UI32) pc->race )->getIntCap();
					SDbgOut("AdvanceOneStat() race %d %s intcap %d\n", pc->race, Race::getName( pc->race )->c_str(), limit );
					break;
			}
		}
		else
		{
			limit = 100;
		}

    if (pc->amxevents[EVENT_CHR_ONGETSTATCAP]!=NULL)
       	limit = pc->amxevents[EVENT_CHR_ONGETSTATCAP]->Call(pc->getSerial32(), type, limit);
	
	/*
	if ( pc->getAmxEvent(EVENT_CHR_ONGETSTATCAP) != NULL )
		limit = pc->runAmxEvent( EVENT_CHR_ONGETSTATCAP, pc->getSerial32(), type, limit);
	*/

		switch( stat )
		{
			case 'S':	pc->modifyStrength(1);
						pc->st3++;
						break;

			case 'D':	pc->dx++;
						pc->dx3++;						
						break;

			case 'I':	pc->in++;
						pc->in3++;
						break;
		}
       	*update=true;
    }

	if( !pc->IsGM() )
	{
		switch( stat )
		{
		case 'S':	if(pc->st3 > limit)	{
						pc->st3=limit;
						pc->setStrength(limit);
					}
					break;

		case 'D':	if(pc->dx3 > limit) {
						pc->dx3=limit;
						pc->dx= limit;
					}
					break;

		case 'I':	if(pc->in3 > limit) {
						pc->in3=limit;
						pc->in= limit;
					}
					break;
		}
		*update= true;
	}
	return *update;
}

/*!
\author Duke
\date 21/03/2000
\brief Advance STR, DEX and INT after use of a skill
\param s crafter character
\param sk skill identifier

checks if STR+DEX+INT are higher than statcap from server.cfg
gives all three stats the chance (from skills.scp & server.cfg) to rise
and reduces the two other stats if necessary
*/
void Skills::AdvanceStats(CHARACTER s, int sk)
{
	if ( sk < 0 || sk >= TRUESKILLS ) //Luxor
		return;


	P_CHAR pc = MAKE_CHARREF_LR(s)

    	// Begin: Determine statcap
    	// 1. get statcap as defined in server.cfg
	// 2. get statcap from racesystem
	// 3. get statcap from char event
	//
	int statcap = SrvParms->statcap;

	if( Race::isRaceSystemActive() )
		statcap = Race::getRace( pc->race )->getStatCap();


	
	if (pc->amxevents[EVENT_CHR_ONGETSTATCAP]!=NULL)
		statcap = pc->amxevents[EVENT_CHR_ONGETSTATCAP]->Call(pc->getSerial32(), STATCAP_CAP, statcap);
	/*
	if ( pc->getAmxEvent(EVENT_CHR_ONGETSTATCAP) != NULL )
		statcap = pc->runAmxEvent( EVENT_CHR_ONGETSTATCAP, pc->getSerial32(), STATCAP_CAP, statcap);
	*/
	// End: Determine statcap

    bool atCap = (pc->st3 + pc->dx3 + pc->in3) > statcap;

    int	i = skillinfo[sk].advance_index;
    int mod	= SrvParms->statsadvancemodifier;
//  int	*pi; // ptr to stat to be decreased
	bool 	update 	= false;

	if ( pc->statGainedToday <= ServerScp::g_nStatDailyLimit )
	{
		bool strCheck = ( Race::isRaceSystemActive() ? Race::getRace( pc->race )->getSkillAdvanceStrength( sk ) : skillinfo[sk].st ) > (UI32)(rand() % mod);
    	bool dexCheck = ( Race::isRaceSystemActive() ? Race::getRace( pc->race )->getSkillAdvanceDexterity( sk ) : skillinfo[sk].dx ) > (UI32)(rand() % mod);
    	bool intCheck = ( Race::isRaceSystemActive() ? Race::getRace( pc->race )->getSkillAdvanceIntelligence( sk ) : skillinfo[sk].in ) > (UI32)(rand() % mod);

       	if ( strCheck )
       		if ( AdvanceOneStat( sk, i, 'S', &update, STATCAP_STR, pc ) && atCap && !pc->IsGM() )
			{
				if( rand()%2 ) {
					pc->dx3--;
					pc->dx--;
				}
				else {
					pc->in3--;
					pc->in--;
				}
			}

		if ( dexCheck )
	       	if ( AdvanceOneStat(sk, i, 'D', &update, STATCAP_DEX, pc ) && atCap && !pc->IsGM() )
			{
				if( rand()%2 ) {
					pc->st3--;
					pc->modifyStrength(-1);
				}
				else {
					pc->in3--;
					pc->in--;
				}
			}

	    if ( intCheck )
	       	if ( AdvanceOneStat(sk, i, 'I', &update, STATCAP_INT, pc ) && atCap && !pc->IsGM() )
			{
				if( rand()%2 ) {
					pc->st3--;
					pc->modifyStrength(-1);
				}
				else {
					pc->dx3--;
					pc->dx--;
				}
			}


    	
		if ( update )
		{
  			
			NXWSOCKET socket = pc->getSocket();

			++pc->statGainedToday;
  
			if ( socket != INVALID )
				statwindow( pc, pc);              // update client's status window

        	
			for ( i = 0;  i < ALLSKILLS; i++ )
				updateSkillLevel(pc,i );     // update client's skill window

			if ( atCap && !pc->IsGM() )
				pc->sysmsg(TRANSLATE("You have reached the stat-cap of %i!") ,statcap );
	        
		}
	}
}

/*!
\brief Spirit speack time on a base of 30 seconds + skill[SPIRITSPEAK]/50 + INT
\param s socket to the character
*/
void Skills::SpiritSpeak(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
    //  Unsure if spirit speaking should they attempt again?
    //  Suggestion: If they attempt the skill and the timer is !0 do not have it raise the skill

    if(!pc->checkSkill(SPIRITSPEAK, 0, 1000))
    {
        sysmessage(s,TRANSLATE("You fail your attempt at contacting the netherworld."));
        return;
    }

    impaction(s,0x11);          // I heard there is no action...but I decided to add one
    pc->playSFX(0x024A);   // only get the sound if you are successful
    sysmessage(s,TRANSLATE("You establish a connection to the netherworld."));
    SetTimerSec(&(pc->spiritspeaktimer),spiritspeak_data.spiritspeaktimer+pc->in);
}

/*!
\brief Skill is clicked on the skill list
\param s socket to the character that used skill
\param x skill identifier
*/
void Skills::SkillUse(NXWSOCKET s, int x)
{
	if ( s < 0 || s >= now || x < 0 || x >= TRUESKILLS) //Luxor
		return;

    
    
	int cc=currchar[s];
	P_CHAR pc = MAKE_CHARREF_LOGGED( currchar[s], err );
	VALIDATEPC(pc);

	if( (pc->skilldelay>uiCurrentTime) && (!pc->IsGM()) )
	{
		sysmessage(s, TRANSLATE("You must wait a few moments before using another skill."));
		return;
	}

	if ( pc->jailed )
	{
		sysmessage(s,TRANSLATE("you are in jail and cant gain skills here!"));
		return;
	}

	if ( pc->dead )
	{
		sysmessage(s,TRANSLATE("You cannot do that as a ghost."));
		return;
	}
    
	if (pc->IsHiddenBySpell())
		return; //Luxor: cannot use skills if under invisible spell
    /*  chars[cc].unHide();*/
    
	//<Luxor> 7 dec 2001
	if (skillinfo[x].unhide_onuse == 1)
		pc->unHide();
	//</Luxor>
    
	
	if( pc->casting )
	{
		sysmessage( s, TRANSLATE("You can't do that while you are casting" ));
		return;
	}

	pc->disturbMed(); // Meditation
	
	AMXEXECSV(s,AMXT_SKILLS, x, AMX_BEFORE);
	
	bool setSkillDelay = true;

	if( Race::isRaceSystemActive() && !(Race::getRace( pc->race )->getCanUseSkill( (UI32) x )) )
	{
		sysmessage(s, TRANSLATE("Your race cannot use that skill") );
		setSkillDelay = false;
	}
	else 
	{
		switch(x)
		{
			case ARMSLORE:
				target(s, 0, 1, 0, 29, TRANSLATE("What item do you wish to get information about?"));
				break;

			case ANATOMY:
				break;

			case ITEMID:
				target(s, 0, 1, 0, 40, TRANSLATE("What do you wish to appraise and identify?"));
				break;
			
			case EVALUATINGINTEL:
				break;
			
			case TAMING:
				target(s, 0, 1, 0, 42, TRANSLATE("Tame which animal?"));
				break;
			
			case HIDING:
				Skills::Hide(s);
				break;
			
			case STEALTH:
				Skills::Stealth(s);
				break;
			
			case DETECTINGHIDDEN:
				target(s, 0, 1, 0, 77, TRANSLATE("Where do you wish to search for hidden characters?"));
				break;
			
			case PEACEMAKING:
				Skills::PeaceMaking(s);
				break;
			
			case PROVOCATION:
				target(s, 0, 1, 0, 79, TRANSLATE("Whom do you wish to incite?"));
				break;
			
			case ENTICEMENT:
				target(s, 0, 1, 0, 81, TRANSLATE("Whom do you wish to entice?"));
				break;
			
			case SPIRITSPEAK:
				Skills::SpiritSpeak(s);
				break;
			
			case STEALING:
				if (SrvParms->rogue)
					target(s,0,1,0,205, TRANSLATE("What do you wish to steal?"));
				else
				{
					sysmessage(s, TRANSLATE("That skill has been disabled."));
					setSkillDelay = false;
				}
				break;
			
			case INSCRIPTION:
				break;
			
			case TRACKING:
				Skills::TrackingMenu(s,TRACKINGMENUOFFSET);
				break;
			
			case BEGGING:
				target(s, 0, 1, 0, 152, TRANSLATE("Whom do you wish to annoy?"));
				break;
			
			case ANIMALLORE:
				target(s, 0, 1, 0, 153, TRANSLATE("What animal do you wish to get information about?"));
				break;
			
			case FORENSICS:
				target(s, 0, 1, 0, 154, TRANSLATE("What corpse do you want to examine?"));
				break;
			
			case POISONING:
				target(s, 0, 1, 0, 155, TRANSLATE("What poison do you want to apply?"));
				break;
			
			case TASTEID:
				break;
			
			case MEDITATION:  //Morrolan - Meditation
				if(SrvParms->armoraffectmana)
					Skills::Meditation(s);
				else
				{
					sysmessage(s, TRANSLATE("Meditation is turned off.  Tell your GM to enable ARMOR_AFFECT_MANA_REGEN in server.cfg to enable it."));
					setSkillDelay = false;
				}
				break;
			
			case REMOVETRAPS:
				target(s, 0, 1, 0, 187, TRANSLATE("What do you want to untrap?"));
				break;
			
			case CARTOGRAPHY:
				Skills::Cartography(s);
				break;
			
			default:
				sysmessage(s, TRANSLATE("That skill has not been implemented yet."));
				setSkillDelay = false;
				break;
		}
        //<Luxor>: Complete skills handling by AMX
		AmxFunction::g_prgOverride->CallFn( AmxFunction::g_prgOverride->getFnOrdinal(AMX_SKILLS_MAIN), s, x);
        //</Luxor>
	}
	
	if ( setSkillDelay )
		SetSkillDelay(cc);

	AMXEXECSV(s,AMXT_SKILLS, x, AMX_AFTER);
}

void Skills::Tracking(NXWSOCKET s,int selection)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	P_CHAR pc_targ=pointers::findCharBySerial(pc->trackingtargets_serial[selection]);
	VALIDATEPC(pc_targ);

	pc->trackingtarget_serial=pc_targ->getSerial32(); // sets trackingtarget that was selected in the gump

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	SetTimerSec(&pc->trackingtimer,(((tracking_data.basetimer*pc->skill[TRACKING])/1000)+1)); // tracking time in seconds ... gm tracker -> basetimer+1 seconds, 0 tracking -> 1 sec, new calc by LB
	SetTimerSec(&pc->trackingdisplaytimer,tracking_data.redisplaytime);

	sprintf(temp,TRANSLATE("You are now tracking %s."), pc_targ->getCurrentNameC());

	sysmessage(s,temp);
	Skills::Track(DEREF_P_CHAR(pc));
}

void Skills::CreateTrackingMenu(NXWSOCKET s,int m)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	unsigned int i;
	int total;
	char lentext;
	char sect[512];
	cScpIterator* iter = NULL;
	char script1[1024];
	char gmtext[MAXTRACKINGTARGETS][257]; // crashfix, LB
	int gmid[MAXTRACKINGTARGETS]; // crashfix, LB
	//int gmnumber;
	int id;

	int id1=62; // default tracking animals
	int id2=399;
	int icon=8404;

	char type[40];
	strcpy(type,TRANSLATE("You see no signs of any animals."));
	unsigned int MaxTrackingTargets=0;
	unsigned int distance=tracking_data.baserange+pc->skill[TRACKING]/50;

	if(m==(2+TRACKINGMENUOFFSET))
	{
		id1=1;
		id2=61;
		icon=0x20d1;
		strcpy(type,TRANSLATE("You see no signs of any creatures."));
	}
	if(m==(3+TRACKINGMENUOFFSET))
	{
		id1=400;
		id2=402;
		icon=8454;
		strcpy(type,TRANSLATE("You see no signs of anyone."));
	}

	sprintf(sect, "SECTION TRACKINGMENU %i", m);
	iter = Scripts::Tracking->getNewIterator(sect);
	if (iter==NULL) return;

	strcpy(script1, iter->getEntry()->getFullLine().c_str());   //this one skips the {
	strcpy(script1, iter->getEntry()->getFullLine().c_str());

	lentext=sprintf(gmtext[0], "%s", script1);


	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( pc->getPosition(), distance, true, false );
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pcm = sc.getChar();
		if( !ISVALIDPC( pcm ) ) 
			continue;

	id=pcm->GetBodyType();

		if( (!pcm->dead)&&(id>=id1&&id<=id2)&& ( pcm->getSerial32()!=pc->getSerial32() ) )
        {
			pc->trackingtargets_serial[MaxTrackingTargets]=pcm->getSerial32();
			MaxTrackingTargets++;
			if (MaxTrackingTargets>=MAXTRACKINGTARGETS) 
				break; // lb crashfix
			switch(Skills::TrackingDirection(s,DEREF_P_CHAR(pcm)))
			{
				case NORTH:
                    strcpy(temp,TRANSLATE("to the North"));
                    break;
				case NORTHWEST:
                    strcpy(temp,TRANSLATE("to the Northwest"));
                    break;
				case NORTHEAST:
                    strcpy(temp,TRANSLATE("to the Northeast"));
                    break;
				case SOUTH:
                    strcpy(temp,TRANSLATE("to the South"));
                    break;
				case SOUTHWEST:
                    strcpy(temp,TRANSLATE("to the Southwest"));
                    break;
				case SOUTHEAST:
                    strcpy(temp,TRANSLATE("to the Southeast"));
                    break;
				case WEST:
                    strcpy(temp,TRANSLATE("to the West"));
                    break;
				case EAST:
                    strcpy(temp,TRANSLATE("to the East"));
                    break;
				default:
                    strcpy(temp,TRANSLATE("right next to you"));
                    break;
			}//switch
            
			UI16 skid = pcm->GetBodyType();
			if (/*(skid < 0)||*/(skid>2047)) 
				skid = 0;
			if (pc->skill[TRACKING] < 800) {	//Luxor
				if (skid == BODY_MALE)
					sprintf(gmtext[MaxTrackingTargets], "a man %s",temp);
				else if( skid == BODY_FEMALE )
					sprintf(gmtext[MaxTrackingTargets], "a woman %s",temp);
				else
					sprintf(gmtext[MaxTrackingTargets], "a creature %s",temp);
			}
			else
			sprintf(gmtext[MaxTrackingTargets], "%s %s", pcm->getCurrentNameC(), temp);

			gmid[MaxTrackingTargets]=creatures[skid].icon; // placing correct icon, LB
		}
    }

	safedelete(iter);

	if(MaxTrackingTargets==0)
	{
		pc->sysmsg(type);
		return;
	}

	unsigned char gmprefix[10]="\x7C\x00\x00\x01\x02\x03\x04\x00\x64";
	total=9+1+lentext+1;
	for (i=1;i<=MaxTrackingTargets;i++)
		total+=4+1+strlen(gmtext[i]);
	gmprefix[1]=total>>8;
	gmprefix[2]=total%256;
	gmprefix[3]= pc->getSerial().ser1;
	gmprefix[4]= pc->getSerial().ser2;
	gmprefix[5]= pc->getSerial().ser3;
	gmprefix[6]= pc->getSerial().ser4;
	gmprefix[7]=(m+TRACKINGMENUOFFSET)>>8;
	gmprefix[8]=(m+TRACKINGMENUOFFSET)%256;
	Xsend(s, gmprefix, 9);
	Xsend(s, &lentext, 1);
	Xsend(s, gmtext[0], lentext);
	Xsend(s, &MaxTrackingTargets, 1);
		unsigned char gmmiddle[5]="\x00\x00\x00\x00";
	for (i=1;i<=MaxTrackingTargets;i++)
	{
		gmmiddle[0]=gmid[i]>>8;
		gmmiddle[1]=gmid[i]%256;
		Xsend(s, gmmiddle, 4);
		lentext=strlen(gmtext[i]);
		Xsend(s, &lentext, 1);
		Xsend(s, gmtext[i], lentext);
	}
}

void Skills::TrackingMenu(NXWSOCKET s,int gmindex)
{
	if (s < 0 || s >= now)
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    int total, i;
    char lentext;
    char sect[512];
    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];
    char gmtext[MAXTRACKINGTARGETS][257];
    int gmid[MAXTRACKINGTARGETS];
    int gmnumber=0;
    if (gmindex<1000)
    {
        Skills::Tracking(s,gmindex);
        return;
    }

    sprintf(sect, "SECTION TRACKINGMENU %i",gmindex);
    iter = Scripts::Tracking->getNewIterator(sect);
    if (iter==NULL) return;

    strcpy(script1, iter->getEntry()->getFullLine().c_str()); //skip the {
    strcpy(script1, iter->getEntry()->getFullLine().c_str()); //and read

    lentext=sprintf(gmtext[0], "%s", script1);
    int loopexit=0;
    do
    {
        iter->parseLine(script1, script2);
        if (script1[0]!='}')
        {
            gmnumber++;
            gmid[gmnumber]=hex2num(script1);
            strcpy(gmtext[gmnumber], script2);
            strcpy(script1, iter->getEntry()->getFullLine().c_str());
        }
    }
    while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

    safedelete(iter);

    total=9+1+lentext+1;
	unsigned char gmprefix[10]="\x7C\x00\x00\x01\x02\x03\x04\x00\x64";
    for (i=1;i<=gmnumber;i++) total+=4+1+strlen(gmtext[i]);
    gmprefix[1]=total>>8;
    gmprefix[2]=total%256;
    gmprefix[3]= pc->getSerial().ser1;
    gmprefix[4]= pc->getSerial().ser2;
    gmprefix[5]= pc->getSerial().ser3;
    gmprefix[6]= pc->getSerial().ser4;
    gmprefix[7]=(gmindex+TRACKINGMENUOFFSET)>>8;
    gmprefix[8]=(gmindex+TRACKINGMENUOFFSET)%256;
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

}

void Skills::Track(CHARACTER i)
{

	P_CHAR src=MAKE_CHAR_REF(i);
	VALIDATEPC(src);

	P_CHAR targ= pointers::findCharBySerial(src->trackingtarget_serial);
	VALIDATEPC(targ);

    NXWCLIENT ps=src->getClient();
	if(ps==NULL )
		return;
	
	NXWSOCKET s=ps->toInt();

	Location charpos= src->getPosition();
	Location targpos= targ->getPosition();
 
	int direction=5;

    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
    if((charpos.y - direction)>= targpos.y)  // North
    {
        sprintf(temp,TRANSLATE("%s is to the North"), targ->getCurrentNameC());

        if((charpos.x - direction) > targpos.x)
            sprintf(temp,TRANSLATE("%s is to the Northwest"), targ->getCurrentNameC());

        if((charpos.x + direction) < targpos.x)
            sprintf(temp,TRANSLATE("%s is to the Northeast"), targ->getCurrentNameC());
    }
    else if((charpos.y + direction) <= targpos.y)  // South
    {
        sprintf(temp,TRANSLATE("%s is to the South"), targ->getCurrentNameC());

        if((charpos.x - direction) > targpos.x)
            sprintf(temp,TRANSLATE("%s is to the Southwest"), targ->getCurrentNameC());

        if((charpos.x + direction) < targpos.x)
            sprintf(temp,TRANSLATE("%s is to the Southeast"), targ->getCurrentNameC());
    }
    else if((charpos.x - direction) >= targpos.x)  // West
    {
        sprintf(temp,TRANSLATE("%s is to the West"), targ->getCurrentNameC());
    }
    else if((charpos.x + direction) <= targpos.x)  // East
    {
        sprintf(temp,TRANSLATE("%s is to the East"), targ->getCurrentNameC());
    }
    else sprintf(temp,TRANSLATE("%s is right next to you"), targ->getCurrentNameC());

    char arrow[7];
    arrow[0]= '\xBA';
    arrow[1]= 1;
    arrow[2]= (targpos.x - 1) >> 8;
    arrow[3]= (targpos.x - 1) % 256;
    arrow[4]= targpos.y >> 8;
    arrow[5]= targpos.y % 256;
    Xsend(s,arrow,6);

}

int Skills::TrackingDirection(NXWSOCKET s, CHARACTER i)
{
	if ( s < 0 || s >= now ) //Luxor
		return 0;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pc,0);

	P_CHAR targ=MAKE_CHAR_REF(i);
	VALIDATEPCR(targ,0);

    int direction=5;
	Location charpos= pc->getPosition();
	Location targpos= targ->getPosition();

    if((charpos.y - direction) >= targpos.y)  // North
    {
        if((charpos.x - direction) > targpos.x)
            return NORTHWEST;
        if((charpos.x + direction) < targpos.x)
            return NORTHEAST;
        return NORTH;
    }
    else if((charpos.y + direction) <= targpos.y)  // South
    {
        if((charpos.x - direction) > targpos.x)
            return SOUTHWEST;
        if((charpos.x + direction) < targpos.x)
            return SOUTHEAST;
        return SOUTH;
    }
    else if((charpos.x - direction) >= targpos.x)  // West
        return WEST;
    else if((charpos.x + direction) <= targpos.x)  // East
        return EAST;
    else return 0;

}

/*!
\breif Callback function for inscription menu
\author Luxor
\todo Luxor should to finish this

This is the point where control is returned after the user
selected a spell to inscribe/engrave from the gump menu.
*/
void TellScroll(char *menu_name, int s, long snum)
{
//Luxor: i have still to finish this :PPPPP
/*
    CHARACTER cc=currchar[s];
    unsigned x,cir,spl;
    int i,k,part;
	bool spbook=false;

    if(snum<=0) return;             // bad spell selection

    i=chars[cc].making; // lets re-grab the item they clicked on
    chars[cc].making=0; // clear it out now that we are done with it.

    cir=(int)((snum-800)/10);       // snum holds the circle/spell as used in inscribe.gmp
    spl=(((snum-800)-(cir*10))+1);  // i.e. 800 + 1-based circle*10 + zero-based spell
                                    // snum is also equals the item # in items.scp of the scrool to be created !
    k=packitem(cc);
    if (k<0) return;

    int num=(8*(cir-1))+spl;    // circle & spell combined to a zero-based index
*/
}

/*!
\author AntiChrist & Duke
\brief Bring up inscription menu
\param s socket to send the menu to
*/
void Skills::Inscribe(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC(pc);


	if( LongFromCharPtr(buffer[s] +11) == INVALID )
	{
		LogError("Morrolan - Inscribing click on invalid object?");
		return;	// don't do this if the buffer got flushed - Morrolan prevents CRASH!
			// are you sure ??? that is click xLoc & click yLoc ! - Duke
	}

	P_ITEM pi=pointers::findItemBySerPtr(buffer[s] +7);
	VALIDATEPI(pi);

	if ( (pi->id()>=0xEEA && pi->id()<=0xEF2) || (!(pi->id()==0x0E34) && !(pi->IsStave()) && (!(pi->id()>=0xDF2 && pi->id()<=0xDF5)))) //not a wand
	{
		pc->sysmsg(TRANSLATE("You could not possibly engrave on that!"));
		pc->making=0;
	}
	else
	{
		pc->making=DEREF_P_ITEM(pi);        //we gotta remember what they clicked on!

		/* select spell gump menu system here, must return control to NoX-Wizard so we dont
		freeze the game. when returning to this routine, use snum to determine where to go
		and snum is also the value of what they selected. */

		showAddMenu (pc, 7000);
	}
}

/*!
\author AntiChrist & Duke
\brief Check mana & regs and set the name of selected spell
\param s socket of the crafter
\param i item index
\param cir circle of the spell
\param spl spell identifier
*/
int Skills::EngraveAction(NXWSOCKET s, int i, int cir, int spl)
{
	if ( s < 0 || s >= now ) //Luxor
		return 0;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pc,0);

    P_ITEM pi=MAKE_ITEMREF_LRV(i,0);

    char *spn;                  // spellname
//    int num=(8*(cir-1))+spl;
//    Magic->DelReagents(currchar[s], spells[num].reagents);

    switch(cir*10 + spl)
    {
    // first circle
    case 11: spn="Clumsy";
            pi->offspell=1;
            break;
    case 12: spn="Create Food";break;
    case 13: spn="Feeblemind";
            pi->offspell=2;
            break;
    case 14: spn="Heal";break;
    case 15: spn="Magic Arrow";
            pi->offspell=3;
            break;
    case 16: spn="Night Sight";break;
    case 17: // Reactive Armor
            sysmessage(s, TRANSLATE("Sorry this spell is not implemented!"));
            return 0;
    case 18: spn="Weaken";
            pi->offspell=4;
            break;
    // 2nd circle
    case 21: spn="Agility";break;
    case 22: spn="Cunning";break;
    case 24: spn="Harm";
            pi->offspell=5;
            break;
    case 23:// Cure
    case 25:// Magic Trap
    case 26:// Magic Untrap
    case 27:// Protection
            sysmessage(s, TRANSLATE("Sorry this spell is not implemented!"));
            return 0;
    case 28: spn="Strength";break;
    // 3rd circle
    case 31: spn="Bless";break;
    case 32:// Fireball
            spn="Daemon's Breath";
            pi->offspell=6;
            break;
    case 34: spn="Poison";break;
    case 33: //Magic lock
    case 35: //Telekinesis
            sysmessage(s, TRANSLATE("Sorry this spell is not implemented!"));
            return 0;
    case 36: spn="Teleport";break;
    case 37: spn="Unlock";break;
    case 38: spn="Wall of Stone";break;
    // 4th circle
    case 41:// Arch Cure
    case 42:// Arch Protection
    case 47:// Mana Drain
            sysmessage(s, TRANSLATE("Sorry this spell is not implemented!"));
            return 0;
    case 43:// Curse
            spn="Evil's Eye";
            pi->offspell=8;
            break;
    case 44: spn="Fire Field";break;
    case 45: spn="Greater Heal";break;
    case 46: spn="Lightning";
            pi->offspell=9;
            break;
    case 48: spn="Recall";break;
    // 5th circle
    case 51: spn="Blade Spirit";break;
    case 52: spn="Dispel Field";break;
    case 54: spn="Magic Reflection";break;
    case 55: spn="Mind Blast";
            pi->offspell=11;
            break;
    case 56://Paralyze
            spn="Ghoul's Touch";
            pi->offspell=12;
            break;
    case 57: spn="Poison Field";break;
    case 53://Incognito
    case 58://Summon Creature
            sysmessage(s, TRANSLATE("Sorry this spell is not implemented!"));
            return 0;
    // 6th circle
    case 61: spn="Dispel";break;
    case 62: spn="Energy Bolt";
            pi->offspell=13;
            break;
    case 63: spn="Explosion";
            pi->offspell=14;
            break;
    case 64: spn="Invisibility";break;
    case 65: spn="Mark";break;
    case 66://Mass Curse
            sysmessage(s, TRANSLATE("Sorry this spell is not implemented!"));
            return 0;
    case 67: spn="Paralyse Field";break;
    case 68: spn="Reveal";break;
    // 7th circle
    case 72: spn="Energy Field";break;
    case 73: spn="Flamestrike";
            pi->offspell=15;
            break;
    case 74: spn="Gate Travel";break;
    case 71:// Chain Lightning
    case 75:// Mana Vampire
    case 76:// Mass Dispel
    case 77:// Meteor Storm
    case 78:// Polymorph
    // 8th circle
    case 81:// Earthquake
            sysmessage(s, TRANSLATE("Sorry this spell is not implemented!"));
            return 0;
    case 82: spn="Energy Vortex";break;
    case 83: spn="Resurrection";break;
    case 84: spn="Summon Air Elemental";break;
    case 85: spn="Summon Daemon";break;
    case 86: spn="Summon earth Elemental";break;
    case 87: spn="Summon Fire Elemental";break;
    case 88: spn="Summon Water Elemental";break;
    default:
        LogError("switch reached default");
        return 0;
    }
    pi->setSecondaryName("of %s with", spn);
    return 1;

}

/*!
\brief Calculate the skill of this character based on the characters baseskill and stats
\param pc pointer to the character
\param s skill identifier
*/
void Skills::updateSkillLevel(P_CHAR pc, int s)
{
	VALIDATEPC(pc);
	if ( s < 0 || s >= TRUESKILLS ) //Luxor
		return;


    int temp = (((skillinfo[s].st * pc->getStrength()) / 100 +
        (skillinfo[s].dx * pc->dx) / 100 +
        (skillinfo[s].in * pc->in) / 100)
        *(1000-pc->baseskill[s]))/1000+pc->baseskill[s];


    pc->skill[s] = qmax(static_cast<short>(pc->baseskill[s]), static_cast<short>(temp));


}

void Skills::TDummy(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int hit;

	P_ITEM pWeapon = pc->getWeapon();

	if (ISVALIDPI(pWeapon))
	{
		if (pc->getWeapon()->IsBowType())
		{
			sysmessage(s, TRANSLATE("Practice archery on archery buttes !"));
			return;
		}
	}

	int skillused = pc->getCombatSkill();
	
	if (pc->isMounting())
		pc->combatOnHorse();
	else
		pc->combatOnFoot();

	hit=rand()%3;

	switch(hit)
	{
		case 0: pc->playSFX(0x013B);
			break;
    
		case 1: pc->playSFX(0x013C);
			break;

		case 2: pc->playSFX(0x013D);
			break;

		default:
			LogError("switch reached default");
			return;
	}
	
	P_ITEM pj = pointers::findItemBySerial( LongFromCharPtr(buffer[s] +1) & 0x7FFFFFFF );

	if (ISVALIDPI(pj))
	{
		if (pj->id()==0x1070)
			pj->setId(0x1071);
        
		if (pj->id()==0x1074)
			pj->setId(0x1075);
	
		tempfx::add(pc, pj, tempfx::TRAINDUMMY, 0, 0, 0); //Luxor
		pj->Refresh();
	}
	
	if(pc->skill[skillused] < 300)
	{
		pc->checkSkill( static_cast<Skill>(skillused), 0, 1000);

		if(pc->skill[TACTICS] < 300)
			pc->checkSkill(TACTICS, 0, 250);  //Dupois - Increase tactics but only by a fraction of the normal rate
	}
	else
		sysmessage(s, TRANSLATE("You feel you would gain no more from using that."));

}

void Skills::AButte(NXWSOCKET s1, P_ITEM pButte)
{
	if ( s1 < 0 || s1 >= now ) //Luxor
		return;
	P_CHAR pc = MAKE_CHAR_REF( currchar[s1] );
	VALIDATEPC(pc);
	


	int v1;
    if(pButte->id()==0x100A)
    { // East Facing Butte
        if ((pButte->getPosition("x") > pc->getPosition("x"))||(pButte->getPosition("y") != pc->getPosition("y")))
            v1= INVALID;
        else v1= pc->getPosition("x") - pButte->getPosition("x");
    }
    else
    { // South Facing Butte
        if ((pButte->getPosition("y") > pc->getPosition("y"))||(pButte->getPosition("x") != pc->getPosition("x")))
            v1= INVALID;
        else v1= pc->getPosition("y") - pButte->getPosition("y");
    }

    int arrowsquant=0;
    if(v1==1)
    {

        if(pButte->more1>0)
        {
            
			P_ITEM pi = item::CreateFromScript( "$item_arrow", pc->getBackpack(), pButte->more1/2 );
			VALIDATEPI(pi);
            pi->Refresh();
        }

        if(pButte->more2>0)
        {
			P_ITEM pi = item::CreateFromScript( "$item_crossbow_bolt", pc->getBackpack(), pButte->more2/2 );
			VALIDATEPI(pi);
            pi->Refresh();
        }

        int i=0;
        if(pButte->more1>0) i++;
        if(pButte->more2>0) i+=2;

        char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

        switch(i)
        {
        case 0:
            sprintf(temp,TRANSLATE("This target is empty"));
            break;
        case 1:
            sprintf(temp,TRANSLATE("You pull %d arrows from the target"),pButte->more1/2);
            break;
        case 2:
            sprintf(temp,TRANSLATE("You pull %d bolts from the target"),pButte->more2/2);
            break;
        case 3:
            sprintf(temp,TRANSLATE("You pull %d arrows and %d bolts from the target"),pButte->more1,pButte->more2/2);
            break;
        default:
            LogError("switch reached default");
            return;
        }
        pc->sysmsg(temp);
        pButte->more1=0;
        pButte->more2=0;
    }

    if( (v1>=5) && (v1<=8) )
    {
        if (!pc->getWeapon()->IsBowType())
        {
            pc->sysmsg( TRANSLATE("You need to equip a bow to use this."));
            return;
        }
        if ((pButte->more1+pButte->more2)>99)
        {
            pc->sysmsg( TRANSLATE("You should empty the butte first!"));
            return;
        }
		if (pc->getWeapon()->IsBow()) 
			arrowsquant=pc->getAmount(0x0F3F); // Sabrewulf
        else 
			arrowsquant=pc->getAmount(0x1BFB);

        if (arrowsquant==0)
        {
            pc->sysmsg( TRANSLATE("You have nothing to fire!"));
            return;
        }
		
		if (pc->getWeapon()->IsBow()) 
        {
            pc->delItems( 0x0F3F, 1);
            pButte->more1++;
            //add moving effect here to item, not character
        }
        else
        {
            pc->delItems( 0x1BFB, 1);
            pButte->more2++;
            //add moving effect here to item, not character
        }
        if (pc->isMounting()) 
			pc->combatOnHorse();
        else 
			pc->combatOnFoot();

        if( pc->skill[ARCHERY] < 350 )
            pc->checkSkill( ARCHERY, 0, 1000 );
        else
            pc->sysmsg( TRANSLATE("You learn nothing from practicing here") );

        switch( ( pc->skill[ARCHERY]+ ( (rand()%200) -100) ) /100 )
        {
		case -1:
		case 0:
		case 1:
			pc->sysmsg( TRANSLATE("You miss the target"));
			pc->playSFX(0x0238);
			break;
		case 2:
		case 3:
			pc->sysmsg( TRANSLATE("You hit the outer ring!"));
			pc->playSFX(0x0234);
			break;
		case 4:
		case 5:
		case 6:
			pc->sysmsg( TRANSLATE("You hit the middle ring!"));
			pc->playSFX(0x0234);
			break;
		case 7:
		case 8:
		case 9:
			pc->sysmsg( TRANSLATE("You hit the inner ring!"));
			pc->playSFX(0x0234);
			break;
		case 10:
		case 11:
			pc->sysmsg( TRANSLATE("You hit the bullseye!!"));
			pc->playSFX(0x0234);
			break;
		default:
			break;
		}
    }
    if ( (v1>1)&&(v1<5) || (v1>8))
		pc->sysmsg( TRANSLATE("You cant use that from here."));

}

/*!
\author Luxor
\brief Implements Meditation skill
*/
void Skills::Meditation (NXWSOCKET  s)
{
	if ( s < 0 || s >= now )
		return;

	P_CHAR pc = pointers::findCharBySerial(currchar[s]);
	VALIDATEPC(pc);

	P_ITEM pi = NULL;

	pc->med = 0;

	if ( pc->war ) {
		pc->sysmsg( TRANSLATE("Your mind is too busy with the war thoughts.") );
		return;
	}

	if ( Skills::GetAntiMagicalArmorDefence(pc->getSerial32()) > 15 ) {
		pc->sysmsg( TRANSLATE("Regenerative forces cannot penetrate your armor.") );
		return;
	}

	pi = pc->getWeapon();
	if ( (ISVALIDPI(pi) && !pi->IsStave()) || pc->getShield() ) {
		pc->sysmsg( TRANSLATE("You cannot meditate with a weapon or shield equipped!") );
		return;
	}

	if ( pc->mn == pc->in ) {
		pc->sysmsg( TRANSLATE("You are at peace.") );
		return;
	}


	//
	// Meditation check
	//
	if ( !pc->checkSkill(MEDITATION, 0, 1000) ) {
		pc->sysmsg( TRANSLATE("You cannot focus your concentration.") );
		return;
	}

	pc->sysmsg( TRANSLATE("You enter a meditative trance.") );
	pc->med = 1;
	pc->playSFX(0x00F9);
}

//AntiChrist - 5/11/99
//
//If you are a ghost and attack a player, you can PERSECUTE him
//and his mana decreases each time you try to persecute him
//decrease=3+(your int/10)
//
/*!
\author AntiChrist
\date 05/11/1999
\param s socket of the persecuter

If you are a ghost and attack a player, you can PERSECUTE him
and his mana decreases each time you try to persecute him
decrease=3+(your int/10)
*/
void Skills::Persecute (NXWSOCKET  s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	P_CHAR pc_targ=pointers::findCharBySerial(pc->targserial);
	VALIDATEPC(pc_targ);

    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    if (pc_targ->IsGM()) return;

    int decrease=(pc->in/10)+3;

    if((pc->skilldelay<=uiCurrentTime) || pc->IsGM())
    {
        if(((rand()%20)+pc->in)>45) //not always
        {
            if( pc_targ->mn <= decrease )
                pc_targ->mn = 0;
            else
                pc_targ->mn-=decrease;//decrease mana
            pc_targ->updateStats(1);//update
		pc->sysmsg(TRANSLATE("Your spiritual forces disturb the enemy!"));
		pc_targ->sysmsg(TRANSLATE("A damned soul is disturbing your mind!"));
            SetSkillDelay(DEREF_P_CHAR(pc));

            sprintf(temp, TRANSLATE("%s is persecuted by a ghost!!"), pc_targ->getCurrentNameC());

            // Dupois pointed out the for loop was changing i which would drive stuff nuts later

			pc_targ->emoteall( temp, 1);
            
        } else
        {
		pc->sysmsg(TRANSLATE("Your mind is not strong enough to disturb the enemy."));
        }
    } else
    {
        
	pc->sysmsg(TRANSLATE("You are unable to persecute him now...rest a little..."));
    }

}

void loadskills()
{
    int i, noskill, l=0;
    char sect[512];
    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];

    for (i=0;i<SKILLS;i++) // lb
    {
        skillinfo[i].st=0;
        skillinfo[i].dx=0;
        skillinfo[i].in=0;
        skillinfo[i].advance_index=l;
        skillinfo[i].unhide_onuse = 1;
        skillinfo[i].unhide_onfail = 0;
        noskill=0;

        sprintf(sect, "SECTION SKILL %i", i);
        safedelete(iter);
        iter = Scripts::Skills->getNewIterator(sect);
        if (iter==NULL) continue;

        int loopexit=0;
        do
        {
            iter->parseLine(script1, script2);
            if ((script1[0]!='}')&&(script1[0]!='{'))
            {
                if (!(strcmp("STR", script1)))
                {
                    skillinfo[i].st=str2num(script2);
                }
                else if (!(strcmp("DEX", script1)))
                {
                    skillinfo[i].dx=str2num(script2);
                }
                else if (!(strcmp("INT", script1)))
                {
                    skillinfo[i].in=str2num(script2);
                }
                else if (!(strcmp("SKILLPOINT", script1)))
                {
                    wpadvance[l].skill=i;
                    gettokennum(script2, 0);
                    wpadvance[l].base=str2num(gettokenstr);
                    gettokennum(script2, 1);
                    wpadvance[l].success=str2num(gettokenstr);
                    gettokennum(script2, 2);
                    wpadvance[l].failure=str2num(gettokenstr);
                    l++;
                }
                else if (!(strcmp("UNHIDEONUSE", script1)))  //Luxor 7 dec 2001
                {
                    if (str2num(script2) == 0)
                        skillinfo[i].unhide_onuse = 0;
                    else
                        skillinfo[i].unhide_onuse = 1;
                }
                else if (!(strcmp("UNHIDEONFAIL", script1))) //Luxor 7 dec 2001
                {
                    if (str2num(script2) > 0)
                        skillinfo[i].unhide_onfail = 1;
                    else
                        skillinfo[i].unhide_onfail = 0;
                }
            }
        }
        while ( (script1[0]!='}') && (!noskill) && (++loopexit < MAXLOOPS) );
    }
    safedelete(iter);
}

void SkillVars()
{
    strcpy(skillinfo[ALCHEMY].madeword,"mixed");
    strcpy(skillinfo[ANATOMY].madeword,"made");
    strcpy(skillinfo[ANIMALLORE].madeword,"made");
    strcpy(skillinfo[ITEMID].madeword,"made");
    strcpy(skillinfo[ARMSLORE].madeword,"made");
    strcpy(skillinfo[PARRYING].madeword,"made");
    strcpy(skillinfo[BEGGING].madeword,"made");
    strcpy(skillinfo[BLACKSMITHING].madeword,"forged");
    strcpy(skillinfo[BOWCRAFT].madeword,"bowcrafted");
    strcpy(skillinfo[PEACEMAKING].madeword,"made");
    strcpy(skillinfo[CAMPING].madeword,"made");
    strcpy(skillinfo[CARPENTRY].madeword,"made");
    strcpy(skillinfo[CARTOGRAPHY].madeword,"wrote");
    strcpy(skillinfo[COOKING].madeword,"cooked");
    strcpy(skillinfo[DETECTINGHIDDEN].madeword,"made");
    strcpy(skillinfo[ENTICEMENT].madeword,"made");
    strcpy(skillinfo[EVALUATINGINTEL].madeword,"made");
    strcpy(skillinfo[HEALING].madeword,"made");
    strcpy(skillinfo[FISHING].madeword,"made");
    strcpy(skillinfo[FORENSICS].madeword,"made");
    strcpy(skillinfo[HERDING].madeword,"made");
    strcpy(skillinfo[HIDING].madeword,"made");
    strcpy(skillinfo[PROVOCATION].madeword,"made");
    strcpy(skillinfo[INSCRIPTION].madeword,"wrote");
    strcpy(skillinfo[LOCKPICKING].madeword,"made");
    strcpy(skillinfo[MAGERY].madeword,"envoked");
    strcpy(skillinfo[MAGICRESISTANCE].madeword,"made");
    strcpy(skillinfo[TACTICS].madeword,"made");
    strcpy(skillinfo[SNOOPING].madeword,"made");
    strcpy(skillinfo[MUSICIANSHIP].madeword,"made");
    strcpy(skillinfo[POISONING].madeword,"made");
    strcpy(skillinfo[ARCHERY].madeword,"made");
    strcpy(skillinfo[SPIRITSPEAK].madeword,"made");
    strcpy(skillinfo[STEALING].madeword,"made");
    strcpy(skillinfo[TAILORING].madeword,"sewn");
    strcpy(skillinfo[TAMING].madeword,"made");
    strcpy(skillinfo[TASTEID].madeword,"made");
    strcpy(skillinfo[TINKERING].madeword,"made");
    strcpy(skillinfo[TRACKING].madeword,"made");
    strcpy(skillinfo[VETERINARY].madeword,"made");
    strcpy(skillinfo[SWORDSMANSHIP].madeword,"made");
    strcpy(skillinfo[MACEFIGHTING].madeword,"made");
    strcpy(skillinfo[FENCING].madeword,"made");
    strcpy(skillinfo[WRESTLING].madeword,"made");
    strcpy(skillinfo[LUMBERJACKING].madeword,"made");
    strcpy(skillinfo[MINING].madeword,"smelted");
    strcpy(skillinfo[MEDITATION].madeword,"envoked");
    strcpy(skillinfo[STEALTH].madeword,"made");
    strcpy(skillinfo[REMOVETRAPS].madeword,"made");

}

int Skills::GetAntiMagicalArmorDefence(CHARACTER p)
{// blackwind

	P_CHAR pc= MAKE_CHAR_REF( p );
	VALIDATEPCR( pc, 0 );

    int ar = 0;
    if (pc->HasHumanBody())
    {
        NxwItemWrapper si;
		si.fillItemWeared( pc, false, true, true );
		for( si.rewind(); !si.isEmpty(); si++ )
        {
            P_ITEM pi=si.getItem();
			if( ISVALIDPI(pi) && pi->layer>1 && pi->layer < 25)
            {
                if (!(strstr(pi->getCurrentNameC(), "leather") || strstr(pi->getCurrentNameC(), "magic") ||
                    strstr(pi->getCurrentNameC(), "boot")|| strstr(pi->getCurrentNameC(), "mask")))
                    ar += pi->def;
            }
        }
    }
    return ar;
}
/*!
\author Polygon
\brief Builds the cartography menu
\param s socket of the crafter

Function is called when clicked on the <i>Cartography</i> button
*/
void Skills::Cartography(NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
    
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    if (Skills::HasEmptyMap(DEREF_P_CHAR(pc)))
    {
        itemmake[s].has = 1;
		Skills::MakeMenu(s, 1200, CARTOGRAPHY);
    }
    else
        sysmessage(s, TRANSLATE("You don't have an empty map to draw on"));
}

/*!
\author Polygon
\brief Check if the player carries an empty map
\param cc character to check if has empty map
\return always false (?)
\todo write it
*/
bool Skills::HasEmptyMap(CHARACTER cc)
{

	/*P_CHAR pc=MAKE_CHAR_REF(cc);
	VALIDATEPCR(pc,false);
    
    P_ITEM pack = pc->getBackpack();    // Get the packitem
	VALIDATEPIR(pack,false);
	
	int ci = 0, loopexit = 0;
	P_ITEM pi;
	while (((pi = ContainerSearch(pack->getSerial32(), &ci)) != NULL) &&(++loopexit < MAXLOOPS))
	{
        if(!ISVALIDPI(pi))
			continue;
		
        if (pi->type == 300)  // Is it the right type
            return true;    // Yay, go on with carto
    }
*/
    return false;
}

/*!
\author Polygon
\brief Delete an empty map from the player's backpack, use HasEmptyMap before!
\return always false (?)
\todo write it
*/
bool Skills::DelEmptyMap(CHARACTER cc)
{

 	/*P_CHAR pc=MAKE_CHAR_REF(cc);
	VALIDATEPCR(pc,false);
    
    P_ITEM pack = pc->getBackpack();    // Get the packitem
	VALIDATEPIR(pack,false);

    int ci=0;       // Stores the last found item
    int loopexit=0; // Avoids the loop to take too much time
	P_ITEM cand=NULL;
	while (((cand = ContainerSearch(pack->getSerial32(), &ci)) != NULL) &&(++loopexit < MAXLOOPS))
	{
        if(!ISVALIDPI(cand))
			continue;

        if (cand->type == 300)  // Is it the right type
        {
            cand->deleteItem();    // Delete it
            return true;        // Go on with cartography
        }
    }
*/
    return false;   // Search lasted too long, abort (shouldn't happen, abort if ya get this)
}

/*!
\author Polygon
\brief Attempt to decipher a tattered map
\param tmap item pointer to the map
\param s socket of the decipher

Called when double-click such a map
*/
void Skills::Decipher(P_ITEM tmap, NXWSOCKET s)
{
	if ( s < 0 || s >= now ) //Luxor
		return;
 	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    char sect[512];         // Needed for script search
    int regtouse;           // Stores the region-number of the TH-region
    int i;                  // Loop variable
    int btlx, btly, blrx, blry; // Stores the borders of the tresure region (topleft x-y, lowright x-y)
    int tlx, tly, lrx, lry;     // Stores the map borders
    int x, y;                   // Stores the final treasure location
    cScpIterator* iter = NULL;
    char script1[1024];

    if(pc->skilldelay<=uiCurrentTime || pc->IsGM()) // Char doin something?
    {
        if (pc->checkSkill( CARTOGRAPHY, tmap->morey * 10, 1000)) // Is the char skilled enaugh to decipher the map
        {
            // Stores the new map
            P_ITEM nmap=item::CreateFromScript( 70025, pc->getBackpack() );
            if (!ISVALIDPI(nmap))
            {
                LogWarning("bad script item # 70025(Item Not found).");
                return; //invalid script item
            }

			nmap->setCurrentName("a deciphered lvl.%d treasure map", tmap->morez);   // Give it the correct name
            nmap->morez = tmap->morez;              // Give it the correct level
            nmap->creator = pc->getCurrentName();  // Store the creator


            sprintf(sect, "SECTION TREASURE %i", nmap->morez);

            iter = Scripts::Regions->getNewIterator(sect);

            if (iter == NULL) {
                LogWarning("Treasure hunting cSkills::Decipher : Unable to find 'SECTION TREASURE %d' in regions-script", nmap->morez);
                return;
            }
            strcpy(script1, iter->getEntry()->getFullLine().c_str());               // skip the {
            strcpy(script1, iter->getEntry()->getFullLine().c_str());               // Get the number of areas
            regtouse = rand()%str2num(script1); // Select a random one
            for (i = 0; i < regtouse; i++)      // Skip the ones before the correct one
            {
                strcpy(script1, iter->getEntry()->getFullLine().c_str());
                strcpy(script1, iter->getEntry()->getFullLine().c_str());
                strcpy(script1, iter->getEntry()->getFullLine().c_str());
                strcpy(script1, iter->getEntry()->getFullLine().c_str());
            }
            strcpy(script1, iter->getEntry()->getFullLine().c_str());
            btlx = str2num(script1);
            strcpy(script1, iter->getEntry()->getFullLine().c_str());
            btly = str2num(script1);
            strcpy(script1, iter->getEntry()->getFullLine().c_str());
            blrx = str2num(script1);
            strcpy(script1, iter->getEntry()->getFullLine().c_str());
            blry = str2num(script1);

            safedelete(iter);

            if ((btlx < 0) || (btly < 0) || (blrx > 0x13FF) || (blry > 0x0FFF)) // Valid region?
            {
                sprintf(sect, "Treasure Hunting cSkills::Decipher : Invalid region borders for lvl.%d , region %d", nmap->morez, regtouse+1);   // Give out detailed warning :D
                LogWarning(sect);
                return;
            }
            x = btlx + (rand()%(blrx-btlx));    // Generate treasure location
            y = btly + (rand()%(blry-btly));
            tlx = x - 250;      // Generate map borders
            tly = y - 250;
            lrx = x + 250;
            lry = y + 250;
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
            nmap->more1 = tlx>>8;   // Store the map extends
            nmap->more2 = tlx%256;
            nmap->more3 = tly>>8;
            nmap->more4 = tly%256;
            nmap->moreb1 = lrx>>8;
            nmap->moreb2 = lrx%256;
            nmap->moreb3 = lry>>8;
            nmap->moreb4 = lry%256;
            nmap->morex = x;        // Store the treasure's location
            nmap->morey = y;
            tmap->deleteItem();    // Delete the tattered map
        }
        else
            pc->sysmsg(TRANSLATE("You fail to decipher the map"));      // Nope :P
        // Set the skill delay, no matter if it was a success or not
        SetTimerSec(&pc->skilldelay,SrvParms->skilldelay);
        pc->playSFX(0x0249); // Do some inscription sound regardless of success or failure
        pc->sysmsg(TRANSLATE("You put the deciphered tresure map in your pack"));       // YAY
    }
    else
        pc->sysmsg(TRANSLATE("You must wait to perform another action"));       // wait a bit
}
