#version 310 es
precision mediump float;

void fwidthCoarse_4e4fc4() {
  vec4 res = fwidth(vec4(1.0f));
}

void fragment_main() {
  fwidthCoarse_4e4fc4();
}

void main() {
  fragment_main();
  return;
}
