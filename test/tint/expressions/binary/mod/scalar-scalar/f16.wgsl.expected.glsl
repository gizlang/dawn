#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_float_modulo(float16_t lhs, float16_t rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


void f() {
  float16_t r = tint_float_modulo(1.0hf, 2.0hf);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
