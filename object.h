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
#include "basics.h"

#define ISVALIDPO(po) ( ( po!=NULL && ( sizeof(*po) == sizeof(cObject) || sizeof(*po) == sizeof(cChar) || sizeof(*po) == sizeof(cItem) ) ) ? (po->getSerial32() >= 0) : false )
#define VALIDATEPO(po) if (!ISVALIDPO(po)) { LogWarning("a non-valid P_OBJECT pointer was used in %s:%d", basename(__FILE__), __LINE__); return; }
#define VALIDATEPOR(po, r) if (!ISVALIDPO(po)) { LogWarning("a non-valid P_OBJECT pointer was used in %s:%d", basename(__FILE__), __LINE__); return r; }

class cScpIterator;

typedef map< UI32, AmxEvent* > AmxEventMap;

typedef slist< tempfx::cTempfx > TempfxVector;

//! == operator redefinition for Location
inline bool operator ==(Location a, Location b)
{ return ((a.x==b.x) && (a.y==b.y) && (a.z==b.z)); }

//! != operator redefinition for Location
inline bool operator !=(Location a, Location b)
{ return ((a.x!=b.x) || (a.y!=b.y) || (a.z!=b.z)); }

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
	inline bool		operator> (const cObject& obj) const
	{ return(getSerial32() >  obj.getSerial32()); }

	inline bool		operator< (const cObject& obj) const
	{ return(getSerial32() <  obj.getSerial32()); }

	inline bool		operator>=(const cObject& obj) const
	{ return(getSerial32() >= obj.getSerial32()); }

	inline bool		operator<=(const cObject& obj) const
	{ return(getSerial32() <= obj.getSerial32()); }

	inline bool		operator==(const cObject& obj) const
	{ return(getSerial32() == obj.getSerial32()); }

	inline bool		operator!=(const cObject& obj) const
	{ return(getSerial32() != obj.getSerial32()); }
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
	//! return the object's serial
	inline const Serial	getSerial() const
	{ return serial; }

	//! return the serial of the object
	inline const SI32	getSerial32() const
	{ return serial.serial32; }

	void			setSerial32(SI32 newserial);
	const void		setSerialByte(UI32 nByte, BYTE value);

	//! return the object's multi serial
	inline const Serial	getMultiSerial() const
	{ return multi_serial; }

	//! return the multi serial of the object
	inline const SI32	getMultiSerial32() const
	{ return multi_serial.serial32; }

	//! Set the multi serial of the object
	inline void		setMultiSerial32Only(SI32 newserial)
	{ multi_serial.serial32= newserial; }

	const void		setMultiSerialByte(UI32 nByte, BYTE value);

	//! return the object's owner serial
	inline const Serial	getOwnerSerial() const
	{ return OwnerSerial; }

	//! return the object's owner serial
	inline const SI32      	getOwnerSerial32() const
	{ return OwnerSerial.serial32; }

	inline void            	setOwnerSerial32Only(SI32 newserial)
	{ OwnerSerial.serial32 = newserial; }

	void            	setOwnerSerial32(SI32 newserial, bool force=false );
	const void		setOwnerSerialByte(UI32 nByte, BYTE value);

	inline void		setSameOwnerAs(const cObject* obj)
	{ setOwnerSerial32Only(obj->getOwnerSerial32()); }
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
	//! return the position of the object
	inline const Location	getPosition() const
	{ return position; }

	SI32			getPosition(const char *what) const;
	void			setPosition(const char *what, SI32 value);
	void			setPosition(Location where);

	//! Set the position of the object
	inline void		setPosition(UI32 x, UI32 y, SI08 z)
	{ setPosition( Loc( x, y, z ) ); }

	inline const Location	getOldPosition() const
	{ return old_position; }

	SI32			getOldPosition(const char *what) const;
	void			setOldPosition(const char *what, SI32 value);

	inline void		setOldPosition(const Location where)
	{ old_position = where; }

	inline void		setOldPosition(SI32 x, SI32 y, signed char z, signed char dispz)
	{ setOldPosition( Loc(x, y, z, dispz) ); }
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
	//! return the real name of object
	inline const string 	getRealName() const
	{ return secondary_name; }

	//! return the real name of object
	inline const char*		getRealNameC() const
	{ return secondary_name.c_str(); }

	//! Set the real name of object
	inline void		setRealName(string s)
	{ secondary_name = s; }

	//! Set the real name of object
	inline void		setRealName(const char *str)
	{ secondary_name = string(str); }

	//! return the current name of object
	inline const string	getCurrentName() const
	{ return current_name;  }

	//! return the current name of object
	inline const char*	getCurrentNameC() const
	{ return current_name.c_str(); }

	//! Set the current name of object
	inline void		setCurrentName(string s)
	{ current_name = s; }

	//! Set the current name of object
	inline void		setCurrentName(const char *str)
	{ current_name = string(str); }

	void			setCurrentName(char *format, ...);

	//! Set the secondary name of object
	inline void		setSecondaryName( string s )
	{ secondary_name = s; }

	//! Get the secondary name of the object
	inline const char*	getSecondaryNameC() const
	{ return secondary_name.c_str(); }

	//! Get the secondary name of the object
	inline const string	getSecondaryName() const
	{ return secondary_name; }

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
	LOGICAL			addTempfx( cObject& src, SI32 num, SI32 more1 = 0, SI32 more2 = 0, SI32 more3 = 0, SI32 dur = 0, SI32 amxcback = INVALID );
	void			delTempfx( SI32 num, LOGICAL executeExpireCode = true, SERIAL funcidx = INVALID );
	void			checkTempfx();
	void			tempfxOn();
	void			tempfxOff();
	LOGICAL			hasTempfx();
	tempfx::cTempfx*	getTempfx( SI32 num, SERIAL funcidx = INVALID );
//@}

	//! return the object's script number
	inline const UI32	getScriptID() const
	{ return ScriptID; }

	//! set the object's script number
	inline void		setScriptID(UI32 sid)
	{ ScriptID = sid; }

	UI32	disabled;	//!< Disabled object timer, cant trigger.
	std::string*	disabledmsg; //!< Object is disabled, so display this message.


public:
	virtual	void		Delete();

private:
	UI16 id_old;
	UI16 id;
	COLOR color;
	COLOR color_old;
public:
	inline void setId( UI16 newId )
	{ id = newId; }

	inline const UI16 getId() const
	{ return id; }

	inline void setOldId( UI16 oldId )
	{ id_old = oldId; }

	inline const UI16 getOldId() const
	{ return id_old; }

	inline void setColor( COLOR newColor )
	{ color = newColor; }

	inline const COLOR getColor() const
	{ return color; }

	inline void setOldColor( COLOR oldColor )
	{ color_old = oldColor; }

	inline const COLOR getOldColor() const
	{ return this->color_old; }

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

