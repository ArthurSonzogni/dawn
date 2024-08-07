fn transpose_dc671a() {
  var res = transpose(mat4x4(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0));
}

@fragment
fn fragment_main() {
  transpose_dc671a();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_dc671a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  transpose_dc671a();
  return out;
}
