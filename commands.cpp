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
#include "debug.h"
#include "cmdtable.h"
#include "sregions.h"
#include "sndpkg.h"
#include "commands.h"
#include "layer.h"
#include "addmenu.h"
#include "data.h"
#include "spawn.h"
#include "set.h"
#include "archive.h"
#include "item.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"


namespace Commands
{
	SI32 cmd_offset;

	void MakeShop(int c)
	{
		P_CHAR pc = MAKE_CHAR_REF( c );
		VALIDATEPC( pc );

		pc->shopkeeper = true;

		P_ITEM pi;
		if (pc->GetItemOnLayer(LAYER_TRADE_RESTOCK) == NULL)
		{
			pi = item::CreateFromScript( "$item_restock", pc );
			if( ISVALIDPI( pi ) )//AntiChrist - to preview crashes
			{
				pi->setContSerial(pc->getSerial32());
				pi->layer=LAYER_TRADE_RESTOCK;
				pi->type=ITYPE_CONTAINER;
				pi->setNewbie();
			}
		}

		if (pc->GetItemOnLayer(LAYER_TRADE_NORESTOCK) == NULL)
		{
			pi = item::CreateFromScript( "$item_restock", pc );
			if( ISVALIDPI( pi ) )
			{
				pi->setContSerial(pc->getSerial32());
				pi->layer=LAYER_TRADE_NORESTOCK;
				pi->type=ITYPE_CONTAINER;
				pi->setNewbie();
			}
		}

		if (pc->GetItemOnLayer(LAYER_TRADE_BOUGHT) == NULL)
		{
			pi = item::CreateFromScript( "$item_restock", pc );
			if( ISVALIDPI( pi ) )
			{
				pi->setContSerial(pc->getSerial32());
				pi->layer=LAYER_TRADE_BOUGHT;
				pi->type=ITYPE_CONTAINER;
				pi->setNewbie();
			}
		}
	}

	void NextCall(NXWSOCKET s, int type)
	{
	// Type is the same as it is in showgmqueue()

	int i;
	int x=0;

		P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);

		if(pc_currchar->callnum!=0)
		{
			donewithcall(s, type);
		}

