  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "item.h"

namespace item
{

	/*!
	\warning for internal use only
	\author Xanathar
	*/
	static P_ITEM spawnItemByIdInternal(int nAmount, const char* cName, short id, short color)
	{
		LOGICAL pile=false;

		tile_st tile;
		Map->SeekTile(id, &tile);
		pile = (tile.flag2&0x08);
		P_ITEM pi=archive::getNewItem();
		if ( pi==NULL )
			return NULL;

		if(cName!=NULL)
			pi->setCurrentName(cName);

		pi->setId(id);
		pi->animSetId(0);
		pi->color1=color>>8;
		pi->color2=color&0x00FF;
		pi->amount=nAmount;
		pi->pileable=(pile) ? '\1' : '\0';
		pi->att=5;
		pi->setDecay();
		item::GetScriptItemSetting(pi); // Added by Magius(CHE) (2)
		pi->Refresh();
		return pi;
	}

	LOGICAL moreItemMemoryRequested = false;

	/*!
	\author Anthalir
	*/
	P_ITEM CreateFromScript(NXWSOCKET so, char *itemname)
	{
		int scid= xss::getIntFromDefine(itemname);
		if( scid==0 )
		{
			LogError("item '%s' is not defined in scripts", itemname);
			return NULL;
		}
		return item::CreateFromScript(so, scid);
	}

