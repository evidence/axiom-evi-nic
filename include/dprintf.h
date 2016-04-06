#ifndef DPRINTF_H
#define DPRINTF_H

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef __KERNEL__
#include <linux/string.h>
#define _DPRINTF(type, _fmt, ... )\
    do {                                                                       \
        struct timeval _t0;                                                    \
        do_gettimeofday(&_t0);                                                 \
        printk(KERN_ERR "%03d.%06d %s[%d]: %s() - %s\n  message: " _fmt "%s\n",\
                (int)(_t0.tv_sec % 1000), (int)_t0.tv_usec,                    \
                __FILENAME__, __LINE__, __func__, type , __VA_ARGS__);         \
    } while (0);
#else /* !__KERNEL__ */
#include <sys/time.h>
#include <string.h>
#define _DPRINTF(type, _fmt, ... )\
    do {                                                                       \
        struct timeval _t0;                                                    \
        gettimeofday(&_t0, NULL);                                              \
        fprintf(stderr, "%03d.%06d %s[%d]: %s() - %s\n  message: " _fmt "%s\n",\
                (int)(_t0.tv_sec % 1000), (int)_t0.tv_usec,                    \
                __FILENAME__, __LINE__, __func__, type , __VA_ARGS__);         \
    } while (0);
#endif /* __KERNEL */

#define EPRINTF(...) _DPRINTF("*ERROR*", __VA_ARGS__, "")
#define IPRINTF(verbose, ...)\
    do {                                                                \
        if (verbose)                                                    \
            _DPRINTF("INFO", __VA_ARGS__, "")                           \
    } while(0);
#define NDPRINTF(...) do {} while(0);
#define NEPRINTF(...) do {} while(0);
#define NIPRINTF(...) do {} while(0);


#ifdef PDEBUG
#define DPRINTF(...) _DPRINTF("DEBUG", __VA_ARGS__, "")
#else
#define DPRINTF(...) do {} while(0);
#endif

#endif /* DPRINTF_H */
