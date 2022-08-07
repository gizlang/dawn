RWByteAddressBuffer sb_rw : register(u0, space0);

uint tint_atomicMax(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedMax(offset, value, original_value);
  return original_value;
}


void atomicMax_51b9be() {
  uint arg_1 = 1u;
  uint res = tint_atomicMax(sb_rw, 0u, arg_1);
}

void fragment_main() {
  atomicMax_51b9be();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMax_51b9be();
  return;
}
