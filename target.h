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
\brief Target Related Stuff
*/

#ifndef __TARGET_H__
#define __TARGET_H__

#include "packets.h"

typedef class cTarget* P_TARGET;
typedef void ( *processTarget )			( NXWCLIENT, P_TARGET );

class cTarget {

public:

	cPacketTargetingCursor pkg;
	SERIAL buffer[4];
	std::string buffer_str[4];
	AmxFunction* amx_callback;
	processTarget code_callback;

	cTarget( bool selectLocation=true );
	~cTarget();

	void send( NXWCLIENT ps );
	virtual bool isValid();
	virtual void error( NXWCLIENT ps );

	Location getLocation();
	SERIAL getClicked();
	UI16 getModel();

};


class cObjectTarget : public cTarget {

public:

	cObjectTarget();
	~cObjectTarget();

	virtual bool isValid();
	virtual void error( NXWCLIENT ps );
};

class cCharTarget : public cObjectTarget {

public:

	cCharTarget();
	~cCharTarget();

	virtual bool isValid();
	virtual void error( NXWCLIENT ps );
};

class cItemTarget : public cObjectTarget {
	
public:

	cItemTarget();
	~cItemTarget();

	virtual bool isValid();
	virtual void error( NXWCLIENT ps );
};

class cLocationTarget : public cTarget {

public:

	cLocationTarget();
	~cLocationTarget();

	virtual bool isValid();
	virtual void error( NXWCLIENT ps );
};


void amxCallback( NXWCLIENT ps, P_TARGET t );

#endif
