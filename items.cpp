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
#include "amx/amxcback.h"
#include "magic.h"
#include "set.h"
#include "itemid.h"
#include "race.h"
#include "debug.h"
#include "pointer.h"
#include "range.h"
#include "data.h"
#include "boats.h"
#include "archive.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "basics.h"
#include "utils.h"
#include "house.h"
#include "spawn.h"

/*!
\author Duke
\brief minimum of free slots that should be left in the array.

otherwise, more memory will be allocated in the mainloop (Duke)
*/
#define ITEM_RESERVE 3000

/*!
\brief Base constructor for cWeapon class
*/
cWeapon::cWeapon(SERIAL serial) : cItem (serial)
{
}

/*!
\author Luxor
\brief operator = for the cItem class, let's dupe :)
\todo dupe books fix
*/
cItem& cItem::operator=(cItem& b)
{
        // NAMES
        setCurrentName(b.getCurrentName());
        setRealName(b.getRealName());

        setScriptID(b.getScriptID());
        creator = b.creator;
        incognito = b.incognito;
        madewith = b.madewith;
        rank = b.rank;
        good = b.good;
        rndvaluerate = b.rndvaluerate;
        //setMultiSerial32(b.getMultiSerial32());
        setId( b.getId() );
        //setPosition(b.getPosition());
        //setOldPosition(b.getOldPosition());
        setColor( b.getColor() );
        setContSerial(INVALID);
        //setContSerial(b.getContSerial(true), true);
        layer = b.layer;
        oldlayer = b.oldlayer;
        scriptlayer = b.scriptlayer;
        itmhand = b.itmhand;
        type = b.type;
        type2 = b.type2;
        offspell = b.offspell;
        weight = b.weight;
        more1 = b.more1;
        more2 = b.more2;
        more3 = b.more3;
        more4 = b.more4;
        moreb1 = b.moreb1;
        moreb2 = b.moreb2;
        moreb3 = b.moreb3;
        moreb4 = b.moreb4;
        morex = b.morex;
        morey = b.morey;
        morez = b.morez;
        amount = b.amount;
        amount2 = b.amount2;
        doordir = b.doordir;
        dooropen = b.dooropen;
        pileable = b.pileable;
        dye = b.dye;
        corpse = b.corpse;
        carve = b.carve;
        att = b.att;
        def = b.def;
        fightskill = b.fightskill;
        reqskill[0] = b.reqskill[0];
        reqskill[1] = b.reqskill[1];
        damagetype = b.damagetype;
        auxdamagetype = b.auxdamagetype;
        auxdamage = b.auxdamage;
        lodamage = b.lodamage;
		hidamage = b.hidamage;
        smelt = b.smelt;
        secureIt = b.secureIt;
        wpsk = b.wpsk;
        hp = b.hp;
        maxhp = b.maxhp;
        st = b.st;
        st2 = b.st2;
        dx = b.dx;
        dx2 = b.dx2;
        in = b.in;
        in2 = b.in2;
        spd = b.spd;
        wipe = b.wipe;
        magic = b.magic;
        gatetime = b.gatetime;
        gatenumber = b.gatenumber;
        decaytime = b.decaytime;
        //setOwnerSerial32(b.getOwnerSerial32());
        visible = b.visible;
        dir = b.dir;
        priv = b.priv;
        value = b.value;
        restock = b.restock;
        trigger = b.trigger;
        trigtype = b.trigtype;
        tuses = b.tuses;
        poisoned = b.poisoned;
        murderer = b.murderer;
        murdertime = b.murdertime;
        time_unused = b.time_unused;
        timeused_last = b.timeused_last;
        animid1 = b.animid1;
        animid2 = b.animid2;
		ammo = b.ammo;
		ammoFx = b.ammoFx;
		this->itemSoundEffect = b.itemSoundEffect;

        UI32 i;
        for ( i = 0; i < MAX_RESISTANCE_INDEX; i++ )
                resists[i] = b.resists[i];

	for ( i=0; i < ALLITEMEVENTS; i++ ) {
		amxevents[i] = b.amxevents[i];
		/*
		AmxEvent* event = b.getAmxEvent( i );
		if ( event == NULL )
			continue;

		setAmxEvent( i, event->getFuncName(), !(event->shouldBeSaved()) );*/
	}
	vendorDescription = b.vendorDescription;
	amxVS.copyVariable(b.getSerial32(), getSerial32());

        return *this;
}


