#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

uniform highp usamplerCubeArray arg_0_1;
uvec2 textureDimensions_22b5b6() {
  int arg_1 = 1;
  uvec2 res = uvec2(textureSize(arg_0_1, arg_1).xy);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = textureDimensions_22b5b6();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

uniform highp usamplerCubeArray arg_0_1;
uvec2 textureDimensions_22b5b6() {
  int arg_1 = 1;
  uvec2 res = uvec2(textureSize(arg_0_1, arg_1).xy);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = textureDimensions_22b5b6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 460

layout(location = 0) flat out uvec2 prevent_dce_1;
uniform highp usamplerCubeArray arg_0_1;
uvec2 textureDimensions_22b5b6() {
  int arg_1 = 1;
  uvec2 res = uvec2(textureSize(arg_0_1, arg_1).xy);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), uvec2(0u, 0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureDimensions_22b5b6();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  prevent_dce_1 = inner_result.prevent_dce;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
