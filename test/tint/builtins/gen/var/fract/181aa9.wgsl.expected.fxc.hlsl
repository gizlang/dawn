SKIP: FAILED

void fract_181aa9() {
  vector<float16_t, 2> arg_0 = (float16_t(1.25h)).xx;
  vector<float16_t, 2> res = frac(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  fract_181aa9();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  fract_181aa9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_181aa9();
  return;
}
