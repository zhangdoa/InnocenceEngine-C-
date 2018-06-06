#include "GLRenderingSystem.h"

void GLRenderingSystem::setup()
{
	// 16x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 16);
	// MSAA
	glEnable(GL_MULTISAMPLE);
	// enable seamless cubemap sampling for lower mip levels in the pre-filter map.
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_TEXTURE_2D);
}

void GLRenderingSystem::initialize()
{
	initializeEnvironmentRenderPass();
	initializeShadowRenderPass();
	initializeGeometryRenderPass();
	initializeLightRenderPass();
	initializeFinalRenderPass();
	initializeDefaultGraphicPrimtives();
	initializeGraphicPrimtivesOfComponents();
}

void GLRenderingSystem::initializeEnvironmentRenderPass()
{
	// generate and bind framebuffer
	glGenFramebuffers(1, &EnvironmentRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, EnvironmentRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);

	// generate and bind renderbuffer
	glGenRenderbuffers(1, &EnvironmentRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, EnvironmentRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, EnvironmentRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, 2048, 2048);

	// generate and bind texture
	glGenTextures(1, &EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassTexture.m_TAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassTexture.m_TAO);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
	////
	glGenTextures(1, &EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassTexture.m_TAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassTexture.m_TAO);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	////
	glGenTextures(1, &EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassTexture.m_TAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassTexture.m_TAO);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	////
	glGenTextures(1, &EnvironmentRenderPassSingletonComponent::getInstance().m_BRDFLUTTexture.m_TAO);
	glBindTexture(GL_TEXTURE_2D, EnvironmentRenderPassSingletonComponent::getInstance().m_BRDFLUTTexture.m_TAO);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		g_pLogSystem->printLog("GLFrameBuffer: EnvironmentRenderPass Framebuffer is not completed!");
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader programs and shaders
	EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassProgram.m_program = glCreateProgram();
	initializeShader(
		EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassProgram.m_program,
		EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassVertexShaderID,
		GL_VERTEX_SHADER,
		"GL3.3/environmentCapturePassPBSVertex.sf");
	initializeShader(
		EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassProgram.m_program,
		EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassFragmentShaderID,
		GL_FRAGMENT_SHADER,
		"GL3.3/environmentCapturePassPBSFragment.sf");

	EnvironmentRenderPassSingletonComponent::getInstance().m_capturePass_uni_equirectangularMap = getUniformLocation(
		EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassProgram.m_program,
		"uni_equirectangularMap");
	updateUniform(
		EnvironmentRenderPassSingletonComponent::getInstance().m_capturePass_uni_equirectangularMap,
		0);
	EnvironmentRenderPassSingletonComponent::getInstance().m_capturePass_uni_p = getUniformLocation(
		EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassProgram.m_program,
		"uni_p");
	EnvironmentRenderPassSingletonComponent::getInstance().m_capturePass_uni_r = getUniformLocation(
		EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassProgram.m_program,
		"uni_r");

	////
	EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassProgram.m_program = glCreateProgram();
	initializeShader(
		EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassProgram.m_program,
		EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassVertexShaderID,
		GL_VERTEX_SHADER,
		"GL3.3/environmentConvolutionPassPBSVertex.sf");
	initializeShader(
		EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassProgram.m_program,
		EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassFragmentShaderID,
		GL_FRAGMENT_SHADER,
		"GL3.3/environmentConvolutionPassPBSFragment.sf");

	EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPass_uni_p = getUniformLocation(
		EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassProgram.m_program,
		"uni_p");
	EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPass_uni_r = getUniformLocation(
		EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassProgram.m_program,
		"uni_r");

	////
	EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassProgram.m_program = glCreateProgram();
	initializeShader(
		EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassProgram.m_program,
		EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassVertexShaderID,
		GL_VERTEX_SHADER,
		"GL3.3/environmentPreFilterPassPBSVertex.sf");
	initializeShader(
		EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassProgram.m_program,
		EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassFragmentShaderID,
		GL_FRAGMENT_SHADER,
		"GL3.3/environmentPreFilterPassPBSFragment.sf");

	EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPass_uni_roughness = getUniformLocation(
		EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassProgram.m_program,
		"uni_roughness");
	EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPass_uni_p = getUniformLocation(
		EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassProgram.m_program,
		"uni_p");
	EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPass_uni_r = getUniformLocation(
		EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassProgram.m_program,
		"uni_r");

	////
	EnvironmentRenderPassSingletonComponent::getInstance().m_BRDFLUTPassProgram.m_program = glCreateProgram();
	initializeShader(
		EnvironmentRenderPassSingletonComponent::getInstance().m_BRDFLUTPassProgram.m_program,
		EnvironmentRenderPassSingletonComponent::getInstance().m_BRDFLUTPassVertexShaderID,
		GL_VERTEX_SHADER,
		"GL3.3/environmentBRDFLUTPassPBSVertex.sf");
	initializeShader(
		EnvironmentRenderPassSingletonComponent::getInstance().m_BRDFLUTPassProgram.m_program,
		EnvironmentRenderPassSingletonComponent::getInstance().m_BRDFLUTPassFragmentShaderID,
		GL_FRAGMENT_SHADER,
		"GL3.3/environmentBRDFLUTPassPBSFragment.sf");
}

void GLRenderingSystem::initializeShadowRenderPass()
{
	// generate and bind framebuffer
	glGenFramebuffers(1, &ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);

	// generate and bind renderbuffer
	glGenRenderbuffers(1, &ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, 2048, 2048);

	// generate and bind texture
	glGenTextures(1, &ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L0.m_TAO);
	glBindTexture(GL_TEXTURE_2D, ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L0.m_TAO);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_ATTACHMENT, 2048, 2048, 0, GL_DEPTH_ATTACHMENT, GL_FLOAT, nullptr);

	glGenTextures(1, &ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L1.m_TAO);
	glBindTexture(GL_TEXTURE_2D, ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L1.m_TAO);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_ATTACHMENT, 2048, 2048, 0, GL_DEPTH_ATTACHMENT, GL_FLOAT, nullptr);

	glGenTextures(1, &ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L2.m_TAO);
	glBindTexture(GL_TEXTURE_2D, ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L2.m_TAO);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_ATTACHMENT, 2048, 2048, 0, GL_DEPTH_ATTACHMENT, GL_FLOAT, nullptr);

	glGenTextures(1, &ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L3.m_TAO);
	glBindTexture(GL_TEXTURE_2D, ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L3.m_TAO);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_ATTACHMENT, 2048, 2048, 0, GL_DEPTH_ATTACHMENT, GL_FLOAT, nullptr);

	// @TODO: clarify the color attachment index
	attachTextureToFramebuffer(
		&ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L0,
		&ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent,
		0, 0, 0
	);
	attachTextureToFramebuffer(
		&ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L1,
		&ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent,
		0, 0, 0
	);
	attachTextureToFramebuffer(
		&ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L2,
		&ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent,
		0, 0, 0
	);
	attachTextureToFramebuffer(
		&ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L3,
		&ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent,
		0, 0, 0
	);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		g_pLogSystem->printLog("GLFrameBuffer: ShadowRenderPass Framebuffer is not completed!");
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader programs and shaders
	ShadowRenderPassSingletonComponent::getInstance().m_shadowPassProgram.m_program = glCreateProgram();
	initializeShader(
		ShadowRenderPassSingletonComponent::getInstance().m_shadowPassProgram.m_program,
		ShadowRenderPassSingletonComponent::getInstance().m_shadowPassVertexShaderID,
		GL_VERTEX_SHADER,
		"GL3.3/shadowPassVertex.sf");
	initializeShader(
		ShadowRenderPassSingletonComponent::getInstance().m_shadowPassProgram.m_program,
		ShadowRenderPassSingletonComponent::getInstance().m_shadowPassFragmentShaderID,
		GL_FRAGMENT_SHADER,
		"GL3.3/shadowPassFragment.sf");

	ShadowRenderPassSingletonComponent::getInstance().m_shadowPass_uni_p = getUniformLocation(
		ShadowRenderPassSingletonComponent::getInstance().m_shadowPassProgram.m_program,
		"uni_p");
	ShadowRenderPassSingletonComponent::getInstance().m_shadowPass_uni_v = getUniformLocation(
		ShadowRenderPassSingletonComponent::getInstance().m_shadowPassProgram.m_program,
		"uni_v");
	ShadowRenderPassSingletonComponent::getInstance().m_shadowPass_uni_m = getUniformLocation(
		ShadowRenderPassSingletonComponent::getInstance().m_shadowPassProgram.m_program,
		"uni_m");
}

