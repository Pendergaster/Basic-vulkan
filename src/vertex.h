/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef VERTEX_H
#define VERTEX_H
#include <vulkan/vulkan.h>
#include "buffer.h"
#include "cmath.h"
#include "objload.h"

typedef struct VertexData {
    Buffer  vertex;
    Buffer  index;
    u32     numIndexes;
} VertexData;

static const Vertex Rectangle[] = {
    {.pos = {-0.5f, -0.5f, 0}, .color = {1.0f, 0.0f, 0.0f}, .uv = {1.0f, 0.0f}},
    {.pos = {0.5f, -0.5f, 0},  .color = {0.0f, 1.0f, 0.0f}, .uv = {0.0f, 0.0f}},
    {.pos = {0.5f, 0.5f, 0},   .color = {0.0f, 0.0f, 1.0f}, .uv = {0.0f, 1.0f}},
    {.pos = {-0.5f, 0.5f, 0},  .color = {1.0f, 1.0f, 1.0f}, .uv = {1.0f, 1.0f}},

    {.pos = {-0.5f, -0.5f, -0.5f}, .color =  {1.0f, 0.0f, 0.0f}, .uv = {0.0f, 0.0f}},
    {.pos = {0.5f, -0.5f, -0.5f},  .color =  {0.0f, 1.0f, 0.0f}, .uv = {1.0f, 0.0f}},
    {.pos = {0.5f, 0.5f, -0.5f},   .color =  {0.0f, 0.0f, 1.0f}, .uv = {1.0f, 1.0f}},
    {.pos = {-0.5f, 0.5f, -0.5f},  .color =  {1.0f, 1.0f, 1.0f}, .uv = {0.0f, 1.0f}}
};

static const u32 RectangleIndexes[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

static VkVertexInputBindingDescription
triangle_get_binding_description() {
    VkVertexInputBindingDescription ret = {};
    // index of the binding in the array of bindings
    ret.binding = 0;
    // One vertex attributes size
    ret.stride = sizeof(Vertex);
    // VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
    // VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
    ret.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return ret;
}

typedef struct VertexAttributeDescription {
    VkVertexInputAttributeDescription pos;
    VkVertexInputAttributeDescription color;
    VkVertexInputAttributeDescription uv;
} VertexAttributeDescription;

static VertexAttributeDescription vertex_get_attribute_descriptions(){
    VertexAttributeDescription desc = {};
    desc.pos.binding = 0;
    // Shader slot 1
    desc.pos.location = 0;
    // vec2
    desc.pos.format = VK_FORMAT_R32G32B32_SFLOAT;
    // Offset in struct
    desc.pos.offset = offsetof(Vertex, pos);

    desc.color.binding = 0;
    // Shader slot 2
    desc.color.location = 1;
    // vec3
    desc.color.format = VK_FORMAT_R32G32B32_SFLOAT;
    // Offset in struct
    desc.color.offset = offsetof(Vertex, color);

    desc.uv.binding = 0;
    // Shader slot 2
    desc.uv.location = 2;
    // vec3
    desc.uv.format = VK_FORMAT_R32G32_SFLOAT;
    // Offset in struct
    desc.uv.offset = offsetof(Vertex, uv);

    return desc;
};


static void
vertexdata_init(VertexData* data, VkDevice device, VkPhysicalDevice physicalDevice,
        VkCommandPool pool, VkQueue graphicsque) {

    //Crate vertex buffer

    VertexLoadData verts = obj_load("models/chalet.obj");
    data->numIndexes = verts.numIndexes;
    void* memData;
    u32 vertexSize = sizeof *verts.vertexes * verts.numVertexes;
    Buffer stagingBuffer = {};

    // use staging buffer as source buffer and move its data to actual vertexbuffer as source buffer
    stagingBuffer = buffer_create(physicalDevice, device, vertexSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // usage
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT// properties
            );

    // Retrieve data pointer to start and copy
    vkMapMemory(device, stagingBuffer.bufferMemory,
            0, //offset
            vertexSize,
            0, // memorymap flags
            &memData);
    memcpy(memData, verts.vertexes, vertexSize);
    // unmapping starts copying mempry to buffer
    vkUnmapMemory(device, stagingBuffer.bufferMemory);

    // create source buffer
    data->vertex = buffer_create(physicalDevice, device,
            vertexSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, // usage
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // copy data immidiately after unmap
    // Copy data from staging buffer to vertex buffer

    buffer_copy(&stagingBuffer, &data->vertex, device, pool, graphicsque);

    buffer_dispose(&stagingBuffer, device);

    // Create index buffer
    u32 indexSize = sizeof *verts.indexes * verts.numIndexes;
    stagingBuffer = buffer_create(physicalDevice, device,
            indexSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // usage
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT// properties
            );

    vkMapMemory(device, stagingBuffer.bufferMemory,
            0, //offset
            indexSize,
            0, // memorymap flags
            &memData);
    memcpy(memData, verts.indexes , indexSize);
    vkUnmapMemory(device, stagingBuffer.bufferMemory);

    data->index = buffer_create(physicalDevice, device,
            indexSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT , // usage
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT// properties
            );

    buffer_copy(&stagingBuffer, &data->index, device, pool, graphicsque);

    buffer_dispose(&stagingBuffer, device);
}

static void
vertexdata_dispose(VertexData *data, VkDevice device) {
    buffer_dispose(&data->vertex, device);
    buffer_dispose(&data->index, device);
}

#endif /* VERTEX_H */
