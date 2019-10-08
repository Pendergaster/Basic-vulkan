/************************************************************
* Check license.txt in project root for license information *
*********************************************************** */

#ifndef VULKAN_CONTEXT
#define VULKAN_CONTEXT

#include <vulkan/vulkan.h>
#include "utils.h"
#include "vulkanExtensions.h"
#include "validationLayers.h"
#include "physicalDevice.h"
#include "logicalDevice.h"

// Vulkan context is heart/start of vulkan application
typedef struct VulkanContext {
    VkInstance                  instance;
    VkDebugUtilsMessengerEXT    debugMessenger;
    PhysicalDevice              physicalDevice;
    VkSurfaceKHR                surface;
} VulkanContext;



static void vulkancontext_init(VulkanContext* context) {

    // if validation layer are enabled check that they are supported
    if(enableValidationLayers && !check_validation_layer_support()) {
        ABORT("Failed to enable validation layers");
    }

    // Info about application
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Opari";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Custom";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Info about instance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    // get GLFWs required extensions
    u32 numExtensions = 0;
    const char** extensions = extensions_get_required(&numExtensions);
    createInfo.ppEnabledExtensionNames =  extensions;
    createInfo.enabledExtensionCount = numExtensions;
    // number of enabled validation layers
    createInfo.enabledLayerCount = SIZEOF_ARRAY(validationLayers);
    createInfo.ppEnabledLayerNames = validationLayers;
#if 0
    {
        // TODO functio?
        // validate extensions
        // first query how many extensions we have
        u32 extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
        VkExtensionProperties* extensionsAVA = (VkExtensionProperties*)malloc(extensionCount * sizeof(VkExtensionProperties));

        // second query available ones
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionsAVA);

        for(u32 i = 0; i < extensionCount; i++) {
            char* name = extensionsAVA[i].extensionName;
            LOG("avaivable extensions %s ",name);
        }
        free(extensionsAVA);
    }
#endif

    // debug info for instance creation and destruction because validation layers will be destroyed in this time
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
    if (enableValidationLayers) {
        init_debug_util_create_info(&debugInfo);
        createInfo.pNext = &debugInfo;
    }
    // Create instance
    if (vkCreateInstance(&createInfo, NULL, &context->instance) != VK_SUCCESS) {

        ABORT("failed to create instance");
    }
    LOG("Vulkan instance created");
    free(extensions);
}

static void vulkancontext_dispose(VulkanContext* context) {

    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    LOG("Surface disposed");
    if(enableValidationLayers) {
        dispose_debug_messenger(context->instance,context->debugMessenger);
        LOG("Debug messenger disposed");
    }

    vkDestroyInstance(context->instance, NULL);
    LOG("Instance disposed");
}

#endif //VULKAN_CONTEXT
