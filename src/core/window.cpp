#define STB_IMAGE_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "window.h"

namespace Glint {
    void createWindow(WindowContext& windowContext, WindowCreateInfo& windowCreateInfo) {
        windowContext.type = windowCreateInfo.type;
        
        if (!glfwInit()) THROW("failed initializing glfw properly!");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        windowContext.window = glfwCreateWindow(windowCreateInfo.width, windowCreateInfo.height, windowCreateInfo.title, nullptr, nullptr);
        windowContext.handle = glfwGetWin32Window(windowContext.window);

        if (windowContext.type == WINDOW_WALLPAPER_TYPE) {
            MonitorDimensions monitorDimensions = getMonitorDimensions();
            HWND workerW = getWorkerwWindow();
            SetParent(windowContext.handle, workerW);
            SetWindowPos(windowContext.handle, 0, 0, 0, monitorDimensions.width(), monitorDimensions.height(), 0);
        }

        glfwSetWindowUserPointer(windowContext.window, &windowContext);

        glfwSetFramebufferSizeCallback(windowContext.window, [] (GLFWwindow *window, int width, int height) {
            WindowContext* _windowContext = reinterpret_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            _windowContext->resized = true;
        });

        windowContext.mvkContext.window = windowContext.window;
    }

    void cleanup(WindowContext& windowContext) {
        vkDeviceWaitIdle(windowContext.mvkContext.device);
        
        if (windowContext.type == WINDOW_DEFAULT_TYPE)
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();       
        }

        windowContext.mvkContext.destroy();
        glfwDestroyWindow(windowContext.window);
        glfwTerminate();

