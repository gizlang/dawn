SKIP: FAILED

void asinh_95ab2b() {
  vector<float16_t, 4> res = (float16_t(0.881347656h)).xxxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  asinh_95ab2b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  asinh_95ab2b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asinh_95ab2b();
  return;
}
