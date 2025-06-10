#include "imgui_self.h"

void renderMenuBar() {
    ImGui::BeginMenuBar();
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "Ctrl+O")) { /* Open action */ }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Save action */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) { /* Exit action */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* Undo action */ }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) { /* Redo action */ }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}


std::vector<const char*> mediaFiles {
    ASSETS_DIR "videos/test.mp4"
};

std::vector<VkImage> mediaThumbnails;


ImTextureID* thumbnails;

void initThumbnails(VkDescriptorSet& descriptorSet) {
    thumbnails = new ImTextureID[1]{ (ImTextureID)descriptorSet };
}

void renderThumbnailGrid() {
    int numThumbnails = sizeof(thumbnails) / sizeof(thumbnails[0]);
    int numColumns = 4; // Number of columns in your grid
    ImVec2 imageSize(320, 180); // Size of each thumbnail image

    for (int i = 0; i < 1; ++i)
    {
        if (ImGui::ImageButton("test", thumbnails[i], imageSize))
        {

        }

        // Put SameLine() after every image except the last in the row
        if ((i + 1) % numColumns != 0)
            ImGui::SameLine();
    }
}