#define DISCORDPP_IMPLEMENTATION
#include "discord_self.h"

namespace MyDiscord {
    const uint64_t APPLICATION_ID { 1383446147120631899 };
    const char* ACTIVITY_DESCRIPTION { "Browsing beautiful wallpapers" };

    TokenInformation getOAuthToken() {
        Glint::ensureSettingsFileExists();

        json settings;
        Glint::openSettingsFile(settings);
        return TokenInformation {
            .token = Glint::getSettingValue<std::string>(settings, Glint::SETTINGS_ITEM_DISCORD_TOKEN),
            .refreshToken = Glint::getSettingValue<std::string>(settings, Glint::SETTINGS_ITEM_DISCORD_REFRESH_TOKEN),
            .expired = Glint::getSettingValue<time_t>(settings, Glint::SETTINGS_ITEM_DISCORD_TOKEN_EXPIRED)
        };
    }

    void saveOAuthToken(std::string& token, std::string& refreshToken, int32_t expiresIn) {
        json settings;
        Glint::openSettingsFile(settings);
        
        constexpr time_t SEVEN_DAYS_IN_SECONDS = 7 * 24 * 60 * 60;
        time_t tokenExpiredTime = time(nullptr) + SEVEN_DAYS_IN_SECONDS;

        Glint::setSettingValue<std::string>(settings, Glint::SETTINGS_ITEM_DISCORD_TOKEN, token);
        Glint::setSettingValue<std::string>(settings, Glint::SETTINGS_ITEM_DISCORD_REFRESH_TOKEN, refreshToken);
        Glint::setSettingValue<time_t>(settings, Glint::SETTINGS_ITEM_DISCORD_TOKEN_EXPIRED, tokenExpiredTime);
        
        Glint::saveSettings(settings);    
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
                activity.SetState(ACTIVITY_DESCRIPTION);

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