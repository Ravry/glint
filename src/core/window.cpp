#define STB_IMAGE_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "window.h"

namespace Glint {
    void createWindow(WindowContext& windowContext, WindowCreateInfo& windowCreateInfo) {
        windowContext.type = windowCreateInfo.type;

        HWND workerwHandle = getWorkerwWindow();
        
        if (!glfwInit()) THROW("failed initializing glfw properly!");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

        if (windowCreateInfo.type == WINDOW_WALLPAPER_TYPE) {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        }
        
        windowContext.window = glfwCreateWindow(windowCreateInfo.width, windowCreateInfo.height, windowCreateInfo.title, nullptr, nullptr);
        windowContext.handle = glfwGetWin32Window(windowContext.window);

        MonitorDimensions monitorDimensions = getMonitorDimensions();
        LOG(fmt::color::pink, "monitor-count: [count: {}]; monitor-dimensions: [width: {}][height: {}]\n", monitorDimensions.count, monitorDimensions.width(), monitorDimensions.height());

        if (windowCreateInfo.type == WINDOW_WALLPAPER_TYPE) {
            SetParent(windowContext.handle, workerwHandle);
            SetWindowPos(windowContext.handle, HWND_TOP, 0, 0, monitorDimensions.width(), monitorDimensions.height(), SWP_SHOWWINDOW);
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
        
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        windowContext.mvkContext.destroy();
        glfwDestroyWindow(windowContext.window);
        glfwTerminate();
    }

    void runWindow(WindowContext& windowContext) {
        Mvk::createInstance(windowContext.mvkContext);
        Mvk::createSurface(windowContext.mvkContext, windowContext.window);
        Mvk::createDevice(windowContext.mvkContext);

        Mvk::createAllocatorVMA(windowContext.mvkContext);

        Mvk::createSwapchain(windowContext.mvkContext);

        // Mvk::createDescriptorSetLayout(windowContext.mvkContext);

        Mvk::createDescriptorSetLayoutUtil(windowContext.mvkContext, windowContext.mvkContext.descriptorSetLayout);

        Mvk::createPipeline(ASSETS_DIR "shader/standard.vert.spv", ASSETS_DIR "shader/standard.frag.spv", windowContext.mvkContext);   
        
        Mvk::createCommandPool(windowContext.mvkContext);
        Mvk::createCommandBuffer(windowContext.mvkContext);
        
        Mvk::createVertexBuffer(windowContext.mvkContext);
        Mvk::createIndexBuffer(windowContext.mvkContext);
        Mvk::createUniformBuffers(windowContext.mvkContext, N);
        // Mvk::createDescriptorPool(windowContext.mvkContext);

        Mvk::createDescriptorPoolImGUI(windowContext.mvkContext);
        // Mvk::allocateDescriptorSets(windowContext.mvkContext);

        Mvk::createFramebuffers(windowContext.mvkContext);
        
        Mvk::createSyncObjects(windowContext.mvkContext);

        if (windowContext.type == WINDOW_DEFAULT_TYPE) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();

            ImGui_ImplGlfw_InitForVulkan(windowContext.window, true);

            VkResult imguiResult;
            ImGui_ImplVulkan_InitInfo init_info {};
            init_info.Instance = windowContext.mvkContext.instance;
            init_info.PhysicalDevice = windowContext.mvkContext.physicalDevice;
            init_info.Device = windowContext.mvkContext.device;
            init_info.Queue = windowContext.mvkContext.graphicsQueue;
            init_info.DescriptorPool = windowContext.mvkContext.imguiDescriptorPool;
            init_info.MinImageCount = windowContext.mvkContext.swapchainDetails.capabilites.minImageCount;
            init_info.ImageCount = (windowContext.mvkContext.swapchainDetails.capabilites.minImageCount + 1);
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = nullptr;
            init_info.CheckVkResultFn = [](VkResult result){};
            init_info.RenderPass = windowContext.mvkContext.renderpass;

            ImGui_ImplVulkan_Init(&init_info);
            
            SetupImGuiStyle();
            ImGui_ImplVulkan_CreateFontsTexture();
        

            std::vector<std::string> directoryContent = readDirectoryContent(ASSETS_DIR "videos/");
            
            std::vector<VkImage> images(directoryContent.size());
            std::vector<VmaAllocation> imageAllocations(directoryContent.size());
            std::vector<VkImageView> imageViews(directoryContent.size());
            std::vector<VkSampler> imageSamplers(directoryContent.size());    

            for (size_t i {0}; i < directoryContent.size(); i++) {
                LOG(fmt::color::light_steel_blue, "directory content - {}\n", directoryContent[i]);
                uint8_t* data = getMediaThumbnail(directoryContent[i].c_str());
                Mvk::createTextureFromData(windowContext.mvkContext, images[i], imageAllocations[i], imageViews[i], imageSamplers[i], data, 320, 180);
            }
        
            VkDescriptorPool descriptorPool;

            Mvk::createDescriptorPoolUtil(windowContext.mvkContext, descriptorPool);
            std::vector<VkDescriptorSet> descriptorSets = Mvk::allocateDescriptorSetsUtil(windowContext.mvkContext, windowContext.mvkContext.descriptorSetLayout, descriptorPool, imageViews, imageSamplers);
            initThumbnails(descriptorSets);
        }

        double lastTime = glfwGetTime();

        uint32_t currentFrame {0};

        LOG(fmt::color::brown, "----------------------------------------\n");
        LOG(fmt::color::brown, "            loop-started\n");
        LOG(fmt::color::brown, "----------------------------------------\n");

        while (!glfwWindowShouldClose(windowContext.window)) {
            double currentTime = glfwGetTime();
            double deltaTime = currentTime - lastTime;
            lastTime = currentTime;
            windowContext.mvkContext.deltaTime = static_cast<float>(deltaTime);
            // LOG(fmt::color::white, "deltaTime: {:.4f}s; fps: {}\n", deltaTime, static_cast<unsigned int>(1.f/deltaTime));
            
            glfwPollEvents();

            vkWaitForFences(windowContext.mvkContext.device, 1, &windowContext.mvkContext.inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);

            uint32_t imageIndex {0};
            VkResult result = vkAcquireNextImageKHR(windowContext.mvkContext.device, windowContext.mvkContext.swapchain, UINT64_MAX, windowContext.mvkContext.imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                recreateSwapchain(windowContext.mvkContext);
                return;
            } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                THROW("failed to acquire swap chain image!");
            }

            vkResetFences(windowContext.mvkContext.device, 1, &windowContext.mvkContext.inFlightFence[currentFrame]); 

            vkResetCommandBuffer(windowContext.mvkContext.commandBuffer[currentFrame], 0);

            Mvk::recordCommandBuffer(windowContext.mvkContext, windowContext.mvkContext.commandBuffer[currentFrame], imageIndex, currentFrame);
            
            VkSemaphore waitSemaphores[] = { windowContext.mvkContext.imageAvailableSemaphore[currentFrame] };
            VkSemaphore signalSemaphores[] = { windowContext.mvkContext.renderFinishedSemaphore[currentFrame] };
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

            VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
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

            VkSwapchainKHR swapchains[] { windowContext.mvkContext.swapchain };
            
            VkPresentInfoKHR presentInfo { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapchains;
            presentInfo.pImageIndices = &imageIndex;

            result = vkQueuePresentKHR(windowContext.mvkContext.presentQueue, &presentInfo);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowContext.resized) {
                windowContext.resized = false;
                recreateSwapchain(windowContext.mvkContext);
            } else if (result != VK_SUCCESS) {
                THROW("failed to present swap chain image!\n");
            }

            const double targetFPS = 165.;
            const double targetHz = 1. / targetFPS;
            double frameEndTime = glfwGetTime();
            double actualHz = frameEndTime - currentTime;
            
            if (actualHz < targetHz) {
                double sleepTime = targetHz - actualHz;
                std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
            }

            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        
        LOG(fmt::color::brown, "----------------------------------------\n");
        LOG(fmt::color::brown, "            loop-ended\n");
        LOG(fmt::color::brown, "----------------------------------------\n");
        
        windowClosed = true;
        cleanup(windowContext);
    }
}