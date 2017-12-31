#include "Window.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb-master/stb_image.h>

#include <array>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <array>

#pragma region Put this stuff in their own files.

static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}



struct Vertex2D {
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex2D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex2D, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex2D, color);
		return attributeDescriptions;

	}

};

struct Vertex3D {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex3D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3D, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3D, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3D, texCoord);

		return attributeDescriptions;
	}
};

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};



//Sample Triangle
const std::vector<Vertex2D> twoDTriangle = {
	{ { 0.0f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
	{ { -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } }
};

const std::vector<Vertex3D> twoSquares = {
	{ { -0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 0.5f, 0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
	{ { -0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },

	{ { -0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 0.5f, 0.5f, -0.5f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
	{ { -0.5f, 0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } }
};

const std::vector<uint16_t> squaresIndices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};


#pragma endregion












Window::Window(Render *renderer, uint32_t size_x, uint32_t size_y, std::string name)
{




	_renderer = renderer;
	_surface_size_x = size_x;
	_surface_size_y = size_y;
	_window_name = name;

	_initGLFWWindow();
	_initSurface();
	_initSwapchain();
	_initSwapchainImages();
	_initRenderPass();
	_initDescriptorSetLayout();
	_initGraphicsPipeline();
	_initCommandPool();
	_initDepthStencilImage();
	_initFramebuffers();
	_initTextureImage();
	_initTextureImageView();
	_initTextureSampler();
	_initVertexBuffer();
	_initIndexBuffer();
	_initUniformBuffer();
	_initDescriptorPool();
	_initDescriptorSet();
	_initCommandBuffers();
	_initSemaphores();
	_initSynchronizations();
}

Window::~Window()
{
	vkQueueWaitIdle(_renderer->GetVulkanQueue());
	_deInitSynchronizations();

	_cleanup();

	_deInitGLFWWindow();
}

void Window::_reRenderSwapchain() {

	vkDeviceWaitIdle(_renderer->GetVulkanDevice());

	_initSwapchain();
	_initSwapchainImages();
	_initDepthStencilImage();
	_initRenderPass();
	_initGraphicsPipeline();
	_initFramebuffers();
	_initCommandBuffers();

}


void Window::Close()
{
	_window_should_run = false;
}

bool Window::Update()
{
	_updateGLFWWindow();

	_updateUniformBuffer();
	_drawFrame();

	vkDeviceWaitIdle(_renderer->GetVulkanDevice());

	return _window_should_run;
}




void Window::BeginRender()
{
	ErrorCheck(vkAcquireNextImageKHR(
		_renderer->GetVulkanDevice(),
		_swapchain,
		UINT64_MAX,
		VK_NULL_HANDLE,
		_swapchain_image_available,
		&_active_swapchain_image_id));
	ErrorCheck(vkWaitForFences(_renderer->GetVulkanDevice(), 1, &_swapchain_image_available, VK_TRUE, UINT64_MAX));
	ErrorCheck(vkResetFences(_renderer->GetVulkanDevice(), 1, &_swapchain_image_available));
	ErrorCheck(vkQueueWaitIdle(_renderer->GetVulkanQueue()));
}

void Window::EndRender(std::vector<VkSemaphore> wait_semaphores)
{
	VkResult present_result = VkResult::VK_RESULT_MAX_ENUM;

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = wait_semaphores.size();
	present_info.pWaitSemaphores = wait_semaphores.data();
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &_swapchain;
	present_info.pImageIndices = &_active_swapchain_image_id;
	present_info.pResults = &present_result;

	ErrorCheck(vkQueuePresentKHR(_renderer->GetVulkanQueue(), &present_info));
	ErrorCheck(present_result);
}




VkRenderPass Window::GetVulkanRenderPass()
{
	return _render_pass;
}

VkFramebuffer Window::GetVulkanActiveFramebuffer()
{
	return _framebuffers[_active_swapchain_image_id];
}

VkExtent2D Window::GetVulkanSurfaceSize()
{
	return{ _surface_size_x, _surface_size_y };
}

VkSurfaceKHR Window::GetVulkanSurface()
{
	return _surface;
}








VkExtent2D Window::_chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetWindowSize(_glfw_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		_surface_size_x = actualExtent.width;
		_surface_size_y = actualExtent.height;

		return actualExtent;
	}
}

VkSurfaceFormatKHR Window::_chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }; 
	}  // B8G8R8A8 stores BGRA channels in that order with 8-bit uints for 32 bits per pixel

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}




void Window::_initSurface()
{
	_initGLFWSurface();

	auto gpu = _renderer->GetVulkanPhysicalDevice();

	VkBool32 WSI_supported = false;
	ErrorCheck(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, _renderer->GetVulkanGraphicsQueueFamilyIndex(), _surface, &WSI_supported));
	if (!WSI_supported) {
		assert(0 && "WSI not supported");
		std::exit(-1);
	}

	ErrorCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, _surface, &_surface_capabilities));
	_chooseSwapExtent(_surface_capabilities);
	/*
	if (_surface_capabilities.currentExtent.width < UINT32_MAX) {
		_surface_size_x = _surface_capabilities.currentExtent.width;
		_surface_size_y = _surface_capabilities.currentExtent.height;
	}
	*/
	{
		uint32_t format_count = 0;
		ErrorCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, _surface, &format_count, nullptr));
		if (format_count == 0) {
			assert(0 && "Surface formats missing.");
			std::exit(-1);
		}

		std::vector<VkSurfaceFormatKHR> formats(format_count); // Sets color depth
		ErrorCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, _surface, &format_count, formats.data()));
		_chooseSwapSurfaceFormat(formats);
		/*
		if (formats[0].format == VK_FORMAT_UNDEFINED) {
			_surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
			_surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; // Indicates SRGB color space is supported
		}
		else {
			_surface_format = formats[0];
		}
		*/
	}
}

