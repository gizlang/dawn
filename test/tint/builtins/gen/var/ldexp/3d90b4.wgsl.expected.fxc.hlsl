SKIP: FAILED

void ldexp_3d90b4() {
  vector<float16_t, 2> arg_0 = (float16_t(0.0h)).xx;
  int2 arg_1 = (1).xx;
  vector<float16_t, 2> res = ldexp(arg_0, arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ldexp_3d90b4();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ldexp_3d90b4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_3d90b4();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000297E2767C20(2,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000297E2767C20(4,10-18): error X3000: syntax error: unexpected token 'float16_t'

