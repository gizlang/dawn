fn select_c31f9e() {
  var arg_0 = true;
  var arg_1 = true;
  var arg_2 = true;
  var res : bool = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_c31f9e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_c31f9e();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_c31f9e();
}