void GLRenderingSystem::initializeGeometryRenderPass()
{
	// generate and bind framebuffer
	glGenFramebuffers(1, &GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);

	// generate and bind renderbuffer
	glGenRenderbuffers(1, &GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);

	// generate and bind texture
	glGenTextures(1, &GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT0.m_TAO);
	glBindTexture(GL_TEXTURE_2D, GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT0.m_TAO);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);

	glGenTextures(1, &GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT1.m_TAO);
	glBindTexture(GL_TEXTURE_2D, GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT1.m_TAO);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);

	glGenTextures(1, &GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT2.m_TAO);
	glBindTexture(GL_TEXTURE_2D, GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT2.m_TAO);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);

	glGenTextures(1, &GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT3.m_TAO);
	glBindTexture(GL_TEXTURE_2D, GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT3.m_TAO);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);

	attachTextureToFramebuffer(
		&GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT0,
		&GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent,
		0, 0, 0
	);
	attachTextureToFramebuffer(
		&GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT1,
		&GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent,
		1, 0, 0
	);
	attachTextureToFramebuffer(
		&GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT2,
		&GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent,
		2, 0, 0
	);
	attachTextureToFramebuffer(
		&GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT3,
		&GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent,
		3, 0, 0
	);

	std::vector<unsigned int> l_colorAttachments;
	for (auto i = (unsigned int)0; i < 4; ++i)
	{
		l_colorAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + i);
	}
	glDrawBuffers(l_colorAttachments.size(), &l_colorAttachments[0]);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		g_pLogSystem->printLog("GLFrameBuffer: ShadowRenderPass Framebuffer is not completed!");
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader programs and shaders
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program = glCreateProgram();
#ifdef CookTorrance
	initializeShader(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassVertexShaderID,
		GL_VERTEX_SHADER,
		"GL3.3/geometryCookTorrancePassVertex.sf");
	initializeShader(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassFragmentShaderID,
		GL_FRAGMENT_SHADER,
		"GL3.3/geometryCookTorrancePassFragment.sf");
#elif BlinnPhong
	initializeShader(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassVertexShaderID,
		GL_VERTEX_SHADER,
		"GL3.3/geometryBlinnPhongPassVertex.sf");
	initializeShader(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassFragmentShaderID,
		GL_FRAGMENT_SHADER,
		"GL3.3/geometryBlinnPhongPassFragment.sf");
#endif
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_p = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_p");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_r = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_r");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_t = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_t");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_m = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_m");
#ifdef CookTorrance
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_p_light = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_p_light");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_v_light = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_v_light");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_normalTexture = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_normalTexture");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_albedoTexture = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_albedoTexture");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_metallicTexture = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_metallicTexture");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_roughnessTexture = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_roughnessTexture");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_aoTexture = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_aoTexture");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_useTexture = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_useTexture");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_albedo = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_albedo");
	GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_MRA = getUniformLocation(
		GeometryRenderPassSingletonComponent::getInstance().m_geometryPassProgram.m_program,
		"uni_MRA");
#elif BlinnPhong
	// @TODO: texture uniforms 
#endif
}

void GLRenderingSystem::initializeLightRenderPass()
{
	// generate and bind framebuffer
	glGenFramebuffers(1, &LightRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, LightRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);

	// generate and bind renderbuffer
	glGenRenderbuffers(1, &LightRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, LightRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, LightRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);

	// generate and bind texture
	glGenTextures(1, &LightRenderPassSingletonComponent::getInstance().m_lightPassTexture.m_TAO);
	glBindTexture(GL_TEXTURE_2D, LightRenderPassSingletonComponent::getInstance().m_lightPassTexture.m_TAO);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);

	attachTextureToFramebuffer(
		&LightRenderPassSingletonComponent::getInstance().m_lightPassTexture,
		&LightRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent,
		0, 0, 0
	);

	std::vector<unsigned int> l_colorAttachments;
	l_colorAttachments.emplace_back(GL_COLOR_ATTACHMENT0);
	glDrawBuffers(1, &l_colorAttachments[0]);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		g_pLogSystem->printLog("GLFrameBuffer: ShadowRenderPass Framebuffer is not completed!");
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader programs and shaders
#ifdef CookTorrance
	initializeShader(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		LightRenderPassSingletonComponent::getInstance().m_lightPassVertexShaderID,
		GL_VERTEX_SHADER,
		"GL3.3/lightCookTorrancePassVertex.sf");
	initializeShader(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		LightRenderPassSingletonComponent::getInstance().m_lightPassFragmentShaderID,
		GL_FRAGMENT_SHADER,
		"GL3.3/lightCookTorrancePassFragment.sf");
#elif BlinnPhong
	initializeShader(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		LightRenderPassSingletonComponent::getInstance().m_lightPassVertexShaderID,
		GL_VERTEX_SHADER,
		"GL3.3/lightBlinnPhongPassVertex.sf");
	initializeShader(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		LightRenderPassSingletonComponent::getInstance().m_lightPassFragmentShaderID,
		GL_FRAGMENT_SHADER,
		"GL3.3/lightBlinnPhongPassFragment.sf");
