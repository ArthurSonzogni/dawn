enable f16;

var<private> u : vec4<f16> = vec4<f16>(vec4<f32>(1.0f));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
