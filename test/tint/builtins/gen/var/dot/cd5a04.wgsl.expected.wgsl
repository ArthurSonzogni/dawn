enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn dot_cd5a04() -> f16 {
  var arg_0 = vec2<f16>(1.0h);
  var arg_1 = vec2<f16>(1.0h);
  var res : f16 = dot(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dot_cd5a04();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = dot_cd5a04();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f16,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = dot_cd5a04();
  return out;
}
