//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct frexp_result_f32 {
  float member_0;
  int member_1;
};

void frexp_4b2200() {
  float arg_0 = 1.0f;
  frexp_result_f32 v = frexp_result_f32(0.0f, 0);
  v.member_0 = frexp(arg_0, v.member_1);
  frexp_result_f32 res = v;
}
void main() {
  frexp_4b2200();
}
//
// compute_main
//
#version 310 es


struct frexp_result_f32 {
  float member_0;
  int member_1;
};

void frexp_4b2200() {
  float arg_0 = 1.0f;
  frexp_result_f32 v = frexp_result_f32(0.0f, 0);
  v.member_0 = frexp(arg_0, v.member_1);
  frexp_result_f32 res = v;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_4b2200();
}
//
// vertex_main
//
#version 310 es


struct frexp_result_f32 {
  float member_0;
  int member_1;
};

struct VertexOutput {
  vec4 pos;
};

void frexp_4b2200() {
  float arg_0 = 1.0f;
  frexp_result_f32 v = frexp_result_f32(0.0f, 0);
  v.member_0 = frexp(arg_0, v.member_1);
  frexp_result_f32 res = v;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f));
  v_1.pos = vec4(0.0f);
  frexp_4b2200();
  return v_1;
}
void main() {
  vec4 v_2 = vertex_main_inner().pos;
  gl_Position = vec4(v_2.x, -(v_2.y), ((2.0f * v_2.z) - v_2.w), v_2.w);
  gl_PointSize = 1.0f;
}
