/* magic.c - PPP Magic Number routines.
 * 
 * Copyright (C) 1989 Carnegie Mellon University.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program - see the file COPYING.
 *
 * See `CHANGES' file for revision history.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include "magic.h"

#ifndef __linux__
extern long mrand48 __P((void));
extern void srand48 __P((long));
#else
#include <sys/stat.h>
#include <fcntl.h>

/* on Linux we use /dev/urandom as random numbers source 
   I find it really cool :) */
int rfd = -1;	/* /dev/urandom */
#endif

static int magic_inited = 0;

/*
 * magic_init - Initialize the magic number generator.
 *
 * Attempts to compute a random number seed which will not repeat.
 * The current method uses the current hostid, current process ID
 * and current time, currently.
 */
void
magic_init()
{
    long seed;
    struct timeval t;

    if (magic_inited)
        return;

/* FIXME this should be ifdef HAVE_DEV_URANDOM + test for /dev/urandom in configure */
#ifdef __linux__
    rfd = open("/dev/urandom", O_RDONLY);
    if(rfd != -1) 
        return;
#endif
    /* if /dev/urandom fails, we try traditional method */
    gettimeofday(&t, NULL);
    seed = gethostid() ^ t.tv_sec ^ t.tv_usec ^ getpid();
    srand48(seed);

    magic_inited = 1;
}

/*
 * magic - Returns the next magic number.
 */
u_int32_t
magic()
{
    magic_init();

#ifdef __linux__
    u_int32_t ret = 0;

    if(rfd > -1) {
        if (read(rfd, &ret, sizeof(ret)) < sizeof(ret)) {
            /* on read() error, fallback to other method */
            return (u_int32_t) mrand48();
        }
        return ret;
    }
#endif
    return (u_int32_t) mrand48();
}

#ifdef NO_DRAND48
/*
 * Substitute procedures for those systems which don't have
 * drand48 et al.
 */

double
drand48()
{
    return (double)random() / (double)0x7fffffffL; /* 2**31-1 */
}

long
mrand48()
{
    return random();
}

void
srand48(seedval)
long seedval;
{
    srandom((int)seedval);
}

#endif
