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
\brief Extraction skills related stuff
\author Endymion
*/

#ifndef __EXTRACTION_SKILLS_H
#define __EXTRACTION_SKILLS_H

#include "sndpkg.h"
#include "amx/amxcback.h"
#include "globals.h"

#undef AMXMINING
#define AMXMINING "__nxw_sk_mining"

class cResource {
public:

	cResource( ) { this->consumed=0; timer=uiCurrentTime; }

	TIMERVAL timer; //!< timer for respawn
	UI32 consumed; //!< amount of resource consumed

};

typedef cResource* P_RESOURCE;

typedef std::map< UI64, cResource > RESOURCE_MAP;

class cResources {


private:

	RESOURCE_MAP resources; //!< all resources

	UI64 getBlocks( Location location  );
	P_RESOURCE createBlock( Location location );
	void deleteBlock( Location Location );
	bool checkRes( P_RESOURCE res );

public:


	UI32		n; //!< number of resource max
	TIMERVAL	time; //!< 
	TIMERVAL	rate; //!< respawn rate
	SI32		stamina; //!< stamina used
	UI32		area_width; //!< resource area width
	UI32		area_height; //!< resource area height

	cResources( UI32 areawidth = 10, UI32 areaheight = 10 );
	~cResources();

	void setDimArea( UI32 areawidth, UI32 areaheight );
	
	P_RESOURCE getResource( Location location );
	void checkResource( Location location, P_RESOURCE& res );
	void decreaseResource( Location location, P_RESOURCE res = NULL );
	bool thereAreSomething( P_RESOURCE res );

	void checkAll();

};


extern cResources ores;


#endif

