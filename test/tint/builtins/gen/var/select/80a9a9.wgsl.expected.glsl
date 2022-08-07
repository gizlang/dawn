#version 310 es

void select_80a9a9() {
  bvec3 arg_0 = bvec3(true);
  bvec3 arg_1 = bvec3(true);
  bvec3 arg_2 = bvec3(true);
  bvec3 res = mix(arg_0, arg_1, arg_2);
}

vec4 vertex_main() {
  select_80a9a9();
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

void select_80a9a9() {
  bvec3 arg_0 = bvec3(true);
  bvec3 arg_1 = bvec3(true);
  bvec3 arg_2 = bvec3(true);
  bvec3 res = mix(arg_0, arg_1, arg_2);
}

void fragment_main() {
  select_80a9a9();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void select_80a9a9() {
  bvec3 arg_0 = bvec3(true);
  bvec3 arg_1 = bvec3(true);
  bvec3 arg_2 = bvec3(true);
  bvec3 res = mix(arg_0, arg_1, arg_2);
}

void compute_main() {
  select_80a9a9();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
