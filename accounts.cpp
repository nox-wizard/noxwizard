  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "nxwcommn.h"
#include "accounts.h"
#include "debug.h"
#include "encryption.h"
#include "network.h"
#include "chars.h"
#include "chars.h"
#include "client.h"
#include "srvparms.h"
#include "globals.h"
#include "logsystem.h"
#include "basics.h"
#include "inlines.h"
#include "items.h"
 
//global accounts object
cAccounts* Accounts = NULL;


/*!
\brief Constructor
\author Endymion
*/
cAccount::cAccount( ACCOUNT num ) 
{
	number=num;
	//
	//	Sparhawk:	gcc 3.3 doesn't like this, also it's not really needed
	//
	//pgs.clear();
	//name.erase();
	//pass.erase();
	ban = false;
	ras = false;
	tempblock=0;
	blockeduntil=0;
	lastlogin=0;
	state=LOG_OUT;
	pc_online=INVALID;
}

/*!
\brief return ip in dotted notation as string
\return string with dotted notation IP
\author Wintermute
*/
std::string cAccount::getLastIp()
{
	std::string temp;
	char buf [16];
	sprintf(buf, "%d.%d.%d.%d",lastIp&0xFF,(lastIp>>8)&0xFF,(lastIp>>16)&0xFF,(lastIp>>24)&0xFF);
	temp=std::string(buf);
	return temp;
}

/*!
\brief Set Account in entering state (LOG_ENTERING)
\author Endymion
*/
void cAccount::setEntering( )
{
	state = LOG_ENTERING;
}

/*!
\brief tells if accounts is entering
\return true if state is LOG_ENTERING
\author Wintermute
*/
bool cAccount::isEntering()
{
	return state==LOG_ENTERING;
}

/*!
\brief Sets the online character of the account
\author Endymion
\param pc the character
*/
void cAccount::setOnline( P_CHAR pc )
{
	if(!ISVALIDPC(pc))
		setOffline();
	else {
		pc_online=pc->getSerial32();
		state=LOG_INGAME;
	}
}

/*!
\brief Check if the account is in LOG_INGAME status

Also performs a validity check of the online character, 
setting the account offline if an invalid character is found.\n
\author Endymion
\return true if online 
*/
bool cAccount::isOnline( )
{
	if ( pc_online > 0 )
	{
		P_CHAR pc = pointers::findCharBySerial(pc_online);
		if ( ! ISVALIDPC(pc) )
			setOffline();
	}
	
	return (state == LOG_INGAME);
}

/*!
\brief Gets the current online character

Performs character validity check with isOnline()
\author Endymion
\return the serial of the character, or INVALID if the acount is not online
*/
SERIAL cAccount::getInWorld( )
{
	return isOnline() ? pc_online : INVALID;
}

/*!
\brief Sets the account offline
\author Endymion
*/
void cAccount::setOffline(  )
{
	pc_online=INVALID;
	state=LOG_OUT;
}

/*!
\brief Sets the account in LOG_ENTERING state

Performs socket validity check
\param socket the socket
\author Endymion
*/
void cAccount::onLogin( NXWSOCKET socket )
{
	if( socket!=INVALID ) {
		lastlogin=time(0);
		state=LOG_ENTERING;
	}
	else 
		setOffline();

}

/*!
\brief Changes password of account

Performs password encryption if required by server param g_nUseAccountEncryption
\param password the new password
\author Endymion
\todo check problem with STL strings, see comment in function body
*/
void cAccount::changePassword ( std::string password )
{

	if (ServerScp::g_nUseAccountEncryption) 
	{ //xan : for account DES encryption :)
		char str[1000];
		strcpy(str,password.c_str());
		char *pwd = pwdcypher(str, -1);
		str[0] = '!'; str[1] = '\0';
		strcat(str,pwd);
		safedelete(pwd); //xan : avoid a repetitive memory leak :)
		password.erase();
		password = str;    //someone with a bit of knowledge about STL strings.. plz chg this!
	}

	this->pass = password;
}

