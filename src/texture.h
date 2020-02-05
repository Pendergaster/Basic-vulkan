/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef TEXTURE_H
#define TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"


typedef struct Texture {
    VkImage         image;
    VkDeviceMemory  memory;
} Texture;

static u8*
_load_texture(const char* path, i32* width, i32* height) {

    i32 channels;
    stbi_uc* data = stbi_load(path, width, height, &channels, STBI_rgb_alpha);
    if(!data) {
        ABORT("Failed to load texture %s", path);
    }

    return data;
}

static Texture
textures_create(VkPhysicalDevice physicalDevice, VkDevice device) {

    Texture ret;

    // Load texture
    i32 width, height;
    u8* data = _load_texture("textures/statue.jpg", &width, &height);
    VkDeviceSize size = width * height * 4;

    // create buffer and copy data to image
    Buffer stagingBuffer = buffer_create(physicalDevice, device, size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // usage
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); // memrequiremets

    // Copy pixel data to staging buffer
    void* stagingDst;
    vkMapMemory(device, stagingBuffer.bufferMemory, 0, size, 0, &stagingDst);
    memcpy(stagingDst, data, size);
    vkUnmapMemory(device, stagingBuffer.bufferMemory);

    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.extent.width = (u32)width;
    info.extent.height = (u32)height;
    info.extent.depth = 1;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.format = VK_FORMAT_R8G8B8A8_UNORM;
    // Pixels are laid out in an implementation defined order, in this mode we cannot access pixels later
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    // only used by one queue family
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // Multisampling
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.flags = 0;

    stbi_image_free(data);
    return ret;
}

#endif /* TEXTURE_H */
