fn inverseSqrt_4ca6d6() {
  var res = inverseSqrt(1.0);
}

@fragment
fn fragment_main() {
  inverseSqrt_4ca6d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_4ca6d6();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  inverseSqrt_4ca6d6();
  return out;
}