	/*!
	\author Xanathar
	\brief adds an item read from items.xss/scp
	\return the created item
	\param so socket (if any)
	\param itemnum item number to be created
	\param cont container to add the item to
	*/
	P_ITEM CreateFromScript(NXWSOCKET  so, int itemnum, cObject* cont )
	{
		char sect[512];
	char script1[1024];
	char script2[1024];
	cScpIterator* iter = NULL;
		int tmp, loopexit = 0;
		tile_st tile;

		sprintf(sect, "SECTION ITEM %i", itemnum);

		if((iter = Scripts::Items->getNewIterator(sect)) == NULL)
			return NULL;

		P_ITEM pi= archive::getNewItem();

		if (!ISVALIDPI(pi)) {
			safedelete(iter);
			return NULL;
		}

		pi->animSetId(0);
		pi->setId(0x0915);
		amxVS.setServerMode();

		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				switch (script1[0])
				{
					case '@':
						if(!LoadItemEventsFromScript(pi,script1,script2))
							WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;
					case 'A':
					case 'a':
						if (!strcmp("AMOUNT", script1))	 pi->amount = str2num(script2);
						else if (!strcmp("ATT", script1)) pi->att = getRangedValue(script2);
						//
						// Old style user variables
						//
						else if (!strcmp("AMXFLAG0", script1)) amxVS.insertVariable( pi->getSerial32(), 0, str2num( script2 ) );
						else if (!strcmp("AMXFLAG1", script1)) amxVS.insertVariable( pi->getSerial32(), 1, str2num( script2 ) );
						else if (!strcmp("AMXFLAG2", script1)) amxVS.insertVariable( pi->getSerial32(), 2, str2num( script2 ) );
						else if (!strcmp("AMXFLAG3", script1)) amxVS.insertVariable( pi->getSerial32(), 3, str2num( script2 ) );
						else if (!strcmp("AMXFLAG4", script1)) amxVS.insertVariable( pi->getSerial32(), 4, str2num( script2 ) );
						else if (!strcmp("AMXFLAG5", script1)) amxVS.insertVariable( pi->getSerial32(), 5, str2num( script2 ) );
						else if (!strcmp("AMXFLAG6", script1)) amxVS.insertVariable( pi->getSerial32(), 6, str2num( script2 ) );
						else if (!strcmp("AMXFLAG7", script1)) amxVS.insertVariable( pi->getSerial32(), 7, str2num( script2 ) );
						//
						// New style user variables
						//
						else if ( !strcmp( "AMXINT", script1 ) )
						{
							splitLine( script2, script1, script3 );
							amxVS.insertVariable( pi->getSerial32(), str2num( script1 ), str2num( script3 ) );
						}
						else if (!strcmp("AMXSTR", script1) )
						{
							splitLine( script2, script1, script3 );
							amxVS.insertVariable( pi->getSerial32(), str2num( script1 ), script3 );
						}
						else if (!strcmp("ANIMID", script1))   pi->animSetId(hex2num(script2));
						else if (!strcmp("AUXDAMAGE", script1))       pi->auxdamage = str2num(script2);
						else if (!strcmp("AUXDAMAGETYPE", script1))   pi->auxdamagetype = static_cast<DamageType>(str2num(script2));
						else if (!strcmp("AMMO", script1))	pi->ammo = str2num(script2);
						else if (!strcmp("AMMOFX", script1))	pi->ammoFx = hex2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'C':
					case 'c':
						if (!strcmp("COLOR", script1))
						{
							tmp = hex2num(script2);
							pi->color1 = tmp >> 8;
							pi->color2 = tmp%256;
						}
						else if (!strcmp("CREATOR", script1))		pi->creator = script2;
						else if (!strcmp("COLORLIST", script1))
						{
							tmp = addrandomcolor(pi, script2);
							{
								pi->color1 = tmp >> 8;
								pi->color2 = tmp%256;
							}
						}
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'D':
					case 'd':
						if (!strcmp("DAMAGE", script1))		pi->att = getRangedValue(script2);
						else if (!strcmp("DAMAGETYPE", script1))
							pi->damagetype = static_cast<DamageType>(str2num(script2));
						else if (!strcmp("DEX", script1))
							pi->dx = str2num(script2);
						else if (!strcmp("DISABLED", script1))
							pi->disabled = uiCurrentTime + (str2num(script2)*MY_CLOCKS_PER_SEC);// AntiChrist
						else if (!strcmp("DISABLEMSG", script1))
							pi->disabledmsg = script2;
						else if (!strcmp("DISPELLABLE", script1))
							pi->setDispellable();
						else if (!strcmp("DECAY", script1))
							pi->setDecay();
						else if (!strcmp("DIR", script1))
							pi->dir = str2num(script2);
						else if (!strcmp("DYE", script1))
							pi->dye = str2num(script2);
						else if (!strcmp("DEXADD", script1))
							pi->dx2 = str2num(script2);
						else if (!strcmp("DEF", script1))
							pi->def = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'F':
					case 'f':
						if (!(strcmp("FIGHTSKILL", script1)))
							pi->fightskill = static_cast<Skill>(str2num(script2)); // Added by Luxor
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'G':
					case 'g':
						if (!(strcmp("GOOD", script1)))
							pi->good = str2num(script2); // Added by Magius(CHE)
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'H':
					case 'h':
						if (!(strcmp("HIDAMAGE", script1)))
							pi->hidamage = str2num(script2);
						else if (!(strcmp("HP", script1)))
							pi->hp = getRangedValue(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'I':
					case 'i':
						if (!(strcmp("ID", script1)))
						{
							tmp = hex2num(script2);
							pi->setId(tmp);
						}
						else if (!strcmp("ITEMLIST", script1))
						{
							pi->deleteItem();
							pi=item::CreateScriptRandomItem(so, script2, cont);
						}
						else if (!strcmp("INT", script1))
							pi->in = str2num(script2);
						else if (!strcmp("INTADD", script1))
							pi->in2 = str2num(script2);
						else if (!strcmp("ITEMHAND", script1))
							pi->itmhand = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'L':
					case 'l':
						// Sparhawk -> altered so that chars may get items on race enlistment
						// Anthalir -> why && ( so==-1 || Race::isRaceSystemActive() ) ???
						// this result that created item won't have layer set
						if (!strcmp("LAYER", script1) /*&& ( so==-1 || Race::isRaceSystemActive() )*/ )
							pi->layer = pi->scriptlayer = str2num(script2);	//Luxor
						else if (!strcmp("LODAMAGE", script1))
							pi->lodamage = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'M':
					case 'm':
						if (!strcmp("MORE", script1))
						{
							tmp = str2num(script2);
							pi->more1 = tmp >> 24;
							pi->more2 = tmp >> 16;
							pi->more3 = tmp >> 8;
							pi->more4 = tmp%256;
						}
						// MORE2 may not be useful ?
						else if (!strcmp("MORE2", script1))
						{
							tmp = str2num(script2);
							pi->moreb1 = tmp >> 24;
							pi->moreb2 = tmp >> 16;
							pi->moreb3 = tmp >> 8;
							pi->moreb4 = tmp%256;
						}
						else if (!strcmp("MOVABLE", script1))
							pi->magic = str2num(script2);
						else if (!strcmp("MAXHP", script1))
							pi->maxhp = str2num(script2); // by Magius(CHE)
						else if (!strcmp("MOREX", script1))
							pi->morex = str2num(script2);
						else if (!strcmp("MOREY", script1))
							pi->morey = str2num(script2);
						else if (!strcmp("MOREZ", script1))
							pi->morez = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'N':
					case 'n':
						if (!strcmp("NEWBIE", script1))
							pi->setNewbie();
						else if (!strcmp("NAME", script1))
							pi->setCurrentName(script2);
						else if (!strcmp("NAME2", script1))
							pi->setSecondaryName(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'O':
					case 'o':
						if (!strcmp("OFFSPELL", script1))
							pi->offspell = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'P':
					case 'p':
						if (!strcmp("POISONED", script1))
							pi->poisoned = (PoisonType)str2num(script2);

						if (!strcmp("PILE", script1))

							pi->pileable = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'R':
					case 'r':
						if (!strcmp("RANK", script1))
						{
							pi->rank = str2num(script2); // By Magius(CHE)
							if (pi->rank <= 0)
								pi->rank = 10;
						}
						else if (!(strcmp("REQSKILL", script1)))
						{// Added by Luxor
							int params[2];
							fillIntArray(script2, params, 2, 0, 10);
							pi->reqskill[0] = params[0];
							pi->reqskill[1] = params[1];
						}
						else if (!(strcmp("RESISTS", script1)))
						{// Added by Luxor
							int params[2];
							fillIntArray(script2, params, 2, 0, 10);
							if (params[0] < MAX_RESISTANCE_INDEX) {
								pi->resists[params[0]] = params[1];
							}
						}
						else if (!(strcmp("RESTOCK", script1)))
							pi->restock = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;


					case 'S':
					case 's':
						if (!strcmp("SK_MADE", script1))
							pi->madewith = str2num(script2); // by Magius(CHE)
						else if (!strcmp("SMELT", script1))
							pi->smelt = str2num(script2);
						else if (!strcmp("STR", script1))
							pi->st = str2num(script2);
						else if (!strcmp("SPD", script1))
							pi->spd = str2num(script2);
						else if (!strcmp("STRADD", script1))
							pi->st2 = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'T':
					case 't':
						if (!strcmp("TYPE", script1))
							pi->type = str2num(script2);
						else if (!strcmp("TRIGGER", script1))
							pi->trigger = str2num(script2);
						else if (!strcmp("TRIGTYPE", script1))
							pi->trigtype = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'U':
					case 'u':
						if (!strcmp("USES", script1))
							pi->tuses = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'V':

					case 'v':
						if (!strcmp("VISIBLE", script1))
							pi->visible = str2num(script2);
						else if (!strcmp("VALUE", script1))
							pi->value = str2num(script2);
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;

					case 'W':
					case 'w':
						if (!strcmp("WEIGHT", script1))
						{
							int anum = 3;
							// anum=4;
							anum = str2num(script2); // Ison 2-20-99
							pi->weight = anum;
						}
						else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", script1, itemnum);
						break;
				}
			}
		}
		while ((script1[0] != '}') &&(++loopexit < MAXLOOPS));

		amxVS.setUserMode();
		pi->useAnimId=(pi->animid()!=0);
		Map->SeekTile(pi->id(), &tile);

		if (tile.flag2&0x08)
			pi->pileable = 1;

		if (!pi->maxhp && pi->hp)
			pi->maxhp = pi->hp; // Magius(CHE)

		safedelete(iter);
		pi->setScriptID(itemnum);

		//Luxor: put it in the container
                if ( ISVALIDPO( cont ) )
			pi->setCont( cont );
		
		return pi;

	}

