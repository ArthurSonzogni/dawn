enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn quadBroadcast_3c3824() -> vec2<f16> {
  var res : vec2<f16> = quadBroadcast(vec2<f16>(1.0h), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_3c3824();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_3c3824();
}
