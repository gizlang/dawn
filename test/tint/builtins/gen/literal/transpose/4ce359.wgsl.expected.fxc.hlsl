void transpose_4ce359() {
  float4x2 res = transpose(float2x4((1.0f).xxxx, (1.0f).xxxx));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_4ce359();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_4ce359();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_4ce359();
  return;
}
