#version 450

layout(local_size_x = 64) in;

layout(binding = 0, std430) readonly buffer b_positions { vec4 positions[]; };
layout(binding = 1, std430) readonly buffer b_texcoords { vec2 texcoords[]; };
layout(binding = 2, std430) readonly buffer b_indices { uint indices[]; };

struct Hit {
  uint version;
  float x;
  float y;
  float z;
};

layout(binding = 3, std430) buffer b_hits {
  uint next_hit_index;
  Hit hits[];
};

uniform bool u_indexed;
uniform uint u_version;
uniform vec3 u_ray_origin;
uniform vec3 u_ray_direction;

// source: https://stackoverflow.com/questions/54564286/triangle-intersection-test-in-opengl-es-glsl
bool IntersectTriangle(vec3 rayOrigin, vec3 rayDirection, vec3 p0, vec3 p1, vec3 p2,
						out float hit, out vec3 barycentricCoord, out vec3 triangleNormal)
{
	const vec3 e0 = p1 - p0;
	const vec3 e1 = p0 - p2;
	triangleNormal = cross( e1, e0 );

	const vec3 e2 = ( 1.0 / dot( triangleNormal, rayDirection ) ) * ( p0 - rayOrigin );
	const vec3 i  = cross( rayDirection, e2 );

	barycentricCoord.y = dot( i, e1 );
	barycentricCoord.z = dot( i, e0 );
	barycentricCoord.x = 1.0 - (barycentricCoord.z + barycentricCoord.y);
	hit   = dot( triangleNormal, e2 );

  const float INTERSECT_EPSILON = 1e-8;
	return (hit > INTERSECT_EPSILON) && all(greaterThanEqual(barycentricCoord, vec3(0.0)));
}

void main() {
  uint gid = gl_GlobalInvocationID.x;    
  uint v0 = gid * 3 + 0;
  uint v1 = gid * 3 + 1;
  uint v2 = gid * 3 + 2;
  if (u_indexed) {
    if (v2 > indices.length())
      return;
    v0 = indices[v0];
    v1 = indices[v1];
    v2 = indices[v2];
  }
  else {
    if (v2 > positions.length())
      return;
  }
    
  float z;
  vec3 barycentric;
  vec3 normal;
  if (IntersectTriangle(
        u_ray_origin,
        u_ray_direction,
        positions[v0].xyz,
        positions[v1].xyz,
        positions[v2].xyz,
        z, barycentric, normal)) {
    vec2 texcoords =
      texcoords[v0] * barycentric.x +
      texcoords[v1] * barycentric.y +
      texcoords[v2] * barycentric.z;
      
    uint index = atomicAdd(next_hit_index, 1) % hits.length();
    
    hits[index] = Hit(
      u_version,
      texcoords.x,
      texcoords.y,
      z);
  }
}
