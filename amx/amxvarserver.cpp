  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "amxvarserver.h"
#include "scp_parser.h"

AMXVARSRV_DATATYPE amxVariable::getType()
{
	return AMXVARSRV_UNDEFINED;
}

SI32 amxVariable::getSize( const SI32 index )
{
	return 0;
}
//
// amxIntegerVariable
//
amxIntegerVariable::amxIntegerVariable( const SI32 initialValue )
{
	value = initialValue;
}

amxIntegerVariable::~amxIntegerVariable()
{
}

AMXVARSRV_DATATYPE	amxIntegerVariable::getType()
{
	return AMXVARSRV_INTEGER;
}

SI32	amxIntegerVariable::getValue()
{
	return value;
}

void	amxIntegerVariable::setValue( const SI32 newValue )
{
	value = newValue;
}

SI32 amxIntegerVariable::getSize()
{
	return sizeof( value );
}

//
// amxIntegerVector
//
amxIntegerVector::amxIntegerVector( const SI32 size, const SI32 initialValue )
{
	value.resize( size, initialValue );
}

amxIntegerVector::~amxIntegerVector()
{
	//
	//	Sparhawk: Dumb gcc 3.3 bug, bogus statement else internal compiler error
	//
	value.clear();
}

AMXVARSRV_DATATYPE amxIntegerVector::getType()
{
	return AMXVARSRV_INTEGERVECTOR;
}

SI32 amxIntegerVector::getValue( const SERIAL index )
{
	if( (UI32)index >= value.size() )
		return -1;
	return value[ index ];
}

void amxIntegerVector::setValue( const SERIAL index, const SI32 newValue )
{
	if( (UI32)index >= value.size() )
		return ;
	value[ index ] = newValue;
}

SI32 amxIntegerVector::getSize( const SI32 index )
{
	if( index == INVALID )
		return value.size();
	else
		return sizeof( SI32 );
}

//
// amxStringVariable
//
amxStringVariable::amxStringVariable( const std::string& initialValue )
{
	value = initialValue;
}

amxStringVariable::~amxStringVariable()
{
}

AMXVARSRV_DATATYPE	amxStringVariable::getType()
{
	return AMXVARSRV_STRING;
}

std::string	amxStringVariable::getValue()
{
	return value;
}

void	amxStringVariable::setValue( const std::string& newValue )
{
	value = newValue;
}

SI32 amxStringVariable::getSize()
{
	return value.size();
}

//@{
/*!
\name AmxScriptId
\brief AmxScriptId variable
*/


/*
\brief
\author Endymion
\todo write it
*/
amxScriptIdVariable::amxScriptIdVariable( char* initialValue )
{
// Akron: this line doesn't compile under serious c++ compilers...
//	amxScriptIdVariable( xss::getIntFromDefine( initialValue ) );
}

/*
\brief
\author Endymion
*/
amxScriptIdVariable::amxScriptIdVariable( SERIAL initialValue )
{
	value=initialValue;
}

/*
\brief
\author Endymion
*/
amxScriptIdVariable::~amxScriptIdVariable()
{
}

/*
\brief
\author Endymion
*/
AMXVARSRV_DATATYPE amxScriptIdVariable::getType()
{
	return AMXVARSRV_SCRIPTID;
}

/*
\brief
\author Endymion
*/
SERIAL amxScriptIdVariable::getValue()
{
	return value;
}

/*
\brief
\author Endymion
*/
void amxScriptIdVariable::setValue( char* newValue )
{
	setValue( xss::getIntFromDefine( newValue ) );
}

/*
\brief
\author Endymion
*/
void amxScriptIdVariable::setValue( SERIAL newValue )
{
	value=newValue;
}

/*
\brief
\author Endymion
*/
SI32 amxScriptIdVariable::getSize()
{
	return sizeof( value );
}

//@}


//
//	amxVariableServer
//
amxVariableServer::amxVariableServer()
{
	setUserMode();
	error = AMXVARSRV_OK;
}

amxVariableServer::~amxVariableServer()
{
}

