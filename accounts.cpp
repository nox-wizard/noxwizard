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
 

cAccounts* Accounts=NULL;


/*!
\brief Constructor
\author Endymion
*/
cAccount::cAccount( ACCOUNT num ) {
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
	lastip.s_addr=0;
	state=LOG_OUT;
	pc_online=INVALID;
}

/*!
\brief Set Account in entering
\author Endymion
*/
void cAccount::setEntering( )
{
	state=LOG_ENTERING;
}

/*!
\brief Set online with given char
\author Endymion
\param pc the char
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
\brief Check if online
\author Endymion
\return true if online 
\note Also when entering is online
*/
bool cAccount::isOnline( )
{
	return (state!=LOG_OUT);
}

/*!
\brief Get char in world
\author Endymion
\return serial of char
*/
SERIAL cAccount::getInWorld( )
{
	return ( isOnline() && (state==LOG_INGAME) )? pc_online : INVALID;
}


/*!
\brief Set offline
\author Endymion
*/
void cAccount::setOffline(  )
{
	pc_online=INVALID;
	state=LOG_OUT;
}

/*!
\brief On login of account
\author Endymion
*/
void cAccount::onLogin( NXWSOCKET socket )
{
	if( socket!=INVALID ) {
		lastlogin=time(0);
		lastip.s_addr=*(unsigned long*)&clientip[socket];
		state=LOG_ENTERING;
	}
	else 
		setOffline();

}

