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

private:

	static SERIAL serial_current;

protected:

	Location loc;
	UI16 model;
	SERIAL clicked;

public:

	SERIAL serial;
	bool type;

	SERIAL buffer[4];
	std::string buffer_str[4];
	AmxFunction* amx_callback;
	processTarget code_callback;

	cTarget( bool selectLocation=true );
	virtual ~cTarget();

	void send( NXWCLIENT ps );
	void receive( NXWCLIENT ps );
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


void amxCallbackOld( NXWCLIENT ps, P_TARGET t );
void amxCallback( NXWCLIENT ps, P_TARGET t );


typedef enum {
	TARG_ALL = 0,
	TARG_OBJ,
	TARG_CHAR,
	TARG_ITEM,
	TARG_LOC
} TARG_TYPE;


P_TARGET createTarget( TARG_TYPE type );









class TargetLocation
{
private:
	P_CHAR m_pc;
	int    m_pcSerial;
	P_ITEM m_pi;
	int    m_piSerial;
	int    m_x;
	int    m_y;
	int    m_z;
	void   init(int x, int y, int z);
	void   init(P_CHAR pc);
	void   init(P_ITEM pi);

public:
	//!creates a target loc from a character
	TargetLocation(P_CHAR pc) { init(pc); }
	//!creates a target loc from an item
	TargetLocation(P_ITEM pi) { init(pi); }
	//!creates a target loc from a target net packet
	TargetLocation( P_TARGET pp );
	//!creates a target loc from a xyz position in the map
	TargetLocation(int x, int y, int z) { init(x,y,z); }

	//!recalculates item&char from their serial
	void revalidate();

	//!gets the targetted char if any, NULL otherwise
	inline P_CHAR getChar() { return m_pc; }
	//!gets the targetted item if any, NULL otherwise
	inline P_ITEM getItem() { return m_pi; }
	//!gets the XYZ location of this target location
	inline void getXYZ(int& x, int& y, int& z) { x = m_x; y = m_y; z = m_z; }
	//!extends a P_ITEM data to P_CHAR and x,y,z
	void extendItemTarget();
};





#endif
