#pragma once
#include <string>
#include <vector>
#include <set>
#include <optional>
#include <algorithm>
#include <fstream>
#include <array>
#include "log.h"
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vk_mem_alloc.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include "media.h"
#include "imgui_self.h"

inline constexpr size_t N = 2;

inline HWND desktop {GetShellWindow()};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

const int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
};

const std::vector<Vertex> vertices {
    {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
    {{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}},
    {{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}},
};

const std::vector<uint16_t> indices {
    0, 1, 2,
    2, 3, 0
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

static inline VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    LOG(fmt::color::orange, "{}\n", pCallbackData->pMessage);

    return VK_FALSE;
}

inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

inline VkDebugUtilsMessengerEXT debugMessenger;

namespace Mvk {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilites;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    
    struct Context {
        GLFWwindow* window;
        double deltaTime;
        
        VkInstance instance;
        QueueFamilyIndices queueFamilyIndices;
        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceProperties physicalDeviceProperties;
        VkDevice device;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        SwapChainSupportDetails swapchainDetails;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;     
        VkRenderPass renderpass;  
        VkDescriptorPool descriptorPool;
        VkDescriptorPool imguiDescriptorPool;
        std::vector<std::vector<VkDescriptorSet>> descriptorSets;
        VkDescriptorSetLayout descriptorSetLayout;
        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;
        std::vector<VkFramebuffer> swapchainFramebuffers;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffer;
        std::vector<VkSemaphore> imageAvailableSemaphore;
        std::vector<VkSemaphore> renderFinishedSemaphore;
        std::vector<VkFence> inFlightFence;

        VmaAllocator allocatorVMA;
        VkBuffer vertexBuffer;
        VmaAllocation vertexBufferAllocation;
        VkBuffer indexBuffer;
        VmaAllocation indexBufferAllocation;

        std::vector<std::vector<VkBuffer>> uniformBuffers;
        std::vector<std::vector<VmaAllocation>> uniformBuffersAllocation;
        std::vector<std::vector<void*>> uniformBuffersMapped;

        VkImage textureImage;
        VmaAllocation textureImageAllocation;
        VkImageView textureImageView;
        VkSampler textureImageSampler;

    
        std::vector<VkImage> images;
        std::vector<VmaAllocation> imageAllocations;
        std::vector<VkImageView> imageViews;
        std::vector<VkSampler> imageSamplers;    
        VkDescriptorPool imageDescriptorPool;
        
        void destroy() {   
            for (size_t i {0}; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroyFence(device, inFlightFence[i], 0);
                vkDestroySemaphore(device, renderFinishedSemaphore[i], 0);
                vkDestroySemaphore(device, imageAvailableSemaphore[i], 0);
            }

            vkDestroyCommandPool(device, commandPool, 0);
            
            for (auto swapchainFramebuffer : swapchainFramebuffers) {
                vkDestroyFramebuffer(device, swapchainFramebuffer, 0);
            }
            
            for (auto& sampler : imageSamplers) {
                vkDestroySampler(device, sampler, 0);
            }

            vkDestroySampler(device, textureImageSampler, 0);

            
            for (auto& view : imageViews) {
                vkDestroyImageView(device, view, 0);
            }
            
            vkDestroyImageView(device, textureImageView, 0);

            
            for (size_t i {0}; i < images.size(); i++) {
                vmaDestroyImage(allocatorVMA, images[i], imageAllocations[i]);
            }

            vmaDestroyImage(allocatorVMA, textureImage, textureImageAllocation);

            vmaDestroyBuffer(allocatorVMA, indexBuffer, indexBufferAllocation);
            vmaDestroyBuffer(allocatorVMA, vertexBuffer, vertexBufferAllocation);

            vkDestroyPipeline(device, pipeline, 0);
            
            vkDestroyPipelineLayout(device, pipelineLayout, 0);

            for (size_t j {0}; j < uniformBuffers.size(); j++) {
                for (size_t i {0}; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    vmaDestroyBuffer(allocatorVMA, uniformBuffers[j][i], uniformBuffersAllocation[j][i]);
                }
            }

            vmaDestroyAllocator(allocatorVMA);

            vkDestroyDescriptorPool(device, imageDescriptorPool, 0);
            vkDestroyDescriptorPool(device, imguiDescriptorPool, 0);
            vkDestroyDescriptorPool(device, descriptorPool, 0);

            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, 0);
            
            vkDestroyRenderPass(device, renderpass, 0);

            for (auto imageView : swapchainImageViews) {
                vkDestroyImageView(device, imageView, 0);
            }

            vkDestroySwapchainKHR(device, swapchain, 0);

            vkDestroyDevice(device, 0);
            
            vkDestroySurfaceKHR(instance, surface, 0);
            
            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(instance, debugMessenger, 0);
            }

            vkDestroyInstance(instance, 0);
        }
    };
    
    void findQueueFamilies(Context& context, VkPhysicalDevice physicalDevice);

    void querySwapChainSupport(VkPhysicalDevice physicalDevice, Context& context, SwapChainSupportDetails& details);
    
    void createInstance(Context& context);

    void createDevice(Context& context);

    void createSurface(Context& context, GLFWwindow* window);

    void createImageView(Context& context, VkImage image, VkFormat format, VkImageView& result);
    void createSwapchain(Context& context);
    void recreateSwapchain(Context& context);

    void createPipeline(const std::string& vertFilepath, const std::string& fragFilepath, Context& context);

    void createFramebuffers(Context& context);

    void createCommandPool(Context& context);

    void createCommandBuffer(Context& context);
    void recordCommandBuffer(Context& context, VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t currentFrame);

    void createSyncObjects(Context& context);

    VkVertexInputBindingDescription getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

    void createAllocatorVMA(Context& context);

    uint32_t findMemoryType(Context& context, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkCommandBuffer beginSingleTimeCommands(Context& context);
    void endSingleTimeCommands(Context& context, VkCommandBuffer commandBuffer);
    void createBuffer(Context& context, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& bufferMemory, VmaAllocationCreateFlags allocFlags, VmaAllocationInfo* allocInfo);
    void createVertexBuffer(Context& context);
    void createIndexBuffer(Context& context);

    void createUniformBuffers(Context& context, size_t count);

    void createDescriptorSetLayout(Context& context);
    
    void createDescriptorPool(Context& context);

    void allocateDescriptorSets(Context& context);

    void createDescriptorPoolUtil(Context& context, VkDescriptorPool& descriptorPool, uint32_t count);
    void createDescriptorSetLayoutUtil(Context& context, VkDescriptorSetLayout& descriptorSetLayout);
    std::vector<VkDescriptorSet> allocateDescriptorSetsUtil(Context& context, VkDescriptorSetLayout& descriptorSetLayout, VkDescriptorPool& descriptorPool, std::vector<VkImageView>& imageViews, std::vector<VkSampler>& samplers);

    void createDescriptorPoolImGUI(Context& context);

    void createTextureImage(Context& context, const char* imageFile);
    void createTexture(Context& context, const int width, const int height); 
    void updateTextureImageDataDynamic(Context& context, void*& pixelData);
    void createTextureFromData(Context& context, VkImage& image, VmaAllocation& allocation, VkImageView& imageView, VkSampler& sampler, uint8_t* pixels, int width, int height);
}