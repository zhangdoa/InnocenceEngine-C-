#pragma once
#include "RenderPassDataComponent.h"
#include "../RenderingBackend/DX12RenderingBackend/DX12Headers.h"

class DX12ResourceBinder : public IResourceBinder
{
public:
	std::vector<void*> m_Resources;
};

class DX12PipelineStateObject : public IPipelineStateObject
{
public:
	D3D12_INPUT_ELEMENT_DESC m_InputLayoutDesc = {};
	D3D12_DEPTH_STENCIL_DESC m_DepthStencilDesc = {};
	D3D12_BLEND_DESC m_BlendDesc = {};
	D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveTopology;
	D3D12_RASTERIZER_DESC m_RasterizerDesc = {};
	D3D12_VIEWPORT m_Viewport = {};
	D3D12_RECT m_Scissor = {};
	D3D12_SAMPLER_DESC m_SamplerDesc = {};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_PSODesc = {};
	ID3D12PipelineState* m_PSO = 0;
};

class DX12CommandList : public ICommandList
{
public:
	ID3D12GraphicsCommandList* m_CommandList = 0;
};

class DX12CommandQueue : public ICommandQueue
{
public:
	D3D12_COMMAND_QUEUE_DESC m_CommandQueueDesc = {};
	ID3D12CommandQueue* m_CommandQueue = 0;
};

class DX12Semaphore : public ISemaphore
{
};

class DX12Fence : public IFence
{
	ID3D12Fence* m_Fence = 0;
	HANDLE m_FenceEvent = 0;
};

class DX12RenderPassDataComponent : public RenderPassDataComponent
{
public:
	ID3D12DescriptorHeap* m_RTVDescriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC m_RTVDescriptorHeapDesc = {};
	D3D12_RENDER_TARGET_VIEW_DESC m_RTVDesc = {};
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_RTVDescriptorCPUHandles;
	std::vector<DX12SRV> m_SRVs;

	ID3D12DescriptorHeap* m_DSVDescriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC m_DSVDescriptorHeapDesc = {};
	D3D12_DEPTH_STENCIL_VIEW_DESC m_DSVDesc = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_DSVDescriptorCPUHandle;

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC m_RootSignatureDesc = {};
	ID3D12RootSignature* m_RootSignature = 0;

	std::vector<ID3D12CommandAllocator*> m_CommandAllocators;

	std::vector<unsigned long long> m_FenceStatus;
};