RWByteAddressBuffer prevent_dce : register(u0, space2);

void atan2_93febc() {
  vector<float16_t, 2> res = (float16_t(0.78515625h)).xx;
  prevent_dce.Store<vector<float16_t, 2> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atan2_93febc();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atan2_93febc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_93febc();
  return;
}
