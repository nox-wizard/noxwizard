  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"

SERIAL cPoll::getSerial()
{
	return serial;
}

void cPoll::setSerial( const SERIAL newSerial )
{
	serial = newSerial;
}

std::string cPoll::getQuestion()
{
	return question;
}

void cPoll::setQuestion( const std::string &newQuestion )
{
	question = newQuestion;
}

UI32 cPoll::getRunForDays()
{
	return runForDays;
}

void cPoll::setRunForDays( UI32 days )
{
	runForDays = days;
}

std::string cPoll::getPollOption( const UI32 option )
{
	if( !pollOptions.empty() )
	{
		miPollOption pOption( pollOptions.begin() );

		if( pOption != pollOptions.end() )
			return pOption->second;
	}
	return "";
}

void cPoll::setPollOption( const UI32 option, const std::string &optionText )
{
	pollOptions[ option ] = optionText;
}

LOGICAL cPoll::castVote( const SERIAL voterSerial, const UI32 option )
{
	if( pollVoters.find( voterSerial ) != pollVoters.end() )
	{
		pollVoters[ voterSerial ] = cGameDate();
		return true;
	}
	return false;
}
