//////////////////////////////////////////////////////////////////////
//
// crypt.cpp
//
// Modified: 2001 Luke 'Infidel' Dunstan
//
// Author:  Beosil
// E-Mail:  beosil@swileys.com
// Version: 1.26.4
// Date:    27. Jan. 2000
//
// Copyright (C) 1999 Bruno 'Beosil' Heidelberger
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//
// History:
// -  8. Dec. 2001 : Added NewGameCrypt class to support 3.0.x clients
// - 27. Jan. 2000 : Keys updated for client 1.26.4
// - 18. Jan. 2000 : Keys updated for client 1.26.3
// - 23. Nov. 1999 : Keys updated for client 1.26.2 and some compatibility fixes
// - 21. Sep. 1999 : Full redesign to fix the "21036 bytes"-bug
// -  9. Sep. 1999 : Keys updated for client 1.26.1
// -  2. Sep. 1999 : Keys and boxes updated for client 1.26.0b and minor bugfixes
// - 13. Aug. 1999 : First release, working with client 1.26.0(a)
//
// To be continued ... =)
//
//////////////////////////////////////////////////////////////////////
#include "crypt.h"

#include "common.h"
#include "../console.h"




// Load long from address C into LL with bytes swapped.
#define N2L(C, LL) \
    LL  = ((unsigned int)(*((C)++))) << 24, \
    LL |= ((unsigned int)(*((C)++))) << 16, \
    LL |= ((unsigned int)(*((C)++))) << 8, \
    LL |= ((unsigned int)(*((C)++)))
// Store long LL at address C with bytes swapped.
#define L2N(LL, C) \
    *((C)++) = (unsigned char)(((LL) >> 24) & 0xff), \
    *((C)++) = (unsigned char)(((LL) >> 16) & 0xff), \
    *((C)++) = (unsigned char)(((LL) >> 8) & 0xff), \
    *((C)++) = (unsigned char)(((LL)) & 0xff)

//////////////////////////////////////////////////////////////////////

#if   defined(min_key)  && !defined(MIN_KEY)
#define MIN_KEY     1           /* toupper() */
#elif defined(part_key) && !defined(PART_KEY)
#define PART_KEY    1
#elif defined(zero_key) && !defined(ZERO_KEY)
#define ZERO_KEY    1
#endif


/*
+*****************************************************************************
*           Constants/Macros/Tables
-****************************************************************************/
#ifndef CONST
#define     CONST                   /* help syntax from C++, NOP here */
#endif
	        fullSbox MDStab;        /* not actually const.  Initialized ONE time */
int         needToBuildMDS=1;       /* is MDStab initialized yet? */

#define     BIG_TAB     0

#if BIG_TAB
BYTE        bigTab[4][256][256];    /* pre-computed S-box */
#endif

/* number of rounds for various key sizes:  128, 192, 256 */
/* (ignored for now in optimized code!) */
CONST int   numRounds[4]= {0,ROUNDS_128,ROUNDS_192,ROUNDS_256};

#if REENTRANT
#define     _sBox_   key->sBox8x32
#else
static      fullSbox _sBox_;        /* permuted MDStab based on keys */
#endif
#define _sBox8_(N) (((BYTE *) _sBox_) + (N)*256)

/*------- see what level of S-box precomputation we need to do -----*/
#if   defined(ZERO_KEY)
#define MOD_STRING  "(Zero S-box keying)"
#define Fe32_128(x,R)   \
    (   MDStab[0][p8(01)[p8(02)[_b(x,R  )]^b0(SKEY[1])]^b0(SKEY[0])] ^  \
        MDStab[1][p8(11)[p8(12)[_b(x,R+1)]^b1(SKEY[1])]^b1(SKEY[0])] ^  \
        MDStab[2][p8(21)[p8(22)[_b(x,R+2)]^b2(SKEY[1])]^b2(SKEY[0])] ^  \
        MDStab[3][p8(31)[p8(32)[_b(x,R+3)]^b3(SKEY[1])]^b3(SKEY[0])] )
#define Fe32_192(x,R)   \
    (   MDStab[0][p8(01)[p8(02)[p8(03)[_b(x,R  )]^b0(SKEY[2])]^b0(SKEY[1])]^b0(SKEY[0])] ^ \
        MDStab[1][p8(11)[p8(12)[p8(13)[_b(x,R+1)]^b1(SKEY[2])]^b1(SKEY[1])]^b1(SKEY[0])] ^ \
        MDStab[2][p8(21)[p8(22)[p8(23)[_b(x,R+2)]^b2(SKEY[2])]^b2(SKEY[1])]^b2(SKEY[0])] ^ \
        MDStab[3][p8(31)[p8(32)[p8(33)[_b(x,R+3)]^b3(SKEY[2])]^b3(SKEY[1])]^b3(SKEY[0])] )
#define Fe32_256(x,R)   \
    (   MDStab[0][p8(01)[p8(02)[p8(03)[p8(04)[_b(x,R  )]^b0(SKEY[3])]^b0(SKEY[2])]^b0(SKEY[1])]^b0(SKEY[0])] ^ \
        MDStab[1][p8(11)[p8(12)[p8(13)[p8(14)[_b(x,R+1)]^b1(SKEY[3])]^b1(SKEY[2])]^b1(SKEY[1])]^b1(SKEY[0])] ^ \
        MDStab[2][p8(21)[p8(22)[p8(23)[p8(24)[_b(x,R+2)]^b2(SKEY[3])]^b2(SKEY[2])]^b2(SKEY[1])]^b2(SKEY[0])] ^ \
        MDStab[3][p8(31)[p8(32)[p8(33)[p8(34)[_b(x,R+3)]^b3(SKEY[3])]^b3(SKEY[2])]^b3(SKEY[1])]^b3(SKEY[0])] )

#define GetSboxKey  DWORD SKEY[4];  /* local copy */ \
                    memcpy(SKEY,key->sboxKeys,sizeof(SKEY));
/*----------------------------------------------------------------*/
#elif defined(MIN_KEY)
#define MOD_STRING  "(Minimal keying)"
#define Fe32_(x,R)(MDStab[0][p8(01)[_sBox8_(0)[_b(x,R  )]] ^ b0(SKEY0)] ^ \
                   MDStab[1][p8(11)[_sBox8_(1)[_b(x,R+1)]] ^ b1(SKEY0)] ^ \
                   MDStab[2][p8(21)[_sBox8_(2)[_b(x,R+2)]] ^ b2(SKEY0)] ^ \
                   MDStab[3][p8(31)[_sBox8_(3)[_b(x,R+3)]] ^ b3(SKEY0)])
#define sbSet(N,i,J,v) { _sBox8_(N)[i+J] = v; }
#define GetSboxKey  DWORD SKEY0 = key->sboxKeys[0]      /* local copy */
/*----------------------------------------------------------------*/
#elif defined(PART_KEY) 
#define MOD_STRING  "(Partial keying)"
#define Fe32_(x,R)(MDStab[0][_sBox8_(0)[_b(x,R  )]] ^ \
                   MDStab[1][_sBox8_(1)[_b(x,R+1)]] ^ \
                   MDStab[2][_sBox8_(2)[_b(x,R+2)]] ^ \
                   MDStab[3][_sBox8_(3)[_b(x,R+3)]])
#define sbSet(N,i,J,v) { _sBox8_(N)[i+J] = v; }
#define GetSboxKey  
/*----------------------------------------------------------------*/
#else   /* default is FULL_KEY */
#ifndef FULL_KEY
#define FULL_KEY    1
#endif
#if BIG_TAB
#define TAB_STR     " (Big table)"
#else
#define TAB_STR
#endif
#ifdef COMPILE_KEY
#define MOD_STRING  "(Compiled subkeys)" TAB_STR
#else
#define MOD_STRING  "(Full keying)" TAB_STR
#endif
/* Fe32_ does a full S-box + MDS lookup.  Need to #define _sBox_ before use.
   Note that we "interleave" 0,1, and 2,3 to avoid cache bank collisions
   in optimized assembly language.
*/
#define Fe32_(x,R) (_sBox_[0][2*_b(x,R  )] ^ _sBox_[0][2*_b(x,R+1)+1] ^ \
                    _sBox_[2][2*_b(x,R+2)] ^ _sBox_[2][2*_b(x,R+3)+1])
        /* set a single S-box value, given the input byte */
#define sbSet(N,i,J,v) { _sBox_[N&2][2*i+(N&1)+2*J]=MDStab[N][v]; }
#define GetSboxKey  
#endif

// CONST       char *moduleDescription ="Optimized C ";
// CONST       char *modeString        =MOD_STRING;


/* macro(s) for debugging help */
#define     CHECK_TABLE     0       /* nonzero --> compare against "slow" table */
#define     VALIDATE_PARMS  0       /* disable for full speed */

/* end of debug macros */

#ifdef GetCodeSize
extern DWORD Here(DWORD x);         /* return caller's address! */
DWORD TwofishCodeStart(void) { return Here(0); }
#endif


//// Members of LoginCrypt class:

LoginCrypt::LoginCrypt()
{
	m_key[0]=0;
	m_key[1]=0;
}

LoginCrypt::~LoginCrypt()
{
}

void LoginCrypt::decrypt(unsigned char * in, unsigned char * out, int len)
{
    for(int i = 0; i < len; i++)
    {
        out[i] = in[i] ^ static_cast<unsigned char>(m_key[0]);

        unsigned int table0 = m_key[0];
        unsigned int table1 = m_key[1];

        m_key[1] =
            (
                (
                    (
                        ((table1 >> 1) | (table0 << 31))
                        ^ m_k1
                    )
                    >> 1
                )
                | (table0 << 31)
            ) ^ m_k1;
        m_key[0] = ((table0 >> 1) | (table1 << 31)) ^ m_k2;
    }
}
// private
// Used for both encryption and decryption
void LoginCrypt::encrypt(unsigned char * in, unsigned char * out, int len)
{
    for(int i = 0; i < len; i++)
    {
        out[i] = in[i] ^ static_cast<unsigned char>(m_key[0]);

        unsigned int table0 = m_key[0];
        unsigned int table1 = m_key[1];

        m_key[1] =
            (
                (
                    (
                        ((table1 >> 1) | (table0 << 31))
                        ^ m_k1
                    )
                    >> 1
                )
                | (table0 << 31)
            ) ^ m_k1;
        m_key[0] = ((table0 >> 1) | (table1 << 31)) ^ m_k2;
    }
}

void LoginCrypt::preview(unsigned char * in, unsigned char * out, int len)
{
	unsigned int old_mkey[2];
	old_mkey[0]=m_key[0];
	old_mkey[1]=m_key[1];
    for(int i = 0; i < len; i++)
    {
        out[i] = in[i] ^ static_cast<unsigned char>(m_key[0]);

        unsigned int table0 = m_key[0];
        unsigned int table1 = m_key[1];

        m_key[1] =
            (
                (
                    (
                        ((table1 >> 1) | (table0 << 31))
                        ^ m_k1
                    )
                    >> 1
                )
                | (table0 << 31)
            ) ^ m_k1;
        m_key[0] = ((table0 >> 1) | (table1 << 31)) ^ m_k2;
    }
	m_key[0]=old_mkey[0];
	m_key[1]=old_mkey[1];
}

unsigned char LoginCrypt::decryptByte(unsigned char in)
{
	return in ^ static_cast<unsigned char>(m_key[0]);
}


void LoginCrypt::init(unsigned char * pseed, unsigned int k1, unsigned int k2)
{
    unsigned int seed;
    N2L(pseed, seed);

    m_key[0] =
            (((~seed) ^ 0x00001357) << 16)
        |   ((seed ^ 0xffffaaaa) & 0x0000ffff);
    m_key[1] =
            ((seed ^ 0x43210000) >> 16)
        |   (((~seed) ^ 0xabcdffff) & 0xffff0000);

    m_k1 = k1;
    m_k2 = k2;
}

//////////////////////////////////////////////////////////////////////

//// Members of OldGameCrypt class:

// Hexadecimal digits of pi

static unsigned long p_box[18] =
{
    0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344, 0xa4093822, 0x299f31d0,
    0x082efa98, 0xec4e6c89, 0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
    0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917, 0x9216d5d9, 0x8979fb1b
};

