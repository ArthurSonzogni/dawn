#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void dpdxCoarse_c28641() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdx(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdxCoarse_c28641();
}

void main() {
  fragment_main();
  return;
}
