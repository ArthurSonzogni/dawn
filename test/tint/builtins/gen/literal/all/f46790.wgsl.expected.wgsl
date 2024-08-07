@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn all_f46790() -> i32 {
  var res : bool = all(vec2<bool>(true));
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = all_f46790();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = all_f46790();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : i32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = all_f46790();
  return out;
}
