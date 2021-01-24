#include "FinalBlendPass.h"
#include "../DefaultGPUBuffers/DefaultGPUBuffers.h"

#include "BillboardPass.h"
#include "DebugPass.h"
#include "LuminanceHistogramPass.h"

#include "../../Engine/Interface/IEngine.h"

using namespace Inno;
extern INNO_ENGINE_API IEngine* g_Engine;

using namespace DefaultGPUBuffers;

namespace FinalBlendPass
{
	RenderPassDataComponent* m_RPDC;
	ShaderProgramComponent* m_SPC;
	TextureDataComponent* m_TDC;
}

bool FinalBlendPass::Setup()
{
	m_SPC = g_Engine->getRenderingServer()->AddShaderProgramComponent("FinalBlendPass/");

	m_SPC->m_ShaderFilePaths.m_CSPath = "finalBlendPass.comp/";

	m_RPDC = g_Engine->getRenderingServer()->AddRenderPassDataComponent("FinalBlendPass/");

	auto l_RenderPassDesc = g_Engine->getRenderingFrontend()->getDefaultRenderPassDesc();

	l_RenderPassDesc.m_RenderTargetCount = 0;
	l_RenderPassDesc.m_RenderPassUsage = RenderPassUsage::Compute;

	m_RPDC->m_RenderPassDesc = l_RenderPassDesc;

	m_RPDC->m_ResourceBinderLayoutDescs.resize(6);
	m_RPDC->m_ResourceBinderLayoutDescs[0].m_GPUResourceType = GPUResourceType::Image;
	m_RPDC->m_ResourceBinderLayoutDescs[0].m_DescriptorSetIndex = 1;
	m_RPDC->m_ResourceBinderLayoutDescs[0].m_DescriptorIndex = 0;
	m_RPDC->m_ResourceBinderLayoutDescs[0].m_IndirectBinding = true;

	m_RPDC->m_ResourceBinderLayoutDescs[1].m_GPUResourceType = GPUResourceType::Image;
	m_RPDC->m_ResourceBinderLayoutDescs[1].m_DescriptorSetIndex = 1;
	m_RPDC->m_ResourceBinderLayoutDescs[1].m_DescriptorIndex = 1;
	m_RPDC->m_ResourceBinderLayoutDescs[1].m_IndirectBinding = true;

	m_RPDC->m_ResourceBinderLayoutDescs[2].m_GPUResourceType = GPUResourceType::Image;
	m_RPDC->m_ResourceBinderLayoutDescs[2].m_DescriptorSetIndex = 1;
	m_RPDC->m_ResourceBinderLayoutDescs[2].m_DescriptorIndex = 2;
	m_RPDC->m_ResourceBinderLayoutDescs[2].m_IndirectBinding = true;

	m_RPDC->m_ResourceBinderLayoutDescs[3].m_GPUResourceType = GPUResourceType::Buffer;
	m_RPDC->m_ResourceBinderLayoutDescs[3].m_DescriptorSetIndex = 1;
	m_RPDC->m_ResourceBinderLayoutDescs[3].m_DescriptorIndex = 3;
	m_RPDC->m_ResourceBinderLayoutDescs[3].m_BindingAccessibility = Accessibility::ReadOnly;
	m_RPDC->m_ResourceBinderLayoutDescs[3].m_ResourceAccessibility = Accessibility::ReadWrite;

	m_RPDC->m_ResourceBinderLayoutDescs[4].m_GPUResourceType = GPUResourceType::Image;
	m_RPDC->m_ResourceBinderLayoutDescs[4].m_DescriptorSetIndex = 2;
	m_RPDC->m_ResourceBinderLayoutDescs[4].m_DescriptorIndex = 0;
	m_RPDC->m_ResourceBinderLayoutDescs[4].m_BindingAccessibility = Accessibility::ReadWrite;
	m_RPDC->m_ResourceBinderLayoutDescs[4].m_ResourceAccessibility = Accessibility::ReadWrite;
	m_RPDC->m_ResourceBinderLayoutDescs[4].m_IndirectBinding = true;

	m_RPDC->m_ResourceBinderLayoutDescs[5].m_GPUResourceType = GPUResourceType::Buffer;
	m_RPDC->m_ResourceBinderLayoutDescs[5].m_DescriptorSetIndex = 0;
	m_RPDC->m_ResourceBinderLayoutDescs[5].m_DescriptorIndex = 0;

	m_RPDC->m_ShaderProgram = m_SPC;

	m_TDC = g_Engine->getRenderingServer()->AddTextureDataComponent("FinalBlendPass/");
	m_TDC->m_TextureDesc = l_RenderPassDesc.m_RenderTargetDesc;

	return true;
}

