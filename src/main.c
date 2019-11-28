/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#include "utils.h"
#include "window.h"
#include "vulkanContext.h"
//#include "validationLayers.h"
//#include "physicalDevice.h"
#include "logicalDevice.h"
//#include "swapchain.h"
//#include "pipeline.h"
//#include "renderpass.h"
//#include "frameBuffer.h"

static void init(VulkanContext* context,LogicalDevice* device);
static void cleanup(VulkanContext* context,LogicalDevice* device);
static void main_loop(LogicalDevice* device);
static void draw_frame(LogicalDevice* device);

    i32
main(const int argc,char **argv)
{
    (void)argc;(void)argv;
    VulkanContext context = {};
    LogicalDevice logicalDevice;
    init(&context,&logicalDevice);

    main_loop(&logicalDevice);

    cleanup(&context,&logicalDevice);
    LOG("All disposed");
    return 0;
}

void
init(VulkanContext* context, LogicalDevice* device) {

    colored_print_init();
    window_init();
    LOG("Window initialized");
    vulkancontext_init(context);
    LOG("Context initialized");
    logicaldevice_init(&context->physicalDevice, device, context->surface);
    LOG("logical parts initialized!");

}

static void
main_loop(LogicalDevice* device) {

    while (!glfwWindowShouldClose(g_window)) {
        glfwPollEvents();
        draw_frame(device);
    }

    vkDeviceWaitIdle(device->device);
}


static void
draw_frame(LogicalDevice* device) {
    static u32 currentFrame = 0;

    vkWaitForFences(device->device, 1, &device->flightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(device->device, 1, &device->flightFences[currentFrame]);

    u32 imageIndex;
    // Get image index
    vkAcquireNextImageKHR(device->device, device->swapchain.swapchain, UINT64_MAX,
            device->imageSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (device->imageFences[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device->device, 1, &device->imageFences[imageIndex], VK_TRUE, UINT64_MAX);
    }
    device->imageFences[imageIndex] = device->flightFences[currentFrame];

    // Submit command buffer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Wait these semaphores before executing
    VkSemaphore* waitSemaphores = &device->imageSemaphore[currentFrame];
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &device->commandBuffer.buffers[imageIndex];

    // Signal these semaphores after executing
    VkSemaphore *signalSemaphores = &device->renderSemaphore[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkSubpassDependency dependency = {};(void) dependency; //TODO
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    if (vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, device->flightFences[currentFrame]) != VK_SUCCESS) {
        ABORT("failed to submit draw command buffer!");
    }

    // Submit result to swap chain to eventually show it
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR* swapChains = &device->swapchain.swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(device->presentQueue, &presentInfo);
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    //vkQueueWaitIdle(device->presentQueue);
}

// free memory, context and other resources
static void
cleanup(VulkanContext* context, LogicalDevice* device) {

    logicalDevice_dispose(device);
    vulkancontext_dispose(context);
    dispose_window();
}
