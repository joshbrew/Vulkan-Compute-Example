#ifndef RENDER_H
#define RENDER_H

#include "Platform.h"
#include "Window.h"

#include <vector>
#include <string>
#include <stdint.h>

#include <assert.h>

class Window;



class Render
{
public:

	Render();
	~Render();
	

	Window								*	OpenWindow(uint32_t size_x, uint32_t size_y, std::string name);

	bool									Run();

	const VkInstance						 GetVulkanInstance()	const; // The connection between your application and the Vulkan library
	const VkPhysicalDevice				  	 GetVulkanPhysicalDevice() const; // Handle for queried physical device properties/features
	const VkDevice						 	 GetVulkanDevice() const; // "Opaque handle to a device object" or the logical device 
	const VkQueue							 GetVulkanQueue() const;  // Instruction call list for GPU
	const VkQueue							 GetVulkanPresentQueue() const;  // Present instruction call list for GPU

	const uint32_t							 GetVulkanGraphicsQueueFamilyIndex() const; // 
	const uint32_t							 GetVulkanGraphicsPresentFamilyIndex() const; // 

	const VkPhysicalDeviceProperties &		 GetVulkanPhysicalDeviceProperties() const; // 
	const VkPhysicalDeviceMemoryProperties & GetVulkanPhysicalDeviceMemoryProperties() const; // 

	struct QueueFamilyIndices {
		int graphicsFamily = -1;
		int presentFamily = -1;

		bool isComplete() {
			return graphicsFamily >= 0; // && presentFamily >= 0;
		}
	};

	static QueueFamilyIndices						 _findQueueFamilies(VkPhysicalDevice device, Window *w);

private:

	void _setupLayersAndExtensions();

	void _initInstance();
	void _deInitInstance();

	void _initDevice();
	void _deInitDevice();

	void _setupDebug();
	void _initDebug();
	void _deInitDebug();

	uint32_t _graphics_family_index = 0; 
	uint32_t _present_family_index = 0;

	VkInstance _instance = VK_NULL_HANDLE;
	VkPhysicalDevice _gpu = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;
	VkQueue _queue = VK_NULL_HANDLE;
	VkQueue _present_queue = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties _gpu_properties = {};
	VkPhysicalDeviceMemoryProperties _gpu_memory_properties = {};

	std::vector<const char*> _instance_layers;
	std::vector<const char*> _instance_extensions;
	std::vector<const char*> _device_extensions;

	VkDebugReportCallbackEXT _debug_report = VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT debug_callback_create_info = {};

	Window * _window = nullptr;

	



};

/* Here's all the pipeline processes from top to bottom. Assemblers and Operations are fixed functions
				All shaders are programmable. Tesselation, Geometry, and Compute shaders are optional. 
Init ------------------------------------------------------------------*
  |                                                                    |
Draw < -------------------------- Indirect Buffer Binding --- >    Dispatch
  |                                                                    |
Input Assembler < --------------- Index Buffer Binding          Compute Assembler       
  |							|										   |
  |							< --- Vertex Buffer Binding			Compute Shader (i.e. CUDA)
  |																	   |
Vertex Shader < -------------- > *									   |
|  |                             | < --- Push Constants ------------ > |
| Tesselation Assembler          |									   |
|  |                             | ..........Descriptor Sets.......... | 
| Tess. Control Shader < ----- > * < --- Sampled Image ------------- > |
|  |							 |									   |
| Tess. Primitive Generator		 | < --- Uniform Texel Buffer ------ > |
|  |							 |									   |
| Tess. Evaluation Shader < -- > * < --- Uniform Buffer ------------ > |
|  |							 |									   |
|--|							 | < - > Storage Image < ----------- > |
| Geometry Assembler			 |									   |
|  |							 | < - > Storage Texel Buffer < ---- > |
|  Geometry Shader < --------- > * < - > Storage Buffer < ---------- > *
*--|							 | ...................................
   |							 |
Primitive Assembler				 |
   |							 |
Rasterization					 |
   |							 |
Per-Fragment Operations < ---- > *
   |							 |      
Fragment Assembler				 | 
   |							 |
Fragment Shader < ------------ > * -------------------------------*
   |					|		...... Framebuffer ......   	  |
   |					< ----------- Input Attachment			  |
   |															  |
Post-Fragment Operations < ---------- Depth/Stencil Attachment < -*
   |
Color/Blending Operations < --------- Color Attachment
								.........................
*/


#endif