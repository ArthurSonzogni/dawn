@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn smoothstep_40864c() -> vec4<f32> {
  var arg_0 = vec4<f32>(2.0f);
  var arg_1 = vec4<f32>(4.0f);
  var arg_2 = vec4<f32>(3.0f);
  var res : vec4<f32> = smoothstep(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = smoothstep_40864c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = smoothstep_40864c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = smoothstep_40864c();
  return out;
}
