#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

ivec2 tint_bitcast_from_f16(f16vec4 src) {
  return ivec2(uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16vec4 a = f16vec4(1.0hf, 2.0hf, 3.0hf, -4.0hf);
  ivec2 b = tint_bitcast_from_f16(a);
}
