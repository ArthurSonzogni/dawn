SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupAdd_2ab40a() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  vector<float16_t, 4> res = WaveActiveSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupAdd_2ab40a());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001AF7F43CA40(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

