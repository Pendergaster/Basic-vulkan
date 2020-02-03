/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef LOGICALDEVICE_H
#define LOGICALDEVICE_H
#include <vulkan/vulkan.h>
#include "utils.h"
#include "validationLayers.h"
#include "physicalDevice.h"
#include "vulkanExtensions.h"
#include "swapchain.h"
#include "pipeline.h"
#include "renderpass.h"
#include "frameBuffer.h"
#include "commandBuffer.h"
#include "vertex.h"

// Store all needed data about Logical device
typedef struct LogicalDevice {
    VkDevice        device;
    VkQueue         graphicsQueue;
    VkQueue         presentQueue;
    SwapChain       swapchain;
    Pipeline        pipeline;
    VkRenderPass    renderPass;
    FrameBuffer     frameBuffer;
    VkCommandPool   commandPool;
    CommandBuffers  commandBuffer;
    VertexData      vertexData;

    struct {
        VkSemaphore     *imageSemaphore;
        VkSemaphore     *renderSemaphore;
    };
    struct { // Ensure that we do not refer to same frame that is in flight
        VkFence* flightFences;
        VkFence* imageFences;
    };
} LogicalDevice;

static void _create_semaphores(LogicalDevice* device) {

    device->imageSemaphore = (VkSemaphore*)malloc(sizeof *device->imageSemaphore * MAX_FRAMES_IN_FLIGHT);
    device->renderSemaphore = (VkSemaphore*)malloc(sizeof *device->renderSemaphore * MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device->device, &semaphoreInfo, NULL, &device->renderSemaphore[i]) != VK_SUCCESS
                || vkCreateSemaphore(device->device, &semaphoreInfo, NULL, &device->imageSemaphore[i]) != VK_SUCCESS) {
            ABORT("failed to create semaphores!");
        }
    }
}

static void _create_fences(LogicalDevice* device, u32 numSwapChainImages) {

    device->flightFences = (VkFence*)malloc(sizeof *device->flightFences * MAX_FRAMES_IN_FLIGHT);
    device->imageFences = (VkFence*)calloc(numSwapChainImages, sizeof *device->flightFences);

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if(vkCreateFence(device->device, &fenceInfo, NULL, &device->flightFences[i]) != VK_SUCCESS) {
            ABORT("failed to create flightFences!");
        }
    }
}

static void
logicaldevice_init(const PhysicalDevice* physicalDevice, LogicalDevice* device, VkSurfaceKHR surface) {

    device->device = physicaldevice_create_logicaldevice(physicalDevice);
    LOG("Logical device created");
    // set proper queues
    vkGetDeviceQueue(device->device, physicalDevice->queues.graphicsFamily, 0, &device->graphicsQueue);
    vkGetDeviceQueue(device->device, physicalDevice->queues.presentFamily, 0, &device->presentQueue);

    swapchain_init(&device->swapchain, physicalDevice->physicalDevice,
            surface, physicalDevice->queues,device->device);
    LOG("Swapchain created");
    device->renderPass = renderpass_create(&device->swapchain, device->device);
    LOG("Renderpass inited");
    pipeline_init(&device->pipeline, device->device, device->swapchain.extent, device->renderPass);
    LOG("Pipeline created");
    framebuffer_init(&device->frameBuffer, device->device, &device->swapchain, device->renderPass);
    LOG("Framebuffer created");
    device->commandPool = commandpool_create(physicalDevice->queues.graphicsFamily, device->device);
    LOG("Commandpool created");

    vertexdata_init(&device->vertexData, device->device,
            physicalDevice->physicalDevice, device->commandPool, device->graphicsQueue);
    LOG("Vertex data inited");

    commandbuffers_init(&device->commandBuffer,
            &device->frameBuffer, device->device, device->renderPass,
            device->swapchain.extent, device->pipeline.graphicsPipeline,
            device->commandPool, &device->vertexData);


    LOG("Commandbuffers created");
    _create_semaphores(device);
    LOG("Semaphores created");
    _create_fences(device, device->swapchain.numImages);
    LOG("Fences created");
}

static void _swapchain_cleanup(LogicalDevice* device) {

    framebuffer_dispose(&device->frameBuffer, device->device);
    LOG("Disposed framebuffer");
    vkFreeCommandBuffers(device->device, device->commandPool,
            device->commandBuffer.numBuffers, device->commandBuffer.buffers);
    LOG("Freed commandbuffers");
    pipeline_dispose(&device->pipeline, device->device);
    LOG("Disposed pipeline");
    renderpass_dispose(device->renderPass, device->device);
    LOG("Disposed renderpass");
    swapchain_dispose(&device->swapchain,device->device);
    LOG("Disposed swapchain");
}

static void _semaphores_dispose(LogicalDevice* device) {
    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device->device, device->renderSemaphore[i], NULL);
        vkDestroySemaphore(device->device, device->imageSemaphore[i], NULL);
        vkDestroyFence(device->device, device->flightFences[i], NULL);
    }
    free(device->renderSemaphore);
    free(device->imageSemaphore);

}

static void
logicalDevice_dispose(LogicalDevice* device) {
    _swapchain_cleanup(device);
    LOG("Disposed swapchain");

    vertexdata_dispose(&device->vertexData, device->device);
    LOG("vertex buffer disposed");

    _semaphores_dispose(device);
    LOG("Disposed semaphores");

    commandbuffers_dispose(&device->commandBuffer);
    LOG("Disposed commandbuffer");

    commandpool_dispose(device->commandPool, device->device);
    LOG("Disposed commandbuffer");

    // device queues are automaticly disposed when device is disposed
    vkDestroyDevice(device->device, NULL);
    LOG("Disposed logicaldevice");
    memset(device, 0 , sizeof *device);
}


static void logicaldevice_resize(LogicalDevice* device,const PhysicalDevice* physicalDevice, VkSurfaceKHR surface) {
    vkDeviceWaitIdle(device->device);

    // cleanup old swapchain
    _swapchain_cleanup(device);

    // reinit everything
    swapchain_init(&device->swapchain, physicalDevice->physicalDevice,
            surface, physicalDevice->queues,device->device);
    LOG("Swapchain recreated");
    device->renderPass = renderpass_create(&device->swapchain, device->device);
    LOG("Renderpass recreated");
    pipeline_init(&device->pipeline, device->device, device->swapchain.extent, device->renderPass);
    LOG("Pipeline recreated");
    framebuffer_init(&device->frameBuffer, device->device, &device->swapchain, device->renderPass);
    LOG("Framebuffer recreated");
    commandbuffers_init(&device->commandBuffer, &device->frameBuffer, device->device, device->renderPass,
            device->swapchain.extent, device->pipeline.graphicsPipeline, device->commandPool, &device->vertexData);
    LOG("Commandbuffers recreated");
}

#endif //LOGICALDEVICE_H
