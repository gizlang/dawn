SKIP: FAILED

void mix_e46a83() {
  vector<float16_t, 2> res = lerp((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, float16_t(0.0h));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  mix_e46a83();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  mix_e46a83();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_e46a83();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001F8B7BA68A0(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

