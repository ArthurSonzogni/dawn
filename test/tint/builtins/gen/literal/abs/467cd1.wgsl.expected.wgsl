@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn abs_467cd1() -> u32 {
  var res : u32 = abs(1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = abs_467cd1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = abs_467cd1();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : u32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = abs_467cd1();
  return out;
}
