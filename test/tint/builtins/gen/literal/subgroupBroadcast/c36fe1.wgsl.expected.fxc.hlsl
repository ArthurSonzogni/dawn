SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupBroadcast_c36fe1() {
  uint res = WaveReadLaneAt(1u, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_c36fe1()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000028F3D630CD0(4,14-35): error X3004: undeclared identifier 'WaveReadLaneAt'

