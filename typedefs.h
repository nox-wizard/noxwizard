  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file typedefs.h
\brief contains typedefs for integer values and common structures.
\todo complete the documentation of types
*/

#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__
#include <set>


typedef double				R64;	//< 64 bits floating point
typedef float				R32;	//< 32 bits floating point

#if defined __GNUC__
	#include <bits/types.h>
	typedef uint8_t			UI08;
	typedef uint16_t		UI16;
	typedef uint32_t		UI32;
	typedef uint64_t		UI64;
	typedef int8_t			SI08;
	typedef int16_t			SI16;
	typedef int32_t			SI32;
	typedef int64_t			SI64;
#elif defined __BORLANDC__
	typedef unsigned __int8		UI08;
	typedef unsigned __int16	UI16;
	typedef unsigned __int32	UI32;
	typedef unsigned __int64	UI64;

	typedef signed __int8 SI08;
	typedef signed __int16 SI16;
	typedef signed __int32 SI32;
	typedef signed __int64 SI64;

	//typedef __int8			UI08;
	//typedef __int16			UI16;
	//typedef __int32			UI32;
	//typedef __int64			UI64;
#else
	#ifdef _MSC_VER
		#if _MSC_VER<=1200
			typedef unsigned long		UI64;
			typedef signed long		SI64;
		#else
			typedef unsigned __int64	UI64;
			typedef signed __int64		SI64;
		#endif
	#else
		typedef unsigned long long	UI64;
		typedef long long		SI64;
	#endif
	typedef unsigned int			UI32;		//!< unsigned 32 bits integer
	typedef signed int			SI32;		//!< signed 32 bits integer
	typedef unsigned short int		UI16;		//!< unsigned 16 bits integer
	typedef signed short int		SI16;		//!< signed 16 bits integer
	typedef unsigned char			UI08;		//!< unsigned 8 bits integer
	typedef signed char			SI08;		//!< signed 8 bits integer
#endif

typedef bool				LOGICAL;	//!< boolean type
typedef char				TEXT;		//!< character type
typedef UI08				ARMORCLASS;
typedef SI32				CHARACTER;
typedef UI16				COLOR;
typedef SI32				SOUND;

typedef SI32				ITEM;
typedef class cNxwClientObj*		NXWCLIENT;
typedef SI32				NXWSOCKET;	//!< socket connection
typedef unsigned char			BYTE;
typedef       class cChar*		P_CHAR;		//!< pointer to cChar
typedef const class cChar*		PC_CHAR;	//!< const pointer to cChar
typedef       class cItem*		P_ITEM;		//!< pointer to cItem
typedef const class cItem*		PC_ITEM;	//!< const pointer to cItem
typedef       class cObject*		P_OBJECT;	//!< pointer to cObject
typedef SI32				SERIAL;		//!< 32-bit serial number
typedef UI32				TIMERVAL;
typedef SI32				ACCOUNT;
typedef SI32				FUNCIDX;				
typedef std::vector<UI32>		UI32VECTOR;	//!< vector of unsigned 32 bits integers
typedef std::vector<SERIAL>		SERIAL_VECTOR;	//!< vector of serials
typedef slist<SERIAL>                   SERIAL_SLIST;	//!< slist of serials
typedef SERIAL 				SCRIPTID; 	//!< a script id
typedef std::set<SERIAL>		SERIAL_SET;	//!< set of serials
typedef std::vector< P_OBJECT >		POBJECT_VECTOR;	//!<  a vector of pointers to cObject
typedef POBJECT_VECTOR::iterator	POBJECT_VECTOR_IT;//!< an iterator to a vector of pointers to cObject
typedef std::vector< P_CHAR >		PCHAR_VECTOR;	//!< a vector of pointers to cChar
typedef PCHAR_VECTOR::iterator		PCHAR_VECTOR_IT;//!< an iterator to a vector of pointers to cChar
typedef std::vector< P_ITEM >		PITEM_VECTOR;	//!<  a vector of pointers to cItem
typedef PITEM_VECTOR::iterator		PITEM_VECTOR_IT;//!< an iterator to a vector of pointers to cChar
typedef class cPartyMember* P_PARTY_MEMBER;
typedef class cParty*	P_PARTY;
typedef class cBasicMenu* P_MENU;
typedef class cGuild* P_GUILD;
typedef class cGuildMember* P_GUILD_MEMBER;
typedef class cGuildRecruit* P_GUILD_RECRUIT;
typedef class cClient* P_CLIENT;







#if defined __GNUC__ && (__GNUC__ < 3 || defined(WIN32))
	typedef std::basic_string <wchar_t> wstring;
#else
	using std::wstring;
#endif

#ifdef __BORLANDC__
	#include <stlport/hash_map>
#else
	//#include <hashmap>
#endif



template < class T >
class cPoint{
public:

	cPoint( T a=0, T b=0 ) { this->a=a; this->b=b; }; 

	T a;
	T b;
};

/*! 
\brief Define a unsigned 32bit integer with 3 saved values
\author Anthalir
\since 0.82
*/
struct UI32_s {
	UI32	value;
	UI32	save1;
	UI32	save2;
	UI32	save3;
};

struct lookuptr_st //Tauriel  used to create pointers to the items dynamically allocated
{               //         so don't mess with it unless you know what you are doing!
  SI32 max;
  SI32 *pointer;
};

