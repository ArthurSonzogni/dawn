#version 310 es

uniform highp isampler2DMS arg_0_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureLoad_7bee94() {
  ivec4 res = texelFetch(arg_0_1, ivec2(uvec2(1u)), 1);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureLoad_7bee94();
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

uniform highp isampler2DMS arg_0_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureLoad_7bee94() {
  ivec4 res = texelFetch(arg_0_1, ivec2(uvec2(1u)), 1);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureLoad_7bee94();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp isampler2DMS arg_0_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureLoad_7bee94() {
  ivec4 res = texelFetch(arg_0_1, ivec2(uvec2(1u)), 1);
  prevent_dce.inner = res;
}

void compute_main() {
  textureLoad_7bee94();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
