  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "client.h"
#include "basics.h"
#include "amx/amxcback.h"
#include "cmdtable.h"
#include "itemid.h"
#include "sndpkg.h"
#include "debug.h"
#include "srvparms.h"
#include "nxw_utils.h"
#include "party.h"
#include "set.h"
#include "trigger.h"
#include "magic.h"
#include "nxwGump.h"
#include "house.h"
#include "targeting.h"
#include "npcai.h"
#include "layer.h"
#include "commands.h"
#include "tmpeff.h"

//extern void tweakmenu(NXWSOCKET  s, SERIAL serial);
extern void targetParty(NXWSOCKET  s);

#define INVALID_SERIAL -1

/*!
\brief base class for all Target classes

encapsulates the basic functions of target processing
\note intendet to become a member of the 'transaction' class in the future
\note the cTargets::multitarget shall become the ctarget::factory (Duke, 07/16/00)
\note Changed my mind. These classes are now considered 'experimental' (Duke, 07/11/00)
 */
class cTarget
{
protected:
    NXWSOCKET s,serial,inx;
    P_ITEM pi;
    P_CHAR pc;
    NXWCLIENT ps;
    void makeSerial()       {serial=LongFromCharPtr(buffer[s]+7);}
    void makeCharIndex()    {inx=calcCharFromSer(serial);}
    void makeItemIndex()    {inx=calcItemFromSer(serial);}
public:
    cTarget(NXWCLIENT pCli)  { s=serial=inx=INVALID; if (pCli!=NULL) { s=pCli->toInt(); ps = pCli; makeSerial(); } }
    virtual void process() = 0;
};

class cCharTarget : public cTarget
{
public:
    cCharTarget(NXWCLIENT pCli) : cTarget(pCli) {}
    virtual void CharSpecific() = 0;
    virtual void process()
    {
        if( buffer[s][1]!=0 ) {
			sysmessage(s,TRANSLATE("That is not a character."));
			return;
		}
        makeSerial();
        makeCharIndex();
        if(inx > -1)
        {
            int err;
            pc = MAKE_CHARREF_LOGGED(inx,err);
            if (err) return;
            CharSpecific();
        }
        else
            sysmessage(s,TRANSLATE("That is not a character."));
    }
};

class cItemTarget : public cTarget
{
public:
    cItemTarget(NXWCLIENT pCli) : cTarget(pCli) {}
    virtual void ItemSpecific() = 0;
    virtual void process()
    {

        if( buffer[s][1]!=0 ) {
            sysmessage(s,TRANSLATE("That is not an item."));
			return;
		}

        makeSerial();
        makeItemIndex();
        if(inx > -1)
        {
            pi = MAKE_ITEMREF_LR(inx);
	    VALIDATEPI(pi);
	    if (addid1[s] == 0)
	    	pi->setDecay(false);
            ItemSpecific();
        }
        else
            sysmessage(s,TRANSLATE("That is not an item."));

	};

};

class cWpObjTarget : public cTarget
{
	public:
		cWpObjTarget(NXWCLIENT pCli) : cTarget(pCli) {}
		virtual void CharSpecific() = 0;
		virtual void ItemSpecific() = 0;
		virtual void process()
		{

			if( buffer[s][1]==0 )
			{ //selected an object
				if(buffer[s][7]>=0x40) // an item's serial ?
				{
					makeItemIndex();
					if(inx > -1)
					{
						pi = MAKE_ITEMREF_LR(inx);
						ItemSpecific();
					}
					else
						sysmessage(s,TRANSLATE("That is not a valid item."));
				}
				else
				{
					makeCharIndex();
					if(inx > -1)
					{
						int err;
						pc = MAKE_CHARREF_LOGGED(inx,err);
						if (err) return;
						CharSpecific();
					}
					else
						sysmessage(s,TRANSLATE("That is not a valid character."));
				}
			}
			else
				sysmessage(s,TRANSLATE("That is not a valid object."));
		}
};

/*!
\author Luxor
\param s socket to attack
\brief Manages all attack command
*/
void cTargets::AllAttackTarget(NXWSOCKET s)
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    P_CHAR pc_target = pointers::findCharBySerPtr(buffer[s] +7);
    VALIDATEPC(pc_target);

    
	NxwCharWrapper sc;
    sc.fillOwnedNpcs( pc, false, true );
    pc->attackStuff(pc_target);
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pet=sc.getChar();
		if( ISVALIDPC(pet))
			npcattacktarget(pet, pc_target);
    }

}



void cTargets::PlVBuy(NXWSOCKET s)//PlayerVendors
{
    if (s==-1) return;
    int v=addx[s];
    P_CHAR pc = MAKE_CHARREF_LR(v);
    P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

    P_ITEM pBackpack= pc_currchar->getBackpack();
    if (!pBackpack) {sysmessage(s,TRANSLATE("Time to buy a backpack")); return; } //LB

    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    P_ITEM pi=pointers::findItemBySerial(serial);     // the item
    if (pi==NULL) return;
    if (pi->isInWorld()) return;
    int price=pi->value;

    
	P_ITEM thepack=(P_ITEM)pi->getContainer();
	VALIDATEPI(thepack);
	P_CHAR pNpc= thepack->getPackOwner();               // the vendor

    if(DEREF_P_CHAR(pNpc)!=v || pc->npcaitype!=NPCAI_PLAYERVENDOR) return;

    if (pc_currchar->isOwnerOf(pc))
    {
        pc->talk(s, TRANSLATE("I work for you, you need not buy things from me!"),0);
        return;
    }

    int gleft=pc_currchar->CountGold();
    if (gleft<pi->value)
    {
        pc->talk(s, TRANSLATE("You cannot afford that."),0);
        return;
    }
    pBackpack->DeleteAmount(price,0x0EED);  // take gold from player

    pc->talk(s, TRANSLATE("Thank you."),0);
    pc->holdg+=pi->value; // putting the gold to the vendor's "pocket"

    // sends item to the proud new owner's pack
    pi->setContSerial( pBackpack->getSerial32() );
    pi->Refresh();

}

/*!
\author Magius
\param s socket
*/
void cTargets::triggertarget(NXWSOCKET s)
{

    if (s<0) return;
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    if (i!=-1)//Char
    {
        //triggernpc//triggerwitem(s,-1,1); //is this used also for npcs?!?!
                              //why shouldn't ? [xan]
    } else
    {//item
        i=calcItemFromSer(serial);
        if(i!=-1)
        {
            P_ITEM pi = MAKE_ITEMREF_LR(i);
            triggerItem(s, pi, TRIGTYPE_TARGET);
        }
    }


}

void cTargets::BanTarg(NXWSOCKET s)
{

}

/*!
\brief Select npc rectangle then store xyz info compressed into FX1 & FX2

Used with small controlled npc's with npcwander 6
\param s socket
\param pp 0x6c packet
*/
static void CodedNpcRectangle(NXWSOCKET s, PKGx6C *pp)
{
	if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
	if (clickx[s]==-1 && clicky[s]==-1)
	{
        	clickx[s]=-2;
        	clicky[s]=-2;
		addx[s]=pp->TxLoc;
		addy[s]=pp->TyLoc;
		addz[s]=pp->TzLoc;
        	target(s,0,1,0,73,TRANSLATE("Select second corner of bounding box."));
        	return;
    	}
	if (clickx[s]==-2 && clicky[s]==-2 )
    	{
		clickx[s] = clicky[s]= -3;
		addx2[s]=pp->TxLoc;
		addy2[s]=pp->TyLoc;
		//addz2[s]=pp->TzLoc;
		target(s,0,1,0,73,TRANSLATE("Select the NPC to set the bounding rectangle for."));
		return;
	}
    	SERIAL serial=LongFromCharPtr(buffer[s]+7);
    	if (serial == 0) //Client sends zero if invalid!
        	return;
    	if (isCharSerial(serial))//Char
    	{
		P_CHAR target = pointers::findCharBySerial(serial);
		if ( target != NULL )
		{
			target->fx1 = ((addx[s]<< 19) | (addy[s]<< 6) | addz[s]);
			target->fy1 = ((addx2[s]<<19) | (addy2[s]<<6) | addz[s]);
		}
	}
	clickx[s] = clicky[s]= -1;
}

/*!
\brief Adds an item when using 'add # #
*/
static void AddTarget(NXWSOCKET s, PKGx6C *pp)
{
    if(pp->TxLoc==-1 || pp->TyLoc==-1) return;

    if (addid1[s]>=0x40) // LB 25-dec-199, changed == to >= for chest multis, probably not really necassairy
    {
        switch (addid2[s])
        {
        case 100:
        case 102:
        case 104:
        case 106:
        case 108:
        case 110:
        case 112:
        case 114:
        case 116:
        case 118:
        case 120:
        case 122:
        case 124:
        case 126:
        case 140:
            buildhouse(s,addid3[s]);//If its a valid house, send it to buildhouse!
            return; // Morrolan, here we WANT fall-thru, don't mess with this switch
        }
    }
    int pileable=0;
    short id=(addid1[s]<<8)+addid2[s];
    tile_st tile;
    Map->SeekTile(id, &tile);
    if (tile.flag2&0x08) pileable=1;

    P_ITEM pi=item::SpawnItem(currchar[s], 1, "#", pileable, id, 0,0);
    VALIDATEPI(pi);
    pi->setDecay( false );
    pi->MoveTo(pp->TxLoc,pp->TyLoc,pp->TzLoc+Map->TileHeight(pp->model));
    pi->Refresh();
    addid1[s]=0;
    addid2[s]=0;

}

/*
class cRenameTarget : public cWpObjTarget
{
public:
    cRenameTarget(NXWCLIENT pCli) : cWpObjTarget(pCli) {}
    void CharSpecific()
    {
        //strcpy(pc->name,xtext[s]);
		pc->setCurrentName( xtext[s] );
    }
    void ItemSpecific()
    {
        if(addx[s]==1) //rename2 //New -- Zippy
            pi->setSecondaryName(xtext[s]);
        else
            pi->setCurrentName(xtext[s]);
    }
};
*/

//we don't need this anymore - AntiChrist (9/99)
// hehe, yes we do
// what about the /tele command ???
// modified for command only purpose and uncommented by LB...

// Changed to have a gmmove effect at origin and at target point
// Aldur
//
static void TeleTarget(NXWSOCKET s, PKGx6C *pp)
{
	if( (pp->TxLoc==-1) || (pp->TyLoc==-1) ) return;

	int cc= currchar[s];
	P_CHAR pc=MAKE_CHAR_REF(cc);
	int x= pp->TxLoc;
	int y= pp->TyLoc;
	signed char z=pp->TzLoc;
	Location charpos= pc->getPosition();

	if ((line_of_sight( s, charpos.x, charpos.y, charpos.z, x, y, z,WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
		(pc->IsGM())))
	{
		pc->doGmEffect();
		pc->MoveTo( x,y,z+Map->TileHeight(pp->model) );
		pc->teleport();
		pc->doGmEffect();
	}

}

class cRemoveTarget : public cWpObjTarget
{
public:
    cRemoveTarget(NXWCLIENT pCli) : cWpObjTarget(pCli) {}
    void CharSpecific()
    {
    	if (!ISVALIDPC(pc)) return;
	
	if (pc->amxevents[EVENT_CHR_ONDISPEL]) {
		pc->amxevents[EVENT_CHR_ONDISPEL]->Call(pc->getSerial32(), -1, DISPELTYPE_GMREMOVE);
	}
	
	//pc->runAmxEvent( EVENT_CHR_ONDISPEL, pc->getSerial32(), -1, DISPELTYPE_GMREMOVE);

        if (pc->account>-1 && !pc->npc) // player check added by LB
        {
            sysmessage(s,TRANSLATE("You cant delete players"));
            return;
        }
        sysmessage(s, TRANSLATE("Removing character."));
	pc->deleteChar();
    }
    void ItemSpecific()
    {
		if (!ISVALIDPI(pi)) return;	

		sysmessage(s, TRANSLATE("Removing item."));
        if (pi->amxevents[EVENT_IONDECAY])
           pi->amxevents[EVENT_IONDECAY]->Call(pi->getSerial32(), DELTYPE_GMREMOVE);
		//pi->runAmxEvent( EVENT_IONDECAY, pi->getSerial32(), DELTYPE_GMREMOVE );
        pi->deleteItem();
    }
};

void DyeTarget(NXWSOCKET s)
{
	SERIAL target_serial = LongFromCharPtr(buffer[s] +7);
	UI16 color, body;

	P_CHAR Me = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(Me);

    if ((addid1[s]==255)&&(addid2[s]==255))
    {
        P_ITEM pi=pointers::findItemBySerial(target_serial);
        if (ISVALIDPI(pi))
        {
            SndDyevat(s,pi->getSerial32(), pi->id());
//            pi->Refresh();
        }

        P_CHAR pc=pointers::findCharBySerial(target_serial);
        if(ISVALIDPC(pc))
	{
            SndDyevat(s,pc->getSerial32(),0x2106);
        }
    }
    else // See Commands::DyeItem(s)
    {
	color = (addid1[s]<<8)|(addid2[s]%256);

        P_ITEM pi=pointers::findItemBySerial(target_serial);
        if (ISVALIDPI(pi))
	{
		if(!(dyeall[s]))
		{
			if (( color<0x0002) || (color>0x03E9))
			{
				color = 0x03E9;
			}
		}


		if (! ((color & 0x4000) || (color & 0x8000)) )
		{
			pi->setColor(color);
		}

		if (color == 0x4631)
		{
			pi->setColor(color);
		}

		pi->Refresh();
		return;
	}

        P_CHAR pc=pointers::findCharBySerial(target_serial);
        if (ISVALIDPC(pc))
	{
		if( !Me->IsGM() ) return; // Only gms dye characters

		body = pc->GetBodyType();

		if(  color < 0x8000  && body >= BODY_MALE && body <= BODY_DEADFEMALE ) color |= 0x8000; // why 0x8000 ?! ^^;

		if ((color & 0x4000) && (body >= BODY_MALE && body<= 0x03E1)) color = 0xF000; // but assigning the only "transparent" value that works, namly semi-trasnparency.

		if (color != 0x8000)
		{
			pc->setSkinColor(color);
			pc->setOldSkinColor(color);
			pc->teleport( TELEFLAG_NONE );

		}
	}
    }
}

class cNewzTarget : public cWpObjTarget
{
public:
    cNewzTarget(NXWCLIENT pCli) : cWpObjTarget(pCli) {}
    void CharSpecific()
    {
	if (!ISVALIDPC(pc)) return; //Luxor
		pc->setPosition("z", addx[s]);
        pc->setPosition("dz", addx[s]);
		P_CHAR pc_s=MAKE_CHAR_REF(inx);
	if (!ISVALIDPC(pc_s)) return; //Luxor
        pc_s->teleport();
    }
    void ItemSpecific()
    {
	if (!ISVALIDPI(pi)) return; //Luxor
        pi->setPosition("z", addx[s]);
		P_ITEM pi_c= MAKE_ITEM_REF(inx);
	if (!ISVALIDPI(pi_c)) return; //Luxor
        pi_c->Refresh();
    }
};

//public !!
void cTargets::IDtarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcItemFromSer(serial);
    if (i!=-1)
    {
	P_ITEM pi=MAKE_ITEMREF_LR(i);
        pi->id1=addx[s];
        pi->id2=addy[s];
        pi->Refresh();
        return;
    }
    i=calcCharFromSer(serial);
    if (i!=-1)
    {
	P_CHAR pc = MAKE_CHARREF_LR(i);
	pc->SetBodyType((addx[s]<<8)|(addy[s]%256));
	pc->SetOldBodyType((addx[s]<<8)|(addy[s]%256));
	pc->teleport();
    }
}

//public !!
void cTargets::XTeleport(NXWSOCKET s, int x)
{

	P_CHAR pc=MAKE_CHAR_REF( currchar[s] );
	if ( !ISVALIDPC( pc ) )
    		return;

	P_CHAR pc_i;
	int i = INVALID, serial = INVALID;

	switch (x)
	{
		case 0:
			serial = LongFromCharPtr( buffer[s] + 7 );
			i = calcCharFromSer(serial);
			pc_i = MAKE_CHAR_REF( i );
			break;
		case 2:
			i = strtonum( 1 );
			if (perm[i])
			{
				i = currchar[i];
				pc_i = MAKE_CHAR_REF( i );
			}
			else
				return;
			break;
		case 3:
			i = currchar[pc->making];
						pc_i = MAKE_CHAR_REF( i );
						VALIDATEPC( pc_i );
			pc_i->MoveTo( pc->getPosition() );
			pc_i->teleport();
			return;
		case 5:
			serial = calcserial(strtonum(1), strtonum(2), strtonum(3), strtonum(4));
			i = calcCharFromSer(serial);
			pc_i = MAKE_CHAR_REF(i);
			break;
	}

	if ( ISVALIDPC( pc_i ) )
	{
		pc_i->MoveTo( pc->getPosition() );
		pc_i->teleport();
		return;// Zippy
	}

	if (serial == INVALID ) return; //xan

	P_ITEM pi = MAKE_ITEM_REF( calcItemFromSer(serial) );
	VALIDATEPI( pi );
	pi->MoveTo( pc->getPosition() );
	pi->Refresh();
}