#endif
	LightRenderPassSingletonComponent::getInstance().m_uni_geometryPassRT0 = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_geometryPassRT0");
	LightRenderPassSingletonComponent::getInstance().m_uni_geometryPassRT1 = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_geometryPassRT1");
	LightRenderPassSingletonComponent::getInstance().m_uni_geometryPassRT2 = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_geometryPassRT2");
	LightRenderPassSingletonComponent::getInstance().m_uni_geometryPassRT3 = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_geometryPassRT3");
	LightRenderPassSingletonComponent::getInstance().m_uni_shadowMap = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_shadowMap");
	LightRenderPassSingletonComponent::getInstance().m_uni_irradianceMap = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_irradianceMap");
	LightRenderPassSingletonComponent::getInstance().m_uni_preFiltedMap = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_preFiltedMap");
	LightRenderPassSingletonComponent::getInstance().m_uni_brdfLUT = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_brdfLUT");
	LightRenderPassSingletonComponent::getInstance().m_uni_viewPos = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_viewPos");
	LightRenderPassSingletonComponent::getInstance().m_uni_dirLight_position = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_dirLight.position");
	LightRenderPassSingletonComponent::getInstance().m_uni_dirLight_direction = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_dirLight.direction");
	LightRenderPassSingletonComponent::getInstance().m_uni_dirLight_color = getUniformLocation(
		LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program,
		"uni_dirLight.color");
	int l_pointLightIndexOffset = 0;
	for (auto i = (unsigned int)0; i < g_pGameSystem->getLightComponents().size(); i++)
	{
		if (g_pGameSystem->getLightComponents()[i]->m_lightType == lightType::DIRECTIONAL)
		{
			l_pointLightIndexOffset -= 1;
		}
		if (g_pGameSystem->getLightComponents()[i]->m_lightType == lightType::POINT)
		{
			std::stringstream ss;
			ss << i + l_pointLightIndexOffset;
			LightRenderPassSingletonComponent::getInstance().m_uni_pointLights_position.emplace_back(
				getUniformLocation(LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program, "uni_pointLights[" + ss.str() + "].position")
			);
			LightRenderPassSingletonComponent::getInstance().m_uni_pointLights_radius.emplace_back(
				getUniformLocation(LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program, "uni_pointLights[" + ss.str() + "].radius")
			);
			LightRenderPassSingletonComponent::getInstance().m_uni_pointLights_color.emplace_back(
				getUniformLocation(LightRenderPassSingletonComponent::getInstance().m_lightPassProgram.m_program, "uni_pointLights[" + ss.str() + "].color")
			);
		}
	}
}

void GLRenderingSystem::initializeFinalRenderPass()
{
}

void GLRenderingSystem::initializeShader(GLuint& shaderProgram, GLuint& shaderID, GLuint shaderType, const std::string & shaderFilePath)
{
	shaderID = glCreateShader(shaderType);

	if (shaderID == 0) {
		g_pLogSystem->printLog("Error: Shader creation failed: memory location invaild when adding shader!");
	}

	auto l_shaderCodeContent = g_pAssetSystem->loadShader(shaderFilePath);
	const char* l_sourcePointer = l_shaderCodeContent.c_str();

	glShaderSource(shaderID, 1, &l_sourcePointer, NULL);

	GLint l_compileResult = GL_FALSE;
	int l_infoLogLength = 0;
	int l_shaderFileLength = 0;
	glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &l_compileResult);
	glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &l_infoLogLength);
	glGetShaderiv(shaderProgram, GL_SHADER_SOURCE_LENGTH, &l_shaderFileLength);

	if (l_infoLogLength > 0) {
		std::vector<char> l_shaderErrorMessage(l_infoLogLength + 1);
		glGetShaderInfoLog(shaderProgram, l_infoLogLength, NULL, &l_shaderErrorMessage[0]);
		g_pLogSystem->printLog("innoShader: " + shaderFilePath + " compile error: " + &l_shaderErrorMessage[0] + "\n -- --------------------------------------------------- -- ");
	}

	glAttachShader(shaderProgram, shaderID);
	glLinkProgram(shaderProgram);
	glValidateProgram(shaderProgram);

	g_pLogSystem->printLog("innoShader: compiling " + shaderFilePath + " ...");

	GLint success;
	GLchar infoLog[1024];
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderID, 1024, NULL, infoLog);
		g_pLogSystem->printLog("innoShader: compile error: " + std::string(infoLog) + "\n -- --------------------------------------------------- -- ");
	}

	g_pLogSystem->printLog("innoShader: " + shaderFilePath + " Shader is compiled.");
}

void GLRenderingSystem::initializeDefaultGraphicPrimtives()
{
	initializeMesh(g_pAssetSystem->getDefaultMesh(meshShapeType::LINE));
	initializeMesh(g_pAssetSystem->getDefaultMesh(meshShapeType::QUAD));
	initializeMesh(g_pAssetSystem->getDefaultMesh(meshShapeType::CUBE));
	initializeMesh(g_pAssetSystem->getDefaultMesh(meshShapeType::SPHERE));

	initializeTexture(g_pAssetSystem->getDefaultTexture(textureType::NORMAL));
	initializeTexture(g_pAssetSystem->getDefaultTexture(textureType::ALBEDO));
	initializeTexture(g_pAssetSystem->getDefaultTexture(textureType::METALLIC));
	initializeTexture(g_pAssetSystem->getDefaultTexture(textureType::ROUGHNESS));
	initializeTexture(g_pAssetSystem->getDefaultTexture(textureType::AMBIENT_OCCLUSION));
}

void GLRenderingSystem::initializeGraphicPrimtivesOfComponents()
{
	for (auto& l_visibleComponent : g_pGameSystem->getVisibleComponents())
	{
		for (auto& l_graphicData : l_visibleComponent->getModelMap())
		{
			if (GLRenderingSystemSingletonComponent::getInstance().m_initializedMeshMap.find(l_graphicData.first) == GLRenderingSystemSingletonComponent::getInstance().m_initializedMeshMap.end())
			{
				auto l_Mesh = g_pAssetSystem->getMesh(l_graphicData.first);
				initializeMesh(l_Mesh);
				std::for_each(l_graphicData.second.begin(), l_graphicData.second.end(), [&](texturePair val) {
					auto l_Texture = g_pAssetSystem->getTexture(val.second);
					initializeTexture(l_Texture);
					GLRenderingSystemSingletonComponent::getInstance().m_initializedTextureMap.emplace(val.second, l_Texture);
				});
				GLRenderingSystemSingletonComponent::getInstance().m_initializedMeshMap.emplace(l_graphicData.first, l_Mesh);
			}
		}
	}
}

