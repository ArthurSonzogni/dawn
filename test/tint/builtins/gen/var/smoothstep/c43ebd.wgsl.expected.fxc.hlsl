SKIP: INVALID

//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> smoothstep_c43ebd() {
  vector<float16_t, 4> arg_0 = (float16_t(2.0h)).xxxx;
  vector<float16_t, 4> arg_1 = (float16_t(4.0h)).xxxx;
  vector<float16_t, 4> arg_2 = (float16_t(3.0h)).xxxx;
  vector<float16_t, 4> v = arg_0;
  vector<float16_t, 4> v_1 = clamp(((arg_2 - v) / (arg_1 - v)), (float16_t(0.0h)).xxxx, (float16_t(1.0h)).xxxx);
  vector<float16_t, 4> res = (v_1 * (v_1 * ((float16_t(3.0h)).xxxx - ((float16_t(2.0h)).xxxx * v_1))));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, smoothstep_c43ebd());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> smoothstep_c43ebd() {
  vector<float16_t, 4> arg_0 = (float16_t(2.0h)).xxxx;
  vector<float16_t, 4> arg_1 = (float16_t(4.0h)).xxxx;
  vector<float16_t, 4> arg_2 = (float16_t(3.0h)).xxxx;
  vector<float16_t, 4> v = arg_0;
  vector<float16_t, 4> v_1 = clamp(((arg_2 - v) / (arg_1 - v)), (float16_t(0.0h)).xxxx, (float16_t(1.0h)).xxxx);
  vector<float16_t, 4> res = (v_1 * (v_1 * ((float16_t(3.0h)).xxxx - ((float16_t(2.0h)).xxxx * v_1))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, smoothstep_c43ebd());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  vector<float16_t, 4> prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation vector<float16_t, 4> VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


vector<float16_t, 4> smoothstep_c43ebd() {
  vector<float16_t, 4> arg_0 = (float16_t(2.0h)).xxxx;
  vector<float16_t, 4> arg_1 = (float16_t(4.0h)).xxxx;
  vector<float16_t, 4> arg_2 = (float16_t(3.0h)).xxxx;
  vector<float16_t, 4> v = arg_0;
  vector<float16_t, 4> v_1 = clamp(((arg_2 - v) / (arg_1 - v)), (float16_t(0.0h)).xxxx, (float16_t(1.0h)).xxxx);
  vector<float16_t, 4> res = (v_1 * (v_1 * ((float16_t(3.0h)).xxxx - ((float16_t(2.0h)).xxxx * v_1))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = smoothstep_c43ebd();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

