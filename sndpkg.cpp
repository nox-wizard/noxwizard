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

void gmyell(char *txt)
//Modified by N6 to use UNICODE packets
{
	unsigned char talk2[19];
	char unicodetext[512];
	int ucl = ( strlen ( txt ) * 2 ) + 2 ;

	int tl = ucl + 48 ;
	char2wchar(txt);
	memcpy(unicodetext, Unicode::temp, ucl);
	talk2[0] = (char)0xAE;
	talk2[1] = tl >> 8;
	talk2[2] = tl&0xFF;
	talk2[3] = 1;
	talk2[4] = 1;
	talk2[5] = 1;
	talk2[6] = 1;
	talk2[7] = 1;
	talk2[8] = 1;
	talk2[9] = 1;
	talk2[10]= 0x00;
	talk2[11]= 0x40;
	talk2[12]= 0;
	talk2[13]= 3;

	talk2[14] = server_data.Unicodelanguage[0];
	talk2[15] = server_data.Unicodelanguage[1];
	talk2[16] = server_data.Unicodelanguage[2];
	talk2[17] = 0;

	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWCLIENT ps_i=sw.getClient();
		if(ps_i==NULL) continue;
		P_CHAR pc=ps_i->currChar();
		if( ISVALIDPC(pc) && pc->IsGM())
		{
			Xsend(ps_i->toInt(), talk2, 18);
			Xsend(ps_i->toInt(), const_cast<char*>("[WebAdmin - GM Only]"), 30);
			Xsend(ps_i->toInt(), unicodetext, ucl);
		}
	}

	Network->ClearBuffers();
}


//keep the target highlighted so that we know who we're attacking =)
//26/10/99//new packet
void SndAttackOK(NXWSOCKET  s, int serial)
{
	UI08 attackok[5] = { 0xAA, 0x00, 0x00, 0x00, 0x00 };
	LongToCharPtr(serial,attackok+1);
	Xsend(s, attackok, 5);
}

void SndDyevat(NXWSOCKET  s, int serial, short id)
{
	unsigned char dyevat[10]="\x95\x40\x01\x02\x03\x00\x00\x0F\xAB";
	LongToCharPtr(serial,(dyevat+1));
	ShortToCharPtr(id,dyevat+7);
	Xsend(s, dyevat, 9);
}

void SndUpdscroll(NXWSOCKET  s, short txtlen, const char* txt)
{
	unsigned char updscroll[11]="\xA6\x01\x02\x02\x00\x00\x00\x00\x01\x02";
	ShortToCharPtr(txtlen+10,updscroll+1);
	updscroll[3]=2;
	ShortToCharPtr(txtlen,updscroll+8);
	Xsend(s, updscroll, 10);
	Xsend(s, txt, txtlen);

}

void SndRemoveitem(int serial)
{
	UI08 removeitem[5] = { 0x1D, 0x00, 0x00, 0x00, 0x00 };
	LongToCharPtr(serial,removeitem+1);
	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ )
		Xsend(sw.getSocket(), removeitem, 5);
}

void SndShopgumpopen(NXWSOCKET  s, int serial)
{
	unsigned char shopgumpopen[8]="\x24\x00\x00\x00\x01\x00\x30";
	LongToCharPtr(serial,shopgumpopen+1);
	Xsend(s, shopgumpopen, 7);
}


/*!
\brief play sound
\param s socket
\param goldtotal ?
*/
void goldsfx(int s, int goldtotal)
{
	if (goldtotal==1) 
		soundeffect(s, 0x00, 0x35);
	else if (goldtotal<6)
		soundeffect(s, 0x00, 0x36);
	else 
		soundeffect(s, 0x00, 0x37);
	return;
}

/*!
\brief play a sound based on item id

added to do easy item sound effects based on an
items id1 and id2 fields in struct items. Then just define the CASE statement
with the proper sound function to play for a certain item as shown.

\author Dupois Duke
\date 09/10/1998 creation
	  25/03/2001 new interface by duke
\param s socket
\param item the item
\note I wasn't sure what the different soundeffect() func's did so I just used
		soundeffect() and it seemed to work fairly well.
\remarks \remark Use the DEFAULT case for ranges of items (like all ingots make the same thump).
		 \remark Sounds: 
			\li coins dropping (all the same no matter what amount because all id's equal 0x0EED
			\li ingot dropping (makes a big thump - used the heavy gem sound)
			\li gems dropping (two type broke them in half to make some sound different then others)
*/
void itemsfx(NXWSOCKET  s, short item)
{
	if( item==0x0EED )
		goldsfx(s, 2);

	else if( (item>=0x0F0F) && (item<=0x0F20) )	// Any gem stone (typically smaller)
		soundeffect(s, 0x00, 0x32);

	else if( (item>=0x0F21) && (item<=0x0F30) )	// Any gem stone (typically larger)
		soundeffect(s, 0x00, 0x34);

	else if( (item>=0x1BE3) && (item<=0x1BFA) )	// Any Ingot
		soundeffect(s, 0x00, 0x33);

	soundeffect(s, 0x00, 0x42);					// play default item move sfx // 00 48
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

	int sound;
	int distance=(VISRANGE+5);
	P_CHAR inrange[15];
	int basesound=0;
	int bigf,xx;
	char sf,bfs;


	NxwCharWrapper sc;
	sc.fillCharsNearXYZ( pc_curr->getPosition(), VISRANGE, true, false );
	int y=0;
	for( sc.rewind(); !sc.isEmpty(); sc++ ) {
		P_CHAR pc=sc.getChar();
		if((pc->npc)&&(!(pc->dead))&&(!(pc->war))&&(y<=10))
		{
			if (char_inRange(pc_curr, pc, distance))
			{
				y++;
				inrange[y]=pc;
			}
		}
	}

	if (y>0)
	{

		sound=((rand()%(y))+1);

		P_CHAR pc_inr=inrange[sound];
		VALIDATEPC(pc_inr);

		xx=DBYTE2WORD(pc_inr->id1,pc_inr->id2);
		if (xx>INVALID && xx<2048)
		{
			basesound=creatures[xx].basesound;
			sf=creatures[xx].soundflag;
		}
		else return;

		if (basesound!=0)
		{
			/*switch(sf) // play only idle sounds, if there arnt any, dont play them !
			{
			case 0: basesound++; break;	// normal case -> play idle sound
			case 1: basesound++; break;	// birds sounds will be implmented later
			case 2:	basesound=0; break;	// no idle sound, so dont play it !
			case 3: basesound=0; break;	// no idle sound, so dont play it !
			case 4:	break; // only a single sound, play it !
			}
			unsigned char sfx[13]="\x54\x01\x12\x34\x00\x00\x06\x40\x05\x9A\x00\x00";
			if (bgsound!=0) // bugfix lb
			{
				Location charpos= pc_inr->getPosition();

				sfx[2]= basesound >> 8;
				sfx[3]= basesound % 256;
				sfx[6]= charpos.x >> 8;
				sfx[7]= charpos.x % 256;
				sfx[8]= charpos.y >> 8;
				sfx[9]= charpos.y % 256;
				Xsend(calcSocketFromChar(DEREF_P_CHAR(pc_curr)), sfx, 12); //bugfix, LB
			} Temp removed by Luxor*/
                        if ( chance(20) )
				pc_inr->playMonsterSound(SND_IDLE);
		}
	}
	// play random mystic-sounds also if no creature is in range ...

	bigf=rand()%3333;
	basesound=0;

	if (bigf==33)

	{
		bfs=rand()%7;
		switch(bfs)
		{
		case 0: basesound=595;break; // gnome sound
		case 1: basesound=287;break; // bigfoot 1
		case 2: basesound=288;break; // bigfoot 2
		case 3: basesound=639;break; // old snake sound
		case 4: basesound=179;break; // lion sound
		case 5: basesound=246;break; // mystic
		case 6: basesound=253;break; // mystic II
		}
		unsigned char sfx[13]="\x54\x01\x12\x34\x00\x00\x06\x40\x05\x9A\x00\x00";
		if (basesound !=0)
		{
			Location charpos= pc_curr->getPosition();

			sfx[2] = (unsigned char) (basesound >> 8);
			sfx[3] = (unsigned char) (basesound % 256);
			sfx[6] = (unsigned char) (charpos.x >> 8);
			sfx[7] = (unsigned char) (charpos.x % 256);
			sfx[8] = (unsigned char) (charpos.y >> 8);
			sfx[9] = (unsigned char) (charpos.y % 256);
			Xsend(calcSocketFromChar(DEREF_P_CHAR(pc_curr)), sfx, 12); //bugfix LB
		}
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
			if (!(strcmp("MIDI",(char*)script1)))
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

void soundeffect(NXWSOCKET s, unsigned char a, unsigned char b) // Play sound effect for player
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	Location charpos= pc->getPosition();
	unsigned char sfx[13]="\x54\x01\x12\x34\x00\x00\x06\x40\x05\x9A\x00\x00";

	sfx[2]= a;
	sfx[3]= b;
	sfx[6]= charpos.x >> 8;
	sfx[7]= charpos.x % 256;
	sfx[8]= charpos.y >> 8;
	sfx[9]= charpos.y % 256;

	NxwSocketWrapper sw;
	sw.fillOnline( pc, false );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWCLIENT ps_i=sw.getClient();
		if(ps_i==NULL) continue;
		P_CHAR pc_i=ps_i->currChar();
		if( ISVALIDPC(pc_i) )
			Xsend(ps_i->toInt(), sfx, 12);
	}
}

