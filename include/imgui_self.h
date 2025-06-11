#pragma once
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "GLFW/glfw3.h"
#include "media.h"
#include "log.h"

inline constexpr size_t WND_WIDTH = 800;
inline constexpr size_t WND_HEIGHT = 600;

namespace MyImGUI {
    void SetupImGuiStyle();
    void initThumbnails(std::vector<VkDescriptorSet> &descriptorSets, std::vector<std::string> &filenames);

    struct SharedSettings
    {
        int fps { 20 };
        float tintColor[3] { 1.f, 1.f, 1.f };
        std::queue<std::string> mediaFile;
    };

    inline std::mutex sharedSettingsMutex;
    inline SharedSettings sharedSettings;

    void renderWindow(GLFWwindow* window, double deltaTime, int width, int height);
}