void GLRenderingSystem::initializeMesh(MeshDataComponent* GLMeshDataComponent)
{
	glGenVertexArrays(1, &GLMeshDataComponent->m_VAO);
	glGenBuffers(1, &GLMeshDataComponent->m_VBO);
	glGenBuffers(1, &GLMeshDataComponent->m_IBO);

	std::vector<float> l_verticesBuffer;
	auto& l_vertices = GLMeshDataComponent->m_vertices;
	auto& l_indices = GLMeshDataComponent->m_indices;

	std::for_each(l_vertices.begin(), l_vertices.end(), [&](Vertex val)
	{
		l_verticesBuffer.emplace_back((float)val.m_pos.x);
		l_verticesBuffer.emplace_back((float)val.m_pos.y);
		l_verticesBuffer.emplace_back((float)val.m_pos.z);
		l_verticesBuffer.emplace_back((float)val.m_texCoord.x);
		l_verticesBuffer.emplace_back((float)val.m_texCoord.y);
		l_verticesBuffer.emplace_back((float)val.m_normal.x);
		l_verticesBuffer.emplace_back((float)val.m_normal.y);
		l_verticesBuffer.emplace_back((float)val.m_normal.z);
	});

	glBindVertexArray(GLMeshDataComponent->m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, GLMeshDataComponent->m_VBO);
	glBufferData(GL_ARRAY_BUFFER, l_verticesBuffer.size() * sizeof(float), &l_verticesBuffer[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLMeshDataComponent->m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, l_indices.size() * sizeof(unsigned int), &l_indices[0], GL_STATIC_DRAW);

	// position attribute, 1st attribution with 3 * sizeof(float) bits of data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

	// texture attribute, 2nd attribution with 2 * sizeof(float) bits of data
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	// normal coord attribute, 3rd attribution with 3 * sizeof(float) bits of data
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
}

void GLRenderingSystem::initializeTexture(TextureDataComponent * GLTextureDataComponent)
{
	if (GLTextureDataComponent->m_textureType == textureType::INVISIBLE)
	{
		return;
	}
	else
	{
		//generate and bind texture object
		glGenTextures(1, &GLTextureDataComponent->m_TAO);
		if (GLTextureDataComponent->m_textureType == textureType::CUBEMAP || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CAPTURE || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CONVOLUTION || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_PREFILTER)
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, GLTextureDataComponent->m_TAO);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, GLTextureDataComponent->m_TAO);
		}

		// set the texture wrapping parameters
		GLenum l_textureWrapMethod;
		switch (GLTextureDataComponent->m_textureWrapMethod)
		{
		case textureWrapMethod::CLAMP_TO_EDGE: l_textureWrapMethod = GL_CLAMP_TO_EDGE; break;
		case textureWrapMethod::REPEAT: l_textureWrapMethod = GL_REPEAT; break;
		case textureWrapMethod::CLAMP_TO_BORDER: l_textureWrapMethod = GL_CLAMP_TO_BORDER; break;
		}
		if (GLTextureDataComponent->m_textureType == textureType::CUBEMAP || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CAPTURE || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CONVOLUTION || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_PREFILTER)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, l_textureWrapMethod);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, l_textureWrapMethod);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, l_textureWrapMethod);
		}
		else if (GLTextureDataComponent->m_textureType == textureType::SHADOWMAP)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, l_textureWrapMethod);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, l_textureWrapMethod);
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, l_textureWrapMethod);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, l_textureWrapMethod);
		}

		// set texture filtering parameters
		GLenum l_minFilterParam;
		switch (GLTextureDataComponent->m_textureMinFilterMethod)
		{
		case textureFilterMethod::NEAREST: l_minFilterParam = GL_NEAREST; break;
		case textureFilterMethod::LINEAR: l_minFilterParam = GL_LINEAR; break;
		case textureFilterMethod::LINEAR_MIPMAP_LINEAR: l_minFilterParam = GL_LINEAR_MIPMAP_LINEAR; break;

		}
		GLenum l_magFilterParam;
		switch (GLTextureDataComponent->m_textureMagFilterMethod)
		{
		case textureFilterMethod::NEAREST: l_magFilterParam = GL_NEAREST; break;
		case textureFilterMethod::LINEAR: l_magFilterParam = GL_LINEAR; break;
		case textureFilterMethod::LINEAR_MIPMAP_LINEAR: l_magFilterParam = GL_LINEAR_MIPMAP_LINEAR; break;

		}
		if (GLTextureDataComponent->m_textureType == textureType::CUBEMAP || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CAPTURE || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CONVOLUTION || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_PREFILTER)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, l_minFilterParam);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, l_magFilterParam);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, l_minFilterParam);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, l_magFilterParam);
		}

		// set texture formats
		GLenum l_internalFormat;
		GLenum l_dataFormat;
		GLenum l_type;
		if (GLTextureDataComponent->m_textureType == textureType::ALBEDO)
		{
			if (GLTextureDataComponent->m_texturePixelDataFormat == texturePixelDataFormat::RGB)
			{
				l_internalFormat = GL_SRGB;
			}
			else if (GLTextureDataComponent->m_texturePixelDataFormat == texturePixelDataFormat::RGBA)
			{
				l_internalFormat = GL_SRGB_ALPHA;
			}
		}
		else
		{
			switch (GLTextureDataComponent->m_textureColorComponentsFormat)
			{
			case textureColorComponentsFormat::RED: l_internalFormat = GL_RED; break;
			case textureColorComponentsFormat::RG: l_internalFormat = GL_RG; break;
			case textureColorComponentsFormat::RGB: l_internalFormat = GL_RGB; break;
			case textureColorComponentsFormat::RGBA: l_internalFormat = GL_RGBA; break;
			case textureColorComponentsFormat::R8: l_internalFormat = GL_R8; break;
			case textureColorComponentsFormat::RG8: l_internalFormat = GL_RG8; break;
			case textureColorComponentsFormat::RGB8: l_internalFormat = GL_RGB8; break;
			case textureColorComponentsFormat::RGBA8: l_internalFormat = GL_RGBA8; break;
			case textureColorComponentsFormat::R16: l_internalFormat = GL_R16; break;
			case textureColorComponentsFormat::RG16: l_internalFormat = GL_RG16; break;
			case textureColorComponentsFormat::RGB16: l_internalFormat = GL_RGB16; break;
			case textureColorComponentsFormat::RGBA16: l_internalFormat = GL_RGBA16; break;
			case textureColorComponentsFormat::R16F: l_internalFormat = GL_R16F; break;
			case textureColorComponentsFormat::RG16F: l_internalFormat = GL_RG16F; break;
			case textureColorComponentsFormat::RGB16F: l_internalFormat = GL_RGB16F; break;
			case textureColorComponentsFormat::RGBA16F: l_internalFormat = GL_RGBA16F; break;
			case textureColorComponentsFormat::R32F: l_internalFormat = GL_R32F; break;
			case textureColorComponentsFormat::RG32F: l_internalFormat = GL_RG32F; break;
			case textureColorComponentsFormat::RGB32F: l_internalFormat = GL_RGB32F; break;
			case textureColorComponentsFormat::RGBA32F: l_internalFormat = GL_RGBA32F; break;
			case textureColorComponentsFormat::SRGB: l_internalFormat = GL_SRGB; break;
			case textureColorComponentsFormat::SRGBA: l_internalFormat = GL_SRGB_ALPHA; break;
			case textureColorComponentsFormat::SRGB8: l_internalFormat = GL_SRGB8; break;
			case textureColorComponentsFormat::SRGBA8: l_internalFormat = GL_SRGB8_ALPHA8; break;
			case textureColorComponentsFormat::DEPTH_COMPONENT: l_internalFormat = GL_DEPTH_COMPONENT; break;
			}
		}
		switch (GLTextureDataComponent->m_texturePixelDataFormat)
		{
		case texturePixelDataFormat::RED:l_dataFormat = GL_RED; break;
		case texturePixelDataFormat::RG:l_dataFormat = GL_RG; break;
		case texturePixelDataFormat::RGB:l_dataFormat = GL_RGB; break;
		case texturePixelDataFormat::RGBA:l_dataFormat = GL_RGBA; break;
		case texturePixelDataFormat::DEPTH_COMPONENT:l_dataFormat = GL_DEPTH_COMPONENT; break;
		}
		switch (GLTextureDataComponent->m_texturePixelDataType)
		{
		case texturePixelDataType::UNSIGNED_BYTE:l_type = GL_UNSIGNED_BYTE; break;
		case texturePixelDataType::BYTE:l_type = GL_BYTE; break;
		case texturePixelDataType::UNSIGNED_SHORT:l_type = GL_UNSIGNED_SHORT; break;
		case texturePixelDataType::SHORT:l_type = GL_SHORT; break;
		case texturePixelDataType::UNSIGNED_INT:l_type = GL_UNSIGNED_INT; break;
		case texturePixelDataType::INT:l_type = GL_INT; break;
		case texturePixelDataType::FLOAT:l_type = GL_FLOAT; break;
		}

		if (GLTextureDataComponent->m_textureType == textureType::CUBEMAP || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CAPTURE || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CONVOLUTION || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_PREFILTER)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, l_internalFormat, GLTextureDataComponent->m_textureWidth, GLTextureDataComponent->m_textureHeight, 0, l_dataFormat, l_type, GLTextureDataComponent->m_textureData[0]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, l_internalFormat, GLTextureDataComponent->m_textureWidth, GLTextureDataComponent->m_textureHeight, 0, l_dataFormat, l_type, GLTextureDataComponent->m_textureData[1]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, l_internalFormat, GLTextureDataComponent->m_textureWidth, GLTextureDataComponent->m_textureHeight, 0, l_dataFormat, l_type, GLTextureDataComponent->m_textureData[2]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, l_internalFormat, GLTextureDataComponent->m_textureWidth, GLTextureDataComponent->m_textureHeight, 0, l_dataFormat, l_type, GLTextureDataComponent->m_textureData[3]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, l_internalFormat, GLTextureDataComponent->m_textureWidth, GLTextureDataComponent->m_textureHeight, 0, l_dataFormat, l_type, GLTextureDataComponent->m_textureData[4]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, l_internalFormat, GLTextureDataComponent->m_textureWidth, GLTextureDataComponent->m_textureHeight, 0, l_dataFormat, l_type, GLTextureDataComponent->m_textureData[5]);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, l_internalFormat, GLTextureDataComponent->m_textureWidth, GLTextureDataComponent->m_textureHeight, 0, l_dataFormat, l_type, GLTextureDataComponent->m_textureData[0]);
		}

		// should generate mipmap or not
		if (GLTextureDataComponent->m_textureMinFilterMethod == textureFilterMethod::LINEAR_MIPMAP_LINEAR)
		{
			// @TODO: generalization...
			if (GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_PREFILTER)
			{
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			}
			else if (GLTextureDataComponent->m_textureType != textureType::CUBEMAP || GLTextureDataComponent->m_textureType != textureType::ENVIRONMENT_CAPTURE || GLTextureDataComponent->m_textureType != textureType::ENVIRONMENT_CONVOLUTION || GLTextureDataComponent->m_textureType != textureType::RENDER_BUFFER_SAMPLER)
			{
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
	}
}

