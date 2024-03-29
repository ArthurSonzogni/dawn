#version 310 es

uniform highp sampler2DArray arg_1_arg_2;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureGather_ea8eb4() {
  vec2 arg_3 = vec2(1.0f);
  uint arg_4 = 1u;
  vec4 res = textureGather(arg_1_arg_2, vec3(arg_3, float(arg_4)), int(1u));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureGather_ea8eb4();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2DArray arg_1_arg_2;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureGather_ea8eb4() {
  vec2 arg_3 = vec2(1.0f);
  uint arg_4 = 1u;
  vec4 res = textureGather(arg_1_arg_2, vec3(arg_3, float(arg_4)), int(1u));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureGather_ea8eb4();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp sampler2DArray arg_1_arg_2;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureGather_ea8eb4() {
  vec2 arg_3 = vec2(1.0f);
  uint arg_4 = 1u;
  vec4 res = textureGather(arg_1_arg_2, vec3(arg_3, float(arg_4)), int(1u));
  prevent_dce.inner = res;
}

void compute_main() {
  textureGather_ea8eb4();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
