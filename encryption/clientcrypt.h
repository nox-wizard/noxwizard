#ifndef _CLIENTCRYPT_H_INCLUDED_
#define _CLIENTCRYPT_H_INCLUDED_
#include "../common_libs.h"
#include "../constants.h"
#include "crypt.h"

enum { CRYPT_NONE, CRYPT_LOGIN, CRYPT_GAME } ;

enum {
	  CRYPT_UNENCRYPTED
	, CRYPT_1_26_4
	, CRYPT_2_0_0
	, CRYPT_2_0_1
	, CRYPT_2_0_2
	, CRYPT_2_0_3
	, CRYPT_2_0_4
	, CRYPT_2_0_5
	, CRYPT_2_0_6
	, CRYPT_2_0_7
	, CRYPT_2_0_8
	, CRYPT_2_0_9
	, CRYPT_3_0_0c
	, CRYPT_3_0_1
	, CRYPT_3_0_2
	, CRYPT_3_0_3a
	, CRYPT_3_0_4p
	, CRYPT_3_0_5
	, CRYPT_3_0_6j
	, CRYPT_3_0_7
	, CRYPT_3_0_8
	, CRYPT_3_0_9
	, CRYPT_4_0_0
	, CRYPT_4_0_1
	, CRYPT_4_0_2
	, CRYPT_LAST
};

const int loginKeys [CRYPT_LAST] [2]  = {
	{0x0		, 0x0		 } // No encryption key
	, {0x32750719, 0x0a2d100b } // crypt 1.26.4
	, {0x2d13a5fd, 0xa39d527f } // crypt 2.0.0
	, {0x2d2ba7ed, 0xa3817e7f } // crypt 2.0.1
	, {0x2d63addd, 0xa3a5227f } // crypt 2.0.2
	, {0x2dbbb7cd, 0xa3c95e7f } // crypt 2.0.3
	, {0x2df385bd, 0xa3ed127f } // crypt 2.0.4
	, {0x2c0b97ad, 0xa310de7f } // crypt 2.0.5
	, {0x2c43ed9d, 0xa334227f } // crypt 2.0.6
	, {0x2c9bc78d, 0xa35bfe7f } // crypt 2.0.7
	, {0x2cd3257d, 0xa37f527f } // crypt 2.0.8
	, {0x2ceb076d, 0xa363be7f } // crypt 2.0.9
	, {0x2d93a5fd, 0xa3dd527f } // crypt 3.0.0c
	, {0x2daba7ed, 0xa3c17e7f } // crypt 3.0.1
	, {0x2de3addd, 0xa3e5227f } // crypt 3.0.2
	, {0x2d3bb7cd, 0xa3895e7f } // crypt 3.0.3a
	, {0x2d7385bd, 0xa3ad127f } // crypt 3.0.4p
	, {0x2c8b97ad, 0xa350de7f } // crypt 3.0.5
	, {0x2cc3ed9d, 0xa374227f } // crypt 3.0.6j
	, {0x2c1bc78d, 0xa31bfe7f } // crypt 3.0.7
	, {0x2C53257d, 0xa33f527f } // crypt 3.0.8
	, {0x3c6b076d, 0xa3d3be7f } // crypt 3.0.9
	, {0x2E13A5FD, 0xA21D527F } // Client 4.0.0
	, {0x2E2BA7ED, 0xA2017E7F } // Client 4.0.1
	, {0x2E63ADDD, 0xA225227F } // Client 4.0.2
};



class ClientCrypt
{
private:
	LoginCrypt loginCrypter;
	GameCrypt *gameCrypter;
	GameCrypt *crypt203;
	UI08 crypt_mode;
	unsigned char clientSeed[4];
	UI32 loginKey1, loginKey2;
	int cryptVersion;
//    NormalCopier m_copier;
//    CompressingCopier m_compressor;
//    DecompressingCopier m_decompressor;
    bool compressed, first_send;
	bool entering;
	unsigned char cryptPacket [MAXBUFFER];
	unsigned char decryptPacket [MAXBUFFER];

public:
	void decrypt(unsigned char *in, unsigned char *out, int length);
	void encrypt(unsigned char *in, unsigned char *out, int length);
	void preview(unsigned char *in, unsigned char *out, int length);
	void setLoginCryptKeys(UI32 key1, UI32 key2);
	void setGameEncryption(int version);
	int getCryptVersion() { return cryptVersion; };
	void setCryptMode(UI08 mode);
	UI08 getCryptMode() { return crypt_mode; };
	virtual ~ClientCrypt();
	ClientCrypt();
	void init (unsigned char seed[4]);
	void ClientCrypt::init(UI32 pseed);
	void setCryptSeed(UI08 pseed[4]);
	void setCryptSeed(UI32 pseed);
	inline UI32 getCryptSeed()
	{ return (clientSeed[0]<< 24)+(clientSeed[1]<< 16)+(clientSeed[2]<< 8)+clientSeed[3]; };
	void setEntering(bool state);
	bool getEntering();
};

#endif