void Window::_deInitSurface()
{
	vkDestroySurfaceKHR(_renderer->GetVulkanInstance(), _surface, nullptr);
}






VkPresentModeKHR Window::_chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {

	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
	/*
	FIFO mode: Display takes image from front of the queue on display refresh while the next render is placed at the back of the queue.
	The program will wait if the queue is full. This is like vertical sync in games.
	*/
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

void Window::_initSwapchain()
{

	if (_swapchain_image_count < _surface_capabilities.minImageCount + 1) _swapchain_image_count = _surface_capabilities.minImageCount + 1;
	if (_surface_capabilities.maxImageCount > 0) {
		if (_swapchain_image_count > _surface_capabilities.maxImageCount) _swapchain_image_count = _surface_capabilities.maxImageCount;
	}

	// Set conditions for showing images to the screen
	//std::vector<VkPresentModeKHR> present_mode;
	//_chooseSwapPresentMode(present_mode);

	
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

	{
		uint32_t present_mode_count = 0;
		ErrorCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(_renderer->GetVulkanPhysicalDevice(), _surface, &present_mode_count, nullptr));
		std::vector<VkPresentModeKHR> present_mode_list(present_mode_count);
		ErrorCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(_renderer->GetVulkanPhysicalDevice(), _surface, &present_mode_count, present_mode_list.data()));
		for (auto m : present_mode_list) {
			if (m == VK_PRESENT_MODE_MAILBOX_KHR) present_mode = m;
		}
	}
	

	// Define swapchain properties
	VkSwapchainCreateInfoKHR swapchain_create_info{};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = _surface;									 // Which surface the swapchain will present images to
	swapchain_create_info.minImageCount = _swapchain_image_count;				 // The min number of presentable images the app needs.
	swapchain_create_info.imageFormat = _surface_format.format;					 // Valid format for swapchains on this surface
	swapchain_create_info.imageColorSpace = _surface_format.colorSpace;			 // Valid colorspace for this surface
	swapchain_create_info.imageExtent.width = _surface_size_x;					 // Size (in pixels) of swapchain
	swapchain_create_info.imageExtent.height = _surface_size_y;
	_swapchain_extent = swapchain_create_info.imageExtent;
	swapchain_create_info.imageArrayLayers = 1;									 // Number of views. 1 for non-stereoscopic views.
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;		 // How the app will use swapchain's presentable images

	Render::QueueFamilyIndices indices = Render::_findQueueFamilies(_renderer->GetVulkanPhysicalDevice(), this);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily };	 //, (uint32_t)indices.presentFamily };

	//if (indices.graphicsFamily != indices.presentFamily) {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Sharing mode on for swapchain images
		swapchain_create_info.queueFamilyIndexCount = 2;					 // #queue families with access to swapchain images
		swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;		 // Points to array of queue family indices
	//}
	//else {
	//	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//}
	/*
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;			 // Sharing mode for swapchain images (off in this case)
	swapchain_create_info.queueFamilyIndexCount = 0;							 // Number of queue families that have access to swapchain images (only if sharing mode is concurrent)
	swapchain_create_info.pQueueFamilyIndices = nullptr;						 // Points to array of queue family indices
	*/
	swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;  //
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = present_mode;
	swapchain_create_info.clipped = VK_TRUE;
	//swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

	ErrorCheck(vkCreateSwapchainKHR(_renderer->GetVulkanDevice(), &swapchain_create_info, nullptr, &_swapchain));

	ErrorCheck(vkGetSwapchainImagesKHR(_renderer->GetVulkanDevice(), _swapchain, &_swapchain_image_count, nullptr));
}

