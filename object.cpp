  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "object.h"


cObject::cObject()
{
	setCurrentName("#");
	setSecondaryName("#");
	serial.serial32 = INVALID;
	multi_serial.serial32 = INVALID;
	OwnerSerial.serial32 = INVALID;
	old_position = Loc(0,0,0);
	position = Loc(0,0,0);
	ScriptID = 0;
	
	for (UI32 i = 0; i < tempfx::MAX_TEMPFX_INDEX; i++)
		tempfx[i] = false;
}

/*
			Operators definitions
*/


int operator ==(Location a, Location b)
{
	return ((a.x==b.x) && (a.y==b.y) && (a.z==b.z));
}

int operator !=(Location a, Location b)
{
	return ((a.x!=b.x) || (a.y!=b.y) || (a.z!=b.z));
}


bool cObject::operator> (cObject &obj){ return(getSerial32() >  obj.getSerial32()); }
bool cObject::operator< (cObject &obj){ return(getSerial32() <  obj.getSerial32()); }
bool cObject::operator>=(cObject &obj){ return(getSerial32() >= obj.getSerial32()); }
bool cObject::operator<=(cObject &obj){ return(getSerial32() <= obj.getSerial32()); }
bool cObject::operator==(cObject &obj){ return(getSerial32() == obj.getSerial32()); }
bool cObject::operator!=(cObject &obj){ return(getSerial32() != obj.getSerial32()); }


LOGICAL cObject::getTempfx( SI32 arg )
{
	if (arg < 0 || arg > tempfx::MAX_TEMPFX_INDEX)
		return false;
	
	return tempfx[arg];
}

void cObject::setTempfx( SI32 arg )
{
	if (arg < 0 || arg > tempfx::MAX_TEMPFX_INDEX)
		return;

	tempfx[arg] = true;
}

void cObject::resetTempfx( SI32 arg )
{
	if (arg < 0 || arg > tempfx::MAX_TEMPFX_INDEX)
		return;
		
	tempfx[arg] = false;
}


/*!
\brief return the object's script number
\author Anthalir
\return UI32
\since 0.82a
*/
UI32 cObject::getScriptID()
{
	return ScriptID;
}

/*!
\brief set the object's script number
\author Anthalir
\param sid the new serial
\since 0.82a
*/
void cObject::setScriptID(UI32 sid)
{
	ScriptID= sid;
}

/*!
\brief return the serial of the object
\author Anthalir
\return unsigned int
\since 0.82a
*/
const SI32 cObject::getSerial32() const
{
	return serial.serial32;
}

/*!
\brief return the object's serial
\author Anthalir
\return Serial structure
\since 0.82a
*/
const Serial cObject::getSerial() const
{
	return serial;
}

/*!
\brief set one byte of the object's serial
\author Sparhawk
\return void
\since 0.82a
\param nByte byte number to get
\param value to set serial byte to
\remarks nByte start at \b 1 not \b 0 and end at 4
*/
const void cObject::setSerialByte(UI32 nByte, BYTE value)
{
	switch(nByte)
	{
	case 1: serial.ser1 = value; break;
	case 2: serial.ser2 = value; break;
	case 3: serial.ser3 = value; break;
	case 4: serial.ser4 = value; break;

	default:
		WarnOut("cannot access byte %i of serial !!", nByte);
		break;
	}
}


void cObject::setSameOwnerAs(const cObject* obj)
{
	setOwnerSerial32Only(obj->getOwnerSerial32());
}

/*!
\brief set one byte of the object's owner serial
\author Sparhawk
\return void
\since 0.82a
\param nByte byte number to get
\param value to set serial byte to
\remarks nByte start at \b 1 not \b 0 and end at 4
*/
const void cObject::setOwnerSerialByte(UI32 nByte, BYTE value)
{
	switch(nByte)
	{
	case 1:
		OwnerSerial.ser1 = value;
		break;
	case 2:
		OwnerSerial.ser2 = value;
		break;
	case 3:
		OwnerSerial.ser3 = value;
		break;
	case 4:
		OwnerSerial.ser4 = value;
		break;
	default:
		WarnOut("cannot access byte %i of serial !!", nByte);
		break;
	}
}

/*!
\brief return the multi serial of the object
\author Anthalir
\return unsigned int
\since 0.82a
\remarks What is the multi serial used for ??? don't know

*/
const SI32 cObject::getMultiSerial32() const
{
	return multi_serial.serial32;
}

/*!
\brief return the object's multi serial
\author Anthalir
\return Serial structure
\since 0.82a
*/
const Serial cObject::getMultiSerial() const
{
	return multi_serial;
}

