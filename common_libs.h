  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file common_libs.h
\brief header
This is a small abstraction layer for threading and STL including
with the explicit purpose of tearing away some platform dependant
warnings/errors/issues.
*/


#ifndef __COMMON_LIBS_H__
#define __COMMON_LIBS_H__

#if !defined(WIN32) && (__GNUC__ == 3) && ( (__GNUC_MINOR__ == 1) || ( __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ < 2 ) )
	#warning The use of GCC 3 with NoX-Wizard is very very dangerous. \
	GCC3.2.2+ of Debian GNU/Linux seems to be ok.
#endif

#ifdef __BORLANDC__
	#define NDEBUG
	#define WIN32
	#ifdef __CONSOLE__
		#define _CONSOLE
	#else
		#define _WINDOWS
	#endif
#endif

#if defined WIN32 || defined _WIN32
	#ifndef _WIN32
		#define _WIN32
	#endif
	#pragma pack(1)

	#define snprintf _snprintf
	typedef int socklen_t;
#endif

#ifndef __GNUC__
	#ifdef _MSC_VER
		#define strncasecmp strncmp
		// can microsoft follow standards? no, obvious... I hate them.... - Akron
	#else
		#define strncasecmp strncmpi
		// on borland compiler exists strncmpi...
	#endif
	#define strcasecmp strcmpi
#endif

#if defined(__BEOS__) && !defined(__unix__)
	#define __unix__
#endif

#ifdef __GNUC__
	#define PACK_NEEDED __attribute__ ((packed))
#else
	#define PACK_NEEDED
#endif

#ifdef  _MSC_VER
	#pragma pack(push,8)		//for Visual C++ using STLport
	#pragma warning(disable: 4786)	//Gets rid of BAD stl warnings
	#pragma warning(disable: 4800)	//needed couse now we can see the real warning
#endif

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <vector>

#ifdef __GNUC__
	#if (__GNUC__ == 2)
		#include <slist.h>
	#else
		#include <ext/slist>
		using __gnu_cxx::slist;
	#endif
#else
	#include <slist>
#endif

#include <map>
#include <queue>
#include <iterator>
#include <algorithm>
#include <stack>
#include <fstream>
#include <utility>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <ctype.h>

#ifdef WIN32
	#include <winsock.h>
	#include <winbase.h>
	#include <io.h>
	#include <dos.h>
	#include <limits.h>
	#include <conio.h>
	#include <process.h>
#endif

#ifdef __unix__
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <sys/time.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <termios.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include <libgen.h>

	#define ioctlsocket ioctl

	#ifndef __BEOS__
		#include <signal.h>
		#include <sys/errno.h>
		#include <arpa/inet.h>
		#define closesocket(s)	close(s)
	#else
		#include <be/NetKit.h>
		#include <be/NetworkKit.h>
		#include <be/net/socket.h>
		typedef int socklen_t;
	#endif
    #include <sys/utsname.h>
#endif


#ifdef  _MSC_VER
	#pragma pack(pop)
	#ifndef STLPORT
		#pragma warning(disable: 4103)
		#pragma warning(disable: 4786)
		#define vsnprintf _vsnprintf
	#endif
#endif

#ifdef __unix__
	typedef unsigned char BYTE ;
	#define SOCKET_ERROR -1
	char *strlwr(char *);
	char *strupr(char *);
#endif

#if !defined WIN32 && !defined _WIN32
    void Sleep(unsigned long msec);
#endif

#ifdef __BEOS__
	extern int errno;
#endif

extern char* getOSVersionString();
enum OSVersion { OSVER_UNKNOWN, OSVER_WIN9X, OSVER_WINNT, OSVER_NONWINDOWS };
extern OSVersion getOSVersion();

using namespace std;

#ifdef  _MSC_VER
	#pragma warning(disable: 4018)
#endif

#ifndef MSG_NOSIGNAL
    #define MSG_NOSIGNAL 0
#endif

#ifdef USE_THREADS
/***************************************************
 T-THREADs (Trivial Threads), by Xanathar 2001, 2002
 This is a simple simple simple and trivial thread
 implementation, using native threads in Win32 and
 Posix threads in Linux and *BSDs.
 Signal handling of *nix thread code by AnomCwrd
 ***************************************************/

#if defined(__OpenBSD__) || defined(__FreeBSD__)
    #include <pthread.h>
#endif

#if defined __unix__ && !defined __BEOS__
    #define PTHREADS
#endif

#if !defined WIN32 && !defined PTHREADS
    #error Your platform is not supported by Trivial Threads :[
#endif

