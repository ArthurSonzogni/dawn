@group(1) @binding(0) var arg_0 : texture_storage_3d<r8uint, write>;

fn textureStore_23c9f7() {
  textureStore(arg_0, vec3<i32>(1i), vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_23c9f7();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_23c9f7();
}
