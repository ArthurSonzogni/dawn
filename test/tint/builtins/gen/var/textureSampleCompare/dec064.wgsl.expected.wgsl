@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_2d;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompare_dec064() -> f32 {
  var arg_2 = vec2<f32>(1.0f);
  var arg_3 = 1.0f;
  const arg_4 = vec2<i32>(1i);
  var res : f32 = textureSampleCompare(arg_0, arg_1, arg_2, arg_3, arg_4);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSampleCompare_dec064();
}
