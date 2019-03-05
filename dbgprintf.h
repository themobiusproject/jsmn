// https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c

#ifndef __DBGPRINTF_H
#define __DBGPRINTF_H

#if defined(NPRINTF)
#define dbgprintf(...)
#elif defined(NDEBUG)
#define dbgprintf(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
    } while (0)
#else
#define dbgprintf(...) \
    do { \
        fprintf(stderr, "%s : %d : %s()\n", __FILE__, __LINE__, __func__); \
        fprintf(stderr, __VA_ARGS__); \
    } while (0)
#endif

#endif // __DBGPRINTF_H
