#include "VXGIConvertPass.h"
#include "../DefaultGPUBuffers/DefaultGPUBuffers.h"

#include "VXGIRenderer.h"
#include "VXGIGeometryProcessPass.h"

#include "../../Engine/Interface/IEngine.h"

using namespace Inno;
extern INNO_ENGINE_API IEngine* g_Engine;

using namespace DefaultGPUBuffers;

bool VXGIConvertPass::Setup(ISystemConfig *systemConfig)
{
	auto l_VXGIRenderingConfig = VXGIRenderer::Get().GetVXGIRenderingConfig();
	
	auto l_RenderPassDesc = g_Engine->getRenderingFrontend()->getDefaultRenderPassDesc();

	m_luminanceVolume = g_Engine->getRenderingServer()->AddTextureDataComponent("VoxelLuminanceVolume/");
	m_luminanceVolume->m_TextureDesc = l_RenderPassDesc.m_RenderTargetDesc;

	m_luminanceVolume->m_TextureDesc.Width = l_VXGIRenderingConfig.m_voxelizationResolution;
	m_luminanceVolume->m_TextureDesc.Height = l_VXGIRenderingConfig.m_voxelizationResolution;
	m_luminanceVolume->m_TextureDesc.DepthOrArraySize = l_VXGIRenderingConfig.m_voxelizationResolution;
	m_luminanceVolume->m_TextureDesc.Usage = TextureUsage::Sample;
	m_luminanceVolume->m_TextureDesc.Sampler = TextureSampler::Sampler3D;
	m_luminanceVolume->m_TextureDesc.UseMipMap = true;

	m_normalVolume = g_Engine->getRenderingServer()->AddTextureDataComponent("VoxelNormalVolume/");
	m_normalVolume->m_TextureDesc = m_luminanceVolume->m_TextureDesc;

	m_SPC = g_Engine->getRenderingServer()->AddShaderProgramComponent("VoxelConvertPass/");

	m_SPC->m_ShaderFilePaths.m_CSPath = "voxelConvertPass.comp/";

	m_RPDC = g_Engine->getRenderingServer()->AddRenderPassDataComponent("VoxelConvertPass/");

	l_RenderPassDesc.m_RenderTargetCount = 0;
	l_RenderPassDesc.m_RenderPassUsage = RenderPassUsage::Compute;
	l_RenderPassDesc.m_IsOffScreen = true;

	m_RPDC->m_RenderPassDesc = l_RenderPassDesc;

	m_RPDC->m_ResourceBindingLayoutDescs.resize(4);

	m_RPDC->m_ResourceBindingLayoutDescs[0].m_GPUResourceType = GPUResourceType::Buffer;
	m_RPDC->m_ResourceBindingLayoutDescs[0].m_BindingAccessibility = Accessibility::ReadWrite;
	m_RPDC->m_ResourceBindingLayoutDescs[0].m_ResourceAccessibility = Accessibility::ReadWrite;
	m_RPDC->m_ResourceBindingLayoutDescs[0].m_DescriptorSetIndex = 1;
	m_RPDC->m_ResourceBindingLayoutDescs[0].m_DescriptorIndex = 0;

	m_RPDC->m_ResourceBindingLayoutDescs[1].m_GPUResourceType = GPUResourceType::Image;
	m_RPDC->m_ResourceBindingLayoutDescs[1].m_BindingAccessibility = Accessibility::ReadWrite;
	m_RPDC->m_ResourceBindingLayoutDescs[1].m_ResourceAccessibility = Accessibility::ReadWrite;
	m_RPDC->m_ResourceBindingLayoutDescs[1].m_DescriptorSetIndex = 1;
	m_RPDC->m_ResourceBindingLayoutDescs[1].m_DescriptorIndex = 1;
	m_RPDC->m_ResourceBindingLayoutDescs[1].m_IndirectBinding = true;

	m_RPDC->m_ResourceBindingLayoutDescs[2].m_GPUResourceType = GPUResourceType::Image;
	m_RPDC->m_ResourceBindingLayoutDescs[2].m_BindingAccessibility = Accessibility::ReadWrite;
	m_RPDC->m_ResourceBindingLayoutDescs[2].m_ResourceAccessibility = Accessibility::ReadWrite;
	m_RPDC->m_ResourceBindingLayoutDescs[2].m_DescriptorSetIndex = 1;
	m_RPDC->m_ResourceBindingLayoutDescs[2].m_DescriptorIndex = 2;
	m_RPDC->m_ResourceBindingLayoutDescs[2].m_IndirectBinding = true;

	m_RPDC->m_ResourceBindingLayoutDescs[3].m_GPUResourceType = GPUResourceType::Buffer;
	m_RPDC->m_ResourceBindingLayoutDescs[3].m_DescriptorSetIndex = 0;
	m_RPDC->m_ResourceBindingLayoutDescs[3].m_DescriptorIndex = 9;

	m_RPDC->m_ShaderProgram = m_SPC;

	m_ObjectStatus = ObjectStatus::Created;
	
	return true;
}