//
// Class methods
//
void cItem::archive()
{
	std::string saveFileName( SrvParms->savePath + SrvParms->itemWorldfile + SrvParms->worldfileExtension );
	std::string timeNow( getNoXDate() );
	for( int i = timeNow.length() - 1; i >= 0; --i )
		switch( timeNow[i] )
		{
			case '/' :
			case ' ' :
			case ':' :
				timeNow[i]= '-';
		}
	std::string archiveFileName( SrvParms->archivePath + SrvParms->itemWorldfile + timeNow + SrvParms->worldfileExtension );
	char tempBuf[60000]; // copy files in 60k chunks
	ifstream oldSave;
	ofstream archiveSave;
	oldSave.open(saveFileName.c_str(), ios::binary );
	archiveSave.open(archiveFileName.c_str(), ios::binary);
	if ( ! archiveSave.is_open() || ! oldSave.is_open() )
	{
		LogWarning("Could not copy file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
		return;
	}
	while ( ! oldSave.eof() )
	{
		int byteCount;
		oldSave.read(&tempBuf[0], sizeof(tempBuf)); 
		byteCount = oldSave.gcount();
		archiveSave.write(&tempBuf[0], byteCount);
	}
	
	InfoOut("Copied file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
}

void cItem::safeoldsave()
{
	std::string oldFileName( SrvParms->savePath + SrvParms->itemWorldfile + SrvParms->worldfileExtension );
	std::string newFileName( SrvParms->savePath + SrvParms->itemWorldfile + SrvParms->worldfileExtension + "$" );
	remove( newFileName.c_str() );
	rename( oldFileName.c_str(), newFileName.c_str() );
}

//
// Object methods
//
/*
void setserial(int nChild, int nParent, int nType)
{ // Sets the serial #'s and adds to pointer arrays
  // types: 1-item container, 2-item spawn, 3-Item's owner 4-container is PC/NPC
  //        5-NPC's owner, 6-NPC spawned

	if (nChild == -1 || nParent == -1) return;
	switch(nType)
	{
	case 1:
	    items[nChild].setContSerial(items[nParent].getSerial32());	//Luxor
		//setptr(&contsp[items[nChild].contserial%HASHMAX], nChild);
		break;
	case 2:				// Set the Item's Spawner
	    items[nChild].spawnserial=items[nParent].getSerial32();
		setptr(&spawnsp[items[nChild].spawnserial%HASHMAX], nChild);
		break;
	case 3:				// Set the Item's Owner
	    items[nChild].setOwnSerialOnly(chars[nParent].getSerial32());
		setptr(&ownsp[items[nChild].ownserial%HASHMAX], nChild);
		break;
	case 4:
	    items[nChild].setContSerial(chars[nParent].getSerial32());	//Luxor
		//setptr(&contsp[items[nChild].contserial%HASHMAX], nChild);
		break;
	case 5:				// Set the Character's Owner
		chars[nChild].setOwnerSerial32Only(chars[nParent].getSerial32());
		setptr(&cownsp[chars[nChild].getOwnerSerial32()%HASHMAX], nChild);
		//taken from 6904t2(5/10/99) - AntiChrist
		if( nChild != nParent )
			chars[nChild].tamed = true;
		else
			chars[nChild].tamed = false;
		break;
	case 6:				// Set the character's spawner
	    chars[nChild].spawnserial=items[nParent].getSerial32();
		setptr(&cspawnsp[chars[nChild].spawnserial%HASHMAX], nChild);
		break;
	case 7:				// Set the Item in a multi
		items[nChild].setMultiSerial32Only( items[nParent].getSerial32() );
		setptr(&imultisp[items[nChild].getMultiSerial32()%HASHMAX], nChild);
		break;
	case 8:				//Set the CHARACTER in a multi
		chars[nChild].setMultiSerial32Only(items[nParent].getSerial32());
		setptr(&cmultisp[chars[nChild].getMultiSerial32()%HASHMAX], nChild);
		break;
	default:
		WarnOut("No handler for nType (%08x) in setserial()", nType);
		break;
	}
}
*/

cItem::~cItem()
{

}

/*!
\brief set the serial of the item's container
\author Anthalir
\since 0.82a
\param serial new serial
\param old set the saved cont serial or the actual one ?
\param update update the container map ?
*/
void cItem::setContSerial(SI32 serial, LOGICAL old, LOGICAL update )
{
	if( old ) {
		oldcontserial.serial32= serial;
	} else { //Luxor bug fix
		oldcontserial.serial32= contserial.serial32;
		contserial.serial32= serial;
		if (serial == INVALID)
			setDecayTime();
	}

	if( update )
		pointers::updContMap(this);
}

void cItem::setContSerialByte(UI32 nByte, BYTE value, LOGICAL old/*= false*/)
{
	Serial *cont;

	if( old )
		cont= &oldcontserial;
	else
		cont= &contserial;

	switch( nByte )
	{
	case 1: cont->ser1= value; break;
	case 2: cont->ser2= value; break;
	case 3: cont->ser3= value; break;
	case 4: cont->ser4= value; break;
	default: LogWarning("cannot access byte %i of serial !!", nByte);
	}
}

BYTE cItem::getContSerialByte(UI32 nByte, LOGICAL old/*= false*/) const
{
	const Serial *cont;

	if( old )
		cont= &oldcontserial;
	else
		cont= &contserial;

	switch( nByte )
	{
	case 1: return cont->ser1;
	case 2: return cont->ser2;
	case 3: return cont->ser3;
	case 4: return cont->ser4;
	default: LogWarning("cannot access byte %i of serial !!", nByte);
	}

	return 0;
}

SI32 cItem::getContSerial(LOGICAL old/*= 0*/) const
{
	if(old)
		return oldcontserial.serial32;
	else
		return contserial.serial32;
}

const cObject* cItem::getContainer() const
{
	SI32 ser= contserial.serial32;

	if( isItemSerial(ser) )			// container is an item
		return pointers::findItemBySerial(ser);

	if( isCharSerial(ser) )			// container is a player
		return pointers::findCharBySerial(ser);

	return NULL;					// container serial is invalid
}

/*!
\author Luxor
\brief execute decay on the item
\return true if decayed (so deleted), false else
*/
LOGICAL cItem::doDecay()
{
	if ( !canDecay() )
		return false;

	if ( magic == 4/* || magic == 2*/ )
		return false;

	if ( !isInWorld() )
		return false;

	if ( TIMEOUT( decaytime ) )
	{


		if ( amxevents[EVENT_IONDECAY] !=NULL )
		{
			g_bByPass = false;
			amxevents[EVENT_IONDECAY]->Call(getSerial32(), DELTYPE_DECAY);
			if ( g_bByPass == true )
				return false;
		}
		/*
		g_bByPass = false;
		runAmxEvent( EVENT_IONDECAY, getSerial32(), DELTYPE_DECAY );
		if ( g_bByPass == true )
			return false;
		*/

		//Multis
		if ( !isFieldSpellItem() && !corpse )
		{
			if ( getMultiSerial32() == INVALID )
			{
				P_HOUSE house=cHouses::findHouse(getPosition());
				if ( house != NULL )
				{
					P_ITEM pi_multi = pointers::findItemBySerial (house->getSerial());
					if ( ISVALIDPI(pi_multi) )
					{
						if ( pi_multi->more4 == 0 )
						{
							setDecayTime();
							SetMultiSerial(pi_multi->getSerial32());
							return false;
						}
					}
				}
			}
			else
			{
				setDecayTime();
				return false;
			}
		}
		//End Multis

		if( type == ITYPE_CONTAINER || ( !SrvParms->lootdecayswithcorpse && corpse ) )
		{
			NxwItemWrapper si;
			si.fillItemsInContainer( this, false );
			for( si.rewind(); !si.isEmpty(); si++ )
			{
				P_ITEM pj = si.getItem();
				if( ISVALIDPI(pj) )
				{
					pj->setContSerial(INVALID);
					pj->MoveTo( getPosition() );
					pj->setDecayTime();
					pj->Refresh();
				}
			}
		}
		Delete();
		return true;
	}
	else
		return false;
}


void cItem::explode(NXWSOCKET  s)
{
	if (s < 0 || s > now) return;	//Luxor

	unsigned int dmg=0,len=0;

	P_CHAR pc_current=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_current);

	if(!isInWorld())
		return;

	type=0; //needed for recursive explosion

	//Luxor - recursive explosions!! :DD
	NxwItemWrapper si;
	si.fillItemsNearXYZ( getPosition(), 5, true );
    for( si.rewind(); !si.isEmpty(); si++ ) {
		P_ITEM p_nearbie=si.getItem();
		if(ISVALIDPI(p_nearbie) && p_nearbie->type == ITYPE_POTION && p_nearbie->morey == 3) { //It's an explosion potion!
			p_nearbie->explode(s);
    	}
    }
	//End Luxor recursive explosions

	staticeffect2(this, 0x36, 0xB0, 0x10, 0x80, 0x00);
	soundeffect3(this, 0x0207);

	len=morex/250; //4 square max damage at 100 alchemy
	switch (morez)
	{
		case 1:dmg=RandomNum( 5,10) ;break;
		case 2:dmg=RandomNum(10,20) ;break;
		case 3:dmg=RandomNum(20,40) ;break;
		default:
			ErrOut("Switch fallout. NoX-Wizard.cpp, explodeitem()\n"); //Morrolan
			dmg=RandomNum(5,10);
	}

	if (dmg<5) dmg=RandomNum(5,10);	// 5 points minimum damage
	if (len<2) len=2;	// 2 square min damage range

	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( getPosition(), len, true );
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {

		P_CHAR pc=sc.getChar();
		if( ISVALIDPC(pc) ) {
			pc->damage( dmg+(2-pc->distFrom(this)), DAMAGE_FIRE );
		}
	}

	Delete();

}

/*
\author Duke
\brief reduce the amount of piled items
\param amt amount to subtract to item amount

Reduces the given item's amount by 'amt' and deletes it if necessary and returns 0.
If the request could not be fully satisfied, the remainder is returned
*/
SI32 cItem::ReduceAmount(const SI16 amt)
{
	long rest=0;
	if( amount > amt )
	{
		amount-=(unsigned short)amt;
		Refresh();
	}
	else
	{
		rest=amt-amount;
		Delete();
	}
	return rest;

}
/*!
\brief increase the amount of piled items
\param amt amount to add to item amount
*/
SI32 cItem::IncreaseAmount(const SI16 amt)
{
	amount+= amt;
	Refresh();
	return amount;
}

/*
// This method does not change the pointer arrays !!
// should only be used in VERY specific situations like initItem... Duke, 6.4.2001
void cItem::setContSerialOnly(SI32 contser)
{
	oldcontserial.serial32= contserial.serial32;	//Luxor
	contserial.serial32= contser;

	cont1=(unsigned char) ((contser&0xFF000000)>>24);
	cont2=(unsigned char) ((contser&0x00FF0000)>>16);
	cont3=(unsigned char) ((contser&0x0000FF00)>>8);
	cont4=(unsigned char) ((contser&0x000000FF));

}

void cItem::setContSerial(SI32 contser)
{
	setContSerial(contser, false, false);
	pointers::updContMap(this);	//Luxor
}
*/

/*
void cItem::setOwnSerialOnly(SI32 ownser)
{
	ownserial=ownser;
	owner1=(unsigned char) ((ownser&0xFF000000)>>24);
	owner2=(unsigned char) ((ownser&0x00FF0000)>>16);
	owner3=(unsigned char) ((ownser&0x0000FF00)>>8);
	owner4=(unsigned char) ((ownser&0x000000FF));
}

void cItem::SetOwnSerial(SI32 ownser)
{
	if (ownserial!=-1)	// if it was set, remove the old one
		removefromptr(&ownsp[ownserial%HASHMAX], DEREF_P_ITEM(this));

	setOwnSerialOnly(ownser);

	if (ownser!=-1)		// if there is an owner, add it
		setptr(&ownsp[ownserial%HASHMAX], DEREF_P_ITEM(this));

}
*/


void cItem::SetMultiSerial(SI32 mulser)
{
	if (getMultiSerial32()!=INVALID)	// if it was set, remove the old one
		pointers::delFromMultiMap(this);

	setMultiSerial32Only(mulser);

	if (getMultiSerial32()!=INVALID)		// if there is multi, add it
		pointers::addToMultiMap(this);

}

/*!
\author Anthalir
*/
void cItem::MoveTo(Location newloc)
{
#ifdef SPAR_I_LOCATION_MAP
	setPosition( newloc );
	pointers::updateLocationMap(this);
#else
	mapRegions->remove(this);
	setPosition( newloc );
	mapRegions->add(this);
#endif
}

/*!
\brief Add item to container
\author Endymion
\param pItem the item to add
\param xx the x location or INVALID if use rand pos
\param yy the y location or INVALID if use rand pos
*/
LOGICAL cItem::AddItem(P_ITEM pItem, short xx, short yy)
{

	VALIDATEPIR(pItem,false);

	NxwSocketWrapper sw;
	sw.fillOnline( pItem );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
		SendDeleteObjectPkt(sw.getSocket(), pItem->getSerial32() );


	if (xx!=-1)	// use the given position
	{
		pItem->setContSerial( getSerial32() );
		pItem->setPosition(xx, yy, 9);
	}
	else		// no pos given
	{
		if( !ContainerPileItem(pItem) )	{ // try to pile
			pItem->SetRandPosInCont(this);		// not piled, random pos
			pItem->setContSerial( getSerial32() );
		}
		else
			return true; //Luxor: we cannot do a refresh because item was piled
	}
	pItem->Refresh();
	return true;

}

/*
\brief Check if two item are similar so pileable
\author Endymion
\todo add amx vars and events
\note if same item is compared, false is returned
*/
inline bool operator ==( cItem& a, cItem& b ) {
	return  ( a.pileable && b.pileable ) &&
			( a.getSerial32() != b.getSerial32() ) &&
			( a.getScriptID() == b.getScriptID() ) &&
			( a.getId() == b.getId() ) &&
			( a.getColor() == b.getColor() ) &&
			( a.poisoned == b.poisoned );
}

/*
\brief Check if two item are not similar so not pileable
\author Endymion
*/
inline bool operator !=( cItem& a, cItem& b ) {
	return !(a==b);
}

#define MAX_ITEM_AMOUNT 65535

/*!
\brief Pile two items
\author
\return true if piled, false else
\note refresh is done if piled
*/
bool cItem::PileItem( P_ITEM pItem )
{
	VALIDATEPIR( pItem, false )
	if( (*this) != (*pItem) )
		return false;	//cannot stack.

	if( amount+ pItem->amount>MAX_ITEM_AMOUNT )
	{
		P_ITEM cont = pointers::findItemBySerial( getContSerial());
		if( ISVALIDPI(cont) )
			pItem->SetRandPosInCont( cont );
		else
			pItem->setPosition( getPosition().x+1, getPosition().y, getPosition().z );

		pItem->setContSerial( getContSerial() );

		pItem->amount=(unsigned short)((amount+pItem->amount)-MAX_ITEM_AMOUNT);
		amount=MAX_ITEM_AMOUNT;
		pItem->Refresh();
	}
	else
	{
		pItem->setPosition( getPosition() );
		pItem->setContSerial( getContSerial() );
		pItem->amount+=(unsigned short)amount;
		pItem->Refresh();
		Delete();
	}

	return true;

}

/*!
\brief try to find an item in the container to stack with
*/
bool cItem::ContainerPileItem( P_ITEM pItem)
{
	VALIDATEPIR(pItem, false );
	NxwItemWrapper si;
	si.fillItemsInContainer( this, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if( ISVALIDPI(pi) ) {
			if( pi->PileItem(pItem) )
				return true;
		}
	}
	return false;

}

/*!
\author Juliunus
\brief delete a determined amount of an item with determined color

Recurses through the container given by serial and deletes items of the given id and color (if given)
until the given amount is reached
*/
int cItem::DeleteAmount(int amount, short id, short color)
{
	int rest=amount;

	NxwItemWrapper si;
	si.fillItemsInContainer( this );
	for( si.rewind(); !si.isEmpty(); si++ ) {
		P_ITEM pi=si.getItem();
		if(ISVALIDPI(pi) && (rest > 0) )
		{
			if (pi->getId()==id && (color==INVALID || (pi->getColor()==color)))
				rest=(short)pi->ReduceAmount((SI16)rest);
		}
	}
	return rest;

}

/*!
\author Anthalir
*/
int cItem::DeleteAmountByID(int amount, unsigned int scriptID)
{
	int rest= amount;

	NxwItemWrapper si;
	si.fillItemsInContainer( this, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pi=si.getItem();
		if( !ISVALIDPI(pi)) continue;

		if (pi->type == ITYPE_CONTAINER)
			rest= pi->DeleteAmountByID(amount, scriptID);

		if (pi->getScriptID() == scriptID)
			rest= (short)pi->ReduceAmount((SI16)rest);

		if (rest<= 0)
			break;

	}
	return rest;

}


/*!
\author Elcabesa
*/
void cItem::animSetId(SI16 id)
{
	animid1=(unsigned char)((id>>8)&0xFF);
	animid2=(unsigned char)(id&0x00FF);
}

/*!
\todo backport
*/
int cItem::getName(char* itemname)
{
	return item::getname(DEREF_P_ITEM(this),itemname);
}

/*!
\brief the weight of the single item
\return the weigth
\note May have to seek it from mul files
*/
R32 cItem::getWeight()
{

	if (getId() == ITEMID_GOLD)
		return (R32)SrvParms->goldweight;

	R32 itemweight=0.0;

	if (weight>0) //weight is defined in scripts for this item
		itemweight=(R32)weight;
	else
	{
		tile_st tile;
		data::seekTile(getId(), tile);
		if (tile.weight==0) // can't find weight
		{
			if(type != ITYPE_FOOD)
				itemweight = 2.0;	// not food weighs .02 stone

			else
				itemweight = 100.0;	//food weighs 1 stone
		}
		else //found the weight from the tile, set it for next time
		{
			weight=(tile.weight*100); // set weight so next time don't have to search
			itemweight = (R32)(weight);
		}

	}
	return itemweight;
}


/*!
\brief the weight of the item ( * number of piled item if there are )
\author Endymion
\return the weigth actual
\todo make return value float
*/

cItem::cItem( SERIAL ser )
{

	setSerial32( ser );
	setOwnerSerial32Only(INVALID);
	setMultiSerial32Only(INVALID);//Multi serial

	setCurrentName("#");
	setSecondaryName("#");
	setScriptID( 0 );
	murderer = string("");
	creator = string("");
	incognito=false;//AntiChrist - incognito
	madewith=0; // Added by Magius(CHE)
	rank=0; // Magius(CHE)
	good=-1; // Magius(CHE)
	rndvaluerate=0; // Magius(CHE) (2)
	setId( 0x0001 ); // Item visuals as stored in the client
	setPosition(100, 100, 0);
	setOldPosition( getPosition() );
	setColor( 0x0000 ); // Hue
	contserial.serial32= INVALID; // Container that this item is found in
	oldcontserial.serial32= INVALID;
	layer=oldlayer=0; // Layer if equipped on paperdoll
	scriptlayer=0;	//Luxor
	itmhand=0; // Layer if equipped on paperdoll
	type=0; // For things that do special things on doubleclicking
	type2=0;
	offspell=0;
	weight=0;
	more1=0; // For various stuff
	more2=0;
	more3=0;
	more4=0;
	moreb1=0;
	moreb2=0;
	moreb3=0;
	moreb4=0;
	morex=0;
	morey=0;
	morez=0;
	amount=1; // Amount of items in pile
	amount2=0; //Used to track things like number of yards left in a roll of cloth
	doordir=0; // Reserved for doors
	dooropen=0;
	pileable=0; // Can item be piled
	dye=0; // Reserved: Can item be dyed by dye kit
	corpse=0; // Is item a corpse
	carve=-1;//AntiChrist-for new carving system
	att=0; // Item attack
	def=0; // Item defense
	fightskill=INVALID_SKILL; //Luxor: skill used by item
	reqskill[0]=0; //Luxor: skill value required by item (skillnum = fightskill)
	reqskill[1]=0;
	damagetype=DAMAGE_PURE; //Luxor: damage types system
	auxdamagetype=DAMAGE_PURE; //Luxor: damage types system
	auxdamage=0;
	lodamage=0; //Minimum Damage weapon inflicts
	hidamage=0; //Maximum damage weapon inflicts
	smelt=0; // for smelting items
	secureIt=0; // secured chests
	wpsk=0; //The skill needed to use the item -> Seems not to work for the moment, useless
	hp=0; //Number of hit points an item has.
	maxhp=0; // Max number of hit points an item can have.
	st=0; // The strength needed to equip the item
	st2=0; // The strength the item gives
	dx=0; // The dexterity needed to equip the item
	dx2=0; // The dexterity the item gives
	in=0; // The intelligence needed to equip the item
	in2=0; // The intelligence the item gives
	spd=0; //The speed of the weapon
	wipe=0; //Should this item be wiped with the /wipe command
	magic=0; // 0=Default as stored in client, 1=Always movable, 2=Never movable, 3=Owner movable.
	gatetime=0;
	gatenumber=INVALID;

	visible=0; // 0=Normally Visible, 1=Owner & GM Visible, 2=GM Visible
	dir=0; // Direction, or light source type.
	priv=0; // Bit 0, decay off/on.  Bit 1, newbie item off/on.  Bit 2 Dispellable
	decaytime = 0;
	value=0; // Price shopkeeper sells item at.
	restock=0; // Number up to which shopkeeper should restock this item
	trigger=0; //Trigger number that item activates
	trigtype=0; //Type of trigger
	tuses=0;    //Number of uses for trigger
	poisoned=POISON_NONE; //AntiChrist -- for poisoning skill
	murdertime=0; //AntiChrist -- for corpse -- when the people has been killed
//	glow=0;
//	glow_effect=0;
//	glow_c1=0;
//	glow_c2=0;
	time_unused=0;
	timeused_last=getclock();
	animSetId(0x0000); // elcabesa animation
	ammo=0;
	ammoFx=0;

	/* Luxor: damage resistances */
	int i;
	for (i=0;i<MAX_RESISTANCE_INDEX;i++)
		resists[i]=0;

	for (int X=0; X<ALLITEMEVENTS; X++)
		amxevents[X] = NULL;
	//desc[0]=0x00;
	vendorDescription = std::string("");
	setDecayTime(); //Luxor

}

LOGICAL LoadItemEventsFromScript (P_ITEM pi, char *script1, char *script2)
{

#define CASEITEMEVENT( NAME, ID ) 	else if (!(strcmp(NAME,script1))) pi->amxevents[ID] = newAmxEvent(script2);

	if (!strcmp("@ONSTART",script1))	{
		pi->amxevents[EVENT_IONSTART] = newAmxEvent(script2);
		newAmxEvent(script2)->Call(pi->getSerial32(), -1);
	}
	CASEITEMEVENT("@ONDAMAGE", EVENT_IONDAMAGE)
	CASEITEMEVENT("@ONEQUIP", EVENT_IONEQUIP)
	CASEITEMEVENT("@ONUNEQUIP", EVENT_IONUNEQUIP)
	CASEITEMEVENT("@ONCLICK", EVENT_IONCLICK)
	CASEITEMEVENT("@ONDBLCLICK", EVENT_IONDBLCLICK)
	CASEITEMEVENT("@ONCHECKCANUSE", EVENT_IONCHECKCANUSE)
	CASEITEMEVENT("@ONPUTINBACKPACK", EVENT_IPUTINBACKPACK)
	CASEITEMEVENT("@ONDROPINLAND", EVENT_IDROPINLAND)
	CASEITEMEVENT("@ONDROPONCHAR", EVENT_IDROPONCHAR)
	CASEITEMEVENT("@ONTRANSFER", EVENT_IONTRANSFER)
	CASEITEMEVENT("@ONSTOLEN", EVENT_IONSTOLEN)
	CASEITEMEVENT("@ONPOISONED", EVENT_IONPOISONED)
	CASEITEMEVENT("@ONDECAY", EVENT_IONDECAY)
	CASEITEMEVENT("@ONREMOVETRAP", EVENT_IONREMOVETRAP)
	CASEITEMEVENT("@ONLOCKPICK", EVENT_IONLOCKPICK)
	CASEITEMEVENT("@ONWALKOVER", EVENT_IONWALKOVER)
	CASEITEMEVENT("@ONPUTITEM", EVENT_IONPUTITEM)
	CASEITEMEVENT("@ONTAKEFROMCONTAINER", EVENT_ITAKEFROMCONTAINER)
	else if (!(strcmp("@ONCREATION",script1))) newAmxEvent(script2)->Call(pi->getSerial32(),-1);
	else return false;
	return true;
}

/*!
\author Xanathar
\brief gets the real name of an item (removing #'s)
\return the real name of an item
*/
const char* cItem::getRealItemName()
{
	if ( strncmp(getCurrentNameC(), "#", 1) )
		return getCurrentNameC();
	else
	{
		tile_st tile;
		data::seekTile(getId(), tile);
        	return reinterpret_cast<char*>(tile.name);
	}
}

/*!
\author Luxor
\brief gets the combat skill of an item
\return the combat skill used by the object
*/
Skill cItem::getCombatSkill()
{
	if (fightskill != INVALID_SKILL) return fightskill;
	else if (IsSwordType())		return SWORDSMANSHIP;
	else if (IsMaceType() || IsSpecialMace())		return MACEFIGHTING;
	else if (IsFencingType())	return FENCING;
	else if (IsBowType())		return ARCHERY;
	return WRESTLING;
}

/*!
\author AXanathar
\brief counts the spells in a spellbook
\return the number of spells in the spellbook
*/
int cItem::countSpellsInSpellBook()
{

	int spellcount=0;

	NxwItemWrapper si;
	si.fillItemsInContainer( this, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pj=si.getItem();
		if(!ISVALIDPI(pj)) continue;
		if (pj->getId() == 0x1F6D)
            spellcount = 64;
        else
            spellcount++;
    }

	return (spellcount < 64) ? spellcount : 64;
}


/*!
\author Xanathar
\brief returns if or not a spellbook contains a spell
\param spellnum spell identifier
\return true if the spell is in the spellbook else false
*/
LOGICAL cItem::containsSpell(magic::SpellId spellnum)
{
    LOGICAL raflag = false;

    if (spellnum==magic::SPELL_REACTIVEARMOUR) raflag = true;
    if ((spellnum>=magic::SPELL_CLUMSY) && (spellnum < magic::SPELL_REACTIVEARMOUR))
		spellnum = static_cast<magic::SpellId>(static_cast<int>(spellnum)+1);
    if (raflag) spellnum=static_cast<magic::SpellId>(0);

	NxwItemWrapper si;
	si.fillItemsInContainer( this, false );
	for( si.rewind(); !si.isEmpty(); si++ )
    {
		P_ITEM pj=si.getItem();
		if(!ISVALIDPI(pj)) continue;

        if((pj->getId()==(0x1F2D+spellnum) || pj->getId()==0x1F6D) || pj->getId() == 0x1F6D)
        {
            return true;
        }
    }
    return false;
}

/*!
\author Luxor
\brief deletes the item
\todo backport
*/
void cItem::Delete()
{
	itemCount-=1;
	if ( itemCount < 0 )
		itemCount = 0;
	// call unequip before an item gets deleted, but don't allow bypassing it
	if (amxevents[EVENT_IONUNEQUIP] != NULL)
	{
		amxevents[EVENT_IONUNEQUIP]->Call(getSerial32(), getSerial32());
	}
	if ( this->getSpawnSerial() > 0 )
	{
		cSpawnDinamic * spawn = Spawns->getDynamicSpawn(this->getSpawnSerial());
		if ( spawn != NULL )
			spawn->remove(this->getSerial32());
	}
	if ( this->type == ITYPE_NPC_SPAWNER || this->type==ITYPE_ITEM_SPAWNER)
	{
		cSpawnDinamic * spawn = Spawns->getDynamicSpawn(this->getSerial32());
		if ( spawn != NULL )
			spawn->clear();
	}
	if( type == ITYPE_CONTAINER || ( !SrvParms->lootdecayswithcorpse && corpse ) )
	{
		NxwItemWrapper si;
		si.fillItemsInContainer( this, false );
		for( si.rewind(); !si.isEmpty(); si++ )
		{
			P_ITEM pj = si.getItem();
			pj->Delete();
		}
	}

	archive::deleteItem(this);
}

/*!
\author Luxor and AntiChrist
\brief Refreshes the item
*/
void cItem::Refresh()
{

	if( getContSerial()==getSerial32() )
	{
		ErrOut("item %s [serial: %i] has dangerous container value, autocorrecting...\n", getCurrentNameC(), getSerial32());
		setContSerial(INVALID);
	}

	NxwSocketWrapper sw;
	sw.fillOnline();
	for ( sw.rewind(); !sw.isEmpty(); sw++ ) {
		NXWCLIENT ps_w = sw.getClient();
		if ( ps_w != NULL )
			ps_w->sendRemoveObject(static_cast<P_OBJECT>(this));
	}

	//first check: let's check if it's on the ground....
	if(isInWorld())
	{

		NxwSocketWrapper sw;
		sw.fillOnline( this );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWSOCKET a=sw.getSocket();
			if(a!=INVALID)
				senditem(a, this);
		}
		return;
	}

	//if not, let's check if it's on a char or in a pack

	if( isCharSerial(getContSerial()) )//container is a player...it means it's equipped on a character!
	{
		// elcabesa this is like a wearit() function, we can use here
		P_CHAR charcont= (P_CHAR)(getContainer());

		NxwSocketWrapper sw;
		sw.fillOnline( charcont, false );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWSOCKET a=sw.getSocket();
			if(a!=INVALID)
				wearIt(a, this);
		}
		return;
	}
	else//container is an item...it means we have to use sendbpitem()!!
	{
		NxwSocketWrapper sw;
		sw.fillOnline();
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWSOCKET a=sw.getSocket();
			if(a!=INVALID)
				sendbpitem(a, this);	//NOTE: there's already the inrange check
							//in the sendbpitem() function, so it's unuseful
							//to do a double check!!
		}
	}
}

