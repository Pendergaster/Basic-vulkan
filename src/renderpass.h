/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vulkan/vulkan.h>
#include "utils.h"
#include "swapchain.h"
#include "physicalDevice.h"


static VkRenderPass
renderpass_create(const SwapChain* swapchain, const VkDevice device,
        const VkPhysicalDevice physicaldevice) {

    VkRenderPass pass;
    VkAttachmentDescription colorAttachment = {};
    VkAttachmentReference colorAttachmentRef = {};
    { // Color attachment
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

        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentDescription depthAttachment = {};
    VkAttachmentReference depthAttachmentRef = {};
    { // Depth attachent
        depthAttachment.format = physicaldevice_find_depth_format(physicaldevice);
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Draw vertexes
    subpass.colorAttachmentCount = 1;
    // This arrays index is called directly in shader
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Describe what sub passes depends on to correctly synchronice them
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    // Wait for swapchain to finish reading from it before we can access it
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachmets[] = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = SIZEOF_ARRAY(attachmets);
    renderPassInfo.pAttachments = attachmets;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, NULL, &pass) != VK_SUCCESS) {
        ABORT("Failed to create renderpass!");
    }

    return pass;
}

inline static void
renderpass_start(VkRenderPass pass, VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkExtent2D extent) {
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pass;
    renderPassInfo.framebuffer = framebuffer;

    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = extent;

    VkClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    VkClearDepthStencilValue depthValue = {.depth = 1.f, .stencil = 0};

    VkClearValue clearValues[] = {
        (VkClearValue){ .color = clearColor },
        (VkClearValue){ .depthStencil = depthValue },
    };

    renderPassInfo.clearValueCount = SIZEOF_ARRAY(clearValues);
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

static void
renderpass_dispose(VkRenderPass pass, const VkDevice device) {

    vkDestroyRenderPass(device, pass, NULL);
}
#endif /* RENDERPASS_H */
