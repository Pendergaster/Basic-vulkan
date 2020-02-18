/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <vulkan/vulkan.h>
#include "utils.h"
#include "window.h"
#include "queueIndexes.h"
#include "cmath.h"
#include "imageview.h"
#include "physicalDevice.h"

typedef struct SwapChain {
    VkSwapchainKHR  swapchain;
    u32             numImages;
    VkImage*        swapchainImages;
    VkFormat        format;
    VkExtent2D      extent;
    VkImageView*    views;
} SwapChain ;



static VkSurfaceFormatKHR _swapchain_choose_format(const VkSurfaceFormatKHR* formats,u32 numFormats) {
    for(u32 i = 0; i < numFormats; i++) {
        // select format if it has b,r,g and a channels and is SRGB color
        if(formats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
                formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }
    return formats[0];
}

static VkPresentModeKHR _swapchain_choose_present_mode(const VkPresentModeKHR* modes,u32 numModes) {
    for(u32 i = 0; i < numModes; i++) {
        // if possible prefer mailbox
        if(modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return modes[i];
        }
    }
    // just default to this
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D _swapchain_choose_extent(const VkSurfaceCapabilitiesKHR* capabilities) {
    if(capabilities->currentExtent.width != numeric_max_u32) {
        return capabilities->currentExtent;
    }
    // Query the size
    int width, height;
    glfwGetFramebufferSize(g_window, &width, &height);
    VkExtent2D actualExtent = {.width = width, .height = height};

    // clamp value between max and min values
    actualExtent.width = clamp_u32(actualExtent.width,
            capabilities->minImageExtent.width,capabilities->maxImageExtent.width);

    actualExtent.height = clamp_u32(actualExtent.height,
            capabilities->minImageExtent.height,capabilities->maxImageExtent.height);

    return actualExtent;
}


static void swapchain_init(SwapChain* swapchain,const VkPhysicalDevice physicalDevice,
        const VkSurfaceKHR surface,const QueueFamilyIndices indexes,const VkDevice logicalDevice) {

    SwapchainSupportDetails supportDetails =
        physicaldevice_get_swapchain_support_details(physicalDevice,surface);

    VkSurfaceFormatKHR surfaceFormat = _swapchain_choose_format(supportDetails.formats,supportDetails.numFormats);
    VkPresentModeKHR presentMode = _swapchain_choose_present_mode(supportDetails.presentModes,supportDetails.numPresentationModes);
    VkExtent2D extent = _swapchain_choose_extent(&supportDetails.capabilities);

    // create one more image than required
    u32 numImages = supportDetails.capabilities.minImageCount + 1;
    // check that we dont exceed the maxinum amount of images
    if(supportDetails.capabilities.maxImageCount < numImages &&  supportDetails.capabilities.maxImageCount != 0) {
        numImages = supportDetails.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = numImages;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.presentMode = presentMode;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    u32 queueFamilyIndexes[] = {indexes.graphicsFamily,indexes.presentFamily};
    if(indexes.graphicsFamily != indexes.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2; // 2 queues
        createInfo.pQueueFamilyIndices = queueFamilyIndexes;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }
    // default image rotation
    createInfo.preTransform = supportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // dont care about color of pixels that are obscured
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(logicalDevice,&createInfo,NULL,&swapchain->swapchain) != VK_SUCCESS) {
        ABORT("Failed to create logical device");
    }
    physicaldevice_dispose_swapchain_support_details(&supportDetails);

    // now get image handles, implementation is allowed to create more images than we requested
    // and set extent and format
    vkGetSwapchainImagesKHR(logicalDevice,swapchain->swapchain,&swapchain->numImages,NULL);
    swapchain->swapchainImages = (VkImage*)malloc(swapchain->numImages * sizeof *swapchain->swapchainImages);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain->swapchain, &swapchain->numImages, swapchain->swapchainImages);
    swapchain->format = surfaceFormat.format;
    swapchain->extent = extent;
    swapchain->views = (VkImageView*)malloc(swapchain->numImages * sizeof *swapchain->views);
    // create each image its view
    swapchain->numImages = swapchain->numImages;
    for(u32 i = 0; i < swapchain->numImages; i++) {
        swapchain->views[i] = imageview_create(swapchain->swapchainImages[i],
                swapchain->format, VK_IMAGE_ASPECT_COLOR_BIT,
                logicalDevice);
    }
}

static void swapchain_dispose(SwapChain* chain,const VkDevice device) {
    for(u32 i = 0; i < chain->numImages;i++) {
        imageview_dispose(chain->views[i],device);
    }
    vkDestroySwapchainKHR(device,chain->swapchain,NULL);
    free(chain->swapchainImages);
    memset(chain,0,sizeof *chain);
}

#endif
