fn tan_7be368() {
  var res = tan(vec2(1.0));
}

@fragment
fn fragment_main() {
  tan_7be368();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_7be368();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  tan_7be368();
  return out;
}
