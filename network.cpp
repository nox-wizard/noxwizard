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
#include "race.h"
#include "commands.h"
#include "packets.h"
#include "basics.h"
#include "menu.h"
#include "addmenu.h"
#include "accounts.h"
#include "party.h"
#include "calendar.h"
#include "dragdrop.h"
#include "speech.h"
#include "data.h"
#include "books.h"
#include "boats.h"
#include "scp_parser.h"
#include "dbl_single_click.h"
#include "trade.h"
#include "sndpkg.h"
#include "walking.h"
#include "rcvpkg.h"
#include "items.h"
#include "chars.h"
#include "range.h"
#include "inlines.h"
#include "skills.h"
#include "nox-wizard.h"
#include "scripts.h"


#ifdef ENCRYPTION
#include "encryption/clientcrypt.h"
#endif
//#define USE_MTHREAD_SEND

cNetwork	*Network;

#define PACKET_LEN_DYNAMIC		0x0000
#define PACKET_LEN_NONE			0xffff

#define PACKET_CREATECHARACTER		0x00
#define PACKET_DISCONNECT		0x01
#define PACKET_MOVE_REQUEST		0x02
#define PACKET_TALK_REQUEST		0x03
#define PACKET_ATTACK_REQUEST		0x05
#define PACKET_DOUBLECLICK		0x06
#define PACKET_PICKUPITEM		0x07
#define PACKET_DROPITEM			0x08
#define PACKET_SINGLECLICK		0x09
#define PACKET_REQUESTACTION		0x12
#define PACKET_WEARITEM			0x13
#define PACKET_RESYNC_REQUEST		0x22
#define PACKET_RESURRECT_CHOICE		0x2C
#define PACKET_STATUS_REQUEST		0x34
#define PACKET_SET_SKILL_LOCKS		0x3A
#define PACKET_BUYITEM			0x3B
#define PACKET_CHARACTERSELECT		0x5D
#define PACKET_READBOOK			0x66
#define PACKET_CHANGE_TEXTCOLOR		0x69
#define PACKET_TARGETING		0x6C
#define PACKET_SECURETRADING 		0x6F
#define PACKET_MSGBOARD			0x71
#define PACKET_WARMODE_CHANGE		0x72
#define PACKET_PING			0x73
#define PACKET_RENAMECHARACTER		0x75
#define PACKET_DIALOG_RESPONSE		0x7D
#define PACKET_FIRSTLOGINREQUEST	0x80
#define PACKET_DELETECHAR		0x83
#define PACKET_LOGINREQUEST		0x91
#define PACKET_BOOKCHANGE		0x93
#define PACKET_DYEITEM			0x95
#define PACKET_REQUESTHELP		0x9B
#define PACKET_SELLITEM			0x9F
#define PACKET_SELECTSERVER		0xA0
#define PACKET_SPYCLIENT		0xA4
#define PACKET_REQUEST_TIP		0xA7
#define PACKET_GUMP_TEXTENTRY_INPUT	0xAC
#define PACKET_UNICODE_TALKREQUEST	0xAD
#define PACKET_GUMPMENU_SELECT		0xB1
#define PACKET_TIPS_REQUEST      	0xB6
#define PACKET_PROFILE_REQUEST		0xB8
#define PACKET_UOMESSENGER		0xBB
#define PACKET_MISC_REQ			0xBF
#define PACKET_CLIENT_VERSION		0xBD

static int m_packetLen[256] =
{
// 0..15
	0x0068,	0x0005,	0x0007,	0x0000,	0x0002,	0x0005,	0x0005,	0x0007,	0x000E,	0x0005,	0x000B,	0x010A,	0x0000,	0x0003,	0x0000,	0x003D,
//16..31
	0x00D7,	0x0000,	0x0000,	0x000A,	0x0006,	0x0009,	0x0001,	0x0000,	0x0000,	0x0000,	0x0000,	0x0025,	0x0000,	0x0005,	0x0004,	0x0008,
//32..47
	0x0013,	0x0008,	0x0003,	0x001A,	0x0007,	0x0014,	0x0005,	0x0002,	0x0005,	0x0001,	0x0005,	0x0002,	0x0002,	0x0011,	0x000F,	0x000A,
//48..63
	0x0005,	0x0001,	0x0002,	0x0002,	0x000A,	0x028D,	0x0000,	0x0008,	0x0007,	0x0009,	0x0000,	0x0000,	0x0000,	0x0002,	0x0025,	0x0000,
//64..79
	0x00C9,	0x0000,	0x0000,	0x0229,	0x02C9,	0x0005,	0x0000,	0x000B,	0x0049,	0x005D, 0x0005,	0x0009,	0x0000,	0x0000,	0x0006,	0x0002,
//80..95
	0x0000,	0x0000,	0x0000,	0x0002,	0x000C,	0x0001,	0x000B,	0x006E,	0x006A,	0x0000,	0x0000,	0x0004,	0x0002,	0x0049,	0x0000,	0x0031,
//96..111
	0x0005,	0x0009,	0x000F,	0x000D,	0x0001,	0x0004,	0x0000,	0x0015,	0x0000,	0x0000,	0x0003,	0x0009,	0x0013,	0x0003,	0x000E,	0x0000,
//112..127
	0x001C,	0x0000,	0x0005,	0x0002,	0x0000,	0x0023,	0x0010,	0x0011,	0x0000,	0x0009,	0x0000,	0x0002,	0x0000,	0x000D,	0x0002,	0x0000,
//128..143
	0x003E,	0x0000,	0x0002,	0x0027,	0x0045,	0x0002,	0x0000,	0x0000,	0x0042,	0x0000,	0x0000,	0x0000,	0x000B,	0x0000,	0x0000,	0x0000,
//144..159
	0x0013,	0x0041,	0x0000,	0x0063,	0x0000,	0x0009,	0x0000,	0x0002,	0x0000,	0x001A,	0x0000,	0x0102,	0x0135,	0x0033,	0x0000,	0x0000,
//160..175
	0x0003,	0x0009,	0x0009,	0x0009,	0x0095,	0x0000,	0x0000,	0x0004,	0x0000,	0x0000,	0x0005,	0x0000,	0x0000,	0x0000,	0x0000,	0x000D,
//176..193
	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0040,	0x0009,	0x0000,	0x0000,	0x0003,	0x0006,	0x0009,	0x0003,	0x0000,	0x0000,	0x0000,
//192..207
	0x0024,	0x0000,	0x0000,	0x0000,	0x0006,	0x00CB,	0x0001,	0x0031,	0x0002,	0x0006,	0x0006,	0x0007,	0x0000,	0x0001,	0x0000,	0x004E,
//208..223
	0x0000,	0x0002,	0x0019,	0x0000,	0x0000,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,
//224..239
	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,
//240..255
	0x0000,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF
};

static unsigned int bit_table[257][2] =
{
{0x0002, 0x0000}, {0x0005, 0x001F}, {0x0006, 0x0022}, {0x0007, 0x0034}, {0x0007, 0x0075}, {0x0006, 0x0028}, {0x0006, 0x003B}, {0x0007, 0x0032},
{0x0008, 0x00E0}, {0x0008, 0x0062}, {0x0007, 0x0056}, {0x0008, 0x0079}, {0x0009, 0x019D}, {0x0008, 0x0097}, {0x0006, 0x002A}, {0x0007, 0x0057},
{0x0008, 0x0071}, {0x0008, 0x005B}, {0x0009, 0x01CC}, {0x0008, 0x00A7}, {0x0007, 0x0025}, {0x0007, 0x004F}, {0x0008, 0x0066}, {0x0008, 0x007D},
{0x0009, 0x0191}, {0x0009, 0x01CE}, {0x0007, 0x003F}, {0x0009, 0x0090}, {0x0008, 0x0059}, {0x0008, 0x007B}, {0x0008, 0x0091}, {0x0008, 0x00C6},
{0x0006, 0x002D}, {0x0009, 0x0186}, {0x0008, 0x006F}, {0x0009, 0x0093}, {0x000A, 0x01CC}, {0x0008, 0x005A}, {0x000A, 0x01AE}, {0x000A, 0x01C0},
{0x0009, 0x0148}, {0x0009, 0x014A}, {0x0009, 0x0082}, {0x000A, 0x019F}, {0x0009, 0x0171}, {0x0009, 0x0120}, {0x0009, 0x00E7}, {0x000A, 0x01F3},
{0x0009, 0x014B}, {0x0009, 0x0100}, {0x0009, 0x0190}, {0x0006, 0x0013}, {0x0009, 0x0161}, {0x0009, 0x0125}, {0x0009, 0x0133}, {0x0009, 0x0195},
{0x0009, 0x0173}, {0x0009, 0x01CA}, {0x0009, 0x0086}, {0x0009, 0x01E9}, {0x0009, 0x00DB}, {0x0009, 0x01EC}, {0x0009, 0x008B}, {0x0009, 0x0085},
{0x0005, 0x000A}, {0x0008, 0x0096}, {0x0008, 0x009C}, {0x0009, 0x01C3}, {0x0009, 0x019C}, {0x0009, 0x008F}, {0x0009, 0x018F}, {0x0009, 0x0091},
{0x0009, 0x0087}, {0x0009, 0x00C6}, {0x0009, 0x0177}, {0x0009, 0x0089}, {0x0009, 0x00D6}, {0x0009, 0x008C}, {0x0009, 0x01EE}, {0x0009, 0x01EB},
{0x0009, 0x0084}, {0x0009, 0x0164}, {0x0009, 0x0175}, {0x0009, 0x01CD}, {0x0008, 0x005E}, {0x0009, 0x0088}, {0x0009, 0x012B}, {0x0009, 0x0172},
{0x0009, 0x010A}, {0x0009, 0x008D}, {0x0009, 0x013A}, {0x0009, 0x011C}, {0x000A, 0x01E1}, {0x000A, 0x01E0}, {0x0009, 0x0187}, {0x000A, 0x01DC},
{0x000A, 0x01DF}, {0x0007, 0x0074}, {0x0009, 0x019F}, {0x0008, 0x008D}, {0x0008, 0x00E4}, {0x0007, 0x0079}, {0x0009, 0x00EA}, {0x0009, 0x00E1},
{0x0008, 0x0040}, {0x0007, 0x0041}, {0x0009, 0x010B}, {0x0009, 0x00B0}, {0x0008, 0x006A}, {0x0008, 0x00C1}, {0x0007, 0x0071}, {0x0007, 0x0078},
{0x0008, 0x00B1}, {0x0009, 0x014C}, {0x0007, 0x0043}, {0x0008, 0x0076}, {0x0007, 0x0066}, {0x0007, 0x004D}, {0x0009, 0x008A}, {0x0006, 0x002F},
{0x0008, 0x00C9}, {0x0009, 0x00CE}, {0x0009, 0x0149}, {0x0009, 0x0160}, {0x000A, 0x01BA}, {0x000A, 0x019E}, {0x000A, 0x039F}, {0x0009, 0x00E5},
{0x0009, 0x0194}, {0x0009, 0x0184}, {0x0009, 0x0126}, {0x0007, 0x0030}, {0x0008, 0x006C}, {0x0009, 0x0121}, {0x0009, 0x01E8}, {0x000A, 0x01C1},
{0x000A, 0x011D}, {0x000A, 0x0163}, {0x000A, 0x0385}, {0x000A, 0x03DB}, {0x000A, 0x017D}, {0x000A, 0x0106}, {0x000A, 0x0397}, {0x000A, 0x024E},
{0x0007, 0x002E}, {0x0008, 0x0098}, {0x000A, 0x033C}, {0x000A, 0x032E}, {0x000A, 0x01E9}, {0x0009, 0x00BF}, {0x000A, 0x03DF}, {0x000A, 0x01DD},
{0x000A, 0x032D}, {0x000A, 0x02ED}, {0x000A, 0x030B}, {0x000A, 0x0107}, {0x000A, 0x02E8}, {0x000A, 0x03DE}, {0x000A, 0x0125}, {0x000A, 0x01E8},
{0x0009, 0x00E9}, {0x000A, 0x01CD}, {0x000A, 0x01B5}, {0x0009, 0x0165}, {0x000A, 0x0232}, {0x000A, 0x02E1}, {0x000B, 0x03AE}, {0x000B, 0x03C6},
{0x000B, 0x03E2}, {0x000A, 0x0205}, {0x000A, 0x029A}, {0x000A, 0x0248}, {0x000A, 0x02CD}, {0x000A, 0x023B}, {0x000B, 0x03C5}, {0x000A, 0x0251},
{0x000A, 0x02E9}, {0x000A, 0x0252}, {0x0009, 0x01EA}, {0x000B, 0x03A0}, {0x000B, 0x0391}, {0x000A, 0x023C}, {0x000B, 0x0392}, {0x000B, 0x03D5},
{0x000A, 0x0233}, {0x000A, 0x02CC}, {0x000B, 0x0390}, {0x000A, 0x01BB}, {0x000B, 0x03A1}, {0x000B, 0x03C4}, {0x000A, 0x0211}, {0x000A, 0x0203},
{0x0009, 0x012A}, {0x000A, 0x0231}, {0x000B, 0x03E0}, {0x000A, 0x029B}, {0x000B, 0x03D7}, {0x000A, 0x0202}, {0x000B, 0x03AD}, {0x000A, 0x0213},
{0x000A, 0x0253}, {0x000A, 0x032C}, {0x000A, 0x023D}, {0x000A, 0x023F}, {0x000A, 0x032F}, {0x000A, 0x011C}, {0x000A, 0x0384}, {0x000A, 0x031C},
{0x000A, 0x017C}, {0x000A, 0x030A}, {0x000A, 0x02E0}, {0x000A, 0x0276}, {0x000A, 0x0250}, {0x000B, 0x03E3}, {0x000A, 0x0396}, {0x000A, 0x018F},
{0x000A, 0x0204}, {0x000A, 0x0206}, {0x000A, 0x0230}, {0x000A, 0x0265}, {0x000A, 0x0212}, {0x000A, 0x023E}, {0x000B, 0x03AC}, {0x000B, 0x0393},
{0x000B, 0x03E1}, {0x000A, 0x01DE}, {0x000B, 0x03D6}, {0x000A, 0x031D}, {0x000B, 0x03E5}, {0x000B, 0x03E4}, {0x000A, 0x0207}, {0x000B, 0x03C7},
{0x000A, 0x0277}, {0x000B, 0x03D4}, {0x0008, 0x00C0}, {0x000A, 0x0162}, {0x000A, 0x03DA}, {0x000A, 0x0124}, {0x000A, 0x01B4}, {0x000A, 0x0264},
{0x000A, 0x033D}, {0x000A, 0x01D1}, {0x000A, 0x01AF}, {0x000A, 0x039E}, {0x000A, 0x024F}, {0x000B, 0x0373}, {0x000A, 0x0249}, {0x000B, 0x0372},
{0x0009, 0x0167}, {0x000A, 0x0210}, {0x000A, 0x023A}, {0x000A, 0x01B8}, {0x000B, 0x03AF}, {0x000A, 0x018E}, {0x000A, 0x02EC}, {0x0007, 0x0062},
{0x0004, 0x000D}
};

