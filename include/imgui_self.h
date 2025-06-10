#pragma once
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "media.h"

void renderMenuBar();
void initThumbnails(VkDescriptorSet& descriptorSet);
void renderThumbnailGrid();