/*!
\brief set one byte of the object's multi serial
\author Sparhawk
\return void
\since 0.82a
\param nByte byte number to get
\param value to set serial byte to
\remarks nByte start at \b 1 not \b 0 and end at 4
*/
const void cObject::setMultiSerialByte(UI32 nByte, BYTE value)
{
	switch(nByte)
	{
	case 1:
		multi_serial.ser1 = value;
		break;
	case 2:
		multi_serial.ser2 = value;
		break;
	case 3:
		multi_serial.ser3 = value;
		break;
	case 4:
		multi_serial.ser4 = value;
		break;
	default:
		WarnOut("cannot access byte %i of serial !!", nByte);
		break;
	}
}
/*!
\brief Set the serial of the object
\author Anthalir
\since 0.82a
\param newserial the new serial (unsigned int)
*/
void cObject::setSerial32(SI32 newserial)
{
	serial.serial32= newserial;
	if( newserial!=INVALID )
		objects.insertObject( this );
}

/*!
\brief Set the multi serial of the object
\author Anthalir
\since 0.82a
\param newserial the new serial (unsigned int)
*/
void cObject::setMultiSerial32Only(SI32 newserial)
{
	multi_serial.serial32= newserial;
}

/*!
\brief return the object's owner serial
\author Anthalir
\return Serial structure
\since 0.82a
*/
const Serial cObject::getOwnerSerial() const
{
	return OwnerSerial;
}

/*!
\brief return the object's owner serial
\author Anthalir
\return SI32
\since 0.82a
*/
const SI32 cObject::getOwnerSerial32() const
{
    return OwnerSerial.serial32;
}

///////////////////////
// Name:	setters for various serials
// history: by Duke, 2.6.2001
// Purpose: encapsulates revoval/adding to the pointer arrays
//
void cObject::setOwnerSerial32Only(SI32 ownser)
{
	OwnerSerial.serial32= ownser;
}

void cObject::setOwnerSerial32(SI32 ownser, bool force)
{

  //Endymion
  //i think need to have only in cObejct the setOwnerOnly and have SetOwnerSerial in cItem and cChar

	if( !force )
		if ( ownser == getOwnerSerial32() ) return;

	if ( getOwnerSerial32() != INVALID ) // if it was set, remove the old one
		if ( isCharSerial( getSerial32() ) )
			pointers::delFromOwnerMap( (P_CHAR)( this ) );
		else
			pointers::delFromOwnerMap( (P_ITEM)( this ) );


	setOwnerSerial32Only(ownser);

	if ( getOwnerSerial32() == INVALID ) {
		if( isCharSerial( getSerial32() ) )
			((P_CHAR)(this))->tamed = false;
		return;
	}

	if ( isCharSerial( getSerial32() ) ) {
		if ( getOwnerSerial32() != getSerial32() )
			((P_CHAR)(this))->tamed = true;
		else
			((P_CHAR)(this))->tamed = false;
	}

	if ( isCharSerial( getSerial32() ) ) // if there is an owner, add it
		pointers::addToOwnerMap( (P_CHAR)( this ) );
	else
		pointers::addToOwnerMap( (P_ITEM)( this ) );
	//End Endymion..
}

/*!
\brief return the position of the object
\author Anthalir
\since 0.82a
\return Location structure containing the current object position
*/
Location cObject::getPosition() const
{
	return position;
}

/*!
\brief return one coord of the object position
\author Anthalir
\since 0.82a
\param what what to return ?	\li "x" = return the x position
								\li "y" = return the y position
								\li "z" = return the z position
								\li "dz"= return the dispz position (used in cChar)
\return signed int
*/
SI32 cObject::getPosition( const char *what ) const
{

	switch( what[0] )
	{
	case 'x':
	case 'X':
		return position.x;

	case 'y':
	case 'Y':
		return position.y;

	case 'z':
	case 'Z':
		return position.z;

	case 'd':
	case 'D':
		if( (what[1] == 'z') || (what[1] == 'Z') )
			return position.dispz;
	}

	WarnOut("getPosition called with wrong parameter: '%s' !!", what);
	return -1;


}

/*!
\brief Set the position of the object
\author Anthalir
\since 0.82a
\param where Location structure representing the new position
*/
void cObject::setPosition(Location where)
{
	position.x= where.x;
	position.y= where.y;
	position.z= where.z;
	position.dispz= where.dispz;
}

/*!
\brief Set the position of the object
\author Anthalir
\since 0.82a
\param x,y,z new coords of the object
*/
void cObject::setPosition(UI32 x, UI32 y, SI08 z)
{
	position.x= x;
	position.y= y;
	position.z= z;
}

