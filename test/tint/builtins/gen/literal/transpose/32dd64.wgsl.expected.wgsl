fn transpose_32dd64() {
  var res = transpose(mat3x4(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0));
}

@fragment
fn fragment_main() {
  transpose_32dd64();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_32dd64();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  transpose_32dd64();
  return out;
}
