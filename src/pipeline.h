/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>
#include "utils.h"
#include "fileutils.h"

typedef struct Pipeline {

} Pipeline;

VkShaderModule shadermodule_create(const u8* src,const size_t size, const VkDevice device) {
	VkShaderModule ret = 0;

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = (const u32*)src;

	if (vkCreateShaderModule(device, &createInfo, NULL, &ret) != VK_SUCCESS) {
		ABORT("Failed to create shader module");
	}

	return ret;
}

static void pipeline_init(Pipeline* pipeline, const VkDevice device) {
	size_t vertSize = 0;
	u8* vert_shader = load_binary_file("shaders/basic_shader_vert.spv",&vertSize);
	size_t fragSize = 0;
	u8* frag_shader = load_binary_file("shaders/basic_shader_frag.spv",&fragSize);
	if(vert_shader == NULL || frag_shader == NULL) {
		ABORT("Failed to load shaders");
	}

	VkShaderModule vertMod = shadermodule_create(vert_shader,vertSize,device);
	VkShaderModule fragMod = shadermodule_create(frag_shader,fragSize,device);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	{ // init vert shader stage info
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

		vertShaderStageInfo.module = vertMod;
		vertShaderStageInfo.pName = "main";

	}

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	{ // init vert shader stage info
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		fragShaderStageInfo.module = fragMod;
		fragShaderStageInfo.pName = "main";
	}

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};



	// Clean things up
	vkDestroyShaderModule(device, vertMod, NULL);
	vkDestroyShaderModule(device, fragMod, NULL);
	free(vert_shader);
	free(frag_shader);
}

#endif /* PIPELINE_H */
