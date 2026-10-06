#ifndef _RNG_H
#define _RNG_H

/*
 * The PCG32 generator below is derived from PCG Random Number Generation for C
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

#include <stdint.h>
#include <stdlib.h>

typedef struct {
        uint64_t state;
        uint64_t inc;
} pcg32_random_t;

extern pcg32_random_t pcg32_global;

void rng_seed(uint64_t seed, uint64_t seq);

static inline uint32_t pcg32_random_r(pcg32_random_t *rng) {
        uint64_t oldstate = rng->state;
        // LCG step (pcg_setseq_64_step_r)
        rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
        // XSH RR output (pcg_output_xsh_rr_64_32), fed the old state so the
        // step and the output function can overlap
        uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
        uint32_t rot = oldstate >> 59u;
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}
static inline uint32_t rng_u(void) { return pcg32_random_r(&pcg32_global); }

static inline float rng_f(void) {
        return (pcg32_random_r(&pcg32_global) >> 8) * 0x1.0p-24f;
}

float rng_norm(void);

float rng_norm_n(size_t rounds);

#endif // _RNG_H
