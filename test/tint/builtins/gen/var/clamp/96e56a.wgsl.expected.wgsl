fn clamp_96e56a() {
  const arg_0 = 1;
  const arg_1 = 1;
  const arg_2 = 1;
  var res = clamp(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  clamp_96e56a();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_96e56a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  clamp_96e56a();
  return out;
}
