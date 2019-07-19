// shadertype=glsl
#include "common.glsl"
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_Normal;

layout(location = 0) out vec3 thefrag_Normal;

void main()
{
	// output the fragment position in world space
	vec4 WS = vec4(in_Position, 1.0);
	vec4 VS = uni_r_camera * uni_t_camera * WS;

	thefrag_Normal = in_Normal;

	gl_Position = uni_p_camera_original * VS;
}