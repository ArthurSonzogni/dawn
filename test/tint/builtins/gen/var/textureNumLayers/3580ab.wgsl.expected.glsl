//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uint inner;
} v;
layout(binding = 1, rg32i) uniform highp writeonly iimage2DArray f_arg_0;
uint textureNumLayers_3580ab() {
  uint res = uint(imageSize(f_arg_0).z);
  return res;
}
void main() {
  v.inner = textureNumLayers_3580ab();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
layout(binding = 1, rg32i) uniform highp writeonly iimage2DArray arg_0;
uint textureNumLayers_3580ab() {
  uint res = uint(imageSize(arg_0).z);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureNumLayers_3580ab();
}
