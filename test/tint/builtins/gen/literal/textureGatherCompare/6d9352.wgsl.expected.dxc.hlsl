Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureGatherCompare_6d9352() {
  float4 res = arg_0.GatherCmp(arg_1, (1.0f).xx, 1.0f);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureGatherCompare_6d9352();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureGatherCompare_6d9352();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureGatherCompare_6d9352();
  return;
}
