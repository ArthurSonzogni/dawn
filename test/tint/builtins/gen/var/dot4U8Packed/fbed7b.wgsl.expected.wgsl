fn dot4U8Packed_fbed7b() {
  var arg_0 = 1u;
  var arg_1 = 1u;
  var res : u32 = dot4U8Packed(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot4U8Packed_fbed7b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot4U8Packed_fbed7b();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot4U8Packed_fbed7b();
}
