#version 310 es

uvec2 u = uvec2(1u);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec2 v = ivec2(u);
}