//unsigned long __stdcall ConnectionThread( void *Arg );

void charGetPopUpHelp(char *str, P_CHAR pc);
void itemGetPopUpHelp(char *str, P_ITEM pi);


#ifdef USE_MTHREAD_SEND
static NetThread* g_NT[MAXCLIENT];
#endif

NXWCLIENT getClientFromSocket( NXWSOCKET socket )
{
	if( socket < 0 )
		return NULL;
	if( socket >= now )
		return NULL;
	P_CHAR pc = pointers::findCharBySerial( currchar[socket] );
	if( ISVALIDPC( pc ) )
		return pc->getClient();
	else
		return NULL;
}

static void initClients()
{
	for (int i=0; i < MAXCLIENT; ++i)
	{
		currchar[i] = INVALID;
#ifdef USE_MTHREAD_SEND
	        g_NT[i] = NULL;
#endif
	}
}

#ifdef USE_MTHREAD_SEND

int MTsend( NXWSOCKET socket, char* xoutbuffer, int len, int boh )
{
    g_NT[ socket ]->enqueue( xoutbuffer, len );
    return 0;
}


#else

int MTsend( NXWSOCKET socket, char* xoutbuffer, int len, int boh )
{
	int sent, loopexit=30;
	while( --loopexit>0 )
	{
		sent = send( client[ socket ], xoutbuffer, len, boh );

		if( sent < 0 && errno == 11 )
		{
			timeval timeout;
			timeout.tv_sec= 0;
			timeout.tv_usec= 5000;
			fd_set sockFD;
			FD_ZERO(&sockFD);
			FD_SET( socket, &sockFD);
			if( select( socket, NULL, &sockFD, NULL, &timeout ) == INVALID )
				LogError("Error selecting socket %i (%s)", errno, strerror( errno ) );
		}
		else
			break;
	}
	return sent;
}

#endif


void cNetwork::DoStreamCode( NXWSOCKET  socket )
{
	int status ;
/*
	FILE *debugout=fopen("d:\\packets.log", "a");
	for ( int i = 0; i < boutlength[socket];++i)
	{
		if ( i % 32 == 0 )
		{
			fprintf(debugout, "\n0x%04x ", i);
		}
		fprintf(debugout, "%02x ", outbuffer[socket][i]);
	}
	fprintf(debugout, "\n------------------------------------------", i);
	fclose(debugout);
*/
	int len = Pack( outbuffer[socket], xoutbuffer, boutlength[socket] );
	// ConOut("Packed %d bytes input to %d bytes out\n", boutlength[socket], len);
	NXWCLIENT ps = getClientFromSocket(socket);
#ifdef ENCRYPTION
	P_CHAR pc_currchar= (ps!=NULL)? ps->currChar() : NULL;
	if ( clientCrypter[socket] != NULL && clientCrypter[socket]->getCryptVersion() >= CRYPT_3_0_0c )
		clientCrypter[socket] ->encrypt((unsigned char *) &xoutbuffer[0], (unsigned char *) &xoutbuffer[0], len);
	else if (pc_currchar != NULL && pc_currchar->getCrypter() != NULL && pc_currchar->getCrypter()->getCryptVersion() >= CRYPT_3_0_0c )
		pc_currchar->getCrypter()->encrypt((unsigned char *) &xoutbuffer[0], (unsigned char *) &xoutbuffer[0], len);
#endif
	if ((status = MTsend(socket, xoutbuffer, len, MSG_NOSIGNAL)) == SOCKET_ERROR)
	{
   		LogSocketError("Socket Send error %s\n", errno) ;
	}

}


void cNetwork::FlushBuffer( NXWSOCKET socket ) // Sends buffered data at once
{

	int status ;
	if ( boutlength[ socket ] > 0 )
	{
		if ( clientInfo[ socket ]->compressOut )
		{
			DoStreamCode( socket );
		}
		else
		{
			if((status = MTsend( socket, (char*)outbuffer[socket], boutlength[socket], MSG_NOSIGNAL))==SOCKET_ERROR)
			{
				LogSocketError("Socket Send Error %s\n", errno) ;
			}
		}
		boutlength[socket]=0;
	}
}

void cNetwork::ClearBuffers() // Sends ALL buffered data
{
	for ( int i = 0; i < now; ++i )
		FlushBuffer( i );
}

void cNetwork::xSend( NXWSOCKET socket, const void *point, int length  ) // Buffering send function
{
	if( socket == INVALID || socket > MAXCLIENT )
	{
		LogWarning("XSend called with an invalid socket");
		return;
	}

	if ( boutlength[ socket ] + length > MAXBUFFER )
		FlushBuffer( socket );
	// Still too long
	if ( boutlength[ socket ] + length > MAXBUFFER )
	{
		memcpy( &outbuffer[ socket ][ boutlength[ socket ] ], point, MAXBUFFER );
		boutlength[ socket ] += MAXBUFFER;
		FlushBuffer( socket );
		xSend(socket, (char *)point + (int )MAXBUFFER, length - MAXBUFFER );
	}
	else
	{
		memcpy( &outbuffer[ socket ][ boutlength[ socket ] ], point, length );
		boutlength[ socket ] += length;
	}
}

void cNetwork::xSend(NXWSOCKET socket, wstring& p, bool alsoTermination )
{
	if( socket == INVALID || socket > MAXCLIENT )
	{
		LogWarning("XSend called with an invalid socket");
		return;
	}

//Luxor: if we are in a system which uses 32bit wchar_t, we must truncate them because UO client supports only 16bit unicode chars.
// So let's use UI16

	SI32 size = sizeof( UI16 );
	SI32 length = p.length() * size;
    if ( alsoTermination ) length += size;

    if ( boutlength[ socket ] + length > MAXBUFFER )
		FlushBuffer( socket );

	wstring::iterator point( p.begin() ), end( p.end() );
	UI16* b = (UI16*)&outbuffer[ socket ][ boutlength[ socket ] ];

	SI32 i = 0;
	for( ; point!=end; point++, ++i )
		b[i] = htons(*point);

	if( alsoTermination )
		b[ i ] = 0x0000;

	boutlength[ socket ] += length;
}



