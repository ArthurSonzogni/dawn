#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
uniform highp sampler3D f_arg_0_arg_1;
vec4 textureSampleBias_594824() {
  vec4 res = textureOffset(f_arg_0_arg_1, vec3(1.0f), ivec3(1), clamp(1.0f, -16.0f, 15.9899997711181640625f));
  return res;
}
void main() {
  v.inner = textureSampleBias_594824();
}
