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

inline int calcserial(unsigned char a1,unsigned char a2,unsigned char a3,unsigned char a4) {return (static_cast<int>((a1<<24))|static_cast<int>((a2<<16)) | static_cast<int>((a3<<8)) | static_cast<int>(a4));}

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

/*inline int calcItemFromSer(unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4)
{
	if((s1==255) && (s2==255) && (s3==255) && (s4==255) ) return -1;//AntiChrist
	int serial = calcserial(s1, s2, s3, s4);
	if (ISVALIDPI(pointers::findItemBySerial(serial))) return (DEREF_P_ITEM(pointers::findItemBySerial(serial)));
	else return (-1);
}*/

inline int calcItemFromSer(int ser) // Aded by Magius(CHE) (2)
{
	if (ISVALIDPI(pointers::findItemBySerial(ser))) return (DEREF_P_ITEM(pointers::findItemBySerial(ser)));
	else return (-1);
}

//Modified by Luxor for the erasing of charcount
/*inline NXWSOCKET  calcSocketFromChar(CHARACTER i)
{
        P_CHAR pc = MAKE_CHAR_REF(i);
        if (!ISVALIDPC(pc)) return INVALID;

        if (pc->npc) return INVALID;

		NXWCLIENT cli = pc->getClient();
        if (cli == NULL) return INVALID;

        return cli->toInt();

}*/

/*inline int calcCharFromSer(unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4)
{
	int serial;

	serial = calcserial(s1, s2, s3, s4);
	if (ISVALIDPC(pointers::findCharBySerial(serial))) return(DEREF_P_CHAR(pointers::findCharBySerial(serial)));
	else return(-1);
}*/

inline int calcCharFromSer(int serial)
{
	if (ISVALIDPC(pointers::findCharBySerial(serial))) return (DEREF_P_CHAR(pointers::findCharBySerial(serial)));
	else return (-1);
}

inline void SetTimerSec( TIMERVAL &timer, const SI16 seconds)
{
	timer=seconds * MY_CLOCKS_PER_SEC + uiCurrentTime;
}

inline bool isCharSerial(SERIAL ser) {return (ser >= 0 && ser <  0x40000000);}
inline bool isItemSerial(SERIAL ser) {return (ser >= 0x40000000);}

#define SETSOCK(A) g_nCurrentSocket = A;

inline std::string toString(int value)
{
	char s[21];
	snprintf(s, 20, "%d", value);
	s[19]=0x00;

	return std::string(s);
}

inline std::string toString(double value)
{
	char s[21];
	snprintf(s, 20, "%f", value);
	s[19]=0x00;

	return std::string(s);
}

/*
\brief Convert a string to an wstring
\author Endymion
\param from the source string
\param to the dest wstring
*/
inline void string2wstring( string& from, wstring& to )
{
	to.erase();
	string::iterator iter( from.begin() ), end( from.end() );
	for( ; iter!=end; iter++ ) {
		to+=static_cast<wchar_t>(*iter);
	}
}

/*
\brief Convert a wstring to an string
\author Endymion
\param from the source wstring
\param to the dest string
*/
inline void wstring2string( wstring& from, string& to )
{
	to.erase();
	wstring::iterator iter( from.begin() ), end( from.end() );
	for( ; iter!=end; iter++ ) {
		to+=static_cast<char>(*iter);
	}
}

/*!
\brief play sound
\param goldtotal the amount of gold to "move"
\return soundsfx to play
*/
inline UI16 goldsfx(UI08 goldtotal)
{
	if (goldtotal==1) 
		return 0x0035;
	else if (goldtotal<6)
		return 0x0036;
	else 
		return 0x0037;
}

/*!
\brief play a sound based on item id

added to do easy item sound effects based on an
items id1 and id2 fields in struct items. Then just define the CASE statement
with the proper sound function to play for a certain item as shown.

\author Dupois Duke
\param item the item
\return soundfx for the item
\remarks \remark Use the DEFAULT case for ranges of items (like all ingots make the same thump).
		 \remark Sounds: 
			\li coins dropping (all the same no matter what amount because all id's equal 0x0EED
			\li ingot dropping (makes a big thump - used the heavy gem sound)
			\li gems dropping (two type broke them in half to make some sound different then others)
*/
inline UI16 itemsfx(UI16 item)
{
	if( item == ITEMID_GOLD )
		return goldsfx(2);
	else if( (item>=0x0F0F) && (item<=0x0F20) )	// Any gem stone (typically smaller)
		return 0x0032;
	else if( (item>=0x0F21) && (item<=0x0F30) )	// Any gem stone (typically larger)
		return 0x0034;
	else if( (item>=0x1BE3) && (item<=0x1BFA) )	// Any Ingot
		return 0x0033;
	else
		return 0x0042;
}

#endif //__INLINES_H__
