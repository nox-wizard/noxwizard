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

static inline void _do_target(NXWSOCKET s, TARGET_S *ts) {
	target(s, ts->a1, ts->a2, ts->a3, ts->a4, ts->txt);
	return;
}

namespace Commands
{
	SI32 cmd_offset;

	/* extensively modified 8/2/99 crackerjack@crackerjack.net -
	* see cmdtable.cpp for more details */
	/* rearranged by Xanathar, for NoX-Wizard 0.70s */
	/* rearranged by Riekr, for NoX-Wizard 0.83a */
	void Command(NXWSOCKET  s, char* speech) // Client entred a '/' command like /ADD
	{
		unsigned char *comm;
		unsigned char nonuni[512];

		cmd_offset = 1;

		P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[s]);

		strcpy((char*)nonuni, speech);
		strcpy((char*)tbuffer, (char*)nonuni);

		strupr((char*)nonuni);
		cline = (char*)&nonuni[0];
		splitline();

		if (tnum<1)	return;
		// Let's ignore the command prefix;
		comm = nonuni + 1;

		P_COMMAND cmd= commands->findCommand((char*)comm);
		NXWCLIENT client= getClientFromSocket(s);

		if(cmd==NULL) {
		client->sysmsg("Unrecognized command: %s", comm);
			return;
		}
		if(cmd->notValid(pc_currchar)) {
			client->sysmsg("Access denied.");
			return;
		}

