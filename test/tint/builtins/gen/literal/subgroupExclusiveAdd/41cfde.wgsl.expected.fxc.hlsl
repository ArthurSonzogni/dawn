SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupExclusiveAdd_41cfde() {
  float3 res = WavePrefixSum((1.0f).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_41cfde()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002044F4DC550(4,16-40): error X3004: undeclared identifier 'WavePrefixSum'

