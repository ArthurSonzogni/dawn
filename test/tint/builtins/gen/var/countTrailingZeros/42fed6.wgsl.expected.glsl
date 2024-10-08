#version 310 es
precision highp float;
precision highp int;

int tint_count_trailing_zeros(int v) {
  uint x = uint(v);
  uint b16 = mix(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  uint b8 = mix(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  uint b4 = mix(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  uint b2 = mix(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  uint b1 = mix(1u, 0u, bool((x & 1u)));
  uint is_zero = mix(0u, 1u, (x == 0u));
  return int((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

int countTrailingZeros_42fed6() {
  int arg_0 = 1;
  int res = tint_count_trailing_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = countTrailingZeros_42fed6();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

int tint_count_trailing_zeros(int v) {
  uint x = uint(v);
  uint b16 = mix(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  uint b8 = mix(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  uint b4 = mix(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  uint b2 = mix(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  uint b1 = mix(1u, 0u, bool((x & 1u)));
  uint is_zero = mix(0u, 1u, (x == 0u));
  return int((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

int countTrailingZeros_42fed6() {
  int arg_0 = 1;
  int res = tint_count_trailing_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

void compute_main() {
  prevent_dce.inner = countTrailingZeros_42fed6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

int tint_count_trailing_zeros(int v) {
  uint x = uint(v);
  uint b16 = mix(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  uint b8 = mix(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  uint b4 = mix(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  uint b2 = mix(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  uint b1 = mix(1u, 0u, bool((x & 1u)));
  uint is_zero = mix(0u, 1u, (x == 0u));
  return int((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(location = 0) flat out int prevent_dce_1;
int countTrailingZeros_42fed6() {
  int arg_0 = 1;
  int res = tint_count_trailing_zeros(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countTrailingZeros_42fed6();
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
