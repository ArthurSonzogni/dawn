#version 460

uniform highp samplerCubeArray t_f_1;
void tint_symbol() {
  uvec2 dims = uvec2(textureSize(t_f_1, 0).xy);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