void soundeffect5(NXWSOCKET  s, unsigned char a, unsigned char b)
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	Location charpos= pc->getPosition();
	unsigned char sfx[13]="\x54\x01\x12\x34\x00\x00\x06\x40\x05\x9A\x00\x00";

	sfx[2]= a;
	sfx[3]= b;
	sfx[6]= charpos.x >> 8;
	sfx[7]= charpos.x % 256;
	sfx[8]= charpos.y >> 8;
	sfx[9]= charpos.y % 256;
	Xsend(s, sfx, 12);
}


void soundeffect3(P_ITEM pi, short sound)
{
	VALIDATEPI(pi);
	
	unsigned char sfx[13]="\x54\x01\x12\x34\x00\x00\x06\x40\x05\x9A\x00\x00";

	ShortToCharPtr(sound,sfx+2);
	ShortToCharPtr(pi->getPosition("x"), sfx+6);
	ShortToCharPtr(pi->getPosition("y"), sfx+8);

	NxwSocketWrapper sw;
	sw.fillOnline( pi );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWCLIENT ps_i=sw.getClient();
		if(ps_i==NULL) continue;
		P_CHAR pc_j=ps_i->currChar();
		if( ISVALIDPC(pc_j))
			Xsend(ps_i->toInt(), sfx, 12);
	}
}

void soundeffect4(int p, NXWSOCKET  s, unsigned char a, unsigned char b)
{
	const PC_ITEM pi=MAKE_ITEMREF_LR(p);	// on error return
	VALIDATEPI(pi);

	unsigned char sfx[13]="\x54\x01\x12\x34\x00\x00\x06\x40\x05\x9A\x00\x00";
	sfx[2]= a;
	sfx[3]= b;
	sfx[6]= pi->getPosition("x") >> 8;
	sfx[7]= pi->getPosition("x") % 256;
	sfx[8]= pi->getPosition("y") >> 8;
	sfx[9]= pi->getPosition("y") % 256;
	Xsend(s, sfx, 12);
}

//xan : fast weather function.. maybe we should find a more complete system like the
//old one below!
void weather(NXWSOCKET  s, unsigned char bolt)
{
	unsigned char packet[4] = { 0x65, 0xFF, 0x40, 0x20 };

	if (wtype==0) packet[2] = 0x00;
	if (wtype==1) packet[1] = 0x00;
	if (wtype==2) { packet[1] = 0x02; packet[3] = 0xEC; }

	Xsend(s, packet, 4);
}

void pweather(NXWSOCKET  s)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	unsigned char packet[4] = { 0x65, 0xFF, 0x40, 0x20 };

	if (region[pc->region].wtype==0) packet[2] = 0x00;
	if (region[pc->region].wtype==1) packet[1] = 0x00;
	if (region[pc->region].wtype==2) { packet[1] = 0x02; packet[3] = 0xEC; }

	Xsend(s, packet, 4);
}

void sysbroadcast(char *txt, ...) // System broadcast in bold text
//Modified by N6 to use UNICODE packets
{

	unsigned char talk2[19];
	char unicodetext[512];

	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
	vsprintf( msg, txt, argptr );
	va_end( argptr );

	int ucl = ( strlen ( msg ) * 2 ) + 2 ;
	int tl = ucl + 48 ;

	char2wchar(msg);
	memcpy(unicodetext, Unicode::temp, ucl);

	talk2[0] = 0xAE;
	ShortToCharPtr(tl, talk2+1);
	talk2[3] = 1;
	talk2[4] = 1;
	talk2[5] = 1;
	talk2[6] = 1;
	talk2[7] = 1;
	talk2[8] = 1;
	talk2[9] = 6;
	talk2[10]=0x08;	//Color1  - Previous default was 0x0040
	talk2[11]=0x4d;  //Color2
	talk2[12]=0;
	talk2[13]=0;

	talk2[14] = server_data.Unicodelanguage[0];
	talk2[15] = server_data.Unicodelanguage[1];
	talk2[16] = server_data.Unicodelanguage[2];
	talk2[17] = 0;

	unsigned char sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	NxwSocketWrapper sw;
	sw.fillOnline();
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET sock=sw.getSocket();
		if( sock!=INVALID )
		{
			Xsend(sock, talk2, 18);
			Xsend(sock, sysname, 30);
			Xsend(sock, unicodetext, ucl);
		}
	}

}


void sysmessage(NXWSOCKET  s, const char *txt, ...) // System message (In lower left corner)
//Modified by N6 to use UNICODE packets
{
	if(s < 0) 
		return;

	unsigned char talk2[19];
	char unicodetext[512];

	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
        vsprintf( msg, txt, argptr );
	va_end( argptr );

	int ucl = ( strlen ( msg ) * 2 ) + 2 ;
	int tl = ucl + 48 ;

	char2wchar(msg);
	memcpy(unicodetext, Unicode::temp, ucl);

	talk2[0] = 0xAE;
	ShortToCharPtr(tl, talk2+1);
	talk2[3] = 1;
	talk2[4] = 1;
	talk2[5] = 1;
	talk2[6] = 1;
	talk2[7] = 1;
	talk2[8] = 1;
	talk2[9] = 6;

	talk2[10]=0x03;	//Color1  - Previous default was 0x0040 - 0x03E9
	talk2[11]=0x87; //Color2
	talk2[12]=0;
	talk2[13]=3;

	talk2[14] = server_data.Unicodelanguage[0];
	talk2[15] = server_data.Unicodelanguage[1];
	talk2[16] = server_data.Unicodelanguage[2];
	talk2[17] = 0;
	unsigned char sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	Xsend(s, talk2, 18);
	Xsend(s, sysname, 30);
	Xsend(s, unicodetext, ucl);

}


void sysmessage(NXWSOCKET  s, short color, const char *txt, ...) // System message (In lower left corner)
//Modified by N6 to use UNICODE packets
{
	if( s < 0)
		return;

	unsigned char talk2[19];
	char unicodetext[512];

	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
	//vsnprintf( msg, sizeof(msg)-1, txt, argptr );
        vsprintf( msg, txt, argptr );
	va_end( argptr );
	int ucl = ( strlen ( msg ) * 2 ) + 2 ;
	int tl = ucl + 48 ;

	char2wchar(msg);
	memcpy(unicodetext, Unicode::temp, ucl);

	UI08 *tptr = talk2;
	*tptr++ = (char)0xAE;
	*tptr++ = tl >> 8;
	*tptr++ = tl&0xFF;
	*tptr++ = 1;
	*tptr++ = 1;
	*tptr++ = 1;
	*tptr++ = 1;
	*tptr++ = 1;
	*tptr++ = 1;
	*tptr++ = 0;
	*tptr++ = color>>8;
	*tptr++ = color%256;
	*tptr++ =0;
	*tptr++ =3;
	*tptr++ = server_data.Unicodelanguage[0];
	*tptr++ = server_data.Unicodelanguage[1];
	*tptr++ = server_data.Unicodelanguage[2];
	*tptr++ = 0;

	unsigned char sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	Xsend(s, talk2, 18);
	Xsend(s, sysname, 30);
	Xsend(s, unicodetext, ucl);

}

void itemmessage(NXWSOCKET  s, char *txt, int serial, short color)
{// The message when an item is clicked (new interface, Duke)
//Modified by N6 to use UNICODE packets
	unsigned char talk2[19];
	char unicodetext[512];
	int ucl = ( strlen ( txt ) * 2 ) + 2 ;

	P_ITEM pi=pointers::findItemBySerial(serial);
	VALIDATEPI(pi);

	if ((pi->type == 1 && color == 0x0000)||
		(pi->type == 9 && color == 0x0000)||
		(pi->id()==0x1BF2 && color == 0x0000))
		color = 0x03B2;

	int tl = ucl + 48 ;
	char2wchar(txt);
	memcpy(unicodetext, Unicode::temp, ucl);

	talk2[0] = (char)0xAE;
	ShortToCharPtr(tl, talk2+1);
	LongToCharPtr(serial,talk2+3);
	talk2[7] = 1;
	talk2[8] = 1;
	talk2[9]=6; // Mode: "You see"
	//ShortToCharPtr(color,talk+10);
	talk2[10]=0x04; // UOLBR patch to prevent client crash by Juliunus
	talk2[11]=0x81;
	talk2[12]=0;
	talk2[13]=3;

	talk2[14] = server_data.Unicodelanguage[0];
	talk2[15] = server_data.Unicodelanguage[1];
	talk2[16] = server_data.Unicodelanguage[2];
	talk2[17] = 0;

	unsigned char sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	Xsend(s, talk2, 18);
	Xsend(s, sysname, 30);
	Xsend(s, unicodetext, ucl);

}

void wearIt(const NXWSOCKET  s, const P_ITEM pi)
{
	VALIDATEPI(pi);
	
	unsigned char wearitem[16]="\x2E\x40\x0A\x00\x01\x00\x00\x00\x01\x00\x05\xA8\x90\x00\x00";
	LongToCharPtr(pi->getSerial32(), wearitem+1);
	ShortToCharPtr(pi->animid(),wearitem+5); // elcabesa animation
	wearitem[8]=pi->layer;
	LongToCharPtr(pi->getContSerial(),wearitem+9);
	wearitem[13]=pi->color1;
	wearitem[14]=pi->color2;
	Xsend(s, wearitem, 15);
}

