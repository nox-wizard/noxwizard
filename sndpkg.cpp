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
#include "particles.h"
#include "debug.h"
#include "basics.h"
#include "sndpkg.h"
#include "sregions.h"
#include "itemid.h"
#include "srvparms.h"
#include "amx/amxcback.h"
#include "tmpeff.h"
#include "speech.h"
#include "packets.h"
#include "layer.h"
#include "weight.h"
#include "accounts.h"
#include "scp_parser.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "basics.h"
#include "skills.h"
#include "range.h"
#include "classes.h"
#include "utils.h"
#include "scripts.h"

void gmyell(char *txt)
{
	UI08 unicodetext[512];
	int ucl = ( strlen ( txt ) * 2 ) + 2 ;

	char2wchar(txt);
	memcpy(unicodetext, Unicode::temp, ucl);

	UI32 lang = calcserial(server_data.Unicodelanguage[0], server_data.Unicodelanguage[1], server_data.Unicodelanguage[2], 0);
	UI08 sysname[30]={ 0x00, };
	strcpy((char *)sysname, "[WebAdmin - GM Only]");


	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWCLIENT ps_i=sw.getClient();
		if(ps_i==NULL) continue;
		P_CHAR pc=ps_i->currChar();
		NXWSOCKET s = ps_i->toInt();
		if( ISVALIDPC(pc) && pc->IsGM())
		{
			SendUnicodeSpeechMessagePkt(s, 0x01010101, 0x0101, 1, 0x0040, 0x0003, lang, sysname, unicodetext,  ucl);
		}
	}

	Network->ClearBuffers();
}


//keep the target highlighted so that we know who we're attacking =)
//26/10/99//new packet
void SndAttackOK(NXWSOCKET  s, int serial)
{
	UI08 attackok[5]={ 0xAA, 0x00, };
	LongToCharPtr(serial, attackok +1);
	Xsend(s, attackok, 5);
//AoS/	Network->FlushBuffer(s);
}

void SndDyevat(NXWSOCKET  s, int serial, short id)
{
	UI08 dyevat[9] ={ 0x95, 0x00, };
	LongToCharPtr(serial, dyevat +1);
	ShortToCharPtr(0x0000, dyevat +5);	// ignored on send ....
	ShortToCharPtr(id, dyevat +7);		// def. on send 0x0FAB
	Xsend(s, dyevat, 9);
//AoS/	Network->FlushBuffer(s);
}

void SndUpdscroll(NXWSOCKET  s, short txtlen, const char* txt)
{
	UI08 updscroll[10]={ 0xA6, };

	ShortToCharPtr(txtlen+10, updscroll +1);
	updscroll[3]=2;				// type: 0x00 tips window, 0x01 ignored, 0x02 updates
	LongToCharPtr(0 , updscroll +4);	// Tip numb.
	ShortToCharPtr(txtlen, updscroll +8);
	Xsend(s, updscroll, 10);
	Xsend(s, txt, txtlen);
//AoS/	Network->FlushBuffer(s);
}

void SndShopgumpopen(NXWSOCKET  s, SERIAL serial)	//it's really necessary ? It is used 1 time, perhaps replace it with the scriptable vers. :/
{
	UI08 shopgumpopen[7]={ 0x24, 0x00, };
	LongToCharPtr(serial, shopgumpopen +1);		// ItemID
	ShortToCharPtr(0x0030, shopgumpopen +5);	// GumpID
	Xsend(s, shopgumpopen, 7);
//AoS/	Network->FlushBuffer(s);
}


/*!
\brief play sound
\param goldtotal ?
\return soundsfx to play
*/
UI16 goldsfx(int goldtotal)
{
	UI16 sound;

	if (goldtotal==1)
		sound = 0x0035;
	else if (goldtotal<6)
		sound = 0x0036;
	else
		sound = 0x0037;

	return sound;
}

/*!
\brief play a sound based on item id

added to do easy item sound effects based on an
items id1 and id2 fields in struct items. Then just define the CASE statement
with the proper sound function to play for a certain item as shown.

\author Dupois Duke
\date 09/10/1998 creation
	  25/03/2001 new interface by duke
\param item the item
\return soundfx for the item
\remarks \remark Use the DEFAULT case for ranges of items (like all ingots make the same thump).
		 \remark Sounds:
			\li coins dropping (all the same no matter what amount because all id's equal 0x0EED
			\li ingot dropping (makes a big thump - used the heavy gem sound)
			\li gems dropping (two type broke them in half to make some sound different then others)
*/
UI16 itemsfx(UI16 item)
{
	UI16 sound = 0x0042;				// play default item move sfx // 00 48

	if( item == ITEMID_GOLD )
		sound = goldsfx(2);

	else if( (item>=0x0F0F) && (item<=0x0F20) )	// Any gem stone (typically smaller)
		sound = 0x0032;

	else if( (item>=0x0F21) && (item<=0x0F30) )	// Any gem stone (typically larger)
		sound = 0x0034;

	else if( (item>=0x1BE3) && (item<=0x1BFA) )	// Any Ingot
		sound = 0x0033;

	return sound;
}

/*!
\brief Plays background sounds of the game
\author LB & Duke
\param s the character index
\todo convert to P_CHAR or add to cChar class
*/
void bgsound(CHARACTER s)
{
    P_CHAR pc_curr=MAKE_CHAR_REF(s);
	VALIDATEPC(pc_curr);

	P_CHAR inrange[15];
	int y=0;

	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( pc_curr->getPosition(), VISRANGE+5, true, false );
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pc=sc.getChar();
		if( pc->npc && !pc->dead && !pc->war )
		{
			inrange[y++]=pc;
			if( y==15 )
				return;
		}
	}

	if (y>0)
	{
		P_CHAR pc_inr=inrange[ rand()%y ];
        if( chance(20) )
			pc_inr->playMonsterSound(SND_IDLE);
	}

	// play random mystic-sounds also if no creature is in range ...

	if(rand()%3333==33)
	{
		SOUND basesound=INVALID;
		switch(rand()%7)
		{
			case 0: basesound=595; break; // gnome sound
			case 1: basesound=287; break; // bigfoot 1
			case 2: basesound=288; break; // bigfoot 2
			case 3: basesound=639; break; // old snake sound
			case 4: basesound=179; break; // lion sound
			case 5: basesound=246; break; // mystic
			case 6: basesound=253; break; // mystic II
		}


		pc_curr->playSFX( basesound, true );
	}
}

/*!
\brief play a midi music
\author ?
\param s socket
*/
void dosocketmidi(NXWSOCKET s)
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);


    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];

	char sect[512];

	if (pc->war)
		strcpy(sect, "MIDILIST COMBAT");
	else
		sprintf(sect, "MIDILIST %i", region[pc->region].midilist);

	iter = Scripts::Regions->getNewIterator(sect);
	if (iter==NULL) return;

	char midiarray[50];
	int i=0;
	int loopexit=0;
	do
	{
		iter->parseLine(script1, script2);
		if ((script1[0]!='}')&&(script1[0]!='{'))
		{
			if (!(strcmp("MIDI",script1)))
			{
				midiarray[i]=str2num(script2);
				i++;
			}
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

	safedelete(iter);

	if (i!=0)
	{
		i=rand()%(i);
		playmidi(s, 0, midiarray[i]);
	}
}

void soundeffect(NXWSOCKET s, UI16 sound) // Play sound effect for player to all
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	pc->playSFX(sound);
}

void soundeffect5(NXWSOCKET  s, UI16 sound) // Play sound effect for player only to me
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	pc->playSFX(sound, true);
}


void soundeffect3(P_ITEM pi, UI16 sound)
{
	VALIDATEPI(pi);

	Location pos = pi->getPosition();

	pos.z = 0;

	NxwSocketWrapper sw;
	sw.fillOnline( pi );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWCLIENT ps_i=sw.getClient();
		if(ps_i==NULL) continue;
		P_CHAR pc_j=ps_i->currChar();
		if( ISVALIDPC(pc_j))
		{
			SendPlaySoundEffectPkt(ps_i->toInt(), 0x01, sound, 0x0000, pos);
		}
	}
}

void soundeffect4(NXWSOCKET s, P_ITEM pi, UI16 sound)
{
	VALIDATEPI(pi);

	Location pos = pi->getPosition();

	pos.z = 0;

	SendPlaySoundEffectPkt(s, 0x01, sound, 0x0000, pos);
}

//xan : fast weather function.. maybe we should find a more complete system like the
//old one below!
void weather(NXWSOCKET  s, unsigned char bolt)
{
	UI08 packet[4] = { 0x65, 0xFF, 0x40, 0x20 };

	if (wtype==0) packet[2] = 0x00;
	if (wtype==1) packet[1] = 0x00;
	if (wtype==2) { packet[1] = 0x02; packet[3] = 0xEC; }

	Xsend(s, packet, 4);
//AoS/	Network->FlushBuffer(s);
}

void pweather(NXWSOCKET  s)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	UI08 packet[4] = { 0x65, 0xFF, 0x40, 0x20 };

	if (region[pc->region].wtype==0) packet[2] = 0x00;
	if (region[pc->region].wtype==1) packet[1] = 0x00;
	if (region[pc->region].wtype==2) { packet[1] = 0x02; packet[3] = 0xEC; }

	Xsend(s, packet, 4);
//AoS/	Network->FlushBuffer(s);
}

void sysbroadcast(char *txt, ...) // System broadcast in bold text
//Modified by N6 to use UNICODE packets
{
	UI08 unicodetext[512];

	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
	vsprintf( msg, txt, argptr );
	va_end( argptr );

	int ucl = ( strlen ( msg ) * 2 ) + 2 ;

	char2wchar(msg);
	memcpy(unicodetext, Unicode::temp, ucl);

	UI32 lang = calcserial(server_data.Unicodelanguage[0], server_data.Unicodelanguage[1], server_data.Unicodelanguage[2], 0);
	UI08 sysname[30]={ 0x00, };
	strcpy((char *)sysname, "System");

	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET sock=sw.getSocket();
		if( sock!=INVALID )
		{
			SendUnicodeSpeechMessagePkt(sock, 0x01010101, 0x0101, 6, 0x084D /*0x0040*/, 0x0000, lang, sysname, unicodetext,  ucl);
		}
	}
}


