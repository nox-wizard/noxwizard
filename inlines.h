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
\brief Inline functions of misc utility
*/

#ifndef __INLINES_H__
#define __INLINES_H__

#define TIMEOUT(X) (((X) <= uiCurrentTime) || overflow)
// Macros & Templates by Xan :

#define qmax(A,B) (((A)>(B)) ? (A) : (B))
#define qmin(A,B) (((A)<(B)) ? (A) : (B))

// this delete is safe from multiple deletes of same ptr
template<typename T> inline void safedelete(T*& p) { delete p; p = NULL; }
template<typename T> inline void safedeletearray(T*& p) { delete[] p; p = NULL; }
template<typename T> inline void qswap(T& a, T& b) { T dummy; dummy = a; a = b; b = dummy; }

#define charsysmsg(PC) if (PC->getClient()!=NULL) PC->getClient()->sysmsg

#define DBYTE2WORD(A,B) (((A)<<8) + ((B)&0xFF))
#define WORD2DBYTE1(A)  ((char)((A)>>8))
#define WORD2DBYTE2(A)  ((char)((A)&0xFF))
#define WORD2DBYTE(A,B,C) { B = WORD2DBYTE1(A); C = WORD2DBYTE2(A); }


inline bool chance(int percent) { return ( (rand()%100) < percent); }

inline int calcCharFromPtr(unsigned char *p)
{
	int serial;
	if((serial=LongFromCharPtr(p)) < 0) return -1;
	if (ISVALIDPC(pointers::findCharBySerial(serial))) return (DEREF_P_CHAR(pointers::findCharBySerial(serial)));
	else return (-1);
}

inline int calcItemFromPtr(unsigned char *p)
{
	int serial;
	if((serial=LongFromCharPtr(p)) < 0) return -1;
	if (ISVALIDPI(pointers::findItemBySerial(serial))) return (DEREF_P_ITEM(pointers::findItemBySerial(serial)));
	else return (-1);
}

inline int calcItemFromSer(unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4)
{
	if((s1==255) && (s2==255) && (s3==255) && (s4==255) ) return -1;//AntiChrist
	int serial = calcserial(s1, s2, s3, s4);
	if (ISVALIDPI(pointers::findItemBySerial(serial))) return (DEREF_P_ITEM(pointers::findItemBySerial(serial)));
	else return (-1);
}

inline int calcItemFromSer(int ser) // Aded by Magius(CHE) (2)
{
	if (ISVALIDPI(pointers::findItemBySerial(ser))) return (DEREF_P_ITEM(pointers::findItemBySerial(ser)));
	else return (-1);
}

//Modified by Luxor for the erasing of charcount
inline NXWSOCKET  calcSocketFromChar(CHARACTER i)
{
        P_CHAR pc = MAKE_CHAR_REF(i);
        if (!ISVALIDPC(pc)) return INVALID;

        if (pc->npc) return INVALID;

		NXWCLIENT cli = pc->getClient();
        if (cli == NULL) return INVALID;

        return cli->toInt();

}

inline int calcCharFromSer(unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4)
{
	int serial;

	serial = calcserial(s1, s2, s3, s4);
	if (ISVALIDPC(pointers::findCharBySerial(serial))) return(DEREF_P_CHAR(pointers::findCharBySerial(serial)));
	else return(-1);
}

inline int calcCharFromSer(int serial)
{
	if (ISVALIDPC(pointers::findCharBySerial(serial))) return (DEREF_P_CHAR(pointers::findCharBySerial(serial)));
	else return (-1);
}

inline void SetTimerSec( TIMERVAL *timer, const short seconds)
{
	*timer=seconds * MY_CLOCKS_PER_SEC + uiCurrentTime;
}

inline bool isCharSerial(long ser) {return (ser >= 0 && ser <  0x40000000);}
inline bool isItemSerial(long ser) {return (/*ser >= 0 && */ser >= 0x40000000);}



#define SETSOCK(A) g_nCurrentSocket = A;



#endif //__INLINES_H__
