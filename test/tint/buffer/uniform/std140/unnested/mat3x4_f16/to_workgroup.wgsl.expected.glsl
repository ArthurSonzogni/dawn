#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  f16vec4 inner_col0;
  f16vec4 inner_col1;
  f16vec4 inner_col2;
} v;
shared f16mat3x4 w;
void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf));
  }
  barrier();
  w = f16mat3x4(v.inner_col0, v.inner_col1, v.inner_col2);
  w[1u] = f16mat3x4(v.inner_col0, v.inner_col1, v.inner_col2)[0u];
  w[1u] = f16mat3x4(v.inner_col0, v.inner_col1, v.inner_col2)[0u].ywxz;
  w[0u].y = f16mat3x4(v.inner_col0, v.inner_col1, v.inner_col2)[1u].x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
