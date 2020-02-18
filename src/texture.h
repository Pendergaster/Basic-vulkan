/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef TEXTURE_H
#define TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "commandBuffer.h"
#include "imageview.h"
#include "physicalDevice.h"

typedef enum TextureType {
    sampledTexture,
    depthTexture,
} TextureType;

typedef struct Texture {
    VkImage         image;
    VkDeviceMemory  memory;
    u32             width, height;
    VkImageView     view;
    VkSampler       sampler;
    TextureType     type;
} Texture;

static u8*
_load_texture_data(const char* path, u32* width, u32* height) {

    i32 channels, _width, _height;
    stbi_uc* data = stbi_load(path, &_width, &_height, &channels, STBI_rgb_alpha);
    if(!data) {
        ABORT("Failed to load texture %s", path);
    }
    *width = _width;
    *height = _height;

    return data;
}

static void
_texture_copy_from_buffer(VkDevice device, VkCommandPool pool, VkBuffer buffer,
        const Texture *tex, VkQueue graphicsQue) {

    VkCommandBuffer cmd = commandbuffer_begin_single_time(device, pool);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // Mipmap data
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){.width = tex->width, .height = tex->height, 1};

    vkCmdCopyBufferToImage(cmd, buffer, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    commandbuffer_end_single_time(device, cmd, pool, graphicsQue);
}

static void
_texture_to_layout(VkDevice device, VkCommandPool pool, VkQueue graphicsQue,
        VkImage image, VkImageLayout old, VkImageLayout new) {

    VkCommandBuffer cmd = commandbuffer_begin_single_time(device, pool);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old;
    barrier.newLayout = new;
    // Dont change queue ownership
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // mipmap data
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.baseArrayLayer = 0;

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if(old == VK_IMAGE_LAYOUT_UNDEFINED && new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        ABORT("Unimplemented imagelayout");
    }

    barrier.srcAccessMask = 0; //TODO
    barrier.dstAccessMask = 0; //TODO

    vkCmdPipelineBarrier(cmd,
            srcStage, // srcStageMask
            dstStage, // dstStageMask
            0, // dependencyFlags
            0, // memoryBarrierCount
            NULL, // pMemoryBarriers
            0,      // bufferMemoryBarrierCount
            NULL,   // pBufferMemoryBarriers
            1,      // imageMemoryBarrierCount
            &barrier); // pImageMemoryBarriers

    commandbuffer_end_single_time(device, cmd, pool, graphicsQue);
}

static VkSampler
_texture_create_sampler(VkDevice device) {

    VkSampler ret;

    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = VK_FILTER_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.anisotropyEnable = VK_TRUE;
    info.maxAnisotropy = 16;
    info.unnormalizedCoordinates = VK_FALSE;
    info.compareEnable = VK_FALSE;
    info.compareOp = VK_COMPARE_OP_ALWAYS;

    //Mipmap data
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.mipLodBias = 0.0f;
    info.minLod = 0.0f;
    info.maxLod = 0.0f;

    if(vkCreateSampler(device, &info, NULL /*allocator*/, &ret) != VK_SUCCESS) {
        ABORT("Failed to create sampler");
    }

    return ret;
}

static Texture
texture_create(VkPhysicalDevice physicalDevice, VkDevice device, VkFormat format, VkImageUsageFlags usage,
        u32 width, u32 height, TextureType type) {


    Texture ret  = {.height = height, .width = width, .type = type};

    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.extent.width = ret.width;
    info.extent.height = ret.height;
    info.extent.depth = 1;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.format = format;
    // Pixels are laid out in an implementation defined order, in this mode we cannot access pixels later
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usage;
    // only used by one queue family
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // Multisampling info
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.flags = 0;

    if(vkCreateImage(device, &info, NULL /*allocator*/, &ret.image) != VK_SUCCESS) {
        ABORT("Failed to create image");
    }

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, ret.image, &requirements);

    // allocate buffer
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = physicaldevice_find_memorytype(
            physicalDevice, requirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // copy data immidiately after unmap

    if(vkAllocateMemory(device, &allocInfo, NULL /*allocator*/, &ret.memory) != VK_SUCCESS) {
        ABORT("Failed to allocate buffer");
    }

    // Bind memory to buffer
    vkBindImageMemory(device, ret.image, ret.memory, 0 /* device size memory offset */);

    return ret;
}

static Texture
texture_load_and_create(const char* path, VkPhysicalDevice physicalDevice, VkDevice device,
        VkCommandPool pool, VkQueue graphicsQue) {

    u32 width,height;

    // Load texture
    u8* data = _load_texture_data(path, &width, &height);
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

    Texture ret = texture_create(physicalDevice, device,
            VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            width, height, sampledTexture);


    // transfer layout
    _texture_to_layout(device, pool, graphicsQue, ret.image,
            VK_IMAGE_LAYOUT_UNDEFINED, //old layout
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); //target layout

    _texture_copy_from_buffer(device, pool, stagingBuffer.bufferId, &ret, graphicsQue);

    // sampling layout
    _texture_to_layout(device, pool, graphicsQue, ret.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //old layout
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); //target layout

    ret.view = imageview_create(ret.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT,device);
    ret.sampler = _texture_create_sampler(device);

    buffer_dispose(&stagingBuffer, device);
    stbi_image_free(data);
    return ret;
}

static void
texture_dispose(Texture* tex, VkDevice device) {

    imageview_dispose(tex->view, device);
    if(tex->type == sampledTexture) {
        vkDestroySampler(device, tex->sampler, NULL);
    }
    vkDestroyImage(device, tex->image, NULL /*allocator*/);
    vkFreeMemory(device, tex->memory, NULL /*allocator*/);
    memset(tex, 0, sizeof *tex);
}

static u32
_format_has_stencil(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

static Texture
texture_depth_create(VkPhysicalDevice physicalDevice, VkDevice device, VkExtent2D swapExtent) {


    VkFormat format = physicaldevice_find_depth_format(physicalDevice);

    Texture ret = texture_create(physicalDevice, device,
            format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            swapExtent.width, swapExtent.height, depthTexture);

    ret.view = imageview_create(ret.image, format, VK_IMAGE_ASPECT_DEPTH_BIT, device);

    return ret;
}



// TODO muuta image nimi
// TODO remove create view
// TODO move view to shader


#endif /* TEXTURE_H */
