// https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c

#ifndef __DBGPRINTF_H
#define __DBGPRINTF_H

#if defined(NPRINTF) // don't print anything
#define dbgprintf(...)

#else

// OS specific print functions
#if defined(__ANDROID__)
#include <android/log.h>
#define __dbgprintf(...) __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", __VA_ARGS__);
#else
#define __dbgprintf(...) fprintf(stderr, __VA_ARGS__);
#endif

#if defined(NDEBUG)
// print debug message
#define dbgprintf(...) \
    do { \
        __dbgprintf(__VA_ARGS__); \
    } while (0)
#else
// print verbose debug message including file, line, and function
#define dbgprintf(...) \
    do { \
        __dbgprintf("%s : %d : %s()\n", __FILE__, __LINE__, __func__); \
        __dbgprintf(__VA_ARGS__); \
    } while (0)
#endif // NDEBUG

#endif // NPRINTF

#endif // __DBGPRINTF_H
