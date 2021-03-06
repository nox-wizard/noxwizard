  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "item.h"
#include "data.h"
#include "scp_parser.h"
#include "archive.h"
#include "map.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "scripts.h"


namespace item
{


	/*!
	\author Anthalir
	*/
	P_ITEM CreateFromScript( char *itemname, cObject* cont, int amount )
	{
		int scid= xss::getIntFromDefine(itemname);
		if( scid==0 )
		{
			LogError("item '%s' is not defined in scripts", itemname);
			return NULL;
		}
		return item::CreateFromScript( scid, cont, amount );
	}

	/*!
	\author Xanathar
	\brief adds an item read from items.xss/scp
	\return the created item
	\param itemnum scriptid of the item
	\param cont container to add the item to
	*/
	P_ITEM CreateFromScript( SCRIPTID itemnum, cObject* cont, int amount )
	{
		char 		sect[512];
		std::string 	lha;
		std::string 	rha;

		cScpIterator* 	iter = NULL;
		int 		tmp;

		LOGICAL 	finished = false;

		sprintf(sect, "SECTION ITEM %i", itemnum);

		if((iter = Scripts::Items->getNewIterator(sect)) == NULL)
		{
			WarnOut("Undefined item number %d\n", itemnum);
			return NULL;
		}
		P_ITEM pi= archive::item::New();

		if (!ISVALIDPI(pi)) {
			safedelete(iter);
			return NULL;
		}

		pi->animSetId(0);
		pi->setId( 0x0915 );
		amxVS.setServerMode();

		do
		{
			iter->parseLine( lha, rha );
			lha=trim(lha);
			rha=trim(rha);
			if ( lha=="")
				continue;
			switch ( lha[0])
			{
				case '{':
					break;
				case '}':
					finished = true;
					break;
				case '@':
					if	( !LoadItemEventsFromScript( pi, const_cast<char*>(lha.c_str()), const_cast<char*>(rha.c_str()) ) )
						WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'A':
					if	( lha == "AMOUNT" )
						pi->amount = (UI16)str2num(rha);
					else if ( lha == "ATT" )
						pi->att = getRangedValue(const_cast<char*>(rha.c_str()));
					//
					// Old style user variables
					//
					else if ( lha == "AMXFLAG0" )
						amxVS.insertVariable( pi->getSerial32(), 0, str2num( rha ) );
					else if ( lha == "AMXFLAG1" )
						amxVS.insertVariable( pi->getSerial32(), 1, str2num( rha ) );
					else if ( lha == "AMXFLAG2" )
						amxVS.insertVariable( pi->getSerial32(), 2, str2num( rha ) );
					else if ( lha == "AMXFLAG3" )
						amxVS.insertVariable( pi->getSerial32(), 3, str2num( rha ) );
					else if ( lha == "AMXFLAG4" )
						amxVS.insertVariable( pi->getSerial32(), 4, str2num( rha ) );
					else if ( lha == "AMXFLAG5" )
						amxVS.insertVariable( pi->getSerial32(), 5, str2num( rha ) );
					else if ( lha == "AMXFLAG6" )
						amxVS.insertVariable( pi->getSerial32(), 6, str2num( rha ) );
					else if ( lha == "AMXFLAG7" )
						amxVS.insertVariable( pi->getSerial32(), 7, str2num( rha ) );
					//
					// New style user variables
					//
					else if (  lha ==  "AMXINT" )
					{
						std::string rha1;
						std::string rha2;
						splitLine( rha, rha1, rha2 );
						amxVS.insertVariable( pi->getSerial32(), str2num( rha1 ), str2num( rha2 ) );
					}
					else if ( lha == "AMXSTR"  )
					{
						std::string rha1;
						std::string rha2;
						splitLine( rha, rha1, rha2 );
						amxVS.insertVariable( pi->getSerial32(), str2num( rha1 ), rha2 );
					}
					else if ( lha == "ANIMID" )
						pi->animSetId((SI16)hex2num(rha));
					else if ( lha == "AUXDAMAGE" )
						pi->auxdamage = str2num(rha);
					else if ( lha == "AUXDAMAGETYPE" )
						pi->auxdamagetype = static_cast<DamageType>(str2num(rha));
					else if ( lha == "AMMO" )
						pi->ammo = str2num(rha);
					else if ( lha == "AMMOFX" )
						pi->ammoFx = hex2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'C':
					if	( lha == "COLOR" )
					{
						tmp = hex2num(rha);
						pi->setColor((UI16)tmp);
					}
					else if ( lha == "CONTAINS" )
					{
						//
						// Sparhawk: 	for containers only, an easy way to fill a container with items
						//		Original idea by Pier Sotto Jox
						//
						switch( pi->type )
						{
							case ITYPE_CONTAINER		:
							case ITYPE_UNLOCKED_CONTAINER	:
							case ITYPE_LOCKED_CONTAINER	:
								{
									std::string	str_scriptId,
											str_itemAmount;
									std::string str_chanceWord, str_chance, remainder, str_remains;
									if ( rha.find("CHANCE")>= 0 && rha.find("CHANCE") < rha.length())
									{
										splitLine( rha, str_scriptId, str_remains );
										splitLine( str_remains, str_chanceWord, remainder);
										// Either an amount was not given and str_chanceWord is nwo the value of amount
										if ( str_chanceWord == "CHANCE")
											str_chance=remainder;
										else
										// or the amount was there so remainder contains CHANCE <chance>
											splitLine(remainder, str_chanceWord, str_chance);

										rha.erase(rha.find(" CHANCE"), rha.length());
										str_scriptId="";
									}
									SI32 chance=str2num(str_chance);
									if ( chance > 0 )
									{
										if ( rand()%100 > chance )
											break;
									}

									splitLine( rha, str_scriptId, str_itemAmount );

									SCRIPTID	scriptId = str2num( str_scriptId );
									SI32		itemAmount = str2num( str_itemAmount );

									if ( itemAmount < 1 )
										itemAmount = 1;

									P_ITEM pItem = CreateFromScript( scriptId, pi, itemAmount );

									if( !ISVALIDPI( pItem ) )
										WarnOut( "CreateFromScript: invalid attribute %s %s", lha.c_str(), rha.c_str() );
								}
								break;
							default				:
								WarnOut( "CreateFromScript: %s attribute in non container item %i", lha.c_str(), itemnum );
						}
						break;
					}
					else if ( lha == "CONTAINLIST" )
					{
						switch( pi->type )
						{
							case ITYPE_CONTAINER		:
							case ITYPE_UNLOCKED_CONTAINER	:
							case ITYPE_LOCKED_CONTAINER	:
							{
								std::string str_scriptid, str_chanceWord, str_chance, str_amount, str_remains, str_keyword;
								std::string strItemList = rha;
								do
								{
									str_keyword="";
									str_remains="";
									splitLine( strItemList, str_keyword, str_remains );
									if ( str_scriptid == "" )
										str_scriptid=str_keyword;
									if ( str_keyword == "AMOUNT")
									{
										strItemList="";
										splitLine( str_remains, str_amount, strItemList );
									}
									else if ( str_keyword == "CHANCE" )
									{
										strItemList="";
										splitLine( str_remains, str_chance, strItemList );
									}
									else
										strItemList=str_remains;
								} while (strItemList != "" );

								SI32 chance=str2num(str_chance);
								SI32 amount=str2num(str_amount);
								if ( chance > 0 )
								{
									if ( rand()%100 > chance )
										break;
								}
								P_ITEM pitem=item::CreateScriptRandomItem( const_cast<char*>(str_scriptid.c_str()), pi);
								if( ISVALIDPI(pitem) )
								{
									if ( amount <= 0 )
										amount=1;
									pitem->amount=(UI16)amount;
								}
								else
									WarnOut( "CreateFromScript: invalid attribute %s %s", lha.c_str(), rha.c_str() );
								break;
							}
							default				:
								WarnOut( "CreateFromScript: %s attribute in non container item %i", lha.c_str(), itemnum );
						}
					}
					else if ( lha == "CREATOR" )
						pi->creator = rha;
					else if ( lha == "COLORLIST" )
					{
						std::string value(cObject::getRandomScriptValue("RANDOMCOLOR", rha ));
						pi->setColor( (UI16)hex2num(value) );
					}
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'D':
					if	( lha == "DAMAGE" )
						pi->att = getRangedValue( const_cast<char*>(rha.c_str()) );
					else if ( lha == "DAMAGETYPE" )
						pi->damagetype = static_cast<DamageType>(str2num(rha));
					else if ( lha == "DEX" )
						pi->dx = str2num(rha);
					else if ( lha == "DISABLED" )
						pi->disabled = uiCurrentTime + (str2num(rha)*MY_CLOCKS_PER_SEC);// AntiChrist
					else if ( lha == "DISABLEMSG" ) {
						if( pi->disabledmsg )
							(*pi->disabledmsg) = rha;
						else
							pi->disabledmsg= new std::string( rha );
					}
					else if ( lha == "DISPELLABLE" )
						pi->setDispellable();
					else if ( lha == "DECAY" )
						pi->setDecay();
					else if ( lha == "DIR" )
						pi->dir = (SI16)str2num(rha);
					else if ( lha == "DYE" )
						pi->dye = str2num(rha);
					else if ( lha == "DEXADD" )
						pi->dx2 = str2num(rha);
					else if ( lha == "DEF" )
						pi->def = str2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'F':
					if ( lha == "FIGHTSKILL" )
						pi->fightskill = static_cast<Skill>(str2num(rha)); // Added by Luxor
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'G':
					if ( lha == "GENDER" )
						pi->setGender((SI08)str2num(rha)); // Added by Wintermute
					else if ( lha == "GOOD" )
						pi->good = str2num(rha); // Added by Magius(CHE)
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'H':
					if ( lha == "HIDAMAGE" )
						pi->hidamage = str2num(rha);
					else if ( lha == "HP" )
						pi->hp = getRangedValue((char*)rha.c_str());
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'I':
					if ( lha == "ID" )
					{
						tmp = hex2num(rha);
						pi->setId((UI16) tmp );
					}
					else if ( lha == "ITEMLIST" )
					{
						pi->Delete();
						std::string str_scriptid="", str_amount="";
						splitLine( rha.c_str(), str_scriptid, str_amount );
						pi=item::CreateScriptRandomItem( const_cast<char*>(str_scriptid.c_str()), cont);
						if( ISVALIDPI(pi) )
							if( strcmp( str_amount.c_str(), "") ) //ndEndy defined amount, 1 by default
								pi->amount=(UI16)str2num( str_amount);

					}
					else if ( lha == "INT" )
						pi->in = str2num(rha);
					else if ( lha == "INTADD" )
						pi->in2 = str2num(rha);
					else if ( lha == "ITEMHAND" )
						pi->itmhand = str2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'L':
					if ( lha == "LAYER" )
						pi->layer = pi->scriptlayer =(SI08) str2num(rha);	//Luxor
					else if ( lha == "LODAMAGE" )
						pi->lodamage = str2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'M':
					if ( lha == "MORE" )
					{

						if ( rha.find("0x") == 0)
							tmp=hex2num(rha);
						else
							tmp = str2num(rha);
						pi->more1 = (UI08)(tmp >> 24);
						pi->more2 = (UI08)(tmp >> 16);
						pi->more3 = (UI08)(tmp >> 8);
						pi->more4 = (UI08)(tmp%256);
					}
					// MORE2 may not be useful ?
					else if ( lha == "MORE2" )
					{
						if ( rha.find("0x") == 0)
							tmp=hex2num(rha);
						else
							tmp = str2num(rha);
						pi->moreb1 = (SI08)(tmp >> 24);
						pi->moreb2 = (SI08)(tmp >> 16);
						pi->moreb3 = (SI08)(tmp >> 8);
						pi->moreb4 = (SI08)(tmp%256);
					}
					else if ( lha == "MOVABLE" )
						pi->magic = (SI08)str2num(rha);
					else if ( lha == "MAXHP" )
						pi->maxhp = str2num(rha); // by Magius(CHE)
					else if ( lha == "MOREX" )
						pi->morex = str2num(rha);
					else if ( lha == "MOREY" )
						pi->morey = str2num(rha);
					else if ( lha == "MOREZ" )
						pi->morez = str2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'N':
					if ( lha == "NEWBIE" )
						pi->setNewbie();
					else if ( lha == "NAME" )
						pi->setCurrentName(rha);
					else if ( lha == "NAME2" )
						pi->setSecondaryName(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'O':
					if ( lha == "OFFSPELL" )
						pi->offspell = (SI08)str2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'P':
					if ( lha == "POISONED" )
						pi->poisoned = (PoisonType)str2num(rha);

					else if ( lha == "PILE" )

						pi->pileable = str2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'R':
					if	( "RACE" == lha )
						pi->setRace((SI32)str2num(script2));
					else if ( lha == "RANK" )
					{
						pi->rank = str2num(rha); // By Magius(CHE)
						if (pi->rank <= 0)
							pi->rank = 10;
					}
					else if ( lha == "REQSKILL" )
					{// Added by Luxor
						int params[2];
						fillIntArray( const_cast<char*>(rha.c_str()), params, 2, 0, 10);
						pi->reqskill[0] = params[0];
						pi->reqskill[1] = params[1];
					}
					else if ( lha == "RESISTS" )
					{// Added by Luxor
						int params[2];
						fillIntArray( const_cast<char*>(rha.c_str()), params, 2, 0, 10);
						if (params[0] < MAX_RESISTANCE_INDEX) {
							pi->resists[params[0]] = params[1];
						}
					}
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'S':
					if ( lha == "SK_MADE" )
						pi->madewith = str2num(rha); // by Magius(CHE)
					else if ( lha == "SMELT" )
						pi->smelt = str2num(rha);
					else if ( lha == "STR" )
						pi->st = str2num(rha);
					else if ( lha == "SPD" )
						pi->spd = str2num(rha);
					else if ( lha == "STRADD" )
						pi->st2 = str2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'T':
					if ( lha == "TYPE" )
						pi->type = str2num(rha);
					else if ( lha == "TRIGGER" )
						pi->trigger = str2num(rha);
					else if ( lha == "TRIGTYPE" )
						pi->trigtype = str2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'U':
					if ( lha == "USES" )
						pi->tuses = str2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'V':
					if ( lha == "VISIBLE" )
						pi->visible = (SI08)str2num(rha);
					else if ( lha == "VALUE" )
						pi->value = str2num(rha);
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
				case 'W':
					if ( lha == "WEIGHT" )
					{
						pi->weight = str2num(rha);
					}
					else WarnOut("Unrecognised attribute : \"%s\", in item number %i\n", lha.c_str(), itemnum);
					break;
			}
		}
		while ( !finished );

		amxVS.setUserMode();
		pi->useAnimId=(pi->animid()!=0);

		pi->itemSoundEffect=-1;

		tile_st tile;
		data::seekTile(pi->getId(), tile);

		if (tile.flags&TILEFLAG_STACKABLE)
			pi->pileable = 1;

		if (!pi->maxhp && pi->hp)
			pi->maxhp = pi->hp; // Magius(CHE)

		safedelete(iter);
		pi->setScriptID(itemnum);

		if( amount!=INVALID )
			pi->amount=(UI16)amount;

		//Luxor: put it in the container
		if ( ISVALIDPO( cont ) )
			if( isItemSerial( cont->getSerial32() ) )
			{
				((P_ITEM)cont)->AddItem(pi);
			}
			else
				pi->setCont( cont );
		itemCount +=1;
		return pi;

	}

	int CreateRandomItem( char * sItemList )//NEW FUNCTION -- 24/6/99 -- AntiChrist merging codes
	{
		int i=0, loopexit=0, iList[256];  //-- no more than 256 items in a single item list
		char sect[512];
		cScpIterator* iter = NULL;
		char script1[1024];

		sprintf(sect, "SECTION ITEMLIST %s", sItemList);

		iter = Scripts::Items->getNewIterator(sect);
		if (iter==NULL) return INVALID;

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

	P_ITEM CreateScriptRandomItem( char * sItemList, cObject* cont )
	{
		int k=CreateRandomItem(sItemList);   // -- Get random Item #
		return (k>0)? item::CreateFromScript( k, cont ) : NULL;
	}




//not checked

	/*!
	\warning for internal use only
	\author Luxor
	*/
	static P_ITEM spawnItemByIdInternal(int nAmount, const char* cName, short id, short color)
	{
		LOGICAL pile=false;

		tile_st tile;
		data::seekTile(id, tile);
		pile = (tile.flags&TILEFLAG_STACKABLE);

		P_ITEM pi = item::CreateFromScript( "$item_hardcoded" );
   		VALIDATEPIR( pi, NULL );
    	pi->setId( id );
		pi->setColor( color );
    		pi->pileable = pile;

		if(cName!=NULL)
			pi->setCurrentName(cName);

		pi->setDecay();
		return pi;
	}

	LOGICAL moreItemMemoryRequested = false;

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
		pi = item::CreateFromScript( itemnum,cont);
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
			zz=(UI16)(buffer[s][16]+tileHeight((buffer[s][17]<<8)+buffer[s][18]));
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
				{
#ifdef SPAR_I_LOCATION_MAP
					pointers::addToLocationMap( pi );
#else
					mapRegions->add(pi);
#endif
				}
		}

		return pi;

	}

