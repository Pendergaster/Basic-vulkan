/************************************************************
* Check license.txt in project root for license information *
*********************************************************** */

#include "utils.h"
#include "window.h"
#include "vulkanContext.h"
#include "validationLayers.h"
#include "physicalDevice.h"
#include "logicalDevice.h"

void cleanup(VulkanContext* context);
void init(VulkanContext* context);

int main(const int argc,char **argv)
{
	(void)argc;(void)argv;
	VulkanContext context = {};
	init(&context);

	// main loop
	while (!glfwWindowShouldClose(g_window)) {
		glfwPollEvents();

	}

	cleanup(&context);
	LOG("All disposed");
	return 0;
}

void init(VulkanContext* context) {
	init_colored_print();
	init_window();
	LOG("Window initialized");
	init_vulkanContext(context);
	if (enableValidationLayers) {
		init_debug_messenger(context->instance,&context->debugMessenger);
		LOG("Debug messenger initialized");

	}
	LOG("Vulkan context initialized");
	create_window_surface(context->instance,&context->surface);
	LOG("Window surface created");
	pick_physical_device(context->instance,&context->physicalDevice,context->surface);
	LOG("Physical device picked");
	init_logical_device(&context->physicalDevice,&context->logicalDevice);
	LOG("Logical device created");
}

// free memory, context and other resources
void cleanup(VulkanContext* context) {
	vulkancontext_dispose(context);
	dispose_window();
}
