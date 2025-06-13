#include "imgui_self.h"

namespace MyImGUI {
    const float titleBarHeight { 30.0f };
    const float marginX { 10.f };
    const int numColumns { 4 };
    
    char dirPathImages[512] = ASSETS_DIR "img/";
    char dirPathVideos[512] = ASSETS_DIR "videos/";

    std::queue<const char *> imagesPathUpdateQueue;
    std::queue<const char *> videosPathUpdateQueue;

    std::vector<std::pair<std::string, ImTextureID>> imageThumbnails;
    std::vector<std::pair<std::string, ImTextureID>> videoThumbnails;

    int selectedWallpaper { -1 };

    int size, w, h;
    ImVec2 windowSize;
    
    enum GLINT_THUMBNAIL_FILE_TYPE {
        GLINT_THUMBNAIL_FILE_TYPE_IMAGE,
        GLINT_THUMBNAIL_FILE_TYPE_VIDEO
    };

    void SetupImGuiStyle() {
        ImGuiIO& io = ImGui::GetIO();
        io.FontDefault = io.Fonts->AddFontFromFileTTF(ASSETS_DIR "fonts/arial.ttf", 18.0f);

        ImGuiStyle &style = ImGui::GetStyle();
        ImVec4 *colors = style.Colors;

        style.WindowRounding = 5.0f;
        style.FrameRounding = 0.0f;
        style.ScrollbarRounding = 5.0f;
        style.GrabRounding = 5.0f;
        style.TabRounding = 5.0f;
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = .0f;
        style.PopupBorderSize = 1.0f;
        style.PopupRounding = 5.0f;

        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.f);
        colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

        colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, .9f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

        style.FramePadding = ImVec2(4.0f, 4.0f);
        style.ItemSpacing = ImVec2(4.0f, 4.0f);
        style.IndentSpacing = 20.0f;
        style.ScrollbarSize = 16.0f;
    }

    void setupThumbnails(Mvk::Context &context, const char *filepath, std::vector<std::pair<std::string, ImTextureID>> &thumbnails, GLINT_THUMBNAIL_FILE_TYPE type)
    {
        std::vector<std::string> directoryContentUsable;

        switch (type) {
            case GLINT_THUMBNAIL_FILE_TYPE_IMAGE: {
                
                break;
            }
                
            case GLINT_THUMBNAIL_FILE_TYPE_VIDEO: {
                break;
            }
        }

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

        context.images.resize(directoryContentCount);
        context.imageAllocations.resize(directoryContentCount);
        context.imageViews.resize(directoryContentCount);
        context.imageSamplers.resize(directoryContentCount);

        for (size_t i{0}; i < directoryContentCount; i++) {
            uint8_t *data = getMediaThumbnail(directoryContentUsable[i].c_str());
            if (data)
                Mvk::createTextureFromData(context, context.images[i], context.imageAllocations[i], context.imageViews[i], context.imageSamplers[i], data, 174, 97);
        }

        Mvk::createDescriptorPoolUtil(context, context.imageDescriptorPool, directoryContentCount);

        std::vector<VkDescriptorSet> descriptorSets = Mvk::allocateDescriptorSetsUtil(context, context.descriptorSetLayout, context.imageDescriptorPool, context.imageViews, context.imageSamplers);
        
        size = descriptorSets.size();
        thumbnails.resize(size);

        for (size_t i{0}; i < size; i++)
        {
            thumbnails[i].first = directoryContentUsable[i];
            thumbnails[i].second = (ImTextureID)descriptorSets[i];
        }
    }

    void handleUpdatePathQueue(Mvk::Context& context) {
        if (imagesPathUpdateQueue.size() > 0) {
            selectedWallpaper = -1;
            size = 0;
            imageThumbnails.clear();
            context.destroyImagesAndBelongings();
            strcpy(dirPathVideos, imagesPathUpdateQueue.front());
            imagesPathUpdateQueue.pop();
            setupThumbnails(context, dirPathVideos, imageThumbnails, GLINT_THUMBNAIL_FILE_TYPE_IMAGE);
        }

        if (videosPathUpdateQueue.size() > 0) {
            selectedWallpaper = -1;
            size = 0;
            videoThumbnails.clear();
            context.destroyImagesAndBelongings();
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
        drawList->AddRectFilled(titleBarMin, titleBarMax, IM_COL32(40, 40, 40, 255));

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
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), PROJ_NAME);

        ImGui::SetCursorPos(ImVec2(windowSize.x - titleBarHeight, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

        if (ImGui::Button("X", ImVec2(titleBarHeight, titleBarHeight))) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        
        ImGui::PopStyleColor(2);
        
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.7f, 1.0f));

        ImGui::SetCursorPos(ImVec2(windowSize.x - titleBarHeight * 2, 0));

        if (ImGui::Button("-", ImVec2(titleBarHeight, titleBarHeight))) {
            glfwIconifyWindow(window);
        }

        ImGui::PopStyleColor(3);
    }

    void renderSettingsHeader() {
        static bool externalSettingsOpened{false};

        if(ImGui::CollapsingHeader("settings")) {
            ImGui::Indent(marginX);
            ImGui::Text("Under maintenance...\nIn case you want to contribute feel free to do so via https://github.com/Ravry/glint");

            ImGui::Separator();

            ImGui::Text("active wallpaper:");
            ImGui::SameLine();
            if (selectedWallpaper >= 0) {
                if (ImGui::ImageButton("active_thumb", videoThumbnails[selectedWallpaper].second, ImVec2(240, 135)))
                {
                    externalSettingsOpened = true;
                }

                if (externalSettingsOpened) {
                    ImGui::Begin("Wallpaper Settings", &externalSettingsOpened);
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
            } else {
                ImGui::Text("none");
            }

            
            ImGui::Separator();

            ImGui::Unindent(marginX);
        }
    }

    void renderThumbnailGrid(std::vector<std::pair<std::string, ImTextureID>>& thumbnails) {
        ImVec2 imageSize(w, h);
        for (int i = 0; i < thumbnails.size(); ++i)
        {
            if (ImGui::ImageButton(("##thumb" + std::to_string(i)).c_str(), thumbnails[i].second, imageSize))
            {
                selectedWallpaper = i;
                {
                    std::lock_guard<std::mutex> lock(MyImGUI::sharedSettingsMutex);
                    MyImGUI::sharedSettings.mediaFile.push(thumbnails[i].first);
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
        if (ImGui::InputText("Directory Path", directory, directory_size))
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

            if (ImGui::TreeNode("image")) {
                renderBrowseDirectory(dirPathImages, sizeof(dirPathImages), imagesPathUpdateQueue);
                renderThumbnailGrid(imageThumbnails);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("video")) {
                renderBrowseDirectory(dirPathVideos, sizeof(dirPathVideos), videosPathUpdateQueue);
                renderThumbnailGrid(videoThumbnails);
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