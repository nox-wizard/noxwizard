  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "network.h"
#include "sndpkg.h"
#include "itemid.h"
#include "debug.h"
#include "amx/amxcback.h"
#include "set.h"
#include "layer.h"
#include "trade.h"
#include "commands.h"
#include "weight.h"
#include "chars.h"
#include "items.h"
#include "inlines.h"
#include "nox-wizard.h"
#include "utils.h"


void buyaction(int s)
{
	if ( s < 0 || s >= now )
		return;

	char clearmsg[8];
	int i, j;
	std::vector< buyeditem > allitemsbuyed;
	//int buyit[512];
	//int amount[512];
	//int layer[512];
	int playergoldtotal;

	int tmpvalue=0; // Fixed for adv trade system -- Magius(CHE) §
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	P_ITEM pack = pc->getBackpack();
	VALIDATEPI(pack);

	P_CHAR npc = pointers::findCharBySerPtr(&buffer[s][3]);
	VALIDATEPC( npc );

	int itemtotal=(((256*(buffer[s][1]))+buffer[s][2])-8)/7;
	if (itemtotal>256)
		return; //LB

	int clear=0;
	int goldtotal=0;
	int soldout=0;

	//P_ITEM buyed=NULL;

	for(i=0;i<itemtotal;i++)
	{
		int pos=8+(7*i);

		buyeditem b;

		b.layer=buffer[s][pos];
		b.item=pointers::findItemBySerPtr(&buffer[s][pos+1]);
		if(!ISVALIDPI(b.item))
			continue;

		b.amount=DBYTE2WORD(buffer[s][pos+5],buffer[s][pos+6]);

		b.item->rank=10;
		// Fixed for adv trade system -- Magius(CHE) §
		tmpvalue=b.item->value;
		tmpvalue=calcValue(DEREF_P_ITEM(b.item), tmpvalue);
		if (SrvParms->trade_system==1)
			tmpvalue=calcGoodValue(s,DEREF_P_ITEM(b.item),tmpvalue,0);
		goldtotal+=b.amount*tmpvalue;
		// End Fix for adv trade system -- Magius(CHE) §

		allitemsbuyed.push_back( b );

	}

	bool useBank;
	useBank = (goldtotal >= SrvParms->CheckBank );

	if( useBank )
		playergoldtotal = pc->countBankGold();//GetBankCount(pc_currchar, 0x0EED );
	else
		playergoldtotal = pc->CountGold();

	if ((playergoldtotal<goldtotal)&&(!pc->IsGM()))
	{
		npc->talkAll( TRANSLATE("Alas, thou dost not possess sufficient gold for this purchase!"),0);
	}
	else {

		std::vector<buyeditem>::iterator iter( allitemsbuyed.begin()), end( allitemsbuyed.end() );
		for (; iter!=end; iter++)
		{
			if ((iter->item)->amount<iter->amount)
			{
				soldout=1;
			}
		}
		if (soldout)
		{
			npc->talk(s, TRANSLATE("Alas, I no longer have all those goods in stock. Let me know if there is something else thou wouldst buy."),0);
			clear=1;
		}
		else
		{
			if (pc->IsGM())
			{
				sprintf(temp, TRANSLATE("Here you are, %s. Someone as special as thee will receive my wares for free of course."), pc->getCurrentNameC());
			}
			else
			{
				if(useBank)
				{
					sprintf(temp, TRANSLATE("Here you are, %s. %d gold coin%s will be deducted from your bank account.  I thank thee for thy business."),
					pc->getCurrentNameC(), goldtotal, (goldtotal==1) ? "" : "s");
				}
			    else
				{
				    sprintf(temp, TRANSLATE("Here you are, %s.  That will be %d gold coin%s.  I thank thee for thy business."),
					pc->getCurrentNameC(), goldtotal, (goldtotal==1) ? "" : "s");
				}
			    pc->playSFX( goldsfx(goldtotal) );
			}
			npc->talkAll(temp,0);
			npc->playAction(0x20);	// bow (Duke, 17/03/2001)

			clear=1;
			if( !(pc->IsGM() ) )
			{
				if( useBank )
				{
					P_ITEM bank= pc->GetBankBox();
					bank->DeleteAmount(goldtotal, ITEMID_GOLD, 0);
				}
				else
				{
					pack->DeleteAmount( goldtotal, ITEMID_GOLD);
				}
			}

			std::vector<buyeditem>::iterator iter( allitemsbuyed.begin()), end( allitemsbuyed.end() );
			for (; iter!=end; iter++) {
				if ((iter->item)->amount>iter->amount)
				{
					if ((iter->item)->pileable)
					{
						item::CreateFromScript(  iter->item->getScriptID(), pack, iter->amount );
					}
					else
					{
						for (j=0;j<iter->amount;j++)
							item::CreateFromScript( iter->item->getScriptID(), pack );
					}
					(iter->item)->amount-=iter->amount;
					(iter->item)->restock+=iter->amount;
				}
				else
				{
					switch(iter->layer)
					{
						case LAYER_TRADE_RESTOCK:
							if ((iter->item)->pileable)
							{

								item::CreateFromScript( iter->item->getScriptID(), pack, iter->amount );

							}
							else
							{
								for (j=0;j<iter->amount;j++)
									item::CreateFromScript( iter->item->getScriptID(), pack );

							}
							(iter->item)->amount-=iter->amount;
							(iter->item)->restock+=iter->amount;
							break;
						case LAYER_TRADE_NORESTOCK:
							if ((iter->item)->pileable)
								(iter->item)->setContSerial( pack->getSerial32() );
							else
							{
								for (j=0;j<iter->amount;j++)
									item::CreateFromScript( iter->item->getScriptID(), pack );

								(iter->item)->setContSerial( pack->getSerial32() );
								(iter->item)->amount=1;
							}
							(iter->item)->Refresh();
							break;
						default:
							ErrOut("Switch fallout. trade.cpp, buyaction()\n"); //Morrolan
					}
				}
			}
		}
	}

	if (clear)
	{
		clearmsg[0]=0x3B;
		clearmsg[1]=0x00;
		clearmsg[2]=0x08;
		clearmsg[3]=buffer[s][3];
		clearmsg[4]=buffer[s][4];
		clearmsg[5]=buffer[s][5];
		clearmsg[6]=buffer[s][6];
		clearmsg[7]=0x00;
		Xsend(s, clearmsg, 8);
	}
	weights::NewCalc(pc);	// Ison 2-20-99
	statwindow(pc,pc);

}


