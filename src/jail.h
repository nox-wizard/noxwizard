  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file jail.h
\brief Definition of classes cPrison for manage jail
\todo complete documentation
*/
#ifndef __JAIL_H__
#define __JAIL_H__

 /*
\class cJailed
\author Endymion

the jailed
*/
class cJailed
{
public:
	cJailed();
	~cJailed();
	
	SERIAL	serial;		//!< serial of char jailed
	Location oldpos;	//!< old position
	std::string	why;	//!< guilty description
	UI32	sec;		//!< duration of jail
	UI32	timer;		//!< timer for jail
	SERIAL	cell;		//!< cell

};

typedef std::vector< cJailed > JAILEDVECTOR;

/*
\class cPrisonCell
\author Endymion

a cell of the prison
*/
class cPrisonCell
{
public:
	cPrisonCell();
	~cPrisonCell();

	SERIAL serial;		//!< serial of the cell
	Location pos;		//!< position of the cell
	bool free;		//!< free

};

	
typedef std::vector<cPrisonCell> PRISONCELLVECTOR;
typedef std::vector<cJailed> JAILEDVECTOR;

/*
\class cPrison
\author Endymion

manage the jail
*/
namespace prison {

	extern PRISONCELLVECTOR cells;	//!< list of cells
	extern JAILEDVECTOR jailed;	//!< list of jailed people

	void archive();
	void safeoldsave();
	void jail( P_CHAR jailer, P_CHAR pc, UI32 secs );
	void release( P_CHAR releaser, P_CHAR pc );
	void checkForFree();
	void freePrisonCell( SERIAL cell );
	void addCell( SERIAL serial, UI16 x, UI16 y, SI08 z );
	void standardJailxyz (int jailnum, int& x, int& y, int& z);

};

#endif
