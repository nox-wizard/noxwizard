  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#define USE_THREADS
#include "common_libs.h"

OSVersion g_OSVer = OSVER_UNKNOWN;

OSVersion getOSVersion()
{
    if (g_OSVer==OSVER_UNKNOWN) {
        getOSVersionString();
    }
    return g_OSVer;
}



#ifdef __unix__
char *strlwr(char *str) {
  for (unsigned int i=0;i<strlen(str);i++)
    str[i]=tolower(str[i]);
  return str;
}
char *strupr(char *str) {
  for (unsigned int i=0;i<strlen(str);i++)
    str[i]=toupper(str[i]);
  return str;
}
#endif

/*!
\brief Thread abstraction namespace
\author Xanathar
*/
namespace tthreads {
/*!
\author Xanathar
\param funk pointer to thread function
\param param pointer to a volatile buffer created with should be eventually
freed by the thread itself
*/
int startTThread( TTHREAD ( *funk )( void * ), void* param )
{
	#ifdef WIN32
	  return _beginthread(reinterpret_cast<void ( * )( void * )>(funk), 0, param);
	#endif
	#ifdef PTHREADS
	  pthread_t pt;
	  return pthread_create( &pt, NULL, funk, param );
	#endif
	return -1;
}

} //namespaze

#ifndef WIN32
    ///////////////////////////////////////////////////////////////////
    //                                                               //
    // POSIX THREAD ISSUES (thx AnomCwrd)                            //
    //                                                               //
    ///////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    // Function name     : Sleep
    // Description       : sleeps some milliseconds, thread-safe
    // Return type       : void
    // Author            : AnomCwrd
    void Sleep(unsigned long msec)
    {
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = msec * 1000;
        select(0, 0, 0, 0, &timeout);
    }

    static volatile bool g_bHUPReceived = false; // Xan : this is not thread safe, but worst case
                                 //       we skip an HangUP.. shouldn't be much a
                                 //       problem.
    static volatile bool g_bShouldClose = false;

    ///////////////////////////////////////////////////////////////////
    // Function name     : pollHUPStatus
    // Description       : returns true if a SIGHUP was handled
    // Return type       : bool
    // Author            : Xanathar
    // Notes             : can miss an HUP -- but should never be a problem
    bool pollHUPStatus ()
    {
        if (g_bHUPReceived) {
           g_bHUPReceived = false;
           return true;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////
    // Function name     : pollCloseRequests
    // Description       : returns true if a SIGQUIT or similar was handled
    // Return type       : bool
    // Author            : Xanathar
    bool pollCloseRequests()
    {
        return g_bShouldClose;
    }



    ///////////////////////////////////////////////////////////////////
    // Function name     : SignalThread
    // Description       : Signal-handling thread
    // Return type       : void
    // Author            : AnomCwrd
    static void* SignalThread(void*)
    {
        sigset_t signals_to_catch;

         // clear out the list of signals to catch
        sigemptyset(&signals_to_catch);

         // now, start adding signals we care about
        sigaddset(&signals_to_catch, SIGHUP);
        sigaddset(&signals_to_catch, SIGINT);
        sigaddset(&signals_to_catch, SIGQUIT);
        sigaddset(&signals_to_catch, SIGTERM);
        sigaddset(&signals_to_catch, SIGUSR1);
        sigaddset(&signals_to_catch, SIGUSR2);

        // we need to know which signal we caught
        int current_signal;

        for (;;) { // loop forever waiting on signals
            sigwait(&signals_to_catch, &current_signal);

            // check what we caught
            if (current_signal == SIGINT || current_signal == SIGQUIT || current_signal == SIGTERM) {
                // tell others about the signal
                printf("Termination signal handled...\n");
                g_bShouldClose = true;
                return NULL;
                //pthread_exit(0);
            }

             // check for a HUP
            if (current_signal == SIGHUP) {
                g_bHUPReceived = true;
                return NULL;
                //pthread_exit(0);
            }
            Sleep(100);
        }
    }

    ///////////////////////////////////////////////////////////////////
    // Function name     : start_signal_thread
    // Description       : starts signal handling thread
    // Return type       : void
    // Author            : AnomCwrd
    void start_signal_thread()
    {
       // frist thing we need to do is setup POSIX signals
       setup_signals();

       // now we can start the signal  thread
       pthread_attr_t thread_attr;
       pthread_t signal_thread_id;

       pthread_attr_init(&thread_attr);
       pthread_attr_setscope(&thread_attr, PTHREAD_SCOPE_SYSTEM);
       pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
       pthread_create(&signal_thread_id, &thread_attr, SignalThread, 0);
    }

    ///////////////////////////////////////////////////////////////////
    // Function name     : setup_signals
    // Description       : starts signal handling thread etc
    // Return type       : void
    // Author            : AnomCwrd
    void setup_signals ()
    {
        sigset_t signals_to_block;
        struct sigaction ignore_handler;

        // we have to use 'struct sigaction' since there is also a
        // function called sigaction in the same scope as the struct. That
        // is okay for C, but C++ is cool and does not require struct to
        // declare a variable of struct type.

         // set the sigaction struct to all zeros
        std::memset(&ignore_handler, 0, sizeof(struct sigaction));

         // set the ignore_handler to SIG_IGN
        ignore_handler.sa_handler = SIG_IGN;

         // fill the 'signals_to_block' variable will all possible signals
        sigfillset(&signals_to_block);

        // set the thread sigmask (add all threads from now on)
        // to one that blocks all signals except SIGPIPE
        sigdelset(&signals_to_block, SIGPIPE);
        sigdelset(&signals_to_block, SIGFPE);
        sigdelset(&signals_to_block, SIGILL);
        sigdelset(&signals_to_block, SIGSEGV);
        sigdelset(&signals_to_block, SIGBUS);
        pthread_sigmask(SIG_BLOCK, &signals_to_block, 0);

        // ignore SIGPIPE, we catch it on Socket::write()
        sigaction(SIGPIPE, &ignore_handler, 0);
    }

#endif // not windows


////////////////////////////////////////
// getOSVersionString, author Xanathar
//
static char g_szOSVerBuffer[1024];
char* getOSVersionString()
{
    g_szOSVerBuffer[0] = '\0';
#ifdef WIN32
    bool l_bWindowsNT = false;
#endif
#ifdef __unix__
    struct utsname info;
    uname(&info);
    sprintf(g_szOSVerBuffer, "%s %s on a %s", info.sysname, info.release, info.machine);
    g_OSVer = OSVER_NONWINDOWS;
#endif

#ifdef WIN32
	{
	   OSVERSIONINFO vi = { sizeof(vi) };
	   char s[80];
	   GetVersionEx(&vi);
	   if ((vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS))
	   {
		   strcpy (s, "Windows 95");
		   if (vi.dwMinorVersion>=10) {
			   strcpy (s, "Windows 98");
			   if (strstr(vi.szCSDVersion, "A")!=NULL) strcpy (s, "Windows 98 2nd Edition");
			   if ((vi.dwMinorVersion>11)||(vi.dwMajorVersion>=5)) strcpy (s, "Windows ME");
		   }
		   l_bWindowsNT = false;
	   }
	   else {
		   strcpy (s, "Windows NT");
		   if (vi.dwMajorVersion>=5) {
			   strcpy (s, "Windows 2000");
			   if ((vi.dwMinorVersion>0)||(vi.dwMajorVersion>5)) strcpy (s, "Windows XP");
		   }
		   l_bWindowsNT = true;
	   }
	
	   if (l_bWindowsNT) {
  		   sprintf(g_szOSVerBuffer, "%s v%d.%d [Build %d] %s",s,vi.dwMajorVersion , vi.dwMinorVersion ,
			   vi.dwBuildNumber , vi.szCSDVersion );
       }
	   else {
		   sprintf(g_szOSVerBuffer, "%s v%d.%d %s [Build %d]",s,vi.dwMajorVersion , vi.dwMinorVersion ,
		   vi.szCSDVersion, vi.dwBuildNumber & 0xFFFF );
       }
    }
    g_OSVer = (l_bWindowsNT) ? OSVER_WINNT : OSVER_WIN9X;
#endif

    return g_szOSVerBuffer;
}



