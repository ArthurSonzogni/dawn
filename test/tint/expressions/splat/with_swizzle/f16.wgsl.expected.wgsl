enable f16;

@compute @workgroup_size(1)
fn f() {
  var a = vec2<f16>(1.0h).y;
  var b = vec3<f16>(1.0h).z;
  var c = vec4<f16>(1.0h).w;
}
