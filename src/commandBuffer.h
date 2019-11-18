/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef COMMANDPOOLS_H
#define COMMANDPOOLS_H

#include <vulkan/vulkan.h>
#include "utils.h"
#include "physicalDevice.h"
#include "renderpass.h"
#include "frameBuffer.h"

typedef struct CommandBuffers {
    VkCommandPool       pool;
    VkCommandBuffer*    buffers;
    u32                 numBuffers;
} CommandBuffers;


static inline VkCommandPool
_commandpool_create(const PhysicalDevice* physicalDevice, const VkDevice device) {

    VkCommandPool ret = 0;

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = physicalDevice->queues.graphicsFamily;
    poolInfo.flags = 0; // Optional

    //VK_COMMAND_POOL_CREATE_TRANSIENT_BIT:
    //  Hint that command buffers are rerecorded with new commands very often

    //VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT:
    //  Allow command buffers to be rerecorded individually, without this flag they all have to be reset together

    if (vkCreateCommandPool(device, &poolInfo, NULL, &ret) != VK_SUCCESS) {
        ABORT("failed to create command pool!");
    }
    return ret;
}

static void
commandbuffers_init(CommandBuffers* buffer, const PhysicalDevice* physicalDevice, const FrameBuffer* framebuffer,
        const VkDevice device, const VkRenderPass renderpass, VkExtent2D swapExtent, VkPipeline gpipeline) {

    // Create poo where buffers will be created
    buffer->pool = _commandpool_create(physicalDevice, device);
    // Create buffer for each framebuffer
    buffer->buffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * framebuffer->numBuffers);
    buffer->numBuffers = framebuffer->numBuffers;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = buffer->pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = buffer->numBuffers;

    if (vkAllocateCommandBuffers(device, &allocInfo, buffer->buffers) != VK_SUCCESS) {
        ABORT("failed to allocate command buffers!");
    }

    // start recording command buffers
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    for(u32 i = 0; i < buffer->numBuffers; i++) {
        if (vkBeginCommandBuffer(buffer->buffers[i], &beginInfo) != VK_SUCCESS) {
            ABORT("failed to begin recording command buffer!");
        }
        // Begin renderpass
        renderpass_start(renderpass, buffer->buffers[i], framebuffer->buffers[i], swapExtent);
        // Bind graphics pipeline
        vkCmdBindPipeline(buffer->buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gpipeline);

        vkCmdDraw(buffer->buffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(buffer->buffers[i]);

        if (vkEndCommandBuffer(buffer->buffers[i]) != VK_SUCCESS) {
            ABORT("failed to record command buffer!");
        }
    }
}

static void commandbuffers_dispose(CommandBuffers* buffer, const VkDevice device) {

    vkDestroyCommandPool(device, buffer->pool, NULL);
    free(buffer->buffers);
    memset(buffer, 0, sizeof *buffer);
}

// TODO cleanup function argumets

#endif /* COMMANDPOOLS_H */
