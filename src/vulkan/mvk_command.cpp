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

    FrameData frameData{};
    void recordCommandBufferWallpaper(Context &context, VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t currentFrame, WorkerWs &workerWs) {
        const float hWidth = context.swapchainExtent.width / 2.f;
        const float hHeight = context.swapchainExtent.height / 2.f;

        VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            THROW("failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderpassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        renderpassInfo.renderPass = context.renderpass;
        renderpassInfo.framebuffer = context.swapchainFramebuffers[imageIndex];
        renderpassInfo.renderArea.offset = {0, 0};
        renderpassInfo.renderArea.extent = context.swapchainExtent;
        VkClearValue clearColor = {{{0.f, 0.f, 0.f, 1.f}}};
        renderpassInfo.clearValueCount = 1;
        renderpassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipeline);

        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)context.swapchainExtent.width;
        viewport.height = (float)context.swapchainExtent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = context.swapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        {
            // note to myself: important otherwise memory leak
            {
                std::lock_guard<std::mutex> lock(frameDataMutex);
                if (frameData.pixels)
                {
                    delete[] static_cast<uint8_t *>(frameData.pixels);
                    frameData.pixels = nullptr;
                }
            }

            {
                std::unique_lock<std::mutex> lock(frameQueueMutex);
                if (frameQueue.size() > 0)
                {
                    {
                        std::lock_guard<std::mutex> lock(frameDataMutex);
                        frameData = frameQueue.front();
                        frameQueue.pop();
                    }

                    bool onlyPlayFocused { false };
                    {
                        std::lock_guard<std::mutex> lock(MyImGUI::sharedSettingsMutex);
                        onlyPlayFocused = MyImGUI::sharedSettings.onlyPlayWhenFocused;
                    }

                    if (onlyPlayFocused) {
                        if (workerWs.focus == workerWs.surW)
                            updateTextureImageDataDynamic(context, frameData.pixels, frameData.width, frameData.height);
                    } else {
                        updateTextureImageDataDynamic(context, frameData.pixels, frameData.width, frameData.height);
                    }
                }
            }
            if (frameQueue.size() < 5)
            {
                frameCond.notify_one();
            }
        }

        VkBuffer vertexBuffers[] { context.vertexBuffer };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, context.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        VkDeviceSize alignment = context.physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
        VkDeviceSize alignedSize = sizeof(UniformBufferObject);
        if (alignment > 0) alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);

        uint8_t* destination = reinterpret_cast<uint8_t*>(context.uniformBuffersMapped[0][currentFrame]);
        for (size_t i {0}; i < N; i++) {
            UniformBufferObject ubo {};
            float tileWidth = hWidth / N;
            ubo.model = glm::scale(
                glm::translate(glm::mat4(1), glm::vec3(((i/static_cast<float>(N)) + .25f) * context.swapchainExtent.width, hHeight, 0.f)),
                glm::vec3(tileWidth, hHeight, 1)
            );
            ubo.view = glm::mat4(1);
            ubo.projection = glm::ortho(0.f, static_cast<float>(context.swapchainExtent.width), 0.f, static_cast<float>(context.swapchainExtent.height));
            
            glm::vec3 tintVec;
            {
                std::lock_guard<std::mutex> lock(MyImGUI::sharedSettingsMutex);
                tintVec.x = MyImGUI::sharedSettings.tintColor[0];
                tintVec.y = MyImGUI::sharedSettings.tintColor[1];
                tintVec.z = MyImGUI::sharedSettings.tintColor[2];
            }
            ubo.tint = tintVec;

            memcpy(destination + i * alignedSize, &ubo, sizeof(UniformBufferObject));
        }

        for (size_t i {0}; i < N; i++) {
            uint32_t dynamicOffset = i * alignedSize;
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipelineLayout, 0, 1, &context.descriptorSets[0][currentFrame], 1, &dynamicOffset);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            THROW("failed to record command buffer!");
    }

    void recordCommandBufferSelector(Context &context, VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
    {        
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
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            MyImGUI::renderWindow(context, context.deltaTime, context.swapchainExtent.width, context.swapchainExtent.height);
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