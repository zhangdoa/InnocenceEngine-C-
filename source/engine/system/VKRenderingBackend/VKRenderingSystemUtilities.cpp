#pragma once
#include "VKRenderingSystemUtilities.h"
#include "../../component/VKRenderingSystemComponent.h"

#include "../ICoreSystem.h"

extern ICoreSystem* g_pCoreSystem;

INNO_PRIVATE_SCOPE VKRenderingSystemNS
{
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	bool copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	bool createShaderModule(VkShaderModule& vkShaderModule, const std::string& shaderFilePath);

	bool summitGPUData(VKMeshDataComponent * rhs);

	bool createVBO(const std::vector<Vertex>& vertices, VkBuffer& VBO);
	bool createIBO(const std::vector<Index>& indices, VkBuffer& IBO);

	bool summitGPUData(VKTextureDataComponent * rhs);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkTextureDataDesc getVKTextureDataDesc(const TextureDataDesc& textureDataDesc);
	VkSamplerAddressMode getTextureWrapMethod(TextureWrapMethod rhs);
	VkSamplerMipmapMode getTextureFilterParam(TextureFilterMethod rhs);
	VkFormat getTextureInternalFormat(TextureColorComponentsFormat rhs);

	VkVertexInputBindingDescription m_vertexBindingDescription;
	std::array<VkVertexInputAttributeDescription, 5> m_vertexAttributeDescriptions;

	std::unordered_map<EntityID, VKMeshDataComponent*> m_initializedVKMDC;
	std::unordered_map<EntityID, VKTextureDataComponent*> m_initializedVKTDC;

	std::unordered_map<EntityID, VKMeshDataComponent*> m_meshMap;
	std::unordered_map<EntityID, VKTextureDataComponent*> m_textureMap;

	void* m_VKRenderPassComponentPool;
	void* m_VKShaderProgramComponentPool;

	const std::string m_shaderRelativePath = std::string{ "res//shaders//" };
}

bool VKRenderingSystemNS::initializeComponentPool()
{
	m_VKRenderPassComponentPool = g_pCoreSystem->getMemorySystem()->allocateMemoryPool(sizeof(VKRenderPassComponent), 32);
	m_VKShaderProgramComponentPool = g_pCoreSystem->getMemorySystem()->allocateMemoryPool(sizeof(VKShaderProgramComponent), 128);

	m_vertexBindingDescription = {};
	m_vertexBindingDescription.binding = 0;
	m_vertexBindingDescription.stride = sizeof(Vertex);
	m_vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	m_vertexAttributeDescriptions = {};

	m_vertexAttributeDescriptions[0].binding = 0;
	m_vertexAttributeDescriptions[0].location = 0;
	m_vertexAttributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	m_vertexAttributeDescriptions[0].offset = offsetof(Vertex, m_pos);

	m_vertexAttributeDescriptions[1].binding = 0;
	m_vertexAttributeDescriptions[1].location = 1;
	m_vertexAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	m_vertexAttributeDescriptions[1].offset = offsetof(Vertex, m_texCoord);

	m_vertexAttributeDescriptions[2].binding = 0;
	m_vertexAttributeDescriptions[2].location = 2;
	m_vertexAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	m_vertexAttributeDescriptions[2].offset = offsetof(Vertex, m_pad1);

	m_vertexAttributeDescriptions[3].binding = 0;
	m_vertexAttributeDescriptions[3].location = 3;
	m_vertexAttributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	m_vertexAttributeDescriptions[3].offset = offsetof(Vertex, m_normal);

	m_vertexAttributeDescriptions[4].binding = 0;
	m_vertexAttributeDescriptions[4].location = 4;
	m_vertexAttributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	m_vertexAttributeDescriptions[4].offset = offsetof(Vertex, m_pad2);

	return true;
}

bool VKRenderingSystemNS::checkValidationLayerSupport()
{
	uint32_t l_layerCount;
	vkEnumerateInstanceLayerProperties(&l_layerCount, nullptr);

	std::vector<VkLayerProperties> l_availableLayers(l_layerCount);
	vkEnumerateInstanceLayerProperties(&l_layerCount, l_availableLayers.data());

	for (const char* layerName : VKRenderingSystemComponent::get().m_validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : l_availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

bool VKRenderingSystemNS::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(VKRenderingSystemComponent::get().m_deviceExtensions.begin(), VKRenderingSystemComponent::get().m_deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices VKRenderingSystemNS::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.m_graphicsFamily = i;
		}

		VkBool32 l_presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VKRenderingSystemComponent::get().m_windowSurface, &l_presentSupport);

		if (queueFamily.queueCount > 0 && l_presentSupport)
		{
			indices.m_presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

VkSurfaceFormatKHR VKRenderingSystemNS::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VKRenderingSystemNS::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	VkPresentModeKHR l_bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			l_bestMode = availablePresentMode;
		}
	}

	return l_bestMode;
}

