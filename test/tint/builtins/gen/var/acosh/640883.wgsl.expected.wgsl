@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn acosh_640883() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.54308068752288818359f);
  var res : vec2<f32> = acosh(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = acosh_640883();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = acosh_640883();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = acosh_640883();
  return out;
}