void backpack2(NXWSOCKET s, SERIAL serial) //int a1, int a2, int a3, int a4) // Send corpse stuff
{
	int count=0, count2;
	unsigned char bpopen2[6]="\x3C\x00\x05\x00\x00";
	unsigned char display1[8]="\x89\x00\x0D\x40\x01\x02\x03";
	unsigned char display2[6]="\x01\x40\x01\x02\x03";

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
	char nul = 0;
	Xsend(s, &nul, 1);	// Terminate with a 0

	ShortToCharPtr(count, bpopen2+3);
	count2=(count*19)+5;
	ShortToCharPtr(count2, bpopen2+1);
	Xsend(s, bpopen2, 5);

	unsigned char bpitem[20]="\x40\x0D\x98\xF7\x0F\x4F\x00\x00\x09\x00\x30\x00\x52\x40\x0B\x00\x1A\x00\x00";

	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if( ISVALIDPI(pi) && (pi->layer!=0) )
		{
			LongToCharPtr(pi->getSerial32(), bpitem);
			ShortToCharPtr(pi->animid(),bpitem+4);
			ShortToCharPtr(pi->amount,bpitem+7);
			ShortToCharPtr(pi->getPosition().x, bpitem+9);
			ShortToCharPtr(pi->getPosition().y, bpitem+11);
			LongToCharPtr(serial, bpitem+13);
			bpitem[17]=pi->color1;
			bpitem[18]=pi->color2;
			//bpitem[19]=pi->decaytime=0;// reseting the decaytimer in the backpack	//moroallan
			bpitem[19]= 0;
			Xsend(s, bpitem, 19);
		}
	}

}

void sendbpitem(NXWSOCKET s, P_ITEM pi)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	unsigned char display3[2]="\x25";
	unsigned char bpitem[20]="\x40\x0D\x98\xF7\x0F\x4F\x00\x00\x09\x00\x30\x00\x52\x40\x0B\x00\x1A\x00\x00";

	LongToCharPtr(pi->getSerial32(), bpitem);
	//AntiChrist - world light sources stuff
	//if player is a gm, this item
	//is shown like a candle (so that he can move it),
	//....if not, the item is a normal
	//invisible light source!
	if(pc->IsGM() && pi->id()==0x1647)
	{///let's show the lightsource like a candle
		bpitem[4]=0x0A;
		bpitem[5]=0x0F;
	} else
	{//else like a normal item
		ShortToCharPtr(pi->animid(),bpitem+4);
	}
	ShortToCharPtr(pi->amount,bpitem+7);
	ShortToCharPtr(pi->getPosition().x, bpitem+9);
	ShortToCharPtr(pi->getPosition().y, bpitem+11);
	LongToCharPtr(pi->getContSerial(),bpitem+13);
	if(pc->IsGM() && pi->id()==0x1647)
	{///let's show the lightsource like a blue item
		bpitem[17]=0;
		bpitem[18]=0xC6;
	}
	else
	{//else like a normal item
		bpitem[17]=pi->color1;
		bpitem[18]=pi->color2;
	}
	//bpitem[19]=pi->decaytime=0; // HoneyJar, array range is 0-19 ! //reseting the decaytimer in the backpack
	bpitem[19] = 0; // HoneyJar, array range is 0-19 ! //reseting the decaytimer in the backpack

	// we need to find the topmost container that the item is in
	// be it a character or another container.


	if( pc->distFrom(pi)<=VISRANGE )
	{
		Xsend(s, display3, 1);
		Xsend(s, bpitem, 19);
	}
	weights::NewCalc(pc);	// Ison 2-20-99
}

void tileeffect(int x, int y, int z, char eff1, char eff2, char speed, char loop)
{//AntiChrist


	unsigned char effect[28];
	for (int i=0;i<28;i++)
	{
		effect[i]=0;
	}
	effect[0]=0x70; // Effect message
	effect[1]=0x02; // Stay at x, y, z effect
	//[2] to [9] are	not applicable here.
	effect[10]=eff1;// Object id of the effect
	effect[11]=eff2;
	ShortToCharPtr(x, effect+12);
	ShortToCharPtr(y, effect+14);
	effect[16]=z;
	//[17] to [21] are not applicable here.
	effect[22]=speed;
	effect[23]=loop; // 0 is really long.	1 is the shortest.
	//[24] to [25] are not applicable here.
	effect[26]=1; // LB possible client crashfix
	effect[27]=0;

	Location location;
	location.x=x;
	location.y=y;
	
	NxwSocketWrapper sw;
	sw.fillOnline( location );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET sock=sw.getSocket();
		if( sock!=INVALID )
		{
			Xsend(sock, effect, 28);
		}
	}

}

void senditem(NXWSOCKET  s, P_ITEM pi) // Send items (on ground)
{
	VALIDATEPI(pi);

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	bool pack;
	unsigned char itmput[21]="\x1A\x00\x13\x40\x01\x02\x03\x20\x42\x00\x32\x06\x06\x06\x4A\x0A\x00\x00\x00";

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
			if (pi->id1<0x40) // LB client crashfix, dont show multis in BP
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
		LongToCharPtr(pi->getSerial32() + 0x80000000, itmput+3);

		//if player is a gm, this item
		//is shown like a candle (so that he can move it),
		//....if not, the item is a normal
		//invisible light source!
		if(pc->IsGM() && pi->id()==0x1647)
		{///let's show the lightsource like a candle
			itmput[7]=0x0A;
			itmput[8]=0x0F;
		} else
		{//else like a normal item
			ShortToCharPtr(pi->animid(),itmput+7); // elcabesa animation tryyy
		}

		ShortToCharPtr(pi->amount, itmput+9);
		ShortToCharPtr(pi->getPosition().x, itmput+11);
		ShortToCharPtr(pi->getPosition().y+0xC000, itmput+13);
		itmput[15]= pi->getPosition("z");

		if(pc->IsGM() && pi->id()==0x1647)
		{///let's show the lightsource like a blue item
			itmput[16]=0;
			itmput[17]=0xC6;
		} else
		{
			itmput[16]=pi->color1;
			itmput[17]=pi->color2;
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
		if (pc->priv2&1)
			itmput[18]|=0x20;
		if ((pi->magic==3 || pi->magic==4) && pc->getSerial32()==pi->getOwnerSerial32())
			itmput[18]|=0x20;

		if (pc->priv2&4)
		{
			if (pi->id()>=0x4000 && pi->id()<=0x40FF) // LB, 25-dec-1999 litle bugfix for treasure multis, ( == changed to >=)
			{
				itmput[7]=0x14;
				itmput[8]=0xF0;
			}
		}

		int dir=0;
		if (pi->dir)
		{
			dir=1;
			itmput[19]=itmput[18];
			itmput[18]=itmput[17];
			itmput[17]=itmput[16];
			itmput[16]=itmput[15];
			itmput[15]=static_cast<unsigned char>(pi->dir);
			itmput[2]=0x14;
			itmput[11]|=0x80;
		}

		itmput[2]=0x13+dir;

		Xsend(s, itmput, 19+dir);
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

	const P_ITEM pi=MAKE_ITEMREF_LR(i);	// on error return

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	
	unsigned char itmput[20]="\x1A\x00\x13\x40\x01\x02\x03\x20\x42\x00\x32\x06\x06\x06\x4A\x0A\x00\x00\x00";

	if ( pi->visible>=1 && !(pc->IsGM()) ) return; // workaround for missing gm-check client side for visibity since client 1.26.2
	// for lsd we dont need extra work for type 1 as in send_item

	if (pi->isInWorld())
	{
		LongToCharPtr(pi->getSerial32() + 0x80000000, itmput+3);
		ShortToCharPtr(pi->id(),itmput+7);
		ShortToCharPtr(pi->amount, itmput+9);
		ShortToCharPtr(pi->getPosition().x, itmput+11);
		ShortToCharPtr(pi->getPosition().y, itmput+13 + 0xC000);
		itmput[15]=z;
		itmput[16]=color1;
		itmput[17]=color2;
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
			if ((pc->id1==0x03 && pc->id2==0xDB) || !pc->IsGM())
				itmput[18]|=0x80;
		}

		if (pi->magic==1) itmput[18]+=0x20;

		if (pc->priv2&1) itmput[18]+=0x20;
		if ((pi->magic==3 || pi->magic==4) && pc->getSerial32()==pi->getOwnerSerial32())
			itmput[18]+=0x20;
		if (pc->priv2&4)
		{
			if (pi->id()>=0x4000 && pi->id()<=0x40FF)
			{
				itmput[7]=0x14;
				itmput[8]=0xF0;
			}
		}
		if (pi->dir)
		{
			itmput[19]=itmput[18];
			itmput[18]=itmput[17];
			itmput[17]=itmput[16];
			itmput[16]=itmput[15];
			itmput[15]=static_cast<unsigned char>(pi->dir);
			itmput[2]=0x14;
			itmput[11]|=0x80;
			Xsend(s, itmput, 20);
		} else
		{
			itmput[2]=0x13;
			Xsend(s, itmput, 19);
		}

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
	unsigned char delete_error_msg[2] = {0x85, 0x05};
	unsigned char delete_resend_char_1[6]={0x86, 0x01, 0x30, 0x00}; // 1 + 2 + 1 + 5*60 = 304 = 0x0130
	unsigned char delete_resend_char_2[61];

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
			return;
		}

		if (ISVALIDPC(TrashMeUp))
		{
			if( SrvParms->checkcharage && 
			   (getclockday() < TrashMeUp->GetCreationDay() + 7) ) {
				delete_error_msg[1] = 0x03;
				Xsend(s, delete_error_msg, 2);
				return;
			}

			if(TrashMeUp->IsOnline()) {
				delete_error_msg[1] = 0x02;
				Xsend(s, delete_error_msg, 2);
				return;
			}
			
			TrashMeUp->deleteChar();

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

			return; // All done ;]
		}
	} 
