var<private> u = vec4<i32>(1i);

@compute @workgroup_size(1)
fn f() {
  let v : vec4<u32> = vec4<u32>(u);
}
