SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupAdd_0dd12a() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  vector<float16_t, 3> res = WaveActiveSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupAdd_0dd12a());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000016724106660(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

