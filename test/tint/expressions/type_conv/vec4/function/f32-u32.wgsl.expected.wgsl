var<private> t : f32;

fn m() -> vec4<f32> {
  t = 1.0f;
  return vec4<f32>(t);
}

@compute @workgroup_size(1)
fn f() {
  var v : vec4<u32> = vec4<u32>(m());
}