cContainerItem::cContainerItem(LOGICAL ser/*= true*/) : cItem(ser)
{
	ItemList.empty();
}

/*!
\author Lord Binary
\brief Return the type of pack to handle its x, y coord system correctly
\return see table

<b>Interpretation of the result</b>
<ul>
	<li>type -1: no pack</li>
	<li>type 1: y-range 50 .. 100</li>
	<li>type 2: y-range 30 .. 80</li>
	<li>type 3: y-range 100 .. 150</li>
	<li>type 4: y-range 40 .. 140</li>
</ul>
x-range 18 .. 118 for 1,2,3; 40 for 4
*/
SI16 cContainerItem::getGumpType()
{
	switch( getId() )
	{
	case 0x09b0:
	case 0x09aa:
	case 0x09a8:
	case 0x0e79:
	case 0x0e7a:
	case 0x0e76:
	case 0x0e7d:
	case 0x0e80:
		return 1;

	case 0x09a9:
	case 0x0e3c:
	case 0x0e3d:
	case 0x0e3e:
	case 0x0e3f:
	case 0x0e78:
	case 0x0e7e:
		return 2;

	case 0x09ab:
	case 0x0e40:
	case 0x0e41:
	case 0x0e42:
	case 0x0e43:
	case 0x0e7c:
		return 3;

	case 0x0e75:
	case 0x09b2:
	case 0x0e77:
	case 0x0e7f:
	case 0x0e83:
	case 0x0EFA: 	// spellbook. Position shouldn't matter, but as it can be opened like a backpack...(Duke)
		return 4;

	case 0x2006:
		return 5;	// a corpse/coffin

	default:
		return -1;
	}
}

