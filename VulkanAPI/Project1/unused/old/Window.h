#ifndef WINDOW_H
#define WINDOW_H

//I used Niko Kauppi's tutorial and the official tutorial.

#include "Platform.h"
#include "Render.h"
#include <string>

class Render;

class Window
{
public:

	Window(Render * renderer, uint32_t size_x, uint32_t size_y, std::string name);
	~Window();

	void Close();
	bool Update();

	void								_cleanup();

	void								BeginRender();
	void								EndRender(std::vector<VkSemaphore> wait_semaphores);

	VkRenderPass						GetVulkanRenderPass();
	VkFramebuffer						GetVulkanActiveFramebuffer();
	VkSurfaceKHR						GetVulkanSurface();
	VkExtent2D							GetVulkanSurfaceSize();

private:

#if USE_FRAMEWORK_GLFW
	void								_initGLFWWindow();
	void								_deInitGLFWWindow();
	void								_updateGLFWWindow();
	void								_initGLFWSurface();
	static void							_onWindowResized(GLFWwindow* window, int width, int height);
#endif


	void								_initSurface(); // Surfaces represent the display area
	void								_deInitSurface();

	void								_initSwapchain(); // Creates virtual framebuffers to prevent stuttering (similar to vsync)
	void								_deInitSwapchain();

	void								_initSwapchainImages(); // Defines image properties on swapchain
	void								_deInitSwapchainImages();

	void								_initDepthStencilImage(); // Enables depth Buffering for 3D
	void								_deInitDepthStencilImage();

	void								_initRenderPass(); // Attaches images to be added to framebuffer
	void								_deInitRenderPass();

	void								_initDescriptorSetLayout();
	//void								_deInitDescriptorSetLayout();

	void								_initGraphicsPipeline();
	//void								_deInitGraphicsPipeline();

	void								_initFramebuffers(); // Framebuffers are the VRAM chunk containing the bitmap to be sent to monitor.  
	void								_deInitFramebuffers();
	
	void								_initCommandPool();
	//void								_deInitCommandPool();

	void								_initTextureImage();
	//void								_deInitTextureImage();

	void								_initTextureImageView();
	//void								_deInitTextureImageView();

	void								_initTextureSampler();
	//void								_deInitTextureSampler();

	void								_initVertexBuffer();
	//void								_deInitVertexBuffer();

	void								_initIndexBuffer();
	//void								_deInitIndexBuffer();
	
	void								_initUniformBuffer();
	void								_updateUniformBuffer();
	//void								_deInitUniformBuffer();

	void								_initDescriptorPool();
	//void								_deInitDescriptorPool();

	void								_initDescriptorSet();
	//void								_deInitDescriptorSet();
	

	void								_initCommandBuffers();
	//void								_deInitCommandBuffers();

	void								_initSemaphores();
	//void								_deInitSemaphores();

	void								_initSynchronizations(); // Creates a fence which synchronizes CPU with GPU.
	void								_deInitSynchronizations();

	void								_reRenderSwapchain();

	void								_drawFrame();
	
	void								_cleanupSwapChain();
	

	VkShaderModule						_initShaderModule(const std::vector<char>& code);
	VkImageView							_initImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void								_initImage(
													uint32_t width,
													uint32_t height,
													VkFormat format,
													VkImageTiling tiling,
													VkImageUsageFlags usage,
													VkMemoryPropertyFlags properties,
													VkImage& image,
													VkDeviceMemory& imageMemory
													);

	void								_transitionImageLayout(
													VkImage image, 
													VkFormat format, 
													VkImageLayout oldLayout, 
													VkImageLayout newLayout
													);

	Render						*		_renderer							= nullptr;

	VkExtent2D							_chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkSurfaceFormatKHR					_chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkSurfaceKHR						_surface							= VK_NULL_HANDLE;

	VkPresentModeKHR 					_chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

	VkSwapchainKHR						_swapchain							= VK_NULL_HANDLE;

	VkRenderPass						_render_pass						= VK_NULL_HANDLE;
	VkDescriptorSetLayout				_descriptor_set_layout			    = VK_NULL_HANDLE;
	VkPipelineLayout					_pipeline_layout					= VK_NULL_HANDLE;
	VkPipeline							_graphics_pipeline					= VK_NULL_HANDLE;
	
	VkExtent2D							_swapchain_extent;

	VkBuffer							_vertex_buffer;
	VkDeviceMemory						_vertex_buffer_memory;
	VkBuffer							_index_buffer;
	VkDeviceMemory						_index_buffer_memory;

	VkBuffer							_uniform_buffer;
	VkDeviceMemory						_uniform_buffer_memory;

	VkDescriptorPool					_descriptor_pool;
	VkDescriptorSet						_descriptor_set;


	void								_createBuffer(
													VkDeviceSize size,
													VkBufferUsageFlags usage,
													VkMemoryPropertyFlags properties,
													VkBuffer& buffer, VkDeviceMemory&
													bufferMemory
													);

	void								_copyBufferToImage(
													VkBuffer buffer,
													VkImage image,
													uint32_t width,
													uint32_t height
													);

	void								_copyBuffer(
													VkBuffer srcBuffer, 
													VkBuffer dstBuffer, 
													VkDeviceSize size
													);

	uint32_t							_findMemoryType(
													uint32_t typeFilter, 
													VkMemoryPropertyFlags properties
													);

	VkCommandBuffer						_beginSingleTimeCommands();

	void								_endSingleTimeCommands(VkCommandBuffer commandBuffer);

	VkCommandPool						_command_pool;
	std::vector<VkCommandBuffer>		_command_buffers;
	

	uint32_t							_surface_size_x						= 512; //Default
	uint32_t							_surface_size_y						= 512;
	std::string							_window_name;
	uint32_t							_swapchain_image_count				= 2;
	uint32_t							_active_swapchain_image_id			= UINT32_MAX;

	VkSemaphore							_image_available_semaphore;
	VkSemaphore							_render_finished_semaphore;

	VkFence								_swapchain_image_available			= VK_NULL_HANDLE;

	std::vector<VkImage>				_swapchain_images;
	std::vector<VkImageView>			_swapchain_image_views;
	std::vector<VkFramebuffer>			_framebuffers;

	bool								_hasStencilComponent(VkFormat format);

	VkImage								_depth_stencil_image				= VK_NULL_HANDLE;
	VkDeviceMemory						_depth_stencil_image_memory			= VK_NULL_HANDLE;
	VkImageView							_depth_stencil_image_view			= VK_NULL_HANDLE;

	VkImage								_texture_image						= VK_NULL_HANDLE;
	VkDeviceMemory						_texture_image_memory				= VK_NULL_HANDLE;
	VkImageView							_texture_image_view					= VK_NULL_HANDLE;
	VkSampler							_texture_sampler;

	VkSurfaceFormatKHR					_surface_format						= {};
	VkSurfaceCapabilitiesKHR			_surface_capabilities				= {};

	VkFormat							_depth_stencil_format				= VK_FORMAT_UNDEFINED;
	bool								_stencil_available					= false;

	bool								_window_should_run					= true;

	GLFWwindow					*		_glfw_window						= nullptr;
};



#endif