        if (windowContext.type == WINDOW_WALLPAPER_TYPE)
            SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, NULL, SPIF_SENDCHANGE);
    }

    void runWallpaperWindow(WindowContext& windowContext) {
        Mvk::createInstance(windowContext.mvkContext);
        Mvk::createSurface(windowContext.mvkContext, windowContext.window);
        Mvk::createDevice(windowContext.mvkContext);
        Mvk::createAllocatorVMA(windowContext.mvkContext);
        Mvk::createSwapchain(windowContext.mvkContext, VK_PRESENT_MODE_MAILBOX_KHR);

        Mvk::createDescriptorSetLayout(windowContext.mvkContext);
        Mvk::createPipeline(ASSETS_DIR "shader/standard/standard.vert.spv", ASSETS_DIR "shader/standard/standard.frag.spv", windowContext.mvkContext, true);
        Mvk::createCommandPool(windowContext.mvkContext);
        Mvk::createCommandBuffer(windowContext.mvkContext);

        Mvk::createVertexBuffer(windowContext.mvkContext);
        Mvk::createIndexBuffer(windowContext.mvkContext);
        Mvk::createUniformBuffers(windowContext.mvkContext, N);

        Mvk::createTexture(windowContext.mvkContext, 1920, 1080);

        Mvk::createDescriptorPool(windowContext.mvkContext);
        Mvk::allocateDescriptorSets(windowContext.mvkContext);

        Mvk::createFramebuffers(windowContext.mvkContext);
        Mvk::createSyncObjects(windowContext.mvkContext);

        uint32_t currentFrame{0};

        LOG(fmt::color::brown, "----------------------------------------\n");
        LOG(fmt::color::brown, "            loop-started                \n");
        LOG(fmt::color::brown, "----------------------------------------\n");

        double timerTime { 0. };
        size_t frameCount { 0 };

        while (!glfwWindowShouldClose(windowContext.window) && !windowClosed) {
            double startTime = glfwGetTime();
            auto frameStartTime = std::chrono::steady_clock::now();
            auto nextFrameTime = frameStartTime;
            {
                std::lock_guard<std::mutex> lock(MyImGUI::sharedSettingsMutex);
                nextFrameTime += std::chrono::milliseconds(static_cast<int64_t>(1000. / MyImGUI::sharedSettings.fps));
            }
            vkWaitForFences(windowContext.mvkContext.device, 1, &windowContext.mvkContext.inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);

            uint32_t imageIndex{0};
            VkResult result = vkAcquireNextImageKHR(windowContext.mvkContext.device, windowContext.mvkContext.swapchain, UINT64_MAX, windowContext.mvkContext.imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                recreateSwapchain(windowContext.mvkContext);
                return;
            }
            else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            {
                THROW("failed to acquire swap chain image!");
            }

            vkResetFences(windowContext.mvkContext.device, 1, &windowContext.mvkContext.inFlightFence[currentFrame]);

            vkResetCommandBuffer(windowContext.mvkContext.commandBuffer[currentFrame], 0);

            Mvk::recordCommandBufferWallpaper(windowContext.mvkContext, windowContext.mvkContext.commandBuffer[currentFrame], imageIndex, currentFrame);

            VkSemaphore waitSemaphores[] = {windowContext.mvkContext.imageAvailableSemaphore[currentFrame]};
            VkSemaphore signalSemaphores[] = {windowContext.mvkContext.renderFinishedSemaphore[currentFrame]};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &windowContext.mvkContext.commandBuffer[currentFrame];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            if (vkQueueSubmit(windowContext.mvkContext.graphicsQueue, 1, &submitInfo, windowContext.mvkContext.inFlightFence[currentFrame]) != VK_SUCCESS) {
                THROW("failed to submit draw command buffer!\n");
            }

            VkSwapchainKHR swapchains[]{windowContext.mvkContext.swapchain};

            VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapchains;
            presentInfo.pImageIndices = &imageIndex;

            result = vkQueuePresentKHR(windowContext.mvkContext.presentQueue, &presentInfo);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowContext.resized) {
                windowContext.resized = false;
                recreateSwapchain(windowContext.mvkContext);
            }
            else if (result != VK_SUCCESS) {
                THROW("failed to present swap chain image!\n");
            }

            std::this_thread::sleep_until(nextFrameTime);

            // ++frameCount;
            // timerTime += glfwGetTime() - startTime;
            // if (timerTime >= 1.) {
            //     LOG(fmt::color::aqua, "fps: {}\n", frameCount);
            //     timerTime = 0;
            //     frameCount = 0;
            // }
            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
    }

    void runSelectorWindow(WindowContext& windowContext) {
        Mvk::createInstance(windowContext.mvkContext);
        Mvk::createSurface(windowContext.mvkContext, windowContext.window);
        Mvk::createDevice(windowContext.mvkContext);
        Mvk::createAllocatorVMA(windowContext.mvkContext);
        Mvk::createSwapchain(windowContext.mvkContext, VK_PRESENT_MODE_FIFO_KHR);

        Mvk::createDescriptorSetLayoutUtil(windowContext.mvkContext, windowContext.mvkContext.descriptorSetLayout);
        Mvk::createPipeline(ASSETS_DIR "shader/default/default.vert.spv", ASSETS_DIR "shader/default/default.frag.spv", windowContext.mvkContext, false);

        Mvk::createCommandPool(windowContext.mvkContext);
        Mvk::createCommandBuffer(windowContext.mvkContext);

        Mvk::createDescriptorPoolImGUI(windowContext.mvkContext);
        
        Mvk::createFramebuffers(windowContext.mvkContext);
        
        Mvk::createSyncObjects(windowContext.mvkContext);
        
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();

            ImGui_ImplGlfw_InitForVulkan(windowContext.window, true);

            VkResult imguiResult;
            ImGui_ImplVulkan_InitInfo init_info{};
            init_info.Instance = windowContext.mvkContext.instance;
            init_info.PhysicalDevice = windowContext.mvkContext.physicalDevice;
            init_info.Device = windowContext.mvkContext.device;
            init_info.Queue = windowContext.mvkContext.graphicsQueue;
            init_info.DescriptorPool = windowContext.mvkContext.imguiDescriptorPool;
            init_info.MinImageCount = windowContext.mvkContext.swapchainDetails.capabilites.minImageCount;
            init_info.ImageCount = (windowContext.mvkContext.swapchainDetails.capabilites.minImageCount + 1);
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = nullptr;
            init_info.CheckVkResultFn = [](VkResult result) {};
            init_info.RenderPass = windowContext.mvkContext.renderpass;

            ImGui_ImplVulkan_Init(&init_info);

            MyImGUI::SetupImGuiStyle();
            ImGui_ImplVulkan_CreateFontsTexture();

            std::vector<std::string> directoryContent = readDirectoryContent(ASSETS_DIR "videos/");
            size_t directoryContentCount{directoryContent.size()};
            windowContext.mvkContext.images.resize(directoryContentCount);
            windowContext.mvkContext.imageAllocations.resize(directoryContentCount);
            windowContext.mvkContext.imageViews.resize(directoryContentCount);
            windowContext.mvkContext.imageSamplers.resize(directoryContentCount);

            for (size_t i{0}; i < directoryContent.size(); i++)
            {
                LOG(fmt::color::light_steel_blue, "directory content - {}\n", directoryContent[i]);
                uint8_t *data = getMediaThumbnail(directoryContent[i].c_str());
                Mvk::createTextureFromData(windowContext.mvkContext, windowContext.mvkContext.images[i], windowContext.mvkContext.imageAllocations[i], windowContext.mvkContext.imageViews[i], windowContext.mvkContext.imageSamplers[i], data, 174, 97);
            }

            Mvk::createDescriptorPoolUtil(windowContext.mvkContext, windowContext.mvkContext.imageDescriptorPool, directoryContentCount);
            std::vector<VkDescriptorSet> descriptorSets = Mvk::allocateDescriptorSetsUtil(windowContext.mvkContext, windowContext.mvkContext.descriptorSetLayout, windowContext.mvkContext.imageDescriptorPool, windowContext.mvkContext.imageViews, windowContext.mvkContext.imageSamplers);
            MyImGUI::initThumbnails(descriptorSets, directoryContent);
        }

        uint32_t currentFrame{0};

        LOG(fmt::color::brown, "----------------------------------------\n");
        LOG(fmt::color::brown, "            loop-started                \n");
        LOG(fmt::color::brown, "----------------------------------------\n");

        while (!glfwWindowShouldClose(windowContext.window))
        {
            glfwPollEvents();
            vkWaitForFences(windowContext.mvkContext.device, 1, &windowContext.mvkContext.inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);

            uint32_t imageIndex{0};
            VkResult result = vkAcquireNextImageKHR(windowContext.mvkContext.device, windowContext.mvkContext.swapchain, UINT64_MAX, windowContext.mvkContext.imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                recreateSwapchain(windowContext.mvkContext);
                return;
            }
            else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                THROW("failed to acquire swap chain image!");
            }

            vkResetFences(windowContext.mvkContext.device, 1, &windowContext.mvkContext.inFlightFence[currentFrame]);

            vkResetCommandBuffer(windowContext.mvkContext.commandBuffer[currentFrame], 0);

            Mvk::recordCommandBufferSelector(windowContext.mvkContext, windowContext.mvkContext.commandBuffer[currentFrame], imageIndex, currentFrame);

            VkSemaphore waitSemaphores[] = {windowContext.mvkContext.imageAvailableSemaphore[currentFrame]};
            VkSemaphore signalSemaphores[] = {windowContext.mvkContext.renderFinishedSemaphore[currentFrame]};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &windowContext.mvkContext.commandBuffer[currentFrame];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            if (vkQueueSubmit(windowContext.mvkContext.graphicsQueue, 1, &submitInfo, windowContext.mvkContext.inFlightFence[currentFrame]) != VK_SUCCESS)
            {
                THROW("failed to submit draw command buffer!\n");
            }

            VkSwapchainKHR swapchains[]{windowContext.mvkContext.swapchain};

            VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapchains;
            presentInfo.pImageIndices = &imageIndex;

            result = vkQueuePresentKHR(windowContext.mvkContext.presentQueue, &presentInfo);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowContext.resized)
            {
                windowContext.resized = false;
                recreateSwapchain(windowContext.mvkContext);
            }
            else if (result != VK_SUCCESS)
            {
                THROW("failed to present swap chain image!\n");
            }

            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        windowClosed = true;
    }

    void runWindow(WindowContext& windowContext) {
        if (windowContext.type == WINDOW_WALLPAPER_TYPE) {
            // MonitorDimensions monitorDimensions = getMonitorDimensions();
            // LOG(fmt::color::pink, "monitor-count: [count: {}]; monitor-dimensions: [width: {}][height: {}]\n", monitorDimensions.count, monitorDimensions.width(), monitorDimensions.height());
            // HWND workerwHandle = getWorkerwWindow();
            // SetParent(windowContext.handle, workerwHandle);
            // SetWindowPos(windowContext.handle, HWND_TOP, 0, 0, monitorDimensions.width(), monitorDimensions.height(), SWP_SHOWWINDOW);
            runWallpaperWindow(windowContext);
        } else {
            runSelectorWindow(windowContext);
        }

        LOG(fmt::color::brown, "----------------------------------------\n");
        LOG(fmt::color::brown, "            loop-ended\n");
        LOG(fmt::color::brown, "----------------------------------------\n");

        cleanup(windowContext);
    }
}