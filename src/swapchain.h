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

typedef struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR	capabilities;
	u32							numFormats;
	VkSurfaceFormatKHR*			formats;
	u32							numPresentationModes;
	VkPresentModeKHR*			presentModes;
} SwapchainSupportDetails;

typedef struct SwapChain {

} SwapChain ;


static SwapchainSupportDetails swapchain_get_support_details(const VkPhysicalDevice device,const VkSurfaceKHR surface) {
	SwapchainSupportDetails details = {};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.numFormats, NULL);
	//query format details
	if (details.numFormats != 0) {
		details.formats = (VkSurfaceFormatKHR*)malloc(details.numFormats * sizeof(VkSurfaceFormatKHR));
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.numFormats, details.formats);
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.numPresentationModes, NULL);
	// query present modes
	if (details.numPresentationModes != 0) {
		details.presentModes = (VkPresentModeKHR*)malloc(details.numFormats * sizeof(VkPresentModeKHR));
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.numPresentationModes, details.presentModes);
	}

	return details;
}

static void swapchain_dispose_support_details(SwapchainSupportDetails* details) {
	free(details->formats);
	free(details->presentModes);
	memset(details,0,sizeof(SwapchainSupportDetails));
}

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
	VkExtent2D actualExtent = {.width = SCREENWIDTH, .height = SCREENHEIGHT};
	actualExtent.width = maxui(capabilities->minImageExtent.width,
			minui(capabilities->maxImageExtent.width,actualExtent.width));
	actualExtent.height = maxui(capabilities->minImageExtent.height,
			minui(capabilities->maxImageExtent.height,actualExtent.height));
	return actualExtent;
}

SwapChain swapchain_create(const VkPhysicalDevice device, const VkSurfaceKHR surface,const QueueFamilyIndices indexes) {
	SwapChain swapchain = {};
	SwapchainSupportDetails supportDetails = swapchain_get_support_details(device,surface);
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
	createInfo.clipped = VK_TRUE;

	return swapchain;
}
#endif
