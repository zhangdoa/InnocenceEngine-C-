// shadertype=glsl
#version 450
layout(location = 0) in vec4 finalColor;
layout(location = 0) out vec4 uni_debuggerPassRT0;

void main()
{
	uni_debuggerPassRT0 = vec4(0.5, 0.2, 0.1, 1.0);
}
