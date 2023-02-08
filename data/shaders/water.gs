#version 330 compatibility
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec2 uvs[];
out vec2 uv;

uniform mat4 projection;
uniform mat4 view;
uniform float time;

varying vec3 frag_normal;
varying vec3 frag_tangent;
varying vec3 frag_bitangent;
varying vec3 frag_position;

#define PI 3.14159265359f

// wave = (direction.x, direction.y, wave_length, steepness)
vec3 apply_wave(vec3 pos, vec4 wave, float time)
{
	vec2 direction = wave.xy;
	float wave_length = wave.z;
	float steepness = wave.w;

	float k = 2 * PI / wave_length;
	float c = sqrt(9.8 / k);
	vec2 d = normalize(direction);
	float f = k * (dot(d, pos.xz) - c * time);
	float a = steepness / k;

	return vec3(d.x * (a * cos(f)),
				a * sin(f),
				d.y * (a * cos(f)));
}

void main()
{	
	vec3 p1 = gl_in[0].gl_Position.xyz;
	vec3 p2 = gl_in[1].gl_Position.xyz;
	vec3 p3 = gl_in[2].gl_Position.xyz;
	
	vec2 uv1 = uvs[0];
	vec2 uv2 = uvs[1];
	vec2 uv3 = uvs[2];

	vec3 delta_pos1 = p2 - p1;
	vec3 delta_pos2 = p3 - p1;

	vec2 delta_uv1 = uv2 - uv1;
	vec2 delta_uv2 = uv3 - uv1;

	float r = 1.0 / (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);
	frag_tangent = normalize((delta_pos1 * delta_uv2.y - delta_pos2 * delta_uv1.y) * r);
    frag_bitangent = normalize((delta_pos2 * delta_uv1.x - delta_pos1 * delta_uv2.x) * r); 
	//frag_normal = normalize(cross(delta_pos2, delta_pos1) * r);
	frag_normal = normalize(cross(p2 - p1, p3 - p1));

	for(int i = 0; i < 3; i++)
	{
		//frag_normal = normals[i];
		//frag_position = gl_in[i].gl_Position.xyz;
		vec3 pos = gl_in[i].gl_Position.xyz;
		vec3 const_pos = pos;
		//pos += apply_wave(const_pos, vec4(1.0, 0.0, 2.0, 0.5), time / 5.0);
		//pos += apply_wave(const_pos, vec4(0.5, 0.5, 1.0, 0.3), time / 5.0);
		uv = uvs[i];
		frag_position = pos;
		gl_Position = projection * view * vec4(pos, 1.0f);
			
		EmitVertex();
	}
	EndPrimitive();
}  