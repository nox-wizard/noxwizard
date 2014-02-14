  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"

#include "racetokenizer.h"

Tokenizer::Tokenizer( const char* initialLine )
{
/*
	line = initialLine;
	tokenStart = 0;
	tokenLength = 0;
*/
	UI32	currentTokenId	= 1;
	UI32	tokenStart	= 0;
	UI32	tokenLength	= 0;
	string	line		= initialLine;
	UI32	lineSize 	= line.size();
	string* token		= 0;

	while ( tokenStart != lineSize )
	{
		while ( tokenStart < lineSize && isspace( line[tokenStart] ) )
			tokenStart++;
		while ( tokenStart + tokenLength < lineSize && !isspace( line[tokenStart+tokenLength] ))
			tokenLength++;

		if ( tokenStart != lineSize )
		{
			token = new string( line.substr( tokenStart, tokenLength ) );
			tokens[currentTokenId] = token;
			++currentTokenId;
		}
		tokenStart	+= tokenLength;
		tokenLength	= 0;
	}
	index = 1;
}

UI32 Tokenizer::count( void )
{
	return tokens.size();
}

bool Tokenizer::setIndex( UI32 newValue )
{
	bool returnValue = false;
	if ( newValue > 1 && newValue <= tokens.size() )
	{
		returnValue = true;
		this->index = newValue;
	}
	return returnValue;
}

string* Tokenizer::first( void )
{
	string* returnValue;

	if ( this->count() )
	{
		this->index = 1;
		returnValue = tokens[this->index];
	}
	else
		returnValue = new string( "" );

	return returnValue;
}

string* Tokenizer::next( void )
{
	string* returnValue;

	if ( this->index < this->count() )
	{
		++this->index;
		returnValue = tokens[this->index];
	}
	else
		returnValue = new string( "" );

	return returnValue;
}

string* Tokenizer::previous( void )
{
	string* returnValue;

	if ( this->index > 1 )
	{
		--this->index;
		returnValue = tokens[this->index];
	}
	else
		returnValue = new string( "" );

	return returnValue;
}

string* Tokenizer::current( void )
{
	string* returnValue;

	if ( this->index > 1 && this->index <= tokens.size() )
		returnValue = tokens[this->index];
	else
		returnValue = new string( "" );

	return returnValue;
}

