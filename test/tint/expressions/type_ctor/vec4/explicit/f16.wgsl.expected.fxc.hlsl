SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static vector<float16_t, 4> v = vector<float16_t, 4>(float16_t(0.0h), float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001D7941D2A60(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