void Window::_deInitSwapchain()
{
	vkDestroySwapchainKHR(_renderer->GetVulkanDevice(), _swapchain, nullptr);
}




void Window::_initSwapchainImages()
{
	_swapchain_images.resize(_swapchain_image_count);
	_swapchain_image_views.resize(_swapchain_image_count);

	ErrorCheck(vkGetSwapchainImagesKHR(_renderer->GetVulkanDevice(), _swapchain, &_swapchain_image_count, _swapchain_images.data()));

	for (uint32_t i = 0; i < _swapchain_image_count; ++i) {
		VkImageViewCreateInfo image_view_create_info{}; // This lets us specify certain traits 
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = _swapchain_images[i];
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = _surface_format.format;
		image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;

		ErrorCheck(vkCreateImageView(_renderer->GetVulkanDevice(), &image_view_create_info, nullptr, &_swapchain_image_views[i]));
	}
}

void Window::_deInitSwapchainImages()
{
	for (auto view : _swapchain_image_views) {
		vkDestroyImageView(_renderer->GetVulkanDevice(), view, nullptr);
	}
}


void Window::_initDepthStencilImage()
{
	std::vector<VkFormat> try_formats{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM
	};
	for (auto f : try_formats) {
		VkFormatProperties format_properties{};
		vkGetPhysicalDeviceFormatProperties(_renderer->GetVulkanPhysicalDevice(), f, &format_properties);
		if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			_depth_stencil_format = f;
			break;
		}
	}
	if (_depth_stencil_format == VK_FORMAT_UNDEFINED) {
		assert(0 && "Depth stencil format not selected.");
		std::exit(-1);
	}
	if ((_depth_stencil_format == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
		(_depth_stencil_format == VK_FORMAT_D24_UNORM_S8_UINT) ||
		(_depth_stencil_format == VK_FORMAT_D16_UNORM_S8_UINT) ||
		(_depth_stencil_format == VK_FORMAT_S8_UINT)) {
		_stencil_available = true;
	}

	_initImage(
		_swapchain_extent.width,
		_swapchain_extent.height,
		_depth_stencil_format, 
		VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		_depth_stencil_image, 
		_depth_stencil_image_memory);

	_depth_stencil_image_view = _initImageView(
												_depth_stencil_image, 
												_depth_stencil_format, 
												VK_IMAGE_ASPECT_DEPTH_BIT);

	_transitionImageLayout(
		_depth_stencil_image, 
		_depth_stencil_format, 
		VK_IMAGE_LAYOUT_UNDEFINED, 
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

/*
void Window::_initDepthStencilImage()
{
	{
		std::vector<VkFormat> try_formats{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM
		};
		for (auto f : try_formats) {
			VkFormatProperties format_properties{};
			vkGetPhysicalDeviceFormatProperties(_renderer->GetVulkanPhysicalDevice(), f, &format_properties);
			if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				_depth_stencil_format = f;
				break;
			}
		}
		if (_depth_stencil_format == VK_FORMAT_UNDEFINED) {
			assert(0 && "Depth stencil format not selected.");
			std::exit(-1);
		}
		if ((_depth_stencil_format == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
			(_depth_stencil_format == VK_FORMAT_D24_UNORM_S8_UINT) ||
			(_depth_stencil_format == VK_FORMAT_D16_UNORM_S8_UINT) ||
			(_depth_stencil_format == VK_FORMAT_S8_UINT)) {
			_stencil_available = true;
		}
	}

	VkImageCreateInfo image_create_info{};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.flags = 0;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = _depth_stencil_format;
	image_create_info.extent.width = _surface_size_x;
	image_create_info.extent.height = _surface_size_y;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; // Allows depth masking
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
	image_create_info.pQueueFamilyIndices = nullptr;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	ErrorCheck(vkCreateImage(_renderer->GetVulkanDevice(), &image_create_info, nullptr, &_depth_stencil_image));

	VkMemoryRequirements image_memory_requirements{};
	vkGetImageMemoryRequirements(_renderer->GetVulkanDevice(), _depth_stencil_image, &image_memory_requirements);

	uint32_t memory_index = FindMemoryTypeIndex(&_renderer->GetVulkanPhysicalDeviceMemoryProperties(), &image_memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = image_memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = memory_index;

	ErrorCheck(vkAllocateMemory(_renderer->GetVulkanDevice(), &memory_allocate_info, nullptr, &_depth_stencil_image_memory));
	ErrorCheck(vkBindImageMemory(_renderer->GetVulkanDevice(), _depth_stencil_image, _depth_stencil_image_memory, 0));

	VkImageViewCreateInfo image_view_create_info{};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.image = _depth_stencil_image;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.format = _depth_stencil_format;
	image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (_stencil_available ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = 1;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = 1;

	ErrorCheck(vkCreateImageView(_renderer->GetVulkanDevice(), &image_view_create_info, nullptr, &_depth_stencil_image_view));
}
*/

void Window::_deInitDepthStencilImage()
{
	vkDestroyImageView(_renderer->GetVulkanDevice(), _depth_stencil_image_view, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), _depth_stencil_image_memory, nullptr);
	vkDestroyImage(_renderer->GetVulkanDevice(), _depth_stencil_image, nullptr);
}




void Window::_initRenderPass() // Attaches formatted images to GPU to be rendered
{
	std::array<VkAttachmentDescription, 2> attachments{};
	attachments[0].flags = 0;
	attachments[0].format = _depth_stencil_format;								   // Format for image being presented to surface.
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;							       // Samples per-pixel of stored images
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;						   // Specifies how color and depth components are treated at beginning of subpass where first used.
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;					   // Specifies how   "	   "	"		 "		 "	  "	   "	end of subpass where last used.
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;				   // Specifies how stencil components are treated at beginning of subpass where first used.
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;				   // Specifies how	   "		"		"	"		"	end of subpass where last used.
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					   // Layout of attachment image subresource when render pass begins.
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Which layout of attachment image subresource to be transitioned to at end of render pass.

	attachments[1].flags = 0;
	attachments[1].format = _surface_format.format; // Formatting for surface.
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


	VkAttachmentReference sub_pass_0_depth_stencil_attachment{};
	sub_pass_0_depth_stencil_attachment.attachment = 0;
	sub_pass_0_depth_stencil_attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Enables depth masking

	std::array<VkAttachmentReference, 1> sub_pass_0_color_attachments{};
	sub_pass_0_color_attachments[0].attachment = 1;
	sub_pass_0_color_attachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Enables color buffering

	std::array<VkSubpassDescription, 1> sub_passes{};
	sub_passes[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;			  // Use compute or graphics pipeline? Corresponding command pool must also support these operations.
	sub_passes[0].colorAttachmentCount = sub_pass_0_color_attachments.size();
	sub_passes[0].pColorAttachments = sub_pass_0_color_attachments.data();		  // layout(location=0) out vec4 FinalColor;
	sub_passes[0].pDepthStencilAttachment = &sub_pass_0_depth_stencil_attachment;

	std::array<VkSubpassDependency, 1> dependencies{};
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_create_info{};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = attachments.size();
	render_pass_create_info.pAttachments = attachments.data();
	render_pass_create_info.subpassCount = sub_passes.size();
	render_pass_create_info.pSubpasses = sub_passes.data();
	render_pass_create_info.pDependencies = dependencies.data();

	ErrorCheck(vkCreateRenderPass(_renderer->GetVulkanDevice(), &render_pass_create_info, nullptr, &_render_pass));
}

void Window::_deInitRenderPass()
{
	vkDestroyRenderPass(_renderer->GetVulkanDevice(), _render_pass, nullptr);
}


void Window::_initDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	ErrorCheck(vkCreateDescriptorSetLayout(_renderer->GetVulkanDevice(), &layoutInfo, nullptr, &_descriptor_set_layout));
}






VkShaderModule	Window::_initShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());


	VkShaderModule shaderModule;

	//Error Check
	ErrorCheck(vkCreateShaderModule(_renderer->GetVulkanDevice(), &createInfo, nullptr, &shaderModule));

	return shaderModule;
}


