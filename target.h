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


/*
0x6C Packet

Targeting Cursor Commands (19 bytes) 

	BYTE cmd 
    BYTE type 
		0x00 = Select Object
		0x01 = Select X, Y, Z 

	BYTE[4] cursorID 
	BYTE Cursor Type 
		Always 0 now  
	
	The following are always sent but are only valid if sent by client 

	BYTE[4] Clicked On ID 
	BYTE[2] click xLoc 
	BYTE[2] click yLoc 
	BYTE unknown (0x00) 
	BYTE click zLoc 
	BYTE[2] model # (if a static tile, 0 if a map/landscape tile)
		Note: the model # shouldn’t be trusted.

*/

class cTargeT {

public:

	cTargeT( );
	void Error( NXWCLIENT ps, char* txt );
	virtual void Do( NXWCLIENT ps );
};

typedef cTargeT* P_TARGET;

typedef void ( *processSerialTarget)	(NXWCLIENT, SERIAL);
typedef void ( *processCharTarget)		(NXWCLIENT, P_CHAR);
typedef void ( *processItemTarget)		(NXWCLIENT, P_ITEM);
typedef void ( *processLocationTarget)	(NXWCLIENT, Location);

class cTargetSerial : public cTargeT {
protected:
	processSerialTarget call;
	bool isObjectTarget( NXWCLIENT ps );
	SERIAL makeSerial( NXWCLIENT ps );
public:
	cTargetSerial( processSerialTarget callThis );
	cTargetSerial( );
	virtual void Do( NXWCLIENT ps );
};

class cTargetItem : public cTargetSerial {
private:
	processItemTarget call;
	bool isItemTarget( NXWCLIENT ps );
public:
	cTargetItem( processItemTarget callThis );
	void Do( NXWCLIENT ps );
};

class cTargetChar : public cTargetSerial {
private:
	processCharTarget call;
	bool isCharTarget( NXWCLIENT ps );
public:
	cTargetChar( processCharTarget callThis );
	void Do( NXWCLIENT ps );
};

class cTargetLocation : public cTargeT {
private:
	Location location;
	processLocationTarget call;
	bool isLocationTarget( NXWCLIENT ps );
	Location makeLocation( NXWCLIENT ps );
public:
	cTargetLocation( processLocationTarget callThis );
	void Do( NXWCLIENT ps );
};


#endif