void sysmessage(NXWSOCKET  s, const char *txt, ...) // System message (In lower left corner)
{
	if(s < 0)
		return;

	UI08 unicodetext[512];

	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
    vsprintf( msg, txt, argptr );
	va_end( argptr );

	SERIAL spyTo = clientInfo[s]->spyTo;
	if( spyTo!=INVALID ) { //spy client
		P_CHAR pc=pointers::findCharBySerial( spyTo );
		if( ISVALIDPC( pc ) ) {
			NXWCLIENT gm = pc->getClient();
			if( gm!=NULL )
				gm->sysmsg( "spy %s : %s", pc->getCurrentNameC(), msg );
			else
				clientInfo[s]->spyTo=INVALID;
		}
		else
			clientInfo[s]->spyTo=INVALID;
	}

	int ucl = ( strlen ( msg ) * 2 ) + 2 ;

	char2wchar(msg);
	memcpy(unicodetext, Unicode::temp, ucl);

	UI32 lang = calcserial(server_data.Unicodelanguage[0], server_data.Unicodelanguage[1], server_data.Unicodelanguage[2], 0);
	UI08 sysname[30]={ 0x00, };
	strcpy((char *)sysname, "System");

	SendUnicodeSpeechMessagePkt(s, 0x01010101, 0x0101, 6, 0x0387 /* Color - Previous default was 0x0040 - 0x03E9*/, 0x0003, lang, sysname, unicodetext,  ucl);

}


void sysmessage(NXWSOCKET  s, short color, const char *txt, ...) // System message (In lower left corner)
{
	if( s < 0)
		return;

	UI08 unicodetext[512];

	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
	//vsnprintf( msg, sizeof(msg)-1, txt, argptr );
        vsprintf( msg, txt, argptr );
	va_end( argptr );
	UI16 ucl = ( strlen ( msg ) * 2 ) + 2 ;

	char2wchar(msg);
	memcpy(unicodetext, Unicode::temp, ucl);

	UI32 lang = calcserial(server_data.Unicodelanguage[0], server_data.Unicodelanguage[1], server_data.Unicodelanguage[2], 0);
	UI08 sysname[30]={ 0x00, };
	strcpy((char *)sysname, "System");

	SendUnicodeSpeechMessagePkt(s, 0x01010101, 0x0101, 0, color, 0x0003, lang, sysname, unicodetext,  ucl);

}

void itemmessage(NXWSOCKET  s, char *txt, int serial, short color)
{
// The message when an item is clicked (new interface, Duke)
//Modified by N6 to use UNICODE packets

	UI08 unicodetext[512];
	UI16 ucl = ( strlen ( txt ) * 2 ) + 2 ;

	P_ITEM pi=pointers::findItemBySerial(serial);
	VALIDATEPI(pi);

	if ((pi->type == ITYPE_CONTAINER && color == 0x0000)||
		(pi->type == ITYPE_SPELLBOOK && color == 0x0000)||
		(pi->getId()==0x1BF2 && color == 0x0000))
		color = 0x03B2;

	char2wchar(txt);
	memcpy(unicodetext, Unicode::temp, ucl);

	color = 0x0481; // UOLBR patch to prevent client crash by Juliunus

	UI32 lang = calcserial(server_data.Unicodelanguage[0], server_data.Unicodelanguage[1], server_data.Unicodelanguage[2], 0);
	UI08 sysname[30]={ 0x00, };
	strcpy((char *)sysname, "System");

	SendUnicodeSpeechMessagePkt(s, serial, 0x0101, 6, color, 0x0003, lang, sysname, unicodetext,  ucl);

}

void wearIt(const NXWSOCKET  s, const P_ITEM pi)
{
	VALIDATEPI(pi);

	UI08 wearitem[15]={ 0x2E, 0x00, };

	LongToCharPtr(pi->getSerial32(), wearitem +1);
	ShortToCharPtr(pi->animid(),wearitem +5); // elcabesa animation
	wearitem[7]= 0x00;
	wearitem[8]= pi->layer;
	LongToCharPtr(pi->getContSerial(),wearitem+9);
	ShortToCharPtr(pi->getColor(), wearitem +13);
	Xsend(s, wearitem, 15);
//AoS/	Network->FlushBuffer(s);
}

void backpack2(NXWSOCKET s, SERIAL serial) // Send corpse stuff
{
	int count=0, count2;
	UI08 display1[7]={ 0x89, 0x00, };
	UI08 display2[5]={ 0x00, };
	UI08 bpopen2[5]={ 0x3C, 0x00, };

	P_ITEM cont=pointers::findItemBySerial( serial );

	NxwItemWrapper si;
	si.fillItemsInContainer( cont, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if( ISVALIDPI(pi) && (pi->layer!=0) )
		{
			count++;
		}
	}
	count2=(count*5)+7 + 1 ; // 5 bytes per object, 7 for this header and 1 for terminator
	ShortToCharPtr(count2, display1+1);
	LongToCharPtr(serial, display1+3);
	Xsend(s, display1, 7);

	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if( ISVALIDPI(pi) && (pi->layer!=0) )
		{
			display2[0]= pi->layer;
			LongToCharPtr(pi->getSerial32(), display2+1);
			Xsend(s, display2, 5);
		}
	}
	UI08 nul = 0;
	Xsend(s, &nul, 1);	// Terminate with a 0
//AoS/	Network->FlushBuffer(s);

	ShortToCharPtr(count, bpopen2+3);
	count2=(count*19)+5;
	ShortToCharPtr(count2, bpopen2+1);
	Xsend(s, bpopen2, 5);

	UI08 bpitem[20]={ 0x00, };

	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if( ISVALIDPI(pi) && (pi->layer!=0) )
		{
			LongToCharPtr(pi->getSerial32(), bpitem);
			ShortToCharPtr(pi->animid(), bpitem +4);
			bpitem[6]=0x00;
			ShortToCharPtr(pi->amount, bpitem +7);
			ShortToCharPtr(pi->getPosition().x, bpitem +9);
			ShortToCharPtr(pi->getPosition().y, bpitem +11);
			LongToCharPtr(serial, bpitem +13);
			ShortToCharPtr(pi->getColor(), bpitem +17);
			Xsend(s, bpitem, 19);
		}
	}

//AoS/	Network->FlushBuffer(s);
}

void sendbpitem(NXWSOCKET s, P_ITEM pi)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	Location pi_pos = pi->getPosition();

	unsigned char display3[1]={ 0x25 };
	unsigned char bpitem[20]={ 0x00, };

	LongToCharPtr(pi->getSerial32(), bpitem);
	//AntiChrist - world light sources stuff
	//if player is a gm, this item
	//is shown like a candle (so that he can move it),
	//....if not, the item is a normal
	//invisible light source!
	if(pc->IsGM() && pi->getId()==0x1647)
	{///let's show the lightsource like a candle
		ShortToCharPtr(0x0A0F, bpitem +4);
	} else
	{//else like a normal item
		ShortToCharPtr(pi->animid(), bpitem +4);
	}
	bpitem[6]=0x00;
	ShortToCharPtr(pi->amount, bpitem +7);
	ShortToCharPtr(pi_pos.x, bpitem +9);
	ShortToCharPtr(pi_pos.y, bpitem +11);
	LongToCharPtr(pi->getContSerial(), bpitem +13);
	if(pc->IsGM() && pi->getId()==0x1647)
	{///let's show the lightsource like a blue item
		ShortToCharPtr(0x00C6, bpitem +17);
	}
	else
	{//else like a normal item
		ShortToCharPtr(pi->getColor(), bpitem +17);
	}

	// we need to find the topmost container that the item is in
	// be it a character or another container.

	if( pc->distFrom(pi)<=VISRANGE )
	{
		Xsend(s, display3, 1);
		Xsend(s, bpitem, 19);
	}
//AoS/	Network->FlushBuffer(s);

	weights::NewCalc(pc);	// Ison 2-20-99
}

void MakeGraphicalEffectPkt_(UI08 pkt[28], UI08 type, UI32 src_serial, UI32 dst_serial, UI16 model_id, Location src_pos, Location dst_pos, UI08 speed, UI08 duration, UI08 adjust, UI08 explode )
{
	pkt[1]=type;
	LongToCharPtr(src_serial, pkt +2);
	LongToCharPtr(dst_serial, pkt +6);
	ShortToCharPtr(model_id, pkt +10);
	ShortToCharPtr(src_pos.x, pkt +12);
	ShortToCharPtr(src_pos.y, pkt +14);
	pkt[16]=src_pos.z;
	ShortToCharPtr(dst_pos.x, pkt +17);
	ShortToCharPtr(dst_pos.y, pkt +19);
	pkt[21]=dst_pos.z;
	pkt[22]=speed;
	pkt[23]=duration;
	ShortToCharPtr(0, pkt +24);		//[24] to [25] are not applicable here.
	pkt[26]=adjust; // LB possible client crashfix
	pkt[27]=explode;
}

void tileeffect(int x, int y, int z, char eff1, char eff2, char speed, char loop)
{//AntiChrist

	UI16 eff = (eff1<<8)|(eff2%256);
	UI08 effect[28]={ 0x70, 0x00, };

Location pos1={ x, y, z, 0}, pos2={ 0, 0, 0, 0};

MakeGraphicalEffectPkt_(effect, 0x02, 0, 0, eff, pos1, pos2, speed, loop, 1, 0);

pos1.z=0;

	NxwSocketWrapper sw;
	sw.fillOnline( pos1 );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET sock=sw.getSocket();
		if( sock!=INVALID )
		{
			Xsend(sock, effect, 28);
//AoS/			Network->FlushBuffer(sock);
		}
	}

}

void senditem(NXWSOCKET  s, P_ITEM pi) // Send items (on ground)
{
	VALIDATEPI(pi);

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	bool pack;
	UI16 len;
	UI08 itmput[20]={ 0x1A, 0x00, };

	if ( pi->visible>=1 && !(pc->IsGM()) )
	return;

	// meaning of the item's attribute visible
	// Visible 0 -> visible to everyone
	// Visible 1 -> only visible to owner and gm's (for owners normal for gm's grayish/hidden color)
	// visible 2 -> only visble to gm's (greyish/hidden color)

	if (!pi->isInWorld())
	{
		pack=true;
		if (isCharSerial(pi->getContSerial()))
		{
			P_CHAR pj=pointers::findCharBySerial(pi->getContSerial());
			if (ISVALIDPC(pj))
				pack=false;
		}
		if (pack)
		{
			if (pi->getId()<0x4000) 			// LB client crashfix, dont show multis in BP
								// we should better move it out of pack, but thats
								// only a first bannaid
			{
				sendbpitem(s,pi);
				return;
			}
		}
	}
	else
	if(item_inVisRange(pc,pi) )
	{
		Location pos = pi->getPosition();

		LongToCharPtr(pi->getSerial32() | 0x80000000, itmput +3);

		//if player is a gm, this item
		//is shown like a candle (so that he can move it),
		//....if not, the item is a normal
		//invisible light source!
		if(pc->IsGM() && pi->getId()==0x1647)
		{///let's show the lightsource like a candle
			ShortToCharPtr(0x0A0F, itmput +7);
		} else
		{//else like a normal item
			ShortToCharPtr(pi->animid(), itmput +7); // elcabesa animation tryyy
		}

		ShortToCharPtr(pi->amount, itmput +9);
		ShortToCharPtr(pos.x, itmput +11);
		ShortToCharPtr(pos.y | 0xC000, itmput +13);
		itmput[15]= pos.z;

		if(pc->IsGM() && pi->getId()==0x1647)
		{///let's show the lightsource like a blue item
			ShortToCharPtr(0x00C6, itmput +16);
		} else
		{
			ShortToCharPtr(pi->getColor(), itmput +16);
		}

		itmput[18]=0;

		bool dontsendcandidate=0;
		if (pi->visible==1)
		{
			if (pc->getSerial32()!=pi->getOwnerSerial32())
			{
				dontsendcandidate=1;
				itmput[18]|=0x80;
			}
		}

		if (dontsendcandidate && !pc->IsGM())
			return; // LB 9-12-99, client 1.26.2 visibility correction

		if (pi->visible==2)
		{
			itmput[18]|=0x80;
		}


		if (pi->magic==1)
			itmput[18]|=0x20;
		if (pc->canAllMove())
			itmput[18]|=0x20;
		if ((pi->magic==3 || pi->magic==4) && pc->getSerial32()==pi->getOwnerSerial32())
			itmput[18]|=0x20;

		if (pc->canViewHouseIcon())
		{
			if (pi->getId()>=0x4000 && pi->getId()<=0x40FF) // LB, 25-dec-1999 litle bugfix for treasure multis, ( == changed to >=)
			{
				ShortToCharPtr(0x14F0, itmput +7);
			}
		}

		len = 19;
		if (pi->dir)
		{
			itmput[19]=itmput[18];
			itmput[18]=itmput[17];
			itmput[17]=itmput[16];
			itmput[16]=itmput[15];
			itmput[15]=static_cast<unsigned char>(pi->dir);
			itmput[11]|=0x80;
			len = 20;
		}

		ShortToCharPtr(len, itmput +1);
		Xsend(s, itmput, len);
//AoS/		Network->FlushBuffer(s);
		//pc->sysmsg( "sent item %s %i", pi->getCurrentNameC(), pi->magic );

		if (pi->IsCorpse())
		{
			backpack2(s, pi->getSerial32());
		}
	}

}

