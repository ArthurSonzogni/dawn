var<private> t : bool;

fn m() -> vec4<bool> {
  t = true;
  return vec4<bool>(t);
}

@compute @workgroup_size(1)
fn f() {
  var v : vec4<u32> = vec4<u32>(m());
}
