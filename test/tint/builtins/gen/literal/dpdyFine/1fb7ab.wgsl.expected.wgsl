fn dpdyFine_1fb7ab() {
  var res : vec3<f32> = dpdyFine(vec3<f32>(1.0f));
}

@fragment
fn fragment_main() {
  dpdyFine_1fb7ab();
}
