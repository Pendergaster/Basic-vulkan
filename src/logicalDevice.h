/************************************************************
* Check license.txt in project root for license information *
*********************************************************** */

#ifndef LOGICALDEVICE_H
#define LOGICALDEVICE_H
#include <vulkan/vulkan.h>
#include "utils.h"
#include "validationLayers.h"
#include "physicalDevice.h"
#include "vulkanExtensions.h"
#include "swapchain.h"
#include "pipeline.h"

// Store all needed data about Logical device
typedef struct LogicalDevice {
	VkDevice	device;
	VkQueue		graphicsQueue;
	VkQueue		presentQueue;
	SwapChain	swapchain;
} LogicalDevice;

static void inline _set_queues(LogicalDevice* device,const QueueFamilyIndices indices) {
	vkGetDeviceQueue(device->device, indices.graphicsFamily, 0, &device->graphicsQueue);
	vkGetDeviceQueue(device->device, indices.presentFamily, 0, &device->presentQueue);
}

static void logicaldevice_init(const PhysicalDevice* physicalDevice, LogicalDevice* device) {
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
	if (vkCreateDevice(physicalDevice->physicalDevice, &createInfo, NULL, &device->device) != VK_SUCCESS) {
		ABORT("Failed to create device");
	}
	// set proper queues
	_set_queues(device,physicalDevice->queues);
}

static void inline logicalDevice_dispose(LogicalDevice* device) {
	swapchain_dispose(&device->swapchain,device->device);
	// device queues are automaticly disposed when device is disposed
	vkDestroyDevice(device->device, NULL);
	LOG("Disposed logicaldevice");
}

#endif //LOGICALDEVICE_H