// this is a q&d fix for 'sell price higher than buy price' bug (Duke, 30.3.2001)
static bool items_match(P_ITEM pi1,P_ITEM pi2)
{
	VALIDATEPIR(pi1,false);
	VALIDATEPIR(pi2,false);

	if (pi1->getId()==pi2->getId() &&
		pi1->type==pi2->type &&
		!(pi1->getId()==0x14F0 && (pi1->morex!=pi2->morex)) &&	// house deeds only differ by morex
		!(pi1->IsShield() && strcmp(pi1->getSecondaryNameC(),pi2->getSecondaryNameC())) &&	// magic shields only differ by secondary name
		pi1->getColor()==pi2->getColor())		// this enables color checking for armour, but disables dyed clothes :)
		return true;
	return false;

}

void sellaction(NXWSOCKET s)
{
	if ( s < 0 || s >= now )
		return;

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	P_ITEM npa=NULL, npb=NULL, npc=NULL;
	int i, amt, value=0, totgold=0;

	if (buffer[s][8]!=0)
	{
		P_CHAR pc_n=pointers::findCharBySerPtr(buffer[s]+3);
		VALIDATEPC(pc_n);

		NxwItemWrapper si;
		si.fillItemWeared( pc_n, true, true, false );
		for( si.rewind(); !si.isEmpty(); si++ )
		{
			P_ITEM pi=si.getItem();
			if(ISVALIDPI(pi)) {
				if (pi->layer == LAYER_TRADE_RESTOCK) npa=pi;	// Buy Restock container
				if (pi->layer == LAYER_TRADE_NORESTOCK) npb=pi;	// Buy no restock container
				if (pi->layer == LAYER_TRADE_BOUGHT) npc=pi;	// Sell container
			}
		}

		// Pre Calculate Total Amount of selling items to STOPS if the items if greater than SELLMAXITEM - Magius(CHE)

		P_ITEM join=NULL;
		UI32 maxsell=0;
		i=buffer[s][8];
		if (i>256) return;
		for (i=0;i<buffer[s][8];i++)
		{
			amt=ShortFromCharPtr(buffer[s]+9+(6*i)+4);
			maxsell+=amt;
		}
		if (maxsell>SrvParms->sellmaxitem)
		{
			char tmpmsg[256];
			sprintf(tmpmsg,TRANSLATE("Sorry %s but i can buy only %i items at time!"), pc->getCurrentNameC(), SrvParms->sellmaxitem);
			pc_n->talkAll(tmpmsg,0);
			return;
		}

		for (i=0;i<buffer[s][8];i++)
		{
			P_ITEM pSell=pointers::findItemBySerPtr(buffer[s]+9+(6*i));	// the item to sell
			if (!ISVALIDPI(pSell))
				continue;
			amt=ShortFromCharPtr(buffer[s]+9+(6*i)+4);

			// player may have taken items out of his bp while the sell menu was up ;-)
			if (pSell->amount<amt)
			{
				pc_n->talkAll(TRANSLATE("Cheating scum! Leave now, before I call the guards!"),0);
				return;
			}

			// Search the buy restock Container
			if( ISVALIDPI(npa) ) {
				NxwItemWrapper si2;
				si2.fillItemsInContainer( npa, false );
				for( si2.rewind(); !si2.isEmpty(); si2++ )
				{
					P_ITEM pi=si2.getItem();
					if( ISVALIDPI(pi) && items_match(pi,pSell))
						join=pi;
				}
			}

			// Search the sell Container to determine the price
			if( ISVALIDPI(npc) ) {
				NxwItemWrapper si2;
				si2.fillItemsInContainer( npc, false );
				for( si2.rewind(); !si2.isEmpty(); si2++ )
				{
					P_ITEM pi=si2.getItem();
					if( ISVALIDPI(pi) && items_match(pi,pSell))
					{
						value=pi->value;
						value=calcValue(DEREF_P_ITEM(pSell), value);
						if (SrvParms->trade_system==1)
							value=calcGoodValue(s,DEREF_P_ITEM(pSell),value,1); // Fixed for adv trade --- by Magius(CHE) §
						break;	// let's take the first match
					}
				}
			}
			totgold+=(amt*value);	// add to the bill

			if (join!=NULL)	// The item goes to the container with restockable items
			{


				join->amount+=amt;
				join->restock-=amt;
				pSell->ReduceAmount(amt);
			}
			else
			{
				if(ISVALIDPI(npb)) {
					NxwSocketWrapper sw;
					sw.fillOnline( pSell );

					for( sw.rewind(); !sw.isEmpty(); sw++ )
					{
						SendDeleteObjectPkt( sw.getSocket(), pSell->getSerial32() );
					}

					pSell->setContSerial( npb->getSerial32() );
					if (pSell->amount!=amt)
						Commands::DupeItem(s, DEREF_P_ITEM(pSell), pSell->amount-amt);
				}
			}
		}
		addgold(pc->getSocket(), totgold);
		pc->playSFX( goldsfx(totgold) );
	}

	char clearmsg[9];
	clearmsg[0]=0x3B;
	clearmsg[1]=0x00;
	clearmsg[2]=0x08;
	clearmsg[3]=buffer[s][3];
	clearmsg[4]=buffer[s][4];
	clearmsg[5]=buffer[s][5];
	clearmsg[6]=buffer[s][6];
	clearmsg[7]=0x00;
	Xsend(s, clearmsg, 8);

}

