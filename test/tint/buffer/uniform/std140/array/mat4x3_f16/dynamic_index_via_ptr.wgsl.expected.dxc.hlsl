cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

matrix<float16_t, 4, 3> a_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = a[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = a[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = a[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  float16_t ubo_load_4_y = f16tof32(ubo_load_4[0] >> 16);
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_7 = a[scalar_offset_3 / 4];
  uint2 ubo_load_6 = ((scalar_offset_3 & 2) ? ubo_load_7.zw : ubo_load_7.xy);
  vector<float16_t, 2> ubo_load_6_xz = vector<float16_t, 2>(f16tof32(ubo_load_6 & 0xFFFF));
  float16_t ubo_load_6_y = f16tof32(ubo_load_6[0] >> 16);
  return matrix<float16_t, 4, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]), vector<float16_t, 3>(ubo_load_4_xz[0], ubo_load_4_y, ubo_load_4_xz[1]), vector<float16_t, 3>(ubo_load_6_xz[0], ubo_load_6_y, ubo_load_6_xz[1]));
}

typedef matrix<float16_t, 4, 3> a_load_ret[4];
a_load_ret a_load(uint offset) {
  matrix<float16_t, 4, 3> arr[4] = (matrix<float16_t, 4, 3>[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = a_load_1((offset + (i_1 * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  int p_a_i_save = i();
  int p_a_i_i_save = i();
  matrix<float16_t, 4, 3> l_a[4] = a_load(0u);
  matrix<float16_t, 4, 3> l_a_i = a_load_1((32u * min(uint(p_a_i_save), 3u)));
  const uint scalar_offset_4 = (((32u * min(uint(p_a_i_save), 3u)) + (8u * min(uint(p_a_i_i_save), 3u)))) / 4;
  uint4 ubo_load_9 = a[scalar_offset_4 / 4];
  uint2 ubo_load_8 = ((scalar_offset_4 & 2) ? ubo_load_9.zw : ubo_load_9.xy);
  vector<float16_t, 2> ubo_load_8_xz = vector<float16_t, 2>(f16tof32(ubo_load_8 & 0xFFFF));
  float16_t ubo_load_8_y = f16tof32(ubo_load_8[0] >> 16);
  vector<float16_t, 3> l_a_i_i = vector<float16_t, 3>(ubo_load_8_xz[0], ubo_load_8_y, ubo_load_8_xz[1]);
  const uint scalar_offset_bytes = (((32u * min(uint(p_a_i_save), 3u)) + (8u * min(uint(p_a_i_i_save), 3u))));
  const uint scalar_offset_index = scalar_offset_bytes / 4;
  s.Store<float16_t>(0u, (((float16_t(f16tof32(((a[scalar_offset_index / 4][scalar_offset_index % 4] >> (scalar_offset_bytes % 4 == 0 ? 0 : 16)) & 0xFFFF))) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x));
  return;
}
