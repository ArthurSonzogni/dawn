var<private> arr = array<array<f32, 2>, 2>(array<f32, 2>(1.0f, 2.0f), array<f32, 2>(3.0f, 4.0f));

@compute @workgroup_size(1)
fn f() {
  var v = arr;
}
