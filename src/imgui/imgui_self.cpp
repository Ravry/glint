#include "imgui_self.h"

namespace MyImGUI {
    const float titleBarHeight { 30.0f };
    const float marginX { 10.f };
    const int numColumns { 4 };

    std::vector<std::pair<std::string, ImTextureID>> thumbnails;
    int selectedWallpaper { -1 };

    bool firstFrame { true };	
    bool externalSettingsOpened { false };

    float windowPaddingX, framePaddingX;
    int size, w, h;
    
    ImVec2 windowSize; 

    float tintColor[3] { 1, 1, 1 };
    int fpsInput {30};

    void SetupImGuiStyle() {
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

        // Setting the colors
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

        // Accent colors changed to darker olive-green/grey shades
        colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);        // Dark gray for check marks
        colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);       // Dark gray for sliders
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // Slightly lighter gray when active
        colors[ImGuiCol_Button] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);     // Button background (dark gray)
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);    // Button hover state
        colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);     // Button active state
        colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, .9f);           // Dark gray for menu headers
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);    // Slightly lighter on hover
        colors[ImGuiCol_HeaderActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);     // Lighter gray when active
        colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);        // Separators in dark gray
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f); // Resize grips in dark gray
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);        // Tabs background
        colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f); // Darker gray on hover
        colors[ImGuiCol_TabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        // Additional styles
        style.FramePadding = ImVec2(4.0f, 4.0f);
        style.ItemSpacing = ImVec2(4.0f, 4.0f);
        style.IndentSpacing = 20.0f;
        style.ScrollbarSize = 16.0f;
    }

    void initThumbnails(std::vector<VkDescriptorSet>& descriptorSets) {
        size = descriptorSets.size();
        thumbnails.resize(size);

        for (size_t i {0}; i < size; i++) {
            thumbnails[i].first = "";
            thumbnails[i].second = (ImTextureID)descriptorSets[i];
        }

        ImGuiStyle& style = ImGui::GetStyle();
        
        windowPaddingX = style.WindowPadding.x;
        style.WindowPadding.x = 0;
        style.WindowPadding.y = 0;
        framePaddingX = style.FramePadding.x;

        w = (WND_WIDTH - (10 * 3) - (marginX * numColumns) - (framePaddingX * numColumns * 2)) / numColumns;
        h = w * (9./16.);
        
        LOG(fmt::color::peru, "thumbnail - [width|{}] [height|{}]\n", w, h);
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
        if(ImGui::CollapsingHeader("settings")) {
            ImGui::Indent(marginX);
            ImGui::Text("Under maintenance...\nIn case you want to contribute feel free to do so via https://github.com/Ravry/glint");

            ImGui::Separator();

            ImGui::Text("active wallpaper:");
            ImGui::SameLine();
            if (selectedWallpaper >= 0) {
                if (ImGui::ImageButton("active_thumb", thumbnails[selectedWallpaper].second, ImVec2(240, 135))) {
                    externalSettingsOpened = true;
                }

                if (externalSettingsOpened) {
                    ImGui::Begin("Wallpaper Settings", &externalSettingsOpened);
                    ImGui::Spacing();
                    ImGui::Indent(marginX);
                    ImGui::Text("general");
                    ImGui::Spacing();
                    ImGui::Indent(marginX);
                    ImGui::InputInt("framerate", &fpsInput);
                    ImGui::ColorEdit3("tint", tintColor);
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

    void renderThumbnailGrid() {
        ImVec2 imageSize(w, h);

        ImGui::Indent(marginX);
        for (int i = 0; i < size; ++i)
        {
            if (ImGui::ImageButton(("##thumb" + std::to_string(i)).c_str(), thumbnails[i].second, imageSize)) {
                selectedWallpaper = i;
            }

            if ((i + 1) % numColumns != 0 && i != (size - 1))
                ImGui::SameLine(0, marginX);
        }
        ImGui::Unindent(marginX); 
        
    }

    void renderMediaHeader() {
        if(ImGui::CollapsingHeader("media", ImGuiTreeNodeFlags_DefaultOpen)) { 
            renderThumbnailGrid();
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
        ImGui::Spacing();
        std::string strFPS = "FPS: " + std::to_string(1. / deltaTime) + " ~ CC0 No Rights Reserved (https://creativecommons.org/public-domain/cc0/)";
        ImGui::Indent(marginX);
        ImGui::Text(strFPS.c_str()); 
        ImGui::Unindent(marginX);   
        ImGui::Spacing();
    }

    void renderWindowContent(GLFWwindow* window, double deltaTime) {
        ImGui::SetCursorPosY(titleBarHeight);
        ImVec2 childSize = ImVec2(windowSize.x, windowSize.y - titleBarHeight);
        ImGui::BeginChild("ContentRegion", childSize, false);
            
        renderSettingsHeader();
        renderMediaHeader();
        renderAboutHeader();
        renderFooter(deltaTime);

        ImGui::EndChild(); 
    }

    void renderWindow(GLFWwindow* window, double deltaTime, int width, int height) {
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

        renderCustomTitlebar(window);
        renderWindowContent(window, deltaTime);
        
        ImGui::End();
    }
}