///////////////////////////////////////////////////////////////////
// Function name     : tradestart
// Return type       : P_ITEM
// Author            : Luxor
P_ITEM tradestart(P_CHAR pc1, P_CHAR pc2)
{
        VALIDATEPCR(pc1, NULL);
        VALIDATEPCR(pc2, NULL);
        if( pc1->getClient() == NULL || pc2->getClient() == NULL) return NULL;
		if( pc1->dead || pc2->dead ) return NULL;
		if( pc1->distFrom( pc2 )>5 ) return NULL;
        SI32 s1 = pc1->getClient()->toInt();
        SI32 s2 = pc2->getClient()->toInt();
        P_ITEM bp1 = pc1->getBackpack();
        if (!ISVALIDPI(bp1)) {
                pc1->sysmsg(TRANSLATE("Time to buy a backpack!"));
                return NULL;
        }
        P_ITEM bp2 = pc2->getBackpack();
        if (!ISVALIDPI(bp2)) {
                pc2->sysmsg(TRANSLATE("Time to buy a backpack!"));
                return NULL;
        }

        P_ITEM cont1 = item::CreateFromScript( "$item_a_bulletin_board" );
		P_ITEM cont2 = item::CreateFromScript( "$item_a_bulletin_board" );
        VALIDATEPIR(cont1, NULL);
        VALIDATEPIR(cont2, NULL);

        cont1->setPosition(26, 0, 0);
        cont2->setPosition(26, 0, 0);
        cont1->setContSerial(pc1->getSerial32());
        cont2->setContSerial(pc2->getSerial32());
        cont1->layer = cont2->layer = 0;
	cont1->type = cont2->type = 1;
	cont1->dye = cont2->dye = 0;
        sendbpitem(s1, cont1);
        sendbpitem(s2, cont1);
        sendbpitem(s1, cont2);
        sendbpitem(s2, cont2);

        cont2->moreb1= cont1->getSerial().ser1;
	cont2->moreb2= cont1->getSerial().ser2;
	cont2->moreb3= cont1->getSerial().ser3;
	cont2->moreb4= cont1->getSerial().ser4;
	cont1->moreb1= cont2->getSerial().ser1;
	cont1->moreb2= cont2->getSerial().ser2;
	cont1->moreb3= cont2->getSerial().ser3;
	cont1->moreb4= cont2->getSerial().ser4;
	cont2->morez=0;
	cont1->morez=0;

        UI08 msg[90];
        msg[0]=0x6F;    //Header Byte
	msg[1]=0;       //Size
	msg[2]=47;      //Size
	msg[3]=0;       //Initiate
        LongToCharPtr(pc2->getSerial32(), msg+4);
	LongToCharPtr(cont1->getSerial32(), msg+8);
	LongToCharPtr(cont2->getSerial32(), msg+12);
	msg[16]=1;
	strcpy((char*)&(msg[17]), pc2->getCurrentNameC());
	Xsend(s1, msg, 47);

        UI08 msg2[90];
        msg2[0]=0x6F;   //Header Byte
	msg2[1]=0;      //Size
	msg2[2]=47;     //Size
	msg2[3]=0;      //Initiate
	LongToCharPtr(pc1->getSerial32(),msg2+4);
	LongToCharPtr(cont2->getSerial32(), msg2+8);
	LongToCharPtr(cont1->getSerial32(), msg2+12);
	msg2[16]=1;
	strcpy((char*)&(msg2[17]), pc1->getCurrentNameC());
	Xsend(s2, msg2, 47);

        return cont1;
}


