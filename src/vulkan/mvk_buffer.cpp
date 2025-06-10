#include "mvk_core.h"

namespace Mvk {
    VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {};
        
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);
        
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, uv);
        
        return attributeDescriptions;
    }

    uint32_t findMemoryType(Context& context, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        THROW("failed to find suitable memory type!");
    }

    void createAllocatorVMA(Context& context) {
        VmaVulkanFunctions vulkanFunctions {};
        vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocatorCreateInfo {};
        allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
        allocatorCreateInfo.physicalDevice = context.physicalDevice;
        allocatorCreateInfo.device = context.device;
        allocatorCreateInfo.instance = context.instance;
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

        if (vmaCreateAllocator(&allocatorCreateInfo, &context.allocatorVMA) != VK_SUCCESS) {
            THROW("failed to create vulkan memory allocator");
        }
    }

    void createBuffer(Context& context, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& bufferAllocation, VmaAllocationCreateFlags allocFlags, VmaAllocationInfo* allocInfo) {
        VkBufferCreateInfo bufferInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocCreateInfo {};
        allocCreateInfo.usage = memoryUsage;
        allocCreateInfo.flags = allocFlags;

        if (vmaCreateBuffer(context.allocatorVMA, &bufferInfo, &allocCreateInfo, &buffer, &bufferAllocation, allocInfo) != VK_SUCCESS) {
            THROW("failed to create buffer!");
        }
    }

    VkCommandBuffer beginSingleTimeCommands(Context& context) {
        VkCommandBufferAllocateInfo allocInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = context.commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(Context& context, VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(context.graphicsQueue);

        vkFreeCommandBuffers(context.device, context.commandPool, 1, &commandBuffer);
    }

    void copyBuffer(Context& context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);

        VkBufferCopy copyRegion {};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        
        endSingleTimeCommands(context, commandBuffer);
    }

    void createVertexBuffer(Context& context) {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
        
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;

        VmaAllocationInfo allocInfo {};
        createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferAllocation, VMA_ALLOCATION_CREATE_MAPPED_BIT, &allocInfo);

        memcpy(allocInfo.pMappedData, vertices.data(), static_cast<size_t>(bufferSize));

        createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, context.vertexBuffer, context.vertexBufferAllocation, 0, 0);

        copyBuffer(context, stagingBuffer, context.vertexBuffer, bufferSize);

        vmaDestroyBuffer(context.allocatorVMA, stagingBuffer, stagingBufferAllocation);
    }

    void createIndexBuffer(Context& context) {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;

        VmaAllocationInfo allocInfo {};
        createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferAllocation, VMA_ALLOCATION_CREATE_MAPPED_BIT, &allocInfo);
        
        memcpy(allocInfo.pMappedData, indices.data(), static_cast<size_t>(bufferSize));
        
        createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, context.indexBuffer, context.indexBufferAllocation, 0, 0);
        
        copyBuffer(context, stagingBuffer, context.indexBuffer, bufferSize);

        vmaDestroyBuffer(context.allocatorVMA, stagingBuffer, stagingBufferAllocation);
    }

    void createUniformBuffers(Context& context, size_t count) {
        VkDeviceSize minAlignment = context.physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
        VkDeviceSize alignedSize = sizeof(UniformBufferObject);
        if (minAlignment > 0) {
            alignedSize = (alignedSize + minAlignment - 1) & ~(minAlignment - 1);
        }

        VkDeviceSize bufferSize = count * alignedSize;

        context.uniformBuffers.resize(1);
        context.uniformBuffersAllocation.resize(1);
        context.uniformBuffersMapped.resize(1);

        for (size_t i {0}; i < MAX_FRAMES_IN_FLIGHT; i++) {
            context.uniformBuffers[0].resize(MAX_FRAMES_IN_FLIGHT);
            context.uniformBuffersAllocation[0].resize(MAX_FRAMES_IN_FLIGHT);
            context.uniformBuffersMapped[0].resize(MAX_FRAMES_IN_FLIGHT);
            
            VmaAllocationInfo allocInfo {};
            createBuffer(context, bufferSize,  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, context.uniformBuffers[0][i], context.uniformBuffersAllocation[0][i], VMA_ALLOCATION_CREATE_MAPPED_BIT, &allocInfo);
            context.uniformBuffersMapped[0][i] = allocInfo.pMappedData;
        }
    }
}