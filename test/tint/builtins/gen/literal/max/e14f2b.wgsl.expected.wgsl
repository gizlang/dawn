enable f16;

fn max_e14f2b() {
  var res : vec4<f16> = max(vec4<f16>(f16()), vec4<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_e14f2b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_e14f2b();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_e14f2b();
}
