enable f16;

var<private> t : f16;

fn m() -> vec3<f16> {
  t = 1.0h;
  return vec3<f16>(t);
}

@compute @workgroup_size(1)
fn f() {
  var v : vec3<bool> = vec3<bool>(m());
}
