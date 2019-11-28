#ifndef VULKANEXTENSIONS_H
#define VULKANEXTENSIONS_H

#include <vulkan/vulkan.h>
#include "defs.h"
#include "validationLayers.h"
#include "window.h"


const char* g_extensionNames[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const char** extensions_get_required(u32* numExtensions) {
    u32 windowExtensionCount = 0;
    const char** windowExtensions = window_get_required_extensions(&windowExtensionCount);
    // if we have validation layers we have to add correct extension (to get the messagehandle)
    u32 numRequiredExtensions = windowExtensionCount;
    if (enableValidationLayers) {
        numRequiredExtensions += 1;
    }

    const char** returnExtensions = (const char**)malloc(sizeof(char*) * numRequiredExtensions);
    // copy windows extensions to allocated array
    for(u32 i = 0; i < windowExtensionCount; i++) {
        returnExtensions[i] = windowExtensions[i];
    }

    // copy wanted validation layer extension to the end
    if(enableValidationLayers) {
        returnExtensions[numRequiredExtensions - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
    if(numExtensions) *numExtensions = numRequiredExtensions;
    return returnExtensions;
}

#endif //VULKANEXTENSIONS_H


