  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#ifndef __NETWORK__H
#define __NETWORK__H

#include "nxwcommn.h"
#include "version.h"
#include <iostream>
#include "debug.h"

class ClientCrypt;
using namespace std ;

#if defined(__unix__)
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <sys/signal.h>
	#include <sys/errno.h>
	#include <arpa/inet.h>

	#define closesocket(s)	close(s)
	#define ioctlsocket ioctl
#else
	typedef int FAR socklen_t ;
#endif

#include "srvparms.h"

#ifdef _WINDOWS
	#include "nxwgui.h"
#endif


#define IPPRINTF(I) ((I)&(0xFF)),((I>>8)&(0xFF)),((I>>16)&(0xFF)),(I>>24)

#define MTMAXBUFFER 65536

typedef struct 
{
	unsigned long address;
	unsigned long mask;
}ip_block_st;

class NetThread 
{
	public:
		char outbuffer[MTMAXBUFFER];
		tthreads::Atomic<int> outhead;
		tthreads::Atomic<int> outtail;
		bool lastopwasinsert;
		tthreads::Mutex mtxrun;
		int realsocket;
		void run();
		void enqueue (char*, int);
		NetThread(int s);
		void set(int s);
};

enum {
	T2A = 0x0001,
	LBR = 0x0002
};


class cNetwork
{
	public:
		cNetwork();

		void enterchar(int s);
		void startchar(int s);
		void LoginMain(int s);
		void xSend(NXWSOCKET socket, const void *point, int length );
		void xSend(NXWSOCKET socket, wstring& p, bool alsoTermination = true );
		void Disconnect(NXWSOCKET s);
		void ClearBuffers();
		void CheckConn();
		void CheckMessage();
		void SockClose();
		void FlushBuffer(NXWSOCKET s);
		void LoadHosts_deny( void );
		bool CheckForBlockedIP(sockaddr_in ip_address);

		int kr,faul; // needed because global varaibles cant be changes in constructores LB

	private:

		std::vector<ip_block_st> hosts_deny;

		void DoStreamCode(NXWSOCKET  s);
		int  Pack(void *pvIn, void *pvOut, int len);
		void Login2(int s);
		void Relay(int s);
		void GoodAuth(int s);
		void charplay (int s);
		void CharList(int s);
		int  Receive(int s, int x, int a);
		void GetMsg(int s);
		char LogOut(NXWSOCKET s);
		void pSplit(char *pass0);
		void sockInit();
		void ActivateFeatures(NXWSOCKET s);
#ifdef ENCRYPTION
		unsigned char calculateLoginKey(unsigned char loginKey [4], unsigned char packetId );
		ClientCrypt * clientCrypter[MAXCLIENT+1]; //! save crypter per client socket
		unsigned char clientSeed[MAXCLIENT+1][4]; 
#endif
};

extern class cNetwork	*Network;

NXWCLIENT getClientFromSocket(int s);

#endif
