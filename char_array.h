  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\brief CharArray Namespace & macros

*/

#ifndef __CHAR_ARRAY
#define __CHAR_ARRAY

#include "nxwcommn.h" // needs global var schei___ and item_st *TYPE*
#include "debug.h"    // for logg macro

// macros

#define MAKE_CHAR_REF(i) pointers::findCharBySerial(i)
#define LOG_INVALID_C_REF(err,meSSage) if(err=CharArray::GetError()) { strcpy(schei___, meSSage); strcat(schei___," errorcode:%i\n"); LogCritical(schei___ _ err); } // strcpy stuff to prevent const string crashes
#define GET_C_ERROR(err) err=CharArray::GetError()

#define MAKE_CHARREF_LOGGED(i,err)  CharArray::MakeCharref(i); if (CharArray::GetError()) { BREAKPOINT; LogCritical("a non-valid P_CHAR pointer was used in %s:%d", basename(__FILE__), __LINE__); }
#define MAKE_CHARREF_C(i)	CharArray::MakeCharref(i); if (CharArray::GetError()) { continue;}
#define MAKE_CHARREF_LR(i)	CharArray::MakeCharref(i); {if (CharArray::GetError()) { BREAKPOINT; LogCritical("a non-valid P_CHAR pointer was used in %s:%d", basename(__FILE__), __LINE__); return;} }
#define MAKE_CHARREF_LRV(i,retval)	CharArray::MakeCharref(i); {if (CharArray::GetError()) { BREAKPOINT; LogCritical("a non-valid P_CHAR pointer was used in %s:%d", basename(__FILE__), __LINE__); return retval;} }

#define DEREF_P_CHAR(pc) ( ( ISVALIDPC(pc) ) ? pc->getSerial32() : INVALID )

#define ISVALIDPC(pc) ( ( pc!=NULL && sizeof(*pc) == sizeof(cChar) ) ? (pc->getSerial32() >= 0) : false )
#define ISVALIDORNULLPC(pc) (ISVALIDPC(pc)||(pc==NULL))

#define VALIDATEPC(pc) if (!ISVALIDPC(pc)) { LogWarning("a non-valid P_CHAR pointer was used in %s:%d", basename(__FILE__), __LINE__); return; }
#define VALIDATEPCR(pc, r) if (!ISVALIDPC(pc)) { LogWarning("a non-valid P_CHAR pointer was used in %s:%d", basename(__FILE__), __LINE__); return r; }
#define VALIDATEPCORNULL(pc) if (!ISVALIDORNULLPC(pc)) return;
#define VALIDATEPCORNULLR(pc, r) if (!ISVALIDORNULLPC(pc)) return r;
// class definiton

/*!
\brief CharArray Management stuff
*/
namespace CharArray
{
	extern LOGICAL error;
	P_CHAR MakeCharref(CHARACTER i);
	inline LOGICAL GetError() { return error; }
}

#endif

