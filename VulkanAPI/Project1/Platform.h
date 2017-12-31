#ifndef PLATFORM_H
#define PLATFORM_H
//I used Niko Kauppi's tutorial


//vulkan-1.lib required to be linked from VulkanSDK

// You will need to add  GLFWs "glfw3.lib" file into the project, this task is up to you.
// GLFW version 3.2 or newer is required.
#define USE_FRAMEWORK_GLFW 1
#define GLFW_INCLUDE_VULKAN
//#include <vulkan/vulkan.h> //GLFW should include this automatically with the above declaration

#include <GLFW3.2.1/include/GLFW/glfw3.h>// Extended library to simplify drawing to screen

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp> // Essential linear algebra operations (matrices, vectors, linear algebra (dot,distance,normalize,etc))

#include <glm/gtc/matrix_transform.hpp> 

#include <stdint.h>
#include <vector>

//For GLFW
void InitPlatform();
void DeInitPlatform();
void AddRequiredPlatformInstanceExtensions(std::vector<const char *> *instance_extensions);

void ErrorCheck(VkResult result);
uint32_t FindMemoryTypeIndex(
	const VkPhysicalDeviceMemoryProperties * gpu_memory_properties, 
	const VkMemoryRequirements * memory_requirements, 
	const VkMemoryPropertyFlags required_properties);

#endif