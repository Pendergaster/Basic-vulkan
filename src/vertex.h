/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef VERTEX_H
#define VERTEX_H
#include <vulkan/vulkan.h>
#include "buffer.h"
#include "cmath.h"


typedef struct Vertex {
    vec2    pos;
    vec3    color;
} Vertex;

typedef struct VertexData {
    Buffer vertex;
    Buffer index;
} VertexData;

static const Vertex Triangle[] = {
    {.pos = {0.0f ,-0.5f }, .color = {1.f , 0.f, 0.f}},
    {.pos = {0.5f , 0.5f }, .color = {0.2f , 0.2f, 0.2f}},
    {.pos = {-0.5f, 0.5f }, .color = {0.f , 0.f, 1.f}},
};

static const Vertex Rectangle[] = {
    {.pos = {-0.5f, -0.5f}, .color = {1.0f, 0.0f, 0.0f}},
    {.pos = {0.5f, -0.5f},  .color = {0.0f, 1.0f, 0.0f}},
    {.pos = {0.5f, 0.5f},   .color = {0.0f, 0.0f, 1.0f}},
    {.pos = {-0.5f, 0.5f},  .color = {1.0f, 1.0f, 1.0f}}
};

static const u32 RectangleIndexes[] = {
    0, 1, 2, 2, 3, 0
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

typedef struct TriangleAttributeDescription {
    VkVertexInputAttributeDescription pos;
    VkVertexInputAttributeDescription color;
} TriangleAttributeDescription;

static TriangleAttributeDescription triangle_get_attribute_descriptions(){
    TriangleAttributeDescription desc = {};
    desc.pos.binding = 0;
    // Shader slot 1
    desc.pos.location = 0;
    // vec2
    desc.pos.format = VK_FORMAT_R32G32_SFLOAT;
    // Offset in struct
    desc.pos.offset = offsetof(Vertex, pos);

    desc.color.binding = 0;
    // Shader slot 2
    desc.color.location = 1;
    // vec3
    desc.color.format = VK_FORMAT_R32G32B32_SFLOAT;
    // Offset in struct
    desc.color.offset = offsetof(Vertex, color);

    return desc;
};


static void
vertexdata_init(VertexData* data, VkDevice device, VkPhysicalDevice physicalDevice,
        VkCommandPool pool, VkQueue graphicsque) {

    //Crate vertex buffer

    void* memData;
    u32 vertexSize = sizeof *Rectangle * SIZEOF_ARRAY(Rectangle);
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
    memcpy(memData, Rectangle, vertexSize);
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
    u32 indexSize = sizeof *RectangleIndexes * SIZEOF_ARRAY(RectangleIndexes);
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
    memcpy(memData, RectangleIndexes, indexSize);
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
