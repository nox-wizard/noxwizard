  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __NXW_UTILS_H__
#define __NXW_UTILS_H__
#include "nxwcommn.h"
#include "basics.h"


void setCrimGrey (P_CHAR pc, int mode);
void usehairdye(int s, P_ITEM pidye);

#ifdef WIN32
    char *getHKLMRegistryString(char *key, char *subkey);
    void setHKLMRegistryString(char *key, char *subkey, char *value);
    char *splitPath (char *p);
#endif

extern double g_fNatRate;
extern double g_fAmxRate;

#endif //__NXW_UTILS_H__
