@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32uint, write>;

fn textureNumLayers_9695c6() -> u32 {
  var res : u32 = textureNumLayers(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureNumLayers_9695c6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureNumLayers_9695c6();
}