VkExtent2D VKRenderingSystemNS::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		auto l_screenResolution = g_pCoreSystem->getVisionSystem()->getRenderingFrontend()->getScreenResolution();

		VkExtent2D l_actualExtent;
		l_actualExtent.width = l_screenResolution.x;
		l_actualExtent.height = l_screenResolution.y;

		l_actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, l_actualExtent.width));
		l_actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, l_actualExtent.height));

		return l_actualExtent;
	}
}

SwapChainSupportDetails VKRenderingSystemNS::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails l_details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VKRenderingSystemComponent::get().m_windowSurface, &l_details.m_capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, VKRenderingSystemComponent::get().m_windowSurface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		l_details.m_formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, VKRenderingSystemComponent::get().m_windowSurface, &formatCount, l_details.m_formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, VKRenderingSystemComponent::get().m_windowSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		l_details.m_presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, VKRenderingSystemComponent::get().m_windowSurface, &presentModeCount, l_details.m_presentModes.data());
	}

	return l_details;
}

bool VKRenderingSystemNS::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.m_formats.empty() && !swapChainSupport.m_presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

VKRenderPassComponent* VKRenderingSystemNS::addVKRenderPassComponent()
{
	auto l_rawPtr = g_pCoreSystem->getMemorySystem()->spawnObject(m_VKRenderPassComponentPool, sizeof(VKRenderPassComponent));
	auto l_VKRPC = new(l_rawPtr)VKRenderPassComponent();

	return l_VKRPC;
}

bool VKRenderingSystemNS::initializeVKRenderPassComponent(VKRenderPassComponent* VKRPC, VKShaderProgramComponent* VKSPC)
{
	bool result = true;

	result &= reserveRenderTargets(VKRPC);

	auto l_vkTextureDesc = getVKTextureDataDesc(VKRPC->m_renderPassDesc.RTDesc);

	result &= createRenderTargets(VKRPC);

	result &= createRenderPass(VKRPC);

	if (VKRPC->m_renderPassDesc.useMultipleFramebuffers)
	{
		result &= createMultipleFramebuffers(VKRPC);
	}
	else
	{
		result &= createSingleFramebuffer(VKRPC);
	}

	result &= createDescriptorSetLayout(VKRPC);
	result &= createDescriptorSet(VKRPC);
	result &= updateDescriptorSet(VKRPC);

	result &= createPipelineLayout(VKRPC);

	result &= createGraphicsPipelines(VKRPC, VKSPC);

	result &= createCommandBuffers(VKRPC);

	result &= createSyncPrimitives(VKRPC);

	return result;
}

bool VKRenderingSystemNS::reserveRenderTargets(VKRenderPassComponent* VKRPC)
{
	size_t l_framebufferNumber = 0;
	if (VKRPC->m_renderPassDesc.useMultipleFramebuffers)
	{
		l_framebufferNumber = VKRPC->m_renderPassDesc.RTNumber;
	}
	else
	{
		l_framebufferNumber = 1;
	}

	// reserve vectors and emplace empty objects
	VKRPC->m_framebuffers.reserve(l_framebufferNumber);
	for (size_t i = 0; i < l_framebufferNumber; i++)
	{
		VKRPC->m_framebuffers.emplace_back();
	}

	VKRPC->m_VKTDCs.reserve(VKRPC->m_renderPassDesc.RTNumber);
	for (size_t i = 0; i < VKRPC->m_renderPassDesc.RTNumber; i++)
	{
		VKRPC->m_VKTDCs.emplace_back();
	}

	for (size_t i = 0; i < VKRPC->m_renderPassDesc.RTNumber; i++)
	{
		VKRPC->m_VKTDCs[i] = addVKTextureDataComponent();
	}

	return true;
}

