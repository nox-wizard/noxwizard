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
\brief Item array stuff
*/

#ifndef __ITEM_ARRAY
#define __ITEM_ARRAY

#include "nxwcommn.h"
#include "debug.h"

// macros

#define MAKE_ITEM_REF(i) pointers::findItemBySerial(i)
#define LOG_INVALID_I_REF(err,meSSage) if(err=ItemArray::GetError()) { LogCritical("%s errorcode: %d\n", meSSage, err); }
#define GET_I_ERROR(err) err=ItemArray::GetError()

#define MAKE_ITEMREF_LOGGED(i,err) ItemArray::MakeItemref(i); err=ItemArray::GetError(); if (err) { BREAKPOINT; LogCritical( "Warning: a non-valid P_ITEM pointer was used in %s:%d\n", basename(__FILE__), __LINE__);  } 
#define MAKE_ITEMREF_C(i)			ItemArray::MakeItemref(i); int err=ItemArray::GetError(); if (err) { continue;}
#define MAKE_ITEMREF_LR(i)			ItemArray::MakeItemref(i); {int err=ItemArray::GetError(); if (err) { BREAKPOINT; LogCritical( "Warning: a non-valid P_ITEM pointer was used in %s:%d\n", basename(__FILE__), __LINE__);  return;} }
#define MAKE_ITEMREF_LRV(i,retval)	ItemArray::MakeItemref(i); {int err=ItemArray::GetError(); if (err) { BREAKPOINT; LogCritical( "Warning: a non-valid P_ITEM pointer was used in %s:%d\n", basename(__FILE__), __LINE__);  return retval;} }

#define DEREF_P_ITEM(pi) ( ( ISVALIDPI(pi) ) ? pi->getSerial32() : INVALID )

#define ISVALIDPI(pi) ( ( pi!=NULL && sizeof(*pi) == sizeof(cItem) ) ? (pi->getSerial32() >= 0) : false )
#define ISVALIDORNULLPI(pi) (ISVALIDPI(pi)||(pi==NULL))

#define VALIDATEPI(pi) if (!ISVALIDPI(pi)) { LogWarning("Warning: a non-valid P_ITEM pointer was used in %s:%d", basename(__FILE__), __LINE__); return; }
#define VALIDATEPIR(pi, r) if (!ISVALIDPI(pi)) { LogWarning("Warning: a non-valid P_ITEM pointer was used in %s:%d", basename(__FILE__), __LINE__); return r; }
#define VALIDATEPIORNULL(pi) if (!ISVALIDORNULLPI(pi)) return;
#define VALIDATEPIORNULLR(pi, r) if (!ISVALIDORNULLPI(pi)) return r;


// class definiton

namespace ItemArray
{
	extern LOGICAL error;

	inline int GetError() { return error; }
	P_ITEM MakeItemref(ITEM i);
};

#endif
