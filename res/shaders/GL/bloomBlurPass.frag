// shadertype=<glsl>
#version 450
layout(location = 0) out vec4 uni_bloomPassRT0;
layout(location = 0) in vec2 thefrag_TexCoord;

layout(location = 0) uniform bool uni_horizontal;
layout(location = 1, binding = 0) uniform sampler2D uni_bloomExtractPassRT0;

const float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

vec4 GaussianBlur(sampler2D image)
{
	vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
	vec3 result = texture(image, thefrag_TexCoord).rgb * weight[0]; // current fragment's contribution
	if (uni_horizontal)
	{
		for (int i = 1; i < 5; ++i)
		{
			result += texture(image, thefrag_TexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			result += texture(image, thefrag_TexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		}
	}
	else
	{
		for (int i = 1; i < 5; ++i)
		{
			result += texture(image, thefrag_TexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			result += texture(image, thefrag_TexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		}
	}
	return vec4(result, 0.0);
}

void main()
{
	uni_bloomPassRT0 = GaussianBlur(uni_bloomExtractPassRT0);
}