#include "clientcrypt.h"



void ClientCrypt::decrypt(unsigned char *in, unsigned char *out, int length)
{
	if ( crypt_mode == CRYPT_LOGIN)
		loginCrypter.decrypt( in, out, length);
	else if ( crypt_mode == CRYPT_GAME )
	{
		gameCrypter->decrypt( in, out, length);
		if (cryptVersion==CRYPT_2_0_3)
			crypt203->decrypt(out, out, length);
	}
}

void ClientCrypt::encrypt(unsigned char *in, unsigned char *out, int length)
{
	if ( crypt_mode == CRYPT_LOGIN)
		loginCrypter.encrypt( in, out, length);
	else if ( crypt_mode == CRYPT_GAME )
		gameCrypter->encrypt( in, out, length);
}

void ClientCrypt::setLoginCryptKeys(UI32 key1, UI32 key2)
{
	loginKey1= key1;
	loginKey2= key2;
}

void ClientCrypt::init(unsigned char *pseed)
{
	memcpy (clientSeed, pseed, 4);
	loginCrypter.init(pseed, loginKey1, loginKey2);
}

void ClientCrypt::setCryptSeed(UI08 pseed[4])
{
	memcpy (clientSeed, pseed, 4);

}

void ClientCrypt::setCryptSeed(UI32 pseed)
{
	clientSeed[0]=(pseed >> 24)&0xFF;
	clientSeed[1]=(pseed >> 16)&0xFF;
	clientSeed[2]=(pseed >> 8)&0xFF;
	clientSeed[3]=pseed &0xFF;

}

void ClientCrypt::setGameEncryption(int version)
{
// 	if ( gameCrypter != NULL)
//		delete gameCrypter;
    cryptVersion = version;
}

void ClientCrypt::setCryptMode(UI08 mode)
{
	if ( mode == CRYPT_NONE || mode == CRYPT_LOGIN || mode == CRYPT_GAME )
		crypt_mode = mode;
	if ( crypt_mode == CRYPT_GAME )
	{

		if (cryptVersion>CRYPT_2_0_3)
			if ( gameCrypter != NULL )
				gameCrypter->reinit(clientSeed);
			else
				gameCrypter = new NewGameCrypt(clientSeed);
		else if (cryptVersion==CRYPT_2_0_3)
		{
			if ( gameCrypter != NULL )
				gameCrypter->reinit(clientSeed);
			else
				gameCrypter = new NewGameCrypt(clientSeed);
				crypt203 = new OldGameCrypt();
				crypt203->init();
		}
		else
			gameCrypter = new OldGameCrypt();

		gameCrypter->init();
	}

}

void ClientCrypt::setEntering(bool state)
{
	entering = state;
}

bool ClientCrypt::getEntering()
{
	return entering;
}

ClientCrypt::~ClientCrypt()
{
	if(gameCrypter != NULL )
		delete gameCrypter;
	if ( crypt203 != NULL )
		delete crypt203;
}

ClientCrypt::ClientCrypt()
{
	cryptVersion=CRYPT_UNENCRYPTED;
	entering=false;
	gameCrypter=NULL;
	crypt203=NULL;
}