void cNetwork::Disconnect ( NXWSOCKET socket ) // Force disconnection of player //Instalog
{
	const char msgDisconnect[]	= "Client %i disconnected. [Total online clients: %i]\n";
	const char msgPart[]		= "%s has left the realm";

        if ( socket < 0 || socket >= now )
		return;

	P_CHAR pc = MAKE_CHAR_REF( currchar[ socket ] );

	int j,i;

	if ( now < 0 )
	{
		LogError("error in now-managment!\n");
		now = 0;
		return;
	}

	time_t ltime;
	time( &ltime );

	InfoOut( (char*)msgDisconnect, socket, now - 1 );

	if (SrvParms->server_log)
		ServerLog.Write( (char*)msgDisconnect, socket, now - 1 );

	if ( ISVALIDPC(pc) )
		if ( pc->account==acctno[ socket ] && SrvParms->partmsg && clientInfo[ socket ]->ingame && !pc->npc )
			sysbroadcast( (char*)msgPart, pc->getCurrentNameC() );

	if ( acctno[ socket ] != -1 )
		Accounts->SetOffline( acctno[ socket ] ); //Bug clearing logged in accounts!
	acctno[ socket ] = INVALID;

//	char val=0;

	if( ISVALIDPC( pc ) )
		if ( pc->IsOnline() )
		{

			LogOut( socket );


			if( pc->party!=INVALID ) {
				P_PARTY party = Partys.getParty( pc->party );
				if( party!=NULL )
					party->removeMember( pc );
			}


			SERIAL pc_serial = pc->getSerial32();

			for ( i = 0; i < now; ++i )
			{
				P_CHAR pi= MAKE_CHAR_REF( currchar[i] );
				if (ISVALIDPC(pi))
					if( pc != pi && char_inVisRange( pc, pi ) && clientInfo[ i ]->ingame )
					{
						SendDeleteObjectPkt(i, pc_serial);
					}
			}
		}

	FlushBuffer( socket );

	closesocket( client[ socket ] ); //so it bombs and free the mutex :]
#ifdef ENCRYPTION
	if ( clientCrypter[socket] != NULL && !clientCrypter[socket]->getEntering())
	{
		delete (ClientCrypt * ) clientCrypter[socket] ;
		clientCrypter[socket]=NULL;
	}
#endif
#ifdef USE_MTHREAD_SEND
	g_NT[ socket ]->mtxrun.enter();
	NetThread* NT = g_NT[ socket ];
#endif

	if( ISVALIDPC(pc) )
	{
		pc->setClient( NULL );

		if( pc->murderrate>uiCurrentTime ) //save murder decay
			pc->murdersave= (pc->murderrate -uiCurrentTime) / MY_CLOCKS_PER_SEC;

	}

	currchar[ socket ] = INVALID;

	safedelete( clientInfo[socket] );

	for ( j = socket; j < now - 1; ++j )
	{
		int jj = j+1;
		client[j]=client[jj];
		currchar[j]=currchar[jj];
		cryptedClient[j]=cryptedClient[jj];
#ifdef ENCRYPTION
		clientCrypter[jj] = clientCrypter[j];
#endif
		clientip[j][0]=clientip[jj][0];
		clientip[j][1]=clientip[jj][1];
		clientip[j][2]=clientip[jj][2];
		clientip[j][3]=clientip[jj][3];
		acctno[j]=acctno[jj];
		binlength[j]=binlength[jj];
		boutlength[j]=boutlength[jj];
		clientInfo[j]=clientInfo[jj];
		walksequence[j]=walksequence[jj];
		clientDimension[j]=clientDimension[jj];

		memcpy(&buffer[j], &buffer[jj], MAXBUFFER); // probably not nec.
		memcpy(&outbuffer[j], &outbuffer[jj], MAXBUFFER); // very important

#ifdef USE_MTHREAD_SEND
		g_NT[j] = g_NT[jj];
#endif
	}
#ifdef USE_MTHREAD_SEND
	g_NT[now] = NT;
#endif

	P_CHAR pj = NULL;
	for ( i = 0; i < MAXCLIENT; ++i ) {
		pj = pointers::findCharBySerial(currchar[i]);
		if ( ISVALIDPC(pj) )
			pj->setClient(NULL);

		if ( i >= now )
			currchar[i] = INVALID;
	}

	--now;

	for ( i = 0; i < now; ++i ) {
		pj = pointers::findCharBySerial(currchar[i]);
		if( ISVALIDPC(pj) )
			pj->setClient(new cNxwClientObj(i));
	}
}

void cNetwork::LoginMain(int s)
{
	signed long int i;
    unsigned char noaccount[2]={0x82, 0x00};
	unsigned char acctused[2]={0x82, 0x01};
	unsigned char acctblock[2]={0x82, 0x02};
	unsigned char nopass[2]={0x82, 0x03};

	SERIAL chrSerial;

	acctno[s]=INVALID;
#ifdef ENCRYPTION
	NXWCLIENT ps = getClientFromSocket(s);
	int length=0x3e;
	int j;
	unsigned char decryptPacket[MAXBUFFER+1];
	unsigned char cryptPacket[MAXBUFFER+1];
	ClientCrypt * crypter;
	memcpy(&cryptPacket[0], &buffer[s][0],length);
	if ( clientCrypter[s] != NULL )
	{
		crypter = clientCrypter[s] ;
		for ( j = 0 ; j <= CRYPT_3_0_6j;++j)
		{
			memcpy(&cryptPacket[0], &buffer[s][0],length);
			crypter->setLoginCryptKeys(loginKeys [j] [0], loginKeys [j] [1]);
			crypter->setCryptMode(CRYPT_LOGIN);
			crypter->setGameEncryption(j+1);
			crypter->init(&clientSeed[s][0]);
			crypter->decrypt(&cryptPacket[0], decryptPacket, length);
			if ( decryptPacket[0x3e-1] == 0xFF )
			{
				// crypt key is correct only if Authentication is ok
				memcpy (&cryptPacket[0], decryptPacket, length);
				pSplit((char*)&cryptPacket[31]);
				i = Accounts->verifyPassword((char*)&cryptPacket[1], (char*)pass1);
				if( i >= 0 )
				{
					memcpy(&buffer[s][0],&cryptPacket[0] ,length);
					// is this client logged in already ?
					if ( Accounts->IsOnline(i) )
					{
						// Look for accountno
						for ( int j = 0;j < s;++j)
						{
							if ( acctno[j] == i )
							{
								// Found logged in account
								Disconnect(acctno[j]);
								break;
							}
						}
					}
					break;
				}
			}
		}
	}
#endif
	pSplit((char*)&buffer[s][31]);
	i = Accounts->Authenticate((char*)&buffer[s][1], (char*)pass1);

	if( i >= 0 )
		acctno[s] = i;
	else
	{
		switch(i)  //Let's check for the error message
		{
		case BAD_PASSWORD:
			currchar[s] = INVALID;
			Xsend(s, nopass, 2);
			return;
		case ACCOUNT_BANNED:
			currchar[s] = INVALID;
			Xsend(s, acctblock, 2);
			return;
		case LOGIN_NOT_FOUND:
			if( !SrvParms->auto_a_create )
			{
				currchar[s] = INVALID;
				Xsend(s, noaccount, 2);
				return;
			} else {
				// Auto create is enable, let's create the new account.
				std::string dummylogin = (char*)&buffer[s][1], dummypass = (char*)&buffer[s][31];

				// Let's check if password isn't blank:
				if (dummypass.empty())
				{
					// User forgot password, let's send a message and return
					Xsend(s, nopass, 2);
					return;
				}
				acctno[s] = Accounts->CreateAccount(dummylogin, dummypass);
			}
		}
	}

	if (Accounts->IsOnline(acctno[s]) )
	{
		Xsend(s, acctused, 2);
		//<Luxor>: Let's kick the current player
		chrSerial = Accounts->GetInWorld(acctno[s]);
		if (chrSerial == INVALID)
			return;
		P_CHAR pc = pointers::findCharBySerial(chrSerial);
		VALIDATEPC(pc);
		pc->kick();
		currchar[s] = INVALID;
		return;
		//</Luxor>
	}

	//ndEndy now better
	if (acctno[s]!=INVALID)
	{
		Accounts->SetEntering(acctno[s]);
		Login2(s);
	}
}


void cNetwork::Login2(int s)
{
	const char msgLogin[] = "Client [%s] connected [first] using Account '%s'.\n";

	UI16 i, tlen;
	unsigned long int ip;
	UI08 newlist1[6]={ 0xA8, 0x00, };
	UI08 newlist2[40]={ 0x00, };

	InfoOut( (char*)msgLogin, inet_ntoa(client_addr.sin_addr), &buffer[s][1] );
	if (SrvParms->server_log)
		ServerLog.Write( (char*)msgLogin, inet_ntoa(client_addr.sin_addr), &buffer[s][1] );

	tlen = 6 + (servcount*40);
	ShortToCharPtr(tlen, newlist1 +1);
	newlist1[3]=0xFF;			// System Info flag
	ShortToCharPtr(servcount, newlist1 +4);
	Xsend(s, newlist1, 6);

	for( i = 0; i < servcount; ++i )
	{
		ShortToCharPtr(i+1, newlist2);

		strcpy((char*)&newlist2[2], serv[i][0]);
		newlist2[34]=0x12;		// %Full
		newlist2[35]=0x01;		// Timezone
		ip=inet_addr(serv[i][1]); 	  // Host-Order
		ip = htonl(ip);			  // swap if needs
		LongToCharPtr(ip, newlist2 +36);  //Network order ...
		Xsend(s, newlist2, 40);
	}
//AoS/	Network->FlushBuffer(s);
}


void cNetwork::Relay(int s) // Relay player to a certain IP
{
	unsigned long int ip;
	int port;

	ip=inet_addr(serv[buffer[s][2]-1][1]);
	port=str2num(serv[buffer[s][2]-1][2]);

	// Enable autodetect unless you bind to a specific ip address
        // otherwise you should lead in some security issue
        if(ServerScp::g_nAutoDetectIP || ServerScp::g_nBehindNat) { //Luxor
		//Xan : plug'n'play mode under windows :)
                //Rik : and for linux too ;) (should run on other bsd compatible systems too)
		socklen_t n = sizeof(sockaddr_in);
                sockaddr_in sa;
		getsockname (client[s], (sockaddr*)(&sa), (socklen_t*)(&n));
		unsigned long oldip = ip;
		//Luxor: REALLY tricky... i should change this soon, but no time to make it better by now :P
		unsigned long int serverip = sa.sin_addr.s_addr;
		unsigned long int clientip = client_addr.sin_addr.s_addr;
//		printf("IP: %d.%d.%d.%d\n", IPPRINTF(ip));
//		printf("SERVER_IP: %d.%d.%d.%d\n", IPPRINTF(serverip));
//		printf("CLIENT_IP: %d.%d.%d.%d\n", IPPRINTF(clientip));
		if (ServerScp::g_nBehindNat) { //if the server is behind a NAT, use the autodetection only if the client is in the same LAN
			if (((serverip&0xFF) == (clientip&0xFF)) && (((serverip>>8)&0xFF) == ((clientip>>8)&0xFF)))
				ip = serverip;
		} else
			ip = serverip;
                if (ip != oldip)
			InfoOut("client %d relayed to IP %d.%d.%d.%d instead of %d.%d.%d.%d\n", s, IPPRINTF(ip), IPPRINTF(oldip));
        }


#ifdef RELAY_TO_NOXSNIFFER
	port = 2595;
	WarnOut("relaying client to NoX-Sniffer!!!\n");
#endif

	UI08 login03[11]={ 0x8C, 0x00, };
	ip = htonl(ip);			// host order -> network order !!!!
	LongToCharPtr(ip, login03 +1);
	ShortToCharPtr(port, login03 +5);
	srand(ip+acctno[s]+now+uiCurrentTime); // Perform randomize
#ifdef ENCRYPTION
	if ( clientCrypter[s] != NULL )
	{
		ClientCrypt *crypter = clientCrypter[s];
		// unsigned int loginseed = clientip[s][3] + ( clientip[s][2] << 8) + (clientip[s][1] << 16) + ( clientip[s][0] << 24);
		unsigned long loginseed = 1 + ( 127 << 24 )  ;
		crypter->setCryptSeed(loginseed);
		crypter->setCryptMode(CRYPT_GAME);
		crypter->setEntering(true);
		// memcpy(login03 +7, &clientip[s][0],4);
		LongToCharPtr (loginseed, login03 +7); // set crypt key
		// if ( clientCrypter[s]->getCryptVersion() < CRYPT_3_0_0c )
		LongToCharPtr (ip, login03 +1); // set game server ip


	}
	else
#endif
		LongToCharPtr( calcserial('a', 'K', 'E', 'Y'), login03 +7);	// New Server Key!
	Xsend(s, login03, 11);
	Network->FlushBuffer(s);
}

