SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupExclusiveAdd_4a1568() {
  float16_t res = WavePrefixSum(float16_t(1.0h));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupExclusiveAdd_4a1568());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000015CCCF51AB0(3,1-9): error X3000: unrecognized identifier 'float16_t'

