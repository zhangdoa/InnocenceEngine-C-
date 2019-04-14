// shadertype=<glsl>
#version 450
layout(location = 0) out vec4 uni_motionBlurPassRT0;
layout(location = 0) in vec2 thefrag_TexCoord;

layout(location = 0, binding = 0) uniform sampler2D uni_motionVectorTexture;
layout(location = 1, binding = 1) uniform sampler2D uni_TAAPassRT0;

const int MAX_SAMPLES = 16;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(uni_TAAPassRT0, 0));
	vec2 screenTexCoords = gl_FragCoord.xy * texelSize;
	vec2 MotionVector = texture(uni_motionVectorTexture, screenTexCoords).xy;

	if (MotionVector == vec2(0.0, 0.0))
	{
		uni_motionBlurPassRT0 = texture(uni_TAAPassRT0, screenTexCoords);
	}
	else
	{
		vec4 result = texture(uni_TAAPassRT0, screenTexCoords);

		for (int i = 0; i < MAX_SAMPLES; i++) {
			vec2 offset = MotionVector * (float(i) / float(MAX_SAMPLES - 1) - 0.5);
			result += texture(uni_TAAPassRT0, screenTexCoords + offset);
		}

		result /= float(MAX_SAMPLES);

		//use alpha channel as mask
		uni_motionBlurPassRT0 = vec4(result.rgb, 1.0);
	}
}