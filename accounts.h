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
\brief Account Stuff
*/

#ifndef __ACCOUNTS_H__
#define __ACCOUNTS_H__

#include "nxwcommn.h"
#include "set.h"

#define ADMIN_ACCOUNT 0

// Authenticate return codes
enum AUTHENTICATE_RESULT {
	LOGIN_NOT_FOUND = -3,
	BAD_PASSWORD = -4,
	ACCOUNT_BANNED = -5,
	ACCOUNT_WIPE = -6,
};


typedef enum {
	LOG_OUT,
	LOG_ENTERING,
	LOG_INGAME,
} ACCOUNT_STATE;



/*!
\brief an Account
*/
class cAccount {
public:
	ACCOUNT number;			//!< Account number
	std::string name;		//!< Account name
	std::string pass;		//!< Account password
	bool ban;			//!< Is banned
	bool ras;			//!< Use Encription
	TIMERVAL tempblock; 		//!< Elcabesa tempblock
	TIMERVAL blockeduntil;		//!< Elcabesa tempblock
	unsigned long lastlogin;	//!< Last login time
	struct in_addr lastip;		//!< Last ip
	ACCOUNT_STATE state;		//!< Account state
	std::vector<SERIAL> pgs;	//!< list of pg of this account
	SERIAL pc_online;		//!< the online player

	cAccount( ACCOUNT num = INVALID );

	SERIAL getInWorld();
	void setEntering( );
	void setOnline( P_CHAR pc );
	void setOffline();
	void onLogin( NXWSOCKET socket );
	bool isOnline();
	void changePassword( std::string password);
	void getAllChars( NxwCharWrapper& sc );
	void addCharToAccount( P_CHAR pc );

};

typedef map<ACCOUNT, cAccount> ACCOUNT_LIST;
typedef map<std::string, ACCOUNT > ACCOUNT_LIST_BY_NAME;


/*!
\brief Class for Account Management
*/
class cAccounts
{
	
private:
	ACCOUNT_LIST acctlist;	//!< All Account, with info
	ACCOUNT_LIST_BY_NAME accbyname;	//!< All Account by name

	UI32 lastusedacctnum, unsavedaccounts;

public:
	UI32 lasttimecheck;

	cAccounts( void );
	~cAccounts( void );

private:
	UI32 saveratio;
	void safeInsert( cAccount& acc );
	void LoadAccount ( ACCOUNT acctnumb, FILE* F );

public:
	void SetSaveRatio ( int );
	void LoadAccounts();
	void SaveAccounts();
	void CheckAccountFile();
	
	int Count();

	SI32 Authenticate(std::string username, std::string password);
	bool AuthenticateRAS(std::string username, std::string password);
	ACCOUNT CreateAccount(std::string username, std::string password);

	bool IsOnline( ACCOUNT acctnum );
	SERIAL GetInWorld( ACCOUNT acctnum );

	void SetOnline( ACCOUNT acctnum, P_CHAR pc );
	void SetEntering( ACCOUNT acctnum );
	void SetOffline( ACCOUNT acctnum );
	void OnLogin( ACCOUNT acctnum, NXWSOCKET socket );
	SI32 verifyPassword(std::string username, std::string password);
	SI32 ChangePassword( ACCOUNT acctnum, std::string password);
	void AddCharToAccount( ACCOUNT acctnum, P_CHAR pc );
	void GetAllChars( ACCOUNT acctnum, NxwCharWrapper& sc );
	ACCOUNT GetAccountByName(std::string name);
	bool RemoveAccount( std::string name );

};

extern cAccounts* Accounts;


#endif // __ACCOUNTS_H__
