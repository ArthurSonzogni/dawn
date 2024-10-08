#version 310 es

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % mix(rhs, 1u, (rhs == 0u)));
}

void f(uvec3 v) {
  uint l = (v.x << (tint_mod(v.y, 1u) & 31u));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_GlobalInvocationID);
  return;
}
