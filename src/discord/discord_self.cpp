#define DISCORDPP_IMPLEMENTATION
#include "discord_self.h"

namespace MyDiscord {
    const uint64_t APPLICATION_ID { 1383446147120631899 };
    const char* SETTINGS_FILE { ASSETS_DIR "settings/settings.json" };
    const char* SETTINGS_ITEM_DISCORD_TOKEN { "discord_token" };
    const char* SETTINGS_ITEM_DISCORD_REFRESH_TOKEN { "discord_refresh_token" };
    const char* SETTINGS_ITEM_DISCORD_TOKEN_EXPIRED { "discord_token_expired" };

    void ensureSettingsFileExists() {
        if (!std::filesystem::exists(SETTINGS_FILE)) {
            std::filesystem::create_directories(ASSETS_DIR "settings/");
            
            nlohmann::json settingsJSON;
            settingsJSON[SETTINGS_ITEM_DISCORD_TOKEN] = "";
            settingsJSON[SETTINGS_ITEM_DISCORD_REFRESH_TOKEN] = "";
            settingsJSON[SETTINGS_ITEM_DISCORD_TOKEN_EXPIRED] = 0;
            
            std::ofstream output(SETTINGS_FILE);
            if (output.is_open()) {
                output << settingsJSON.dump(4);
                output.close();
            }
        }
    }

    struct TokenInformation {
        std::string token;
        std::string refreshToken;
        time_t expired;
    };

    TokenInformation getOAuthToken() {
        ensureSettingsFileExists();

        std::ifstream file(SETTINGS_FILE);
        nlohmann::json settingsJSON;
        file >> settingsJSON;
        return TokenInformation {
            .token = settingsJSON[SETTINGS_ITEM_DISCORD_TOKEN],
            .refreshToken = settingsJSON[SETTINGS_ITEM_DISCORD_REFRESH_TOKEN],
            .expired = settingsJSON[SETTINGS_ITEM_DISCORD_TOKEN_EXPIRED]
        };
    }

    void saveOAuthToken(std::string& token, std::string& refreshToken, int32_t expiresIn) {
        std::ifstream input(SETTINGS_FILE);
        nlohmann::json settingsContentJSON;
        
        if (input.is_open()) {
            input >> settingsContentJSON;
            input.close();
        }
        
        constexpr time_t SEVEN_DAYS_IN_SECONDS = 7 * 24 * 60 * 60;
        time_t tokenExpiredTime = time(nullptr) + SEVEN_DAYS_IN_SECONDS;

        settingsContentJSON[SETTINGS_ITEM_DISCORD_TOKEN] = token;
        settingsContentJSON[SETTINGS_ITEM_DISCORD_REFRESH_TOKEN] = refreshToken;
        settingsContentJSON[SETTINGS_ITEM_DISCORD_TOKEN_EXPIRED] = tokenExpiredTime;
        
        std::ofstream output(SETTINGS_FILE);
        output << settingsContentJSON.dump(4);
        output.close();    
    }

    void oAuthDiscordClient(discordpp::Client* client) {
        auto codeVerifier = client->CreateAuthorizationCodeVerifier();

        discordpp::AuthorizationArgs args{};
        args.SetClientId(APPLICATION_ID);
        args.SetScopes(discordpp::Client::GetDefaultPresenceScopes());
        args.SetCodeChallenge(codeVerifier.Challenge());

        client->Authorize(args, [client, codeVerifier](auto result, auto code, auto redirectUri) {
            if (!result.Successful()) {
                return;
            } else {
                client->GetToken(APPLICATION_ID, code, codeVerifier.Verifier(), redirectUri, [client](discordpp::ClientResult result, std::string accessToken, std::string refreshToken, discordpp::AuthorizationTokenType tokenType, int32_t expiresIn, std::string scopes) {
                    saveOAuthToken(accessToken, refreshToken, expiresIn);
                    client->UpdateToken(discordpp::AuthorizationTokenType::Bearer,  accessToken, [client](auto result) {
                        if(result.Successful()) {
                            client->Connect();
                        }
                    });
                });
            }
        });
    }

    void initDiscordClient() {
        auto client = std::make_shared<discordpp::Client>();
        client->SetStatusChangedCallback([client](discordpp::Client::Status status, discordpp::Client::Error error, int32_t errorDetail) {
            if (status == discordpp::Client::Status::Ready) {
                discordpp::Activity activity;
                activity.SetType(discordpp::ActivityTypes::Playing);
                activity.SetState("Browsing beautiful wallpapers");

                client->UpdateRichPresence(activity, [](discordpp::ClientResult result) {
                    if(result.Successful()) {}
                });
            }
        });

        TokenInformation tokenInformation = getOAuthToken();
        if (!tokenInformation.token.empty()) {
            time_t currentTime = time(nullptr);
            if (tokenInformation.expired < currentTime) {
                client->RefreshToken(APPLICATION_ID, tokenInformation.refreshToken, [client](discordpp::ClientResult result, std::string accessToken, std::string refreshToken, discordpp::AuthorizationTokenType tokenType, int32_t expiresIn, std::string scopes) {
                    saveOAuthToken(accessToken, refreshToken, expiresIn);
                    client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken, [client](auto result) {
                        if (result.Successful()) {
                            client->Connect();
                        }
                    });
                });
            } else {
                client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, tokenInformation.token, [client](auto result) {
                    if (result.Successful()) {
                        client->Connect();
                    }
                });
            }
        } else {
            oAuthDiscordClient(client.get());
        }
        
    }

    void handleDiscordEvents() {
        discordpp::RunCallbacks();
    }
}