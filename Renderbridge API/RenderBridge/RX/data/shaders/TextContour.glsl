
//@implements: sampler2D
struct TextContour {
  sampler2D sampler;
};

vec4 texture(TextContour s, vec2 texcoord) {
  vec4 color = texture(s.sampler, texcoord);
  
  vec3 arccoords = v_user0.xyz;  
  if (arccoords.z != 0) {	
	  vec2 p = arccoords.xy;
		
	  // gradients  
	  vec2 px = dFdx(p);  
	  vec2 py = dFdy(p);

	  // chain rule  
	  float fx = (2 * p.x) * px.x - px.y;  
	  float fy = (2 * p.x) * py.x - py.y;

	  // signed distance  
	  float sd = (p.x * p.x - p.y) / sqrt(fx * fx + fy * fy);  
	  if (arccoords.z > 1)
		  sd = -sd;

	  // linear alpha  
	  color.a *= clamp(0.5 - sd, 0, 1);
  }
  color *= color.a; // Premultiplied Alpha
  return color;
}
