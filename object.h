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

#define ISVALIDPO(po) ( ( po!=NULL && ( sizeof(*po) == sizeof(cObject) || sizeof(*po) == sizeof(cChar) || sizeof(*po) == sizeof(cItem) ) ) ? (po->getSerial32() >= 0) : false )
#define VALIDATEPO(po) if (!ISVALIDPO(po)) { LogWarning("a non-valid P_OBJECT pointer was used in %s:%d", basename(__FILE__), __LINE__); return; }
#define VALIDATEPOR(po, r) if (!ISVALIDPO(po)) { LogWarning("a non-valid P_OBJECT pointer was used in %s:%d", basename(__FILE__), __LINE__); return r; }

Location Loc(SI32 x, SI32 y, signed char z, signed char dispz=0);


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

	cObject();
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
	const char*		getSecondaryNameC() const;
	string			getSecondaryName() const;
	void			setSecondaryName(const char *format, ...);
//@}

//@{
/*!
\name Others
*/
private:
	UI32			ScriptID;
	LOGICAL			tempfx[tempfx::MAX_TEMPFX_INDEX];            //!< Luxor - customizable non-repeteable temp effects system

public:
	void			setTempfx( SI32 arg );
	void			resetTempfx( SI32 arg );
	LOGICAL			getTempfx( SI32 arg );
	UI32			getScriptID();
	void			setScriptID(UI32 sid);
//@}
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

