#ifndef DPRINTF_H
#define DPRINTF_H


#define _DPRINTF(type, _fmt, ... )\
    do {                                                          \
        struct timeval _t0;                                       \
        gettimeofday(&_t0, NULL);                                 \
        fprintf(stderr, "%03d.%06d %s():%d - %s - " _fmt "%s\n",  \
                (int)(_t0.tv_sec % 1000), (int)_t0.tv_usec,       \
                __func__, __LINE__, type , __VA_ARGS__);          \
    } while (0);
#define EPRINTF(...) _DPRINTF("*ERROR*", __VA_ARGS__, "")
#define IPRINTF(...) _DPRINTF("INFO", __VA_ARGS__, "")
#define NDPRINTF(...) do {} while(0);
#define NEPRINTF(...) do {} while(0);
#define NIPRINTF(...) do {} while(0);
#ifdef PDEBUG
#define DPRINTF(...) _DPRINTF("DEBUG", __VA_ARGS__, "")
#else
#define DPRINTF(...) do {} while(0);
#endif

#endif /* DPRINTF_H */