		if(type==1) //Player is a GM
		{
			for(i=1;i<MAXPAGES;i++)
			{
				if(gmpages[i].handled==0)
				{
					P_CHAR pj=pointers::findCharBySerial( gmpages[i].serial.serial32 );
					if(ISVALIDPC(pj))
					{
						Location dest= pj->getPosition();

						pc_currchar->sysmsg("");
						pc_currchar->sysmsg("Transporting to next call: %s", gmpages[i].name);
						pc_currchar->sysmsg("Problem: %s.", gmpages[i].reason);
						pc_currchar->sysmsg("Serial number: %x", gmpages[i].serial.serial32);
						pc_currchar->sysmsg("Paged at %s.", gmpages[i].timeofcall);
						gmpages[i].handled=1;
						pc_currchar->MoveTo( dest );
						pc_currchar->callnum=i;
						pc_currchar->teleport();
						x++;
					}// if
					if(x>0)break;
				}// if
			}// for
			if(x==0) pc_currchar->sysmsg("The GM queue is currently empty");
		} //end first IF
		else //Player is only a counselor
		{
			x=0;
			for(i=1;i<MAXPAGES;i++)
			{
				if(counspages[i].handled==0)
				{
					P_CHAR pj=pointers::findCharBySerial( counspages[i].serial.serial32 );
					if(ISVALIDPC(pj))
					{
						Location dest= pj->getPosition();

						pc_currchar->sysmsg("");
						pc_currchar->sysmsg("Transporting to next call: %s", counspages[i].name);
						pc_currchar->sysmsg("Problem: %s.", counspages[i].reason);
						pc_currchar->sysmsg("Serial number: %x", counspages[i].serial.serial32);
						pc_currchar->sysmsg("Paged at %s.", counspages[i].timeofcall);
						counspages[i].handled=1;
						pc_currchar->MoveTo( dest );
						pc_currchar->callnum=i;
						pc_currchar->teleport();
						x++;
						break;
					}// if
				}// else
				if(x>0)break;
			}//for
			if(x==0) pc_currchar->sysmsg("The Counselor queue is currently empty");
		}//if
	}

	void KillSpawn(NXWSOCKET s, int r)  //courtesy of Revana
	{
		/*unsigned int i;
		int killed=0;
		char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

		r++; // synch with 1-indexed real storage, casue 0 is no region indicator, LB

		if (r<=0 || r>=255) return;

		sysmessage(s,"Killing spawn, this may cause lag...");

		for(i=0;i<charcount;i++)
		{
			P_CHAR pc_i=MAKE_CHAR_REF(i);
			if(ISVALIDPC(pc_i)) {

				if(pc_i->spawnregion==r && !pc_i->free)
				{
					pc_i->boltFX()
					pc_i->playSFX( 0x0029);
					pc_i->deleteChar();
					killed++;
				}
			}
		}


		for(i=0;i<itemcount;i++)
		{
			P_ITEM pi =MAKE_ITEM_REF(i);
			if(ISVALIDPI(pi))
			{
				if(pi->spawnregion==r && !pi->free)
				{
					pi->deleteItem();
					killed++;
				}
			}
		}

		gcollect();
		sysmessage(s, "Done.");
		sprintf(temp, "%i of Spawn %i have been killed.",killed,r-1);
		sysmessage(s, temp);*/
	}

	void RegSpawnMax (NXWSOCKET s, int r ) // rewrite LB
	{
		P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC(pc);

	//	unsigned int currenttime=uiCurrentTime;

		sysbroadcast(TRANSLATE("Spawn %d is spawning all items/NPCs, this will cause some lag."), r);

		Spawns->doSpawnAll( r );

		pc->sysmsg("[DONE] All NPCs/items spawned in spawn %d.", r );
	}

	void RegSpawnNum (NXWSOCKET s, int r, int n) // rewrite by LB
	{
	/*	int i, spawn=0;
		unsigned int currenttime=uiCurrentTime;
		char *temps;

		r++;
		if (r<=0 || r>=255) return;

		temps = new char[100];

			spawn = (spawnregion[r].max-spawnregion[r].current);
			if (n > spawn)
			{
				sprintf(temps, "%d too many for region %d, spawning %d to reach MAX:%d instead.",n, r-1, spawn, spawnregion[r].max);
				sysmessage(s, temps);
				n=spawn;
			}
			sprintf(temps, TRANSLATE("Region %d is Spawning %d NPCs/items, this will cause some lag."), r-1, spawn);
			sysbroadcast(temps);

			for(i=1;i<spawn;i++)
			{
				doregionspawn(r);
			}//for

			spawnregion[r].nexttime=currenttime+(MY_CLOCKS_PER_SEC*60*RandomNum(spawnregion[r].mintime,spawnregion[r].maxtime));

			sprintf(temps, "[DONE] %d total NPCs/items spawned in Spawnregion %d.",spawn,r-1);
			sysmessage(s, temps);
		if (temps!=NULL) delete [] temps; // fixing memory leak, LB
	*/
	}//regspawnnum

	void KillAll(NXWSOCKET s, int percent, char* sysmsg)
	{
	return;
	/*	int i;
		sysmessage(s,TRANSLATE("Killing all characters, this may cause some lag..."));
		sysbroadcast(sysmsg);
		for(i=0;i<charcount;i++)
		{
			P_CHAR pj=MAKE_CHAR_REF(i);
			if(!ISVALIDPC(pj))
				continue;

			if(!pj->IsGM())
			{
				if(rand()%100+1<=percent)
				{
					pj->boltFX();
					pj->playSFX(0x0029);
					pj->Kill();
				}
			}
		}
		sysmessage(s, "Done.");*/
	}

	//o---------------------------------------------------------------------------o
	//|   Function -  void cpage(NXWSOCKET s,char *reason)
	//|   Date     -  UnKnown
	//|   Programmer  -  UnKnown
	//o---------------------------------------------------------------------------o
	//|   Purpose     -
	//o---------------------------------------------------------------------------o
	void CPage(NXWSOCKET s, std::string reason) // Help button (Calls Counselor Call Menus up)
	{
		int i;
		SERIAL a;
		int x2=0;
		char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

		P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC( pc_currchar );

		a = pc_currchar->getSerial32();

		for(i=1;i<MAXPAGES;i++)
		{
			if(counspages[i].handled==1)
			{
				counspages[i].handled=0;
				strcpy(counspages[i].name,pc_currchar->getCurrentNameC());
				strcpy(counspages[i].reason,reason.c_str());
				counspages[i].serial.serial32 = a;
				time_t current_time = time(0);
				struct tm *local = localtime(&current_time);
				sprintf(counspages[i].timeofcall, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
				sprintf(temp,"%s [ %08x ] called at %s, %s",counspages[i].name,a,counspages[i].timeofcall,counspages[i].reason);
				pc_currchar->playercallnum=i;
				pc_currchar->pagegm=2;
				x2++;
				break;
			}
		}
		if(x2==0)
		{
			pc_currchar->sysmsg(TRANSLATE("The Counselor Queue is currently full. Contact the shard operator"));
			pc_currchar->sysmsg(TRANSLATE("and ask them to increase the size of the queue."));
		}
		else
		{
			if(reason == "OTHER")
			{
				pc_currchar->pagegm=0;
				sprintf(temp, "Counselor Page from %s [ %08x ]: %s",
				pc_currchar->getCurrentNameC(), a, reason.c_str());
				bool found=false;

				NxwSocketWrapper sw;
				sw.fillOnline();
				for( sw.rewind(); !sw.isEmpty(); sw++ ) {

					NXWCLIENT ps_i=sw.getClient();
					if(ps_i==NULL)
						continue;
					P_CHAR pj=ps_i->currChar();
					if ( ISVALIDPC(pj) && pj->IsCounselor() )
					{
						found=true;
						ps_i->sysmsg( temp);
					}

				}
				if (found)
					pc_currchar->sysmsg(TRANSLATE("Available Counselors have been notified of your request."));
				else
					pc_currchar->sysmsg(TRANSLATE("There was no Counselor available to take your call."));
			}
			else pc_currchar->sysmsg(TRANSLATE("Please enter the reason for your Counselor request"));
		}
	}

	//o---------------------------------------------------------------------------o
	//|   Function :  void gmpage(NXWSOCKET s,char *reason)
	//|   Date     :  Unknown
	//|   Programmer  :  Unknown
	//o---------------------------------------------------------------------------o
	//|   Purpose     :  Help button (Calls GM Call Menus up)
	//o---------------------------------------------------------------------------o
	void GMPage(NXWSOCKET s, std::string reason)
	{
		int i;
		SERIAL a;
		int x2=0;
		char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

		P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);

		a = pc_currchar->getSerial32();

		for(i=1;i<MAXPAGES;i++)
		{
			if(gmpages[i].handled==1)
			{
				gmpages[i].handled=0;
				strcpy(gmpages[i].name, pc_currchar->getCurrentNameC());
				strcpy(gmpages[i].reason,reason.c_str());
				gmpages[i].serial.serial32 = a;
				time_t current_time = time(0);
				struct tm *local = localtime(&current_time);
				sprintf(gmpages[i].timeofcall, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
				sprintf(temp,"%s [ %08x ] called at %s, %s",gmpages[i].name,a,gmpages[i].timeofcall,gmpages[i].reason);
				pc_currchar->playercallnum=i;
				pc_currchar->pagegm=1;
				x2++;
				break;
			}
		}
		if (x2==0)
		{
			pc_currchar->sysmsg(TRANSLATE("The GM Queue is currently full. Contact the shard operator"));
			pc_currchar->sysmsg(TRANSLATE("and ask them to increase the size of the queue."));
		}
		else
		{
			if(reason == "OTHER")
			{
				pc_currchar->pagegm=0;
				sprintf(temp, "Page from %s [ %08x ]: %s",
				pc_currchar->getCurrentNameC(), a, reason.c_str());
				bool found=false;

				NxwSocketWrapper sw;
				sw.fillOnline();
				for( sw.rewind(); !sw.isEmpty(); sw++ ) {

					NXWCLIENT ps_i=sw.getClient();
					if(ps_i==NULL)
						continue;
					P_CHAR pc_i=ps_i->currChar();
					if ( ISVALIDPC(pc_i) && pc_i->IsGM() )
					{
						found=true;
						pc_i->sysmsg( temp);
					}
				}
				if (found)
					pc_currchar->sysmsg(TRANSLATE("Available Game Masters have been notified of your request."));
				else
					pc_currchar->sysmsg(TRANSLATE("There was no Game Master available to take your call."));
			}
			else pc_currchar->sysmsg(TRANSLATE("Please enter the reason for your GM request"));
		}
	}

	void DyeItem(NXWSOCKET s) // Rehue an item
	{
		UI16 color, body;

		P_CHAR Me = MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC(Me);

		P_ITEM pi = pointers::findItemBySerPtr(buffer[s] +1);
		if( ISVALIDPI(pi) )
		{

			if( !Me->IsGMorCounselor() ) {

				if( !pi->dye )
					return;

				P_ITEM outmost = pi->getOutMostCont();
				SERIAL cont = outmost->getContSerial();
				if( isCharSerial( cont ) ) {
					if( cont!=Me->getSerial32() )
						return;
				}
				else if( isItemSerial( cont ) ) {
					P_ITEM backpack = Me->getBackpack();
					if( ISVALIDPI( backpack ) && ( cont!=backpack->getSerial32() ) )
						return;
				}
				else { //on ground
					if( pi->magic==4 )
						return;
				}
			}

			color = ShortFromCharPtr(buffer[s] +7);


			if (( color<0x0002) || (color>0x03E9))
			{
				color = 0x03E9;
			}


			if (! ((color & 0x4000) || (color & 0x8000)) )
			{
				pi->setColor( color );
			}

			if (color == 0x4631)
			{
				pi->setColor( color );
			}

			pi->Refresh();

			Me->playSFX(0x023E);
			return;
		}

		P_CHAR pc = pointers::findCharBySerPtr(buffer[s] +1);
		if( ISVALIDPC(pc) && Me->IsGMorCounselor() )
		{
			color = ShortFromCharPtr(buffer[s] +7);


			body = pc->getId();

			if(  color < 0x8000  && body >= BODY_MALE && body <= BODY_DEADFEMALE ) color |= 0x8000; // why 0x8000 ?! ^^;

			if ((color & 0x4000) && (body >= BODY_MALE && body<= 0x03E1)) color = 0xF000; // but assigning the only "transparent" value that works, namly semi-trasnparency.

			if (color != 0x8000)
			{
				pc->setColor(color);
				pc->setOldColor(color);
				pc->teleport( TELEFLAG_NONE );

				Me->playSFX(0x023E);
			}
		}
	}


	void AddHere(NXWSOCKET s, UI16 id, char z)
	{
		if ( s < 0 || s >= now )
			return;
		LOGICAL pileable=false;
		tile_st tile;

		data::seekTile( id, tile);
		if (tile.flags&TILEFLAG_STACKABLE) pileable=true;

		P_ITEM pi = item::CreateFromScript( "$item_hardcoded" );
		VALIDATEPI( pi );
		pi->setId( id );
		pi->pileable = pileable;

		if(ISVALIDPI(pi))//AntiChrist - to preview crashes
		{
			P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
			Location charpos= pc_currchar->getPosition();

			pi->MoveTo( charpos.x, charpos.y, z );
			pi->doordir=0;
			pi->setDecay( false );
			pi->setNewbie( false );
			pi->setDispellable( false );
			pi->Refresh();//AntiChrist
		}

	}


	void DupeItem(NXWSOCKET s, int i, int amount)
	{
		P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC( pc );
		P_ITEM pi_from = MAKE_ITEM_REF( i );
		VALIDATEPI( pi_from );
		P_ITEM pack = pc->getBackpack();
		VALIDATEPI( pack );

		if ( !pi_from->corpse )
		{
			P_ITEM pi_to = archive::item::New();
			(*pi_to)=(*pi_from);

			pi_to->setContSerial( pack->getSerial32() );
			pi_to->SetRandPosInCont( pack );

			pi_to->amount = amount;

			//if (pi_from->getOwnerSerial32() != INVALID)
			//	pi_to->setOwnerSerial32( pi_from->getOwnerSerial32() );

			pi_to->Refresh();
		}
	}

	void ShowGMQue(NXWSOCKET s, int type) // Shows next unhandled call in the GM queue
	{
		P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC(pc);

		// Type is 0 if it is a Counselor doing the command (or a GM doing /cq) and 1 if it is a GM

		int i;
		int x=0;

		if(type==1) //Player is a GM
		{
			for(i=1;i<MAXPAGES;i++)
			{
				if (gmpages[i].handled==0)
				{
					if(x==0)
					{
						pc->sysmsg("");
						pc->sysmsg("Next unhandled page from %s", gmpages[i].name);
						pc->sysmsg("Problem: %s.", gmpages[i].reason);
						pc->sysmsg("Serial number %x", gmpages[i].serial.serial32);
						pc->sysmsg("Paged at %s.", gmpages[i].timeofcall);
					}
					x++;
				}
			}
			if (x>0)
			{
				pc->sysmsg("");
				pc->sysmsg("Total pages in queue: %i",x);
			}
			else pc->sysmsg("The GM queue is currently empty");
		} //end of first if
		else //Player is a counselor so show counselor queue
		{
			for(i=1;i<MAXPAGES;i++)
			{
				if (counspages[i].handled==0)
				{
					if(x==0)
					{
						pc->sysmsg("");
						pc->sysmsg("Next unhandled page from %s", counspages[i].name);
						pc->sysmsg("Problem: %s.", counspages[i].reason);
						pc->sysmsg("Serial number %x", counspages[i].serial.serial32);
						pc->sysmsg("Paged at %s.", counspages[i].timeofcall);
					}
					x++;
				}
			}
			if (x>0)
			{
				pc->sysmsg("");
				pc->sysmsg("Total pages in queue: %i",x);
			}
			else pc->sysmsg("The Counselor queue is currently empty");
		}
	}
	// new wipe function, basically it prints output on the console when someone wipes so that
	// if a malicious GM wipes the world you know who to blame

	/*!
	\brief wipes every wipeable object
	\author Luxor
	*/
	void Wipe(NXWSOCKET s)
	{
		if (s < 0 || s >= now)
			return;

		P_CHAR pc = pointers::findCharBySerial(currchar[s]);
		VALIDATEPC(pc);
		InfoOut( "%s has initiated an item wipe\n", pc->getCurrentNameC() );
		cAllObjectsIter objs;
		P_ITEM pi = NULL;
		for( objs.rewind(); !objs.IsEmpty(); objs++ ) {
			pi = pointers::findItemBySerial( objs.getSerial() );
			if ( ISVALIDPI(pi) && pi->isInWorld() && pi->wipe == 0 )
				pi->Delete();
		}

		sysbroadcast( TRANSLATE("All items have been wiped") );
	}


	//New Luxor's possess command :) Enjoy!
	/*!
	\brief possess command for GMs
	\author Luxor
	\param s the socket of the GM
	*/
	void Possess(NXWSOCKET s)
	{
		P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC(pc);
		P_CHAR pcPos = pointers::findCharBySerPtr(buffer[s]+7);
		VALIDATEPC(pcPos);

		pc->possess(pcPos);

	}
};