static unsigned long s_box[4 * 256] =
{
    0xd1310ba6, 0x98dfb5ac, 0x2ffd72db, 0xd01adfb7,
    0xb8e1afed, 0x6a267e96, 0xba7c9045, 0xf12c7f99,
    0x24a19947, 0xb3916cf7, 0x0801f2e2, 0x858efc16,
    0x636920d8, 0x71574e69, 0xa458fea3, 0xf4933d7e,
    0x0d95748f, 0x728eb658, 0x718bcd58, 0x82154aee,
    0x7b54a41d, 0xc25a59b5, 0x9c30d539, 0x2af26013,
    0xc5d1b023, 0x286085f0, 0xca417918, 0xb8db38ef,
    0x8e79dcb0, 0x603a180e, 0x6c9e0e8b, 0xb01e8a3e,
    0xd71577c1, 0xbd314b27, 0x78af2fda, 0x55605c60,
    0xe65525f3, 0xaa55ab94, 0x57489862, 0x63e81440,
    0x55ca396a, 0x2aab10b6, 0xb4cc5c34, 0x1141e8ce,
    0xa15486af, 0x7c72e993, 0xb3ee1411, 0x636fbc2a,
    0x2ba9c55d, 0x741831f6, 0xce5c3e16, 0x9b87931e,
    0xafd6ba33, 0x6c24cf5c, 0x7a325381, 0x28958677,
    0x3b8f4898, 0x6b4bb9af, 0xc4bfe81b, 0x66282193,
    0x61d809cc, 0xfb21a991, 0x487cac60, 0x5dec8032,
    0xef845d5d, 0xe98575b1, 0xdc262302, 0xeb651b88,
    0x23893e81, 0xd396acc5, 0x0f6d6ff3, 0x83f44239,
    0x2e0b4482, 0xa4842004, 0x69c8f04a, 0x9e1f9b5e,
    0x21c66842, 0xf6e96c9a, 0x670c9c61, 0xabd388f0,
    0x6a51a0d2, 0xd8542f68, 0x960fa728, 0xab5133a3,
    0x6eef0b6c, 0x137a3be4, 0xba3bf050, 0x7efb2a98,
    0xa1f1651d, 0x39af0176, 0x66ca593e, 0x82430e88,
    0x8cee8619, 0x456f9fb4, 0x7d84a5c3, 0x3b8b5ebe,
    0xe06f75d8, 0x85c12073, 0x401a449f, 0x56c16aa6,
    0x4ed3aa62, 0x363f7706, 0x1bfedf72, 0x429b023d,
    0x37d0d724, 0xd00a1248, 0xdb0fead3, 0x49f1c09b,
    0x075372c9, 0x80991b7b, 0x25d479d8, 0xf6e8def7,
    0xe3fe501a, 0xb6794c3b, 0x976ce0bd, 0x04c006ba,
    0xc1a94fb6, 0x409f60c4, 0x5e5c9ec2, 0x196a2463,
    0x68fb6faf, 0x3e6c53b5, 0x1339b2eb, 0x3b52ec6f,
    0x6dfc511f, 0x9b30952c, 0xcc814544, 0xaf5ebd09,
    0xbee3d004, 0xde334afd, 0x660f2807, 0x192e4bb3,
    0xc0cba857, 0x45c8740f, 0xd20b5f39, 0xb9d3fbdb,
    0x5579c0bd, 0x1a60320a, 0xd6a100c6, 0x402c7279,
    0x679f25fe, 0xfb1fa3cc, 0x8ea5e9f8, 0xdb3222f8,
    0x3c7516df, 0xfd616b15, 0x2f501ec8, 0xad0552ab,
    0x323db5fa, 0xfd238760, 0x53317b48, 0x3e00df82,
    0x9e5c57bb, 0xca6f8ca0, 0x1a87562e, 0xdf1769db,
    0xd542a8f6, 0x287effc3, 0xac6732c6, 0x8c4f5573,
    0x695b27b0, 0xbbca58c8, 0xe1ffa35d, 0xb8f011a0,
    0x10fa3d98, 0xfd2183b8, 0x4afcb56c, 0x2dd1d35b,
    0x9a53e479, 0xb6f84565, 0xd28e49bc, 0x4bfb9790,
    0xe1ddf2da, 0xa4cb7e33, 0x62fb1341, 0xcee4c6e8,
    0xef20cada, 0x36774c01, 0xd07e9efe, 0x2bf11fb4,
    0x95dbda4d, 0xae909198, 0xeaad8e71, 0x6b93d5a0,
    0xd08ed1d0, 0xafc725e0, 0x8e3c5b2f, 0x8e7594b7,
    0x8ff6e2fb, 0xf2122b64, 0x8888b812, 0x900df01c,
    0x4fad5ea0, 0x688fc31c, 0xd1cff191, 0xb3a8c1ad,
    0x2f2f2218, 0xbe0e1777, 0xea752dfe, 0x8b021fa1,
    0xe5a0cc0f, 0xb56f74e8, 0x18acf3d6, 0xce89e299,
    0xb4a84fe0, 0xfd13e0b7, 0x7cc43b81, 0xd2ada8d9,
    0x165fa266, 0x80957705, 0x93cc7314, 0x211a1477,
    0xe6ad2065, 0x77b5fa86, 0xc75442f5, 0xfb9d35cf,
    0xebcdaf0c, 0x7b3e89a0, 0xd6411bd3, 0xae1e7e49,
    0x00250e2d, 0x2071b35e, 0x226800bb, 0x57b8e0af,
    0x2464369b, 0xf009b91e, 0x5563911d, 0x59dfa6aa,
    0x78c14389, 0xd95a537f, 0x207d5ba2, 0x02e5b9c5,
    0x83260376, 0x6295cfa9, 0x11c81968, 0x4e734a41,
    0xb3472dca, 0x7b14a94a, 0x1b510052, 0x9a532915,
    0xd60f573f, 0xbc9bc6e4, 0x2b60a476, 0x81e67400,
    0x08ba6fb5, 0x571be91f, 0xf296ec6b, 0x2a0dd915,
    0xb6636521, 0xe7b9f9b6, 0xff34052e, 0xc5855664,
    0x53b02d5d, 0xa99f8fa1, 0x08ba4799, 0x6e85076a,
    0x4b7a70e9, 0xb5b32944, 0xdb75092e, 0xc4192623,
    0xad6ea6b0, 0x49a7df7d, 0x9cee60b8, 0x8fedb266,
    0xecaa8c71, 0x699a17ff, 0x5664526c, 0xc2b19ee1,
    0x193602a5, 0x75094c29, 0xa0591340, 0xe4183a3e,
    0x3f54989a, 0x5b429d65, 0x6b8fe4d6, 0x99f73fd6,
    0xa1d29c07, 0xefe830f5, 0x4d2d38e6, 0xf0255dc1,
    0x4cdd2086, 0x8470eb26, 0x6382e9c6, 0x021ecc5e,
    0x09686b3f, 0x3ebaefc9, 0x3c971814, 0x6b6a70a1,
    0x687f3584, 0x52a0e286, 0xb79c5305, 0xaa500737,
    0x3e07841c, 0x7fdeae5c, 0x8e7d44ec, 0x5716f2b8,
    0xb03ada37, 0xf0500c0d, 0xf01c1f04, 0x0200b3ff,
    0xae0cf51a, 0x3cb574b2, 0x25837a58, 0xdc0921bd,
    0xd19113f9, 0x7ca92ff6, 0x94324773, 0x22f54701,
    0x3ae5e581, 0x37c2dadc, 0xc8b57634, 0x9af3dda7,
    0xa9446146, 0x0fd0030e, 0xecc8c73e, 0xa4751e41,
    0xe238cd99, 0x3bea0e2f, 0x3280bba1, 0x183eb331,
    0x4e548b38, 0x4f6db908, 0x6f420d03, 0xf60a04bf,
    0x2cb81290, 0x24977c79, 0x5679b072, 0xbcaf89af,
    0xde9a771f, 0xd9930810, 0xb38bae12, 0xdccf3f2e,
    0x5512721f, 0x2e6b7124, 0x501adde6, 0x9f84cd87,
    0x7a584718, 0x7408da17, 0xbc9f9abc, 0xe94b7d8c,
    0xec7aec3a, 0xdb851dfa, 0x63094366, 0xc464c3d2,
    0xef1c1847, 0x3215d908, 0xdd433b37, 0x24c2ba16,
    0x12a14d43, 0x2a65c451, 0x50940002, 0x133ae4dd,
    0x71dff89e, 0x10314e55, 0x81ac77d6, 0x5f11199b,
    0x043556f1, 0xd7a3c76b, 0x3c11183b, 0x5924a509,
    0xf28fe6ed, 0x97f1fbfa, 0x9ebabf2c, 0x1e153c6e,
    0x86e34570, 0xeae96fb1, 0x860e5e0a, 0x5a3e2ab3,
    0x771fe71c, 0x4e3d06fa, 0x2965dcb9, 0x99e71d0f,
    0x803e89d6, 0x5266c825, 0x2e4cc978, 0x9c10b36a,
    0xc6150eba, 0x94e2ea78, 0xa5fc3c53, 0x1e0a2df4,
    0xf2f74ea7, 0x361d2b3d, 0x1939260f, 0x19c27960,
    0x5223a708, 0xf71312b6, 0xebadfe6e, 0xeac31f66,
    0xe3bc4595, 0xa67bc883, 0xb17f37d1, 0x018cff28,
    0xc332ddef, 0xbe6c5aa5, 0x65582185, 0x68ab9802,
    0xeecea50f, 0xdb2f953b, 0x2aef7dad, 0x5b6e2f84,
    0x1521b628, 0x29076170, 0xecdd4775, 0x619f1510,
    0x13cca830, 0xeb61bd96, 0x0334fe1e, 0xaa0363cf,
    0xb5735c90, 0x4c70a239, 0xd59e9e0b, 0xcbaade14,
    0xeecc86bc, 0x60622ca7, 0x9cab5cab, 0xb2f3846e,
    0x648b1eaf, 0x19bdf0ca, 0xa02369b9, 0x655abb50,
    0x40685a32, 0x3c2ab4b3, 0x319ee9d5, 0xc021b8f7,
    0x9b540b19, 0x875fa099, 0x95f7997e, 0x623d7da8,
    0xf837889a, 0x97e32d77, 0x11ed935f, 0x16681281,
    0x0e358829, 0xc7e61fd6, 0x96dedfa1, 0x7858ba99,
    0x57f584a5, 0x1b227263, 0x9b83c3ff, 0x1ac24696,
    0xcdb30aeb, 0x532e3054, 0x8fd948e4, 0x6dbc3128,
    0x58ebf2ef, 0x34c6ffea, 0xfe28ed61, 0xee7c3c73,
    0x5d4a14d9, 0xe864b7e3, 0x42105d14, 0x203e13e0,
    0x45eee2b6, 0xa3aaabea, 0xdb6c4f15, 0xfacb4fd0,
    0xc742f442, 0xef6abbb5, 0x654f3b1d, 0x41cd2105,
    0xd81e799e, 0x86854dc7, 0xe44b476a, 0x3d816250,
    0xcf62a1f2, 0x5b8d2646, 0xfc8883a0, 0xc1c7b6a3,
    0x7f1524c3, 0x69cb7492, 0x47848a0b, 0x5692b285,
    0x095bbf00, 0xad19489d, 0x1462b174, 0x23820e00,
    0x58428d2a, 0x0c55f5ea, 0x1dadf43e, 0x233f7061,
    0x3372f092, 0x8d937e41, 0xd65fecf1, 0x6c223bdb,
    0x7cde3759, 0xcbee7460, 0x4085f2a7, 0xce77326e,
    0xa6078084, 0x19f8509e, 0xe8efd855, 0x61d99735,
    0xa969a7aa, 0xc50c06c2, 0x5a04abfc, 0x800bcadc,
    0x9e447a2e, 0xc3453484, 0xfdd56705, 0x0e1e9ec9,
    0xdb73dbd3, 0x105588cd, 0x675fda79, 0xe3674340,
    0xc5c43465, 0x713e38d8, 0x3d28f89e, 0xf16dff20,
    0x153e21e7, 0x8fb03d4a, 0xe6e39f2b, 0xdb83adf7,
    0xe93d5a68, 0x948140f7, 0xf64c261c, 0x94692934,
    0x411520f7, 0x7602d4f7, 0xbcf46b2e, 0xd4a20068,
    0xd4082471, 0x3320f46a, 0x43b7d4b7, 0x500061af,
    0x1e39f62e, 0x97244546, 0x14214f74, 0xbf8b8840,
    0x4d95fc1d, 0x96b591af, 0x70f4ddd3, 0x66a02f45,
    0xbfbc09ec, 0x03bd9785, 0x7fac6dd0, 0x31cb8504,
    0x96eb27b3, 0x55fd3941, 0xda2547e6, 0xabca0a9a,
    0x28507825, 0x530429f4, 0x0a2c86da, 0xe9b66dfb,
    0x68dc1462, 0xd7486900, 0x680ec0a4, 0x27a18dee,
    0x4f3ffea2, 0xe887ad8c, 0xb58ce006, 0x7af4d6b6,
    0xaace1e7c, 0xd3375fec, 0xce78a399, 0x406b2a42,
    0x20fe9e35, 0xd9f385b9, 0xee39d7ab, 0x3b124e8b,
    0x1dc9faf7, 0x4b6d1856, 0x26a36631, 0xeae397b2,
    0x3a6efa74, 0xdd5b4332, 0x6841e7f7, 0xca7820fb,
    0xfb0af54e, 0xd8feb397, 0x454056ac, 0xba489527,
    0x55533a3a, 0x20838d87, 0xfe6ba9b7, 0xd096954b,
    0x55a867bc, 0xa1159a58, 0xcca92963, 0x99e1db33,
    0xa62a4a56, 0x3f3125f9, 0x5ef47e1c, 0x9029317c,
    0xfdf8e802, 0x04272f70, 0x80bb155c, 0x05282ce3,
    0x95c11548, 0xe4c66d22, 0x48c1133f, 0xc70f86dc,
    0x07f9c9ee, 0x41041f0f, 0x404779a4, 0x5d886e17,
    0x325f51eb, 0xd59bc0d1, 0xf2bcc18f, 0x41113564,
    0x257b7834, 0x602a9c60, 0xdff8e8a3, 0x1f636c1b,
    0x0e12b4c2, 0x02e1329e, 0xaf664fd1, 0xcad18115,
    0x6b2395e0, 0x333e92e1, 0x3b240b62, 0xeebeb922,
    0x85b2a20e, 0xe6ba0d99, 0xde720c8c, 0x2da2f728,
    0xd0127845, 0x95b794fd, 0x647d0862, 0xe7ccf5f0,
    0x5449a36f, 0x877d48fa, 0xc39dfd27, 0xf33e8d1e,
    0x0a476341, 0x992eff74, 0x3a6f6eab, 0xf4f8fd37,
    0xa812dc60, 0xa1ebddf8, 0x991be14c, 0xdb6e6b0d,
    0xc67b5510, 0x6d672c37, 0x2765d43b, 0xdcd0e804,
    0xf1290dc7, 0xcc00ffa3, 0xb5390f92, 0x690fed0b,
    0x667b9ffb, 0xcedb7d9c, 0xa091cf0b, 0xd9155ea3,
    0xbb132f88, 0x515bad24, 0x7b9479bf, 0x763bd6eb,
    0x37392eb3, 0xcc115979, 0x8026e297, 0xf42e312d,
    0x6842ada7, 0xc66a2b3b, 0x12754ccc, 0x782ef11c,
    0x6a124237, 0xb79251e7, 0x06a1bbe6, 0x4bfb6350,
    0x1a6b1018, 0x11caedfa, 0x3d25bdd8, 0xe2e1c3c9,
    0x44421659, 0x0a121386, 0xd90cec6e, 0xd5abea2a,
    0x64af674e, 0xda86a85f, 0xbebfe988, 0x64e4c3fe,
    0x9dbc8057, 0xf0f7c086, 0x60787bf8, 0x6003604d,
    0xd1fd8346, 0xf6381fb0, 0x7745ae04, 0xd736fccc,
    0x83426b33, 0xf01eab71, 0xb0804187, 0x3c005e5f,
    0x77a057be, 0xbde8ae24, 0x55464299, 0xbf582e61,
    0x4e58f48f, 0xf2ddfda2, 0xf474ef38, 0x8789bdc2,
    0x5366f9c3, 0xc8b38e74, 0xb475f255, 0x46fcd9b9,
    0x7aeb2661, 0x8b1ddf84, 0x846a0e79, 0x915f95e2,
    0x466e598e, 0x20b45770, 0x8cd55591, 0xc902de4c,
    0xb90bace1, 0xbb8205d0, 0x11a86248, 0x7574a99e,
    0xb77f19b6, 0xe0a9dc09, 0x662d09a1, 0xc4324633,
    0xe85a1f02, 0x09f0be8c, 0x4a99a025, 0x1d6efe10,
    0x1ab93d1d, 0x0ba5a4df, 0xa186f20f, 0x2868f169,
    0xdcb7da83, 0x573906fe, 0xa1e2ce9b, 0x4fcd7f52,
    0x50115e01, 0xa70683fa, 0xa002b5c4, 0x0de6d027,
    0x9af88c27, 0x773f8641, 0xc3604c06, 0x61a806b5,
    0xf0177a28, 0xc0f586e0, 0x006058aa, 0x30dc7d62,
    0x11e69ed7, 0x2338ea63, 0x53c2dd94, 0xc2c21634,
    0xbbcbee56, 0x90bcb6de, 0xebfc7da1, 0xce591d76,
    0x6f05e409, 0x4b7c0188, 0x39720a3d, 0x7c927c24,
    0x86e3725f, 0x724d9db9, 0x1ac15bb4, 0xd39eb8fc,
    0xed545578, 0x08fca5b5, 0xd83d7cd3, 0x4dad0fc4,
    0x1e50ef5e, 0xb161e6f8, 0xa28514d9, 0x6c51133c,
    0x6fd5c7e7, 0x56e14ec4, 0x362abfce, 0xddc6c837,
    0xd79a3234, 0x92638212, 0x670efa8e, 0x406000e0,
    0x3a39ce37, 0xd3faf5cf, 0xabc27737, 0x5ac52d1b,
    0x5cb0679e, 0x4fa33742, 0xd3822740, 0x99bc9bbe,
    0xd5118e9d, 0xbf0f7315, 0xd62d1c7e, 0xc700c47b,
    0xb78c1b6b, 0x21a19045, 0xb26eb1be, 0x6a366eb4,
    0x5748ab2f, 0xbc946e79, 0xc6a376d2, 0x6549c2c8,
    0x530ff8ee, 0x468dde7d, 0xd5730a1d, 0x4cd04dc6,
    0x2939bbdb, 0xa9ba4650, 0xac9526e8, 0xbe5ee304,
    0xa1fad5f0, 0x6a2d519a, 0x63ef8ce2, 0x9a86ee22,
    0xc089c2b8, 0x43242ef6, 0xa51e03aa, 0x9cf2d0a4,
    0x83c061ba, 0x9be96a4d, 0x8fe51550, 0xba645bd6,
    0x2826a2f9, 0xa73a3ae1, 0x4ba99586, 0xef5562e9,
    0xc72fefd3, 0xf752f7da, 0x3f046f69, 0x77fa0a59,
    0x80e4a915, 0x87b08601, 0x9b09e6ad, 0x3b3ee593,
    0xe990fd5a, 0x9e34d797, 0x2cf0b7d9, 0x022b8b51,
    0x96d5ac3a, 0x017da67d, 0xd1cf3ed6, 0x7c7d2d28,
    0x1f9f25cf, 0xadf2b89b, 0x5ad6b472, 0x5a88f54c,
    0xe029ac71, 0xe019a5e6, 0x47b0acfd, 0xed93fa9b,
    0xe8d3c48d, 0x283b57cc, 0xf8d56629, 0x79132e28,
    0x785f0191, 0xed756055, 0xf7960e44, 0xe3d35e8c,
    0x15056dd4, 0x88f46dba, 0x03a16125, 0x0564f0bd,
    0xc3eb9e15, 0x3c9057a2, 0x97271aec, 0xa93a072a,
    0x1b3f6d9b, 0x1e6321f5, 0xf59c66fb, 0x26dcf319,
    0x7533d928, 0xb155fdf5, 0x03563482, 0x8aba3cbb,
    0x28517711, 0xc20ad9f8, 0xabcc5167, 0xccad925f,
    0x4de81751, 0x3830dc8e, 0x379d5862, 0x9320f991,
    0xea7a90c2, 0xfb3e7bce, 0x5121ce64, 0x774fbe32,
    0xa8b6e37e, 0xc3293d46, 0x48de5369, 0x6413e680,
    0xa2ae0810, 0xdd6db224, 0x69852dfd, 0x09072166,
    0xb39a460a, 0x6445c0dd, 0x586cdecf, 0x1c20c8ae,
    0x5bbef7dd, 0x1b588d40, 0xccd2017f, 0x6bb4e3bb,
    0xdda26a7e, 0x3a59ff45, 0x3e350a44, 0xbcb4cdd5,
    0x72eacea8, 0xfa6484bb, 0x8d6612ae, 0xbf3c6f47,
    0xd29be463, 0x542f5d9e, 0xaec2771b, 0xf64e6370,
    0x740e0d8d, 0xe75b1357, 0xf8721671, 0xaf537d5d,
    0x4040cb08, 0x4eb4e2cc, 0x34d2466a, 0x0115af84,
    0xe1b00428, 0x95983a1d, 0x06b89fb4, 0xce6ea048,
    0x6f3f3b82, 0x3520ab82, 0x011a1d4b, 0x277227f8,
    0x611560b1, 0xe7933fdc, 0xbb3a792b, 0x344525bd,
    0xa08839e1, 0x51ce794b, 0x2f32c9b7, 0xa01fbac9,
    0xe01cc87e, 0xbcc7d1f6, 0xcf0111c3, 0xa1e8aac7,
    0x1a908749, 0xd44fbd9a, 0xd0dadecb, 0xd50ada38,
    0x0339c32a, 0xc6913667, 0x8df9317c, 0xe0b12b4f,
    0xf79e59b7, 0x43f5bb3a, 0xf2d519ff, 0x27d9459c,
    0xbf97222c, 0x15e6fc2a, 0x0f91fc71, 0x9b941525,
    0xfae59361, 0xceb69ceb, 0xc2a86459, 0x12baa8d1,
    0xb6c1075e, 0xe3056a0c, 0x10d25065, 0xcb03a442,
    0xe0ec6e0e, 0x1698db3b, 0x4c98a0be, 0x3278e964,
    0x9f1f9532, 0xe0d392df, 0xd3a0342b, 0x8971f21e,
    0x1b0a7441, 0x4ba3348c, 0xc5be7120, 0xc37632d8,
    0xdf359f8d, 0x9b992f2e, 0xe60b6f47, 0x0fe3f11d,
    0xe54cda54, 0x1edad891, 0xce6279cf, 0xcd3e7e6f,
    0x1618b166, 0xfd2c1d05, 0x848fd2c5, 0xf6fb2299,
    0xf523f357, 0xa6327623, 0x93a83531, 0x56cccd02,
    0xacf08162, 0x5a75ebb5, 0x6e163697, 0x88d273cc,
    0xde966292, 0x81b949d0, 0x4c50901b, 0x71c65614,
    0xe6c6c7bd, 0x327a140a, 0x45e1d006, 0xc3f27b9a,
    0xc9aa53fd, 0x62a80f00, 0xbb25bfe2, 0x35bdd2f6,
    0x71126905, 0xb2040222, 0xb6cbcf7c, 0xcd769c2b,
    0x53113ec0, 0x1640e3d3, 0x38abbd60, 0x2547adf0,
    0xba38209c, 0xf746ce76, 0x77afa1c5, 0x20756060,
    0x85cbfe4e, 0x8ae88dd8, 0x7aaaf9b0, 0x4cf9aa7e,
    0x1948c25c, 0x02fb8a8c, 0x01c36ae4, 0xd6ebe1f9,
    0x90d4f869, 0xa65cdea0, 0x3f09252d, 0xc208e69f,
    0xb74e6132, 0xce77e25b, 0x578fdfe3, 0x3ac372e6
};

