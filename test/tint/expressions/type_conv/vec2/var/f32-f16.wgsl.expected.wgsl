enable f16;

var<private> u = vec2<f32>(1.0f);

@compute @workgroup_size(1)
fn f() {
  let v : vec2<f16> = vec2<f16>(u);
}