// Any possible error ....
	Xsend(s, delete_error_msg, 2);
}

// Send targetting cursor to client
void sendTargetCursor(NXWSOCKET s, int a1, int a2, int a3, int a4) {
	unsigned char tarcrs[20]="\x6C\x01\x40\x01\x02\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	targetok[s]=1;
	tarcrs[2]=a1;
	tarcrs[3]=a2;
	tarcrs[4]=a3;
	*(int*)&tarcrs[5]=a4;
	Xsend(s, tarcrs, 19);
}

void target(NXWSOCKET  s, int a1, int a2, int a3, int a4, char *txt) // Send targetting cursor to client
{
	unsigned char tarcrs[20]="\x6C\x01\x40\x01\x02\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	targetok[s]=1;
	tarcrs[2]=a1;
	tarcrs[3]=a2;
	tarcrs[4]=a3;
	*(int*)&tarcrs[5]=a4;
	sysmessage(s, txt);
	Xsend(s, tarcrs, 19);

}

void skillwindow(NXWSOCKET s) // Opens the skills list, updated for client 1.26.2b by LB
{

	P_CHAR pc= MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	unsigned char skillstart[5]="\x3A\x01\x5d\x00"; // hack for that 3 new skills+1.26.2 client, LB 4'th dec 1999
	unsigned char skillmid[8] = "\x00\x00\x00\x00\x00\x00\x00"; // changed for 1.26.2 clients [size 7 insted of 4]
	unsigned char skillend[3]="\x00\x00";
	char x;



	Xsend(s, skillstart, 4);
	for (int i=0;i<TRUESKILLS;i++)
	{
		Skills::updateSkillLevel(pc,i);
		skillmid[1]=i+1;
		skillmid[2]=pc->skill[i]>>8;
		skillmid[3]=pc->skill[i]%256;
		skillmid[4]=pc->baseskill[i]>>8;
		skillmid[5]=pc->baseskill[i]%256;

		x=pc->lockSkill[i];
		if (x!=0 && x!=1 && x!=2) x=0;
		skillmid[6]=x; // leave it unlocked, regardless
		Xsend(s, skillmid, 7);
	}
	Xsend(s, skillend, 2);

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
	char updater[10]="\xA1\x01\x02\x03\x04\x01\x03\x01\x02";

	switch (stat)
	{
	case 0:
		a=this->getStrength();
		b=this->hp;
		break;
	case 1:
		a=this->in;
		b=this->mn;
		break;
	case 2:
		a=this->dx;
		b=this->stm;
		break;
	}

	updater[0]=0xA1+stat;
	updater[1]=this->getSerial().ser1;
	updater[2]=this->getSerial().ser2;
	updater[3]=this->getSerial().ser3;
	updater[4]=this->getSerial().ser4;
	updater[5]=a>>8;
	updater[6]=a%256;
	updater[7]=b>>8;
	updater[8]=b%256;
	if (stat == 0)  //Send to all, only if it's Health change
	{
		NxwSocketWrapper sw;
		sw.fillOnline( this, false );
		for( sw.rewind(); !sw.isEmpty(); sw++ ) 
		{
			NXWSOCKET i=sw.getSocket();
			if( i!=INVALID )
				Xsend(i, updater, 9);
		}
	} else {
		int s = calcSocketFromChar(DEREF_P_CHAR(this));
		if (s != INVALID)
			Xsend(s, updater, 9);
	}

}

void statwindow(P_CHAR pc_to, P_CHAR pc) // Opens the status window
{

	VALIDATEPC(pc);
	VALIDATEPC(pc_to);

	int x;
	TEXT statstring[67]="\x11\x00\x42\x00\x05\xA8\x90XYZ\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x12\x00\x34\xFF\x01\x00\x00\x5F\x00\x60\x00\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00\x00\x75\x30\x01\x2C\x00\x00";
	bool ghost;


	if ((pc->id1==0x01 && pc->id2==0x92) || (pc->id1==0x01 && pc->id2==0x93)) ghost = true; else ghost = false;

	statstring[3]=pc->getSerial().ser1;
	statstring[4]=pc->getSerial().ser2;
	statstring[5]=pc->getSerial().ser3;
	statstring[6]=pc->getSerial().ser4;
	strncpy(&statstring[7], pc->getCurrentNameC(), 30); // can not be more than 30 at least no without changing packet lenght

	if (!ghost)
	{
		statstring[37]=pc->hp>>8;
		statstring[38]=pc->hp%256;
	}
	else
	{
		statstring[37] = statstring[38] = 0;
	}
	//Changed, so ghosts can see their maximum hit points.
	statstring[39]=pc->getStrength()>>8;
	statstring[40]=pc->getStrength()%256;

	if (((pc_to->IsGM())||(pc->getOwnerSerial32()==pc_to->getSerial32()))&&(pc_to!=pc))
	{
		statstring[41]='\xFF';
	}
	else if ((pc_to->getSerial32()==pc->getOwnerSerial32())&&(pc_to!=pc)) //Morrolan - from Banter
	{
		statstring[41]='\xFF';
	}
	else
	{
		statstring[41]=0x00;
	}

	if (ghost) statstring[41]=0x00;

	// packet #42 has some problems, dont try to be smart and replace the workaround by
	// if (ghost) statstring[42]=0; else statstring[42]=1, LB

	if ((pc->id1==0x01)&&(pc->id2==0x91)) statstring[43]=1;
	else if ((pc->id1==0x01) && (pc->id2==0x93)) statstring[43]=1;
	else statstring[43]=0; // LB, prevents very female looking male players ... :-)

	//Changed so ghosts can see their str, dex and int, their char haven't lost those attributes.
	statstring[44]=pc->getStrength()>>8;
	statstring[45]=pc->getStrength()%256;
	statstring[46]=pc->dx>>8;
	statstring[47]=pc->dx%256;
	statstring[48]=pc->in>>8; // Real INT
	statstring[49]=pc->in%256;

	if (!ghost)
	{
		statstring[50]=pc->stm>>8;
		statstring[51]=pc->stm%256;
		statstring[54]=pc->mn>>8;
		statstring[55]=pc->mn%256;
	}
	else
	{
		// Sets to 0 stamina and mana
		for (int a = 50; a <= 57; a++)
			statstring[a] = 0;
	}
	// ghosts will see their mana as 0/x, ie 0/100
	// This will show red bars when status are displayed as percentages (little status window)
	statstring[52]=pc->dx>>8; // MaxStamina
	statstring[53]=pc->dx%256;
	statstring[56]=pc->in>>8; // MaxMana
	statstring[57]=pc->in%256;


	x = pc->CountGold();
	statstring[58]=x>>24;
	statstring[59]=x>>16;
	statstring[60]=x>>8;
	statstring[61]=x%256;

	x = pc->calcDef(0);
	statstring[62]=x>>8; // AC
	statstring[63]=x%256;
	x = (int)(pc->weight);
	statstring[64]=x>>8;
	statstring[65]=x%256;
	Xsend(calcSocketFromChar(DEREF_P_CHAR(pc_to)), statstring, 66);


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
	unsigned char updscroll[11]="\xA6\x01\x02\x02\x00\x00\x00\x00\x01\x02";
	updscroll[1]=y>>8;
	updscroll[2]=y%256;
	updscroll[3]=2;
	updscroll[8]=(y-10)>>8;
	updscroll[9]=(y-10)%256;
	Xsend(s, updscroll, 10);
	for (j=0;j<x;j++)
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		sprintf(temp, "%s ", script1);
		Xsend(s, temp, strlen(temp));
	}
	safedelete(iter);

}

void tips(int s, int i) // Tip of the day window
{


	int x, y, j;
	char temp[512];
    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];

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
		tips(s, 1);
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
	unsigned char updscroll[11]="\xA6\x01\x02\x02\x00\x00\x00\x00\x01\x02";
	updscroll[1]=y>>8;
	updscroll[2]=y%256;
	updscroll[3]=0;
	updscroll[7]=i;
	updscroll[8]=(y-10)>>8;
	updscroll[9]=(y-10)%256;
	Xsend(s, updscroll, 10);
	for (j=0;j<x;j++)
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());//discards the {
		sprintf(temp, "%s ", script1);
		Xsend(s, temp, strlen(temp));
	}
	safedelete(iter);

}


void deny(NXWSOCKET  k,P_CHAR pc, int sequence)
{

	cPacketWalkReject walkdeny;
	walkdeny.sequence= sequence;
	walkdeny.x= pc->getPosition().x;
	walkdeny.y= pc->getPosition().y;
	walkdeny.direction=pc->dir;
	walkdeny.z= pc->getPosition().dispz;
	walkdeny.send( pc->getClient() );
	walksequence[k]=INVALID;

}

void weblaunch(int s, const char *txt) // Direct client to a web page
{
	int l;
	char launchstr[4]="\xA5\x00\x00";

	sysmessage(s, TRANSLATE("Launching your web browser. Please wait..."));
	l=strlen(txt)+4;
	launchstr[1]=l>>8;
	launchstr[2]=l%256;
	Xsend(s, launchstr, 3);
	Xsend(s, txt, strlen(txt)+1);

}

