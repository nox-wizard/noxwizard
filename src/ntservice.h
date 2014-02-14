  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __NTSERVICE_H__
#define __NTSERVICE_H__
#ifdef WIN32

// Xanathar.. we prepare this stuff for NT-Service Executables
// not supported yet though :(

	

void __cdecl svcprintf(char *txt, ...); 

#ifdef __cplusplus
	extern "C" {
#endif

void __cdecl svcprintf_C(char *txt, ...); 
void initService ( void );


#ifdef __cplusplus
}
#endif


#endif //WIN32
#endif //__NTSERVICE_H__
