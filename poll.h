  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __CPOLL_
#define __CPOLL_


#include "gamedate.h"

class cPoll
{
	public:
				cPoll();
				~cPoll();
	private:
		SERIAL		serial;
	public:
		SERIAL		getSerial();
		void		setSerial( const SERIAL serial );
	private:
		std::string	question;
	public:
		std::string	getQuestion();
		void		setQuestion( const std::string &question );
	private:
		UI32		runForDays;
	public:
		UI32		getRunForDays();
		void		setRunForDays( const UI32 days );
	
	typedef map< UI32, std::string > mPollOption	;
	typedef mPollOption::iterator	 miPollOption	;
	
	private:
		mPollOption	pollOptions;
	public:
		std::string	getPollOption( const UI32 option );
		void		setPollOption( const UI32 option, const std::string &optionText );
	
	typedef map< SERIAL, cGameDate > mPollVoter;
	typedef mPollVoter::iterator miPollVoter;
	
	private:
		mPollVoter	pollVoters;
	public:
		LOGICAL		castVote( const SERIAL voterSerial, const UI32 option );
};

#endif