void cNetwork::ActivateFeatures(NXWSOCKET s)
{
	UI08 feat[3] = {0xB9, 0x00, 0x00};
	UI16 features = 0;  //<-- BitMask ?
	// 0x0001 => Button Chat ( T2A ??? )
	// 0x0003 => LBR (+ T2A)
	// 0x801F => AoS and previous .... (AoS + LBR + T2A)
	// 0xFFFF => ... (*ALL* <grin>)

	switch(server_data.feature)
	{
		case 1:	// T2A Features, button chat, popup help ..
			features |= T2A;
			break;
		case 2: // LBR plus previous vers.
			features |= (LBR | T2A);
			break;
		default:  // I don't know, what you want :P
			return;
			break;
	}

	ShortToCharPtr(features, feat+1);
	Xsend(s, feat, 3);
//AoS/	Network->FlushBuffer(s);
}

void cNetwork::GoodAuth(int s)
{
	UI32 j;
	UI16 tlen;
	UI08 login04a[4]={ 0xA9, 0x09, 0x24, 0x02 }, n = startcount;

	tlen=4+(5*60)+1+(startcount*63) +4;

	ShortToCharPtr(tlen, login04a +1);

	Accounts->OnLogin(acctno[s],s);

	//Endy now much fast
	NxwCharWrapper sc;
	Accounts->GetAllChars( acctno[s], sc );

	ActivateFeatures(s);

	login04a[3] = sc.size(); //Number of characters found
	Xsend(s, login04a, 4);

	j=0;

	UI08 login04b[60]={ 0, };

	for ( sc.rewind(); !sc.isEmpty(); sc++ )
	{
		P_CHAR pc_a=sc.getChar();
		if(!ISVALIDPC(pc_a) )
			continue;

		strcpy((char*)login04b, pc_a->getCurrentNameC());
		Xsend(s, login04b, 60);
		j++;
	}

	UI32 i=0;
	memset(login04b, 0, 60);
	for ( i=j;i<5;i++)
	{
		Xsend(s, login04b, 60);
	}

	Xsend(s, &n, 1);  // startcount

	UI08 login04d[63]= { 0, };

	for (i=0;i<startcount;i++)
	{
		memset(login04d, 0, 63);
		login04d[0]=i;
		for (j=0;j<=strlen(start[i][0]);j++) login04d[j+1]=start[i][0][j];
		for (j=0;j<=strlen(start[i][1]);j++) login04d[j+32]=start[i][1][j];
		Xsend(s, login04d, 63);
	}

	UI08 tail[4]={0, }; //Fix for new clients, else it stuck on "Connecting ..."

	if(server_data.feature == 2) 	// LBR: NPC Popup Menu   (not currently impl.)
		tail[3] = 0x08;
	Xsend(s, tail, 4);
//AoS/	Network->FlushBuffer(s);
}

void cNetwork::CharList(int s) // Gameserver login and character listing
{

	SI32 i;
	UI08 noaccount[2]={0x82, 0x00};
	UI08 nopass[2]={0x82, 0x03};
	UI08 acctblock[2]={0x82, 0x02};

	acctno[s]=-1;

	pSplit((char*)&buffer[s][35]);
	i = Accounts->Authenticate((char*)&buffer[s][5], (char*)pass1);

	if( i >= 0 )
		acctno[s] = i;
	else
	{
		switch(i)  //Let's check for the error message
		{
		case BAD_PASSWORD:
			Xsend(s, nopass, 2);
			currchar[s] = INVALID;
//AoS/			Network->FlushBuffer(s);
			return;
		case ACCOUNT_BANNED:
			Xsend(s, acctblock, 2);
			currchar[s] = INVALID;
//AoS/			Network->FlushBuffer(s);
			return;
		case LOGIN_NOT_FOUND:
			Xsend(s, noaccount, 2);
			currchar[s] = INVALID;
//AoS/			Network->FlushBuffer(s);
			return;
		}
	}

	if (acctno[s] >= 0)
		GoodAuth(s);
}

void cNetwork::pSplit (char *pass0) // Split login password into NoX-Wizard password and UO password
{

	int i,loopexit=0;
	i=0;
	pass1[0]=0;
	while ( (pass0[i]!='/') && (pass0[i]!=0) && (++loopexit < MAXLOOPS) ) i++;
	strncpy(pass1,pass0,i);
	pass1[i]=0;
	if (pass0[i]!=0) strcpy(pass2, pass0+i+1);

}

void cNetwork::charplay (int s) // After hitting "Play Character" button //Instalog
{
	if ( s < 0 || s >= now )
		return;

	currchar[s] = INVALID;

	P_CHAR pc_k=NULL;

	if (acctno[s]>INVALID)
	{
		int j=0;
		Accounts->SetOffline(acctno[s]);
		NxwCharWrapper sc;
		Accounts->GetAllChars( acctno[s], sc );
		for( sc.rewind(); !sc.isEmpty(); sc++ ) {
			P_CHAR pc_i=sc.getChar();
			if(!ISVALIDPC(pc_i))
				continue;
			if (j==buffer[s][0x44]) {
				pc_k=pc_i;
				break;
			}
			j++;
		}

		if (ISVALIDPC(pc_k))
		{
			pc_k->setClient(NULL);
			SI32 nSer = pc_k->getSerial32();
			for ( SI32 idx = 0; idx < now; idx++ ) {
				if ( nSer == currchar[idx] ) {
					UI08 msg2[2]={ 0x53, 0x05 };
					Xsend(s, msg2, 2);
//AoS/					Network->FlushBuffer(s);
					Disconnect(s);
					Disconnect(idx);
					return;
				}
			}

			Accounts->SetOnline(acctno[s], pc_k);
			pc_k->logout=INVALID;

			currchar[s] = pc_k->getSerial32();

			pc_k->setClient(new cNxwClientObj(s));
			startchar(s);
		}
		else
		{
			UI08 msg[2]={ 0x53, 0x05 };
			Xsend(s, msg, 2);
//AoS/			Network->FlushBuffer(s);
			Disconnect(s);
		}
	}
}

void cNetwork::enterchar(int s)
{
	if (s < 0 || s >= now) return; //Luxor
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	UI08 startup[38]="\x1B\x00\x05\xA8\x90\x00\x00\x00\x00\x01\x90\x06\x08\x06\x49\x00\x0A\x04\x00\x00\x00\x7F\x00\x00\x00\x00\x00\x07\x80\x09\x60\x00\x00\x00\x00\x00\x00";
	UI08 techstuff[5]={ 0x69, 0x00, 0x05, 0x01, 0x00 };
	UI08 world[6]={0xBF, 0x00, 0x06, 0x00, 0x08, 0x00};
	UI08 modeset[5]={0x72, 0x00, 0x00, 0x32, 0x00};
	UI08 LoginOK[1] = {0x55}, TimeZ[4]={0x5B, 0x0C, 0x13, 0x03};

	if (map_height<300) world[5]=0x02;

	cPacketFeatures features;
	features.feature= FEATURE_T2A | FEATURE_LBR;
	features.send( pc->getClient() );

	clientInfo[s]->ingame=true;

	Location charpos= pc->getPosition();

	LongToCharPtr(pc->getSerial32(), startup +1);
	ShortToCharPtr(pc->getId(), startup +9);
	ShortToCharPtr(charpos.x, startup +11);
	ShortToCharPtr(charpos.y, startup +13);
	startup[16]= charpos.z;
	startup[17]= pc->dir;
	startup[28]= 0;

	if(pc->poisoned) startup[28]=0x04; else startup[28]=0x00; //AntiChrist -- thnx to SpaceDog

	Xsend(s, world, 6);
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	Xsend(s, startup, 37);
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	pc->war=0;
	Xsend(s, modeset, 5);
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	techstuff[3]=0x01;
	Xsend(s, techstuff, 5);
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	techstuff[3]=0x02;
	Xsend(s, techstuff, 5);
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	techstuff[3]=0x03;
	Xsend(s, techstuff, 5);
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	Xsend(s, LoginOK, 1);
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	Xsend(s, TimeZ, 4);
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	pc->spiritspeaktimer=uiCurrentTime;
	pc->begging_timer=uiCurrentTime;

	pc->stealth=INVALID;
	if (!(pc->IsGMorCounselor()))
		pc->hidden=UNHIDDEN;

	pc->wresmove=0;	//Luxor
	pc->teleport();

	Calendar::commitSeason(pc);

}


void cNetwork::startchar(int s) // Send character startup stuff to player
{

	if ( s < 0 || s >= now ) //Luxor
		return;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
#ifdef ENCRYPTION
	pc->setCrypter(clientCrypter[s]);
#endif
	//<Luxor>: possess stuff
	if (pc->possessedSerial != INVALID) {
		P_CHAR pcPos = pointers::findCharBySerial(pc->possessedSerial);
		if (ISVALIDPC(pcPos)) {
			currchar[s] = pcPos->getSerial32();
			pcPos->setClient(new cNxwClientObj(s));
#ifdef ENCRYPTION
			pcPos->setCrypter(clientCrypter[s]);
			pc->setCrypter(NULL);
#endif
			pc->setClient(NULL);
			Accounts->SetOffline(pc->account);
			Accounts->SetOnline(pc->account, pcPos);
		} else pc->possessedSerial = INVALID;
	}
	//</Luxor>
#ifdef ENCRYPTION
	clientCrypter[s]=NULL;
#endif
	char zbuf[255];
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	AMXEXECSV( pc->getSerial32(),AMXT_SPECIALS, 4, AMX_BEFORE);

	enterchar( s );
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	sysmessage(s,0x058, TRANSLATE("%s %s %s [%s] Compiled by %s"), PRODUCT, VER, VERNUMB, OS , NAME);
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	sysmessage(s,0x038, TRANSLATE("Programmed by: %s"),PROGRAMMERS);
#ifdef ENCRYPTION
	Network->FlushBuffer(s);
#endif
	// log last time signed on
	time_t ltime;
	time( &ltime );

	if (SrvParms->joinmsg)
	{
		if (!strcmp(pc->getCurrentNameC(), "pty Slot --")) pc->setCurrentName("A new Character");//AntiChrist
		sprintf((char*)temp,TRANSLATE("%s entered the realm"),pc->getCurrentNameC());//message upon entering a server
		sysbroadcast((char*)temp);//message upon entering a server
	}

	sprintf(zbuf,"%s Logged in the game",pc->getCurrentNameC()); //for logging to UOXmon

	// very stupid stuff
	//pc->murderrate=uiCurrentTime+repsys.murderdecay*MY_CLOCKS_PER_SEC; // LB, bugfix for murder-count getting --'ed each start
	pc->murderrate=uiCurrentTime+pc->murdersave*MY_CLOCKS_PER_SEC;

	updates(s);

	const char * t;
	std::vector<std::string>::const_iterator vis( clientsAllowed.begin() ), vis_end( clientsAllowed.end() );

	t = (*vis).c_str();
	strcpy(temp,t);

	AMXEXECSV(pc->getSerial32(),AMXT_SPECIALS, 4, AMX_AFTER);

	//
	// Sparhawk	Race system support
	//		When the race system is active and users doesn't belong to a valid race start enlistment procedure
	//
	if ( Race::isRaceSystemActive() )
		if ( pc->race <= 0 )
		{
			pc->race = 0;

			AmxFunction* race_enlist = NULL;
			if( race_enlist==NULL )
				race_enlist = new AmxFunction( "__race_enlist" );

			race_enlist->Call( pc->getSerial32() );
		}
		//else
		//	validate pc race and decide what to do if race is invalid or has been deactivated

	if ( !(strcmp(temp, "ALL") ) )
	{
  	  pc->sysmsg(TRANSLATE("There is NO client version checking active on this shard. The recommanded-dev-team-supported client version for this server version is client version %s though"), SUPPORTED_CLIENT);
#ifdef ENCRYPTION
	  Network->FlushBuffer(s);
#endif
	  return;

	} else if ( !(strcmp(temp, "SERVER_DEFAULT") ) )
	{
	  pc->sysmsg(TRANSLATE("This shard requires the recommanded-dev-team-supported client version for this server version client version %s"), SUPPORTED_CLIENT);
#ifdef ENCRYPTION
	  Network->FlushBuffer(s);
#endif
	  return;
	}
	else
	{
	   sprintf(idname, TRANSLATE("This shard requires client version[s] %s"),temp);
	}

	// remark: although it doesn't look good [without], don't add /n's
	// the (2-d) client doesn't like them

	temp2[0]=0;
	++vis;
	for ( ; vis != vis_end; ++vis)
	{
		t = (*vis).c_str();
		strcpy(temp,t);
		strcat(temp2,TRANSLATE(" or "));
		strcat(temp2,temp);
	}

	strcat(idname, temp2);
	strcat(idname,TRANSLATE(" The NoX-Wizard team recommanded client is "));
	strcat(idname, SUPPORTED_CLIENT);

	pc->sysmsg(idname);
	Network->FlushBuffer(s);
}

