void min_82b28f() {
  uint2 res = min((1u).xx, (1u).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  min_82b28f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  min_82b28f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_82b28f();
  return;
}
