enable f16;

var<private> t : f16;

fn m() -> f16 {
  t = 1.0h;
  return f16(t);
}

@compute @workgroup_size(1)
fn f() {
  var v : bool = bool(m());
}
