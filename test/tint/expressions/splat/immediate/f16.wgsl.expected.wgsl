enable f16;

@compute @workgroup_size(1)
fn f() {
  var v2 : vec2<f16> = vec2<f16>(1.0h);
  var v3 : vec3<f16> = vec3<f16>(1.0h);
  var v4 : vec4<f16> = vec4<f16>(1.0h);
}
