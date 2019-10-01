/************************************************************
* Check license.txt in project root for license information *
*********************************************************** */

#include "utils.h"
#include "window.h"
#include "vulkanContext.h"
#include "validationLayers.h"
#include "physicalDevice.h"
#include "logicalDevice.h"
#include "swapchain.h"

void init(VulkanContext* context,LogicalDevice* device);
void cleanup(VulkanContext* context,LogicalDevice* device);

int main(const int argc,char **argv)
{
	(void)argc;(void)argv;
	VulkanContext context = {};
	LogicalDevice logicalDevice;
	init(&context,&logicalDevice);

	// main loop
	while (!glfwWindowShouldClose(g_window)) {
		glfwPollEvents();

	}

	cleanup(&context,&logicalDevice);
	LOG("All disposed");
	return 0;
}

void init(VulkanContext* context,LogicalDevice* device) {
	colored_print_init();
	window_init();
	LOG("Window initialized");
	vulkancontext_init(context);
	if (enableValidationLayers) {
		init_debug_messenger(context->instance,&context->debugMessenger);
		LOG("Debug messenger initialized");

	}
	LOG("Vulkan context initialized");
	window_create_surface(context->instance,&context->surface);
	LOG("Window surface created");
	physical_device_pick(context->instance,&context->physicalDevice,context->surface);
	LOG("Physical device picked");
	logicaldevice_init(&context->physicalDevice,device);
	LOG("Logical device created");
	swapchain_init(&device->swapchain,context->physicalDevice.physicalDevice,
			context->surface,context->physicalDevice.queues,device->device);
	LOG("Swapchain created");
}

// free memory, context and other resources
void cleanup(VulkanContext* context, LogicalDevice* device) {
	logicalDevice_dispose(device);
	vulkancontext_dispose(context);
	dispose_window();
}
