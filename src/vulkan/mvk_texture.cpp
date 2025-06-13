#include "mvk_core.h"

namespace Mvk {
    void createImage(Context& context, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImage& image, VmaAllocation& imageAllocation) {
        VkImageCreateInfo imageCreateInfo { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = static_cast<uint32_t>(width);
        imageCreateInfo.extent.height = static_cast<uint32_t>(height);
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = usage;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        VmaAllocationCreateInfo allocInfo {};
        allocInfo.usage = memoryUsage;
        
        // ADD THIS: For CPU-accessible memory, you need the MAPPED flag
        if (memoryUsage == VMA_MEMORY_USAGE_CPU_TO_GPU || memoryUsage == VMA_MEMORY_USAGE_CPU_ONLY) {
            allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        if (vmaCreateImage(context.allocatorVMA, &imageCreateInfo, &allocInfo, &image, &imageAllocation, 0) != VK_SUCCESS) {
            THROW("failed to create image!");
        }
    }

    void transitionImageLayout(Context& context, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);
                
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!\n");
        }


        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(context, commandBuffer);
    }

    void copyBufferToImage(Context& context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endSingleTimeCommands(context, commandBuffer);
    }
    
    void createTextureImageView(Context& context) {
        createImageView(context, context.textureImage, VK_FORMAT_R8G8B8A8_SRGB, context.textureImageView);
    }

    void createTextureSampler(Context& context, VkSampler& sampler) {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(context.physicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        
        if (vkCreateSampler(context.device, &samplerInfo, 0, &sampler) != VK_SUCCESS) {
            THROW("failed to create texture sampler!");
        }
    }

    void createTextureImage(Context &context, const char *imageFile, VkImage &image, VmaAllocation &allocation, VkImageView &imageView, VkSampler &sampler)
    {
        int width, height, nrChannels;
        stbi_uc* pixels = stbi_load(imageFile, &width, &height, &nrChannels, STBI_rgb_alpha);

        if (!pixels) {
            THROW("failed to load texture image!");
        }

        VkDeviceSize imageSize = width * height * 4;

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        VmaAllocationInfo allocInfo {};
        createBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingBufferAllocation, VMA_ALLOCATION_CREATE_MAPPED_BIT, &allocInfo);

        memcpy(allocInfo.pMappedData, pixels, static_cast<size_t>(imageSize));

        stbi_image_free(pixels);
        
        createImage(context, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, image, allocation);
        
        transitionImageLayout(context, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(context, stagingBuffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

        transitionImageLayout(context, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
        vmaDestroyBuffer(context.allocatorVMA, stagingBuffer, stagingBufferAllocation);

        createImageView(context, image, VK_FORMAT_R8G8B8A8_SRGB, imageView);

        createTextureSampler(context, sampler);
    }

    void createTextureFromData(Context& context, VkImage& image, VmaAllocation& allocation, VkImageView& imageView, VkSampler& sampler, uint8_t* pixels, int width, int height) {
        VkDeviceSize imageSize = width * height * 4;

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        VmaAllocationInfo allocInfo {};
        createBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingBufferAllocation, VMA_ALLOCATION_CREATE_MAPPED_BIT, &allocInfo);

        memcpy(allocInfo.pMappedData, pixels, static_cast<size_t>(imageSize));

        delete[] pixels;
        
        createImage(context, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, image, allocation);
        
        transitionImageLayout(context, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(context, stagingBuffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

        transitionImageLayout(context, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
        vmaDestroyBuffer(context.allocatorVMA, stagingBuffer, stagingBufferAllocation);

        createImageView(context, image, VK_FORMAT_R8G8B8A8_SRGB, imageView);

        createTextureSampler(context, sampler);
    }

    
    void createTexture(Context& context, const int width, const int height) { 
        VkDeviceSize imageSize = width * height * 4;

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        VmaAllocationInfo allocInfo {};
        createBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingBufferAllocation, VMA_ALLOCATION_CREATE_MAPPED_BIT, &allocInfo);
        
        createImage(context, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, context.textureImage, context.textureImageAllocation);
        
        transitionImageLayout(context, context.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(context, stagingBuffer, context.textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

        transitionImageLayout(context, context.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
        vmaDestroyBuffer(context.allocatorVMA, stagingBuffer, stagingBufferAllocation);

        createTextureImageView(context);

        createTextureSampler(context, context.textureImageSampler);
    }

    void updateTextureImageDataDynamic(Context& context, void*& pixelData, const int width, const int height) {
        VkDeviceSize imageSize = width * height * 4;

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        VmaAllocationInfo allocInfo {};

        createBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingBufferAllocation, VMA_ALLOCATION_CREATE_MAPPED_BIT, &allocInfo);
        
        {
            std::lock_guard<std::mutex> lock(frameDataMutex);
            if (pixelData != nullptr)
                memcpy(allocInfo.pMappedData, pixelData, static_cast<size_t>(imageSize));
        }

        transitionImageLayout(context, context.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(context, stagingBuffer, context.textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

        transitionImageLayout(context, context.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vmaDestroyBuffer(context.allocatorVMA, stagingBuffer, stagingBufferAllocation);
    }
}