SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupOr_4d4eb0() {
  int4 arg_0 = (1).xxxx;
  int4 res = asint(WaveActiveBitOr(asuint(arg_0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupOr_4d4eb0()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000017424300340(5,20-49): error X3004: undeclared identifier 'WaveActiveBitOr'

