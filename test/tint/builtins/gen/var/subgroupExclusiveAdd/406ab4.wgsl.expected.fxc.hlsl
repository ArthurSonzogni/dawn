SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupExclusiveAdd_406ab4() {
  int4 arg_0 = (1).xxxx;
  int4 res = WavePrefixSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_406ab4()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001DCC507C5C0(5,14-33): error X3004: undeclared identifier 'WavePrefixSum'

