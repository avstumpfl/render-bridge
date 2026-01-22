//@implements: sampler2D
struct WarpBlendColorCorrection {
	sampler2D sampler;
	sampler3D lookup_table_texture;
	sampler2D parameters_texture;
	mat4 matrix;
	mat4 parameters2;
};

#define PI 3.1415926538
#define GLOBAL_SETTINGS_CT 2

float get_y_rot(inout mat4 m) {
	float cos_y_angle = sqrt(pow(m[0].x, 2.0) + pow(m[0].y, 2.0));
	return - atan(-m[0].z, cos_y_angle) * 180.0 / PI;
}

float get_modifier(int sub_lut_index,float y_rot_current,inout sampler2D poses) {
	vec3 sub_lut_settings = texelFetch(poses, ivec2(GLOBAL_SETTINGS_CT + sub_lut_index, 0), 0).rgb;
	int pose_offset = int(sub_lut_settings.x);
	int pose_ct = int(sub_lut_settings.y);
	if(pose_ct <= 0) {
		return 0.0;
	}
	
	vec2 y_rot_mod;
	
	if(pose_ct >= 2) {
		vec2 pre_y_rot_mod = texelFetch(poses, ivec2(pose_offset, 0), 0).xy;
		for(int i = 1; i < pose_ct; ++i) {
			y_rot_mod = texelFetch(poses, ivec2(i + pose_offset, 0), 0).xy;
			if(y_rot_current >= pre_y_rot_mod.x && y_rot_current < y_rot_mod.x) {
				float delta = y_rot_mod.x - pre_y_rot_mod.x;
				if(delta <= 0.0) {
					return 0.0;
				}
				return pre_y_rot_mod.y + (y_rot_current - pre_y_rot_mod.x)/delta * (y_rot_mod.y - pre_y_rot_mod.y);
			}
			pre_y_rot_mod = y_rot_mod;
		}
	}

	y_rot_mod = texelFetch(poses, ivec2(pose_offset, 0), 0).xy;
	if(y_rot_current < y_rot_mod.x) {
		return y_rot_mod.y;
	}

	y_rot_mod = texelFetch(poses, ivec2(pose_offset + pose_ct - 1, 0), 0).xy;
	if(y_rot_current >= y_rot_mod.x) {
		return y_rot_mod.y;
	}
	
	return 0.0;
}

vec4 get_corrected_color(inout sampler3D lookup_table_texture,inout sampler2D parameters_texture,inout mat4 matrix,inout mat4 parameters2,vec4 color) {
	vec3 settings = texelFetch(parameters_texture, ivec2(0, 0), 0).rgb;
	int sub_lut_index = 0;
	float ct_sub_luts = settings.y;
	vec3 dim_sizes = vec3(settings.x);
	vec3 col_scale = vec3(1) - vec3(1)/dim_sizes;
	vec3 col_off = vec3(0.5)/dim_sizes;
	if(ct_sub_luts > 1) {
		sub_lut_index = int(parameters2[0].x);
		if(sub_lut_index < 0 || sub_lut_index >= ct_sub_luts) {
			sub_lut_index = 0;
		}
		col_scale.b = col_scale.b/ct_sub_luts;
		col_off.b = (sub_lut_index + col_off.b)/ct_sub_luts;
	}
	
	float col_scale_unit = texelFetch(parameters_texture, ivec2(1, 0), 0).r;
	color.rgb = texture(lookup_table_texture, (color.rgb * col_scale_unit) * col_scale + col_off).rgb;

	float modifier = 0.0;
	if(settings.z == 1) {
		modifier = get_modifier(sub_lut_index, get_y_rot(matrix), parameters_texture);
	}
	else if(settings.z == 2) {
		modifier = texelFetch(parameters_texture, ivec2(GLOBAL_SETTINGS_CT, 0), 0).rgb.r;
	}
	
	color.rgb *= 0.5 + 0.5 * pow(1.0 + modifier/20.0, 2.0);
	return color;
}

vec4 texture(WarpBlendColorCorrection s, vec2 tex_coords) {
	return get_corrected_color(s.lookup_table_texture, s.parameters_texture, s.matrix, s.parameters2,
		texture(s.sampler, tex_coords));
}
	