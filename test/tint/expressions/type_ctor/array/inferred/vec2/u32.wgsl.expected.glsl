#version 310 es

uvec2 arr[2] = uvec2[2](uvec2(1u), uvec2(2u));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 v[2] = arr;
}