/*!
\brief Get list of pg of this account
\author Endymion
\param sc the list
\return the list with serials of all characters in the account
*/
void cAccount::getAllChars(  NxwCharWrapper& sc )
{
	sc.clear();
	for( unsigned int i=0; i< pgs.size(); i++ ) 
		sc.insertSerial( pgs[i] );
}


/*!
\brief Add given char to account
\author Endymion
\param pc the char
*/
void cAccount::addCharToAccount( P_CHAR pc )
{
	if( pgs.size() < 5 ) 
	{
		pgs.push_back( pc->getSerial32() );
		pc->account= number;
	}
	else 
		pc->account=INVALID;
}

//
//cAccounts class members start here
//

/*!
\brief Constructor of cAccounts
*/
cAccounts::cAccounts()
{
	unsavedaccounts = 0;
	saveratio = 0;         // Save everyaccount
}

/*!
\brief Destructor of cAccounts

Saves unsaved accounts
*/
cAccounts::~cAccounts()
{
	if (unsavedaccounts > 0)
		SaveAccounts();
}

/*!
\brief Inserts a new account to the list

Account is inserted at acc.number position in the list
\param acc the account
*/
void cAccounts::safeInsert( cAccount& acc )
{
	if( acc.number==INVALID )
		return;
	acctlist[acc.number]= acc;
	accbyname[acc.name] = acc.number;
}