// Set of keys

static unsigned char g_key_table[CRYPT_GAMEKEY_COUNT][CRYPT_GAMEKEY_LENGTH] =
{
    { 0x91, 0x3C, 0x2B, 0x0F, 0x44, 0xC6 },
    { 0x0C, 0x96, 0xD2, 0x40, 0x93, 0x21 },
    { 0xF2, 0x12, 0xA5, 0xAA, 0xDA, 0xE9 },
    { 0x9A, 0xD4, 0xF7, 0x14, 0x97, 0xD0 },
    { 0xFC, 0xC9, 0xC7, 0xD6, 0xA8, 0xA3 },
    { 0x7B, 0x67, 0x36, 0x9B, 0x0B, 0x1A },
    { 0x03, 0xAC, 0xF9, 0x02, 0xAE, 0x2D },
    { 0x01, 0x77, 0x79, 0x6B, 0x0C, 0x67 },
    { 0xA4, 0xB4, 0x1E, 0xD7, 0xAA, 0x51 },
    { 0xD6, 0xE1, 0xBC, 0x27, 0x15, 0x25 },
    { 0x17, 0x17, 0x47, 0x65, 0x40, 0x8B },
    { 0xB8, 0x19, 0xDB, 0x4E, 0x17, 0x74 },
    { 0xAA, 0x63, 0xAC, 0x37, 0xA0, 0x8F },
    { 0x77, 0xCD, 0x5D, 0x23, 0xEF, 0xB7 },
    { 0x13, 0x2B, 0x83, 0xBF, 0x0F, 0x8C },
    { 0xB1, 0x0B, 0xC8, 0x6F, 0x39, 0x4D },
    { 0xA1, 0xA5, 0xFA, 0x2B, 0xC6, 0xE2 },
    { 0x9C, 0x29, 0xCC, 0x26, 0xE9, 0x2D },
    { 0xCD, 0x6F, 0xD2, 0xCA, 0xBE, 0x47 },
    { 0x9B, 0x21, 0xAE, 0x3E, 0x31, 0x69 },
    { 0xE7, 0x0B, 0xE6, 0x6F, 0xCF, 0x91 },
    { 0x88, 0x59, 0xAF, 0x90, 0xC5, 0x2D },
    { 0xAE, 0xD2, 0x52, 0xB5, 0x28, 0x98 },
    { 0x3B, 0x7F, 0x65, 0xED, 0x5E, 0x93 },
    { 0x30, 0xBF, 0x0A, 0x34, 0xDB, 0x3D }
};

