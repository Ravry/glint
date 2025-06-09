#include "window.h"

constexpr size_t WND_WIDTH = 800;
constexpr size_t WND_HEIGHT = 600;

int main(int argv, char** argc) {
    std::thread media_thread(media_func, ASSETS_DIR "videos/test.mp4");

    Window wallpaperWindow("window", WND_WIDTH, WND_HEIGHT);

    try {
        wallpaperWindow.run();
    } catch (std::exception& ex) {
        fmt::print(fmt::fg(fmt::color::blue) | fmt::emphasis::bold, ex.what());
        return -1;
    }

    media_thread.join();

    return 0;
}