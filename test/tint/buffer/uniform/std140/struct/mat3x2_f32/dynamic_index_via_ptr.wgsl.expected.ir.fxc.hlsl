struct Inner {
  float3x2 m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

float3x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = a[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = a[((16u + start_byte_offset) / 16u)];
  return float3x2(v_2, v_4, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

Inner v_6(uint start_byte_offset) {
  Inner v_7 = {v(start_byte_offset)};
  return v_7;
}

typedef Inner ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      Inner v_11 = v_6((start_byte_offset + (v_10 * 64u)));
      a_2[v_10] = v_11;
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  Inner v_12[4] = a_2;
  return v_12;
}

Outer v_13(uint start_byte_offset) {
  Inner v_14[4] = v_8(start_byte_offset);
  Outer v_15 = {v_14};
  return v_15;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_16(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      Outer v_19 = v_13((start_byte_offset + (v_18 * 256u)));
      a_1[v_18] = v_19;
      {
        v_17 = (v_18 + 1u);
      }
      continue;
    }
  }
  Outer v_20[4] = a_1;
  return v_20;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_21 = (256u * min(uint(i()), 3u));
  uint v_22 = (64u * min(uint(i()), 3u));
  uint v_23 = (8u * min(uint(i()), 2u));
  Outer l_a[4] = v_16(0u);
  Outer l_a_i = v_13(v_21);
  Inner l_a_i_a[4] = v_8(v_21);
  Inner l_a_i_a_i = v_6((v_21 + v_22));
  float3x2 l_a_i_a_i_m = v((v_21 + v_22));
  uint4 v_24 = a[(((v_21 + v_22) + v_23) / 16u)];
  float2 l_a_i_a_i_m_i = asfloat((((((((v_21 + v_22) + v_23) % 16u) / 4u) == 2u)) ? (v_24.zw) : (v_24.xy)));
  uint v_25 = (((v_21 + v_22) + v_23) + (min(uint(i()), 1u) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_25 / 16u)][((v_25 % 16u) / 4u)]);
}

