#version 310 es

layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v;
uniform highp sampler2D t_f;
uniform highp isampler2D t_i;
uniform highp usampler2D t_u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = (v.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint fdims = uvec2(textureSize(t_f, int(min(uint(1), v_1)))).x;
  uint v_2 = (v.metadata[(1u / 4u)][(1u % 4u)] - 1u);
  uint idims = uvec2(textureSize(t_i, int(min(uint(1), v_2)))).x;
  uint v_3 = (v.metadata[(2u / 4u)][(2u % 4u)] - 1u);
  uint udims = uvec2(textureSize(t_u, int(min(uint(1), v_3)))).x;
}
