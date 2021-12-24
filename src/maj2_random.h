#pragma once

#include <math.h>
typedef unsigned uint;
typedef __attribute__((vector_size(8))) float vec2;
typedef __attribute__((vector_size(8))) uint uvec2;

/*
 * maj2_random
 *
 * maj2_random is a simplified floating point hash function derived from SHA-2,
 * retaining its high quality entropy compression function modified to permute
 * entropy from a vec2 (designed for UV coordinates) returning float values
 * between 0.0 and 1.0. since maj2_random is a hash function it will return
 * coherent noise. vector argument can be truncated prior to increase grain.
 *
 * converted to Clang/GCC vectors
 *
 * compile: cc -O3 -march=skylake-avx512 -c maj2_random.c -o maj2_random.o
 */

/* first 8 rounds of the SHA-256 k constant */
static uint sha256_k[8] =
{
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u
};

static uint ror(uint x, int d) { return (x >> d) | (x << (32-d)); }
static uint sigma0(uint h1) { return ror(h1, 2) ^ ror(h1, 13) ^ ror(h1, 22); }
static uint sigma1(uint h4) { return ror(h4, 6) ^ ror(h4, 11) ^ ror(h4, 25); }
static uint ch(uint x, uint y, uint z) { return z ^ (x & (y ^ z)); }
static uint maj(uint x, uint y, uint z) { return (x & y) ^ ((x ^ y) & z); }
static uint gamma0(uint a) { return ror(a, 7) ^ ror(a, 18) ^ (a >> 3); }
static uint gamma1(uint b) { return ror(b, 17) ^ ror(b, 19) ^ (b >> 10); }

static vec2 unorm(uvec2 n) {
    return (vec2) { n[0] & ((1u<<23)-1u), n[1] & ((1u<<23)-1u) }
         / (vec2) { (1u<<23)-1u, (1u<<23)-1u };
}
static vec2 truncx(vec2 uv, float d) {
    return (vec2){ (float)uv[0] / d, (float)uv[1] / d } * (vec2) { d , d };
}
static uvec2 sign(vec2 v) {
    return (uvec2) { v[0] < 0.0f, v[1] < 0.0f };
}

static uvec2 maj_extract(vec2 uv)
{
    /*
     * extract 48-bits of entropy from mantissas to create truncated
     * two word initialization vector 'W' composed using the 48-bits
     * of 'uv' entropy rotated and copied to keep the field equalized.
     * the exponent is ignored because the inputs are expected to be
     * normalized 'uv' values such as texture coordinates. it would be
     * beneficial to include the exponent entropy but we can't depend
     * on frexp or ilogb and log2 would be inaccurate.
     */
    uvec2 s = sign(uv);
    uint x = (uint)(fabsf(uv[0]) * (float)(1u<<23)) | (s[0] << 23);
    uint y = (uint)(fabsf(uv[1]) * (float)(1u<<23)) | (s[1] << 23);

    return (uvec2) { (x) | (y << 24), (y >> 8) | (x << 16) };
}

static vec2 maj_random(vec2 uv, uint NROUNDS)
{
    uint H[8] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
    uint W[2];
    uint T0,T1;
    int i;

    uvec2 st = maj_extract(uv);

    W[0] = st[0];
    W[1] = st[1];

    for (i=0; i<NROUNDS; i++) {
        W[i] = gamma1(W[(i-2)&1]) + W[(i-7)&1] + gamma0(W[(i-15)&1]) + W[(i-16)&1];
    }

    /* we use N rounds instead of 64 and alternate 2 words of iv in W */
    for (i=0; i<NROUNDS; i++) {
        T0 = W[i&1] + H[7] + sigma1(H[4]) + ch(H[4], H[5], H[6]) + sha256_k[i];
        T1 = maj(H[0], H[1], H[2]) + sigma0(H[0]);
        H[7] = H[6];
        H[6] = H[5];
        H[5] = H[4];
        H[4] = H[3] + T0;
        H[3] = H[2];
        H[2] = H[1];
        H[1] = H[0];
        H[0] = T0 + T1;
    }

    return unorm((uvec2){H[0] ^ H[1] ^ H[2] ^ H[3],
                         H[4] ^ H[5] ^ H[6] ^ H[7]});
}

static vec2 maj2_random(vec2 uv) { return maj_random(uv, 2); }
static vec2 maj8_random(vec2 uv) { return maj_random(uv, 8); }
