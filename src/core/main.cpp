#include "window.h"

int main(int argv, char** argc) {
    /* ██╗    ██╗ █████╗ ██╗     ██╗     ██████╗  █████╗ ██████╗ ███████╗██████╗  */
    /* ██║    ██║██╔══██╗██║     ██║     ██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔══██╗ */
    /* ██║ █╗ ██║███████║██║     ██║     ██████╔╝███████║██████╔╝█████╗  ██████╔╝ */
    /* ██║███╗██║██╔══██║██║     ██║     ██╔═══╝ ██╔══██║██╔═══╝ ██╔══╝  ██╔══██╗ */
    /* ╚███╔███╔╝██║  ██║███████╗███████╗██║     ██║  ██║██║     ███████╗██║  ██║ */
    /*  ╚══╝╚══╝ ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚═╝  ╚═╝╚═╝     ╚══════╝╚═╝  ╚═╝ */
    Glint::WindowContext wallpaperWindowContext {};
    Glint::WindowCreateInfo wallpaperWindowCreateInfo {
        .type = Glint::WINDOW_WALLPAPER_TYPE,
        .title = "wallpaper[glint]",
        .width = WND_WIDTH,
        .height = WND_HEIGHT
    };
    Glint::createWindow(wallpaperWindowContext, wallpaperWindowCreateInfo);
    std::thread wallpaperWindow_thread([&] () {
        try {
            std::thread media_thread(media_func, ASSETS_DIR "videos/another.mp4");
            Glint::runWindow(wallpaperWindowContext); 
            media_thread.join();
        }
        catch (std::exception& ex) { fmt::print(fmt::fg(fmt::color::blue) | fmt::emphasis::bold, ex.what()); return -1; }
        return 0;
    });

    /* ███████╗███████╗██╗     ███████╗ ██████╗████████╗ ██████╗ ██████╗  */
    /* ██╔════╝██╔════╝██║     ██╔════╝██╔════╝╚══██╔══╝██╔═══██╗██╔══██╗ */
    /* ███████╗█████╗  ██║     █████╗  ██║        ██║   ██║   ██║██████╔╝ */
    /* ╚════██║██╔══╝  ██║     ██╔══╝  ██║        ██║   ██║   ██║██╔══██╗ */
    /* ███████║███████╗███████╗███████╗╚██████╗   ██║   ╚██████╔╝██║  ██║ */
    /* ╚══════╝╚══════╝╚══════╝╚══════╝ ╚═════╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝ */
    Glint::WindowContext selectorWindowContext {};
    Glint::WindowCreateInfo selectorWindowCreateInfo {
        .type = Glint::WINDOW_DEFAULT_TYPE,
        .title = "glint",
        .width = WND_WIDTH,
        .height = WND_HEIGHT
    };
    Glint::createWindow(selectorWindowContext, selectorWindowCreateInfo);
    
    try { Glint::runWindow(selectorWindowContext); } catch (std::exception& ex) { fmt::print(fmt::fg(fmt::color::blue) | fmt::emphasis::bold, ex.what()); return -1; }

    wallpaperWindow_thread.join();

    return 0;
}