fn max_482d23() {
  const arg_0 = vec3(1);
  const arg_1 = vec3(1);
  var res = max(arg_0, arg_1);
}

@fragment
fn fragment_main() {
  max_482d23();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_482d23();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  max_482d23();
  return out;
}
