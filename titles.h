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
\brief Titles Functions
*/

#ifndef __TITLES_H__
#define __TITLES_H__

char *title1(P_CHAR pc);
char *title2(P_CHAR pc);
char *title3(P_CHAR pc);
char *complete_title(P_CHAR pc);

// for newbie stuff
int bestskill(P_CHAR p);

//For custom titles
void loadcustomtitle();

#endif