char cNetwork::LogOut(NXWSOCKET s)//Instalog
{
	if (s < 0 || s >= now) return 0; //Luxor

	P_CHAR pc = pointers::findCharBySerial(currchar[s]);
	VALIDATEPCR(pc, 0);

	UI32 a, valid=0;
	Location charpos= pc->getPosition();
	UI32 x= charpos.x, y= charpos.y;


	AMXEXECSVNR(pc->getSerial32(),AMXT_SPECIALS, 8, AMX_BEFORE);

	P_ITEM pack;
	for(a=0;a<logoutcount;a++)
	{
		if (logout[a].x1<=x && logout[a].y1<=y && logout[a].x2>=x && logout[a].y2>=y)
		{
			valid=1;
			break;
		}
	}

	if(pc->IsGMorCounselor() || pc->account==0) valid=1;

	P_ITEM p_multi=NULL;
	if (pc->getMultiSerial32() == INVALID )
		p_multi=findmulti( pc->getPosition() );
	else
		p_multi = pointers::findItemBySerial( pc->getMultiSerial32() );

	if (ISVALIDPI(p_multi) && !valid)//It they are in a multi... and it's not already valid (if it is why bother checking?)
	{
		pack= pc->getBackpack();
		if( ISVALIDPI(pack))
		{
			NxwItemWrapper si;
			si.fillItemsInContainer( pack, false );
			for( si.rewind(); !si.isEmpty(); si++ ) {

				P_ITEM p_ci=si.getItem();
				if (!ISVALIDPI(p_ci))
					if (p_ci->type==ITYPE_KEY &&
						(p_multi->getSerial32() == calcserial(p_ci->more1, p_ci->more2, p_ci->more3, p_ci->more4)) )
					{//a key to this multi
						valid=1;//Log 'em out now!
						break;
					}
			}
		}
	}

	Accounts->SetOffline(pc->account);
	if (valid)//||region[chars[p].region].priv&0x17)
	{
		pc->logout=INVALID; // LB bugfix, was timeout
	} else {
		if ( clientInfo[s]->ingame )
		{
		    pc->logout=uiCurrentTime+SrvParms->quittime*MY_CLOCKS_PER_SEC;
		}
	}

	NxwSocketWrapper sw;
	sw.fillOnline( pc, true );
	for( sw.rewind(); !sw.isEmpty(); sw++ )
	{
		NXWSOCKET s=sw.getSocket();
		if( s!=INVALID )
			impowncreate(s,pc,0);
	}

	return valid;

}

/*!
\brief Receive data from a socket
\param s the socket to receive from
\param x the numero of bytes to receive
\param a the buffer offset
\return the number of actually read bytes
*/
int cNetwork::Receive(int s, int x, int a )
{
	if ( (x+a) >= MAXBUFFER) return 0;

	int count,loopexit=0;
	do
	{
		if((count = recv(client[s], (char*)&buffer[s][a], x, MSG_NOSIGNAL))==SOCKET_ERROR)
			LogSocketError("Socket Recv Error %s\n", errno) ;

	}
	while ( (count!=x) && (count>0) && (++loopexit < MAXBUFFER ));

	return count;
}

void cNetwork::sockInit()
{
	int bcode;

	initClients();


	kr=1;
	faul=0;

#ifndef __unix__
	wVersionRequested=0x0002;
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err!=0)
	{
		if (ServerScp::g_nDeamonMode==0) MessageBox(NULL, "Winsock 2.0 not found. This program requires Winsock 2.0 or later. ", "NoX-Wizard Network initialization", MB_ICONSTOP);
		ErrOut("ERROR: Winsock 2.0 not found...\n");
		keeprun=false;
		error=1;
		kr=0;
		faul=1;
		return;
	}
	static char temp[600];
#endif

	a_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (a_socket < 0 )
	{
		ErrOut("ERROR: Unable to create socket\n");
		keeprun=false;
		error=1;
		kr=0;
		faul=1;
		return;
	}

	g_nMainTCPPort = str2num(serv[0][2]);

	len_connection_addr=sizeof (struct sockaddr_in);
	connection.sin_family=AF_INET;
	connection.sin_addr.s_addr=INADDR_ANY;		// All interfaces
	connection.sin_port=htons(g_nMainTCPPort);	// Port

	bcode = bind(a_socket, (struct sockaddr *)&connection, len_connection_addr);

	if (bcode!=0)
	{
#ifdef WIN32
		bcode = WSAGetLastError ();
		switch(bcode) {
			case WSANOTINITIALISED :
				sprintf(temp, "Winsock2 initialization problems (%d)", bcode);
				break;
			case WSAENETDOWN:
				sprintf(temp, "Network subsystem failure (%d)", bcode);
				break;
			case WSAEADDRINUSE:
				sprintf(temp, "Address+port already in use (%d)", bcode);
				break;
			case WSAEADDRNOTAVAIL:
				sprintf(temp, "Address use not valid for this machine (%d)", bcode);
				break;
			case WSAEFAULT:
				sprintf(temp, "Access violation during binding (%d)", bcode);
				break;
			case WSAEINPROGRESS:
				sprintf(temp, "Service provider busy (%d)", bcode);
				break;
			case WSAEINVAL:
				sprintf(temp, "Socket already bound (%d)", bcode);
				break;
			case WSAENOBUFS:
				sprintf(temp, "Not enough buffers available (%d)", bcode);
				break;
			case WSAENOTSOCK:
				sprintf(temp, "Invalid socket (%d)", bcode);
				break;
			default:
				sprintf(temp, "Unknown error (%d)", bcode);
				break;
		}
		ErrOut("ERROR: Unable to bind socket\n    Error code: %s\n",temp);

	if (ServerScp::g_nDeamonMode==0) {
		MessageBox(NULL, temp, "NoX-Wizard network error [bind]", MB_ICONSTOP);
	}
#endif //win32
#ifndef WIN32
		ErrOut("ERROR: Unable to bind socket - Error code: %i\n",bcode);
#endif
		keeprun=false;
		error=1;
		kr=0;
		faul=1;
		return;
	}

	bcode = listen(a_socket, MAXCLIENT);

	if (bcode!=0)
	{
		ErrOut("ERROR: Unable to set socket in listen mode  - Error code: %i\n",bcode);
		keeprun=false;
		error=1;
		kr=0;
		faul=1;
		return;
	}
	// Ok, we need to set this socket (or listening one as non blocking).  The reason is we d a
	// select, and then do an accept.  However, if the client has terminated the connection between the small
	// time from the select and accept, we would block (accept is blocking).  So, set it non blocking
	unsigned long nonzero = 1;
#if defined(__unix__)
	ioctl(a_socket,FIONBIO,&nonzero) ;
#endif
#ifdef _CONSOLE
	ioctlsocket(a_socket,FIONBIO,&nonzero) ;
#endif
#ifdef _WINDOWS
	bcode = WSAAsyncSelect (a_socket, g_HWnd, 0, 0);
#endif

}

void cNetwork::SockClose () // Close all sockets for shutdown
{

	int i;
	closesocket(a_socket);
	for ( i = 0; i < MAXCLIENT; ++i )
		closesocket(client[i]);

}