void XgoTarget(NXWSOCKET s)
{
	
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
	P_CHAR pc=MAKE_CHAR_REF(i);

    if (i!=-1)
    {
        pc->MoveTo( addx[s],addy[s],addz[s] );
	pc->teleport();
    }
}

static void PrivTarget(NXWSOCKET s, P_CHAR pc)
{
    VALIDATEPC(pc);
	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);
	
	if (SrvParms->gm_log)   //Logging
		WriteGMLog(curr, "%s as given %s Priv [%x][%x]\n", curr->getCurrentNameC(), pc->getCurrentNameC(), addid1[s], addid2[s]);

    pc->SetPriv(addid1[s]);
    pc->priv2=addid2[s];
}
//
// Sparhawk:	NICETOHAVE	sound effects
//

static void KeyTarget(NXWSOCKET s, P_ITEM pi) // new keytarget by Morollan
{
    
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	
	if (ISVALIDPI(pi))
    {
        if ((pi->more1==0)&&(pi->more2==0)&&
            (pi->more3==0)&&(pi->more4==0))
        {
            if ( pi->type==ITYPE_KEY && (item_inRange(MAKE_CHAR_REF(currchar[s]),pi,2) || (!pi->isInWorld()) ) )
            {
                if (!pc->checkSkill(TINKERING, 400, 1000))
                {
                    sysmessage(s,TRANSLATE("You fail and destroy the key blank."));
                    // soundeffect3( pi, <whatever> );
                    pi->deleteItem();
                }
                else
                {
                    pi->more1=addid1[s];
                    pi->more2=addid2[s];
                    pi->more3=addid3[s];
                    pi->more4=addid4[s];
                    // soundeffect3( pi, <whatever> );
                    sysmessage(s, TRANSLATE("You copy the key.")); //Morrolan can copy keys
                }
            }
            return;
        }//if
        else if (((pi->more1==addid1[s])&&(pi->more2==addid2[s])&&
            (pi->more3==addid3[s])&&(pi->more4==addid4[s]))||
            (addid1[s]==(unsigned char) 0xFF))
        {
            if (((pi->type==ITYPE_CONTAINER)||(pi->type==ITYPE_UNLOCKED_CONTAINER))&&(item_inRange(pc,pi,2)))
            {
                if(pi->type==ITYPE_CONTAINER) pi->type=ITYPE_LOCKED_ITEM_SPAWNER;
                if(pi->type==ITYPE_UNLOCKED_CONTAINER) pi->type=ITYPE_LOCKED_CONTAINER;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You lock the container."));
                return;
            }
            else if ((pi->type==ITYPE_KEY)&&(item_inRange(pc,pi,2)))
            {
                pc->keyserial=pi->getSerial32();
                sysmessage(s,TRANSLATE("Enter new name for key."));//morrolan rename keys
                return;
            }
            else if ((pi->type==ITYPE_LOCKED_ITEM_SPAWNER)||(pi->type==ITYPE_LOCKED_CONTAINER)&&(item_inRange(pc,pi,2)))
            {
                if(pi->type==ITYPE_LOCKED_ITEM_SPAWNER) pi->type=ITYPE_CONTAINER;
                if(pi->type==ITYPE_LOCKED_CONTAINER) pi->type=ITYPE_UNLOCKED_CONTAINER;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You unlock the container."));
                return;
            }
            else if ((pi->type==ITYPE_DOOR)&&(item_inRange(pc,pi,2)))
            {
                pi->type=ITYPE_LOCKED_DOOR;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You lock the door."));
                return;
            }
            else if ((pi->type==ITYPE_LOCKED_DOOR)&&(item_inRange(pc,pi,2)))
            {
                pi->type=ITYPE_DOOR;
                // soundeffect3( pi, <whatever> );
                sysmessage(s, TRANSLATE("You unlock the door."));
                return;
            }
            else if (pi->id()==0x0BD2)
            {
                sysmessage(s, TRANSLATE("What do you wish the sign to say?"));
                pc->keyserial=pi->getSerial32(); //Morrolan sign kludge
                return;
            }

            //Boats ->
            else if(pi->type==ITYPE_BOATS && pi->type2==3)
            {
                Boats->OpenPlank(pi);
                pi->Refresh();
            }
            //End Boats --^
        }//else if
        else
        {
            if (pi->type==ITYPE_KEY) sysmessage (s, TRANSLATE("That key is not blank!"));
            else if (pi->more1==0x00) sysmessage(s, TRANSLATE("That does not have a lock."));
            else sysmessage(s, TRANSLATE("The key does not fit into that lock."));
            return;
        }//else
        return;
    }//if
}//keytarget()

void cTargets::IstatsTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
    if (serial == 0)
    {
        tile_st tile;
        Map->SeekTile(((buffer[s][0x11]<<8)+buffer[s][0x12]), &tile);
        sprintf(temp, "Item [Static] ID [%x %x]",buffer[s][0x11], buffer[s][0x12]);
        sysmessage(s, temp);
        sprintf(temp, "ID2 [%i], Height [%i]",((buffer[s][0x11]<<8)+buffer[s][0x12]), tile.height);
        sysmessage(s, temp);
    }
    else
    {
        PC_ITEM pi=pointers::findItemBySerial(serial);
        if (pi!=NULL)
        {
	    P_CHAR pc = MAKE_CHAR_REF( currchar[s] );   
	    VALIDATEPC( pc );   
	    newAmxEvent("gui_itemProps")->Call( pi->getSerial32(), pc->getSerial32(), 0 ); 
        }
    }
}
void /*cTargets::*/TargIdTarget(NXWSOCKET s) // Fraz
{
    char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
    const P_ITEM pi = pointers::findItemBySerPtr(buffer[s] + 7);
    if (pi)
    {
        if (buffer[s][7] >= 0x40)
        {
            if (pi && pi->magic != 4)
            {
                if (pi->getSecondaryNameC() &&(strncmp(pi->getSecondaryNameC(), "#", 1)))
                    pi->setCurrentName(pi->getSecondaryNameC());

                if ( strncmp(pi->getCurrentNameC(), "#", 1) )
                    pi->getName(temp2);
                else
                    strcpy(temp2, pi->getCurrentNameC());

                sprintf(temp, TRANSLATE("You found that this item appears to be called: %s"), temp2);
                sysmessage(s,  temp);
            }
        }
        if (pi->type != 15)
        {
            if (pi->type != 404)
            {
                sysmessage(s, TRANSLATE("This item has no hidden magical properties."));
            }
            else
            {
                sprintf(temp, TRANSLATE("It is enchanted with item identification, and has %d charges remaining."), pi->morex);
                sysmessage(s, temp);
            }
        }
        else
        {
            sprintf(temp, TRANSLATE("It is enchanted with the spell %s, and has %d charges remaining."), spellname[(8*(pi->morex - 1)) + pi->morey - 1], pi->morez);
            sysmessage(s, temp);
        }
    }
}
static void CstatsTarget(NXWCLIENT client, P_CHAR pc_stats )
{
	if ( client != NULL)
	{
		P_CHAR pc_user = MAKE_CHAR_REF( currchar[client->toInt()] );
		VALIDATEPC( pc_user );
		VALIDATEPC( pc_stats);
		newAmxEvent("gui_charProps")->Call( pc_stats->getSerial32(), pc_user->getSerial32(), 0 );
	}
}

static void MoveBelongingsToBp(P_CHAR pc, P_CHAR pc_2)
{

#pragma message( "MoveBelongingsToBp is written stupid and for what?" )
    P_ITEM pPack= pc->getBackpack();
    if (!pPack)
    {
        P_ITEM pPack=item::CreateFromScript( "$item_backpack" );
        VALIDATEPI(pPack);
		pc->packitemserial=pPack->getSerial32();
        //setserial(DEREF_P_ITEM(pPack),DEREF_P_CHAR(pc_2),4);
		pPack->setContSerial(pc_2->getSerial32());
    }

    NxwItemWrapper si;
	si.fillItemWeared(pc, false );
	for( si.rewind(); !si.isEmpty(); si++ )
    {
        P_ITEM pi=si.getItem();
		if(!ISVALIDPI(pi)) continue;
		if (pi->layer!=0x15 && pi->layer!=0x1D &&
            pi->layer!=0x10 && pi->layer!=0x0B )
        {
            pi->setContSerial( pPack->getSerial32() );
			pi->SetRandPosInCont( pPack );
            pi->layer=0x00;
            SndRemoveitem( pi->getSerial32() );
            pi->Refresh();
        }
        else if (pc->IsWearing(pi) &&
            (pi->layer==0x0B || pi->layer==0x10))   // hair & beard (Duke)
        {
            pi->deleteItem();
        }
    }
    pc_2->teleport();
}

static void GMTarget(NXWCLIENT ps, P_CHAR pc)
{
	VALIDATEPC(pc);

	if (ps == NULL) return;

//    NXWSOCKET  s = ps->toInt(); // unused variable
    //CHARACTER c = DEREF_P_CHAR(pc);

	P_CHAR curr=ps->currChar();
	VALIDATEPC(curr);
	
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    int i;
    if (pc->dead) return;
    if (SrvParms->gm_log)
		WriteGMLog(curr, "%s as made %s a GM.\n", curr->getCurrentNameC(), pc->getCurrentNameC());
 
    pc->unmountHorse();    //AntiChrist bugfix
    pc->gmrestrict = 0;
    pc->SetBodyType(BODY_GMSTAFF);
    pc->SetOldBodyType(BODY_GMSTAFF);
    pc->setSkinColor(0x8021);
    pc->setOldSkinColor(0x8021);
    pc->SetPriv(0xF7);
    pc->priv2 = (unsigned char) (0xD9);

    for (i = 0; i < 7; i++) // this overwrites all previous settings !
    {
        pc->priv3[i]=metagm[0][i]; // gm defaults
        pc->menupriv=1;
        if (pc->account==0) pc->priv3[i]=0xffffffff;
        pc->menupriv=-1; // LB, disabling menupriv stuff for gms per default
    }

    for (i = 0; i < TRUESKILLS; i++)
    {
        pc->baseskill[i]=1000;
        pc->skill[i]=1000;
    }

    // All stats to 100
    pc->setStrength(100);
    pc->st2 = 100;
    pc->hp  = 100;
    pc->stm = 100;
    pc->in  = 100;
    pc->in2 = 100;
    pc->mn  = 100;
    pc->mn2 = 100;
    pc->dx  = 100;
    pc->dx2 = 100;

//	if (strncmp(pc->name, "GM", 2))
	if (strncmp(pc->getCurrentNameC(), "GM", 2))
    {
        sprintf(temp, "GM %s", pc->getCurrentNameC());
		pc->setCurrentName(temp);
        //strcpy(pc->name,(char*)temp);
		//pc->setCurrentName( "GM " + pc->getCurrentName() );
    }
    MoveBelongingsToBp(pc,pc);
}

static void CnsTarget(NXWCLIENT ps, P_CHAR pc)
{
	if (ps == NULL) return;
	VALIDATEPC(pc);
//    NXWSOCKET  s = ps->toInt(); // unused variable

	P_CHAR curr=ps->currChar();
	VALIDATEPC(curr);

    //CHARACTER c = DEREF_P_CHAR(pc);
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    if (SrvParms->gm_log)
		WriteGMLog(curr, "%s as made %s a Counselor.\n", curr->getCurrentNameC(), pc->getCurrentNameC());

    pc->SetBodyType(BODY_GMSTAFF);
    pc->SetOldBodyType(BODY_GMSTAFF);
    pc->setSkinColor(0x8003);
    pc->setOldSkinColor(0x8002);
    pc->SetPriv(0xB6);
    pc->SetPriv2(0x8D);
    pc->gmrestrict = 0;
    if (strncmp(pc->getCurrentNameC(), "Counselor", 9))
    {
        sprintf(temp, "Counselor %s", pc->getCurrentNameC());
		pc->setCurrentName(temp);
//		strcpy(pc->name,(char*)temp);
//		pc->setCurrentName( "Counselor " + pc->getCurrentName() );
    }
    for (int u=0;u<7;u++) // this overwrites all previous settigns !!!
    {
        pc->priv3[u]=metagm[1][u]; // counselor defaults
        pc->menupriv=4;
        if (pc->account==0) pc->priv3[u]=0xffffffff;
    }
    MoveBelongingsToBp(pc,pc);
}

void KillTarget(P_CHAR pc, int ly)
{
    VALIDATEPC(pc);
	
	P_ITEM pi=pc->GetItemOnLayer( ly );
	if(ISVALIDPI(pi))
		pi->deleteItem();
}

void cTargets::GhostTarget(NXWSOCKET s)
{
    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
    if(ISVALIDPC(pc))
    {
        if(!pc->dead)
        {
		pc->attackerserial=pc->getSerial32();
		bolteffect(DEREF_P_CHAR(pc), true);
		pc->playSFX( 0x0029);
		pc->Kill();
        }
        else
            sysmessage(s,TRANSLATE("That player is already dead."));
    }
}

class cBoltTarget : public cCharTarget
{
public:
    cBoltTarget(NXWCLIENT pCli) : cCharTarget(pCli) {}
    void CharSpecific()
    {
        if (w_anim[0]==0 && w_anim[1]==0)
        {
            bolteffect(inx, true);
            P_CHAR pc_inx=MAKE_CHAR_REF(inx);
			if(ISVALIDPC(pc_inx))
				pc_inx->playSFX( 0x0029);
        }
        else
        {
            for (int j=0;j<=333;j++) bolteffect2(inx,w_anim[0],w_anim[1]);
        }
    }
};

class cSetAmountTarget : public cItemTarget
{
public:
    cSetAmountTarget(NXWCLIENT pCli) : cItemTarget(pCli) {}
    void ItemSpecific()
    {
        if (addx[s] > 64000) //Ripper..to fix a client bug for over 64k.
        {
            sysmessage(s, TRANSLATE("No amounts over 64k in a pile!"));
            return;
        }
		P_ITEM pi= MAKE_ITEM_REF(inx);
		VALIDATEPI(pi);

        pi->amount=addx[s];
        pi->Refresh();
    }
};

void cTargets::CloseTarget(NXWSOCKET s)
{
    if(s<=INVALID)
		return;
	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);
	P_CHAR pc=pointers::findCharBySerPtr(buffer[s]+7);
    if(ISVALIDPC(pc))
    {
		curr->sysmsg( TRANSLATE("Kicking player"));
        pc->kick();
    }
}

int cTargets::AddMenuTarget(NXWSOCKET s, int x, int addmitem)
{
    if ( (s>=0) && LongFromCharPtr(buffer[s]+11) == (long)0xFFFFFFFF)
		return INVALID;

    P_ITEM pi = item::CreateScriptItem(s, addmitem, 0);
    VALIDATEPIR( pi, INVALID );

    if (x)
		pi->Refresh();
    
    return DEREF_P_ITEM(pi);
}

// public !!!
int cTargets::NpcMenuTarget(NXWSOCKET s)
{
    if (s < 0) return INVALID; // Luxor

    cPacketTargeting targetData;

    if( targetData.getX( s ) <= 0 && targetData.getY( s ) <= 0 )
		return INVALID;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pc,INVALID);

    P_CHAR npc = npcs::AddRespawnNPC(pc,addmitem[s]);
    VALIDATEPCR(npc, INVALID);
    return DEREF_P_CHAR(npc);
}

void cTargets::VisibleTarget (NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);

    if(isItemSerial(serial))//item
    {
        P_ITEM pi = pointers::findItemBySerial(serial);
        if(ISVALIDPI(pi))
        {
            pi->visible=addx[s];
            pi->Refresh();
        }
    } else
    {//char
        P_CHAR pc = pointers::findCharBySerial(serial);
        if(ISVALIDPC(pc))
        {
            pc->hidden=addx[s];
	    pc->teleport();
        }
    }
}

/*!
\author Duke
\date 29/12/2000
\brief used by the '#empty' user-command
*/
static void ContainerEmptyTarget1(NXWCLIENT ps, P_ITEM pi)
{
	if (ps == NULL) return;
	VALIDATEPI(pi);
    NXWSOCKET  s = ps->toInt();
    if (pi->getSerial32() >= 0x40000000  // a container ?
        && pi->type==1)
    {
        addx[s]= pi->getSerial32();
        target(s, 0, 1, 0, 72, TRANSLATE("Select container to fill:"));
    }
    else
        sysmessage(s,TRANSLATE("That is not a valid container!"));

}
static void ContainerEmptyTarget2(NXWCLIENT ps, P_ITEM pNewCont)
{
	if (ps == NULL) return;
	/*VALIDATEPI(pNewCont);
    NXWSOCKET  s = ps->toInt();
    if (pNewCont->type==1)
    {
        P_ITEM pi;  // item to move from old container
        int ci=0,loopexit=0;
        while ( ((pi=ContainerSearch(addx[s],&ci)) != NULL) && (++loopexit < MAXLOOPS) )
            pNewCont->AddItem(pi);
    }
    else
        sysmessage(s,TRANSLATE("That is not a valid container!"));
*/
	sysmessage( ps->toInt(), "DISABLED" );
}

