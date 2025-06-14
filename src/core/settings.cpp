#include "settings.h"

namespace Glint {
    void ensureSettingsFileExists() {
        if (!std::filesystem::exists(SETTINGS_FILE)) {
            std::filesystem::create_directories(ASSETS_DIR "settings/");
            
            json settingsJSON;
            settingsJSON[SETTINGS_ITEM_DISCORD_TOKEN] = "";
            settingsJSON[SETTINGS_ITEM_DISCORD_REFRESH_TOKEN] = "";
            settingsJSON[SETTINGS_ITEM_DISCORD_TOKEN_EXPIRED] = 0;
            settingsJSON[SETTINGS_ITEM_IMAGES_DIR] = "";
            settingsJSON[SETTINGS_ITEM_VIDEOS_DIR] = "";
            
            std::ofstream output(SETTINGS_FILE);
            if (output.is_open()) {
                output << settingsJSON.dump(4);
                output.close();
            }
        }
    }

    void openSettingsFile(json& settingsJSON) {
        std::ifstream file(SETTINGS_FILE);
        file >> settingsJSON;
        file.close();
    }

    void saveSettings(json& settingsJSON) {
        std::ofstream output(SETTINGS_FILE);
        output << settingsJSON.dump(4);
        output.close();
    }
}