void clearalltrades()
{
/*        P_ITEM pi = NULL;
        P_ITEM pj = NULL;
        P_CHAR pc = NULL;
        P_ITEM pack = NULL;
        UI32 i = 0;
        for (i = 0; i < itemcount; i++) {
                pi = MAKE_ITEM_REF(i);
                if (!ISVALIDPI(pi)) continue;
                if ((pi->type==1) && (pi->getPosition("x")==26) && (pi->getPosition("y")==0) && (pi->getPosition("z")==0) &&
			(pi->id()==0x1E5E))
		{
                        pc = pointers::findCharBySerial(pi->getContSerial());
                        if (ISVALIDPC(pc)) {
                                pack = pc->getBackpack();
                                if (ISVALIDPI(pack)) {
                                    NxwItemWrapper si;
									si.fillItemsInContainer( pi );
									for( si.rewind(); !si.isEmpty(); si++ ) {
										pj = si.getItem();
                                        if( ISVALIDPI(pj)) {
                       						pj->setContSerial(pack->getSerial32());
                        				}
									}
                                    pi->deleteItem();
        							InfoOut("trade cleared\n");
                                }
                        }
                }
        }*/
}


void trademsg(int s)
{
	P_ITEM cont1, cont2;

	switch(buffer[s][3])
	{
	case 0://Start trade - Never happens, sent out by the server only.
		break;
	case 2://Change check marks. Possibly conclude trade
		cont1 = pointers::findItemBySerPtr(buffer[s] +4);

		if (cont1) cont2 = pointers::findItemBySerial(calcserial(cont1->moreb1, cont1->moreb2, cont1->moreb3, cont1->moreb4));
		else cont2=NULL;

		if (cont2)
		{
			cont1->morez=buffer[s][11];
			sendtradestatus(cont1, cont2);
			if (cont1->morez && cont2->morez)
			{
				dotrade(cont1, cont2);
				endtrade( LongFromCharPtr(buffer[s] +4) );
			}
		}
		break;
	case 1://Cancel trade. Send each person cancel messages, move items.
		endtrade( LongFromCharPtr(buffer[s] +4) );
		break;
	default:
		ErrOut("Switch fallout. trade.cpp, trademsg()\n"); //Morrolan
	}
}

