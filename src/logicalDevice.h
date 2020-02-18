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
#include "uniformobjects.h"
#include "pipeline.h"
#include "renderpass.h"
#include "frameBuffer.h"
#include "commandBuffer.h"
#include "vertex.h"
#include "texture.h"

// Store all needed data about Logical device
typedef struct LogicalDevice {
    VkDevice            device;
    VkQueue             graphicsQueue;
    VkQueue             presentQueue;
    SwapChain           swapchain;
    Pipeline            pipeline;
    VkRenderPass        renderPass;
    FrameBuffer         frameBuffer;
    VkCommandPool       commandPool;
    CommandBuffers      commandBuffer;
    VertexData          vertexData;

    struct {
        VkDescriptorPool    descriptorPool;
        VkDescriptorSet*    descriptorSets;
    };

    UniformObject       ubo;
    Buffer*             uniformBuffers;

    struct {
        VkSemaphore*    imageSemaphore;
        VkSemaphore*    renderSemaphore;
    };
    struct { // Ensure that we do not refer to same frame that is in flight
        VkFence*        flightFences;
        VkFence*        imageFences;
    };

    Texture texture;
    Texture depth;

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

    device->renderPass = renderpass_create(&device->swapchain,
            device->device,
            physicalDevice->physicalDevice);
    LOG("Renderpass inited");

    uniformobject_init(&device->ubo, device->device);
    LOG("uniform objects created");

    pipeline_init(&device->pipeline, device->device,
            device->swapchain.extent, device->renderPass, device->ubo.uboLayout);
    LOG("Pipeline created");

    device->commandPool = commandpool_create(physicalDevice->queues.graphicsFamily, device->device);
    LOG("Commandpool created");

    device->depth = texture_depth_create(physicalDevice->physicalDevice, device->device, device->swapchain.extent);
    LOG("Creted depth texture");

    framebuffer_init(&device->frameBuffer, device->device,
            &device->swapchain, device->renderPass, device->depth.view);
    LOG("Framebuffer created");

    vertexdata_init(&device->vertexData, device->device,
            physicalDevice->physicalDevice, device->commandPool, device->graphicsQueue);
    LOG("Vertex data inited");

    device->texture = texture_load_and_create("textures/chalet.jpg", physicalDevice->physicalDevice,
            device->device, device->commandPool, device->graphicsQueue);
    LOG("Texture loaded and created");

    device->uniformBuffers = uniformbuffers_create(device->swapchain.numImages,
            device->device, physicalDevice->physicalDevice);
    LOG("Uniformbuffers created");

    device->descriptorPool = descriptorpool_create(device->swapchain.numImages, device->device);
    LOG("descriptorpool created");

    device->descriptorSets = descriptorsets_create( device->descriptorPool, device->device,
            device->swapchain.numImages, device->ubo.uboLayout, device->uniformBuffers, &device->texture);
    LOG("descriptorsets created");


    commandbuffers_init(&device->commandBuffer,
            &device->frameBuffer, device->device, device->renderPass,
            device->swapchain.extent, &device->pipeline,
            device->commandPool, &device->vertexData, device->descriptorSets);


    LOG("Commandbuffers created");
    _create_semaphores(device);
    LOG("Semaphores created");
    _create_fences(device, device->swapchain.numImages);
    LOG("Fences created");
}

static void _swapchain_cleanup(LogicalDevice* device) {

    texture_dispose(&device->depth, device->device);
    LOG("Disposed depth texture");

    framebuffer_dispose(&device->frameBuffer, device->device);
    LOG("Disposed framebuffer");

    vkFreeCommandBuffers(device->device, device->commandPool,
            device->commandBuffer.numBuffers, device->commandBuffer.buffers);
    LOG("Freed commandbuffers");

    pipeline_dispose(&device->pipeline, device->device);
    LOG("Disposed pipeline");

    renderpass_dispose(device->renderPass, device->device);
    LOG("Disposed renderpass");

    uniformbuffer_dispose(&device->uniformBuffers, device->swapchain.numImages, device->device);
    LOG("Disposed uniformbuffers");

    descriptorpool_dispose(device->descriptorPool, device->device);
    LOG("Disposed descriptorpool");

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

    uniformobject_dispose(&device->ubo, device->device);
    LOG("Disposed uniform object");

    descriptorsets_dispose(device->descriptorSets);

    vertexdata_dispose(&device->vertexData, device->device);
    LOG("Diposed vertex buffer");

    texture_dispose(&device->texture, device->device);

    _semaphores_dispose(device);
    LOG("Disposed semaphores");

    commandbuffers_dispose(&device->commandBuffer);
    LOG("Disposed commandbuffer");

    commandpool_dispose(device->commandPool, device->device);
    LOG("Disposed commandpool");

    // device queues are automaticly disposed when device is disposed
    vkDestroyDevice(device->device, NULL);
    LOG("Disposed logicaldevice");
    memset(device, 0 , sizeof *device);
}


static void
logicaldevice_resize(LogicalDevice* device,const PhysicalDevice* physicalDevice, VkSurfaceKHR surface) {

    LOG_COLOR(CONSOLE_COLOR_BLUE, "Resizing window");
    vkDeviceWaitIdle(device->device);

    // cleanup old swapchain
    _swapchain_cleanup(device);

    // reinit everything
    swapchain_init(&device->swapchain, physicalDevice->physicalDevice,
            surface, physicalDevice->queues,device->device);
    LOG("Swapchain recreated");

    device->renderPass = renderpass_create(&device->swapchain,
            device->device, physicalDevice->physicalDevice);
    LOG("Renderpass recreated");

    pipeline_init(&device->pipeline, device->device,
            device->swapchain.extent, device->renderPass, device->ubo.uboLayout);
    LOG("Pipeline recreated");

    device->depth = texture_depth_create(physicalDevice->physicalDevice, device->device, device->swapchain.extent);
    LOG("Creted depth texture");

    framebuffer_init(&device->frameBuffer, device->device,
            &device->swapchain, device->renderPass, device->depth.view);
    LOG("Framebuffer recreated");

    device->uniformBuffers = uniformbuffers_create(device->swapchain.numImages,
            device->device, physicalDevice->physicalDevice);
    LOG("Uniformbuffers created");

    device->descriptorPool = descriptorpool_create(device->swapchain.numImages, device->device);
    LOG("descriptorpool created");

    device->descriptorSets = descriptorsets_create( device->descriptorPool, device->device,
            device->swapchain.numImages, device->ubo.uboLayout, device->uniformBuffers, &device->texture);
    LOG("descriptorsets created");

    commandbuffers_init(&device->commandBuffer, &device->frameBuffer, device->device,
            device->renderPass, device->swapchain.extent, &device->pipeline,
            device->commandPool, &device->vertexData, device->descriptorSets);
    LOG("Commandbuffers recreated");
    LOG_COLOR(CONSOLE_COLOR_BLUE, "Done resizing window");
}

#endif //LOGICALDEVICE_H
