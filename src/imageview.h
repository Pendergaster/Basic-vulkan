/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */
#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <vulkan/vulkan.h>
#include "utils.h"

typedef struct ImageView {
    VkImageView view;
} ImageView;

static ImageView imageview_create(const VkImage image, VkFormat format,const VkDevice device) {
    ImageView ret = {};

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

    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &createInfo, NULL, &ret.view) != VK_SUCCESS) {
        ABORT("Failed to create image view");
    }

    return ret;
}

static void imageview_dispose(const ImageView view,const VkDevice device) {
    vkDestroyImageView(device,view.view,NULL);
}

#endif /* IMAGEVIEW_H */