void broadcast(int s) // GM Broadcast (Done if a GM yells something)
//Modified by N6 to use UNICODE packets
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int i,tl;
	TEXT nonuni[512];

	if(pc->unicode)
		for (i=13;i<(buffer[s][1]<<8)+buffer[s][2];i=i+2)
		{
			nonuni[(i-13)/2]=buffer[s][i];
		}
		if(!(pc->unicode))
		{
			tl=44+strlen((char*)&buffer[s][8])+1;
			unsigned char talk[15]="\x1C\x00\x00\x01\x02\x03\x04\x01\x90\x00\x00\x38\x00\x03";
			talk[1]=tl>>8;
			talk[2]=tl%256;
			talk[3]=pc->getSerial().ser1;
			talk[4]=pc->getSerial().ser2;
			talk[5]=pc->getSerial().ser3;
			talk[6]=pc->getSerial().ser4;
			talk[7]=pc->id1;
			talk[8]=pc->id2;
			talk[9]=1;
			talk[10]=buffer[s][4];
			talk[11]=buffer[s][5];
			talk[12]=buffer[s][6];
			talk[13]=pc->fonttype;
			
			NxwSocketWrapper sw;
			sw.fillOnline();
			for( sw.rewind(); !sw.isEmpty(); sw++ )
			{
				NXWSOCKET i=sw.getSocket();
				{
					Xsend(i, talk, 14);
					Xsend(i, pc->getCurrentNameC(), 30);
					Xsend(i, (void *)&buffer[s][8], strlen((const char*)&buffer[s][8])+1);
				}
			}
		} // end unicode IF
		else
		{
			unsigned char talk2[19];
			char unicodetext[512];
			int ucl = ( strlen ( &nonuni[0] ) * 2 ) + 2 ;
			tl=tl = ucl + 48 ;
			char2wchar(&nonuni[0]);
			memcpy(unicodetext, Unicode::temp, ucl);


			talk2[0] = (char)0xAE;
			talk2[1] = tl >> 8;
			talk2[2] = tl&0xFF;
			talk2[3]=pc->getSerial().ser1;
			talk2[4]=pc->getSerial().ser2;
			talk2[5]=pc->getSerial().ser3;
			talk2[6]=pc->getSerial().ser4;
			talk2[7]=pc->id1;
			talk2[8]=pc->id2;
			talk2[9]=1;
			talk2[10]=buffer[s][4];
			talk2[11]=buffer[s][5];
			talk2[12]=buffer[s][6];
			talk2[13]=pc->fonttype;

			talk2[14] = buffer[s][8];
			talk2[15] = buffer[s][9];
			talk2[16] = buffer[s][10];
			talk2[17] = buffer[s][11];

			NxwSocketWrapper sw;
			sw.fillOnline();
			for( sw.rewind(); !sw.isEmpty(); sw++ )
			{
				NXWSOCKET i=sw.getSocket();
				{
					Xsend(i, talk2, 18);
					Xsend(i, pc->getCurrentNameC(), 30);
					Xsend(i, unicodetext, ucl);
				}
			}
		}

}

