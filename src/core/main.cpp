#include "window.h"

int main(int argv, char** argc) {
    std::thread media_thread(media_func, ASSETS_DIR "videos/another.mp4");

    Glint::WindowContext windowContext {};
    Glint::WindowCreateInfo windowCreateInfo {
        .type = Glint::WINDOW_DEFAULT_TYPE,
        .title = "wallpaper window",
        .width = WND_WIDTH,
        .height = WND_HEIGHT
    };
    Glint::createWindow(windowContext, windowCreateInfo);

    // Glint::WindowContext windowContext2 {};
    // Glint::WindowCreateInfo windowCreateInfo2 {
    //     .type = Glint::WINDOW_DEFAULT_TYPE,
    //     .title = "default window",
    //     .width = WND_WIDTH,
    //     .height = WND_HEIGHT
    // };

    // Glint::createWindow(windowContext2, windowCreateInfo2);

    // std::thread window_thread([&] () {
    //     try {
    //         Glint::runWindow(windowContext2);
    //     } catch (std::exception& ex) {
    //         fmt::print(fmt::fg(fmt::color::blue) | fmt::emphasis::bold, ex.what());
    //         return -1;
    //     }    
    // });

    try {
        Glint::runWindow(windowContext);
    } catch (std::exception& ex) {
        fmt::print(fmt::fg(fmt::color::blue) | fmt::emphasis::bold, ex.what());
        return -1;
    }

    media_thread.join();
    // window_thread.join();

    return 0;
}