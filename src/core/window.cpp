#define STB_IMAGE_IMPLEMENTATION
#include "window.h"

Window::Window(const char* title, int width, int height) {
    HWND workerwHandle = getWorkerwWindow();
    
    if (!glfwInit()) {
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    handle = glfwGetWin32Window(window);

    // SetParent(handle, workerwHandle);

    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, [] (GLFWwindow *window, int width, int height) {
        Window* _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        _window->resized = true;
    });

    mvkContext.window = window;
    Mvk::createInstance(mvkContext);
    Mvk::createSurface(mvkContext, window);
    Mvk::createDevice(mvkContext);
    Mvk::createSwapchain(mvkContext);

    Mvk::createDescriptorSetLayout(mvkContext);

    Mvk::createPipeline(ASSETS_DIR "shader/standard.vert.spv", ASSETS_DIR "shader/standard.frag.spv", mvkContext);   
    
    Mvk::createCommandPool(mvkContext);
    Mvk::createCommandBuffer(mvkContext);
    
    Mvk::createVertexBuffer(mvkContext);
    Mvk::createIndexBuffer(mvkContext);
    Mvk::createUniformBuffers(mvkContext);

    Mvk::createTextureImage(mvkContext, ASSETS_DIR "img/statue.jpg");

    Mvk::createDescriptorPool(mvkContext);
    Mvk::allocateDescriptorSets(mvkContext);

    Mvk::createFramebuffers(mvkContext);
    
    Mvk::createSyncObjects(mvkContext);
}

void Window::run() {
    double lastTime = glfwGetTime();

    uint32_t currentFrame {0};

    LOG(fmt::color::brown, "----------------------------------------\n");
    LOG(fmt::color::brown, "            loop-started\n");
    LOG(fmt::color::brown, "----------------------------------------\n");
    
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        mvkContext.deltaTime = static_cast<float>(deltaTime);
        // LOG(fmt::color::white, "deltaTime: {:.4f}s; fps: {}\n", deltaTime, static_cast<unsigned int>(1.f/deltaTime));

        glfwPollEvents();

        vkWaitForFences(mvkContext.device, 1, &mvkContext.inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex {0};
        VkResult result = vkAcquireNextImageKHR(mvkContext.device, mvkContext.swapchain, UINT64_MAX, mvkContext.imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapchain(mvkContext);
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            THROW("failed to acquire swap chain image!");
        }

        vkResetFences(mvkContext.device, 1, &mvkContext.inFlightFence[currentFrame]); 

        vkResetCommandBuffer(mvkContext.commandBuffer[currentFrame], 0);

        Mvk::recordCommandBuffer(mvkContext, mvkContext.commandBuffer[currentFrame], imageIndex, currentFrame);
        
        VkSemaphore waitSemaphores[] = { mvkContext.imageAvailableSemaphore[currentFrame] };
        VkSemaphore signalSemaphores[] = { mvkContext.renderFinishedSemaphore[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mvkContext.commandBuffer[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(mvkContext.graphicsQueue, 1, &submitInfo, mvkContext.inFlightFence[currentFrame]) != VK_SUCCESS) {
            THROW("failed to submit draw command buffer!\n");
        }

        VkSwapchainKHR swapchains[] { mvkContext.swapchain };
        
        VkPresentInfoKHR presentInfo { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(mvkContext.presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized) {
            resized = false;
            recreateSwapchain(mvkContext);
        } else if (result != VK_SUCCESS) {
            THROW("failed to present swap chain image!\n");
        }

        const double targetFPS = 60.;
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
}

Window::~Window() {
    vkDeviceWaitIdle(mvkContext.device);
    mvkContext.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
}