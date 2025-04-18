//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint max_0c0aae() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint res = max(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(max_0c0aae()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint max_0c0aae() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint res = max(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(max_0c0aae()));
  return;
}
//
// vertex_main
//
uint max_0c0aae() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint res = max(arg_0, arg_1);
  return res;
}

struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation uint prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = max_0c0aae();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
