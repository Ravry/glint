#include "imgui_self.h"

namespace MyImGUI {
    const float titleBarHeight { 30.0f };
    const float marginX { 10.f };
    const int numColumns { 4 };

    std::queue<const char *> imagesPathUpdateQueue;
    std::queue<const char *> videosPathUpdateQueue;

    std::vector<std::pair<std::string, ImTextureID>> imageThumbnails;
    std::vector<std::pair<std::string, ImTextureID>> videoThumbnails;

    struct Selection {
        GLINT_THUMBNAIL_FILE_TYPE type { GLINT_THUMBNAIL_FILE_TYPE_VIDEO };
        int selected { -1 };
    };

    Selection selection;

    int size, w, h;
    ImVec2 windowSize;

    void SetupImGuiStyle() {
        ImGuiIO& io = ImGui::GetIO();
        io.FontDefault = io.Fonts->AddFontFromFileTTF(ASSETS_DIR "fonts/arial.ttf", 18.0f);

        ImGuiStyle &style = ImGui::GetStyle();
        ImVec4 *colors = style.Colors;

        style.WindowRounding    = 0.0f;
        style.FrameRounding     = 0.0f;
        style.GrabRounding      = 5.0f;
        style.TabRounding       = 5.0f;
        style.ScrollbarRounding = 0.0f;
        style.WindowBorderSize  = 1.0f;
        style.FrameBorderSize   = 0.0f;
        style.PopupBorderSize   = 1.0f;
        style.PopupRounding     = 0.0f;
        style.FramePadding      = ImVec2(4.0f, 4.0f);
        style.ItemSpacing       = ImVec2(4.0f, 4.0f);
        style.IndentSpacing     = 10.0f;
        style.ScrollbarSize     = 20.0f;

        colors[ImGuiCol_Text]                 = ImVec4(0.98f, 0.92f, 1.00f, 1.00f); // Soft white-lavender
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.40f, 0.60f, 1.00f); // Muted lavender

        // Dark canvas background
        colors[ImGuiCol_WindowBg]             = ImVec4(0.08f, 0.05f, 0.10f, 1.00f); 
        colors[ImGuiCol_ChildBg]              = ImVec4(0.10f, 0.05f, 0.12f, 1.00f);
        colors[ImGuiCol_PopupBg]              = ImVec4(0.12f, 0.08f, 0.16f, 0.94f);

        // Borders
        colors[ImGuiCol_Border]               = ImVec4(0.50f, 0.30f, 0.55f, 0.60f);
        colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        // Frames with a pink-violet palette
        colors[ImGuiCol_FrameBg]              = ImVec4(0.22f, 0.10f, 0.26f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.72f, 0.20f, 0.58f, 1.00f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.85f, 0.22f, 0.55f, 1.00f);

        // Title bar
        colors[ImGuiCol_TitleBg]              = ImVec4(0.18f, 0.08f, 0.22f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.35f, 0.10f, 0.45f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.10f, 0.05f, 0.12f, 0.60f);

        // Menus & scrollbars
        colors[ImGuiCol_MenuBarBg]            = ImVec4(0.12f, 0.07f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.06f, 0.12f, 0.60f);
        colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.50f, 0.20f, 0.50f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.80f, 0.30f, 0.70f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(1.00f, 0.40f, 0.80f, 1.00f);

        // Checkmark and sliders
        colors[ImGuiCol_CheckMark]            = ImVec4(1.00f, 0.50f, 0.90f, 1.00f);
        colors[ImGuiCol_SliderGrab]           = ImVec4(0.80f, 0.30f, 0.75f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]     = ImVec4(1.00f, 0.45f, 0.85f, 1.00f);

        // Buttons
        colors[ImGuiCol_Button]               = ImVec4(0.30f, 0.00f, 0.30f, 0.75f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.60f, 0.00f, 0.50f, 1.00f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.65f, 0.00f, 0.40f, 1.00f);

        // Headers & Tabs
        colors[ImGuiCol_Header]               = ImVec4(0.30f, 0.00f, 0.30f, 0.75f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.60f, 0.00f, 0.50f, 1.00f);
        colors[ImGuiCol_HeaderActive]         = ImVec4(0.65f, 0.00f, 0.40f, 1.00f);

