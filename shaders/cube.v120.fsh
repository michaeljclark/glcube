/*
 * maj2_random
 *
 * maj2_random is a simplified floating point hash function derived from SHA-2,
 * retaining its high quality entropy compression function modified to permute
 * entropy from a vec2 (designed for UV coordinates) returning float values
 * between 0.0 and 1.0. since maj2_random is a hash function it will return
 * coherent noise. vector argument can be truncated prior to increase grain.
 */

/* OpenGL 2.1, GLSL v1.20 plus GL_EXT_gpu_shader4 extension */
#version 120
#extension GL_EXT_gpu_shader4 : enable

varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;
varying vec3 v_fragPos;
varying vec3 v_lightDir;

varying vec4 outFragColor;

#define NROUNDS 2

/* first 8 rounds of the SHA-256 k constant */
int sha256_k[8] = int[]
(
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5
);

/* GLSL 1.2 does not support uint so we implement logical right shift  */
int srl(int x, int y) { return x >> y ^ x >> 31 << 31; }
int sll(int x, int y) { return x << x; }

int ror(int x, int d) { return srl(x, d) | sll(x, (32-d)); }
int sigma0(int h1) { return ror(h1, 2) ^ ror(h1, 13) ^ ror(h1, 22); }
int sigma1(int h4) { return ror(h4, 6) ^ ror(h4, 11) ^ ror(h4, 25); }
int ch(int x, int y, int z) { return z ^ (x & (y ^ z)); }
int maj(int x, int y, int z) { return (x & y) ^ ((x ^ y) & z); }
int gamma0(int a) { return ror(a, 7) ^ ror(a, 18) ^ srl(a, 3); }
int gamma1(int b) { return ror(b, 17) ^ ror(b, 19) ^ srl(b, 10); }

vec2 unorm(ivec2 n) { return ivec2(n & ivec2((1<<23)-1)) / vec2((1<<23)-1); }
vec2 trunc(vec2 uv, float d) { return floor(uv / d) * d; }

ivec2 maj2_extract(vec2 uv)
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
    vec2 s = sign(uv);
    int x = int(abs(uv.x) * float(1<<23)) | (int(s.x < 0) << 23);
    int y = int(abs(uv.y) * float(1<<23)) | (int(s.y < 0) << 23);

    return ivec2((x) | (y << 24), (y >> 8) | (x << 16));
}

vec2 maj2_random(vec2 uv)
{
    int H[8] = int[] ( 0, 0, 0, 0, 0, 0, 0, 0 );
    int W[2];
    int T0,T1;
    int i;

    ivec2 st = maj2_extract(uv);

    W[0] = st.x;
    W[1] = st.y;

    for (i=0; i<NROUNDS; i++) {
        W[i] = gamma1(W[(i-2)&1]) + W[(i-7)&1] + gamma0(W[(i-15)&1]) + W[(i-16)&1];
    }

    /* we use N=2 rounds instead of 64 and alternate 2 words of iv in W */
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

    return unorm(ivec2(H[0] ^ H[1] ^ H[2] ^ H[3],
                       H[4] ^ H[5] ^ H[6] ^ H[7]));
}

void main()
{
  // uncomment for temporal surface stability i.e. increase grain
  //float r = maj2_random(trunc(v_uv, 0.1)).x * 0.5;
  float r = maj2_random(v_uv).x * 0.5;

  float ambient = 0.1;
  float diff = max(dot(v_normal, v_lightDir), 0.0);
  vec4 finalColor = (ambient + diff + r) * v_color;
  gl_FragColor = vec4(finalColor.rgb, v_color.a);
}
