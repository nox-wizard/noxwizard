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
\brief Gumps Functions
*/

#ifndef __GUMPS_H__
#define __GUMPS_H__

void whomenu(int s, int type);
void playermenu(int s, int type);
void gmmenu(int s, int m);
void choice(int s);
void who(int s);
void itemmenu(int s, int m);

void entrygump(int s, unsigned char tser1, unsigned char tser2, unsigned char tser3, unsigned char tser4, unsigned char type, char index, short int maxlength, char *text1);


#endif
