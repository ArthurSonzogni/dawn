var<private> u = vec3<i32>(1i);

@compute @workgroup_size(1)
fn f() {
  let v : vec3<u32> = vec3<u32>(u);
}
