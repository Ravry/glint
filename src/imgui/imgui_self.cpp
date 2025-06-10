#include "imgui_self.h"

ImTextureID* thumbnails;
int size;

void initThumbnails(std::vector<VkDescriptorSet>& descriptorSets) {
    size = descriptorSets.size();

    thumbnails = new ImTextureID[descriptorSets.size()];
    for (size_t i {0}; i < descriptorSets.size(); i++) {
        thumbnails[i] = (ImTextureID)descriptorSets[i];
    }
}

void renderThumbnailGrid() {
    int numThumbnails = sizeof(thumbnails) / sizeof(thumbnails[0]);
    int numColumns = 4; // Number of columns in your grid
    int w = WND_WIDTH / numColumns;
    int h = w * (9./16.);
    ImVec2 imageSize(WND_WIDTH / numColumns, h); // Size of each thumbnail image

    for (int i = 0; i < size; ++i)
    {
        if (ImGui::ImageButton(("##thumb" + std::to_string(i)).c_str(), thumbnails[i], imageSize))
        {

        }

        // Put SameLine() after every image except the last in the row
        if ((i + 1) % numColumns != 0)
            ImGui::SameLine();
    }
}