static void OwnerTarget(NXWCLIENT ps, P_CHAR pc)
{
	if (ps == NULL) return;
	VALIDATEPC(pc);
    NXWSOCKET  s = ps->toInt();

    int addser=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
    pc->setOwnerSerial32(addser);
    if (addser==-1)
    {
        pc->tamed=false;
    }
    else
    {
        pc->tamed=true;
    }
}

static void OwnerTarget(NXWCLIENT ps, P_ITEM pi)
{
	if (ps == NULL) return;
	VALIDATEPI(pi);
    NXWSOCKET  s = ps->toInt();

    int os=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
    pi->setOwnerSerial32(os);

    if (pi->visible==1)
    {
        for (int j=0;j<now;j++) if (perm[j]) senditem(j,pi); // necassairy for items with visible value of 1
    }
}

void cTargets::DvatTarget(NXWSOCKET s)
{
	P_CHAR Me = MAKE_CHAR_REF(currchar[s]);   
	VALIDATEPC(Me); 
    
	P_ITEM pi=pointers::findItemBySerPtr(buffer[s] +7);

	if (pi && pi->dye==1)//if dyeable
	{
		P_CHAR pc = pi->getPackOwner();

		if( pc == Me  || pi->isInWorld())
		{
			pi->setColor( (addid1[s]<<8)|(addid2[s]%256) );
			pi->Refresh();
			Me->playSFX(0x023E); // plays the dye sound, LB
		} else {
			Me->sysmsg(TRANSLATE("That is not yours!!"));
		}
	} else {
		Me->sysmsg(TRANSLATE("You can only dye clothes with this."));
	}
}

static void AddNpcTarget(NXWSOCKET s, PKGx6C *pp)
{
	if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
	P_CHAR pc=archive::getNewChar();

	pc->setCurrentName("Dummy");
	pc->SetBodyType((addid1[s]<<8)|(addid2[s]%256));
	pc->SetOldBodyType((addid1[s]<<8)|(addid2[s]%256));
	pc->setSkinColor(0);
	pc->setOldSkinColor(0);
	pc->SetPriv(0x10);
	/*
    pc->x= pp->TxLoc;
    pc->y= pp->TyLoc;
    pc->dispz=pc->z= pp->TzLoc+Map->TileHeight(pp->model);
	*/
	pc->MoveTo( pp->TxLoc, pp->TyLoc, pp->TzLoc+Map->TileHeight(pp->model) );
	pc->npc=1;
        pc->teleport();
}

void cTargets::AllSetTarget(NXWSOCKET s)
{
    int j, k;

    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if(i!=-1)
    {
        k=calcSocketFromChar(i);
        if (addx[s]<TRUESKILLS)
        {
            pc->baseskill[addx[s]]=addy[s];
			Skills::updateSkillLevel(pc, addx[s]);
            if (k!=-1) updateskill(k, addx[s]);
        }
        else if (addx[s]==ALLSKILLS)
        {
            for (j=0;j<TRUESKILLS;j++)
            {
                pc->baseskill[j]=addy[s];
				Skills::updateSkillLevel(pc, j);
                if (k!=-1) updateskill(k,j);
            }
        }
        else if (addx[s]==STR)
        {
            pc->setStrength(addy[s]);
            pc->st3=addy[s];
            for (j=0;j<TRUESKILLS;j++)
            {
				Skills::updateSkillLevel(pc,j);
                if (k!=-1) updateskill(k,j);
            }
            if (k!=-1) statwindow(pc,pc);
        }
        else if (addx[s]==DEX)
        {
            pc->dx=addy[s];
            pc->dx3=addy[s];
            for (j=0;j<TRUESKILLS;j++)
            {
				Skills::updateSkillLevel(pc,j);
                if (k!=-1) updateskill(k,j);
            }
            if (k!=-1) statwindow(pc,pc);
        }
        else if (addx[s]==INTEL)
        {
            pc->in=addy[s];
            pc->in3=addy[s];
            for (j=0;j<TRUESKILLS;j++)
            {
				Skills::updateSkillLevel(pc,j);
                if (k!=-1) updateskill(k,j);
            }
            if (k!=-1) statwindow(pc,pc);
        }
        else if (addx[s]==FAME)
        {
            pc->SetFame(addy[s]);
        }
        else if (addx[s]==KARMA)
        {
            pc->SetKarma(addy[s]);
        }
        else if (addx[s]==I_ACCOUNT)
        {
            pc->account = addy[s];
        }
        else if ((addx[s]>=NXWFLAG0)&&(addx[s]<=NXWFLAG3))
        {
            pc->nxwflags[addx[s]-NXWFLAG0] = addy[s];
        }
        else if ((addx[s]>=AMXFLAG0)&&(addx[s]<=AMXFLAGF))
        {
						//
						// OLD AMXFLAGS ARE NOW HANDLED BY NEW STYLE AMXVARS
						//
						amxVS.updateVariable( pc->getSerial32(), addx[s]-AMXFLAG0, addy[s] );
        }


    }
}

static void InfoTarget(NXWSOCKET s, PKGx6C *pp) // rewritten to work also with map-tiles, not only static ones by LB
{
    int tilenum,x1,y1,x2,y2,x,y;
    signed char z;
    unsigned long int pos;
    tile_st tile;
    struct map_st
    {
        short int id;
        signed char z;
    };
    map_st map1;
    land_st land;

    if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
    x=pp->TxLoc;
    y=pp->TyLoc;
    z=pp->TzLoc;

    if (pp->model==0)   // damn osi not me why the tilenum is only send for static tiles, LB
    {   // manually calculating the ID's if it's a maptype
        x1=x/8;
        y1=y/8;
        x2=(x-(x1*8));
        y2=(y-(y1*8));
        pos=(x1*512*196)+(y1*196)+(y2*24)+(x2*3)+4;
        fseek(mapfile, pos, SEEK_SET);
        fread(&map1, 3, 1, mapfile);
        Map->SeekLand(map1.id, &land);
        ConOut("type: map-tile\n");
        ConOut("tilenum: %i\n",map1.id);
        ConOut("Flag1:%x\n", land.flag1);
        ConOut("Flag2:%x\n", land.flag2);
        ConOut("Flag3:%x\n", land.flag3);
        ConOut("Flag4:%x\n", land.flag4);
        ConOut("Unknown1:%lx\n", land.unknown1);
        ConOut("Unknown2:%x\n", land.unknown2);
        ConOut("Name:%s\n", land.name);
    }
    else
    {
        tilenum=pp->model; // lb, bugfix
        Map->SeekTile(tilenum, &tile);
        ConOut("type: static-tile\n");
        ConOut("tilenum: %i\n",tilenum);
        ConOut("Flag1:%x\n", tile.flag1);
        ConOut("Flag2:%x\n", tile.flag2);
        ConOut("Flag3:%x\n", tile.flag3);
        ConOut("Flag4:%x\n", tile.flag4);
        ConOut("Weight:%x\n", tile.weight);
        ConOut("Layer:%x\n", tile.layer);
        ConOut("Anim:%lx\n", tile.animation);
        ConOut("Unknown1:%lx\n", tile.unknown1);
        ConOut("Unknown2:%x\n", tile.unknown2);
        ConOut("Unknown3:%x\n", tile.unknown3);
        ConOut("Height:%x\n", tile.height);
        ConOut("Name:%s\n", tile.name);
    }
    sysmessage(s, TRANSLATE("Item info has been dumped to the console."));
    ConOut("\n");
}

/*
void cTargets::TweakTarget(NXWSOCKET s)//Lag fix -- Zippy
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if (serial == 0) //Client sends zero if invalid!
        return;
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );   
    VALIDATEPC( pc ); 

    if (isCharSerial(serial))//Char
    {
        if (pointers::findCharBySerial(serial) == NULL) // but let's make sure
            return;
		newAmxEvent("gui_charProps")->Call( serial, pc->getSerial32(), 1 );
        //tweakmenu(s, serial);
    }
    else
    {//item
        if (FindItemBySerial(serial) == NULL)
            return;
		newAmxEvent("gui_itemProps")->Call( serial, pc->getSerial32(), 1 );
        //tweakmenu(s, serial);
    }
}
*/

static void SetInvulFlag(NXWCLIENT ps, P_CHAR pc)
{
    if (addx[ps->toInt()]==1)
        pc->MakeInvulnerable();
    else
        pc->MakeVulnerable();
}

static void Tiling(NXWSOCKET s, PKGx6C *pp) // Clicking the corners of tiling calls this function - Crwth 01/11/1999
{
    if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
    if (clickx[s]==-1 && clicky[s]==-1)
    {
        clickx[s]=pp->TxLoc;
        clicky[s]=pp->TyLoc;
        target(s,0,1,0,198,TRANSLATE("Select second corner of bounding box."));
        return;
    }

    int pileable=0;
    tile_st tile;
    int x1=clickx[s],x2=pp->TxLoc;
    int y1=clicky[s],y2=pp->TyLoc;

    clickx[s]=-1;clicky[s]=-1;

    if (x1>x2) { int temp=x1;x1=x2;x2=temp;}
    if (y1>y2) { int temp=y1;y1=y2;y2=temp;}

    if (addid1[s]==0x40)
    {
        switch (addid2[s])
        {
        case 100:
        case 102:
        case 104:
        case 106:
        case 108:
        case 110:
        case 112:
        case 114:
        case 116:
        case 118:
        case 120:
        case 122:
        case 124:
        case 126:
        case 140:
            AddTarget(s,pp);
            return;
        }
    }

    int x,y;

    Map->SeekTile((addid1[s]<<8)+addid2[s], &tile);
    if (tile.flag2&0x08) pileable=1;
    for (x=x1;x<=x2;x++)
        for (y=y1;y<=y2;y++)
        {
            P_ITEM pi=item::SpawnItem(s, 1, "#", pileable, (addid1[s]<<8)+addid2[s], 0, 0,0);
            VALIDATEPI(pi);
            pi->setDecay( false );
		pi->setPosition( x, y, pp->TzLoc+Map->TileHeight(pp->model));
            pi->Refresh();
            mapRegions->add(pi); // lord Binary, xan, God Rah
        }

    addid1[s]=0;
    addid2[s]=0;
}

/*
void cTargets::Wiping(NXWSOCKET s) // Clicking the corners of wiping calls this function - Crwth 01/11/1999
{
    if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return;

    if (clickx[s]==-1 && clicky[s]==-1) {
        clickx[s]=(buffer[s][11]<<8)+buffer[s][12];
        clicky[s]=(buffer[s][13]<<8)+buffer[s][14];
        if (addid1[s]) target(s,0,1,0,199,TRANSLATE("Select second corner of inverse wiping box."));
        else target(s,0,1,0,199,TRANSLATE("Select second corner of wiping box."));
        return;
    }

    int x1=clickx[s],x2=(buffer[s][11]<<8)+buffer[s][12];
    int y1=clicky[s],y2=(buffer[s][13]<<8)+buffer[s][14];

    clickx[s]=-1;clicky[s]=-1;

    int c;
    if (x1>x2) {c=x1;x1=x2;x2=c;}
    if (y1>y2) {c=y1;y1=y2;y2=c;}
    unsigned int i ;
    if (addid1[s]==1)
    { // addid1[s]==1 means to inverse wipe
        for ( i=0;i<itemcount;i++)
        {
            P_ITEM pi=MAKE_ITEM_REF(i);
			if(!ISVALIDPI(pi))
				continue;
            if (!((pi->getPosition().x >= x1) &&
				(pi->getPosition().x <= x2) &&
				(pi->getPosition().y >= y1) &&
				(pi->getPosition().y <= y2)) && pi->isInWorld() && (pi->wipe==0))
					pi->deleteItem();
        }
    }
    else
    {
        for (i=0;i<itemcount;i++)
        {
            P_ITEM pi=MAKE_ITEM_REF(i);
			if(!ISVALIDPI(pi))
				continue;
            if ((pi->getPosition().x >= x1) &&
				(pi->getPosition().x <= x2) &&
				(pi->getPosition().y >= y1) &&
				(pi->getPosition().y <= y2) && pi->isInWorld() && pi->wipe==0)
					pi->deleteItem();
        }
    }
}
*/

static void ExpPotionTarget(NXWSOCKET s, PKGx6C *pp) //Throws the potion and places it (unmovable) at that spot
{
    P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    int x, y, z;
    if(pp->TxLoc==-1 || pp->TyLoc==-1) return;
    x=pp->TxLoc;
    y=pp->TyLoc;
    z=pp->TzLoc;

    // ANTICHRIST -- CHECKS LINE OF SIGHT!
    if(line_of_sight(s, pc->getPosition(), Loc(x,y,z,z), WALLS_CHIMNEYS + DOORS + ROOFING_SLANTED))
    {
        P_ITEM pi=pointers::findItemBySerial(calcserial(addid1[s],addid2[s],addid3[s],addid4[s]));
        if (ISVALIDPI(pi)) // crashfix LB
        {
            pi->MoveTo( x, y, z);
            pi->setContSerial(INVALID);
            pi->magic=2; //make item unmovable once thrown
            movingeffect2(DEREF_P_CHAR(pc), DEREF_P_ITEM(pi), 0x0F, 0x0D, 0x11, 0x00, 0x00);
            pi->Refresh();
        }
    }
    else sysmessage(s,TRANSLATE("You cannot throw the potion there!"));
}

static void Priv3Target(NXWSOCKET  s, P_CHAR pc)
{
    VALIDATEPC(pc)

    pc->priv3[0]=priv3a[s];
    pc->priv3[1]=priv3b[s];
    pc->priv3[2]=priv3c[s];
    pc->priv3[3]=priv3d[s];
    pc->priv3[4]=priv3e[s];
    pc->priv3[5]=priv3f[s];
    pc->priv3[6]=priv3g[s];
}

void cTargets::SquelchTarg(NXWSOCKET s)
{


    P_CHAR pc =pointers::findCharBySerPtr(buffer[s]+7);
    if (ISVALIDPC(pc))
    {
        if(pc->IsGM())
        {
            sysmessage(s, TRANSLATE("You cannot squelch GMs."));
            return;
        }
        if (pc->squelched)
        {
            pc->squelched=0;
            sysmessage(s, TRANSLATE("Un-squelching..."));
            sysmessage(calcSocketFromChar(DEREF_P_CHAR(pc)), TRANSLATE("You have been unsquelched!"));
            pc->mutetime=0;
        }
        else
        {
            pc->mutetime=0;
            pc->squelched=1;
            sysmessage(s, TRANSLATE("Squelching..."));
            sysmessage(calcSocketFromChar(DEREF_P_CHAR(pc)), TRANSLATE("You have been squelched!"));

            if (addid1[s]!=255 || addid1[s]!=0)
            {
                pc->mutetime=(unsigned int) (uiCurrentTime+(addid1[s]*MY_CLOCKS_PER_SEC));
                addid1[s]=255;
                pc->squelched=2;
            }
        }
    }
}

/*!
\brief implements the 'telestuff GM command
\author Luxor
*/
static void TeleStuff(NXWSOCKET s, PKGx6C *pp)
{
	static P_OBJECT po = NULL;

	if (s < 0 || s >= now)
		return;

	P_CHAR pc = pointers::findCharBySerial(currchar[s]);
	VALIDATEPC(pc);

	if ( po == NULL ) {
		po = objects.findObject( LongFromCharPtr(buffer[s]+7) );
		if ( !ISVALIDPO(po) ) {
			po = NULL;
			return;
		}
		target(s,0,1,0,222,TRANSLATE("Select location to put this object."));
	} else {
		SI32 x, y, z;
		x = pp->TxLoc;
		y = pp->TyLoc;
		z = pp->TzLoc + Map->TileHeight(pp->model);
		if ( x < 0 || y < 0 ) {
			po = NULL;
			return;
		}

		SERIAL nSerial = INVALID;
		nSerial = po->getSerial32();
		if (isCharSerial(nSerial)) {
			P_CHAR pt = static_cast<P_CHAR>(po);
			if (!ISVALIDPC(pt)) {
				po = NULL;
				return;
			}
			pt->MoveTo(x,y,z);
			pt->teleport();
		} else if (isItemSerial(nSerial)) {
			P_ITEM pi = static_cast<P_ITEM>(po);
			if (!ISVALIDPI(pi)) {
				po = NULL;
				return;
			}
			pi->MoveTo(x,y,z);
			pi->Refresh();
		}
		po = NULL;
	}
}

