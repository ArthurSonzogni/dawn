SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupBroadcast_34fa3d() {
  uint3 res = WaveReadLaneAt((1u).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_34fa3d()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002247C411740(4,15-42): error X3004: undeclared identifier 'WaveReadLaneAt'

