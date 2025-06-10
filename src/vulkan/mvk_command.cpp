#include "mvk_core.h"

namespace Mvk {
    void createCommandPool(Context& context) {
        VkCommandPoolCreateInfo commandPoolInfo { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.queueFamilyIndex = context.queueFamilyIndices.graphicsFamily.value();
        
        if (vkCreateCommandPool(context.device, &commandPoolInfo, 0, &context.commandPool) != VK_SUCCESS) {
            THROW("failed to create command pool!");
        }
    }

    void createCommandBuffer(Context &context)
    {
        context.commandBuffer.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.commandPool = context.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
       
        if (vkAllocateCommandBuffers(context.device, &allocInfo, context.commandBuffer.data()) != VK_SUCCESS) {
            THROW("failed to allocate command buffers!");
        }
    }

    FrameData frameData {};
    void recordCommandBuffer(Context &context, VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
    {        
        const float hWidth = context.swapchainExtent.width / 2.f;
        const float hHeight = context.swapchainExtent.height / 2.f;

        VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) THROW("failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderpassInfo { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderpassInfo.renderPass = context.renderpass;
        renderpassInfo.framebuffer = context.swapchainFramebuffers[imageIndex];
        renderpassInfo.renderArea.offset = {0, 0};
        renderpassInfo.renderArea.extent = context.swapchainExtent;
        VkClearValue clearColor = {{{0.f, 0.f, 0.f , 1.f}}};
        renderpassInfo.clearValueCount = 1;
        renderpassInfo.pClearValues = &clearColor;
        
        vkCmdBeginRenderPass(commandBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipeline);

        VkViewport viewport {};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)context.swapchainExtent.width;
        viewport.height = (float)context.swapchainExtent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor {};
        scissor.offset = {0, 0};
        scissor.extent = context.swapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        {
            //note to myself: important otherwise memory leak
            {
                std::lock_guard<std::mutex> lock(frameDataMutex);
                if (frameData.pixels) {
                    delete[] static_cast<uint8_t*>(frameData.pixels);
                    frameData.pixels = nullptr;
                }
            }
                
            {
                std::unique_lock<std::mutex> lock(frameQueueMutex);
                if (frameQueue.size() > 0) {
                    {
                        std::lock_guard<std::mutex> lock(frameDataMutex);
                        frameData = frameQueue.front();
                        frameQueue.pop();
                    }
                    // updateTextureImageDataDynamic(context, frameData.pixels);
                    // LOG(fmt::color::lime, "consumer received frame data (from producer) - size: {}\n", frameQueue.size());                
                }   
            }
            if (frameQueue.size() < 5) {
                frameCond.notify_one();
            }
        }

        // VkBuffer vertexBuffers[] { context.vertexBuffer };
        // VkDeviceSize offsets[] = { 0 };

        // vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        // vkCmdBindIndexBuffer(commandBuffer, context.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        
        // VkDeviceSize alignment = context.physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
        // VkDeviceSize alignedSize = sizeof(UniformBufferObject);
        // if (alignment > 0) alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);
        
        // uint8_t* destination = reinterpret_cast<uint8_t*>(context.uniformBuffersMapped[0][currentFrame]);
        // for (size_t i {0}; i < N; i++) {
        //     UniformBufferObject ubo {};
        //     float tileWidth = hWidth / N;
        //     ubo.model = glm::scale(
        //         glm::translate(glm::mat4(1), glm::vec3(((i/static_cast<float>(N)) + .25f) * context.swapchainExtent.width, hHeight, 0.f)),
        //         glm::vec3(tileWidth, hHeight, 1)
        //     );
        //     ubo.view = glm::mat4(1);
        //     ubo.projection = glm::ortho(0.f, static_cast<float>(context.swapchainExtent.width), 0.f, static_cast<float>(context.swapchainExtent.height));

        //     memcpy(destination + i * alignedSize, &ubo, sizeof(UniformBufferObject));
        // }

        // for (size_t i {0}; i < N; i++) {
        //     uint32_t dynamicOffset = i * alignedSize;
        //     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipelineLayout, 0, 1, &context.descriptorSets[0][currentFrame], 1, &dynamicOffset);
        //     vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        // }
        




        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            static bool firstFrame = true;
            if (firstFrame) {
                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(ImVec2(context.swapchainExtent.width, context.swapchainExtent.height));
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
            
            
            const float titleBarHeight = 30.0f;
            const ImVec2 windowPos = ImGui::GetWindowPos();
            const ImVec2 windowSize = ImGui::GetWindowSize();

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 titleBarMin = windowPos;
            ImVec2 titleBarMax = ImVec2(windowPos.x + windowSize.x, windowPos.y + titleBarHeight);
            drawList->AddRectFilled(titleBarMin, titleBarMax, IM_COL32(60, 60, 60, 255));

            static bool dragging = false;
            static ImVec2 dragOffset;

            ImVec2 dragZoneSize = ImVec2(windowSize.x - titleBarHeight * 3, titleBarHeight);
            ImGui::InvisibleButton("drag_zone", dragZoneSize);

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                dragging = true;
                double cursorX, cursorY;
                glfwGetCursorPos(context.window, &cursorX, &cursorY);
                dragOffset = ImVec2((float)cursorX, (float)cursorY);
            }

            if (dragging && ImGui::IsMouseDown(0)) {
                double cursorX, cursorY;
                glfwGetCursorPos(context.window, &cursorX, &cursorY);

                int winX, winY;
                glfwGetWindowPos(context.window, &winX, &winY);

                int newX = winX + static_cast<int>(cursorX - dragOffset.x);
                int newY = winY + static_cast<int>(cursorY - dragOffset.y);

                glfwSetWindowPos(context.window, newX, newY);
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
                glfwSetWindowShouldClose(context.window, GLFW_TRUE);
            }
            
            ImGui::PopStyleColor(2);
            
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.7f, 1.0f));

