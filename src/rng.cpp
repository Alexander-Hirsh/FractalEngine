/*
 * The PCG32 seeding routine below is derived from PCG Random Number Generation for C
 * (https://github.com/imneme/pcg-c):
 *
 *   Copyright 2014-2019 Melissa O'Neill <oneill@pcg-random.org> and the PCG Project
 *   contributors.
 *
 * That code is offered under (Apache-2.0 OR MIT); it is used here under the MIT option,
 * the same license as the rest of this library. See https://www.pcg-random.org for
 * details on the generation scheme.
 *
 * SPDX-License-Identifier: MIT
 */

#include "rng.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define PCG32_INITIALIZER {0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL}

pcg32_random_t pcg32_global = PCG32_INITIALIZER;

void pcg32_srandom_r(pcg32_random_t *rng, uint64_t initstate, uint64_t initseq) {
        rng->state = 0U;
        rng->inc = (initseq << 1u) | 1u;
        pcg32_random_r(rng);
        rng->state += initstate;
        pcg32_random_r(rng);
}

void rng_seed(uint64_t seed, uint64_t seq) {
        pcg32_srandom_r(&pcg32_global, seed, seq);
}

// TODO(richard): More accurate and performant way to sample a normal distribution
float rng_norm(void) {
        float result = -6.0f;

        for (size_t i = 0; i < 12; ++i) {
                result += rng_f();
        }

        return result;
}

float rng_norm_n(size_t rounds) {
        float result = -0.5 * rounds;

        for (size_t i = 0; i < rounds; ++i) {
                result += rng_f();
        }

        return result * sqrt(12.0f / rounds);
}