/*!
\brief Set one coord of the object position
\author Anthalir
\since 0.82a
\param what what to set ?		\li "x" = set the x position
								\li "y" = set the y position
								\li "z" = set the z position
								\li "dz"= set the dispz position (used in cChar)
*/
void cObject::setPosition( const char *what, SI32 value)
{
	switch( what[0] )
	{
	case 'x':
	case 'X':
		position.x= value;
		break;

	case 'y':
	case 'Y':
		position.y= value;
		break;

	case 'z':
	case 'Z':
		position.z= value;
		break;

	case 'd':
	case 'D':
		if( (what[1] == 'z') || (what[1] == 'Z') )
			position.dispz= value;
		break;
	}
}
/*
void cObject::MoveTo(SI32 x, SI32 y, SI08 z)
{
	MoveTo( Loc(x, y, z) );
}


void cObject::MoveTo(Location newloc)
{

	int n;

	// Avoid crash if go to 0,0
	if (newloc.x < 1 || newloc.y < 1)
		return;

	if( (Type!="char") && (Type!="item") ) ConOut("Error in MoveTo, cObject has invalid type: '%s'\n", Type.c_str());

	if( Type=="char" )
	{
		n= DEREF_P_CHAR(this);
		mapRegions->RemoveItem(n + CharacterOffset);
		setPosition( newloc );
		mapRegions->AddItem(n + CharacterOffset);
	}

	if( Type=="item" )
	{
		n= DEREF_P_ITEM(this);
		mapRegions->RemoveItem(n);
		setPosition( newloc );
		mapRegions->AddItem(n);
	}


}
*/


Location cObject::getOldPosition() const
{
	return old_position;
}

SI32 cObject::getOldPosition( const char *what) const
{
	switch( what[0] )
	{
	case 'x':
	case 'X':
		return old_position.x;

	case 'y':
	case 'Y':
		return old_position.y;

	case 'z':
	case 'Z':
		return old_position.z;
	}

	WarnOut("getPosition called with wrong parameter: '%s' !!", what);
	return -1;
}

void cObject::setOldPosition( const char *what, SI32 value)
{
	switch( what[0] )
	{
	case 'x':
	case 'X':
		old_position.x= value;
		break;

	case 'y':
	case 'Y':
		old_position.y= value;
		break;

	case 'z':
	case 'Z':
		old_position.z= value;
		break;
	}
}

void cObject::setOldPosition(SI32 x, SI32 y, signed char z, signed char dispz)
{
	setOldPosition( Loc(x, y, z, dispz) );
}

void cObject::setOldPosition(Location where)
{
	old_position= where;
}

/*!
\brief return the real name of object
\author Anthalir
\since 0.82a
\return C++ string object
*/
string cObject::getRealName() const
{
	return string(secondary_name);
}


/*!
\brief return the real name of object
\author Anthalir
\since 0.82a
\return C char pointer
\attention the pointer you get is not the pointer to the real data, don't use it to set the name !!
*/
const char* cObject::getRealNameC() const
{
	return secondary_name;
}

/*!
\brief Set the real name of object
\author Anthalir
\since 0.82a
\param s C++ string
*/
void cObject::setRealName(string s)
{
	strcpy(secondary_name, s.c_str());
}


/*!
\brief Set the real name of object
\author Anthalir
\since 0.82a
\param str C char pointer
*/
void cObject::setRealName( const char *str )
{
	strncpy(secondary_name, str, sizeof(secondary_name));
	secondary_name[sizeof(secondary_name)-1]= '\0';
}

/*!
\brief return the current name of object
\author Anthalir
\since 0.82a
\return C++ string
*/
string cObject::getCurrentName() const
{
	return string(current_name);
}


/*!
\brief return the current name of object
\author Anthalir
\since 0.82a
\return C char pointer
\attention the pointer you get is not the pointer to the real data, don't use it to set the name !!
*/
const char* cObject::getCurrentNameC() const
{
	return current_name;
}

/*!
\brief Set the current name of object
\author Anthalir
\since 0.82a
\param s C++ string, the new name
*/
void cObject::setCurrentName( string s )
{
	strcpy(current_name, s.c_str());
}

/*!
\brief Set the current name of object
\author Anthalir
\since 0.82a
\param format,... formatted C string
*/
void cObject::setCurrentName( char *format, ... )
{
        va_list vargs;
        va_start(vargs, format);
        vsnprintf(current_name, sizeof(current_name)-1, format, vargs);
        va_end(vargs);

        current_name[sizeof(current_name)-1] = '\0';
}

/*!
\brief Set the current name of object
\author Anthalir
\since 0.82a
\param str C char pointer, the new name
*/
void cObject::setCurrentName( const char *str )
{
	strncpy(current_name, str, sizeof(current_name));
	current_name[sizeof(current_name)-1]= '\0';
}

/*!
\brief Set the secondary name of the object
\author Anthalir
\since 0.82a
\param format C char pointer - see printf
\warning This function must only be used by items because it use same var than real name
*/
void cObject::setSecondaryName(const char *format, ...)
{
        va_list vargs;
        va_start(vargs, format);
        vsnprintf(secondary_name, sizeof(secondary_name)-1, format, vargs);
        va_end(vargs);

        secondary_name[sizeof(secondary_name)-1] = '\0';
}

/*!
\brief Get the secondary name of the object
\author Anthalir
\since 0.82a
\return C char pointer
\warning This function must only be used by items because it use same var than real name of chars
*/
const char *cObject::getSecondaryNameC() const
{
	return secondary_name;
}

/*!
\brief Get the secondary name of the object
\author Anthalir
\since 0.82a
\return C char pointer
\warning This function must only be used by items because it use same var than real name of chars
*/
string cObject::getSecondaryName() const
{
	return string(secondary_name);
}

