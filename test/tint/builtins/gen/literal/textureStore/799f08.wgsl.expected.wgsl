@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r16snorm, read_write>;

fn textureStore_799f08() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_799f08();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_799f08();
}
