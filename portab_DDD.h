// Update to DDS v2.8
// Soren Hein, November 2014

#if defined(_WIN32)
  #if defined(__MINGW32__) && !defined(WINVER)
    #define WINVER 0x500
  #endif
  
  #if defined(_MSC_VER)
    #include <intrin.h>
  #endif

  #include <io.h>

  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
  #define snprintf _snprintf

  #define access _access
  #define F_OK 0

#elif defined(__CYGWIN__)
  #include <time.h>
  #include <sys/time.h>

#elif defined(__linux)
  #include <unistd.h>
  #include <errno.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <time.h>
  #include <sys/time.h>

#elif defined(__APPLE__)
  #include <unistd.h>
  #include <errno.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <time.h>
  #include <sys/time.h>

#endif


// In C++11 UNUSED(x) is explicitly provided
#if __cplusplus <= 199711L
  #if defined (_MSC_VER)
    #define UNUSED(x) (void) (x);
  #else
    #define UNUSED(x) (void) (sizeof((x), 0))
  #endif
  #define nullptr NULL
#endif
