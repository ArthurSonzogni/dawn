var<private> u = i32(1i);

@compute @workgroup_size(1)
fn f() {
    let v : f32 = f32(u);
}