void GLRenderingSystem::update()
{
	if (GLRenderingSystemSingletonComponent::getInstance().m_shouldUpdateEnvironmentMap)
	{
		updateEnvironmentRenderPass();
		GLRenderingSystemSingletonComponent::getInstance().m_shouldUpdateEnvironmentMap = false;
	}
	updateShadowRenderPass();
	updateGeometryRenderPass();
	updateLightRenderPass();
}

void GLRenderingSystem::updateEnvironmentRenderPass()
{
	// bind to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, EnvironmentRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);
	glBindRenderbuffer(GL_RENDERBUFFER, EnvironmentRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	// draw environment capture texture
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, 2048, 2048);
	glViewport(0, 0, 2048, 2048);

	mat4 captureProjection;
	captureProjection.initializeToPerspectiveMatrix((90.0 / 180.0) * PI, 1.0, 0.1, 10.0);
	std::vector<mat4> captureViews =
	{
		mat4().lookAt(vec4(0.0, 0.0, 0.0, 1.0), vec4(1.0,  0.0,  0.0, 1.0), vec4(0.0, -1.0,  0.0, 0.0)),
		mat4().lookAt(vec4(0.0, 0.0, 0.0, 1.0), vec4(-1.0,  0.0,  0.0, 1.0), vec4(0.0, -1.0,  0.0, 0.0)),
		mat4().lookAt(vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0,  1.0,  0.0, 1.0), vec4(0.0,  0.0,  1.0, 0.0)),
		mat4().lookAt(vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, -1.0,  0.0, 1.0), vec4(0.0,  0.0, -1.0, 0.0)),
		mat4().lookAt(vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0,  0.0,  1.0, 1.0), vec4(0.0, -1.0,  0.0, 0.0)),
		mat4().lookAt(vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0,  0.0, -1.0, 1.0), vec4(0.0, -1.0,  0.0, 0.0))
	};

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glUseProgram(EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassProgram.m_program);
	updateUniform(
		EnvironmentRenderPassSingletonComponent::getInstance().m_capturePass_uni_p,
		captureProjection);

	auto& l_visibleComponents = g_pGameSystem->getVisibleComponents();

	if (l_visibleComponents.size() > 0)
	{
		for (auto& l_visibleComponent : l_visibleComponents)
		{
			if (l_visibleComponent->m_visiblilityType == visiblilityType::SKYBOX)
			{
				for (auto& l_graphicData : l_visibleComponent->getModelMap())
				{
					// activate equiretangular texture and remap equiretangular texture to cubemap
					auto l_equiretangularTexture = g_pAssetSystem->getTexture(l_graphicData.second.find(textureType::EQUIRETANGULAR)->second);
					activateTexture(l_equiretangularTexture, 0);
					for (unsigned int i = 0; i < 6; ++i)
					{
						updateUniform(
							EnvironmentRenderPassSingletonComponent::getInstance().m_capturePass_uni_r,
							captureViews[i]);
						glBindTexture(GL_TEXTURE_CUBE_MAP, EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassTexture.m_TAO);
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassTexture.m_TAO, 0);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						auto l_mesh = g_pAssetSystem->getMesh(l_graphicData.first);
						activateMesh(l_mesh);
						drawMesh(l_mesh);
					}
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				}
			}
		}
	}

	// draw environment convolution texture
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, 128, 128);
	glViewport(0, 0, 128, 128);
	glUseProgram(EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassProgram.m_program);
	updateUniform(
		EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPass_uni_p,
		captureProjection);

	if (l_visibleComponents.size() > 0)
	{
		for (auto& l_visibleComponent : l_visibleComponents)
		{
			if (l_visibleComponent->m_visiblilityType == visiblilityType::SKYBOX)
			{
				for (auto& l_graphicData : l_visibleComponent->getModelMap())
				{
					auto l_environmentCaptureTexture = &EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassTexture;
					auto l_environmentConvolutionTexture = &EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPassTexture;
					activateTexture(l_environmentCaptureTexture, 1);
					for (unsigned int i = 0; i < 6; ++i)
					{
						updateUniform(
							EnvironmentRenderPassSingletonComponent::getInstance().m_convolutionPass_uni_r,
							captureViews[i]);
						attachTextureToFramebuffer(l_environmentConvolutionTexture, &EnvironmentRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent, 0, i, 0);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						auto l_mesh = g_pAssetSystem->getMesh(l_graphicData.first);
						activateMesh(l_mesh);
						drawMesh(l_mesh);
					}
				}
			}
		}
	}

	// draw environment pre-filter texture
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, 128, 128);
	glViewport(0, 0, 128, 128);
	glUseProgram(EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassProgram.m_program);
	updateUniform(
		EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPass_uni_p,
		captureProjection);

	if (l_visibleComponents.size() > 0)
	{
		for (auto& l_visibleComponent : l_visibleComponents)
		{
			if (l_visibleComponent->m_visiblilityType == visiblilityType::SKYBOX)
			{
				for (auto& l_graphicData : l_visibleComponent->getModelMap())
				{
					auto l_environmentCaptureTexture = &EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassTexture;
					auto l_environmentPrefilterTexture = &EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassTexture;
					activateTexture(l_environmentPrefilterTexture, 2);
					unsigned int maxMipLevels = 5;
					for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
					{
						// resize framebuffer according to mip-level size.
						unsigned int mipWidth = (int)(128 * std::pow(0.5, mip));
						unsigned int mipHeight = (int)(128 * std::pow(0.5, mip));

						glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, mipWidth, mipHeight);
						glViewport(0, 0, mipWidth, mipHeight);

						double roughness = (double)mip / (double)(maxMipLevels - 1);
						updateUniform(EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPass_uni_roughness, roughness);
						for (unsigned int i = 0; i < 6; ++i)
						{
							updateUniform(EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPass_uni_r, captureViews[i]);
							attachTextureToFramebuffer(l_environmentPrefilterTexture, &EnvironmentRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent, 0, i, mip);

							glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
							auto l_mesh = g_pAssetSystem->getMesh(l_graphicData.first);
							activateMesh(l_mesh);
							drawMesh(l_mesh);
						}
					}
				}
			}
		}
	}

	// draw environment BRDF look-up table texture
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, 512, 512);
	glViewport(0, 0, 512, 512);
	glUseProgram(EnvironmentRenderPassSingletonComponent::getInstance().m_BRDFLUTPassProgram.m_program);

	auto l_environmentBRDFLUTTexture = &EnvironmentRenderPassSingletonComponent::getInstance().m_BRDFLUTTexture;
	attachTextureToFramebuffer(l_environmentBRDFLUTTexture, &EnvironmentRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent, 0, 0, 0);

	// draw environment map BRDF LUT rectangle
	auto l_mesh = g_pAssetSystem->getDefaultMesh(meshShapeType::QUAD);
	activateMesh(l_mesh);
	drawMesh(l_mesh);
}

