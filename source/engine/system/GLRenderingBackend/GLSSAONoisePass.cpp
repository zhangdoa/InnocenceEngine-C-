#include "GLSSAONoisePass.h"
#include "GLOpaquePass.h"

#include "GLRenderingSystemUtilities.h"
#include "../../component/GLRenderingSystemComponent.h"

using namespace GLRenderingSystemNS;

#include "../ICoreSystem.h"

extern ICoreSystem* g_pCoreSystem;

INNO_PRIVATE_SCOPE GLSSAONoisePass
{
	void initializeShaders();

	void generateSSAONoiseTexture();

	EntityID m_entityID;

	GLRenderPassComponent* m_GLRPC;

	GLShaderProgramComponent* m_GLSPC;

	ShaderFilePaths m_shaderFilePaths = { "GL//SSAONoisePassVertex.sf" , "", "GL//SSAONoisePassFragment.sf" };

	unsigned int m_kernelSize = 64;
	std::vector<vec4> m_SSAOKernel;
	std::vector<vec4> m_SSAONoise;

	TextureDataComponent* m_SSAONoiseTDC;
	GLTextureDataComponent* m_SSAONoiseGLTDC;

	CameraDataPack m_cameraDataPack;
}

bool GLSSAONoisePass::initialize()
{
	m_entityID = InnoMath::createEntityID();

	m_GLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	initializeShaders();

	generateSSAONoiseTexture();

	return true;
}

void GLSSAONoisePass::initializeShaders()
{
	// shader programs and shaders
	auto rhs = addGLShaderProgramComponent(m_entityID);

	initializeGLShaderProgramComponent(rhs, m_shaderFilePaths);

	m_GLSPC = rhs;
}

void GLSSAONoisePass::generateSSAONoiseTexture()
{
	std::uniform_real_distribution<GLfloat> randomFloats(0.0f, 1.0f);
	std::default_random_engine generator;

	m_SSAOKernel.reserve(m_kernelSize);

	for (unsigned int i = 0; i < m_kernelSize; ++i)
	{
		auto sample = vec4(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator), 0.0f);
		sample = sample.normalize();
		sample = sample * randomFloats(generator);
		float scale = float(i) / float(m_kernelSize);

		// scale samples s.t. they're more aligned to center of kernel
		auto alpha = scale * scale;
		scale = 0.1f + 0.9f * alpha;
		sample = sample * scale;

		m_SSAOKernel.emplace_back(sample);
	}

	auto l_textureSize = 4;

	m_SSAONoise.reserve(l_textureSize * l_textureSize);

	for (size_t i = 0; i < m_SSAONoise.capacity(); i++)
	{
		// rotate around z-axis (in tangent space)
		auto noise = vec4(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, 0.0f, 0.0f);
		noise = noise.normalize();
		m_SSAONoise.push_back(noise);
	}

	m_SSAONoiseTDC = g_pCoreSystem->getAssetSystem()->addTextureDataComponent();

	m_SSAONoiseTDC->m_textureDataDesc.samplerType = TextureSamplerType::SAMPLER_2D;
	m_SSAONoiseTDC->m_textureDataDesc.usageType = TextureUsageType::RENDER_TARGET;
	m_SSAONoiseTDC->m_textureDataDesc.colorComponentsFormat = TextureColorComponentsFormat::RGB32F;
	m_SSAONoiseTDC->m_textureDataDesc.pixelDataFormat = TexturePixelDataFormat::RGBA;
	m_SSAONoiseTDC->m_textureDataDesc.minFilterMethod = TextureFilterMethod::NEAREST;
	m_SSAONoiseTDC->m_textureDataDesc.magFilterMethod = TextureFilterMethod::NEAREST;
	m_SSAONoiseTDC->m_textureDataDesc.wrapMethod = TextureWrapMethod::REPEAT;
	m_SSAONoiseTDC->m_textureDataDesc.width = l_textureSize;
	m_SSAONoiseTDC->m_textureDataDesc.height = l_textureSize;
	m_SSAONoiseTDC->m_textureDataDesc.pixelDataType = TexturePixelDataType::FLOAT;

	std::vector<float> l_pixelBuffer;
	auto l_containerSize = m_SSAONoise.size() * 4;
	l_pixelBuffer.reserve(l_containerSize);

	std::for_each(m_SSAONoise.begin(), m_SSAONoise.end(), [&](vec4 val)
	{
		l_pixelBuffer.emplace_back(val.x);
		l_pixelBuffer.emplace_back(val.y);
		l_pixelBuffer.emplace_back(val.z);
		l_pixelBuffer.emplace_back(val.w);
	});

	m_SSAONoiseTDC->m_textureData.emplace_back(&l_pixelBuffer[0]);

	m_SSAONoiseGLTDC = generateGLTextureDataComponent(m_SSAONoiseTDC);
}

bool GLSSAONoisePass::update()
{
	// copy camera data pack for local scope
	auto l_cameraDataPack = g_pCoreSystem->getVisionSystem()->getRenderingFrontend()->getCameraDataPack();
	if (l_cameraDataPack.has_value())
	{
		m_cameraDataPack = l_cameraDataPack.value();
	}

	activateRenderPass(m_GLRPC);

	activateShaderProgram(m_GLSPC);

	activateTexture(GLOpaquePass::getGLRPC()->m_GLTDCs[0], 0);
	activateTexture(GLOpaquePass::getGLRPC()->m_GLTDCs[1], 1);
	activateTexture(m_SSAONoiseGLTDC, 2);

	updateUniform(0, m_cameraDataPack.p_jittered);
	updateUniform(1, m_cameraDataPack.r);
	updateUniform(2, m_cameraDataPack.t);
	updateUniform(6, m_SSAOKernel);

	auto l_MDC = g_pCoreSystem->getAssetSystem()->getMeshDataComponent(MeshShapeType::QUAD);
	drawMesh(l_MDC);

	return true;
}

bool GLSSAONoisePass::resize(unsigned int newSizeX, unsigned int newSizeY)
{
	resizeGLRenderPassComponent(m_GLRPC, newSizeX, newSizeY);

	return true;
}

bool GLSSAONoisePass::reloadShader()
{
	deleteShaderProgram(m_GLSPC);

	initializeGLShaderProgramComponent(m_GLSPC, m_shaderFilePaths);

	return true;
}

GLRenderPassComponent * GLSSAONoisePass::getGLRPC()
{
	return m_GLRPC;
}