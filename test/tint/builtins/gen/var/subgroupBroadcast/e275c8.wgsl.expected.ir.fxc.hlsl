SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupBroadcast_e275c8() {
  int3 arg_0 = (1).xxx;
  int3 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_e275c8()));
}

FXC validation failure:
c:\src\dawn\Shader@0x000001BCE1E03F50(5,14-38): error X3004: undeclared identifier 'WaveReadLaneAt'