void Window::_initGraphicsPipeline()
{

	auto vShader = readFile("shaders/vert.txt");
	auto fShader = readFile("shaders/frag.txt");

	//auto cShader = readFile("compute.txt"); // Compute shading.
	//auto gShader = readFile("geometry.txt"); // Geometry shading
	//VkShaderModule cShaderModule = createShaderModule(cShader);
	//VkShaderModule gShaderModule = createShaderModule(gShader);


	VkShaderModule vShaderModule = _initShaderModule(vShader);
	VkShaderModule fShaderModule = _initShaderModule(fShader);


	VkPipelineShaderStageCreateInfo vShaderStageInfo = {};
	vShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vShaderStageInfo.module = vShaderModule;
	vShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fShaderStageInfo = {};
	fShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fShaderStageInfo.module = fShaderModule;
	fShaderStageInfo.pName = "main";

	/*
	VkPipelineShaderStageCreateInfo cShaderStageInfo = {};
	cShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	cShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	//cShaderStageInfo.module = cShaderModule;
	cShaderStageInfo.pName = "Primary";

	VkPipelineShaderStageCreateInfo gShaderStageInfo = {};
	gShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	gShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
	//gShaderStageInfo.module = gShaderModule;
	gShaderStageInfo.pName = "Primary";
	*/

	VkPipelineShaderStageCreateInfo shaderStages[] = { vShaderStageInfo, fShaderStageInfo }; // , cShaderStageInfo, gShaderStageInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	
	auto bindingDescription = Vertex2D::getBindingDescription();
	auto attributeDescriptions = Vertex2D::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)_surface_size_x;
	viewport.height = (float)_surface_size_y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = _swapchain_extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // For comparing transparent objects.
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;


	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	ErrorCheck(vkCreatePipelineLayout(_renderer->GetVulkanDevice(), &pipelineLayoutInfo, nullptr, &_pipeline_layout));

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.layout = _pipeline_layout;
	pipelineInfo.renderPass = _render_pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	ErrorCheck(vkCreateGraphicsPipelines(_renderer->GetVulkanDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphics_pipeline));

	vkDestroyShaderModule(_renderer->GetVulkanDevice(), fShaderModule, nullptr);
	vkDestroyShaderModule(_renderer->GetVulkanDevice(), vShaderModule, nullptr);
}




