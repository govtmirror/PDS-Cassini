/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef RNG_H
#define RNG_H

/**
 * <p>
 * This file defined the random number generator type.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

#include "common.h"


struct RNG {
    unsigned int MT[624];
    int index;

    RNG(int seed = 1) {
        init(seed);
    }

    void init(int seed = 1) {
        MT[0] = seed;
        FOR(i, 1, 624) MT[i] = (1812433253UL * (MT[i-1] ^ (MT[i-1] >> 30)) + i);
        index = 0;
    }

    void generate() {
        const unsigned int MULT[] = {0, 2567483615UL};
        REP(i, 227) {
            unsigned int y = (MT[i] & 0x8000000UL) + (MT[i+1] & 0x7FFFFFFFUL);
            MT[i] = MT[i+397] ^ (y >> 1);
            MT[i] ^= MULT[y&1];
        }
        FOR(i, 227, 623) {
            unsigned int y = (MT[i] & 0x8000000UL) + (MT[i+1] & 0x7FFFFFFFUL);
            MT[i] = MT[i-227] ^ (y >> 1);
            MT[i] ^= MULT[y&1];
        }
        unsigned int y = (MT[623] & 0x8000000UL) + (MT[0] & 0x7FFFFFFFUL);
        MT[623] = MT[623-227] ^ (y >> 1);
        MT[623] ^= MULT[y&1];
    }

    unsigned int rand() {
        if (index == 0) {
            generate();
        }

        unsigned int y = MT[index];
        y ^= y >> 11;
        y ^= y << 7  & 2636928640UL;
        y ^= y << 15 & 4022730752UL;
        y ^= y >> 18;
        index = index == 623 ? 0 : index + 1;
        return y;
    }

    INLINE int next() {
        return rand();
    }

    INLINE int next(int x) {
        return rand() % x;
    }

    INLINE int next(int a, int b) {
        return a + (rand() % (b - a));
    }

    INLINE double nextDouble() {
        return (rand() + 0.5) * (1.0 / 4294967296.0);
    }
};


#endif // RNG_H

