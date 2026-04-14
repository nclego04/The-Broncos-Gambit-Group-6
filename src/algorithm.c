#include "algorithm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/time.h>
#endif

static long long stop_time = 0;
static int stop_search = 0;
static int nodes = 0;

/**
 * @brief Gets the current system time in milliseconds.
 * Handles cross-platform differences between Windows and POSIX systems.
 */
static long long get_time_ms(void) {
#if defined(_WIN32) || defined(_WIN64)
    return GetTickCount64();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
#endif
}
