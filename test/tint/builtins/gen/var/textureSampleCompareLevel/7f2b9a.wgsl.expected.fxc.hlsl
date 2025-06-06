//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float textureSampleCompareLevel_7f2b9a() {
  float2 arg_2 = (1.0f).xx;
  float arg_3 = 1.0f;
  float res = arg_0.SampleCmpLevelZero(arg_1, arg_2, arg_3, int2((1).xx));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompareLevel_7f2b9a()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float textureSampleCompareLevel_7f2b9a() {
  float2 arg_2 = (1.0f).xx;
  float arg_3 = 1.0f;
  float res = arg_0.SampleCmpLevelZero(arg_1, arg_2, arg_3, int2((1).xx));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompareLevel_7f2b9a()));
  return;
}
//
// vertex_main
//
Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float textureSampleCompareLevel_7f2b9a() {
  float2 arg_2 = (1.0f).xx;
  float arg_3 = 1.0f;
  float res = arg_0.SampleCmpLevelZero(arg_1, arg_2, arg_3, int2((1).xx));
  return res;
}

struct VertexOutput {
  float4 pos;
  float prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation float prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureSampleCompareLevel_7f2b9a();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
