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
\brief Functions that send packages to the Client
\note cut from NoX-Wizard.cpp by Duke, 25.10.00
*/

#if !defined(AFX_SNDPKG_H__D538CC68_E5B0_480A_9752_F00069A33137__INCLUDED_)
#define AFX_SNDPKG_H__D538CC68_E5B0_480A_9752_F00069A33137__INCLUDED_

#include "particles.h"

void SndAttackOK(NXWSOCKET  s, int serial);
void SndDyevat(NXWSOCKET  s, int serial, short id);
void SndUpdscroll(NXWSOCKET  s, short txtlen, const char* txt);
void SndRemoveitem(int serial);
void SndShopgumpopen(NXWSOCKET  s, int serial);

void sysbroadcast(char *txt, ...);
	//!< System broadcast in bold text
void sysmessage(NXWSOCKET  s, const char *txt, ...);
	//!< System message (In lower left corner);
void sysmessage(NXWSOCKET  s, short color, const char *txt, ...);
void sysmessageflat(NXWSOCKET  s, short color, const char *txt);
	//!< System message (In lower left corner)
void itemmessage(NXWSOCKET  s, char *txt, int serial, short color=0x0000);
void wearIt(const NXWSOCKET  s, const P_ITEM pi);
void backpack2(NXWSOCKET s, SERIAL serial);
	//!< Send corpse stuff
void sendbpitem(NXWSOCKET s, P_ITEM pi);
void tileeffect(int x, int y, int z, char eff1, char eff2, char speed, char loop);
void senditem(NXWSOCKET  s, P_ITEM pi);
	//!< Send items (on ground);
void senditem_lsd(NXWSOCKET  s, ITEM i,char color1, char color2, int x, int y, signed char z);
void chardel (NXWSOCKET  s);
	//!< Deletion of character
void sendTargetCursor(NXWSOCKET s, int a1, int a2, int a3, int a4);
	//!< Send targetting cursor to client
void updatechar(P_CHAR pc);
	//!< If character status has been changed (Polymorph);, resend him
void target(NXWSOCKET  s, int a1, int a2, int a3, int a4, char *txt);
	//!< Send targetting cursor to client
void skillwindow(int s);
	//!< Opens the skills list, updated for client 1.26.2b by LB
void statwindow(P_CHAR pc_to, P_CHAR pc);
	//!< Opens the status window
void updates(NXWSOCKET  s);
	//!< Update Window
void tips(NXWSOCKET s, UI16 i, UI08 flag);
	//!< Tip of the day window
void deny(NXWSOCKET  k,P_CHAR pc, int sequence);
void weblaunch(int s, const char *txt);
	//!< Direct client to a web page
void broadcast(int s);
	//!< GM Broadcast (Done if a GM yells something);
void itemtalk( P_ITEM pi, char *txt);
	//!< Item "speech"

void MakeGraphicalEffectPkt(UI08 pkt[28], UI08 type, UI32 src_serial, UI32 dst_serial, UI16 model_id, Location src_pos, Location dst_pos, UI08 speed, UI08 duration, UI08 adjust, UI08 explode );

void staticeffect3(UI16 x, UI16 y, SI08 z, unsigned char eff1, unsigned char eff2, char speed, char loop, char explode);
#if 0
void movingeffect3(CHARACTER source, unsigned short x, unsigned short y, signed char z, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode);
#endif

void dolight(int s, char level);
void updateskill(int s, int skillnum);
void deathaction(P_CHAR player_id, P_ITEM corpse_id);
	//!< Character does a certain action
void deathmenu(NXWSOCKET s);
void SendPauseResumePkt(NXWSOCKET s, UI08 flag);
void SendDeleteObjectPkt(NXWSOCKET s, SERIAL serial);
void SendDrawObjectPkt(NXWSOCKET s, P_CHAR pc, int z);
void SendSecureTradingPkt(NXWSOCKET s, UI08 action, UI32 id1, UI32 id2, UI32 id3);
void SendSpeechMessagePkt(NXWSOCKET s, UI32 id, UI16 model, UI08 type, UI16 color, UI16 fonttype, UI08 sysname[30], UI08 *text);
void SendUnicodeSpeechMessagePkt(NXWSOCKET s, UI32 id, UI16 model, UI08 type, UI16 color, UI16 fonttype, UI32 lang, UI08 sysname[30], UI08 *unicodetext, UI16 unicodelen);
void SendUpdatePlayerPkt(NXWSOCKET s, UI32 player_id, UI16 model, Location pos, UI08 dir, UI16 color, UI08 flag, UI08 hi_color);
void SendDrawGamePlayerPkt(NXWSOCKET s, UI32 player_id, UI16 model, UI08 unk1, UI16 color, UI08 flag, Location pos, UI16 unk2, UI08 dir, bool useDispZ = false); 
void SendPlaySoundEffectPkt(NXWSOCKET s, UI08 mode, UI16 sound_model, UI16 unkn, Location pos, bool useDispZ = false);
void SendSetWeatherPkt(NXWSOCKET s, UI08 type, UI08 num, UI08 temperature);
void impowncreate(NXWSOCKET s, P_CHAR pc, int z);
	//!< socket, player to send
void sendshopinfo(int s, int c, P_ITEM pi);
int sellstuff(int s, int i);
void playmidi(int s, char num1, char num2);
void sendtradestatus(P_ITEM cont1, P_ITEM cont2);
void endtrade(SERIAL serial);
void tellmessage(int i, int s, char *txt);
void gmyell(char *txt);

void weather(NXWSOCKET s);
void dosocketmidi(int s);
void wornitems(NXWSOCKET  s, P_CHAR pc);
void bgsound(int s);
void pweather(NXWSOCKET  s);

#endif