SI32 amxVariableServer::getError()
{
	return error;
}

LOGICAL amxVariableServer::inUserMode()
{
	return mode;
}

LOGICAL amxVariableServer::inServerMode()
{
	return !mode;
}

void	amxVariableServer::setUserMode()
{
	mode = true;
}

void amxVariableServer::setServerMode()
{
	mode = false;
}

SI32 amxVariableServer::firstVariable( const SERIAL serial )
{
	
	amxObjectVariableMapIterator object = varMap.find( serial );

	if( object != varMap.end() )
		if( !object->second.empty() )
			return object->second.begin()->first;
	return -1;
}

SI32 amxVariableServer::nextVariable( const SERIAL serial, const SI32 previous )
{
	amxObjectVariableMapIterator object = varMap.find( serial );

	if( object != varMap.end() )
	{
		amxVariableMapIterator variable = object->second.find( previous );
		if( ++variable != object->second.end() )
			return variable->first;
	}
	return -1;
}

AMXVARSRV_DATATYPE amxVariableServer::typeOfVariable( const SERIAL serial, const SI32 variable )
{
	amxObjectVariableMapIterator object = varMap.find( serial );

	if( object != varMap.end() )
	{
		amxVariableMapIterator subject = object->second.find( variable );
		return subject->second->getType();
	}
	return AMXVARSRV_UNDEFINED;
}

LOGICAL amxVariableServer::insertVariable( const SERIAL serial, const SI32 variable, const SI32 value )
{
	if ( variable < 1000 && inUserMode() )
	{
			error = AMXVARSRV_ACCESS_DENIED;
			return false;
	}

	if( !existsVariable( serial, variable, 0 ) )
	{
		varMap[ serial ][ variable ] = new amxIntegerVariable( value );
		error = AMXVARSRV_OK;
		return true;
	}
	error = AMXVARSRV_DUPLICATE_VAR;
	return false;
}

LOGICAL amxVariableServer::insertVariable( const SERIAL serial, const SI32 variable, const std::string& value )
{
	if ( variable < 1000 && inUserMode() )
	{
			error = AMXVARSRV_ACCESS_DENIED;
			return false;
	}

	if( !existsVariable( serial, variable, 0 ) && variable >= 1000 )
	{
		varMap[ serial ][ variable ] = new amxStringVariable( value );
		error = AMXVARSRV_OK;
		return true;
	}
	error = AMXVARSRV_DUPLICATE_VAR;
	return false;
}

LOGICAL amxVariableServer::insertVariable( const SERIAL serial, const SI32 variable, const SI32 size, const SI32 value )
{
	if ( variable < 1000 && inUserMode() )
	{
			error = AMXVARSRV_ACCESS_DENIED;
			return false;
	}

	if( !existsVariable( serial, variable, 0 ) )
	{
		varMap[ serial ][ variable ] = new amxIntegerVector( size, value );
		error = AMXVARSRV_OK;
		return true;
	}
	error = AMXVARSRV_DUPLICATE_VAR;
	return false;
}

LOGICAL amxVariableServer::deleteVariable( const SERIAL serial, const SI32 variable )
{
	if ( variable < 1000 && inUserMode() )
	{
			error = AMXVARSRV_ACCESS_DENIED;
			return false;
	}

	if( existsVariable( serial, variable, 0 ) && variable >= 1000 )
	{
		varMap[ serial ].erase( variable );
		error = AMXVARSRV_OK;
		return true;
	}
	error = AMXVARSRV_UNKNOWN_VAR;
	return false;
}

LOGICAL amxVariableServer::deleteVariable( const SERIAL serial )
{
	error = error = AMXVARSRV_OK;
	return varMap.erase( serial );
}

