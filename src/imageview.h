/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */
#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <vulkan/vulkan.h>
#include "utils.h"

static VkImageView imageview_create(const VkImage image, u32 miplevels, VkFormat format,
        VkImageAspectFlags aspectFlag, const VkDevice device) {
    VkImageView ret = 0;

    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;

    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;

    // default
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = aspectFlag;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = miplevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &createInfo, NULL, &ret) != VK_SUCCESS) {
        ABORT("Failed to create image view");
    }

    return ret;
}

static void imageview_dispose(const VkImageView view,const VkDevice device) {
    vkDestroyImageView(device,view,NULL);
}

#endif /* IMAGEVIEW_H */
