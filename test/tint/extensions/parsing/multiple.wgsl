enable chromium_experimental_full_ptr_parameters, f16;

@fragment
fn main() -> @location(0) vec4<f32> {
    return vec4<f32>(0.1, 0.2, 0.3, 0.4);
}
