fn sign_0799fd() {
  const arg_0 = vec2(1.0);
  var res = sign(arg_0);
}

@fragment
fn fragment_main() {
  sign_0799fd();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_0799fd();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sign_0799fd();
  return out;
}
