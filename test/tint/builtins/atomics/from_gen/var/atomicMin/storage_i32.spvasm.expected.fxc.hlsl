RWByteAddressBuffer sb_rw : register(u0, space0);

int tint_atomicMin(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedMin(offset, value, original_value);
  return original_value;
}


void atomicMin_8e38dc() {
  int arg_1 = 0;
  int res = 0;
  arg_1 = 1;
  const int x_13 = tint_atomicMin(sb_rw, 0u, arg_1);
  res = x_13;
  return;
}

void fragment_main_1() {
  atomicMin_8e38dc();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicMin_8e38dc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
