//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec2 inner;
} v;
uvec2 select_1e960b() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  bvec2 arg_2 = bvec2(true);
  uvec2 res = mix(arg_0, arg_1, arg_2);
  return res;
}
void main() {
  v.inner = select_1e960b();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec2 inner;
} v;
uvec2 select_1e960b() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  bvec2 arg_2 = bvec2(true);
  uvec2 res = mix(arg_0, arg_1, arg_2);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = select_1e960b();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

layout(location = 0) flat out uvec2 tint_interstage_location0;
uvec2 select_1e960b() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  bvec2 arg_2 = bvec2(true);
  uvec2 res = mix(arg_0, arg_1, arg_2);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), uvec2(0u));
  v.pos = vec4(0.0f);
  v.prevent_dce = select_1e960b();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
