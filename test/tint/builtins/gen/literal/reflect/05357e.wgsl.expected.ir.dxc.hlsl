//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 reflect_05357e() {
  float4 res = (-7.0f).xxxx;
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(reflect_05357e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 reflect_05357e() {
  float4 res = (-7.0f).xxxx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(reflect_05357e()));
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


float4 reflect_05357e() {
  float4 res = (-7.0f).xxxx;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = reflect_05357e();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

