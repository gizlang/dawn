fn sin_01f241() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = sin(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_01f241();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_01f241();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_01f241();
}