// Seed Table

static unsigned char g_seed_table[2][CRYPT_GAMESEED_COUNT][2][CRYPT_GAMESEED_LENGTH] =
{
    {
        {
            { 0x9E, 0xEC, 0x5B, 0x3C, 0x8F, 0xA8, 0x8C, 0x55 },
            { 0xB6, 0x21, 0x71, 0x98, 0xA4, 0x47, 0x22, 0x58 }
        },

        {
            { 0xF8, 0xC4, 0xD8, 0x72, 0x54, 0xFC, 0xF9, 0xDE },
            { 0x2D, 0x53, 0xDB, 0x32, 0x03, 0x10, 0x5A, 0x18 }
        },

        {
            { 0x89, 0x9F, 0x5C, 0x53, 0x06, 0x7F, 0x44, 0x38 },
            { 0x32, 0xCE, 0xAC, 0xDB, 0x91, 0x44, 0x4E, 0x1E }
        },

        {
            { 0x29, 0x78, 0x5A, 0xF0, 0xAB, 0x00, 0x7F, 0x91 },
            { 0xE6, 0xB6, 0xD2, 0xE7, 0xA0, 0x05, 0xC2, 0xF2 }
        },

        {
            { 0x8D, 0x46, 0xA9, 0xBB, 0x52, 0x1B, 0x41, 0xDF },
            { 0xF0, 0x4A, 0xC9, 0x14, 0x27, 0xA9, 0x6B, 0x4A }
        },

        {
            { 0x91, 0x4B, 0x8A, 0x80, 0xF5, 0xCF, 0xBB, 0x3C },
            { 0xBC, 0xF4, 0xC9, 0xD5, 0x42, 0x7A, 0xFA, 0xB7 }
        },

        {
            { 0xD5, 0x8C, 0x01, 0xC0, 0xFD, 0x1E, 0xAA, 0x57 },
            { 0xC1, 0x20, 0x7A, 0x38, 0x2C, 0xB7, 0xCD, 0x14 }
        },

        {
            { 0x55, 0x9F, 0xD1, 0x5B, 0xFB, 0x70, 0xC0, 0x77 },
            { 0xA4, 0x15, 0xB3, 0x9F, 0x6B, 0xBB, 0x10, 0x5A }
        },

        {
            { 0x80, 0x9D, 0x16, 0x54, 0x6B, 0x7C, 0x5F, 0xAD },
            { 0x35, 0xCB, 0x92, 0x24, 0x08, 0x11, 0xD9, 0x61 }
        },

        {
            { 0x24, 0xA7, 0x75, 0xBF, 0x4D, 0x7E, 0x70, 0x0C },
            { 0x90, 0xCF, 0x9C, 0x04, 0xAC, 0x53, 0x89, 0xEF }
        },

        {
            { 0x99, 0x22, 0xF6, 0x89, 0x10, 0xE6, 0x72, 0x23 },
            { 0x0A, 0x5C, 0xA5, 0xFF, 0x9C, 0x78, 0xDA, 0x7F }
        },

        {
            { 0xDF, 0xFF, 0xBB, 0x11, 0x6B, 0x75, 0xF0, 0x29 },
            { 0xA5, 0x86, 0xD0, 0x53, 0x77, 0xE7, 0xB1, 0x0D }
        },

        {
            { 0x4C, 0x06, 0xDA, 0x55, 0x4E, 0x50, 0x1B, 0x7A },
            { 0x1C, 0x90, 0xCE, 0x64, 0xD6, 0x17, 0x52, 0xFB }
        },

        {
            { 0x00, 0x26, 0x75, 0x25, 0xCD, 0x95, 0x15, 0x0F },
            { 0x13, 0xD8, 0xAB, 0x30, 0xF1, 0xC5, 0xC5, 0xFA }
        },

        {
            { 0x0C, 0x8E, 0x86, 0x1E, 0x3F, 0xCB, 0x8B, 0xD1 },
            { 0xEC, 0xCE, 0xA9, 0x96, 0x91, 0x11, 0xB4, 0x97 }
        },

        {
            { 0x1E, 0x65, 0x5F, 0xA4, 0x55, 0xEB, 0xEC, 0xCF },
            { 0x19, 0xD9, 0x9F, 0xE0, 0x5E, 0x57, 0x45, 0x73 }
        },

        {
            { 0x0E, 0x2D, 0x18, 0xE1, 0x55, 0x05, 0x04, 0xBF },
            { 0x5E, 0x81, 0x1F, 0xDD, 0xFF, 0x5C, 0xC3, 0xF4 }
        },

        {
            { 0xF2, 0x06, 0x56, 0x54, 0x4D, 0xFB, 0x96, 0x54 },
            { 0x33, 0x97, 0x07, 0x43, 0x4F, 0x39, 0xC4, 0xA8 }
        },

        {
            { 0x5E, 0x02, 0x37, 0x17, 0x7B, 0x64, 0xE6, 0xA2 },
            { 0x2E, 0x24, 0x13, 0x07, 0xFE, 0xA1, 0x88, 0xB7 }
        },

        {
            { 0x60, 0xDD, 0x4C, 0xE0, 0xA1, 0xDC, 0xBA, 0x6C },
            { 0x81, 0x5C, 0x3F, 0x93, 0x7A, 0x1F, 0x2A, 0x1C }
        },

        {
            { 0xAE, 0x5C, 0xBE, 0x9D, 0x84, 0x6F, 0xCB, 0x51 },
            { 0x4D, 0x13, 0xC6, 0x81, 0x28, 0xC3, 0x03, 0x34 }
        },

        {
            { 0xB0, 0x5D, 0xCB, 0x8D, 0x69, 0x1C, 0xDE, 0x29 },
            { 0x31, 0xF1, 0x22, 0xC3, 0x1C, 0x82, 0x8A, 0x57 }
        },

        {
            { 0x08, 0x32, 0x8B, 0xA2, 0x1E, 0x12, 0xC9, 0xB9 },
            { 0xCD, 0xA8, 0xE6, 0x1C, 0x59, 0xAC, 0x0C, 0xF6 }
        },

        {
            { 0xA5, 0x3B, 0xE4, 0x64, 0x2F, 0x45, 0x33, 0xA2 },
            { 0x4A, 0xDA, 0x39, 0xE2, 0x0E, 0x94, 0xF2, 0xAA }
        },

        {
            { 0xB0, 0x82, 0xB7, 0x33, 0xD2, 0x6F, 0xC0, 0x00 },
            { 0xD7, 0x8D, 0x1F, 0x8E, 0x79, 0x85, 0x3E, 0x2A }
        }
    },

    {
        {
            { 0xD2, 0xB7, 0xF6, 0x9C, 0xCF, 0x06, 0xE8, 0xC1 },
            { 0xAE, 0xEB, 0x7F, 0xE9, 0x87, 0x28, 0x1C, 0x9B },
        },

        {
            { 0xE8, 0x8C, 0x2A, 0x97, 0xD1, 0xD2, 0xA6, 0x76 },
            { 0xAD, 0x23, 0x69, 0xA0, 0xEF, 0x1F, 0x8C, 0xBA },
        },

        {
            { 0x24, 0x62, 0x40, 0x0B, 0x21, 0xC6, 0x07, 0x89 },
            { 0xBA, 0x60, 0x9E, 0x26, 0x98, 0x18, 0xAF, 0x01 },
        },

        {
            { 0xDF, 0x2B, 0x56, 0xC9, 0xB3, 0x72, 0x35, 0x8D },
            { 0x1D, 0x4F, 0x61, 0xAF, 0x53, 0x12, 0x6E, 0x49 },
        },

        {
            { 0x1C, 0x87, 0x6C, 0xB1, 0xD4, 0x1B, 0xA2, 0xB2 },
            { 0xD4, 0xA1, 0x2C, 0xE2, 0x2F, 0xE9, 0xA4, 0x62 },
        },

        {
            { 0x17, 0x83, 0x1C, 0x68, 0xB3, 0xD6, 0x65, 0x2D },
            { 0x81, 0x5B, 0x4D, 0x9B, 0x15, 0x6F, 0x0B, 0xDF },
        },

        {
            { 0xCE, 0x91, 0xB9, 0x8A, 0x61, 0x20, 0xB1, 0xF9 },
            { 0xCA, 0x0A, 0xC4, 0x76, 0x5B, 0x4B, 0xAB, 0x16 },
        },

        {
            { 0x5B, 0xD2, 0x4A, 0xFD, 0x44, 0xB7, 0xDF, 0x1F },
            { 0x8B, 0x6F, 0xAB, 0x0C, 0xAB, 0x3D, 0x0C, 0x7A },
        },

        {
            { 0x35, 0x6C, 0xBD, 0xFF, 0x62, 0x53, 0x77, 0x44 },
            { 0xF2, 0x44, 0x5F, 0x8C, 0x59, 0x25, 0x5F, 0x6B },
        },

        {
            { 0xB5, 0x27, 0x0D, 0xD2, 0x23, 0xBE, 0x40, 0xB3 },
            { 0x3E, 0x8B, 0x92, 0xB1, 0x78, 0x57, 0xCB, 0xB0 },
        },

        {
            { 0xB3, 0xB4, 0xB6, 0xD5, 0xB6, 0xA7, 0x66, 0x6E },
            { 0xFB, 0xA7, 0x32, 0x93, 0xEE, 0x79, 0x61, 0x45 },
        },

        {
            { 0x49, 0xD7, 0x93, 0x34, 0x90, 0x1A, 0xAD, 0x2C },
            { 0x84, 0x3E, 0xE9, 0x0B, 0x2C, 0xC6, 0xB3, 0xB1 },
        },

        {
            { 0x82, 0xFB, 0x86, 0xEC, 0xA8, 0x76, 0x55, 0x98 },
            { 0x7E, 0xE3, 0xA2, 0x47, 0xB6, 0x72, 0x05, 0x61 },
        },

        {
            { 0x0B, 0xA5, 0x72, 0x17, 0xCB, 0x18, 0xAE, 0x03 },
            { 0x8C, 0x61, 0x32, 0xD9, 0x2B, 0x42, 0xEF, 0xF2 },
        },

        {
            { 0x3F, 0x0A, 0x06, 0x82, 0x09, 0xC9, 0x76, 0xF2 },
            { 0x3D, 0x54, 0x50, 0xFD, 0x25, 0xA2, 0x2F, 0x2E },
        },

        {
            { 0xF1, 0x34, 0x64, 0x94, 0xDC, 0x90, 0x58, 0x5D },
            { 0x1E, 0x6F, 0xB4, 0xEF, 0x73, 0xE8, 0xB0, 0xED },
        },

        {
            { 0xC0, 0xD2, 0xE1, 0x42, 0xEC, 0x04, 0x69, 0xA8 },
            { 0x27, 0x9C, 0x7C, 0x79, 0x87, 0x9A, 0xB2, 0x48 },
        },

        {
            { 0x50, 0x73, 0xEC, 0x1E, 0x4D, 0xD0, 0x80, 0x51 },
            { 0x46, 0x21, 0xC9, 0xF8, 0x93, 0xCC, 0xE8, 0x41 },
        },

        {
            { 0x70, 0xC9, 0xE4, 0x78, 0x8F, 0x6B, 0x2C, 0x27 },
            { 0x4C, 0x7E, 0x2C, 0x5A, 0x15, 0x69, 0x64, 0xDD },
        },

        {
            { 0x00, 0xC7, 0x09, 0xCD, 0xF6, 0x2D, 0x2D, 0x31 },
            { 0x6F, 0x01, 0x01, 0x3E, 0xCD, 0x60, 0x16, 0xB4 },
        },

        {
            { 0xE7, 0xE8, 0x76, 0xC4, 0x50, 0x4F, 0x08, 0x5B },
            { 0x62, 0x28, 0x24, 0x42, 0x7D, 0x9A, 0x19, 0x26 },
        },

        {
            { 0x2F, 0xD4, 0x67, 0xB9, 0x24, 0x0C, 0xBB, 0x14 },
            { 0x7D, 0x19, 0xC8, 0x73, 0x79, 0xA7, 0x70, 0xCF },
        },

        {
            { 0x2D, 0x53, 0xDC, 0x91, 0x83, 0xF2, 0x0C, 0x12 },
            { 0x3B, 0xAF, 0x1B, 0x6B, 0x02, 0x99, 0x8B, 0x61 },
        },

        {
            { 0xE3, 0x2C, 0xA2, 0x54, 0xCD, 0x51, 0xAF, 0xE5 },
            { 0x18, 0x58, 0x11, 0x7F, 0xF0, 0x50, 0x9C, 0x15 },
        },

        {
            { 0x6E, 0x26, 0x01, 0xE9, 0xDB, 0x50, 0x13, 0xEA },
            { 0x22, 0x59, 0x30, 0x3B, 0xE4, 0x5F, 0x43, 0x1E }
        }
    }
};