void dotrade(P_ITEM cont1, P_ITEM cont2)
{
        VALIDATEPI(cont1);
        VALIDATEPI(cont2);
        P_CHAR pc1 = pointers::findCharBySerial(cont1->getContSerial());
        P_CHAR pc2 = pointers::findCharBySerial(cont2->getContSerial());
        VALIDATEPC(pc1);
        VALIDATEPC(pc2);
        P_ITEM bp1 = pc1->getBackpack();
        P_ITEM bp2 = pc2->getBackpack();
        VALIDATEPI(bp1);
        VALIDATEPI(bp2);
        SI32 s1, s2;
        if (pc1->getClient() == NULL || pc2->getClient() == NULL) return;
        s1 = pc1->getClient()->toInt();
        s2 = pc2->getClient()->toInt();

        if (cont1->morez == 0 || cont2->morez == 0) {
                //If the trade is not accepted, then give items back to original owners
                P_CHAR pc_dummy = NULL;
                pc_dummy = pc1;
                pc1 = pc2;
                pc2 = pc_dummy;
        }

        //Player1 items go to player2

	NxwItemWrapper si;
	si.fillItemsInContainer( cont1, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi = si.getItem();
		if( ISVALIDPI(pi))
		{

			if (pi->amxevents[EVENT_IONTRANSFER]!=NULL) {
				g_bByPass = false;
				pi->amxevents[EVENT_IONTRANSFER]->Call(pi->getSerial32(), pc1->getSerial32(), pc2->getSerial32());
				if (g_bByPass==true) continue; //skip item, I hope
			}
			/*
			g_bByPass = false;
			pi->runAmxEvent( EVENT_IONTRANSFER, pi->getSerial32(), s1, s2 );
			if (g_bByPass==true)
				continue; //skip item, I hope
			*/
			pi->setCont( bp2 );
			pi->setPosition( 50+(rand()%80), 50+(rand()%80), 9);
			sendbpitem(s2, pi);
			pi->Refresh();
		}
	}


	si.clear();
	si.fillItemsInContainer( cont2, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi = si.getItem();
		if( ISVALIDPI(pi))
		{

			if (pi->amxevents[EVENT_IONTRANSFER]!=NULL) {
        		g_bByPass = false;
        		pi->amxevents[EVENT_IONTRANSFER]->Call(pi->getSerial32(), pc2->getSerial32(), pc1->getSerial32());
        		if (g_bByPass==true) continue; //skip item, I hope
			}
			/*
			g_bByPass = false;
			pi->runAmxEvent( EVENT_IONTRANSFER, pi->getSerial32(), s2, s1 );
			if (g_bByPass==true)
				continue; //skip item, I hope
			*/

			pi->setCont( bp1 );
			pi->setPosition( 50+(rand()%80), 50+(rand()%80), 9);
			sendbpitem(s1, pi);
			pi->Refresh();
		}
	}
}

