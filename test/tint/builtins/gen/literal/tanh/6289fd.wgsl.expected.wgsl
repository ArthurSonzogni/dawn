fn tanh_6289fd() {
  var res = tanh(vec3(1.0));
}

@fragment
fn fragment_main() {
  tanh_6289fd();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_6289fd();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  tanh_6289fd();
  return out;
}
