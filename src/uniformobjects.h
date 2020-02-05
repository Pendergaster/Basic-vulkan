/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */
#ifndef UNIFORMOBJECTS_H
#define UNIFORMOBJECTS_H

#include <vulkan/vulkan.h>
#include "utils.h"
#include "buffer.h"
#include "swapchain.h"

typedef struct UniformObject {
    VkDescriptorSetLayout   uboLayout;
    struct {
        mat4                    model;
        mat4                    view;
        mat4                    projection;
    } data;
} UniformObject;

static void
uniformobject_init(UniformObject *object, VkDevice device) {

    // Where object is bound
    VkDescriptorSetLayoutBinding uboBinding = {};
    uboBinding.binding = 0; // 0 position
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    // only accessed from vertex shader
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // Create object
    VkDescriptorSetLayoutCreateInfo layout = {};
    layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout.bindingCount = 1;
    layout.pBindings = &uboBinding;

    if(vkCreateDescriptorSetLayout(device, &layout,
                NULL /*allocator*/,
                &object->uboLayout) != VK_SUCCESS) {
        ABORT("Failed to create descriptor layouts!");
    }
    identify_mat4(&object->data.model);
    identify_mat4(&object->data.view);
    identify_mat4(&object->data.projection);

    const float FOV = 90;
    //TODO g_ prefix
    vec3 eye = {2.f,3.f,2.f};
    vec3 target = {0.f,0.f,0.f};
    create_lookat_mat4(&object->data.view, eye, target, world_up);
    perspective(&object->data.projection, FOV * deg2rad,
            (float)SCREENWIDTH / (float)SCREENHEIGHT, 0.1f, 10.f);

    //object->data.projection.mat[1][1] *= -1;
}

static void
uniformobject_dispose(UniformObject *object, VkDevice device) {
    vkDestroyDescriptorSetLayout(device, object->uboLayout, NULL /*allocator*/);
    memset(object, 0, sizeof *object);
}

//typedef struct UniformBuffers {
//    Buffer*  uniformBuffers;
//} UniformBuffers;

static Buffer*
uniformbuffers_create(u32 numImages, VkDevice device, VkPhysicalDevice physicalDevice) {

    // create buffer for each swapchain image
    Buffer* uniformBuffers = malloc(sizeof(Buffer) * numImages);
    u32 size = MEMBER_SIZE(UniformObject, data);

    for(u32 i = 0; i < numImages; i++) {
        uniformBuffers[i] = buffer_create(physicalDevice, device, size,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //usage
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT); // properties
    }

    return uniformBuffers;
}

static void
uniformbuffer_dispose(Buffer** buffers, u32 numImages, VkDevice device) {

    for(u32 i = 0; i < numImages; i++) {
        buffer_dispose(&(*buffers)[i], device);
    }
    free(*buffers);
}

static void
uniformbuffer_update(Buffer* buffer, UniformObject* object, VkDevice device) {

    double time = glfwGetTime();
#if 0
    float y =  time * 0.2f;
    if(y > 6.f) {
        y = 0;
    }
    vec3 eye = {2.f,-3.f + y,2.f};
#else
    vec3 eye = {2.f, 0 ,0.f};
#endif
    vec3 target = {0.f,0.f,0.f};
    create_lookat_mat4(&object->data.view, eye, target, world_up);


    //rotate the mesh
    vec3 axis = {0.f, 1.f, 0};
    normalize_inside_vec3(&axis);
    const quat rotation = quat_from_axis(axis, time * 0.1);
    mat4_from_quat(&object->data.model, rotation);
    void *data;
    vkMapMemory(device, buffer->bufferMemory, 0, MEMBER_SIZE(UniformObject, data), 0, &data);
    memcpy(data, &object->data, MEMBER_SIZE(UniformObject, data));
    vkUnmapMemory(device, buffer->bufferMemory);
}

static VkDescriptorPool
descriptorpool_create(u32 numImages, VkDevice device) {

    // Size of the pool (one desc per frame)
    VkDescriptorPoolSize size = {};
    size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    size.descriptorCount = numImages;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &size;
    // Max desc to allocate
    poolInfo.maxSets = numImages;

    // Create the pool
    VkDescriptorPool ret;
    if(vkCreateDescriptorPool(device, &poolInfo, NULL /*allocator*/, &ret) != VK_SUCCESS) {
        ABORT("Failed to create descriptoy pool");
    }
    return ret;
}

static void
descriptorpool_dispose(VkDescriptorPool pool, VkDevice device) {
    vkDestroyDescriptorPool(device, pool, NULL /*allocator*/);
}

static VkDescriptorSet*
descriptorsets_create(VkDescriptorPool pool, VkDevice device,
        u32 numImages, VkDescriptorSetLayout layout, Buffer* uniformBuffers) {

    VkDescriptorSet* ret = malloc(sizeof(VkDescriptorSet) * numImages);
    VkDescriptorSetLayout* layouts = malloc(sizeof(VkDescriptorSetLayout) * numImages);
    for(u32 i = 0; i < numImages; i++) {
        layouts[i] = layout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = numImages;
    // Create one descriptorset per swapchain image
    allocInfo.pSetLayouts = layouts;

    if(vkAllocateDescriptorSets(device, &allocInfo, ret) != VK_SUCCESS) {
        ABORT("Failed to allocate description sets");
    }

    // populate descs
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.range = MEMBER_SIZE(UniformObject, data);

    // write information
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufferInfo;

    // for each set
    for(u32 i = 0; i < numImages; i++) {
        bufferInfo.buffer = uniformBuffers[i].bufferId;
        write.dstSet = ret[i];

        vkUpdateDescriptorSets(device, 1, &write, 0 /*copy count*/, NULL /*copies*/);
    }

    free(layouts);
    return ret;
}

static void
descriptorsets_dispose(VkDescriptorSet* sets) {

    // no need to free sets because the pool is already disposed
    free(sets);
}

#endif /* UNIFORMOBJECTS_H */