struct location_st
{
 SI32 x1;
 SI32 y1;
 SI32 x2;
 SI32 y2;
 //char region;
 UI08 region;
};

struct logout_st//Instalog
{
        UI32 x1;
        UI32 y1;
        UI32 x2;
        UI32 y2;
};

struct path_st {
	UI32 x;
	UI32 y;
};


//REPSYS
struct repsys_st
{
	SI32	murderdecay;
	UI32		maxkills;
	SI32		crimtime;
};


struct resource_st
{
	UI32		logs;
	TIMERVAL	logtime;
	TIMERVAL	lograte;//AntiChrist
	UI32		logarea;
	SI32		logtrigger;//AntiChrist
	SI32		logstamina;//AntiChrist
};


struct creat_st
{
	SI32 basesound;
	UI08 soundflag;
	UI08 who_am_i;
	SI32 icon;
};




// XYZZY
struct unitile_st
{
 SI08 basez;
 UI08 type; // 0=Terrain, 1=Item
 UI16 id;
 UI08 flag1;
 UI08 flag2;
 UI08 flag3;
 UI08 flag4;
 SI08 height;
 UI08 weight;
} PACK_NEEDED;








/*
	Sparhawk	-- moved to regions.h
	
struct region_st
{
	LOGICAL inUse;
 	TEXT name[50];
 	SI32 midilist;
 	UI08 priv;	// 0x01 guarded, 0x02, mark allowed, 0x04, gate allowed, 0x08, recall
			// 0x10 raining, 0x20, snowing, 0x40 magic damage reduced to 0
	TEXT guardowner[50];
	UI08 snowchance;
	UI08 rainchance;
	UI08 drychance;
	UI08 keepchance;
	UI08 wtype;
	SI32 guardnum[10];
	SI32 goodsell[256]; // Magius(CHE)
	SI32 goodbuy[256]; // Magius(CHE)
	SI32 goodrnd1[256]; // Magius(CHE) (2)
	SI32 goodrnd2[256]; // Magius(CHE) (2)
	SI32 forcedseason;
	LOGICAL ignoreseason;
};
*/

struct skill_st
{
 SI32 st;
 SI32 dx;
 SI32 in;
 SI32 unhide_onuse;
 SI32 unhide_onfail;
 SI32 advance_index;
 TEXT madeword[50]; // Added by Magius(CHE)
};

struct advance_st
{
 UI08 skill;
 SI32 base;
 SI32 success;
 SI32 failure;
};

#ifndef DOXYGEN	// this definitions are used when compiling

	struct Serial{
	#define ser1 aa._ser1
	#define ser2 aa._ser2
	#define ser3 aa._ser3
	#define ser4 aa._ser4
		union{
			struct { BYTE _ser4, _ser3, _ser2, _ser1; }aa;
			SI32 serial32;
		};
	};

	struct Color{
		union{
			UI16 color16;
			struct{ BYTE _ser2, _ser1; }aa;
		};
	};

#else			// these are only for doxygen to make a good documentation
	/*! 
	\brief Define a serial number
	\author Anthalir
	\since 0.82a
	\remarks each object in the UO world have a single serial
	*/
	struct Serial{
			BYTE ser1;			//!< byte 1
			BYTE ser2;			//!< byte 2
			BYTE ser3;			//!< byte 3
			BYTE ser4;			//!< byte 4
			SI32 serial32;		//!< full serial (32bits)
	};

	/*! 
	\brief Define a Color
	\author Anthalir
	\since 0.82
	*/
	struct Color{
			BYTE ser1;		//!< byte 1
			BYTE ser2;		//!< byte 2
			UI16 color16;		//!< full color (16bits)
	}
#endif // DOXYGEN

struct gmpage_st
{
	TEXT reason[80];
	Serial serial;
	TEXT timeofcall[9];
	TEXT name[20];
	SI32 handled;
} PACK_NEEDED;

struct begging_st
{
 SI32 timer;
 UI32 range;
 TEXT text[3][256];
};

struct fishing_st
{
 TIMERVAL basetime;
 TIMERVAL randomtime;
};

struct spiritspeak_st
{
 TIMERVAL spiritspeaktimer;
};

/*!
\brief struct for manage lag fix
\note extended by AntiChrist and Ripper
*/
struct speed_st
{
	SI32 nice;
	R32 itemtime;
	TIMERVAL srtime;
	R32 npctime;
	R32 tamednpctime;
	R32 npcfollowtime;
	R32 npcaitime;
};


/*!
\brief struct for manage custom titles
*/
struct title_st
{
 TEXT fame[50];
 TEXT skill[50];
 TEXT prowess[50];
 TEXT other[50];
};

/*! 
\brief Represent a point on the map
\author Anthalir
\since 0.82a
*/
struct Location{
	UI32			x, y;
	signed char		z, dispz;		// dispz is used for the char location
};


struct tele_locations_st {
	Location destination, origem;
};


/*! 
\brief Stat regeneration info
\author Endymion
\since 0.83
\note regeneration rage is in milliseconds
*/
typedef struct {
	TIMERVAL timer; //!< the timer
	UI32 rate_eff; //!< regeneration rate current
	UI32 rate_real; //!< regeneration rate real
} regen_st;

#endif //__TYPEDEFS_H__