LOGICAL cContainerItem::pileItem( P_ITEM pItem)	// try to find an item in the container to stack with
{

	NxwItemWrapper si;
	si.fillItemsInContainer( this, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if(!ISVALIDPI(pi)) continue;

		if (!(pileable && pItem->pileable &&
			getId()==pItem->getId() &&
			getColor()==pItem->getColor() ))
			return false;	//cannot stack.

		if (amount+pItem->amount>65535)
		{
			pItem->setPosition( getPosition("x"), getPosition("y"), 9);
			pItem->amount=(amount+pItem->amount)-65535;
			amount=65535;
			pItem->Refresh();
		}
		else
		{
			amount+=pItem->amount;
			pItem->Delete();
		}
		Refresh();
		return true;
	}
	return false;

}

void cContainerItem::setRandPos(P_ITEM pItem)
{
	pItem->setPosition("x", RandomNum(18, 118));
	pItem->setPosition("z", 9);

	switch( getGumpType() )
	{
	case 1:
		pItem->setPosition("y", RandomNum(50, 100));
		break;

	case 2:
		pItem->setPosition("y", RandomNum(30, 80));
		break;

	case 3:
		pItem->setPosition("y", RandomNum(100, 140));
		break;

	case 4:
		pItem->setPosition("y", RandomNum(60, 140));
		pItem->setPosition("x", RandomNum(60, 140));
		break;

	case 5:
		pItem->setPosition("y", RandomNum(85, 160));
		pItem->setPosition("x", RandomNum(20, 70));
		break;

	default:
		pItem->setPosition("y", RandomNum(30, 80));
		break;
	}
}