	/*!
	\brief adds an item from spawner or gm 'ADDITEM command
	\return the pointer to the item added
	\param s socket of the gm or -1
	\param itemnum spawner item or -1
	\param nSpawned is spawned?
	\param cont container to add the item to
	*/
	P_ITEM CreateScriptItem(NXWSOCKET s, SI32 itemnum, LOGICAL nSpawned, cObject* cont )
	{
		P_ITEM pi= NULL;
		pi = item::CreateFromScript(s,itemnum,cont);
		if (!ISVALIDPI(pi))
		{
			LogWarning("ITEM <%i> not found in the scripts",itemnum);
			return NULL;
		}

		if ((s!=INVALID) && (!nSpawned))
		{
			short xx,yy,zz;
			xx=(buffer[s][11]<<8)+buffer[s][12];
			yy=(buffer[s][13]<<8)+buffer[s][14];
			zz=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]);
			pi->MoveTo(xx,yy,(SI08)zz);
		}
		else
		{
			if( cont!=NULL ) {
				pi->setCont( cont );
				if( isItemSerial( cont->getSerial32() ) )
					pi->SetRandPosInCont( (P_ITEM)cont );
				pi->Refresh();
			}
			else
				if (pi->isInWorld())
					mapRegions->add(pi);
		}

		return pi;

	}

	int CreateRandomItem(char * sItemList)//NEW FUNCTION -- 24/6/99 -- AntiChrist merging codes
	{
		int i=0, loopexit=0, iList[256];  //-- no more than 256 items in a single item list
		char sect[512];
		cScpIterator* iter = NULL;
		char script1[1024];

		sprintf(sect, "SECTION ITEMLIST %s", sItemList);

		iter = Scripts::Items->getNewIterator(sect);
		if (iter==NULL) return -1;

		do  // -- count items storing item #'s in iList[]
		{
			strcpy(script1, iter->getEntry()->getFullLine().c_str());
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				iList[i]=str2num(script1);
				i++;
			}
		}
		while ((script1[0]!='}')&&(++loopexit < MAXLOOPS));
		safedelete(iter);

		if (i==0) return iList[0]; else return(iList[rand()%i]);

	}

	P_ITEM CreateScriptRandomItem(SI32 s, char * sItemList, cObject* cont )
	{
		int k=CreateRandomItem(sItemList);   // -- Get random Item #

		if (k>0) {
			return CreateScriptItem(s, k, 1, cont);  // -- Create Item
			// This stuff smells buggy >:[, Xan
		}
		else return NULL;
	}

	/*!
	\author Duke
	\brief Create an item for a socket
	\note I took the idea (to overload) from the 'official' source. So thanks to fur!
	\param nSocket the socket to add the item for
	\param nAmount amount of item to spawn
	\param cName name of the item
	\param nStackable true if item is stackable
	\param cItemId item's ID
	\param cColorId item's color
	\param nPack if true the item is spawned in socket's backpack
	\param nSend if true the item is sent to all clients

	OK, here's the scoop, in a few key places we were calling add item when there was
	flat out no way to get a socket, say if a region spawner made an item, there is no
	associated socket.  This was causing the socket to be -1, and it was indexing into
	curchar[-1] and blowing up.  So I made a new version that takes a socket and a
	a character (the following function) if you want to call that one with socket = -1
	thats fine, because you have to pass the character in as well.  If you call this
	function (which was the original) you are not allowed to pass in -1 for the socket
	because it will crash. I've put the assert() in its place, so at least when it does
	crash you'll know why. If this assert() ever happens to you, it means you need to
	fix the CALLER of this function to pass the character in and call the other function
	instead. Taking the assert() out won't help, its just liable to crash on currchar[-1]
	anyways.  - fur

	what fur said about the assert only partially applies to this version. Duke

	*/
	P_ITEM SpawnItem(NXWSOCKET  nSocket, int nAmount, char* cName, LOGICAL nStackable,
						SI16 cItemId, SI16 cColorId,
						LOGICAL nPack, LOGICAL nSend)
	{
		if (nSocket < 0)
		{
			ErrOut("SpawnItem called with bad socket\n");
			return NULL;
		}
		else
		{
			return item::SpawnItem(nSocket, currchar[nSocket], nAmount, cName, nStackable, cItemId, cColorId, nPack, nSend);
		}
	}