		switch(cmd->cmd_type) {
		// Single step commands
			case CMD_FUNC:
				(*((CMD_EXEC)cmd->cmd_extra)) (s);
				break;
			case CMD_ITEMMENU:
				itemmenu(s, (int)cmd->cmd_extra);
				break;
			case CMD_TARGET:
				_do_target(s, (TARGET_S *)cmd->cmd_extra);
				break;
			case CMD_TARGETX:
				if(tnum==2) {
					addx[s]=strtonum(1);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes one number as an argument.");
				}
				break;
			case CMD_TARGETXY:
				if(tnum==3) {
					addx[s]=strtonum(1);
					addy[s]=strtonum(2);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes two numbers as arguments.");
				}
				break;
			case CMD_TARGETXYZ:
				if(tnum==4) {
					addx[s]=strtonum(1);
					addy[s]=strtonum(2);
					addz[s]=strtonum(3);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					sysmessage(s, "This command takes three numbers as arguments.");
				}
				break;
			case CMD_TARGETHX:
				if(tnum==2) {
					addx[s]=strtonum(1);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes one hex number as an argument.");
				}
				break;
			case CMD_TARGETHXY:
				if(tnum==3) {
					addx[s]=strtonum(1);
					addy[s]=strtonum(2);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes two hex numbers as arguments.");
				}
				break;
			case CMD_TARGETHXYZ:
				if(tnum==4) {
					addx[s]=strtonum(1);
					addy[s]=strtonum(2);
					addz[s]=strtonum(3);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes three hex numbers as arguments.");
				}
				break;
			case CMD_TARGETID1:
				if(tnum==2) {
					addid1[s]=strtonum(1);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes one number as an argument.");
				}
				break;
			case CMD_TARGETID2:
				if(tnum==3) {
					addid1[s]=strtonum(1);
					addid2[s]=strtonum(2);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes two numbers as arguments.");
				}
				break;
			case CMD_TARGETID3:
				if(tnum==4) {
					addid1[s]=strtonum(1);
					addid2[s]=strtonum(2);
					addid3[s]=strtonum(3);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes three numbers as arguments.");
				}
				break;
			case CMD_TARGETID4:
				if(tnum==5) {
					addid1[s]=strtonum(1);
					addid2[s]=strtonum(2);
					addid3[s]=strtonum(3);
					addid4[s]=strtonum(4);
					//ConOut("1: %i 2: %i 3: %i 4: %i\n",addid1[s],addid2[s],addid2[s],addid3[s],addid4[s]);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes four numbers as arguments.");
				}
				break;
			case CMD_TARGETHID1:
				if(tnum==2) {
					addid1[s]=strtonum(1);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes one hex number as an argument.");
				}
				break;
			case CMD_TARGETHID2:
				if(tnum==3) {
					addid1[s]=strtonum(1);
					addid2[s]=strtonum(2);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes two hex numbers as arguments.");
				}
				break;
			case CMD_TARGETHID3:
				if(tnum==4) {
					addid1[s]=strtonum(1);
					addid2[s]=strtonum(2);
					addid3[s]=strtonum(3);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes three hex numbers as arguments.");
				}
				break;
			case CMD_TARGETHID4:
				if(tnum==5) {
					addid1[s]=strtonum(1);
					addid2[s]=strtonum(2);
					addid3[s]=strtonum(3);
					addid4[s]=strtonum(4);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes four hex numbers as arguments.");
				}
				break;
			case CMD_TARGETTMP:
				if(tnum==2) {
					tempint[s]=strtonum(1);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes a number as an argument.");
				}
				break;
			case CMD_TARGETHTMP:
				if(tnum==2) {
					tempint[s]=strtonum(1);
					_do_target(s, (TARGET_S *)cmd->cmd_extra);
				} else {
					client->sysmsg("This command takes a hex number as an argument.");
				}
				break;
			case CMD_MANAGEDCMD:
			client->startCommand(cmd, speech);
			break;
			default:
				client->sysmsg("INTERNAL ERROR: Command has a bad command type set!");
				break;
		}

	}


	void MakeShop(int c)
	{
		P_CHAR pc = MAKE_CHAR_REF( c );
		VALIDATEPC( pc );

		pc->shopkeeper = true;

		P_ITEM pi;	
		if (pc->GetItemOnLayer(LAYER_TRADE_RESTOCK) == NULL)
		{
			pi = item::SpawnItem(INVALID,c,1,"#",0,0x2AF8,0,0,0);
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
			pi = item::SpawnItem(INVALID,c,1,"#",0,0x2AF8,0,0,0);
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
			pi = item::SpawnItem(INVALID,c,1,"#",0,0x2AF8,0,0,0);
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
					bolteffect(i, true);
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
					bolteffect(i, true);
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
		if(ISVALIDPI(pi))
		{
			color = ShortFromCharPtr(buffer[s] +7);


			if(!(dyeall[s]))
			{
				if (( color<0x0002) || (color>0x03E9))
				{
					color = 0x03E9;
				}
			}


			if (! ((color & 0x4000) || (color & 0x8000)) )
			{
				pi->color1 = color >> 8;
				pi->color2 = color % 256;
			}

			if (color == 0x4631)
			{
				pi->color1 = color >> 8;
				pi->color2 = color % 256;
			}

			pi->Refresh();

			Me->playSFX(0x023E);
			return;
		}

		P_CHAR pc = pointers::findCharBySerPtr(buffer[s] +1);
		if(ISVALIDPC(pc))
		{
			color = ShortFromCharPtr(buffer[s] +7);

			if( !Me->IsGM() ) return; // Only gms dye characters

			body = pc->GetBodyType();

			if(  color < 0x8000  && body >= BODY_MALE && body <= BODY_DEADFEMALE ) color |= 0x8000; // why 0x8000 ?! ^^;

			if ((color & 0x4000) && (body >= BODY_MALE && body<= 0x03E1)) color = 0xF000; // but assigning the only "transparent" value that works, namly semi-trasnparency.

			if (color != 0x8000)
			{
				pc->setSkinColor(color);
				pc->setOldSkinColor(color);
				pc->teleport( TELEFLAG_NONE );

				Me->playSFX(0x023E);
			}
		}
	}


	void SetItemTrigger(NXWSOCKET s)
	{
		P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC(pc);

		P_ITEM pi = pointers::findItemBySerPtr(buffer[s] + 7);
		if (ISVALIDPI(pi))
		{
			pc->sysmsg("Item triggered");
			pi->trigger=addx[s];
		}
	}

	void SetTriggerType(NXWSOCKET s)
	{
		P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC(pc);

		P_ITEM pi = pointers::findItemBySerPtr(buffer[s] + 7);
		if (ISVALIDPI(pi))
		{
			pc->sysmsg("Trigger type set");
			pi->trigtype=addx[s];
		}
	}

	void SetTriggerWord(NXWSOCKET s)
	{
		P_CHAR Me=MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC(Me);

		P_CHAR pc=pointers::findCharBySerPtr(buffer[s]+7);
		VALIDATEPC(pc);

		Me->sysmsg("Trigger word set");
		strcpy(pc->trigword,xtext[s]);
	}

	void AddHere(NXWSOCKET s, char z)
	{
		LOGICAL pileable=false;
		tile_st tile;

		Map->SeekTile((addid1[s]<<8)|addid2[s], &tile);
		if (tile.flag2&0x08) pileable=true;

		P_ITEM pi=item::SpawnItem(-1,s, 1, "#", pileable, (addid1[s]<<8)|addid2[s], 0, 0, 0);

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

		addid1[s]=0;
		addid2[s]=0;
	}


	void SetNPCTrigger(NXWSOCKET s)
	{
		P_CHAR Me=MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC(Me);

		P_CHAR pc=pointers::findCharBySerPtr(buffer[s]+7);
		if( ISVALIDPC(pc))
		{
			Me->sysmsg("NPC triggered");
			pc->trigger=addx[s];
		}
	}


	void WhoCommand(NXWSOCKET s, int type,int buttonnum)
	{
		char sect[512];
		short int textlines;
		int k;
		unsigned int line, i;
		char menuarray[7*(MAXCLIENT)+50][50];  /** lord binary **/
		char menuarray1[7*(MAXCLIENT)+50][50]; /** the ( IS important !!! **/
		unsigned int linecount=0;
		unsigned int linecount1=0;

		P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
		VALIDATEPC(pc);

		if (buttonnum < 0) return;
		k=buttonnum;


		P_CHAR pj=pointers::findCharBySerial(whomenudata[buttonnum]);
		if(!ISVALIDPC(pj))
		{
			pc->sysmsg("selected character not found");
			return;
		}

			//--static pages
		strcpy(menuarray[linecount++], "nomove");
		strcpy(menuarray[linecount++], "noclose");
		strcpy(menuarray[linecount++], "page 0");
		strcpy(menuarray[linecount++], "resizepic 0 0 5120 260 280");    //The background
		strcpy(menuarray[linecount++], "button 20 240 2130 2129 1 0 1"); //OKAY
		strcpy(menuarray[linecount++], "text 20 10 300 0");           //text <Spaces from Left> <Space from top> <Length, Color?> <# in order>
		strcpy(menuarray[linecount++], "text 20 30 300 1");

		//--Command Button Page
		strcpy(menuarray[linecount++], "page 1");
		strcpy(menuarray[linecount++], "text 20 60 300 2");	//goto text
		strcpy(menuarray[linecount++], "button 150 60 1209 1210 1 0 200"); //goto button
		strcpy(menuarray[linecount++], "text 20 80 300 3");	//gettext
		strcpy(menuarray[linecount++], "button 150 80 1209 1210 1 0 201"); //get button
		strcpy(menuarray[linecount++], "text 20 100 300 4");	//Jail text
		strcpy(menuarray[linecount++], "button 150 100 1209 1210 1 0 202"); //Jail button
		strcpy(menuarray[linecount++], "text 20 120 300 5");	//Release text
		strcpy(menuarray[linecount++], "button 150 120 1209 1210 1 0 203"); //Release button
		strcpy(menuarray[linecount++], "text 20 140 300 6");	//Kick user text
		strcpy(menuarray[linecount++], "button 150 140 1209 1210 1 0 204"); //kick button
		strcpy(menuarray[linecount++], "text 20 180 300 7");


		short int length=21;
		short int length2=1;

		for(line=0;line<linecount;line++)
		{

			if (strlen(menuarray[line])==0)
				break;

			length+=strlen(menuarray[line])+4;
			length2+=strlen(menuarray[line])+4;

		}

		length+=3;
		textlines=0;
		line=0;

		sprintf(menuarray1[linecount1++], "User %i selected (account %i)",buttonnum,pj->account);
		sprintf(menuarray1[linecount1++], "Name: %s",pj->getCurrentNameC());
		sprintf(menuarray1[linecount1++], "Goto Character:");
		sprintf(menuarray1[linecount1++], "Get Character:");
		sprintf(menuarray1[linecount1++], "Jail Character:");
		sprintf(menuarray1[linecount1++], "Release Character:");
		sprintf(menuarray1[linecount1++], "Kick Character:");
		sprintf(menuarray1[linecount1++], "Serial#[%i]",pj->getSerial32());

		for(line=0;line<linecount1;line++)
		{

			if (strlen(menuarray1[line])==0)
				break;

			length+=strlen(menuarray1[line])*2 +2;
			textlines++;
		}

		UI08 gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";

		ShortToCharPtr(length, gump1 +1);
		gump1[7]=0;
		gump1[8]=0;
		gump1[9]=0;
		gump1[10]=type; // Gump Number
		ShortToCharPtr(length2, gump1 +19);
		Xsend(s, gump1, 21);

		for(line=0;line<linecount;line++)
		{
			sprintf(sect, "{ %s }", menuarray[line]);
			Xsend(s, sect, strlen(sect));
		}

		UI08 gump2[3]={ 0x00, };
		ShortToCharPtr(textlines, gump2 +1);

		Xsend(s, gump2, 3);

		UI08 gump3[2]= { 0x00, 0x00 };
		for(line=0;line<linecount1;line++)
		{
			if (strlen(menuarray1[line])==0)
				break;

			ShortToCharPtr(strlen(menuarray1[line]), gump3);
			Xsend(s, gump3, 2);
			gump3[0]=0;
			for (i=0;i<strlen(menuarray1[line]);i++)
			{
				gump3[1]=menuarray1[line][i];
				Xsend(s, gump3, 2);
			}
		}

//AoS/		Network->FlushBuffer(s);
	}


	void MakePlace(NXWSOCKET s, int i) // Decode a teleport location number into X/Y/Z
	{
		int zz;
		location2xyz(i, addx[s], addy[s], zz);
		addz[s] = zz;
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
			P_ITEM pi_to = archive::getNewItem();
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
				pi->deleteItem();
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

	/*	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
		P_CHAR pPos = FindCharBySerPtr(buffer[s]+7);	// char to posess
		if (!pPos) return;

		if (pPos->shopkeeper)
		{
			sysmessage(s,"You cannot use shopkeepers.");
			return;
		}
		if (!pPos->npc)
		{
			sysmessage( s, "You can only possess NPCs." );
			return;
		}

		unsigned char tmp;
		P_CHAR pc_currchar  = MAKE_CHARREF_LR(currchar[s]);

		if( pPos->npc == 17 ) // Char's old body
		{
			tmp = pPos->GetPriv();
			pPos->SetPriv(pc_currchar->GetPriv());
			pc_currchar->SetPriv(tmp);

			tmp = pPos->priv2;
			pPos->priv2 = pc_currchar->priv2;
			pc_currchar->priv2 = tmp;

			tmp = pPos->commandLevel;
			pPos->commandLevel = pc_currchar->commandLevel;
			pc_currchar->commandLevel = tmp;

			for(int i = 0; i < 7; i++)
			{
		int tempi = pPos->priv3[i];
		pPos->priv3[i] = pc_currchar->priv3[i];
		pc_currchar->priv3[i] = tempi;
			}

			pPos->npc = 0;
			pc_currchar->npc = 1;
			pc_currchar->account = INVALID;
			currchar[s] = DEREF_P_CHAR(pPos);
			//Network->startchar( s );
			Network->enterchar( s );
			sysmessage( s, "Welcome back to your old body." );
		}
		else if( pPos->npc )
		{
			tmp = pPos->GetPriv();
			pPos->SetPriv(pc_currchar->GetPriv());
			pc_currchar->SetPriv(tmp);

			tmp = pPos->priv2;
			pPos->priv2 = pc_currchar->priv2;
			pc_currchar->priv2 = tmp;

			tmp = pPos->commandLevel;
			pPos->commandLevel = pc_currchar->commandLevel;
			pc_currchar->commandLevel = tmp;

			for(int i = 0; i < 7; i++)
			{
		int tempi = pPos->priv3[i];
		pPos->priv3[i] = pc_currchar->priv3[i];
		pc_currchar->priv3[i] = tempi;
			}

			pPos->npc = 0;
			pPos->account = pc_currchar->account;
			pc_currchar->npc = 17;
			pc_currchar->npcWander = 0;
			currchar[s] = DEREF_P_CHAR(pPos);
			//Network->startchar( s );
			Network->enterchar( s );
			sprintf((char*)temp,"Welcome to %s's body!", pPos->getCurrentNameC() );
			sysmessage(s, (char*)temp);
		}
		else*/
	}
};
