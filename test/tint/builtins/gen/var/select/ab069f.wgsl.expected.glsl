#version 310 es

void select_ab069f() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  bool arg_2 = true;
  ivec4 res = (arg_2 ? arg_1 : arg_0);
}

vec4 vertex_main() {
  select_ab069f();
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
precision mediump float;

void select_ab069f() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  bool arg_2 = true;
  ivec4 res = (arg_2 ? arg_1 : arg_0);
}

void fragment_main() {
  select_ab069f();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void select_ab069f() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  bool arg_2 = true;
  ivec4 res = (arg_2 ? arg_1 : arg_0);
}

void compute_main() {
  select_ab069f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
