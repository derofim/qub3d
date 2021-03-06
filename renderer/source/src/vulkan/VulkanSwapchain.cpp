#include <viking/vulkan/VulkanSwapchain.hpp>

using namespace viking::vulkan;

viking::vulkan::VulkanSwapchain::VulkanSwapchain(VulkanDevice* device, IWindow* window, IVulkanSurface* surface)
{
	m_device = device;
	m_window = window;
	m_surface = surface;
	getSwapChainSupport(m_swap_chain_config);
	m_surface_format = chooseSwapSurfaceFormat(m_swap_chain_config.formats);
	m_present_mode = chooseSwapPresentMode(m_swap_chain_config.present_modes);
	initSwapchain();
	createCommandBuffer();
	createSemaphores();
	rebuildCommandBuffers();
}

VulkanSwapchain::~VulkanSwapchain()
{
	destroySemaphores();
	deinitSwapchain();
}

void VulkanSwapchain::rebuildSwapchain()
{
	vkDeviceWaitIdle(m_device->GetVulkanDevice());
	deinitSwapchain();
	initSwapchain();
	rebuildCommandBuffers();
}

void VulkanSwapchain::rebuildCommandBuffers()
{
	VkCommandBufferBeginInfo begin_info = VulkanInitializers::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	std::array<VkClearValue, 2> clear_values;
	clear_values[0].color = { 0.2f, 0.2f, 0.2f, 1.0f };
	clear_values[1].depthStencil = { 1.0f, 0 };
	VkRenderPassBeginInfo render_pass_info = VulkanInitializers::renderPassBeginInfo(m_render_pass, m_extent, clear_values);

	for (uint32_t i = 0; i < m_command_buffers.size(); i++)
	{
		vkResetCommandBuffer(
			m_command_buffers[i],
			0
		);
		render_pass_info.framebuffer = m_swap_chain_framebuffers[i];

		bool success = VulkanInitializers::validate(vkBeginCommandBuffer(
			m_command_buffers[i],
			&begin_info
		));

		vkCmdBeginRenderPass(
			m_command_buffers[i],
			&render_pass_info,
			VK_SUBPASS_CONTENTS_INLINE
		);

		// Pipeline and model code
		/*
		
		*/


		vkCmdEndRenderPass(
			m_command_buffers[i]
		);

        success = VulkanInitializers::validate(vkEndCommandBuffer(
			m_command_buffers[i]
		));
	}
}

void VulkanSwapchain::render()
{
	VkResult check = vkAcquireNextImageKHR(
		m_device->GetVulkanDevice(),
		m_swap_chain,
		UINT32_MAX,
		m_image_available_semaphore,
		VK_NULL_HANDLE,
		&m_active_swapchain_image
	);
	if (check == VK_ERROR_OUT_OF_DATE_KHR)
	{
		rebuildSwapchain();
		return;
	}

	vkQueueWaitIdle(
		*m_device->GetPresentQueue()
	);
	VkSemaphore wait_semaphores[] = { m_image_available_semaphore };
	VkSemaphore signal_semaphores[] = { m_render_finished_semaphore };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo sumbit_info = {};
	sumbit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sumbit_info.waitSemaphoreCount = 1;
	sumbit_info.pWaitSemaphores = wait_semaphores;
	sumbit_info.pWaitDstStageMask = wait_stages;
	sumbit_info.commandBufferCount = 1;
	sumbit_info.pCommandBuffers = &m_command_buffers[m_active_swapchain_image];
	sumbit_info.signalSemaphoreCount = 1;
	sumbit_info.pSignalSemaphores = signal_semaphores;

	bool success = VulkanInitializers::validate(vkQueueSubmit(
		*m_device->GetGraphicsQueue(),
		1,
		&sumbit_info,
		VK_NULL_HANDLE
	));
    success = VulkanInitializers::validate(vkQueueWaitIdle(*m_device->GetGraphicsQueue()));

	VkResult present_result = VkResult::VK_RESULT_MAX_ENUM;
	VkSwapchainKHR swap_chains[] = { m_swap_chain };
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &m_active_swapchain_image;
	present_info.pResults = nullptr;

	vkQueuePresentKHR(
		*m_device->GetPresentQueue(),
		&present_info
	);
}

void VulkanSwapchain::initSwapchain()
{
	createSwapchain();
	createSwapchainImages();
	createRenderPass();
	createDepthImage();
	createFrameBuffer();
}

void VulkanSwapchain::deinitSwapchain()
{
	destroyFrameBuffer();
	destroyRenderPass();
	destroySwapchainImages();
	destroySwapchain();
}