void cNetwork::CheckConn() // Check for connection requests
{

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	char temp2[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	int s;
	socklen_t len;

	if (now<MAXIMUM)

	{
		FD_ZERO(&conn);
		FD_SET(a_socket, &conn);
		nfds=a_socket+1;

		s=select(nfds, &conn, NULL, NULL, &nettimeout);

		if (s>0)
		{
			len=sizeof (struct sockaddr_in);
			client[now] = accept(a_socket, (struct sockaddr *)&client_addr, &len);
			if ((client[now]<0))
			{
				ErrOut("Unknown error at client connection!\n");
				error=1;
				keeprun=true;
				return;
			}
			if ( CheckForBlockedIP( client_addr ) )
			{
				InfoOut("IPBlocking: Blocking IP address [%s] listed in hosts_deny\n", inet_ntoa( client_addr.sin_addr ));

				closesocket(client[now]);
			}
			else
			{
				unsigned long nonzero = 1;
			#if defined(__unix__)
				ioctl(client[now],FIONBIO,&nonzero) ;
			#else
				ioctlsocket(client[now],FIONBIO,&nonzero) ;
			#endif

				currchar[now] = INVALID;
				cryptedClient[now]=true;
				acctno[now]=-1;
				binlength[now]=0;
				boutlength[now]=0;
				clientInfo[now]= new cClient();
				walksequence[now]=-1;

				clientDimension[now]=2;

				++global_lis; // not 100% correct, but only cosmetical stuff, hence ok not to be 100% correct :>
                          			// doesnt get correct status if kicked out due to worng pw etc.

            			if (global_lis % 2 == 0) sprintf((char*)temp2, "connecting"); else sprintf((char*)temp2, "connected");

				sprintf((char*)temp,"client %i [%s] %s [Total:%i].\n",now,inet_ntoa(client_addr.sin_addr), temp2, now+1);
				InfoOut(temp);

				if (SrvParms->server_log)
					ServerLog.Write("%s", temp);

#ifdef USE_MTHREAD_SEND
                		if (g_NT[now]==NULL)
					g_NT[now] = new NetThread(client[now]);
                		else
					g_NT[now]->set(client[now]);
                		g_NT[now]->mtxrun.leave();
#endif
				++now;
			}
			return;

		}
		else if (s<0)
		{
			ErrOut("select (Conn) failed!\n");
			keeprun=true;
			error=1;
			return;
		}
	}

}

void cNetwork::CheckMessage() // Check for messages from the clients
{

	int s, i, oldnow;
	int lp, loops;     //Xan : rewritten to support more than 64 concurrent clients

	oldnow = now;
	loops = now / 64; //xan : we should do loops of 64 players

	for (lp = 0; lp <= loops; lp++) {


		FD_ZERO(&all);
		FD_ZERO(&errsock);
		nfds=0;

		for (i=0+(64*lp);i<((lp<loops) ? 64 : oldnow);i++)
		{
			FD_SET(client[i],&all);
			FD_SET(client[i],&errsock);
			if (client[i]+1>nfds) nfds=client[i]+1;

		}

		s=select(nfds, &all, NULL, &errsock, &nettimeout);

		if (s>0)
		{
			for (i=0+(64*lp);i<((lp<loops) ? 64 : oldnow);i++)
			{
				if (FD_ISSET(client[i],&errsock))
				{
					Network->Disconnect(i);
				}


				if ((FD_ISSET(client[i],&all))&&(oldnow==now))
				{
					Network->GetMsg(i);
					if (executebatch) batchcheck(i);
				}
			}
		}
	}
}


cNetwork::cNetwork() // Initialize sockets
{
    sockInit();
#ifdef ENCRYPTION
	for (int i = 0;i< MAXCLIENT+1;++i)
	{
		clientCrypter[i]=0;
	}
#endif
}

int cNetwork::Pack(void *pvIn, void *pvOut, int len)
{

	unsigned char *pIn = (unsigned char *)pvIn;
	unsigned char *pOut = (unsigned char *)pvOut;

	int actByte = 0;
	int bitByte = 0;
	int nrBits;
	unsigned int value;

	if (len<=0) return 0; // ensure termination, got a bit paranoid bout server freezes'

	while(len--)
	{
		nrBits = bit_table[*pIn][0];
		value = bit_table[*pIn++][1];

		while(nrBits--)
		{
			pOut[actByte] = (pOut[actByte] << 1) | (unsigned char)((value >> nrBits) & 0x1);

			bitByte = (bitByte + 1) & 0x07;
			if(!bitByte) actByte++;
		}
	}

	nrBits = bit_table[256][0];
	value = bit_table[256][1];

	if ( nrBits <= 0 ) return 0;
	while(nrBits--)
	{
		pOut[actByte] = (pOut[actByte] << 1) | (unsigned char)((value >> nrBits) & 0x1);

		bitByte = (bitByte + 1) & 0x07;
		if(!bitByte) actByte++;
	}

	if(bitByte)
	{
		while(bitByte < 8)
		{
			pOut[actByte] <<= 1;
			bitByte++;
		}

		actByte++;
	}

	return actByte;

}

#ifdef ENCRYPTION
unsigned char cNetwork::calculateLoginKey(unsigned char loginKey[4], unsigned char packetId)
{
	unsigned int seed;
	int  m_key [2], index = -1;
	unsigned char out;
	bool found;
	out=packetId;
	seed = (loginKey[0] << 24 ) + (loginKey[1] << 16 ) + (loginKey[2] << 8 ) + (loginKey[3] );
	found = true;
	for ( int i = 0; i < 6; ++i)
	{
		index=i;
		m_key[0] =
				(((~seed) ^ 0x00001357) << 16)
			|   ((seed ^ 0xffffaaaa) & 0x0000ffff);
		m_key[1] =
				((seed ^ 0x43210000) >> 16)
			|   (((~seed) ^ 0xabcdffff) & 0xffff0000);
		unsigned char med = static_cast<unsigned char>(m_key[0]);
	    out = packetId ^ med;
		if ( (out != 0x80) && (out != 0x91) )
		{
			found = false;
			continue;
		}
		else
			break;
	}
	return out;
}
#endif

void cNetwork::GetMsg(int s) // Receive message from client
{
#ifdef ENCRYPTION
	ClientCrypt *crypter=NULL;
#endif
	NXWCLIENT ps = getClientFromSocket(s);
	P_CHAR pc_currchar= (ps!=NULL)? ps->currChar() : NULL;
#ifdef ENCRYPTION
	if ( pc_currchar != NULL )
	{
		crypter=pc_currchar->getCrypter();
	}
	else if ( clientCrypter[s] != NULL )
	{
		crypter=clientCrypter[s];
	}
#endif

	int count, i, book,length, dyn_length,loopexit=0, fb;
	unsigned char nonuni[512];
	unsigned char packet;
	int  myoffset,  myj, mysize, subcommand;//, subsubcommand ;
	unsigned char mytempbuf[512] ;
	char client_lang[4];

	std::string cpps;
	std::vector<std::string>::const_iterator viter;

	if ( clientInfo[s]->newclient )
	{

		if((count=recv(client[s], (char*)buffer[s], 4, MSG_NOSIGNAL))==SOCKET_ERROR)
		{
			LogSocketError("Socket Recv Error %s\n", errno) ;
		}
		// an encrypted client sends its ip as a seed for the encryption

#ifdef ENCRYPTION
		memcpy(&clientSeed[s][0], &buffer[s][0], 4);
#endif
		struct sockaddr_in tmp_addr;
		socklen_t tmp_addr_len=sizeof(struct sockaddr_in);

		if(getpeername(client[s],(struct sockaddr*)&tmp_addr,&tmp_addr_len)!=SOCKET_ERROR)
			*(unsigned long*)&clientip[s]=tmp_addr.sin_addr.s_addr;
		else
		{
			WarnOut("Unable to determine client's IP [s:%i]\n",s);
			*(unsigned long*)&clientip[s]=0;
		}

		clientInfo[s]->newclient=false;
		clientInfo[s]->firstpacket=true;
	}
	else
	{
#ifdef ENCRYPTION
		int cryptBufferOffset=0;
#endif
		fb = Receive(s, 1, 0);
		if (fb >0)
		{
#ifdef ENCRYPTION
			if ( crypter != NULL )
			{
				crypter->decrypt( &buffer[s][0], &buffer[s][0], 1);
				// crypter->preview( &buffer[s][0], &buffer[s][0], 1);
				cryptBufferOffset++;
			}
#endif
			packet = buffer[s][0];

			length = m_packetLen[packet];

			// Lets assure the data is valid, this stops server freezes caused by receiving nonsense data
			// (remark: useres that dont use ignition do that)
			if ( clientInfo[s]->firstpacket && packet != 0x80 && packet !=0x91 )
			{
#ifdef ENCRYPTION
				// Let's see if the paket is encrypted
				packet = calculateLoginKey(clientSeed[s], packet);
#endif
				if (packet != 0x80 && packet !=0x91 )
				{
#ifdef ENCRYPTION
					// reset crypt memories
					if ( clientCrypter[s] != NULL )
					{
						delete (ClientCrypt * ) crypter ;
						clientCrypter[s]=NULL;
					}
#endif
					Disconnect(s);
					InfoOut("received garbage from a client, disconnected it to prevent bad things.\n User probably didnt use ignition or UO-RICE\n");
					return;
				}
#ifdef ENCRYPTION
				else
				{
					ClientCrypt * crypter = new ClientCrypt();
					crypter->setCryptMode(CRYPT_NONE);
					clientCrypter[s] = crypter;
					length = m_packetLen[packet];
				}
#endif
			}


			if (length==PACKET_LEN_NONE)
			{
				FD_ZERO(&all); FD_SET(client[s],&all);nfds=client[s]+1;
				if (select(nfds, &all, NULL, NULL, &nettimeout)>0) Receive(s, MAXBUFFER-2, 0);
				// I think thats supposed to be a re-synch attempt for next msg-in case of getting garbage (LB)
				return;

			}
			int readstat ;

			if (length==PACKET_LEN_DYNAMIC)
			{
				if ((readstat = Receive(s, 2, 1)) > 0)
				{
#ifdef ENCRYPTION
					if ( crypter != NULL )
					{
						unsigned char decryptPacket[3];
						crypter->decrypt(&buffer[s][0]+cryptBufferOffset, decryptPacket, 2);
						cryptBufferOffset+=2;
						buffer[s][1]=decryptPacket[0];
						buffer[s][2]=decryptPacket[1];
					}
#endif
					dyn_length = (int) (  ( (int) buffer[s][1]<<8) + (int) buffer[s][2] );
					length=dyn_length;
					readstat = Receive(s, dyn_length-3, 3) ;
				//ConOut("dyn-length: %i\n",dyn_length);
				} else return;


			} else
			{
				readstat = Receive(s, length-1, 1);
				dyn_length = length; //useless, just to avoid boring warning
			}

			if (readstat > SOCKET_ERROR)
			{
				if (ISVALIDPC(pc_currchar) && packet !=0x73 && packet!=0x80 && packet!=0xA4 && packet!=0xA0 && packet!=0x90 && packet!=0x91 ) {
					pc_currchar->clientidletime=SrvParms->inactivitytimeout*MY_CLOCKS_PER_SEC+uiCurrentTime;
				}
        		    // LB, client activity-timestamp !!! to detect client crashes, ip changes etc and disconnect in that case
        		    // 0x73 (idle packet) also counts towards client idle time

				if (ISVALIDPC(pc_currchar))
					AMXEXECSV(pc_currchar->getSerial32(),AMXT_NETRCV, packet, AMX_BEFORE);

				//if (packet != PACKET_FIRSTLOGINREQUEST && !ISVALIDPC(pc_currchar)) return;
#ifdef ENCRYPTION
				if ( packet == PACKET_LOGINREQUEST && clientCrypter[s] != NULL )
				{
					crypter->decrypt(&buffer[s][0]+cryptBufferOffset, &buffer[s][0]+cryptBufferOffset, 4);
					unsigned int loginseed= buffer[s][4] + ( buffer[s][3] << 8) + (buffer[s][2] << 16) + ( buffer[s][1] << 24);
					// clientCrypter[s]->setCryptSeed(&buffer[s][1]);
					cryptBufferOffset+=4;
				}

				if ( crypter != NULL && crypter->getCryptMode() > 0 )
				{
					crypter->decrypt(&buffer[s][0]+cryptBufferOffset, &buffer[s][0]+cryptBufferOffset, length-cryptBufferOffset);
				}
#endif
				switch(packet)
				{

				case 0x04:
					// Expermintal for God client
					{
						UI08 packet[2] = {0x2B, 0x01};
						if (pc_currchar->IsGM())
						{
							Xsend(s, packet, 2);
//AoS/							Network->FlushBuffer(s);
							LogMessage("%s connected in with God Client!\n", pc_currchar->getCurrentNameC());
						}
						else
						{
							sysmessage(s, TRANSLATE("Access Denied!!!"));
							Disconnect(s);
							LogWarning("%s tried connecting in with God Client but has no priviledges!\n", pc_currchar->getCurrentNameC());
						}

					}
					break;

				case PACKET_DISCONNECT:// Main Menu on the character select screen
				// remark LB : no longer send :(
					Disconnect(s);
					break;

				case PACKET_FIRSTLOGINREQUEST:
#ifdef ENCRYPTION
					if ( clientCrypter[s] != NULL )
						clientCrypter[s]->setCryptMode(CRYPT_LOGIN) ;
#endif
					clientInfo[s]->firstpacket=false;
					LoginMain(s);
					break;

				case PACKET_SELECTSERVER:
					Relay(s);
					break;

				case PACKET_LOGINREQUEST:
					clientInfo[s]->firstpacket=false;
					clientInfo[s]->compressOut=true;
#ifdef ENCRYPTION
					if ( clientCrypter[s] != NULL )
						clientCrypter[s]->setEntering(false);
#endif
					CharList(s);
					break;

				case PACKET_DELETECHAR:
					chardel(s);
					break;

				case PACKET_CREATECHARACTER:
					charcreate(s);
					break;

				case PACKET_CHARACTERSELECT:
					charplay(s);
					break;

				case PACKET_MOVE_REQUEST:
					if( pc_currchar!=NULL ) {
						walking(pc_currchar, buffer[s][1], buffer[s][2]);
						pc_currchar->disturbMed();
					}
					break;

				case PACKET_PING:
					Xsend(s, buffer[s], 2);
//AoS/					Network->FlushBuffer(s);
					break;

				case PACKET_RESYNC_REQUEST:
					if( pc_currchar!=NULL ) {
						pc_currchar->teleport();
					}
					break;

				case PACKET_TALK_REQUEST:
					if( pc_currchar!=NULL ) {
						pc_currchar->unicode = false;
						strcpy((char*)nonuni, (char*)&buffer[s][8]);
						talking(s, (char*)nonuni);
					}
					break;


				// Thx a lot to Beosil for laying out the basics of the new structure of that packet since client 2.0.7
				// Thx a lot to Punt for the implementation of it (4'th Feb 2001)
				// Fixed a few erratas in that initial packet interpretation (LB 14-Feb 2001)

				case PACKET_UNICODE_TALKREQUEST:
					if( pc_currchar!=NULL ) {
						pc_currchar->unicode=true;
			    		// Check for command word versions of this packet

						if ( (buffer[s][3]) >=0xc0 )
						{

							buffer[s][3] = buffer[s][3] & 0x0F ; // set to normal (cutting off the ascii indicator since we are converting back to unicode)

							int num_words,/*idx=0,*/ num_unknown;

						// number of distict trigger words
						num_words = ( (static_cast<int>(buffer[s][12])) << 24 ) + ( (static_cast<int>(buffer[s][13])) << 16 );
						num_words = num_words & 0xfff00000;
						num_words = (num_words >> 20);

						/*************************************/
						// plz dont delete yet
						// trigger word index in/from speech.mul, not required [yet]
						/*idx = ( (static_cast<int>(buffer[s][13])) << 24 ) + ( (static_cast<int>(buffer[s][14])) << 16);
						idx = idx & 0x0fff0000;
						idx = ( (idx << 4) >> 20) ;*/
						//cout << "#keywords was " << hex << num_words << "\n" << hex << static_cast<int>(buffer[s][12]) << " " << hex << static_cast<int> (buffer[s][13]) << " " << static_cast<int> (buffer[s][14]) << " " << static_cast<int> (buffer[s][15]) << endl ;
						// cout << "idx: " << idx << endl;
						/*************************************/

						if ((num_words %2) == 1)  // odd number ?
							num_unknown = ( num_words / 2 ) * 3;
						else
							num_unknown = ((num_words / 2 ) * 3 ) - 1 ;

						myoffset = 15 + num_unknown;

						//
						//	Now adjust the buffer
						int iWord ;
						//int iTempBuf ;
						iWord = static_cast<int> ((buffer[s][1] << 8)) + static_cast<int> (buffer[s][2]) ;
						myj = 12 ;

						//cout << "Max length characters will be " << dec << (iWord - myoffset) << endl ;
						mysize = iWord - myoffset ;

						for (i=0; i < mysize ; i++)
							mytempbuf[i] = buffer[s][i+myoffset] ;

						for (i=0; i < mysize ; i++)
						{
							myj++ ;
							buffer[s][myj] = mytempbuf[i] ;
							//iTempBuf = static_cast<int> (mytempbuf[i]) ;
							//cout << "Copying value of " << hex << iTempBuf << endl ;
							myj++;
							buffer[s][myj] = 0 ;
						}

						iWord = (((iWord - myoffset ) * 2) + 12) ;
						//cout << "Setting buffer size to " << dec << iWord << endl ;
						buffer[s][1] = static_cast<unsigned char> ( ( ( iWord & 0xFF00 ) >>8 ) ) ;
						buffer[s][2] = static_cast<unsigned char> ( iWord & 0x00FF ) ;
					}

					wchar2char((char*)&buffer[s][13]);
					strncpy((char*)nonuni, Unicode::temp, ((buffer[s][1]<<8)+buffer[s][2])/2);
					talking(s, (char*)nonuni);
					}
					break;

				case PACKET_DOUBLECLICK:
					doubleclick(ps);
					break;

				case PACKET_SINGLECLICK:
					singleclick(s);
					break;

				case PACKET_TARGETING: {
					P_TARGET target = clientInfo[s]->getTarget();
					if( target==NULL )
						return;

					target->receive( ps );

					if( !target->isValid() )
						target->error( ps );
					else
						target->code_callback( ps, target );
				}
				break;

				case PACKET_WEARITEM:
					wear_item(ps);
					break;

				case PACKET_PICKUPITEM:
					get_item(ps);
					break;

				case PACKET_DROPITEM:
					drop_item(ps);
					break;

				case PACKET_WARMODE_CHANGE:
					if( pc_currchar!=NULL ) {
						pc_currchar->war=buffer[s][1];
						pc_currchar->targserial=INVALID;
						Xsend(s, buffer[s], 5);
//AoS/						Network->FlushBuffer(s);

						if (pc_currchar->dead && pc_currchar->war) // Invisible ghost, resend.
							pc_currchar->teleport( TELEFLAG_NONE );

						walking2(pc_currchar);
						dosocketmidi(s);
						pc_currchar->disturbMed();
					}
					break;

				case PACKET_REQUESTACTION:
				if( pc_currchar!=NULL ) {

					if (buffer[s][3]==0xC7) // Action
					{
						if (pc_currchar->isMounting())
							break;

						if (!(strcmp((char*)&buffer[s][4],"bow"))) pc_currchar->playAction(0x20);
						if (!(strcmp((char*)&buffer[s][4],"salute"))) pc_currchar->playAction(0x21);
						break; // Morrolan
					}
					else if (buffer[s][3]==0x24) // Skill
					{
						i=4;
						while ( (buffer[s][i]!=' ') && (++loopexit < MAXLOOPS) ) i++;
						buffer[s][i]=0;
						Skills::SkillUse(s, str2num((char*)&buffer[s][4]));
						break;
					}
					else if ((buffer[s][3]==0x27)||(buffer[s][3]==0x56))  // Spell
					{
						P_ITEM p_j = NULL;
						P_ITEM pack= pc_currchar->getBackpack();
						if(ISVALIDPI(pack)) //lb
						{
							NxwItemWrapper gri;
							gri.fillItemsInContainer( pack, false );
							gri.fillItemWeared( pc_currchar, true, true, false );
							for( gri.rewind(); !gri.isEmpty(); gri++ )
							{
								P_ITEM pj=gri.getItem();
								if (ISVALIDPI(pj))
									if (pj->type==ITYPE_SPELLBOOK)
									{
										p_j=pj;
										break;
									}
							}
						}
						if (p_j!=NULL)
						{
							book=buffer[s][4]-0x30;
							if (buffer[s][5]>0x20)
							{
								book=(book*10)+(buffer[s][5]-0x30);
							}

							if (pc_currchar->dead) {
								pc_currchar->sysmsg(TRANSLATE("Ethereal souls really can't cast spells"));
							} else {
								if (pc_currchar->isFrozen()) {
									if (pc_currchar->casting)
										pc_currchar->sysmsg(TRANSLATE("You are already casting a spell."));
									else
										pc_currchar->sysmsg(TRANSLATE("You cannot cast spells while frozen."));
								} else {

									if (!pc_currchar->knowsSpell(static_cast<magic::SpellId>(book-1))) {
										pc_currchar->sysmsg("You don't know that spell yet.");
									} else {
										magic::beginCasting(static_cast<magic::SpellId>(book-1),ps,magic::CASTINGTYPE_SPELL);
									} // if don't knows spell
								} // if frozen
					        	} // if alive
						}
					}
					else
					{
						if (ShortFromCharPtr(buffer[s] +2) == 0x0543)  // Open spell book
						{
							ps->sendSpellBook(NULL);
						}
						break;
					}
				}
					break; // Lord Binary !!!!

				case PACKET_REQUESTHELP:
					gmmenu(s, 1);
					break;

				case PACKET_DIALOG_RESPONSE:
					//choice(s);
					Menus.handleMenu( ps );
					break;

				case PACKET_DYEITEM:
					Commands::DyeItem(s);
					break;

				case PACKET_STATUS_REQUEST:
                                        if ( pc_currchar != NULL ) {
						if (buffer[s][5]==4)
							statwindow(pc_currchar, pointers::findCharBySerPtr(buffer[s] +6));

						if (buffer[s][5]==5)
							skillwindow(s);
					}
					break;

				case PACKET_RENAMECHARACTER: ///Lag Fix -- Zippy //Bug Fix -- Zippy
					{
						P_CHAR pc_t=pointers::findCharBySerPtr(buffer[s]+1);
						if(ISVALIDPC(pc_t) && ( pc_currchar->IsGMorCounselor() || pc_currchar->isOwnerOf( pc_t ) ) )
							pc_t->setCurrentName( (char*)&buffer[s][5] );

					break;
					}

				case PACKET_READBOOK:
					{
					int size;
					size=dyn_length;
					P_ITEM pBook=pointers::findItemBySerPtr(buffer[s]+3);
					if(ISVALIDPI(pBook))
					{
						if (pBook->morez == 0)
							Books::addNewBook(pBook);

						if (pBook->morex!=666 && pBook->morex!=999)
							pBook->morex = 666;

						if (pBook->morex==666) // writeable book -> copy page data send by client to the class-page buffer
//							Books::books[pBook->morez].ChangePages((char*)buffer[s]+13, size-13, ShortFromCharPtr(buffer[s]+11) );
//							Books::books[pBook->morez].ChangePages((char*)buffer[s]+13, ShortFromCharPtr(buffer[s]+9), ShortFromCharPtr(buffer[s]+11) );
							Books::books[pBook->morez].ChangePages((char*)buffer[s]+13, ShortFromCharPtr(buffer[s]+9), ShortFromCharPtr(buffer[s]+11), size-13 );
						else if (pBook->morex==999)
							Books::books[pBook->morez].SendPageReadOnly(s, pBook, ShortFromCharPtr(buffer[s]+9));
					}
					break;
					}
				// client sends them out if the titel and/or author gets changed on a writable book
				// its NOT send (anymore?) when a book is closed as many packet docus state.
				// LB 7-dec 1999
				case PACKET_BOOKCHANGE:
					{
						int j= 9;
						char author[31],title[61],ch= 1;

						P_ITEM pBook=pointers::findItemBySerPtr(buffer[s]+1);
						if(!ISVALIDPI(pBook))
							break;

						while(ch!=0)
						{
							title[j-9]=buffer[s][j];
							j++;
							if (j>69) ch=0;
						}

						j=69;ch=1;
						while(ch!=0)
						{
							author[j-69]=buffer[s][j];
							j++;
							if (j>99) ch=0;
						}

						Books::books[pBook->morez].ChangeAuthor(author);
						Books::books[pBook->morez].ChangeTitle(title);

					}
					break;

				case PACKET_SET_SKILL_LOCKS:

								// client 1.26.2b+ skill managment packet
								// -> 0,1,2,3 -> ignore them
								// -> 4 = skill number
								// -> 5 = 0 raising (up), 1 falling=candidate for atrophy, 2 = locked
                                        if ( ISVALIDPC( pc_currchar ) )
						pc_currchar->lockSkill[buffer[s][4]]=buffer[s][5]; // save skill managment changes
					break;

				case PACKET_REQUEST_TIP:
					tips(s, ShortFromCharPtr(buffer[s] +1), 1/*buffer[s][3]*/); // flag:  1 = Next, 0 = Prev
					break;

				case PACKET_ATTACK_REQUEST:
					RcvAttack(ps);
					break;

				case PACKET_GUMPMENU_SELECT:
					Menus.handleMenu( ps );

					/*gumps::Button(	s,
							LongFromCharPtr(buffer[s] +11),
							buffer[s][3],
							buffer[s][4],
							buffer[s][5],
							buffer[s][6],
							LongFromCharPtr(buffer[s] +7),
							buffer[s][22]);
					*/
					break;

				case PACKET_GUMP_TEXTENTRY_INPUT:
					//gumps::Input(s);
					break;

				case PACKET_RESURRECT_CHOICE:
					if(buffer[s][1]==0x02)
					{
						P_CHAR murderer=pointers::findCharBySerial(pc_currchar->murdererSer);
						if( ( ISVALIDPC(murderer) ) && SrvParms->bountysactive )
						{
							sysmessage( s,TRANSLATE("To place a bounty on %s, use the command BOUNTY <Amount>."),
								murderer->getCurrentNameC()  );
						}
						sysmessage(s, TRANSLATE("You are now a ghost."));
					}
					if(buffer[s][1]==0x01)
						sysmessage(s, TRANSLATE("The connection between your spirit and the world is too weak."));
					break;

				case PACKET_BUYITEM:
					buyaction(s);
					break;

				case PACKET_SELLITEM:
					sellaction(s);
					break;

				case PACKET_SECURETRADING:
					trademsg(s);
					break;

				case PACKET_TIPS_REQUEST:// T2A Popuphelp request
					{
						if (ServerScp::g_nPopUpHelp==0) break;

						P_CHAR pc=pointers::findCharBySerPtr(buffer[s]+1);
						P_ITEM pi=pointers::findItemBySerPtr(buffer[s]+1);

						int len = 0;
						UI08 packet[4000]; packet[0] = '\0';
						if ( ISVALIDPC(pc_currchar) && pc_currchar->IsGM()) {
							if (ISVALIDPC(pc) )
								sprintf((char *)packet, "char n°%d serial : %x", DEREF_P_CHAR(pc), pc->getSerial32());
							if (ISVALIDPI(pi) )
								sprintf((char *)packet, "item n°%d serial : %x", DEREF_P_ITEM(pi), pi->getSerial32());
						}
						else
						{
							if (ISVALIDPC(pc))
								charGetPopUpHelp((char *)packet, pc);
							if (ISVALIDPI(pi))
								itemGetPopUpHelp((char *)packet, pi);
						}

						if ( !ISVALIDPC(pc) && !ISVALIDPI(pi))
							break;

						if (packet[0]=='\0') break;
						char2wchar((char *)packet);
						packet[0] = 0xB7;
						LongToCharPtr( LongFromCharPtr(buffer[s] +1), packet +3);
						int p = 0;
						while (p<600) {
							packet[7+p] = (UI08) Unicode::temp[p];
							packet[8+p] = (UI08) Unicode::temp[p+1];
							if ((Unicode::temp[p]=='\0')&&(Unicode::temp[p+1]=='\0')) break;
							p += 2;
						}
						p += 2;
						len = 7+p;
						ShortToCharPtr(len, packet +1);
						Xsend(s, packet, len);
//AoS/						Network->FlushBuffer(s);
					}
					break;

				case PACKET_PROFILE_REQUEST: {// T2A Profile request

						cPacketCharProfileReq p;
						p.receive( ps );
						profileStuff( ps, p );
					}
					break;

				case PACKET_MSGBOARD:
					MsgBoards::MsgBoardEvent(s);
					break;

				// Client version packet ... OSI clients always send that one on login.
				// even on re-logins
				case PACKET_CLIENT_VERSION: 	// client version control system (LB)

					char clientNumber[TEMP_STR_SIZE];
					strcpy((char*)clientNumber,(char*) &buffer[s][3]); // copy client version data
					if ( strlen (clientNumber) > 10) clientDimension[s] = 3; else clientDimension[s] = 2;
					sysmessage(s,TRANSLATE("You are using a %iD client, version %s"), clientDimension[s], clientNumber);

					viter = find(clientsAllowed.begin(), clientsAllowed.end(), "ALL");
					if ( viter != clientsAllowed.end() ) break; // ALL mode found/activated -> quit

					viter = find(clientsAllowed.begin(), clientsAllowed.end(), "SERVER_DEFAULT");
					if ( viter != clientsAllowed.end() )  // server_default mode ?
					{
						if ( strcmp( clientNumber, SUPPORTED_CLIENT) ) // check if client version matches
						{
							Disconnect(s);
							break;
						}
						break;
					}
					else
					{
				   		cpps.assign(clientNumber);
				   		viter = find(clientsAllowed.begin(), clientsAllowed.end(), cpps);
				   		if (viter != clientsAllowed.end() ) break; else
						{
							InfoOut("client %i disconnected by Client Version Control System\n", s);
							Disconnect(s);
						}
					}
           			break;

				case PACKET_MISC_REQ:

				// can't beleive this mega multipurpose packet isn't used :)
				// thought it's about time to change this , LB 30-March 2001
				// note: bf packet is used server and client side, here are only the client side ones
                		// I have encountered

					subcommand = ShortFromCharPtr(buffer[s] +3);

					// please don't remove the // unknowns ... want to have them as dokumentation
					switch (subcommand)
					{
						case 5: break; // unknown, sent once on login

				   		case GEN_INFO_SUBCMD_PARTY:
							Partys.recive( ps );
							break;

						case 9:	//Luxor: Wrestling Disarm Macro support
                                                        if ( ISVALIDPC( pc_currchar ) )
                                                        	pc_currchar->setWresMove(WRESDISARM);
							break;
						case 10: //Luxor: Wrestling Stun punch Macro support
                                                        if ( ISVALIDPC( pc_currchar ) )
								pc_currchar->setWresMove(WRESSTUNPUNCH);
							break;

						case 11: // client language, might be used for server localisation

							// please no strcpy or memcpy optimization here, because the input ain't 0-termianted and memcpy might be overkill
							client_lang[0]=buffer[s][5];
							client_lang[1]=buffer[s][6];
							client_lang[2]=buffer[s][7];
							client_lang[3]=0;
							// do dometihng with language information from client
							// ...
						   	break;

						case 12: break; // close gumps, client message

				   		case 14: // UO:3D menus
                                                        if ( ISVALIDPC( pc_currchar ) )
								pc_currchar->playAction(buffer[s][8]);
					  		break;

						case 15: break; // unknown, sent once on login

				   		default: break;

					}

					break;

				default:
					break;

    				} // end switch
				if (ISVALIDPC(pc_currchar))
					AMXEXECSV(pc_currchar->getSerial32(),AMXT_NETRCV, packet, AMX_AFTER);

			}
			else
			{
			// reset crypt memories
#ifdef ENCRYPTION
				if ( clientCrypter[s] != NULL)
				{
					delete (ClientCrypt * ) clientCrypter[s] ;
					clientCrypter[s]=NULL;
				}
#endif
				Disconnect(s) ; // Error on a read
			}
  		} // end if recv >0
  		else
  		{
	  //ConOut("FB: %i perm: %i\n",fb,perm[s]);
		  Disconnect(s); // extremly important (and tricky too ;-) !!!
	  // osi client closes socket AFTRER the first 4 bytes and re-opens it afterward.
	  // this line handles this correctly
		  }

 	} // end if newclient[]


}

void cNetwork::LoadHosts_deny()
{
	if( !hosts_deny.empty() )
		hosts_deny.clear();

	cScpIterator* iter = Scripts::HostDeny->getNewIterator("SECTION HOST_DENY");

	if( iter != NULL )
	{
		ip_block_st	ip_block;
		UI32		ip_address;
		std::string	sScript1,
				sToken1;
		UI32 		siEnd ;

		do
		{
			//let's load a IP addresss/NetMask
			sScript1 = iter->getEntry()->getFullLine();
			siEnd = sScript1.find("/") ;
			sToken1 = sScript1.substr(0,siEnd) ;
			ip_address = inet_addr(sToken1.c_str()) ;

			if (ip_address != INADDR_NONE)
			{
				ip_block.address = ip_address;

				// Get the rest of the string, after the '/' token
				if (siEnd != std::string::npos)
				{
					sToken1 = sScript1.substr(siEnd+1) ;
					ip_address = inet_addr(sToken1.c_str()) ;
					if (ip_address != INADDR_NONE)
						ip_block.mask = ip_address;
					else
						ip_block.mask = static_cast<unsigned long>(~0); // mask is not required. (fills all bits with 1's)
				}
				else
					ip_block.mask = static_cast<unsigned long>(~0);
				hosts_deny.push_back(ip_block);
			}
		}
		while ( sScript1[0] != '}' );
	}
	else
	{
		WarnOut("Can't find SECTION HOST_DENY! \n");
	}
}

bool cNetwork::CheckForBlockedIP(sockaddr_in ip_address)
{
	const int size = hosts_deny.size();

	for ( int i = 0; i < size; ++i )
		if( (ip_address.sin_addr.s_addr&hosts_deny[i].mask) == (hosts_deny[i].address&hosts_deny[i].mask ))
			return true;

	return false;
}

void NetThread::enqueue (char* buff, int len)
{
    for ( int i=0; i < len; ++i )
    {
        int p = ((int)(outtail)+i) % MTMAXBUFFER;
        if ( p == outhead && lastopwasinsert )
		return;
        lastopwasinsert = true;
        outbuffer[p] = buff[i];
    }
    outtail = ((int)(outtail)+len) % MTMAXBUFFER;
}

void NetThread::run()
{
    while(true)
    {
        mtxrun.enter();

        if ((((int)outhead) != ((int)outtail))||(lastopwasinsert)) {
            //now comes the worst of ever : the socket send in a circular buffer :]
            int tail = outtail; // store our own tail, any following data will be out
            int head = outhead; // just 'cos it's faster :]
            if (tail > head) { // no wrappz :] phew!
                send(realsocket, outbuffer+head, tail-head,0);
            } else {
                send(realsocket, outbuffer+head, MTMAXBUFFER-head,0);
                send(realsocket, outbuffer, tail,0);
            }
            lastopwasinsert = false;
            outhead = tail;
        }

        mtxrun.leave();
        Sleep(10);
   }

}


static TTHREAD startNetThread (void *ptr)
{
    NetThread *ns = static_cast<NetThread*>(ptr);
    ns->run();
    EXIT_TTHREAD;     // we don't dealloc mem since it's static :]
}

void NetThread::set(int s)
{
    realsocket=s;
    outtail = 0;
    outhead = 0;
    lastopwasinsert = false;
}

NetThread::NetThread(int s)
{
    set(s);
    mtxrun.enter();
    int n = tthreads::startTThread(startNetThread, this);
    InfoOut("Starting new network thread [%d]\n", n);
}


