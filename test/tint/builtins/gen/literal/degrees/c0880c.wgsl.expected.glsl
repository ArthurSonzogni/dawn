//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

void degrees_c0880c() {
  vec3 res = vec3(57.295780181884765625f);
}
void main() {
  degrees_c0880c();
}
//
// compute_main
//
#version 310 es

void degrees_c0880c() {
  vec3 res = vec3(57.295780181884765625f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  degrees_c0880c();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void degrees_c0880c() {
  vec3 res = vec3(57.295780181884765625f);
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  degrees_c0880c();
  return v;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
