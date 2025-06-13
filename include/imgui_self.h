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

namespace Mvk {
    class Context;
}

enum GLINT_THUMBNAIL_FILE_TYPE
{
    GLINT_THUMBNAIL_FILE_TYPE_IMAGE,
    GLINT_THUMBNAIL_FILE_TYPE_VIDEO
};


namespace MyImGUI {
    inline std::mutex sharedSettingsMutex;

    struct MediaItem
    {
        std::string file;
        GLINT_THUMBNAIL_FILE_TYPE type;
    };

    struct SharedSettings
    {
        int fps { 20 };
        float tintColor[3] { 1.f, 1.f, 1.f };
        bool onlyPlayWhenFocused { false };
        std::queue<MediaItem> mediaFile;
    };

    inline SharedSettings sharedSettings;

    void SetupImGuiStyle();
    void initMyImGUI(Mvk::Context &context);
    void handleUpdatePathQueue(Mvk::Context &context);

    void renderWindow(Mvk::Context &context, double deltaTime, int width, int height);
}