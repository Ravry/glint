#include "mvk_core.h"

namespace Mvk {
    void findQueueFamilies(Context& context, VkPhysicalDevice physicalDevice) {
        uint32_t queueFamilyCount {0};
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, 0);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        size_t i {0};
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                context.queueFamilyIndices.graphicsFamily = i;
                
                VkBool32 presentSupport {false};
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, context.surface, &presentSupport);

                if (presentSupport)
                    context.queueFamilyIndices.presentFamily = i;

                if (context.queueFamilyIndices.isComplete())
                    break;
            }
            i++;
        }
    }

    void querySwapChainSupport(VkPhysicalDevice physicalDevice, Context& context, SwapChainSupportDetails& details) {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, context.surface, &details.capabilites);

        uint32_t formatCount {0};
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, context.surface, &formatCount, 0);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, context.surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount {0};
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, context.surface, &presentModeCount, 0);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, context.surface, &presentModeCount, details.presentModes.data());
        }
    }

    bool isPhysicalDeviceSuitable(const VkPhysicalDevice& physicalDevice, Context& context) {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        context.physicalDeviceProperties = physicalDeviceProperties;

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

        findQueueFamilies(context, physicalDevice);
    
        bool extensionsSupported {false};

        uint32_t extensionCount {0};
        vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionCount, 0);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionCount, availableExtensions.data());
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        extensionsSupported = requiredExtensions.empty();

        
        bool swapChainAdequate {false};
        if (extensionsSupported) {
            SwapChainSupportDetails details;
            querySwapChainSupport(physicalDevice, context, details);
            swapChainAdequate = !details.formats.empty() && !details.formats.empty();
        }
        
        VkPhysicalDeviceFeatures supportedPhysicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &supportedPhysicalDeviceFeatures);

        LOG(fmt::color::lime_green, "checking phyiscal device for suitability: {}\n", physicalDeviceProperties.deviceName);

        return context.queueFamilyIndices.graphicsFamily.has_value() && extensionsSupported & swapChainAdequate && supportedPhysicalDeviceFeatures.samplerAnisotropy;
    }

    void pickPhysicalDevice(Context& context) {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(context.instance, &deviceCount, 0);

        if (deviceCount == 0)
            THROW("failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(context.instance, &deviceCount, physicalDevices.data());

        for (const auto& _physicalDevice : physicalDevices) {
            if (isPhysicalDeviceSuitable(_physicalDevice, context)) {
                physicalDevice = _physicalDevice;
                break;
            }            
        }

        if (physicalDevice == VK_NULL_HANDLE) 
            THROW("failed to find suitable GPU!");

        context.physicalDevice = physicalDevice;
    }

    void createDevice(Context& context) {
        pickPhysicalDevice(context);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; 
        std::set<uint32_t> uniqueQueueFamilies = { context.queueFamilyIndices.graphicsFamily.value(), context.queueFamilyIndices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {       
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());    
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(context.physicalDevice, &createInfo, 0, &context.device)) {
            THROW("failed to create logical device");
        }

        vkGetDeviceQueue(context.device, context.queueFamilyIndices.graphicsFamily.value(), 0, &context.graphicsQueue);
        vkGetDeviceQueue(context.device, context.queueFamilyIndices.presentFamily.value(), 0, &context.presentQueue);

        LOG(fmt::color::white, "graphics queue index: {}; present queue index: {}\n", context.queueFamilyIndices.graphicsFamily.value(), context.queueFamilyIndices.presentFamily.value());
    }
}