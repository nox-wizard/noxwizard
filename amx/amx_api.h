#ifndef __AMX_API_H__
#define __AMX_API_H__
/*
    This file is part of NoX-Wizard
    -----------------------------------------------------------------------------
    Portions are copyright 2001, Marco Mastropaolo (Xanathar).

    NoX-Wizard is a modified version of Uox (and before that of wolfpack)
    so all previous copyright notices applies and above all

	Copyright 1997, 98 by Marcus Rating (Cironian)

    -----------------------------------------------------------------------------
    
    NoX-Wizard is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    NoX-Wizard is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with NoX-Wizard; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    -----------------------------------------------------------------------------

    NoX-Wizard also contains portions of code from the Small toolkit.

    The software toolkit "Small", the compiler, the abstract machine and the
    documentation, are copyright (c) 1997-2001 by ITB CompuPhase. 
	See amx_vm.c and/or small_license.txt for more information about this.

    ==============================================================================
*/

/*********************************************************************************

 PURPOSE OF THIS FILE :
	MOVE ALL NEEDED API CALLS/DEFINES/ETC OUTSIDE AMX_VM.H TO AVOID THE INFAMOUS
	#PRAGMA PACK GCC/G++ BUG :)

 *********************************************************************************/


#if !defined CELL_TYPE
  #define CELL_TYPE
  #if defined(BIT16)
    typedef unsigned short int ucell;     /* only for type casting */
    typedef short int cell;
  #else
    typedef unsigned long int ucell;
    typedef long int cell;
  #endif
#endif

typedef struct __amx AMX;


#define AMX_NATIVE_CALL
#define AMXAPI

typedef cell (AMX_NATIVE_CALL *AMX_NATIVE)(struct __amx *amx, cell *params);
typedef int (AMXAPI *AMX_CALLBACK)(struct __amx *amx, cell index,
                                   cell *result, cell *params);
typedef int (AMXAPI *AMX_DEBUG)(struct __amx *amx);

#include "amx.h"

extern "C" {
int AMXAPI amx_GetAddr(AMX *amx,cell amx_addr,cell **phys_addr);
int AMXAPI amx_GetString(char *dest,cell *source);
int AMXAPI amx_SetString(cell *dest, char *source, int pack);
int AMXAPI amx_SetUserData(AMX *amx, long tag, void *ptr);
int AMXAPI amx_StrLen(cell *cstring, int *length);
int printstring(AMX *amx,cell *cstr,cell *params,int num);

}



#endif //__AMX_API_H__