/*!
	\param nSocket the socket to add the item for
	\param ch the character to add the item for
	\param nAmount amount of item to spawn
	\param cName name of the item
	\param nStackable true if item is stackable
	\param cItemId item's ID
	\param cColorId item's color
	\param nPack if true the item is spawned in socket's backpack
	\param nSend if true the item is sent to all clients
*/
	P_ITEM SpawnItem(NXWSOCKET  nSocket, CHARACTER ch,
						SI32 nAmount, char* cName, LOGICAL nStackable,
						SI16 cItemId, SI16 cColorId,
						LOGICAL nPack, LOGICAL nSend)
	{
		P_ITEM pi= item::SpawnItem(ch, nAmount, cName, nStackable, cItemId, cColorId, nPack);
		if (pi==NULL) return NULL;
		if (nSend && nSocket!=INVALID) {
			P_CHAR pc=MAKE_CHAR_REF(currchar[nSocket]);
			VALIDATEPCR(pc,NULL);
			statwindow(pc,MAKE_CHAR_REF(ch));
		}
		return pi;
	}

	P_ITEM SpawnItemBank(CHARACTER ch, int nItem)
	{

		if (ch < 0)
			return NULL;

		P_CHAR pc=MAKE_CHAR_REF(ch);
		VALIDATEPCR(pc,NULL);

		P_ITEM bankbox = NULL;
		bankbox = pc->GetBankBox();

		if (bankbox == NULL)
		{
			LogWarning("Bank box not found in SpawnItemBank()");
			return NULL;
		}

		NXWSOCKET  s = calcSocketFromChar(ch);          // Don't check if s == -1, it's ok if it is.
		P_ITEM pi = item::CreateScriptItem(s, nItem, 1);
		VALIDATEPIR(pi,NULL);
		item::GetScriptItemSetting(pi);
		bankbox->AddItem(pi);
		statwindow(pc,pc);
		return pi;
	}

	SI32 getname(int i, char* itemname)
	{
		tile_st tile;
		int j, len, mode, used, ok, namLen;
		P_ITEM pi=MAKE_ITEMREF_LRV(i,1);
		if ( strncmp(pi->getCurrentNameC(), "#", 1) )	// if name[0] != '#'
		{
			strcpy(itemname, pi->getCurrentNameC());
			return strlen(itemname)+1;
		}
		Map->SeekTile(pi->id(), &tile);
		if (tile.flag2&0x80) strcpy(itemname, "an ");
		else if (tile.flag2&0x40) strcpy(itemname, "a ");
		else itemname[0]=0;
		namLen = strlen( itemname );
		mode=0;
		used=0;
		len=strlen((char *) tile.name);
		for (j=0;j<len;j++)
		{
			ok=0;
			if ((tile.name[j]=='%')&&(mode==0)) mode=2;
			else if ((tile.name[j]=='%')&&(mode!=0)) mode=0;
			else if ((tile.name[j]=='/')&&(mode==2)) mode=1;
			else if (mode==0) ok=1;
			else if ((mode==1)&&(pi->amount==1)) ok=1;
			else if ((mode==2)&&(pi->amount>1)) ok=1;
			if (ok)
			{
				itemname[namLen++] = tile.name[j];
				itemname[namLen] = '\0';
				if (mode) used=1;
			}
		}
		return strlen(itemname)+1;

	}

	P_ITEM SpawnItem(CHARACTER ch, SI32 nAmount, char* cName, LOGICAL pileable, SI16 id, SI16 color, LOGICAL nPack)
	{
		P_CHAR pc=MAKE_CHAR_REF(ch);
		VALIDATEPCR(pc,NULL);
		P_ITEM pPack= pc->getBackpack();
		LOGICAL pile = false;

		if (pileable)
		{					// make sure it's REALLY pileable ! (Duke)
			tile_st tile;
			Map->SeekTile(id, &tile);
			if (tile.flag2&0x08)
				pile=true;
			else
			{
				LogWarning("Spawning an item that is not truly pileable, id=%x",id);
				pile=pileable;
			}
		}

		// no such item found, so let's create it
		P_ITEM pi=archive::getNewItem();

		if(cName!=NULL)
			pi->setCurrentName(cName);
		pi->setId(id);
		pi->animSetId(0);
		pi->useAnimId = false;
		pi->setScriptID(0);
		pi->color1=color>>8;
		pi->color2=color&0x00FF;
		pi->amount=nAmount;
		pi->pileable=pile;
		pi->att=5;
		pi->setDecay();

		if (nPack)
		{
			if (ISVALIDPI(pPack))
			{
				SERIAL piled = pPack->ContainerPileItem(pi);
				if( piled==INVALID ) { 
					pi->setContSerial( pPack->getSerial32() );
					pi->SetRandPosInCont( pPack );
				}
				else 
					return pointers::findItemBySerial( piled );				
			}
			else
			{// LB place it at players feet if he hasnt got backpack
				pi->MoveTo( pc->getPosition() );
			}
		}

		//ConOut("Adding Harditems settings in items.cpp:spawnitem\n");
		item::GetScriptItemSetting(pi); // Added by Magius(CHE) (2)
		pc->making=DEREF_P_ITEM(pi);
		pi->Refresh();
		return pi;

	}

	/*!
	\author Magius(CHE), bugfixed by AntiChrist
	*/
	void GetScriptItemSetting(P_ITEM pi)
	{
		//ConOut("GetScriptItemSetting....");//debug


		int loopexit=0;
		cScpIterator* iter = NULL;
		char script1[1024];
		char script2[1024];
		int tmp;
		LOGICAL sectfound=false;
		char buff[512];


		if(pi == NULL) return;

		sprintf(buff,"x%x",pi->id());


		pi->animSetId(0);


		safedelete(iter);
		iter = Scripts::HardItems->getNewIteratorInStr(buff);

		if (iter==NULL) return;


		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if (script1[0] != 'S')
					continue;
				if (!(strcmp("SECTION",script1)))
					if(strstr(script2,buff)) sectfound = true;//AntiChrist bugfix

				if(sectfound)//AntiChrist bugfix
				{
					switch (script1[0])
					{
					case 'A':
					case 'a':
						if (!(strcmp("AMOUNT",script1))) pi->amount=str2num(script2); // -Fraz- moved from Case C
						else if (!strcmp("ANIMID", script1)) pi->animSetId(hex2num(script2));
						break;
					case 'C':
					case 'c':
						if (!(strcmp("CREATOR", script1))) pi->creator = script2; // by Magius(CHE)
						else if (!(strcmp("COLOR",script1)))
						{
							tmp=hex2num(script2);
							pi->color1=tmp>>8;
							pi->color2=tmp%256;
						}
					break;

					case 'D':
					case 'd':
						if (!(strcmp("DEX", script1))) pi->dx=str2num(script2);
						else if (!(strcmp("DEXADD", script1))) pi->dx2=str2num(script2);
						else if ((!(strcmp("DAMAGE",script1)))||(!(strcmp("ATT",script1))))
							pi->att = getRangedValue(script2);
						else if (!(strcmp("DEF",script1))) pi->def=str2num(script2);
						else if (!(strcmp("DYE",script1))) pi->dye=str2num(script2);
						else if (!(strcmp("DIR",script1))) pi->dir=str2num(script2);
						else if (!(strcmp("DECAY",script1))) pi->setDecay();
						else if (!(strcmp("DISPELLABLE",script1))) pi->setDispellable();
						else if (!(strcmp("DISABLEMSG",script1))) pi->disabledmsg = script2;  // Added by Magius(CHE) �
						else if (!(strcmp("DISABLED",script1))) pi->disabled=uiCurrentTime+(str2num(script2)*MY_CLOCKS_PER_SEC);//AntiChrist
					break;

					case 'G':
					case 'g':
					case 'H':
					case 'h':
						if (!(strcmp("GOOD",script1))) pi->good=str2num(script2); // Added by Magius(CHE)
						else if (!(strcmp("HP", script1))) pi->hp=str2num(script2);
						else if (!(strcmp("HIDAMAGE", script1))) pi->hidamage=str2num(script2);
					break;

					case 'I':
					case 'i':
					case 'L':
					case 'l':
						if (!(strcmp("INT", script1))) pi->in=str2num(script2);
						else if (!(strcmp("INTADD", script1))) pi->in2=str2num(script2);
						else if (!(strcmp("ITEMHAND",script1))) pi->itmhand=str2num(script2);
						else if (!(strcmp("LAYER",script1))) pi->layer=str2num(script2);
						else if (!(strcmp("LODAMAGE", script1))) pi->lodamage=str2num(script2);
					break;

					case 'M':
					case 'm':
						if (!(strcmp("MAXHP", script1))) pi->maxhp=str2num(script2); // by Magius(CHE)
						else if (!(strcmp("MOVABLE",script1))) pi->magic=str2num(script2);
						else if (!(strcmp("MORE", script1)))
						{
							tmp=str2num(script2);
							pi->more1 = (unsigned char) (tmp>>24);
							pi->more2 = (unsigned char) (tmp>>16);
							pi->more3 = (unsigned char) (tmp>>8);
							pi->more4 = (unsigned char) (tmp%256);
						}
						//MORE2 may not be useful ?
						else if (!(strcmp("MORE2", script1)))
						{
							tmp=str2num(script2);
							pi->moreb1=tmp>>24;
							pi->moreb2=tmp>>16;
							pi->moreb3=tmp>>8;
							pi->moreb4=tmp%256;
						}
						else if (!(strcmp("MOREX",script1))) pi->morex=str2num(script2);
						else if (!(strcmp("MOREY",script1))) pi->morey=str2num(script2);
						else if (!(strcmp("MOREZ",script1))) pi->morez=str2num(script2);
					break;

					case 'N':
					case 'n':
					case 'O':
					case 'o':
						if (!(strcmp("NAME",script1))) pi->setCurrentName(script2);
						else if (!(strcmp("NAME2",script1))) pi->setSecondaryName(script2);
						else if (!(strcmp("NEWBIE",script1))) pi->setNewbie();
						else if (!(strcmp("OFFSPELL",script1))) pi->offspell=str2num(script2);

					break;

					case 'P':
					case 'p':
					case 'R':
					case 'r':
						if (!strcmp("POISONED",script1)) pi->poisoned=(PoisonType)str2num(script2);
						else if (!strcmp("RESTOCK",script1)) pi->restock=str2num(script2);
						else if (!strcmp("RANK",script1))
						{
							pi->rank=str2num(script2); // By Magius(CHE)
							if (pi->rank==0) pi->rank=10;
						}
					break;

					case 'S':
					case 's':
						if (!(strcmp("SPD",script1))) pi->spd=str2num(script2);
						else if (!(strcmp("SK_MADE", script1))) pi->madewith=str2num(script2); // by Magius(CHE)
						else if (!(strcmp("STR", script1))) pi->st=str2num(script2);
						else if (!(strcmp("STRADD", script1))) pi->st2=str2num(script2);
						else if (!(strcmp("SMELT", script1))) pi->smelt=str2num(script2);
					break;

					case 'T':
					case 't':
					case 'U':
					case 'u':
						if (!(strcmp("TRIGGER",script1))) pi->trigger=str2num(script2);
						else if (!(strcmp("TRIGTYPE",script1))) pi->trigtype=str2num(script2);
						else if (!(strcmp("TYPE",script1))) pi->type=str2num(script2);
						else if (!(strcmp("USES",script1))) pi->tuses=str2num(script2);
					break;

					default:
						if (!(strcmp("VISIBLE",script1))) pi->visible=str2num(script2);
						else if (!(strcmp("VALUE",script1))) pi->value=str2num(script2);

						else if (!(strcmp("WEIGHT",script1)))
						{
							int anum=3;
							//anum=4;
							anum=str2num(script2); // Ison 2-20-99
							pi->weight=anum;
							//ConOut("SETTING WEIGTH TO %i\n",pi->weight);
						}
						//Added by Krozy on 7-Sep-98
					break;
					}
				}
			}
			if(script1[0]=='}') break;
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

		safedelete(iter);
	}

	P_ITEM SpawnItemBackpack2(NXWSOCKET  s, int nItem, int nDigging) // Added by Genesis 11-5-98
	{

		P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
		P_ITEM backpack = pc->getBackpack();
		if (backpack == NULL)
			return NULL;

		P_ITEM pi= item::CreateScriptItem(s, nItem, 1);
		VALIDATEPIR(pi, NULL);

		if(nDigging)
		{
			if (pi->value!=0) pi->value=1+(rand()%(pi->value));
			if (pi->hp!=0) pi->hp=1+(rand()%(pi->hp));
			// blackwinds fix
			if (pi->maxhp != 0)
			{
				pi->maxhp=1+(rand()%pi->maxhp);
				pi->hp=1+(rand()%pi->maxhp);
			}
		}

		item::GetScriptItemSetting(pi);

		backpack->AddItem(pi);
		pi->Refresh();

		statwindow(pc,pc);
		return pi;

	}


	/*
	//NEW RESPAWNITEM FUNCTION STARTS HERE -- AntiChrist merging codes -- (24/6/99)

	void RespawnItem(unsigned int currenttime, int i)
	{
		int  j, k,m,serial,serhash,ci, c;
		//char ilist[66]="101010100010100101010100001101010000110101010101011010";
		P_ITEM pi=MAKE_ITEMREF_LR(i);
		if (pi->free) return;

		for(c=0;c<pi->amount;c++)
		{
			if(pi->gatetime+(c*pi->morez*MY_CLOCKS_PER_SEC)<=currenttime)// && chars[i].hp<=chars[i].st)
			{
				if ((pi->disabled>0)&&((pi->disabled<=currenttime)||(overflow)))
				{
					pi->disabled=0;
				}
				m=0;
				if (pi->type==61)
				{
					k=0;
					serial= pi->getSerial32();
					serhash= serial%HASHMAX;
					for (j=0;j<spawnsp[serhash].max;j++)
					{
						ci=spawnsp[serhash].pointer[j];

						P_ITEM p_ci=MAKE_ITEM_REF(ci);
						if (ISVALIDPI(p_ci))
						if(pi->getSerial32()==p_ci->spawnserial && (p_ci->free==0))
						{
							if ((pi!=p_ci) && (p_ci->getPosition() == pi->getPosition()) )
							{
								k=1;
								break;
							}
						}
					}

					if (k==0)
					{
						if (pi->gatetime==0)
						{
							pi->gatetime=(rand()%((int)(1+((pi->morez-pi->morey)*(MY_CLOCKS_PER_SEC*60))))) +
								(pi->morey*MY_CLOCKS_PER_SEC*60)+currenttime;
						}
						if ((pi->gatetime<=currenttime ||(overflow)) && pi->morex!=0)
						{
							item::AddRespawnItem(DEREF_P_ITEM(pi),pi->morex, 0);
							pi->gatetime=0;
						}
					}
				}
				if (pi->type==62 || pi->type==69 || pi->type==125)
				{
					k=0;
					serial= pi->getSerial32();
					serhash= serial%HASHMAX;
					for (j=0;j<cspawnsp[serhash].max;j++)
					{
						P_CHAR pc_ci=MAKE_CHAR_REF(cspawnsp[serhash].pointer[j]);
						if (ISVALIDPC(pc_ci))
							if (pc_ci->spawnserial==serial && !pc_ci->free)
								k++;
					}

					if (k<pi->amount)
					{
						if (pi->gatetime==0)
						{
							pi->gatetime=(rand()%((int)(1+
								((pi->morez-pi->morey)*(MY_CLOCKS_PER_SEC*60))))) +

								(pi->morey*MY_CLOCKS_PER_SEC*60)+currenttime;
						}
						if ((pi->gatetime<=currenttime || (overflow)) && pi->morex!=0)
						{
							npcs::AddRespawnNPC(pi,pi->morex);
							pi->gatetime=0;					
						}
					}
				}
				if ((pi->type==63)||(pi->type==64)||(pi->type==65)||(pi->type==66)||(pi->type==8))
				{
					serial= pi->getSerial32();
					if (serial > -1)
					{
						int size = pointers::pContMap[serial].size();
						for (j=0;j<size;j++)
						{
							P_ITEM p_ci=pointers::pContMap[serial][j];
							if (ISVALIDPI(p_ci))
								if (p_ci->getContSerial()==serial && p_ci->free==0)
								{
									m++;
								}
						}
					}
					if(m<pi->amount)
					{
						if (pi->gatetime==0)
						{
							pi->gatetime=(rand()%((int)(1+((pi->morez-pi->morey)*(MY_CLOCKS_PER_SEC*60))))) +
								(pi->morey*MY_CLOCKS_PER_SEC*60)+uiCurrentTime;
						}
						if ((pi->gatetime<=currenttime ||(overflow)) && pi->morex!=0)
						{
							if(pi->type==63) pi->type=64; //Lock the container
							//numtostr(pi->morex,m); //ilist); //LB, makes chest spawners using random Itemlist items instead of a single type, LB
							if(pi->morex)
							{
								item::AddRespawnItem(DEREF_P_ITEM(pi),pi->morex,1);//If the item contains an item list then it will randomly choose one from the list, JM
							}
							else
							{
								ci= item::CreateRandomItem("70"); //default itemlist);
								item::AddRespawnItem(DEREF_P_ITEM(pi),ci, 1);
							}
							pi->gatetime=0;
						}
					}
				}
			}//If time
		}//for

	}
	*//*
	void AddRespawnItem(int s, int x, int y)
	{
		P_ITEM pi=CreateScriptItem(-1, x, 1); // lb, bugfix
		VALIDATEPI(pi);

		P_ITEM p_s=MAKE_ITEM_REF(s);

		if(y<=0)
		{
			pi->MoveTo( p_s->getPosition() ); //add spawned item to map cell if not in a container
		}
		else
		{
			pi->setContSerial( p_s->getSerial32() ); //set item in pointer array
		}
		pi->SetSpawnSerial( p_s->getSerial32() );


		// Lb bugfix for spawning in wrong pack positions
		*//*if (y>0)
		{
			P_ITEM pChest;
			if (pi->spawnserial!=-1)
			{
				pChest=FindItemBySerial(pi->spawnserial);
				if (pChest)	pi->SetRandPosInCont(pChest);
			}
		}
		pi->Refresh();//AntiChrist

	}
	*/

	P_ITEM add (int itemid, int x, int y, int z)
	{
		P_ITEM pi= item::CreateFromScript(-1, itemid);
		if ((pi!=NULL)&&(x!=INVALID)) {
			z = Map->Height( x, y, z );
			pi->MoveTo(x,y,z);
			pi->Refresh();
		}
		return pi;
	}

	/*!
	\author Anthalir
	*/
	P_ITEM addByID(int id, UI32 nAmount, const char *cName, int color, Location where)
	{
		return item::addByID(id, nAmount, cName, color, where.x, where.y, where.z);
	}

	P_ITEM addByID (int id, UI32 nAmount, const char* cName, int color, int x, int y, int z)
	{
		P_ITEM pi = spawnItemByIdInternal(nAmount, cName, id, color);
		if ((pi!=NULL)&&(x!=INVALID)) {
			z = Map->Height( x, y, z );
			pi->MoveTo(x,y,z);
			pi->Refresh();
		}
		return pi;
	}


	P_ITEM SpawnRandomItem(NXWSOCKET s, char* cList, char* cItemID)
	{
		char sect[512];
		char script1[1024];
		cScpIterator* iter = NULL;
		int i=0,item[256]={0};


		sprintf(sect, "SECTION %s %s", cList, cItemID);

		iter = Scripts::Necro->getNewIterator(sect);
		if (iter==NULL) return NULL;

		int loopexit=0;
		do
		{
			strcpy(script1, iter->getEntry()->getFullLine().c_str());
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
			item[i]=str2num(script1);
			i++;
			}
		}
		while( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

		safedelete(iter);

		if(i>0)
		{
			i=rand()%(i);
			if(item[i]!=INVALID)
			return item::SpawnItemBackpack2(s,item[i],1);;
		}
		return NULL;

	}
};