// Expanded Key Tables

static unsigned int p_table[CRYPT_GAMEKEY_COUNT][18];
static unsigned int s_table[CRYPT_GAMEKEY_COUNT][1024];

#define ROUND(LL, R, S, P) \
    LL = \
        (LL) \
        ^ (P) \
        ^ ( \
            ( \
                S[(R) >> 24] \
                + S[0x0100 + (((R) >> 16) & 0xff)] \
            ) ^ S[0x0200 + (((R) >> 8) & 0xff)] \
        ) + S[0x0300 + ((R) & 0xff)]


// static
bool OldGameCrypt::m_tables_ready = false;

OldGameCrypt::OldGameCrypt()
{
}

OldGameCrypt::~OldGameCrypt()
{
}

void OldGameCrypt::reinit(uint8 IP[4])
{
	// dummy function
}


// private
void OldGameCrypt::init_tables()
{
    for(int key_index = 0; key_index < CRYPT_GAMEKEY_COUNT; key_index++)
    {
        int i;
        // Initialise the P-array and S-boxes with the hex digits of pi
        memcpy(p_table[key_index], p_box, sizeof(p_box));
        memcpy(s_table[key_index], s_box, sizeof(s_box));

        unsigned char * pkey = g_key_table[key_index];
        unsigned char * pkey_end = g_key_table[key_index + 1];

        // XOR all bits of the key into the P-array until the entire P-array
        // has been processed (wrap at the end of the input key).
        for(i = 0; i < 18; i++)
        {
            unsigned int mask = *pkey++;
            if(pkey >= pkey_end)
                pkey = g_key_table[key_index];

            mask = ( mask << 8) | *pkey++;
            if(pkey >= pkey_end)
                pkey = g_key_table[key_index];

            mask = ( mask << 8) | *pkey++;
            if(pkey >= pkey_end)
                pkey = g_key_table[key_index];

            mask = ( mask << 8) | *pkey++;
            if(pkey >= pkey_end)
                pkey = g_key_table[key_index];

            p_table[key_index][i] ^= mask;
        }

        // Encrypt the all-zero string with Blowfish, using the key just
        // generated.
        unsigned int value[2] = { 0, 0 };

        for(i = 0; i < 18; i += 2)
        {
            raw_encrypt(value, key_index);
            // Replace P1 and P2 with the output of the encryption.
            p_table[key_index][i] = value[0];
            p_table[key_index][i+1] = value[1];
            // Repeat for the whole P-array
        }

        // Repeat with the S-boxes
        for(i = 0; i < 1024; i += 2)
        {
            raw_encrypt(value, key_index);
            s_table[key_index][i] = value[0];
            s_table[key_index][i+1] = value[1];
        }
    }

    m_tables_ready = true;
}

// private
void OldGameCrypt::raw_encrypt(unsigned int * values, int table)
{
    unsigned int left = values[0];
    unsigned int right = values[1];

    left ^= p_table[table][0];

    ROUND(right, left, s_table[table], p_table[table][1]);
    ROUND(left, right, s_table[table], p_table[table][2]);
    ROUND(right, left, s_table[table], p_table[table][3]);
    ROUND(left, right, s_table[table], p_table[table][4]);
    ROUND(right, left, s_table[table], p_table[table][5]);
    ROUND(left, right, s_table[table], p_table[table][6]);
    ROUND(right, left, s_table[table], p_table[table][7]);
    ROUND(left, right, s_table[table], p_table[table][8]);
    ROUND(right, left, s_table[table], p_table[table][9]);
    ROUND(left, right, s_table[table], p_table[table][10]);
    ROUND(right, left, s_table[table], p_table[table][11]);
    ROUND(left, right, s_table[table], p_table[table][12]);
    ROUND(right, left, s_table[table], p_table[table][13]);
    ROUND(left, right, s_table[table], p_table[table][14]);
    ROUND(right, left, s_table[table], p_table[table][15]);
    ROUND(left, right, s_table[table], p_table[table][16]);

    right ^= p_table[table][17];

    values[1] = left;
    values[0] = right;
}

void OldGameCrypt::decrypt(unsigned char * in, unsigned char * out, int len)
{
    while(m_stream_pos + len > CRYPT_GAMETABLE_TRIGGER)
    {
        int len_remaining = CRYPT_GAMETABLE_TRIGGER - m_stream_pos;

        encrypt(in, out, len_remaining);

        m_table_index = (m_table_index + CRYPT_GAMETABLE_STEP) %
            CRYPT_GAMETABLE_MODULO;
        memcpy(m_seed, g_seed_table[1][m_table_index][0],
            CRYPT_GAMESEED_LENGTH);
        m_stream_pos = 0;
        m_block_pos = 0;

        in += len_remaining;
        out += len_remaining;
        len -= len_remaining;
    }

    for(int i = 0; i < len; i++)
    {
	    if(m_block_pos == 0)
		{
			unsigned int values[2];

			unsigned char * seed = m_seed;
			N2L(seed, values[0]);
			N2L(seed, values[1]);

			raw_encrypt(values, m_table_index);

			seed = m_seed;
			L2N(values[0], seed);
			L2N(values[1], seed);
		}

/*
        // CFB (Cipher FeedBack) decrypt
        unsigned char c = *in++;
        *out++ = m_seed[m_block_pos] ^ c;
        unsigned char c = (*in++) ^ m_seed[m_block_pos];
        *out++ = c;
		
        m_seed[m_block_pos] = c;
        m_block_pos = (m_block_pos + 1) % 8;

*/
        // CFB (Cipher FeedBack) encrypt
        unsigned char c = in[i];
        out[i] = c  ^ m_seed[m_block_pos];
		m_seed[m_block_pos]=c;
        m_block_pos = (m_block_pos + 1) % 8;
    }

    m_stream_pos += len;
}

void OldGameCrypt::encrypt(unsigned char * in, unsigned char * out, int len)
{
	FILE *logfile = fopen("d:/temp/packet.log", "w");
	fprintf(logfile, "\n\n");
	short byteCount=0;
    while(m_stream_pos + len > CRYPT_GAMETABLE_TRIGGER)
    {
        int len_remaining = CRYPT_GAMETABLE_TRIGGER - m_stream_pos;

        encrypt(in, out, len_remaining);

        m_table_index = (m_table_index + CRYPT_GAMETABLE_STEP) %
            CRYPT_GAMETABLE_MODULO;
        memcpy(m_seed, g_seed_table[1][m_table_index][0],
            CRYPT_GAMESEED_LENGTH);
        m_stream_pos = 0;
        m_block_pos = 0;

        in += len_remaining;
        out += len_remaining;
        len -= len_remaining;
    }

    for(int i = 0; i < len; i++)
    {
	    if(m_block_pos == 0)
		{
			unsigned int values[2];

			unsigned char * seed = m_seed;
			N2L(seed, values[0]);
			N2L(seed, values[1]);

			raw_encrypt(values, m_table_index);

			seed = m_seed;
			L2N(values[0], seed);
			L2N(values[1], seed);
		}

/*
        // CFB (Cipher FeedBack) decrypt
        unsigned char c = *in++;
        *out++ = m_seed[m_block_pos] ^ c;
*/
        // CFB (Cipher FeedBack) encrypt
        // unsigned char c = (*in++) ^ m_seed[m_block_pos];
		fprintf(logfile, "%x ", in[i]);
		byteCount++;
		if ( byteCount % 32 == 0 )
		{
			fprintf(logfile, "\n");
			byteCount=0;
		}
		unsigned char c = in[i] ^ m_seed[m_block_pos];
        out[i] = c;

        m_seed[m_block_pos] = c;
        m_block_pos = (m_block_pos + 1) % 8;
    }
    m_stream_pos += len;
	
}

void OldGameCrypt::init()
{
    if(!m_tables_ready)
        init_tables();

    m_table_index = CRYPT_GAMETABLE_START;
    memcpy(m_seed, g_seed_table[0][m_table_index][0], CRYPT_GAMESEED_LENGTH);
    m_stream_pos = 0;
    m_block_pos = 0;
}


NewGameCrypt::NewGameCrypt(uint8 IP[4])
{
    // m_IP = (IP[0] << 24 ) + (IP[1] << 16 ) + ( IP[2] << 8 ) + IP[3];
	m_IP = (IP[3] << 24 ) + (IP[2] << 16 ) + ( IP[1] << 8 ) + IP[0];
}

void NewGameCrypt::reinit(uint8 IP[4])
{
    // m_IP = (IP[0] << 24 ) + (IP[1] << 16 ) + ( IP[2] << 8 ) + IP[3];
	m_IP = (IP[3] << 24 ) + (IP[2] << 16 ) + ( IP[1] << 8 ) + IP[0];
}

NewGameCrypt::~NewGameCrypt()
{
	m_IP = 0;

}

void NewGameCrypt::encrypt(unsigned char * in, unsigned char * out, int len)
{
    // NOTE: It seems this needs to change based on the m_seed (m_IP).
    // Only used in Ver 2.0.4 and above.

    // This table generated basec on DWORD id passed at start. 127.0.0.1
    static const BYTE sm_bData[] = { 0x05, 0x92, 0x66, 0x23, 0x67, 0x14, 0xE3,
        0x62, 0xDC, 0x60, 0x8C, 0xD6, 0xFE, 0x7C, 0x25, 0x69 };

    // @ 04264A5 in 2.0.4
    DWORD dwTmpIndex = dwIndex;
    for ( int i=0; i<len; i++ )
    {
        out [i] = in[i] ^ sm_bData[dwTmpIndex%16];
        dwTmpIndex++;
    }
    dwIndex = dwTmpIndex;
}