// sends item in differnt color and position than it actually is
// used for LSd potions now, LB 5'th nov 1999
void senditem_lsd(NXWSOCKET  s, ITEM i,char color1, char color2, int x, int y, signed char z)
{
	const P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPI(pi);

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	UI16 color = (color1<<8)|(color2%256);
	UI16 len;
	UI08 itmput[20]={ 0x1A, 0x00, };

	if ( pi->visible>=1 && !(pc->IsGM()) ) return; // workaround for missing gm-check client side for visibity since client 1.26.2
	// for lsd we dont need extra work for type 1 as in send_item

	if (pi->isInWorld())
	{
		Location pos = pi->getPosition();

		LongToCharPtr(pi->getSerial32() | 0x80000000, itmput +3);
		ShortToCharPtr(pi->getId(), itmput +7);
		ShortToCharPtr(pi->amount, itmput +9);
		ShortToCharPtr(pos.x, itmput +11);
		ShortToCharPtr(pos.y | 0xC000, itmput +13);
		itmput[15]=z;
		ShortToCharPtr(color, itmput +16);
		itmput[18]=0;

		if (pi->visible==1)
		{
			if (pc->getSerial32()!=pi->getOwnerSerial32())
			{
				itmput[18]|=0x80;
			}
		}
		if (pi->visible==2)
		{
			itmput[18]|=0x80;
		}

		if (pi->visible==3)
		{
			if ((pc->getId() == BODY_GMSTAFF) || !pc->IsGM())
				itmput[18]|=0x80;
		}

		if (pi->magic==1) itmput[18]|=0x20;

		if (pc->canAllMove()) itmput[18]|=0x20;

		if ((pi->magic==3 || pi->magic==4) && pc->getSerial32()==pi->getOwnerSerial32())
			itmput[18]|=0x20;

		if (pc->canViewHouseIcon())
		{
			if (pi->getId()>=0x4000 && pi->getId()<=0x40FF)
			{
				ShortToCharPtr(0x14F0, itmput +7);
			}
		}

		len = 19;

		if (pi->dir)
		{
			itmput[19]=itmput[18];
			itmput[18]=itmput[17];
			itmput[17]=itmput[16];
			itmput[16]=itmput[15];
			itmput[15]=static_cast<unsigned char>(pi->dir);
			itmput[11]|=0x80;
			len = 20;
		}

		ShortToCharPtr(len, itmput +1);
		Xsend(s, itmput, len);
//AoS/		Network->FlushBuffer(s);

		if (pi->IsCorpse() )
		{
			backpack2(s, pi->getSerial32());
		}
	}
}

void chardel (NXWSOCKET  s) // Deletion of character
{
/* PkG 0x85,
 *      0x00 => That character password is invalid.
 *      0x01 => That character doesn't exist.
 *      0x02 => That character is being played right now.
 *      0x03 => That charater is not old enough to delete.
                The character must be 7days old before it can be deleted.
 *      0x04 => That character is currently queued for backup and cannot be
 *              deleted.
 *      0x05 => Couldn't carry out your request.
 */

	int i;
	UI08 delete_error_msg[2] = {0x85, 0x05};
	UI08 delete_resend_char_1[6]={0x86, 0x01, 0x30, 0x00}; // 1 + 2 + 1 + 5*60 = 304 = 0x0130
	UI08 delete_resend_char_2[61];

	P_CHAR TrashMeUp = NULL;
	NxwCharWrapper sc;

	Accounts->GetAllChars( acctno[s], sc );

	for ( i=0, sc.rewind(); !sc.isEmpty(); sc++)
	{
		P_CHAR pc_a=sc.getChar();
		if(!ISVALIDPC(pc_a))
			continue;

		if(i == buffer[s][0x22])
			TrashMeUp = pc_a;

		i++;
	}

	if (ServerScp::g_nPlayersCanDeleteRoles != 0) {
	/// Do Character Deletion ... and return if all ok
		if(!TrashMeUp) {
			delete_error_msg[1] = 0x01;
			Xsend(s, delete_error_msg, 2);
//AoS/			Network->FlushBuffer(s);
			return;
		}

		if (ISVALIDPC(TrashMeUp))
		{
			if( SrvParms->checkcharage &&
			   (getclockday() < TrashMeUp->GetCreationDay() + 7) ) {
				delete_error_msg[1] = 0x03;
				Xsend(s, delete_error_msg, 2);
//AoS/				Network->FlushBuffer(s);
				return;
			}

			if(TrashMeUp->IsOnline()) {
				delete_error_msg[1] = 0x02;
				Xsend(s, delete_error_msg, 2);
//AoS/				Network->FlushBuffer(s);
				return;
			}

			TrashMeUp->Delete();

			Accounts->GetAllChars( acctno[s], sc );

			delete_resend_char_1[3] = sc.size();

			Xsend(s, delete_resend_char_1, 4);

			for ( i=0, sc.rewind(); !sc.isEmpty(); sc++) {
				P_CHAR pc_a=sc.getChar();
				if(!ISVALIDPC(pc_a))
					continue;

				strcpy((char *)delete_resend_char_2, pc_a->getCurrentNameC());
				Xsend(s, delete_resend_char_2, 60);

				i++;
			}

			memset(delete_resend_char_2, 0, 60);

			for (;i<5;i++) {
				Xsend(s, delete_resend_char_2, 60);
			}
//AoS/			Network->FlushBuffer(s);

			return; // All done ;]
		}
	}
// Any possible error ....
	Xsend(s, delete_error_msg, 2);
//AoS/	Network->FlushBuffer(s);
}

void skillwindow(NXWSOCKET s) // Opens the skills list, updated for client 1.26.2b by LB
{

	P_CHAR pc= MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	UI08 skillstart[4]={ 0x3A, 0x00, };
	UI08 skillmid[7]={ 0x00, };
	UI08 skillend[2]={ 0x00, };
	UI16 len;
	char x;

	len = 0x015D;					// Hardcoded -_-;  // hack for that 3 new skills+1.26.2 client, LB 4'th dec 1999
	ShortToCharPtr(len, skillstart +1);
	skillstart[3] = 0x00;				// Type:
							// 0x00 = full list, 0xFF = single skill update,
							// 0x02 = full list with skillcap, 0xDF = single skill update with cap

	Xsend(s, skillstart, 4);
	for (int i=0;i<TRUESKILLS;i++)
	{
		Skills::updateSkillLevel(pc,i);
		ShortToCharPtr(i+1, skillmid +0);
		ShortToCharPtr(pc->skill[i], skillmid +2);
		ShortToCharPtr(pc->baseskill[i], skillmid +4);

		x=pc->lockSkill[i];
		if (x!=0 && x!=1 && x!=2) x=0;
		skillmid[6]=x; // leave it unlocked, regardless
		Xsend(s, skillmid, 7);
	}
	Xsend(s, skillend, 2);
//AoS/	Network->FlushBuffer(s);
}

/*!
\author Xanathar
\brief Updates stats to nearbye players
\param stat stat which changed
*/
void cChar::updateStats(SI32 stat)
{
	checkSafeStats();

	int a = 0, b = 0;

	UI08 updater[9]={ 0xA1, 0x00, };

	switch (stat)
	{
	case 0:
		a=getStrength();
		b=hp;
		break;
	case 1:
		a=in;
		b=mn;
		break;
	case 2:
		a=dx;
		b=stm;
		break;
	}

	updater[0]=0xA1+stat;
	LongToCharPtr(getSerial32(), updater +1);
	ShortToCharPtr(a, updater +5);
	ShortToCharPtr(b, updater +7);

	if (stat == 0)  //Send to all, only if it's Health change
	{
		NxwSocketWrapper sw;
		sw.fillOnline( this, false );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWSOCKET i=sw.getSocket();
			if( i!=INVALID )
			{
				Xsend(i, updater, 9);
//AoS/				Network->FlushBuffer(i);
			}
		}
	} else {
		NXWSOCKET s = getSocket();
		if (s != INVALID)
		{
			Xsend(s, updater, 9);
//AoS/			Network->FlushBuffer(s);
		}
	}
}

