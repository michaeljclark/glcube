#version 150

varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;
varying vec3 v_fragPos;
varying vec3 v_lightDir;

uint ror(uint x, int d) { return (x >> d) | (x << (32-d)); }
uint sigma0(uint h1) { return ror(h1, 2) ^ ror(h1, 13) ^ ror(h1, 22); }
uint sigma1(uint h4) { return ror(h4, 6) ^ ror(h4, 11) ^ ror(h4, 25); }
uint ch(uint x, uint y, uint z) { return z ^ (x & (y ^ z)); }
uint maj(uint x, uint y, uint z) { return (x & y) ^ ((x ^ y) & z); }

float random(vec2 v)
{
    uint H[8] = uint[] ( 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u );
    uint W[2];
    uint T0,T1;
    int i;

    uint x = uint(abs(v.x) * float(1u<<24));
    uint y = uint(abs(v.y) * float(1u<<24));
    x = x - x % (1u<<16);
    y = y - y % (1u<<16);

    W[0] = x | (y << 24);
    W[1] = (y >> 8) | (x << 16);

    for (i=0; i<8; i++) {
        T0 = W[i&1] + H[7] + sigma1(H[4]) + ch(H[4], H[5], H[6]);
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
