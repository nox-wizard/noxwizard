  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file
\brief Remote Admin Console Stuff
\todo port to namespace? - Akron
\todo add useful commands ;)
*/

#ifndef __REMADMIN_H__
#define __REMADMIN_H__

/*******************************************************************************************
 REMOTE ADMINISTRATION
 *******************************************************************************************/
#define MAXRACLIENT 128
#define MAXLENGHT 128

//status of the connections :
#define RACST_CHECK_USR 0	//! this socket is entering the username
#define RACST_CHECK_PWD 1	//! this socket is entering the password
#define RACST_STDIN		2	//! this socket is using the standard commands

#define RACST_ACCESS_DENIED -1	//! this socket was denied access to any resource

#define DBG (MAXRACLIENT+12)


class RemoteAdmin
{
private:
	char inputbufs[MAXRACLIENT][MAXRACLIENT];
	char loginname[MAXRACLIENT][MAXRACLIENT];
	int  inputptrs[MAXRACLIENT];
	int  sockets[MAXRACLIENT];
	int  status[MAXRACLIENT];
	char oldinp[MAXRACLIENT][1024];
	int  racSocket;
	int  rac_port;
	int raclen_connection_addr;
	struct sockaddr_in racconnection;
	struct sockaddr_in racsockets_addr;
	struct sockaddr_in rac_sockets_addr;
	int  debugSocket;
	int racnow;

public:
	RemoteAdmin();
	~RemoteAdmin();
	void Init();
	void Exit();
	void CheckInp();
	void CheckConn();
	void Printf(int sock, char *fmt, ...);
	void Disconnect(int sock);
	void Rcv(int sock);
	void ProcessInput(int s);
};

#endif //__REMADMIN_H__