void statwindow(P_CHAR pc_to, P_CHAR pc) // Opens the status window
{

	VALIDATEPC(pc);
	VALIDATEPC(pc_to);

	UI32 x;
	UI16 len;
	UI08 statstring[66]={ 0x11, 0x00, };
	bool ghost;


	if ((pc->getId() == BODY_DEADMALE) || (pc->getId() == BODY_DEADFEMALE)) ghost = true; else ghost = false;

	LongToCharPtr(pc->getSerial32(), statstring +3);
	strncpy((char *)&statstring[7], pc->getCurrentNameC(), 30); // can not be more than 30 at least no without changing packet lenght

	if (!ghost)
	{
		ShortToCharPtr(pc->hp, statstring +37);
	}
	else
	{
		ShortToCharPtr(0, statstring +37);
	}

	//Changed, so ghosts can see their maximum hit points.
	ShortToCharPtr(pc->getStrength(), statstring +39);

	if ((pc_to->IsGM() || (pc->getOwnerSerial32()==pc_to->getSerial32())) && (pc_to!=pc))
	{
		statstring[41]=0xFF;
	}
	else if ((pc_to->getSerial32()==pc->getOwnerSerial32())&&(pc_to!=pc)) //Morrolan - from Banter
	{
		statstring[41]=0xFF;
	}
	else
	{
		statstring[41]=0x00;
	}

	if (ghost) statstring[41]=0x00;

	statstring[42]=0x01;					// Flag: 0x00  no more data following (end of packet here)
								//       0x01  more data after this flag following
								//       0x03  like 1, extended info
								//       0x04  even more extended info (client 4.0 and above)

	if (pc->getId() == BODY_FEMALE) statstring[43]=1;
	else if (pc->getId() == BODY_DEADFEMALE) statstring[43]=1;
	else statstring[43]=0; // LB, prevents very female looking male players ... :-)

	//Changed so ghosts can see their str, dex and int, their char haven't lost those attributes.
	ShortToCharPtr(pc->getStrength(), statstring +44);
	ShortToCharPtr(pc->dx, statstring +46);
	ShortToCharPtr(pc->in, statstring +48); // Real INT

	if (!ghost)
	{
		ShortToCharPtr(pc->stm, statstring +50);
		ShortToCharPtr(pc->mn, statstring +54);
	}
	else
	{ // ghosts will see their mana as 0/x, ie 0/100
		ShortToCharPtr(0, statstring +50);
		ShortToCharPtr(0, statstring +54);
	}
	// This will show red bars when status are displayed as percentages (little status window)
	ShortToCharPtr(pc->dx, statstring +52); // MaxStamina
	ShortToCharPtr(pc->in, statstring +56); // MaxMana

	x = pc->CountGold();
	LongToCharPtr(x, statstring +58);

	x = pc->calcDef(0);
	ShortToCharPtr(x, statstring +62); // AC

	x = (int)(pc->weight);
	ShortToCharPtr(x, statstring +64);

	len = 0x0042;
	ShortToCharPtr(len, statstring +1);

	NXWSOCKET s = pc_to->getSocket();
	Xsend(s, statstring, len);
//AoS/	Network->FlushBuffer(s);
}

void updates(NXWSOCKET  s) // Update Window
{
	int x, y, j;
	char temp[512];
    cScpIterator* iter = NULL;
    char script1[1024];

    iter = Scripts::Misc->getNewIterator("SECTION MOTD");
    if (iter==NULL) return;
	strcpy(script1, iter->getEntry()->getFullLine().c_str()); //discard the {

	x=-1;
	y=-2;
	int loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		x++;
		y+=strlen(script1)+1;
	}
	while ( (strcmp(script1, "}")) && (++loopexit < MAXLOOPS) );
	y+=10;
	iter->rewind();
	strcpy(script1, iter->getEntry()->getFullLine().c_str());

	UI08 updscroll[10]={ 0xA6, 0x00, };
	ShortToCharPtr(y, updscroll +1); 		// len of pkt.
	updscroll[3]=2; 				// MOTD ? Type: 0x00 tips, 0x02 updates
	LongToCharPtr(0, updscroll +4);			// tip num.
	ShortToCharPtr(y-10, updscroll +8);		// len of only mess.
	Xsend(s, updscroll, 10); 		// Send 1st part (header)

	for (j=0;j<x;j++)
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		sprintf(temp, "%s ", script1);
		Xsend(s, temp, strlen(temp)); 	// Send the rest
	}
	safedelete(iter);

//AoS/	Network->FlushBuffer(s);
}

void tips(NXWSOCKET s, UI16 i, UI08 want_next) // Tip of the day window
{
	int x, y, j;
	char temp[512];

	cScpIterator* iter = NULL;
	char script1[1024];
	char script2[1024];

	if(want_next) i = i+1;
	else i = i-1;

	if (i==0) i=1;

	iter = Scripts::Misc->getNewIterator("SECTION TIPS");
	if (iter==NULL) return;

	x=i;
	int loopexit=0;
	do
	{
		iter->parseLine(script1, script2);
		if (!(strcmp("TIP", script1))) x--;
	}
	while ((x>0)&&script1[0]!='}'&&script1[0]!=0 && (++loopexit < MAXLOOPS) );

	safedelete(iter);

	if (!(strcmp("}", script1)))
	{
		tips(s, 1, want_next);
		return;
	}

	sprintf(temp, "SECTION TIP %i", str2num(script2));
    iter = Scripts::Misc->getNewIterator(temp);

	if (iter==NULL) return;
	strcpy(script1, iter->getEntry()->getFullLine().c_str());//discards the {

	x=-1;
	y=-2;
	loopexit=0;
	do
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		x++;
		y+=strlen(script1)+1;
	}
	while ( (strcmp(script1, "}")) && (++loopexit < MAXLOOPS) );
	y+=10;
	iter->rewind();
	strcpy(script1, iter->getEntry()->getFullLine().c_str());//discards the {

	UI08 updscroll[10]={ 0xA6, 0x00, };
	ShortToCharPtr(y, updscroll +1); 		// len of pkt.
	updscroll[3]=0; 				// Type: 0x00 tips, 0x02 updates
	LongToCharPtr(i, updscroll +4);			// tip num.
	ShortToCharPtr(y-10, updscroll +8);		// len of only mess.
	Xsend(s, updscroll, 10); 		// Send 1st part (header)

	for (j=0;j<x;j++)
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());//discards the {
		sprintf(temp, "%s ", script1);
		Xsend(s, temp, strlen(temp)); // Send the rest
	}
	safedelete(iter);

//AoS/	Network->FlushBuffer(s);
}


void deny(NXWSOCKET  s, P_CHAR pc, int sequence)
{
	cPacketWalkReject walkdeny;
	walkdeny.sequence= sequence;
	walkdeny.x= pc->getPosition().x;
	walkdeny.y= pc->getPosition().y;
	walkdeny.direction=pc->dir;
	walkdeny.z= pc->getPosition().dispz;
	walkdeny.send( pc->getClient() );
	walksequence[s]=INVALID;
}

void weblaunch(int s, const char *txt) // Direct client to a web page
{
	cPacketWebBrowser launch;

	launch.link+=txt;
	launch.send( getClientFromSocket(s) );
}

void broadcast(int s) // GM Broadcast (Done if a GM yells something)
//Modified by N6 to use UNICODE packets
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int i;
	TEXT nonuni[512];

	if(pc->unicode)
		for (i=13;i<ShortFromCharPtr(buffer[s] +1);i=i+2)
		{
			nonuni[(i-13)/2]=buffer[s][i];
		}
		if(!(pc->unicode))
		{
			UI32 id;
			UI16 model,font, color;

			id = pc->getSerial32();
			model = pc->getId();
			color = ShortFromCharPtr(buffer[s] +4);		// use color from client
			font = (buffer[s][6]<<8)|(pc->fonttype%256);	// use font ("not only") from  client

			UI08 name[30]={ 0x00, };
			strcpy((char *)name, pc->getCurrentNameC());

			NxwSocketWrapper sw;
			sw.fillOnline();
			for( sw.rewind(); !sw.isEmpty(); sw++ )
			{
				NXWSOCKET i=sw.getSocket();

				SendSpeechMessagePkt(i, id, model, 1, color, font, name, (char*)&buffer[s][8]);
			}
		} // end unicode IF
		else
		{
			UI32 id;
			UI16 model,font, color;
			UI08 unicodetext[512];
			UI16 ucl = ( strlen ( &nonuni[0] ) * 2 ) + 2 ;

			char2wchar(&nonuni[0]);
			memcpy(unicodetext, Unicode::temp, ucl);

			id = pc->getSerial32();
			model = pc->getId();
			color = ShortFromCharPtr(buffer[s] +4);		// use color from client
			font = (buffer[s][6]<<8)|(pc->fonttype%256);	// use font ("not only") from  client

			UI32 lang =  LongFromCharPtr(buffer[s] +9);
			UI08 name[30]={ 0x00, };
			strcpy((char *)name, pc->getCurrentNameC());

			NxwSocketWrapper sw;
			sw.fillOnline();
			for( sw.rewind(); !sw.isEmpty(); sw++ )
			{
				NXWSOCKET i=sw.getSocket();
				SendUnicodeSpeechMessagePkt(i, id, model, 1, color, font, lang, name, unicodetext,  ucl);
			}
		}
}

void itemtalk(P_ITEM pi, char *txt)
// Item "speech"
//Modified by N6 to use UNICODE packets
{

	VALIDATEPI(pi);

	NxwSocketWrapper sw;
	sw.fillOnline( pi );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET s=sw.getSocket();
		if(s==INVALID) continue;

		UI08 unicodetext[512];
		UI16 ucl = ( strlen ( txt ) * 2 ) + 2 ;

		char2wchar(txt);
		memcpy(unicodetext, Unicode::temp, ucl);

		UI32 lang = calcserial(server_data.Unicodelanguage[0], server_data.Unicodelanguage[1], server_data.Unicodelanguage[2], 0);
		UI08 name[30]={ 0x00, };
		strcpy((char *)name, pi->getCurrentNameC());

		SendUnicodeSpeechMessagePkt(s, pi->getSerial32(), pi->getId(), 0, 0x0481, 0x0003, lang, name, unicodetext,  ucl);

	}
}

// Last touch: LB 8'th April 2001 for particleSystem

// if UO_3DonlyEffect is true, sta has to be valid and contain particleSystem data (if not, crash)
// for particleSystem data layout see staticeffectUO3d and updated packetDoku
// for old 2d staticeffect stuff , the new (3d client) pararamters UO_3Donlyeffect, sta and skip_old are defaulted in such a way that they behave like they did before
// simply dont set them in that case
// the last parameter is for particlesystem optimization only (dangerous). don't use unless you know 101% what you are doing.

