/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef PHYSICALDEVICE_H
#define PHYSICALDEVICE_H
#include <vulkan/vulkan.h>
#include "utils.h"
#include "queueIndexes.h"
#include "vulkanExtensions.h"
#include "swapchain.h"

// Store all needed data about physical device
typedef struct PhysicalDevice {
    VkPhysicalDevice            physicalDevice;
    QueueFamilyIndices          queues;
} PhysicalDevice;

static u8
_check_device_extension_support(VkPhysicalDevice device) {
    // query avaivable extensions and compare to required ones
    u32 extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties* availableExtensions =
        (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionCount);

    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    for(u32 i = 0; i < SIZEOF_ARRAY(g_extensionNames); i++) {
        u8 found = 0;
        for(u32 i2 = 0; i2 < extensionCount; i2++) {
            if(!strcmp(g_extensionNames[i],availableExtensions[i2].extensionName)) {
                found = 1;
                break;
            }
        }
        if(!found) {
            free(availableExtensions);
            return 0;
        }
    }

    free(availableExtensions);
    return 1;
}

static u8
_is_device_suitable(const VkPhysicalDevice device,const VkSurfaceKHR surface) {
    // get device properties (like discreate GPU or integrated GPU)
    // and what features GPU has (like geometry shaders)
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    QueueFamilyIndices families = _find_queue_families(device,surface);

    u8 extensionsSupported = _check_device_extension_support(device);
    u8 swapChainSupported = 0;
    if (extensionsSupported) {
        SwapchainSupportDetails swapchainSupport = swapchain_get_support_details(device,surface);
        swapChainSupported = swapchainSupport.numFormats && swapchainSupport.numPresentationModes;
        swapchain_dispose_support_details(&swapchainSupport);
    }

    // integrated or discreate and can use geometry shader
    // has graphics queue family
    return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
            deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
        deviceFeatures.geometryShader &&
        extensionsSupported &&
        swapChainSupported &&
        _verify_queueFamilyIndices(&families);
}


static void
physical_device_pick(const VkInstance instance, PhysicalDevice* device, const VkSurfaceKHR surface) {
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

    ASSERT_MESSAGE(deviceCount,"No suitable devices!");

    VkPhysicalDevice* devices = (VkPhysicalDevice*) malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    // check all devices and pick the first suitable
    // (TODO here we could pick best suitable also but i wont be developing with multiple gpus)
    for(u32 i = 0; i < deviceCount; i++) {
        if(_is_device_suitable(devices[i],surface)) {
            selectedDevice = devices[i];
            break;
        }
    }

    ASSERT_MESSAGE(VK_NULL_HANDLE != selectedDevice,"failed to select physical device!");
    device->physicalDevice = selectedDevice;
    device->queues = _find_queue_families(selectedDevice,surface);
    free(devices);
}

static VkDevice
physicaldevice_create_logicaldevice(const PhysicalDevice* physicalDevice) {

    // if present and graphics queues are not same we need to create two separate
    VkDeviceQueueCreateInfo queueCreateInfos[(sizeof(QueueFamilyIndices)) / (sizeof(u32))] = {};
    // get unique indexes
    u32 uniqueIndexes[(sizeof(QueueFamilyIndices)) / (sizeof(u32))] = {};
    const u32* inputIndexes = (const u32*)&physicalDevice->queues;
    u32 numIndexes = (sizeof(QueueFamilyIndices)) / (sizeof(u32));
    memcpy(uniqueIndexes,inputIndexes,sizeof(QueueFamilyIndices));

    // remove all non unique indexes from set
    for(u32 i = 0; i < numIndexes; i++) {
        for(u32 i2 = (i + 1); i2 < numIndexes; i2++) {
            if(uniqueIndexes[i] == uniqueIndexes[i2]) {
                // remove i2 index
                numIndexes -= 1;
                uniqueIndexes[i2] = uniqueIndexes[numIndexes];
                break;
            }
        }
    }

    float queuePriority = 1.0f;
    for(u32 i = 0; i < numIndexes; i++) {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = uniqueIndexes[i];
        queueCreateInfos[i].queueCount = 1;

        // queues priority 0.0 - 1.0
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    // specify what device features we are using
    VkPhysicalDeviceFeatures deviceFeatures = {};
    LOG("initialized %d unique queue(s), graphics queue %d and presentation queue %d",
            numIndexes,physicalDevice->queues.graphicsFamily,physicalDevice->queues.presentFamily);

    // logical devices create info
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = numIndexes;
    createInfo.pEnabledFeatures = &deviceFeatures;
    // specify validation layers is not nessecery anymore but good habbit for older implementations
    createInfo.enabledLayerCount = SIZEOF_ARRAY(validationLayers);
    createInfo.ppEnabledLayerNames = validationLayers;
    // enabled extension
    createInfo.ppEnabledExtensionNames = g_extensionNames;
    createInfo.enabledExtensionCount = SIZEOF_ARRAY(g_extensionNames);

    VkDevice device;
    if (vkCreateDevice(physicalDevice->physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS) {
        ABORT("Failed to create device");
    }
    return device;
}


#endif //PHYSICALDEVICE_H
