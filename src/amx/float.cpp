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
 \brief Float arithmetic for the Small AMX engine
 \author Greg Garner <gmg@artran.com>

 Copyright &copy; Artran, Inc. 19999
 This file may be freely used. No warranties of any kind.
 */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>      /* for NULL */
#include <stdlib.h>     /* for atof() */
#include <math.h>
#include "amx.h"

#ifdef _DEBUG
//#define new DEBUG_NEW where is defined DEBUG_NEW ???
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*!
 Private function to allocate and fill a C style string from a small
 type string.
 */
static char *pcCreateAndFillStringFromCell(AMX *amx,cell params)
{
    char *szDest;
    int nLen;
    cell *pString;

    // Get the real address of the string.
    amx_GetAddr(amx,params,&pString);

    // Find out how long the string is in characters.
    amx_StrLen(pString, &nLen);
    szDest = new char[nLen+1];

    // Now convert the Small String into a C type null terminated string
    amx_GetString(szDest, pString);

    return szDest;
}

/******************************************************************/
static cell ConvertFloatToCell(float fValue)
{
    float *pFloat;
    cell fCell;

    // Get a pointer to the cell that is a float pointer.
    pFloat = (float *)((void *)&fCell);

    // Now put the float value into the cell.
    *pFloat = fValue;

    // Return the cell that contains the float
    return fCell;
}

/******************************************************************/
static float fConvertCellToFloat(cell cellValue)
{
    float *pFloat;

    // Get a pointer to the cell that is a float pointer.
    pFloat = (float *)((void *)&cellValue);

    // Return the float
    return *pFloat;
}


/******************************************************************/
static cell _float(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = long value to convert to a float
    */
    float fValue;

    // Convert to a float. Calls the compilers long to float conversion.
    fValue = (float) params[1];

    // Return the cell.
    return ConvertFloatToCell(fValue);
}

/******************************************************************/
static cell _floatstr(AMX *amx,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = virtual string address to convert to a float
    */
    char *szSource;
    float fNum;
    long lCells;

    lCells = params[0]/sizeof(cell);

    // They should have sent us 1 cell.
    assert(lCells==1);

    // Convert the Small string to a C style string.
    szSource = pcCreateAndFillStringFromCell(amx, params[1]);

    // Now convert this to a float.
    fNum = (float)atof(szSource);

    // Delete the string storage that was newed...
    delete[] szSource;

    return ConvertFloatToCell(fNum);
}

/******************************************************************/
static cell _floatmul(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fA, fB, fRes;

    fA = fConvertCellToFloat(params[1]);
    fB = fConvertCellToFloat(params[2]);
    fRes = fA * fB;

    return ConvertFloatToCell(fRes);
}

/******************************************************************/
static cell _floatdiv(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float dividend (top)
    *   params[2] = float divisor (bottom)
    */
    float fA, fB, fRes;

    fA = fConvertCellToFloat(params[1]);
    fB = fConvertCellToFloat(params[2]);
    fRes = fA / fB;

    return ConvertFloatToCell(fRes);
}

/******************************************************************/
static cell _floatadd(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fA, fB, fRes;

    fA = fConvertCellToFloat(params[1]);
    fB = fConvertCellToFloat(params[2]);
    fRes = fA + fB;

    return ConvertFloatToCell(fRes);
}

/******************************************************************/
static cell _floatsub(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fA, fB, fRes;

    fA = fConvertCellToFloat(params[1]);
    fB = fConvertCellToFloat(params[2]);
    fRes = fA - fB;

    return ConvertFloatToCell(fRes);
}

/******************************************************************/
//! Return fractional part of float
static cell _floatfract(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand
    */
    float fA;

    fA = fConvertCellToFloat(params[1]);
    fA = fA - (float)(floor((double)fA));

    return ConvertFloatToCell(fA);
}

/******************************************************************/
//! Return integer part of float, rounded
static cell _floatround(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand
    *   params[2] = Type of rounding (long)
    */
    float fA;

    fA = fConvertCellToFloat(params[1]);

    switch (params[2])
    {
        case 1:       /* round downwards (truncate) */
            fA = (float)(floor((double)fA));
            break;
        case 2:       /* round upwards */
            float fValue;
            fValue = (float)(floor((double)fA));
            if ( (fA>=0) && ((fA-fValue)!=0) )
                fValue++;
            fA = fValue;
            break;
        default:      /* standard, round to nearest */
            fA = (float)(floor((double)fA+.5));
            break;
    }

    return (long)fA;
}

/******************************************************************/
static cell _floatcmp(AMX *,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fA, fB;

    fA = fConvertCellToFloat(params[1]);
    fB = fConvertCellToFloat(params[2]);
    if (fA == fB)
        return 0;
    else if (fA>fB)
        return 1;
    else
        return -1;

}

extern "C"
{
	AMX_NATIVE_INFO float_Natives[] = {
  { "float",         _float },
  { "floatstr",      _floatstr },
  { "floatmul",      _floatmul },
  { "floatdiv",      _floatdiv },
  { "floatadd",      _floatadd },
  { "floatsub",      _floatsub },
  { "floatfract",    _floatfract},
  { "floatround",    _floatround},
  { "floatcmp",      _floatcmp},
  { NULL, NULL }        /* terminator */
};
}

