#ifndef DPRINTF_H
#define DPRINTF_H
/*!
 * \file dprintf.h
 *
 * \version     v0.5
 * \date        2016-04-29
 *
 * This file contains some functions to print debugging information
 *
 */

/*! \brief Print ERROR debug messages */
#define EPRINTF(...) _DPRINTF("*ERROR*", __VA_ARGS__, "")
/*!
 * \brief Print INFO debug messages
 * \param verbose       If it is true, the message is printed
 */
#define IPRINTF(verbose, ...)\
    do {                                                                \
        if (verbose)                                                    \
            _DPRINTF("INFO", __VA_ARGS__, "")                           \
    } while(0);
/*! \brief Disable print debug messages */
#define NDPRINTF(...) do {} while(0);
/*! \brief Disable print ERROR debug messages */
#define NEPRINTF(...) do {} while(0);
/*! \brief Disable print INFO debug messages */
#define NIPRINTF(...) do {} while(0);


#ifdef PDEBUG
#define DPRINTF(...) _DPRINTF("DEBUG", __VA_ARGS__, "")
#else
/*! \brief Print debug messages if PDEBUG macro is defined */
#define DPRINTF(...) do {} while(0);
#endif



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


#endif /* DPRINTF_H */
