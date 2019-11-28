/************************************************************
* Check license.txt in project root for license information *
*********************************************************** */

#ifndef QUEUINDEXES_H
#define QUEUINDEXES_H

#include <vulkan/vulkan.h>
#include "utils.h"

#define QUEUE_NOT_EXISTING numeric_max_u32
// Store all needed queues
typedef struct QueueFamilyIndices {
    u32     graphicsFamily;
    // every device may not support presenting image and this queue will ensure it does
    u32     presentFamily;
} QueueFamilyIndices ;

inline static u8 _verify_queueFamilyIndices(const QueueFamilyIndices* queue) {
    return queue->graphicsFamily != QUEUE_NOT_EXISTING &&
        queue->presentFamily != QUEUE_NOT_EXISTING;
}

QueueFamilyIndices _find_queue_families(const VkPhysicalDevice device,const VkSurfaceKHR surface) {
    QueueFamilyIndices indices = {};
    indices.graphicsFamily = QUEUE_NOT_EXISTING;
    indices.presentFamily = QUEUE_NOT_EXISTING;
    // Logic to find queue family indices to populate struct with

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties* queueFamilies =
        (VkQueueFamilyProperties*)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    // Select one queue that supports graphic operations
    u32 i = 0;
    for (; i < queueFamilyCount; i++) {
        VkBool32 presentSupport = 0;
        // does device support presenting to surface
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        VkQueueFamilyProperties* family = &queueFamilies[i];
        if (family->queueCount > 0 && family->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        if (family->queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }
        if(_verify_queueFamilyIndices(&indices)) {
            break;
        }
    }

    free(queueFamilies);
    return indices;
}

#endif /* QUEUINDEXES_H */
