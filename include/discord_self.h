#pragma once
#include <atomic>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <ctime>
#include "settings.h"
#include "discordpp.h"

namespace MyDiscord {
    struct TokenInformation {
        std::string token;
        std::string refreshToken;
        time_t expired;
    };

    void initDiscordClient();
    void handleDiscordEvents();
}