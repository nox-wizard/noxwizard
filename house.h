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
\brief House Related Stuff
*/
#ifndef _HOUSE_H_
#define _HOUSE_H_

extern std::map< SERIAL, P_CHAR > houses;

void 	addthere(int s, int xx, int yy, int zz, int t);
int 	add_hlist(int c, int h, int t);
void	buildhouse(int s, int i);
int 	check_house_decay();
void 	deedhouse(NXWSOCKET  s, P_ITEM pi);
int 	del_hlist(int c, int h);
int		on_hlist(P_ITEM pi, unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4, int *li);
LOGICAL house_speech( P_CHAR pc, NXWSOCKET socket, std::string &talk);
void 	killkeys(SERIAL serial);
void 	mtarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt);


#endif
