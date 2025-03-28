enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupShuffleXor_1f2590() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var arg_1 = 1u;
  var res : vec3<f16> = subgroupShuffleXor(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_1f2590();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_1f2590();
}
