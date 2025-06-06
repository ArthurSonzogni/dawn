#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  f16vec4 inner_col0;
  f16vec4 inner_col1;
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  f16mat2x4 inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = f16mat2x4(v.inner_col0, v.inner_col1);
  v_1.inner[1u] = f16mat2x4(v.inner_col0, v.inner_col1)[0u];
  v_1.inner[1u] = f16mat2x4(v.inner_col0, v.inner_col1)[0u].ywxz;
  v_1.inner[0u].y = f16mat2x4(v.inner_col0, v.inner_col1)[1u].x;
}
