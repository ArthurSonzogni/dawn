#version 310 es
precision highp float;
precision highp int;

struct modf_result_vec2_f32 {
  vec2 fract;
  vec2 whole;
};


void modf_732aa6() {
  modf_result_vec2_f32 res = modf_result_vec2_f32(vec2(-0.5f), vec2(-1.0f));
}

struct VertexOutput {
  vec4 pos;
};

void fragment_main() {
  modf_732aa6();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct modf_result_vec2_f32 {
  vec2 fract;
  vec2 whole;
};


void modf_732aa6() {
  modf_result_vec2_f32 res = modf_result_vec2_f32(vec2(-0.5f), vec2(-1.0f));
}

struct VertexOutput {
  vec4 pos;
};

void compute_main() {
  modf_732aa6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

struct modf_result_vec2_f32 {
  vec2 fract;
  vec2 whole;
};


void modf_732aa6() {
  modf_result_vec2_f32 res = modf_result_vec2_f32(vec2(-0.5f), vec2(-1.0f));
}

struct VertexOutput {
  vec4 pos;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_732aa6();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
