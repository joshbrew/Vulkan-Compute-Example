#include "Render.h"


#include <cstdlib>
#include <assert.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <set>

//Windows only for message boxes
#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef _WIN64
#include <Windows.h>
#endif


//Constructor
Render::Render()
{
	InitPlatform();
	_setupLayersAndExtensions();
	_setupDebug();
	_initInstance();
	_initDebug();
	_initDevice();
}

//Destructor
Render::~Render()
{

	_deInitDevice();
	_deInitDebug();
	_deInitInstance();
	DeInitPlatform();

}

Window * Render::OpenWindow(uint32_t size_x, uint32_t size_y, std::string name)
{
	_window = new Window(this, size_x, size_y, name);
	return		_window;
}

bool Render::Run()
{
	if (nullptr != _window) {
		return _window->Update();
	}
	return true;
}

const VkInstance Render::GetVulkanInstance() const
{
	return _instance;
}

const VkPhysicalDevice Render::GetVulkanPhysicalDevice() const
{
	return _gpu;
}

const VkDevice Render::GetVulkanDevice() const
{
	return _device;
}

const VkQueue Render::GetVulkanQueue() const
{
	return _queue;
}

const uint32_t Render::GetVulkanGraphicsQueueFamilyIndex() const
{
	return _graphics_family_index;
}

const VkPhysicalDeviceProperties & Render::GetVulkanPhysicalDeviceProperties() const
{
	return _gpu_properties;
}

const VkPhysicalDeviceMemoryProperties & Render::GetVulkanPhysicalDeviceMemoryProperties() const
{
	return _gpu_memory_properties;
}


const VkQueue Render::GetVulkanPresentQueue() const {

	return _present_queue;

}

const uint32_t Render::GetVulkanGraphicsPresentFamilyIndex() const {

return _present_family_index;

}


void Render::_setupLayersAndExtensions()
{
	_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	AddRequiredPlatformInstanceExtensions(&_instance_extensions);

	_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}


void Render::_initInstance() {

	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pEngineName = "Rootin Tootin Skiddlybop Machine 9000";
	app_info.apiVersion = VK_MAKE_VERSION(1,0,61); // This SDK ver 1.0.61.1
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1); // Ver a.b.c
	app_info.pApplicationName = "Vulkan Test API";

	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pNext = NULL;
	instance_create_info.flags = 0;
	instance_create_info.pApplicationInfo = &app_info;
	instance_create_info.enabledLayerCount = _instance_layers.size();
	instance_create_info.ppEnabledLayerNames = _instance_layers.data();
	instance_create_info.enabledExtensionCount = _instance_extensions.size();
	instance_create_info.ppEnabledExtensionNames = _instance_extensions.data();


	ErrorCheck(vkCreateInstance(&instance_create_info, nullptr, &_instance));

}

void Render::_deInitInstance() {

	vkDestroyInstance(_instance, nullptr);
	_instance = VK_NULL_HANDLE;
}

Render::QueueFamilyIndices Render::_findQueueFamilies(VkPhysicalDevice device, Window *w) 
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	bool found = false;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			found = true;
			indices.graphicsFamily = i;
			
		}
		if (!found) {

			assert(0 && "Vulkan ERROR: Graphics queue family not found.");
			std::exit(-1); //Simple exit, modify for real applications
		}
		
		if (w != nullptr) {
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, w->GetVulkanSurface(), &presentSupport);

			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamily = i;
			}
		};
		
		if (indices.isComplete()) {
			break;
		}
		i++;
	}

	return indices;
}

