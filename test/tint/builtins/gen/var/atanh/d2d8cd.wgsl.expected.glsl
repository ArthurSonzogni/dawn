#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

float16_t tint_atanh(float16_t x) {
  return mix(atanh(x), 0.0hf, (x >= 1.0hf));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

float16_t atanh_d2d8cd() {
  float16_t arg_0 = 0.5hf;
  float16_t res = tint_atanh(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  float16_t prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = atanh_d2d8cd();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_atanh(float16_t x) {
  return mix(atanh(x), 0.0hf, (x >= 1.0hf));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

float16_t atanh_d2d8cd() {
  float16_t arg_0 = 0.5hf;
  float16_t res = tint_atanh(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  float16_t prevent_dce;
};

void compute_main() {
  prevent_dce.inner = atanh_d2d8cd();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_atanh(float16_t x) {
  return mix(atanh(x), 0.0hf, (x >= 1.0hf));
}

layout(location = 0) flat out float16_t prevent_dce_1;
float16_t atanh_d2d8cd() {
  float16_t arg_0 = 0.5hf;
  float16_t res = tint_atanh(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  float16_t prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0.0hf);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = atanh_d2d8cd();
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
