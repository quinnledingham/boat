#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec2 uvs[];
out vec2 uv;

out vec4 frag_normal;
out vec3 frag_tangent;
out vec3 frag_bitangent;
out vec4 frag_position;
out vec2 uvs;

vec3 get_normal(vec3 p1, vec3 p2, vec3 p3)
{
	vec3 u = p2 - p1;
	vec3 v = p3 - p1;

	vec3 normal;
	normal.x = (u.y * v.z) - (u.z * v.y);
	normal.y = (u.z * v.x) - (u.x - v.z);
	normal.z = (u.x * v.y) - (u.y - v.x);
	return normal;
}

void main()
{
	vec3 v[3];
	v[0] = gl_in[0].gl_Position.xyz;
	v[1] = gl_in[1].gl_Position.xyz;
	v[2] = gl_in[2].gl_Position.xyz;

	for(int i = 0; i < 3; i++)
	{
		vec3 pos = v[i];
		vec3 const_pos = pos;
		pos += apply_wave(const_pos, vec4(1.0, 0.0, 2.0, 0.5), time / 5.0);
		//v0 += apply_wave(const_pos, vec4(0.5, 0.5, 1.0, 0.3), time / 5.0);
		v[i] = pos;
	}

	frag_normal =  get_normal(v[0], v[2], v[1]);
	frag_position = vec4(v[0], 1.0f);
	gl_Position = projection * view * vec4(v[0], 1.0f);
	EmitVertex();
	frag_position = vec4(v[1], 1.0f);
	gl_Position = projection * view * vec4(v[1], 1.0f);
	EmitVertex();
	frag_position = vec4(v[2], 1.0f);
	gl_Position = projection * view * vec4(v[2], 1.0f);
	EmitVertex();
	//frag_position = vec4(v[2], 1.0f);
	//vec3 delta_pos1 = v1 - v0;
	//vec3 delta_pos2 = v2 - v0;

	//vec2 delta_uv1 = uv1 - uv0;
	//vec2 delta_uv2 = uv2 - uv0;

	//float r = 1.0 / (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);
	//frag_tangent = normalize((delta_pos1 * delta_uv2.y - delta_pos2 * delta_uv1.y) * r);
    //frag_bitangent = normalize((delta_pos2 * delta_uv1.x - delta_pos1 * delta_uv2.x) * r); 
	//frag_normal = normalize(cross(delta_pos2, delta_pos1) * r);


	EndPrimitive();
}  