void CarveTarget(NXWSOCKET s, int feat, int ribs, int hides, int fur, int wool, int bird)
{

	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC( pc );

	P_ITEM pi1=item::SpawnItem(s,1,"#",0,0x122A,0,0,0);    //add the blood puddle
	VALIDATEPI(pi1);
	P_ITEM pi2=MAKE_ITEMREF_LR(npcshape[0]);
	VALIDATEPI(pi2);

	mapRegions->remove(pi1);

	pi1->setPosition( pi2->getPosition() );
	pi1->magic=2;//AntiChrist - makes the item unmovable

	mapRegions->add(pi1); // lord Binary

	pi1->setDecayTime();
	pi1->Refresh();

	if(feat>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_feathers", pc->getBackpack() );
		VALIDATEPI(pi);
		pi->setAmount( feat );
		pi->Refresh();
		sysmessage(s,TRANSLATE("You pluck the bird and get some feathers."));
	}
	if(ribs>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_cuts_of_raw_ribs", pc->getBackpack() );
		VALIDATEPI(pi);
		pi->setAmount( ribs );
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You carve away some meat."));
	}

	if(hides>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_hide", pc->getBackpack() );
		VALIDATEPI(pi);
		pi->setAmount( hides );
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You skin the corpse and get the hides."));
	}
	if(fur>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_hide", pc->getBackpack() );
		VALIDATEPI(pi);
		pi->setAmount( fur );
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You skin the corpse and get the hides."));
	}
	if(wool>0)
	{
		P_ITEM pi=item::CreateFromScript( "$item_piles_of_wool", pc->getBackpack() );
		VALIDATEPI(pi);
		pi->setAmount( wool );
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You skin the corpse and get some unspun wool."));
	}
	if(bird>0)
	{
		P_ITEM pi=item::SpawnItem(s,bird,"raw bird",1,0x09B9,0,1,1);
		VALIDATEPI(pi);
		pc->sysmsg(TRANSLATE("You carve away some raw bird."));
	}

	weights::NewCalc(pc);
}

/*!
\author AntiChrist
\date 03/11/1999
\brief New Carving System
\note Human-corpse carving code added
\note Scriptable carving product added
*/
static void newCarveTarget(NXWSOCKET  s, ITEM i)
{
	bool deletecorpse=false;
	NXWSOCKET storeval;
	char sect[512];
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	P_ITEM pi1=item::SpawnItem(s,1,"#",0,0x122A,0,0,0);    //add the blood puddle
	VALIDATEPI(pi1);
	P_ITEM pi2=MAKE_ITEMREF_LR(npcshape[0]);
	VALIDATEPI(pi2);
	P_ITEM pi3=MAKE_ITEMREF_LR(i);
	VALIDATEPI(pi3);
	mapRegions->remove(pi1);
	pi1->setPosition( pi2->getPosition() );
	pi1->magic=2;//AntiChrist - makes the item unmovable
	mapRegions->add(pi1); // lord Binary

	pi1->setDecayTime();

	pi1->Refresh();

	//if it's a human corpse
	if(pi3->morey)
	{
		pc->modifyFame(ServerScp::g_nChopFameLoss); // Ripper..lose fame and karma and criminal.
		pc->IncreaseKarma(+ServerScp::g_nChopKarmaLoss);
		pc->sysmsg(TRANSLATE("You lost some fame and karma!"));
		setCrimGrey(pc, ServerScp::g_nChopWillCriminal);//Blue and not attacker and not guild

		//create the Head
		sprintf(temp,"the head of %s",pi3->getSecondaryNameC());
		P_ITEM pi=item::SpawnItem(s,1,temp,0,0x1DA0,0,0,0);
		VALIDATEPI(pi);
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		//AntiChrist & Magius(CHE) - store item's owner, so that lately
		//if we want to know the owner we can do it
		pi->setSameOwnerAs(pi3);
		pi->Refresh();

		//create the Heart
		sprintf(temp,"the heart of %s",pi3->getSecondaryNameC());
		pi=item::CreateFromScript( "$item_a_heart" );
		VALIDATEPI(pi);
		sprintf(temp,"the heart of %s",pi3->getSecondaryNameC());
		pi->setCurrentName( temp );
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->Refresh();

		//create the Body
		sprintf(temp,"the body of %s",pi3->getSecondaryNameC());
		pi=item::SpawnItem(s,1,temp,0,0x1DAD,0,0,0);
		VALIDATEPI(pi);
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->Refresh();

		//create the Left Arm
		sprintf(temp,"the left arm of %s",pi3->getSecondaryNameC());
		pi=item::SpawnItem(s,1,temp,0,0x1DA1,0,0,0);
		VALIDATEPI(pi);
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->Refresh();

		//create the Right Arm
		sprintf(temp,"the right arm of %s",pi3->getSecondaryNameC());
		pi=item::SpawnItem(s,1,temp,0,0x1DA2,0,0,0);
		VALIDATEPI(pi);
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->Refresh();

		//create the Left Leg
		sprintf(temp,"the left leg of %s",pi3->getSecondaryNameC());
		pi=item::SpawnItem(s,1,temp,0,0x1DA3,0,0,0);
		VALIDATEPI(pi);
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->Refresh();

		//create the Rigth Leg
		sprintf(temp,"the right leg of %s",pi3->getSecondaryNameC());
		pi=item::SpawnItem(s,1,temp,0,0x1DA4,0,0,0);
		VALIDATEPI(pi);
		pi->setContSerial(INVALID);
		pi->MoveTo(pi3->getPosition());
		pi->layer=0x01;
		pi->att=5;
		pi->setSameOwnerAs(pi3);  // see above
		pi->Refresh();

		//human: always delete corpse!
		deletecorpse=true;
	}
		else
	{
		cScpIterator* iter = NULL;
		char script1[1024];
		char script2[1024];
		sprintf(sect,"SECTION CARVE %i",pi3->carve);
		iter = Scripts::Carve->getNewIterator(sect);
		if (iter==NULL) return;


		int loopexit=0;
		do
		{
		iter->parseLine(script1, script2);
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			if (!(strcmp("ADDITEM",script1)))
			{
			storeval=str2num(script2);
			P_ITEM pi=item::CreateScriptItem(s,storeval,0);
					if(ISVALIDPI(pi)) {
						pi->layer=0;
						pi->setCont(pi3);
						pi->setPosition( 20+(rand()%50), 85+(rand()%75), 9);
						pi->Refresh();//let's finally refresh the item
					}
                }
            }
        }
        while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

        safedelete(iter);
    }


    if(deletecorpse)//if corpse has to be deleted
    {
        
		NxwItemWrapper si;
		si.fillItemsInContainer( pi3, false );
		for( si.rewind(); !si.isEmpty(); si++ )
        {
            P_ITEM pj=si.getItem();
			if(ISVALIDPI(pj)) {
				pj->setContSerial(INVALID);
				pj->MoveTo( pi3->getPosition() );
				pj->setDecayTime();
				pj->Refresh();
			}
        }
        pi3->deleteItem();
    }
}

static void CorpseTarget(const NXWCLIENT pC)
{
	if (pC == NULL) return;
    int n=0;
    NXWSOCKET  s = pC->toInt();

    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcItemFromSer(serial);
    P_ITEM pi=MAKE_ITEMREF_LR(i);
    P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
    if(ISVALIDPI(pi))
    {
        if(item_inRange(pc,pi,1))
        {
            npcshape[0]=i;
            pc->playAction(0x20);
            n=1;
            if(pi->more1==0)
            {
                pi->more1=1;//corpse being carved...can't carve it anymore

                if(pi->morey || pi->carve>-1)
                {//if specified, use enhanced carving system!
                    newCarveTarget(s, i);//AntiChrist
                } else
                {//else use standard carving
                    switch(pi->amount) {
                    case 0x01: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Ogre
                    case 0x02: CarveTarget(s, 0, 5, 0, 0, 0, 0); break; //Ettin
                    case 0x03: break;   //Zombie
                    case 0x04: break;   //Gargoyle
                    case 0x05: CarveTarget(s,36, 0, 0, 0, 0, 1); break; //Eagle
                    case 0x06: CarveTarget(s,25, 0, 0, 0, 0, 1); break; //Bird
                    case 0x07: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Orc w/axe
                    case 0x08: break;   //Corpser
                    case 0x09: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Deamon
                    case 0x0A: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Deamon w/sword
                    case 0x0B: break;   //-NULL-
                    case 0x0C: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Dragon (green)
                    case 0x0D: break;   //Air Elemental
                    case 0x0E: break;   //Earth Elemental
                    case 0x0F: break;   //Fire Elemental
                    case 0x10: break;   //Water Elemental
                    case 0x11: CarveTarget(s, 0, 3, 0, 0, 0, 0); break; //Orc
                    case 0x12: CarveTarget(s, 0, 5, 0, 0, 0, 0); break; //Ettin w/club
                    case 0x13: break; //-NULL-
                    case 0x14: break; //-NULL-
                    case 0x15: CarveTarget(s, 0, 4,20, 0, 0, 0); break; //Giant Serpent
                    case 0x16: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Gazer
                    case 0x17: break;   //-NULL-
                    case 0x18: break;   //Liche
                    case 0x19: break;   //-NULL-
                    case 0x1A: break;   //Ghoul
                    case 0x1B: break;   //-NULL-
                    case 0x1C: break;   //Spider
                    case 0x1D: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Gorilla
                    case 0x1E: CarveTarget(s,50, 0, 0, 0, 0, 1); break; //Harpy
                    case 0x1F: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Headless
                    case 0x20: break;   //-NULL-
                    case 0x21: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman
                    case 0x0122: CarveTarget(s, 0,10, 0, 0, 0, 0); break; // Boar
                    case 0x23: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman w/spear
                    case 0x24: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Lizardman w/mace
                    case 0x25: break;   //-NULL-
                    case 0x26: break;   //-NULL-
                    case 0x27: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Mongbat
                    case 0x28: break;   //-NULL-
                    case 0x29: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Orc w/club
                    case 0x2A: break;   //Ratman
                    case 0x2B: break;   //-NULL-
                    case 0x2C: break;   //Ratman w/axe
                    case 0x2D: break;   //Ratman w/dagger
                    case 0x2E: break;   //-NULL-
                    case 0x2F: break;   //Reaper
                    case 0x30: break;   //Scorpion
                    case 0x31: break;   //-NULL-
                    case 0x32: break;   //Skeleton
                    case 0x33: break;   //Slime
                    case 0x34: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Snake
                    case 0x35: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll w/axe
                    case 0x36: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll
                    case 0x37: CarveTarget(s, 0, 2, 0, 0, 0, 0); break; //Troll w/club
                    case 0x38: break;   //Skeleton w/axe
                    case 0x39: break;   //Skeleton w/sword
                    case 0x3A: break;   //Wisp
                    case 0x3B: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Dragon (red)
                    case 0x3C: CarveTarget(s, 0,10,20, 0, 0, 0); break; //Drake (green)
                    case 0x3D: CarveTarget(s, 0,10,20, 0, 0, 0); break; //Drake (red)
                    case 0x46: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen Matriarche - t2a
                    case 0x47: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen drone - t2a
                    case 0x48: CarveTarget(s, 0, 0, 0, 0, 0, 0); break; //Terathen warrior, Terathen Avenger - t2a
                    case 0x4B: CarveTarget(s, 0,4, 0, 0, 0, 0); break; //Titan - t2a
                    case 0x4C: CarveTarget(s, 0, 4, 0, 0, 0, 0); break; //Cyclopedian Warrior - t2a
                    case 0x50: CarveTarget(s, 0,10, 2, 0, 0, 0); break; //Giant Toad - t2a
                    case 0x51: CarveTarget(s, 0, 4, 1, 0, 0, 0); break; //Bullfrog - t2a
                    case 0x55: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian apprentice, Ophidian Shaman - t2a
                    case 0x56: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian warrior, Ophidian Enforcer, Ophidian Avenger - t2a
                    case 0x57: CarveTarget(s, 0, 5, 7, 0, 0, 0); break; //Ophidian Matriarche - t2a
                    case 0x5F: CarveTarget(s, 0,19,20, 0, 0, 0); break; //Kraken - t2a
                        //case 0x3E-case 0x95: break; //-NULL-
                    case 0x96: CarveTarget(s, 0,10, 0, 0, 0, 0); break; //Sea Monster
                    case 0x97: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Dolphin
                        //case 0x98-case 0xC7: break; //-NULL-
                    case 0xC8: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (tan)
                    case 0xC9: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Cat
                    case 0xCA: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Alligator
                    case 0xCB: CarveTarget(s, 0, 6, 0, 0, 0, 0); break; //Pig
                    case 0xCC: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (dark)
                    case 0xCD: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Rabbit
                    case 0xCE: CarveTarget(s, 0, 1,12, 0 ,0, 0); break; //Lava Lizard - t2a
                    case 0xCF: CarveTarget(s, 0, 3, 0, 0, 1, 0); break; //Sheep
                    case 0xD0: CarveTarget(s,25, 0, 0, 0, 0, 1); break; //Chicken
                    case 0xD1: CarveTarget(s, 0, 2, 8, 0, 0, 0); break; //Goat
                    case 0xD2: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Desert Ostarge - t2a
                    case 0xD3: CarveTarget(s, 0, 1, 12, 0, 0, 0); break; //Bear
                    case 0xD4: CarveTarget(s, 0, 1, 0, 2, 0, 0); break; //Grizzly Bear
                    case 0xD5: CarveTarget(s, 0, 2, 0, 3, 0, 0); break; //Polar Bear
                    case 0xD6: CarveTarget(s, 0, 1, 10, 0, 0, 0); break; //Cougar
                    case 0xD7: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Giant Rat
                    case 0xD8: CarveTarget(s, 0, 8,12, 0, 0, 0); break; //Cow (black)
                    case 0xD9: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Dog
                    case 0xDA: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Frenzied Ostard - t2a
                    case 0xDB: CarveTarget(s, 0,15, 0, 0, 0, 0); break; //Forest Ostard - t2a
                    case 0xDC: CarveTarget(s, 0, 1, 12,0, 0, 0); break; //Llama
                    case 0xDD: CarveTarget(s, 0, 1,12, 0, 0, 0); break; //Walrus
                    case 0xDE: break;   //-NULL-
                    case 0xDF: CarveTarget(s, 0, 3, 0, 0, 0, 0); break; //Sheep (BALD)
                    case 0xE1: CarveTarget(s, 0, 1, 0, 1, 0, 0); break; //Timber Wolf
                    case 0xE2: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (Silver)
                    case 0xE3: break;   //-NULL-
                    case 0xE4: CarveTarget(s, 0, 3,10, 0, 0, 0); break; //Horse (tan)
                    case 0xE5: break;   //-NULL-
                    case 0xE6: break;   //-NULL-
                    case 0xE7: CarveTarget(s, 0, 8,12, 0, 0, 0); break; //Cow (brown)
                    case 0xE8: CarveTarget(s, 0,10,15, 0, 0, 0); break; //Bull (brown)
                    case 0xE9: CarveTarget(s, 0,10,15, 0, 0, 0); break; //Bull (d-brown)
                    case 0xEA: CarveTarget(s, 0, 6,15, 0, 0, 0); break; //Great Heart
                    case 0xEB: break;   //-NULL-
                    case 0xEC: break;   //-NULL-
                    case 0xED: CarveTarget(s, 0, 5, 8, 0, 0, 0); break; //Hind
                    case 0xEE: CarveTarget(s, 0, 1, 0, 0, 0, 0); break; //Rat
                        //case 0xEF-case 0xFF: break; //-NULL-
                    default:
                        LogError("Fallout of switch statement, corpsetarget()");
                    }// switch
                }//if morey || carve>-1
            } else {
                 sysmessage(s, TRANSLATE("You carve the corpse but find nothing usefull."));
            }// if more1==0
        //break;
        }
    }// if i!=-1
    if (!n) sysmessage(s, TRANSLATE("That is too far away."));
}

void cTargets::SwordTarget(const NXWCLIENT pC)
{
	if (pC == NULL) return;
	NXWSOCKET  s = pC->toInt();
        P_CHAR pc = pC->currChar();
	VALIDATEPC(pc);

	if (LongFromCharPtr(buffer[s] +11) == INVALID) return;

	short id = ShortFromCharPtr(buffer[s] +17);
	if (itemById::IsTree2(id))
	{
		int px,py,cx,cy;
		Location pcpos= pc->getPosition();
   	//<Luxor>
		px= ShortFromCharPtr(buffer[s] +11);
		py= ShortFromCharPtr(buffer[s] +13);
		cx= abs((int)pcpos.x - px);
		cy= abs((int)pcpos.y - py);
		if(!((cx<=5)&&(cy<=5)))
		{
			pc->sysmsg(TRANSLATE("You are to far away to reach that"));
			return;
		}
	//</Luxor>

		pc->playAction( pc->isMounting() ? 0x0D : 0x01D );
		pc->playSFX(0x013E);

		const P_ITEM pi=item::CreateFromScript( "$item_kindling" );
		VALIDATEPI(pi);

		pi->setPosition( pcpos );
		mapRegions->add(pi);
		pi->Refresh();
		pc->sysmsg(TRANSLATE("You hack at the tree and produce some kindling."));
	}
	else if(itemById::IsLog(id)) // vagrant
	{
		Skills::BowCraft(s);
	} else if(itemById::IsCorpse(id))
	{
		CorpseTarget(pC);
	} else
		pc->sysmsg(TRANSLATE("You can't think of a way to use your blade on that."));
}

void cTargets::NpcTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s] +7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
    {
        addid1[s]= pc->getSerial().ser1;
        addid2[s]= pc->getSerial().ser2;
        addid3[s]= pc->getSerial().ser3;
        addid4[s]= pc->getSerial().ser4;
        target(s, 0, 1, 0, 57, TRANSLATE("Select NPC to follow this player."));
    }
}

