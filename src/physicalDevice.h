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



static u8 _check_device_extension_support(VkPhysicalDevice device) {
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

static u8 _is_device_suitable(const VkPhysicalDevice device,const VkSurfaceKHR surface) {
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


static void pick_physical_device(const VkInstance instance,PhysicalDevice* device,const VkSurfaceKHR surface) {
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
#endif //PHYSICALDEVICE_H