void VulkanSwapchain::createSwapchain()
{
	m_extent = chooseSwapExtent(m_swap_chain_config.capabilities);

	uint32_t image_count = m_swap_chain_config.capabilities.minImageCount + 1;

	if (m_swap_chain_config.capabilities.maxImageCount > 0 && image_count > m_swap_chain_config.capabilities.maxImageCount)
		image_count = m_swap_chain_config.capabilities.maxImageCount;

	VulkanQueueFamilyIndices* indices = m_device->GetPhysicalDevice().getQueueFamilies();
	VkSwapchainCreateInfoKHR create_info = VulkanInitializers::swapchainCreateInfoKHR(
		m_surface_format,
		m_extent,
		m_present_mode,
		image_count,
		m_surface->GetSurface(),
		*indices,
		m_swap_chain_config
	);

	bool success = VulkanInitializers::validate(vkCreateSwapchainKHR(
		m_device->GetVulkanDevice(),
		&create_info,
		nullptr,
		&m_swap_chain
	));

    success = VulkanInitializers::validate(vkGetSwapchainImagesKHR(
		m_device->GetVulkanDevice(),
		m_swap_chain,
		&image_count,
		nullptr
	));

	m_swap_chain_images.resize(image_count);


    success = VulkanInitializers::validate(vkGetSwapchainImagesKHR(
		m_device->GetVulkanDevice(),
		m_swap_chain,
		&image_count,
		m_swap_chain_images.data()
	));
}

void VulkanSwapchain::destroySwapchain()
{
	vkDestroySwapchainKHR(
		m_device->GetVulkanDevice(),
		m_swap_chain,
		nullptr
	);
}

void VulkanSwapchain::createSwapchainImages()
{
	m_swap_chain_image_views.resize(m_swap_chain_images.size());
	int i = 0;
	for (auto swapchain_image : m_swap_chain_images)
	{
		VulkanCommon::createImageView(m_device,swapchain_image, m_surface_format.format, VK_IMAGE_ASPECT_COLOR_BIT, m_swap_chain_image_views[i]);
		i++;
	}
}

void VulkanSwapchain::destroySwapchainImages()
{
	for (uint32_t i = 0; i < m_swap_chain_image_views.size(); i++)
	{
		vkDestroyImageView(
			m_device->GetVulkanDevice(),
			m_swap_chain_image_views[i],
			nullptr
		);
	}

	m_swap_chain_image_views.clear();
}

