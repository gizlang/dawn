SKIP: FAILED

void sinh_924f19() {
  vector<float16_t, 2> arg_0 = (float16_t(0.0h)).xx;
  vector<float16_t, 2> res = sinh(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  sinh_924f19();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  sinh_924f19();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sinh_924f19();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000023F87A98990(2,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000023F87A98990(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

