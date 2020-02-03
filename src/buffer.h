/************************************************************
 * Check license.txt in project root for license information*
 ************************************************************/

#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.h>
#include "utils.h"

typedef struct Buffer {
    VkBuffer        bufferId;
    VkDeviceMemory  bufferMemory;
    size_t          size;
} Buffer;

Buffer
buffer_create(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size,
        VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags properties) {

    Buffer ret;
    ret.size = size;
    // create buffer
    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = usageFlags;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device, &info, NULL /*allocator*/, &ret.bufferId) != VK_SUCCESS) {
        ABORT("failed to create buffer");
    }

    // get memory requirements
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device, ret.bufferId, &requirements);

    // allocate buffer
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = physicaldevice_find_memorytype(
            physicalDevice, requirements.memoryTypeBits,
            properties); // copy data immidiately after unmap

    if(vkAllocateMemory(device, &allocInfo, NULL /*allocator*/, &ret.bufferMemory) != VK_SUCCESS) {
        ABORT("Failed to allocate buffer");
    }

    // Bind memory to buffer
    vkBindBufferMemory(device, ret.bufferId, ret.bufferMemory, 0 /* device size memory offset */);
    return ret;
}

static void
buffer_copy(const Buffer* src, const Buffer* dst, VkDevice device, VkCommandPool pool, VkQueue que) {
    // Memory operation are executed using command buffers
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    // Allocate comand buffer
    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer);

    // record commandbuffer
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferCopy reqion = {};
    reqion.size = src->size;
    vkCmdCopyBuffer(cmdBuffer, src->bufferId, dst->bufferId, 1 /* reqion count */, &reqion);

    vkEndCommandBuffer(cmdBuffer);

    // Submit command buffer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    vkQueueSubmit(que, 1 /*submit count*/, &submitInfo, VK_NULL_HANDLE /*fence*/);
    // wait for transfer to happen
    vkQueueWaitIdle(que);

    //free the used commandbuffers
    vkFreeCommandBuffers(device, pool, 1, &cmdBuffer);
}

static void
buffer_dispose(Buffer* data, VkDevice device) {

    vkDestroyBuffer(device, data->bufferId, NULL /*allocator*/);
    vkFreeMemory(device ,data->bufferMemory, NULL /*allocator*/);
    memset(data, 0, sizeof *data);
}

#endif /* BUFFER_H */
