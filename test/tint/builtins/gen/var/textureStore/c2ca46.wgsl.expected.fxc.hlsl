RWTexture2DArray<int4> arg_0 : register(u0, space1);

void textureStore_c2ca46() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  int4 arg_3 = (1).xxxx;
  arg_0[uint3(arg_1, arg_2)] = arg_3;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureStore_c2ca46();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureStore_c2ca46();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_c2ca46();
  return;
}
