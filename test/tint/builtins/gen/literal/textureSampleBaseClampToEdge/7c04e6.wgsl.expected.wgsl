@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_external;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleBaseClampToEdge_7c04e6() -> vec4<f32> {
  var res : vec4<f32> = textureSampleBaseClampToEdge(arg_0, arg_1, vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSampleBaseClampToEdge_7c04e6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureSampleBaseClampToEdge_7c04e6();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureSampleBaseClampToEdge_7c04e6();
  return out;
}