/*
UI32 cContainerItem::countItems(UI32 scriptID, LOGICAL bAddAmounts)
{
	UI32 count= 0;
	vector<SI32>::iterator it= ItemList.begin();

	do
	{
		P_ITEM pi= pointers::findItemBySerial(*it);
		if( !ISVALIDPI(pi) )
		{
			LogWarning("item's serial not valid: %d", *it);
			continue;
		}

		if( bAddAmounts )
			count+= pi->amount;
		else
			count++;
	}
	while( ++it != ItemList.end() );

	return count;
}

*/
UI32 cContainerItem::removeItems(UI32 scriptID, UI32 amount/*= 1*/)
{
	UI32 rest= amount;
	vector<SI32>::iterator it= ItemList.begin();

	do
	{
		P_ITEM pi= pointers::findItemBySerial(*it);
		VALIDATEPIR(pi, 0);

		if( pi->getScriptID()==scriptID )
			rest= pi->ReduceAmount((SI16)rest);

		if (rest<= 0)
			break;

	}
	while( ++it!=ItemList.end() );

	return rest;
}

/*!
\brief remove item from container but don't delete it from world
*/
void cContainerItem::dropItem(P_ITEM pi)
{
	int ser= pi->getSerial32();
	vector<SI32>::iterator it= ItemList.begin();

	do
	{
		if( *it==ser )	// item found
			ItemList.erase(it);
	}
	while( ++it!=ItemList.end() );
}

