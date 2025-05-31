#pragma once
#include <thread>
#include <chrono>
#include "mvk_core.h"
#include "utils.h"

class Window {
private:
    GLFWwindow* window;
    HWND handle;

    Mvk::Context mvkContext {};
public:
    bool resized {false};

    Window(const char* title, int width, int height);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window&) = delete;

    void run();

    #pragma region Getters
    GLFWwindow* getWindowGLFW() {
        return window;
    }
    HWND getWindowHandle() {
        return handle;
    }
    #pragma endregion Getters
};