void staticeffect(CHARACTER player, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop,  bool UO3DonlyEffekt, ParticleFx *sta, bool skip_old)
{
	P_CHAR pc=MAKE_CHAR_REF(player);
	VALIDATEPC(pc);

	UI16 eff = (eff1<<8)|(eff2%256);
	UI08 effect[28]={ 0x70, 0x00, };

    	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	 int a0,a1,a2,a3,a4;
	 Location charpos= pc->getPosition();

	 if (!skip_old)
	 {
Location pos2;
pos2.x = 0; pos2.y = 0; pos2.z = 0;
MakeGraphicalEffectPkt_(effect, 0x03, pc->getSerial32(), 0, eff, charpos, pos2, speed, loop, 1, 0);
	 }

	 if (!UO3DonlyEffekt) // no UO3D effect ? lets send old effect to all clients
	 {

		 NxwSocketWrapper sw;
		 sw.fillOnline( pc, false );
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			NXWSOCKET s = sw.getSocket();
			Xsend(s, effect, 28);
//AoS/			Network->FlushBuffer(s);
		 }

	   return;
	}
	else
	{
		// UO3D effect -> let's check which client can see it
	   unsigned char particleSystem[49];


		NxwSocketWrapper sw;
	    sw.fillOnline( pc, false );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			 NXWSOCKET j=sw.getSocket();
			 if(j==INVALID) continue;
			 if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd
			 {
				 Xsend(j, effect, 28);
//AoS/				Network->FlushBuffer(j);
			 } else if (clientDimension[j]==3) // 3d client, send 3d-Particles
			 {
				staticeffectUO3D(player, sta);

				// allow to fire up to 4 layers at same time (like on OSI servers)
				a0 = sta->effect[10];

				a1 = ( ( a0 & 0x000000ff )       );
				a2 = ( ( a0 & 0x0000ff00 ) >> 8  );
				a3 = ( ( a0 & 0x00ff0000 ) >> 16 );
				a4 = ( ( a0 & 0xff000000 ) >> 24 );

				if (a1!=0xff) { particleSystem[46] = a1; Xsend(j, particleSystem, 49); }
				if (a2!=0xff) { particleSystem[46] = a2; Xsend(j, particleSystem, 49); }
				if (a3!=0xff) { particleSystem[46] = a3; Xsend(j, particleSystem, 49); }
				if (a4!=0xff) { particleSystem[46] = a4; Xsend(j, particleSystem, 49); }

//AoS/				Network->FlushBuffer(j);
				//sprintf(temp, "a0: %x a1: %x a2: %x a3: %x a4: %x \n",a0,a1,a2,a3,a4);
				//ConOut(temp);
			 }
			 else if (clientDimension[j] != 2 && clientDimension[j] !=3 ) { sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); } // attention: a simple else is wrong !

	   } // end for
	} // end UO:3D effect

	// remark: if a UO:3D effect is send and ALL clients are UO:3D ones, the pre-calculation of the 2-d packet
	// is redundant. but we can never know, and probably it will take years till the 2d cliet dies.
	// I think it's too infrequnet to consider this as optimization.
}


void movingeffect(CHARACTER source, CHARACTER dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode, bool UO3DonlyEffekt, ParticleFx *str, bool skip_old )
{

	P_CHAR src=MAKE_CHAR_REF(source);
	VALIDATEPC(src);
	P_CHAR dst=MAKE_CHAR_REF(dest);
	VALIDATEPC(dst);

	UI16 eff = (eff1<<8)|(eff2%256);
	UI08 effect[28]={ 0x70, 0x00, };

 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	Location srcpos= src->getPosition();
	Location destpos= dst->getPosition();

	if (!skip_old)
	{
MakeGraphicalEffectPkt_(effect, 0x00, src->getSerial32(), dst->getSerial32(), eff, srcpos, destpos, speed, loop, 0, explode);
	}

	 if (!UO3DonlyEffekt) // no UO3D effect ? lets send old effect to all clients
	 {

		 NxwSocketWrapper sw;
		 sw.fillOnline( );
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			 NXWSOCKET j=sw.getSocket();
			 if ( (char_inVisRange(src,MAKE_CHAR_REF(currchar[j])))&&(char_inVisRange(MAKE_CHAR_REF(currchar[j]),dst))&&(clientInfo[j]->ingame))
			 {
				Xsend(j, effect, 28);
//AoS/				Network->FlushBuffer(j);
			 }
		 }
	   return;
	}
	else
	{
		// UO3D effect -> let's check which client can see it

		NxwSocketWrapper sw;
		sw.fillOnline();
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			 NXWSOCKET j=sw.getSocket();
			 if ( (char_inVisRange(src,MAKE_CHAR_REF(currchar[j])))&&(char_inVisRange(MAKE_CHAR_REF(currchar[j]),dst))&&(clientInfo[j]->ingame))
			 {
				 if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd
				 {
					 Xsend(j, effect, 28);
//AoS/					Network->FlushBuffer(j);
				 } else if (clientDimension[j]==3) // 3d client, send 3d-Particles
				 {

					movingeffectUO3D(source, dest, str);
					unsigned char particleSystem[49];
					Xsend(j, particleSystem, 49);
//AoS/					Network->FlushBuffer(j);
				}
				else if (clientDimension[j] != 2 && clientDimension[j] !=3 ) { sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); }
			}
		}
	}
}

// staticeffect2 is for effects on items
void staticeffect2(P_ITEM pi, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode, bool UO3DonlyEffekt,  ParticleFx *str, bool skip_old )
{
	VALIDATEPI(pi);

	UI16 eff = (eff1<<8)|(eff2%256);
	UI08 effect[28]={ 0x70, 0x00, };

 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	Location pos = pi->getPosition();

	if (!skip_old)
	{
		MakeGraphicalEffectPkt_(effect, 0x02, pi->getSerial32(), pi->getSerial32(), eff, pos, pos, speed, loop, 1, explode);
	}

	if (!UO3DonlyEffekt) // no UO3D effect ? lets send old effect to all clients
	{
		 NxwSocketWrapper sw;
		 sw.fillOnline( pi );
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			 NXWSOCKET j=sw.getSocket();
			 if( j!=INVALID )
			 {
				Xsend(j, effect, 28);
//AoS				Network->FlushBuffer(j);
			 }
		}
		return;
	}
	else
	{
		// UO3D effect -> let's check which client can see it
		 NxwSocketWrapper sw;
		 sw.fillOnline( pi );
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			 NXWSOCKET j=sw.getSocket();
			 if( j!=INVALID )
			 {
				if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd
				{
					Xsend(j, effect, 28);
//AoS/					Network->FlushBuffer(j);
				}
				else if (clientDimension[j]==3) // 3d client, send 3d-Particles
				{
					itemeffectUO3D(pi, str);
					unsigned char particleSystem[49];
					Xsend(j, particleSystem, 49);
//AoS/					Network->FlushBuffer(j);
				}
				else if (clientDimension[j] != 2 && clientDimension[j] !=3 )
				{ sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); }
			}
		}
	}
}


void bolteffect2(CHARACTER player,char a1,char a2)	// experimenatal, lb
{
	P_CHAR pc=MAKE_CHAR_REF(player);
	VALIDATEPC(pc);

	UI16 eff = (a1<<8)|(a2%256);
	UI08 effect[28]={ 0x70, 0x00, };

	int x,y;
	Location charpos = pc->getPosition(), pos2;

	y=rand()%36;
	x=rand()%36;

	if (rand()%2==0) x=x*-1;
	if (rand()%2==0) y=y*-1;
	pos2.x = charpos.x + x;
	pos2.y = charpos.y + y;
	if (pos2.x<0) pos2.x=0;
	if (pos2.y<0) pos2.y=0;
	if (pos2.x>6144) pos2.x=6144;
	if (pos2.y>4096) pos2.y=4096;

charpos.z = 0; pos2.z = 127;
MakeGraphicalEffectPkt_(effect, 0x00, pc->getSerial32(), 0, eff, charpos, pos2, 0, 0, 1, 0);

	// ConOut("bolt: %i %i %i %i %i %i\n",x2,y2,chars[player].x,chars[player].y,x,y);

	 NxwSocketWrapper sw;
	 sw.fillOnline( pc );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
		{
			Xsend(j, effect, 28);
//AoS/			Network->FlushBuffer(j);
		}
	}
}

//	- Movingeffect3 is used to send an object from a char
//    to another object (like purple potions)
void movingeffect3(CHARACTER source, unsigned short x, unsigned short y, signed char z, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode)
{

	P_CHAR src=MAKE_CHAR_REF(source);
	VALIDATEPC(src);

	UI16 eff = (eff1<<8)|(eff2%256);
	UI08 effect[28]={ 0x70, 0x00, };

	Location srcpos= src->getPosition(), pos2 = { x, y, z, 0};

MakeGraphicalEffectPkt_(effect, 0x00, src->getSerial32(), 0, eff, srcpos, pos2, speed, loop, 0, explode);

	 NxwSocketWrapper sw;
	 sw.fillOnline( src );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
		{
			Xsend(j, effect, 28);
//AoS/			Network->FlushBuffer(j);
		}
	 }

}

// staticeffect3 is for effects on items
void staticeffect3(UI16 x, UI16 y, SI08 z, unsigned char eff1, unsigned char eff2, char speed, char loop, char explode)
{
	UI16 eff = (eff1<<8)|(eff2%256);
	UI08 effect[28]={ 0x70, 0x00, };

Location pos = { x, y, z, 0};

MakeGraphicalEffectPkt_(effect, 0x02, 0, 0, eff, pos, pos, speed, loop, 1, explode);

pos.z = 0;

	 NxwSocketWrapper sw;
	 sw.fillOnline( pos );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
		{
			Xsend(j, effect, 28);
//AoS/			Network->FlushBuffer(j);
		}
	}
}

void movingeffect3(CHARACTER source, CHARACTER dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode,unsigned char unk1,unsigned char unk2,unsigned char ajust,unsigned char type)
{
	P_CHAR src=MAKE_CHAR_REF(source);
	VALIDATEPC(src);
	P_CHAR dst=MAKE_CHAR_REF(dest);
	VALIDATEPC(dst);


	//0x0f 0x42 = arrow 0x1b 0xfe=bolt
	UI16 eff = (eff1<<8)|(eff2%256);
	UI08 effect[28]={ 0x70, 0x00, };

	Location srcpos= src->getPosition();
	Location destpos= dst->getPosition();

MakeGraphicalEffectPkt_(effect, type, src->getSerial32(), dst->getSerial32(), eff, srcpos, destpos, speed, loop, ajust, explode);

	 NxwSocketWrapper sw;
	 sw.fillOnline( );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
		{
			Xsend(j, effect, 28);
//AoS/			Network->FlushBuffer(j);
		}
	}
}



//	- Movingeffect2 is used to send an object from a char
//	to another object (like purple potions)
void movingeffect2(CHARACTER source, int dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode)
{
	//0x0f 0x42 = arrow 0x1b 0xfe=bolt

	const P_ITEM pi=MAKE_ITEM_REF(dest);
	VALIDATEPI(pi);
	P_CHAR pc_source = MAKE_CHAR_REF(source);
	VALIDATEPC(pc_source);

	UI16 eff = (eff1<<8)|(eff2%256);
	UI08 effect[28]={ 0x70, 0x00, };

	Location srcpos= pc_source->getPosition(), pos2 = pi->getPosition();

MakeGraphicalEffectPkt_(effect, 0x00, pc_source->getSerial32(), pi->getSerial32(), eff, srcpos, pos2, speed, loop, 0, explode);

	 NxwSocketWrapper sw;
	 sw.fillOnline( );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
		{
			Xsend(j, effect, 28);
//AoS/			Network->FlushBuffer(j);
		}
	}
}

