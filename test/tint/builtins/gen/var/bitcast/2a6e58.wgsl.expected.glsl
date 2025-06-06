//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec2 inner;
} v;
vec2 tint_bitcast_from_f16(f16vec4 src) {
  return uintBitsToFloat(uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw)));
}
vec2 bitcast_2a6e58() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  vec2 res = tint_bitcast_from_f16(arg_0);
  return res;
}
void main() {
  v.inner = bitcast_2a6e58();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec2 inner;
} v;
vec2 tint_bitcast_from_f16(f16vec4 src) {
  return uintBitsToFloat(uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw)));
}
vec2 bitcast_2a6e58() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  vec2 res = tint_bitcast_from_f16(arg_0);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = bitcast_2a6e58();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  vec2 prevent_dce;
};

layout(location = 0) flat out vec2 tint_interstage_location0;
vec2 tint_bitcast_from_f16(f16vec4 src) {
  return uintBitsToFloat(uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw)));
}
vec2 bitcast_2a6e58() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  vec2 res = tint_bitcast_from_f16(arg_0);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), vec2(0.0f));
  v.pos = vec4(0.0f);
  v.prevent_dce = bitcast_2a6e58();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
