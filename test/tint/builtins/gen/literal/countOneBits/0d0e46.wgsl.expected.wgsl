@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn countOneBits_0d0e46() -> vec4<u32> {
  var res : vec4<u32> = countOneBits(vec4<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = countOneBits_0d0e46();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = countOneBits_0d0e46();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = countOneBits_0d0e46();
  return out;
}