void itemtalk(P_ITEM pi, char *txt) // Item "speech"
//Modified by N6 to use UNICODE packets
{

	VALIDATEPI(pi);

	NxwSocketWrapper sw;
	sw.fillOnline( pi );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET s=sw.getSocket();
		if(s==INVALID) continue;

		unsigned char talk2[19];
		char unicodetext[512];
		int ucl = ( strlen ( txt ) * 2 ) + 2 ;
		int tl = ucl + 48 ;
		char2wchar(txt);
		memcpy(unicodetext, Unicode::temp, ucl);

		
		talk2[0] = (char)0xAE;
		talk2[1] = tl >> 8;
		talk2[2] = tl&0xFF;
		LongToCharPtr(pi->getSerial32(),talk2+3);
		ShortToCharPtr(pi->id(),talk2+7);
		talk2[9]=0; // Type
		talk2[10]=0x04;
		talk2[11]='\x81';
		talk2[12]=0;
		talk2[13]=3;

		talk2[14] = server_data.Unicodelanguage[0];
		talk2[15] = server_data.Unicodelanguage[1];
		talk2[16] = server_data.Unicodelanguage[2];
		talk2[17] = 0;

		Xsend(s, talk2, 18);
		Xsend(s, pi->getCurrentNameC(), 30);
		Xsend(s, unicodetext, ucl);
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


    	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	 int a0,a1,a2,a3,a4;
	 char effect[29];
	 Location charpos= pc->getPosition();

	 if (!skip_old)
	 {
	    memset(&effect[0], 0, 29);
	    effect[0]= 0x70; // Effect message
	    effect[1]= 0x03; // Static effect
	    effect[2]= pc->getSerial().ser1;
	    effect[3]= pc->getSerial().ser2;
	    effect[4]= pc->getSerial().ser3;
	    effect[5]= pc->getSerial().ser4;
	    //[6] to [9] are the target ser, not applicable here.
	    effect[10]= eff1;// Object id of the effect
	    effect[11]= eff2;
	    effect[12]= charpos.x >> 8;
	    effect[13]= charpos.x % 256;
	    effect[14]= charpos.y >> 8;
	    effect[15]= charpos.y % 256;
	    effect[16]= charpos.z;
	    //[17] to [21] are the target's position, not applicable here.
	    effect[22]= speed;
	    effect[23]= loop; // 0 is really long. 1 is the shortest.
	    effect[24]= 0; // This value is unknown
	    effect[25]= 0; // This value is unknown
	    effect[26]= 1; // CLient side crashfix
	    effect[27]= 0; // This value is used for moving effects that explode on impact.
	 }

	 if (!UO3DonlyEffekt) // no UO3D effect ? lets send old effect to all clients
	 {

		 NxwSocketWrapper sw;
		 sw.fillOnline( pc, false );
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			Xsend(sw.getSocket(), effect, 28);
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

	char effect[29];
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	Location srcpos= src->getPosition();
	Location destpos= dst->getPosition();

	if (!skip_old)
	{
	   effect[0]= 0x70; // Effect message
	   effect[1]= 0x00; // Moving effect
	   effect[2]= src->getSerial().ser1;
	   effect[3]= src->getSerial().ser2;
	   effect[4]= src->getSerial().ser3;
	   effect[5]= src->getSerial().ser4;
	   effect[6]= dst->getSerial().ser1;
	   effect[7]= dst->getSerial().ser2;
	   effect[8]= dst->getSerial().ser3;
	   effect[9]= dst->getSerial().ser4;
	   effect[10]= eff1;// Object id of the effect
	   effect[11]= eff2;
	   effect[12]= srcpos.x >> 8;
	   effect[13]= srcpos.x % 256;
	   effect[14]= srcpos.y >> 8;
	   effect[15]= srcpos.y % 256;
	   effect[16]= srcpos.z;
	   effect[17]= destpos.x >> 8;
	   effect[18]= destpos.x % 256;
	   effect[19]= destpos.y >> 8;
	   effect[20]= destpos.y % 256;
	   effect[21]= destpos.z;
	   effect[22]= speed;
	   effect[23]= loop; // 0 is really long. 1 is the shortest.
	   effect[24]= 0; // This value is unknown
	   effect[25]= 0; // This value is unknown
	   effect[26]= 0; //1; // LB, potential crashfix
	   effect[27]= explode; // This value is used for moving effects that explode on impact.
	}


	 if (!UO3DonlyEffekt) // no UO3D effect ? lets send old effect to all clients
	 {
	   
		 NxwSocketWrapper sw;
		 sw.fillOnline( );
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			 NXWSOCKET j=sw.getSocket();
			 if ( (char_inVisRange(src,MAKE_CHAR_REF(currchar[j])))&&(char_inVisRange(MAKE_CHAR_REF(currchar[j]),dst))&&(perm[j]))
			 {
				Xsend(j, effect, 28);
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
			 if ( (char_inVisRange(src,MAKE_CHAR_REF(currchar[j])))&&(char_inVisRange(MAKE_CHAR_REF(currchar[j]),dst))&&(perm[j]))
			 {
				 if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd
				 {
					 Xsend(j, effect, 28);
	
				 } else if (clientDimension[j]==3) // 3d client, send 3d-Particles
				 {

					movingeffectUO3D(source, dest, str);
					unsigned char particleSystem[49];
					Xsend(j, particleSystem, 49);
				}
				else if (clientDimension[j] != 2 && clientDimension[j] !=3 ) { sprintf(temp, "Invalid Client Dimension: %i\n",clientDimension[j]); LogError(temp); }
			}
		}
	}


}

void bolteffect(CHARACTER player, bool UO3DonlyEffekt, bool skip_old )
{

	P_CHAR pc=MAKE_CHAR_REF(player);
	VALIDATEPC(pc);


	char effect[29];
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	Location charpos= pc->getPosition();

	if (!skip_old)
	{
	  memset(&effect[0], 0, 29);
	  effect[0]= 0x70; // Effect message
	  effect[1]= 0x01; // Bolt effect
	  effect[2]= pc->getSerial().ser1;
	  effect[3]= pc->getSerial().ser2;
	  effect[4]= pc->getSerial().ser3;
	  effect[5]= pc->getSerial().ser4;
	  //[6] to [11] are not applicable here.
	  effect[12]= charpos.x >> 8;
	  effect[13]= charpos.x % 256;
	  effect[14]= charpos.y >> 8;
	  effect[15]= charpos.y % 256;
	  effect[16]= charpos.z;
	  //[17] to [27] are not applicable here.
	  effect[26]=1; // LB possible client crashfix
	  effect[27]=0;
	}

	 if (!UO3DonlyEffekt) // no UO3D effect ? lets send old effect to all clients
	 {
		 NxwSocketWrapper sw;
		 sw.fillOnline( pc );
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			 NXWSOCKET j=sw.getSocket();
			 if( j!=INVALID )
			 {
				Xsend(j, effect, 28);
			}
	   }
	   return;
	}
	else
	{
		 NxwSocketWrapper sw;
		 sw.fillOnline( pc );
		 for( sw.rewind(); !sw.isEmpty(); sw++ )
		 {
			 NXWSOCKET j=sw.getSocket();
			 if( j!=INVALID )
			 {
			 if (clientDimension[j]==2 && !skip_old) // 2D client, send old style'd
			 {
				 Xsend(j, effect, 28);

			 } else if (clientDimension[j]==3) // 3d client, send 3d-Particles
			 {

				bolteffectUO3D(player);
				unsigned char particleSystem[49];
				Xsend(j, particleSystem, 49);
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

	char effect[29];
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	if (!skip_old)
	{
		memset(&effect[0], 0, 29);
		effect[0]=0x70; // Effect message
		effect[1]=0x02; // Static effect
		effect[2]= pi->getSerial().ser1;
		effect[3]= pi->getSerial().ser2;
		effect[4]= pi->getSerial().ser3;
		effect[5]= pi->getSerial().ser4;
		effect[6]= pi->getSerial().ser1;
		effect[7]= pi->getSerial().ser2;
		effect[8]= pi->getSerial().ser3;
		effect[9]= pi->getSerial().ser4;
		//[6] to [9] are the target ser, not applicable here.
		effect[10]=eff1;// Object id of the effect
		effect[11]=eff2;
		effect[12]= pi->getPosition("x") >> 8;
		effect[13]= pi->getPosition("x") % 256;
		effect[14]= pi->getPosition("y") >> 8;
		effect[15] =pi->getPosition("y") % 256;
		effect[16]= pi->getPosition("z");
		effect[17]= pi->getPosition("x") >> 8;
		effect[18]= pi->getPosition("x") % 256;
		effect[19]= pi->getPosition("y") >> 8;
		effect[20]= pi->getPosition("y") % 256;
		effect[21]= pi->getPosition("z");
		//[17] to [21] are the target's position, not applicable here.
		effect[22]=speed;
		effect[23]=loop; // 0 is really long. 1 is the shortest.
		effect[24]=0; // This value is unknown
		effect[25]=0; // This value is unknown
		effect[26]=1; // LB, client side crashfix
		effect[27]=explode; // This value is used for moving effects that explode on impact.
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
				}
				else if (clientDimension[j]==3) // 3d client, send 3d-Particles
				{
					itemeffectUO3D(pi, str);
					unsigned char particleSystem[49];
					Xsend(j, particleSystem, 49);
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

	char effect[29];
	int i, x2,x,y2,y;
	Location charpos= pc->getPosition();


	for (i=0;i<29;i++)
	{
		effect[i]=0;
	}
	effect[0]=0x70; // Effect message
	effect[1]=0x00; // effect from source to dest
	effect[2]=pc->getSerial().ser1;
	effect[3]=pc->getSerial().ser2;
	effect[4]=pc->getSerial().ser3;
	effect[5]=pc->getSerial().ser4;

	effect[10]=a1;
	effect[11]=a2;

	y=rand()%36;
	x=rand()%36;

	if (rand()%2==0) x=x*-1;
	if (rand()%2==0) y=y*-1;
	x2= charpos.x + x;
	y2= charpos.y + y;
	if (x2<0) x2=0;
	if (y2<0) y2=0;
	if (x2>6144) x2=6144;
	if (y2>4096) y2=4096;

	// ConOut("bolt: %i %i %i %i %i %i\n",x2,y2,chars[player].x,chars[player].y,x,y);

	effect[12]= charpos.x >> 8; // source coordinates
	effect[13]= charpos.x % 256;
	effect[14]= charpos.y >> 8;
	effect[15]= charpos.y % 256;
	effect[16]= 0;

	effect[17]= x2 >> 8;	//target coordiantes
	effect[18]= x2 % 256;
	effect[19]= y2 >> 8;
	effect[20]= y2 % 256;
	effect[21]= 127;

	//[22] to [27] are not applicable here.

	effect[26]= 1; // client crash bugfix
	effect[27]= 0;

	 NxwSocketWrapper sw;
	 sw.fillOnline( pc );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
		{
			Xsend(j, effect, 28);
		}
	}

}

//	- Movingeffect3 is used to send an object from a char
//    to another object (like purple potions)
void movingeffect3(CHARACTER source, unsigned short x, unsigned short y, signed char z, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode)
{

	P_CHAR src=MAKE_CHAR_REF(source);
	VALIDATEPC(src);

	char effect[29];
	Location srcpos= src->getPosition();

	memset (&effect, 0, 29);
	effect[0]= 0x70; // Effect message
	effect[1]= 0x00; // Moving effect
	effect[2]= src->getSerial().ser1;
	effect[3]= src->getSerial().ser2;
	effect[4]= src->getSerial().ser3;
	effect[5]= src->getSerial().ser4;
	effect[6]= 0;
	effect[7]= 0;
	effect[8]= 0;
	effect[9]= 0;
	effect[10]= eff1;// Object id of the effect
	effect[11]= eff2;
	effect[12]= srcpos.x >> 8;
	effect[13]= srcpos.x % 256;
	effect[14]= srcpos.y >> 8;
	effect[15]= srcpos.y % 256;
	effect[16]= srcpos.z;
	effect[17]= x >> 8;
	effect[18]= x % 256;
	effect[19]= y >> 8;
	effect[20]= y % 256;
	effect[21]= z;
	effect[22]= speed;
	effect[23]= loop; // 0 is really long.  1 is the shortest.
	effect[24]= 0; // This value is unknown
	effect[25]= 0; // This value is unknown
	effect[26]= 0; // This value is unknown
	effect[27]= explode; // This value is used for moving effects that explode on impact.

	 NxwSocketWrapper sw;
	 sw.fillOnline( src );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
		{   
			Xsend(j, effect, 28);
		
		}
	 }

}

// staticeffect3 is for effects on items
void staticeffect3(UI16 x, UI16 y, SI08 z, unsigned char eff1, unsigned char eff2, char speed, char loop, char explode)
{


	char effect[29];
	memset (&effect, 0, 29);
	effect[0]=0x70; // Effect message
	effect[1]=0x02; // Static effect
	//[6] to [9] are the target ser, not applicable here.
	effect[10]=eff1;// Object id of the effect
	effect[11]=eff2;
	effect[12]=x>>8;
	effect[13]=x%256;
	effect[14]=y>>8;
	effect[15]=y%256;
	effect[16]=z;
	effect[17]=x>>8;
	effect[18]=x%256;
	effect[19]=y>>8;
	effect[20]=y%256;
	effect[21]=z;
	//[17] to [21] are the target's position, not applicable here.
	effect[22]=speed;
	effect[23]=loop; // 0 is really long.  1 is the shortest.
	effect[24]=0; // This value is unknown
	effect[25]=0; // This value is unknown
	effect[26]=1; // LB changed to 1
	effect[27]=explode; // This value is used for moving effects that explode on impact.


	Location location;
	location.x=x;
	location.y=y;

	 NxwSocketWrapper sw;
	 sw.fillOnline( location );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
			Xsend(j, effect, 28);
	}

}

void movingeffect3(CHARACTER source, CHARACTER dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode,unsigned char unk1,unsigned char unk2,unsigned char ajust,unsigned char type)
{

	P_CHAR src=MAKE_CHAR_REF(source);
	VALIDATEPC(src);
	P_CHAR dst=MAKE_CHAR_REF(dest);
	VALIDATEPC(dst);


	//0x0f 0x42 = arrow 0x1b 0xfe=bolt
	char effect[29];
	Location srcpos= src->getPosition();
	Location destpos= dst->getPosition();

	effect[0]= 0x70; // Effect message
	effect[1]= type; // Moving effect
	effect[2]= src->getSerial().ser1;
	effect[3]= src->getSerial().ser2;
	effect[4]= src->getSerial().ser3;
	effect[5]= src->getSerial().ser4;
	effect[6]= dst->getSerial().ser1;
	effect[7]= dst->getSerial().ser2;
	effect[8]= dst->getSerial().ser3;
	effect[9]= dst->getSerial().ser4;
	effect[10]= eff1;// Object id of the effect
	effect[11]= eff2;
	effect[12]= srcpos.x >> 8;
	effect[13]= srcpos.x%256;
	effect[14]= srcpos.y>>8;
	effect[15]= srcpos.y%256;
	effect[16]= srcpos.z;
	effect[17]= destpos.x>>8;
	effect[18]= destpos.x%256;
	effect[19]= destpos.y>>8;
	effect[20]= destpos.y%256;
	effect[21]= destpos.z;
	effect[22]=speed;
	effect[23]=loop; // 0 is really long. 1 is the shortest.
	effect[24]=unk1; // This value is unknown
	effect[25]=unk2; // This value is unknown
	effect[26]=ajust; // LB, potential crashfix
	effect[27]=explode; // This value is used for moving effects that explode on impact.

	 NxwSocketWrapper sw;
	 sw.fillOnline( );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
			Xsend(j, effect, 28);
	}

}



//	- Movingeffect2 is used to send an object from a char
//	to another object (like purple potions)
void movingeffect2(CHARACTER source, int dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode)
{
	//0x0f 0x42 = arrow 0x1b 0xfe=bolt

	const P_ITEM pi=MAKE_ITEMREF_LR(dest);	// on error return
	P_CHAR pc_source = MAKE_CHAR_REF(source);
	VALIDATEPC(pc_source);

	UI08 effect[29];
	Location srcpos= pc_source->getPosition();

	effect[0]= 0x70; // Effect message
	effect[1]= 0x00; // Moving effect
	LongToCharPtr(pc_source->getSerial32(), effect+2);
	LongToCharPtr(pi->getSerial32(), effect+6);
	effect[10]= eff1;// Object id of the effect
	effect[11]= eff2;
	effect[12]= srcpos.x>>8;
	effect[13]= srcpos.x%256;
	effect[14]= srcpos.y>>8;
	effect[15]= srcpos.y%256;
	effect[16]= srcpos.z;
	effect[17]= pi->getPosition("x") >> 8;
	effect[18]= pi->getPosition("x") % 256;
	effect[19]= pi->getPosition("y") >> 8;
	effect[20]= pi->getPosition("y") % 256;
	effect[21]= pi->getPosition("z");
	effect[22]= speed;
	effect[23]= loop; // 0 is really long. 1 is the shortest.
	effect[24]= 0; // This value is unknown
	effect[25]= 0; // This value is unknown
	effect[26]= 0; //1; // LB potential crashfix
	effect[27]=explode; // This value is used for moving effects that explode on impact.

	 NxwSocketWrapper sw;
	 sw.fillOnline( );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET j=sw.getSocket();
		if( j!=INVALID )
			Xsend(j, effect, 28);
	}

}

