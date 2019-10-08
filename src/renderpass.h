/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vulkan/vulkan.h>
#include "utils.h"
#include "swapchain.h"

typedef struct RenderPass {
    VkRenderPass renderPass;
} RenderPass;

static void renderpass_init(RenderPass* pass,const SwapChain* swapchain) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchain->format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // Clear values to constant at the start
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // dont care about stencil values
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // care only about presenting the image
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

}
#endif /* RENDERPASS_H */
