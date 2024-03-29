#version 310 es

layout(r32i) uniform highp writeonly iimage2DArray arg_0;
void textureStore_28a7ec() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), ivec4(1));
}

vec4 vertex_main() {
  textureStore_28a7ec();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;
precision highp int;

layout(r32i) uniform highp writeonly iimage2DArray arg_0;
void textureStore_28a7ec() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), ivec4(1));
}

void fragment_main() {
  textureStore_28a7ec();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(r32i) uniform highp writeonly iimage2DArray arg_0;
void textureStore_28a7ec() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), ivec4(1));
}

void compute_main() {
  textureStore_28a7ec();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
