//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  int inner;
} v;
int select_c31f9e() {
  bool arg_0 = true;
  bool arg_1 = true;
  bool arg_2 = true;
  bool res = mix(arg_0, arg_1, arg_2);
  return mix(0, 1, (res == false));
}
void main() {
  v.inner = select_c31f9e();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int select_c31f9e() {
  bool arg_0 = true;
  bool arg_1 = true;
  bool arg_2 = true;
  bool res = mix(arg_0, arg_1, arg_2);
  return mix(0, 1, (res == false));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = select_c31f9e();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int tint_interstage_location0;
int select_c31f9e() {
  bool arg_0 = true;
  bool arg_1 = true;
  bool arg_2 = true;
  bool res = mix(arg_0, arg_1, arg_2);
  return mix(0, 1, (res == false));
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), 0);
  v.pos = vec4(0.0f);
  v.prevent_dce = select_c31f9e();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
