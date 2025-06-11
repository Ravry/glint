#include "mvk_core.h"

namespace Mvk {
    VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR desiredPresentMode) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == desiredPresentMode) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(Context& context, const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(context.window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }

    void createSwapchain(Context &context, VkPresentModeKHR desiredPresentMode)
    {
        SwapChainSupportDetails details;
        querySwapChainSupport(context.physicalDevice, context, details);

        context.swapchainDetails = details;

        VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(details.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(details.presentModes, desiredPresentMode);
        VkExtent2D extent = chooseSwapExtent(context, details.capabilites);

        uint32_t imageCount = details.capabilites.minImageCount + 1;

        if (details.capabilites.maxImageCount > 0 && imageCount > details.capabilites.maxImageCount) {
            imageCount = details.capabilites.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        createInfo.surface = context.surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        
        uint32_t queueFamilyIndices[] = {context.queueFamilyIndices.graphicsFamily.value(), context.queueFamilyIndices.presentFamily.value()};

        if (context.queueFamilyIndices.graphicsFamily != context.queueFamilyIndices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = details.capabilites.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(context.device, &createInfo, nullptr, &context.swapchain) != VK_SUCCESS) {
            THROW("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(context.device, context.swapchain, &imageCount, 0);
        context.swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(context.device, context.swapchain, &imageCount, context.swapchainImages.data());

        context.swapchainImageFormat = surfaceFormat.format;
        context.swapchainExtent = extent;

        context.swapchainImageViews.resize(context.swapchainImages.size());

        for (size_t i {0}; i < context.swapchainImages.size(); i++) {
            createImageView(context, context.swapchainImages[i], context.swapchainImageFormat, context.swapchainImageViews[i]);
        }
    }

    void recreateSwapchain(Context& context) {
        int width = 0, height = 0;
        glfwGetFramebufferSize(context.window, &width, &height);
        
        while(width == 0 || height == 0) {
            if (glfwWindowShouldClose(context.window))
                return;

            glfwGetFramebufferSize(context.window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(context.device);

        for (size_t i {0}; i < context.swapchainFramebuffers.size(); i++) {
            vkDestroyFramebuffer(context.device, context.swapchainFramebuffers[i], 0);
        }

        for (size_t i {0}; i < context.swapchainImageViews.size(); i++) {
            vkDestroyImageView(context.device, context.swapchainImageViews[i], 0);
        }

        vkDestroySwapchainKHR(context.device, context.swapchain, 0);

        createSwapchain(context, VK_PRESENT_MODE_FIFO_KHR);
        createFramebuffers(context);
    }
}