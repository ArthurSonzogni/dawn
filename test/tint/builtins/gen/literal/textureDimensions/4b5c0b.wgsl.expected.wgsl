requires texel_buffers;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba8sint, read_write>;

fn textureDimensions_4b5c0b() -> u32 {
  var res : u32 = textureDimensions(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_4b5c0b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_4b5c0b();
}
