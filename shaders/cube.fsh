#version 130

varying in vec3 v_normal;
varying in vec2 v_uv;
varying in vec4 v_color;
varying in vec3 v_fragPos;
varying in vec3 v_lightDir;

uint ror(uint x, int d) { return (x >> d) | (x << (32-d)); }
uint sigma0(uint h1) { return ror(h1, 2) ^ ror(h1, 13) ^ ror(h1, 22); }
uint sigma1(uint h4) { return ror(h4, 6) ^ ror(h4, 11) ^ ror(h4, 25); }
uint gamma0(uint a) { return ror(a, 7) ^ ror(a, 18) ^ (a >> 3); }
uint gamma1(uint b) { return ror(b, 17) ^ ror(b, 19) ^ (b >> 10); }
uint ch(uint x, uint y, uint z) { return z ^ (x & (y ^ z)); }
uint maj(uint x, uint y, uint z) { return (x & y) ^ ((x ^ y) & z); }

uint init_H[8] = uint[8](
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
);

uint k[64] = uint[64](
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
);

float random(vec2 v)
{
	uint H[8];
	uint W[64];
    uint T0,T1;
    int i;

	uint x = uint(abs(v.x) * float(1u<<24));
	uint y = uint(abs(v.y) * float(1u<<24));
	x = x - x % (1u<<16);
	y = y - y % (1u<<16);

    W[0] = x | (y << 24);
    W[1] = (y >> 8) | (x << 16);
    for (i = 2; i < 16; i++) {
	    W[i] = ror(W[i%2],(i%2)*8);
    }

    for (i = 0; i < 8; i++) {
        H[i] = init_H[i];
    }

    for (i=16; i<64; i++) {
        W[i] = gamma1(W[i - 2]) + W[i - 7] + gamma0(W[i - 15]) + W[i - 16];
    }

	for (i=0; i<64; i++) {
        T0 = W[i] + H[7] + sigma1(H[4]) + ch(H[4], H[5], H[6]) + k[i];
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

    uint r = 0u;
    for (i = 0; i < 8; i++) {
        r ^= H[i];
    }

    return float(r&((1u<<24)-1u)) / float(1u<<24);
}

void main()
{
  float ambient = 0.1;
  float diff = max(dot(v_normal, v_lightDir), 0.0);
 
  float r = random(v_uv) * 0.1;
  vec4 finalColor = (ambient + diff + r) * v_color;

  gl_FragColor = vec4(finalColor.rgb, v_color.a);
}