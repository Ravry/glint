#pragma once
#include <string_view>
#include <filesystem>
#include <fstream>
#include "json.hpp"

using nlohmann::json;

namespace Glint {
    static const char* SETTINGS_FILE { ASSETS_DIR "settings/settings.json" };
    static const char* SETTINGS_ITEM_DISCORD_TOKEN { "discord_token" };
    static const char* SETTINGS_ITEM_DISCORD_REFRESH_TOKEN { "discord_refresh_token" };
    static const char* SETTINGS_ITEM_DISCORD_TOKEN_EXPIRED { "discord_token_expired" };
    static const char* SETTINGS_ITEM_IMAGES_DIR { "images_dir" };
    static const char* SETTINGS_ITEM_VIDEOS_DIR { "videos_dir" };

    void ensureSettingsFileExists();
    void openSettingsFile(nlohmann::json& settingsJSON);

    
    template <typename T> T getSettingValue(json& settingsJSON, const char* identifier) {
        T result = settingsJSON[identifier].get<T>();
        return result;
    }

    template <typename T> void setSettingValue(json& settingsJSON, const char* identifier, T value) {
        settingsJSON[identifier] = value;
    }

    void saveSettings(nlohmann::json& settingsJSON);
}