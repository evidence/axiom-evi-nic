#ifndef AXIOM_UTILITY_h
#define AXIOM_UTILITY_h

/************************* AXIOM timespec utility *****************************/

static __inline int
timespec_ge(const struct timespec *a, const struct timespec *b)
{

	if (a->tv_sec > b->tv_sec)
		return (1);
	if (a->tv_sec < b->tv_sec)
		return (0);
	if (a->tv_nsec >= b->tv_nsec)
		return (1);
	return (0);
}

static __inline struct timespec
timeval2spec(const struct timeval *a)
{
	struct timespec ts = {
		.tv_sec = a->tv_sec,
		.tv_nsec = a->tv_usec * 1000
	};
	return ts;
}

static __inline struct timeval
timespec2val(const struct timespec *a)
{
	struct timeval tv = {
		.tv_sec = a->tv_sec,
		.tv_usec = a->tv_nsec / 1000
	};
	return tv;
}


static __inline struct timespec
timespec_add(struct timespec a, struct timespec b)
{
	struct timespec ret = { a.tv_sec + b.tv_sec, a.tv_nsec + b.tv_nsec };
	if (ret.tv_nsec >= 1000000000) {
		ret.tv_sec++;
		ret.tv_nsec -= 1000000000;
	}
	return ret;
}

static __inline struct timespec
timespec_sub(struct timespec a, struct timespec b)
{
	struct timespec ret = { a.tv_sec - b.tv_sec, a.tv_nsec - b.tv_nsec };
	if (ret.tv_nsec < 0) {
		ret.tv_sec--;
		ret.tv_nsec += 1000000000;
	}
	return ret;
}

static __inline double
usec2msec(uint64_t usec)
{
    return ((double)(usec) / 1000);
}

static __inline double
nsec2msec(uint64_t nsec)
{
    return ((double)(nsec) / 1000000);
}

#endif /* AXIOM_UTILITY_h */
