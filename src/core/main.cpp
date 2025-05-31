#include "window.h"

constexpr size_t WND_WIDTH = 800;
constexpr size_t WND_HEIGHT = 600;

int main(int argv, char** argc) {
    Window wallpaperWindow("window", WND_WIDTH, WND_HEIGHT);

    try {
        wallpaperWindow.run();
    } catch (std::exception& ex) {
        fmt::print(fmt::fg(fmt::color::blue) | fmt::emphasis::bold, ex.what());
        return -1;
    }

    return 0;
}