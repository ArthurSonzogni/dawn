//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<int4> arg_0 : register(u0, space1);
int4 textureLoad_5f6d30() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint v = arg_2;
  int2 v_1 = int2(arg_1);
  int4 res = arg_0.Load(int4(v_1, int(v), int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_5f6d30()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<int4> arg_0 : register(u0, space1);
int4 textureLoad_5f6d30() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint v = arg_2;
  int2 v_1 = int2(arg_1);
  int4 res = arg_0.Load(int4(v_1, int(v), int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_5f6d30()));
}

