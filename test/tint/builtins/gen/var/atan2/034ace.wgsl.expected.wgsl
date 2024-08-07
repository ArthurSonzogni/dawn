fn atan2_034ace() {
  const arg_0 = 1.0;
  const arg_1 = 1.0;
  var res = atan2(arg_0, arg_1);
}

@fragment
fn fragment_main() {
  atan2_034ace();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_034ace();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  atan2_034ace();
  return out;
}