LOGICAL amxVariableServer::updateVariable( const SERIAL serial, const SI32 variable, const SI32 value )
{
	if( existsVariable( serial, variable, AMXVARSRV_INTEGER ) )
	{
		static_cast<amxIntegerVariable*>(varMap[serial][variable])->setValue( value );
		error = AMXVARSRV_OK;
		return true;
	}
	else
		if ( variable <= 16 )
		{
			setServerMode();
			insertVariable( serial, variable, value );
			setUserMode();
			error = AMXVARSRV_OK;
			return true;
		}
	error = AMXVARSRV_UNKNOWN_VAR;
	return false;
}

LOGICAL amxVariableServer::updateVariable( const SERIAL serial, const SI32 variable, const std::string& value )
{
	if( existsVariable( serial, variable, AMXVARSRV_STRING ) )
	{
		static_cast<amxStringVariable*>(varMap[serial][variable])->setValue( value );
		error = AMXVARSRV_OK;
		return true;
	}
	error = AMXVARSRV_UNKNOWN_VAR;
	return false;
}

LOGICAL	amxVariableServer::updateVariable( const SERIAL serial, const SI32 variable, const SI32 index, const SI32 value )
{
	if( existsVariable( serial, variable, AMXVARSRV_INTEGERVECTOR ) )
	{
		static_cast<amxIntegerVector*>(varMap[serial][variable])->setValue( index, value );
		error = AMXVARSRV_OK;
		return true;
	}
	error = AMXVARSRV_UNKNOWN_VAR;
	return false;
}

LOGICAL	amxVariableServer::selectVariable( const SERIAL serial, const SI32 variable, const SI32 index, SI32& value )
{
	if( existsVariable( serial, variable, AMXVARSRV_INTEGERVECTOR ) )
	{
		value = static_cast<amxIntegerVector*>(varMap[serial][variable])->getValue( index );
		error = AMXVARSRV_OK;
		return true;
	}
	error = AMXVARSRV_UNKNOWN_VAR;
	return false;
}

LOGICAL amxVariableServer::selectVariable( const SERIAL serial, const SI32 variable, SI32& value )
{
	if( existsVariable( serial, variable, AMXVARSRV_INTEGER ) )
	{
		value = static_cast<amxIntegerVariable*>(varMap[serial][variable])->getValue();
		error = AMXVARSRV_OK;
		return true;
	}
	else
		if( variable < 16 )
		{
			value = 0;
			error = AMXVARSRV_OK;
			return true;
		}
	error = AMXVARSRV_UNKNOWN_VAR;
	return false;
}

LOGICAL amxVariableServer::selectVariable( const SERIAL serial, const SI32 variable, std::string& value )
{
	if( existsVariable( serial, variable, AMXVARSRV_STRING ) )
	{
		value = static_cast<amxStringVariable*>(varMap[serial][variable])->getValue();
		error = AMXVARSRV_OK;
		return true;
	}

	error = AMXVARSRV_UNKNOWN_VAR;
	return false;
}

LOGICAL amxVariableServer::existsVariable( const SERIAL serial, const SI32 variable, const SI32 type )
{
	amxObjectVariableMapIterator ovmIt( varMap.find( serial ) );
	if( ovmIt == varMap.end() )
	{
/*		if ( variable < 16 )
			if( type == AMXVARSRV_UNDEFINED || type == AMXVARSRV_INTEGER )
			{
				error = AMXVARSRV_OK;
				return true;
			}
			else
				error = AMXVARSRV_WRONG_TYPE;
		else
			error = AMXVARSRV_UNKNOWN_VAR;
*/
		error = AMXVARSRV_UNKNOWN_VAR;
		return false;
	}
	amxVariableMapIterator vmIt( ovmIt->second.find( variable ) );
	if( vmIt == ovmIt->second.end() )
	{
		error = AMXVARSRV_UNKNOWN_VAR;
		return false;
	}
	if( vmIt->second->getType() == type || type == 0 )
	{
		error = AMXVARSRV_OK;
		return true;
	}
	error = AMXVARSRV_WRONG_TYPE;
	return false;
}

SI32 amxVariableServer::countVariable()
{
	amxObjectVariableMapIterator ovmItBegin( varMap.begin() ), ovmItEnd( varMap.end() );
	SI32 count = 0;
	while( ovmItBegin != ovmItEnd )
	{
		count += ovmItBegin->second.size();
		++ovmItBegin;
	}
	return count;
}

