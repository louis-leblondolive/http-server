#ifndef COMPAT
#define COMPAT

#include <string.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>

#if !defined(__APPLE__)

#if defined(__GLIBC__) && (__GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 38))
static inline size_t strlcpy(char *dst, const char *src, size_t size) {
    size_t srclen = strlen(src);
    if (size > 0) {
        size_t copylen = srclen < size - 1 ? srclen : size - 1;
        memcpy(dst, src, copylen);
        dst[copylen] = '\0';
    }
    return srclen;
}
#endif

#if !defined(__linux__)
static inline time_t timegm(struct tm *tm) {
    time_t t;
    char *tz = getenv("TZ");
    setenv("TZ", "UTC", 1);
    tzset();
    t = mktime(tm);
    if (tz) setenv("TZ", tz, 1);
    else unsetenv("TZ");
    tzset();
    return t;
}
#endif

#endif // !__APPLE__
#endif // COMPAT