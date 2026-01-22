
// source: https://gist.github.com/mattatz/40a91588d5fb38240403f198a938a593

vec4 quat_identity() {
  return vec4(0, 0, 0, 1);
}

// Quaternion multiplication
// http://mathworld.wolfram.com/Quaternion.html
vec4 quat_mul(vec4 q1, vec4 q2) {
  return vec4(
    q2.xyz * q1.w + q1.xyz * q2.w + cross(q1.xyz, q2.xyz),
    q1.w * q2.w - dot(q1.xyz, q2.xyz)
  );
}

// Vector rotation with a quaternion
// http://mathworld.wolfram.com/Quaternion.html
vec3 quat_rotate_vector(vec3 v, vec4 r) {
  vec4 r_c = r * vec4(-1, -1, -1, 1);
  return quat_mul(r, quat_mul(vec4(v, 0), r_c)).xyz;
}

// A given angle of rotation about a given axis
vec4 quat_rotate_around_axis(vec3 axis, float angle) {
  float sn = sin(angle * 0.5);
  float cs = cos(angle * 0.5);
  return vec4(axis * sn, cs);
}