void dolight(NXWSOCKET s, char level)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	UI08 light[2]={ 0x4F, 0x00 };

	if ((s==INVALID)||(!clientInfo[s]->ingame)) return;

	light[1]=level;
	if (worldfixedlevel!=255)
	{
		light[1]=worldfixedlevel;
	} else {
		if (pc->fixedlight!=255)
		{
			light[1]=pc->fixedlight;
		} else {
			if (indungeon(pc))
			{
				light[1]=dungeonlightlevel;
			}
			else
			{
				light[1]=level;
			}
		}
	}

	Xsend(s, light, 2);
//AoS/	Network->FlushBuffer(s);
}

void updateskill(NXWSOCKET s, int skillnum) // updated for client 1.26.2b by LB
{

	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_currchar);

	UI16 len;
	UI08 update[11]={ 0x3A, 0x00, };
	char x;

	len = 11; // Length of message

	update[3] = 0xFF; // single list

	update[4] = 0x00;
	update[5] = (char)skillnum;
	ShortToCharPtr(pc_currchar->skill[skillnum], update +6);
	ShortToCharPtr(pc_currchar->baseskill[skillnum], update +8);

	x = pc_currchar->lockSkill[skillnum];
	if (x != 0 && x != 1 && x != 2)
		x = 0;
	update[10] = x;

	// CRASH_IF_INVALID_SOCK(s);

	ShortToCharPtr(len, update +1);
	Xsend(s, update, 11);
//AoS/	Network->FlushBuffer(s);
}

void deathaction(P_CHAR pc, P_ITEM pi)
{
	UI08 deathact[13]={ 0xAF, 0x00, };

	LongToCharPtr(pc->getSerial32(), deathact +1);
	LongToCharPtr(pi->getSerial32(), deathact +5);

	 NxwSocketWrapper sw;
	 sw.fillOnline( pc, true );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET i=sw.getSocket();
		if( i!=INVALID )
		{
			Xsend(i, deathact, 13);
//AoS/			Network->FlushBuffer(i);
		}
	}
}

void deathmenu(NXWSOCKET s) // Character sees death menu
{
	UI08 testact[2]={ 0x2C, 0x00 };
	Xsend(s, testact, 2);
//AoS/	Network->FlushBuffer(s);
}

void SendPauseResumePkt(NXWSOCKET s, UI08 flag)
{
/* Flag: 0=pause, 1=resume */ // uhm.... O_o ... or viceversa ? -_-;
	UI08 m2[2]={ 0x33, 0x00 };

	m2[1]=flag;
	Xsend(s, m2, 2);
//AoS/	Network->FlushBuffer(s);
}

void SendDeleteObjectPkt(NXWSOCKET s, SERIAL serial)
{
	UI08 removeitem[5] = { 0x1D, 0x00, };
	LongToCharPtr(serial, removeitem +1);

	Xsend(s, removeitem, 5);
//AoS/	Network->FlushBuffer(s);
}

void SendDrawGamePlayerPkt(NXWSOCKET s, UI32 player_id, UI16 model, UI08 unk1, UI16 color, UI08 flag, Location pos, UI16 unk2, UI08 dir, bool useDispZ)
{
	UI08 goxyz[19]={ 0x20, 0x00, };

	LongToCharPtr(player_id, goxyz +1);
	ShortToCharPtr(model, goxyz +5);
	goxyz[7] = unk1;
	ShortToCharPtr(color, goxyz +8);
	goxyz[10] = flag;
	ShortToCharPtr(pos.x, goxyz +11);
	ShortToCharPtr(pos.y, goxyz +13);
	ShortToCharPtr(unk2, goxyz +15);
	goxyz[17]= dir;
	goxyz[18]= (useDispZ)? pos.dispz : pos.z;
	Xsend(s, goxyz, 19);
//AoS/	Network->FlushBuffer(s);
}

void SendUpdatePlayerPkt(NXWSOCKET s, UI32 player_id, UI16 model, Location pos, UI08 dir, UI16 color, UI08 flag, UI08 hi_color)
{
	UI08 extmove[17]={ 0x77, 0x00 };

	LongToCharPtr(player_id, extmove +1);
	ShortToCharPtr(model, extmove +5);
	ShortToCharPtr(pos.x, extmove +7);
	ShortToCharPtr(pos.y, extmove +9);
	extmove[11]=pos.dispz;			// ??!?!?!?!? .z ?!
	extmove[12]=dir;
	ShortToCharPtr(color, extmove +13);
	extmove[15]=flag;
	extmove[16]=hi_color;

	Xsend(s, extmove, 17);
//AoS/	Network->FlushBuffer(s);
}

void SendDrawObjectPkt(NXWSOCKET s, P_CHAR pc, int z)
{
	P_CHAR pc_currchar=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_currchar);
	UI32 k;
	UI08 oc[1024]={ 0x78, 0x00, };

	Location charpos = pc->getPosition();

	LongToCharPtr(pc->getSerial32(), oc +3);
	ShortToCharPtr(pc->getId(), oc +7); 	// Character art id
	ShortToCharPtr(charpos.x, oc+9);
	ShortToCharPtr(charpos.y, oc+11);
	if (z)
		oc[13]= charpos.dispz; 			// Character z position
	else
		oc[13]= charpos.z;
	oc[14]= pc->dir; 				// Character direction
	ShortToCharPtr(pc->getColor(), oc +15);	// Character skin color
	oc[17]=0; 					// Character flags
	if (pc->IsHidden() || !(pc->IsOnline()||pc->npc))
		oc[17]|=0x80; 				// .... show hidden state correctly
	if (pc->poisoned)
		oc[17]|=0x04; //AntiChrist -- thnx to SpaceDog

	k=19;
	int guild;
	guild=Guilds->Compare(pc_currchar,pc);
	if (guild==1)					//Same guild (Green)
		oc[18]=2;
	else if (guild==2) 				// Enemy guild.. set to orange
		oc[18]=5;
	else if (pc->IsGrey()) oc[18] = 3;
	else switch(pc->flag)
	{//1=blue 2=green 5=orange 6=Red 7=Transparent(Like skin 66 77a)
		case 0x01: oc[18]=6; break;// If a bad, show as red.
		case 0x04: oc[18]=1; break;// If a good, show as blue.
		case 0x08: oc[18]=2; break; //green (guilds)
		case 0x10: oc[18]=5; break;//orange (guilds)
		default: oc[18]=3; break;//grey (Can be pretty much any number.. I like 3 :-)
	}

	for (int j=0;j<MAXLAYERS;j++) layers[j] = 0;

	NxwItemWrapper si;
	si.fillItemWeared( pc, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ ) {

		P_ITEM pj=si.getItem();
		if (ISVALIDPI(pj))
			if ( layers[pj->layer] == 0 )
			{
				LongToCharPtr(pj->getSerial32(), oc+k+0);
				ShortToCharPtr(pj->getId(), oc+k+4);
				oc[k+6]=pj->layer;
				k += 7;
				if (pj->getColor() != 0)
				{
					oc[k-3]|=0x80;
					ShortToCharPtr(pj->getColor(), oc+k);
					k+= 2;
				}
				layers[pj->layer] = 1;
			}
	}

	UI32 ser = 0; 	// Not well understood. It's a serial number. I set this to my serial number,
			// and all of my messages went to my paperdoll gump instead of my character's
			// head, when I was a character with serial number 0 0 0 1.
	LongToCharPtr(ser, oc+k);
	k=k+4;
	// unimportant remark: its a packet "terminator" !!! LB

	ShortToCharPtr(k, oc +1);
	Xsend(s, oc, k);
//AoS/	Network->FlushBuffer(s);
}

void SendSecureTradingPkt(NXWSOCKET s, UI08 action, UI32 id1, UI32 id2, UI32 id3)
{
	UI16 len;
	UI08 msg[17]={ 0x6F, 0x00, };


	len = 17;		//Size - no name in this message -  so len is fixed
	msg[3]=action;		//State
	LongToCharPtr(id1, msg +4);
	LongToCharPtr(id2, msg +8);
	LongToCharPtr(id3, msg +12);
	msg[16]=0; 		// No name in this message

	ShortToCharPtr(len, msg +1);
	Xsend(s, msg, len);
//AoS/	Network->FlushBuffer(s);
}

void SendSpeechMessagePkt(NXWSOCKET s, UI32 id, UI16 model, UI08 type, UI16 color, UI16 fonttype, UI08 sysname[30],  char *text)
{
        UI16 tl, len = strlen((char *)text) + 1;
        UI08 talk[14]={ 0x1C, 0x00, };

        tl = 14 + 30  + len;  // 44(header) + len + null term.

	ShortToCharPtr(tl, talk +1);
	LongToCharPtr(id, talk +3);
	ShortToCharPtr(model, talk +7);
	talk[9]=type;
	ShortToCharPtr(color, talk +10);
	ShortToCharPtr(fonttype, talk +12);

	Xsend(s, talk, 14);
	Xsend(s, sysname, 30);
	Xsend(s, text, len);
//AoS/	Network->FlushBuffer(s);
}


void SendUnicodeSpeechMessagePkt(NXWSOCKET s, UI32 id, UI16 model, UI08 type, UI16 color, UI16 fonttype, UI32 lang, UI08 sysname[30], UI08 *unicodetext, UI16 unicodelen)
{
	UI16 tl;
	UI08 talk2[18]={ 0xAE, 0x00, };

	tl = 18 + 30 + unicodelen;

	ShortToCharPtr(tl, talk2 +1);
	LongToCharPtr(id, talk2 +3);
	ShortToCharPtr(model, talk2 +7);
	talk2[9]=type;
	ShortToCharPtr(color, talk2 +10);
	ShortToCharPtr(fonttype, talk2 +12);
	LongToCharPtr(lang, talk2 +14);

	Xsend(s, talk2, 18);
	Xsend(s, sysname, 30);
	Xsend(s, unicodetext, unicodelen);
//AoS/	Network->FlushBuffer(s);
}

void SendPlaySoundEffectPkt(NXWSOCKET s, UI08 mode, UI16 sound_model, UI16 unkn, Location pos, bool useDispZ)
{
	UI08 sfx[12]={ 0x54, 0x00, };
	SI16 Z;

	Z = (useDispZ)? pos.dispz : pos.z;

	sfx[1] = mode;					// Mode: 0x00 repeating, 0x01 single
	ShortToCharPtr(sound_model, sfx +2);		// Sound model
	ShortToCharPtr(unkn, sfx +4);			// unkn, (speed/volume modifier? Line of sight stuff?)
	ShortToCharPtr(pos.x, sfx +6);			// POS:  X
	ShortToCharPtr(pos.y, sfx +8);			//       Y
	ShortToCharPtr(Z , sfx +10);			//       Z
	Xsend(s, sfx, 12);
//AoS/	Network->FlushBuffer(s);
}

