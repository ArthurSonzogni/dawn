#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  vec3 tint_symbol_col0;
  uint tint_pad;
  vec3 tint_symbol_col1;
  uint tint_pad_1;
  vec3 tint_symbol_col2;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3 t = transpose(mat3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2));
  float l = length(mat3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2)[1]);
  float a = abs(mat3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2)[0].zxy[0u]);
}
