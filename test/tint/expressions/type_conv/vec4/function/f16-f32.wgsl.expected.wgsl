enable f16;

var<private> t : f16;

fn m() -> vec4<f16> {
  t = 1.0h;
  return vec4<f16>(t);
}

@compute @workgroup_size(1)
fn f() {
  var v : vec4<f32> = vec4<f32>(m());
}
