SKIP: FAILED

void tanh_5b19af() {
  float16_t arg_0 = float16_t(0.0h);
  float16_t res = tanh(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  tanh_5b19af();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  tanh_5b19af();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_5b19af();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001A07EE64130(2,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001A07EE64130(2,13-17): error X3000: unrecognized identifier 'arg_0'

