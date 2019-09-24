/************************************************************
* Check license.txt in project root for license information *
*********************************************************** */

#ifndef VALIDATIONLAYERS_H
#define VALIDATIONLAYERS_H

#include <vulkan/vulkan.h>
#include <string.h>
#include "utils.h"
#if BUILD_DEBUG
static const u8 enableValidationLayers = 1;
const char* validationLayers [] = {
	"VK_LAYER_KHRONOS_validation"
};

#else
static const u8 enableValidationLayers = 0;
const char* validationLayers[0];
#endif

static u8 check_validation_layer_support() {
	u32 layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(layerCount * sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	// Compare our wanted layers against available
	for(u32 i = 0; i < SIZEOF_ARRAY(validationLayers); i++) {
		const char* wantedLayerName = validationLayers[i];
		u8 found = 0;
		for (u32 i2 = 0; i2 < layerCount; i2++) {
			char* availableLayerName = availableLayers[i2].layerName;
			if(!strcmp(wantedLayerName,availableLayerName)) {
				// Layer found and we can continue
				found = 1;
				break;
			}
		}
		// Layer not found and we can return 0 or abort
		if (!found) {
			ABORT("Failed to enable %s validationlayer!", wantedLayerName);
			return 0;
		}
	}

	free(availableLayers);
	LOG("Validated Vulkan layers");
	return 1;
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
	(void)pUserData;
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		// Message is important enough to show
		switch(messageType) {
			case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
				LOG_COLOR_ERR(CONSOLE_COLOR_RED,"VALIDATION LAYER EVENT MESSAGE : %s\n", pCallbackData->pMessage );
				break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:

				LOG_COLOR_ERR(CONSOLE_COLOR_RED,"VALIDATION LAYER ERROR MESSAGE : %s\n", pCallbackData->pMessage );
				break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
				LOG_COLOR_ERR(CONSOLE_COLOR_RED,"VALIDATION LAYER ERROR MESSAGE : %s\n", pCallbackData->pMessage );
				break;
			default:
				LOG_COLOR_ERR(CONSOLE_COLOR_RED,"VALIDATION LAYER UNKNOWN MESSAGE : %s\n", pCallbackData->pMessage );

		}
	}
	return VK_FALSE;
}

// loaded function to destroy messenger
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = NULL;

static void inline init_debug_util_create_info(VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
	// What kind of struct this is
	createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo->pfnUserCallback = debugCallback;
	createInfo->pUserData = NULL; // Optional
}

static void init_debug_messenger(VkInstance instance,VkDebugUtilsMessengerEXT*	debugMessenger) {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	init_debug_util_create_info(&createInfo);
	// Load the extension
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	ASSERT_MESSAGE(func,"FAILED TO LOAD THE DEBUG MESSENGER EXTENSION");

	if(func(instance, &createInfo, NULL /*Allocator*/, debugMessenger) != VK_SUCCESS) {
		ASSERT_MESSAGE(func,"FAILED TO CREATE THE DEBUG MESSENGER EXTENSION");
	}
	// load destroy function
	vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	ASSERT_MESSAGE(func,"FAILED TO LOAD THE DEBUG MESSENGER DISPOSING EXTENSION");
}

static void inline dispose_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger) {
	ASSERT_MESSAGE(vkDestroyDebugUtilsMessenger ,"FUNCTION IS NOT LOADED");
	vkDestroyDebugUtilsMessenger(instance,debugMessenger,NULL /*Allocator*/);
}



#endif //VALIDATIONLAYERS_H