bool FinalBlendPass::Initialize()
{
	g_Engine->getRenderingServer()->InitializeShaderProgramComponent(m_SPC);
	g_Engine->getRenderingServer()->InitializeRenderPassDataComponent(m_RPDC);
	g_Engine->getRenderingServer()->InitializeTextureDataComponent(m_TDC);
	g_Engine->getRenderingServer()->SetUserPipelineOutput(m_TDC);

	return true;
}

bool FinalBlendPass::Render(GPUResourceComponent* input)
{
	auto l_viewportSize = g_Engine->getRenderingFrontend()->getScreenResolution();
	auto l_PerFrameCBufferGBDC = GetGPUBufferDataComponent(GPUBufferUsageType::PerFrame);

	g_Engine->getRenderingServer()->CommandListBegin(m_RPDC, 0);
	g_Engine->getRenderingServer()->BindRenderPassDataComponent(m_RPDC);
	g_Engine->getRenderingServer()->CleanRenderTargets(m_RPDC);

	g_Engine->getRenderingServer()->BindGPUResource(m_RPDC, ShaderStage::Compute, input, 0, 0);
	g_Engine->getRenderingServer()->BindGPUResource(m_RPDC, ShaderStage::Compute, BillboardPass::GetRPDC()->m_RenderTargets[0], 1, 1);
	g_Engine->getRenderingServer()->BindGPUResource(m_RPDC, ShaderStage::Compute, DebugPass::GetRPDC()->m_RenderTargets[0], 2, 2);
	g_Engine->getRenderingServer()->BindGPUResource(m_RPDC, ShaderStage::Compute, LuminanceHistogramPass::GetAverageLuminance(), 3, 3, Accessibility::ReadOnly);
	g_Engine->getRenderingServer()->BindGPUResource(m_RPDC, ShaderStage::Compute, m_TDC, 4, 0, Accessibility::ReadWrite);
	g_Engine->getRenderingServer()->BindGPUResource(m_RPDC, ShaderStage::Compute, l_PerFrameCBufferGBDC, 5, 0, Accessibility::ReadOnly);

	g_Engine->getRenderingServer()->Dispatch(m_RPDC, uint32_t(l_viewportSize.x / 8.0f), uint32_t(l_viewportSize.y / 8.0f), 1);

	g_Engine->getRenderingServer()->UnbindGPUResource(m_RPDC, ShaderStage::Compute, input, 0, 0);
	g_Engine->getRenderingServer()->UnbindGPUResource(m_RPDC, ShaderStage::Compute, BillboardPass::GetRPDC()->m_RenderTargets[0], 1, 1);
	g_Engine->getRenderingServer()->UnbindGPUResource(m_RPDC, ShaderStage::Compute, DebugPass::GetRPDC()->m_RenderTargets[0], 2, 2);
	g_Engine->getRenderingServer()->UnbindGPUResource(m_RPDC, ShaderStage::Compute, m_TDC, 4, 0, Accessibility::ReadWrite);
	g_Engine->getRenderingServer()->UnbindGPUResource(m_RPDC, ShaderStage::Compute, LuminanceHistogramPass::GetAverageLuminance(), 3, 3, Accessibility::ReadOnly);

	g_Engine->getRenderingServer()->CommandListEnd(m_RPDC);

	g_Engine->getRenderingServer()->ExecuteCommandList(m_RPDC);

	g_Engine->getRenderingServer()->WaitForFrame(m_RPDC);

	return true;
}

bool FinalBlendPass::Terminate()
{
	g_Engine->getRenderingServer()->DeleteRenderPassDataComponent(m_RPDC);

	return true;
}

RenderPassDataComponent* FinalBlendPass::GetRPDC()
{
	return m_RPDC;
}

ShaderProgramComponent* FinalBlendPass::getSPC()
{
	return m_SPC;
}

GPUResourceComponent* FinalBlendPass::GetResult()
{
	return m_TDC;
}