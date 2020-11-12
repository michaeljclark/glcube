#version 150

varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;
varying vec3 v_fragPos;
varying vec3 v_lightDir;

uint ror(uint x, int d) { return (x >> d) | (x << (32-d)); }
uint sigma0(uint h1) { return ror(h1, 2) ^ ror(h1, 13) ^ ror(h1, 22); }
uint sigma1(uint h4) { return ror(h4, 6) ^ ror(h4, 11) ^ ror(h4, 25); }
uint gamma0(uint a) { return ror(a, 7) ^ ror(a, 18) ^ (a >> 3); }
uint gamma1(uint b) { return ror(b, 17) ^ ror(b, 19) ^ (b >> 10); }
uint ch(uint x, uint y, uint z) { return z ^ (x & (y ^ z)); }
uint maj(uint x, uint y, uint z) { return (x & y) ^ ((x ^ y) & z); }

uint init_H[8] = uint[8](
	0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
	0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
);

uint k[64] = uint[64](
	0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
	0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
	0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
	0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
	0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
	0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
	0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
	0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
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