/*!
\brief Change password of account
\author Endymion
*/
void cAccount::changePassword ( std::string password )
{

	if (ServerScp::g_nUseAccountEncryption) { //xan : for account DES encryption :)
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
*/
void cAccount::getAllChars(  NxwCharWrapper& sc )
{
	sc.clear();
	for( unsigned int i=0; i<this->pgs.size(); i++ ) {
		sc.insertSerial( this->pgs[i] );
	}
}


/*!
\brief Add given char to account
\author Endymion
\param pc the char
*/
void cAccount::addCharToAccount( P_CHAR pc )
{
	if( this->pgs.size() < 5 ) {
		this->pgs.push_back( pc->getSerial32() );
		pc->account=this->number;
	}
	else 
		pc->account=INVALID;
}

/*!
\brief Constructor of cAccount
*/
cAccounts::cAccounts()
{
	unsavedaccounts = 0;
	saveratio = 0;         // Save everyaccount
}

/*!
\brief Destructor of cAccount
*/
cAccounts::~cAccounts()
{
	if (unsavedaccounts > 0)
		SaveAccounts();
}


/*!
\brief Safe insert info list
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
\brief Reload the Account
\param acctnumb Account number
\param F Account file
*/
void cAccounts::LoadAccount( ACCOUNT acctnumb, FILE* F )
{
#ifdef WIN32
	static bool bWarned = false;
#endif
	int loopexit=0;
	cAccount account( acctnumb );
    char script1[1024];
    char script2[1024];

	do
	{
		readSplitted(F, script1, script2);
		if (!strcmp(script1, "NAME"))		account.name =  script2;
		else if (!strcmp(script1, "PASS"))	account.pass = script2;
		else if (!strcmp(script1, "BAN"))	account.ban = true;
		else if (!strcmp(script1, "REMOTEADMIN")) account.ras = true;
		else if (!strcmp(script1, "LASTLOGIN")) account.lastlogin = atoi(script2);
		else if (!strcmp(script1, "LASTIP")) account.lastip.s_addr = inet_addr(script2);
	}
	while ( (!feof(F))&&(strcmp(script1, "}")) && (strcmp(script1, "EOF")) && (++loopexit < MAXLOOPS) );
	#ifdef WIN32
	if ((ServerScp::g_nDeamonMode==0)&&(ServerScp::g_nLoadDebugger==0)) {
		if ((!bWarned) && (account.number==ADMIN_ACCOUNT) && (account.name=="admin") && (account.pass=="admin"))
		{
			MessageBox(NULL, "You have yet the standard Admin password.\nRemember to change it before allowing public access to your shard!\nRead the configuration help (NXWUSER.PDF) to learn how to do this.", "Security warning",MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
			bWarned = true; //otherwise the warning appears too many times! :)
		}
	}
	#endif
	safeInsert( account );
	lasttimecheck = uiCurrentTime;
}

/*!
\brief Get the number of account
\return int
*/
int cAccounts::Count()
{
	return acctlist.size();
}

/*!
\brief Reload all account
*/
void cAccounts::LoadAccounts( void )
{
	int b,c,ac;
	unsigned int account;
	char accnumb[264];
	char acc[264];
	char *t;
	lastusedacctnum = 0;
    char script1[1024];
    char script2[1024];

	FILE *F = fopen("config/accounts.adm", "rt");
	if (F==NULL) {
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

	acctlist.clear();
	while (!feof(F))
	{
		readSplitted(F, script1, script2);
		if (feof(F)) break;
		if (!(strcmp(script1, "SECTION")))
		{
			c = strlen(script2);
			for (b=0; b<9; b++) acc[b]=script2[b];
			for (b=8; b<c; b++) accnumb[b-8]=script2[b];
			accnumb[b-8]=0; acc[8]=0;
			ac = strtol(accnumb, &t, 10);
			if (strlen(t)!=0) ac=-1;

			if (strcmp(acc,"ACCOUNT ") || ac < 0 )
			{
				ConOut("Error loading accounts, skipping invalid account entry!\n");

			} else {
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
\brief Save all account
*/
void cAccounts::SaveAccounts( void )
{
	ACCOUNT_LIST::iterator iter_account( this->acctlist.begin() ), iter_account_end( this->acctlist.end() );

	std::string  line ;
	FILE* F = fopen("config/accounts.adm", "wt") ;
	if (F == NULL) {
		ErrOut("Can't open config/accounts.adm for writing\n");
		return;
	}

	unsigned int maxacctnumb = 0;     // Saving the number of loaded accounts

	for (; iter_account != iter_account_end; ++iter_account )
	{
		fprintf(F, "SECTION ACCOUNT %d\n{\nNAME %s\nPASS %s\n", iter_account->second.number,
			iter_account->second.name.c_str(), iter_account->second.pass.c_str() );

		if (iter_account->second.lastlogin) fprintf(F,"LASTLOGIN %lu\n",iter_account->second.lastlogin);
		if (iter_account->second.lastip.s_addr) fprintf(F,"LASTIP %s\n",inet_ntoa(iter_account->second.lastip));
		if (iter_account->second.ban) fprintf(F, "BAN\n");
		if (iter_account->second.ras) fprintf(F, "REMOTEADMIN\n");

		fprintf(F, "}\n\n");
	}
	fprintf(F, "\n\n// Note: Last used Account Number was: %d\nEOF\n" , maxacctnumb );

	fclose(F);
	unsavedaccounts = 0;
}

/*!
\brief Verify a password, currently only needed by encryption due to two crypt versions returning a valid username, but one an invalid password
\return account, of why not right
\param username Username
\param password Password
*/

SI32 cAccounts::verifyPassword(std::string username, std::string password)
{
	ACCOUNT_LIST_BY_NAME::iterator iter_account_by_name(this->accbyname.find(username));

	if (iter_account_by_name != this->accbyname.end())
	{
		ACCOUNT_LIST::iterator iter_account( this->acctlist.find(iter_account_by_name->second) );
		if( iter_account==this->acctlist.end() )
			return LOGIN_NOT_FOUND;

		if ((iter_account->second.pass[0])=='!') {
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
		{
			return BAD_PASSWORD;
		}
	}
	return 0;
}


/*!
\brief Get info about account by username and password
\return account, of why not right
\param username Username
\param password Password
*/
SI32 cAccounts::Authenticate(std::string username, std::string password)
{

	ACCOUNT_LIST_BY_NAME::iterator iter_account_by_name(this->accbyname.find(username));

	if (iter_account_by_name != this->accbyname.end())
	{
		ACCOUNT_LIST::iterator iter_account( this->acctlist.find(iter_account_by_name->second) );
		if( iter_account==this->acctlist.end() )
			return LOGIN_NOT_FOUND;

		if ((iter_account->second.pass[0])=='!') {
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
\return true if can, false else
\param username Username
\param password Password
\remarks Always the Account n° 0 can use Ras 
*/
bool cAccounts::AuthenticateRAS(std::string username, std::string password)
{
	ACCOUNT_LIST_BY_NAME::iterator iter_account_by_name(this->accbyname.find(username));

	if (iter_account_by_name != this->accbyname.end())
	{

		ACCOUNT_LIST::iterator iter_account( this->acctlist.find(iter_account_by_name->second) );
		if( iter_account==this->acctlist.end() )
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
\return ACCOUNT the account id or INVALID if not created
\param username Username
\param password Password
*/
ACCOUNT cAccounts::CreateAccount(std::string username, std::string password)
{
	lastusedacctnum++;

	if ( accbyname.count(username) )	// if there's another account with this name..
		return INVALID;
	
	if (ServerScp::g_nUseAccountEncryption) { //xan : for account DES encryption :)
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
\remarks Reload Account if modified
*/
void cAccounts::CheckAccountFile()
{

	struct stat filestatus;
	static time_t lastchecked;

	stat("config/accounts.adm", &filestatus);

	if (difftime(filestatus.st_mtime, lastchecked) > 0.0)
		LoadAccounts();

	lastchecked = filestatus.st_mtime;
	lasttimecheck = uiCurrentTime;

}

/*!
\brief Check if the Account is online
\return boolean
\param acctnum Account Number
*/
bool cAccounts::IsOnline( ACCOUNT acctnum )
{
	if ( acctnum <= INVALID )
		return false;
	ACCOUNT_LIST::iterator iter( this->acctlist.find(acctnum) );
	if (iter != this->acctlist.end())
	{
		return iter->second.isOnline();
	} 
	else 
		return false;
}

/*!
\brief Get char in game of given account
\return Serial if exist, else INVALID
\param acctnum Account Number
*/
SERIAL cAccounts::GetInWorld( ACCOUNT acctnum )
{
	if (acctnum <= INVALID )
		return INVALID;
	ACCOUNT_LIST::iterator iter( this->acctlist.find(acctnum) );
	if (iter != this->acctlist.end())
	{
		return iter->second.getInWorld();
	} 
	else 
		return INVALID;
}

/*!
\brief Set Online the player
\param acctnum Account number
\param pc The Character
*/
void cAccounts::SetOnline( ACCOUNT acctnum, P_CHAR pc )
{
	VALIDATEPC(pc);
	ACCOUNT_LIST::iterator iter( this->acctlist.find(pc->account) );
	if (iter != this->acctlist.end())
	{
		iter->second.setOnline( pc );
	}
}





/*!
\brief Set Offline the player
\param acctnum Account number
*/
void cAccounts::SetOffline( ACCOUNT acctnum )
{
	ACCOUNT_LIST::iterator iter( this->acctlist.find(acctnum) );
	if (iter != this->acctlist.end())
	{
		iter->second.setOffline();
	}

}

/*!
\brief Update account information
\param acct Account number
\param sck Socket
*/
void cAccounts::OnLogin(ACCOUNT acct, NXWSOCKET sck)
{
	if(sck<=INVALID) 
		return;

	ACCOUNT_LIST::iterator iter_account( this->acctlist.find(acct) );

	if(iter_account!=this->acctlist.end())
	{
		iter_account->second.onLogin( sck );
	}
}


/*!
\brief Account are loggin into
\param acctnum Account number
*/
void cAccounts::SetEntering( ACCOUNT acctnum ) 
{
	ACCOUNT_LIST::iterator iter_account( this->acctlist.find(acctnum) );

	if(iter_account!=this->acctlist.end())
	{
		iter_account->second.setEntering();
	}
}


/*!
\brief Change the Password
\return SI32 0 if ok, INVALID else
\param acctnum Account number
\param password new Password
*/
SI32 cAccounts::ChangePassword( ACCOUNT acctnum, std::string password)
{

	ACCOUNT_LIST::iterator iter( this->acctlist.find( acctnum ));
	if( iter!=this->acctlist.end() )
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
*/
void cAccounts::GetAllChars( ACCOUNT acctnum, NxwCharWrapper& sc )
{
	ACCOUNT_LIST::iterator iter( this->acctlist.find( acctnum ));
	if( iter!=this->acctlist.end() )
	{
		iter->second.getAllChars( sc );
	}
	else
		sc.clear();
}

/*!
\brief Add given char to account
\author Endymion
\param acctnum the account
\param pc the char
*/
void cAccounts::AddCharToAccount( ACCOUNT acctnum, P_CHAR pc )
{
	if(!ISVALIDPC(pc)  || pc->npc || acctnum==INVALID ) {
		pc->account=INVALID;
		return;
	}

	ACCOUNT_LIST::iterator iter( this->acctlist.find( acctnum ));
	if( iter!=this->acctlist.end() )
	{
		iter->second.addCharToAccount( pc );
	}
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
\brief Delete an account
\author Akron
\return true if account is deleted correctly, or false
\param name the account name
*/
bool cAccounts::RemoveAccount(std::string name)
{
	ACCOUNT_LIST_BY_NAME::iterator iter( accbyname.find( name ));
	ACCOUNT acc;
	if ( iter==accbyname.end() )
		return false;
	else
		acc = iter->second;
	
	if ( IsOnline(acc) )
	{
		unsigned int r = pointers::findCharBySerial(GetInWorld(acc))->getClient()->toInt();
		Network->Disconnect(r);
	}
	
	NxwCharWrapper acc_chars;
	GetAllChars( acc, acc_chars );
	for( acc_chars.rewind(); !acc_chars.isEmpty(); acc_chars++ )
	{
		P_CHAR pc = acc_chars.getChar();
		if(ISVALIDPC(pc))
			pc->Delete();
	}
	
	accbyname.erase( name );
	acctlist.erase( acc );
	SaveAccounts();
	return true;
}