void cTargets::NpcTarget2(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
        if (pc->npc==1)
        {
            pc->ftargserial=calcserial(addid1[s], addid2[s], addid3[s], addid4[s]);
            pc->npcWander=1;
        }
}

void cTargets::NpcRectTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
        if ((pc->npc==1))
        {
            pc->fx1=addx[s];
            pc->fy1=addy[s];
            pc->fz1=-1;
            pc->fx2=addx2[s];
            pc->fy2=addy2[s];
            pc->npcWander=3;
        }
}

void cTargets::NpcCircleTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
        if ((pc->npc==1))
        {
            pc->fx1=addx[s];
            pc->fy1=addy[s];
            pc->fz1=-1;
            pc->fx2=addx2[s];
            pc->npcWander=2; // bugfix, LB
        }
}

void cTargets::NpcWanderTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
        if ((pc->npc==1)) pc->npcWander=npcshape[0];
}

//taken from 6904t2(5/10/99) - AntiChrist
void cTargets::NpcAITarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
    {
        pc->npcaitype=addx[s];
        sysmessage(s, "Npc AI changed.");//AntiChrist
    }
}

void cTargets::xBankTarget(NXWSOCKET s)
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	P_CHAR pc2 = pointers::findCharBySerPtr(buffer[s] +7);
	VALIDATEPC(pc);
	VALIDATEPC(pc2);

	pc->openBankBox(pc2);
}

void cTargets::xSpecialBankTarget(NXWSOCKET s)
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	P_CHAR pc2 = pointers::findCharBySerPtr(buffer[s] +7);
	VALIDATEPC(pc);
	VALIDATEPC(pc2);

        pc->openSpecialBank(pc2);
}

void cTargets::SellStuffTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    if (i!=-1)
    {
        sellstuff(s, i);
    }
}

void cTargets::ReleaseTarget(NXWSOCKET s, int c)
{
    
	P_CHAR pc = NULL;
	if (c==INVALID)
		pc = pointers::findCharBySerPtr(buffer[s] + 7);
	else
		pc = pointers::findCharBySerial(c);
	P_CHAR rel = MAKE_CHAR_REF(currchar[s]);

	prison::release( rel, pc  );
}

/*!
\brief opens a creature's backpack
\author Luxor
*/
void cTargets::GmOpenTarget(NXWSOCKET s)
{
	P_ITEM pi;
	P_CHAR pc= MAKE_CHAR_REF(currchar[s]);
	P_CHAR pc_target = pointers::findCharBySerPtr(buffer[s]+7);
	VALIDATEPC(pc);
	VALIDATEPC(pc_target);
    
	NxwItemWrapper si;
	si.fillItemWeared(pc_target);
	for (si.rewind(); !si.isEmpty(); si++) {
        	pi = si.getItem();
		if (pi->layer == addmitem[s]) {
			pc->showContainer(pi);
			return;
		}
	}

	pc->sysmsg(TRANSLATE("No object was found at that layer on that character"));
}

void cTargets::StaminaTarget(NXWSOCKET s)
{
    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
    if (ISVALIDPC(pc))
    {
        pc->playSFX( 0x01F2);
        staticeffect(DEREF_P_CHAR(pc), 0x37, 0x6A, 0x09, 0x06);
        pc->stm=pc->dx;
        pc->updateStats(2);
        return;
    }
    sysmessage(s,TRANSLATE("That is not a person."));
}

void cTargets::ManaTarget(NXWSOCKET s)
{
    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
    if (ISVALIDPC(pc))
    {
        pc->playSFX( 0x01F2);
        staticeffect(DEREF_P_CHAR(pc), 0x37, 0x6A, 0x09, 0x06);
        pc->mn=pc->in;
        pc->updateStats(1);
        return;
    }
    sysmessage(s,TRANSLATE("That is not a person."));
}

void cTargets::MakeShopTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
	P_CHAR pc=MAKE_CHAR_REF(i);
    if (pc!=NULL)
    {
        Commands::MakeShop(i);
        pc->teleport();
        sysmessage(s, TRANSLATE("The buy containers have been added."));
        return;
    }
    sysmessage(s, TRANSLATE("Target character not found..."));
}

void cTargets::JailTarget(NXWSOCKET s, int c)
{

    P_CHAR pc = NULL;
	if (c==INVALID)
		pc = pointers::findCharBySerPtr( buffer[s] + 7 );
    else
        pc = pointers::findCharBySerial(c);

	P_CHAR jai = MAKE_CHAR_REF( currchar[s] );

	//note Additem array used for jail time here..
	prison::jail( jai, pc, addmitem[s] );
    addmitem[s] = 0; // clear it
		
	/*    if (pc==NULL)
        return; // lb

    if (pc->cell>0)
    {
        sysmessage(s, "That player is already in jail!");
        return;
    }

    for (i = 1; i < 11; i++)
    {
        if (jails[i].occupied == 0)
        {
			P_CHAR character = MAKE_CHARREF_LR(i);
			*//*
			Location pcpos= pc->getPosition();
            pc->oldx = pcpos.x;
            pc->oldy = pcpos.y;
            pc->oldz = pcpos.z;
			*//*
			pc->setPosition( pc->getOldPosition() );
            //Char_MoveTo(tmpnum, jails[i].x, jails[i].y, jails[i].z);
			character->MoveTo( jails[i].x, jails[i].y, jails[i].z );
            pc->cell = i;
            pc->priv2 = 2; // freeze them  Ripper


            // blackwinds jail
            pc->jailsecs = addmitem[s]; // Additem array used for jail time here..
            addmitem[s] = 0; // clear it
            pc->jailtimer = uiCurrentTime +(MY_CLOCKS_PER_SEC*pc->jailsecs);
            pc->teleport();
            NXWSOCKET  prisoner = calcSocketFromChar(tmpnum);
            jails[i].occupied = 1;
            sysmessage(prisoner, TRANSLATE("You are jailed !"));
            sysmessage(prisoner, TRANSLATE("You notice you just got something new at your backpack.."));
            sysmessage(s, "Player %s has been jailed in cell %i.", pc->getCurrentNameC(), i);
            item::SpawnItemBackpack2(prisoner, 50040, 0); // spawn crystall ball of justice to prisoner.
            // end blackwinds jail

            x++;
            break;
        }
    }
    if (x == 0)
        sysmessage(s, "All jails are currently full!");
*/
}

void cTargets::AttackTarget(NXWSOCKET s)
{


    P_CHAR pc_t1=pointers::findCharBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s]));
    VALIDATEPC(pc_t1);
	P_CHAR pc_t2=pointers::findCharBySerPtr(buffer[s] +7);
	VALIDATEPC(pc_t2);

    AttackStuff(s,pc_t2); //xan : flag them all!
    npcattacktarget(pc_t1, pc_t2);
}

void cTargets::FollowTarget(NXWSOCKET s)
{

    P_CHAR pc = pointers::findCharBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s]));
    VALIDATEPC(pc);
	P_CHAR pc2 = pointers::findCharBySerPtr(buffer[s] +7);
	VALIDATEPC(pc2);

    pc->ftargserial=pc2->getSerial32();
    pc->npcWander=1;
}

void cTargets::TransferTarget(NXWSOCKET s)
{

    P_CHAR pc1 = pointers::findCharBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s]));
	VALIDATEPC(pc1);
    P_CHAR pc2 = pointers::findCharBySerPtr(buffer[s] +7);
	VALIDATEPC(pc2);

	//Araknesh Call OnTransfer Event Passing Animal,NewOwner
	
	if (pc1->amxevents[EVENT_CHR_ONTRANSFER])
	{
		g_bByPass = false;
		pc1->amxevents[EVENT_CHR_ONTRANSFER]->Call(pc1->getSerial32(),pc2->getSerial32());
		if (g_bByPass==true) return ;
	}
	/*
	pc1->runAmxEvent( EVENT_CHR_ONTRANSFER, pc1->getSerial32(),pc2->getSerial32());
	if (g_bByPass==true)
		return ;
	*/
    char t[120];
    sprintf(t,TRANSLATE("* %s will now take %s as his master *"), pc1->getCurrentNameC(), pc2->getCurrentNameC());
    pc1->talkAll(t,0);

    pc1->setOwner( pc2 );
    pc1->npcWander=1;
    pc1->ftargserial=INVALID;
    pc1->npcWander=0;
}

void cTargets::BuyShopTarget(NXWSOCKET s)
{
    P_CHAR pc =pointers::findCharBySerPtr(buffer[s]+7);
    if (ISVALIDPC(pc))
        if ((pc->getSerial32() == pc->getSerial32()))
        {
            Targ->BuyShop(s, DEREF_P_CHAR(pc));
            return;
        }
	sysmessage(s, TRANSLATE("Target shopkeeper not found..."));
}

int cTargets::BuyShop(NXWSOCKET s, CHARACTER c)
{
    P_ITEM buyRestockContainer=NULL, buyNoRestockContainer=NULL;

    P_CHAR pc = MAKE_CHARREF_LRV(c, 0);
	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(curr,0);


	NxwItemWrapper si;
	si.fillItemWeared( pc, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ )
    {
        P_ITEM pi=si.getItem();
		if(!ISVALIDPI(pi)) continue;

		if( pi->layer==LAYER_TRADE_RESTOCK )
            buyRestockContainer=pi;

        if( pi->layer==LAYER_TRADE_NORESTOCK )
            buyNoRestockContainer=pi;

		if( ISVALIDPI(buyRestockContainer) && ISVALIDPI(buyNoRestockContainer) )
			break;
    }

    if (!ISVALIDPI(buyRestockContainer) || !ISVALIDPI(buyNoRestockContainer) )
        return 0;

    impowncreate(s, pc, 0); // Send the NPC again to make sure info is current. (OSI does this we might not have to)

    sendshopinfo(s, DEREF_P_CHAR(pc), buyRestockContainer); // Send normal shop items
//  sendshopinfo(s, c, buyNoRestockContainer); // Send items sold to shop by players
    SndShopgumpopen(s,pc->getSerial32());
    statwindow(curr,curr); // Make sure the gold total has been sent.
    return 1;
}

//////////////////////////////////
// Changed hideing to make flamestrike and hide work better
//
//
void cTargets::permHideTarget(NXWSOCKET s)
{
	SERIAL serial = LongFromCharPtr(buffer[s] + 7);
	int i = calcCharFromSer(serial);
	P_CHAR pc = MAKE_CHARREF_LR(i);
	Location pcpos= pc->getPosition();
    
	if (i!=-1)
	{
		if (pc->hidden == HIDDEN_BYSKILL)
		{
			if (i == currchar[s])
				sysmessage(s, TRANSLATE("You are already hiding."));
			else
				sysmessage(s, TRANSLATE("He is already hiding."));
			return;
		}
        
		pc->priv2 |= CHRPRIV2_PERMAHIDDEN;
		staticeffect3(pcpos.x + 1, pcpos.y + 1, pcpos.z + 10, 0x37, 0x09, 0x09, 0x19, 0);
		pc->playSFX(0x0208);
		tempfx::add(pc, pc, tempfx::GM_HIDING, 1, 0, 0);

		return;
    }
}
//
//
// Aldur
//////////////////////////////////

//////////////////////////////////
// Changed unhideing to make flamestrike and unhide work better
//
//
void cTargets::unHideTarget(NXWSOCKET s)
{
	SERIAL serial = LongFromCharPtr(buffer[s] + 7);
	int i = calcCharFromSer(serial);
	P_CHAR pc = MAKE_CHARREF_LR(i);

	Location pcpos= pc->getPosition();
    
	if (i!=-1)
	{
		if (pc->hidden == UNHIDDEN)
		{
			if (i == currchar[s])
				sysmessage(s, TRANSLATE("You are not hiding."));
			else
				sysmessage(s, TRANSLATE("He is not hiding."));
			return;
		}
        
		pc->priv2 &= ~CHRPRIV2_PERMAHIDDEN;
		
		staticeffect3(pcpos.x + 1, pcpos.y + 1, pcpos.z + 10, 0x37, 0x09, 0x09, 0x19, 0);
		pc->playSFX(0x0208);
		tempfx::add(pc, pc, tempfx::GM_UNHIDING, 1, 0, 0);

		return;
	}
}
//
//
// Aldur
//////////////////////////////////

void cTargets::SetSpeechTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    if (i!=-1)
    {
        P_CHAR pc = MAKE_CHARREF_LR(i);
        if (pc->npc==0)
        {
            sysmessage(s,"You can only change speech for npcs.");
            return;
        }
        pc->speech=addx[s];
    }
}

static void SetSpAttackTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
    {
        pc->spattack=tempint[s];
    }
}

void cTargets::SetSpaDelayTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
    {
        pc->spadelay=tempint[s];
    }
}

void cTargets::SetPoisonTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
    {
        pc->poison=tempint[s];
    }
}

void cTargets::SetPoisonedTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
    {
        pc->poisoned=(PoisonType)tempint[s];
        pc->poisonwearofftime=uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParms->poisontimer); // lb, poison wear off timer setting
        impowncreate(calcSocketFromChar(i),pc,1); //Lb, sends the green bar !
    }
}

void cTargets::FullStatsTarget(NXWSOCKET s)
{
    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
    if (ISVALIDPC(pc))
    {
        pc->playSFX( 0x01F2);
        staticeffect(DEREF_P_CHAR(pc), 0x37, 0x6A, 0x09, 0x06);
        pc->mn=pc->in;
        pc->hp=pc->getStrength();
        pc->stm=pc->dx;
        pc->updateStats(0);
        pc->updateStats(1);
        pc->updateStats(2);
        return;
    }
    sysmessage(s,TRANSLATE("That is not a person."));
}

void cTargets::SetAdvObjTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
    {
        pc->advobj=tempint[s];
    }
}

////////////////
// name:        CanTrainTarget
// history:     by Antrhacks 1-3-99
// Purpose:     Used for training by NPC's
//
void cTargets::CanTrainTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
    {
        if (pc->npc==0)
        {
            sysmessage(s, "Only NPC's may train.");
            return;
        }
        pc->cantrain = !pc->cantrain;   //turn on if off, off if on
    }
}

void cTargets::SetSplitTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
    {
        pc->split=tempint[s];
    }
}

void cTargets::SetSplitChanceTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    if (i!=-1)
    {
        pc->splitchnc=tempint[s];
    }
}

static void AxeTarget(NXWCLIENT pC, PKGx6C *pp)
{
	if (pC == NULL) return;
    NXWSOCKET  s = pC->toInt();

    short id=pp->model;
    if (itemById::IsTree(id))
		Skills::TreeTarget(s);
    else if (itemById::IsCorpse(id))
        CorpseTarget(pC);
    else if (itemById::IsLog(id)) //Luxor bug fix
		Skills::BowCraft(s);
}

void cTargets::SetDirTarget(NXWSOCKET s)
{
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	if (serial == -1)
		return;

	if (isItemSerial(serial))
	{
		P_ITEM pi = pointers::findItemBySerial(serial);
		if (pi)
		{
			pi->dir=addx[s];
			pi->Refresh();
			return;
		}
	}
	else
	{
		P_CHAR pc = pointers::findCharBySerial(serial);
		if (pc)
		{
			pc->dir=addx[s];
			pc->teleport();
			return;
		}
	}
}

void cTargets::NewXTarget(NXWSOCKET s) // Notice a high similarity to th function above? Wonder why. - Gandalf
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcItemFromSer(serial);
    P_ITEM pi=MAKE_ITEMREF_LR(i);
    if (i!=-1)
    {
        //item::MoveTo(i,addx[s],pi->y,pi->z);
		pi->MoveTo( addx[s], pi->getPosition("y"), pi->getPosition("z"));
        pi->Refresh();
    }

    i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
	Location pcpos= pc->getPosition();

    if (i!=-1)
    {
        //Char_MoveTo(i, addx[s], pcpos.y, pcpos.z);
		pc->MoveTo( addx[s], pcpos.y, pcpos.z );
        pc->teleport();
    }
}

void cTargets::NewYTarget(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcItemFromSer(serial);
    P_ITEM pi=MAKE_ITEMREF_LR(i);
    if (i!=-1)
    {
        //item::MoveTo(i,pi->x,addx[s],pi->z);
		pi->MoveTo( pi->getPosition("x"), addx[s], pi->getPosition("z"));
        pi->Refresh();
    }

    i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
	Location pcpos= pc->getPosition();

    if (i!=-1)
    {
        //Char_MoveTo(i, pcpos.x, addx[s], pi->z);
		pc->MoveTo( pcpos.x, addx[s], pcpos.z );
        pc->teleport();
    }
}

void cTargets::IncXTarget(NXWSOCKET s)
{
	Location position;
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	int i=calcItemFromSer(serial);
	P_ITEM pi=MAKE_ITEM_REF(i);
	if( ISVALIDPI( pi ) )
	{
		position = pi->getPosition();
		position.x += addx[s];
		pi->MoveTo( position );
		pi->Refresh();
	}
	else
	{
		i=calcCharFromSer(serial);
		P_CHAR pc = MAKE_CHAR_REF(i);
		if ( ISVALIDPC( pc ) )
		{
			position = pc->getPosition();
			position.x += addx[s];
			pc->MoveTo( position );
			pc->teleport();
		}
	}
}