void dolight(NXWSOCKET s, char level)
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);


	char light[3]="\x4F\x00";

	if ((s==INVALID)||(!perm[s])) return;

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

}

void updateskill(NXWSOCKET s, int skillnum) // updated for client 1.26.2b by LB
{

	P_CHAR pc_currchar = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_currchar);

	char update[11];
	char x;


	update[0] = 0x3A; // Skill Update Message
	update[1] = 0x00; // Length of message
	update[2] = 0x0B; // Length of message
	update[3] = '\xFF'; // single list

	update[4] = 0x00;
	update[5] = (char)skillnum;
	update[6] = pc_currchar->skill[skillnum] >> 8;
	update[7] = pc_currchar->skill[skillnum]%256;
	update[8] = pc_currchar->baseskill[skillnum] >> 8;
	update[9] = pc_currchar->baseskill[skillnum]%256;
	x = pc_currchar->lockSkill[skillnum];
	if (x != 0 && x != 1 && x != 2)
		x = 0;
	update[10] = x;

	// CRASH_IF_INVALID_SOCK(s);

	Xsend(s, update, 11);

}

void deathaction(P_CHAR pc, P_ITEM pi)
{
	unsigned char deathact[14]="\xAF\x01\x02\x03\x04\x01\x02\x00\x05\x00\x00\x00\x00";

	LongToCharPtr(pc->getSerial32(), deathact+1);
	LongToCharPtr(pi->getSerial32(), deathact+5);

	 NxwSocketWrapper sw;
	 sw.fillOnline( pc, true );
	 for( sw.rewind(); !sw.isEmpty(); sw++ )
	 {
		NXWSOCKET i=sw.getSocket();
		if( i!=INVALID )
			Xsend(i, deathact, 13);
	}


}

void deathmenu(NXWSOCKET s) // Character sees death menu
{


	char testact[3]="\x2C\x00";
	Xsend(s, testact, 2);

}

void impowncreate(NXWSOCKET s, P_CHAR pc, int z) //socket, player to send
{
	int k;
	unsigned char oc[1024];
	P_CHAR pc_currchar=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc_currchar);

	Location charpos= pc->getPosition();

	UI08 removeitem[5]={ 0x1D, 0x00, 0x00, 0x00, 0x00 };

	if (pc->stablemaster_serial!=INVALID || pc->mounted) 
		return; // dont **show** stabled pets

	bool sendit = true; //Luxor bug fix
	if (pc->IsHidden() && pc->getSerial32()!=pc_currchar->getSerial32() && !pc_currchar->IsGM()) 
		sendit=false; 

	if( !pc->npc && !pc->IsOnline()  && !pc_currchar->IsGM() )
	{
		sendit=false;
		LongToCharPtr(pc->getSerial32(), removeitem+1);
		Xsend(s, removeitem, 5);
	}
	// hidden chars can only be seen "grey" by themselves or by gm's
	// other wise they are invisible=dont send the packet
	if (!sendit) 
		return;

	oc[0]=0x78; // Message type 78

	LongToCharPtr(pc->getSerial32(), oc+3);
	oc[7]= pc->id1; // Character art id
	oc[8]= pc->id2; // Character art id
	ShortToCharPtr(charpos.x, oc+9);
	ShortToCharPtr(charpos.y, oc+11);
	if (z)
		oc[13]= charpos.dispz; // Character z position
	else
		oc[13]= charpos.z;
	oc[14]= pc->dir; // Character direction
	oc[15]= pc->skin1; // Character skin color
	oc[16]= pc->skin2; // Character skin color
	oc[17]=0; // Character flags
	if (pc->IsHidden() || !(pc->IsOnline()||pc->npc))
		oc[17]|=0x80; // Show hidden state correctly
	if (pc->poisoned)
		oc[17]|=0x04; //AntiChrist -- thnx to SpaceDog

	k=19;
	int guild;
	guild=Guilds->Compare(pc_currchar,pc);
	if (guild==1)//Same guild (Green)
		oc[18]=2;
	else if (guild==2) // Enemy guild.. set to orange
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
				ShortToCharPtr(pj->id(), oc+k+4);
				oc[k+6]=pj->layer;
				k=k+7;
				if (pj->color1!=0 || pj->color2!=0)
				{
					oc[k-3]|=0x80;
					oc[k+0]=pj->color1;
					oc[k+1]=pj->color2;
					k=k+2;
				}
				layers[pj->layer] = 1;
			}
	}

	oc[k+0]=0;// Not well understood. It's a serial number. I set this to my serial number,
	oc[k+1]=0;// and all of my messages went to my paperdoll gump instead of my character's
	oc[k+2]=0;// head, when I was a character with serial number 0 0 0 1.
	oc[k+3]=0;
	k=k+4;

	// unimportant remark: its a packet "terminator" !!! LB

	oc[1]=k>>8;
	oc[2]=k%256;
	Xsend(s, oc, k);
	//pc_currchar->sysmsg( "sended %s", pc->getCurrentNameC() );


}

void sendshopinfo(int s, int c, P_ITEM pi)
{

	VALIDATEPI(pi);

	unsigned char m1[6096];
	unsigned char m2[6096];
	char itemname[256];
	char cFoundItems=0;
	memset(m1,0,6096);
	memset(m2,0,6096);
	memset(itemname,0,256);
	int k, m1t, m2t, value,serial;

	m1[0]=0x3C; // Container content message
	m1[1]=0;// Size of message
	m1[2]=0;// Size of message
	m1[3]=0;//  Count of items
	m1[4]=0;// Count of items
	m2[0]=0x74;// Buy window details message
	m2[1]=0;// Size of message
	m2[2]=8;// Size of message
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
				if (m2t>6000 || m1t>6000) break;

				LongToCharPtr(pj->getSerial32(), m1+m1t+0);//Item serial number
				ShortToCharPtr(pj->id(),m1+m1t+4);
				m1[m1t+6]=0;			//Always zero
				m1[m1t+7]=pj->amount>>8;//Amount for sale
				m1[m1t+8]=pj->amount%256;//Amount for sale
				m1[m1t+9]=loopexit>>8;//pj->getPosition("x")>>8;//items[j].x/256; //Item x position
				m1[m1t+10]=loopexit%256;//pj->getPosition("x")%256;//items[j].x%256;//Item x position
				m1[m1t+11]=loopexit>>8;//pj->getPosition("y")>>8;//items[j].y/256;//Item y position
				m1[m1t+12]=loopexit%256;//pj->getPosition("y")%256;//items[j].y%256;//Item y position
				LongToCharPtr(pi->getSerial32(), m1+m1t+13); //Container serial number
				m1[m1t+17]=pj->color1;//Item color
				m1[m1t+18]=pj->color2;//Item color
				m1[4]++; // Increase item count.
				m1t=m1t+19;
				value=pj->value;
				value=calcValue(DEREF_P_ITEM(pj), value);
				if (SrvParms->trade_system==1) 
					value=calcGoodValue(c,DEREF_P_ITEM(pj),value,0); // by Magius(CHE)
				m2[m2t+0]=value>>24;// Item value/price
				m2[m2t+1]=value>>16;//Item value/price
				m2[m2t+2]=value>>8; // Item value/price
				m2[m2t+3]=value%256; // Item value/price
				m2[m2t+4]=pj->getName(itemname); // Item name length

				for(k=0;k<m2[m2t+4];k++)
				{
				  	m2[m2t+5+k]=itemname[k];
				}

				m2t=m2t+(m2[m2t+4])+5;
				m2[7]++;
				cFoundItems=1; //we found items so send message

			}
	}

	m1[1]=m1t>>8;
	m1[2]=m1t%256;
	m2[1]=m2t>>8;
	m2[2]=m2t%256;

	if (cFoundItems==1)
	{
		Xsend(s, m1, m1t);
		Xsend(s, m2, m2t);
	}

}