bool VKRenderingSystemNS::createRenderTargets(VKRenderPassComponent* VKRPC)
{
	for (size_t i = 0; i < VKRPC->m_renderPassDesc.RTNumber; i++)
	{
		auto l_TDC = VKRPC->m_VKTDCs[i];

		l_TDC->m_textureDataDesc = VKRPC->m_renderPassDesc.RTDesc;

		if (l_TDC->m_textureDataDesc.samplerType == TextureSamplerType::CUBEMAP)
		{
			l_TDC->m_textureData = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		}
		else
		{
			l_TDC->m_textureData = { nullptr };
		}

		initializeVKTextureDataComponent(l_TDC);
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: Render targets have been created.");

	return true;
}

bool VKRenderingSystemNS::createSingleFramebuffer(VKRenderPassComponent* VKRPC)
{
	// create frame buffer and attach image view

	std::vector<VkImageView> attachments(VKRPC->m_VKTDCs.size());

	for (size_t i = 0; i < VKRPC->m_VKTDCs.size(); i++)
	{
		attachments[i] = VKRPC->m_VKTDCs[i]->m_imageView;
	}

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = VKRPC->m_renderPass;
	framebufferInfo.attachmentCount = (uint32_t)VKRPC->m_VKTDCs.size();
	framebufferInfo.pAttachments = &attachments[0];
	framebufferInfo.width = (uint32_t)VKRPC->viewport.width;
	framebufferInfo.height = (uint32_t)VKRPC->viewport.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(VKRenderingSystemComponent::get().m_device, &framebufferInfo, nullptr, &VKRPC->m_framebuffers[0]) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create VkFramebuffer!");
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: Single VkFramebuffer has been created.");
	return true;
}

bool VKRenderingSystemNS::createMultipleFramebuffers(VKRenderPassComponent* VKRPC)
{
	for (size_t i = 0; i < VKRPC->m_VKTDCs.size(); i++)
	{
		// create frame buffer and attach image view
		VkImageView attachments[] = { VKRPC->m_VKTDCs[i]->m_imageView };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = VKRPC->m_renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = (uint32_t)VKRPC->viewport.width;
		framebufferInfo.height = (uint32_t)VKRPC->viewport.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(VKRenderingSystemComponent::get().m_device, &framebufferInfo, nullptr, &VKRPC->m_framebuffers[i]) != VK_SUCCESS)
		{
			g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create VkFramebuffer!");
		}
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: Multiple VkFramebuffers have been created.");
	return true;
}

bool VKRenderingSystemNS::createRenderPass(VKRenderPassComponent* VKRPC)
{
	if (vkCreateRenderPass(VKRenderingSystemComponent::get().m_device, &VKRPC->renderPassCInfo, nullptr, &VKRPC->m_renderPass) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create VkRenderPass!");
		return false;
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: VkRenderPass has been created.");
	return true;
}

bool VKRenderingSystemNS::createDescriptorSetLayout(VKRenderPassComponent* VKRPC)
{
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(VKRPC->descriptorSetLayoutBindings.size());
	layoutInfo.pBindings = VKRPC->descriptorSetLayoutBindings.data();

	if (vkCreateDescriptorSetLayout(VKRenderingSystemComponent::get().m_device, &layoutInfo, nullptr, &VKRPC->descriptorSetLayout) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create VkDescriptorSetLayout!");
		return false;
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: VkDescriptorSetLayout has been created.");
	return true;
}

bool VKRenderingSystemNS::createDescriptorSet(VKRenderPassComponent* VKRPC)
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = VKRenderingSystemComponent::get().m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &VKRPC->descriptorSetLayout;

	if (vkAllocateDescriptorSets(VKRenderingSystemComponent::get().m_device, &allocInfo, &VKRPC->descriptorSet) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to allocate VkDescriptorSet!");
		return false;
	}

	for (auto& i : VKRPC->writeDescriptorSets)
	{
		i.dstSet = VKRPC->descriptorSet;
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: VkDescriptorSet has been allocated.");
	return true;
}

bool VKRenderingSystemNS::updateDescriptorSet(VKRenderPassComponent* VKRPC)
{
	vkUpdateDescriptorSets(
		VKRenderingSystemComponent::get().m_device,
		static_cast<uint32_t>(VKRPC->writeDescriptorSets.size()),
		VKRPC->writeDescriptorSets.data(),
		0,
		nullptr);

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: Write VkDescriptorSet has been updated.");
	return true;
}

bool VKRenderingSystemNS::createPipelineLayout(VKRenderPassComponent* VKRPC)
{
	VKRPC->pipelineLayoutCInfo.pSetLayouts = &VKRPC->descriptorSetLayout;

	if (vkCreatePipelineLayout(VKRenderingSystemComponent::get().m_device, &VKRPC->pipelineLayoutCInfo, nullptr, &VKRPC->m_pipelineLayout) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create VkPipelineLayout!");
		return false;
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: VkPipelineLayout has been created.");
	return true;
}

bool VKRenderingSystemNS::createGraphicsPipelines(VKRenderPassComponent* VKRPC, VKShaderProgramComponent* VKSPC)
{
	// attach shader module and create pipeline
	std::vector<VkPipelineShaderStageCreateInfo> l_shaderStages = { VKSPC->m_vertexShaderStageCInfo, VKSPC->m_fragmentShaderStageCInfo };

	VKRPC->pipelineCInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	VKRPC->pipelineCInfo.stageCount = (uint32_t)l_shaderStages.size();
	VKRPC->pipelineCInfo.pStages = &l_shaderStages[0];
	VKRPC->pipelineCInfo.pVertexInputState = &VKSPC->m_vertexInputStateCInfo;
	VKRPC->pipelineCInfo.pInputAssemblyState = &VKRPC->inputAssemblyStateCInfo;
	VKRPC->pipelineCInfo.pViewportState = &VKRPC->viewportStateCInfo;
	VKRPC->pipelineCInfo.pRasterizationState = &VKRPC->rasterizationStateCInfo;
	VKRPC->pipelineCInfo.pMultisampleState = &VKRPC->multisampleStateCInfo;
	VKRPC->pipelineCInfo.pColorBlendState = &VKRPC->colorBlendStateCInfo;
	VKRPC->pipelineCInfo.layout = VKRPC->m_pipelineLayout;
	VKRPC->pipelineCInfo.renderPass = VKRPC->m_renderPass;
	VKRPC->pipelineCInfo.subpass = 0;
	VKRPC->pipelineCInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(VKRenderingSystemComponent::get().m_device, VK_NULL_HANDLE, 1, &VKRPC->pipelineCInfo, nullptr, &VKRPC->m_pipeline) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to to create VkPipeline!");
		return false;
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: VkPipeline has been created.");
	return true;
}

bool VKRenderingSystemNS::createCommandBuffers(VKRenderPassComponent* VKRPC)
{
	VKRPC->m_commandBuffers.resize(VKRPC->m_framebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VKRenderingSystemComponent::get().m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)VKRPC->m_commandBuffers.size();

	if (vkAllocateCommandBuffers(VKRenderingSystemComponent::get().m_device, &allocInfo, VKRPC->m_commandBuffers.data()) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to allocate VkCommandBuffer!");
		return false;
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: VkCommandBuffer has been created.");
	return true;
}

bool VKRenderingSystemNS::createSyncPrimitives(VKRenderPassComponent* VKRPC)
{
	VKRPC->m_renderFinishedSemaphores.resize(VKRPC->m_maxFramesInFlight);
	VKRPC->m_inFlightFences.resize(VKRPC->m_maxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VKRPC->submitInfo = submitInfo;

	for (size_t i = 0; i < VKRPC->m_maxFramesInFlight; i++)
	{
		if (vkCreateSemaphore(VKRenderingSystemComponent::get().m_device, &semaphoreInfo, nullptr, &VKRPC->m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(VKRenderingSystemComponent::get().m_device, &fenceInfo, nullptr, &VKRPC->m_inFlightFences[i]) != VK_SUCCESS)
		{
			g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create synchronization primitives!");
			return false;
		}
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: Synchronization primitives has been created.");

	return true;
}

bool VKRenderingSystemNS::destroyVKRenderPassComponent(VKRenderPassComponent* VKRPC)
{
	for (size_t i = 0; i < VKRPC->m_maxFramesInFlight; i++)
	{
		vkDestroySemaphore(VKRenderingSystemComponent::get().m_device, VKRenderingSystemComponent::get().m_swapChainVKRPC->m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(VKRenderingSystemComponent::get().m_device, VKRenderingSystemComponent::get().m_swapChainVKRPC->m_inFlightFences[i], nullptr);
	}

	for (auto framebuffer : VKRPC->m_framebuffers)
	{
		vkDestroyFramebuffer(VKRenderingSystemComponent::get().m_device, framebuffer, nullptr);
	}
	vkDestroyPipeline(VKRenderingSystemComponent::get().m_device, VKRPC->m_pipeline, nullptr);
	vkDestroyPipelineLayout(VKRenderingSystemComponent::get().m_device, VKRPC->m_pipelineLayout, nullptr);
	vkDestroyRenderPass(VKRenderingSystemComponent::get().m_device, VKRPC->m_renderPass, nullptr);
	for (auto VKTDC : VKRPC->m_VKTDCs)
	{
		vkDestroyImageView(VKRenderingSystemComponent::get().m_device, VKTDC->m_imageView, nullptr);
	}

	return true;
}

VkCommandBuffer VKRenderingSystemNS::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = VKRenderingSystemComponent::get().m_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(VKRenderingSystemComponent::get().m_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VKRenderingSystemNS::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(VKRenderingSystemComponent::get().m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(VKRenderingSystemComponent::get().m_graphicsQueue);

	vkFreeCommandBuffers(VKRenderingSystemComponent::get().m_device, VKRenderingSystemComponent::get().m_commandPool, 1, &commandBuffer);
}

uint32_t VKRenderingSystemNS::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(VKRenderingSystemComponent::get().m_physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to find suitable memory type!!");
	return 0;
}

bool VKRenderingSystemNS::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VKRenderingSystemComponent::get().m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create VkBuffer!");
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(VKRenderingSystemComponent::get().m_device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(VKRenderingSystemComponent::get().m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to allocate VkDeviceMemory for VkBuffer!");
		return false;
	}

	vkBindBufferMemory(VKRenderingSystemComponent::get().m_device, buffer, bufferMemory, 0);

	return true;
}

bool VKRenderingSystemNS::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);

	return true;
}

bool VKRenderingSystemNS::initializeVKMeshDataComponent(VKMeshDataComponent* rhs)
{
	if (rhs->m_objectStatus == ObjectStatus::ALIVE)
	{
		return true;
	}
	else
	{
		summitGPUData(rhs);

		rhs->m_objectStatus = ObjectStatus::ALIVE;

		return true;
	}
}

bool VKRenderingSystemNS::summitGPUData(VKMeshDataComponent * rhs)
{
	createVBO(rhs->m_vertices, rhs->m_VBO);
	createIBO(rhs->m_indices, rhs->m_IBO);

	rhs->m_objectStatus = ObjectStatus::ALIVE;

	m_initializedVKMDC.emplace(rhs->m_parentEntity, rhs);

	return true;
}

bool VKRenderingSystemNS::createVBO(const std::vector<Vertex>& vertices, VkBuffer& VBO)
{
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(VKRenderingSystemComponent::get().m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	std::memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(VKRenderingSystemComponent::get().m_device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VBO, VKRenderingSystemComponent::get().m_vertexBufferMemory);

	copyBuffer(stagingBuffer, VBO, bufferSize);

	vkDestroyBuffer(VKRenderingSystemComponent::get().m_device, stagingBuffer, nullptr);
	vkFreeMemory(VKRenderingSystemComponent::get().m_device, stagingBufferMemory, nullptr);

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_VERBOSE, "VKRenderingSystem: VBO " + InnoUtility::pointerToString(VBO) + " is initialized.");

	return true;
}

bool  VKRenderingSystemNS::createIBO(const std::vector<Index>& indices, VkBuffer& IBO)
{
	VkDeviceSize bufferSize = sizeof(Index) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(VKRenderingSystemComponent::get().m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	std::memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(VKRenderingSystemComponent::get().m_device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IBO, VKRenderingSystemComponent::get().m_indexBufferMemory);

	copyBuffer(stagingBuffer, IBO, bufferSize);

	vkDestroyBuffer(VKRenderingSystemComponent::get().m_device, stagingBuffer, nullptr);
	vkFreeMemory(VKRenderingSystemComponent::get().m_device, stagingBufferMemory, nullptr);

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_VERBOSE, "VKRenderingSystem: IBO " + InnoUtility::pointerToString(IBO) + " is initialized.");

	return true;
}

bool VKRenderingSystemNS::initializeVKTextureDataComponent(VKTextureDataComponent * rhs)
{
	if (rhs->m_objectStatus == ObjectStatus::ALIVE)
	{
		return true;
	}
	else
	{
		if (rhs->m_textureDataDesc.usageType == TextureUsageType::INVISIBLE)
		{
			g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_WARNING, "VKRenderingSystem: try to generate VKTextureDataComponent for TextureUsageType::INVISIBLE type!");
			return false;
		}
		else
		{
			if (rhs->m_textureData.size() > 0)
			{
				summitGPUData(rhs);

				rhs->m_objectStatus = ObjectStatus::ALIVE;

				if (rhs->m_textureDataDesc.usageType != TextureUsageType::RENDER_TARGET)
				{
					// @TODO: release raw data in heap memory
				}

				return true;
			}
			else
			{
				g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_WARNING, "VKRenderingSystem: try to generate VKTextureDataComponent without raw data!");
				return false;
			}
		}
	}
}

bool VKRenderingSystemNS::summitGPUData(VKTextureDataComponent * rhs)
{
	rhs->m_VkTextureDataDesc = getVKTextureDataDesc(rhs->m_textureDataDesc);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(rhs->m_VkTextureDataDesc.imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	auto l_srcData = rhs->m_textureData[0];
	if (l_srcData != nullptr)
	{
		void* l_dstData;
		vkMapMemory(VKRenderingSystemComponent::get().m_device, stagingBufferMemory, 0, rhs->m_VkTextureDataDesc.imageSize, 0, &l_dstData);
		memcpy(l_dstData, rhs->m_textureData[0], static_cast<size_t>(rhs->m_VkTextureDataDesc.imageSize));
		vkUnmapMemory(VKRenderingSystemComponent::get().m_device, stagingBufferMemory);
	}

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = rhs->m_textureDataDesc.width;
	imageInfo.extent.height = rhs->m_textureDataDesc.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = rhs->m_VkTextureDataDesc.internalFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (rhs->m_textureDataDesc.usageType == TextureUsageType::RENDER_TARGET)
	{
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	else
	{
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	}

	if (vkCreateImage(VKRenderingSystemComponent::get().m_device, &imageInfo, nullptr, &rhs->m_image) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create VkImage!");
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(VKRenderingSystemComponent::get().m_device, rhs->m_image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(VKRenderingSystemComponent::get().m_device, &allocInfo, nullptr, &VKRenderingSystemComponent::get().m_textureImageMemory) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to allocate VkDeviceMemory for VkImage!");
		return false;
	}

	vkBindImageMemory(VKRenderingSystemComponent::get().m_device, rhs->m_image, VKRenderingSystemComponent::get().m_textureImageMemory, 0);

	if (rhs->m_textureDataDesc.usageType != TextureUsageType::RENDER_TARGET)
	{
		transitionImageLayout(rhs->m_image, imageInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		if (l_srcData != nullptr)
		{
			copyBufferToImage(stagingBuffer, rhs->m_image, static_cast<uint32_t>(imageInfo.extent.width), static_cast<uint32_t>(imageInfo.extent.height));
		}
		transitionImageLayout(rhs->m_image, imageInfo.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	vkDestroyBuffer(VKRenderingSystemComponent::get().m_device, stagingBuffer, nullptr);
	vkFreeMemory(VKRenderingSystemComponent::get().m_device, stagingBufferMemory, nullptr);

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_VERBOSE, "VKRenderingSystem: VkImage " + InnoUtility::pointerToString(rhs->m_image) + " is initialized.");

	createImageView(rhs);

	rhs->m_objectStatus = ObjectStatus::ALIVE;

	m_initializedVKTDC.emplace(rhs->m_parentEntity, rhs);

	return true;
}

void VKRenderingSystemNS::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Unsupported transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(commandBuffer);
}

void VKRenderingSystemNS::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer);
}

bool VKRenderingSystemNS::createImageView(VKTextureDataComponent* VKTDC)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = VKTDC->m_image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VKTDC->m_VkTextureDataDesc.internalFormat;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(VKRenderingSystemComponent::get().m_device, &viewInfo, nullptr, &VKTDC->m_imageView) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create VkImageView!");
		return false;
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_VERBOSE, "VKRenderingSystem: VkImageView " + InnoUtility::pointerToString(VKTDC->m_imageView) + " is initialized.");
	return true;
}

bool VKRenderingSystemNS::recordCommand(VKRenderPassComponent* VKRPC, unsigned int commandBufferIndex, const std::function<void()>& commands)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	if (vkBeginCommandBuffer(VKRPC->m_commandBuffers[commandBufferIndex], &beginInfo) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to begin recording command buffer!");
		return false;
	}

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = VKRPC->m_renderPass;
	renderPassInfo.framebuffer = VKRPC->m_framebuffers[commandBufferIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = VKRPC->scissor.extent;

	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(VKRPC->m_commandBuffers[commandBufferIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(VKRPC->m_commandBuffers[commandBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, VKRPC->m_pipeline);

	commands();

	vkCmdEndRenderPass(VKRPC->m_commandBuffers[commandBufferIndex]);

	if (vkEndCommandBuffer(VKRPC->m_commandBuffers[commandBufferIndex]) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to record command!");
		return false;
	}

	return true;
}

bool VKRenderingSystemNS::recordDescriptorBinding(VKRenderPassComponent* VKRPC, unsigned int commandBufferIndex)
{
	vkCmdBindDescriptorSets(VKRPC->m_commandBuffers[commandBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, VKRPC->m_pipelineLayout, 0, 1, &VKRPC->descriptorSet, 0, nullptr);
	return true;
}

bool VKRenderingSystemNS::recordDrawCall(VKRenderPassComponent* VKRPC, unsigned int commandBufferIndex, VKMeshDataComponent * VKMDC)
{
	VkBuffer vertexBuffers[] = { VKMDC->m_VBO };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(VKRPC->m_commandBuffers[commandBufferIndex], 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(VKRPC->m_commandBuffers[commandBufferIndex], VKMDC->m_IBO, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(VKRPC->m_commandBuffers[commandBufferIndex], static_cast<uint32_t>(VKMDC->m_indicesSize), 1, 0, 0, 0);
	return true;
}

bool VKRenderingSystemNS::waitForFence(VKRenderPassComponent* VKRPC)
{
	vkWaitForFences(VKRenderingSystemComponent::get().m_device,
		1,
		&VKRPC->m_inFlightFences[VKRPC->m_currentFrame],
		VK_TRUE,
		std::numeric_limits<uint64_t>::max()
	);
	return true;
}

bool VKRenderingSystemNS::summitCommand(VKRenderPassComponent* VKRPC, unsigned int commandBufferIndex)
{
	// submit the draw command buffer with a rendering finished signal semaphore
	// command buffer
	VKRPC->submitInfo.commandBufferCount = 1;
	VKRPC->submitInfo.pCommandBuffers = &VKRPC->m_commandBuffers[commandBufferIndex];

	// signal semaphore
	VKRPC->submitInfo.signalSemaphoreCount = 1;
	VKRPC->submitInfo.pSignalSemaphores = &VKRPC->m_renderFinishedSemaphores[VKRPC->m_currentFrame];

	vkResetFences(VKRenderingSystemComponent::get().m_device, 1, &VKRPC->m_inFlightFences[VKRPC->m_currentFrame]);

	// submit to queue
	if (vkQueueSubmit(VKRenderingSystemComponent::get().m_graphicsQueue, 1, &VKRPC->submitInfo, VKRPC->m_inFlightFences[VKRPC->m_currentFrame]) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to submit command buffer!");
		return false;
	}

	return true;
}

bool VKRenderingSystemNS::updateUBOImpl(VkDeviceMemory& UBOMemory, size_t size, const void* UBOValue)
{
	void* data;
	vkMapMemory(VKRenderingSystemComponent::get().m_device, UBOMemory, 0, size, 0, &data);
	std::memcpy(data, UBOValue, size);
	vkUnmapMemory(VKRenderingSystemComponent::get().m_device, UBOMemory);

	return true;
}

VKShaderProgramComponent * VKRenderingSystemNS::addVKShaderProgramComponent(const EntityID & rhs)
{
	auto l_rawPtr = g_pCoreSystem->getMemorySystem()->spawnObject(m_VKShaderProgramComponentPool, sizeof(VKShaderProgramComponent));
	auto l_VKSPC = new(l_rawPtr)VKShaderProgramComponent();
	return l_VKSPC;
}

bool VKRenderingSystemNS::initializeVKShaderProgramComponent(VKShaderProgramComponent * rhs, const ShaderFilePaths & shaderFilePaths)
{
	bool l_result = true;
	if (shaderFilePaths.m_VSPath != "")
	{
		l_result &= createShaderModule(rhs->m_vertexShaderModule, shaderFilePaths.m_VSPath);
		rhs->m_vertexShaderStageCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		rhs->m_vertexShaderStageCInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		rhs->m_vertexShaderStageCInfo.module = rhs->m_vertexShaderModule;
		rhs->m_vertexShaderStageCInfo.pName = "main";

		rhs->m_vertexInputStateCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		rhs->m_vertexInputStateCInfo.vertexBindingDescriptionCount = 1;
		rhs->m_vertexInputStateCInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_vertexAttributeDescriptions.size());
		rhs->m_vertexInputStateCInfo.pVertexBindingDescriptions = &m_vertexBindingDescription;
		rhs->m_vertexInputStateCInfo.pVertexAttributeDescriptions = m_vertexAttributeDescriptions.data();
	}
	if (shaderFilePaths.m_FSPath != "")
	{
		l_result &= createShaderModule(rhs->m_fragmentShaderModule, shaderFilePaths.m_FSPath);
		rhs->m_fragmentShaderStageCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		rhs->m_fragmentShaderStageCInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		rhs->m_fragmentShaderStageCInfo.module = rhs->m_fragmentShaderModule;
		rhs->m_fragmentShaderStageCInfo.pName = "main";
	}

	return l_result;
}

bool VKRenderingSystemNS::createShaderModule(VkShaderModule& vkShaderModule, const std::string& shaderFilePath)
{
	auto l_binData = g_pCoreSystem->getFileSystem()->loadBinaryFile(m_shaderRelativePath + shaderFilePath);

	VkShaderModuleCreateInfo l_createInfo = {};
	l_createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	l_createInfo.codeSize = l_binData.size();
	l_createInfo.pCode = reinterpret_cast<const uint32_t*>(l_binData.data());

	if (vkCreateShaderModule(VKRenderingSystemComponent::get().m_device, &l_createInfo, nullptr, &vkShaderModule) != VK_SUCCESS)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create VkShaderModule for: " + shaderFilePath + "!");
		return false;
	}

	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: innoShader: " + shaderFilePath + " has been loaded.");
	return true;
}

bool VKRenderingSystemNS::generateUBO(VkBuffer& UBO, VkDeviceSize UBOSize, VkDeviceMemory& UBOMemory)
{
	auto l_result = createBuffer(UBOSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		UBO,
		UBOMemory);

	if (!l_result)
	{
		g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_ERROR, "VKRenderingSystem: Failed to create UBO!");
		return false;
	}
	g_pCoreSystem->getLogSystem()->printLog(LogType::INNO_DEV_SUCCESS, "VKRenderingSystem: UBO has been created.");
	return true;
}

VkTextureDataDesc VKRenderingSystemNS::getVKTextureDataDesc(const TextureDataDesc & textureDataDesc)
{
	VkTextureDataDesc l_result;

	l_result.textureWrapMethod = getTextureWrapMethod(textureDataDesc.wrapMethod);
	l_result.minFilterParam = getTextureFilterParam(textureDataDesc.minFilterMethod);
	l_result.magFilterParam = getTextureFilterParam(textureDataDesc.magFilterMethod);
	l_result.internalFormat = getTextureInternalFormat(textureDataDesc.colorComponentsFormat);
	l_result.imageSize = textureDataDesc.width * textureDataDesc.height * ((unsigned int)textureDataDesc.pixelDataFormat + 1);

	return l_result;
}

VkSamplerAddressMode VKRenderingSystemNS::getTextureWrapMethod(TextureWrapMethod rhs)
{
	VkSamplerAddressMode result;

	switch (rhs)
	{
	case TextureWrapMethod::CLAMP_TO_EDGE:
		result = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		break;
	case TextureWrapMethod::REPEAT:
		result = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	case TextureWrapMethod::CLAMP_TO_BORDER:
		result = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		break;
	default:
		break;
	}

	return result;
}

VkSamplerMipmapMode VKRenderingSystemNS::getTextureFilterParam(TextureFilterMethod rhs)
{
	VkSamplerMipmapMode result;

	switch (rhs)
	{
	case TextureFilterMethod::NEAREST:
		result = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
		break;
	case TextureFilterMethod::LINEAR:
		result = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	case TextureFilterMethod::LINEAR_MIPMAP_LINEAR:
		result = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_MAX_ENUM; // ????????
		break;
	default:
		break;
	}

	return result;
}

VkFormat VKRenderingSystemNS::getTextureInternalFormat(TextureColorComponentsFormat rhs)
{
	VkFormat result = VK_FORMAT_R8_UNORM;

	// @TODO: with pixel format together
	switch (rhs)
	{
	case TextureColorComponentsFormat::RED:
		result = VkFormat::VK_FORMAT_R8_UNORM;
		break;
	case TextureColorComponentsFormat::RG:
		result = VkFormat::VK_FORMAT_R8G8_UNORM;
		break;
	case TextureColorComponentsFormat::RGB:
		result = VkFormat::VK_FORMAT_R8G8B8_UNORM;
		break;
	case TextureColorComponentsFormat::RGBA:
		result = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
		break;
	case TextureColorComponentsFormat::R8:
		break;
	case TextureColorComponentsFormat::RG8:
		break;
	case TextureColorComponentsFormat::RGB8:
		break;
	case TextureColorComponentsFormat::RGBA8:
		break;
	case TextureColorComponentsFormat::R8I:
		break;
	case TextureColorComponentsFormat::RG8I:
		break;
	case TextureColorComponentsFormat::RGB8I:
		break;
	case TextureColorComponentsFormat::RGBA8I:
		break;
	case TextureColorComponentsFormat::R8UI:
		break;
	case TextureColorComponentsFormat::RG8UI:
		break;
	case TextureColorComponentsFormat::RGB8UI:
		break;
	case TextureColorComponentsFormat::RGBA8UI:
		break;
	case TextureColorComponentsFormat::R16:
		break;
	case TextureColorComponentsFormat::RG16:
		break;
	case TextureColorComponentsFormat::RGB16:
		break;
	case TextureColorComponentsFormat::RGBA16:
		break;
	case TextureColorComponentsFormat::R16I:
		break;
	case TextureColorComponentsFormat::RG16I:
		break;
	case TextureColorComponentsFormat::RGB16I:
		break;
	case TextureColorComponentsFormat::RGBA16I:
		break;
	case TextureColorComponentsFormat::R16UI:
		break;
	case TextureColorComponentsFormat::RG16UI:
		break;
	case TextureColorComponentsFormat::RGB16UI:
		break;
	case TextureColorComponentsFormat::RGBA16UI:
		break;
	case TextureColorComponentsFormat::R16F:
		result = VkFormat::VK_FORMAT_R16_SFLOAT;
		break;
	case TextureColorComponentsFormat::RG16F:
		result = VkFormat::VK_FORMAT_R16G16_SFLOAT;
		break;
	case TextureColorComponentsFormat::RGB16F:
		result = VkFormat::VK_FORMAT_R16G16B16_SFLOAT;
		break;
	case TextureColorComponentsFormat::RGBA16F:
		result = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
		break;
	case TextureColorComponentsFormat::R32I:
		break;
	case TextureColorComponentsFormat::RG32I:
		break;
	case TextureColorComponentsFormat::RGB32I:
		break;
	case TextureColorComponentsFormat::RGBA32I:
		break;
	case TextureColorComponentsFormat::R32UI:
		break;
	case TextureColorComponentsFormat::RG32UI:
		break;
	case TextureColorComponentsFormat::RGB32UI:
		break;
	case TextureColorComponentsFormat::RGBA32UI:
		break;
	case TextureColorComponentsFormat::R32F:
		break;
	case TextureColorComponentsFormat::RG32F:
		break;
	case TextureColorComponentsFormat::RGB32F:
		break;
	case TextureColorComponentsFormat::RGBA32F:
		break;
	case TextureColorComponentsFormat::SRGB:
		break;
	case TextureColorComponentsFormat::SRGBA:
		break;
	case TextureColorComponentsFormat::SRGB8:
		break;
	case TextureColorComponentsFormat::SRGBA8:
		break;
	case TextureColorComponentsFormat::DEPTH_COMPONENT:
		break;
	case TextureColorComponentsFormat::BGR:
		result = VkFormat::VK_FORMAT_B8G8R8_UNORM;
		break;
	case TextureColorComponentsFormat::BGRA:
		result = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
		break;
	default:
		break;
	}

	return result;
}