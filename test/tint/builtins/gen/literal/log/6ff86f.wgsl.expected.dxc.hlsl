void log_6ff86f() {
  vector<float16_t, 3> res = log((float16_t(0.0h)).xxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  log_6ff86f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  log_6ff86f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log_6ff86f();
  return;
}