void impowncreate(NXWSOCKET s, P_CHAR pc, int z) //socket, player to send
{
        if ( s < 0 || s > now ) // Luxor
		return;
	P_CHAR pc_currchar=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_currchar);

	if (pc->isStabled() || pc->mounted)
		return; // dont **show** stabled pets

	bool sendit = true; //Luxor bug fix
	if (pc->IsHidden() && pc->getSerial32()!=pc_currchar->getSerial32() && !pc_currchar->IsGM())
		sendit=false;

	if( !pc->npc && !pc->IsOnline()  && !pc_currchar->IsGM() )
	{
		sendit=false;
		SendDeleteObjectPkt(s, pc->getSerial32());
	}
	// hidden chars can only be seen "grey" by themselves or by gm's
	// other wise they are invisible=dont send the packet
	if (!sendit)
		return;

	SendDrawObjectPkt(s, pc, z);
	//pc_currchar->sysmsg( "sended %s", pc->getCurrentNameC() );
}

void sendshopinfo(int s, int c, P_ITEM pi)
{
	VALIDATEPI(pi);

	char cFoundItems=0;
	int k, m1t, m2t, value,serial;

	UI08 itemname[256]={ 0x00, };

	UI08 m1[6096]={ 0x3C, 0x00, };	// Container content message

	UI08 m2[6096]={ 0x74, 0x00, };	// Buy window details message

	LongToCharPtr(pi->getSerial32(), m2+3); //Container serial number
	m2[7]=0; // Count of items;
	m1t=5;
	m2t=8;
	serial= pi->getSerial32();

	NxwItemWrapper si;
	si.fillItemsInContainer( pi, false );
	int loopexit=0;
	for( si.rewind(); !si.isEmpty(); si++, ++loopexit )
	{
		P_ITEM pj=si.getItem();
		if (ISVALIDPI(pj))
			if ((m2[7]!=255) && (pj->amount!=0) ) // 255 items max per shop container
			{
				UI08 namelen;
				if (m2t>6000 || m1t>6000) break;

				LongToCharPtr(pj->getSerial32(), m1+m1t+0);//Item serial number
				ShortToCharPtr(pj->getId(), m1+m1t+4);
				m1[m1t+6]=0;			//Always zero
				ShortToCharPtr(pj->amount, m1+m1t+7); //Amount for sale
				ShortToCharPtr(loopexit, m1+m1t+9);
				ShortToCharPtr(loopexit, m1+m1t+11);
				LongToCharPtr(pi->getSerial32(), m1+m1t+13); //Container serial number
				ShortToCharPtr(pj->getColor(), m1+m1t+17);
				m1[4]++; // Increase item count.
				m1t += 19;
				value=pj->value;
				value=calcValue(DEREF_P_ITEM(pj), value);
				if (SrvParms->trade_system==1)
					value=calcGoodValue(c,DEREF_P_ITEM(pj),value,0); // by Magius(CHE)
				LongToCharPtr(value, m2+m2t+0);		// Item value/price
				namelen = pj->getName((char *)itemname);
				m2[m2t+4]=namelen; 			// Item name length

				for(k=0;k<namelen;k++)
				{
				  	m2[m2t+5+k]=itemname[k];
				}

				m2t += namelen +5;
				m2[7]++;
				cFoundItems=1; //we found items so send message
			}
	}

	ShortToCharPtr(m1t, m1 +1); // Size of message
	ShortToCharPtr(m2t, m2 +1); // Size of message

	if (cFoundItems==1)
	{
		Xsend(s, m1, m1t);
		Xsend(s, m2, m2t);
//AoS/		Network->FlushBuffer(s);
	}
}

int sellstuff(NXWSOCKET s, CHARACTER i)
{
	if (s < 0 || s >= now) return 0; //Luxor
    P_CHAR pc = MAKE_CHAR_REF(i);
	VALIDATEPCR(pc, 0);
	P_CHAR pcs = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pcs,0);

	char itemname[256];
	int m1t, z, value;
	int serial,serial1;
	char ciname[256]; // By Magius(CHE)
	char cinam2[256]; // By Magius(CHE)

	serial=pc->getSerial32();
	/*for (ci=0;ci<pointers::pContMap[serial].size();ci++)
	{*/
	//<Luxor>

	P_ITEM pp=pc->GetItemOnLayer(LAYER_TRADE_BOUGHT);
	VALIDATEPIR(pp,0);

	SendPauseResumePkt(s, 0x01);

	P_ITEM pack= pcs->getBackpack();
	VALIDATEPIR(pack, 0);

	UI08 m1[2048]={ 0x9E, 0x00, };

	LongToCharPtr(pc->getSerial32(), m1 +3);
	ShortToCharPtr(0, m1 +7);	// Num items  m1[7],m1[8]

	m1t=9;

	serial= pp->getSerial32();
	serial1= pack->getSerial32();

	NxwItemWrapper s_pack;
	s_pack.fillItemsInContainer( pack, false );

	NxwItemWrapper si;
	si.fillItemsInContainer( pp, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pj=si.getItem();
		if (ISVALIDPI(pj))
		{

			for( s_pack.rewind(); !s_pack.isEmpty(); s_pack++ )
			{
				if (m1[8] >= 50) continue;

				P_ITEM pj1 = s_pack.getItem();
				if (ISVALIDPI(pj1)) // LB crashfix
				{
					sprintf(ciname,"'%s'",pj1->getCurrentNameC()); // Added by Magius(CHE)
					sprintf(cinam2,"'%s'",pj->getCurrentNameC()); // Added by Magius(CHE)
					strupr(ciname); // Added by Magius(CHE)
					strupr(cinam2); // Added by Magius(CHE)

					if (pj1->getId()==pj->getId()  &&
						pj1->type==pj->type &&
						((SrvParms->sellbyname==0)||(SrvParms->sellbyname==1 && (!strcmp(ciname,cinam2))))) // If the names are the same! --- Magius(CHE)
					{
						UI08 namelen;
						LongToCharPtr(pj1->getSerial32(), m1+m1t+0);
						ShortToCharPtr(pj1->getId(),m1+m1t+4);
						ShortToCharPtr(pj1->getColor(),m1+m1t+6);
						ShortToCharPtr(pj1->amount,m1+m1t+8);
						value=pj->value;
						value=calcValue(DEREF_P_ITEM(pj1), value);
						if (SrvParms->trade_system==1)
							value=calcGoodValue(i,DEREF_P_ITEM(pj1),value,1); // by Magius(CHE)
						ShortToCharPtr(value, m1+m1t+10);
						namelen = pj1->getName(itemname);
						m1[m1t+12]=0;// Unknown... 2nd length byte for string?
						m1[m1t+13] = namelen;
						m1t += 14;
						for(z=0;z<namelen;z++)
						{
							m1[m1t+z]=itemname[z];
						}
						m1t += namelen;
						m1[8]++;
					}
				}
			}
		}
	}

	ShortToCharPtr(m1t, m1 +1);

	if (m1[8]<51) //With too many items, server crashes
	{
		if (m1[8]!=0)
		{
			Xsend(s, m1, m1t);
//AoS/			Network->FlushBuffer(s);
		}
		else
		{
			pc->talkAll(TRANSLATE("Thou dont posses nothing of interest to me."),0);
		}
	}
	else
	{
			pc->talkAll( TRANSLATE("Sorry i cannot take so many items.."),0);
	}

	SendPauseResumePkt(s, 0x00);

	return 1;
}

void playmidi(int s, char num1, char num2)
{
	UI16 music_id = (num1<<8)|(num2%256);
	UI08 msg[3] = { 0x06D, 0x00, };

	ShortToCharPtr(music_id, msg +1);
	Xsend(s, msg, 3);
//AoS/	Network->FlushBuffer(s);
}

void sendtradestatus(P_ITEM c1, P_ITEM c2)
{
	VALIDATEPI(c1);
	VALIDATEPI(c2);

	NXWSOCKET s1, s2;

	P_CHAR p1, p2;

	p1 = pointers::findCharBySerial(c1->getContSerial());
	VALIDATEPC(p1);
	p2 = pointers::findCharBySerial(c2->getContSerial());
	VALIDATEPC(p2);

	s1 = p1->getSocket();
	s2 = p2->getSocket();

	SendSecureTradingPkt(s1, 0x02, c1->getSerial32(), (UI32) (c1->morez%256), (UI32) (c2->morez%256));
	SendSecureTradingPkt(s2, 0x02, c2->getSerial32(), (UI32) (c2->morez%256), (UI32) (c1->morez%256));

}

void endtrade(SERIAL serial)
{
	P_ITEM c1=pointers::findItemBySerial(serial);
	VALIDATEPI(c1);
	P_ITEM c2=pointers::findItemBySerial(calcserial(c1->moreb1, c1->moreb2, c1->moreb3, c1->moreb4));
	VALIDATEPI(c2);

	P_CHAR pc1=pointers::findCharBySerial(c1->getContSerial());
	VALIDATEPC(pc1);

	P_CHAR pc2=pointers::findCharBySerial(c2->getContSerial());
	VALIDATEPC(pc2);


	P_ITEM bp1= pc1->getBackpack();
	VALIDATEPI(bp1);
	P_ITEM bp2= pc2->getBackpack();
	VALIDATEPI(bp2);

	NXWSOCKET s1 = pc1->getSocket();
	NXWSOCKET s2 = pc2->getSocket();

	if (s1 > -1)	// player may have been disconnected (Duke)
		SendSecureTradingPkt(s1, 0x01, c1->getSerial32(), 0, 0);

	if (s2 > -1)	// player may have been disconnected (Duke)
		SendSecureTradingPkt(s2, 0x01, c2->getSerial32(), 0, 0);


	NxwItemWrapper si;
	si.fillItemsInContainer( c1, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pj=si.getItem(); //</Luxor>
		if (ISVALIDPI(pj))
		{
			pj->setContSerial(bp1->getSerial32());

			pj->SetRandPosInCont( bp1 );
			if (s1!=INVALID)
				pj->Refresh();
		}
	}

	NxwItemWrapper si2;
	si2.fillItemsInContainer( c2, false );
	for( si2.rewind(); !si2.isEmpty(); si2++ )
	{

		P_ITEM pj=si2.getItem();
		if (ISVALIDPI(pj))
		{
			pj->setContSerial( bp2->getSerial32() );

			pj->SetRandPosInCont( bp2 );
			if (s2!=INVALID)
				pj->Refresh();
		}
	}

	c1->Delete();
	c2->Delete();
}

void tellmessage(int i, int s, char *txt)
//Modified by N6 to use UNICODE packets
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	UI08 unicodetext[512];
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	sprintf(temp, TRANSLATE("GM tells %s: %s"), pc->getCurrentNameC(), txt);

	UI16 ucl = ( strlen ( temp ) * 2 ) + 2 ;
	char2wchar(temp);
	memcpy(unicodetext, Unicode::temp, ucl);

	UI32 lang = calcserial(server_data.Unicodelanguage[0], server_data.Unicodelanguage[1], server_data.Unicodelanguage[2], 0);
	UI08 sysname[30]={ 0x00, };
	strcpy((char *)sysname, "System");

	SendUnicodeSpeechMessagePkt(s, 0x01010101, 0x0101, 0, 0x0035, 0x0003, lang, sysname, unicodetext,  ucl);
	SendUnicodeSpeechMessagePkt(i, 0x01010101, 0x0101, 0, 0x0035, 0x0003, lang, sysname, unicodetext,  ucl); //So Person who said it can see too

}