void cTargets::IncYTarget(NXWSOCKET s)
{
	Location position;
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	int i=calcItemFromSer(serial);
	P_ITEM pi=MAKE_ITEM_REF(i);
	if( ISVALIDPI( pi ) )
	{
		position = pi->getPosition();
		position.y += addx[s];
		pi->MoveTo( position );
		pi->Refresh();
	}
	else
	{
		i=calcCharFromSer(serial);
		P_CHAR pc = MAKE_CHAR_REF(i);
		if ( ISVALIDPC( pc ) )
		{
			position = pc->getPosition();
			position.y += addx[s];
			pc->MoveTo( position );
			pc->teleport();
		}
	}
}

/*
void cTargets::IncZTarget(NXWSOCKET s)
{
	Location position;
	SERIAL serial=LongFromCharPtr(buffer[s]+7);
	int i=calcItemFromSer(serial);
	P_ITEM pi=MAKE_ITEM_REF(i);
	if( ISVALIDPI( pi ) )
	{
		position = pi->getPosition();
		position.z += addx[s];
		pi->MoveTo( position );
		pi->Refresh();
	}
	else
	{
		i=calcCharFromSer(serial);
		P_CHAR pc = MAKE_CHAR_REF(i);
		if ( ISVALIDPC( pc ) )
		{
			position = pi->getPosition();
			position.z += addx[s];
			pc->MoveTo( position );
			pc->teleport();
		}
	}
}
*/

void cTargets::Priv3XTarget( NXWSOCKET socket )
//Set Priv3 to target
{
	SERIAL serial=LongFromCharPtr(buffer[socket]+7);
	int i=calcCharFromSer(serial);
	P_CHAR pc = MAKE_CHARREF_LR(i);
	int grantcmd=1;
	int revokecmd;
	//
	//	Sparhawk:	Very very dirty trick to get setpriv3 cmd working again
	//
	P_COMMAND cmd = (P_COMMAND) addx[ socket ];

	if( addy[ socket ] )
	{
		pc->priv3[ cmd->cmd_priv_m ] |= ( grantcmd << cmd->cmd_priv_b );
		sysmessage( socket, "%s has been granted access to the %s command.", pc->getCurrentNameC(), cmd->cmd_name );
	}
	else
	{
		revokecmd = 0xFFFFFFFF - ( grantcmd << cmd->cmd_priv_b );
		pc->priv3[ cmd->cmd_priv_m ] &= revokecmd;
		sysmessage( socket, "%s has been revoked access to the %s command.", pc->getCurrentNameC(), cmd->cmd_name );
	}
/*
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(i);
    int grantcmd=1; //it was 0-0xFFFFFFFF, Ummon
    int revokecmd;
    if (i!=-1)
    {
        ConOut("setpriv3target: %s\n", pc->getCurrentNameC());
        struct cmdtable_s *pct=&command_table[addx[s]];
        if(addy[s])
        {
            pc->priv3[pct->cmd_priv_m] |= (grantcmd<<pct->cmd_priv_b); //Ummon
//          pc->priv3[pct->cmd_priv_m] |= (0-0xFFFFFFFF<<pct->cmd_priv_b);
            sysmessage(s, "%s has been granted access to the %s command.",pc->getCurrentNameC(), pct->cmd_name);

        }
        else
        {
            revokecmd=0xFFFFFFFF-(grantcmd<<pct->cmd_priv_b); //Ummon
            pc->priv3[pct->cmd_priv_m] &= revokecmd;
//          pc->priv3[pct->cmd_priv_m] -= (0-0xFFFFFFFF<<pct->cmd_priv_b);
            sysmessage(s, "%s has been revoked access to the %s command.",pc->getCurrentNameC(), pct->cmd_name);
        }
    }
*/
}

/*
void cTargets::ShowPriv3Target(NXWSOCKET s) // crackerjack, jul 25/99
{
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int p=calcCharFromSer(serial);
     P_CHAR pc = MAKE_CHARREF_LR(p);
    if (p!=-1)
    {
        char priv_info[10248];
        int i;
        sprintf(priv_info, "%s can execute the following commands:\n", pc->getCurrentNameC());
        i=0; int loopexit=0;
        while(command_table[i].cmd_name && (++loopexit < MAXLOOPS) )
        {
            if(command_table[i].cmd_priv_m==255||
                (pc->priv3[command_table[i].cmd_priv_m]&
                (0-0xFFFFFFFF<<command_table[i].cmd_priv_b)))
            {
                sprintf((char*)temp, " %s", command_table[i].cmd_name);
                strcpy(&priv_info[strlen(priv_info)], (char*)temp);
            }
            i++;
        }
        SndUpdscroll(s, strlen(priv_info), priv_info);
    }
    else
        sysmessage(s, "You cannot retrieve privilige information on that.");
}
*/

void cTargets::HouseOwnerTarget(NXWSOCKET s) // crackerjack 8/10/99 - change house owner
{
	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);
	
	P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
	VALIDATEPC(pc);

	P_ITEM pSign=pointers::findItemBySerial(calcserial(addid1[s],addid2[s],addid3[s],addid4[s]));
	VALIDATEPI(pSign);

	P_ITEM pHouse=pointers::findItemBySerial(calcserial(pSign->more1, pSign->more2, pSign->more3, pSign->more4));
	VALIDATEPI(pHouse);


	if(pc->getSerial32() == curr->getSerial32())
	{
		sysmessage(s, "you already own this house!");
		return;
	}

	pSign->setOwnerSerial32(pc->getSerial32());

	pHouse->setOwnerSerial32(pc->getSerial32());

	killkeys( pHouse->getSerial32() );

	
	NXWCLIENT osc=pc->getClient();
	NXWSOCKET os= (osc!=NULL)? osc->toInt() : INVALID;

	P_ITEM pi3=item::CreateFromScript( "$item_gold_key" ); //gold key for everything else
	VALIDATEPI(pi3);
	pi3->setCurrentName( "a house key" );
	if(os!=INVALID)
	{
		pi3->setCont( pc->getBackpack() );
	}
	else
	{
		pi3->MoveTo( pc->getPosition() );
	}
	pi3->Refresh();
	pi3->more1= pHouse->getSerial().ser1;
	pi3->more2= pHouse->getSerial().ser2;
	pi3->more3= pHouse->getSerial().ser3;
	pi3->more4= pHouse->getSerial().ser4;
	pi3->type=7;

	sysmessage(s, "You have transferred your house to %s.", pc->getCurrentNameC());
	char temp[520];
	sprintf(temp, "%s has transferred a house to %s.", curr->getCurrentNameC(), pc->getCurrentNameC());

	NxwSocketWrapper sw;
	sw.fillOnline( pc, false );
	for( sw.rewind(); !sw.isEmpty(); sw++ ) 
	{
		NXWSOCKET k=sw.getSocket();
		if(k!=INVALID)
			sysmessage(k, temp);
	}
}


void cTargets::HouseEjectTarget(NXWSOCKET s) // crackerjack 8/11/99 - kick someone out of house
{
    int c, h;
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if(serial==-1) return;
    c=calcCharFromSer(serial);
    P_CHAR pc = MAKE_CHARREF_LR(c);
	Location pcpos= pc->getPosition();
    serial=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);

    if(serial==-1) return;

    h=calcItemFromSer(serial);

	P_ITEM pi_h=MAKE_ITEM_REF(h);

    if((c!=-1)&&(h!=-1)) {
        NXWSOCKET sx, sy, ex, ey;
        Map->MultiArea(pi_h, &sx,&sy,&ex,&ey);
        if((pcpos.x>=(UI32)sx) && (pcpos.y>=(UI32)sy) && (pcpos.x<=(UI32)ex) && (pcpos.y<=(UI32)ey))
        {
            //Char_MoveTo(c, ex, ey, pcpos.z);
			pc->MoveTo( ex, ey, pcpos.z );
            pc->teleport();
            sysmessage(s, TRANSLATE("Player ejected."));
        } else {
            sysmessage(s, TRANSLATE("That is not inside the house."));
        }
    }
}

void cTargets::HouseBanTarget(NXWSOCKET s) // crackerjack 8/12/99 - ban someobdy from the house
{
	Targ->HouseEjectTarget(s);	// first, eject the player

	P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
	VALIDATEPC(pc);

	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);

	P_ITEM pi=pointers::findItemBySerial(calcserial(addid1[s],addid2[s],addid3[s],addid4[s]));
	if(ISVALIDPI(pi))
	{
		if(pc->getSerial32() == curr->getSerial32()) 
			return;
		int r=add_hlist(DEREF_P_CHAR(pc), DEREF_P_ITEM(pi), H_BAN);
		if(r==1)
		{
			sysmessage(s, "%s has been banned from this house.", pc->getCurrentNameC());
		} 
		else if(r==2)
		{
			sysmessage(s, "That player is already on a house register.");
		} 
		else
			sysmessage(s, "That player is not on the property.");
	}
}

void cTargets::HouseFriendTarget(NXWSOCKET s) // crackerjack 8/12/99 - add somebody to friends list
{
	P_CHAR Friend = pointers::findCharBySerPtr(buffer[s]+7);

	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);

	P_ITEM pi=pointers::findItemBySerial(calcserial(addid1[s],addid2[s],addid3[s],addid4[s]));

	if(ISVALIDPC(Friend) && ISVALIDPI(pi))
	{
		if(Friend->getSerial32() == curr->getSerial32())
		{
			sysmessage(s,"You cant do that!");
			return;
		}
		int r=add_hlist(DEREF_P_CHAR(Friend), DEREF_P_ITEM(pi), H_FRIEND);
		if(r==1)
		{
			sysmessage(s, "%s has been made a friend of the house.", Friend->getCurrentNameC());
		} 
		else if(r==2)
		{
			sysmessage(s, "That player is already on a house register.");
		} 
		else
			sysmessage(s, "That player is not on the property.");
	}
}

void cTargets::HouseUnlistTarget(NXWSOCKET s) // crackerjack 8/12/99 - remove somebody from a list
{
	P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
    P_ITEM pi= pointers::findItemBySerial(calcserial(addid1[s],addid2[s],addid3[s],addid4[s]));
    if(ISVALIDPC(pc) && ISVALIDPI(pi))
    {
        int r=del_hlist(DEREF_P_CHAR(pc), DEREF_P_ITEM(pi));
        if(r>0)
        {
            sysmessage(s, TRANSLATE("%s has been removed from the house registry."), pc->getCurrentNameC());
        }
        else
            sysmessage(s, TRANSLATE("That player is not on the house registry."));
    }
}
void cTargets::HouseLockdown( NXWSOCKET  s ) // Abaddon
// PRE:     S is the socket of a valid owner/coowner and is in a valid house
// POST:    either locks down the item, or puts a message to the owner saying he's a moron
// CODER:   Abaddon
// DATE:    17th December, 1999
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    P_ITEM pi=pointers::findItemBySerPtr(buffer[s]+7);
    if(ISVALIDPI(pi))
    {
        
        /*houseSer = calcserial( addid1[s], addid2[s], addid3[s], addid4[s] );  // let's find our house
        house = calcItemFromSer(houseSer);*/

        // not needed anymore, cause called from house_sped that already checks that ...

        // time to lock it down!

        if( pi->isFieldSpellItem() )
        {
            sysmessage(s,TRANSLATE("you cannot lock this down!"));
            return;
        }
        if (pi->type==12 || pi->type==13 || pi->type==203)
        {
            sysmessage(s, TRANSLATE("You cant lockdown doors or signs!"));
            return;
        }
        if ( pi->IsAnvil() )
        {
            sysmessage(s, TRANSLATE("You cant lockdown anvils!"));
            return;
        }
        if ( pi->IsForge() )
        {
            sysmessage(s, TRANSLATE("You cant lockdown forges!"));
            return;
        }

        P_ITEM multi = findmulti( pi->getPosition() );
        if( ISVALIDPI(multi))
        {
            if(pi->magic==4)
            {
                sysmessage(s,TRANSLATE("That item is already locked down, release it first!"));
                return;
            }
            pi->magic = 4;  // LOCKED DOWN!
            DRAGGED[s]=0;
            pi->setOwnerSerial32Only(pc->getSerial32());
            pi->Refresh();
            return;
        }
        else
        {
            // not in a multi!
            sysmessage( s, TRANSLATE("That item is not in your house!" ));
            return;
        }
    }
    else
    {
        sysmessage( s, TRANSLATE("Invalid item!" ));
        return;
    }
}

void cTargets::HouseSecureDown( NXWSOCKET  s ) // Ripper
// For locked down and secure chests
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    P_ITEM pi=pointers::findItemBySerPtr(buffer[s]+7);
    if(ISVALIDPI(pi))
    {
        // time to lock it down!

        if( pi->isFieldSpellItem() )
        {
            sysmessage(s,TRANSLATE("you cannot lock this down!"));
            return;
        }
        if (pi->type==12 || pi->type==13 || pi->type==203)
        {
            sysmessage(s, TRANSLATE("You cant lockdown doors or signs!"));
            return;
        }
        if(pi->magic==4)
        {
            sysmessage(s,TRANSLATE("That item is already locked down, release it first!"));
            return;
        }

        P_ITEM multi = findmulti( pi->getPosition() );
        if( ISVALIDPI(multi) && pi->type==1)
        {
            pi->magic = 4;  // LOCKED DOWN!
            pi->secureIt = 1;
            DRAGGED[s]=0;
            pi->setOwnerSerial32Only(pc->getSerial32());
            pi->Refresh();
            return;
        }
        if(pi->type!=1)
        {
            sysmessage(s,TRANSLATE("You can only secure chests!"));
            return;
        }
        else
        {
            // not in a multi!
            sysmessage( s, TRANSLATE("That item is not in your house!" ));
            return;
        }
    }
    else
    {
        sysmessage( s, TRANSLATE("Invalid item!" ));
        return;
    }
}

void cTargets::HouseRelease( NXWSOCKET  s ) // Abaddon & Ripper
// PRE:     S is the socket of a valid owner/coowner and is in a valid house, the item is locked down
// POST:    either releases the item from lockdown, or puts a message to the owner saying he's a moron
// CODER:   Abaddon
// DATE:    17th December, 1999
// update: 5-8-00
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    P_ITEM pi=pointers::findItemBySerPtr(buffer[s]+7);
    if(ISVALIDPI(pi))
    {
        if(pi->getOwnerSerial32() != pc->getSerial32())
        {
            sysmessage(s,TRANSLATE("This is not your item!"));
            return;
        }
        if( pi->isFieldSpellItem() )
        {
            sysmessage(s,TRANSLATE("you cannot release this!"));
            return;
        }
        if (pi->type==12 || pi->type==13 || pi->type==203)
        {
            sysmessage(s, TRANSLATE("You cant release doors or signs!"));
            return;
        }
        /*houseSer = calcserial( addid1[s], addid2[s], addid3[s], addid4[s] );  // let's find our house
        house = calcItemFromSer(houseSer);*/
        // time to lock it down!
        P_ITEM multi = findmulti( pi->getPosition() );
        if( ISVALIDPI(multi) && pi->magic==4 || pi->type==1)
        {
            pi->magic = 1;  // Default as stored by the client, perhaps we should keep a backup?
            pi->secureIt = 0;
            pi->Refresh();
            return;
        }
        else if( !ISVALIDPI(multi) )
        {
            // not in a multi!
            sysmessage( s, TRANSLATE("That item is not in your house!" ));
            return;
        }
    }
    else
    {
        sysmessage( s, TRANSLATE("Invalid item!" ));
        return;
    }
}

void cTargets::SetMurderCount( NXWSOCKET s )
{
    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
	VALIDATEPC(pc);

	pc->kills = addmitem[s];
    setcharflag(pc);

}

