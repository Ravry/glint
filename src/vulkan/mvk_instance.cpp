#include "mvk_core.h"

namespace Mvk {
    bool checkValidationLayerSupport(std::vector<const char*> validationLayers) {
        uint32_t layerCount {0};
        vkEnumerateInstanceLayerProperties(&layerCount, 0);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
        for (auto layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
                return false;
        }

        return true;
    }

    void createInstance(Context& context) {
        VkApplicationInfo appInfo { VK_STRUCTURE_TYPE_APPLICATION_INFO };
        appInfo.pApplicationName = "Experimenting with Vulkan api";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vulkan Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
    
        VkInstanceCreateInfo createInfo { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        createInfo.pApplicationInfo = &appInfo;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        
        if (enableValidationLayers) {
            if (!checkValidationLayerSupport(validationLayers)) {
                THROW("validation layers requested, but not available!");
            }

            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers)
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(0, &extensionCount, extensions.data());

        for (const auto& extension : extensions) {
            bool extensionIncluded { false };
            for (auto requiredExtension : requiredExtensions) {        
                if (std::strcmp(extension.extensionName, requiredExtension) == 0) {
                    extensionIncluded = true;
                    break;
                }
            }
            LOG(extensionIncluded ? fmt::color::lime_green : fmt::color::gray, "extension: {}\n", extension.extensionName);
        }

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        
        if (vkCreateInstance(&createInfo, 0, &context.instance) != VK_SUCCESS) {
            THROW("failed to create vulkan instance!");
        }

        if (!enableValidationLayers)
            return; 
            
        if (CreateDebugUtilsMessengerEXT(context.instance, &debugCreateInfo, nullptr, &context.debugMessenger) != VK_SUCCESS) {
            THROW("failed to set up debug messenger!");
        }
    }
}