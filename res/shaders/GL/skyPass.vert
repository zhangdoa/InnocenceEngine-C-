// shadertype=glsl
#version 450
layout(location = 0) in vec3 in_Position;

layout(location = 0) out vec3 TexCoords;
layout(location = 1) out mat4 inv_p;
layout(location = 5) out mat4 inv_v;

layout(location = 0) uniform mat4 uni_p;
layout(location = 1) uniform mat4 uni_r;

void main()
{
	TexCoords = in_Position * -1.0;
	vec4 pos = uni_p * uni_r * -1.0 * vec4(in_Position, 1.0);
	inv_p = inverse(uni_p);
	inv_v = inverse(uni_r);
	gl_Position = pos.xyww;
}