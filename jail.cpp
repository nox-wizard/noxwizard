  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "jail.h"
#include "items.h"
#include "chars.h"
#include "basics.h"
#include "inlines.h"

PRISONCELLVECTOR prison::cells;
JAILEDVECTOR prison::jailed;
 
 
/*!
\brief Constructor of cJailed
\author Endymion
*/
cJailed::cJailed()
{
	this->sec = 0;
	this->timer = 0;
	this->why = "";
	this->cell=INVALID;
};

/*!
\brief Destructor of cJailed
\author Endymion
*/
cJailed::~cJailed() { };

/*!
\brief Constructor of cPrisonCell
\author Endymion
*/
cPrisonCell::cPrisonCell()
{
	this->pos.x=0; this->pos.y=0; this->pos.z=0; this->pos.dispz=0;
	this->serial=INVALID;
	this->free=true;
};

/*!
\brief Destructor of cJailed
\author Endymion
*/
cPrisonCell::~cPrisonCell() { };

namespace prison {
/*!
\brief archive a jail worldfile
\author Sparhawk
*/
void archive()
{
	std::string saveFileName( SrvParms->savePath + SrvParms->jailWorldfile + SrvParms->worldfileExtension );
	std::string timeNow( getNoXDate() );
	for( int i = timeNow.length() - 1; i >= 0; --i )
		switch( timeNow[i] )
		{
			case '/' :
			case ' ' :
			case ':' :
				timeNow[i]= '-';
		}
	std::string archiveFileName( SrvParms->archivePath + SrvParms->jailWorldfile + timeNow + SrvParms->worldfileExtension );


	if( rename( saveFileName.c_str(), archiveFileName.c_str() ) != 0 )
	{
		LogWarning("Could not rename/move file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
	}
	else
	{
		InfoOut("Renamed/moved file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
	}
}

}

void prison::safeoldsave()
{
	std::string oldFileName( SrvParms->savePath + SrvParms->jailWorldfile + SrvParms->worldfileExtension );
	std::string newFileName( SrvParms->savePath + SrvParms->jailWorldfile + SrvParms->worldfileExtension + "$" );
	remove( newFileName.c_str() );
	rename( oldFileName.c_str(), newFileName.c_str() );
}

/*!
\brief jail a character
\author Endymion
\param jailer the jailer
\param pc the player who jail
\param secs the second of jail
*/
void prison::jail( P_CHAR jailer, P_CHAR pc, UI32 secs )
{

	VALIDATEPC(pc);

	if( pc->jailed ) 
	{
		if( ISVALIDPC( jailer ) )
			jailer->sysmsg("That player is already in jail!");
		return;
	}

	PRISONCELLVECTOR::iterator cell( prison::cells.begin() ), end( prison::cells.end() );
	while( (cell!=end) && !(cell->free) ) ++cell;

	if( cell==end ) //no cell free
	{
		if( ISVALIDPC( jailer ) )
			jailer->sysmsg(TRANSLATE("No free cells to jail %s"), pc->getRealNameC() );
		return;
	}

	cell->free = false;

	cJailed j;

	j.serial=pc->getSerial32();
	j.oldpos=pc->getPosition();
	j.sec=secs;
	j.timer=uiCurrentTime + MY_CLOCKS_PER_SEC *secs;
	j.cell=cell->serial;
	prison::jailed.push_back( j );

	pc->MoveTo( cell->pos );
	pc->jailed=true;
	pc->teleport();


	pc->sysmsg(TRANSLATE("You are jailed !"));

	if( ISVALIDPC( jailer ) )
		jailer->sysmsg( "Player %s has been jailed in cell %i.", pc->getCurrentNameC(), cell->serial);

}

/*!
\brief Auto check and free of jailed people
\author Endymion
*/
void prison::checkForFree()
{
	JAILEDVECTOR::iterator next;
	for( JAILEDVECTOR::iterator j = prison::jailed.begin(); j!=prison::jailed.end(); j=next )
	{
		next=j; next++;
		if( TIMEOUT( (*j).timer ) ) {
			P_CHAR pc = pointers::findCharBySerial( (*j).serial );
			if( ISVALIDPC( pc ) ) {
				prison::release( NULL, pc );
			}
			else { //invalid char in jail.. so remove it
				WarnOut( "invalid char serial [ %i ] found in jail, removing..", (*j).serial );
				prison::freePrisonCell( (*j).cell );
				prison::jailed.erase( j );
			}
			return;
		}
		
	}

}

/*!
\brief Release a jailed player
\author Endymion
\param releaser the releaser
\param pc the player jailed
*/
void prison::release( P_CHAR releaser, P_CHAR pc )
{
	VALIDATEPC(pc);
	JAILEDVECTOR::iterator j = prison::jailed.begin();
	while(  j!=prison::jailed.end() && (*j).serial!=pc->getSerial32() )	j++;
	if(j==prison::jailed.end()) {
		if( ISVALIDPC( releaser ) ) 
			releaser->sysmsg( "The player isn't jailed" );
		return;
	}

	pc->MoveTo(  j->oldpos );

	prison::freePrisonCell( j->cell );
	prison::jailed.erase( j );

	pc->jailed=false;
	pc->teleport();

	if( !ISVALIDPC( releaser ) ) {
		char temp[TEMP_STR_SIZE];
		sprintf( temp, "%s is auto-released from jail \n", pc->getCurrentNameC() );
		ServerLog.Write("%s", temp);
		pc->sysmsg(TRANSLATE( "Your jail time is over!" ));
	}
	else {
		releaser->sysmsg("%s is now free", pc->getCurrentNameC());
		pc->sysmsg( "%s have released you", releaser->getCurrentNameC() );
	}
	

}

/*!
\brief Free a cell
\author Endymion
\param cell the cell serial
\note only use internal.. use release for release a player
*/
void prison::freePrisonCell( SERIAL cell )
{
	PRISONCELLVECTOR::iterator iter= prison::cells.begin();
	while( iter!=prison::cells.end() && (*iter).serial!=cell ) iter++;
	if( iter==prison::cells.end() ) {
		WarnOut( "Free a cell not found ( serial %i )", cell );
		return;
	}

	(*iter).free=true;


}


void prison::addCell( SERIAL serial, UI32 x, UI32 y, UI32 z )
{
	for( PRISONCELLVECTOR::iterator j = prison::cells.begin(); j!=prison::cells.end(); j++ )
		if( (*j).serial == serial )
			return;

	cPrisonCell c;

	c.serial=serial;
	c.pos.x=x;
	c.pos.y=y;
	c.pos.z=z;
	c.pos.dispz=z;

	prison::cells.push_back( c );


	
}

/*!
\brief Get the defaul position for prison cell
\author Sabrewulf, moved here by Endymion
\param jailnum 
\param x the x position
\param y the y position
\param z the z position
\remark this is just an helper function to get the location of the original jails
*/
void prison::standardJailxyz (int jailnum, int& x, int& y, int& z)
{
	switch (jailnum)
	{
		case 1:
			x=5276; // Jail1
			y=1164;
			z=0;
			break;
		case 2:
			x=5286; // Jail2
			y=1164;
			z=0;
			break;
		case 3:
			x=5296; // Jail3
			y=1164;
			z=0;
			break;
		case 4:
			x=5306; // Jail4
			y=1164;
			z=0;
			break;
		case 5:
			x=5276; // Jail5
			y=1174;
			z=0;
			break;
		case 6:
			x=5286; // Jail6
			y=1174;
			z=0;
			break;
		case 7:
			x=5296; // Jail7
			y=1174;
			z=0;
			break;
		case 8:
			x=5306; // Jail8
			y=1174;
			z=0;
			break;
		case 9:
			x=5283; // Jail9
			y=1184;
			z=0;
			break;
		case 10:
			x=5304; // Jail10
			y=1184;
			z=0;
			break;
		default:
			x=INVALID;
			y=INVALID;
			z=INVALID;
	}
}



