@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn smoothstep_6c4975() -> f32 {
  var res : f32 = smoothstep(2.0f, 4.0f, 3.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = smoothstep_6c4975();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = smoothstep_6c4975();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = smoothstep_6c4975();
  return out;
}
