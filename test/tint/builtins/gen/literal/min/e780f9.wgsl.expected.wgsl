enable f16;

fn min_e780f9() {
  var res : vec2<f16> = min(vec2<f16>(f16()), vec2<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_e780f9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_e780f9();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_e780f9();
}
