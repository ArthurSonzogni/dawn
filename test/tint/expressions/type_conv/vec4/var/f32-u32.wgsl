var<private> u = vec4<f32>(1.0f);

@compute @workgroup_size(1)
fn f() {
    let v : vec4<u32> = vec4<u32>(u);
}