void VulkanSwapchain::createRenderPass()
{
	std::vector<VkAttachmentDescription> attachments = {
		VulkanInitializers::attachmentDescription(m_surface_format.format, VK_ATTACHMENT_STORE_OP_STORE,VK_IMAGE_LAYOUT_PRESENT_SRC_KHR),
		VulkanInitializers::attachmentDescription(getDepthImageFormat(), VK_ATTACHMENT_STORE_OP_DONT_CARE,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	};

	VkAttachmentReference color_attachment_refrence = VulkanInitializers::attachmentReference(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0);
	VkAttachmentReference depth_attachment_refrence = VulkanInitializers::attachmentReference(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
	VkSubpassDescription subpass = VulkanInitializers::subpassDescription(color_attachment_refrence, depth_attachment_refrence);
	VkSubpassDependency subpass_dependency = VulkanInitializers::subpassDependency();
	VkRenderPassCreateInfo render_pass_info = VulkanInitializers::renderPassCreateInfo(attachments, subpass, subpass_dependency);

	bool success = VulkanInitializers::validate(vkCreateRenderPass(
		m_device->GetVulkanDevice(),
		&render_pass_info,
		nullptr,
		&m_render_pass
	));

}

void VulkanSwapchain::destroyRenderPass()
{
	vkDestroyRenderPass(
		m_device->GetVulkanDevice(),
		m_render_pass,
		nullptr);
}

void VulkanSwapchain::createDepthImage()
{
	m_depth_image_format = getDepthImageFormat();
	VulkanCommon::createImage(m_device, m_extent, m_depth_image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depth_image, m_depth_image_memory);
	VulkanCommon::createImageView(m_device, m_depth_image, m_depth_image_format, VK_IMAGE_ASPECT_DEPTH_BIT, m_depth_image_view);

	VulkanCommon::transitionImageLayout(m_device, m_depth_image, m_depth_image_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void VulkanSwapchain::createFrameBuffer()
{
	m_swap_chain_framebuffers.resize(m_swap_chain_image_views.size());
	for (uint32_t i = 0; i < m_swap_chain_image_views.size(); i++)
	{
		// What 
		std::vector<VkImageView> attachments = {
			m_swap_chain_image_views[i],
			m_depth_image_view
		};
		// Frame buffer create info
		VkFramebufferCreateInfo framebuffer_info = VulkanInitializers::framebufferCreateInfo(m_extent, attachments, m_render_pass);

		bool success = VulkanInitializers::validate(vkCreateFramebuffer(
			m_device->GetVulkanDevice(),
			&framebuffer_info,
			nullptr,
			&m_swap_chain_framebuffers[i]
		));
	}
}

void VulkanSwapchain::destroyFrameBuffer()
{
	for (auto framebuffer : m_swap_chain_framebuffers)
	{
		vkDestroyFramebuffer(
			m_device->GetVulkanDevice(),
			framebuffer,
			nullptr
		);
	}
}

void VulkanSwapchain::createCommandBuffer()
{
	m_command_buffers.resize(m_swap_chain_framebuffers.size());
	VkCommandBufferAllocateInfo alloc_info = VulkanInitializers::commandBufferAllocateInfo(
		*m_device->GetGraphicsCommandPool(),
		static_cast<uint32_t>(m_command_buffers.size())
	);
	bool sucsess = VulkanInitializers::validate(vkAllocateCommandBuffers(
		m_device->GetVulkanDevice(),
		&alloc_info,
		m_command_buffers.data()
	));
}

void VulkanSwapchain::createSemaphores()
{
	VkSemaphoreCreateInfo semaphore_info = VulkanInitializers::semaphoreCreateInfo();

	bool sucsess = VulkanInitializers::validate(vkCreateSemaphore(m_device->GetVulkanDevice(), &semaphore_info, nullptr, &m_image_available_semaphore));
	sucsess = VulkanInitializers::validate(vkCreateSemaphore(m_device->GetVulkanDevice(), &semaphore_info, nullptr, &m_render_finished_semaphore));
}

void VulkanSwapchain::destroySemaphores()
{
	vkDestroySemaphore(m_device->GetVulkanDevice(), m_image_available_semaphore, nullptr);
	vkDestroySemaphore(m_device->GetVulkanDevice(), m_render_finished_semaphore, nullptr);
}

void VulkanSwapchain::getSwapChainSupport(VulkanSwapChainConfiguration & support)
{
	bool success = VulkanInitializers::validate(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		m_device->GetPhysicalDevice().GetPhysicalDevice(),
		m_surface->GetSurface(),
		&support.capabilities
	));

	uint32_t format_count = 0;
    success = VulkanInitializers::validate(vkGetPhysicalDeviceSurfaceFormatsKHR(
		m_device->GetPhysicalDevice().GetPhysicalDevice(),
		m_surface->GetSurface(),
		&format_count, 
		nullptr
	));

	if (format_count == 0) return;

	support.formats.resize(format_count);
    success = VulkanInitializers::validate(vkGetPhysicalDeviceSurfaceFormatsKHR(
		m_device->GetPhysicalDevice().GetPhysicalDevice(),
		m_surface->GetSurface(),
		&format_count,
		support.formats.data()
	));

	uint32_t present_mode_count;
    success = VulkanInitializers::validate(vkGetPhysicalDeviceSurfacePresentModesKHR(
		m_device->GetPhysicalDevice().GetPhysicalDevice(),
		m_surface->GetSurface(),
		&present_mode_count,
		nullptr
	));

	if (present_mode_count == 0) return;
	support.present_modes.resize(present_mode_count);

    success = VulkanInitializers::validate(vkGetPhysicalDeviceSurfacePresentModesKHR(
		m_device->GetPhysicalDevice().GetPhysicalDevice(),
		m_surface->GetSurface(),
		&present_mode_count,
		support.present_modes.data()
	));
}

VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
	if (available_formats.size() == 1 && available_formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& format : available_formats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}
	if (available_formats.size() > 0)return available_formats[0];

	return{ VK_FORMAT_UNDEFINED };
}

VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> available_present_modes)
{
	for (const auto& available_present_mode : available_present_modes)
	{
		if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return available_present_mode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		m_window->GetSize(width, height);
		VkExtent2D extent = { (uint32_t)width, (uint32_t)height };
		if (extent.width > capabilities.maxImageExtent.width)extent.width = capabilities.maxImageExtent.width;
		if (extent.width < capabilities.minImageExtent.width)extent.width = capabilities.minImageExtent.width;
		if (extent.height > capabilities.maxImageExtent.width)extent.height = capabilities.maxImageExtent.height;
		if (extent.height < capabilities.minImageExtent.width)extent.height = capabilities.minImageExtent.height;
		return extent;
	}
}

VkFormat VulkanSwapchain::getDepthImageFormat()
{
	return selectSutableFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkFormat VulkanSwapchain::selectSutableFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_device->GetPhysicalDevice().GetPhysicalDevice(), format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}
	return VK_FORMAT_UNDEFINED;
}
