#pragma once
#include <thread>
#include <chrono>
#include "mvk_core.h"
#include "utils.h"
#include "discord_self.h"
#include "settings.h"

namespace Glint {
    enum WindowType {
        WINDOW_DEFAULT_TYPE,  
        WINDOW_WALLPAPER_TYPE
    };
    
    struct WindowContext {
        WindowType type;
        GLFWwindow* window;
        HWND handle;
        Mvk::Context mvkContext {};
        bool resized {false};
    };

    struct WindowCreateInfo {
        WindowType type;
        const char* title;
        int width;
        int height;
    };

    void createWindow(WindowContext& windowContext, WindowCreateInfo& windowCreateInfo);
    void runWindow(WindowContext& windowContext);
}