void NewGameCrypt::init()
{
    uint8 tmpBuff[0x100];

    memset(&decrypt_ki,0,sizeof(decrypt_ki));
    memset(&decrypt_ci,0,sizeof(decrypt_ci));

    int i;
    makeKey(&decrypt_ki,DIR_DECRYPT,0x80,NULL);

    cipherInit(&decrypt_ci,MODE_ECB,NULL);

    decrypt_ki.key32[0]=decrypt_ki.key32[1]=decrypt_ki.key32[2]=decrypt_ki.key32[3]=m_IP;
//0x0100007f or 0x8edb49ad or may be client_ip xor 0x8fdb49d2

    reKey(&decrypt_ki);
    for(i=0; i<256; i++)
        m_dec_subData3[i]=i;
    blockEncrypt(&decrypt_ci,&decrypt_ki,m_dec_subData3,256*8,tmpBuff);
    memcpy(m_dec_subData3,tmpBuff,256);

    m_pos_dec = 0;  
    dwIndex=0;

}

void NewGameCrypt::decrypt(unsigned char * in, unsigned char * out, int len)
{
    // NOTE: It seems this needs to change based on the m_seed (m_IP).
    // Only used in Ver 2.0.4 and above.

    // This table generated basec on DWORD id passed at start. 127.0.0.1
    uint8 tmpBuff[0x100];
    for(int i=0; i<len; i++)
    {
        if(m_pos_dec == 0x100)
        {
            blockEncrypt(&decrypt_ci, &decrypt_ki, m_dec_subData3, 0x800, tmpBuff);
            memcpy(m_dec_subData3, tmpBuff, 0x100);
            m_pos_dec = 0;
        }
        out[i] = in[i] ^ m_dec_subData3[m_pos_dec++];
    }
}

/*
+*****************************************************************************
*
* Function Name:    ParseHexDword
*
* Function:         Parse ASCII hex nibbles and fill in key/iv dwords
*
* Arguments:        bit         =   # bits to read
*                   srcTxt      =   ASCII source
*                   d           =   ptr to dwords to fill in
*                   dstTxt      =   where to make a copy of ASCII source
*                                   (NULL ok)
*
* Return:           Zero if no error.  Nonzero --> invalid hex or length
*
* Notes:  Note that the parameter d is a DWORD array, not a byte array.
*   This routine is coded to work both for little-endian and big-endian
*   architectures.  The character stream is interpreted as a LITTLE-ENDIAN
*   byte stream, since that is how the Pentium works, but the conversion
*   happens automatically below. 
*
-****************************************************************************/

int NewGameCrypt::ParseHexDword(int bits,CONST char *srcTxt,DWORD *d,char *dstTxt)
    {
    int i;
    char c;
    DWORD b;

    union   /* make sure LittleEndian is defined correctly */
        {
        BYTE  b[4];
        DWORD d[1];
        } v;
    v.d[0]=1;
    if (v.b[0 ^ ADDR_XOR] != 1)
        return BAD_ENDIAN;      /* make sure compile-time switch is set ok */

#if VALIDATE_PARMS
  #if ALIGN32
    if (((int)d) & 3)
        return BAD_ALIGN32; 
  #endif
#endif

    for (i=0;i*32<bits;i++)
        d[i]=0;                 /* first, zero the field */

    for (i=0;i*4<bits;i++)      /* parse one nibble at a time */
        {                       /* case out the hexadecimal characters */
        c=srcTxt[i];
        if (dstTxt) dstTxt[i]=c;
        if ((c >= '0') && (c <= '9'))
            b=c-'0';
        else if ((c >= 'a') && (c <= 'f'))
            b=c-'a'+10;
        else if ((c >= 'A') && (c <= 'F'))
            b=c-'A'+10;
        else
            return BAD_KEY_MAT; /* invalid hex character */
        /* works for big and little endian! */
        d[i/8] |= b << (4*((i^1)&7));       
        }

    return 0;                   /* no error */
    }


/*
+*****************************************************************************
*
* Function Name:    BuildMDS
*
* Function:         Initialize the MDStab array
*
* Arguments:        None.
*
* Return:           None.
*
* Notes:
*   Here we precompute all the fixed MDS table.  This only needs to be done
*   one time at initialization, after which the table is "CONST".
*
-****************************************************************************/
void NewGameCrypt::BuildMDS(void)
    {
    int i;
    DWORD d;
    BYTE m1[2],mX[2],mY[4];

    for (i=0;i<256;i++)
        {
        m1[0]=P8x8[0][i];       /* compute all the matrix elements */
        mX[0]=(BYTE) Mul_X(m1[0]);
        mY[0]=(BYTE) Mul_Y(m1[0]);

        m1[1]=P8x8[1][i];
        mX[1]=(BYTE) Mul_X(m1[1]);
        mY[1]=(BYTE) Mul_Y(m1[1]);

#undef  Mul_1                   /* change what the pre-processor does with Mij */
#undef  Mul_X
#undef  Mul_Y
#define Mul_1   m1              /* It will now access m01[], m5B[], and mEF[] */
#define Mul_X   mX              
#define Mul_Y   mY

#define SetMDS(N)                   \
        b0(d) = M0##N[P_##N##0];    \
        b1(d) = M1##N[P_##N##0];    \
        b2(d) = M2##N[P_##N##0];    \
        b3(d) = M3##N[P_##N##0];    \
        MDStab[N][i] = d;

        SetMDS(0);              /* fill in the matrix with elements computed above */
        SetMDS(1);
        SetMDS(2);
        SetMDS(3);
        }
#undef  Mul_1
#undef  Mul_X
#undef  Mul_Y
#define Mul_1   Mx_1            /* re-enable true multiply */
#define Mul_X   Mx_X
#define Mul_Y   Mx_Y
    
#if BIG_TAB
    {
    int j,k;
    BYTE *q0,*q1;

    for (i=0;i<4;i++)
        {
        switch (i)
            {
            case 0: q0=p8(01); q1=p8(02);   break;
            case 1: q0=p8(11); q1=p8(12);   break;
            case 2: q0=p8(21); q1=p8(22);   break;
            case 3: q0=p8(31); q1=p8(32);   break;
            }
        for (j=0;j<256;j++)
            for (k=0;k<256;k++)
                bigTab[i][j][k]=q0[q1[k]^j];
        }
    }
#endif

    needToBuildMDS=0;           /* NEVER modify the table again! */
    }

/*
+*****************************************************************************
*
* Function Name:    ReverseRoundSubkeys
*
* Function:         Reverse order of round subkeys to switch between encrypt/decrypt
*
* Arguments:        key     =   ptr to keyInstance to be reversed
*                   newDir  =   new direction value
*
* Return:           None.
*
* Notes:
*   This optimization allows both blockEncrypt and blockDecrypt to use the same
*   "fallthru" switch statement based on the number of rounds.
*   Note that key->numRounds must be even and >= 2 here.
*
-****************************************************************************/
void NewGameCrypt::ReverseRoundSubkeys(keyInstance *key,BYTE newDir)
    {
    DWORD t0,t1;
    register DWORD *r0=key->subKeys+ROUND_SUBKEYS;
    register DWORD *r1=r0 + 2*key->numRounds - 2;

    for (;r0 < r1;r0+=2,r1-=2)
        {
        t0=r0[0];           /* swap the order */
        t1=r0[1];
        r0[0]=r1[0];        /* but keep relative order within pairs */
        r0[1]=r1[1];
        r1[0]=t0;
        r1[1]=t1;
        }

    key->direction=newDir;
    }

/*
+*****************************************************************************
*
* Function Name:    Xor256
*
* Function:         Copy an 8-bit permutation (256 bytes), xoring with a byte
*
* Arguments:        dst     =   where to put result
*                   src     =   where to get data (can be same asa dst)
*                   b       =   byte to xor
*
* Return:           None
*
* Notes:
*   BorlandC's optimization is terrible!  When we put the code inline,
*   it generates fairly good code in the *following* segment (not in the Xor256
*   code itself).  If the call is made, the code following the call is awful!
*   The penalty is nearly 50%!  So we take the code size hit for inlining for
*   Borland, while Microsoft happily works with a call.
*
-****************************************************************************/
void NewGameCrypt::Xor256(void *dst,void *src,BYTE b)
    {
    register DWORD  x=b*0x01010101u;    /* replicate byte to all four bytes */
    register DWORD *d=(DWORD *)dst;
    register DWORD *s=(DWORD *)src;
#define X_8(N)  { d[N]=s[N] ^ x; d[N+1]=s[N+1] ^ x; }
#define X_32(N) { X_8(N); X_8(N+2); X_8(N+4); X_8(N+6); }
    X_32(0 ); X_32( 8); X_32(16); X_32(24); /* all inline */
    d+=32;  /* keep offsets small! */
    s+=32;
    X_32(0 ); X_32( 8); X_32(16); X_32(24); /* all inline */
    }

/*
+*****************************************************************************
*
* Function Name:    RS_MDS_encode
*
* Function:         Use (12,8) Reed-Solomon code over GF(256) to produce
*                   a key S-box dword from two key material dwords.
*
* Arguments:        k0  =   1st dword
*                   k1  =   2nd dword
*
* Return:           Remainder polynomial generated using RS code
*
* Notes:
*   Since this computation is done only once per reKey per 64 bits of key,
*   the performance impact of this routine is imperceptible. The RS code
*   chosen has "simple" coefficients to allow smartcard/hardware implementation
*   without lookup tables.
*
-****************************************************************************/
DWORD NewGameCrypt::RS_MDS_Encode(DWORD k0,DWORD k1)
    {
    int i,j;
    DWORD r;

    for (i=r=0;i<2;i++)
        {
        r ^= (i) ? k0 : k1;         /* merge in 32 more key bits */
        for (j=0;j<4;j++)           /* shift one byte at a time */
            RS_rem(r);              
        }
    return r;
    }