        colors[ImGuiCol_Tab]                  = ImVec4(0.30f, 0.10f, 0.35f, 1.00f);
        colors[ImGuiCol_TabHovered]           = ImVec4(0.90f, 0.35f, 0.80f, 1.00f);
        colors[ImGuiCol_TabActive]            = ImVec4(0.70f, 0.25f, 0.65f, 1.00f);
        colors[ImGuiCol_TabUnfocused]         = ImVec4(0.25f, 0.08f, 0.30f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.45f, 0.15f, 0.50f, 1.00f);

        // Separator & Resize Grip
        colors[ImGuiCol_Separator]            = ImVec4(0.40f, 0.10f, 0.45f, 0.70f);
        colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.80f, 0.25f, 0.70f, 1.00f);
        colors[ImGuiCol_SeparatorActive]      = ImVec4(1.00f, 0.35f, 0.85f, 1.00f);

        colors[ImGuiCol_ResizeGrip]           = ImVec4(0.70f, 0.20f, 0.70f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.90f, 0.30f, 0.80f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]     = ImVec4(1.00f, 0.40f, 0.90f, 0.95f);

        // Plot Colors
        colors[ImGuiCol_PlotLines]            = ImVec4(0.85f, 0.45f, 0.95f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]     = ImVec4(1.00f, 0.60f, 0.80f, 1.00f);
        colors[ImGuiCol_PlotHistogram]        = ImVec4(0.95f, 0.70f, 0.90f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.80f, 0.90f, 1.00f);

        // Modal overlay
        colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.15f, 0.05f, 0.20f, 0.80f);
    }

    void setupImageThumbnails(Mvk::Context &context, const char *filepath, std::vector<std::pair<std::string, ImTextureID>> &thumbnails) {
        std::vector<std::string> directoryContentUsable;
        {
            std::vector<std::string> directoryContents = readDirectoryContent(filepath);
            for (auto &directoryContent : directoryContents)
            {
                if (isImageFile(directoryContent))
                {
                    directoryContentUsable.push_back(directoryContent);
                    LOG(fmt::color::light_steel_blue, "image-file ~ {}\n", directoryContent);
                }
                else
                {
                    LOG(fmt::color::dark_orange, "not-image-file ~ {}\n", directoryContent);
                }
            }
        }

        size_t directoryContentCount{directoryContentUsable.size()};

        context.imageDatas["media_image"].images.resize(directoryContentCount);
        context.imageDatas["media_image"].imageAllocations.resize(directoryContentCount);
        context.imageDatas["media_image"].imageViews.resize(directoryContentCount);
        context.imageDatas["media_image"].imageSamplers.resize(directoryContentCount);

        if (directoryContentCount == 0)
            return;

        for (size_t i{0}; i < directoryContentCount; i++)
        {
            uint8_t *data = getImageThumbnail(directoryContentUsable[i].c_str());

            if (data)
                Mvk::createTextureFromData(context, context.imageDatas["media_image"].images[i], context.imageDatas["media_image"].imageAllocations[i], context.imageDatas["media_image"].imageViews[i], context.imageDatas["media_image"].imageSamplers[i], data, 174, 97);
            else
            {
                i--;
                directoryContentCount--;
                continue;
            }
        }

        context.imageDatas["media_image"].images.resize(directoryContentCount);
        context.imageDatas["media_image"].imageAllocations.resize(directoryContentCount);
        context.imageDatas["media_image"].imageViews.resize(directoryContentCount);
        context.imageDatas["media_image"].imageSamplers.resize(directoryContentCount);

        Mvk::createDescriptorPoolUtil(context, context.imageDatas["media_image"].imageDescriptorPool, directoryContentCount);

        std::vector<VkDescriptorSet> descriptorSets = Mvk::allocateDescriptorSetsUtil(context, context.descriptorSetLayout, context.imageDatas["media_image"].imageDescriptorPool, context.imageDatas["media_image"].imageViews, context.imageDatas["media_image"].imageSamplers);

        size = descriptorSets.size();
        thumbnails.resize(size);

        for (size_t i{0}; i < size; i++)
        {
            thumbnails[i].first = directoryContentUsable[i];
            thumbnails[i].second = (ImTextureID)descriptorSets[i];
        }
    }

    void setupVideoThumbnails(Mvk::Context &context, const char *filepath, std::vector<std::pair<std::string, ImTextureID>> &thumbnails) {
        std::vector<std::string> directoryContentUsable;
        {
            std::vector<std::string> directoryContents = readDirectoryContent(filepath);
            for (auto &directoryContent : directoryContents)
            {
                if (isVideoFile(directoryContent))
                {
                    directoryContentUsable.push_back(directoryContent);
                    LOG(fmt::color::light_steel_blue, "video-file ~ {}\n", directoryContent);
                }
                else
                {
                    LOG(fmt::color::dark_orange, "not-video-file ~ {}\n", directoryContent);
                }
            }
        }

        size_t directoryContentCount {directoryContentUsable.size()};
        
        if (directoryContentCount == 0)
            return;

        context.imageDatas["media_video"].images.resize(directoryContentCount);
        context.imageDatas["media_video"].imageAllocations.resize(directoryContentCount);
        context.imageDatas["media_video"].imageViews.resize(directoryContentCount);
        context.imageDatas["media_video"].imageSamplers.resize(directoryContentCount);

        for (size_t i{0}; i < directoryContentCount; i++) {
            uint8_t *data = getMediaThumbnail(directoryContentUsable[i].c_str());
            if (data)
                Mvk::createTextureFromData(context, context.imageDatas["media_video"].images[i], context.imageDatas["media_video"].imageAllocations[i], context.imageDatas["media_video"].imageViews[i], context.imageDatas["media_video"].imageSamplers[i], data, 174, 97);
        }

        Mvk::createDescriptorPoolUtil(context, context.imageDatas["media_video"].imageDescriptorPool, directoryContentCount);

        std::vector<VkDescriptorSet> descriptorSets = Mvk::allocateDescriptorSetsUtil(context, context.descriptorSetLayout, context.imageDatas["media_video"].imageDescriptorPool, context.imageDatas["media_video"].imageViews, context.imageDatas["media_video"].imageSamplers);

        size = descriptorSets.size();
        thumbnails.resize(size);

        for (size_t i{0}; i < size; i++)
        {
            thumbnails[i].first = directoryContentUsable[i];
            thumbnails[i].second = (ImTextureID)descriptorSets[i];
        }
    }

    void setupThumbnails(Mvk::Context &context, const char *filepath, std::vector<std::pair<std::string, ImTextureID>> &thumbnails, GLINT_THUMBNAIL_FILE_TYPE type)
    {
        switch (type) {
            case GLINT_THUMBNAIL_FILE_TYPE_IMAGE: {
                setupImageThumbnails(context, filepath, thumbnails);
                break;
            }
            case GLINT_THUMBNAIL_FILE_TYPE_VIDEO: {
                setupVideoThumbnails(context, filepath, thumbnails);
                break;
            }
        }
    }

    void handleUpdatePathQueue(Mvk::Context& context) {
        vkDeviceWaitIdle(context.device);

        if (imagesPathUpdateQueue.size() > 0)
        {
            selection.selected = -1;
            size = 0;
            imageThumbnails.clear();
            context.destroyImagesAndBelongings(context.imageDatas["media_image"]);
            strcpy(dirPathImages, imagesPathUpdateQueue.front());
            imagesPathUpdateQueue.pop();
            setupThumbnails(context, dirPathImages, imageThumbnails, GLINT_THUMBNAIL_FILE_TYPE_IMAGE);
        }

        if (videosPathUpdateQueue.size() > 0) {
            selection.selected = -1;
            size = 0;
            videoThumbnails.clear();
            context.destroyImagesAndBelongings(context.imageDatas["media_video"]);
            strcpy(dirPathVideos, videosPathUpdateQueue.front());
            videosPathUpdateQueue.pop();
            setupThumbnails(context, dirPathVideos, videoThumbnails, GLINT_THUMBNAIL_FILE_TYPE_VIDEO);
        }
    }

    void initMyImGUI(Mvk::Context &context)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        float windowPaddingX, framePaddingX;

        windowPaddingX = style.WindowPadding.x;
        style.WindowPadding.x = 0;
        style.WindowPadding.y = 0;
        framePaddingX = style.FramePadding.x;

        w = (WND_WIDTH - (10 * 3) - (marginX * (numColumns + 2)) - (framePaddingX * numColumns * 2)) / numColumns;
        h = w * (9./16.);
        
        LOG(fmt::color::peru, "thumbnail - [width|{}] [height|{}]\n", w, h);

        setupThumbnails(context, dirPathImages, imageThumbnails, GLINT_THUMBNAIL_FILE_TYPE_IMAGE);
        setupThumbnails(context, dirPathVideos, videoThumbnails, GLINT_THUMBNAIL_FILE_TYPE_VIDEO);
    }

    void renderCustomTitlebar(GLFWwindow* window) {
        const ImVec2 windowPos = ImGui::GetWindowPos();
        windowSize = ImGui::GetWindowSize();

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 titleBarMin = windowPos;
        ImVec2 titleBarMax = ImVec2(windowPos.x + windowSize.x, windowPos.y + titleBarHeight);
        drawList->AddRectFilled(titleBarMin, titleBarMax, IM_COL32(0.08f * 255, 0.00f * 255, 0.12f * 255, 255));

        static bool dragging = false;
        static ImVec2 dragOffset;

        ImVec2 dragZoneSize = ImVec2(windowSize.x - titleBarHeight * 3, titleBarHeight);
        ImGui::InvisibleButton("drag_zone", dragZoneSize);

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
            dragging = true;
            double cursorX, cursorY;
            glfwGetCursorPos(window, &cursorX, &cursorY);
            dragOffset = ImVec2((float)cursorX, (float)cursorY);
        }

        if (dragging && ImGui::IsMouseDown(0)) {
            double cursorX, cursorY;
            glfwGetCursorPos(window, &cursorX, &cursorY);

            int winX, winY;
            glfwGetWindowPos(window, &winX, &winY);

            int newX = winX + static_cast<int>(cursorX - dragOffset.x);
            int newY = winY + static_cast<int>(cursorY - dragOffset.y);

            glfwSetWindowPos(window, newX, newY);
        }

        if (!ImGui::IsMouseDown(0)) {
            dragging = false;
        }
        
        
        ImGui::SetCursorPos(ImVec2(10, (titleBarHeight - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::Text(PROJ_NAME);

        ImGui::SetCursorPos(ImVec2(windowSize.x - titleBarHeight, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.00f, 0.12f, 1.00f));

        if (ImGui::Button("x", ImVec2(titleBarHeight, titleBarHeight))) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        

        ImGui::SetCursorPos(ImVec2(windowSize.x - titleBarHeight * 2, 0));

        if (ImGui::Button("-", ImVec2(titleBarHeight, titleBarHeight))) {
            glfwIconifyWindow(window);
        }

        ImGui::PopStyleColor(1);
    }

    void renderSettingsHeader() {
        static bool externalSettingsOpened{false};

        if(ImGui::CollapsingHeader("settings")) {
            ImGui::Indent(marginX);
            ImGui::Text("Under maintenance...\nIn case you want to contribute feel free to do so via https://github.com/Ravry/glint");

            ImGui::Separator();

            ImGui::Text("active wallpaper:");
            ImGui::SameLine();
            if (selection.selected >= 0) {
                bool settingsButtonPressed { false };
                switch (selection.type) {
                    case GLINT_THUMBNAIL_FILE_TYPE_IMAGE: {
                        settingsButtonPressed = ImGui::ImageButton("active_thumb", imageThumbnails[selection.selected].second, ImVec2(240, 135));
                        break;
                    }
                    case GLINT_THUMBNAIL_FILE_TYPE_VIDEO: {
                        settingsButtonPressed = ImGui::ImageButton("active_thumb", videoThumbnails[selection.selected].second, ImVec2(240, 135));
                        break;
                    } 
                }

                if (settingsButtonPressed) externalSettingsOpened = true;

                if (externalSettingsOpened) {
                    ImGui::Begin("settings", &externalSettingsOpened);
                    ImGui::Spacing();
                    ImGui::Indent(marginX);
                    ImGui::Text("general");
                    ImGui::Spacing();
                    ImGui::Indent(marginX);
                    {
                        std::lock_guard<std::mutex> lock(sharedSettingsMutex);
                        ImGui::InputInt("framerate", &sharedSettings.fps);
                        ImGui::ColorEdit3("tint", sharedSettings.tintColor);
                        ImGui::Checkbox("only play when focused", &sharedSettings.onlyPlayWhenFocused);
                    }
                    ImGui::Unindent(marginX * 2);
                    ImGui::End();
                }
            }
            else
            {
                ImGui::Text("none");
            }

            ImGui::Separator();

            ImGui::Unindent(marginX);
        }
    }

    void renderThumbnailGrid(std::vector<std::pair<std::string, ImTextureID>>& thumbnails, GLINT_THUMBNAIL_FILE_TYPE type) {
        ImVec2 imageSize(w, h);
        for (int i = 0; i < thumbnails.size(); ++i)
        {
            if (ImGui::ImageButton(("##thumb" + std::to_string(i)).c_str(), thumbnails[i].second, imageSize))
            {
                selection = Selection {
                    .type = type,
                    .selected = i 
                };

                {
                    std::lock_guard<std::mutex> lock(MyImGUI::sharedSettingsMutex);
                    MyImGUI::sharedSettings.mediaFile.push(MyImGUI::MediaItem{
                        .file = thumbnails[i].first,
                        .type = type
                    });
                }
            }

            if ((i + 1) % numColumns != 0 && i != (thumbnails.size() - 1))
                ImGui::SameLine(0, marginX);
        }
    }

    void renderBrowseDirectory(char* directory, size_t directory_size, std::queue<const char*>& addQueue) {
        if (ImGui::Button("Browse"))
        {
            std::string selectedPath = OpenFolderDialog();
            if (!selectedPath.empty())
            {
                strncpy(directory, selectedPath.c_str(), directory_size);
                directory[directory_size - 1] = '\0';
            }
        }
        ImGui::SameLine();
        if (ImGui::InputText("##directory_input", directory, directory_size))
        {
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply"))
        {
            addQueue.push(directory);
        }
    }

    void renderMediaHeader(Mvk::Context &context) {
        if(ImGui::CollapsingHeader("media", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent(marginX);

            if (ImGui::TreeNodeEx("image", ImGuiTreeNodeFlags_DefaultOpen)) {
                renderBrowseDirectory(dirPathImages, 512, imagesPathUpdateQueue);
                renderThumbnailGrid(imageThumbnails, GLINT_THUMBNAIL_FILE_TYPE_IMAGE);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("video", ImGuiTreeNodeFlags_DefaultOpen)) {
                renderBrowseDirectory(dirPathVideos, 512, videosPathUpdateQueue);
                renderThumbnailGrid(videoThumbnails, GLINT_THUMBNAIL_FILE_TYPE_VIDEO);
                ImGui::TreePop();
            }
            ImGui::Unindent(marginX);
        }
    }

    void renderAboutHeader() {
        if(ImGui::CollapsingHeader("about", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent(marginX);
            ImGui::Text("This open-source wallpaper engine was made by https://github.com/Ravry.\nIn case you want to contribute feel free to do so via https://github.com/Ravry/glint\nThe used resources are fmt, glfw, vulkan-api, glm and ffmpeg.");
            ImGui::Unindent(marginX);
            ImGui::Separator();
        }   
    }

    void renderFooter(double deltaTime) {
        static double fpsTimer{0};
        static int fps{0};

        fpsTimer += deltaTime;
        if (fpsTimer >= 1.) {
            fps = static_cast<int>(1. / deltaTime);
            fpsTimer = 0;
        }
        
        ImGui::Spacing();
        std::string strFPS = "make sure to enable:\nenvironment variables->performance->visual effects->[X] animate controls and elements inside windows\n\notherwise the wallpaper might not be placed behind the icons correctly\nFPS: " + std::to_string(fps) + " ~ CC0 No Rights Reserved (https://creativecommons.org/public-domain/cc0/)";
        ImGui::Indent(marginX);
        ImGui::Text(strFPS.c_str()); 
        ImGui::Unindent(marginX);   
        ImGui::Spacing();
    }

    void renderWindowContent(Mvk::Context &context, double deltaTime) {
        ImGui::SetCursorPosY(titleBarHeight);
        ImVec2 childSize = ImVec2(windowSize.x, windowSize.y - titleBarHeight);
        ImGui::BeginChild("ContentRegion", childSize, false);
            
        renderSettingsHeader();
        renderMediaHeader(context);
        renderAboutHeader();
        renderFooter(deltaTime);

        ImGui::EndChild();
    }

    void renderWindow(Mvk::Context &context, double deltaTime, int width, int height)
    {
        static bool firstFrame{true};
        if (firstFrame) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(width, height));
            firstFrame = false;
        }

        ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNavFocus |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoTitleBar;

        ImGui::Begin("Wallpaper Selector", nullptr, flags);

        renderCustomTitlebar(context.window);
        renderWindowContent(context, deltaTime);
        
        ImGui::End();
    }
}