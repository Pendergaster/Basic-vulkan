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
#include "vertex.h"
#include "pipeline.h"


typedef struct CommandBuffers {
    //VkCommandPool       pool;
    VkCommandBuffer*    buffers;
    u32                 numBuffers;
} CommandBuffers;


static inline VkCommandPool
commandpool_create(u32 graphicsFamily, const VkDevice device) {

    VkCommandPool ret = 0;

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsFamily;
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
commandbuffers_init(CommandBuffers* buffer, const FrameBuffer* framebuffer,
        const VkDevice device, const VkRenderPass renderpass, VkExtent2D swapExtent,
        Pipeline* pipeline, VkCommandPool pool, VertexData* vertexData, VkDescriptorSet* descSets) {

    // Create pool where buffers will be created
    //buffer->pool = _commandpool_create(physicalDevice, device);
    // Create buffer for each framebuffer
    buffer->buffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * framebuffer->numBuffers);
    buffer->numBuffers = framebuffer->numBuffers;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
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
        vkCmdBindPipeline(buffer->buffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->graphicsPipeline);

        // Bind vertex buffer
        VkBuffer vertBuffers[] = {vertexData->vertex.bufferId};
        VkDeviceSize offsets[] = {0}; // byte offset where start to read vertex data from

        vkCmdBindVertexBuffers(buffer->buffers[i],
                0, // firstbinding
                1, // bindingcount
                vertBuffers, offsets);

        vkCmdBindIndexBuffer(buffer->buffers[i], vertexData->index.bufferId,
                0,
                VK_INDEX_TYPE_UINT32);


        //vkCmdDraw(buffer->buffers[i], SIZEOF_ARRAY(Triangle), 1, 0, 0); no indexes

        // Bind descriptors
        vkCmdBindDescriptorSets( buffer->buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline->pipelineLayout,
                0 /*first set*/,
                1/*desc count*/,
                &descSets[i],
                0/*dynamic offset*/,
                NULL /*dynamic offsets*/);

        vkCmdDrawIndexed(buffer->buffers[i], SIZEOF_ARRAY(RectangleIndexes),
                1,  // instance count
                0, // first index
                0, // vertexoffset
                0); // first instance
        vkCmdEndRenderPass(buffer->buffers[i]);

        if (vkEndCommandBuffer(buffer->buffers[i]) != VK_SUCCESS) {
            ABORT("failed to record command buffer!");
        }
    }
}

static void commandpool_dispose(VkCommandPool pool, const VkDevice device) {

    vkDestroyCommandPool(device, pool, NULL);
}

static void commandbuffers_dispose(CommandBuffers* buffer) {

    free(buffer->buffers);
    memset(buffer, 0, sizeof *buffer);
}

// TODO cleanup function argumets

#endif /* COMMANDPOOLS_H */