/*
+*****************************************************************************
*
* Function Name:    reKey
*
* Function:         Initialize the Twofish key schedule from key32
*
* Arguments:        key         =   ptr to keyInstance to be initialized
*
* Return:           TRUE on success
*
* Notes:
*   Here we precompute all the round subkeys, although that is not actually
*   required.  For example, on a smartcard, the round subkeys can 
*   be generated on-the-fly using f32()
*
-****************************************************************************/
int NewGameCrypt::reKey(keyInstance *key)
    {
    int     i,j,k64Cnt,keyLen;
    int     subkeyCnt;
    DWORD   A=0,B=0,q;
    DWORD   sKey[MAX_KEY_BITS/64],k32e[MAX_KEY_BITS/64],k32o[MAX_KEY_BITS/64];
    BYTE    L0[256],L1[256];    /* small local 8-bit permutations */

#if VALIDATE_PARMS
  #if ALIGN32
    if (((int)key) & 3)
        return BAD_ALIGN32;
    if ((key->keyLen % 64) || (key->keyLen < MIN_KEY_BITS))
        return BAD_KEY_INSTANCE;
  #endif
#endif

    if (needToBuildMDS)         /* do this one time only */
        BuildMDS();

#define F32(res,x,k32)  \
    {                                                           \
    DWORD t=x;                                                  \
    switch (k64Cnt & 3)                                         \
        {                                                       \
        case 0:  /* same as 4 */                                \
                    b0(t)   = p8(04)[b0(t)] ^ b0(k32[3]);       \
                    b1(t)   = p8(14)[b1(t)] ^ b1(k32[3]);       \
                    b2(t)   = p8(24)[b2(t)] ^ b2(k32[3]);       \
                    b3(t)   = p8(34)[b3(t)] ^ b3(k32[3]);       \
                 /* fall thru, having pre-processed t */        \
        case 3:     b0(t)   = p8(03)[b0(t)] ^ b0(k32[2]);       \
                    b1(t)   = p8(13)[b1(t)] ^ b1(k32[2]);       \
                    b2(t)   = p8(23)[b2(t)] ^ b2(k32[2]);       \
                    b3(t)   = p8(33)[b3(t)] ^ b3(k32[2]);       \
                 /* fall thru, having pre-processed t */        \
        case 2:  /* 128-bit keys (optimize for this case) */    \
            res=    MDStab[0][p8(01)[p8(02)[b0(t)] ^ b0(k32[1])] ^ b0(k32[0])] ^    \
                    MDStab[1][p8(11)[p8(12)[b1(t)] ^ b1(k32[1])] ^ b1(k32[0])] ^    \
                    MDStab[2][p8(21)[p8(22)[b2(t)] ^ b2(k32[1])] ^ b2(k32[0])] ^    \
                    MDStab[3][p8(31)[p8(32)[b3(t)] ^ b3(k32[1])] ^ b3(k32[0])] ;    \
        }                                                       \
    }


#if !CHECK_TABLE
#if defined(USE_ASM)                /* only do this if not using assember */
if (!(useAsm & 4))
#endif
#endif
    {
    subkeyCnt = ROUND_SUBKEYS + 2*key->numRounds;
    keyLen=key->keyLen;
    k64Cnt=(keyLen+63)/64;          /* number of 64-bit key words */
    for (i=0,j=k64Cnt-1;i<k64Cnt;i++,j--)
        {                           /* split into even/odd key dwords */
        k32e[i]=key->key32[2*i  ];
        k32o[i]=key->key32[2*i+1];
        /* compute S-box keys using (12,8) Reed-Solomon code over GF(256) */
        sKey[j]=key->sboxKeys[j]=RS_MDS_Encode(k32e[i],k32o[i]);    /* reverse order */
        }
    }

#ifdef USE_ASM
if (useAsm & 4)
    {
    #if defined(COMPILE_KEY) && defined(USE_ASM)
        key->keySig     = VALID_SIG;            /* show that we are initialized */
        key->codeSize   = sizeof(key->compiledCode);    /* set size */
    #endif
    reKey_86(key);
    }
else
#endif
    {
    for (i=q=0;i<subkeyCnt/2;i++,q+=SK_STEP)    
        {                           /* compute round subkeys for PHT */
        F32(A,q        ,k32e);      /* A uses even key dwords */
        F32(B,q+SK_BUMP,k32o);      /* B uses odd  key dwords */
        B = ROL(B,8);
        key->subKeys[2*i  ] = A+B;  /* combine with a PHT */
        B = A + 2*B;
        key->subKeys[2*i+1] = ROL(B,SK_ROTL);
        }
#if !defined(ZERO_KEY)
    switch (keyLen) /* case out key length for speed in generating S-boxes */
        {
        case 128:
        #if defined(FULL_KEY) || defined(PART_KEY)
#if BIG_TAB
            #define one128(N,J) sbSet(N,i,J,L0[i+J])
            #define sb128(N) {                      \
                BYTE *qq=bigTab[N][b##N(sKey[1])];  \
                Xor256(L0,qq,b##N(sKey[0]));        \
                for (i=0;i<256;i+=2) { one128(N,0); one128(N,1); } }
#else
            #define one128(N,J) sbSet(N,i,J,p8(N##1)[L0[i+J]]^k0)
            #define sb128(N) {                  \
                Xor256(L0,p8(N##2),b##N(sKey[1]));  \
                { register DWORD k0=b##N(sKey[0]);  \
                for (i=0;i<256;i+=2) { one128(N,0); one128(N,1); } } }
#endif
        #elif defined(MIN_KEY)
            #define sb128(N) Xor256(_sBox8_(N),p8(N##2),b##N(sKey[1]))
        #endif
            sb128(0); sb128(1); sb128(2); sb128(3);
            break;
        case 192:
        #if defined(FULL_KEY) || defined(PART_KEY)
            #define one192(N,J) sbSet(N,i,J,p8(N##1)[p8(N##2)[L0[i+J]]^k1]^k0)
            #define sb192(N) {                      \
                Xor256(L0,p8(N##3),b##N(sKey[2]));  \
                { register DWORD k0=b##N(sKey[0]);  \
                  register DWORD k1=b##N(sKey[1]);  \
                  for (i=0;i<256;i+=2) { one192(N,0); one192(N,1); } } }
        #elif defined(MIN_KEY)
            #define one192(N,J) sbSet(N,i,J,p8(N##2)[L0[i+J]]^k1)
            #define sb192(N) {                      \
                Xor256(L0,p8(N##3),b##N(sKey[2]));  \
                { register DWORD k1=b##N(sKey[1]);  \
                  for (i=0;i<256;i+=2) { one192(N,0); one192(N,1); } } }
        #endif
            sb192(0); sb192(1); sb192(2); sb192(3);
            break;
        case 256:
        #if defined(FULL_KEY) || defined(PART_KEY)
            #define one256(N,J) sbSet(N,i,J,p8(N##1)[p8(N##2)[L0[i+J]]^k1]^k0)
            #define sb256(N) {                                      \
                Xor256(L1,p8(N##4),b##N(sKey[3]));                  \
                for (i=0;i<256;i+=2) {L0[i  ]=p8(N##3)[L1[i]];      \
                                      L0[i+1]=p8(N##3)[L1[i+1]]; }  \
                Xor256(L0,L0,b##N(sKey[2]));                        \
                { register DWORD k0=b##N(sKey[0]);                  \
                  register DWORD k1=b##N(sKey[1]);                  \
                  for (i=0;i<256;i+=2) { one256(N,0); one256(N,1); } } }
        #elif defined(MIN_KEY)
            #define one256(N,J) sbSet(N,i,J,p8(N##2)[L0[i+J]]^k1)
            #define sb256(N) {                                      \
                Xor256(L1,p8(N##4),b##N(sKey[3]));                  \
                for (i=0;i<256;i+=2) {L0[i  ]=p8(N##3)[L1[i]];      \
                                      L0[i+1]=p8(N##3)[L1[i+1]]; }  \
                Xor256(L0,L0,b##N(sKey[2]));                        \
                { register DWORD k1=b##N(sKey[1]);                  \
                  for (i=0;i<256;i+=2) { one256(N,0); one256(N,1); } } }
        #endif
            sb256(0); sb256(1); sb256(2); sb256(3);
            break;
        }
#endif
    }

#if CHECK_TABLE                     /* sanity check  vs. pedagogical code*/
    {
    GetSboxKey;
    for (i=0;i<subkeyCnt/2;i++)
        {
        A = f32(i*SK_STEP        ,k32e,keyLen); /* A uses even key dwords */
        B = f32(i*SK_STEP+SK_BUMP,k32o,keyLen); /* B uses odd  key dwords */
        B = ROL(B,8);
        assert(key->subKeys[2*i  ] == A+  B);
        assert(key->subKeys[2*i+1] == ROL(A+2*B,SK_ROTL));
        }
  #if !defined(ZERO_KEY)            /* any S-boxes to check? */
    for (i=q=0;i<256;i++,q+=0x01010101)
        assert(f32(q,key->sboxKeys,keyLen) == Fe32_(q,0));
  #endif
    }
#endif /* CHECK_TABLE */


    if (key->direction == DIR_ENCRYPT)  
        ReverseRoundSubkeys(key,DIR_ENCRYPT);   /* reverse the round subkey order */

    return TRUE;
    }
/*
+*****************************************************************************
*
* Function Name:    makeKey
*
* Function:         Initialize the Twofish key schedule
*
* Arguments:        key         =   ptr to keyInstance to be initialized
*                   direction   =   DIR_ENCRYPT or DIR_DECRYPT
*                   keyLen      =   # bits of key text at *keyMaterial
*                   keyMaterial =   ptr to hex ASCII chars representing key bits
*
* Return:           TRUE on success
*                   else error code (e.g., BAD_KEY_DIR)
*
* Notes:    This parses the key bits from keyMaterial.  Zeroes out unused key bits
*
-****************************************************************************/
int NewGameCrypt::makeKey(keyInstance *key, BYTE direction, int keyLen,CONST char *keyMaterial)
    {
#if VALIDATE_PARMS              /* first, sanity check on parameters */
    if (key == NULL)            
        return BAD_KEY_INSTANCE;/* must have a keyInstance to initialize */
    if ((direction != DIR_ENCRYPT) && (direction != DIR_DECRYPT))
        return BAD_KEY_DIR;     /* must have valid direction */
    if ((keyLen > MAX_KEY_BITS) || (keyLen < 8) || (keyLen & 0x3F))
        return BAD_KEY_MAT;     /* length must be valid */
    key->keySig = VALID_SIG;    /* show that we are initialized */
  #if ALIGN32
    if ((((int)key) & 3) || (((int)key->key32) & 3))
        return BAD_ALIGN32;
  #endif
#endif

    key->direction  = direction;/* set our cipher direction */
    key->keyLen     = (keyLen+63) & ~63;        /* round up to multiple of 64 */
    key->numRounds  = numRounds[(keyLen-1)/64];
    memset(key->key32,0,sizeof(key->key32));    /* zero unused bits */
    key->keyMaterial[MAX_KEY_SIZE]=0;   /* terminate ASCII string */

    if ((keyMaterial == NULL) || (keyMaterial[0]==0))
        return TRUE;            /* allow a "dummy" call */
        
    if (ParseHexDword(keyLen,keyMaterial,key->key32,key->keyMaterial))
        return BAD_KEY_MAT; 

    return reKey(key);          /* generate round subkeys */
    }


/*
+*****************************************************************************
*
* Function Name:    cipherInit
*
* Function:         Initialize the Twofish cipher in a given mode
*
* Arguments:        cipher      =   ptr to cipherInstance to be initialized
*                   mode        =   MODE_ECB, MODE_CBC, or MODE_CFB1
*                   IV          =   ptr to hex ASCII test representing IV bytes
*
* Return:           TRUE on success
*                   else error code (e.g., BAD_CIPHER_MODE)
*
-****************************************************************************/
int NewGameCrypt::cipherInit(cipherInstance *cipher, BYTE mode,CONST char *IV)
    {
    int i;
#if VALIDATE_PARMS              /* first, sanity check on parameters */
    if (cipher == NULL)         
        return BAD_PARAMS;      /* must have a cipherInstance to initialize */
    if ((mode != MODE_ECB) && (mode != MODE_CBC) && (mode != MODE_CFB1))
        return BAD_CIPHER_MODE; /* must have valid cipher mode */
    cipher->cipherSig   =   VALID_SIG;
  #if ALIGN32
    if ((((int)cipher) & 3) || (((int)cipher->IV) & 3) || (((int)cipher->iv32) & 3))
        return BAD_ALIGN32;
  #endif
#endif

    if ((mode != MODE_ECB) && (IV)) /* parse the IV */
        {
        if (ParseHexDword(BLOCK_SIZE,IV,cipher->iv32,NULL))
            return BAD_IV_MAT;
        for (i=0;i<BLOCK_SIZE/32;i++)   /* make byte-oriented copy for CFB1 */
            ((DWORD *)cipher->IV)[i] = Bswap(cipher->iv32[i]);
        }

    cipher->mode        =   mode;

    return TRUE;
    }

/*
+*****************************************************************************
*
* Function Name:    blockEncrypt
*
* Function:         Encrypt block(s) of data using Twofish
*
* Arguments:        cipher      =   ptr to already initialized cipherInstance
*                   key         =   ptr to already initialized keyInstance
*                   input       =   ptr to data blocks to be encrypted
*                   inputLen    =   # bits to encrypt (multiple of blockSize)
*                   outBuffer   =   ptr to where to put encrypted blocks
*
* Return:           # bits ciphered (>= 0)
*                   else error code (e.g., BAD_CIPHER_STATE, BAD_KEY_MATERIAL)
*
* Notes: The only supported block size for ECB/CBC modes is BLOCK_SIZE bits.
*        If inputLen is not a multiple of BLOCK_SIZE bits in those modes,
*        an error BAD_INPUT_LEN is returned.  In CFB1 mode, all block 
*        sizes can be supported.
*
-****************************************************************************/
int NewGameCrypt::blockEncrypt(cipherInstance *cipher, keyInstance *key,CONST BYTE *input,
                int inputLen, BYTE *outBuffer)
    {
    int   i,n;                      /* loop counters */
    DWORD x[BLOCK_SIZE/32];         /* block being encrypted */
    DWORD t0,t1;                    /* temp variables */
    int   rounds=key->numRounds;    /* number of rounds */
    BYTE  bit,bit0,ctBit,carry;     /* temps for CFB */

    /* make local copies of things for faster access */
    int   mode = cipher->mode;
    DWORD sk[TOTAL_SUBKEYS];
    DWORD IV[BLOCK_SIZE/32];

    GetSboxKey;

#if VALIDATE_PARMS
    if ((cipher == NULL) || (cipher->cipherSig != VALID_SIG))
        return BAD_CIPHER_STATE;
    if ((key == NULL) || (key->keySig != VALID_SIG))
        return BAD_KEY_INSTANCE;
    if ((rounds < 2) || (rounds > MAX_ROUNDS) || (rounds&1))
        return BAD_KEY_INSTANCE;
    if ((mode != MODE_CFB1) && (inputLen % BLOCK_SIZE))
        return BAD_INPUT_LEN;
  #if ALIGN32
    if ( (((int)cipher) & 3) || (((int)key      ) & 3) ||
         (((int)input ) & 3) || (((int)outBuffer) & 3))
        return BAD_ALIGN32;
  #endif
#endif

    if (mode == MODE_CFB1)
        {   /* use recursion here to handle CFB, one block at a time */
        cipher->mode = MODE_ECB;    /* do encryption in ECB */
        for (n=0;n<inputLen;n++)
            {
            blockEncrypt(cipher,key,cipher->IV,BLOCK_SIZE,(BYTE *)x);
            bit0  = 0x80 >> (n & 7);/* which bit position in byte */
            ctBit = (input[n/8] & bit0) ^ ((((BYTE *) x)[0] & 0x80) >> (n&7));
            outBuffer[n/8] = (outBuffer[n/8] & ~ bit0) | ctBit;
            carry = ctBit >> (7 - (n&7));
            for (i=BLOCK_SIZE/8-1;i>=0;i--)
                {
                bit = cipher->IV[i] >> 7;   /* save next "carry" from shift */
                cipher->IV[i] = (cipher->IV[i] << 1) ^ carry;
                carry = bit;
                }
            }
        cipher->mode = MODE_CFB1;   /* restore mode for next time */
        return inputLen;
        }

    /* here for ECB, CBC modes */
    if (key->direction != DIR_ENCRYPT)
        ReverseRoundSubkeys(key,DIR_ENCRYPT);   /* reverse the round subkey order */

#ifdef USE_ASM
    if ((useAsm & 1) && (inputLen))
  #ifdef COMPILE_KEY
        if (key->keySig == VALID_SIG)
            return ((CipherProc *)(key->encryptFuncPtr))(cipher,key,input,inputLen,outBuffer);
  #else 
        return (*blockEncrypt_86)(cipher,key,input,inputLen,outBuffer);
  #endif
#endif
    /* make local copy of subkeys for speed */
    memcpy(sk,key->subKeys,sizeof(DWORD)*(ROUND_SUBKEYS+2*rounds));
    if (mode == MODE_CBC)
        BlockCopy(IV,cipher->iv32)
    else
        IV[0]=IV[1]=IV[2]=IV[3]=0;

    for (n=0;n<inputLen;n+=BLOCK_SIZE,input+=BLOCK_SIZE/8,outBuffer+=BLOCK_SIZE/8)
        {
#define LoadBlockE(N)  x[N]=Bswap(((DWORD *)input)[N]) ^ sk[INPUT_WHITEN+N] ^ IV[N]
        LoadBlockE(0);  LoadBlockE(1);  LoadBlockE(2);  LoadBlockE(3);
#define EncryptRound(K,R,id)    \
            t0     = Fe32##id(x[K  ],0);                    \
            t1     = Fe32##id(x[K^1],3);                    \
            x[K^3] = ROL(x[K^3],1);                         \
            x[K^2]^= t0 +   t1 + sk[ROUND_SUBKEYS+2*(R)  ]; \
            x[K^3]^= t0 + 2*t1 + sk[ROUND_SUBKEYS+2*(R)+1]; \
            x[K^2] = ROR(x[K^2],1);
#define     Encrypt2(R,id)  { EncryptRound(0,R+1,id); EncryptRound(2,R,id); }

#if defined(ZERO_KEY)
        switch (key->keyLen)
            {
            case 128:
                for (i=rounds-2;i>=0;i-=2)
                    Encrypt2(i,_128);
                break;
            case 192:
                for (i=rounds-2;i>=0;i-=2)
                    Encrypt2(i,_192);
                break;
            case 256:
                for (i=rounds-2;i>=0;i-=2)
                    Encrypt2(i,_256);
                break;
            }
#else
        Encrypt2(14,_);
        Encrypt2(12,_);
        Encrypt2(10,_);
        Encrypt2( 8,_);
        Encrypt2( 6,_);
        Encrypt2( 4,_);
        Encrypt2( 2,_);
        Encrypt2( 0,_);
#endif

        /* need to do (or undo, depending on your point of view) final swap */
#if LittleEndian
#define StoreBlockE(N)  ((DWORD *)outBuffer)[N]=x[N^2] ^ sk[OUTPUT_WHITEN+N]
#else
#define StoreBlockE(N)  { t0=x[N^2] ^ sk[OUTPUT_WHITEN+N]; ((DWORD *)outBuffer)[N]=Bswap(t0); }
#endif
        StoreBlockE(0); StoreBlockE(1); StoreBlockE(2); StoreBlockE(3);
        if (mode == MODE_CBC)
            {
            IV[0]=Bswap(((DWORD *)outBuffer)[0]);
            IV[1]=Bswap(((DWORD *)outBuffer)[1]);
            IV[2]=Bswap(((DWORD *)outBuffer)[2]);
            IV[3]=Bswap(((DWORD *)outBuffer)[3]);
            }
        }

    if (mode == MODE_CBC)
        BlockCopy(cipher->iv32,IV);

    return inputLen;
    }

/*
+*****************************************************************************
*
* Function Name:    blockDecrypt
*
* Function:         Decrypt block(s) of data using Twofish
*
* Arguments:        cipher      =   ptr to already initialized cipherInstance
*                   key         =   ptr to already initialized keyInstance
*                   input       =   ptr to data blocks to be decrypted
*                   inputLen    =   # bits to encrypt (multiple of blockSize)
*                   outBuffer   =   ptr to where to put decrypted blocks
*
* Return:           # bits ciphered (>= 0)
*                   else error code (e.g., BAD_CIPHER_STATE, BAD_KEY_MATERIAL)
*
* Notes: The only supported block size for ECB/CBC modes is BLOCK_SIZE bits.
*        If inputLen is not a multiple of BLOCK_SIZE bits in those modes,
*        an error BAD_INPUT_LEN is returned.  In CFB1 mode, all block 
*        sizes can be supported.
*
-****************************************************************************/
int NewGameCrypt::blockDecrypt(cipherInstance *cipher, keyInstance *key,CONST BYTE *input,
                int inputLen, BYTE *outBuffer)
    {
    int   i,n;                      /* loop counters */
    DWORD x[BLOCK_SIZE/32];         /* block being encrypted */
    DWORD t0,t1;                    /* temp variables */
    int   rounds=key->numRounds;    /* number of rounds */
    BYTE  bit,bit0,ctBit,carry;     /* temps for CFB */

    /* make local copies of things for faster access */
    int   mode = cipher->mode;
    DWORD sk[TOTAL_SUBKEYS];
    DWORD IV[BLOCK_SIZE/32];

    GetSboxKey;

#if VALIDATE_PARMS
    if ((cipher == NULL) || (cipher->cipherSig != VALID_SIG))
        return BAD_CIPHER_STATE;
    if ((key == NULL) || (key->keySig != VALID_SIG))
        return BAD_KEY_INSTANCE;
    if ((rounds < 2) || (rounds > MAX_ROUNDS) || (rounds&1))
        return BAD_KEY_INSTANCE;
    if ((cipher->mode != MODE_CFB1) && (inputLen % BLOCK_SIZE))
        return BAD_INPUT_LEN;
  #if ALIGN32
    if ( (((int)cipher) & 3) || (((int)key      ) & 3) ||
         (((int)input)  & 3) || (((int)outBuffer) & 3))
        return BAD_ALIGN32;
  #endif
#endif

    if (cipher->mode == MODE_CFB1)
        {   /* use blockEncrypt here to handle CFB, one block at a time */
        cipher->mode = MODE_ECB;    /* do encryption in ECB */
        for (n=0;n<inputLen;n++)
            {
            blockEncrypt(cipher,key,cipher->IV,BLOCK_SIZE,(BYTE *)x);
            bit0  = 0x80 >> (n & 7);
            ctBit = input[n/8] & bit0;
            outBuffer[n/8] = (outBuffer[n/8] & ~ bit0) |
                             (ctBit ^ ((((BYTE *) x)[0] & 0x80) >> (n&7)));
            carry = ctBit >> (7 - (n&7));
            for (i=BLOCK_SIZE/8-1;i>=0;i--)
                {
                bit = cipher->IV[i] >> 7;   /* save next "carry" from shift */
                cipher->IV[i] = (cipher->IV[i] << 1) ^ carry;
                carry = bit;
                }
            }
        cipher->mode = MODE_CFB1;   /* restore mode for next time */
        return inputLen;
        }

    /* here for ECB, CBC modes */
    if (key->direction != DIR_DECRYPT)
        ReverseRoundSubkeys(key,DIR_DECRYPT);   /* reverse the round subkey order */
#ifdef USE_ASM
    if ((useAsm & 2) && (inputLen))
  #ifdef COMPILE_KEY
        if (key->keySig == VALID_SIG)
            return ((CipherProc *)(key->decryptFuncPtr))(cipher,key,input,inputLen,outBuffer);
  #else 
        return (*blockDecrypt_86)(cipher,key,input,inputLen,outBuffer);
  #endif
#endif
    /* make local copy of subkeys for speed */
    memcpy(sk,key->subKeys,sizeof(DWORD)*(ROUND_SUBKEYS+2*rounds));
    if (mode == MODE_CBC)
        BlockCopy(IV,cipher->iv32)
    else
        IV[0]=IV[1]=IV[2]=IV[3]=0;

    for (n=0;n<inputLen;n+=BLOCK_SIZE,input+=BLOCK_SIZE/8,outBuffer+=BLOCK_SIZE/8)
        {
#define LoadBlockD(N) x[N^2]=Bswap(((DWORD *)input)[N]) ^ sk[OUTPUT_WHITEN+N]
        LoadBlockD(0);  LoadBlockD(1);  LoadBlockD(2);  LoadBlockD(3);

#define DecryptRound(K,R,id)                                \
            t0     = Fe32##id(x[K  ],0);                    \
            t1     = Fe32##id(x[K^1],3);                    \
            x[K^2] = ROL (x[K^2],1);                        \
            x[K^2]^= t0 +   t1 + sk[ROUND_SUBKEYS+2*(R)  ]; \
            x[K^3]^= t0 + 2*t1 + sk[ROUND_SUBKEYS+2*(R)+1]; \
            x[K^3] = ROR (x[K^3],1);                        \

#define     Decrypt2(R,id)  { DecryptRound(2,R+1,id); DecryptRound(0,R,id); }

#if defined(ZERO_KEY)
        switch (key->keyLen)
            {
            case 128:
                for (i=rounds-2;i>=0;i-=2)
                    Decrypt2(i,_128);
                break;
            case 192:
                for (i=rounds-2;i>=0;i-=2)
                    Decrypt2(i,_192);
                break;
            case 256:
                for (i=rounds-2;i>=0;i-=2)
                    Decrypt2(i,_256);
                break;
            }
#else
        {
        Decrypt2(14,_);
        Decrypt2(12,_);
        Decrypt2(10,_);
        Decrypt2( 8,_);
        Decrypt2( 6,_);
        Decrypt2( 4,_);
        Decrypt2( 2,_);
        Decrypt2( 0,_);
        }
#endif
        if (cipher->mode == MODE_ECB)
            {
#if LittleEndian
#define StoreBlockD(N)  ((DWORD *)outBuffer)[N] = x[N] ^ sk[INPUT_WHITEN+N]
#else
#define StoreBlockD(N)  { t0=x[N]^sk[INPUT_WHITEN+N]; ((DWORD *)outBuffer)[N] = Bswap(t0); }
#endif
            StoreBlockD(0); StoreBlockD(1); StoreBlockD(2); StoreBlockD(3);
#undef  StoreBlockD
            continue;
            }
        else
            {
#define StoreBlockD(N)  x[N]   ^= sk[INPUT_WHITEN+N] ^ IV[N];   \
                        IV[N]   = Bswap(((DWORD *)input)[N]);   \
                        ((DWORD *)outBuffer)[N] = Bswap(x[N]);
            StoreBlockD(0); StoreBlockD(1); StoreBlockD(2); StoreBlockD(3);
#undef  StoreBlockD
            }
        }
    if (mode == MODE_CBC)   /* restore iv32 to cipher */
        BlockCopy(cipher->iv32,IV)

    return inputLen;
    }