namespace tthreads {

class Mutex {
  private:
        bool m_bLocked;
        bool m_bDebug;
        char* m_szMutexName;
    #ifdef WIN32
    	CRITICAL_SECTION m_cs;
    	CRITICAL_SECTION m_cs2;
    #elif defined PTHREADS
    	pthread_mutex_t* m_mutex;
    #endif
   inline void init(bool alreadylocked) {
        #ifdef WIN32
    	    InitializeCriticalSection(&m_cs);
    	    InitializeCriticalSection(&m_cs2);
	#elif defined PTHREADS
    	    m_mutex = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
    	    pthread_mutex_init (m_mutex, NULL);
        #endif
        m_bLocked = false;
        m_bDebug = false;
        if (alreadylocked) enter();
    }
  public:
    inline Mutex(bool alreadylocked = false) { init(alreadylocked); }
    inline Mutex(char *name, bool alreadylocked = false) { init(alreadylocked); setDebugMode(name); }
    inline ~Mutex() {
        if (m_bDebug) {
            printf("DBG-MUTEX: Mutex %s destroyed\n", m_szMutexName);
            if (m_szMutexName!=NULL) delete[] m_szMutexName;
        }
        #ifdef WIN32
    	    DeleteCriticalSection(&m_cs);
    	    DeleteCriticalSection(&m_cs2);
	#elif defined PTHREADS
    	    pthread_mutex_destroy (m_mutex);
        	free(m_mutex);
        #endif
    }
    inline void enter () {
        if (m_bDebug) printf("DBG-MUTEX: Entering mutex %s\n", m_szMutexName);
        #ifdef WIN32
    	    EnterCriticalSection(&m_cs2);
    	    EnterCriticalSection(&m_cs);
	#elif defined PTHREADS
    	    pthread_mutex_lock(m_mutex);
        #endif
        m_bLocked = true;
        #ifdef WIN32
    	    LeaveCriticalSection(&m_cs2);
        #endif
        if (m_bDebug) printf("DBG-MUTEX: Entered mutex %s\n", m_szMutexName);
    }
    inline void leave () {
        if (m_bDebug) printf("DBG-MUTEX: Leaving mutex %s\n", m_szMutexName);
        m_bLocked = false;
        #ifdef WIN32
    	    LeaveCriticalSection(&m_cs);
	#elif defined PTHREADS
        	pthread_mutex_unlock(m_mutex);
        #endif
    }
    inline bool tryEnter () {
        if (m_bDebug) printf("DBG-MUTEX: Try-Locking mutex %s\n", m_szMutexName);
        #ifdef WIN32
            //if (getOSVersion()==OSVER_WINNT) return TryEnterCriticalSection(&m_cs);
            // Xan : workaround for TryEnterCriticalSection not working in Win9x
            EnterCriticalSection(&m_cs2);
            //we're here, we've control of m_bLocked.
            if (m_bLocked) {
                LeaveCriticalSection(&m_cs2);
                return false;
            }
            // mmm so we'll enter!, thx to critsections are recursive.
            enter();
       	    LeaveCriticalSection(&m_cs2);
            return true;
	#elif defined PTHREADS
            return pthread_mutex_trylock(m_mutex)==0;
        #endif
    }
    void setDebugMode (char *name) { m_bDebug = false; m_szMutexName = new char[strlen(name)+2]; strcpy(m_szMutexName, name);}
};




template <typename T> class Atomic {
  protected:
    Mutex m_mutex;
    volatile T m_val;
    inline void enterIfEq(const T& compval)  { while(true) { m_mutex.enter(); if (m_val==compval) return; m_mutex.leave(); Sleep(1); } }
    inline void enterIfNEq(const T& compval) { while(true) { m_mutex.enter(); if (m_val!=compval) return; m_mutex.leave(); Sleep(1); } }
  public:
	inline T setIfEq  (const T& newval, const T& compval = 0) { enterIfEq(compval); T old = m_val; m_val = newval; m_mutex.leave(); return old;}
	inline T setIfNEq (const T& newval, const T& compval = 0) { enterIfNEq(compval); T old = m_val; m_val = newval; m_mutex.leave();  return old;}
	inline T decIfEq  (const T& newval = 1, const T& compval = 0) { enterIfEq(compval); T old = m_val; m_val -= newval; m_mutex.leave();  return old;}
	inline T decIfNEq (const T& newval = 1, const T& compval = 0) { enterIfNEq(compval); T old = m_val; m_val -= newval; m_mutex.leave();  return old;}
	inline T incIfEq  (const T& newval = 1, const T& compval = 0) { return dec (-newval, compval); }
	inline T incIfNEq (const T& newval = 1, const T& compval = 0) { return dec (-newval, compval); }
	inline T set      (const T& newval) { m_mutex.enter(); T old = m_val; m_val = newval; m_mutex.leave();  return old;}
    inline T dec      (const T& newval = 1) { m_mutex.enter(); T old = m_val; m_val -= newval; m_mutex.leave();  return old;}
    inline T inc      (const T& newval = 1) { return dec(-newval); }

    inline Atomic<T>& operator++() { inc(); return *this;}
    inline Atomic<T>& operator--() { dec(); return *this;}
    inline Atomic<T>& operator+=(const T& x) { inc(x);  return *this;}
    inline Atomic<T>& operator-=(const T& x) { dec(x);  return *this;}
    inline Atomic<T>& operator=(const T& newval) { set(newval);  return *this;}

    inline operator T() { return m_val; }
    inline bool operator==(const T& v) { return (m_val==v); }
};


class Semaphore : public Atomic<int> {
public :
    Semaphore(int n = 1) { set(n); }
    inline void enter() { decIfNEq(); }
    inline bool tryEnter() {
        m_mutex.enter();
        if (m_val!=0) {
            m_val--;
            m_mutex.leave();
            return true;
        }
        m_mutex.leave();
        return false;
    }
    void exit() { inc(); }
};



#ifdef WIN32
    #define TTHREAD void
    #define EXIT_TTHREAD { return; }
#elif defined PTHREADS
    #define TTHREAD void*
    #define EXIT_TTHREAD { return NULL; }
#endif

int startTThread( TTHREAD ( *funk )( void * ), void* param = NULL );

};

#ifdef WIN32
    inline bool pollHUPStatus () { return false; }
    inline bool pollCloseRequests () { return false; }
    inline void setup_signals (){ return; }
    inline void start_signal_thread() {return;}
#elif defined PTHREADS
    extern bool pollHUPStatus ();
    extern bool pollCloseRequests ();
    extern void setup_signals ();
    extern void start_signal_thread();
#endif

#endif // USE THREADS

#endif //__COMMON_LIBS_H__

