#pragma once
#include <atomic>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <ctime>
#include "discordpp.h"
#include "json.hpp"

namespace MyDiscord {
    void initDiscordClient();
    void handleDiscordEvents();
}