SI32 amxVariableServer::countVariable( const SERIAL serial )
{
	amxObjectVariableMapIterator ovmItBegin( varMap.find( serial ) ), ovmItEnd( varMap.end() );
	if( ovmItBegin != ovmItEnd )
		return ovmItBegin->second.size();
	return 0;
}

SI32 amxVariableServer::countVariable( const SERIAL serial, const SERIAL type )
{
	SI32 count = 0;
	amxObjectVariableMapIterator ovmItBegin( varMap.begin() ), ovmItEnd( varMap.end() );
	if( ovmItBegin != ovmItEnd )
	{
		amxVariableMapIterator vmItBegin( ovmItBegin->second.begin() ), vmItEnd( ovmItBegin->second.end() );
		while( vmItBegin != vmItEnd )
		{
			if( type == 0 || vmItBegin->second->getType() == type )
				++count;
			++vmItBegin;
		}
	}
	return count;
}

LOGICAL amxVariableServer::moveVariable( const SERIAL fromSerial, const SERIAL toSerial )
{
	if( copyVariable( fromSerial, toSerial ) )
	{
		deleteVariable( fromSerial );
		return true;
	}
	return false;
}

LOGICAL amxVariableServer::copyVariable( const SERIAL fromSerial, const SERIAL toSerial )
{
	amxObjectVariableMapIterator ovmIt( varMap.find( fromSerial ) );
	if( ovmIt == varMap.end() )
		return false;
	deleteVariable( toSerial );
	varMap[ toSerial ] = varMap[ fromSerial ];
	return true;
}

SI32	amxVariableServer::size( const SERIAL serial, const SI32 variable, const SI32 index )
{
	if( existsVariable( serial, variable, AMXVARSRV_UNDEFINED ) )
	{
		//value = static_cast<amxStringVariable*>(varMap[serial][variable])->getValue();
		error = AMXVARSRV_OK;
		return varMap[serial][variable]->getSize( index );
	}

	error = AMXVARSRV_UNKNOWN_VAR;
	return INVALID;
}

void amxVariableServer::saveVariable( SERIAL serial, FILE * stream )
{
	amxObjectVariableMapIterator ovmItBegin( varMap.find( serial ) ), ovmItEnd( varMap.end() );
	if( ovmItBegin != ovmItEnd )
	{
		amxVariableMapIterator vmItBegin( ovmItBegin->second.begin() ), vmItEnd( ovmItBegin->second.end() );
		while( vmItBegin != vmItEnd )
		{
			switch( vmItBegin->second->getType() )
			{
				case AMXVARSRV_UNDEFINED:
					break;
				case AMXVARSRV_INTEGER	:
					if( !(vmItBegin->first < 16 && static_cast<amxIntegerVariable*>(vmItBegin->second)->getValue() == 0) )
						fprintf( stream, "AMXINT %d %d\n", vmItBegin->first, static_cast<amxIntegerVariable*>(vmItBegin->second)->getValue() );
					break;
				case AMXVARSRV_LOGICAL	:
					break;
				case AMXVARSRV_STRING		:
					fprintf( stream, "AMXSTR %d %s\n", vmItBegin->first, static_cast<amxStringVariable*>(vmItBegin->second)->getValue().c_str() );
					break;
				case AMXVARSRV_INTEGERVECTOR	:
					{
					SI32 vectorSize = static_cast<amxIntegerVector*>(vmItBegin->second)->getSize();
					fprintf( stream, "AMXINTVEC %d %d\n{\n", vmItBegin->first, vectorSize );
					for( SI32 vectorIndex = 0; vectorIndex < vectorSize; ++vectorIndex )
						fprintf( stream, "%d\n", static_cast<amxIntegerVector*>(vmItBegin->second)->getValue( vectorIndex ) );
					fprintf( stream, "}\n" );
					}
					break;
			}
			++vmItBegin;
		}
	}
}
