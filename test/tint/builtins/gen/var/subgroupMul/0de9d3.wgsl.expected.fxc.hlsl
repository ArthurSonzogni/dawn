SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupMul_0de9d3() {
  float arg_0 = 1.0f;
  float res = WaveActiveProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_0de9d3()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000190E3105D30(5,15-38): error X3004: undeclared identifier 'WaveActiveProduct'

