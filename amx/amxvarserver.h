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
\brief AMX Var Server
\todo document it
*/

#ifndef __AMXVARSERVER_H__
#define __AMXVARSERVER_H__

#include "nxwcommn.h"

typedef enum
{
	AMXVARSRV_OK = 0,
	AMXVARSRV_UNKNOWN_VAR,
	AMXVARSRV_DUPLICATE_VAR,
	AMXVARSRV_WRONG_TYPE,
	AMXVARSRV_ACCESS_DENIED
} AMXVARSRV_ERROR;

typedef enum
{
	AMXVARSRV_UNDEFINED		= 0,
	AMXVARSRV_INTEGER		= 1,
	AMXVARSRV_LOGICAL		= 2,
	AMXVARSRV_STRING		= 3,
	AMXVARSRV_INTEGERVECTOR	= 4,
	AMXVARSRV_SCRIPTID		= 5
} AMXVARSRV_DATATYPE;

class amxVariable
{
	public:
		virtual				~amxVariable() {}
		virtual	AMXVARSRV_DATATYPE	getType();
		virtual SI32			getSize( const SI32 index = -1 );
};

class amxIntegerVariable : public amxVariable
{
	private:
		SI32				value;
	public:
						amxIntegerVariable( const SI32 initialValue = 0 );
						~amxIntegerVariable();
		AMXVARSRV_DATATYPE		getType();
		SI32				getValue();
		void				setValue( const SI32 newValue );
		SI32				getSize();
};

class amxIntegerVector : public amxVariable
{
	private:
		vector< SI32 >			value;
	public:
						amxIntegerVector( const SI32 size, const SI32 initialValue = 0 );
						~amxIntegerVector();
		AMXVARSRV_DATATYPE		getType();
		SI32				getValue( const SERIAL index );
		void				setValue( const SERIAL index, const SI32 newValue );
		SI32				getSize( const SI32 index = -1 );
};

class amxStringVariable : public amxVariable
{
	private:
		std::string			value;
	public:
						amxStringVariable( const std::string& initialValue = "");
						~amxStringVariable();
		AMXVARSRV_DATATYPE		getType();
		std::string			getValue();
		void				setValue( const std::string& newValue = "");
		SI32				getSize();
};

class amxScriptIdVariable : public amxVariable
{
	private:
		SERIAL	value; //!< script id serial ( is a number into core, string only on save and load )
	public:
						
		amxScriptIdVariable( char* initialValue );
		amxScriptIdVariable( SERIAL initialValue = INVALID );
		~amxScriptIdVariable();
	
		AMXVARSRV_DATATYPE getType();
		SERIAL getValue();
		void setValue( char* newValue );
		void setValue( SERIAL initialValue = INVALID );
		SI32 getSize();
};

typedef pair< SI32, amxVariable* >		amxVariablePair;
typedef map< SI32, amxVariable* > 		amxVariableMap;
typedef amxVariableMap::iterator		amxVariableMapIterator;

typedef pair< SI32, amxVariableMap >		amxObjectVariablePair;
typedef map< SI32, amxVariableMap >		amxObjectVariableMap;
typedef amxObjectVariableMap::iterator		amxObjectVariableMapIterator;

class amxVariableServer
{
	private:
		amxObjectVariableMap		varMap;
		SI32				error;
		LOGICAL				mode;
	public:
						amxVariableServer();
						~amxVariableServer();
		LOGICAL				inUserMode();
		LOGICAL				inServerMode();
		void				setUserMode();
		void				setServerMode();
		SI32				getError();
		SI32				firstVariable( const SERIAL serial );
		SI32				nextVariable( const SERIAL serial, const SI32 previous );
		AMXVARSRV_DATATYPE		typeOfVariable( const SERIAL serial, const SI32 variable );
		//
		//	Integer variable
		//
		LOGICAL				insertVariable( const SERIAL serial, const SI32 variable, const SI32 value );
		LOGICAL				updateVariable( const SERIAL serial, const SI32 variable, const SI32 value );
		LOGICAL				selectVariable( const SERIAL serial, const SI32 variable, SI32& value );
		//
		//	String variable
		//
		LOGICAL				insertVariable( const SERIAL serial, const SI32 variable, const std::string& value );
		LOGICAL				updateVariable( const SERIAL serial, const SI32 variable, const std::string& value );
		LOGICAL				selectVariable( const SERIAL serial, const SI32 variable, std::string& value );
		//
		//	Integer vectors
		//
		LOGICAL 			insertVariable( const SERIAL serial, const SI32 variable, const SI32 size, const SI32 value );
		LOGICAL				updateVariable( const SERIAL serial, const SI32 variable, const SI32 index, const SI32 value );
		LOGICAL				selectVariable( const SERIAL serial, const SI32 variable, const SI32 index, SI32& value );
		//
		//	Type aspecific methods
		//
		LOGICAL				deleteVariable( const SERIAL serial );
		LOGICAL				deleteVariable( const SERIAL serial, const SI32 variable );
		LOGICAL				existsVariable( const SERIAL serial, const SI32 variable, const SI32 type );
		SI32				countVariable();
		SI32				countVariable( const SERIAL serial );
		SI32				countVariable( const SERIAL serial, const SERIAL type );
		LOGICAL				copyVariable( const SERIAL fromSerial, const SERIAL toSerial );
		LOGICAL				moveVariable( const SERIAL fromSerial, const SERIAL toSerial );
		void				saveVariable( const SERIAL serial, FILE * stream );
		SI32				size( const SERIAL serial, const SI32 variable, const SI32 index = -1 );
};

extern amxVariableServer amxVS;
#endif