/*
void restock(bool total)
{
        //Luxor: new cAllObjects system -> this should be changed soon... too slow!!

	cAllObjectsIter objs;
	P_ITEM pi;
	for( objs.rewind(); !objs.IsEmpty(); objs++ )
	{
		if( isCharSerial( objs.getSerial() ) )
			continue;

		pi = (P_ITEM)(objs.getObject());

		if(!ISVALIDPI(pi) || pi->layer!=0x1A )
			continue;

		NxwItemWrapper si;
		si.fillItemsInContainer( pi, false ); //ndEndy We don't need subcontainer right?
		for( si.rewind(); !si.isEmpty(); si++ ) {
			P_ITEM pj=si.getItem();
			if( !ISVALIDPI(pj) || !pj->restock )
				continue;

			if (total==true)
			{
				pj->amount+=pj->restock;
				pj->restock=0;
			}
			else
			{
				if (pj->restock>0)
				{
					int a=qmin(pj->restock, (pj->restock/2)+1);
					pj->amount+=a;
					pj->restock-=a;
				}
			}
		}
		//ndEndy i dont know why is here and what do..
		//if (SrvParms->trade_system==1) StoreItemRandomValue(pi,-1);// Magius(CHE) (2)
	}
}
*/








cRestockMng::cRestockMng()
{
	timer=uiCurrentTime;
}

void cRestockMng::doRestock()
{


	if( !TIMEOUT( timer ) )
		return;

	if( needrestock.empty() ) {
		rewindList();
	}

	int count= RESTOCK_PER_TIME;

	while( ( --count>0 ) && ( !needrestock.empty() ) ) {

		P_ITEM pi= pointers::findItemBySerial( needrestock.front() );
		this->needrestock.pop();
		if( ISVALIDPI(pi) && pi->layer==LAYER_TRADE_RESTOCK  ) {

			NxwItemWrapper si;
			si.fillItemsInContainer( pi, false ); //ndEndy We don't need subcontainer right?
			for( si.rewind(); !si.isEmpty(); si++ ) {

				P_ITEM pj=si.getItem();
				if( !ISVALIDPI(pj) || ( pj->restock <= 0 ) )
					continue;

				int a=qmin(pj->restock, (pj->restock/2)+1);
				pj->amount+=a;
				pj->restock-=a;

			}

			restocked.push( pi->getSerial32() );

		}

	}

	updateTimer();


}

void cRestockMng::doRestockAll()
{

	rewindList();

	while( !needrestock.empty() ) {

		P_ITEM pi= pointers::findItemBySerial( needrestock.front() );
		this->needrestock.pop();
		if( ISVALIDPI(pi) && pi->layer==LAYER_TRADE_RESTOCK ) {

			NxwItemWrapper si;
			si.fillItemsInContainer( pi, false ); //ndEndy We don't need subcontainer right?
			for( si.rewind(); !si.isEmpty(); si++ ) {

				P_ITEM pj=si.getItem();
				if( !ISVALIDPI(pj) || ( pj->restock <= 0 ) )
					continue;

				pj->amount+=pj->restock;
				pj->restock=0;

			}

			restocked.push( pi->getSerial32() );

		}

	}

	updateTimer();

}

void cRestockMng::addNewRestock( P_ITEM pi )
{
	VALIDATEPI(pi);
	this->needrestock.push( pi->getSerial32() );
}

void cRestockMng::rewindList()
{
	while( !restocked.empty() ) {
		needrestock.push( restocked.front() );
		restocked.pop();
	}
}

void cRestockMng::updateTimer()
{
	if( needrestock.empty() ) //end restock.. next after much time
		timer=uiCurrentTime+ServerScp::g_nRestockTimeRate*60*MY_CLOCKS_PER_SEC;
	else
		timer=uiCurrentTime+CHECK_RESTOCK_EVERY*MY_CLOCKS_PER_SEC;
}



cRestockMng* Restocks = NULL;














