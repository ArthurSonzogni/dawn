struct S {
  int before;
  float3x4 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(float3x4 m) {
}

void d(float4 v) {
}

void e(float f_1) {
}

float3x4 v_1(uint start_byte_offset) {
  return float3x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]), asfloat(u[((32u + start_byte_offset) / 16u)]));
}

S v_2(uint start_byte_offset) {
  int v_3 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float3x4 v_4 = v_1((16u + start_byte_offset));
  S v_5 = {v_3, v_4, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_5;
}

typedef S ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  S a_2[4] = (S[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      S v_9 = v_2((start_byte_offset + (v_8 * 128u)));
      a_2[v_8] = v_9;
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  S v_10[4] = a_2;
  return v_10;
}

[numthreads(1, 1, 1)]
void f() {
  S v_11[4] = v_6(0u);
  a(v_11);
  S v_12 = v_2(256u);
  b(v_12);
  c(v_1(272u));
  d(asfloat(u[2u]).ywxz);
  e(asfloat(u[2u]).ywxz.x);
}