void Render::_initDevice() {

	{ //Operators
		uint32_t gpuCount = 0;
		vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr);
		std::vector<VkPhysicalDevice> gpuList(gpuCount);
		vkEnumeratePhysicalDevices(_instance, &gpuCount, gpuList.data());
		_gpu = gpuList[0]; //Gets first GPU on device list for now (assuming it's the most powerful) 
		vkGetPhysicalDeviceProperties(_gpu, &_gpu_properties);
		vkGetPhysicalDeviceMemoryProperties(_gpu, &_gpu_memory_properties);
	}

	//Queue families support different types of operations (graphics,compute, and/or transfer), they must be selected accordingly.
	QueueFamilyIndices _indices = _findQueueFamilies(_gpu, _window);
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos; // Storage for multiple queues
	//std::set<int> uniqueQueueFamilies = { _indices.graphicsFamily };
	std::set<int> uniqueQueueFamilies = { _indices.graphicsFamily,_indices.presentFamily };
	_graphics_family_index = _indices.graphicsFamily;
	_present_family_index = _indices.presentFamily;

	/*
	//What kind of queue families does our GPU have? We want to use the graphics queue
	{
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &familyCount, nullptr); //Get # of queue families on this GPU
		std::vector<VkQueueFamilyProperties> familyPropertyList(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &familyCount, familyPropertyList.data()); // Get the queue family names on the GPU

		bool found = false;
		for (uint32_t i = 0; i < familyCount; ++i) {

			if (familyPropertyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) //We want this family to support graphics
			{
				found = true;
				_graphics_family_index = i;
			}
		}
		if (!found) {

			assert(0 && "Vulkan ERROR: Graphics queue family not found.");
			std::exit(-1); //Simple exit, modify for real applications
		}

	}
	*/
	/*
	{ //Instance Debug Layers // Activate to view available debug layers

		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> layerPropertyList(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layerPropertyList.data()); // How many layers installed in system?


		std::cout << "Instance Layers : \n";
		for (auto &i : layerPropertyList) {
			std::cout << "   " << i.layerName << "\t\t\ | " << i.description << std::endl;
		}
		std::cout << std::endl;
		system("PAUSE");

	}
	*/


	float queue_priorities[]
	{ 1.0f };
	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo device_queue_create_info = {};
		device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_create_info.queueFamilyIndex = queueFamily;		// Set queue family
		device_queue_create_info.queueCount = 1;						// Only one queue in this example
		device_queue_create_info.pQueuePriorities = queue_priorities;	// List of floats
		queue_create_infos.push_back(device_queue_create_info);
	}
	

	VkPhysicalDeviceFeatures device_features = {};
	device_features.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.pEnabledFeatures = &device_features;
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(_device_extensions.size());
	device_create_info.ppEnabledExtensionNames =_device_extensions.data();

	ErrorCheck(vkCreateDevice(_gpu, &device_create_info, nullptr, &_device));

	vkGetDeviceQueue(_device, _graphics_family_index, 0, &_queue);
	/*
	if(_window != nullptr)
		vkGetDeviceQueue(_device, _present_family_index, 0, &_present_queue);
		*/
}

void Render::_deInitDevice() {

	vkDestroyDevice(_device, nullptr);
	_device = VK_NULL_HANDLE;

}











//Debug stuffs

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugReportFlagsEXT flags,		 // Type of warning
	VkDebugReportObjectTypeEXT obj_type, // Type of object creating the error (ex: command pool, framebuffer, sampler, etc)
	uint64_t src_obj,					 // Pointer to the object throwing the flag
	size_t location,					 // Which # layer the error happened??
	int32_t msg_code,					 // The error code or extension
	const char * layer_prefix,			 // Which layer called back
	const char * msg,					 // Readable string
	void * user_data					 // Function for user data? Don't worry about it..
) {

	std::stringstream stream;
		stream << "VkDebug: ";
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		stream << "WARNING: ";
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		stream << "PERFORMANCE: ";
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		stream << "ERROR: ";
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		stream << "DEBUG: ";
	}
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		stream << "INFORMATION: ";
	}
	/*
	if (flags & VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT) {
		stream << "FLAGGED: ";
	} 
	*/
	stream << "@[" << layer_prefix << "]: ";

	stream << msg << std::endl;

	std::cout << stream.str();

#ifdef _WIN32
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		MessageBox(NULL, stream.str().c_str(), "Vulkan Error!", 0);
	}
#endif
#ifdef _WIN64
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		MessageBox(NULL, stream.str().c_str(), "Vulkan Error!", 0);
	}
#endif

	return false;
}

void Render::_setupDebug() {

	//Define Debug callback info
	debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_callback_create_info.pfnCallback = VulkanDebugCallback;
	debug_callback_create_info.flags =
		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		0;

	_instance_layers.push_back("VK_LAYER_LUNARG_standard_validation"); //Enables standard debug validation layers
	
	_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); //Need this to create debug report callback
}

PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = nullptr;

void Render::_initDebug() {

	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
	if (nullptr == fvkCreateDebugReportCallbackEXT || nullptr == fvkDestroyDebugReportCallbackEXT)
	{
		assert(1 && "Vulkan ERROR: Can't fetch debug function pointers.");
		std::exit(-1);
	}


	fvkCreateDebugReportCallbackEXT(_instance, &debug_callback_create_info, nullptr, &_debug_report);

}

void Render::_deInitDebug() {

	fvkDestroyDebugReportCallbackEXT(_instance, _debug_report, nullptr);
	_debug_report = VK_NULL_HANDLE;
	system("PAUSE");

}