	/*
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
	/*P_ITEM SpawnItem(NXWSOCKET  nSocket, int nAmount, char* cName, LOGICAL nStackable,
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
	}*/

/*
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
	/*P_ITEM SpawnItem(NXWSOCKET  nSocket, CHARACTER ch,
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
	}*/

	SI32 getname(int i, char* itemname)
	{
		tile_st tile;
		int j, len, mode, used, ok, namLen;
		P_ITEM pi=MAKE_ITEM_REF(i);
		VALIDATEPIR(pi,1)
		if ( strncmp(pi->getCurrentNameC(), "#", 1) )	// if name[0] != '#'
		{
			strcpy(itemname, pi->getCurrentNameC());
			return strlen(itemname)+1;
		}
		data::seekTile(pi->getId(), tile);
		if (tile.flags&TILEFLAG_PREFIX_AN) strcpy(itemname, "an ");
		else if (tile.flags&TILEFLAG_PREFIX_A) strcpy(itemname, "a ");
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

	/*P_ITEM SpawnItem(CHARACTER ch, SI32 nAmount, char* cName, LOGICAL pileable, SI16 id, SI16 color, LOGICAL nPack)
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
		pi->setColor(color);
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

	}*/

	/*!
	\author Magius(CHE), bugfixed by AntiChrist
	*/
	void GetScriptItemSetting(P_ITEM pi)
	{
		//ConOut("GetScriptItemSetting....");//debug


		int loopexit=0;
		cScpIterator* iter = NULL;
		std::string script1;
		std::string script2;
		int tmp;
		LOGICAL sectfound=false;
		char buff[512];


		if(pi == NULL) return;

		sprintf(buff,"x%x",pi->getId());


		pi->animSetId(0);


		safedelete(iter);
		iter = Scripts::HardItems->getNewIteratorInStr(buff);

		if (iter==NULL) return;


		do
		{
			iter->parseLine(script1, script2);
			script1=trim(script1);
			script2=trim(script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if (script1[0] != 'S')
					continue;
				if (script1=="SECTION")
					if(script2.find(buff)) sectfound = true;//AntiChrist bugfix

				if(sectfound)//AntiChrist bugfix
				{
					if ( script1=="" )
						continue;
					switch (script1[0])
					{
					case 'A':
					case 'a':
						if (script1=="AMOUNT") pi->amount=(UI16)str2num(script2); // -Fraz- moved from Case C
						else if (script1=="ANIMID") pi->animSetId(hex2num(script2));
						break;
					case 'C':
					case 'c':
						if ( script1=="CREATOR") pi->creator = script2; // by Magius(CHE)
						else if (script1=="COLOR")
						{
							tmp=hex2num(script2);
							pi->setColor((UI16) tmp );
						}

					break;

					case 'D':
					case 'd':
						if ( script1=="DEX") pi->dx=str2num(script2);
						else if ( script1=="DEXADD") pi->dx2=str2num(script2);
						else if ((script1=="DAMAGE")||(script1=="ATT"))
							pi->att = getRangedValue(script2);
						else if (script1=="DEF") pi->def=str2num(script2);
						else if (script1=="DYE") pi->dye=str2num(script2);
						else if (script1=="DIR") pi->dir=str2num(script2);
						else if (script1=="DECAY") pi->setDecay();
						else if (script1=="DISPELLABLE") pi->setDispellable();
						else if (script1=="DISABLEMSG") {
							if( pi->disabledmsg!=NULL )
								(*pi->disabledmsg) = script2;
							else
								pi->disabledmsg = new std::string( script2 );
						}
						else if (script1=="DISABLED") pi->disabled=uiCurrentTime+(str2num(script2)*MY_CLOCKS_PER_SEC);//AntiChrist
					break;

					case 'G':
					case 'g':
					case 'H':
					case 'h':
						if (script1=="GOOD") pi->good=str2num(script2); // Added by Magius(CHE)
						else if ( script1=="HP") pi->hp=str2num(script2);
						else if ( script1=="HIDAMAGE") pi->hidamage=str2num(script2);
					break;

					case 'I':
					case 'i':
					case 'L':
					case 'l':
						if ( script1=="INT") pi->in=str2num(script2);
						else if ( script1=="INTADD") pi->in2=str2num(script2);
						else if (script1=="ITEMHAND") pi->itmhand=str2num(script2);
						else if (script1=="LAYER") pi->layer=(SI08)str2num(script2);
						else if ( script1=="LODAMAGE") pi->lodamage=str2num(script2);
					break;

					case 'M':
					case 'm':
						if ( script1=="MAXHP") pi->maxhp=str2num(script2); // by Magius(CHE)
						else if (script1=="MOVABLE") pi->magic=(SI08)str2num(script2);
						else if ( script1=="MORE")
						{
							tmp=str2num(script2);
							pi->more1 = (unsigned char) (tmp>>24);
							pi->more2 = (unsigned char) (tmp>>16);
							pi->more3 = (unsigned char) (tmp>>8);
							pi->more4 = (unsigned char) (tmp%256);
						}
						//MORE2 may not be useful ?
						else if ( script1=="MORE2")
						{
							tmp=str2num(script2);
							pi->moreb1=(SI08)(tmp>>24);
							pi->moreb2=(SI08)(tmp>>16);
							pi->moreb3=(SI08)(tmp>>8);
							pi->moreb4=(SI08)(tmp%256);
						}
						else if (script1=="MOREX") pi->morex=str2num(script2);
						else if (script1=="MOREY") pi->morey=str2num(script2);
						else if (script1=="MOREZ") pi->morez=str2num(script2);
					break;

					case 'N':
					case 'n':
						if (script1=="NAME") pi->setCurrentName(script2);
						else if (script1=="NAME2") pi->setSecondaryName(script2);
						else if (script1=="NEWBIE") pi->setNewbie();
					break;

					case 'O':
					case 'o':
						if (script1=="OFFSPELL") pi->offspell=str2num(script2);
					break;

					case 'P':
					case 'p':
					case 'R':
					case 'r':
						if (script1=="POISONED") pi->poisoned=(PoisonType)str2num(script2);
						else if (script1=="RANK")
						{
							pi->rank=str2num(script2); // By Magius(CHE)
							if (pi->rank==0) pi->rank=10;
						}
					break;

					case 'S':
					case 's':
						if (script1=="SOUNDFX") pi->setSound((UI16) str2num(script2));
						else if (script1=="SPD") pi->spd=str2num(script2);
						else if ( script1=="SK_MADE") pi->madewith=str2num(script2); // by Magius(CHE)
						else if ( script1=="STR") pi->st=str2num(script2);
						else if ( script1=="STRADD") pi->st2=str2num(script2);
						else if ( script1=="SMELT") pi->smelt=str2num(script2);
					break;

					case 'T':
					case 't':
					case 'U':
					case 'u':
						if (script1=="TRIGGER") pi->trigger=str2num(script2);
						else if (script1=="TRIGTYPE") pi->trigtype=str2num(script2);
						else if (script1=="TYPE") pi->type=str2num(script2);
						else if (script1=="USES") pi->tuses=str2num(script2);
					break;

					default:
						if (script1=="VISIBLE") pi->visible=(SI08)str2num(script2);
						else if (script1=="VALUE") pi->value=str2num(script2);

						else if (script1=="WEIGHT")
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

	/*P_ITEM add (int itemid, int x, int y, int z)
	{
		P_ITEM pi= item::CreateFromScript( itemid);
		if ((pi!=NULL)&&(x!=INVALID)) {
			z = Map->Height( x, y, z );
			pi->MoveTo(x,y,z);
			pi->Refresh();
		}
		return pi;
	}*/

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
			z = getHeight( Loc( x, y, z ) );
			pi->MoveTo(x,y,z);
			pi->Refresh();
		}
		return pi;
	}


	P_ITEM SpawnRandomItem(NXWSOCKET s, char* cList, char* cItemID)
	{

		P_CHAR pc=MAKE_CHAR_REF( currchar[s] );
		VALIDATEPCR( pc, NULL );

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
			if(item[i]!=INVALID) {
				return item::CreateFromScript( item[i], pc->getBackpack());
			}
		}
		return NULL;

	}
};