            ImGui::SetCursorPos(ImVec2(windowSize.x - titleBarHeight * 2, 0));

            if (ImGui::Button("-", ImVec2(titleBarHeight, titleBarHeight))) {
                glfwIconifyWindow(context.window);
            }

            ImGui::PopStyleColor(3);
    
            ImGui::SetCursorPosY(titleBarHeight);
            ImVec2 childSize = ImVec2(windowSize.x, windowSize.y - titleBarHeight);
            ImGui::BeginChild("ContentRegion", childSize, false);

            constexpr float marginX = 10.f;
             
            if(ImGui::CollapsingHeader("settings")) {
                ImGui::Indent(marginX);
                ImGui::Text("Under maintenance...\nIn case you want to contribute feel free to do so via https://github.com/Ravry/glint");
                ImGui::Unindent(marginX);
            }
            
            if(ImGui::CollapsingHeader("media")) {
                MyImGUI::renderThumbnailGrid();
            }

            if(ImGui::CollapsingHeader("about")) {
                ImGui::Indent(marginX);
                ImGui::Text("This open-source wallpaper engine was made by https://github.com/Ravry.\nIn case you want to contribute feel free to do so via https://github.com/Ravry/glint\nThe used resources are fmt, glfw, vulkan-api, glm and ffmpeg.");
                ImGui::Unindent(marginX);
                ImGui::Separator();
            }

            ImGui::Spacing();
            std::string strFPS = "FPS: " + std::to_string(1. / context.deltaTime);
            ImGui::Indent(marginX);
            ImGui::Text(strFPS.c_str()); 
            ImGui::Unindent(marginX);   
            ImGui::Spacing();

            ImGui::EndChild(); 

            ImGui::End();

            ImGui::Render();
            ImDrawData* draw_data = ImGui::GetDrawData();
            ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
        }

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) THROW("failed to record command buffer!");
    }

    void createSyncObjects(Context& context) {
        context.imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
        context.renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
        context.inFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

        VkFenceCreateInfo fenceInfo { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i {0}; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &context.imageAvailableSemaphore[i]) != VK_SUCCESS ||
                vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &context.renderFinishedSemaphore[i]) != VK_SUCCESS ||
                vkCreateFence(context.device, &fenceInfo, nullptr, &context.inFlightFence[i]) != VK_SUCCESS) {
                THROW("failed to create semaphores!");
            }
        }
    }
};