void Window::_initFramebuffers()
{
	_framebuffers.resize(_swapchain_image_count);
	for (uint32_t i = 0; i < _swapchain_image_count; ++i) {
		std::array<VkImageView, 2> attachments{};
		attachments[0] = _depth_stencil_image_view; 
		attachments[1] = _swapchain_image_views[i];

		VkFramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass = _render_pass;
		framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebuffer_create_info.pAttachments = attachments.data();
		framebuffer_create_info.width = _surface_size_x;
		framebuffer_create_info.height = _surface_size_y;
		framebuffer_create_info.layers = 1;

		ErrorCheck(vkCreateFramebuffer(_renderer->GetVulkanDevice(), &framebuffer_create_info, nullptr, &_framebuffers[i]));
	}
}

void Window::_initCommandPool() {

	//Command Pool allocates memory for command buffers.
	VkCommandPool command_pool = VK_NULL_HANDLE;
	VkCommandPoolCreateInfo pool_create_info{};
	pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_create_info.queueFamilyIndex = _renderer->GetVulkanGraphicsQueueFamilyIndex();
	vkCreateCommandPool(_renderer->GetVulkanDevice(), &pool_create_info, nullptr, &command_pool);

	if (vkCreateCommandPool(_renderer->GetVulkanDevice(), &pool_create_info, nullptr, &_command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}


void Window::_deInitFramebuffers()
{
	for (auto f : _framebuffers) {
		vkDestroyFramebuffer(_renderer->GetVulkanDevice(), f, nullptr);
	}
}



void Window::_initSemaphores() {
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(_renderer->GetVulkanDevice(), &semaphoreInfo, nullptr, &_image_available_semaphore) != VK_SUCCESS ||
		vkCreateSemaphore(_renderer->GetVulkanDevice(), &semaphoreInfo, nullptr, &_render_finished_semaphore) != VK_SUCCESS) {

		throw std::runtime_error("failed to create semaphores!");
	}
}






void Window::_initTextureImage() {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	_createBuffer(
		imageSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, 
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(_renderer->GetVulkanDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(_renderer->GetVulkanDevice(), stagingBufferMemory);

	stbi_image_free(pixels);

	_initImage(
		texWidth, 
		texHeight, 
		VK_FORMAT_R8G8B8A8_UNORM, 
		VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		_texture_image, 
		_texture_image_memory
	);

	_transitionImageLayout(
		_texture_image, 
		VK_FORMAT_R8G8B8A8_UNORM, 
		VK_IMAGE_LAYOUT_UNDEFINED, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	_copyBufferToImage(
		stagingBuffer, 
		_texture_image, 
		static_cast<uint32_t>(texWidth), 
		static_cast<uint32_t>(texHeight)
	);

	_transitionImageLayout(
		_texture_image, 
		VK_FORMAT_R8G8B8A8_UNORM, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	vkDestroyBuffer(_renderer->GetVulkanDevice(), stagingBuffer, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), stagingBufferMemory, nullptr);
}

void Window::_initTextureImageView() {
	_texture_image_view = _initImageView(_texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Window::_initTextureSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(_renderer->GetVulkanDevice(), &samplerInfo, nullptr, &_texture_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

VkImageView Window::_initImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(_renderer->GetVulkanDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

void Window::_initImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(_renderer->GetVulkanDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(_renderer->GetVulkanDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = _findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(_renderer->GetVulkanDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(_renderer->GetVulkanDevice(), image, imageMemory, 0);
}

bool Window::_hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Window::_transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();


	//This will pipeline the image memory to the desired stage
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (_hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

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
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	_endSingleTimeCommands(commandBuffer);
}

void Window::_copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

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

	_endSingleTimeCommands(commandBuffer);
}

void Window::_initVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(twoSquares[0]) * twoSquares.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	_createBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, 
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(_renderer->GetVulkanDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, twoSquares.data(), (size_t)bufferSize);
	vkUnmapMemory(_renderer->GetVulkanDevice(), stagingBufferMemory);

	_createBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		_vertex_buffer, 
		_vertex_buffer_memory
	);

	_copyBuffer(stagingBuffer, _vertex_buffer, bufferSize);

	vkDestroyBuffer(_renderer->GetVulkanDevice(), stagingBuffer, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), stagingBufferMemory, nullptr);
}

void Window::_initIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(squaresIndices[0]) * squaresIndices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	_createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(_renderer->GetVulkanDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, squaresIndices.data(), (size_t)bufferSize);
	vkUnmapMemory(_renderer->GetVulkanDevice(), stagingBufferMemory);

	_createBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		_index_buffer, 
		_index_buffer_memory
	);

	_copyBuffer(stagingBuffer, _index_buffer, bufferSize);

	vkDestroyBuffer(_renderer->GetVulkanDevice(), stagingBuffer, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), stagingBufferMemory, nullptr);
}

void Window::_initUniformBuffer() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	_createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		_uniform_buffer, 
		_uniform_buffer_memory);
}

void Window::_initDescriptorPool() {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(_renderer->GetVulkanDevice(), &poolInfo, nullptr, &_descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void Window::_initDescriptorSet() {
	VkDescriptorSetLayout layouts[] = { _descriptor_set_layout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptor_pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(_renderer->GetVulkanDevice(), &allocInfo, &_descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = _uniform_buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformBufferObject);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = _texture_image_view;
	imageInfo.sampler = _texture_sampler;

	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = _descriptor_set;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = _descriptor_set;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(_renderer->GetVulkanDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Window::_createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(_renderer->GetVulkanDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(_renderer->GetVulkanDevice(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = _findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(_renderer->GetVulkanDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(_renderer->GetVulkanDevice(), buffer, bufferMemory, 0);
}

VkCommandBuffer Window::_beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_renderer->GetVulkanDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Window::_endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(_renderer->GetVulkanQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_renderer->GetVulkanQueue());

	vkFreeCommandBuffers(_renderer->GetVulkanDevice(), _command_pool, 1, &commandBuffer);
}

void Window::_copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	_endSingleTimeCommands(commandBuffer);
}

uint32_t Window::_findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties = _renderer->GetVulkanPhysicalDeviceMemoryProperties();
	vkGetPhysicalDeviceMemoryProperties(_renderer->GetVulkanPhysicalDevice(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void Window::_initCommandBuffers() {
	_command_buffers.resize(_framebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)_command_buffers.size();

	if (vkAllocateCommandBuffers(_renderer->GetVulkanDevice(), &allocInfo, _command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < _command_buffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(_command_buffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = _render_pass;
		renderPassInfo.framebuffer = _framebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = _swapchain_extent;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(_command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);

		VkBuffer vertexBuffers[] = { _vertex_buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_command_buffers[i], 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(_command_buffers[i], _index_buffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_descriptor_set, 0, nullptr);

		vkCmdDrawIndexed(_command_buffers[i], static_cast<uint32_t>(squaresIndices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(_command_buffers[i]);

		if (vkEndCommandBuffer(_command_buffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

/* //For Triangle
void Window::_initCommandBuffers() {
_command_buffers.resize(_framebuffers.size());

VkCommandBufferAllocateInfo allocInfo = {};
allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
allocInfo.commandPool = _command_pool;
allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
allocInfo.commandBufferCount = (uint32_t)_command_buffers.size();

if (vkAllocateCommandBuffers(_renderer->GetVulkanDevice(), &allocInfo, _command_buffers.data()) != VK_SUCCESS) {
throw std::runtime_error("failed to allocate command buffers!");
}

for (size_t i = 0; i < _command_buffers.size(); i++) {
VkCommandBufferBeginInfo beginInfo = {};
beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

vkBeginCommandBuffer(_command_buffers[i], &beginInfo);

VkRenderPassBeginInfo renderPassInfo = {};
renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
renderPassInfo.renderPass = _render_pass;
renderPassInfo.framebuffer = _framebuffers[i];
renderPassInfo.renderArea.offset = { 0, 0 };
renderPassInfo.renderArea.extent = _swapchain_extent;

VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
renderPassInfo.clearValueCount = 1;
renderPassInfo.pClearValues = &clearColor;

vkCmdBeginRenderPass(_command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

vkCmdBindPipeline(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);

vkCmdDraw(_command_buffers[i], 3, 1, 0, 0);

vkCmdEndRenderPass(_command_buffers[i]);

if (vkEndCommandBuffer(_command_buffers[i]) != VK_SUCCESS) {
throw std::runtime_error("failed to record command buffer!");
}
}
}
*/


void Window::_updateUniformBuffer() {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), _swapchain_extent.width / (float)_swapchain_extent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(_renderer->GetVulkanDevice(), _uniform_buffer_memory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(_renderer->GetVulkanDevice(), _uniform_buffer_memory);
}







void Window::_drawFrame() {
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		_renderer->GetVulkanDevice(),
		_swapchain, 
		std::numeric_limits<uint64_t>::max(), 
		_image_available_semaphore, VK_NULL_HANDLE, 
		&imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		_reRenderSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { _image_available_semaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_command_buffers[imageIndex];

	VkSemaphore signalSemaphores[] = { _render_finished_semaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(_renderer->GetVulkanQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { _swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	/*
	result = vkQueuePresentKHR(_renderer->GetVulkanPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		_reRenderSwapchain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
	*/
	vkQueueWaitIdle(_renderer->GetVulkanQueue());
	//	vkQueueWaitIdle(_renderer->GetVulkanPresentQueue());
}






void Window::_initSynchronizations()
{
	VkFenceCreateInfo fence_create_info{};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(_renderer->GetVulkanDevice(), &fence_create_info, nullptr, &_swapchain_image_available);
}

void Window::_deInitSynchronizations()
{
	vkDestroyFence(_renderer->GetVulkanDevice(), _swapchain_image_available, nullptr);
}


void Window::_cleanupSwapChain() 
{
	_deInitDepthStencilImage();

	_deInitFramebuffers();

	vkFreeCommandBuffers(
		_renderer->GetVulkanDevice(), 
		_command_pool, 
		static_cast<uint32_t>(_command_buffers.size()), 
		_command_buffers.data());

	vkDestroyPipeline(_renderer->GetVulkanDevice(), _graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(_renderer->GetVulkanDevice(), _pipeline_layout, nullptr);
	
	_deInitRenderPass();

	_deInitSwapchainImages();

	_deInitSwapchain();

}

void Window::_cleanup()
{

	_cleanupSwapChain();

	vkDestroySampler(_renderer->GetVulkanDevice(), _texture_sampler, nullptr);
	vkDestroyImageView(_renderer->GetVulkanDevice(), _texture_image_view, nullptr);
	
	vkDestroyImage(_renderer->GetVulkanDevice(), _texture_image, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), _texture_image_memory, nullptr);

	vkDestroyDescriptorPool(_renderer->GetVulkanDevice(), _descriptor_pool, nullptr);

	vkDestroyDescriptorSetLayout(_renderer->GetVulkanDevice(), _descriptor_set_layout, nullptr);
	vkDestroyBuffer(_renderer->GetVulkanDevice(), _uniform_buffer, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), _uniform_buffer_memory, nullptr);

	vkDestroyBuffer(_renderer->GetVulkanDevice(), _index_buffer, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), _index_buffer_memory, nullptr);

	vkDestroyBuffer(_renderer->GetVulkanDevice(), _vertex_buffer, nullptr);
	vkFreeMemory(_renderer->GetVulkanDevice(), _vertex_buffer_memory, nullptr);


	vkDestroySemaphore(_renderer->GetVulkanDevice(), _render_finished_semaphore, nullptr);
	vkDestroySemaphore(_renderer->GetVulkanDevice(), _image_available_semaphore, nullptr);

	vkDestroyCommandPool(_renderer->GetVulkanDevice(), _command_pool, nullptr);

	_deInitSurface();
}







#if USE_FRAMEWORK_GLFW

// GLFW Window Functions
void Window::_initGLFWWindow()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	_glfw_window = glfwCreateWindow(_surface_size_x, _surface_size_y, _window_name.c_str(), nullptr, nullptr);
	glfwGetFramebufferSize(_glfw_window, (int*)&_surface_size_x, (int*)&_surface_size_y);
}

void Window::_deInitGLFWWindow()
{
	glfwDestroyWindow(_glfw_window);
}

void Window::_updateGLFWWindow()
{
	glfwPollEvents();

	if (glfwWindowShouldClose(_glfw_window)) {
		Close();
	}
}

void Window::_initGLFWSurface()
{
	// glfwGetRequiredInstanceExtensions; // This is set in platform.h

	if (VK_SUCCESS != glfwCreateWindowSurface(_renderer->GetVulkanInstance(), _glfw_window, nullptr, &_surface)) {
		glfwTerminate();
		assert(0 && "GLFW could not create the window surface.");
		return;
	}
	
	}

void Window::_onWindowResized(GLFWwindow* window, int width, int height) {
	if (width == 0 || height == 0) return;

	Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	app->_reRenderSwapchain();
}


#endif