/*void cTargets::GlowTarget(NXWSOCKET s) // LB 4/9/99, makes items glow
{
    int c,i;

    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if(serial==-1) return;
    i=calcItemFromSer(serial);
    P_ITEM pi1=MAKE_ITEMREF_LR(i);
    if (i==-1)
    {
        sysmessage(s,TRANSLATE("Item not found."));
        return;
    }

    int cc=currchar[s];
	Location charpos= chars[cc].getPosition();

    if (!pi1->isInWorld())
    {
		P_CHAR pc;

		P_ITEM pj= (P_ITEM)pi1->getContainer(); // in bp ?
		P_CHAR pl= (P_CHAR)pi1->getContainer(); // equipped ?

        if ( !ISVALIDPC(pl) ) 
			pc= pj->getPackOwner();
		else 
			pc= pl;

        if ( !(ISVALIDPC(pc) && pc->getSerial32()==chars[cc].getSerial32()))  // creation only allowed in the creators pack/char otherwise things could go wrong
        {
            sysmessage(s,TRANSLATE("you can't create glowing items in other perons packs or hands"));
            return;
        }
    }

    if (pi1->glow!=0)
    {
        sysmessage(s,TRANSLATE("that object already glows!\n"));
        return;
    }

    c=0x99;
    pi1->glow_c1=pi1->color1; // backup old colors
    pi1->glow_c2=pi1->color2;

    pi1->color1=c<<8; // set new color to yellow
    pi1->color2=c%256;

    c=item::SpawnItem(s,1,"glower",0,0x16,0x47,0,0,0,1); // new client 1.26.2 glower object
    P_ITEM pi2=MAKE_ITEMREF_LR(c);

    if(c==-1) return;
    pi2->dir=29; // set light radius maximal
    pi2->visible=0;

    pi2->magic=3;

    mapRegions->RemoveItem(c); // remove if add in spawnitem
    pi2->layer=pi1->layer;
    if (pi2->layer==0) // if not equipped -> coords of the light-object = coords of the
    {
		pi2->setPosition( pi1->getPosition() );
    } else // if equipped -> place lightsource at player ( height= approx hand level )
    {
        pi2->setPosition("x", charpos.x);
        pi2->setPosition("y", charpos.y);
        pi2->setPosition("z", charpos.z + 4);
    }

    //mapRegions->AddItem(c);
    pi2->priv=0; // doesnt decay

    pi1->glow= pi2->getSerial32(); // set glow-identifier


    pi1->Refresh();
    pi2->Refresh();
    //setptr(&glowsp[chars[cc].serial%HASHMAX],i);

    impowncreate(s,cc,0); // if equipped send new color too
}

void cTargets::UnglowTaget(NXWSOCKET s) // LB 4/9/99, removes the glow-effect from items
{
    int c,i;

    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if(serial==-1) return;
    i=calcItemFromSer(serial);
    P_ITEM pi=MAKE_ITEMREF_LR(i);
    if (i==-1)
    {
        sysmessage(s,TRANSLATE("no item found"));
        return;
    }

    if (!pi->isInWorld())
    {
		P_CHAR pc;
        P_ITEM pj= (P_ITEM)pi->getContainer(); // in bp ?
        P_CHAR pl= (P_CHAR)pi->getContainer(); // equipped ?

        if ( !ISVALIDPC(pl) ) 
			pc= pj->getPackOwner(); 
		else 
			pc= pl;

        if (pc->getSerial32()!=chars[currchar[s]].getSerial32()) // creation only allowed in the creators pack/char otherwise things could go wrong
        {
            sysmessage(s,TRANSLATE("you can't unglow items in other perons packs or hands"));
            return;
        }
    }

    c=pi->glow;
    if(c==-1) return;
    int j=calcItemFromSer(c);

    if (pi->glow==0 || j==-1 )
    {
        sysmessage(s,TRANSLATE("that object doesnt glow!\n"));
        return;
    }

    pi->color1=pi->glow_c1;
    pi->color2=pi->glow_c2; // restore old color

    item_mem::DeleItem(j); // delete glowing object

    pi->glow=0; // remove glow-identifier
    pi->Refresh();

    impowncreate(s,currchar[s],0); // if equipped send new old color too

    chars[currchar[s]].removeHalo(pi);
//  removefromptr(&glowsp[chars[currchar[s]].serial%HASHMAX],i);

    //sysmessage(s,"unglow under cosntruction");
}*/

void cTargets::MenuPrivTarg(NXWSOCKET s)//LB's menu privs
{
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
	VALIDATEPC(pc);

	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);

	int i=addid1[s];
	sprintf(temp,"Setting Menupriv number %i",i);
    sysmessage(s,temp);
	sprintf(temp,"Menupriv %i set by %s", i, curr->getCurrentNameC());
	pc->sysmsg(temp);
	pc->menupriv=i;
    
}

void cTargets::ShowSkillTarget(NXWSOCKET s) // LB's showskills
{
    int a,j,k,b=0,c,z,zz,ges=0;
    char skill_info[(ALLSKILLS+1)*40];
    char sk[25];
    char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);

    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
	if(ISVALIDPC(pc)) {

		z=addx[s];
        if (z<0 || z>3) z=0;
        if (z==2 || z==3)
            sprintf(skill_info, "%s's skills:", pc->getCurrentNameC());
        else
            sprintf(skill_info, "%s's baseskills:", pc->getCurrentNameC());

        //b=strlen(pc->name)+11;
		b= strlen(pc->getCurrentNameC()) + 11;
        if (b>23) b=23;

        for (c=b;c<=26;c++)
            strcpy(&skill_info[strlen(skill_info)], " ");

        numtostr(ges,sk);
        sprintf(temp,"sum: %s",sk);
        strcpy(&skill_info[strlen(skill_info)],temp);

        for (a=0;a<ALLSKILLS;a++)
        {
            if (z==0 || z==1) k=pc->baseskill[a]; else k=pc->skill[a];
            if (z==0 || z==2) zz=9; else zz=-1;

            if (k>zz) // show only if skills >=1
            {
                if (z==2 || z==3) j=pc->skill[a]/10; else j=pc->baseskill[a]/10;    // get skill value
                numtostr(j,sk);     // skill-value string in sk
                ges+=j;
                sprintf(temp, "%s %s", skillname[a],sk);
                strcpy(&skill_info[strlen(skill_info)],temp);

                b=strlen(skillname[a])+strlen(sk)+1; // it doesnt like \n's, so insert spaces till end of line
                if (b>23) b=23;
                for (c=b;c<=26;c++)
                     strcpy(&skill_info[strlen(skill_info)], " ");
            }
        }
        numtostr(ges,sk);
        sprintf(temp,"sum: %s  ",sk);
        strcpy(&skill_info[strlen(skill_info)],temp);

        SndUpdscroll(s, strlen(skill_info), skill_info);
    }
    else
        sysmessage(s,TRANSLATE("no valid target"));
}

void cTargets::FetchTarget(NXWSOCKET  s) // Ripper
{
    sysmessage(s,TRANSLATE("Fetch is not available at this time."));
}

void cTargets::GuardTarget( NXWSOCKET  s )
{
    P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	
	P_CHAR pPet = pointers::findCharBySerial(addx[s]);
    if (!ISVALIDPC(pPet)) LogError("Lost pet serial");

    P_CHAR pToGuard = pointers::findCharBySerPtr(buffer[s]+7);
    if( !ISVALIDPC(pToGuard) || pToGuard->getSerial32() != pPet->getOwnerSerial32() )
    {
        sysmessage( s, TRANSLATE("Currently can't guard anyone but yourself!" ));
        return;
    }
    pPet->npcaitype = NPCAI_PETGUARD;
    pPet->ftargserial=pc->getSerial32();
    pPet->npcWander=1;
    sysmessage(s, TRANSLATE("Your pet is now guarding you."));
    pc->guarded = true;
}

void cTargets::ResurrectionTarget( NXWSOCKET  s )
{
	P_CHAR curr=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(curr);

    P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);
	if(ISVALIDPC(pc)) {

        if (pc->dead)
        {
            pc->resurrect();
            return;
        }
    }
}

//AntiChrist - shows the COMMENT line in the account section of player current acct.
void cTargets::ShowAccountCommentTarget(NXWSOCKET s)
{
//TO BE REMOVED
}

void cTargets::SetHome(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if(serial==-1) return;
    int i=calcCharFromSer(serial);
     P_CHAR pc = MAKE_CHARREF_LR(i);
    if(i!=-1)
    {
        pc->homeloc.x=addx[s];
        pc->homeloc.y=addy[s];
        pc->homeloc.z=addz[s];
    }
}

void cTargets::SetWork(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if(serial==-1) return;
    int i=calcCharFromSer(serial);
     P_CHAR pc = MAKE_CHARREF_LR(i);
    if(i!=-1)
    {
        pc->workloc.x=addx[s];
        pc->workloc.y=addy[s];
        pc->workloc.z=addz[s];
    }
}

void cTargets::SetFood(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    if(serial==-1) return;
    int i=calcCharFromSer(serial);
     P_CHAR pc = MAKE_CHARREF_LR(i);
    if(i!=-1)
    {
        pc->foodloc.x=addx[s];
        pc->foodloc.y=addy[s];
        pc->foodloc.z=addz[s];
    }
}

static void ItemTarget(NXWCLIENT ps, PKGx6C *pt)
{
	if (ps == NULL) return;
    NXWSOCKET  s=ps->toInt();
    P_ITEM pi=pointers::findItemBySerial(pt->Tserial);
    if (pi==NULL) return;
    switch(pt->Tnum)
    {
    case 10://MoreTarget
        pi->more1=addid1[s];
        pi->more2=addid2[s];
        pi->more3=addid3[s];
        pi->more4=addid4[s];
        pi->Refresh();
        break;
    case  28://MovableTarget
    case 111://yes, it's duplicate
        pi->magic=addx[s];
        pi->Refresh();
        break;
    case 31://ColorsTarget
        if (pi->id()==0x0FAB ||                     //dye vat
            pi->id()==0x0EFF || pi->id()==0x0E27 )  //hair dye
            SndDyevat(s,pi->getSerial32(), pi->id());
        else
            sysmessage(s, TRANSLATE("You can only use this item on a dye vat."));
        break;
    case 63://MoreXTarget
        pi->morex=addx[s];
        break;
    case 64://MoreYTarget
        pi->morey=addx[s];
        break;
    case 65://MoreZTarget
        pi->morez=addx[s];
        break;
    case 66://MoreXYZTarget
        pi->morex=addx[s];
        pi->morey=addy[s];
        pi->morez=addz[s];
        break;
    case 89://ObjPrivTarget
    	switch( addid1[s] )
	{
        	case 0	:
			//pi->priv=pi->priv&0xFE; // lb ...
			pi->setDecay( false );
			pi->setNewbie();
			pi->setDispellable();
			break;
        	case 1	:
			pi->setDecay();
			break;
        	case 3	:
			pi->priv = addid2[s];
			break;
	}
        break;
    case 122://SetValueTarget
        pi->value=addx[s];
        break;
    case 123://SetRestockTarget
        pi->restock=addx[s];
        break;
    case 129://SetAmount2Target
        if (addx[s] > 64000) //Ripper..to fix a client bug for over 64k.
        {
            sysmessage(s, TRANSLATE("No amounts over 64k in a pile!"));
            return;
        }
        pi->amount2=addx[s];
        pi->Refresh();
        break;
    case 133://SetWipeTarget
        pi->wipe=addid1[s];
        pi->Refresh();
        break;
    }
}

void cTargets::LoadCannon(NXWSOCKET s)
{
    SERIAL serial=LongFromCharPtr(buffer[s]+7);
    int i=calcItemFromSer(serial);
    P_ITEM pi=MAKE_ITEMREF_LR(i);
    if (i!=-1)
    {
        //if((items[i].id1==0x0E && items[i].id2==0x91) && items[i].morez==0)
        if (((pi->more1==addid1[s])&&(pi->more2==addid2[s])&&
            (pi->more3==addid3[s])&&(pi->more4==addid4[s]))||
            (addid1[s]==(unsigned char)'\xFF'))
        {
            if ((pi->morez==0)&&(item_inRange(MAKE_CHAR_REF(currchar[s]),pi,2)))
            {
                if(pi->morez==0)
                pi->type=15;
                pi->morex=8;
                pi->morey=10;
                pi->morez=1;
                sysmessage(s, TRANSLATE("You load the cannon."));
            }
            else
            {
                if (pi->more1=='\x00') sysmessage(s, TRANSLATE("That doesn't work in cannon."));
                else sysmessage(s, TRANSLATE("That object doesn't fit into cannon."));
            }
        }
    }
}

void cTargets::DupeTarget(NXWSOCKET s)
{
    if (addid1[s]>=1)
    {
        P_ITEM pi=pointers::findItemBySerPtr(buffer[s]+7);
        if (ISVALIDPI(pi))
        {
            for (int j=0;j<addid1[s];j++)
            {
                Commands::DupeItem(s, DEREF_P_ITEM(pi), pi->amount);
                sysmessage(s,"DupeItem done.");//AntiChrist
            }
        }
    }
}

void cTargets::MoveToBagTarget(NXWSOCKET s)
{
    P_ITEM pi=pointers::findItemBySerPtr(buffer[s]+7);
	VALIDATEPI(pi);

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

    P_ITEM pack= pc->getBackpack();
	VALIDATEPI(pack);

    //setserial(DEREF_P_ITEM(pi),DEREF_P_ITEM(pack),1);
	pi->setContSerial(pack->getSerial32());
	pi->setPosition( 50+rand()%80, 50+rand()%80, 9);
    pi->layer=0x00;
    pi->setDecayTime(0);

    SndRemoveitem( pi->getSerial32() );
    pi->Refresh();
}