bool VXGIConvertPass::Initialize()
{	
	g_Engine->getRenderingServer()->InitializeShaderProgramComponent(m_SPC);
	g_Engine->getRenderingServer()->InitializeRenderPassDataComponent(m_RPDC);
	g_Engine->getRenderingServer()->InitializeTextureDataComponent(m_luminanceVolume);
	g_Engine->getRenderingServer()->InitializeTextureDataComponent(m_normalVolume);

	m_ObjectStatus = ObjectStatus::Activated;

	return true;
}

bool VXGIConvertPass::Terminate()
{
	g_Engine->getRenderingServer()->DeleteRenderPassDataComponent(m_RPDC);

	m_ObjectStatus = ObjectStatus::Terminated;

	return true;
}

ObjectStatus VXGIConvertPass::GetStatus()
{
	return m_ObjectStatus;
}

bool VXGIConvertPass::PrepareCommandList(IRenderingContext* renderingContext)
{	
	auto l_VXGIRenderingConfig = VXGIRenderer::Get().GetVXGIRenderingConfig();
	auto l_numThreadGroup = l_VXGIRenderingConfig.m_voxelizationResolution / 8;

	g_Engine->getRenderingServer()->CommandListBegin(m_RPDC, 0);
	g_Engine->getRenderingServer()->BindRenderPassDataComponent(m_RPDC);
	g_Engine->getRenderingServer()->CleanRenderTargets(m_RPDC);

	g_Engine->getRenderingServer()->BindGPUResource(m_RPDC, ShaderStage::Compute, VXGIGeometryProcessPass::Get().GetResult(), 0, Accessibility::ReadWrite);
	g_Engine->getRenderingServer()->BindGPUResource(m_RPDC, ShaderStage::Compute, m_luminanceVolume, 1, Accessibility::ReadWrite);
	g_Engine->getRenderingServer()->BindGPUResource(m_RPDC, ShaderStage::Compute, m_normalVolume, 2, Accessibility::ReadWrite);
	g_Engine->getRenderingServer()->BindGPUResource(m_RPDC, ShaderStage::Compute, VXGIRenderer::Get().GetVoxelizationCBuffer(), 3, Accessibility::ReadOnly);

	g_Engine->getRenderingServer()->Dispatch(m_RPDC,l_numThreadGroup, l_numThreadGroup, l_numThreadGroup);

	g_Engine->getRenderingServer()->UnbindGPUResource(m_RPDC, ShaderStage::Compute, VXGIGeometryProcessPass::Get().GetResult(), 0, Accessibility::ReadWrite);
	g_Engine->getRenderingServer()->UnbindGPUResource(m_RPDC, ShaderStage::Compute, m_luminanceVolume, 1, Accessibility::ReadWrite);
	g_Engine->getRenderingServer()->UnbindGPUResource(m_RPDC, ShaderStage::Compute, m_normalVolume, 2, Accessibility::ReadWrite);

	g_Engine->getRenderingServer()->CommandListEnd(m_RPDC);

	g_Engine->getRenderingServer()->GenerateMipmap(m_luminanceVolume);
	g_Engine->getRenderingServer()->GenerateMipmap(m_normalVolume);

	return true;
}

RenderPassDataComponent* VXGIConvertPass::GetRPDC()
{
	return m_RPDC;
}

GPUResourceComponent * VXGIConvertPass::GetLuminanceVolume()
{
	return m_luminanceVolume;
}

GPUResourceComponent * VXGIConvertPass::GetNormalVolume()
{
	return m_normalVolume;
}