/*!
\brief Get the out most container
\author Endymion
\return outer container
\param rec not need to use, only internal for have a max number or recursion
\note max recursion = 50
*/
P_ITEM cItem::getOutMostCont( short rec )
{
	if ( rec<0	// too many recursions
		|| (isCharSerial(getContSerial()))	//weared
		|| (isInWorld()) )	// in the world
		return this;
	P_ITEM pOut=pointers::findItemBySerial(getContSerial());	// up one level
	if (!ISVALIDPI(pOut))
	{
		char ttt[222];
		sprintf(ttt,"item <%i> has a bad contserial <%i>", getSerial32(), getContSerial());
		LogCritical(ttt);
		return this;
	}
	else
		return pOut->getOutMostCont( --rec );
}

/*!
\brief Get the owner of this pack ( automatic recurse for out most pack )
\author Endymion
\return pointer to pack owner
\note automatic recurse for get out most container
*/
P_CHAR cItem::getPackOwner()
{
	P_ITEM cont=getOutMostCont();
	if(cont->isInWorld())
		return NULL;
	else
		return pointers::findCharBySerial(cont->getContSerial());
}

/*!
\brief Get item's distance from the given char
\author Endymion
\return distance ( if invalid is returned VERY_VERY_FAR )
\param pc the char
\note it check also if is subcontainer, or weared. so np call freely
*/
UI32 cItem::distFrom( P_CHAR pc )
{
	VALIDATEPCR(pc, VERY_VERY_FAR );
	return pc->distFrom( this );
}

/*!
\brief Get item's distance from the given item
\author Endymion
\return distance ( if invalid is returned VERY_VERY_FAR )
\param pi the item
\note it check also if is subcontainer, or weared. so np call freely
*/
UI32 cItem::distFrom( P_ITEM pi )
{
	VALIDATEPIR(pi, VERY_VERY_FAR);
	P_ITEM cont=pi->getOutMostCont(); //return at least itself
	VALIDATEPIR(cont, VERY_VERY_FAR);
	P_ITEM mycont=getOutMostCont();
	VALIDATEPIR(mycont, VERY_VERY_FAR );

	if(cont->isInWorld() ) {
		if( mycont->isInWorld() )
			return (int)dist(mycont->getPosition(),cont->getPosition());
		else { //this is weared
			SERIAL outcontserial=cont->getContSerial();
			if(isCharSerial(outcontserial)) { //can be weared
				P_CHAR pc=pointers::findCharBySerial( outcontserial );
				VALIDATEPCR(pc,VERY_VERY_FAR);
				return (int)dist(pc->getPosition(),cont->getPosition());
			}
			else
				return VERY_VERY_FAR; //not world, not weared.. and another cont can't be
		}
	}
	else { //cont is weared
		SERIAL outcontserial=cont->getContSerial();
		if(isCharSerial(outcontserial)) { //can be weared
			P_CHAR pc_i=pointers::findCharBySerial( outcontserial );
			VALIDATEPCR(pc_i,VERY_VERY_FAR);
			if( mycont->isInWorld() )
				return (int)dist(pc_i->getPosition(),mycont->getPosition());
			else
				return pc_i->distFrom(mycont);
		}
		else return VERY_VERY_FAR;

	}
}

void cItem::setDecay( const LOGICAL on )
{
	if( on )
		priv |= 0x01;
	else
		priv &= ~0x01;
}

void cItem::setNewbie( const LOGICAL on )
{
	if( on )
		priv |= 0x02;
	else
		priv &= ~0x02;
}

