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
\brief Utility functions
*/

#ifndef __UTILS_H__
#define __UTILS_H__

#include "globals.h"

class cScriptCommand {

	public:

		std::string command;
		std::string param;

		cScriptCommand( );
		cScriptCommand( std::string command, std::string param );
		~cScriptCommand();

		void execute( NXWSOCKET s );

};


int calcSerFromChar(int ser);
int checkBoundingBox(int xPos, int yPos, int fx1, int fy1, int fz1, int fx2, int fy2);
int checkBoundingCircle(int xPos, int yPos, int fx1, int fy1, int fz1, int radius);
int fielddir(int s, int x, int y, int z);
void npcsimpleattacktarget(int target2, int target);

// Day and Night related prototypes
char indungeon(P_CHAR pc);
void setabovelight(unsigned char);

void scriptcommand (NXWSOCKET s, std::string script1, std::string script2);
void endmessage(int x);
void delequan(int s, short id, int amount, int & not_deleted = dummy__);
void donewithcall(int s, int type);
void dooruse(NXWSOCKET s, P_ITEM pi/*int item*/);
int calcValue(int i, int value);
int calcGoodValue(int npcnum, int i, int value,int goodtype); // by Magius(CHE) for trade system

int whichbit( int number, int bit );
int numbitsset( int number );

void getSextantCoords(SI32 x, SI32 y, LOGICAL t2a, char *sextant);
void splitline();

int hexnumber(int countx);

void batchcheck(int s);
void location2xyz(int loc, int& x, int& y, int& z);

UI32 getclock();
UI32 getsysclock();
UI32 getclockday();
void initclock();


//@{
/*!
\name Strings
\brief Strings functions
*/

char *linestart(char *line);
void strupr(std::string &str);
void strlwr(std::string &str);

int strtonum(int countx, int base= 0);

//@}

#endif