/*!
\brief Load an account from a file

\a F must point to the first line of a \a SECTION \a ACCOUNT \n
The function will parse the account declaration until the end of the section
\param acctnumb Account number
\param F Account file
\todo remove Windows prompt when admin logs with standard pw ?
*/
void cAccounts::LoadAccount( ACCOUNT acctnumb, FILE* F )
{
	int loopexit=0; //loop count
	cAccount account( acctnumb ); 
    	char script1[1024]; //strings that will be filled with scripts
    	char script2[1024];

	//parse section commands, stop on closing brace
	do
	{
		readSplitted(F, script1, script2);
		if (!strcmp(script1, "NAME"))		account.name =  script2;
		else if (!strcmp(script1, "PASS"))	account.pass = script2;
		else if (!strcmp(script1, "BAN"))	account.ban = true;
		else if (!strcmp(script1, "REMOTEADMIN")) account.ras = true;
		else if (!strcmp(script1, "LASTLOGIN")) account.lastlogin = atoi(script2);
	}
	while ( !feof(F) && strcmp(script1, "}") && strcmp(script1, "EOF") && ++loopexit < MAXLOOPS);
	
	//windows-gui only section: shows warning if admin logs with standard admin pass
	#ifdef WIN32
	static bool bWarned = false; //this makes the warning appear one time only
	if ( ServerScp::g_nDeamonMode == 0 &&
	     ServerScp::g_nLoadDebugger ==0 &&
	     bWarned && 
	     account.number == ADMIN_ACCOUNT && 
	     account.name == "admin" && 
	     account.pass == "admin")
	{
			MessageBox(NULL, 
			"You have yet the standard Admin password.\nRemember to change it before allowing public access to your shard!\nRead the configuration help (NXWUSER.PDF) to learn how to do this.", "Security warning",MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
			bWarned = true;
	}
	
	#endif
	
	safeInsert( account );
	lasttimecheck = uiCurrentTime;
}

/*!
\brief Get the number of accounts
\return int number of accounts
*/
int cAccounts::Count()
{
	return acctlist.size();
}

/*!
\brief Reload all accounts

Parses accounts file and loads its content

\todo check horrible code, see comments in function body
*/
void cAccounts::LoadAccounts()
{
	int b,c,ac;
	unsigned int account;
	char accnumb[264];
	char acc[264];
	char *t;
	lastusedacctnum = 0;
	char script1[1024];
	char script2[1024];

	FILE *F = fopen(ACCOUNTS_FILENAME, "rt");
	if (F==NULL) 
	{
	#ifndef WIN32
		ConOut("************\nCan't load accounts. Can't continue.\n***********");
		ConOut("\nPress <return> to exit.");
		char str[80];
		fgets(str, 70, stdin);
	#else
		MessageBox(NULL, "Can't load accounts. Can't continue.", "Fatal Error - NoX-Wizard", MB_ICONHAND);
	#endif
		exit(1);
	}
	
	//clear old accounts
	acctlist.clear();

	while (!feof(F))
	{
		readSplitted(F, script1, script2);
		
		if (feof(F)) break;
		
		if (!(strcmp(script1, "SECTION")))
		{
			//WHAT'S THIS?!? please change to something reasonable ...
			c = strlen(script2);
			for (b=0; b<9; b++) acc[b]=script2[b];
			for (b=8; b<c; b++) accnumb[b-8]=script2[b];
			accnumb[b-8]=0; acc[8]=0;
			ac = strtol(accnumb, &t, 10);
			if (strlen(t)!=0) ac=-1;

			if (strcmp(acc,"ACCOUNT ") || ac < 0 )
			{
				ConOut("Error loading accounts, skipping invalid account entry!\n");

			} 
			else 
			{
			   account=ac;
			   LoadAccount(account, F);
			   if (account > lastusedacctnum)
				   lastusedacctnum = account;
			}
		}
	}
	fclose(F);
}

/*!
\brief Save all accounts

Saves account information to accounts file
*/
void cAccounts::SaveAccounts()
{
	//open and check file
	FILE* F = fopen(ACCOUNTS_FILENAME, "wt") ;
	if (F == NULL) 
	{
		ErrOut("Can't open %s for writing\n",ACCOUNTS_FILENAME);
		return;
	}

	unsigned int maxacctnumb = 0;     // Saving the number of loaded accounts
	std::string  line ;
	
	//scan acctlist and print account data to file
	ACCOUNT_LIST::iterator iter_account(acctlist.begin()); 
	ACCOUNT_LIST::iterator iter_account_end(acctlist.end()); 
	
	for (; iter_account != iter_account_end; ++iter_account )
	{
		fprintf(F, "SECTION ACCOUNT %d\n{\nNAME %s\nPASS %s\n", iter_account->second.number,
			iter_account->second.name.c_str(), iter_account->second.pass.c_str() );
		
		if (iter_account->second.lastlogin) 
		fprintf(F,"LASTLOGIN %lu\n",iter_account->second.lastlogin);
		
		if ( iter_account->second.getLastIp() != "0.0.0.0" ) 
		fprintf(F,"LASTIP %s\n",(iter_account->second.getLastIp().c_str()));
		
		if (iter_account->second.ban) fprintf(F, "BAN\n");
		
		if (iter_account->second.ras) fprintf(F, "REMOTEADMIN\n");

		fprintf(F, "}\n\n");
	}
	
	fprintf(F, "\n\n// Note: Last used Account Number was: %d\nEOF\n" , maxacctnumb );

	fclose(F);
	unsavedaccounts = 0;
}

/*!
\brief Verify a password
 
Currently only needed by encryption due to two crypt versions returning a valid username, 
but one an invalid password

\param username Username
\param password Password
\return AUTHENTICATE_RESULT value
\todo check STL strings problem, see comment in function body
*/
SI32 cAccounts::verifyPassword(std::string username, std::string password)
{
	ACCOUNT_LIST_BY_NAME::iterator iter_account_by_name(accbyname.find(username));

	if (iter_account_by_name != accbyname.end())
	{
		ACCOUNT_LIST::iterator iter_account(acctlist.find(iter_account_by_name->second) );
		
		if( iter_account == acctlist.end() )
			return LOGIN_NOT_FOUND;

		if ((iter_account->second.pass[0])=='!') 
		{
			//these are encrypted passwords!!!
			char str[100];
			strcpy(str,password.c_str());
			char *pwd = pwdcypher(str, (iter_account->second.pass[1])-'A');
			
			if (pwd==NULL) 
				return LOGIN_NOT_FOUND;
			
			str[0] = '!'; str[1] = '\0';
			strcat(str,pwd);
			safedelete(pwd); //xan : avoid a repetitive memory leak :)
			password.erase();
			password = str;    //someone with a bit of knowledge about STL strings.. plz chg this!
		}

		if( iter_account->second.pass != password )
			return BAD_PASSWORD;
	}
	return 0;
}


/*!
\brief authenticate username and password
\param username Username
\param password Password
\return AUTHENTICATE_RESULT value
*/
SI32 cAccounts::Authenticate(std::string username, std::string password)
{

	ACCOUNT_LIST_BY_NAME::iterator iter_account_by_name(this->accbyname.find(username));

	if (iter_account_by_name != this->accbyname.end())
	{
		ACCOUNT_LIST::iterator iter_account( this->acctlist.find(iter_account_by_name->second) );
		if( iter_account==this->acctlist.end() )
			return LOGIN_NOT_FOUND;

		if ((iter_account->second.pass[0])=='!') 
		{
			//these are encrypted passwords!!!
			char str[100];
			strcpy(str,password.c_str());
			char *pwd = pwdcypher(str, (iter_account->second.pass[1])-'A');
			if (pwd==NULL) 
				return LOGIN_NOT_FOUND;
			str[0] = '!'; str[1] = '\0';
			strcat(str,pwd);
			safedelete(pwd); //xan : avoid a repetitive memory leak :)
			password.erase();
			password = str;    //someone with a bit of knowledge about STL strings.. plz chg this!
		}

		if( iter_account->second.pass == password )
		{
			if( iter_account->second.ban )
			{
				InfoOut("account banned\n "); //elcabesa tempblock
				return ACCOUNT_BANNED;
			}
			else if( (server_data.blockaccbadpass==1) && (iter_account->second.tempblock>=server_data.n_badpass) && (iter_account->second.blockeduntil>uiCurrentTime) )//elcabesa tempblock
			{
				InfoOut("account blocked \n"); //elcabesa tempblock
				return BAD_PASSWORD;//elcabesa tempblock
			}
			else
			{
				iter_account->second.tempblock=0;//elcabesa tempblock
				iter_account->second.blockeduntil=0;//elcabesa tempblock
				return iter_account->second.number;
			}
		} else
		{
			if(server_data.blockaccbadpass==1)		//elcabesa tempblock
			{										//elcabesa tempblock
				iter_account->second.tempblock++;//elcabesa tempblock
				iter_account->second.blockeduntil=uiCurrentTime+MY_CLOCKS_PER_SEC*60*server_data.time_badpass;//elcabesa tempblock
			}										//elcabesa tempblock
			return BAD_PASSWORD;
		}
	} else
		return LOGIN_NOT_FOUND;
}

/*!
\brief Check if the user can use the RAS ( Remote Admin System )
\param username Username
\param password Password
\remarks Always the Account n° 0 can use Ras 
\return true if can, false else
\todo write a function for password decryption and use it instead of copyng the same code among functions
*/
bool cAccounts::AuthenticateRAS(std::string username, std::string password)
{
	ACCOUNT_LIST_BY_NAME::iterator iter_account_by_name(accbyname.find(username));

	if (iter_account_by_name != accbyname.end())
	{

		ACCOUNT_LIST::iterator iter_account(acctlist.find(iter_account_by_name->second) );
		if( iter_account == acctlist.end() )
			return false;

		if ((iter_account->second.pass.c_str())[0]=='!') {
			//these are encrypted passwords!!!
			char str[100];
			strcpy(str,password.c_str());
			char *pwd = pwdcypher(str, (iter_account->second.pass.c_str())[1]-'A');
			if (pwd==NULL) return false;
			str[0] = '!'; str[1] = '\0';
			strcat(str,pwd);
			safedelete(pwd); //xan : avoid a repetitive memory leak :)
			password.erase();
			password = str;    //someone with a bit of knowledge about STL strings.. plz chg this!
		}

		if( iter_account->second.pass == password )
		{
			if( iter_account->second.ban )
				return false;
			else
				return (iter_account->second.ras)||(iter_account->second.number==ADMIN_ACCOUNT);
		} else
			return false;
	} else
		return false;
}


/*!
\brief Create a new account
Creates a new account with the given usernam and password

\note Performs accounts save if needed
\param username Username
\param password Password

\return ACCOUNT the account id or INVALID if not created
\todo check problem with STL strings, see comments in function body
*/
ACCOUNT cAccounts::CreateAccount(std::string username, std::string password)
{
	lastusedacctnum++;
	
	// if there's another account with this name..
	if ( accbyname.count(username) )	
		return INVALID;
	
	if (ServerScp::g_nUseAccountEncryption) 
	{ //xan : for account DES encryption :)
		char str[1000];
		strcpy(str,password.c_str());
		char *pwd = pwdcypher(str, -1);
		if (pwd!=NULL)
		{   //if pwd fails (any reason) use unencrypted pwd! :)
			str[0] = '!'; str[1] = '\0';
			strcat(str,pwd);
			safedelete(pwd); //xan : avoid a repetitive memory leak :)
			password.erase();
			password = str;    //someone with a bit of knowledge about STL strings.. plz chg this!
		}
	}


	cAccount account(lastusedacctnum);
	account.name = username;
	account.pass = password;
	safeInsert( account );
	unsavedaccounts++;
	
	if (unsavedaccounts >= saveratio)
		SaveAccounts();
	
	return account.number;
}

/*!
\brief Check the account file status

Checks the accounts file and reloads accounts if the file changed.
*/
void cAccounts::CheckAccountFile()
{

	struct stat filestatus;
	static time_t lastchecked;

	stat(ACCOUNTS_FILENAME, &filestatus);

	if (difftime(filestatus.st_mtime, lastchecked) > 0.0)
		LoadAccounts();

	lastchecked = filestatus.st_mtime;
	lasttimecheck = uiCurrentTime;

}

/*!
\brief Check if an account is online

\return boolean
\param acctnum Account Number
\sa cAccount::isOnline()
*/
bool cAccounts::IsOnline( ACCOUNT acctnum )
{
	if ( acctnum <= INVALID )
		return false;
	
	ACCOUNT_LIST::iterator iter( acctlist.find(acctnum) );
	if (iter != acctlist.end())
		return iter->second.isOnline();
	else 
		return false;
}

/*!
\brief Get char in game of given account
\return Serial if exist, else INVALID
\param acctnum Account Number
\sa cAccount::getInWorld()
*/
SERIAL cAccounts::GetInWorld( ACCOUNT acctnum )
{
	if (acctnum <= INVALID )
		return INVALID;
	
	ACCOUNT_LIST::iterator iter( acctlist.find(acctnum) );
	if (iter != acctlist.end())
		return iter->second.getInWorld();
	else 
		return INVALID;
}

/*!
\brief Set Online the player
\param acctnum Account number
\param pc The Character
\sa cAccount::setOnline()
*/
void cAccounts::SetOnline( P_CHAR pc )
{
	VALIDATEPC(pc);
	ACCOUNT_LIST::iterator iter( acctlist.find(pc->account) );
	if (iter != acctlist.end())
		iter->second.setOnline( pc );
}

/*!
\brief Set Offline the player
\param acctnum Account number
\sa cAccount::setOffline()
*/
void cAccounts::SetOffline( ACCOUNT acctnum )
{
	ACCOUNT_LIST::iterator iter(acctlist.find(acctnum) );
	if (iter != acctlist.end())
		iter->second.setOffline();
}

/*!
\brief Update account information
\param acct Account number
\param sck Socket
\sa cAccount::onLogin()
*/
void cAccounts::OnLogin(ACCOUNT acct, NXWSOCKET sck)
{
	if(sck<=INVALID) 
		return;

	ACCOUNT_LIST::iterator iter_account(acctlist.find(acct) );

	if(iter_account!=acctlist.end())
		iter_account->second.onLogin( sck );
}

/*!
\brief Account are loggin into
\param acctnum Account number
*/
void cAccounts::SetEntering( ACCOUNT acctnum ) 
{
	ACCOUNT_LIST::iterator iter_account(acctlist.find(acctnum) );

	if(iter_account != acctlist.end())
		iter_account->second.setEntering();
}


/*!
\brief Change the Password
\return SI32 0 if ok, INVALID else
\param acctnum Account number
\param password new Password
\sa cAccount::changePassword()
*/
SI32 cAccounts::ChangePassword( ACCOUNT acctnum, std::string password)
{

	ACCOUNT_LIST::iterator iter(acctlist.find( acctnum ));
	if( iter != acctlist.end() )
	{
		iter->second.changePassword( password );
		unsavedaccounts++;
		if (unsavedaccounts >= saveratio) SaveAccounts();
		return 0;
	}
	else
		return INVALID;	
}

/*!
\brief Get list of pg of this account
\return SI32 0 if ok, INVALID else
\param acctnum Account number
\param sc the list
\sa cAccount::getAllChars()
*/
void cAccounts::GetAllChars( ACCOUNT acctnum, NxwCharWrapper& sc )
{
	ACCOUNT_LIST::iterator iter(acctlist.find( acctnum ));
	if( iter != acctlist.end() )
		iter->second.getAllChars( sc );
	else
		sc.clear();
}

/*!
\brief Add given char to account
\author Endymion
\param acctnum the account
\param pc the char
\sa cAccount::addCharToAccount()
*/
void cAccounts::AddCharToAccount( ACCOUNT acctnum, P_CHAR pc )
{
	if(!ISVALIDPC(pc)  || pc->npc || acctnum==INVALID ) 
	{
		pc->account=INVALID;
		return;
	}

	ACCOUNT_LIST::iterator iter( acctlist.find( acctnum ));
	if( iter != acctlist.end() )
		iter->second.addCharToAccount( pc );
	else 
		pc->account=INVALID;
}

/*!
\brief Get an account from his name
\author Akron
\return the account object, or INVALID if no accounts found
\param accname the account name
*/
ACCOUNT cAccounts::GetAccountByName(std::string accname)
{
	ACCOUNT_LIST_BY_NAME::iterator iter( accbyname.find( accname ));
	if ( iter!=accbyname.end() )
		return iter->second;
	else
		return INVALID;
}

/*!
\brief Get an account from his number
\author Akron
\return the account object, or INVALID if no accounts found
\param acctnumb account number
*/
cAccount *cAccounts::GetAccount(ACCOUNT acctnumb)
{
	ACCOUNT_LIST::iterator iter( acctlist.find( acctnumb ));
	if ( iter!=acctlist.end() )
		return &(iter->second);
	else
		return NULL;
}

/*!
\brief Delete an account, by number
\note accounts are saved after removal
\author Akron
\return true if account is deleted correctly, or false
\param ACCOUNT acctnumb
\sa cAccounts::RemoveAccount(std::string)
*/
bool cAccounts::RemoveAccount(ACCOUNT acctnumb)
{
	//disconnect if it's online
	if ( IsOnline(acctnumb) )
	{
		unsigned int r = pointers::findCharBySerial(GetInWorld(acctnumb))->getClient()->toInt();
		Network->Disconnect(r);
	}
	
	//delete all characters
	NxwCharWrapper acc_chars;
	GetAllChars( acctnumb, acc_chars );
	for( acc_chars.rewind(); !acc_chars.isEmpty(); acc_chars++ )
	{
		P_CHAR pc = acc_chars.getChar();
		if(ISVALIDPC(pc))
			pc->Delete();
	}
	
	acctlist.erase( acctnumb );
	SaveAccounts();
	return true;
}

/*!
\brief Delete an account, by name

Overloaded version of cAccounts::RemoveAccount(ACCOUNT)
\author Akron
\return true if account is deleted correctly, or false
\param name the account name
\sa cAccounts::RemoveAccount(ACCOUNT)
*/
bool cAccounts::RemoveAccount(std::string name)
{
	ACCOUNT_LIST_BY_NAME::iterator iter( accbyname.find( name ));
	ACCOUNT acc;
	if ( iter == accbyname.end() )
		return false;
	else
		acc = iter->second;
	
	return RemoveAccount(acc);
}