void GLRenderingSystem::updateShadowRenderPass()
{
	// bind to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);
	glBindRenderbuffer(GL_RENDERBUFFER, ShadowRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// draw each lightComponent's shadowmap
	for (size_t i = 0; i < 4; i++)
	{
		for (auto& l_lightComponent : g_pGameSystem->getLightComponents())
		{
			if (l_lightComponent->m_lightType == lightType::DIRECTIONAL)
			{
				glUseProgram(ShadowRenderPassSingletonComponent::getInstance().m_shadowPassProgram.m_program);
				updateUniform(
					ShadowRenderPassSingletonComponent::getInstance().m_shadowPass_uni_p,
					l_lightComponent->getProjectionMatrix(i));
				updateUniform(
					ShadowRenderPassSingletonComponent::getInstance().m_shadowPass_uni_v,
					g_pGameSystem->getTransformComponent(l_lightComponent->getParentEntity())->m_transform.getInvertGlobalRotMatrix());

				// draw each visibleComponent
				for (auto& l_visibleComponent : g_pGameSystem->getVisibleComponents())
				{
					if (l_visibleComponent->m_visiblilityType == visiblilityType::STATIC_MESH)
					{
						updateUniform(
							ShadowRenderPassSingletonComponent::getInstance().m_shadowPass_uni_m,
							g_pGameSystem->getTransformComponent(l_visibleComponent->getParentEntity())->m_transform.caclGlobalTransformationMatrix());

						// draw each graphic data of visibleComponent
						for (auto& l_graphicData : l_visibleComponent->getModelMap())
						{
							// draw meshes
							auto l_mesh = g_pAssetSystem->getMesh(l_graphicData.first);
							activateMesh(l_mesh);
							drawMesh(l_mesh);
						}
					}
				}
			}
		}
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

void GLRenderingSystem::updateGeometryRenderPass()
{
	// bind to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);
	glBindRenderbuffer(GL_RENDERBUFFER, GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if (g_pGameSystem->getCameraComponents().size() > 0)
	{
		mat4 p = g_pGameSystem->getCameraComponents()[0]->m_projectionMatrix;
		mat4 r = g_pGameSystem->getTransformComponent(g_pGameSystem->getCameraComponents()[0]->getParentEntity())->m_transform.getInvertGlobalRotMatrix();
		mat4 t = g_pGameSystem->getTransformComponent(g_pGameSystem->getCameraComponents()[0]->getParentEntity())->m_transform.getInvertGlobalTranslationMatrix();

		updateUniform(
			GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_p,
			p);
		updateUniform(
			GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_r,
			r);
		updateUniform(
			GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_t,
			t);

#ifdef CookTorrance
		//Cook-Torrance
		if (g_pGameSystem->getLightComponents().size() > 0)
		{
			for (auto& l_lightComponent : g_pGameSystem->getLightComponents())
			{
				// update light space transformation matrices
				if (l_lightComponent->m_lightType == lightType::DIRECTIONAL)
				{
					updateUniform(GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_p_light, 
						l_lightComponent->getProjectionMatrix(0));
					updateUniform(GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_v_light, 
						g_pGameSystem->getTransformComponent(l_lightComponent->getParentEntity())->m_transform.caclGlobalRotMatrix());

					// draw each visibleComponent
					for (auto& l_visibleComponent : g_pGameSystem->getVisibleComponents())
					{
						if (l_visibleComponent->m_visiblilityType == visiblilityType::STATIC_MESH)
						{
							glStencilFunc(GL_ALWAYS, 0x01, 0xFF);

							updateUniform(
								GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_m,
								g_pGameSystem->getTransformComponent(l_visibleComponent->getParentEntity())->m_transform.caclGlobalTransformationMatrix());

							// draw each graphic data of visibleComponent
							for (auto& l_graphicData : l_visibleComponent->getModelMap())
							{
								//active and bind textures
								// is there any texture?
								auto l_textureMap = &l_graphicData.second;
								if (l_textureMap != nullptr)
								{
									// any normal?
									auto l_normalTextureID = l_textureMap->find(textureType::NORMAL);
									if (l_normalTextureID != l_textureMap->end())
									{
										auto l_textureData = g_pAssetSystem->getTexture(l_normalTextureID->second);
										activateTexture(l_textureData, 0);
									}
									// any albedo?
									auto l_albedoTextureID = l_textureMap->find(textureType::ALBEDO);
									if (l_albedoTextureID != l_textureMap->end())
									{
										auto l_textureData = g_pAssetSystem->getTexture(l_albedoTextureID->second);
										activateTexture(l_textureData, 1);
									}
									// any metallic?
									auto l_metallicTextureID = l_textureMap->find(textureType::METALLIC);
									if (l_metallicTextureID != l_textureMap->end())
									{
										auto l_textureData = g_pAssetSystem->getTexture(l_metallicTextureID->second);
										activateTexture(l_textureData, 2);
									}
									// any roughness?
									auto l_roughnessTextureID = l_textureMap->find(textureType::ROUGHNESS);
									if (l_roughnessTextureID != l_textureMap->end())
									{
										auto l_textureData = g_pAssetSystem->getTexture(l_roughnessTextureID->second);
										activateTexture(l_textureData, 3);
									}
									// any ao?
									auto l_aoTextureID = l_textureMap->find(textureType::AMBIENT_OCCLUSION);
									if (l_aoTextureID != l_textureMap->end())
									{
										auto l_textureData = g_pAssetSystem->getTexture(l_aoTextureID->second);
										activateTexture(l_textureData, 4);
									}
								}
								updateUniform(GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_useTexture, l_visibleComponent->m_useTexture);
								updateUniform(GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_albedo, l_visibleComponent->m_albedo.x, l_visibleComponent->m_albedo.y, l_visibleComponent->m_albedo.z);
								updateUniform(GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_MRA, l_visibleComponent->m_MRA.x, l_visibleComponent->m_MRA.y, l_visibleComponent->m_MRA.z);
								// draw meshes
								auto l_mesh = g_pAssetSystem->getMesh(l_graphicData.first);
								activateMesh(l_mesh);
								drawMesh(l_mesh);
							}
						}
						else if (l_visibleComponent->m_visiblilityType == visiblilityType::EMISSIVE)
						{
							glStencilFunc(GL_ALWAYS, 0x02, 0xFF);

							updateUniform(
								GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_m,
								g_pGameSystem->getTransformComponent(l_visibleComponent->getParentEntity())->m_transform.caclGlobalTransformationMatrix());

							// draw each graphic data of visibleComponent
							for (auto& l_graphicData : l_visibleComponent->getModelMap())
							{
								updateUniform(GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_useTexture, l_visibleComponent->m_useTexture);
								updateUniform(GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_albedo, l_visibleComponent->m_albedo.x, l_visibleComponent->m_albedo.y, l_visibleComponent->m_albedo.z);
								// draw meshes
								auto l_mesh = g_pAssetSystem->getMesh(l_graphicData.first);
								activateMesh(l_mesh);
								drawMesh(l_mesh);
							}
						}
						else
						{
							glStencilFunc(GL_ALWAYS, 0x00, 0xFF);
						}
					}
				}
			}
		}
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_CLAMP);
		glDisable(GL_STENCIL_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#elif BlinnPhong
		// draw each visibleComponent
		for (auto& l_visibleComponent : visibleComponents)
		{
			if (l_visibleComponent->m_visiblilityType == visiblilityType::STATIC_MESH)
			{
				updateUniform(
					GeometryRenderPassSingletonComponent::getInstance().m_geometryPass_uni_m,
					g_pGameSystem->getTransformComponent(l_visibleComponent->getParentEntity())->m_transform.caclGlobalTransformationMatrix());

				// draw each graphic data of visibleComponent
				for (auto& l_graphicData : l_visibleComponent->getModelMap())
				{
					//active and bind textures
					// is there any texture?
					auto l_textureMap = &l_graphicData.second;
					if (l_textureMap != nullptr)
					{
						// any normal?
						auto l_normalTextureID = l_textureMap->find(textureType::NORMAL);
						if (l_normalTextureID != l_textureMap->end())
						{
							auto l_textureData = g_pAssetSystem->getTexture(l_normalTextureID->second);
							activateTexture(l_textureData, 0);
						}
						// any diffuse?
						auto l_diffuseTextureID = l_textureMap->find(textureType::ALBEDO);
						if (l_diffuseTextureID != l_textureMap->end())
						{
							auto l_textureData = g_pAssetSystem->getTexture(l_diffuseTextureID->second);
							activateTexture(l_textureData, 1);
						}
						// any specular?
						auto l_specularTextureID = l_textureMap->find(textureType::METALLIC);
						if (l_specularTextureID != l_textureMap->end())
						{
							auto l_textureData = g_pAssetSystem->getTexture(l_specularTextureID->second);
							activateTexture(l_textureData, 2);
						}
					}
					// draw meshes
					auto l_mesh = g_pAssetSystem->getMesh(l_graphicData.first);
					activateMesh(l_mesh);
					drawMesh(l_mesh);
				}
			}
		}
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_CLAMP);
#endif
	}
}

void GLRenderingSystem::updateLightRenderPass()
{
	// bind to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, LightRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);
	glBindRenderbuffer(GL_RENDERBUFFER, LightRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_RBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	// draw environment capture texture
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, 2048, 2048);
	glViewport(0, 0, 2048, 2048);

	// copy depth buffer from G-Pass
	glBindFramebuffer(GL_READ_FRAMEBUFFER, GeometryRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, LightRenderPassSingletonComponent::getInstance().m_GLFrameBufferComponent.m_FBO);
	glBlitFramebuffer(0, 0, 1280, 720, 0, 0, 1280, 720, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBlitFramebuffer(0, 0, 1280, 720, 0, 0, 1280, 720, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

#ifdef CookTorrance
	// Cook-Torrance
	// world space position + metallic
	activateTexture(&GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT0, 0);
	// normal + roughness
	activateTexture(&GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT1, 1);
	// albedo + ambient occlusion
	activateTexture(&GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT2, 2);
	// light space position
	activateTexture(&GeometryRenderPassSingletonComponent::getInstance().m_geometryPassTexture_RT3, 3);
	// shadow map
	activateTexture(&ShadowRenderPassSingletonComponent::getInstance().m_shadowForwardPassTexture_L0, 4);
	// irradiance environment map
	activateTexture(&EnvironmentRenderPassSingletonComponent::getInstance().m_capturePassTexture, 5);
	// pre-filter specular environment map
	activateTexture(&EnvironmentRenderPassSingletonComponent::getInstance().m_preFilterPassTexture, 6);
	// BRDF look-up table
	activateTexture(&EnvironmentRenderPassSingletonComponent::getInstance().m_BRDFLUTTexture, 7);
#endif
	if (g_pGameSystem->getLightComponents().size() > 0)
	{
		int l_pointLightIndexOffset = 0;
		for (auto i = (unsigned int)0; i < g_pGameSystem->getLightComponents().size(); i++)
		{
			auto l_viewPos = g_pGameSystem->getTransformComponent(g_pGameSystem->getCameraComponents()[0]->getParentEntity())->m_transform.caclGlobalPos();
			auto l_lightPos = g_pGameSystem->getTransformComponent(g_pGameSystem->getLightComponents()[i]->getParentEntity())->m_transform.caclGlobalPos();
			auto l_dirLightDirection = g_pGameSystem->getTransformComponent(g_pGameSystem->getLightComponents()[i]->getParentEntity())->m_transform.getDirection(direction::BACKWARD);
			auto l_lightColor = g_pGameSystem->getLightComponents()[i]->m_color;
			updateUniform(
				LightRenderPassSingletonComponent::getInstance().m_uni_viewPos,
				l_viewPos.x, l_viewPos.y, l_viewPos.z);
			//updateUniform(m_uni_textureMode, (int)in_shaderDrawPair.second);

			if (g_pGameSystem->getLightComponents()[i]->m_lightType == lightType::DIRECTIONAL)
			{
				l_pointLightIndexOffset -= 1;
				updateUniform(
					LightRenderPassSingletonComponent::getInstance().m_uni_dirLight_position,
					l_lightPos.x, l_lightPos.y, l_lightPos.z);
				updateUniform(
					LightRenderPassSingletonComponent::getInstance().m_uni_dirLight_direction,
					l_dirLightDirection.x, l_dirLightDirection.y, l_dirLightDirection.z);
				updateUniform(
					LightRenderPassSingletonComponent::getInstance().m_uni_dirLight_color,
					l_lightColor.x, l_lightColor.y, l_lightColor.z);
			}
			else if (g_pGameSystem->getLightComponents()[i]->m_lightType == lightType::POINT)
			{
				updateUniform(
					LightRenderPassSingletonComponent::getInstance().m_uni_pointLights_position[i + l_pointLightIndexOffset],
					l_lightPos.x, l_lightPos.y, l_lightPos.z);
				updateUniform(
					LightRenderPassSingletonComponent::getInstance().m_uni_pointLights_radius[i + l_pointLightIndexOffset],
					g_pGameSystem->getLightComponents()[i]->m_radius);
				updateUniform(
					LightRenderPassSingletonComponent::getInstance().m_uni_pointLights_color[i + l_pointLightIndexOffset],
					l_lightColor.x, l_lightColor.y, l_lightColor.z);
			}
		}
	}
	// draw light pass rectangle
	auto l_mesh = g_pAssetSystem->getDefaultMesh(meshShapeType::QUAD);
	activateMesh(l_mesh);
	drawMesh(l_mesh);
}

void GLRenderingSystem::updateFinalRenderPass()
{
}

void GLRenderingSystem::shutdown()
{
}

const objectStatus & GLRenderingSystem::getStatus() const
{
	return m_objectStatus;
}

GLuint GLRenderingSystem::getUniformLocation(GLuint shaderProgram, const std::string & uniformName)
{
	int uniformLocation = glGetUniformLocation(shaderProgram, uniformName.c_str());
	if (uniformLocation == 0xFFFFFFFF)
	{
		g_pLogSystem->printLog("innoShader: Error: Uniform lost: " + uniformName);
		return -1;
	}
	return uniformLocation;
}

void GLRenderingSystem::updateUniform(const GLint uniformLocation, bool uniformValue) const
{
	glUniform1i(uniformLocation, (int)uniformValue);
}

void GLRenderingSystem::updateUniform(const GLint uniformLocation, int uniformValue) const
{
	glUniform1i(uniformLocation, uniformValue);
}

void GLRenderingSystem::updateUniform(const GLint uniformLocation, double uniformValue) const
{
	glUniform1f(uniformLocation, (GLfloat)uniformValue);
}

void GLRenderingSystem::updateUniform(const GLint uniformLocation, double x, double y) const
{
	glUniform2f(uniformLocation, (GLfloat)x, (GLfloat)y);
}

void GLRenderingSystem::updateUniform(const GLint uniformLocation, double x, double y, double z) const
{
	glUniform3f(uniformLocation, (GLfloat)x, (GLfloat)y, (GLfloat)z);
}

void GLRenderingSystem::updateUniform(const GLint uniformLocation, double x, double y, double z, double w) const
{
	glUniform4f(uniformLocation, (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void GLRenderingSystem::updateUniform(const GLint uniformLocation, const mat4 & mat) const
{
#ifdef USE_COLUMN_MAJOR_MEMORY_LAYOUT
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &mat.m[0][0]);
#endif
#ifdef USE_ROW_MAJOR_MEMORY_LAYOUT
	glUniformMatrix4fv(uniformLocation, 1, GL_TRUE, &mat.m[0][0]);
#endif
}

void GLRenderingSystem::attachTextureToFramebuffer(const GLTextureDataComponent * GLTextureDataComponent, const GLFrameBufferComponent * GLFrameBufferComponent, int colorAttachmentIndex, int textureIndex, int mipLevel)
{
	if (GLTextureDataComponent->m_textureType == textureType::CUBEMAP || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CAPTURE || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CONVOLUTION || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_PREFILTER)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, GLTextureDataComponent->m_textureID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachmentIndex, GL_TEXTURE_CUBE_MAP_POSITIVE_X + textureIndex, GLTextureDataComponent->m_textureID, mipLevel);
	}
	else if (GLTextureDataComponent->m_textureType == textureType::SHADOWMAP)
	{
		glBindTexture(GL_TEXTURE_2D, GLTextureDataComponent->m_textureID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GLTextureDataComponent->m_textureID, mipLevel);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, GLTextureDataComponent->m_textureID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachmentIndex, GL_TEXTURE_2D, GLTextureDataComponent->m_textureID, mipLevel);
	}

}

void GLRenderingSystem::activateShaderProgram(const GLShaderProgramComponent * GLShaderProgramComponent)
{
	glUseProgram(GLShaderProgramComponent->m_program);
}

void GLRenderingSystem::activateMesh(const MeshDataComponent * GLTextureDataComponent)
{
	glBindVertexArray(GLTextureDataComponent->m_VAO);
}

void GLRenderingSystem::drawMesh(const MeshDataComponent * GLTextureDataComponent)
{
	glDrawElements(GL_TRIANGLES + (int)GLTextureDataComponent->m_meshDrawMethod, GLTextureDataComponent->m_indices.size(), GL_UNSIGNED_INT, 0);
}

void GLRenderingSystem::activateTexture(const TextureDataComponent * GLTextureDataComponent, int activateIndex)
{
	glActiveTexture(GL_TEXTURE0 + activateIndex);
	if (GLTextureDataComponent->m_textureType == textureType::CUBEMAP || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CAPTURE || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_CONVOLUTION || GLTextureDataComponent->m_textureType == textureType::ENVIRONMENT_PREFILTER)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, GLTextureDataComponent->m_textureID);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, GLTextureDataComponent->m_textureID);
	}

}
