#pragma once
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "GLFW/glfw3.h"
#include "media.h"
#include "log.h"
#include "utils.h"
#include "mvk_core.h"

inline constexpr size_t WND_WIDTH = 800;
inline constexpr size_t WND_HEIGHT = 600;

namespace Mvk
{
    class Context;
}

namespace MyImGUI {
    inline std::queue<const char*> pathUpdateQueue;

    void SetupImGuiStyle();
    void initMyImGUI(Mvk::Context &context);
    void handleUpdatePathQueue(Mvk::Context &context);

    struct SharedSettings
    {
        int fps { 20 };
        float tintColor[3] { 1.f, 1.f, 1.f };
        bool onlyPlayWhenFocused { false };
        std::queue<std::string> mediaFile;
    };

    inline std::mutex sharedSettingsMutex;
    inline SharedSettings sharedSettings;

    void renderWindow(Mvk::Context &context, double deltaTime, int width, int height);
}