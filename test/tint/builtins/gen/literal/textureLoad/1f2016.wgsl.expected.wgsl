@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_3d<f32>;

fn textureLoad_1f2016() -> vec4<f32> {
  var res : vec4<f32> = textureLoad(arg_0, vec3<i32>(1i), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_1f2016();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_1f2016();
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
  out.prevent_dce = textureLoad_1f2016();
  return out;
}
