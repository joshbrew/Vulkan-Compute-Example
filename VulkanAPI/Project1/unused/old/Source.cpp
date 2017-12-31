#include <array>
#include <chrono>
#include <iostream>

#include "Render.h"
#include "Window.h"



int main() {
	Render app;
	try {
		auto w = app.OpenWindow(800, 600, "VulkanAPI Test Window");

		app.Run();
		
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


/*
constexpr double PI = 3.14159265358979323846;
constexpr double CIRCLE_RAD = PI * 2;
constexpr double CIRCLE_THIRD = CIRCLE_RAD / 3.0;
constexpr double CIRCLE_THIRD_1 = 0;
constexpr double CIRCLE_THIRD_2 = CIRCLE_THIRD;
constexpr double CIRCLE_THIRD_3 = CIRCLE_THIRD * 2;

int main() {

	Render r;

	auto w = r.OpenWindow(800, 600, "VulkanAPI Test Window");

	//Command Pool allocates memory for command buffers.
	VkCommandPool command_pool = VK_NULL_HANDLE;
	VkCommandPoolCreateInfo pool_create_info{};
	pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_create_info.queueFamilyIndex = r.GetVulkanGraphicsQueueFamilyIndex();
	vkCreateCommandPool(r.GetVulkanDevice(), &pool_create_info, nullptr, &command_pool);

	// Vertex buffer goes here.

	//Command Buffer records commands to send to GPU
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo	command_buffer_allocate_info{};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = command_pool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //Secondary command buffers are accessed thru primary buffer in this case.
	command_buffer_allocate_info.commandBufferCount = 1;
	vkAllocateCommandBuffers(r.GetVulkanDevice(), &command_buffer_allocate_info, &command_buffer);
	//Secondary CBs allow multiple threads to generate rendering commands to the main render pass.
	//EX: For storing meshes. For viewport setup (changed only when viewport size is changed). 
	//     For memory barriers (provide what resources are transitioning between source and destination pipeline stages) via vkCmdPipelineBarrier.

	//Semaphores coordinate queue operations within or between GPU queues.
	VkSemaphore render_complete_semaphore = VK_NULL_HANDLE;
	VkSemaphoreCreateInfo semaphore_create_info{};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(r.GetVulkanDevice(), &semaphore_create_info, nullptr, &render_complete_semaphore);
	// Fences sync GPU to CPU
	// Events and Barriers set when specific command buffer resources are executed within a queue.


	float color_rotator = 0.0f;
	auto timer = std::chrono::steady_clock();
	auto last_time = timer.now();
	uint64_t frame_counter = 0;
	uint64_t fps = 0; 

	while (r.Run()) {
		
		// CPU logic
		++frame_counter;
		if (last_time + std::chrono::seconds(1) < timer.now()) {
			last_time = timer.now();
			fps = frame_counter;
			frame_counter = 0;
			std::cout << "FPS: " << fps << std::endl;
		}

		// Begin render
		w->BeginRender();
		// Record command buffer
		VkCommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		/*
		
		     VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT indicates that each recording of the command buffer will 
			 only be submitted once, and the command buffer will be reset and recorded again between each submission.
			 
			 VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT indicates that a secondary command buffer is considered 
			 to be entirely inside a render pass. If this is a primary command buffer, then this bit is ignored.
			 
			 VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT allows the command buffer to be resubmitted to a queue 
			 or recorded into a primary command buffer while it is pending execution. 

			 So in my case, the command buffer records to the primary command buffer for a single render pass per frame

		*/ /*

		// Begin recording command buffer
		vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info); 

		VkRect2D render_area{};
		render_area.offset.x = 0;
		render_area.offset.y = 0;
		render_area.extent = w->GetVulkanSurfaceSize();

		color_rotator += 0.001;

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].depthStencil.depth = 0.0f;
		clear_values[0].depthStencil.stencil = 0;
		clear_values[1].color.float32[0] = std::sin(color_rotator + CIRCLE_THIRD_1) * 0.5 + 0.5; //B
		clear_values[1].color.float32[1] = std::sin(color_rotator + CIRCLE_THIRD_2) * 0.5 + 0.5; //G
		clear_values[1].color.float32[2] = std::sin(color_rotator + CIRCLE_THIRD_3) * 0.5 + 0.5; //R
		clear_values[1].color.float32[3] = 1.0f;												 //A

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = w->GetVulkanRenderPass();
		render_pass_begin_info.framebuffer = w->GetVulkanActiveFramebuffer();
		render_pass_begin_info.renderArea = render_area;
		render_pass_begin_info.clearValueCount = clear_values.size();
		render_pass_begin_info.pClearValues = clear_values.data();

		//Begin first subpass of new render pass as specified by the command buffer
		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		// VK_SUBPASS_CONTENTS_INLINE: contents of the subpass will be recorded inline in the primary command buffer.
		// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: contents are recorded in secondary command buffers that will be called from the primary command buffer.
		
		//End first subpass. (Or main pass in this case)
		vkCmdEndRenderPass(command_buffer);
		
		// End recording of commands
		vkEndCommandBuffer(command_buffer); 

		// Submit command buffer to queue
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 0;							// Number of semaphore
		submit_info.pWaitSemaphores = nullptr;						// Points to array of semaphores to wait before command buffers for this batch execute.
		submit_info.pWaitDstStageMask = nullptr;					// Points to array of pipeline stages at which each semaphore wait occurs
		submit_info.commandBufferCount = 1;							// Number of command buffers to execute in batch.
		submit_info.pCommandBuffers = &command_buffer;				// points to command buffer array to execute
		submit_info.signalSemaphoreCount = 1;						// Number of semaphores to be signaled on execution completion.
		submit_info.pSignalSemaphores = &render_complete_semaphore; // Pointer to arrayof semaphores to signal when this batch is completed

		// Submit queue with recorded command buffer
		vkQueueSubmit(r.GetVulkanQueue(), 1, &submit_info, VK_NULL_HANDLE);

		// End render
		w->EndRender({ render_complete_semaphore });
	}

	// Ensure GPU and CPU are synchronized
	vkQueueWaitIdle(r.GetVulkanQueue());

	//Destroy semaphore
	vkDestroySemaphore(r.GetVulkanDevice(), render_complete_semaphore, nullptr);
	//Destroy command pool
	vkDestroyCommandPool(r.GetVulkanDevice(), command_pool, nullptr);

	return 0;
}

*/