int sellstuff(NXWSOCKET s, CHARACTER i)
{
	if (s < 0 || s >= now) return 0; //Luxor
    P_CHAR pc = MAKE_CHARREF_LRV(i, 0);
	P_CHAR pcs = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pcs,0);

	char itemname[256];
	int m1t, z, value;
	int serial,serial1;
	unsigned char m1[2048];
	unsigned char m2[2];
	char ciname[256]; // By Magius(CHE)
	char cinam2[256]; // By Magius(CHE)

	serial=pc->getSerial32();
	/*for (ci=0;ci<pointers::pContMap[serial].size();ci++)
	{*/
	//<Luxor>

	P_ITEM pp=pc->GetItemOnLayer(0x1C);
	VALIDATEPIR(pp,0);

	m2[0]=0x33;
	m2[1]=0x01;
	Xsend(s, m2, 2);

	P_ITEM pack= pcs->getBackpack();
	VALIDATEPIR(pack, 0);

	m1[0]='\x9E'; // Header
	m1[1]=0; // Size
	m1[2]=0; // Size
	m1[3]=pc->getSerial().ser1;
	m1[4]=pc->getSerial().ser2;
	m1[5]=pc->getSerial().ser3;
	m1[6]=pc->getSerial().ser4;
	m1[7]=0; // Num items
	m1[8]=0; // Num items
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

					if (pj1->id()==pj->id()  && 
						pj1->type==pj->type && 
						((SrvParms->sellbyname==0)||(SrvParms->sellbyname==1 && (!strcmp(ciname,cinam2))))) // If the names are the same! --- Magius(CHE)
					{
						LongToCharPtr(pj1->getSerial32(), m1+m1t+0);
						ShortToCharPtr(pj1->id(),m1+m1t+4);
						ShortToCharPtr(pj1->color(),m1+m1t+6);
						ShortToCharPtr(pj1->amount,m1+m1t+8);
						value=pj->value;
						value=calcValue(DEREF_P_ITEM(pj1), value);
						if (SrvParms->trade_system==1)
							value=calcGoodValue(i,DEREF_P_ITEM(pj1),value,1); // by Magius(CHE)
						m1[m1t+10]=value>>8;
						m1[m1t+11]=value%256;
						m1[m1t+12]=0;// Unknown... 2nd length byte for string?
						m1[m1t+13]=pj1->getName(itemname);
						m1t=m1t+14;
						for(z=0;z<m1[m1t-1];z++)
						{
							m1[m1t+z]=itemname[z];
						}
						m1t=m1t+m1[m1t-1];
						m1[8]++;
					}
				}
				
			}
		}
	}

	m1[1]=m1t>>8;
	m1[2]=m1t%256;
	if (m1[8]<51) //With too many items, server crashes
	{
		if (m1[8]!=0)
		{
			Xsend(s, m1, m1t);
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
	m2[0]=0x33;
	m2[1]=0x00;
	Xsend(s, m2, 2);
	return 1;

}

void playmidi(int s, char num1, char num2)
{
	UI08 msg[3] = { 0x06D, num1, num2 };
	Xsend(s, msg, 3);
}

void sendtradestatus(int cont1, int cont2)
{

	P_ITEM c1=MAKE_ITEM_REF(cont1);
	VALIDATEPI(c1);
	P_ITEM c2=MAKE_ITEM_REF(cont2);
	VALIDATEPI(c2);


	unsigned char msg[30];
	int p1, p2, s1, s2;

	p1=calcCharFromSer(c1->getContSerial());
	p2=calcCharFromSer(c2->getContSerial());
	s1=calcSocketFromChar(p1);
	s2=calcSocketFromChar(p2);

	msg[0]=0x6F;//Header
	msg[1]=0x00;//Size
	msg[2]=0x11;//Size
	msg[3]=0x02;//State
	LongToCharPtr(c1->getSerial32(), msg+4);
	msg[8]=0;
	msg[9]=0;
	msg[10]=0;
	msg[11]=c1->morez%256;
	msg[12]=0;
	msg[13]=0;
	msg[14]=0;
	msg[15]=c2->morez%256;
	msg[16]=0; // No name in this message
	Xsend(s1, msg, 17);

	LongToCharPtr(c2->getSerial32(), msg+4);
	msg[11]=c2->morez%256;
	msg[15]=c1->morez%256;
	Xsend(s2, msg, 17);

}

void endtrade(int b1, int b2, int b3, int b4)
{
	P_ITEM c1=pointers::findItemBySerial(calcserial(b1, b2, b3, b4));
	VALIDATEPI(c1);
	P_ITEM c2=pointers::findItemBySerial(calcserial(c1->moreb1, c1->moreb2, c1->moreb3, c1->moreb4));
	VALIDATEPI(c2);

	unsigned char msg[30];

	P_CHAR pc1=pointers::findCharBySerial(c1->getContSerial());
	VALIDATEPC(pc1);

	P_CHAR pc2=pointers::findCharBySerial(c2->getContSerial());
	VALIDATEPC(pc2);


	P_ITEM bp1= pc1->getBackpack();
	VALIDATEPI(bp1);
	P_ITEM bp2= pc2->getBackpack();
	VALIDATEPI(bp2);
	NXWSOCKET s1=calcSocketFromChar(DEREF_P_CHAR(pc1));
	NXWSOCKET s2=calcSocketFromChar(DEREF_P_CHAR(pc2));

	msg[0]=0x6F;//Header Byte
	msg[1]=0x00;//Size
	msg[2]=0x11;//Size
	msg[3]=0x01;//State byte
	LongToCharPtr(c1->getSerial32(), msg+4);
	msg[8]=0;
	msg[9]=0;
	msg[10]=0;
	msg[11]=0;
	msg[12]=0;
	msg[13]=0;
	msg[14]=0;
	msg[15]=0;
	msg[16]=0;
	if (s1 > -1)	// player may have been disconnected (Duke)
		Xsend(s1, msg, 17);

	msg[0]=0x6F;//Header Byte
	msg[1]=0x00;//Size
	msg[2]=0x11;//Size
	msg[3]=0x01;//State byte
	LongToCharPtr(c2->getSerial32(), msg+4);
	msg[8]=0;
	msg[9]=0;
	msg[10]=0;
	msg[11]=0;
	msg[12]=0;
	msg[13]=0;
	msg[14]=0;
	msg[15]=0;
	msg[16]=0;
	if (s2 > -1)	// player may have been disconnected (Duke)
		Xsend(s2, msg, 17);

	
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

	c1->deleteItem();
	c2->deleteItem();

}

void tellmessage(int i, int s, char *txt)
//Modified by N6 to use UNICODE packets
{

	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	unsigned char talk2[19];
	char unicodetext[512];
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	sprintf(temp, TRANSLATE("GM tells %s: %s"), pc->getCurrentNameC(), txt);

	int ucl = ( strlen ( temp ) * 2 ) + 2 ;
	int tl = ucl + 48 ;
	char2wchar(temp);
	memcpy(unicodetext, Unicode::temp, ucl);

	talk2[0] = (char)0xAE;
	talk2[1] = tl >> 8;
	talk2[2] = tl&0xFF;
	talk2[3]=1;
	talk2[4]=1;
	talk2[5]=1;
	talk2[6]=1;
	talk2[7]=1;
	talk2[8]=1;
	talk2[9]=0;
	talk2[10]=0x00; //First Part  \_Yellow
	talk2[11]=0x35; //Second Part /
	talk2[12]=0;
	talk2[13]=3;

	talk2[14] = server_data.Unicodelanguage[0];
	talk2[15] = server_data.Unicodelanguage[1];
	talk2[16] = server_data.Unicodelanguage[2];
	talk2[17] = 0;

	unsigned char sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	Xsend(s, talk2, 18);
	Xsend(s, sysname, 30);
	Xsend(s, unicodetext, ucl);
	Xsend(i, talk2, 18);//So Person who said it can see too
	Xsend(i, sysname, 30);
	Xsend(i, unicodetext, ucl);

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


   PC_CHAR pc_cs=MAKE_CHARREF_LOGGED(source,err);
   if (err) return;
   PC_CHAR pc_cd=MAKE_CHARREF_LOGGED(dest, err);
   if (err) return;

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

void sysmessageflat(NXWSOCKET  s, short color, const char *txt) // System message (In lower left corner)
//Modified by N6 to use UNICODE packets
{

	unsigned char talk2[19];
	char unicodetext[512];
	int ucl = ( strlen ( txt ) * 2 ) + 2 ;

	int tl = ucl + 48 ;
	char2wchar(txt);
	memcpy(unicodetext, Unicode::temp, ucl);

	talk2[0] = (char)0xAE;
	talk2[1] = tl >> 8;
	talk2[2] = tl&0xFF;
	talk2[3] = 1;
	talk2[4] = 1;
	talk2[5] = 1;
	talk2[6] = 1;
	talk2[7] = 1;
	talk2[8] = 1;
	talk2[9] = 6;
	talk2[10]= color>>8; // UOLBR patch to prevent client crash by Juliunus
	talk2[11]= color%256;
	talk2[12] = 0;
	talk2[13] = 3;

	talk2[14] = server_data.Unicodelanguage[0];
	talk2[15] = server_data.Unicodelanguage[1];
	talk2[16] = server_data.Unicodelanguage[2];
	talk2[17] = 0;
	unsigned char sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	Xsend(s, talk2, 18);
	Xsend(s, sysname, 30);
	Xsend(s, unicodetext, ucl);

}

void wornitems(NXWSOCKET  s, CHARACTER j) // Send worn items of player j
{
	P_CHAR pj=MAKE_CHAR_REF(j);
	VALIDATEPC(pj);

	NxwItemWrapper si;
	si.fillItemWeared( pj, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if(ISVALIDPI(pi))
			wearIt(s,pi);
	}

}

