#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16mat3x4 m = f16mat3x4(f16vec4(0.0hf, 1.0hf, 2.0hf, 3.0hf), f16vec4(4.0hf, 5.0hf, 6.0hf, 7.0hf), f16vec4(8.0hf, 9.0hf, 10.0hf, 11.0hf));
layout(binding = 0, std430)
buffer out_block_1_ssbo {
  f16mat3x4 inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = f16mat3x4(m);
}