void cItem::setDispellable( const LOGICAL on )
{
	if( on )
		priv |= 0x04;
	else
		priv &= ~0x04;
}
/*
LOGICAL cItem::isValidAmxEvent( UI32 eventId )
{
	if( eventId < ALLITEMEVENTS )
		return true;
	else
		return false;
}
*/
/*
const unsigned int NAME_ID=0xFA00;
const unsigned int TITLE_ID=0xFA01;
const unsigned int ACCOUNT_ID=0xFA02;
const unsigned int CREATIONDAY_ID=0xFA03;
const unsigned int GMMOVEEFF_ID=0xFA04;
const unsigned int GUILDTYPE=0xFA05;
const unsigned int GUILDTRAITOR_ID=0xFA06;
const unsigned int POS_X_ID=0xFA07;
const unsigned int POS_Y_ID=0xFA08;
const unsigned int POS_Z_ID=0xFA09;
const unsigned int POS_DISPZ_ID=0xFA0A;
const unsigned int POS_M_ID=0xFA0B;
const unsigned int POS_OLDX_ID=0xFA0C;
const unsigned int POS_OLDY_ID=0xFA0D;
const unsigned int POS_OLDZ_ID=0xFA0E;
const unsigned int POS_OLDM_ID=0xFA0F;
const unsigned int DIR_ID = 0xFA10;
const unsigned int DOORUSE_ID = 0xFA11;
const unsigned int BODY_ID = 0xFA12;
const unsigned int XBODY_ID = 0xFA13;
const unsigned int SKIN_ID = 0xFA14;
const unsigned int XSKIN_ID = 0xFA15;
const unsigned int PRIV_ID = 0xFA16;
const unsigned int ALLMOVE_ID = 0xFA17;
const unsigned int DAMAGETYPE_ID = 0xFA18;
const unsigned int STABLEMASTER_ID = 0xFA19;
const unsigned int NPCTYPE_ID = 0xFA1A;
const unsigned int TIME_UNUSED_ID = 0xFA1B;
const unsigned int FONT_ID = 0xFA1C;
const unsigned int SAY_ID = 0xFA1D;
const unsigned int EMOTE_ID = 0xFA1E;
const unsigned int STRENGTH_ID = 0xFA1F;
const unsigned int STRENGTH2_ID = 0xFA20;
const unsigned int DEXTERITY_ID = 0xFA21;
const unsigned int DEXTERITY2_ID = 0xFA22;
const unsigned int INTELLIGENCE_ID = 0xFA23;
const unsigned int INTELLIGENCE2_ID = 0xFA24;
const unsigned int HITPOINTS_ID = 0xFA25;
const unsigned int STAMINA_ID = 0xFA26;
const unsigned int MANA_ID = 0xFA27;
const unsigned int NPC_ID = 0xFA28;
const unsigned int POSSESSEDSERIAL_ID = 0xFA29;
const unsigned int HOLDGOLD_ID = 0xFA2A;
const unsigned int OWN_ID = 0xFA2B;
const unsigned int ROBE_ID = 0xFA2C;
const unsigned int KARMA_ID = 0xFA2D;
const unsigned int FAME_ID = 0xFA2E;
const unsigned int KILLS_ID = 0xFA2F;
const unsigned int DEATHS_ID = 0xFA30;
const unsigned int FIXEDLIGHT_ID = 0xFA31;
const unsigned int SPEECH_ID = 0xFA32;

void cItem::saveBin( ofstream *out)
{
	static cItem dummy( false );
	if( isInWorld() ) 
	{
		if ( ( pi->getPosition().x < 100) && ( pi->getPosition().y < 100 ) ) 
		{ //garbage positions
			//Luxor: we must check if position is 0,0 for dragged items
			if (pi->getPosition().x != 0 && pi->getPosition().y != 0) 
			{
				pi->Delete();
				return;
			}
		}
		if( pi->doDecay() )
			return;
	}
	// converting old house styles to new housesystem
	if ( pi->IsHouse() )
	{
		// Look if pi serial connects to a house in cHouses
		P_HOUSE house=cHouses::findHouse(pi->getSerial32());
		if ( house == NULL )
		{
			// No house information has been made yet
			P_ITEM temp = item::CreateFromScript( pi->morex, NULL);
			if ( !ISVALIDPI(temp) )
			{
				objects.eraseObject (temp);
				return;
			}
			P_HOUSE newHouse = new cHouse();
			newHouse->setSerial(pi->getSerial32());
			newHouse->createMulti(temp->morex, pi);
			newHouse->setOwner(pi->getOwnerSerial32());
			cHouses::addHouse(newHouse );
			temp->Delete();

		}
	}
	if ( (pi->type == ITYPE_NPC_SPAWNER )|| (pi->type == ITYPE_ITEM_SPAWNER ))
	{
		cSpawnDinamic *spawn = Spawns->getDynamicSpawn(pi->getSerial32());
		if ( spawn == NULL )
		{
			// No spawner has been made yet
			Spawns->loadFromItem(pi);
			spawn = Spawns->getDynamicSpawn(pi->getSerial32());
			spawn->clear();
			pi->amount2=0;
			spawn->current=0;
			spawn->nextspawn=uiCurrentTime+ (60*RandomNum( pi->morey, pi->morez)*MY_CLOCKS_PER_SEC);
		}
	}
	if ( ( !pi->isInWorld() || ((pi->getPosition("x") > 1) && (pi->getPosition("x") < 6144) && (pi->getPosition("y") < 4096))))
	{
		out->write( (char *)  "SECTION WORLDITEM %i\n", this->itm_curr++);
		out->write( (char *)  "{\n");
		out->write( (char *)  "SERIAL %i\n", pi->getSerial32());
		out->write( (char *)  "NAME %s\n", pi->getCurrentNameC());
		//<Luxor>: if the item is beard or hair of a morphed char, we must save the original ID and COLOR value
		if ( (pi->layer == LAYER_BEARD || pi->layer == LAYER_HAIR) && isCharSerial( pi->getContSerial() ) ) {
			P_CHAR pc_morphed = (P_CHAR)(pi->getContainer());
			if (ISVALIDPC(pc_morphed)) 
			{
				if (pc_morphed->morphed) 
				{
					if (pi->layer == LAYER_BEARD) 
					{ //beard
						if ( pc_morphed->getBackupStats() != NULL )
						{
							out->write( (char *)  "ID %i\n", pc_morphed->getBackupStats()->getBeardStyle());
							out->write( (char *)  "COLOR %i\n", pc_morphed->getBackupStats()->getBeardColor());

						}
					} 
					else 
					{ //hair
						if ( pc_morphed->getBackupStats() != NULL )
						{
							out->write( (char *)  "ID %i\n", pc_morphed->getBackupStats()->getHairStyle());
							out->write( (char *)  "COLOR %i\n", pc_morphed->getBackupStats()->getHairColor());

						}
					}
				} 
				else 
				{
					out->write( (char *)  "ID %i\n", pi->getId());
					if (pi->getColor()!=dummy.getColor())
						out->write( (char *)  "COLOR %i\n", pi->getColor());
				}
			} else {
				out->write( (char *)  "ID %i\n", pi->getId());
				if (pi->getColor()!=dummy.getColor())
					out->write( (char *)  "COLOR %i\n", pi->getColor());
			}
		} else {
			out->write( (char *)  "ID %i\n", pi->getId());
			if (pi->getColor()!=dummy.getColor())
				out->write( (char *)  "COLOR %i\n", pi->getColor());
		}
		//</Luxor>
		if( pi->getScriptID()!=dummy.getScriptID() )
			out->write( (char *)  "SCRIPTID %u\n", pi->getScriptID());
		if ((pi->animid()!=pi->getId() )&&(pi->animid()!=dummy.animid()))
			out->write( (char *)  "ANIMID %i\n", pi->animid());
		out->write( (char *)  "NAME2 %s\n", pi->getSecondaryNameC());
#ifndef DESTROY_REFERENCES
		if ( !pi->creator.empty())	out->write( (char *)  "CREATOR %s\n", pi->creator.c_str() ); // by Magius(CHE)
#endif
		if (pi->madewith!=dummy.madewith)
			out->write( (char *)  "SK_MADE %i\n", pi->madewith ); // by Magius(CHE)

		out->write( (char *)  "X %i\n", pi->getPosition().x);
		out->write( (char *)  "Y %i\n", pi->getPosition().y);
		out->write( (char *)  "Z %i\n", pi->getPosition().z);

		if (pi->getContSerial()!=dummy.getContSerial())
			out->write( (char *)  "CONT %i\n", pi->getContSerial());
		if (pi->layer!=dummy.layer)
			out->write( (char *)  "LAYER %i\n", pi->layer);
		if (pi->itmhand!=dummy.itmhand)
			out->write( (char *)  "ITEMHAND %i\n", pi->itmhand);
		if (pi->type!=dummy.type)
			out->write( (char *)  "TYPE %i\n", pi->type);
		//xan : don't save type2 for boats, so they'll restart STOPPED
		// elcabesa se non lo salvi per le barche poi non le riapri =)
		if (pi->type2!=dummy.type2)
		{
			if ( !( (pi->type==117) && ((pi->type2==1) ||(pi->type2==2)) ) )
			{
				out->write( (char *)  "TYPE2 %i\n", pi->type2);
			}
		}
		if (pi->offspell!=dummy.offspell)
			out->write( (char *)  "OFFSPELL %i\n", pi->offspell);
		if (((unsigned char)pi->more1<<24)+((unsigned char)pi->more2<<16)+((unsigned char)pi->more3<<8)+(unsigned char)pi->more4) //;
			out->write( (char *)  "MORE %i\n", ((unsigned char)pi->more1<<24)+((unsigned char)pi->more2<<16)+((unsigned char)pi->more3<<8)+(unsigned char)pi->more4);
		if (((unsigned char)pi->moreb1<<24)+((unsigned char)pi->moreb2<<16)+((unsigned char)pi->moreb3<<8)+(unsigned char)pi->moreb4)
			out->write( (char *)  "MORE2 %i\n", ((unsigned char)pi->moreb1<<24)+((unsigned char)pi->moreb2<<16)+((unsigned char)pi->moreb3<<8)+(unsigned char)pi->moreb4);
		if (pi->morex!=dummy.morex)
			out->write( (char *)  "MOREX %i\n", pi->morex);
		if (pi->morey!=dummy.morey)
			out->write( (char *)  "MOREY %i\n", pi->morey);
		if (pi->morez!=dummy.morez)
			out->write( (char *)  "MOREZ %i\n", pi->morez);
		if (pi->amount!=dummy.amount)
			out->write( (char *)  "AMOUNT %i\n", pi->amount);
		if (pi->amount2!=dummy.amount)
			out->write( (char *)  "AMOUNT2 %i\n", pi->amount2);
		if (pi->pileable!=dummy.pileable)
			out->write( (char *)  "PILEABLE %i\n", pi->pileable);
		if (pi->doordir!=dummy.doordir)
			out->write( (char *)  "DOORFLAG %i\n", pi->doordir);
		if (pi->dye!=dummy.dye)
			out->write( (char *)  "DYEABLE %i\n", pi->dye);
		if (pi->corpse!=dummy.corpse)
			out->write( (char *)  "CORPSE %i\n", pi->corpse);
		if (pi->att!=dummy.att)
			out->write( (char *)  "ATT %i\n", pi->att);
		if (pi->def!=dummy.def)
			out->write( (char *)  "DEF %i\n", pi->def);
		if (pi->hidamage!=dummy.hidamage)
			out->write( (char *)  "HIDAMAGE %i\n", pi->hidamage);
		if (pi->lodamage!=dummy.lodamage)
			out->write( (char *)  "LODAMAGE %i\n", pi->lodamage);
		if (pi->auxdamage!=dummy.auxdamage)
			out->write( (char *)  "AUXDAMAGE %i\n", pi->auxdamage);
		if (pi->damagetype!=dummy.damagetype)
			out->write( (char *)  "DAMAGETYPE %i\n", pi->damagetype);
		if (pi->auxdamagetype!=dummy.auxdamagetype)
			out->write( (char *)  "AUXDAMAGETYPE %i\n", pi->auxdamagetype);
		if (pi->ammo !=dummy.ammo )
			out->write( (char *)  "AMMO %i\n", pi->ammo);
		if (pi->ammoFx !=dummy.ammoFx )
			out->write( (char *)  "AMMOFX %i\n", pi->ammoFx);
		if (pi->st!=dummy.st)
			out->write( (char *)  "ST %i\n", pi->st);
		if (pi->time_unused!=dummy.time_unused)
			out->write( (char *)  "TIME_UNUSED %i\n", pi->time_unused);
		if (pi->weight!=dummy.weight)
			out->write( (char *)  "WEIGHT %i\n", pi->weight);
		if (pi->hp!=dummy.hp)
			out->write( (char *)  "HP %i\n", pi->hp);
		if (pi->maxhp!=dummy.maxhp)
			out->write( (char *)  "MAXHP %i\n", pi->maxhp ); // Magius(CHE)
		if (pi->rank!=dummy.rank)
			out->write( (char *)  "RANK %i\n", pi->rank ); // Magius(CHE)
		if (pi->st2!=dummy.st2)
			out->write( (char *)  "ST2 %i\n", pi->st2);
		if (pi->dx!=dummy.dx)
			out->write( (char *)  "DX %i\n", pi->dx);
		if (pi->dx2!=dummy.dx2)
			out->write( (char *)  "DX2 %i\n", pi->dx2);
		if (pi->in!=dummy.in)
			out->write( (char *)  "IN %i\n", pi->in);
		if (pi->in2!=dummy.in2)
			out->write( (char *)  "IN2 %i\n", pi->in2);
		if (pi->spd!=dummy.spd)
			out->write( (char *)  "SPD %i\n", pi->spd);
		if (pi->poisoned!=dummy.poisoned)
			out->write( (char *)  "POISONED %i\n", pi->poisoned);
		if (pi->wipe!=dummy.wipe)
			out->write( (char *)  "WIPE %i\n", pi->wipe);
		if (pi->magic!=dummy.magic)
			out->write( (char *)  "MOVABLE %i\n", pi->magic);
		if (pi->getOwnerSerial32()!=dummy.getOwnerSerial32())
			out->write( (char *)  "OWNER %i\n", pi->getOwnerSerial32());
		if (pi->visible!=dummy.visible)
			out->write( (char *)  "VISIBLE %i\n", pi->visible);
		if (pi->dir!=dummy.dir)
			out->write( (char *)  "DIR %i\n", pi->dir);
		if (pi->priv!=dummy.priv)
			out->write( (char *)  "PRIV %i\n", pi->priv);
		if (pi->value!=dummy.value)
			out->write( (char *)  "VALUE %i\n", pi->value);
		if (pi->restock!=dummy.restock)
			out->write( (char *)  "RESTOCK %i\n", pi->restock);
		if (pi->trigger!=dummy.trigger)
			out->write( (char *)  "TRIGGER %i\n", pi->trigger);
		if (pi->trigtype!=dummy.trigtype)
			out->write( (char *)  "TRIGTYPE %i\n", pi->trigtype);
		if (pi->disabled!=dummy.disabled)
			out->write( (char *)  "DISABLED %i\n", pi->disabled);
		if (pi->disabledmsg!=NULL)
			out->write( (char *)  "DISABLEMSG %s\n", pi->disabledmsg->c_str() );
		if (pi->tuses!=dummy.tuses)
			out->write( (char *)  "USES %i\n", pi->tuses);
		if (pi->good!=dummy.good )
			out->write( (char *)  "GOOD %i\n", pi->good); // Magius(CHE)
		if (pi->secureIt!=dummy.secureIt)
			out->write( (char *)  "SECUREIT %i\n", pi->secureIt);
		if (pi->smelt!=dummy.smelt)
			out->write( (char *)  "SMELT %i\n", pi->smelt);
		if (pi->itemSoundEffect!=dummy.itemSoundEffect)
			out->write( (char *)  "SOUNDFX %i\n", pi->itemSoundEffect);
		// Spawns
		if (pi->getSpawnSerial() != dummy.getSpawnSerial())
			out->write( (char *) "SPAWNSERIAL %i\n", pi->getSpawnSerial());
		if (pi->getSpawnRegion() != dummy.getSpawnRegion())
			out->write( (char *) "SPAWNREGION %i\n", pi->getSpawnRegion());
		if (!pi->vendorDescription.empty())
			out->write( (char *)  "DESC %s\n", pi->vendorDescription.c_str() );
		if (pi->hasTempfx())
		{
			TempfxVector *itemTempfxVec = pi->getTempfxVec( );
			TempfxVector::iterator iter=itemTempfxVec->begin();
			for ( ; iter != itemTempfxVec->end(); iter++)
			{
				tempfx::cTempfx fx = *iter;
				fx.save(iWsc);
			}
		}
#define SAVEITEMEVENT(A,B) { if (pi->amxevents[B]) if (pi->amxevents[B]->shouldBeSaved()) out->write( (char *)  "%s %s\n", A, pi->amxevents[B]->getFuncName()); }
//#define SAVEITEMEVENT(A,B) { AmxEvent *event = pi->getAmxEvent( B ); if ( event ) if ( event->shouldBeSaved() ) out->write( (char *)  "%s %s\n", A, event->getFuncName()); }
		SAVEITEMEVENT("@ONSTART", EVENT_IONSTART);
		SAVEITEMEVENT("@ONCHECKCANUSE", EVENT_IONCHECKCANUSE);
		SAVEITEMEVENT("@ONCLICK", EVENT_IONCLICK);
		SAVEITEMEVENT("@ONDAMAGE", EVENT_IONDAMAGE);
		SAVEITEMEVENT("@ONDBLCLICK", EVENT_IONDBLCLICK);
		SAVEITEMEVENT("@ONDECAY", EVENT_IONDECAY);
		SAVEITEMEVENT("@ONDROPINLAND", EVENT_IDROPINLAND);
		SAVEITEMEVENT("@ONDROPONCHAR", EVENT_IDROPONCHAR);
		SAVEITEMEVENT("@ONEQUIP", EVENT_IONEQUIP);
		SAVEITEMEVENT("@ONLOCKPICK", EVENT_IONLOCKPICK);
		SAVEITEMEVENT("@ONPOISONED", EVENT_IONPOISONED);
		SAVEITEMEVENT("@ONPUTINBACKPACK", EVENT_IPUTINBACKPACK);
		SAVEITEMEVENT("@ONREMOVETRAP", EVENT_IONREMOVETRAP);
		SAVEITEMEVENT("@ONSTOLEN", EVENT_IONSTOLEN);
		SAVEITEMEVENT("@ONTRANSFER", EVENT_IONTRANSFER);
		SAVEITEMEVENT("@ONUNEQUIP", EVENT_IONUNEQUIP);
		SAVEITEMEVENT("@ONWALKOVER", EVENT_IONWALKOVER);
		SAVEITEMEVENT("@ONPUTITEM", EVENT_IONPUTITEM);
		SAVEITEMEVENT("@ONTAKEFROMCONTAINER", EVENT_ITAKEFROMCONTAINER);
		//
		// SAVE NEW AMX VARS
		//
		amxVS.saveVariable( pi->getSerial32(), iWsc );
		out->write( (char *)  "}\n\n");
	}
}
*/

