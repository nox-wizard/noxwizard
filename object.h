  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file object.h
\brief Declaration of class cObject
\author Anthalir
*/

#ifndef __OBJECT_H
#define __OBJECT_H

#include <stdarg.h>
#include "constants.h"
#include "typedefs.h"
#include "amx/amxcback.h"
#include "tmpeff.h"


#define ISVALIDPO(po) ( ( po!=NULL && ( sizeof(*po) == sizeof(cObject) || sizeof(*po) == sizeof(cChar) || sizeof(*po) == sizeof(cItem) ) ) ? (po->getSerial32() >= 0) : false )
#define VALIDATEPO(po) if (!ISVALIDPO(po)) { LogWarning("a non-valid P_OBJECT pointer was used in %s:%d", basename(__FILE__), __LINE__); return; }
#define VALIDATEPOR(po, r) if (!ISVALIDPO(po)) { LogWarning("a non-valid P_OBJECT pointer was used in %s:%d", basename(__FILE__), __LINE__); return r; }

class cScpIterator;

Location Loc(SI32 x, SI32 y, signed char z, signed char dispz=0);

typedef map< UI32, AmxEvent* > AmxEventMap;

typedef slist< tempfx::cTempfx > TempfxVector;

//! == operator redefinition for Location
int operator ==(Location a, Location b);
//! != operator redefinition for Location
int operator !=(Location a, Location b);

/*!
\brief Base class of cItem and cChar
\author Anthalir
\since 0.82a
*/
class cObject
{
protected:
	enum{OBJECT, ITEM, CHAR, } objtype;
//@{
/*!
\name Operators
*/
public:
	bool			operator> (cObject&);
	bool			operator< (cObject&);
	bool			operator>=(cObject&);
	bool			operator<=(cObject&);
	bool			operator==(cObject&);
	bool			operator!=(cObject&);
//@}

public:
	static std::string	getRandomScriptValue( std::string section, std::string& sectionId );
private:
	static cScpIterator*	getScriptIterator( std::string section, std::string& sectionId );

public:
	cObject();
	virtual ~cObject();
//@{
/*!
\name Serials
\brief functions for handle serials stuff
*/
private:
	Serial			serial;			//!< serial of the object
	Serial			multi_serial;		//!< multi serial of the object (don't know what it is used for)
	Serial			OwnerSerial;		//!< If Char is an NPC, this sets its owner

public:
	const Serial		getSerial() const;
	const SI32		getSerial32() const;
	void			setSerial32(SI32 newserial);
	const void		setSerialByte(UI32 nByte, BYTE value);

	const Serial		getMultiSerial() const;
	const SI32		getMultiSerial32() const;
	void			setMultiSerial32Only(SI32 newserial);
	const void		setMultiSerialByte(UI32 nByte, BYTE value);

	const Serial		getOwnerSerial() const;
	const SI32      	getOwnerSerial32() const;
	void            	setOwnerSerial32(SI32 newserial, bool force=false );
	void            	setOwnerSerial32Only(SI32 newserial);
	const void		setOwnerSerialByte(UI32 nByte, BYTE value);
	void			setSameOwnerAs(const cObject* obj);
//@}

//@{
/*!
\name Positions
\brief Position related stuff
*/
private:
	Location		old_position;		//!< old position of object
	Location		position;		//!< current position of object

public:
	Location		getPosition() const;
	SI32			getPosition(const char *what) const;
	void			setPosition(const char *what, SI32 value);
	void			setPosition(UI32 x, UI32 y, SI08 z);
	void			setPosition(Location where);

	Location		getOldPosition() const;
	SI32			getOldPosition(const char *what) const;
	void			setOldPosition(const char *what, SI32 value);
	void			setOldPosition(SI32 x, SI32 y, signed char z, signed char dispz);
	void			setOldPosition(Location where);
//@}

//@{
/*!
\name Appearence
*/
protected:
	string			secondary_name;
/*!<
Real name of the char, 30 chars max + '\\0'<br>
Also used to store the secondary name of items.
*/
	string			current_name;	//!< Name displayed everywhere for this object, 30 char max + '\\0'

public:
	string 			getRealName() const;
	const char*		getRealNameC() const;
	void			setRealName(string s);
	void			setRealName(const char *str);

	string			getCurrentName() const;
	const char*		getCurrentNameC() const;
	void			setCurrentName(string s);
	void			setCurrentName(const char *str);
	void			setCurrentName(char *format, ...);

	// for items
	void 			setSecondaryName( string s );
	const char*		getSecondaryNameC() const;
	string			getSecondaryName() const;
	void			setSecondaryName(const char *format, ...);
//@}

//@{
/*!
\name Temp - Fx
*/
private:
	UI32			ScriptID;
	TempfxVector		*tempfx;

public:
	LOGICAL			addTempfx( cObject& src, SI32 num, SI32 more1, SI32 more2, SI32 more3, SI32 dur, SI32 amxcback );
	void			delTempfx( SI32 num, LOGICAL executeExpireCode = true, SERIAL funcidx = INVALID );
	void			checkTempfx();
	void			tempfxOn();
	void			tempfxOff();
	LOGICAL			hasTempfx();
	tempfx::cTempfx*	getTempfx( SI32 num, SERIAL funcidx = INVALID );
	UI32			getScriptID();
	void			setScriptID(UI32 sid);

//@}

	UI32	disabled;	//!< Disabled object timer, cant trigger.
	std::string*	disabledmsg; //!< Object is disabled, so display this message.
					

public:
	virtual	void		Delete();
#ifdef SPAR_LOCATION_MAP
private:
	UI32			locationKey;
public:
	UI32			getLocationKey();
	void			setLocationKey();
#endif
/*
private:
	AmxEventMap		*amxEvents;
public:
	virtual	LOGICAL		isValidAmxEvent( UI32 eventId );
	AmxEvent*		getAmxEvent( UI32 eventId );
	AmxEvent*		setAmxEvent( UI32 eventId, char *amxFunction, LOGICAL dynamic = false );
	cell			runAmxEvent( UI32 eventID, SI32 param1, SI32 param2 = INVALID, SI32 param3 = INVALID, SI32 param4 = INVALID );
	void 			delAmxEvent( UI32 eventID );
*/
} PACK_NEEDED;

/*!
\brief spawner
*/
class cSpawner : public cObject
{
};

class cItemSpawner : public cSpawner
{
};

class cNpcSpawner : public cSpawner
{
};

#endif	// __OBJECT_H

