SKIP: FAILED

vector<float16_t, 2> tint_radians(vector<float16_t, 2> param_0) {
  return param_0 * 0.017453292519943295474;
}

void radians_fbacf0() {
  vector<float16_t, 2> res = tint_radians((float16_t(0.0h)).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  radians_fbacf0();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  radians_fbacf0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  radians_fbacf0();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000028D5138C350(1,8-16): error X3000: syntax error: unexpected token 'float16_t'

