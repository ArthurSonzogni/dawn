enable f16;

var<private> u : vec2<i32> = vec2<i32>(vec2<f16>(1.0h));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