// particleSystem core functions, LB 2-April 2001

// sta_str layout:

// 0..3 already used in 2d-staticeffect
// effect 4  -> tile1
// effect 5  -> tile2
// effect 6  -> speed1
// effect 7  -> speed1
// effect 8  -> effect1
// effect 9  -> effect2
// effect 10 -> reserved, dont use
// effect 11 ->
// effect 12 ->


void staticeffectUO3D(CHARACTER player, ParticleFx *sta)
{

   PC_CHAR pc_cs=MAKE_CHAR_REF(player);
   VALIDATEPC(pc_cs);
   Location charpos= pc_cs->getPosition();

   // please no optimization of p[...]=0's yet :)

   unsigned char particleSystem[49];
   particleSystem[0]= 0xc7;
   particleSystem[1]= 0x3;

   particleSystem[2]= pc_cs->getSerial().ser1;
   particleSystem[3]= pc_cs->getSerial().ser2;
   particleSystem[4]= pc_cs->getSerial().ser3;
   particleSystem[5]= pc_cs->getSerial().ser4;

   particleSystem[6]= 0x0; // always 0 for this type
   particleSystem[7]= 0x0;
   particleSystem[8]= 0x0;
   particleSystem[9]= 0x0;

   particleSystem[10]= sta->effect[4]; // tileid1
   particleSystem[11]= sta->effect[5]; // tileid2

   particleSystem[12]= (charpos.x)>>8;
   particleSystem[13]= (charpos.x)%256;
   particleSystem[14]= (charpos.y)>>8;
   particleSystem[15]= (charpos.y)%256;
   particleSystem[16]= (charpos.z);

   particleSystem[17]= (charpos.x)>>8;
   particleSystem[18]= (charpos.x)%256;
   particleSystem[19]= (charpos.y)>>8;
   particleSystem[20]= (charpos.y)%256;
   particleSystem[21]= (charpos.z);

   particleSystem[22]= sta->effect[6]; // unkown1
   particleSystem[23]= sta->effect[7]; // unkown2

   particleSystem[24]=0x0; // only non zero for type 0
   particleSystem[25]=0x0;

   particleSystem[26]=0x1;
   particleSystem[27]=0x0;

   particleSystem[28]=0x0;
   particleSystem[29]=0x0;
   particleSystem[30]=0x0;
   particleSystem[31]=0x0;
   particleSystem[32]=0x0;
   particleSystem[33]=0x0;
   particleSystem[34]=0x0;
   particleSystem[35]=0x0;

   particleSystem[36]=sta->effect[8]; // effekt #
   particleSystem[37]=sta->effect[9];

   particleSystem[38]=sta->effect[11];
   particleSystem[39]=sta->effect[12];

   particleSystem[40]=0x00;
   particleSystem[41]=0x00;

   particleSystem[42]=pc_cs->getSerial().ser1;
   particleSystem[43]=pc_cs->getSerial().ser2;
   particleSystem[44]=pc_cs->getSerial().ser3;
   particleSystem[45]=pc_cs->getSerial().ser4;

   particleSystem[46]=0; // layer, gets set afterwards for multi layering

   particleSystem[47]=0x0; // has to be always 0 for all types
   particleSystem[48]=0x0;


}

// ParticleFx layout:
// 0..4 already used in 2d-move_effect

// effect 5  -> tile1
// effect 6  -> tile2
// effect 7  -> speed1
// effect 8  -> speed2
// effect 9  -> effect1
// effect 10 -> effect2
// effect 11 -> impact effect1
// effect 12 -> impact effect2
// effect 13 -> unkown1, does nothing, but gets set on OSI shards
// effect 14 -> unkown2
// effect 15 -> adjust
// effect 16 -> explode on impact

void movingeffectUO3D(CHARACTER source, CHARACTER dest, ParticleFx *sta)
{


   PC_CHAR pc_cs=MAKE_CHAR_REF(source);
   VALIDATEPC(pc_cs);
   PC_CHAR pc_cd=MAKE_CHAR_REF(dest);
   VALIDATEPC(pc_cd);

   Location srcpos= pc_cs->getPosition();
   Location destpos= pc_cd->getPosition();

   unsigned char particleSystem[49];
   particleSystem[0]=0xc7;
   particleSystem[1]=0x0;

   particleSystem[2]=pc_cs->getSerial().ser1;
   particleSystem[3]=pc_cs->getSerial().ser2;
   particleSystem[4]=pc_cs->getSerial().ser3;
   particleSystem[5]=pc_cs->getSerial().ser4;

   particleSystem[6]=pc_cd->getSerial().ser1;
   particleSystem[7]=pc_cd->getSerial().ser2;
   particleSystem[8]=pc_cd->getSerial().ser3;
   particleSystem[9]=pc_cd->getSerial().ser4;

   particleSystem[10]=sta->effect[5]; // tileid1
   particleSystem[11]=sta->effect[6]; // tileid2

   particleSystem[12]= (srcpos.x)>>8;
   particleSystem[13]= (srcpos.x)%256;
   particleSystem[14]= (srcpos.y)>>8;
   particleSystem[15]= (srcpos.y)%256;
   particleSystem[16]= (srcpos.z);

   particleSystem[17]= (destpos.x)>>8;
   particleSystem[18]= (destpos.x)%256;
   particleSystem[19]= (destpos.y)>>8;
   particleSystem[20]= (destpos.y)%256;
   particleSystem[21]= (destpos.z);

   particleSystem[22]= sta->effect[7]; // speed1
   particleSystem[23]= sta->effect[8]; // speed2

   particleSystem[24]=0x0;
   particleSystem[25]=0x0;

   particleSystem[26]=sta->effect[15]; // adjust
   particleSystem[27]=sta->effect[16]; // explode

   particleSystem[28]=0x0;
   particleSystem[29]=0x0;
   particleSystem[30]=0x0;
   particleSystem[31]=0x0;
   particleSystem[32]=0x0;
   particleSystem[33]=0x0;
   particleSystem[34]=0x0;
   particleSystem[35]=0x0;

   particleSystem[36]=sta->effect[9]; //  moving effekt
   particleSystem[37]=sta->effect[10];
   particleSystem[38]=sta->effect[11]; // effect on explode
   particleSystem[39]=sta->effect[12];

   particleSystem[40]=sta->effect[13]; // ??
   particleSystem[41]=sta->effect[14];

   particleSystem[42]=0x00;
   particleSystem[43]=0x00;
   particleSystem[44]=0x00;
   particleSystem[45]=0x00;

   particleSystem[46]=0xff; // layer, has to be 0xff in that modus

   particleSystem[47]=sta->effect[17];
   particleSystem[48]=0x0;


}

// same sta-layout as staticeffectuo3d
void itemeffectUO3D(P_ITEM pi, ParticleFx *sta)
{
	// please no optimization of p[...]=0's yet :)

	VALIDATEPI(pi);

	unsigned char particleSystem[49];
	particleSystem[0]=0xc7;
	particleSystem[1]=0x2;

	if ( !sta->effect[11] )
	{
		particleSystem[2]= pi->getSerial().ser1;
		particleSystem[3]= pi->getSerial().ser2;
		particleSystem[4]= pi->getSerial().ser3;
		particleSystem[5]= pi->getSerial().ser4;
	}
	else
	{
		particleSystem[2]=0x00;
		particleSystem[3]=0x00;
		particleSystem[4]=0x00;
		particleSystem[5]=0x00;
	}

	particleSystem[6]=0x0; // always 0 for this type
	particleSystem[7]=0x0;
	particleSystem[8]=0x0;
	particleSystem[9]=0x0;

	particleSystem[10]=sta->effect[4]; // tileid1
	particleSystem[11]=sta->effect[5]; // tileid2

	particleSystem[12]= pi->getPosition("x") >> 8;
	particleSystem[13]= pi->getPosition("x") % 256;
	particleSystem[14]= pi->getPosition("y") >> 8;
	particleSystem[15]= pi->getPosition("y") % 256;
	particleSystem[16]= pi->getPosition("z");

	particleSystem[17]= pi->getPosition("x") >> 8;
	particleSystem[18]= pi->getPosition("x") % 256;
	particleSystem[19]= pi->getPosition("y") >> 8;
	particleSystem[20]= pi->getPosition("y") % 256;
	particleSystem[21]= pi->getPosition("z") ;

	particleSystem[22]= sta->effect[6]; // unkown1
	particleSystem[23]= sta->effect[7]; // unkown2

	particleSystem[24]=0x0; // only non zero for type 0
	particleSystem[25]=0x0;

	particleSystem[26]=0x1;
	particleSystem[27]=0x0;

	particleSystem[28]=0x0;
	particleSystem[29]=0x0;
	particleSystem[30]=0x0;
	particleSystem[31]=0x0;
	particleSystem[32]=0x0;
	particleSystem[33]=0x0;
	particleSystem[34]=0x0;
	particleSystem[35]=0x0;

	particleSystem[36]=sta->effect[8]; // effekt #
	particleSystem[37]=sta->effect[9];

	particleSystem[38]=0; // unknown
	particleSystem[39]=1;

	particleSystem[40]=0x00;
	particleSystem[41]=0x00;

	particleSystem[42]= pi->getSerial().ser1;
	particleSystem[43]= pi->getSerial().ser2;
	particleSystem[44]= pi->getSerial().ser3;
	particleSystem[45]= pi->getSerial().ser4;

	particleSystem[46]=0xff;

	particleSystem[47]=0x0;
	particleSystem[48]=0x0;

}

void bolteffectUO3D(CHARACTER player)
{

/*	Magic->doStaticEffect(player, 30);
*/
}

void sysmessageflat(NXWSOCKET  s, short color, const char *txt)
// System message (In lower left corner)
//Modified by N6 to use UNICODE packets
{
	UI08 unicodetext[512];
	UI16 ucl = ( strlen ( txt ) * 2 ) + 2 ;

	char2wchar(txt);
	memcpy(unicodetext, Unicode::temp, ucl);

	UI32 lang = calcserial(server_data.Unicodelanguage[0], server_data.Unicodelanguage[1], server_data.Unicodelanguage[2], 0);
	UI08 sysname[30]={ 0x00, };
	strcpy((char *)sysname, "System");

	SendUnicodeSpeechMessagePkt(s, 0x01010101, 0x0101, 6, color, 0x0003, lang, sysname, unicodetext,  ucl);

}

void wornitems(NXWSOCKET  s, P_CHAR pc) // Send worn items of player
{
	VALIDATEPC(pc);

	NxwItemWrapper si;
	si.fillItemWeared( pc, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if(ISVALIDPI(pi))
			wearIt(s,pi);
	}
}

