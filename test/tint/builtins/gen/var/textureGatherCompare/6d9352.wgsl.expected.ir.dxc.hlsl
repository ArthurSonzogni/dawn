//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float4 textureGatherCompare_6d9352() {
  float2 arg_2 = (1.0f).xx;
  float arg_3 = 1.0f;
  float4 res = arg_0.GatherCmp(arg_1, arg_2, arg_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureGatherCompare_6d9352()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float4 textureGatherCompare_6d9352() {
  float2 arg_2 = (1.0f).xx;
  float arg_3 = 1.0f;
  float4 res = arg_0.GatherCmp(arg_1, arg_2, arg_3);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureGatherCompare_6d9352()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float4 textureGatherCompare_6d9352() {
  float2 arg_2 = (1.0f).xx;
  float arg_3 = 1.0f;
  float4 res = arg_0.GatherCmp(arg_1, arg_2, arg_3);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = textureGatherCompare_6d9352();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

