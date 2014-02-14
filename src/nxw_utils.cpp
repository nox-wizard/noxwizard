  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "basics.h"

#include "cmdtable.h"
#include "speech.h"
#include "sndpkg.h"
#include "sregions.h"
#include "srvparms.h"
#include "nxw_utils.h"
#include "layer.h"
#include "set.h"
#include "items.h"
#include "chars.h"
#include "nox-wizard.h"


#ifdef WIN32
#include <process.h>
#endif
#ifdef PTHREADS
#include <pthread.h>
#endif


//xan : until we'll have to port code... keep it little-endian by default
#ifndef LITTLE_ENDIAN
	#define LITTLE_ENDIAN
#endif

int g_nNoXRating = 0;

double g_fNatRate = 0.0;
double g_fAmxRate = 0.0;

///////////////////////////////////////////////////////////////////
// Function name     : setCrimGrey 
// Description       : sets criminal or grey depending on a server.cfg setting
// Return type       : void 
// Author            : Xanathar 
// Argument          : int chr -> charachter to be flagged
// Argument          : int mode -> the server.cfg setting
// Changes           : none yet
void setCrimGrey(P_CHAR pc, int mode)
{
	VALIDATEPC(pc);
	switch(mode)
	{
	case 1:
		pc->SetGrey();
		break;
	case 2:
		criminal(pc);
		break;
	default:
		break;
	}
}


/*******************************************************************************************
 THIS PORTION IS ABOUT WIN REGISTRY KEYS
 *******************************************************************************************/
#ifdef WIN32

///////////////////////////////////////////////////////////////////
// Function name     : getHKLMRegistryString
// Description       : gets a string value from the HKEY_LOCAL_MACHINE registry
// Return type       : char* 
// Author            : Xanathar 
// Argument          : char *key -> the key name
// Argument          : char *subkey -> subkey name
// Changes           : none yet
char* getHKLMRegistryString(char *key, char *subkey)
{
	// Get values from registry, use REGEDIT to see how data is stored 
	HKEY hTestKey;
	DWORD dwRegType, dwBuffSize;
	unsigned char val[5000];

	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, key, 0, KEY_READ,  &hTestKey) == ERROR_SUCCESS)
	{
		dwBuffSize = 4096;

		int ret = RegQueryValueEx (hTestKey, subkey, NULL,  &dwRegType,  val,  &dwBuffSize);
		if (ret!=ERROR_SUCCESS) return NULL;
		int n = strlen((const char *)val);
		char *p = new char[n+4];
		strcpy(p,(const char *)val);
		RegCloseKey (hTestKey);
		return p;
	}

	return NULL;

}





///////////////////////////////////////////////////////////////////
// Function name     : setHKLMRegistryString
// Description       : sets a key in the HKEY_LOCAL_MACHINE registry
// Return type       : void 
// Author            : Xanathar 
// Argument          : char *key -> key name
// Argument          : char *subkey -> subkey name
// Argument          : char *value -> value
// Changes           : none yet
void setHKLMRegistryString(char *key, char *subkey, char *value)
{
	// Demonstrate registry open and create functions
	HKEY hTestKey;
	DWORD dwCreateResult;
	 
	// Access using preferred 'Ex' functions
	if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, key, 0, subkey, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,  &hTestKey,  &dwCreateResult) == ERROR_SUCCESS) 
	{
		RegSetValueEx(hTestKey, subkey, 0, REG_SZ, (CONST BYTE *)value, strlen(value));
		RegCloseKey (hTestKey);
	}
}	 


///////////////////////////////////////////////////////////////////
// Function name     : splitPath 
// Description       : splits a path between path and file name
// Return type       : char * -> ptr to the filename portion
// Author            : Xanathar 
// Argument          : char *p -> the complete path which gets truncated
// Changes           : none yet
char *splitPath (char *p)
{
	int i, ln = strlen(p);
	
	for (i=ln-1; i>=0; i--) {
		if ((p[i]=='\\')) { //||(p[i]=='/')) {
			p[i] = '\0';
			return p+i+1;
		}
	}
	return p; //no backslash found
}



#endif


void usehairdye(NXWSOCKET  s, P_ITEM piDye)	// x is the hair dye bottle object number
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	VALIDATEPI(piDye);

	NxwItemWrapper si;
	si.fillItemWeared( pc, true, true, true );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if(ISVALIDPI(pi))
			if(pi->layer==LAYER_BEARD || pi->layer==LAYER_HAIR)
			{
				pi->setColor( piDye->getColor() ); //Now change the color to the hair dye bottle color!
				pi->Refresh();
			}
	}
	piDye->Delete();	//Now delete the hair dye bottle!

}
