/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vulkan/vulkan.h>
#include "utils.h"
#include "swapchain.h"

typedef struct FrameBuffer {
    VkFramebuffer*  buffers;
    u32             numBuffers;
} FrameBuffer;

static void
framebuffer_init(FrameBuffer* buffer, const VkDevice device, const SwapChain* swapChain, VkRenderPass renderPass) {

    buffer->buffers = (VkFramebuffer*)malloc(sizeof *buffer->buffers * swapChain->numImages);
    buffer->numBuffers = swapChain->numImages;

    // Create Framebuffer for each image in swap chain
    for (u32 i = 0; i < swapChain->numImages; i++) {
        VkImageView* attachments = &swapChain->views[i];

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChain->extent.width;
        framebufferInfo.height = swapChain->extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &buffer->buffers[i]) != VK_SUCCESS) {
            ABORT("failed to create framebuffer!");
        }
    }
}

static void
framebuffer_dispose(FrameBuffer* buffer, const VkDevice device) {

    for (u32 i =  0; i < buffer->numBuffers; i++) {
        vkDestroyFramebuffer(device, buffer->buffers[i], NULL);
    }
    free(buffer->buffers);
    memset(buffer, 0 , sizeof *buffer);
}

#endif /* FRAMEBUFFER_H */
