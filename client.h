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
\brief cNxwClientObj class
\author Luxor & Xanathar
*/
#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "cmdtable.h"


typedef std::vector< std::string > td_cmdparams;
typedef std::vector< TargetLocation > td_targets;

class cNxwClientObj {
public:
/*    P_CHAR m_pcCurrChar;
    bool m_bDragging;
    bool m_bNoWeather;
    bool m_bEvilDraggg; // for UO:3D clients
    bool m_bFirstPacket;
    unsigned m_uPriv3[8];
    int  m_nNewClient;
    bool m_bShouldPack;
    BYTE m_RcvBuffer[MAXBUFFER];
    BYTE m_SndBuffer[MAXBUFFER];
    int m_nSndBufLen;
    int m_nRcvBufLen;
    int m_nRealSocket;
    short m_sWalkSequence;
    unsigned char m_ucAddId[8];
    int m_nAdd[6];
    int m_iAccount;
    make_st m_ItemMake;
    bool m_bInGame;
*/
/*  -- not yet implemented ;) --
    void doStreamCode();
    void flushBuffer();
    void sendAccessDenied(int reason);
    void sendServerList();
    void sendRelayPacket();
    void sendStartInfos();
    void secondLogin();
    void charPlay();
    void sendStartUpStuff();
    bool logout();
    int receive(int len, int ofs);
	-- end of future stuff --  */
	NXWSOCKET m_sck;
	P_COMMANDSTEP currentCommand;
public:
	cNxwClientObj( NXWSOCKET s );
	int toInt();
	NXWSOCKET toLegacySocket();
	void setLegacySocket(NXWSOCKET s);

	//@{
	/*!
	\name Client Status
	*/
	bool isDragging();
	void setDragging();
	void resetDragging();
	bool inGame();
	P_CHAR currChar();
	int currCharIdx();
	int getRealSocket();
	BYTE *getRcvBuffer();
	//@}

	//@{
	/*!
	\name Packets
	*/
	void send(const void *point, int length);
	void sendSpellBook(P_ITEM pi);
	void sendSFX(unsigned char a, unsigned char b, bool bIncludeNearby = true);
	void sendRemoveObject(P_OBJECT po);
	void sysmsg(char* szFormat, ...);
	void sysmsg(short color, char* szFormat, ...);
	//@}

	void receiveTarget(TargetLocation tl);
	void doTargeting(char* message);
	td_targets getTargets();
	TargetLocation* getLastTarget();
	void startCommand(P_COMMAND cmd, char* params);
	void continueCommand();
	string getParamsAsString();

	td_cmdparams cmdParams;
	td_targets targets;
};



/*typedef std::map< SERIAL, cNxwClientObj > CLIENTS_MAP;
extern CLIENTS_MAP allclients;*/

typedef std::vector< SERIAL > SOCKETS_VECT;
//extern SOCKETS_VECT currchar;
extern SERIAL currchar[MAXCLIENT];

// the best very stupid class
/*class cCurrCharWrapper {
public:
	SERIAL operator[]( NXWSOCKET s ) {
		return allsockets[s];
	}
};

extern cCurrCharWrapper currchar;*/



#endif
