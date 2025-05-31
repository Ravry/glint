#include "mvk_core.h"

namespace Mvk {
    void createSurface(Context& context, GLFWwindow* window) {
        if (glfwCreateWindowSurface(context.instance, window, 0, &context.surface) != VK_SUCCESS) {
            THROW("failed to create window surface!");
        }

        
    }
};