void cTargets::MultiTarget(NXWCLIENT ps) // If player clicks on something with the targetting cursor
{
	if (ps == NULL)
		return;
	NXWSOCKET  s=ps->toInt();
	if (s < 0) return;
	P_CHAR curr=ps->currChar();
	VALIDATEPC(curr);


	targetok[s]=0;

	PKGx6C tbuf, *pt=&tbuf;
	pt->type=buffer[s][1];
	pt->Tnum=buffer[s][5];
	pt->Tserial=LongFromCharPtr(buffer[s]+7);
	pt->TxLoc=ShortFromCharPtr(buffer[s]+11);
	pt->TyLoc=ShortFromCharPtr(buffer[s]+13);
	pt->TzLoc=buffer[s][16];
	pt->model=ShortFromCharPtr(buffer[s]+17);

	if (pt->TxLoc==-1 && pt->TyLoc==-1) // do nothing if user cancelled
		if (pt->Tserial==0 && pt->model==0) // this seems to be the complete 'cancel'-criteria (Duke)
			return;

//  cClient cli(s), *ps = &cli;

    if ((buffer[s][2]==0)&&(buffer[s][3]==1)&&(buffer[s][4]==0))
    {
        bool Iready=false; //valid item
		bool Cready=false; //valid char
		bool Lready=false; //valid location

        P_ITEM pi = NULL;
        P_CHAR pc = NULL;
		if(buffer[s][1]==0) { //selected object
			if(buffer[s][7]>=0x40) // an item's serial ?
			{
	            pi=pointers::findItemBySerial(pt->Tserial);
				if (ISVALIDPI(pi))
	                Iready=true;
			}
			else
			{
	            pc=pointers::findCharBySerial(pt->Tserial);
				if (ISVALIDPC(pc))
	                Cready=true;
		    }
		}
		else if( buffer[s][1]==1 ) { //selected location
			Lready=true;
		}

        switch(pt->Tnum)
        {
        case 0: if (Lready) AddTarget(s,pt); break;
        case 1: /* { cRenameTarget     T(ps);      T.process();} */ break;
        case 2: if (Lready) TeleTarget(s,pt); break; // LB, bugfix, we need it for the /tele command
        case 3: { cRemoveTarget     T(ps);      T.process();} break;
        case 4: DyeTarget(s); break;
        case 5: { cNewzTarget       T(ps);      T.process();} break;
        case 6: if (Iready) pi->type=addid1[s]; break; //Typetarget
        case 7: Targ->IDtarget(s); break;
        case 8: XgoTarget(s); break;
        case 9: if (Cready) PrivTarget(s,pc); break;
        case 10: ItemTarget(ps,pt); break;//MoreTarget
        case 11: if (Iready) KeyTarget(s,pi); break;
        case 12: Targ->IstatsTarget(s); break;
        case 13: if (Cready) CstatsTarget(ps,pc); break;
        case 14: if (Cready) GMTarget(ps,pc); break;
        case 15: if (Cready) CnsTarget(ps,pc); break;
        case 16: if (Cready) KillTarget(pc, 0x0b); break;
        case 17: if (Cready) KillTarget(pc, 0x10); break;
        case 18: if (Cready) KillTarget(pc, 0x15); break;
        case 19: if (Cready) pc->fonttype=addid1[s]; break;
        case 20: if (Cready) Targ->GhostTarget(s); break;
        case 21: if (Cready) Targ->ResurrectionTarget(s); break; // needed for /resurrect command
        case 22: { cBoltTarget      T(ps);  T.process();} break;
        case 23: { cSetAmountTarget T(ps);  T.process();} break;
        case 24:
            {
                SERIAL serial=LongFromCharPtr(buffer[s]+7);
                int i=calcItemFromSer(serial);
                if(i>=0)
                {
                    P_ITEM pi = MAKE_ITEMREF_LR(i);
		    VALIDATEPI(pi);
                    triggerItem(s,pi, TRIGTYPE_ENVOKED);
                    curr->envokeid1=0x00;
                    curr->envokeid2=0x00;
                    return;
                }
                // Checking if target is an NPC --- By Magius(CHE) º
                i=calcCharFromSer(serial);
                if(i>=0)
                {
                    ConOut("Envoke triggered on npc :]\n");
                    pc = MAKE_CHARREF_LR(i);
		    VALIDATEPC(pc);
                    triggerNpc(s, pc, TRIGTYPE_NPCENVOKED);
                    curr->envokeid1=0x00;
                    curr->envokeid2=0x00;
                    return;
                }
                // End Addons by Magius(CHE) º
                triggerTile(s);
                ConOut("Envoke triggered in the void :]\n");
                curr->envokeid1=0x00;
                curr->envokeid2=0x00;
                return;
            }
        case 25: Targ->CloseTarget(s); break;
        case 26: Targ->AddMenuTarget(s, 1, addmitem[s]); break;
        case 27: Targ->NpcMenuTarget(s); break;
        case 28: ItemTarget(ps,pt); break;//MovableTarget
        case 29: Skills::ArmsLoreTarget(s); break;
        case 30: if (Cready) 
				    OwnerTarget(ps,pc); 
				 else if (Iready) 
				 	OwnerTarget(ps,pi); 
				 break;
        case 31: ItemTarget(ps,pt); break;//ColorsTarget
        case 32: Targ->DvatTarget(s); break;
        case 33: if (Lready) AddNpcTarget(s,pt); break;
        case 34: if (Cready) { pc->priv2|=2; pc->teleport(); } break;
        case 35: if (Cready) { pc->priv2&=0xfd; pc->teleport(); } break; // unfreeze, AntiChris used LB bugfix
								 /*if (Cready)
								 {
								  Targ->CloseTarget(s);
								 	pc->priv2&=0xfd;
									Targ->XTeleport(s,0);
									pc->priv2&=0xfd;
								 }
								 break;*/
        case 36: Targ->AllSetTarget(s); break;
        case 37: Skills::AnatomyTarget(s); break;
        case 38: /*Magic->Recall(s); break;*/
        case 39: /*Magic->Mark(s); break;*/
        case 40: Skills::ItemIdTarget(s); break;
        case 41: Skills::Evaluate_int_Target(s); break;
        case 42: Skills::TameTarget(s); break;
        case 43: /*Magic->Gate(s); break;*/
        case 44:	//Luxor
		{
        	P_CHAR pc_toheal = pointers::findCharBySerial(LongFromCharPtr(buffer[s]+7));
        	VALIDATEPC(pc_toheal);
			pc_toheal->hp = pc_toheal->getStrength();
        	pc_toheal->updateStats(STAT_HP);
        }
        break;
        case 45: Fishing->FishTarget(ps); break;
        case 46: InfoTarget(s,pt); break;
        case 47: /* if (Cready) strcpy(pc->title,xtext[s]); */ break;//TitleTarget
        case 48: break; //XAN : THIS *IS* FREE
        case 49: Skills::CookOnFire(s,0x097B,"fish steaks"); break;
        case 50: Skills::Smith(s); break;
        case 51: Skills::Mine(s); break;
        case 52: Skills::SmeltOre(s); break;
        case 53: npcact(s); break;
        case 54: Skills::CookOnFire(s,0x09B7,"bird"); break;
        case 55: Skills::CookOnFire(s,0x160A,"lamb"); break;
        case 56: Targ->NpcTarget(s); break;
        case 57: Targ->NpcTarget2(s); break;
        case 58: VALIDATEPC(curr); curr->resurrect(); break;
        case 59: Targ->NpcCircleTarget(s); break;
        case 60: Targ->NpcWanderTarget(s); break;
        case 61: Targ->VisibleTarget(s); break;
        case 62: /* Targ->TweakTarget(s); */ break;
        case 63: //MoreXTarget
        case 64: //MoreYTarget
        case 65: //MoreZTarget
        case 66: ItemTarget(ps,pt); break;//MoreXYZTarget
        case 67: Targ->NpcRectTarget(s); break;
        case 68: Skills::CookOnFire(s,0x09F2,"ribs"); break;
        case 69: Skills::CookOnFire(s,0x1608,"chicken legs"); break;
        case 70: Skills::TasteIDTarget(s); break;
        case 71: if (Iready) ContainerEmptyTarget1(ps,pi); break;
        case 72: if (Iready) ContainerEmptyTarget2(ps,pi); break;
        case 73: CodedNpcRectangle(s,pt); break;
        case 76: AxeTarget(ps,pt); break;
        case 77: Skills::DetectHidden(s); break;

        case 79: Skills::ProvocationTarget1(s); break;
        case 80: Skills::ProvocationTarget2(s); break;
        case 81: Skills::EnticementTarget1(s); break;
        case 82: Skills::EnticementTarget2(s); break;

        case 86: Targ->SwordTarget(ps); break;
        case 87: /*Magic->SbOpenContainer(s);*/ break;
        case 88: Targ->SetDirTarget(s); break;
        case 89: ItemTarget(ps,pt); break;//ObjPrivTarget

        case 100: /*Magic->NewCastSpell( s );*/ break;  // we now have this as our new spell targeting location

        case 105: Targ->xSpecialBankTarget(s); break;//AntiChrist
        case 106: Targ->NpcAITarget(s); break;
        case 107: Targ->xBankTarget(s); break;
        case 108: Skills::AlchemyTarget(s); break;
        case 109: Skills::BottleTarget(s); break;
        case 110: Targ->DupeTarget(s); break;
        case 111: ItemTarget(ps,pt); break;//MovableTarget
        case 112: Targ->SellStuffTarget(s); break;
        case 113: Targ->ManaTarget(s); break;
        case 114: Targ->StaminaTarget(s); break;
        case 115: Targ->GmOpenTarget(s); break;
        case 116: Targ->MakeShopTarget(s); break;
        case 117: Targ->FollowTarget(s); break;
        case 118: Targ->AttackTarget(s); break;
        case 119: Targ->TransferTarget(s); break;
        case 120: Targ->GuardTarget( s ); break;
        case 121: Targ->BuyShopTarget(s); break;
        case 122: ItemTarget(ps,pt); break;//SetValueTarget
        case 123: ItemTarget(ps,pt); break;//SetRestockTarget
        case 124: Targ->FetchTarget(s); break;

        case 126: Targ->JailTarget(s,-1); break;
        case 127: Targ->ReleaseTarget(s,-1); break;
        case 128: Skills::CreateBandageTarget(s); break;
        case 129: ItemTarget(ps,pt); break;//SetAmount2Target
        case 130: Skills::HealingSkillTarget(s); break;
        case 131: VALIDATEPC(curr); if (curr->IsGM()) Targ->permHideTarget(s); break; /* not used */
        case 132: VALIDATEPC(curr); if (curr->IsGM()) Targ->unHideTarget(s); break; /* not used */
        case 133: ItemTarget(ps,pt); break;//SetWipeTarget
        case 134: Skills::Carpentry(s); break;
        case 135: Targ->SetSpeechTarget(s); break;
        case 136: Targ->XTeleport(s,0); break;

        case 150: SetSpAttackTarget(s); break;
        case 151: Targ->FullStatsTarget(s); break;
        case 152: Skills::BeggingTarget(s); break;
        case 153: Skills::AnimalLoreTarget(s); break;
        case 154: Skills::ForensicsTarget(s); break;
        case 155:
            {
                curr->poisonserial=LongFromCharPtr(buffer[s]+7);
                target(s, 0, 1, 0, 156, TRANSLATE("What item do you want to poison?"));
                return;
            }
        case 156: Skills::PoisoningTarget(ps); break;

        case 160: Skills::Inscribe(s); break;

        case 162: Skills::LockPick(ps); break;

        case 164: Skills::Wheel(s, YARN); break;
        case 165: Skills::Loom(s); break;
        case 166: Skills::Wheel(s, THREAD); break;
        case 167: Skills::Tailoring(s); break;

        case 170: Targ->LoadCannon(s); break;
        case 171: /*Magic->BuildCannon(s);*/ break;
        case 172: Skills::Fletching(s); break;
        case 173: Skills::MakeDough(s); break;
        case 174: Skills::MakePizza(s); break;
        case 175: Targ->SetPoisonTarget(s); break;
        case 176: Targ->SetPoisonedTarget(s); break;
        case 177: Targ->SetSpaDelayTarget(s); break;
        case 178: Targ->SetAdvObjTarget(s); break;
        case 179: if (Cready) SetInvulFlag(ps,pc); break;
        case 180: Skills::Tinkering(s); break;
        case 181: Skills::PoisoningTarget(ps); break;

        case 183: Skills::TinkerAxel(s); break;
        case 184: Skills::TinkerAwg(s); break;
        case 185: Skills::TinkerClock(s); break;
        //case 186: Necro::vialtarget(s); break;
        case 187: Skills::RemoveTraps(s); break;
        /* -- BEGIN Custom NoX-Wizard targets */
        case 191:
        { //<Luxor>: AMX Target Callback
            TargetLocation TL(pt);
            targetCallback(s, TL);
            break;
        } //</Luxor>
        case 192: PartySystem::targetParty(ps); break;   // Xan Party System
        case 193: Targ->AllAttackTarget(s); break;      // Luxor All Attack
		case 194:
		{
		     TargetLocation TL(pt);
		     magic::castSpell(curr->spell, TL, curr);
		     break;
        }
        /* -- END Custom NoX-Wizard targets */

        case 198: Tiling(s,pt); break;
        case 199: /* Targ->Wiping(s); */ break;
        case 200: Commands::SetItemTrigger(s); break;
        case 201: Commands::SetNPCTrigger(s); break;
        case 202: Commands::SetTriggerType(s); break;
        case 203: Commands::SetTriggerWord(s); break;
        case 204: triggertarget(s); break; // Fixed by Magius(CHE)
        case 205: Skills::StealingTarget(ps); break;
        case 206: Targ->CanTrainTarget(s); break;
        case 207: ExpPotionTarget(s,pt); break;
        case 209: Targ->SetSplitTarget(s); break;
        case 210: Targ->SetSplitChanceTarget(s); break;
        case 212: Commands::Possess(s); break;
        case 213: Skills::PickPocketTarget(ps); break;

        case 220: Guilds->Recruit(s); break;
        case 221: Guilds->TargetWar(s); break;
        case 222: TeleStuff(s,pt); break;
        case 223: Targ->SquelchTarg(s); break;//Squelch
        case 224: Targ->PlVBuy(s); break;//PlayerVendors
        case 225: Targ->Priv3XTarget(s); break; // SETPRIV3 +/- target
        case 226: /* Targ->ShowPriv3Target(s); */ break; // SHOWPRIV3
        case 227: Targ->HouseOwnerTarget(s); break; // cj aug11/99
        case 228: Targ->HouseEjectTarget(s); break; // cj aug11/99
        case 229: Targ->HouseBanTarget(s); break; // cj aug12/99
        case 230: Targ->HouseFriendTarget(s); break; // cj aug 12/99
        case 231: Targ->HouseUnlistTarget(s); break; // cj aug 12/99
        case 232: Targ->HouseLockdown( s ); break; // Abaddon 17th December 1999
        case 233: Targ->HouseRelease( s ); break; // Abaddon 17th December 1999
        case 234: Targ->HouseSecureDown( s ); break; // Ripper
        case 235: Targ->BanTarg(s); break;
        case 236: Skills::RepairTarget(s); break; //Ripper..Repairing item
        //case 237: Skills->SmeltItemTarget(s); break; Ripper..Smelting item
        //taken from 6904t2(5/10/99) - AntiChrist
        case 240: Targ->SetMurderCount( s ); break; // Abaddon 13 Sept 1999

        case 245: buildhouse(s,addid3[s]);   break;

        case 247: Targ->ShowSkillTarget(s);break; //showskill target
        case 248: Targ->MenuPrivTarg(s);break; // menupriv target
        //case 249: Targ->UnglowTaget(s);break; // unglow
        case 250: if ((Cready)&&(ISVALIDPC(pc))) Priv3Target(s,pc); break; // meta gm target
        case 251: Targ->NewXTarget(s); break; // NEWX
        case 252: Targ->NewYTarget(s); break; // NEWY
        case 253: Targ->IncXTarget(s); break; // INCX
        case 254: Targ->IncYTarget(s); break; // INCY
//        case 255: Targ->IncZTarget(s); break; // INCZ
        // 255 Specially handled for managed commands (Rik)
        case 255: {
            TargetLocation target(pt);
            ps->receiveTarget(target);
            ps->continueCommand();
            break;
        }
        case 256: Targ->SetHome(s); break;
        case 257: Targ->SetWork(s); break;
        case 258: Targ->SetFood(s); break;

        default:
            LogError("Fallout of switch statement, multitarget(), value=(%i)",pt->Tnum);
        }
    }

}

///////////////////////////////////////////////////////////////////////
//
// TARGETLOCATION CLASS
//
///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Function name     : TargetLocation::TargetLocation
// Author            : Xanathar
// Changes           : none yet
void TargetLocation::init(P_CHAR pc)
{
	Location pcpos= pc->getPosition();

	m_pc = pc;
	m_x = pcpos.x;
	m_y = pcpos.y;
	m_z = pcpos.z;
	m_pi = NULL;
	m_piSerial = INVALID;
	m_pcSerial = pc->getSerial32();
}
///////////////////////////////////////////////////////////////////
// Function name     : void TargetLocation::init
// Author            : Xanathar
// Changes           : none yet
void TargetLocation::init(P_ITEM pi)
{
	m_pc = NULL;
	if (pi->isInWorld()) {
		m_x = pi->getPosition("x");
		m_y = pi->getPosition("y");
		m_z = pi->getPosition("z");
	} else {
		m_x = m_y = m_z = 0;
	}
	m_pi = pi;
	m_piSerial = pi->getSerial32();
	m_pcSerial = INVALID;
}

///////////////////////////////////////////////////////////////////
// Function name     : TargetLocation::init
// Author            : Xanathar
// Changes           : none yet
void TargetLocation::init(int x, int y, int z)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_pi = NULL;
	m_pc = NULL;
	m_piSerial = m_pcSerial = INVALID;
}


///////////////////////////////////////////////////////////////////
// Function name     : TargetLocation::revalidate
// Description       : recalculates item & char from their serial :)
// Return type       : void
// Author            : Xanathar
// Changes           : none yet
void TargetLocation::revalidate()
{
	m_pi=pointers::findItemBySerial(m_piSerial);

	m_pc=pointers::findCharBySerial(m_piSerial);

	if (ISVALIDPI(m_pi)) {
		m_pi = NULL;
		m_piSerial = INVALID;
	}

	if (ISVALIDPC(m_pc)) {
		m_pc = NULL;
		m_pcSerial = INVALID;
	}
}


///////////////////////////////////////////////////////////////////
// Function name     : TargetLocation::extendItemTarget
// Description       : extends item data to x,y,z and eventually owner char
// Return type       : void
// Author            : Xanathar
// Changes           : none yet
void TargetLocation::extendItemTarget()
{
	if (m_pc!=NULL) 
		return;
	if (m_pi==NULL) 
		return;
	if (m_pi->isInWorld()) {
		m_x = m_pi->getPosition("x");
		m_y = m_pi->getPosition("y");
		m_z = m_pi->getPosition("z");
	} 
	else {
		int it, ch;
		getWorldCoordsFromSerial (m_pi->getSerial32(), m_x, m_y, m_z, ch, it);
		m_pc=MAKE_CHAR_REF(ch);
		m_pcSerial = (ISVALIDPC(m_pc))? m_pc->getSerial32() : INVALID;
	}
	revalidate();
}



///////////////////////////////////////////////////////////////////
// Function name     : TargetLocation::TargetLocation
// Author            : Xanathar
// Changes           : none yet
TargetLocation::TargetLocation(PKGx6C* pp)
{
	if( pp->type==0 ) {
		P_CHAR pc= pointers::findCharBySerial(pp->Tserial);
		P_ITEM pi= pointers::findItemBySerial(pp->Tserial);

		if(ISVALIDPC(pc)) 
			init(pc);
		else if (ISVALIDPI(pi)) 
			init(pi);
	}
	else if( pp->type==1 )
		init(pp->TxLoc,pp->TyLoc,pp->TzLoc);
	else {
		this->m_pc=NULL;
		this->m_pcSerial=INVALID;
		this->m_pi=NULL;
		this->m_piSerial=INVALID;
		this->m_x=0;
		this->m_y=0;
		this->m_z=0;
	}
}



cPacketTargeting::cPacketTargeting()
{
}

cPacketTargeting::~cPacketTargeting()
{
}

UI08 cPacketTargeting::getTargetType( NXWSOCKET socket )
{
	return buffer[socket][1];
}

SI32 cPacketTargeting::getCharacterSerial( NXWSOCKET socket )
{
	return LongFromCharPtr( buffer[socket] + 2 );
}

UI08 cPacketTargeting::getCursorType( NXWSOCKET socket )
{
	return buffer[socket][6];
}

SI32 cPacketTargeting::getItemSerial( NXWSOCKET socket )
{
	return LongFromCharPtr( buffer[socket] + 7 );
}

SI16 cPacketTargeting::getX( NXWSOCKET socket )
{
	return ShortFromCharPtr( buffer[socket] + 11 );
}

SI16 cPacketTargeting::getY( NXWSOCKET socket )
{
	return ShortFromCharPtr( buffer[socket] + 13 );
}

SI08 cPacketTargeting::getZ( NXWSOCKET socket )
{
	return (buffer[socket][17]);
}

SI16 cPacketTargeting::getModel( NXWSOCKET socket )
{
	return ShortFromCharPtr( buffer[socket] + 17 );
}

SI08 cPacketTargeting::getUnknown( NXWSOCKET socket )
{
	return (buffer[socket][15]);
}

