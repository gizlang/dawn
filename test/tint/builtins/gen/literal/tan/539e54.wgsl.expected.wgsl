enable f16;

fn tan_539e54() {
  var res : vec4<f16> = tan(vec4<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_539e54();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_539e54();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_539e54();
}
